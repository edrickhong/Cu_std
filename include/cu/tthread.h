#pragma once

#ifdef _WIN32

#include "iintrin.h"
#include "Windows.h"

typedef struct TThreadContext{
    HANDLE handle;
}TThreadContext;

typedef HANDLE TSemaphore;
typedef DWORD TThreadID;

#else

#include "thread_mode.h"

#if _use_pthread

#include "pthread.h"
#include "semaphore.h"
#include "linux/sched.h"

typedef struct TThreadContext{
    pthread_t handle;
}TThreadContext;

typedef sem_t* TSemaphore;
typedef pthread_t TThreadID;

#else

#include "linux/sched.h"

typedef u32 TThreadContext;
typedef TThreadContext TThreadID;
typedef volatile _cachealign u32* TSemaphore;

#endif

#endif


typedef enum TSchedulerPoicy{
    
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
    TSCHED_POLICY_STATIC_NORMAL = SCHED_NORMAL,
    
    TSCHED_LINUX_POLICY_STATIC_BATCH = SCHED_BATCH,
    TSCHED_LINUX_POLICY_REALTIME_FIFO = SCHED_FIFO,
    TSCHED_LINUX_POLICY_REALTIME_RR = SCHED_RR,
    
    
    //NOTE: this is not posix standard and is only available on Linux 3.14 and above
    TSCHED_LINUX_POLICY_REALTIME_DEADLINE = SCHED_DEADLINE,
    
#endif
    
}TSchedulerPoicy;

typedef struct TLinuxSchedulerDeadline{
    u64 runtime;
    u64 deadline;
    u64 period;
    
    //SCHED_FLAG_RECLAIM or SCHED_FLAG_DL_OVERRUN
    u32 flags;
}TLinuxSchedulerDeadline;


TThreadContext TCreateThread(s64(*call_fptr)(void*),u32 stack_size,void* args);

TSemaphore TCreateSemaphore(u32 value);

void TDestroySemaphore(TSemaphore sem);

void TSignalSemaphore(TSemaphore sem);

void TWaitSemaphore(TSemaphore sem);

void TWaitSemaphoreTimed(TSemaphore sem,f32 time_ms);

TThreadID TGetThisThreadID();

void TSetThreadAffinity(u32 cpu_mask,TThreadID id);

void TSetThreadPriority(TSchedulerPoicy policy,f32 priority,TLinuxSchedulerDeadline deadline,TThreadID id);

void TGetThreadPriority(TSchedulerPoicy* policy,f32* priority,TLinuxSchedulerDeadline* deadline,TThreadID id);
