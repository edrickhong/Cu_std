#pragma once



#include "x86intrin.h"
#include "time.h"

#define Rdtsc __rdtsc

typedef struct timespec TimeSpec;

#define TInitTimer();


void _ainline GetTime(TimeSpec* timespec){
    clock_gettime(CLOCK_MONOTONIC,timespec);
}

#ifdef __cplusplus
extern "C"{
#endif

f32 GetTimeDifferenceMS(TimeSpec,TimeSpec);

#ifdef __cplusplus
}
#endif

TimeSpec _ainline MsToTimespec(f32 time){
    f32 sec = (f32)((u32)(time/1000.0f));
    f32 nsec = (time * 1000000.0f) - (sec * 1000000000.0f);

    TimeSpec spec = {(long)sec,(long)(nsec)};
    
    return spec;
}

void _ainline SleepMS(f32 time){
    
    TimeSpec time1 = MsToTimespec(time);
    
    nanosleep(&time1,0);
}
