#pragma once
#include "iintrin.h"
#include "debugtimer.h"

typedef void(*WorkProc)(void*,void*); // function args, threadcontext


typedef volatile u32 SpinMutex;
typedef SpinMutex EntryMutex;

struct ThreadWorkEntry{
    WorkProc workcall;
    void* data;
};

struct ThreadWorkQueue{
    volatile ThreadWorkEntry buffer[32];
    volatile  _cachealign u32 count = 0;
    volatile  _cachealign u32 index = 0;
    volatile  _cachealign u32 completed_count = 0;
};

void _ainline Clear(ThreadWorkQueue* queue){
    queue->count = 0;
    queue->index = 0;
    queue->completed_count = 0;
}


void PushThreadWorkQueue(ThreadWorkQueue* queue,WorkProc proc,void* data,
                         TSemaphore sem);

b32 ExecuteThreadWorkQueue(ThreadWorkQueue* queue,void* thread_data);

void inline MainThreadDoWorkQueue(ThreadWorkQueue* queue,void* thread_data){
    
    TIMEBLOCK(Green);
    
    while(queue->completed_count != queue->count){
        ExecuteThreadWorkQueue(queue,thread_data);
    }
    
}



void _ainline TSpinLock(SpinMutex* mutex){
    
    TIMEBLOCK(LimeGreen);
    
    SpinLock_tryagain:  
    
    u32 value = LockedCmpXchg(mutex,0,1);
    
    if(value){
        
        while(*mutex){
            _mm_pause();
        }
        
        goto SpinLock_tryagain;
    }
    
    
    
    *mutex = 1;
}

void _ainline TSpinUnlock(SpinMutex* mutex){
    (*mutex) = 0;
}

//NOTE: only allows a single thread to enter at a time to run a proc
void TSingleEntryLock(EntryMutex* mutex,WorkProc proc,void* args,void* threadcontext);

void _ainline TSingleEntryUnlock(EntryMutex* mutex){
    (*mutex) = 0;
}


u32 TGetEntryIndex(volatile u32* cur_index);
u32 TGetEntryIndexD(volatile u32* cur_index,u32 max_count);

u32 TGetEntryOffset(volatile u32* cur_offset,u32 size);
u32 TGetEntryOffsetD(volatile u32* cur_offset,u32 size,u32 max_size);

u32 TGetEntryAlignedOffset(volatile u32* cur_offset,u32 size,u32 alignment);
u32 TGetEntryAlignedOffsetD(volatile u32* cur_offset,u32 size,u32 alignment,u32 max_size);

#ifndef DEBUG

#define TGetEntryIndexD(a,b) TGetEntryIndex(a)
#define TGetEntryOffsetD(a,b,c) TGetEntryOffset(a,b)
#define TGetEntryAlignedOffsetD(a,b,c,d) TGetEntryAlignedOffset(a,b,c)

#endif