#include "iintrin.h"
#include "tthreadx.h"


#ifdef TAlloc
#undef TAlloc
#endif

struct AAllocatorContext{
    
    const s8* frame_ptr = 0;
    volatile u32 curframe_count = 0;
    
#ifdef DEBUG
    
    struct DebugAllocEntry{
        void* ptr;
        TThreadID tid;
        u32 size;
        const s8* file;
        const s8* function;
        u32 line;
    };
    
    u32 maxframe_count = 0;
    
    DebugAllocEntry alloc_array[1024] = {};
    volatile u32 alloc_count = 0;
    
    DebugAllocEntry malloc_array[1024] = {};
    
#endif
};

_global AAllocatorContext* alloc_context = 0;

#ifdef DEBUG

void DebugSubmitTAlloc(void* base_ptr,u32 size,const s8* file,const s8* function,u32 line){
    
    auto actual_count = TGetEntryIndexD(&alloc_context->alloc_count,_arraycount( alloc_context->alloc_array));
    
    for(u32 i = 0; i < actual_count; i++){
        _kill("duplicate submission\n",alloc_context->alloc_array[i].ptr == base_ptr);
    }
    
    alloc_context->alloc_array[actual_count] = {base_ptr,TGetThisThreadID(),size,file,function,line};
    
}

void DebugDumpTAllocs(){
    
    for(u32 i = 0; i < alloc_context->alloc_count; i++){
        auto entry = alloc_context->alloc_array[i];
        
        printf("%d %d %s %s %d: base_ptr %p %d\n",i,(u32)entry.tid,entry.file,entry.function,
               entry.line,entry.ptr,entry.size);
    }
    
}

void DebugPrintTAllocEntry(u32 i){
    
    auto entry = alloc_context->alloc_array[i];
    
    printf("%d %d %s %s %d: base_ptr %p %d\n",i,(u32)entry.tid,entry.file,entry.function,
           entry.line,entry.ptr,entry.size);
}


void DebugSubmitMalloc(void* base_ptr,u32 size,const s8* file,const s8* function,u32 line){
    
    if(!alloc_context){
        return;
    }
    
    u32 total_count = _arraycount(alloc_context->malloc_array);
    AAllocatorContext::DebugAllocEntry* d = 0;
    
    for(u32 i = 0; i < total_count; i++){
        
        auto entry = &alloc_context->malloc_array[i];
        auto ptr = (s64)entry->ptr;
        
        if(ptr == 0){
            
            auto actual_ptr = LockedCmpXchg64((s64 volatile*)&entry->ptr,(s64)ptr,(s64)base_ptr);
            
            if(actual_ptr == ptr){
                d = entry;
                break;
            }
        }
    }
    
    _kill("Could not get a valid entry\n",!d);
    
    *d = {base_ptr,TGetThisThreadID(),size,file,function,line
    };
    
}

void DebugRemoveMalloc(void* base_ptr){
    
    if(!alloc_context){
        return;
    }
    
    u32 total_count = _arraycount(alloc_context->malloc_array);
    
    for(u32 i = 0; i < total_count; i++){
        
        auto entry = &alloc_context->malloc_array[i];
        
        if(entry->ptr == base_ptr){
            entry->ptr = 0;
            return;
        }
        
    }
    
    _kill("invalid free\n",1);
}

u32 DebugDumpTotalMallocSize(){
    
    if(!alloc_context){
        return (u32)-1;
    }
    
    u32 size = 0;
    u32 total_count = _arraycount(alloc_context->malloc_array);
    
    for(u32 i = 0; i < total_count; i++){
        
        auto entry = &alloc_context->malloc_array[i];
        
        if(entry->ptr){
            size += entry->size;
        }
        
    }
    
    return size;
}
u32 DebugDumpTotalMallocCount(){
    
    if(!alloc_context){
        return (u32)-1;
    }
    
    u32 total_count = _arraycount(alloc_context->malloc_array);
    u32 count = 0;
    
    for(u32 i = 0; i < total_count; i++){
        
        auto entry = &alloc_context->malloc_array[i];
        
        if(entry->ptr){
            count++;
        }
        
    }
    
    return count;
}

void DebugDumpMallocs(){
    
    if(!alloc_context){
        return;
    }
    
    u32 total_count = _arraycount(alloc_context->malloc_array);
    
    for(u32 i = 0; i < total_count; i++){
        
        auto entry = alloc_context->malloc_array[i];
        
        if(entry.ptr){
            printf("%d %d %s %s %d: base_ptr %p %d\n",i,(u32)entry.tid,entry.file,entry.function,
                   entry.line,entry.ptr,entry.size);
        }
    }
    
}

void DebugPrintMallocEntry(u32 i){
    
    if(!alloc_context){
        return;
    }
    
    auto entry = alloc_context->malloc_array[i];
    
    printf("%d %d %s %s %d: base_ptr %p %d\n",i,(u32)entry.tid,entry.file,entry.function,
           entry.line,entry.ptr,entry.size);
}


void* DebugMalloc(size_t size,const s8* file,const s8* function,u32 line){
    
    _kill("alloc size cannot be 0\n",!size);
    
    auto ptr = malloc(size);
    DebugSubmitMalloc(ptr,size,file,function,line);
    
    memset(ptr,0,size);
    return ptr;
}
void DebugFree(void* ptr){
    DebugRemoveMalloc(ptr);
    free(ptr);
}


void* DebugRealloc(void* old_ptr,size_t size,const s8* file,const s8* function,u32 line){
    
    _kill("alloc size cannot be 0\n",!size);
    
    u32 total_count = _arraycount(alloc_context->malloc_array);
    for(u32 i = 0; i < total_count; i++){
        
        auto entry = &alloc_context->malloc_array[i];
        
        if(entry->ptr == old_ptr){
            
            auto ptr = realloc(old_ptr,size);
            
            entry->ptr = ptr;
            entry->tid = TGetThisThreadID();
            entry->size = size;
            entry->file = file;
            entry->function = function;
            entry->line = line;
            
            return ptr;
        }
        
    }
    
    return 0;
}

void* DebugTAlloc(u32 size,const s8* file,const s8* function,u32 line){
    
    _kill("alloc size cannot be 0\n",!size);
    
    auto ptr = TAlloc(size);
    
    auto aligned_size = _align16(size);
    
    DebugSubmitTAlloc(ptr,aligned_size,file,function,line);
    
    return ptr;
}

#endif


void* TAlloc(u32 size){
    
    auto aligned_size = _align16(size);
    
    _kill("this happened for some reason\n",aligned_size < size);
    
    //TODO: replace this with get offset
    u32 expected_curframe_count;
    u32 actual_curframe_count;
    u32 new_curframe_count;
    void* ptr;
    
    do{
        
        expected_curframe_count = alloc_context->curframe_count;
        new_curframe_count = alloc_context->curframe_count + aligned_size;
        
        actual_curframe_count = LockedCmpXchg(&alloc_context->curframe_count,expected_curframe_count,
                                              new_curframe_count);
        
        ptr = (s8*)alloc_context->frame_ptr + actual_curframe_count;
        
    }while(expected_curframe_count != actual_curframe_count);
    
    _kill("no more frame stack space\n",(u32)((s8*)ptr - alloc_context->frame_ptr) > (u32)alloc_context->maxframe_count);
    
    memset(ptr,0,aligned_size);
    
    return ptr;
}




void InitTAlloc(u32 size){
    alloc_context->frame_ptr = (s8*)alloc(size);
    
#ifdef DEBUG
    
    alloc_context->maxframe_count = size;
    
#endif
}

void InitInternalAllocator(){
    
    _kill("already allocated\n",alloc_context);
    
    alloc_context = (AAllocatorContext*)alloc(sizeof(AAllocatorContext));
}

void ResetTAlloc(){
    
    u32 expected_curframe_count;
    u32 actual_curframe_count;
    u32 new_curframe_count;
    
    do{
        
        expected_curframe_count = alloc_context->curframe_count;
        new_curframe_count = 0;
        
        actual_curframe_count = LockedCmpXchg(&alloc_context->curframe_count,expected_curframe_count,
                                              new_curframe_count);
        
    }while(expected_curframe_count != actual_curframe_count);
    
#ifdef DEBUG
    alloc_context->alloc_count = 0;
#endif
}


AAllocatorContext* GetAAllocatorContext(){
    return alloc_context;
}
void SetAAllocatorContext(AAllocatorContext* this_context){
    alloc_context = this_context;
}

#ifdef DEBUG

#ifdef VERBOSE
#pragma message ("MALLOC TRACKING ENABLED")
#endif

#if  _enable_allocboundscheck

#ifdef VERBOSE
#pragma message ("TALLOC BOUNDS CHECKING ENABLED")
#endif

#endif

#endif
