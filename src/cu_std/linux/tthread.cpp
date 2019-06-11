#include "tthread.h"
#include "libload.h"
#include "iintrin.h"
#include "aallocator.h"
#include "ttimer.h"

#include "linux/futex.h"
#include "linux_syscall.h"
#include "ssys.h"

#include "linux/sched/types.h"

#define _expected_sem_value 0

void TWaitSemaphore(TSemaphore sem,f32 time_ms){
    
    b32 condition = true;
    
    do{
        u32 err = 0;
        auto time =  MsToTimespec(time_ms);
        _sys_futex(sem,FUTEX_WAIT,_expected_sem_value,&time,0,0,err);
        
        //over here we are woken up
        condition = true;
        
        auto expected = *sem;
        
        if(expected){
            auto actual = LockedCmpXchg(sem,expected,expected - 1);
            condition = actual != expected;
        }
        
        
    }
    while(condition);
}

void TWaitSemaphore(TSemaphore sem){
    
    b32 condition = true;
    
    do{
        u32 err = 0;
        _sys_futex(sem,FUTEX_WAIT,_expected_sem_value,0,0,0,err);
        
        //over here we are woken up
        condition = true;
        
        auto expected = *sem;
        
        if(expected){
            auto actual = LockedCmpXchg(sem,expected,expected - 1);
            condition = actual != expected;
        }
        
        
    }
    while(condition);
    
}

TThreadID TGetThisThreadID(){
    TThreadID id = 0;
    _sys_gettid(id);
    return id;
}

void TSetThreadAffinity(u32 cpu_mask,TThreadID id = 0){
    
    _kill("mask of 0 used\n",!cpu_mask);
    
    u32 err = 0;
    _sys_setaffinity(id,sizeof(cpu_mask),&cpu_mask,err);
    
    _kill("failed to set affinity\n",!err);
}

extern "C" void Linux_ThreadProc();

TThreadContext TCreateThread(s64 (*call_fptr)(void*),u32 stack_size,void* args){
    
    void* stack = 0;
    {
        
        _sys_mmap(0,stack_size,MEMPROT_READWRITE,MAP_ANONYMOUS | MAP_PRIVATE | MAP_GROWSDOWN,-1,0,stack);
        
        auto s = 
            (u64*)(((s8*)stack) + stack_size); //suppose to be 16 byte aligned
        
        s--;
        *s = (u64)stack;
        
        s--;
        *s = (u64)stack_size;
        
        s--;
        *s = (u64)call_fptr;
        
        s--;
        *s = (u64)args;
        
        s--;
        *s = (u64)Linux_ThreadProc;
        
        stack = s;
    }
    
    u64 flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_PARENT | CLONE_THREAD | CLONE_IO;
    
    auto tid = _sys_clone(flags,stack,0,0);
    
    return tid;
}

TSemaphore TCreateSemaphore(u32 value){
    auto t = (TSemaphore)alloc(sizeof(u32));
    *t = value;
    return t;
}

void TDestroySemaphore(TSemaphore sem){
    unalloc((void*)sem);
}

void TSignalSemaphore(TSemaphore sem){
    
    LockedIncrement(sem);
    
    u32 err = 0;
    _sys_futex(sem,FUTEX_WAKE,1,0,0,0,err);
}

#if 0

struct sched_attr{
    __u32 size;
    
    __u32 sched_policy;
    __u64 sched_flags;
    
    /* SCHED_NORMAL, SCHED_BATCH */
    __s32 sched_nice;
    
    /* SCHED_FIFO, SCHED_RR */
    __u32 sched_priority;
    
    /* SCHED_DEADLINE */
    __u64 sched_runtime;
    __u64 sched_deadline;
    __u64 sched_period;
};

#endif


void TGetThreadPriority(TSchedulerPoicy* pl,f32* pr,TLinuxSchedulerDeadline* dl,TThreadID id){
    
    u32 err = 0;
    s32 policy = 0;
    sched_param sched = {};
    
    _sys_sched_getscheduler(id,policy);
    _kill("failed to get policy",policy < 0);
    
    if(policy == SCHED_DEADLINE){
        
        sched_attr attr = {};
        
        _sys_sched_getattr(id,&attr,sizeof(sched_attr),0,err);
        _kill("failed to get deadline",err);
        
        if(dl){
            dl->runtime = attr.sched_runtime;
            dl->deadline = attr.sched_deadline;
            dl->period = attr.sched_period;
            dl->flags = attr.sched_flags;
        }
        
        
        
    }
    else{
        
        _sys_sched_getparam(id,&sched,err);
        _kill("failed to get param",err);
        
        if(pl){
            *pl = (TSchedulerPoicy)policy;
        }
        
        if(pr){
            
            auto min = _syscall_sched_get_priority_min(policy);
            auto max = _syscall_sched_get_priority_max(policy);
            
            if(!max){
                *pr = 0.0f;
            }
            
            else{
                
                f32 len = (f32)(max - min);
                *pr = (f32)(sched.sched_priority)/len;
                
                _kill("priority out of range\n",(*pr) > 1.0f || (*pr) < 0.0f);
            }
            
        }
        
    }
    
}

void TSetThreadPriority(TSchedulerPoicy policy,f32 priority,TLinuxSchedulerDeadline deadline,TThreadID id){
    
    _kill("priority out of range\n",policy > 1.0f || policy < 0.0f);
    
    if(policy == TSCHED_LINUX_POLICY_REALTIME_DEADLINE){
        
        sched_attr attr = {sizeof(sched_attr),policy,deadline.flags,0,0,deadline.runtime,deadline.deadline,deadline.period};
        
        u32 err = 0;
        
        _sys_sched_setattr(id,&attr,0,err);
        
        _kill("call failed\n",err);
        
    }
    
    else{
        sched_param sched = {};
        
        auto min = _syscall_sched_get_priority_min(policy);
        auto max = _syscall_sched_get_priority_max(policy);
        
        f32 len = (f32)(max - min);
        
        sched.sched_priority = (s32)(priority * len) + min;
        
        u32 err = 0;
        
        _sys_sched_setscheduler(id,policy,&sched,err);
        
        _kill("call failed\n",err);
    }
}