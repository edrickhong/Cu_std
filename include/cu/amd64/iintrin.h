#pragma once

#include "xmmintrin.h"//sse (ivy bridge)
#include "emmintrin.h"//sse2 (ivy bridge)
#include "immintrin.h"//fma (haswell)
#include "smmintrin.h"// sse4.1 ()


#ifdef _WIN32

#include "Windows.h"

#define LockedIncrement(value) InterlockedIncrement(value)

#define LockedDecrement(value) InterlockedDecrement(value)

long _ainline LockedAdd(u32* value1,u32 value2){
    
    long k = 0;
    
    for(u32 i = 0; i < value2; i++){
        k = LockedIncrement(value1);
    }
    
    return k;
}

long _ainline LockedSub(u32* value1,u32 value2){
    
    long k = 0;
    
    for(u32 i = 0; i < value2; i++){
        k = LockedDecrement(value1);
    }
    
    return k;  
}

#define LockedCmpXchg(ptr,oldval,newval) InterlockedCompareExchange(ptr,newval,oldval)

#define LockedCmpXchg64(ptr,oldval,newval) InterlockedCompareExchange64 (ptr,newval,oldval)

u32 _ainline BSF(u32 r){
    u32 res;
    _BitScanForward((DWORD*)&res,r);
    return res;
}

u32 _ainline BSR(u32 r){
    u32 res;
    _BitScanReverse((DWORD*)&res,r);
    return res;
}

#define MOVSB(ds,sc,ln) __movsb(ds,sc,ln)

#define MOVSW(ds,sc,ln) __movsw(ds,sc,ln)

#define MOVSD(ds,sc,ln) __movsd(ds,sc,ln)

#define MOVSQ(ds,sc,ln) __movsq(ds,sc,ln)

__m128 _ainline _intrin_cvtpi16_ps(__m64 a){
    
    __m128i ret = {0};
    
    s16* src = (s16*)&a;
    s32* dst = (s32*)&ret;
    
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
    
    return _mm_cvtepi32_ps(ret);
}

__m64 _ainline _intrin_cvtps_pi16(__m128 a){
    
    __m128i pi = _mm_cvtps_epi32(a);
    __m64 ret = {0};
    
    s32* src = (s32*)&pi;
    s16* dst = (s16*)&ret;
    
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
    
    return ret;
}

#else

#define LockedIncrement(value) __sync_add_and_fetch(value,1)

#define LockedDecrement(value) __sync_sub_and_fetch(value,1)

#define LockedAdd(value1,value2) __sync_add_and_fetch(value1,value2)
#define LockedSub(value1,value2) __sync_sub_and_fetch(value,value2)


#define LockedCmpXchg(ptr,oldval,newval) __sync_val_compare_and_swap (ptr,oldval,newval)

#define LockedCmpXchg64(ptr,oldval,newval) __sync_val_compare_and_swap (ptr,oldval,newval)

#define BSR(r)  __builtin_clz (r)

#define BSF(r)  __builtin_ctz (r)

#define MOVSB(ds,sc,ln) __asm__ volatile ( \
"cld\n"  \
"rep movsb\n"  \
:  \
: [len] "c" (ln), [src] "S" (sc), [dst] "D" (ds) \
: "memory" \
);

#define MOVSW(ds,sc,ln) __asm__ volatile ( \
"cld\n"  \
"rep movsw\n"  \
:  \
: [len] "c" (ln), [src] "S" (sc), [dst] "D" (ds) \
: "memory" \
);

#define MOVSD(ds,sc,ln) __asm__ volatile ( \
"cld\n"  \
"rep movsl\n"  \
:  \
: [len] "c" (ln), [src] "S" (sc), [dst] "D" (ds) \
: "memory" \
);

#define MOVSQ(ds,sc,ln) __asm__ volatile ( \
"cld\n"  \
"rep movsq\n"  \
:  \
: [len] "c" (ln), [src] "S" (sc), [dst] "D" (ds) \
: "memory" \
);

#define _intrin_cvtpi16_ps(a) _mm_cvtpi16_ps(a)
#define _intrin_cvtps_pi16(a) _mm_cvtps_pi16(a)

#endif

#ifdef __FMA__

#define _intrin_fmadd_ps(a,b,c) _mm_fmadd_ps(a,b,c)

#else

#pragma message ("FMA is not enabled")

#define _intrin_fmadd_ps(a,b,c) _mm_add_ps(_mm_mul_ps(a,b),c)

#endif

