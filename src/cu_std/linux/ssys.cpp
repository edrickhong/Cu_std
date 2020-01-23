#include "ssys.h"

_intern u32 ConvertStringGroupToInt(s8* buffer,u32 size){
    
    s8 buffer1[64] = {};
    s8 buffer2[64] = {};
    
    u32 offset = 0;
    
    for(u32 i = 0; i < size; i++){
        
        auto c = buffer[i];
        
        if(c == '-'){
            memcpy(buffer1,&buffer[offset],i - offset);
            i++;
            offset = i;
        }
        
        if(c == 0){
            memcpy(buffer2,&buffer[offset],i - offset);
        }
        
    }
    
    u32 len1 = strlen(buffer1);
    u32 len2 = strlen(buffer2);
    
    u32 val1 = 0;
    u32 val2 = 0;
    
    if(len1){
        val1 = PStringToInt(buffer1);
    }
    
    if(len2){
        val2 = PStringToInt(buffer2);
    }
    
    if(!len1 && len2){
        return 1;
    }
    
    return val2 - val1 + 1;
}

u32 SGetTotalThreads(){
    
    s8 buffer[512] = {};
    
    auto file = FOpenFile("/sys/devices/system/cpu/online",F_FLAG_READONLY);
    auto size = FGetFileSize(file);
    
    size = size < sizeof(buffer) ? size : sizeof(buffer);
    
    FRead(file,buffer,size);
    
    
    
    FCloseFile(file);
    
    u32 count = 0;
    u32 offset = 0;
    
    for(u32 i = 0;  i < size; i++){
        
        auto c = buffer[i];
        
        if(c == ',' || c == 0 || !PIsVisibleChar(c)){
            
            s8 convert_buffer[256] = {};
            memcpy(convert_buffer,&buffer[offset],i - offset);
            i++;
            offset = i;
            
            count += ConvertStringGroupToInt(convert_buffer,strlen(convert_buffer) + 1);
            
            if(c == 0 || !PIsVisibleChar(c)){
                break;
            }
        }
        
    }
    
    return count;
}