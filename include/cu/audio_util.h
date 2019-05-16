#pragma once 

struct DeInterleavedSamples{
    __m128 l_channel;
    __m128 r_channel;
}_align(16);

void Convert_S16_TO_F32(void* dst,void* src,u32 sample_count);
void Convert_F32_TO_S16(void* dst,void* src,u32 sample_count);


//All the functions below assume f32 cos it is faster

//writes 4 samples to left and right
void Deinterleave_2(f32* left,f32* right,f32* src);

//writes 8 samples to dst
void Interleave_2(f32* dst,f32* left,f32* right);

void Deinterleave_2(f32* left,f32* right,f32* src,u32 samples);
void Interleave_2(f32* dst,f32* left,f32* right,u32 samples);

//writes 4 samples to dst
void Convert_Factor(f32* dst,f32* src,f32 index,f32 step);
void Convert_Factor(f32* dst,f32* src,u32 samples,f32 step);