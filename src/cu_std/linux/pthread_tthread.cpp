#include "tthread.h"
#include "libload.h"
#include "iintrin.h"
#include "aallocator.h"

#include "pthread.h"
#include "semaphore.h"

#include "ttimer.h"

void TWaitSemaphore(TSemaphore sem,f32 time_ms){
    auto time =  MsToTimespec(time_ms);
    sem_timedwait(sem,&time); 
}

void TWaitSemaphore(TSemaphore sem){
    sem_wait(sem);
}

TThreadID TGetThisThreadID(){
    return pthread_self();
}

void TSetThreadAffinity(u32 cpu_mask,TThreadID id){
    
    _kill("mask of 0 used\n",!cpu_mask);
    
    if(!id){
        id = TGetThisThreadID();
    }
    
    TThreadID threadid = id;
    
    u32 count = 32 - BSR(cpu_mask);
    
    cpu_set_t cpuset;
    
    CPU_ZERO(&cpuset);
    
    for(u32 i = 0; i < count; i++){
        
        if( (1 << i) & cpu_mask){
            CPU_SET(i, &cpuset);
        }
    }
    
    auto res = pthread_setaffinity_np(threadid,sizeof(cpuset),&cpuset);
    
    _kill("failed to set affinity\n",res);
    
}

TThreadContext TCreateThread(s64 (*call_fptr)(void*),u32 stack_size,void* args){
    
    TThreadContext context;
    pthread_attr_t attr;
    
    pthread_attr_init(&attr);
    
    u32 err = pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    
    _kill("failed to set detached\n",err != 0);
    
    err = pthread_attr_setstacksize(&attr,stack_size);
    
    _kill("failed to set stack size\n",err != 0);
    
    err = pthread_create(&context.handle,&attr,(void*(*)(void*))((void*)call_fptr),args);
    
    pthread_attr_destroy(&attr);
    
    _kill("failed to create thread\n",err != 0);
    
    return context;
}

TSemaphore TCreateSemaphore(u32 value){
    
    TSemaphore sem = (TSemaphore)alloc(sizeof(sem_t));
    sem_init(sem,0,value);
    
    return sem;
}

void TDestroySemaphore(TSemaphore sem){
    sem_destroy(sem);
    unalloc(sem);
}

void TSignalSemaphore(TSemaphore sem){
    sem_post(sem);
}


void TSetThreadPriority(TSchedulerPoicy policy,f32 priority,TLinuxSchedulerDeadline deadline,TThreadID id){
    
    if(!id){
        id = TGetThisThreadID();
    }
    
    _kill("not supported yet\n",policy == TSCHED_LINUX_POLICY_REALTIME_DEADLINE);
    _kill("priority out of range\n",policy > 1.0f || policy < 0.0f);
    
    sched_param sched = {};
    
    auto min = sched_get_priority_min(policy);
    auto max = sched_get_priority_max(policy);
    
    f32 len = (f32)(max - min);
    
    sched.sched_priority = (s32)(priority * len) + min;
    
    
    auto ret = pthread_setschedparam(id,(s32)policy,&sched);
    
    _kill("call failed\n",ret);
    
}

void TGetThreadPriority(TSchedulerPoicy* pl,f32* pr,TLinuxSchedulerDeadline* dl,TThreadID id){
    
    if(!id){
        id = TGetThisThreadID();
    }
    
    s32 policy = 0;
    sched_param sched = {};
    
    auto ret = pthread_getschedparam(id,&policy,&sched);
    
    _kill("call failed\n",ret);
    
    if(pl){
        *pl = (TSchedulerPoicy)policy;
    }
    
    if(pr){
        
        auto min = sched_get_priority_min(policy);
        auto max = sched_get_priority_max(policy);
        
        if(!max){
            *pr = 0.0f;
        }
        
        else{
            
            f32 len = (f32)(max - min);
            *pr = (f32)(sched.sched_priority)/len;
            
            _kill("priority out of range\n",policy > 1.0f || policy < 0.0f);
        }
        
    }
    
}