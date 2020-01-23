#pragma once
#include "x86intrin.h"
#include "time.h"

#define Rdtsc __rdtsc

typedef timespec TimeSpec;

#define TInitTimer();


void _ainline GetTime(TimeSpec* timespec){
    clock_gettime(CLOCK_MONOTONIC,timespec);
}

f32 GetTimeDifferenceMS(TimeSpec,TimeSpec);

_intern timespec MsToTimespec(f32 time){
    auto sec = (f32)((u32)(time/1000.0f));
    auto nsec = (time * 1000000.0f) - (sec * 1000000000.0f);
    
    return {(long)sec,(long)(nsec)};
}

void _ainline SleepMS(f32 time){
    
    auto time1 = MsToTimespec(time);
    
    nanosleep(&time1,0);
}
