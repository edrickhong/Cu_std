

#ifdef TGetEntryIndexD
#undef TGetEntryIndexD
#undef TGetEntryOffsetD
#undef TGetEntryAlignedOffsetD
#endif

void ThreadSubmit(TSemaphore sem,u32 threads){
    
    for(u32 i = 0; i < threads;i++){
        TSignalSemaphore(sem);  
    }
    
}


void PushThreadWorkQueue(ThreadWorkQueue* queue,WorkProc proc,void* data,
                         TSemaphore sem){
    
    queue->buffer[queue->count].workcall = proc;
    queue->buffer[queue->count].data = data;
    queue->count++;
    TSignalSemaphore(sem);
}

b32 ExecuteThreadWorkQueue(ThreadWorkQueue* queue,void* thread_data){
    
    
    u32 index = queue->index;
    
    if(index >= queue->count){
        return false;
    }
    
    TIMEBLOCK(Magenta);
    
    u32 actual_index = LockedCmpXchg(&queue->index,index,index + 1);
    
    if(actual_index == index && index < queue->count){
        
        _kill("work entry contains no call. we should be worried and start tracing\n",
              !queue->buffer[index].workcall);
        
        queue->buffer[index].workcall(queue->buffer[index].data,thread_data);
        
        LockedIncrement(&queue->completed_count);
        
    }
    
    return true;
}

u32 TGetEntryOffset(volatile u32* cur_offset,u32 size){
    
    u32 expected_offset = 0;
    u32 actual_offset = 0;
    
    do{
        
        expected_offset = *cur_offset;
        actual_offset = LockedCmpXchg(cur_offset,expected_offset,expected_offset + size);
        
    }while(expected_offset != actual_offset);
    
    return actual_offset;
    
}

u32 TGetEntryOffsetD(volatile u32* cur_offset,u32 size,u32 max_size){
    
    auto ret = TGetEntryOffset(cur_offset,size);
    
    _kill("exceeded max size\n",ret + size > max_size);
    return ret;
}


u32 TGetEntryAlignedOffset(volatile u32* cur_offset,u32 size,u32 alignment){
    
    u32 expected_offset = 0;
    u32 actual_offset = 0;
    u32 padsize = 0;
    
    do{
        
        expected_offset = *cur_offset;
        padsize = _alignpow2(expected_offset,alignment) - expected_offset;
        actual_offset = LockedCmpXchg(cur_offset,expected_offset,expected_offset + size + padsize);
        
    }while(expected_offset != actual_offset);
    
    return actual_offset + padsize;
}

u32 TGetEntryAlignedOffsetD(volatile u32* cur_offset,u32 size,u32 alignment,u32 max_size){
    
    auto ret = TGetEntryAlignedOffset(cur_offset,size,alignment);
    
    _kill("exceeded max size\n",ret + size > max_size);
    
    return ret;
}


u32 TGetEntryIndex(volatile u32* cur_index){
    
    u32 expected_count = 0;
    u32 actual_count = 0;
    
    do{
        
        expected_count = *cur_index;
        
        actual_count = LockedCmpXchg(cur_index,expected_count,expected_count + 1);
        
    }while(expected_count != actual_count);
    
    return actual_count;
}

u32 TGetEntryIndexD(volatile u32* cur_index,u32 max_count){
    
    auto ret = TGetEntryIndex(cur_index);
    
    _kill("exceeded max count\n",ret >= max_count);
    
    return ret;
}

void TSingleEntryLock(EntryMutex* mutex,WorkProc proc,void* args,void* threadcontext){
    
    if((*mutex)){
        return;
    }
    
    auto is_locked = (*mutex);
    
    u32 actual_islocked = LockedCmpXchg(mutex,is_locked,is_locked + 1);
    
    if((is_locked != actual_islocked) || is_locked){
        return;
    }
    
    proc(args,threadcontext);
}