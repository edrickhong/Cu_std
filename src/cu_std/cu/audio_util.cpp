#include "audio_util.h"

#include "iintrin.h"

//TODO: use FMA instructions?? assume f32

#define _max_s16 (1 << 15)

void Convert_S16_TO_F32(void* dst,void* src,u32 sample_count){
    
    auto c_src = (s16*)src;
    auto c_dst = (f32*)dst;
    
    f32 factor = 1.0f/((f32)_max_s16);
    
    auto k = _mm_set1_ps(factor);
    for(;sample_count >= 4; sample_count-=4){
        
        __m64 m = {};
        
        memcpy(&m,c_src,sizeof(m));
        
        auto ps = _mm_mul_ps(_mm_cvtpi16_ps(m),k);
        
        _mm_store_ps(c_dst,ps);
        
        c_src +=4;
        c_dst +=4;
    }
    
    for(u32 i = 0; i < sample_count; i++){
        c_dst[i] = c_src[i] * factor;
    }
}

void Convert_F32_TO_S16(void* dst,void* src,u32 sample_count) {
    
    auto c_src = (f32*)src;
    auto c_dst = (s16*)dst;
    
    f32 factor = ((f32)_max_s16);
    
    auto k = _mm_set1_ps(factor);
    for(;sample_count >= 4; sample_count-=4){
        
        auto ps = _mm_mul_ps(_mm_load_ps(c_src),k);
        
        auto m = _mm_cvtps_pi16 (ps);
        
        memcpy(c_dst,&m,sizeof(m));
        
        c_src +=4;
        c_dst +=4;
    }
    
    for(u32 i = 0; i < sample_count; i++){
        c_dst[i] = c_src[i] * factor;
    }
}

void Convert_S16_48_To_96(f32* sample1,f32* sample2,f32* in_sample){
    
    auto a = _mm_load_ps(in_sample);
    
    auto b = _mm_shuffle_ps(a,a,_MM_SHUFFLE(0,3,2,1));
    ((f32*)&b)[3] = in_sample[4];
    
    auto step = _mm_set1_ps(0.5f);
    
    auto half = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(b,a),step),a);
    
    auto res1 = _mm_shuffle_ps(a,half,_MM_SHUFFLE(1,0,1,0));
    res1 = _mm_shuffle_ps(res1,res1,_MM_SHUFFLE(3,1,2,0));
    
    auto res2 = _mm_shuffle_ps(a,half,_MM_SHUFFLE(3,2,3,2));
    res2 = _mm_shuffle_ps(res2,res2,_MM_SHUFFLE(3,1,2,0));
    
    _mm_store_ps(sample1,res1);
    _mm_store_ps(sample2,res2);
}

void Deinterleave_2(f32* left,f32* right,f32* src){
    
    __m128 a = _mm_load_ps(src);
    __m128 b = _mm_load_ps(src + 4);
    
    auto l_res = _mm_shuffle_ps(a,b,_MM_SHUFFLE(2,0,2,0));
    auto r_res = _mm_shuffle_ps(a,b,_MM_SHUFFLE(3,1,3,1));
    
    _mm_store_ps(left,l_res);
    _mm_store_ps(right,r_res);
    
}

void Interleave_2(f32* dst,f32* left,f32* right){
    
    __m128 l = _mm_load_ps(left);
    __m128 r = _mm_load_ps(right);
    
    auto lo = _mm_shuffle_ps(l,r,_MM_SHUFFLE(1,0,1,0));
    auto hi = _mm_shuffle_ps(l,r,_MM_SHUFFLE(3,2,3,2));
    
    auto res1 = _mm_shuffle_ps(lo,lo,_MM_SHUFFLE(3,1,2,0));
    auto res2 = _mm_shuffle_ps(hi,hi,_MM_SHUFFLE(3,1,2,0));
    
    _mm_store_ps(dst,res1);
    _mm_store_ps(dst + 4,res2);
}

void Deinterleave_2(f32* left,f32* right,f32* src,u32 samples){
    
    u32 count = 0;
    
    for(u32 i = 0; i < samples; i += 8){
        
        DeInterleavedSamples samples = {};
        
        memcpy(&samples,src + i,sizeof(f32) * 8);
        
        Deinterleave_2((f32*)&samples.l_channel,(f32*)&samples.r_channel,(f32*)&samples);
        
        _mm_store_ps(left + count,samples.l_channel);
        _mm_store_ps(right + count,samples.r_channel);
        count +=4;
    }
}

void Interleave_2(f32* dst,f32* left,f32* right,u32 samples){
    
    u32 count = 0;
    
    for(u32 i = 0; i < samples; i += 4){
        
        auto l = left + i;
        auto r = right + i;
        
        Interleave_2((f32*)(dst + count),l,r);
        count+=8;
    }
}


void Convert_Factor(f32* dst,f32* src,u32 samples,f32 factor){
    
    u32 count = 0;
    auto inv_factor = 1.0f/factor;
    
    for(f32 cur = 0.0f; cur < (f32)samples; cur += (inv_factor * 4)){
        
        __m128 a = {};
        __m128 b = {};
        
        {
            u32 l0 = (u32)(cur);
            u32 l1 = (u32)(cur + inv_factor);
            u32 l2 = (u32)(cur + (inv_factor * 2));
            u32 l3 = (u32)(cur + (inv_factor * 3));
            a = _mm_set_ps(src[l3],src[l2],src[l1],src[l0]);
        }
        
        {
            u32 l0 = (u32)(cur + 1.0f);
            u32 l1 = (u32)(cur + inv_factor + 1.0f);
            u32 l2 = (u32)(cur + (inv_factor * 2) + 1.0f);
            u32 l3 = (u32)(cur + (inv_factor * 3) + 1.0f);
            b = _mm_set_ps(src[l3],src[l2],src[l1],src[l0]);
        }
        
        __m128 step = {};
        {
            f32 l0 = (cur);
            f32 l1 = (cur + inv_factor);
            f32 l2 = (cur + (inv_factor * 2));
            f32 l3 = (cur + (inv_factor * 3));
            step = _mm_set_ps(l3,l2,l1,l0);
            
            //just get the fract part
            __m128 integer = _mm_cvtepi32_ps(_mm_cvttps_epi32(step));
            step = _mm_sub_ps(step,integer);
        }
        
        auto res = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(b,a),step),a);
        _mm_store_ps(dst + count,res);//FIXME: we are overwriting here
        count+=4;
    }
    
}