#pragma once 

struct DeInterleavedSamples{
    __m128 l_channel;
    __m128 r_channel;
}_align(16);

void Convert_S16_TO_F32(void* dst,void* src,u32 sample_count);
void Convert_F32_TO_S16(void* dst,void* src,u32 sample_count);


//All the functions below assume f32 cos it is faster

void Deinterleave_2(f32* left,f32* right,f32* src);
void Interleave_2(f32* dst,f32* left,f32* right);

void Deinterleave_2(f32* left,f32* right,f32* src,u32 samples);
void Interleave_2(f32* dst,f32* left,f32* right,u32 samples);


//converts 4 samples at a time but need 5 samples of reference. spits out 8 samples
void Convert_S16_48_To_96(f32* sample1,f32* sample2,f32* src);
//void Convert_S16_48_To_44_1(void* dst, void* src, u32 sample_count);
//void Convert_S16_48_To_192(void* dst, void* src, u32 sample_count);

void Convert_Factor(f32* dst,f32* src,u32 samples,f32 factor);