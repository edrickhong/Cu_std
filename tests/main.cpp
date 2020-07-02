// this is for running tests
#include "aallocator.h"
#include "hash_array.h"
#include "mmath.h"
#include "mode.h"
#include "pparse.h"
#include "stdio.h"
#include "ttype.h"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE 1
#define GLM_FORCE_RIGHT_HANDED 1
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/projection.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/dual_quaternion.hpp"
#include "glm/gtx/intersect.hpp"
#include "glm/gtx/vector_angle.hpp"

extern "C" DualQ ConjugateDualQ(DualQ q);

struct HashEntry {
	u64 hash;
	const s8* string_array[1024 * 8];
	u32 string_count = 0;
};

void AddHash(HashEntry* table, u32* c, u64 hash, const s8* string) {
	auto count = *c;

	for (u32 i = 0; i < count; i++) {
		auto k = &table[i];

		if (k->hash == hash) {
			k->string_array[k->string_count] = string;
			k->string_count++;

			return;
		}
	}

	table[count].hash = hash;
	table[count].string_array[0] = string;
	table[count].string_count = 1;
	count++;

	*c = count;
}

b32 PrintHashDiagnostics(HashEntry* table, u32 count) {
	u32 collide_count = 0;

	for (u32 i = 0; i < count; i++) {
		auto entry = &table[i];

		if (entry->string_count > 1) {
			collide_count += entry->string_count;

			printf("\n\nHASH COLLISION(%llu): ", entry->hash);

			for (u32 j = 0; j < entry->string_count; j++) {
				printf("%s ", entry->string_array[j]);
			}

			printf("\n");
		}
	}

	auto percent =
	    ((f64)collide_count / (f64)(_arraycount(hash_array))) * 100.0f;

	printf("%d collisions out of %d hashes. (%f percent collision rate)\n",
	       collide_count, (u32)_arraycount(hash_array), percent);

	return percent > 20.0;
}

b32 HashTest() {
	printf("RUNNING PHashString Collision test\n");

	auto hash_table =
	    (HashEntry*)alloc(sizeof(HashEntry) * (_arraycount(hash_array)));

	u32 hash_count = 0;

	for (u32 i = 0; i < _arraycount(hash_array); i++) {
		auto hash = PHashString(hash_array[i]);

		AddHash(hash_table, &hash_count, hash, hash_array[i]);
	}

	return PrintHashDiagnostics(hash_table, hash_count);
}



void DiffElement(void* a_1, void* b_1, u32 count,const s8* file,
		const s8* function,u32 line) {
	auto a = (f32*)a_1;
	auto b = (f32*)b_1;

	f32 diff = 0.0f;

	for (u32 i = 0; i < count; i++) {
		diff += fabsf(b[i] - a[i]);
	}

	f32 avg = diff / (f32)count;

	f32 margin_err = 0.0001;

	if (avg > margin_err) {

		printf("ERROR Exceeded margin of Error %f: %s %s %d\n",
				(f64)avg,file,function,line);

#if 0
		_breakpoint();
#endif
		return;
	}

}

Vec3 NewOP(Vec3 a,Vec3 b){
	return a - RejectVec3(a,b);
}

#if 1
#define DiffElement(a,b,c) DiffElement(a,b,c,__FILE__,__FUNCTION__,__LINE__)

#endif

_global Mat4 transforms[] = {
	{
		10,20,10,2,
		4,5,6,-1,
		2,3,5,2,
		0,0,0,1,
	},
	{
		25.2919, 47.6146, 47.1678,1,
		68.7935, 90.9444, 77.1529,4,
		63.6141, 38.1667, 19.3661,5,
		0,0,0,1
	},
	{
		88.4585, 8.5391, 96.2915,-4,
		9.0760, 56.7465, 72.5661,9,
		86.1256, 32.2091, 50.3410,0,
		0, 0, 0,1,
	},
};

_global Mat4 mat4[] = {
	{
		12.9212, 22.0446, 90.5516, 18.2823,
		98.1340, 59.1364, 3.3073, 70.6183,
		93.1204, 37.9339, 85.5814, 35.4140,
		46.0602, 41.4153, 22.0703, 3.2971,
	},
	{
		96.8699, 54.2778, 61.4214, 13.2176,
		53.2939, 61.9448, 59.6839, 78.7674,
		24.5856, 16.2350, 31.2080, 50.3813,
		25.8469, 54.7340, 76.7431, 1.7616,
	},
	{
		11.2620, 3.9130, 23.6870, 39.5326,
		71.5399, 12.5743, 36.1785, 55.6013,
		34.3654, 88.3443, 27.2348, 66.8142,
		45.3723, 39.4601, 43.1630, 83.6811,
	},
};

_global Mat3 mat3[] = {
	{
		80.1066, 27.6461, 66.1057,
		51.8226, 61.9752, 87.4610,
		97.4381, 72.4122, 81.6955,
	},
	{
		43.3802, 48.0465, 36.8774,
		89.4086, 64.4052, 73.3082,
		42.4715, 79.5170, 84.4931,
	},
	{
		35.6854, 33.3402, 7.3887,
		76.2633, 99.7578, 55.7929,
		25.8683, 91.4528, 87.9986,
	},
};

_global Mat2 mat2[] = {
	{
		61.4439, 38.9487,
		74.4360, 84.7355,
	},
	{
		9.6600, 64.3617,
		51.4195, 48.9164,
	},
	{
		0.8387, 31.5826,
		14.2628, 28.6956,
	},
};

_global Vec4 vec4[] = {
	{79.8354, 48.5731, 14.5320, 70.5055,},
	{66.3241, 33.0981, 64.5561, 89.4826,},
	{10.2782, 84.4469, 70.2671, 55.6599,},
};

_global Vec3 vec3[] = {
	{72.7098, 69.1509, 4.3042,},
	{94.2468, 11.0857, 13.1305,},
	{26.4829, 23.1464, 30.2243,},
};

_global Vec2 vec2[] = {
	{48.2357, 12.9792,},
	{31.1238, 55.0970,},
	{17.5348, 12.6420,},
};

_global Quat quat[] = {
	ConstructQuat({87.4912, 2.8514, 57.5126},49.0606),
	ConstructQuat({91.2534, 39.7049, 48.3562},9.0578),
	ConstructQuat({82.4747, 65.5843, 1.4699},17.1199),
};

_global DualQ dq[] = {
	ConstructDualQ(quat[0], vec3[0]),
	ConstructDualQ(quat[1], vec3[1]),
	ConstructDualQ(quat[2], vec3[2]),
};

_global Plane planes[] = {
	{NormalizeVec3(vec3[0]),12.6420},
	{NormalizeVec3(vec3[1]),11.0857},
	{NormalizeVec3(vec3[2]),-12.6420},
};

_global Ray3 ray3[] = {
	ConstructRay3(vec3[0],vec3[1]),
	ConstructRay3(vec3[1],vec3[2]),
	ConstructRay3(vec3[2],vec3[0]),
};


_global Line3 line3[] = {
	ConstructLine3(vec3[0],vec3[1]),
	ConstructLine3(vec3[1],vec3[2]),
	ConstructLine3(vec3[2],vec3[0]),
};

void MathConversions(){
	/*
	 * NOTE: Not tested:
	 *Conversions that just expand, ommit, or directly pass
	 components are not tested

	 DualQToMat4
	 WorldSpaceToClipSpaceVec4
	 ClipSpaceToWorldSpaceVec4
	 * */

	for(u32 i = 0; i < _arraycount(quat); i++){
		auto q = quat[i];
		auto t1 = *((glm::fquat*)&q);

		auto m = QuatToMat4(q);
		auto t = glm::transpose(glm::toMat4(t1));

		DiffElement(&m,&t,_arraycount(m.container));
	} 


#if 0

	/*
	 * We aren't testing this cos there's no good way to test this.
	 * There are many different ways to get an arbitrary axis of
	 * rotation, thus also the angle of rotation. We will know we
	 * are wrong cos we ALWAYS use this for skeletal animation
	 * */

	for(u32 i = 0; i < _arraycount(mat4); i++){
		auto k = mat4[i];
		auto t1 = *((glm::mat4*)&k);

		auto m = Mat4ToQuat(k);
		auto t = glm::toQuat(glm::transpose(t1));

		DiffElement(&m,&t,_arraycount(m.container));
	}

#endif

}


Vec4 operator*(Mat4 lhs, Vec4 rhs);
Vec3 operator*(Mat3 lhs, Vec3 rhs);

u32 WrapAround(u32 i, u32 count){
	return (i >= count) ? 0 : count;
}

void MathConstructor(){
	/*
	 * NOTE: Not tested:
	 *Line functions
	 Plane functions
	 Sphere functions

	MQuatIdentity
	AQuatIdentity
	TransformRelativeMat4
	ScaleDir
	ScalePerpenDir
	SkewMat3
	SkewDirMat3
	InvolMat3
	ConstructColor4
	ConstructColor3
	GetSphereNormalVec3
	ConstructQuat
	ConstructVecQuat
	ConstructDualQ
	ConstructDualQM

	We aren't gonna test WorldMat* cos they consist of many
	sub matrices
	 * */
	//ReflectPointPlaneMat (kind of)
	for(u32 i = 0; i < _arraycount(vec3); i++){
		auto v = NormalizeVec3(vec3[i]);

		Vec3 k = {};

		if(i + 1 < _arraycount(vec3)){
			k = vec3[i + 1];
		}
		else{
			k = vec3[0];
		}


		auto t1 = *((glm::vec3*)&v);
		auto t2 = *((glm::vec3*)&k);

		auto p = ConstructPlaneD(v,0.0f);

		auto g = ReflectPointPlaneMat4(p);
		auto t = glm::reflect(t2,t1);

		auto m = Vec4ToVec3(g * Vec3ToVec4(k));

		DiffElement(&m,&t,_arraycount(m.container));
	}

	//ViewMat
	for(u32 i = 0; i < _arraycount(vec3); i++){

		auto p = vec3[i];
		auto l = vec3[WrapAround(i + 1,_arraycount(vec3))];
		auto u = NormalizeVec3(vec3[WrapAround(i + 2,
					_arraycount(vec3))]);

		auto t1 = *((glm::vec3*)&p);
		auto t2 = *((glm::vec3*)&l);
		auto t3 = *((glm::vec3*)&u);

		auto m = ViewMat4(p,l,u);
		auto t = glm::transpose(glm::lookAt(t1,t2,t3));

		DiffElement(&m,&t,_arraycount(m.container));
	}

	//ProjectionMat4
	for(u32 i = 0; i < _arraycount(vec3); i++){

		auto p = vec3[i];
		auto t1 = *((glm::vec3*)&p);

		auto m = ProjectionMat4(p.x,p.y/p.x,p.z,p.y);
		auto t = glm::transpose(
				glm::perspective(t1.x,t1.y/t1.x,
					t1.z,t1.y));

		DiffElement(&m,&t,_arraycount(m.container));
	}

	//TranslateMat4
	for(u32 i = 0; i < _arraycount(vec3); i++){
		
		auto p = vec3[i];
		auto t1 = *((glm::vec3*)&p);

		auto m = TranslateMat4(p);
		auto t = glm::transpose(glm::translate(glm::mat4(),t1));

		DiffElement(&m,&t,_arraycount(m.container));
	}


	//RotationMat3
	for(u32 i = 0; i < _arraycount(vec3); i++){
		
		auto p = vec3[i];
		auto t1 = *((glm::vec3*)&p);

		auto m = Mat3ToMat4(RotateMat3(p));

		glm::mat4 t = {};

		{
			auto x = glm::vec3(1,0,0);
			auto y = glm::vec3(0,1,0);
			auto z = glm::vec3(0,0,1);

			auto mx = glm::rotate(t1.x,x);
			auto my = glm::rotate(t1.y,y);
			auto mz = glm::rotate(t1.z,z);

			t = glm::transpose(mz * my * mx);
		}


		DiffElement(&m,&t,_arraycount(m.container));
	}


	for(u32 i = 0; i < _arraycount(vec3); i++){
		
		auto p = NormalizeVec3(vec3[i]);
		auto t1 = *((glm::vec3*)&p);

		auto m = Mat3ToMat4(RotationAxisMat3(p,0.3f));
		auto t = glm::transpose(glm::rotate(glm::mat4(),
					0.3f,t1));

		DiffElement(&m,&t,_arraycount(m.container));
	}

	//ScaleMat3
	for(u32 i = 0; i < _arraycount(vec3); i++){
		
		auto p = vec3[i];
		auto t1 = *((glm::vec3*)&p);

		auto m = Mat3ToMat4(ScaleMat3(p));
		auto t = glm::transpose(glm::scale(glm::mat4(),t1));

		DiffElement(&m,&t,_arraycount(m.container));
	}

	//ReflectMat3
	for(u32 i = 0; i < _arraycount(vec3); i++){
		auto v = NormalizeVec3(vec3[i]);
		auto k = vec3[WrapAround(i + 1,_arraycount(vec3))];

		auto t1 = *((glm::vec3*)&v);
		auto t2 = *((glm::vec3*)&k);

		auto g = ReflectMat3(v);
		auto m = g * k;

		auto t = glm::reflect(t2,t1);

		DiffElement(&m,&t,_arraycount(m.container));
	}

	//ConstructCrossMat3
	for(u32 i = 0; i < _arraycount(vec3); i++){
		auto v = vec3[i];
		auto k = vec3[WrapAround(i + 1,_arraycount(vec3))];

		auto t1 = *((glm::vec3*)&v);
		auto t2 = *((glm::vec3*)&k);

		auto g = ConstructCrossMat3(v);
		auto m = g * k;

		auto t = glm::cross(t1,t2);

		DiffElement(&m,&t,_arraycount(m.container));
	}

	//ConstructProjectOntoMat3
	for(u32 i = 0; i < _arraycount(vec3); i++){
		auto v = vec3[i];
		auto k = vec3[WrapAround(i + 1,_arraycount(vec3))];

		auto t1 = *((glm::vec3*)&v);
		auto t2 = *((glm::vec3*)&k);

		auto g = ConstructProjectOntoMat3(k);
		auto m = g * v;

		auto t = glm::proj(t1,t2);

		DiffElement(&m,&t,_arraycount(m.container));
	}

	//ConstructRejectMat3
	for(u32 i = 0; i < _arraycount(vec3); i++){
		auto v = vec3[i];
		auto k = vec3[WrapAround(i + 1,_arraycount(vec3))];

		auto t1 = *((glm::vec3*)&v);
		auto t2 = *((glm::vec3*)&k);

		auto g = ConstructRejectMat3(k);
		auto m = g * v;

		auto t = t1 - glm::proj(t1,t2);

		DiffElement(&m,&t,_arraycount(m.container));
	}
}

void MathDeconstructor(){
	/*
	 * NOTE: Not tested:
	 *DeconstructQuat glm doesn't support this case apparently
	 * */
}

void MathOps(){

	/*
	 * NOTE:Not tested are functions not supported by glm
	 * */

	for(u32 i = 0; i < _arraycount(mat4); i++){
		auto a = mat4[i];
		auto b = a;

		if(i + 1 < _arraycount(mat4)){
			b = mat4[i + 1];
		}
		else{
			b = mat4[0];
		}

		auto t1 = *((glm::mat4*)&a);
		auto t2 = *((glm::mat4*)&b);

		//Add

		auto c = a + b;

		auto t = 
			glm::transpose(glm::transpose(t1) + 
					glm::transpose(t2));

		DiffElement(&c,&t,_arraycount(c.container));

		//Sub

		 c = a - b;

		 t = 
			glm::transpose(glm::transpose(t1) - 
					glm::transpose(t2));

		DiffElement(&c,&t,_arraycount(c.container));

		//Mul
		 c = a * b;

		 t = 
			glm::transpose(glm::transpose(t1) * 
					glm::transpose(t2));

		DiffElement(&c,&t,_arraycount(c.container));

		//MulconstL

		 c = 1.5f * b;

		 t = 
			glm::transpose(1.5f * 
					glm::transpose(t2));

		DiffElement(&c,&t,_arraycount(c.container));
		//MulconstR

		 c = a * 1.5f;

		 t = 
			glm::transpose(glm::transpose(t1) * 
					1.5f);

		DiffElement(&c,&t,_arraycount(c.container));
		//Div

		 c = a / b;

		 t = 
			glm::transpose(glm::transpose(t1) / 
					glm::transpose(t2));

		DiffElement(&c,&t,_arraycount(c.container));
	}


	for(u32 i = 0; i < _arraycount(mat3); i++){
		auto a = mat3[i];
		auto b = a;

		if(i + 1 < _arraycount(mat3)){
			b = mat3[i + 1];
		}
		else{
			b = mat3[0];
		}

		auto t1 = *((glm::mat3*)&a);
		auto t2 = *((glm::mat3*)&b);

		//Add

		auto c = a + b;

		auto t = 
			glm::transpose(glm::transpose(t1) + 
					glm::transpose(t2));

		DiffElement(&c,&t,_arraycount(c.container));

		//Sub

		 c = a - b;

		 t = 
			glm::transpose(glm::transpose(t1) - 
					glm::transpose(t2));

		DiffElement(&c,&t,_arraycount(c.container));

		//Mul
		 c = a * b;

		 t = 
			glm::transpose(glm::transpose(t1) * 
					glm::transpose(t2));

		DiffElement(&c,&t,_arraycount(c.container));

		//MulconstL

		 c = 1.5f * b;

		 t = 
			glm::transpose(1.5f * 
					glm::transpose(t2));

		DiffElement(&c,&t,_arraycount(c.container));
		//MulconstR

		 c = a * 1.5f;

		 t = 
			glm::transpose(glm::transpose(t1) * 
					1.5f);

		DiffElement(&c,&t,_arraycount(c.container));
		//Div

		 c = a / b;

		 t = 
			glm::transpose(glm::transpose(t1) / 
					glm::transpose(t2));

		DiffElement(&c,&t,_arraycount(c.container));
	}

	for(u32 i = 0; i < _arraycount(mat2); i++){
		auto a = mat2[i];
		auto b = a;

		if(i + 1 < _arraycount(mat2)){
			b = mat2[i + 1];
		}
		else{
			b = mat2[0];
		}

		auto t1 = *((glm::mat2*)&a);
		auto t2 = *((glm::mat2*)&b);

		//Add

		auto c = a + b;

		auto t = 
			glm::transpose(glm::transpose(t1) + 
					glm::transpose(t2));

		DiffElement(&c,&t,_arraycount(c.container));

		//Sub

		 c = a - b;

		 t = 
			glm::transpose(glm::transpose(t1) - 
					glm::transpose(t2));

		DiffElement(&c,&t,_arraycount(c.container));

		//Mul
		 c = a * b;

		 t = 
			glm::transpose(glm::transpose(t1) * 
					glm::transpose(t2));

		DiffElement(&c,&t,_arraycount(c.container));

		//MulconstL

		 c = 1.5f * b;

		 t = 
			glm::transpose(1.5f * 
					glm::transpose(t2));

		DiffElement(&c,&t,_arraycount(c.container));
		//MulconstR

		 c = a * 1.5f;

		 t = 
			glm::transpose(glm::transpose(t1) * 
					1.5f);

		DiffElement(&c,&t,_arraycount(c.container));
		
		//Div

		 c = a / b;

		 t = 
			glm::transpose(glm::transpose(t1) / 
					glm::transpose(t2));

		DiffElement(&c,&t,_arraycount(c.container));
	}

	for(u32 i = 0; i < _arraycount(vec4); i++){
		auto a = vec4[i];
		auto b = a;

		if(i + 1 < _arraycount(vec4)){
			b = vec4[i + 1];
		}
		else{
			b = vec4[0];
		}

		auto t1 = *((glm::vec4*)&a);
		auto t2 = *((glm::vec4*)&b);

		//Add

		auto c = a + b;

		auto t = 
			((t1) + 
					(t2));

		DiffElement(&c,&t,_arraycount(c.container));

		//Sub

		 c = a - b;

		 t = 
			((t1) - 
					(t2));

		DiffElement(&c,&t,_arraycount(c.container));

		//MulconstL

		 c = 1.5f * b;

		 t = 
			(1.5f * 
					(t2));

		DiffElement(&c,&t,_arraycount(c.container));
		//MulconstR

		 c = a * 1.5f;

		 t = 
			((t1) * 
					1.5f);

		DiffElement(&c,&t,_arraycount(c.container));
		
		//Div

		 c = a / 1.5f;

		 t = 
			((t1) / 
					(1.5f));

		DiffElement(&c,&t,_arraycount(c.container));
	}


	for(u32 i = 0; i < _arraycount(vec3); i++){
		auto a = vec3[i];
		auto b = a;

		if(i + 1 < _arraycount(vec3)){
			b = vec3[i + 1];
		}
		else{
			b = vec3[0];
		}

		auto t1 = *((glm::vec3*)&a);
		auto t2 = *((glm::vec3*)&b);

		//Add

		auto c = a + b;

		auto t = 
			((t1) + 
					(t2));

		DiffElement(&c,&t,_arraycount(c.container));

		//Sub

		 c = a - b;

		 t = 
			((t1) - 
					(t2));

		DiffElement(&c,&t,_arraycount(c.container));

		//MulconstL

		 c = 1.5f * b;

		 t = 
			(1.5f * 
					(t2));

		DiffElement(&c,&t,_arraycount(c.container));
		//MulconstR

		 c = a * 1.5f;

		 t = 
			((t1) * 
					1.5f);

		DiffElement(&c,&t,_arraycount(c.container));
		
		//Div

		 c = a / 1.5f;

		 t = 
			((t1) / 
					(1.5f));

		DiffElement(&c,&t,_arraycount(c.container));
	}

	for(u32 i = 0; i < _arraycount(vec2); i++){
		auto a = vec2[i];
		auto b = a;

		if(i + 1 < _arraycount(vec2)){
			b = vec2[i + 1];
		}
		else{
			b = vec2[0];
		}

		auto t1 = *((glm::vec2*)&a);
		auto t2 = *((glm::vec2*)&b);

		//Add

		auto c = a + b;

		auto t = 
			((t1) + 
					(t2));

		DiffElement(&c,&t,_arraycount(c.container));

		//Sub

		 c = a - b;

		 t = 
			((t1) - 
					(t2));

		DiffElement(&c,&t,_arraycount(c.container));

		//MulconstL

		 c = 1.5f * b;

		 t = 
			(1.5f * 
					(t2));

		DiffElement(&c,&t,_arraycount(c.container));
		//MulconstR

		 c = a * 1.5f;

		 t = 
			((t1) * 
					1.5f);

		DiffElement(&c,&t,_arraycount(c.container));
		
		//Div

		 c = a / 1.5f;

		 t = 
			((t1) / 
					(1.5f));

		DiffElement(&c,&t,_arraycount(c.container));
	}

	for(u32 i = 0; i < _arraycount(quat); i++){
		auto a = quat[i];
		auto b = a;

		if(i + 1 < _arraycount(quat)){
			b = quat[i + 1];
		}
		else{
			b = quat[0];
		}

		auto t1 = *((glm::fquat*)&a);
		auto t2 = *((glm::fquat*)&b);

		//Mul

		 auto c = a * b;

		 auto t = 
			((t1) * 
					(t2));

		DiffElement(&c,&t,_arraycount(c.container));
	}

	for(u32 i = 0; i < _arraycount(dq); i++){
		auto a = dq[i];
		auto b = a;

		if(i + 1 < _arraycount(dq)){
			b = dq[i + 1];
		}
		else{
			b = dq[0];
		}

		auto t1 = *((glm::fdualquat*)&a);
		auto t2 = *((glm::fdualquat*)&b);

		//Mul

		 auto c = a * b;

		 auto t = 
			((t1) * 
					(t2));

		DiffElement(&c,&t,_arraycount(c.container));
	}

}


void SpecialMathOps(){

	/*
	
	MARK: NOT TESTED:
	Dist functions are not tested
	Line to Line intersections are not tested
	Plane to plane intersections are not tested
	2D intersections are not tested
	Typed intersections are not tested
	ProjectVec3OntoPlane
	InvolVec3
	AngleQuadrant
	GetVecRotation

	Calling IntersectOut* functions implicitly calls the 
	corresponding Intersect* functions
	 * */
	//NOTE: For quats, we don't test operations that are analogous
	//to their vec4 counterparts
	
	
	//ReflectPointPlaneVec3 (kind of)
	for(u32 i = 0; i < _arraycount(vec3); i++){
		auto v = NormalizeVec3(vec3[i]);

		Vec3 k = {};

		if(i + 1 < _arraycount(vec3)){
			k = vec3[i + 1];
		}
		else{
			k = vec3[0];
		}


		auto t1 = *((glm::vec3*)&v);
		auto t2 = *((glm::vec3*)&k);

		auto p = ConstructPlaneD(v,0.0f);

		auto m = ReflectPointPlaneVec3(p,k);
		auto t = glm::reflect(t2,t1);

		DiffElement(&m,&t,_arraycount(m.container));
	}

	// Schur
	for(u32 i = 0; i < _arraycount(mat4); i++){

		auto m1 = mat4[i];
		auto m2 = m1;

		if(i + 1 < _arraycount(mat4)){
			m2 = mat4[i + 1];
		}
		else{
			m2 = mat4[0];
		}

		auto t1 = m1;
		auto t2 = m2;
	
		auto mat = SchurMat4(m1,m2);

		Mat4 t = {};

		//glm doesn't have a schur product
		for(u32 i = 0; i < _arraycount(t.container); i++){
			t.container[i] = 
				t1.container[i] * t2.container[i];
		}

		DiffElement(&mat,&t,_arraycount(mat.container));
	}

	for(u32 i = 0; i < _arraycount(mat3); i++){

		auto m1 = mat3[i];
		auto m2 = m1;

		if(i + 1 < _arraycount(mat3)){
			m2 = mat3[i + 1];
		}
		else{
			m2 = mat3[0];
		}

		auto t1 = m1;
		auto t2 = m2;
	
		auto mat = SchurMat3(m1,m2);

		Mat3 t = {};

		//glm doesn't have a schur product
		for(u32 i = 0; i < _arraycount(t.container); i++){
			t.container[i] = 
				t1.container[i] * t2.container[i];
		}

		DiffElement(&mat,&t,_arraycount(mat.container));
	}


	// Transpose

	for(u32 i = 0; i < _arraycount(mat4); i++){
		auto mat = mat4[i];
		glm::mat4 t = *((glm::mat4*)&mat);

	
		mat = TransposeMat4(mat);
		t = glm::transpose(t);

		DiffElement(&mat,&t,_arraycount(mat.container));
	}

	for(u32 i = 0; i < _arraycount(mat3); i++){
		auto mat = mat3[i];
		glm::mat3 t = *((glm::mat3*)&mat);

	
		mat = TransposeMat3(mat);
		t = glm::transpose(t);

		DiffElement(&mat,&t,_arraycount(mat.container));
	}

	for(u32 i = 0; i < _arraycount(mat2); i++){
		auto mat = mat2[i];
		glm::mat2 t = *((glm::mat2*)&mat);

	
		mat = TransposeMat2(mat);
		t = glm::transpose(t);

		DiffElement(&mat,&t,_arraycount(mat.container));
	}

	// Matrix inverses 
	for(u32 i = 0; i < _arraycount(transforms); i++){
		auto mat = transforms[i];
		glm::mat4 t = *((glm::mat4*)&mat);

	
		mat = InverseTransform(mat);
		t = glm::inverse(t);

		DiffElement(&mat,&t,_arraycount(mat.container));
	}


	for(u32 i = 0; i < _arraycount(mat4); i++){
		auto mat = mat4[i];
		glm::mat4 t = *((glm::mat4*)&mat);

	
		mat = InverseMat4(mat);
		t = glm::inverse(t);

		DiffElement(&mat,&t,_arraycount(mat.container));
	}


	for(u32 i = 0; i < _arraycount(mat3); i++){
		auto mat = mat3[i];
		glm::mat3 t = *((glm::mat3*)&mat);

	
		mat = InverseMat3(mat);
		t = glm::inverse(t);

		DiffElement(&mat,&t,_arraycount(mat.container));
	}


	for(u32 i = 0; i < _arraycount(mat2); i++){
		auto mat = mat2[i];
		glm::mat2 t = *((glm::mat2*)&mat);

	
		mat = InverseMat2(mat);
		t = glm::inverse(t);

		DiffElement(&mat,&t,_arraycount(mat.container));
	}

	//Outer products
	
	for(u32 i = 0; i < _arraycount(vec4); i++){
		auto v1 = vec4[i];

		Vec4 v2 = {};
		if(i + 1 < _arraycount(vec4)){
			v2 = vec4[i + 1];
		}
		else{
			v2 = vec4[0];
		}

		glm::vec4 t1 = *((glm::vec4*)&v1);
		glm::vec4 t2 = *((glm::vec4*)&v2);

		auto mat = OuterMat4(v1,v2);
		auto t = glm::transpose(glm::outerProduct(t1,t2));

		DiffElement(&mat,&t,_arraycount(mat.container));
	}

	for(u32 i = 0; i < _arraycount(vec3); i++){
		auto v1 = vec3[i];

		Vec3 v2 = {};
		if(i + 1 < _arraycount(vec3)){
			v2 = vec3[i + 1];
		}
		else{
			v2 = vec3[0];
		}

		glm::vec3 t1 = *((glm::vec3*)&v1);
		glm::vec3 t2 = *((glm::vec3*)&v2);

		auto mat = OuterMat3(v1,v2);
		auto t = glm::transpose(glm::outerProduct(t1,t2));

		DiffElement(&mat,&t,_arraycount(mat.container));
	}

	//Magnitude
	for(u32 i = 0; i < _arraycount(vec4); i++){
		auto v = vec4[i];
		glm::vec4 t = *((glm::vec4*)&v);

		auto magnitude = MagnitudeVec4(v);
		auto m = glm::length(t);

		DiffElement(&magnitude,&m,1);
	}	

	for(u32 i = 0; i < _arraycount(vec3); i++){
		auto v = vec3[i];
		glm::vec3 t = *((glm::vec3*)&v);

		auto magnitude = MagnitudeVec3(v);
		auto m = glm::length(t);

		DiffElement(&magnitude,&m,1);
	}	

	for(u32 i = 0; i < _arraycount(vec2); i++){
		auto v = vec2[i];
		glm::vec2 t = *((glm::vec2*)&v);

		auto magnitude = MagnitudeVec2(v);
		auto m = glm::length(t);

		DiffElement(&magnitude,&m,1);
	}	
	//Dot
	
	for(u32 i = 0; i < _arraycount(vec4); i++){
		auto v1 = vec4[i];

		Vec4 v2 = {};
		if(i + 1 < _arraycount(vec4)){
			v2 = vec4[i + 1];
		}
		else{
			v2 = vec4[0];
		}

		glm::vec4 t1 = *((glm::vec4*)&v1);
		glm::vec4 t2 = *((glm::vec4*)&v2);

		auto m = DotVec4(v1,v2);
		auto t = glm::dot(t1,t2);

		DiffElement(&m,&t,1);
	}

	for(u32 i = 0; i < _arraycount(vec3); i++){
		auto v1 = vec3[i];

		Vec3 v2 = {};
		if(i + 1 < _arraycount(vec3)){
			v2 = vec3[i + 1];
		}
		else{
			v2 = vec3[0];
		}

		glm::vec3 t1 = *((glm::vec3*)&v1);
		glm::vec3 t2 = *((glm::vec3*)&v2);

		auto m = DotVec3(v1,v2);
		auto t = glm::dot(t1,t2);

		DiffElement(&m,&t,1);
	}

	for(u32 i = 0; i < _arraycount(vec2); i++){
		auto v1 = vec2[i];

		Vec2 v2 = {};
		if(i + 1 < _arraycount(vec2)){
			v2 = vec2[i + 1];
		}
		else{
			v2 = vec2[0];
		}

		glm::vec2 t1 = *((glm::vec2*)&v1);
		glm::vec2 t2 = *((glm::vec2*)&v2);

		auto m = DotVec2(v1,v2);
		auto t = glm::dot(t1,t2);

		DiffElement(&m,&t,1);
	}
	//Norm
	
	for(u32 i = 0; i < _arraycount(vec4); i++){
		auto v = vec4[i];
		glm::vec4 t = *((glm::vec4*)&v);

		auto n = NormalizeVec4(v);
		auto m = glm::normalize(t);

		DiffElement(&n,&m,_arraycount(n.container));
	}	
	for(u32 i = 0; i < _arraycount(vec3); i++){
		auto v = vec3[i];
		glm::vec3 t = *((glm::vec3*)&v);

		auto n = NormalizeVec3(v);
		auto m = glm::normalize(t);

		DiffElement(&n,&m,_arraycount(n.container));
	}	
	for(u32 i = 0; i < _arraycount(vec2); i++){
		auto v = vec2[i];
		glm::vec2 t = *((glm::vec2*)&v);

		auto n = NormalizeVec2(v);
		auto m = glm::normalize(t);

		DiffElement(&n,&m,_arraycount(n.container));
	}	


	for(u32 i = 0; i < _arraycount(dq); i++){
		auto d = dq[i];
		auto t = *((glm::fdualquat*)&d);

		auto n = NormalizeDualQ(d);
		auto m = glm::normalize(t);

		DiffElement(&n,&m,8);
	}	

	//Schur
	
	for(u32 i = 0; i < _arraycount(vec4); i++){
		auto v1 = vec4[i];

		Vec4 v2 = {};
		if(i + 1 < _arraycount(vec4)){
			v2 = vec4[i + 1];
		}
		else{
			v2 = vec4[0];
		}

		glm::vec4 t1 = *((glm::vec4*)&v1);
		glm::vec4 t2 = *((glm::vec4*)&v2);

		auto m = SchurVec4(v1,v2);
		auto t = t1 * t2;

		DiffElement(&m,&t,_arraycount(m.container));
	}

	for(u32 i = 0; i < _arraycount(vec3); i++){
		auto v1 = vec3[i];

		Vec3 v2 = {};
		if(i + 1 < _arraycount(vec3)){
			v2 = vec3[i + 1];
		}
		else{
			v2 = vec3[0];
		}

		glm::vec3 t1 = *((glm::vec3*)&v1);
		glm::vec3 t2 = *((glm::vec3*)&v2);

		auto m = SchurVec3(v1,v2);
		auto t = t1 * t2;

		DiffElement(&m,&t,_arraycount(m.container));
	}

	for(u32 i = 0; i < _arraycount(vec2); i++){
		auto v1 = vec2[i];

		Vec2 v2 = {};
		if(i + 1 < _arraycount(vec2)){
			v2 = vec2[i + 1];
		}
		else{
			v2 = vec2[0];
		}

		glm::vec2 t1 = *((glm::vec2*)&v1);
		glm::vec2 t2 = *((glm::vec2*)&v2);

		auto m = SchurVec2(v1,v2);
		auto t = t1 * t2;

		DiffElement(&m,&t,_arraycount(m.container));
	}
	
	//Lerp

	for(u32 i = 0; i < _arraycount(vec4); i++){
		auto v1 = vec4[i];

		Vec4 v2 = {};
		if(i + 1 < _arraycount(vec4)){
			v2 = vec4[i + 1];
		}
		else{
			v2 = vec4[0];
		}

		glm::vec4 t1 = *((glm::vec4*)&v1);
		glm::vec4 t2 = *((glm::vec4*)&v2);

		auto m = LerpVec4(v1,v2,0.5f);
		auto t = glm::mix(t1,t2,0.5f);

		DiffElement(&m,&t,_arraycount(m.container));
	}

	for(u32 i = 0; i < _arraycount(vec3); i++){
		auto v1 = vec3[i];

		Vec3 v2 = {};
		if(i + 1 < _arraycount(vec3)){
			v2 = vec3[i + 1];
		}
		else{
			v2 = vec3[0];
		}

		glm::vec3 t1 = *((glm::vec3*)&v1);
		glm::vec3 t2 = *((glm::vec3*)&v2);

		auto m = LerpVec3(v1,v2,0.5f);
		auto t = glm::mix(t1,t2,0.5f);

		DiffElement(&m,&t,_arraycount(m.container));
	}

	for(u32 i = 0;i < _arraycount(vec3); i++){
		f32 s = vec3[i].x;
		f32 e = vec3[i].y;

		f32 a = Lerp(s,e,0.5f);
		f32 b = glm::mix(s,e,0.5f);

		DiffElement(&a,&b,1);
	}

	//Projection
	
	for(u32 i = 0; i < _arraycount(vec3); i++){
		auto v1 = vec3[i];

		Vec3 v2 = {};
		if(i + 1 < _arraycount(vec3)){
			v2 = vec3[i + 1];
		}
		else{
			v2 = vec3[0];
		}

		glm::vec3 t1 = *((glm::vec3*)&v1);
		glm::vec3 t2 = *((glm::vec3*)&v2);

		auto m = ProjectOntoVec3(v1,v2);
		auto t = glm::proj(t1,t2);

		DiffElement(&m,&t,_arraycount(m.container));
	}
	//Rejection

	for(u32 i = 0; i < _arraycount(vec3); i++){
		auto v1 = vec3[i];

		Vec3 v2 = {};
		if(i + 1 < _arraycount(vec3)){
			v2 = vec3[i + 1];
		}
		else{
			v2 = vec3[0];
		}

		glm::vec3 t1 = *((glm::vec3*)&v1);
		glm::vec3 t2 = *((glm::vec3*)&v2);

		auto m = RejectVec3(v1,v2);
		auto t = t1 - glm::proj(t1,t2);

		DiffElement(&m,&t,_arraycount(m.container));
	}
	
	//RotateVec
	for(u32 i = 0; i < _arraycount(vec3); i++){
		auto v = vec3[i];

		Vec3 r = {};
		if(i + 1 < _arraycount(vec3)){
			r = vec3[i + 1];
		}
		else{
			r = vec3[0];
		}


		glm::vec3 t1 = *((glm::vec3*)&v);
		glm::vec3 t2 = *((glm::vec3*)&r);

		auto m = RotateVec3(v,r);
		auto t = glm::rotateX(t1,t2.x);
		t = glm::rotateY(t,t2.y);
		t = glm::rotateZ(t,t2.z);

		DiffElement(&m,&t,_arraycount(m.container));
	}

	for(u32 i = 0; i < _arraycount(vec2); i++){
		auto v = vec2[i];

		Vec2 r = {};
		if(i + 1 < _arraycount(vec2)){
			r = vec2[i + 1];
		}
		else{
			r = vec2[0];
		}

		glm::vec2 t1 = *((glm::vec2*)&v);
		glm::vec2 t2 = *((glm::vec2*)&r);

		auto m = RotateVec2(v,r.x);
		auto t = glm::rotate(t1,t2.x);

		DiffElement(&m,&t,_arraycount(m.container));
	}
	
	//RotateAxisVec3
	for(u32 i = 0; i < _arraycount(vec3); i++){
		auto v = vec3[i];

		Vec3 r = {};
		if(i + 1 < _arraycount(vec3)){
			r = vec3[i + 1];
		}
		else{
			r = vec3[0];
		}

		r = NormalizeVec3(r);


		glm::vec3 t1 = *((glm::vec3*)&v);
		glm::vec3 t2 = *((glm::vec3*)&r);

		auto m = RotateAxisVec3(v,r,0.123f);
		auto t = glm::rotate(t1,0.123f,t2);

		DiffElement(&m,&t,_arraycount(m.container));
	}

	//QuatRotateVec3
	for(u32 i = 0; i < _arraycount(quat); i++){
		auto q = quat[i];
		auto v = vec3[i];

		auto t1 = *(glm::fquat*)&q;
		auto t2 = *(glm::vec3*)&v;

		auto m = QuatRotateVec3(v,q);
		auto t = glm::rotate(t1,t2);
	}

	//Cosf
	
	for(u32 i = 0; i < _arraycount(vec3); i++){
		auto v1 = vec3[i];

		Vec3 v2 = {};
		if(i + 1 < _arraycount(vec3)){
			v2 = vec3[i + 1];
		}
		else{
			v2 = vec3[0];
		}

		glm::vec3 t1 = *((glm::vec3*)&v1);
		glm::vec3 t2 = *((glm::vec3*)&v2);

		auto m = Cosf(v1,v2);
		auto t = glm::dot(glm::normalize(t1),glm::normalize(t2));

		DiffElement(&m,&t,1);
	}

	//Reflect

	for(u32 i = 0; i < _arraycount(vec3); i++){
		auto v1 = vec3[i];

		Vec3 v2 = {};
		if(i + 1 < _arraycount(vec3)){
			v2 = vec3[i + 1];
		}
		else{
			v2 = vec3[0];
		}

		auto n = NormalizeVec3(CrossVec3(v1,v2));

		glm::vec3 t1 = *((glm::vec3*)&v1);
		glm::vec3 t2 = *((glm::vec3*)&n);

		auto m = ReflectVec3(v1,n);
		auto t = glm::reflect(t1,t2);

		DiffElement(&m,&t,1);
	}

	//Quat inverses
	for(u32 i = 0; i < _arraycount(quat); i++){
		auto q = quat[i];
		auto t = *(glm::fquat*)&q;

		q = InverseQuat(q);
		t = glm::inverse(t);

		DiffElement(&q,&t,4);
	}

	//NLerp
	for(u32 i = 0; i < _arraycount(quat); i++){
		auto q1 = quat[i];

		Quat q2 = {};

		if(i + 1 < _arraycount(quat)){
			q2 = quat[i + 1];
		}
		else{
			q2 = quat[0];
		}


		auto t1 = *(glm::fquat*)&q1;
		auto t2 = *(glm::fquat*)&q2;

		//NOTE: glm doesn't do a neighbourhood operation
		if(glm::dot(t1,t2) < 0.0f){
			t2 = t2 * -1.0f;
		}

		auto q = NLerpQuat(q1,q2,0.5f);
		auto t = glm::fastMix(t1,t2,0.5f);

		DiffElement(&q,&t,4);
	}
	//SLerp
	for(u32 i = 0; i < _arraycount(quat); i++){
		auto q1 = quat[i];

		Quat q2 = {};

		if(i + 1 < _arraycount(quat)){
			q2 = quat[i + 1];
		}
		else{
			q2 = quat[0];
		}


		auto t1 = *(glm::fquat*)&q1;
		auto t2 = *(glm::fquat*)&q2;


		if(glm::dot(t1,t2) < 0.0f){
			t2 = t2 * -1.0f;
		}

		auto q = SLerpQuat(q1,q2,0.5f);
		auto t = glm::mix(t1,t2,0.5f);

		DiffElement(&q,&t,4);
	}

	//Conjugate
	for(u32 i = 0; i < _arraycount(quat); i++){
		auto q = quat[i];
		auto t = *(glm::fquat*)&q;

		q = ConjugateQuat(q);
		t = glm::conjugate(t);

		DiffElement(&q,&t,4);
	}
	//Intersection code
	
	for(u32 i = 0; i < _arraycount(ray3); i++){
		auto r = ray3[i];
		auto p = planes[i];

		if(i == 0){
			auto pos = r.pos + (r.dir * 20.0f);
			auto norm = r.dir * -1.0f;

			p = ConstructPlanePos(norm,pos);

		}

		Vec3 out = {};
		f32 t = 0.0f;

		auto b1 = IntersectOutRay3Plane(r,p,&out);
		bool b2 = false;

		{
			auto rpos = *((glm::vec3*)&r.pos);
			auto rd = *((glm::vec3*)&r.dir);

			auto pos = GetPlanePos(p);

			auto ppos = *((glm::vec3*)&pos);
			auto pn = *((glm::vec3*)&p.norm);

			b2 = glm::intersectRayPlane(rpos,rd,
					ppos,
					pn,t);
		}

		if(b1 != b2){
			printf("ERROR Diff results\n");
			_breakpoint();
		}

		if(b1 && b2){
			auto t_p = (t * r.dir) + r.pos;
			DiffElement(&out,&t_p,3);
		}

	}


	for(u32 i = 0; i < _arraycount(line3); i++){
		auto r = line3[i];
		auto p = planes[i];


		Vec3 out = {};
		f32 t = 0.0f;

		auto b1 = IntersectOutLine3Plane(r,p,&out);
		bool b2 = false;

		{
			auto rpos = *((glm::vec3*)&r.pos);
			auto rd = *((glm::vec3*)&r.dir);

			auto pos = GetPlanePos(p);

			auto ppos = *((glm::vec3*)&pos);
			auto pn = *((glm::vec3*)&p.norm);

			auto t1 = glm::intersectRayPlane(rpos,rd,
					ppos,
					pn,t);

			bool t2 = false;

			if(!t1){
				t2 = glm::intersectRayPlane(rpos,-rd,
						ppos,
						pn,t);
				t *= -1.0f;
			}


			b2 = t1 || t2;
		}

		if(b1 != b2){
			printf("ERROR Diff results\n");
			_breakpoint();
		}

		if(b1 && b2){
			auto t_p = (t * r.dir) + r.pos;
			DiffElement(&out,&t_p,3);
		}

	}


	for(u32 i = 0; i < _arraycount(line3); i++){
		auto r = line3[i];

		auto s = ConstructSphere(r.pos + (r.dir * 50.0f),30.0f);

		Vec3 out = {};

		auto b1 = IntersectClosestOutLine3Sphere(r,s,&out);

		bool b2 = false;

		f32 t = 0.0f;

		{
			auto rpos = *((glm::vec3*)&r.pos);
			auto rd = *((glm::vec3*)&r.dir);

			auto sp = *((glm::vec3*)&s.pos);
			f32 r = s.radius * s.radius;

			auto t1 = glm::intersectRaySphere(rpos,rd,
					sp,r,t);

			bool t2 = false;

			if(!t1){
			t2 = glm::intersectRaySphere(rpos,-rd,
					sp,r,t);

			t *= -1.0f;
			}
			b2 = t1 || t2;
		}
		if(b1 != b2){
			printf("ERROR Diff results\n");
			_breakpoint();
		}

		if(b1 && b2){
			auto t_p = (t * r.dir) + r.pos;
			DiffElement(&out,&t_p,3);
		}

	}

	for(u32 i = 0; i < _arraycount(ray3); i++){
		auto r = ray3[i];

		auto s = ConstructSphere(r.pos + (r.dir * 50.0f),30.0f);

		Vec3 out = {};

		auto b1 = IntersectClosestOutRay3Sphere(r,s,&out);

		bool b2 = false;

		f32 t = 0.0f;

		{
			auto rpos = *((glm::vec3*)&r.pos);
			auto rd = *((glm::vec3*)&r.dir);

			auto sp = *((glm::vec3*)&s.pos);
			f32 r = s.radius * s.radius;
			b2 = glm::intersectRaySphere(rpos,rd,
					sp,r,t);
		}
		if(b1 != b2){
			printf("ERROR Diff results\n");
			_breakpoint();
		}

		if(b1 && b2){
			auto t_p = (t * r.dir) + r.pos;
			DiffElement(&out,&t_p,3);
		}

	}


	//LinearDQ
	for(u32 i = 0; i < _arraycount(dq); i++){
		auto a = dq[i];
		auto b = dq[WrapAround(i + 1,_arraycount(dq))];
		auto c = LerpDualQ(a,b,0.5f);

		auto t1 = *((glm::fdualquat*)&a);
		auto t2 = *((glm::fdualquat*)&b);

		if(glm::dot(t2.real,t1.real)< 0.0f){
			t2 = t2 * -1.0f;
		}

		auto t = glm::normalize(glm::lerp(t1,t2,0.5f));

		DiffElement(&c,&t,_arraycount(c.container));
	}

	//ScLerp -- The results will deviate by quite a bit (glm::lerp)
	//is not accurate here
	
	printf("START SCLERP. ERRORS EXPECTED\n");
	for(u32 i = 0; i < _arraycount(dq); i++){
		auto a = dq[i];
		auto b = dq[WrapAround(i + 1,_arraycount(dq))];
		auto c = ScLerp(a,b,0.5f);

		auto t1 = *((glm::fdualquat*)&a);
		auto t2 = *((glm::fdualquat*)&b);

		if(glm::dot(t2.real,t1.real)< 0.0f){
			t2 = t2 * -1.0f;
		}

		auto t = glm::normalize(glm::lerp(t1,t2,0.5f));


		DiffElement(&c,&t,_arraycount(c.container));
	}
	printf("END SCLERP\n");

}

void MathTest() {
	// NOTE: We will test against glm
	printf("\n\nRUNNING Math tests\n");

	MathConstructor();
	MathDeconstructor();
	MathConversions();
	MathOps();
	SpecialMathOps();
}

int main(s32 argc, s8** argv) {
	MathTest();

	auto res = HashTest();

	return res;
}
