#pragma once

/*

  NOTE:
  Vec4(x,y,z,1) - point  rotating this changes the point position
  Vec4(x,y,z,0) - vector rotating this will create a new vector(eg rotating a translation)
*/

// AVX2 is slow. to get full speed of avx, it has to downclock the cpu
// maybe this will change in time
// (https://gist.github.com/rygorous/32bc3ea8301dba09358fd2c64e02d774)

#include "iintrin.h"
#include "math.h"  //we can replace this too I guess

#define _pi 3.1415926535897f
#define _twopi (3.1415926535897f * 2.0f)
#define _fourpi (3.1415926535897f * 4.0f)
#define _halfpi (3.1415926535897f * 0.5f)

#ifdef _row_major

#define MATRIX_ROW_MAJOR _row_major

#else

#define MATRIX_ROW_MAJOR 1

#endif

#define DEPTH_ZERO_TO_ONE 1
#define Z_RHS 1

#define _radians(degrees) (degrees * (_pi / 180.0f))
#define _degrees(radians) (radians * (180.0f / _pi))

// direct element access -- just make these into numbers by themselves
#define _ac4(x, y) (x + (y * 4))
#define _ac3(x, y) (x + (y * 3))
#define _ac2(x, y) (x + (y * 2))

// access element as if it were row major
#if MATRIX_ROW_MAJOR

#define _rc4(x, y) _ac4(x, y)
#define _rc3(x, y) _ac3(x, y)
#define _rc2(x, y) _ac2(x, y)

#else

#define _rc4(y, x) [x + (y * 4)]
#define _rc3(y, x) [x + (y * 3)]
#define _rc2(y, x) [x + (y * 2)]

#endif

#define _clampf(x, lower, upper) (fminf(upper, fmaxf(x, lower)))

// MARK:Linear Algebra

typedef union Quaternion {
	__m128 simd;

	struct {
		f32 w, x, y, z;
	};

} Quaternion _align(16);

typedef struct DualQuaternion {
	Quaternion q1, q2;
} DualQuaternion;

typedef struct Vector2 {
	f32 x, y;
} Vector2;

typedef union Vector4 {
	__m128 simd;

	struct {
		f32 x, y, z, w;
	};

	Vector2 vec2[2];

	f32 floats[4];

} Vector4 _align(16);

typedef struct Vector4SOA {
	ptrsize count;
	union {
		struct {  // refers to 4 floats at a time
			__m128* simd_x;
			__m128* simd_y;
			__m128* simd_z;
			__m128* simd_w;
		};

		struct {  // refers to 1 floats at a time
			f32* x;
			f32* y;
			f32* z;
			f32* w;
		};
	};

} Vector4SOA;

typedef union Vector3 {
	struct {
		f32 x, y, z;
	};

	f32 floats[3];
	Vector2 vec2;
} Vector3;

Vector4 _ainline Vec3ToVec4(Vector3 vec) {
	Vector4 ret = {vec.x, vec.y, vec.z, 1.0f};
	return ret;
}

Vector3 _ainline Vec4ToVec3(Vector4 vec) { 

	Vector3 ret = {vec.x, vec.y, vec.z};


	return ret; 
}

Vector2 _ainline Vec4ToVec2(Vector4 vec) { 

	Vector2 ret = {vec.x, vec.y};

	return ret; 
}

Vector2 _ainline Vec3ToVec2(Vector3 vec) { 
	Vector2 ret = {vec.x, vec.y};
	return ret; 
}

typedef union Mat4 {
	f32 container[16];
	__m128 simd[4];

#ifdef __cplusplus_
	f32& operator[](u32 index) { return container[index]; }
#endif
} Mat4 _align(16);

// NOTE: do we want to use simd for 3
typedef union Mat3 {
	f32 container[9];

	struct {
		__m128 simd[2];
		f32 k;
	};
#ifdef __cplusplus
	f32& operator[](u32 index) { return container[index]; }
#endif
} Mat3 _align(16);

typedef union Mat2 {
	f32 container[4];

	struct {
		__m128 simd;
	};
#ifdef __cplusplus
	f32& operator[](u32 index) { return container[index]; }
#endif
} Mat2 _align(16);



void Mat4SetMember(Mat4 mat,u32 index,f32 value){

	 mat.container[index] = value;
	
}
void Mat3SetMember(Mat3 mat,u32 index,f32 value){

	 mat.container[index] = value;
}

void Mat2SetMember(Mat2 mat,u32 index,f32 value){

	 mat.container[index] = value;
}


f32 Mat4GetMember(Mat4 mat,u32 index){

	return mat.container[index];
	
}
f32 Mat3GetMember(Mat3 mat,u32 index){

	return mat.container[index];
}

f32 Mat2GetMember(Mat2 mat,u32 index){

	return mat.container[index];
}

Mat4 _ainline IdentityMat4() {
	Mat4 matrix = {

	    {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}};

	return matrix;
}

Mat3 _ainline IdentityMat3() {
	Mat3 matrix = {{
	    1,
	    0,
	    0,
	    0,
	    1,
	    0,
	    0,
	    0,
	    1,
	}};

	return matrix;
}

Mat2 _ainline IdentityMat2() {
	Mat2 matrix = {{
	    1,
	    0,
	    0,
	    1,
	}};

	return matrix;
}

Mat2 _ainline Mat3ToMat2(Mat3 mat) {
	Mat2 m;

	Mat2SetMember(m,0,Mat3GetMember(mat,_ac3(0,0)));
	Mat2SetMember(m,1,Mat3GetMember(mat,_ac3(1,0)));

	Mat2SetMember(m,2,Mat3GetMember(mat,_ac3(0,1)));
	Mat2SetMember(m,3,Mat3GetMember(mat,_ac3(1,1)));

	return m;
}

Mat2 _ainline Mat4ToMat2(Mat4 mat) {
	Mat2 m;

	Mat2SetMember(m,0,Mat4GetMember(mat,_ac4(0,0)));
	Mat2SetMember(m,1,Mat4GetMember(mat,_ac4(1,0)));

	Mat2SetMember(m,2,Mat4GetMember(mat,_ac4(0,1)));
	Mat2SetMember(m,3,Mat4GetMember(mat,_ac4(1,1)));

	return m;
}

Mat3 _ainline Mat2ToMat3(Mat2 mat) {
	Mat3 m = IdentityMat3();

	Mat3SetMember(m,_ac3(0,0),Mat2GetMember(mat,0));
	Mat3SetMember(m,_ac3(1,0),Mat2GetMember(mat,1));

	Mat3SetMember(m,_ac3(0,1),Mat2GetMember(mat,2));
	Mat3SetMember(m,_ac3(1,1),Mat2GetMember(mat,3));

	return m;
}

Mat3 _ainline Mat4ToMat3(Mat4 mat) {
	Mat3 m;

	Mat3SetMember(m,0,Mat4GetMember(mat,_ac4(0,0)));
	Mat3SetMember(m,1,Mat4GetMember(mat,_ac4(1,0)));
	Mat3SetMember(m,2,Mat4GetMember(mat,_ac4(2,0)));

	Mat3SetMember(m,3,Mat4GetMember(mat,_ac4(0,1)));
	Mat3SetMember(m,4,Mat4GetMember(mat,_ac4(1,1)));
	Mat3SetMember(m,5,Mat4GetMember(mat,_ac4(2,1)));

	Mat3SetMember(m,6,Mat4GetMember(mat,_ac4(0,2)));
	Mat3SetMember(m,7,Mat4GetMember(mat,_ac4(1,2)));
	Mat3SetMember(m,8,Mat4GetMember(mat,_ac4(2,2)));

	return m;
}

Mat4 _ainline Mat2ToMat4(Mat2 mat) {
	Mat4 m = IdentityMat4();

 	Mat4SetMember(m,_ac4(0,0),Mat2GetMember(mat,0));
	Mat4SetMember(m,_ac4(1,0),Mat2GetMember(mat,1));

	Mat4SetMember(m,_ac4(0,1),Mat2GetMember(mat,2));
	Mat4SetMember(m,_ac4(1,1),Mat2GetMember(mat,3));

	return m;
}

Mat4 _ainline Mat3ToMat4(Mat3 mat) {
	Mat4 m = IdentityMat4();

	Mat4SetMember(m,_ac4(0,0),Mat3GetMember(mat,0));
	Mat4SetMember(m,_ac4(1,0),Mat3GetMember(mat,1));
	Mat4SetMember(m,_ac4(2,0),Mat3GetMember(mat,2));

	Mat4SetMember(m,_ac4(0,1),Mat3GetMember(mat,3));
	Mat4SetMember(m,_ac4(1,1),Mat3GetMember(mat,4));
	Mat4SetMember(m,_ac4(2,1),Mat3GetMember(mat,5));

	Mat4SetMember(m,_ac4(0,2),Mat3GetMember(mat,6));
	Mat4SetMember(m,_ac4(1,2),Mat3GetMember(mat,7));
	Mat4SetMember(m,_ac4(2,2),Mat3GetMember(mat,8));

	return m;
}

// Math operations
#ifndef __cplusplus

//C operations here

#else


Mat4 operator+(Mat4 lhs, Mat4 rhs);
Mat4 operator-(Mat4 lhs, Mat4 rhs);
Mat4 operator*(Mat4 lhs, Mat4 rhs);
Mat4 operator*(f32 lhs, Mat4 rhs);
Mat4 operator*(Mat4 lhs, f32 rhs);
Mat4 operator/(Mat4 lhs, Mat4 rhs);


Mat3 operator+(Mat3 lhs, Mat3 rhs);
Mat3 operator-(Mat3 lhs, Mat3 rhs);
Mat3 operator*(Mat3 lhs, Mat3 rhs);
Mat3 operator*(f32 lhs, Mat3 rhs);
Mat3 operator*(Mat3 lhs, f32 rhs);
Mat3 operator/(Mat3 lhs, Mat3 rhs);


Mat2 operator+(Mat2 lhs, Mat2 rhs);
Mat2 operator-(Mat2 lhs, Mat2 rhs);
Mat2 operator*(Mat2 lhs, Mat2 rhs);
Mat2 operator*(f32 lhs, Mat2 rhs);
Mat2 operator*(Mat2 lhs, f32 rhs);
Mat2 operator/(Mat2 lhs, Mat2 rhs);

#endif

Mat4 Transpose(Mat4 matrix);
Mat4 Inverse(Mat4 matrix);
Mat3 Transpose(Mat3 matrix);
Mat3 Inverse(Mat3 matrix);
Mat2 Transpose(Mat2 matrix);
Mat2 Inverse(Mat2 matrix);

// conversions
Mat4 QuaternionToMat(Quaternion quaternion);
Quaternion MatToQuaternion(Mat4 matrix);
DualQuaternion ConstructDualQuaternion(Mat4 transform);
Mat4 DualQuaternionToMat(DualQuaternion d);

Vector3 _ainline MatToTranslationVector(Mat4 matrix) {
	return Vector3{matrix _rc4(3, 0), matrix _rc4(3, 1), matrix _rc4(3, 2)};
}

// print
void PrintMat(Mat4 matrix);
void PrintMat(Mat3 matrix);

Mat4 ViewMat(Vector3 position, Vector3 lookpoint, Vector3 updir);
Mat4 ProjectionMat(f32 fov, f32 aspectration, f32 nearz, f32 farz);

Mat4 _ainline PositionMat(Vector3 position) {
	Mat4 matrix = IdentityMat4();

	matrix _rc4(3, 0) = position.x;
	matrix _rc4(3, 1) = position.y;
	matrix _rc4(3, 2) = position.z;

	return matrix;
}

// MARK: not tested
Mat3 _ainline RotationMat(Vector3 rotation) {
	f32 cosv = cosf(rotation.x);
	f32 sinv = sinf(rotation.x);

	Mat3 rotationx_matrix3 = IdentityMat3();

	rotationx_matrix3 _rc3(1, 1) = cosv;
	rotationx_matrix3 _rc3(2, 1) = -sinv;
	rotationx_matrix3 _rc3(1, 2) = sinv;
	rotationx_matrix3 _rc3(2, 2) = cosv;

	cosv = cosf(rotation.y);
	sinv = sinf(rotation.y);

	Mat3 rotationy_matrix3 = IdentityMat3();

	rotationy_matrix3 _rc3(0, 0) = cosv;
	rotationy_matrix3 _rc3(2, 0) = sinv;
	rotationy_matrix3 _rc3(0, 2) = -sinv;
	rotationy_matrix3 _rc3(2, 2) = cosv;

	cosv = cosf(rotation.z);
	sinv = sinf(rotation.z);

	Mat3 rotationz_matrix3 = IdentityMat3();

	rotationz_matrix3 _rc3(0, 0) = cosv;
	rotationz_matrix3 _rc3(1, 0) = -sinv;
	rotationz_matrix3 _rc3(0, 1) = sinv;
	rotationz_matrix3 _rc3(1, 1) = cosv;

	return rotationz_matrix3 * rotationy_matrix3 * rotationx_matrix3;
}

Mat4 _ainline ScaleMat(Vector3 scale) {
	Mat4 matrix = {{scale.x, 0, 0, 0, 0, scale.y, 0, 0, 0, 0, scale.z, 0, 0, 0, 0, 1}};

	return matrix;
}

Mat4 WorldMat(Mat4 position, Mat4 rotation, Mat4 scale);

Mat4 WorldMat(Vector3 position, Vector3 rotation, Vector3 scale);

Mat4 WorldMat(Vector3 position, Quaternion rotation, Vector3 scale);

Vector4 WorldSpaceToClipSpaceVec4(Vector4 pos, Mat4 viewproj);
Vector4 ClipSpaceToWorldSpaceVec4(Vector4 pos, Mat4 viewproj);

Vector3 _ainline WorldSpaceToClipSpaceVec3(Vector3 pos, Mat4 viewproj) {
	return Vec4ToVec3(WorldSpaceToClipSpaceVec4(Vec3ToVec4(pos), viewproj));
}

Vector3 _ainline ClipSpaceToWorldSpaceVec3(Vector3 pos, Mat4 viewproj) {
	return Vec4ToVec3(ClipSpaceToWorldSpaceVec4(Vec3ToVec4(pos), viewproj));
}

typedef Vector4 Point4;
typedef Vector3 Point3;
typedef Vector2 Point2;

struct Line3 {
	Point3 pos;
	Vector3 dir;
};

struct Plane {
	Point3 pos;
	Vector3 norm;
};

struct Line2 {
	Point2 pos;
	Vector2 dir;
};

enum IntersectType : u8 {
	INTERSECT_FALSE = 0,
	INTERSECT_FINITE = 1,
	INTERSECT_INFINITE = 2,
};

2 Intersect(Line2 a, Line2 b);

2 Intersect(Line2 a, Line2 b, Point2* out_point);

2 TypedIntersect(Line2 a, Line2 b);

f32 Magnitude(Vector3 vec);
Vector3 Cross(Vector3 vec1, Vector3 vec2);
f32 Dot(Vector3 vec1, Vector3 vec2);
f32 cosf(Vector3 vec1, Vector3 vec2);
Vector3 Normalize(Vector3 vec);
Vector3 GetVectorRotation(Vector3 lookat);

Vector3 Vec3(f32 x, f32 y, f32 z);

f32 Component(Vector3 a, Vector3 b);

Vector3 ProjectOnto(Vector3 a, Vector3 b);

Vector3 ProjectVectorOntoPlane(Vector3 vec, Plane plane);

// line intersections

2 Intersect(Line3 a, Line3 b);

2 Intersect(Line3 a, Line3 b, Point3* out_point);

2 TypedIntersect(Line3 a, Line3 b);

// plane intersections

2 Intersect(Line3 a, Plane b);

2 TypedIntersect(Line3 a, Plane b);

2 Intersect(Line3 a, Plane b, Point3* out_point);

Vector4 Normalize(Vector4 vec);
f32 Magnitude(Vector4 vec);
f32 Dot(Vector4 vec1, Vector4 vec2);
Vector4 VectorComponentMul(Vector4 a, Vector4 b);

Vector4 operator+(Vector4 lhs, Vector4 rhs);
Vector4 operator-(Vector4 lhs, Vector4 rhs);
Vector4 operator*(f32 lhs, Vector4 rhs);
Vector4 operator*(Vector4 lhs, f32 rhs);
Vector4 operator/(Vector4 lhs, f32 rhs);

Vector3 operator+(Vector3 lhs, Vector3 rhs);
Vector3 operator-(Vector3 lhs, Vector3 rhs);
Vector3 operator*(f32 lhs, Vector3 rhs);
Vector3 operator*(Vector3 lhs, f32 rhs);
Vector3 operator/(Vector3 lhs, f32 rhs);

Vector2 operator+(Vector2 lhs, Vector2 rhs);
Vector2 operator-(Vector2 lhs, Vector2 rhs);
Vector2 operator*(f32 lhs, Vector2 rhs);
Vector2 operator*(Vector2 lhs, f32 rhs);
Vector2 operator/(Vector2 lhs, f32 rhs);

Vector2 Normalize(Vector2 a);

f32 Dot(Vector2 a, Vector2 b);

Vector2 CompMul(Vector2 a, Vector2 b);

f32 AngleQuadrant(f32 x, f32 y);

f32 Magnitude(Vector2 vec);

Vector2 RotateVector(Vector2 vec, f32 rotation);

Vector3 RotateVector(Vector3 vec, Vector3 rotation);

void PrintVector4(Vector4 vec);
void PrintVector3(Vector3 vec);
void PrintVector2(Vector2 vec);

struct Triangle {
	Point3 a;
	Point3 b;
	Point3 c;
};

//?
struct Teathedron {};

struct Polygon {
	Point3* point_array;
	ptrsize count;

	Point3& operator[](u32 index) { return point_array[index]; }
};

void MinkowskiAddition(Point3* a, ptrsize a_count, Point3* b, ptrsize b_count, Point3** ret);

void MinkowskiDifference(Point3* a, ptrsize a_count, Point3* b, ptrsize b_count, Point3** ret);

typedef Vector4SOA PolygonSOA;

Vector4 _ainline QuatToVec4(Quaternion q) {
	Vector4 v;

	v.x = q.w;
	v.y = q.x;
	v.z = q.y;
	v.w = q.z;

	return v;
}

Quaternion _ainline Vec4ToQuat(Vector4 v) {
	Quaternion q;

	q.w = v.x;
	q.x = v.y;
	q.y = v.z;
	q.z = v.w;

	return q;
}

Quaternion _ainline Normalize(Quaternion a) { return Vec4ToQuat(Normalize(QuatToVec4(a))); }

f32 _ainline Magnitude(Quaternion a) { return Magnitude(QuatToVec4(a)); }

f32 _ainline Dot(Quaternion a, Quaternion b) { return Dot(QuatToVec4(a), QuatToVec4(b)); }

Quaternion _ainline operator+(Quaternion lhs, Quaternion rhs) {
	Vector4 l = QuatToVec4(lhs);
	Vector4 r = QuatToVec4(rhs);

	return Vec4ToQuat(l + r);
}

Quaternion _ainline operator-(Quaternion lhs, Quaternion rhs) {
	Vector4 l = QuatToVec4(lhs);
	Vector4 r = QuatToVec4(rhs);

	return Vec4ToQuat(l - r);
}

Quaternion _ainline operator*(f32 lhs, Quaternion rhs) {
	Vector4 r = QuatToVec4(rhs);

	return Vec4ToQuat(lhs * r);
}

Quaternion _ainline operator*(Quaternion lhs, f32 rhs) {
	Vector4 l = QuatToVec4(lhs);

	return Vec4ToQuat(l * rhs);
}

Quaternion _ainline operator/(Quaternion lhs, f32 rhs) {
	Vector4 l = QuatToVec4(lhs);

	return Vec4ToQuat(l / rhs);
}

Quaternion operator*(Quaternion lhs, Quaternion rhs);

Quaternion Inverse(Quaternion q);

Vector3 RotateVector(Vector3 v, Quaternion q);

Quaternion ConstructQuaternion(Vector3 vector, f32 angle);

Quaternion ConjugateQuaternion(Quaternion quaternion);

void DeconstructQuaternion(Quaternion quaternion, Vector3* vector, f32* angle);

Quaternion _ainline MQuaternionIdentity() { return Quaternion{1.0f, 0.0f, 0.0f, 0.0f}; }

Quaternion _ainline AQuaternionIdentity() { return Quaternion{0.0f, 0.0f, 0.0f, 0.0f}; }

void PrintQuaternion(Quaternion quat);

Vector4 _ainline InterpolateVector(Vector4 a, Vector4 b, f32 step) {
	return {_intrin_fmadd_ps(_mm_sub_ps(b.simd, a.simd), _mm_set1_ps(step), a.simd)};
}

Quaternion _ainline InterpolateQuaternion(Quaternion a, Quaternion b, f32 step) {
	return {_intrin_fmadd_ps(_mm_sub_ps(b.simd, a.simd), _mm_set1_ps(step), a.simd)};
}

f32 _ainline Interpolate(f32 a, f32 b, f32 step) { return (a + (step * (b - a))); }

Quaternion NLerp(Quaternion a, Quaternion b, f32 step);

Quaternion SLerp(Quaternion a, Quaternion b, f32 step);

DualQuaternion ConstructDualQuaternion(Quaternion rotation, Vector3 translation);

DualQuaternion operator+(DualQuaternion lhs, DualQuaternion rhs);
DualQuaternion operator-(DualQuaternion lhs, DualQuaternion rhs);
DualQuaternion operator*(DualQuaternion lhs, DualQuaternion rhs);
DualQuaternion operator*(f32 lhs, DualQuaternion rhs);
DualQuaternion operator*(DualQuaternion lhs, f32 rhs);

DualQuaternion Normalize(DualQuaternion d);
