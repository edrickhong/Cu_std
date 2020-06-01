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
#include "ttype.h"

#define _pi 3.1415926535897f
#define _twopi (_pi * 2.0f)
#define _fourpi (_pi * 4.0f)
#define _halfpi (_pi * 0.5f)

// NOTE: This defines the margin of error for fp intersection tests
#define _f32_error_offset 0.0001f

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
#define _ac4(mat,x, y) mat.container[(x + (y * 4))]
#define _ac3(mat,x, y) mat.container[(x + (y * 3))]
#define _ac2(mat,x, y) mat.container[(x + (y * 2))]

// access element as if it were row major
#if MATRIX_ROW_MAJOR

#define _rc4(mat,x, y) _ac4(mat,x, y)
#define _rc3(mat,x, y) _ac3(mat,x, y)
#define _rc2(mat,x, y) _ac2(mat,x, y)

#else

#define _rc4(mat,x, y) _ac4(mat,y, x)
#define _rc3(mat,x, y) _ac3(mat,y, x)
#define _rc2(mat,x, y) _ac2(mat,y, x)

#endif

#define _clampf(x, lower, upper) (fminf(upper, fmaxf(x, lower)))

// MARK: structs and unions
typedef struct Vec2 {
	f32 x, y;
} Vec2;

typedef union Vec3 {
	struct {
		f32 x, y, z;
	};

	struct {
		f32 r, g, b;
	};

	struct {
		f32 R, G, B;
	};

	f32 floats[3];
	Vec2 vec2;
} Vec3;

_align(16) typedef union Vec4 {
	__m128 simd;

	struct {
		f32 x, y, z, w;
	};

	struct {
		f32 R, G, B, A;
	};

	struct {
		f32 r, g, b, a;
	};

	Vec2 vec2[2];

	f32 floats[4];

} Vec4;

typedef struct Vec4SOA {
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

} Vec4SOA;

_align(16) typedef union Quat {
	__m128 simd;

	struct {
		f32 w, x, y, z;
	};

} Quat;

typedef struct DualQuat {
	Quat q1, q2;
} DualQuat;

typedef Vec4 Color4;
typedef Vec3 Color3;
typedef Vec4 Point4;
typedef Vec3 Point3;
typedef Vec2 Point2;

_align(16) typedef union Mat4 {
	f32 container[16];
	__m128 simd[4];

#ifdef __cplusplus_
	f32& operator[](u32 index) { return container[index]; }
#endif
} Mat4;

// NOTE: do we want to use simd for 3
_align(16) typedef union Mat3 {
	f32 container[9];

	struct {
		__m128 simd[2];
		f32 k;
	};
#ifdef __cplusplus
	f32& operator[](u32 index) { return container[index]; }
#endif
} Mat3;

_align(16) typedef union Mat2 {
	f32 container[4];

	struct {
		__m128 simd;
	};
#ifdef __cplusplus
	f32& operator[](u32 index) { return container[index]; }
#endif
} Mat2;

typedef struct Triangle {
	Point3 a;
	Point3 b;
	Point3 c;
} Triangle;

// typedef struct Teathedron{}Teathedron;

typedef struct Poly {
	Point3* point_array;
	ptrsize count;

#ifdef __cplusplus
	Point3& operator[](u32 index) { return point_array[index]; }
#endif

} Poly;

typedef struct Line3 {
	Point3 pos;
	Vec3 dir;
} Line3;

typedef struct Plane {
	Vec3 norm;
	f32 d;
} Plane;

typedef struct Line2 {
	Point2 pos;
	Vec2 dir;
} Line2;

typedef struct Ray2 {
	Point2 pos;
	Vec2 dir;
} Ray2;

typedef struct Ray3 {
	Point3 pos;
	Vec3 dir;
} Ray3;

typedef struct Sphere {
	Vec3 pos;
	f32 radius;
} Sphere;

typedef enum IntersectType {
	INTERSECT_FALSE = 0,
	INTERSECT_FINITE = 1,
	INTERSECT_INFINITE = 2,
} IntersectType;

typedef Vec4SOA PolygonSOA;

// MARK: Identities

#ifdef __cplusplus
extern "C" {
#endif

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

Quat _ainline MQuatIdentity() {
	Quat q = {1.0f, 0.0f, 0.0f, 0.0f};
	return q;
}
Quat _ainline AQuatIdentity() {
	Quat q = {0.0f, 0.0f, 0.0f, 0.0f};
	return q;
}

// MARK: Conversions

Mat2 _ainline Mat3ToMat2(Mat3 mat) {
	Mat2 m;

	m.container[0] = _ac3(mat,0, 0);
	m.container[1] = _ac3(mat,1, 0);

	m.container[2] = _ac3(mat,0, 1);
	m.container[3] = _ac3(mat,1, 1);

	return m;
}

Mat2 _ainline Mat4ToMat2(Mat4 mat) {
	Mat2 m;

	m.container[0] = _ac4(mat,0, 0);
	m.container[1] = _ac4(mat,1, 0);

	m.container[2] = _ac4(mat,0, 1);
	m.container[3] = _ac4(mat,1, 1);

	return m;
}

Mat3 _ainline Mat2ToMat3(Mat2 mat) {
	Mat3 m = IdentityMat3();

	_ac3(m,0, 0) = mat.container[0];
	_ac3(m,1, 0) = mat.container[1];

	_ac3(m,0, 1) = mat.container[2];
	_ac3(m,1, 1) = mat.container[3];

	return m;
}

Mat3 _ainline Mat4ToMat3(Mat4 mat) {
	Mat3 m;

	m.container[0] = _ac4(mat,0, 0);
	m.container[1] = _ac4(mat,1, 0);
	m.container[2] = _ac4(mat,2, 0);

	m.container[3] = _ac4(mat,0, 1);
	m.container[4] = _ac4(mat,1, 1);
	m.container[5] = _ac4(mat,2, 1);

	m.container[6] = _ac4(mat,0, 2);
	m.container[7] = _ac4(mat,1, 2);
	m.container[8] = _ac4(mat,2, 2);

	return m;
}

Mat4 _ainline Mat2ToMat4(Mat2 mat) {
	Mat4 m = IdentityMat4();

	_ac4(m,0, 0) = mat.container[0];
	_ac4(m,1, 0) = mat.container[1];
	_ac4(m,0, 1) = mat.container[2];
	_ac4(m,1, 1) = mat.container[3];

	return m;
}

Mat4 _ainline Mat3ToMat4(Mat3 mat) {
	Mat4 m = IdentityMat4();

	_ac4(m,0, 0) = mat.container[0];
	_ac4(m,1, 0) = mat.container[1];
	_ac4(m,2, 0) = mat.container[2];

	_ac4(m,0, 1) = mat.container[3];
	_ac4(m,1, 1) = mat.container[4];
	_ac4(m,2, 1) = mat.container[5];

	_ac4(m,0, 2) = mat.container[6];
	_ac4(m,1, 2) = mat.container[7];
	_ac4(m,2, 2) = mat.container[8];

	return m;
}

Vec4 _ainline Vec3ToVec4(Vec3 vec) {
	Vec4 ret = {vec.x, vec.y, vec.z, 1.0f};
	return ret;
}

Vec3 _ainline Vec4ToVec3(Vec4 vec) {
	Vec3 ret = {vec.x, vec.y, vec.z};

	return ret;
}

Vec2 _ainline Vec4ToVec2(Vec4 vec) {
	Vec2 ret = {vec.x, vec.y};

	return ret;
}

Vec2 _ainline Vec3ToVec2(Vec3 vec) {
	Vec2 ret = {vec.x, vec.y};
	return ret;
}

Ray3 _ainline Line3ToRay3(Line3 line){
	Ray3 ray = {0};

	ray.pos = line.pos;
	ray.dir = line.dir;

	return ray;
}

Line3 _ainline Ray3ToLine3(Ray3 ray){
	Line3 line = {0};

	line.pos = ray.pos;
	line.dir = ray.dir;

	return line;
}

Ray2 _ainline Line2ToRay2(Line2 line){
	Ray2 ray = {0};

	ray.pos = line.pos;
	ray.dir = line.dir;

	return ray;
}

Line2 _ainline Ray2ToLine2(Ray2 ray){
	Line2 line = {0};

	line.pos = ray.pos;
	line.dir = ray.dir;

	return line;
}

Mat4 QuatToMat4(Quat quaternion);
Quat Mat4ToQuat(Mat4 matrix);
Mat4 DualQToMat4(DualQuat d);

Vec3 _ainline Mat4ToTranslationVec(Mat4 matrix) {
	Vec3 ret = {_rc4(matrix,3, 0), _rc4(matrix,3, 1),
		    _rc4(matrix,3, 2)};

	return ret;
}

Vec4 _ainline QuatToVec4(Quat q) {
	Vec4 v;

	v.x = q.w;
	v.y = q.x;
	v.z = q.y;
	v.w = q.z;

	return v;
}

Quat _ainline Vec4ToQuat(Vec4 v) {
	Quat q;

	q.w = v.x;
	q.x = v.y;
	q.y = v.z;
	q.z = v.w;

	return q;
}

Vec4 WorldSpaceToClipSpaceVec4(Vec4 pos, Mat4 viewproj);
Vec4 ClipSpaceToWorldSpaceVec4(Vec4 pos, Mat4 viewproj);

Vec3 _ainline WorldSpaceToClipSpaceVec3(Vec3 pos, Mat4 viewproj) {
	return Vec4ToVec3(WorldSpaceToClipSpaceVec4(Vec3ToVec4(pos), viewproj));
}

Vec3 _ainline ClipSpaceToWorldSpaceVec3(Vec3 pos, Mat4 viewproj) {
	return Vec4ToVec3(ClipSpaceToWorldSpaceVec4(Vec3ToVec4(pos), viewproj));
}

// MARK: MATH OPS (OPS that can be expressed w math notation -- * - / etc
Mat4 AddMat4(Mat4 lhs, Mat4 rhs);
Mat4 SubMat4(Mat4 lhs, Mat4 rhs);
Mat4 MulMat4(Mat4 lhs, Mat4 rhs);
Mat4 MulConstLMat4(f32 lhs, Mat4 rhs);
Mat4 MulConstRMat4(Mat4 lhs, f32 rhs);
Mat4 DivMat4(Mat4 lhs, Mat4 rhs);

Mat3 AddMat3(Mat3 lhs, Mat3 rhs);
Mat3 SubMat3(Mat3 lhs, Mat3 rhs);
Mat3 MulMat3(Mat3 lhs, Mat3 rhs);
Mat3 MulConstLMat3(f32 lhs, Mat3 rhs);
Mat3 MulConstRMat3(Mat3 lhs, f32 rhs);
Mat3 DivMat3(Mat3 lhs, Mat3 rhs);

Mat2 AddMat2(Mat2 lhs, Mat2 rhs);
Mat2 SubMat2(Mat2 lhs, Mat2 rhs);
Mat2 MulMat2(Mat2 lhs, Mat2 rhs);
Mat2 MulConstLMat2(f32 lhs, Mat2 rhs);
Mat2 MulConstRMat2(Mat2 lhs, f32 rhs);
Mat2 DivMat2(Mat2 lhs, Mat2 rhs);

Vec4 AddVec4(Vec4 lhs, Vec4 rhs);
Vec4 SubVec4(Vec4 lhs, Vec4 rhs);
Vec4 MulConstLVec4(f32 lhs, Vec4 rhs);
Vec4 MulConstRVec4(Vec4 lhs, f32 rhs);
Vec4 DivConstRVec4(Vec4 lhs, f32 rhs);

Vec3 AddVec3(Vec3 lhs, Vec3 rhs);
Vec3 SubVec3(Vec3 lhs, Vec3 rhs);
Vec3 MulConstLVec3(f32 lhs, Vec3 rhs);
Vec3 MulConstRVec3(Vec3 lhs, f32 rhs);
Vec3 DivConstRVec3(Vec3 lhs, f32 rhs);

Vec2 AddVec2(Vec2 lhs, Vec2 rhs);
Vec2 SubVec2(Vec2 lhs, Vec2 rhs);
Vec2 MulConstLVec2(f32 lhs, Vec2 rhs);
Vec2 MulConstRVec2(Vec2 lhs, f32 rhs);
Vec2 DivConstRVec2(Vec2 lhs, f32 rhs);

Quat _ainline AddQuat(Quat lhs, Quat rhs) {
	Vec4 l = QuatToVec4(lhs);
	Vec4 r = QuatToVec4(rhs);

	return Vec4ToQuat(AddVec4(l, r));
}

Quat _ainline SubQuat(Quat lhs, Quat rhs) {
	Vec4 l = QuatToVec4(lhs);
	Vec4 r = QuatToVec4(rhs);

	return Vec4ToQuat(SubVec4(l, r));
}

Quat _ainline MulConstLQuat(f32 lhs, Quat rhs) {
	Vec4 r = QuatToVec4(rhs);

	return Vec4ToQuat(MulConstLVec4(lhs, r));
}

Quat _ainline MulConstRQuat(Quat lhs, f32 rhs) { return MulConstLQuat(rhs, lhs); }

Quat _ainline DivConstRQuat(Quat lhs, f32 rhs) {
	Vec4 l = QuatToVec4(lhs);

	return Vec4ToQuat(DivConstRVec4(l, rhs));
}

Quat MulQuat(Quat lhs, Quat rhs);

DualQuat AddDualQ(DualQuat lhs, DualQuat rhs);
DualQuat SubDualQ(DualQuat lhs, DualQuat rhs);
DualQuat MulDualQ(DualQuat lhs, DualQuat rhs);
DualQuat MulConstLDualQ(f32 lhs, DualQuat rhs);
DualQuat MulConstRDualQ(DualQuat lhs, f32 rhs);

// MARK: SPECIAL MATH OPS (can only be expressed in functions)
// TODO: we should also include Rejection functios
Mat4 SchurMat4(Mat4 a, Mat4 b);
Mat4 TransposeMat4(Mat4 matrix);
Mat4 InverseMat4(Mat4 matrix);	// there is a way to compute this faster
// a transform is a mat4 w the last row 0 0 0 1
Mat4 InverseTransform(Mat4 matrix);
Mat4 OuterMat4(Vec4 a, Vec4 b);

Mat3 SchurMat3(Mat3 a, Mat3 b);
Mat3 TransposeMat3(Mat3 matrix);
Mat3 InverseMat3(Mat3 matrix);

Mat2 TransposeMat2(Mat2 matrix);
Mat2 InverseMat2(Mat2 matrix);

f32 MagnitudeVec4(Vec4 vec);
f32 DotVec4(Vec4 vec1, Vec4 vec2);
Vec4 NormalizeVec4(Vec4 vec);
Vec4 SchurVec4(Vec4 a, Vec4 b);
Vec4 InterpolateVec4(Vec4 a, Vec4 b, f32 step);

f32 MagnitudeVec3(Vec3 vec);
f32 DotVec3(Vec3 vec1, Vec3 vec2);
Vec3 NormalizeVec3(Vec3 vec);
Vec3 CrossVec3(Vec3 vec1, Vec3 vec2);
Vec3 VecTripleVec3(Vec3 a,Vec3 b,Vec3 c);//Vec triple product
f32 ScalTripleVec3(Vec3 a,Vec3 b,Vec3 c);
f32 CompVec3(Vec3 a, Vec3 b);

Vec3 _ainline InterpolateVec3(Vec3 a, Vec3 b, f32 step) {
	return Vec4ToVec3(InterpolateVec4(Vec3ToVec4(a), Vec3ToVec4(b), step));
}

Vec3 _ainline SchurVec3(Vec3 a, Vec3 b) {
	Vec4 v_a = Vec3ToVec4(a);
	Vec4 v_b = Vec3ToVec4(b);

	return Vec4ToVec3(SchurVec4(v_a, v_b));
}

Vec3 ProjectOntoVec3(Vec3 a, Vec3 b);
Vec3 RejectVec3(Vec3 a, Vec3 b);
Vec3 ProjectVec3OntoPlane(Vec3 vec, Plane plane);
Vec3 GetVecRotation(Vec3 lookat);
Vec3 QuatRotateVec3(Vec3 v, Quat q);
Vec3 RotateVec3(Vec3 vec, Vec3 rotation);
f32 Cosf(Vec3 vec1, Vec3 vec2);
Vec3 ReflectVec3(Vec3 vec, Vec3 normal);

f32 MagnitudeVec2(Vec2 vec);
f32 DotVec2(Vec2 a, Vec2 b);
Vec2 NormalizeVec2(Vec2 a);
Vec2 SchurVec2(Vec2 a, Vec2 b);
Vec2 RotateVec2(Vec2 vec, f32 rotation);

f32 _ainline MagnitudeQuat(Quat a) { return MagnitudeVec4(QuatToVec4(a)); }
f32 _ainline DotQuat(Quat a, Quat b) { return DotVec4(QuatToVec4(a), QuatToVec4(b)); }
Quat _ainline NormalizeQuat(Quat a) { return Vec4ToQuat(NormalizeVec4(QuatToVec4(a))); }
Quat InverseQuat(Quat q);
Quat NLerpQuat(Quat a, Quat b, f32 step);
Quat SLerpQuat(Quat a, Quat b, f32 step);
Quat ConjugateQuat(Quat quaternion);
Quat InterpolateQuat(Quat a, Quat b, f32 step);

DualQuat NormalizeDualQ(DualQuat d);

b32 IntersectLine3(Line3 a, Line3 b);
b32 IntersectOutLine3(Line3 a, Line3 b, Point3* out_point);
b32 TypedIntersectLine3(Line3 a, Line3 b);

b32 IntersectLine3Plane(Line3 a, Plane b);
b32 IntersectOutLine3Plane(Line3 a, Plane b, Point3* out_point);
b32 TypedIntersectLine3Plane(Line3 a, Plane b);

b32 IntersectLine2(Line2 a, Line2 b);
b32 IntersectOutLine2(Line2 a, Line2 b, Point2* out_point);
b32 TypedIntersectLine2(Line2 a, Line2 b);

b32 IntersectRay3(Ray3 a, Ray3 b);
b32 IntersectOutRay3(Ray3 a, Ray3 b, Point3* out_point);
b32 TypedIntersectRay3(Ray3 a, Ray3 b);

b32 IntersectRay3Plane(Ray3 a, Plane b);
b32 IntersectOutRay3Plane(Ray3 a, Plane b, Point3* out_point);
b32 TypedIntersectRay3Plane(Ray3 a, Plane b);

b32 IntersectClosestOutLine3Sphere(Line3 line, Sphere sphere, Point3* point);
b32 IntersectClosestOutRay3Sphere(Ray3 ray, Sphere sphere, Point3* point);

b32 IntersectRay2(Ray2 a, Ray2 b);
b32 IntersectOutRay2(Ray2 a, Ray2 b, Point2* out_point);
b32 TypedIntersectRay2(Ray2 a, Ray2 b);

void MinkowskiAddition(Point3* a, ptrsize a_count, Point3* b, ptrsize b_count, Point3** ret);
void MinkowskiDifference(Point3* a, ptrsize a_count, Point3* b, ptrsize b_count, Point3** ret);

f32 _ainline Interpolate(f32 a, f32 b, f32 step) { return (a + (step * (b - a))); }
f32 AngleQuadrant(f32 x, f32 y);

// MARK: constructors

Plane _ainline ConstructPlaneD(Vec3 norm,f32 d){
	Plane plane = {norm,d};
	return plane;
}

Plane _ainline ConstructPlanePos(Vec3 norm,Vec3 pos){
	Vec3 n = NormalizeVec3(norm);
	f32 d = DotVec3(pos,n);

	return ConstructPlaneD(n,d);
}

Mat4 ViewMat4(Vec3 position, Vec3 lookpoint, Vec3 updir);
Mat4 ProjectionMat4(f32 fov, f32 aspectration, f32 nearz, f32 farz);

Mat4 WorldMat4M(Mat4 position, Mat4 rotation, Mat4 scale);
Mat4 WorldMat4V(Vec3 position, Vec3 rotation, Vec3 scale);
Mat4 WorldMat4Q(Vec3 position, Quat rotation, Vec3 scale);

Mat4 _ainline PositionMat4(Vec3 position) {
	Mat4 matrix = IdentityMat4();

	_rc4(matrix,3, 0) = position.x;
	_rc4(matrix,3, 1) = position.y;
	_rc4(matrix,3, 2) = position.z;

	return matrix;
}

// TODO: not tested
Mat3 _ainline RotationMat3(Vec3 rotation) {
	f32 cosv = cosf(rotation.x);
	f32 sinv = sinf(rotation.x);

	Mat3 rotationx_matrix3 = IdentityMat3();

	_rc3(rotationx_matrix3,1, 1) = cosv;
	_rc3(rotationx_matrix3,2, 1) = -sinv;
	_rc3(rotationx_matrix3,1, 2) = sinv;
	_rc3(rotationx_matrix3,2, 2) = cosv;

	cosv = cosf(rotation.y);
	sinv = sinf(rotation.y);

	Mat3 rotationy_matrix3 = IdentityMat3();

	_rc3(rotationy_matrix3,0, 0) = cosv;
	_rc3(rotationy_matrix3,2, 0) = sinv;
	_rc3(rotationy_matrix3,0, 2) = -sinv;
	_rc3(rotationy_matrix3,2, 2) = cosv;

	cosv = cosf(rotation.z);
	sinv = sinf(rotation.z);

	Mat3 rotationz_matrix3 = IdentityMat3();

	_rc3(rotationz_matrix3,0, 0) = cosv;
	_rc3(rotationz_matrix3,1, 0) = -sinv;
	_rc3(rotationz_matrix3,0, 1) = sinv;
	_rc3(rotationz_matrix3,1, 1) = cosv;

	return MulMat3(MulMat3(rotationz_matrix3, rotationy_matrix3), rotationx_matrix3);
}

Mat4 _ainline ScaleMat4(Vec3 scale) {
	Mat4 matrix = {{scale.x, 0, 0, 0, 0, scale.y, 0, 0, 0, 0, scale.z, 0, 0, 0, 0, 1}};

	return matrix;
}

//NOTE: Given vectors a and b, this function constructs a matrix A 
//from a such that A * b computes CrossVec3(a,b)
Mat3 ConstructCrossMat3(Vec3 vec);

//NOTE: Given vectors a and b, this function constructs a matrix B 
//from b such that B * a computes ProjectOntoVec3(a,b)
Mat3 ConstructProjectOntoMat3(Vec3 vec);

//NOTE: Given vectors a and b, this function constructs a matrix B 
//from b such that B * a computes RejectVec3(a,b);
Mat3 ConstructRejectMat3(Vec3 vec);

Quat ConstructQuat(Vec3 vector, f32 angle);

DualQuat ConstructDualQ(Quat rotation, Vec3 translation);
DualQuat ConstructDualQM(Mat4 transform);

Color4 _ainline ConstructColor4(f32 R,f32 G,f32 B,f32 A){
	Color4 color = {R,G,B,A};
	return color;
}

Color3 _ainline ConstructColor3(f32 R,f32 G,f32 B){
	Color3 color = {R,G,B};
	return color;
}

Vec3 GetSphereNormalVec3(Sphere sphere, Point3 point_on_sphere);

// MARK: deconstructors

void DeconstructQuat(Quat quaternion, Vec3* vector, f32* angle);

// MARK: UTIL OPS
void PrintMat4(Mat4 matrix);
void PrintMat3(Mat3 matrix);

void PrintVec4(Vec4 vec);
void PrintVec3(Vec3 vec);
void PrintVec2(Vec2 vec);

void PrintQuat(Quat quat);

#ifdef __cplusplus
}

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

Vec4 operator+(Vec4 lhs, Vec4 rhs);
Vec4 operator-(Vec4 lhs, Vec4 rhs);
Vec4 operator*(f32 lhs, Vec4 rhs);
Vec4 operator*(Vec4 lhs, f32 rhs);
Vec4 operator/(Vec4 lhs, f32 rhs);

Vec3 operator+(Vec3 lhs, Vec3 rhs);
Vec3 operator-(Vec3 lhs, Vec3 rhs);
Vec3 operator*(f32 lhs, Vec3 rhs);
Vec3 operator*(Vec3 lhs, f32 rhs);
Vec3 operator/(Vec3 lhs, f32 rhs);

Vec2 operator+(Vec2 lhs, Vec2 rhs);
Vec2 operator-(Vec2 lhs, Vec2 rhs);
Vec2 operator*(f32 lhs, Vec2 rhs);
Vec2 operator*(Vec2 lhs, f32 rhs);
Vec2 operator/(Vec2 lhs, f32 rhs);

Quat _ainline operator+(Quat lhs, Quat rhs) { return AddQuat(lhs, rhs); }

Quat _ainline operator-(Quat lhs, Quat rhs) { return SubQuat(lhs, rhs); }

Quat _ainline operator*(f32 lhs, Quat rhs) { return MulConstLQuat(lhs, rhs); }

Quat _ainline operator*(Quat lhs, f32 rhs) { return MulConstRQuat(lhs, rhs); }

Quat _ainline operator/(Quat lhs, f32 rhs) { return DivConstRQuat(lhs, rhs); }

Quat operator*(Quat lhs, Quat rhs);

DualQuat operator+(DualQuat lhs, DualQuat rhs);
DualQuat operator-(DualQuat lhs, DualQuat rhs);
DualQuat operator*(DualQuat lhs, DualQuat rhs);
DualQuat operator*(f32 lhs, DualQuat rhs);
DualQuat operator*(DualQuat lhs, f32 rhs);

#endif

