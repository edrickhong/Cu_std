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





TThreadContext TCreateThread(s64(*call_fptr)(void*),u32 stack_size,void* args);

TSemaphore TCreateSemaphore(u32 value = 0);

void TDestroySemaphore(TSemaphore sem);

void TSignalSemaphore(TSemaphore sem);

void TWaitSemaphore(TSemaphore sem);

void TWaitSemaphore(TSemaphore sem,f32 time_ms);

ThreadID TGetThisThreadID();

void TSetThreadAffinity(u32 cpu_mask);

void TRevertThreadPriority();
void TSetThreadPriority();
void TGetThreadPriority();