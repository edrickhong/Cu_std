#pragma once

#ifdef _WIN32

#include "iintrin.h"
#include "Windows.h"

struct TThreadContext{
    HANDLE handle;
};

typedef HANDLE TSemaphore;
typedef DWORD ThreadID;

#else

#include "pthread.h"
#include "semaphore.h"

struct TThreadContext{
    pthread_t handle;
};

typedef sem_t* TSemaphore;

typedef pthread_t ThreadID;

#endif


enum TSchedulerPoicy{
    
#ifdef _WIN32
    
    TSCHED_POLICY_STATIC_IDLE,
    TSCHED_POLICY_STATIC_NORMAL,
    
    TSCHED_WIN32_POLICY_STATIC_LOWEST = THREAD_PRIORITY_LOWEST,
    TSCHED_WIN32_POLICY_STATIC_BELOW_NORMAL = THREAD_PRIORITY_BELOW_NORMAL,
    TSCHED_WIN32_POLICY_STATIC_ABOVE_NORMAL = THREAD_PRIORITY_ABOVE_NORMAL,
    TSCHED_WIN32_POLICY_STATIC_HIGHEST = THREAD_PRIORITY_HIGHEST,
    TSCHED_WIN32_POLICY_STATIC_TIME_CRITICAL = THREAD_PRIORITY_TIME_CRITICAL,
    
#else
    
    TSCHED_POLICY_STATIC_IDLE = SCHED_IDLE,
    TSCHED_POLICY_STATIC_NORMAL = SCHED_OTHER,
    
    TSCHED_LINUX_POLICY_STATIC_BATCH = SCHED_BATCH,
    TSCHED_LINUX_POLICY_REALTIME_FIFO = SCHED_FIFO,
    TSCHED_LINUX_POLICY_REALTIME_RR = SCHED_RR,
    
    //TODO: implement this when we overhaul everything to not use pthreads
    TSCHED_LINUX_POLICY_REALTIME_DEADLINE = SCHED_DEADLINE,
    
#endif
    
};

struct TLinuxSchedulerDeadline{
    u64 sched_runtime;
    u64 sched_deadline;
    u64 sched_period;
};


TThreadContext TCreateThread(s64(*call_fptr)(void*),u32 stack_size,void* args);

TSemaphore TCreateSemaphore(u32 value = 0);

void TDestroySemaphore(TSemaphore sem);

void TSignalSemaphore(TSemaphore sem);

void TWaitSemaphore(TSemaphore sem);

void TWaitSemaphore(TSemaphore sem,f32 time_ms);

ThreadID TGetThisThreadID();

void TSetThreadAffinity(u32 cpu_mask);

void TSetThreadPriority(TSchedulerPoicy policy,f32 priority,TLinuxSchedulerDeadline deadline = {});

void TGetThreadPriority(TSchedulerPoicy* policy,f32* priority,TLinuxSchedulerDeadline* deadline = 0);