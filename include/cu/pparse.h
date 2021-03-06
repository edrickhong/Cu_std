#pragma once

//MARK: C STDLIB
#include "string.h"
#include "ttype.h"

/*
This is for general string parsing and to support the general parsing of C-like functions

TODO: 
our symbol extracting functions put the cur position on the char after the symbol. we should put it at the last char on the symbol instead

clean this up. we may have some duplicate functions
*/

#define _P_NULL_CHAR_ENCOUNTERED (u32)(-1)

enum PTokenType{
    
    PTOKEN_SYMBOL,
    PTOKEN_NUMBER,
    
    PTOKEN_OPENPAREN = '(',
    PTOKEN_CLOSEPAREN = ')',
    PTOKEN_OPENBRACKET = '[',
    PTOKEN_CLOSEBRACKET = ']',
    PTOKEN_OPENBRACE = '{',
    PTOKEN_CLOSEBRACE = '}',
    
    PTOKEN_COLON = ':',
    PTOKEN_SEMICOLON = ';',
    PTOKEN_COMMA = ',',
    PTOKEN_PERIOD = '.',
    PTOKEN_HYPHEN = '-',
    PTOKEN_OPENANGLEBRACKET = '<',
    PTOKEN_CLOSETANGLEBRACKET = '>',
    
    PTOKEN_ASTERISK = '*',
    PTOKEN_ADD = '+',
    PTOKEN_SUBTRACT = PTOKEN_HYPHEN,
    PTOKEN_POUND = '#',
    PTOKEN_AMPERSAND = '&',
    PTOKEN_PERCENT = '%',
    PTOKEN_EQUALS = '=',
    
    PTOKEN_SINGLEQUOTE = '\'',
    PTOKEN_DOUBLEQUOTE = '"',
    
    PTOKEN_BACKSLASH = '/',
    PTOKEN_FORWARDSLASH = '\\',
    
    PTOKEN_DOLLAR = '$',
};


void _ainline PCharToHexString(u8 c,s8* dst_buffer){
    
    *dst_buffer = '0';
    dst_buffer++;
    
    *dst_buffer = 'x';
    dst_buffer++;
    
    if(c < 16){
        
        *dst_buffer = '0';
        dst_buffer++;
    }
    
    s8 tbuffer[128] = {};
    u32 count = 0;
    
    while(c){
        
        auto k = c % 16;
        c /= 16;
        
        if(k < 10){
            
            tbuffer[count] = k + 48;
            count++;
        }
        
        else{
            
            switch(k){
                
                case 10:{
                    tbuffer[count] = 'A';
                    count++;
                }break;
                
                case 11:{
                    tbuffer[count] = 'B';
                    count++;
                }break;
                
                case 12:{
                    tbuffer[count] = 'C';
                    count++;
                }break;
                
                case 13:{
                    tbuffer[count] = 'D';
                    count++;
                }break;
                
                case 14:{
                    tbuffer[count] = 'E';
                    count++;
                }break;
                
                case 15:{
                    tbuffer[count] = 'F';
                    count++;
                }break;
                
            }
        }
    }
    
    for(u32 i = count - 1; i != (u32)-1; i--){
        
        *dst_buffer = tbuffer[i];
        dst_buffer++;
    }
    
}


b32 _ainline PIsWhiteSpace(s8 c){
    return (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r');
}

b32 _ainline PIsAlphabet(s8 c){
    
    return ((c > 64) && (c < 91)) || ((c > 96) && (c < 123));
}

b32 _ainline PIsNumeric(s8 c){
    return (c > 47) && (c < 58);
}

b32 _ainline PIsSymbol(s8 c){
    return PIsAlphabet(c) || PIsNumeric(c) || c == '$' || c == '#' || c == '_';
}

b32 _ainline PIsVisibleChar(s8 c){
    return (c > 32) && (c < 127);
}

constexpr ptrsize PStrLen(const s8* string){
    
    u32 len = 0;
    
    for(;;){
        
        if(string[len] == 0){
            break;
        }
        
        len++;
    }
    
    return len;
}

//TODO: replace things that use this hash to this type for clarity

typedef u64 str_hash64;
typedef u32 str_hash32;

constexpr str_hash64 PHashString(const s8* string){
    
    u64 prime_array[] = {
        13,5701,41,353,7,5023,193,11,13883,7109,8929,3,59149,21107,83,179
    };
    
#define _prime_value(a) prime_array[(a) % _arraycount(prime_array)]
    
    auto len = PStrLen(string);
    
    u64 hash = 0;
    
    for(u32 i = 0; i < len; i++){
        
        u32 h = string[i];
        u32 g = 0;
        u32 k = 0;
        
        if(i == 0){
            g = string[i + 1];
            k = string[len >> 1];
        }
        
        else{
            
            g = string[i - 1];
            k = string[len - i];
        }
        
        u32 a = (h * (i + 1)) * _prime_value(h - g);
        u32 b = h * _prime_value(hash * a);
        u32 c = ((hash + h) | _prime_value(k - (g * b + a))) | 1;
        
        hash += a ^ b ^ c;
        
        hash >>= (~(a - c) % 7);
    }
    
#undef _prime_value
    
    return hash;
}


u32 IdentifyToken(s8 c);


f32 PHexStringToFloat(const s8* string);

//We should add an function pointer to allow different criteria 

void PGetWord(s8* dst_string,s8* src_string,ptrsize* pos,u32* word_count);

void PGetSymbol(s8* dst_string,s8* src_string,ptrsize* pos,u32* word_count);

void PGetLine(s8* dst_string,s8* src_string,ptrsize* pos,u32* len);

void PSkipLine(s8* src_string,ptrsize* pos);

u32 PSkipWhiteSpace(s8* src_string,ptrsize* pos);

void PSkipUntilChar(s8* src_string,ptrsize* pos,s8 c);

void PParseUntilChar(s8* dst_string,s8* src_string,ptrsize* pos,s8 c,u32* len);

void PGetFileExtension(s8* dst_string,const s8* file,u32* len);

b32 _ainline PIsStringInt(s8* string){
    
    b32 is_numeric = true;
    
    auto len = strlen(string);
    
    if(!len){
        return false;
    }
    
    for(u32 i = 0; i < len; i++){
        auto c = string[i];
        is_numeric = is_numeric &&(PIsNumeric(c) || (c == '-' && i == 0));
        
        if(!is_numeric){
            break;
        }
    }
    
    return is_numeric;
}

b32 _ainline PIsStringFloat(s8* string){
    
    b32 is_numeric = true;
    
    u32 dot_count = 0;
    
    auto len = strlen(string);
    
    if(!len){
        return false;
    }
    
    for(u32 i = 0; i < len; i++){
        
        auto c = string[i];
        
        is_numeric =
            is_numeric &&(PIsNumeric(c) || c == '.' || (c == '-' && i == 0)) && (dot_count < 2);
        
        if(c == '.'){
            
            if(!i){
                return false;	
            }
            
            dot_count ++;
        }
        
        if(!is_numeric){
            break;
        }
    }
    
    return is_numeric;
    
}

constexpr u32 PFindStringInBuffer(const s8* target_string,const s8* buffer,
                                  ptrsize buffer_len){
    
    auto target_len = PStrLen(target_string);
    
    auto offset = 0;
    
    for(u32 i = 0; i < buffer_len; i++){
        
        auto c = buffer[i];
        
        offset = i;
        
        if(c == target_string[0]){
            
            for(u32 j = 0; j < target_len; j++){
                
                if(target_string[j] != buffer[i]){
                    break;
                }
                
                if(j == (target_len - 1)){
                    return offset;
                }
                
                i++;
            }
            
        }
        
    }
    
    return (u32)-1;
}

#define PFindStringInString(target,ref) PFindStringInBuffer(target,ref,PStrLen(ref))


void PBufferToByteArrayString(s8* array_name,s8* src_buffer,ptrsize src_size,s8* dst_buffer,ptrsize* dst_size);


void PBufferListToArrayString(s8* array_name,s8* src_buffer,ptrsize src_size,s8* dst_buffer,ptrsize* dst_size,u32* arraycount = 0);

constexpr b32 PStringCmp(const s8* string1,const s8* string2){
    
    auto len = PStrLen(string1);
    
    if(len != PStrLen(string2)){
        return false;
    }
    
    for(u32 i = 0; i < len; i++){
        
        if(string1[i] != string2[i]){
            return false;
        }
        
    }
    
    return true;
}

b32 _ainline PIsPreprocessorC(s8 c){
    return c == '#';
}

b32  _ainline PIsCommentC(s8 c1,s8 c2){
    return c1 == '/'  && c2 == '/';
}

b32  _ainline PIsStartCommentC(s8 c1,s8 c2){
    return c1 == '/'  && c2 == '*';
}

b32  _ainline PIsEndCommentC(s8 c1,s8 c2){
    return c1 == '*'  && c2 == '/';
}

void _ainline PIgnorePreprocessorAndCommentsC(s8* buffer,ptrsize* cur){
    
    auto k = *cur;
    
    
    
    auto is_block = PIsStartCommentC(buffer[k],buffer[k + 1]);
    
    while(is_block){
        
        is_block = PIsEndCommentC(buffer[k],buffer[k + 1]);
        k++;
        
        if(!is_block){
            
            k+=2;
            PSkipWhiteSpace(buffer,cur);
        }
        
    }
    
    *cur = k;
}


b32 PSanitizeStringC(s8* buffer,ptrsize* cur);

//NOTE: we will crash if we encounter a '}' first
void _ainline PSkipBracketBlock(s8* buffer,ptrsize* a){
    
    auto cur = *a;
    
    u32 k = 0;
    
    for(;;cur++){
        
        auto c = buffer[cur];
        
        if(c == '{'){
            k++;
        }
        
        if(c == '}'){
            
            _kill("incomplete scope error\n",!k);
            
            k --;
        }
        
        if(!k){
            break;
        }
    }
    
    *a = cur;
}

void _ainline PExtractScopeC(s8* scope_buffer,s8* buffer,ptrsize* a){
    
    u32 scope_count = 0;
    u32 count = *a;
    u32 i = 0;
    
    for(;;){
        
        auto c = buffer[count];
        scope_buffer[i] = c;
        
        if(c == '{'){
            scope_count++;
        }
        
        if(c == '}'){
            scope_count--;
        }
        
        
        count++;
        i++;
        
        
        if(!scope_count){
            break;
        }
    }
    
    *a = count;
}

enum CParseTags{
    TAG_SYMBOL = 0,
    TAG_CTYPE,
    TAG_STRUCT,
    TAG_KEY,
    TAG_VALUE,
    TAG_START_ARG,
    TAG_END_ARG,
    TAG_ENUM,
    TAG_UNION,
    TAG_INDIR,
    TAG_ASSIGN,
    
    TAG_START_SQUARE,
    TAG_END_SQUARE,
    TAG_START_CURLY,
    TAG_END_CURLY,
    
    TAG_COMMA,
    
    TAG_DOUBLE_QUOTE,
    
    //for math
    TAG_MUL = TAG_INDIR,
    TAG_DIV,
    TAG_ADD,
    TAG_SUB,
    
    //These have more context in C++
    TAG_COLON,
};

struct EvalChar{
    u64 hash;
    s8 string[128] = {};
    
    u32 tag;
};




s8 PFillEvalBufferC(s8* buffer,ptrsize* cur,EvalChar* eval_buffer,u32* eval_count,s8* terminator_array,u32 terminator_count,void (*tagevalbuffer)(EvalChar*,u32));



s8 _ainline PFillEvalBufferC(s8* buffer,ptrsize* cur,EvalChar* eval_buffer,u32* eval_count,s8 terminator,void (*tagevalbuffer)(EvalChar*,u32)){
    return PFillEvalBufferC(buffer,cur,eval_buffer,eval_count,&terminator,1,tagevalbuffer);
}






//string execution

enum OpCharType{
    
    OpChar_UNKNOWN = 0,
    
    OpChar_VALUE = 1,
    
    OpChar_ADD = 2,
    OpChar_SUB = 3,
    
    OpChar_DIV = 4,
    OpChar_MUL = 5,
    
    OpChar_BRACEBLOCK_START = 6,
    OpChar_BRACEBLOCK_END = 7,
    
};

enum OpExecMode{
    
    OpExecMode_S32,
    OpExecMode_U32,
    
    OpExecMode_S64,
    OpExecMode_U64,
    
    OpExecMode_F32,
    OpExecMode_F64,
};

struct OpChar{
    OpCharType type;
    s8 string[128];
};

//TODO: actually use OpExecMode


m64 PEvaluateMathString(OpChar* char_array,u32 char_count,OpExecMode mode = OpExecMode_S32);


m64 PEvaluateMathString(s8* string,OpExecMode mode = OpExecMode_S32);

s32 PStringToInt(s8* buffer);