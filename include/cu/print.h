#pragma once

#ifdef __cplusplus_

void Print(f64 a);
void Print(f32 a);
void Print(s32 a);
void Print(u32 a);
void Print(const s8* s);

template<typename T,typename... Args>
void Print(T a,Args... args){
	Print(a);
	Print(args...);
}


template<typename T,typename... Args>
void Println(T a,Args... args){
	Print(a,args...);
	Print("\n");
}


#endif

#ifdef DEBUG
#define _Print(...) Print(...)
#else
#define _Print(...)
#endif


