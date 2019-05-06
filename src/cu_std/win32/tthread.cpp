
#include "tthread.h"
#include "aallocator.h"

struct InternalArgs{
    s64 (*threadproc)(void*);
    void* args;
};

DWORD WINAPI InternalThreadProc(void* args){
    
    auto internal = *((InternalArgs*)args);
    
    unalloc(args);
    
    internal.threadproc(internal.args);
    
    return 0;
}

TThreadContext TCreateThread(s64 (*call_fptr)(void*),u32 stack_size,void* args){
    
    auto intargs = (InternalArgs*)alloc(sizeof(InternalArgs));
    *intargs = {call_fptr,args};
    
    return {CreateThread(0,stack_size,InternalThreadProc,intargs,0,0)};
}

TSemaphore TCreateSemaphore(u32 value){
    return CreateSemaphore(0,value,value + 1000,0);
}

void TSignalSemaphore(TSemaphore sem){
    ReleaseSemaphore(sem,1,0);
}

void TWaitSemaphore(TSemaphore sem){
    WaitForSingleObject(sem,INFINITE);
}

void TWaitSemaphore(TSemaphore sem,f32 time_ms){
    WaitForSingleObject(sem,(DWORD)time_ms);
}



ThreadID TGetThisThreadID(){
    return GetCurrentThreadId();
}

void TSetThreadAffinity(u32 cpu_mask){
    
    ThreadID threadid = TGetThisThreadID();
    
    auto handle = OpenThread(THREAD_SET_LIMITED_INFORMATION  |
                             THREAD_QUERY_LIMITED_INFORMATION ,false,threadid);
    
    _kill("failed to get handle in affinity\n",!handle);
    
    
    auto res = SetThreadAffinityMask(handle,cpu_mask);
    
    _kill("failed to set affinity\n",!res);
    
    res = CloseHandle(handle);
    
    _kill("failed to close in thread affinity\n",!res);
}

void TSetThreadPriority(TSchedulerPoicy policy,f32 priority,TLinuxSchedulerDeadline deadline){
    
    _kill("priority out of range\n",policy > 1.0f || policy < 0.0f);
    
    switch(policy){
        case TSCHED_POLICY_STATIC_IDLE:{
            
            TSchedulerPoicy array[] = {
                TSCHED_POLICY_STATIC_IDLE,
                TSCHED_WIN32_POLICY_STATIC_LOWEST,
                TSCHED_WIN32_POLICY_STATIC_BELOW_NORMAL,
            };
            
            u32 range = _arraycount(array) - 1;
            policy = array[(u32)((f32)range * priority)];
            
        }break;
        
        case TSCHED_POLICY_STATIC_NORMAL:{
            
            TSchedulerPoicy array[] = {
                TSCHED_POLICY_STATIC_NORMAL,
                TSCHED_WIN32_POLICY_STATIC_ABOVE_NORMAL,
                TSCHED_WIN32_POLICY_STATIC_HIGHEST,
                TSCHED_WIN32_POLICY_STATIC_TIME_CRITICAL
            };
            
            u32 range = _arraycount(array) - 1;
            policy = array[(u32)((f32)range * priority)];
        }break;
    }
    
    
    auto res = SetThreadPriority(GetCurrentThread(),(s32)policy);
    
    _kill("failed to set priority\n",!res);
}

void TGetThreadPriority(TSchedulerPoicy* policy,f32* priority,TLinuxSchedulerDeadline* deadline){
    
    auto res = GetThreadPriority(GetCurrentThread());
    
    _kill("failed to get priority\n",res == THREAD_PRIORITY_ERROR_RETURN);
    
    if(policy){
        *policy = (TSchedulerPoicy)res;
    }
    
}