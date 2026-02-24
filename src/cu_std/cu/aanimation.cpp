#include "ccolor.h"
#include "debugtimer.h"


#define _ms2s(ms)  ((f32)ms/1000.0f)


struct VectorTransform{
    Vec4 translation;
    Quat rotation;
    Vec4 scale;
};

VectorTransform _ainline IdentityVectorTransform(){
    VectorTransform transform;
    transform.translation = Vec4{};
    transform.rotation = MQuatIdentity();
    transform.scale = Vec4{1.0f,1.0f,1.0f,1.0f};
    
    return transform;
}

Vec4 DebugLerpAnimation(AAnimationKey* key_array,u32 key_count,f32 animationtime){
    
    if(key_count ==1){
        return key_array[0].value;
    }
    
    u32 frameindex = 0;
    
    for(u32 i = 0; i < key_count -1;i++){
        
        if(animationtime < key_array[i + 1].time){
            frameindex = i;
            break;
        }
        
    }
    
    AAnimationKey current = key_array[frameindex];
    AAnimationKey next =
        key_array[(frameindex + 1) % key_count];
    
    f32 step = (animationtime - current.time)/(next.time - current.time);
    
    printf("------------------------------------------------\n");
    printf("%f\n",(f64)step);
    printf("cur %f    %f    %f    \n",(f64)current.value.x,(f64)current.value.y,(f64)current.value.z);
    printf("next %f    %f    %f    \n",(f64)next.value.x,(f64)next.value.y,(f64)next.value.z);
    
    return LerpVec4(current.value,next.value,step);
}

// void PrintAnimationData(AAnimationSet animation){

//   for(u32 i = 0; i < animation.animationdata_count;i++){

//     AAnimationData data = animation.animationdata_array[i];

//     for(u32 j = 0; j < data.positionkey_count; j++){
//       AAnimationKey key = data.positionkey_array[j];

//       printf("time %f   pos   %f   %f   %f   %f   \n",
// 	     key.time,key.value.x,key.value.y,key.value.z,1.0f);
//     }

//     for(u32 j = 0; j < data.rotationkey_count; j++){
//       AAnimationKey key = data.rotationkey_array[j];

//       printf("time %f   rot   %f   %f   %f   %f   \n",
// 	     key.time,key.value.x,key.value.y,key.value.z,key.value.w);
//     }

//     for(u32 j = 0; j < data.scalekey_count; j++){
//       AAnimationKey key = data.scalekey_array[j];

//       printf("time %f   scale   %f   %f   %f   %f   \n",
// 	     key.time,key.value.x,key.value.y,key.value.z,1.0f);
//     }    

//   }
// }


Vec4 _ainline ALerpAnimation(AAnimationKey* key_array,u32 key_count,f32 animationtime){
    
    if(key_count ==1){
        return key_array[0].value;
    }
    
    u32 frameindex = 0;
    
    for(u32 i = 0; i < key_count -1;i++){
        
        if(animationtime < key_array[i + 1].time){
            frameindex = i;
            break;
        }
        
    }
    
    //we need to align the data
    AAnimationKey current = key_array[frameindex];
    
    
    AAnimationKey next =
        key_array[(frameindex + 1) % key_count];
    
    f32 step = (animationtime - current.time)/(next.time - current.time);
    
    return LerpVec4(current.value,next.value,step);
}


//TODO: these changes are temp -- currently packed as wxyz
Quat _ainline  ALerpAnimationQuat(AAnimationKey* key_array,u32 key_count,
                                                     f32 animationtime){

    if(key_count ==1){
        
        Quat ret = Vec4ToQuat(key_array[0].value);
        return ret;
    }
    
    u32 frameindex = 0;
    
    for(u32 i = 0; i < key_count -1;i++){
        
        if(animationtime < key_array[i + 1].time){
            frameindex = i;
            break;
        }
        
    }
    
    AAnimationKey current = key_array[frameindex];
    AAnimationKey next =
        key_array[(frameindex + 1) % key_count];
    
    
    f32 step = (animationtime - current.time)/(next.time - current.time);

    return NLerpQuat(Vec4ToQuat(current.value),Vec4ToQuat(next.value),step);

}

void ALinearTransformLinearSkeleton(u32 animation_index,f32 animationtime,
                                    ALinearBone* _restrict node,Mat4 parent_matrix,
                                    DBGPTR(Mat4) result,
                                    u32* _restrict result_count){
    
    //hand optimize this. the compiler has no idea wtf it's doing
    
    Mat4 matrix;
    VectorTransform transform = IdentityVectorTransform();
    
    AAnimationData* animationdata = node->animationdata_array;
    
    if(animationdata){
        
        animationdata += animation_index;
        
        transform.translation = 
            ALerpAnimation(animationdata->positionkey_array,
                                  animationdata->positionkey_count,animationtime);
        
        transform.scale =
            ALerpAnimation(animationdata->scalekey_array,
                                  animationdata->scalekey_count,animationtime);
        
        transform.rotation = 
            ALerpAnimationQuat(animationdata->rotationkey_array,
                                            animationdata->rotationkey_count,animationtime);
    }
    
    matrix =
        WorldMat4Q(Vec4ToVec3(transform.translation),
                    transform.rotation,
                    Vec4ToVec3(transform.scale));
    
    matrix = parent_matrix * matrix;
    
    result[*result_count] = 
#if _row_major
    
        TransposeMat4(matrix * node->offset);
    
#else
    
    matrix * node->offset;
    
#endif
    
    
    (*result_count)++;
    
    for(u32 i = 0; i < node->children_count;i++){
        
        ALinearBone* child = node->children_array[i];
        
        ALinearTransformLinearSkeleton(animation_index,animationtime,child,matrix,result,
                                       result_count);
    }
    
}


void ALinearBlend(f32 time_ms,u32 animation_index,AAnimationSet* animation_array,
                  ALinearBone* root,DBGPTR(Mat4) result){
    
    TIMEBLOCK(DarkViolet);

#if 1
    
    AAnimationSet animation = animation_array[animation_index];
    
    f32 tps = _either(animation.tps != 0.0f,animation.tps,25.0f);
    f32 ticks = tps * _ms2s(time_ms);
    
    //returns the remainder of x divided by y
    f32 animationtime = fmodf(ticks,animation.duration);
    
    u32 result_count = 0;
    
    ALinearTransformLinearSkeleton(animation_index,animationtime,root,
                                   IdentityMat4(),result,&result_count);
#else

    for(u32 i = 0; i < 43; i++){
	    result[i] = IdentityMat4();
    }
#endif
}

