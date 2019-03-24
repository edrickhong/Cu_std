//this is for running tests
#include "stdio.h"
#include "mode.h"
#include "ttype.h"
#include "aallocator.h"
#include "pparse.h"

#include "hash_array.h"

struct HashEntry{
    u64 hash;
    const s8* string_array[1024 * 8];
    u32 string_count = 0;
};

void AddHash(HashEntry* table,u32* c,u64 hash,const s8* string){
    
    auto count = *c;
    
    
    
    for(u32 i = 0; i < count; i++){
        
        auto k = &table[i];
        
        if(k->hash == hash){
            
            k->string_array[k->string_count] = string;
            k->string_count++;
            
            return;
        }
        
    }
    
    table[count].hash = hash;
    table[count].string_array[0] = string;
    table[count].string_count = 1;
    count++;
    
    *c = count;
    
}

b32 PrintHashDiagnostics(HashEntry* table,u32 count){
    
    u32 collide_count = 0;
    
    for(u32 i = 0; i < count; i++){
        
        auto entry = &table[i];
        
        if(entry->string_count > 1){
            
            collide_count += entry->string_count;
            
            printf("HASH COLLISION(%llu): ",entry->hash);
            
            for(u32 j = 0; j < entry->string_count; j++){
                
                printf("%s ",entry->string_array[j]);
                
            }
            
            printf("\n");
            
        }
        
    }
    
    auto percent = ((f64)collide_count/(f64)(_arraycount(hash_array))) * 100.0f;
    
    
    printf("%d collisions out of %d hashes. (%f percent collision rate)\n",collide_count,(u32)_arraycount(hash_array),percent);
    
    return percent > 20.0;
}

b32 HashTest(){
    
    printf("RUNNING PHashString Collision test\n");
    
    auto hash_table = (HashEntry*)alloc(sizeof(HashEntry) * (_arraycount(hash_array)));
    
    u32 hash_count = 0;
    
    for(u32 i = 0; i < _arraycount(hash_array); i++){
        
        auto hash = PHashString(hash_array[i]);
        
        AddHash(hash_table,&hash_count,hash,hash_array[i]);
    }
    
    return PrintHashDiagnostics(hash_table,hash_count);
    
}



int main(s32 argc,s8** argv){
    
    auto res = HashTest();
    
    return res;
}
