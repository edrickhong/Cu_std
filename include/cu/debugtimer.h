#pragma once

#include "ttimer.h"

//MARK:
#include "tthread.h"

#include "ccolor.h"

#if _enable_gui

#ifdef DEBUG

#define _enable_debugtimer 1

#endif

#else

#define _enable_debugtimer 0

#endif

#if (_enable_debugtimer)

#define TIMEBLOCK(COLOR) TimeBlock t_##__LINE__((const s8*)__FILE__,__LINE__,(const s8*)__FUNCTION__,COLOR)

#define TIMEBLOCKTAGGED(NAME,COLOR) TimeBlock t_##__LINE__((const s8*)__FILE__,__LINE__,(const s8*)NAME,COLOR)

#define MASTERTIMEBLOCKSTART(COLOR) MasterTimeBlock t_##__LINE__((const s8*)__FILE__,__LINE__,(const s8*)"MASTERTIMEBLOCK",COLOR)

#define EXECTIMEBLOCK(COLOR) ExecTimeBlock t_##__LINE__((const s8*)__FILE__,__LINE__,(const s8*)"EXECBLOCK",COLOR)

#define RECORDTHREAD() RecordThread()

#define BUILDGUIGRAPH(to_draw) BuildGraph(to_draw);

#define PRINTBLOCKS() PrintEntries()

#define PRINTTIMEBLOCK() PrintTimeBlock t_##__LINE__((const s8*)__FILE__,__LINE__,(const s8*)__FUNCTION__,White)

#define PRINTTIMEBLOCKTAGGED(NAME) PrintTimeBlock t_##__LINE__((const s8*)__FILE__,__LINE__,(const s8*)NAME,White)

#define PAUSEGRAPH() PauseTimer()

#define INIT_DEBUG_TIMER() InitDebugTimer()

#define DEBUGTIMERGETCONTEXT() DebugTimerGetContext()
#define DEBUGTIMERSETCONTEXT(context) DebugTimerSetContext(context)


#else

#define RECORDTHREAD()

#define TIMEBLOCK(COLOR)
#define TIMEBLOCKTAGGED(NAME,COLOR)
#define MASTERTIMEBLOCKSTART(COLOR)
#define EXECTIMEBLOCK(COLOR)
#define BUILDGUIGRAPH(to_draw)
#define PRINTBLOCKS()
#define PRINTTIMEBLOCK(COLOR)
#define PAUSEGRAPH()
#define INIT_DEBUG_TIMER()
#define DEBUGTIMERGETCONTEXT() 0
#define DEBUGTIMERSETCONTEXT(context)
#define PRINTTIMEBLOCKTAGGED(NAME)
#endif

typedef struct DebugRecord{
    
    Color4 color;
    
    TimeSpec start_stamp;
    f32 timelen;
    u64 cyclelen;
    
    const s8* file;
    const s8* function;
    u32 line;
}DebugRecord;

void InitDebugTimer();

void SubmitRecord(TThreadID tid,DebugRecord record);

void RecordThread();

void SetStartTimeBlock(TimeSpec timestamp);

void SetFrameTime(f32 time);

void SetExecTime(f32 time);

u32 GetThreadIndex(TThreadID tid);

typedef struct TimeBlock{
    
    TimeSpec start_stamp;
    u64 start_cycle;
    
    const s8* file;
    const s8* function;
    u32 line;
    Color4 color;

#ifdef __cplusplus
    
    
    TimeBlock(const s8* File,u32 Line,const s8* Function,Color4 c){
        
        file = File;
        line = Line;
        function = Function;
        color = c;
        
        start_cycle = Rdtsc();
        
        GetTime(&start_stamp);
    }
    
    ~TimeBlock(){
        
        auto end_cycle = Rdtsc();
        
        auto cyclediff = end_cycle - start_cycle;
        
        TimeSpec end;  
        
        GetTime(&end);
        
        auto len = GetTimeDifferenceMS(start_stamp,end);
        
        SubmitRecord(TGetThisThreadID(),{color,start_stamp,len,cyclediff,file,function,line});
    }
#endif
}TimeBlock;

struct MasterTimeBlock : TimeBlock{
    
    MasterTimeBlock(const s8* File,u32 Line,const s8* Function,Color4 c) :
    TimeBlock(File,Line,Function,c){
        SetStartTimeBlock(start_stamp);
    }
    
    ~MasterTimeBlock(){
        
        TimeSpec end;  
        
        GetTime(&end);
        
        SetFrameTime(GetTimeDifferenceMS(start_stamp,end));
    }
    
};


struct ExecTimeBlock : TimeBlock{
    
    ExecTimeBlock(const s8* File,u32 Line,const s8* Function,Color4 c) :
    TimeBlock(File,Line,Function,c){}
    
    ~ExecTimeBlock(){
        
        TimeSpec end;  
        
        GetTime(&end);
        
        SetExecTime(GetTimeDifferenceMS(start_stamp,end));
    }
    
};

struct PrintTimeBlock : TimeBlock{
    
    PrintTimeBlock(const s8* File,u32 Line,const s8* Function,Color4 c) :
    TimeBlock(File,Line,Function,c){
    }
    
    ~PrintTimeBlock(){
        
        TimeSpec end;  
        
        GetTime(&end);
        
        auto len = GetTimeDifferenceMS(start_stamp,end);
        printf("%s %s %d : %f\n",file,function,line,(f64)len);
    }
};



void BuildGraph(b32 to_draw);


void PrintEntries();

void InitDebugTimer();

void PauseTimer();


struct _cachealign RecordArray{
    DebugRecord array[100];
    
    DebugRecord& operator [](ptrsize index){
        return array[index];
    }
};

struct DebugTable{
    TimeSpec timestamp;
    volatile TThreadID threadid_array[15] = {};//should set this to your threadcount
    volatile u32 thread_count = 0;
    
    //We should cache align these
    // DebugRecord record_array[15][100] = {};
    RecordArray record_array[15];
    u32 recordcount_array[15] = {};
};

void* DebugTimerGetContext();
void DebugTimerSetContext(void* context);
