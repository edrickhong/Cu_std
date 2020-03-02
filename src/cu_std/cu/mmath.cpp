/*
TODO:
move glm compare to tests
make quaternions xyzw
have automatic conversions between f32* to our types for better integration with
external code bases
*/

// MARK: Internal
#ifdef DEBUG

#if (_test__matrices)

#ifdef VERBOSE
#pragma message("MATRIX TESTING ENABLED")
#endif

#define GLM_FORCE_DEPTH_ZERO_TO_ONE 1
#define GLM_FORCE_RIGHT_HANDED 1
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"

void InternalCmpMat(f32* f1, f32* f2) {
#if MATRIX_ROW_MAJOR

	auto k = (Mat4*)f1;

	*k = TransposeMat4(*k);

#endif

	for (u32 i = 0; i < 16; i++) {
		auto a = f1[i];
		auto b = f2[i];

		if (a == 0.0f && b == 0.0f) {
			f1[i] = 0;
			f2[i] = 0;
		}

		if (f1[i] - f2[i] > 0.1f) {
			printf("--------------\n");

#if MATRIX_ROW_MAJOR

			PrintMat4(TransposeMat4(*((Mat4*)f1)));
			PrintMat4(TransposeMat4(*((Mat4*)f2)));

#else

			PrintMat4(*((Mat4*)f1));
			PrintMat4(*((Mat4*)f2));

#endif

			_kill("", 1);
		}
	}
}

#endif
#endif

void _ainline GetMinorMat(f32* in__matrix, u32 n, u32 k_x, u32 k_y,
			  f32* out__matrix) {
	u32 index = 0;

	for (u32 y = 0; y < n; y++) {
		for (u32 x = 0; x < n; x++) {
			if (y != k_y && x != k_x) {
				out__matrix[index] = in__matrix[(y * n) + x];
				index++;
			}
		}
	}
}

Mat4 _ainline ViewMatRHS(Vec3 position, Vec3 lookpoint, Vec3 updir) {
	Vec3 forward = NormalizeVec3(lookpoint - position);

	Vec3 side = NormalizeVec3(CrossVec3(forward, updir));

	Vec3 up = CrossVec3(side, forward);

	f32 a = -1.0f * DotVec3(side, position),
	    b = -1.0f * DotVec3(up, position), c = DotVec3(forward, position);

	Mat4 matrix = IdentityMat4();

	matrix.container[_rc4(0, 0)] = side.x;
	matrix.container[_rc4(1, 0)] = side.y;
	matrix.container[_rc4(2, 0)] = side.z;
	matrix.container[_rc4(3, 0)] = a;

	matrix.container[_rc4(0, 1)] = up.x;
	matrix.container[_rc4(1, 1)] = up.y;
	matrix.container[_rc4(2, 1)] = up.z;
	matrix.container[_rc4(3, 1)] = b;

	matrix.container[_rc4(0, 2)] = -forward.x;
	matrix.container[_rc4(1, 2)] = -forward.y;
	matrix.container[_rc4(2, 2)] = -forward.z;
	matrix.container[_rc4(3, 2)] = c;

#ifdef DEBUG

#if _test__matrices

	auto t__mat =
	    glm::lookAt(glm::vec3(position.x, position.y, position.z),
			glm::vec3(lookpoint.x, lookpoint.y, lookpoint.z),
			glm::vec3(updir.x, updir.y, updir.z));

	auto ref__matrix = matrix;

	auto f1 = (f32*)&ref__matrix;
	auto f2 = (f32*)&t__mat;

	InternalCmpMat(f1, f2);

#endif

#endif

	return matrix;
}

Vec4 InternalCompDiv(Vec4 a, Vec4 b) {
	a.simd = _mm_div_ps(a.simd, b.simd);
	return a;
}

f32 inline GenericGetDeterminant(f32* in__matrix, u32 n) {
	_kill("we do not support this case\n", n > 4);

	if (n == 2) {
		f32 a = in__matrix[0];
		f32 b = in__matrix[1];
		f32 c = in__matrix[2];
		f32 d = in__matrix[3];

		return (a * d) - (b * c);
	}

	f32 res = 0;

	for (u32 i = 0; i < n; i++) {
		auto entry = in__matrix[i];

		if (i & 1) {
			entry *= -1.0f;
		}

		f32 minor__mat[16] = {};

		GetMinorMat(in__matrix, n, i, 0, &minor__mat[0]);

		auto det = GenericGetDeterminant(&minor__mat[0], n - 1);

		res += det * entry;
	}

	return res;
}

Quat _ainline Neighbourhood(Quat a, Quat b) {
	f32 dot = DotQuat(a, b);

	// neighbourhood operator.
	if (dot < 0.0f) {
		b = b * -1.0f;
	}

	return b;
}

// MARK: conversions

extern "C" {
Mat4 QuatToMat4(Quat quaternion) {
	Quat squared;

	squared.simd = _mm_mul_ps(quaternion.simd, quaternion.simd);

	f32 a = 1.0f - (2.0f * (squared.y + squared.z));
	f32 b =
	    ((quaternion.x * quaternion.y) - (quaternion.w * quaternion.z)) *
	    2.0f;
	f32 c =
	    ((quaternion.x * quaternion.z) + (quaternion.w * quaternion.y)) *
	    2.0f;

	f32 d =
	    ((quaternion.x * quaternion.y) + (quaternion.w * quaternion.z)) *
	    2.0f;
	f32 e = 1.0f - (2.0f * (squared.x + squared.z));
	f32 f =
	    ((quaternion.y * quaternion.z) - (quaternion.w * quaternion.x)) *
	    2.0f;

	f32 g =
	    ((quaternion.x * quaternion.z) - (quaternion.w * quaternion.y)) *
	    2.0f;
	f32 h =
	    ((quaternion.y * quaternion.z) + (quaternion.w * quaternion.x)) *
	    2.0f;
	f32 i = 1.0f - (2.0f * (squared.x + squared.y));

	Mat4 matrix = {};

	matrix.container[_rc4(0, 0)] = a;
	matrix.container[_rc4(1, 0)] = b;
	matrix.container[_rc4(2, 0)] = c;

	matrix.container[_rc4(0, 1)] = d;
	matrix.container[_rc4(1, 1)] = e;
	matrix.container[_rc4(2, 1)] = f;

	matrix.container[_rc4(0, 2)] = g;
	matrix.container[_rc4(1, 2)] = h;
	matrix.container[_rc4(2, 2)] = i;

	matrix.container[_rc4(3, 3)] = 1.0f;

	return matrix;
}
Quat Mat4ToQuat(Mat4 matrix) {
	Quat q;

	f32 trs = matrix.container[_rc4(0, 0)] + matrix.container[_rc4(1, 1)] +
		  matrix.container[_rc4(2, 2)];

	if (trs > 0.0f) {
		f32 s = sqrtf(trs + 1.0f) * 2.0f;
		q.w = 0.25f * s;
		q.x = (matrix.container[_rc4(1, 2)] -
		       matrix.container[_rc4(2, 1)]) /
		      s;
		q.y = (matrix.container[_rc4(2, 0)] -
		       matrix.container[_rc4(0, 2)]) /
		      s;
		q.z = (matrix.container[_rc4(0, 1)] -
		       matrix.container[_rc4(1, 0)]) /
		      s;
	}

	else if ((matrix.container[_rc4(0, 0)] >
		  matrix.container[_rc4(1, 1)]) &&
		 (matrix.container[_rc4(0, 0)] >
		  matrix.container[_rc4(2, 2)])) {
		f32 s = sqrtf(1.0f + matrix.container[_rc4(0, 0)] -
			      matrix.container[_rc4(1, 1)] -
			      matrix.container[_rc4(2, 2)]) *
			2.0f;
		q.w = (matrix.container[_rc4(1, 2)] -
		       matrix.container[_rc4(2, 1)]) /
		      s;
		q.x = 0.25f * s;
		q.y = (matrix.container[_rc4(1, 0)] +
		       matrix.container[_rc4(1, 0)]) /
		      s;
		q.z = (matrix.container[_rc4(2, 0)] +
		       matrix.container[_rc4(0, 2)]) /
		      s;
	}

	else if (matrix.container[_rc4(1, 1)] > matrix.container[_rc4(2, 2)]) {
		f32 s = sqrtf(1.0f + matrix.container[_rc4(1, 1)] -
			      matrix.container[_rc4(0, 0)] -
			      matrix.container[_rc4(2, 2)]) *
			2.0f;

		q.w = (matrix.container[_rc4(2, 0)] -
		       matrix.container[_rc4(0, 2)]) /
		      s;
		q.x = (matrix.container[_rc4(1, 0)] +
		       matrix.container[_rc4(1, 0)]) /
		      s;
		q.y = 0.25f * s;
		q.z = (matrix.container[_rc4(2, 1)] +
		       matrix.container[_rc4(1, 2)]) /
		      s;
	}

	else {
		f32 s = sqrtf(1.0f + matrix.container[_rc4(2, 2)] -
			      matrix.container[_rc4(0, 0)] -
			      matrix.container[_rc4(1, 1)]) *
			2.0f;

		q.w = (matrix.container[_rc4(0, 1)] -
		       matrix.container[_rc4(1, 0)]) /
		      s;
		q.x = (matrix.container[_rc4(2, 0)] +
		       matrix.container[_rc4(0, 2)]) /
		      s;
		q.y = (matrix.container[_rc4(2, 1)] +
		       matrix.container[_rc4(1, 2)]) /
		      s;
		q.z = 0.25f * s;
	}

	return NormalizeQuat(q);
}

Mat4 DualQToMat4(DualQuat d) {
	d = NormalizeDualQ(d);

	Mat4 matrix = QuatToMat4(d.q1);
	Quat t = d.q2 * 2.0f;
	t = t * ConjugateQuat(d.q1);

	matrix.container[_rc4(3, 0)] = t.x;
	matrix.container[_rc4(3, 1)] = t.y;
	matrix.container[_rc4(3, 2)] = t.z;
	matrix.container[_rc4(3, 3)] = 1.0f;

	return matrix;
}

Vec4 WorldSpaceToClipSpaceVec4(Vec4 pos, Mat4 viewproj) {
#if !MATRIX_ROW_MAJOR

	viewproj = TransposeMat4(viewproj);

#endif

	auto vert = pos.simd;

	auto a = _mm_mul_ps(viewproj.simd[0], vert);
	auto b = _mm_mul_ps(viewproj.simd[1], vert);
	auto c = _mm_mul_ps(viewproj.simd[2], vert);
	auto d = _mm_mul_ps(viewproj.simd[3], vert);

	f32 x = ((f32*)&a)[0] + ((f32*)&a)[1] + ((f32*)&a)[2] + ((f32*)&a)[3];

	f32 y = ((f32*)&b)[0] + ((f32*)&b)[1] + ((f32*)&b)[2] + ((f32*)&b)[3];

	f32 z = ((f32*)&c)[0] + ((f32*)&c)[1] + ((f32*)&c)[2] + ((f32*)&c)[3];

	f32 w = ((f32*)&d)[0] + ((f32*)&d)[1] + ((f32*)&d)[2] + ((f32*)&d)[3];

	Vec4 ret = {x, y, z, w};

	return ret / ret.w;
}

Vec4 ClipSpaceToWorldSpaceVec4(Vec4 pos, Mat4 viewproj) {
	auto inv_viewproj = InverseMat4(viewproj);

#if !MATRIX_ROW_MAJOR

	inv_viewproj = TransposeMat4(inv_viewproj);

#endif

	auto vert = pos.simd;

	auto a = _mm_mul_ps(inv_viewproj.simd[0], vert);
	auto b = _mm_mul_ps(inv_viewproj.simd[1], vert);
	auto c = _mm_mul_ps(inv_viewproj.simd[2], vert);
	auto d = _mm_mul_ps(inv_viewproj.simd[3], vert);

	f32 x = ((f32*)&a)[0] + ((f32*)&a)[1] + ((f32*)&a)[2] + ((f32*)&a)[3];

	f32 y = ((f32*)&b)[0] + ((f32*)&b)[1] + ((f32*)&b)[2] + ((f32*)&b)[3];

	f32 z = ((f32*)&c)[0] + ((f32*)&c)[1] + ((f32*)&c)[2] + ((f32*)&c)[3];

	f32 w = ((f32*)&d)[0] + ((f32*)&d)[1] + ((f32*)&d)[2] + ((f32*)&d)[3];

	Vec4 ret = {x, y, z, w};

	return ret / ret.w;
}

// MARK: MATH OPS

Mat4 AddMat4(Mat4 lhs, Mat4 rhs) {
	Mat4 matrix;

	_mm_store_ps(&matrix.container[_ac4(0, 0)],
		     _mm_add_ps(lhs.simd[0], rhs.simd[0]));

	_mm_store_ps(&matrix.container[_ac4(0, 1)],
		     _mm_add_ps(lhs.simd[1], rhs.simd[1]));

	_mm_store_ps(&matrix.container[_ac4(0, 2)],
		     _mm_add_ps(lhs.simd[2], rhs.simd[2]));

	_mm_store_ps(&matrix.container[_ac4(0, 3)],
		     _mm_add_ps(lhs.simd[3], rhs.simd[3]));

	return matrix;
}

Mat4 SubMat4(Mat4 lhs, Mat4 rhs) {
	Mat4 matrix;

	_mm_store_ps(&matrix.container[_ac4(0, 0)],
		     _mm_sub_ps(lhs.simd[0], rhs.simd[0]));

	_mm_store_ps(&matrix.container[_ac4(0, 1)],
		     _mm_sub_ps(lhs.simd[1], rhs.simd[1]));

	_mm_store_ps(&matrix.container[_ac4(0, 2)],
		     _mm_sub_ps(lhs.simd[2], rhs.simd[2]));

	_mm_store_ps(&matrix.container[_ac4(0, 3)],
		     _mm_sub_ps(lhs.simd[3], rhs.simd[3]));

	return matrix;
}

Mat4 MulMat4(Mat4 lhs, Mat4 rhs) {
	Mat4 matrix;

#if MATRIX_ROW_MAJOR

	rhs = TransposeMat4(rhs);

#else

	lhs = TransposeMat4(lhs);

#endif

	for (u32 i = 0; i < 4; i++) {
#if MATRIX_ROW_MAJOR

		__m128 res1 = _mm_mul_ps(lhs.simd[i], rhs.simd[0]);
		__m128 res2 = _mm_mul_ps(lhs.simd[i], rhs.simd[1]);
		__m128 res3 = _mm_mul_ps(lhs.simd[i], rhs.simd[2]);
		__m128 res4 = _mm_mul_ps(lhs.simd[i], rhs.simd[3]);

#else

		__m128 res1 = _mm_mul_ps(rhs.simd[i], lhs.simd[0]);
		__m128 res2 = _mm_mul_ps(rhs.simd[i], lhs.simd[1]);
		__m128 res3 = _mm_mul_ps(rhs.simd[i], lhs.simd[2]);
		__m128 res4 = _mm_mul_ps(rhs.simd[i], lhs.simd[3]);

#endif

		f32* _restrict r1 = (f32*)&res1;
		f32* _restrict r2 = (f32*)&res2;
		f32* _restrict r3 = (f32*)&res3;
		f32* _restrict r4 = (f32*)&res4;

		matrix.container[_ac4(0, i)] = r1[0] + r1[1] + r1[2] + r1[3];
		matrix.container[_ac4(1, i)] = r2[0] + r2[1] + r2[2] + r2[3];
		matrix.container[_ac4(2, i)] = r3[0] + r3[1] + r3[2] + r3[3];
		matrix.container[_ac4(3, i)] = r4[0] + r4[1] + r4[2] + r4[3];
	}

	// FIXME: test isn't running when not row major
#if !MATRIX_ROW_MAJOR
	{
		lhs = TransposeMat4(lhs);

		glm::mat4 l;
		glm::mat4 r;

		memcpy(&l, &lhs, sizeof(lhs));
		memcpy(&r, &rhs, sizeof(rhs));

		auto res = l * r;

		auto ref__matrix = matrix;

		InternalCmpMat((f32*)&ref__matrix, (f32*)&res);
	}

#endif

	return matrix;
}

Mat4 MulConstLMat4(f32 lhs, Mat4 rhs) {
	Mat4 matrix;
	__m128 k = _mm_set1_ps(lhs);

	__m128 res = _mm_mul_ps(rhs.simd[0], k);

	_mm_store_ps(&matrix.container[_ac4(0, 0)], res);

	res = _mm_mul_ps(rhs.simd[1], k);

	_mm_store_ps(&matrix.container[_ac4(0, 1)], res);

	res = _mm_mul_ps(rhs.simd[2], k);

	_mm_store_ps(&matrix.container[_ac4(0, 2)], res);

	res = _mm_mul_ps(rhs.simd[3], k);

	_mm_store_ps(&matrix.container[_ac4(0, 3)], res);

	return matrix;
}

Mat4 MulConstRMat4(Mat4 lhs, f32 rhs) { return MulConstLMat4(rhs, lhs); }

Mat4 DivMat4(Mat4 lhs, Mat4 rhs) { return lhs * InverseMat4(rhs); }

Mat3 AddMat3(Mat3 lhs, Mat3 rhs) {
	lhs.simd[0] = _mm_add_ps(lhs.simd[0], rhs.simd[0]);
	lhs.simd[1] = _mm_add_ps(lhs.simd[1], rhs.simd[1]);
	lhs.k = lhs.k + rhs.k;

	return lhs;
}

Mat3 SubMat3(Mat3 lhs, Mat3 rhs) {
	lhs.simd[0] = _mm_sub_ps(lhs.simd[0], rhs.simd[0]);
	lhs.simd[1] = _mm_sub_ps(lhs.simd[1], rhs.simd[1]);
	lhs.k = lhs.k - rhs.k;

	return lhs;
}

Mat3 MulMat3(Mat3 lhs, Mat3 rhs) {
	Mat3 matrix = {};

#if MATRIX_ROW_MAJOR

	rhs = TransposeMat3(rhs);

#else

	lhs = TransposeMat3(lhs);

#endif

	__m128 a = rhs.simd[0];

	__m128 b =
	    _mm_shuffle_ps(rhs.simd[1], rhs.simd[1], _MM_SHUFFLE(2, 1, 0, 3));
	((f32*)&b)[0] = ((f32*)&a)[3];

	__m128 c =
	    _mm_shuffle_ps(rhs.simd[1], rhs.simd[1], _MM_SHUFFLE(0, 0, 3, 2));
	((f32*)&c)[2] = rhs.k;
	((f32*)&c)[3] = ((f32*)&a)[0];

	__m128 d =
	    _mm_shuffle_ps(rhs.simd[0], rhs.simd[0], _MM_SHUFFLE(0, 3, 2, 1));
	((f32*)&d)[3] = ((f32*)&rhs.simd[1])[0];

	__m128 e = rhs.simd[1];

	__m128 f =
	    _mm_shuffle_ps(rhs.simd[1], rhs.simd[0], _MM_SHUFFLE(1, 0, 0, 3));
	((f32*)&f)[1] = rhs.k;

	__m128 g = {((f32*)&rhs.simd[0])[2], ((f32*)&rhs.simd[1])[1], rhs.k};

	auto h = _mm_mul_ps(lhs.simd[0], a);
	auto i = _mm_mul_ps(lhs.simd[0], b);
	auto j = _mm_mul_ps(lhs.simd[0], c);

	auto k = _mm_mul_ps(lhs.simd[1], d);
	auto l = _mm_mul_ps(lhs.simd[1], e);
	auto m = _mm_mul_ps(lhs.simd[1], f);

	auto n = _mm_set1_ps(lhs.k);

	auto o = _mm_mul_ps(n, g);

	// NOTE: god I hope the compiler reorders this

	matrix.container[_rc3(0, 0)] =
	    ((f32*)&h)[0] + ((f32*)&h)[1] + ((f32*)&h)[2];
	matrix.container[_rc3(1, 0)] =
	    ((f32*)&i)[0] + ((f32*)&i)[1] + ((f32*)&i)[2];
	matrix.container[_rc3(2, 0)] =
	    ((f32*)&j)[0] + ((f32*)&j)[1] + ((f32*)&j)[2];

	matrix.container[_rc3(0, 1)] =
	    ((f32*)&j)[3] + ((f32*)&k)[0] + ((f32*)&k)[1];
	matrix.container[_rc3(1, 1)] =
	    ((f32*)&h)[3] + ((f32*)&l)[0] + ((f32*)&l)[1];
	matrix.container[_rc3(2, 1)] =
	    ((f32*)&i)[3] + ((f32*)&m)[0] + ((f32*)&m)[1];

	matrix.container[_rc3(0, 2)] =
	    ((f32*)&m)[2] + ((f32*)&m)[3] + ((f32*)&o)[0];
	matrix.container[_rc3(1, 2)] =
	    ((f32*)&k)[2] + ((f32*)&k)[3] + ((f32*)&o)[1];
	matrix.container[_rc3(2, 2)] =
	    ((f32*)&l)[2] + ((f32*)&l)[3] + ((f32*)&o)[2];

	return matrix;
}

Mat3 MulConstLMat3(f32 lhs, Mat3 rhs) {
	__m128 k = _mm_set1_ps(lhs);

	rhs.simd[0] = _mm_mul_ps(rhs.simd[0], k);
	rhs.simd[1] = _mm_mul_ps(rhs.simd[1], k);
	rhs.k = rhs.k * lhs;

	return rhs;
}

Mat3 MulConstRMat3(Mat3 lhs, f32 rhs) { return MulConstLMat3(rhs, lhs); }

Mat3 DivMat3(Mat3 lhs, Mat3 rhs) { return lhs * InverseMat3(rhs); }

Mat2 AddMat2(Mat2 lhs, Mat2 rhs) {
	Mat2 matrix;

	matrix.simd = _mm_add_ps(lhs.simd, rhs.simd);

	return matrix;
}

Mat2 SubMat2(Mat2 lhs, Mat2 rhs) {
	Mat2 matrix;

	matrix.simd = _mm_sub_ps(lhs.simd, rhs.simd);

	return matrix;
}

Mat2 MulMat2(Mat2 lhs, Mat2 rhs) {
	Mat2 matrix = {};

#if MATRIX_ROW_MAJOR

	rhs = TransposeMat2(rhs);

#else

	lhs = TransposeMat2(lhs);

#endif

	__m128 a = _mm_mul_ps(lhs.simd, rhs.simd);

	rhs.simd = _mm_shuffle_ps(rhs.simd, rhs.simd, _MM_SHUFFLE(1, 0, 3, 2));

	__m128 b = _mm_mul_ps(lhs.simd, rhs.simd);

	__m128 c = _mm_shuffle_ps(a, b, _MM_SHUFFLE(1, 3, 3, 0));
	c = _mm_shuffle_ps(c, c, _MM_SHUFFLE(1, 2, 3, 0));

	__m128 d = _mm_shuffle_ps(a, b, _MM_SHUFFLE(0, 2, 2, 1));
	d = _mm_shuffle_ps(d, d, _MM_SHUFFLE(1, 2, 3, 0));

	matrix.simd = _mm_add_ps(c, d);

	return matrix;
}

Mat2 MulConstLMat2(f32 lhs, Mat2 rhs) {
	Mat2 matrix;
	__m128 k = _mm_set1_ps(lhs);

	matrix.simd = _mm_mul_ps(rhs.simd, k);

	return matrix;
}

Mat2 MulConstRMat2(Mat2 lhs, f32 rhs) { return (rhs, lhs); }

Mat2 DivMat2(Mat2 lhs, Mat2 rhs) { return lhs * InverseMat2(rhs); }

Vec4 AddVec4(Vec4 lhs, Vec4 rhs) {
	Vec4 vec;

	__m128 res = _mm_add_ps(lhs.simd, rhs.simd);

	_mm_storeu_ps(&vec.x, res);

	return vec;
}

Vec4 SubVec4(Vec4 lhs, Vec4 rhs) {
	Vec4 vec;

	__m128 res = _mm_sub_ps(lhs.simd, rhs.simd);

	_mm_storeu_ps(&vec.x, res);

	return vec;
}

Vec4 MulConstLVec4(f32 lhs, Vec4 rhs) {
	Vec4 vec;

	__m128 k = _mm_set1_ps(lhs);

	__m128 res = _mm_mul_ps(rhs.simd, k);

	_mm_storeu_ps(&vec.x, res);

	return vec;
}

Vec4 MulConstRVec4(Vec4 lhs, f32 rhs) { return MulConstLVec4(rhs, lhs); }

Vec4 DivConstRVec4(Vec4 lhs, f32 rhs) {
	Vec4 vec;
	__m128 k = _mm_set1_ps(rhs);

	__m128 res = _mm_div_ps(lhs.simd, k);

	_mm_storeu_ps(&vec.x, res);

	return vec;
}

// TODO: we should use simd still. just throw away the last value
Vec3 AddVec3(Vec3 lhs, Vec3 rhs) {
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	lhs.z += rhs.z;

	return lhs;
}

Vec3 SubVec3(Vec3 lhs, Vec3 rhs) {
	lhs.x -= rhs.x;
	lhs.y -= rhs.y;
	lhs.z -= rhs.z;

	return lhs;
}

Vec3 MulConstLVec3(f32 lhs, Vec3 rhs) {
	rhs.x *= lhs;
	rhs.y *= lhs;
	rhs.z *= lhs;

	return rhs;
}

Vec3 MulConstRVec3(Vec3 lhs, f32 rhs) { return MulConstLVec3(rhs, lhs); }

Vec3 DivConstRVec3(Vec3 lhs, f32 rhs) {
	lhs.x /= rhs;
	lhs.y /= rhs;
	lhs.z /= rhs;

	return lhs;
}

// TODO: we should use simd still. just throw away the last value
Vec2 AddVec2(Vec2 lhs, Vec2 rhs) {
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	return lhs;
}

Vec2 SubVec2(Vec2 lhs, Vec2 rhs) {
	lhs.x -= rhs.x;
	lhs.y -= rhs.y;
	return lhs;
}

Vec2 MulConstLVec2(f32 lhs, Vec2 rhs) {
	rhs.x *= lhs;
	rhs.y *= lhs;
	return rhs;
}

Vec2 MulConstRVec2(Vec2 lhs, f32 rhs) { return MulConstLVec2(rhs, lhs); }

Vec2 DivConstRVec2(Vec2 lhs, f32 rhs) {
	lhs.x /= rhs;
	lhs.y /= rhs;
	return lhs;
}

Quat MulQuat(Quat lhs, Quat rhs) {
	/*
       x = w1x2 + x1w2 + y1z2 - z1y2
       y = w1y2 + y1w2 + z1x2 - x1z2
       z = w1z2 + z1w2 + x1y2 - y1x2
       w=  w1w2 - x1x2 - y1y2 - z1z2
	*/

	Quat quaternion;

	quaternion.x = (lhs.w * rhs.x) + (lhs.x * rhs.w) + (lhs.y * rhs.z) -
		       (lhs.z * rhs.y);
	quaternion.y = (lhs.w * rhs.y) + (lhs.y * rhs.w) + (lhs.z * rhs.x) -
		       (lhs.x * rhs.z);
	quaternion.z = (lhs.w * rhs.z) + (lhs.z * rhs.w) + (lhs.x * rhs.y) -
		       (lhs.y * rhs.x);
	quaternion.w = (lhs.w * rhs.w) - (lhs.x * rhs.x) - (lhs.y * rhs.y) -
		       (lhs.z * rhs.z);

	return quaternion;
}

DualQuat AddDualQ(DualQuat lhs, DualQuat rhs) {
	lhs.q1 = lhs.q1 + rhs.q1;
	lhs.q2 = lhs.q2 + rhs.q2;
	return lhs;
}

DualQuat SubDualQ(DualQuat lhs, DualQuat rhs) {
	lhs.q1 = lhs.q1 - rhs.q1;
	lhs.q2 = lhs.q2 - rhs.q2;
	return lhs;
}

DualQuat MulDualQ(DualQuat lhs, DualQuat rhs) {
	DualQuat d;
	d.q1 = lhs.q1 * rhs.q1;
	d.q2 = (lhs.q2 * rhs.q1) + (lhs.q1 * rhs.q2);
	return d;
}

DualQuat MulConstLDualQ(f32 lhs, DualQuat rhs) {
	rhs.q1 = rhs.q1 * lhs;
	rhs.q2 = rhs.q2 * lhs;
	return rhs;
}

DualQuat MulConstRDualQ(DualQuat lhs, f32 rhs) {
	return MulConstLDualQ(rhs, lhs);
}

// MARK: SPECIAL MATH OPS

Mat4 SchurMat4(Mat4 a, Mat4 b) {
	Mat4 matrix;

	for (u32 i = 0; i < 4; i++) {
		matrix.simd[i] = _mm_mul_ps(a.simd[i], b.simd[i]);
	}

	return matrix;
}

Mat4 TransposeMat4(Mat4 matrix) {
	Mat4 store__matrix;

	__m128 tmp1 = {}, row0 = {}, row1 = {}, row2 = {}, row3 = {};

	f32* src = matrix.container;

	tmp1 =
	    _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(src)), (__m64*)(src + 4));

	row1 = _mm_loadh_pi(_mm_loadl_pi(row1, (__m64*)(src + 8)),
			    (__m64*)(src + 12));

	row0 = _mm_shuffle_ps(tmp1, row1, 0x88);
	row1 = _mm_shuffle_ps(row1, tmp1, 0xDD);
	tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(src + 2)),
			    (__m64*)(src + 6));
	row3 = _mm_loadh_pi(_mm_loadl_pi(row3, (__m64*)(src + 10)),
			    (__m64*)(src + 14));

	row2 = _mm_shuffle_ps(tmp1, row3, 0x88);
	row3 = _mm_shuffle_ps(row3, tmp1, 0xDD);

	_mm_storeu_ps(&store__matrix.container[_ac4(0, 0)], row0);

	// this is swapped
	row1 = _mm_shuffle_ps(row1, row1, _MM_SHUFFLE(1, 0, 3, 2));
	_mm_storeu_ps(&store__matrix.container[_ac4(0, 1)], row1);

	_mm_storeu_ps(&store__matrix.container[_ac4(0, 2)], row2);

	// this is swapped

	row3 = _mm_shuffle_ps(row3, row3, _MM_SHUFFLE(1, 0, 3, 2));
	_mm_storeu_ps(&store__matrix.container[_ac4(0, 3)], row3);

	return store__matrix;
}

Mat4 InverseMat4(Mat4 matrix) {
	/*
	  Computes the inverse matrix using the adjugate formula
	  inverse A = 1/(det(A)) x adj(A)

	  where det is the determinant of the matrix and adj the adjoint matrix
	  of A. where 1 <= i <= n and 1 <= j <= n of an n by n matrix and (i,j)
	  corresponds to a value of matrix A adjoint__matrix of elements =
	  -(-1)^(i * j) * det(MinorMat(matrix,i,j)) adj(A) =
	  Transpose(adjoint__matrix)
	*/

	// we'll use these to get the det. Calling generic det will compute the
	// first row minor matrix det twice
	f32 first_row[] = {
	    matrix.container[_ac4(0, 0)], matrix.container[_ac4(1, 0)],
	    matrix.container[_ac4(2, 0)], matrix.container[_ac4(3, 0)]};

	// get the adjoint matrix, create matrices of cofactors

	Mat4 adj__matrix = {};

	for (u32 y = 0; y < 4; y++) {
		for (u32 x = 0; x < 4; x++) {
			Mat3 minormatrix = {};

			GetMinorMat((f32*)&matrix.container[0], 4, x, y,
				    (f32*)&minormatrix);

			adj__matrix.container[_ac4(x, y)] =
			    GenericGetDeterminant((f32*)&minormatrix, 3);
		}
	}

	Mat4 checkerboard__matrix = {
	    1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, -1, -1, 1, -1, 1,
	};

	adj__matrix = SchurMat4(adj__matrix, checkerboard__matrix);

	f32 det = 0;

	for (u32 i = 0; i < 4; i++) {
		det += first_row[i] * adj__matrix.container[_ac4(i, 0)];
	}

	adj__matrix = (1.0f / det) * TransposeMat4(adj__matrix);

	return adj__matrix;
}

Mat3 SchurMat3(Mat3 a, Mat3 b) {
	Mat3 matrix;

	matrix.simd[0] = _mm_mul_ps(a.simd[0], b.simd[0]);
	matrix.simd[1] = _mm_mul_ps(a.simd[1], b.simd[1]);
	matrix.k = a.k * b.k;

	return matrix;
}

Mat3 TransposeMat3(Mat3 matrix) {
	auto a = matrix.simd[0];
	auto b = matrix.simd[1];

	auto c = _mm_shuffle_ps(a, b, _MM_SHUFFLE(3, 0, 3, 0));
	auto d = _mm_shuffle_ps(a, b, _MM_SHUFFLE(2, 1, 2, 1));

	matrix.simd[0] = _mm_shuffle_ps(c, d, _MM_SHUFFLE(0, 3, 1, 0));
	matrix.simd[1] = _mm_shuffle_ps(c, d, _MM_SHUFFLE(2, 1, 3, 2));

	return matrix;
}

Mat3 InverseMat3(Mat3 matrix) {
	// see Inverse(Mat4)

	f32 first_row[] = {matrix.container[_ac3(0, 0)],
			   matrix.container[_ac3(1, 0)],
			   matrix.container[_ac3(2, 0)]

	};

	Mat3 adj__matrix = {};

	for (u32 y = 0; y < 3; y++) {
		for (u32 x = 0; x < 3; x++) {
			Mat2 minormatrix = {};

			GetMinorMat((f32*)&matrix[0], 3, x, y,
				    (f32*)&minormatrix);

			f32 a = minormatrix.container[_rc2(0, 0)];
			f32 b = minormatrix.container[_rc2(1, 0)];
			f32 c = minormatrix.container[_rc2(0, 1)];
			f32 d = minormatrix.container[_rc2(1, 1)];

			adj__matrix.container[_ac3(x, y)] = ((a * d) - (b * c));
		}
	}

	Mat3 checkerboard__matrix = {
	    1, -1, 1, -1, 1, -1, 1, -1, 1,
	};

	adj__matrix = SchurMat3(adj__matrix, checkerboard__matrix);

	f32 det = 0;

	for (u32 i = 0; i < 3; i++) {
		det += first_row[i] * adj__matrix.container[_ac3(i, 0)];
	}

	adj__matrix = (1.0f / det) * TransposeMat3(adj__matrix);

	return adj__matrix;
}

Mat2 TransposeMat2(Mat2 matrix) {
	matrix.simd =
	    _mm_shuffle_ps(matrix.simd, matrix.simd, _MM_SHUFFLE(3, 1, 2, 0));

	return matrix;
}

Mat2 InverseMat2(Mat2 matrix) {
	f32 a = matrix.container[_rc2(0, 0)];
	f32 b = matrix.container[_rc2(1, 0)];
	f32 c = matrix.container[_rc2(0, 1)];
	f32 d = matrix.container[_rc2(1, 1)];

	f32 k = 1.0f / ((a * d) - (b * c));

	matrix.container[_rc2(0, 0)] = d;
	matrix.container[_rc2(1, 0)] = b;
	matrix.container[_rc2(0, 1)] = c;
	matrix.container[_rc2(1, 1)] = a;

	return matrix * k;
}

f32 MagnitudeVec4(Vec4 vec) {
	// m = sqrtf(x^2 + y^2 + z^2)

	f32 res = DotVec4(vec, vec);
	res = sqrtf(res);

	return res;
}

f32 DotVec4(Vec4 vec1, Vec4 vec2) {
	//|a| * |b| * cos(angle between a and b)
	// or for a(1,2,3....n) and b(1,2,3....n). a.b = a1b1 + a2b2+ ...anbn

	__m128 mul = _mm_mul_ps(vec1.simd, vec2.simd);

	f32* _restrict r = (f32*)&mul;

	f32 res = r[0] + r[1] + r[2] + r[3];

	return res;
}

Vec4 NormalizeVec4(Vec4 vec) { return (vec) / (MagnitudeVec4(vec)); }

Vec4 SchurVec4(Vec4 a, Vec4 b) {
	a.simd = _mm_mul_ps(a.simd, b.simd);

	return a;
}

Vec4 InterpolateVec4(Vec4 a, Vec4 b, f32 step) {
	Vec4 v = {_intrin_fmadd_ps(_mm_sub_ps(b.simd, a.simd),
				   _mm_set1_ps(step), a.simd)};
	return v;
}

f32 MagnitudeVec3(Vec3 vec) {
	// m = sqrtf(x^2 * y^2 * z^2)

	f32 res = DotVec3(vec, vec);

	res = sqrtf(res);

	return res;
}

f32 DotVec3(Vec3 vec1, Vec3 vec2) {
	//|a| * |b| * cos(angle between a and b)
	// or for a(1,2,3....n) and b(1,2,3....n). a.b = a1b1 + a2b2+ ...anbn

	vec1.x *= vec2.x;
	vec1.y *= vec2.y;
	vec1.z *= vec2.z;

	return vec1.x + vec1.y + vec1.z;
}

Vec3 NormalizeVec3(Vec3 vec) { return (vec) / (MagnitudeVec3(vec)); }

Vec3 CrossVec3(Vec3 a, Vec3 b) {
	/* a(x,y,z) b(x,y,z)
	   cx = aybz - azby
	   cy = azbx - axbz
	   cz = axby - aybx

	*/

	Vec3 vec;

	vec.x = (a.y * b.z) - (a.z * b.y);
	vec.y = (a.z * b.x) - (a.x * b.z);
	vec.z = (a.x * b.y) - (a.y * b.x);

	return vec;
}

f32 CompVec3(Vec3 a, Vec3 b) { return DotVec3(a, NormalizeVec3(b)); }

Vec3 ProjectOntoVec3(Vec3 a, Vec3 b) {
	return CompVec3(a, b) * NormalizeVec3(b);
}

Vec3 ProjectVec3OntoPlane(Vec3 vec, Plane plane) {
	auto w = ProjectOntoVec3(vec, plane.norm);
	return vec - w;
}

Vec3 GetVecRotation(Vec3 lookat) {
	// updown,leftright,roll left roll right
	f32 x, y, z;

	x = AngleQuadrant(lookat.z, lookat.y);

	y = AngleQuadrant(lookat.x, lookat.z);

	z = AngleQuadrant(lookat.x, lookat.y);

	return Vec3{x, y, z};
}

Vec3 QuatRotateVec3(Vec3 v, Quat q) {
	/*
	  the rotated vector v' is given by
	  v' = q * v * inverse(q)
	  we are gonna optimize this case so we're not gonna do this verbatim
	*/

#if 1

	// better version? we need to condense the math
	Vec3 q_v = {q.x, q.y, q.z};
	f32 q_w = q.w;

	auto r = (2.0f * DotVec3(q_v, v) * q_v) +
		 (((q_w * q_w) - DotVec3(q_v, q_v)) * v) +
		 (2.0f * q_w * CrossVec3(q_v, v));

	return r;

#else

	// non-optimal (verbatim formula above)
	Quat t = {0.0f, v.x, v.y, v.z};
	auto q_inv = Inverse(q);

	auto k = q * t * q_inv;

	return Vec3{k.x, k.y, k.z, 1.0f};

#endif
}

Vec3 RotateVec3(Vec3 vec, Vec3 rotation) {
	/*

	  Multiply rotation matrix by vec

	  {rotated x,  = {cos0 ,-sin0,  * {x,
	  rotated y}      sin0,cos0}       y}
	*/

	auto rot__matrix = RotationMat3(rotation);

	auto a = rot__matrix.simd[0];
	auto b = rot__matrix.simd[1];
	auto c = rot__matrix.k;

	auto k = Vec3ToVec4(vec).simd;

	auto d = _mm_shuffle_ps(k, k, _MM_SHUFFLE(3, 2, 1, 0));
	auto e = _mm_shuffle_ps(k, k, _MM_SHUFFLE(1, 0, 2, 1));

	auto f = _mm_mul_ps(a, d);
	auto g = _mm_mul_ps(b, e);
	auto h = c * vec.z;

	// MARK: technically we can shuffle here and do an sse add.
	// idk what the wins are for
	auto x = ((f32*)&f)[0] + ((f32*)&f)[1] + ((f32*)&f)[2];
	auto y = ((f32*)&f)[3] + ((f32*)&g)[0] + ((f32*)&g)[1];
	auto z = ((f32*)&g)[2] + ((f32*)&g)[3] + h;

	return {x, y, z};
}

f32 Cosf(Vec3 vec1, Vec3 vec2) {
	return DotVec3(vec1, vec2) /
	       (MagnitudeVec3(vec1) * MagnitudeVec3(vec2));
}

f32 MagnitudeVec2(Vec2 a) {
	a.x = a.x * a.x;
	a.y = a.y * a.y;

	return sqrtf(a.x + a.y);
}

f32 DotVec2(Vec2 a, Vec2 b) {
	a = SchurVec2(a, b);
	return a.x + a.y;
}

Vec2 NormalizeVec2(Vec2 a) { return a / MagnitudeVec2(a); }

Vec2 SchurVec2(Vec2 a, Vec2 b) {
	a.x *= b.x;
	a.y *= b.y;
	return a;
}

Vec2 RotateVec2(Vec2 vec, f32 rotation) {
	vec.x = (vec.x * cosf(rotation)) - (vec.y * sinf(rotation));
	vec.y = (vec.x * sinf(rotation)) + (vec.y * cosf(rotation));

	return vec;
}

Quat InverseQuat(Quat q) {
	/*
	 The Inverse of the quaternion is defined as the
	 ConjugateQuat(q)/(Magnitude(q))^2 However, as the quaternions used are
	 unit quaternions, the Magnitude(q) is always 1. Just insert the
	 conjugate here.
	 */

	q.w *= -1.0f;

	return q * -1.0f;
}

Quat NLerpQuat(Quat a, Quat b, f32 step) {
	b = Neighbourhood(a, b);

	Quat q = InterpolateQuat(a, b, step);

	q = NormalizeQuat(q);

	return q;
}

Quat SLerpQuat(Quat a, Quat b, f32 step) {
	f32 dot = _clampf(DotQuat(a, b), -1.0f, 1.0f);

	f32 angle = acosf(dot) * step;

	Quat n = NormalizeQuat(b - (a * dot));

	return (n * sinf(angle)) + (a * cosf(angle));
}

Quat ConjugateQuat(Quat quaternion) {
	__m128 k = _mm_set1_ps(-1.0f);

	quaternion.simd = _mm_mul_ps(quaternion.simd, k);

	quaternion.w *= -1.0f;

	return quaternion;
}

Quat InterpolateQuat(Quat a, Quat b, f32 step) {
	Quat q = {_intrin_fmadd_ps(_mm_sub_ps(b.simd, a.simd),
				   _mm_set1_ps(step), a.simd)};
	return q;
}

DualQuat NormalizeDualQ(DualQuat d) {
	f32 magnitude = DotQuat(d.q1, d.q1);

	_kill("dq normalize error\n", magnitude <= 0.000001f);

	d.q1 = d.q1 / magnitude;
	d.q2 = d.q2 / magnitude;

	return d;
}


b32 IntersectLine3(Line3 a, Line3 b) {
	/*
	  the form of a vector as a line is as follows:
	  r = p + sd
	  r is any point on the line (var)
	  p is a specific point of the line (const)
	  d is the dir of the line relative to point p (const)
	  s is the scale factor of dir. (var)

	  s functionally moves r along the line by scaling dir and adding it to
	  point p.

	  if 2 lines intersect, a == b

	  a.pos + (t * a.dir) = b.pos + (k * b.dir)
	  (t * a.dir) = (b.pos - a.pos) + (k * b.dir)

	  crossing both sides by b.dir gets rid of the (k * b.dir) (cross(x,x) =
	  0), leaving

	  t * cross(a.dir,b.dir) = cross((b.pos - a.pos),b.dir)

	  if they are the same then norm(cross(a.dir,b.dir)) = norm(cross((b.pos
	  - a.pos),b.dir)) are the same or norm(cross(a.dir,b.dir)) =
	  norm(cross((b.pos - a.pos),b.dir))

	*/

	auto cross_ab = CrossVec3(a.dir, b.dir);
	auto cross_diff = CrossVec3(b.pos - a.pos, b.dir);

	auto dot = DotVec3(NormalizeVec3(cross_ab), NormalizeVec3(cross_diff));

	return (u32)(fabsf(dot) + _f32_error_offset);
}

b32 IntersectOutLine3(Line3 a, Line3 b, Point3* out_point) {
	auto cross_ab = CrossVec3(a.dir, b.dir);
	auto cross_diff = CrossVec3(b.pos - a.pos, b.dir);

	auto dot = DotVec3(NormalizeVec3(cross_ab), NormalizeVec3(cross_diff));

	if (!(u32)(fabsf(dot) + _f32_error_offset)) {
		return false;
	}

	f32 t = 0.0f;

	for (u32 i = 0; i < 3; i++) {
		m32 fi1;

		fi1.f = cross_ab.floats[i];

		if (fi1.u) {
			t = cross_diff.floats[i] / cross_ab.floats[i];
			break;
		}
	}

	*out_point = (a.dir * t) + a.pos;

	return true;
}

b32 TypedIntersectLine3(Line3 a, Line3 b) {
	auto dir =
	    NormalizeVec3(b.pos - a.pos);  // checks if the lines are the same

	b32 res = (u32)(fabsf(DotVec3(NormalizeVec3(a.dir), dir)) +
			_f32_error_offset) +
		  IntersectLine3(a, b);

	if (res) {
		return res;
	}

	return false;
}

b32 IntersectLine3Plane(Line3 a, Plane b) {
	m32 fi1;

	// if they are perpendicular, f is 0 and they do not intersect
	fi1.f = DotVec3(NormalizeVec3(a.dir), NormalizeVec3(b.norm));

	return fi1.u != 0;
}

b32 IntersectOutLine3Plane(Line3 a, Plane b, Point3* out_point) {
	/*
	  we first imagine a line (A) from our plane normal position (B) to any
	  position on our line (L). the angle between the normal (N) and the
	  line (A) will be known as (d). As the line (A) moves along the line
	  (L), approaching the point of intersection between line (L) and the
	  plane, the angle (d) will approach 90 degrees. So it is given that the
	  dot product of line (A) and the normal line (N) will be 0 at the point
	  of intersection. Given the vector from of a line: r = P + t * dir
	  where r is any point on the line, p is a known point on the line, t is
	  the scale factor to the direction vector and dir is the direction
	  vector, we can say:

	  dot((P + (t * dir) - B),N) = 0
	  which can be rewritten to:
	  t = -(dot((P - B),N))/(dot(dir,N))
	  or
	  t = (dot((B - P),N))/(dot(dir,N))
	*/

	if (!IntersectLine3Plane(a, b)) {
		return false;
	}

	auto n = NormalizeVec3(b.norm);
	auto dir = NormalizeVec3(a.dir);

	auto t = (DotVec3((b.pos - a.pos), n)) / (DotVec3(dir, n));

	*out_point = a.pos + (t * dir);

	return true;
}

b32 TypedIntersectLine3Plane(Line3 a, Plane b) {
	/*
	  if the dir of a line is perpendicular to a plane normal, it will never
	  intersect unless the line is on the plane
	*/

	auto is_intersect = IntersectLine3Plane(a, b);

	m32 fi;

	auto dir = NormalizeVec3(b.pos - a.pos);

	fi.f = DotVec3(dir, NormalizeVec3(b.norm));  // check if on the plane

	auto is_perpendicular = !fi.u;

	if (!is_intersect && is_perpendicular) {
		return INTERSECT_INFINITE;
	}

	else if (!is_intersect) {
		return INTERSECT_FALSE;
	}

	return INTERSECT_FINITE;
}

b32 IntersectLine2(Line2 a_2, Line2 b_2) {
	a_2.dir = NormalizeVec2(a_2.dir);
	b_2.dir = NormalizeVec2(b_2.dir);

	auto dot = DotVec2(a_2.dir, b_2.dir);

	return !(u32)(fabsf(dot) + _f32_error_offset);
}

b32 IntersectOutLine2(Line2 a_2, Line2 b_2, Point2* out_point) {
	a_2.dir = NormalizeVec2(a_2.dir);
	b_2.dir = NormalizeVec2(b_2.dir);

	auto dot = DotVec2(a_2.dir, b_2.dir);

	if (!(u32)(fabsf(dot) + _f32_error_offset)) {
		return false;
	}

	// this uses y = mx + c (TODO: test this)

	auto m1 = a_2.dir.y / a_2.dir.x;
	auto m2 = b_2.dir.y / b_2.dir.x;

	auto c1 = a_2.pos.y - (a_2.pos.x * m1);
	auto c2 = b_2.pos.y - (b_2.pos.x * m2);

	auto x = (-1.0f * (c1 - c2)) / (m1 - m2);
	auto y = (m1 * x) + c1;

	*out_point = {x, y};

	return true;
}

b32 TypedIntersectLine2(Line2 a_2, Line2 b_2) {
	Line3 a = {Vec3{a_2.pos.x, a_2.pos.y, 1.0f},
		   Vec3{a_2.dir.x, a_2.dir.y, 1.0f}};
	Line3 b = {Vec3{b_2.pos.x, b_2.pos.y, 1.0f},
		   Vec3{b_2.dir.x, b_2.dir.y, 1.0f}};

	auto dir =
	    NormalizeVec3(b.pos - a.pos);  // checks if the lines are the same

	b32 res = (u32)(fabsf(DotVec3(NormalizeVec3(a.dir), dir)) +
			_f32_error_offset) +
		  IntersectLine3(a, b);

	if (res) {
		return res;
	}

	return false;
}

b32 IntersectRay3(Ray3 a, Ray3 b) {
	auto cross_ab = CrossVec3(a.dir, b.dir);
	auto cross_diff = CrossVec3(b.pos - a.pos, b.dir);

	auto dot = DotVec3(NormalizeVec3(cross_ab), NormalizeVec3(cross_diff));


	return dot >= _f32_error_offset;
}

b32 IntersectOutRay3(Ray3 a, Ray3 b, Point3* out_point){
	auto cross_ab = CrossVec3(a.dir, b.dir);
	auto cross_diff = CrossVec3(b.pos - a.pos, b.dir);

	auto dot = DotVec3(NormalizeVec3(cross_ab), NormalizeVec3(cross_diff));

	if (dot >= _f32_error_offset) {
		return false;
	}

	f32 t = 0.0f;

	for (u32 i = 0; i < 3; i++) {
		m32 fi1;

		fi1.f = cross_ab.floats[i];

		if (fi1.u) {
			t = cross_diff.floats[i] / cross_ab.floats[i];
			break;
		}
	}

	*out_point = (a.dir * t) + a.pos;

	return true;
}
b32 TypedIntersectRay3(Ray3 a, Ray3 b){
	auto dir =
	    NormalizeVec3(b.pos - a.pos);  // checks if the lines are the same

	b32 res = (u32)(fabsf(DotVec3(NormalizeVec3(a.dir), dir)) +
			_f32_error_offset) +
		  IntersectRay3(a, b);

	if (res) {
		return res;
	}

	return false;
}

b32 IntersectRay3Plane(Ray3 a, Plane b){

	// if they are perpendicular, f is 0 and they do not intersect
	return DotVec3(NormalizeVec3(a.dir), NormalizeVec3(b.norm))
		> _f32_error_offset;
}

b32 IntersectOutRay3Plane(Ray3 a, Plane b, Point3* out_point){
	if (!IntersectRay3Plane(a, b)) {
		return false;
	}

	auto n = NormalizeVec3(b.norm);
	auto dir = NormalizeVec3(a.dir);

	auto t = (DotVec3((b.pos - a.pos), n)) / (DotVec3(dir, n));

	*out_point = a.pos + (t * dir);

	return true;
}

b32 TypedIntersectRay3Plane(Ray3 a, Plane b){
	auto is_intersect = IntersectRay3Plane(a, b);

	m32 fi = {};

	auto dir = NormalizeVec3(b.pos - a.pos);

	fi.f = DotVec3(dir, NormalizeVec3(b.norm));  // check if on the plane

	auto is_perpendicular = !fi.u;

	if (!is_intersect && is_perpendicular) {
		return INTERSECT_INFINITE;
	}

	else if (!is_intersect) {
		return INTERSECT_FALSE;
	}

	return INTERSECT_FINITE;
}

b32 IntersectRay2(Ray2 a_2, Ray2 b_2){
	a_2.dir = NormalizeVec2(a_2.dir);
	b_2.dir = NormalizeVec2(b_2.dir);

	auto dot = DotVec2(a_2.dir, b_2.dir);

	return dot >= _f32_error_offset;
}

b32 IntersectOutRay2(Ray2 a_2, Ray2 b_2, Point2* out_point){
	a_2.dir = NormalizeVec2(a_2.dir);
	b_2.dir = NormalizeVec2(b_2.dir);

	auto dot = DotVec2(a_2.dir, b_2.dir);

	if (dot >= _f32_error_offset) {
		return false;
	}

	// this uses y = mx + c (TODO: test this)

	auto m1 = a_2.dir.y / a_2.dir.x;
	auto m2 = b_2.dir.y / b_2.dir.x;

	auto c1 = a_2.pos.y - (a_2.pos.x * m1);
	auto c2 = b_2.pos.y - (b_2.pos.x * m2);

	auto x = (-1.0f * (c1 - c2)) / (m1 - m2);
	auto y = (m1 * x) + c1;

	*out_point = {x, y};

	return true;
}

b32 TypedIntersectRay2(Ray2 a_2, Ray2 b_2){
	Ray3 a = {Vec3{a_2.pos.x, a_2.pos.y, 1.0f},
		   Vec3{a_2.dir.x, a_2.dir.y, 1.0f}};
	Ray3 b = {Vec3{b_2.pos.x, b_2.pos.y, 1.0f},
		   Vec3{b_2.dir.x, b_2.dir.y, 1.0f}};

	auto dir =
	    NormalizeVec3(b.pos - a.pos);  // checks if the lines are the same

	b32 res = (u32)(fabsf(DotVec3(NormalizeVec3(a.dir), dir)) +
			_f32_error_offset) +
		  IntersectRay3(a, b);

	if (res) {
		return res;
	}

	return false;
}

void MinkowskiAddition(Point3* a, ptrsize a_count, Point3* b, ptrsize b_count,
		       Point3** ret) {
	u32 watch = 0;

	for (u32 i = 0; i < a_count; i++) {
		for (u32 j = 0; j < b_count; j++) {
			watch = j + (i * (b_count));

			(*ret)[watch] = a[i] + b[j];
		}
	}
}

void MinkowskiDifference(Point3* a, ptrsize a_count, Point3* b, ptrsize b_count,
			 Point3** ret) {
	u32 watch = 0;

	for (u32 i = 0; i < a_count; i++) {
		for (u32 j = 0; j < b_count; j++) {
			watch = j + (i * (b_count));

			(*ret)[watch] = a[i] - b[j];
		}
	}
}

f32 AngleQuadrant(f32 x, f32 y) {
	if (x == 0.0f && y == 0.0f) {
		return 0.0f;
	}

	f32 teeter = atan2f(x, y);

#if 0
    
    if(teeter < 0.0f){
        teeter += _twopi;
    }

#endif

	return teeter;
}

// MARK: constructors

Mat4 ViewMat4(Vec3 position, Vec3 lookpoint, Vec3 updir) {
#if Z_RHS

	return ViewMatRHS(position, lookpoint, updir);

#else

#error Z_LHS not supported

#endif
}

Mat4 ProjectionMat4(f32 fov, f32 aspectratio, f32 nearz, f32 farz) {
	f32 tanhalf_fov = tanf(fov / 2.0f);

	f32 a = (1.0f / (aspectratio * tanhalf_fov));
	f32 b = (1.0f / (tanhalf_fov));

#if DEPTH_ZERO_TO_ONE

	f32 c = (farz) / (nearz - farz);
	f32 d = -(farz * nearz) / (farz - nearz);

#else

	f32 c = -(farz + nearz) / (farz - nearz);
	f32 d = -(2.0f * farz * nearz) / (farz - nearz);

#endif

	Mat4 matrix = {};

	matrix.container[_rc4(0, 0)] = a;
	matrix.container[_rc4(1, 1)] = b;
	matrix.container[_rc4(2, 2)] = c;
	matrix.container[_rc4(3, 2)] = d;
	matrix.container[_rc4(2, 3)] = -1.0f;

#ifdef DEBUG

#if _test__matrices

	auto ref__matrix = matrix;

	auto t__mat = glm::perspective(fov, aspectratio, nearz, farz);

	auto f1 = (f32*)&ref__matrix;
	auto f2 = (f32*)&t__mat;

	InternalCmpMat(f1, f2);

#endif

#endif

	return matrix;
}

Mat4 WorldMat4M(Mat4 position, Mat4 rotation, Mat4 scale) {
	return position * rotation * scale;
}

Mat4 WorldMat4V(Vec3 position, Vec3 rotation, Vec3 scale) {
	Mat4 matrix;

	Mat4 position__matrix4 = PositionMat4(position);

	Mat4 scale__matrix4 = ScaleMat4(scale);

	Mat4 rotation__matrix4 = Mat3ToMat4(RotationMat3(rotation));

	matrix =
	    WorldMat4M(position__matrix4, rotation__matrix4, scale__matrix4);

	return matrix;
}

Mat4 WorldMat4Q(Vec3 position, Quat rotation, Vec3 scale) {
	Mat4 matrix;

	Mat4 position__matrix4 = PositionMat4(position);

	Mat4 scale__matrix4 = ScaleMat4(scale);

	Mat4 rotation__matrix4 = QuatToMat4(rotation);

	matrix =
	    WorldMat4M(position__matrix4, rotation__matrix4, scale__matrix4);

	return matrix;
}

Quat ConstructQuat(Vec3 vector, f32 angle) {
	vector = NormalizeVec3(vector);

	Quat quaternion;

	f32 k = sinf(angle / 2.0f);

	vector.x *= k;
	vector.y *= k;
	vector.z *= k;

	quaternion.w = cosf(angle / 2.0f);
	quaternion.x = vector.x;
	quaternion.y = vector.y;
	quaternion.z = vector.z;

	return quaternion;
}

DualQuat ConstructDualQM(Mat4 transform) {
	DualQuat d = {};

	d.q1 = Mat4ToQuat(transform);
	Vec3 translation = Mat4ToTranslationVec(transform);

	d.q2 =
	    Quat{0, translation.x, translation.y, translation.z} * d.q1 * 0.5f;

	return d;
}

DualQuat ConstructDualQ(Quat rotation, Vec3 translation) {
	/*
	  real = rotation
	  dual = 0.5 * translation * rotation
	*/

	DualQuat d;
	d.q1 = rotation;
	d.q2 =
	    Quat{0, translation.x, translation.y, translation.z} * d.q1 * 0.5f;
	return d;
}

// MARK: deconstrutors

void DeconstructQuat(Quat quaternion, Vec3* vector, f32* angle) {
	f32 anglew = acosf(quaternion.w) * 2.0f;

	f32 scale = sinf(anglew);

	// we should handle the case scale == 0
	if (scale == 0.0f) {
		*vector = Vec3{1, 0, 0};
		*angle = 0;
		return;
	}
	vector->x = quaternion.w / scale;
	vector->y = quaternion.x / scale;
	vector->z = quaternion.y / scale;

	*angle = anglew;
}

// MARK: UTIL OPS

void PrintMat4(Mat4 matrix) {
	printf("\n");

	for (u32 i = 0; i < 16; i++) {
		if (i % 4 == 0) printf("\n");

		printf("%f   ", (f64)matrix.container[i]);
	}

	printf("\n");
}

void PrintMat3(Mat3 matrix) {
	printf("\n");

	for (u32 i = 0; i < 9; i++) {
		if (i % 3 == 0) printf("\n");

		printf("%f   ", (f64)matrix[i]);
	}

	printf("\n");
}

void PrintVec4(Vec4 vec) {
	printf("%f   %f   %f   %f\n", (f64)vec.x, (f64)vec.y, (f64)vec.z,
	       (f64)vec.w);
}

void PrintVec3(Vec3 vec) {
	printf("%f   %f   %f\n", (f64)vec.x, (f64)vec.y, (f64)vec.z);
}

void PrintVec2(Vec2 vec) { printf("%f   %f\n", (f64)vec.x, (f64)vec.y); }

void PrintQuat(Quat vec) {
	printf("%f   %f   %f   %f\n", (f64)vec.w, (f64)vec.x, (f64)vec.y,
	       (f64)vec.z);
}
}

// MARK:cpp Math operators here

Mat4 operator+(Mat4 lhs, Mat4 rhs) { return AddMat4(lhs, rhs); }

Mat4 operator-(Mat4 lhs, Mat4 rhs) { return SubMat4(lhs, rhs); }

Mat4 operator*(Mat4 lhs, Mat4 rhs) { return MulMat4(lhs, rhs); }

Mat4 operator*(f32 lhs, Mat4 rhs) { return MulConstLMat4(lhs, rhs); }

Mat4 operator*(Mat4 lhs, f32 rhs) { return rhs * lhs; }

Mat4 operator/(Mat4 lhs, Mat4 rhs) { return DivMat4(lhs, rhs); }

Mat3 operator+(Mat3 lhs, Mat3 rhs) { return AddMat3(lhs, rhs); }

Mat3 operator-(Mat3 lhs, Mat3 rhs) { return SubMat3(lhs, rhs); }

Mat3 operator*(Mat3 lhs, Mat3 rhs) { return MulMat3(lhs, rhs); }

Mat3 operator*(f32 lhs, Mat3 rhs) { return MulConstLMat3(lhs, rhs); }

Mat3 operator*(Mat3 lhs, f32 rhs) { return rhs * lhs; }

Mat3 operator/(Mat3 lhs, Mat3 rhs) { return DivMat3(lhs, rhs); }

Mat2 operator+(Mat2 lhs, Mat2 rhs) { return AddMat2(lhs, rhs); }

Mat2 operator-(Mat2 lhs, Mat2 rhs) { return SubMat2(lhs, rhs); }

Mat2 operator*(Mat2 lhs, Mat2 rhs) { return MulMat2(lhs, rhs); }

Mat2 operator*(f32 lhs, Mat2 rhs) { return MulConstLMat2(lhs, rhs); }

Mat2 operator*(Mat2 lhs, f32 rhs) { return rhs * lhs; }

Vec4 operator+(Vec4 lhs, Vec4 rhs) { return AddVec4(lhs, rhs); }

Vec4 operator-(Vec4 lhs, Vec4 rhs) { return SubVec4(lhs, rhs); }

Vec4 operator*(f32 lhs, Vec4 rhs) { return MulConstLVec4(lhs, rhs); }

Vec4 operator*(Vec4 lhs, f32 rhs) { return rhs * lhs; }

Vec4 operator/(Vec4 lhs, f32 rhs) { return DivConstRVec4(lhs, rhs); }

Vec3 operator+(Vec3 lhs, Vec3 rhs) { return AddVec3(lhs, rhs); }

Vec3 operator-(Vec3 lhs, Vec3 rhs) { return SubVec3(lhs, rhs); }

Vec3 operator*(f32 lhs, Vec3 rhs) { return MulConstLVec3(lhs, rhs); }

Vec3 operator*(Vec3 lhs, f32 rhs) { return rhs * lhs; }

Vec3 operator/(Vec3 lhs, f32 rhs) { return DivConstRVec3(lhs, rhs); }

Vec2 operator+(Vec2 lhs, Vec2 rhs) { return AddVec2(lhs, rhs); }

Vec2 operator-(Vec2 lhs, Vec2 rhs) { return SubVec2(lhs, rhs); }

Vec2 operator*(f32 lhs, Vec2 rhs) { return MulConstLVec2(lhs, rhs); }

Vec2 operator*(Vec2 lhs, f32 rhs) { return rhs * lhs; }

Vec2 operator/(Vec2 lhs, f32 rhs) { return DivConstRVec2(lhs, rhs); }

Quat operator*(Quat lhs, Quat rhs) { return MulQuat(lhs, rhs); }

DualQuat operator+(DualQuat lhs, DualQuat rhs) { return AddDualQ(lhs, rhs); }

DualQuat operator-(DualQuat lhs, DualQuat rhs) { return SubDualQ(lhs, rhs); }

DualQuat operator*(DualQuat lhs, DualQuat rhs) { return MulDualQ(lhs, rhs); }

DualQuat operator*(f32 lhs, DualQuat rhs) { return MulConstLDualQ(lhs, rhs); }

DualQuat operator*(DualQuat lhs, f32 rhs) { return rhs * lhs; }
