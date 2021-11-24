#pragma once
#include "ccontainer.h"
#include "mmath.h"

#define _using_TAlloc 1 

#if _using_TAlloc


#include "aallocator.h"

#define DBGPTR(A) DEBUGPTR(A)

#else

#define DBGPTR(A) A*

#endif

//TODO: We will use the skeleton to store the final result

enum AAnimationBehaviour{
  AANIMATION_DEFAULT = 0,
  AANIMATION_CONSTANT = 1,
  AANIMATION_LINEAR = 2,
  AANIMATION_REPEAT = 3,
  AANIMATION_FORCE32BIT = 0x8fffffff,
};

struct AAnimationKey{
  f32 time;
  Vec4 value;
};


//MARK: this is obviosly for linearblend. we should rename this
struct AAnimationData{

  //MARK: could have precision issues
  u16 scalekey_count;
  u16 positionkey_count;
  u32 rotationkey_count;
  
  AAnimationKey* positionkey_array;
  AAnimationKey* rotationkey_array;
  AAnimationKey* scalekey_array;
};


struct AAnimationSet{
#if 0
	u64 hash;
#endif
	u32 animationdata_count;
	f32 duration;
	f32 tps;

	AAnimationData* animationdata_array;

#if 0
	const s8 name[20];
#endif
};

struct ASkel{
	u32* childcount_array;
	Mat4* offset_array;
	AAnimationData* animationdata_array;
	u32 count;

};


//MARK:Cache can still be better
struct ALinearBone{
  Mat4 offset;
  AAnimationData* animationdata_array;
  ALinearBone** children_array;
  u32 children_count;
};

struct ADQBone{
  DualQ offset;
  DualQ final;//we will be using this afterall
  //u32 bone_hash;//we only use this to map to corresponding animation data.might as well replace w data
  //AAnimationData* animationdata
  u32 children_count;
  u32 childrenindex_array[10];
};


void ALinearBlend(f32 time_ms,u32 animation_index,AAnimationSet* animation_array,
		  ALinearBone* root,DBGPTR(Mat4) result);

void ADualQBlend(f32 time_ms,ADQBone* root,AAnimationSet animation);

//NOTE: We can interpolate between both. sounds slow though?
//void LinearDQ(f32 time_ms,LinearSkeleton skeleton,AAnimation animation);
