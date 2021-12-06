#pragma once



#ifdef __cplusplus

void PPrint(f64 a);
void PPrint(f32 a);
void PPrint(s32 a);
void PPrint(u32 a);
void PPrint(const s8* s);

template<typename T,typename... Args>
void PPrint(T a,Args... args){
	PPrint(a);
	PPrint(args...);
}


template<typename T,typename... Args>
void PPrintln(T a,Args... args){
	PPrint(a,args...);
	PPrint("\n");
}


#ifdef DEBUG

#define _PPrint(...) PPrint(...)
#define _PPrintl(...) PPrintl(...)

#else

#define _PPrint(...)
#define _PPrintl(...)

#endif

#endif
