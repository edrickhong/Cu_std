/*
TODO:
Implement dualq interpolation
Remove extraneous Normalizes:
Lines,rays and planes are easy to remove extraneous normalized. they
are pretty opaque objects and don't have operations that directly act
on them. We would like to do this to our quats and dqs as well, but
they are harder to gaurantee to never change

When we have the parser working on comments, we can start using Latex
to write the docs here
*/

// Compares value a to check if it is within the error range that we'd
// consider this 0
b32 CmpErrorZero(f32 a) {
	return a < _f32_error_offset && a > (-1.0f * _f32_error_offset);
}

// MARK: This is internal for now
Vec3 operator*(Mat3 lhs, Vec3 rhs) {
	auto a = lhs.simd[0];
	auto b = lhs.simd[1];
	auto c = lhs.k;

	auto k = Vec3ToVec4(rhs).simd;

	auto d = _mm_shuffle_ps(k, k, _MM_SHUFFLE(0, 2, 1, 0));
	auto e = _mm_shuffle_ps(k, k, _MM_SHUFFLE(1, 0, 2, 1));

	auto f = _mm_mul_ps(a, d);
	auto g = _mm_mul_ps(b, e);
	auto h = c * rhs.z;

	// MARK: technically we can shuffle here and do an sse add.
	// idk what the wins are
	auto x = ((f32*)&f)[0] + ((f32*)&f)[1] + ((f32*)&f)[2];
	auto y = ((f32*)&f)[3] + ((f32*)&g)[0] + ((f32*)&g)[1];
	auto z = ((f32*)&g)[2] + ((f32*)&g)[3] + h;

	return {x, y, z};
}
#if 0
// MARK: This is internal for now
// TODO: remove this pretty sure it does the same thing
Vec4 operator*(Mat4 lhs, Vec4 rhs) {
	Vec4 result = {};

	for (u32 i = 0; i < 4; i++) {
		auto s = lhs.simd[i];

		auto r = _mm_mul_ps(s, rhs.simd);

		auto k = (f32*)&r;

		result.container[i] = k[0] + k[1] + k[2] + k[3];  
	}

	return result;
}
#endif

void _ainline GetMinorMat(f32* in_matrix, u32 n, u32 k_x, u32 k_y,
			  f32* out_matrix) {
	u32 index = 0;

	for (u32 y = 0; y < n; y++) {
		for (u32 x = 0; x < n; x++) {
			if (y != k_y && x != k_x) {
				out_matrix[index] = in_matrix[(y * n) + x];
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

	_rc4(matrix, 0, 0) = side.x;
	_rc4(matrix, 1, 0) = side.y;
	_rc4(matrix, 2, 0) = side.z;
	_rc4(matrix, 3, 0) = a;

	_rc4(matrix, 0, 1) = up.x;
	_rc4(matrix, 1, 1) = up.y;
	_rc4(matrix, 2, 1) = up.z;
	_rc4(matrix, 3, 1) = b;

	_rc4(matrix, 0, 2) = -forward.x;
	_rc4(matrix, 1, 2) = -forward.y;
	_rc4(matrix, 2, 2) = -forward.z;
	_rc4(matrix, 3, 2) = c;

	return matrix;
}

Vec4 InternalCompDiv(Vec4 a, Vec4 b) {
	a.simd = _mm_div_ps(a.simd, b.simd);
	return a;
}

f32 inline GenericGetDeterminant(f32* in_matrix, u32 n) {
	_kill("we do not support this case\n", n > 4);

	if (n == 2) {
		f32 a = in_matrix[0];
		f32 b = in_matrix[1];
		f32 c = in_matrix[2];
		f32 d = in_matrix[3];

		return (a * d) - (b * c);
	}

	f32 res = 0;

	for (u32 i = 0; i < n; i++) {
		auto entry = in_matrix[i];

		if (i & 1) {
			entry *= -1.0f;
		}

		f32 minor_mat[16] = {};

		GetMinorMat(in_matrix, n, i, 0, &minor_mat[0]);

		auto det = GenericGetDeterminant(&minor_mat[0], n - 1);

		res += det * entry;
	}

	return res;
}

// TODO: maybe double cover would be better
template <class T>
T Neighbourhood(Quat a, T b) {
	auto b_q = *(Quat*)(&b.container[0]);
	f32 dot = DotQuat(a, b_q);

	if (dot < 0.0f) {
		b = b * -1.0f;
	}

	return b;
}

_ainline b32 InternalIsInline(Line3 a, Line3 b) {
	auto diff = NormalizeVec3(b.pos - a.pos);
	f32 dot = DotVec3(diff, a.dir);
	return (u32)(fabsf(dot) + _f32_error_offset);
}

_ainline b32 InternalIsInline(Line2 a, Line2 b) {
	auto diff = NormalizeVec2(b.pos - a.pos);
	f32 dot = DotVec2(diff, a.dir);
	return (u32)(fabsf(dot) + _f32_error_offset);
}

_ainline b32 InternalIsInline(Ray3 a, Ray3 b) {
	return InternalIsInline(Ray3ToLine3(a), Ray3ToLine3(b));
}

_ainline b32 InternalIsInline(Ray2 a, Ray2 b) {
	return InternalIsInline(Ray2ToLine2(a), Ray2ToLine2(b));
}

_ainline b32 InternalIsParallel(Line3 a, Line3 b) {
	f32 dot = DotVec3(a.dir, b.dir);
	return (u32)(fabsf(dot) + _f32_error_offset);
}

_ainline b32 InternalIsParallel(Line2 a, Line2 b) {
	f32 dot = DotVec2(a.dir, b.dir);
	return (u32)(fabsf(dot) + _f32_error_offset);
}

_ainline b32 InternalIsParallel(Ray3 a, Ray3 b) {
	return InternalIsParallel(Ray3ToLine3(a), Ray3ToLine3(b));
}

_ainline b32 InternalIsParallel(Ray2 a, Ray2 b) {
	return InternalIsParallel(Ray2ToLine2(a), Ray2ToLine2(b));
}

// MARK: conversions

extern "C" {

Mat4 QuatToMat4(Quat quaternion) {
	Quat squared = {};

	squared.simd = _mm_mul_ps(quaternion.simd, quaternion.simd);

	f32 a = 1.0f - (2.0f * (squared.y + squared.z));

	// MARK: I feel like these can be simd'd

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

	_rc4(matrix, 0, 0) = a;
	_rc4(matrix, 1, 0) = b;
	_rc4(matrix, 2, 0) = c;

	_rc4(matrix, 0, 1) = d;
	_rc4(matrix, 1, 1) = e;
	_rc4(matrix, 2, 1) = f;

	_rc4(matrix, 0, 2) = g;
	_rc4(matrix, 1, 2) = h;
	_rc4(matrix, 2, 2) = i;

	_rc4(matrix, 3, 3) = 1.0f;

	return matrix;
}
Quat Mat4ToQuat(Mat4 matrix) {
	Quat q = {};

	f32 trs = _rc4(matrix, 0, 0) + _rc4(matrix, 1, 1) + _rc4(matrix, 2, 2);

	if (trs > 0.0f) {
		f32 s = sqrtf(trs + 1.0f) * 2.0f;
		q.w = 0.25f * s;
		q.x = (_rc4(matrix, 1, 2) - _rc4(matrix, 2, 1)) / s;
		q.y = (_rc4(matrix, 2, 0) - _rc4(matrix, 0, 2)) / s;
		q.z = (_rc4(matrix, 0, 1) - _rc4(matrix, 1, 0)) / s;
	}

	else if ((_rc4(matrix, 0, 0) > _rc4(matrix, 1, 1)) &&
		 (_rc4(matrix, 0, 0) > _rc4(matrix, 2, 2))) {
		f32 s = sqrtf(1.0f + _rc4(matrix, 0, 0) - _rc4(matrix, 1, 1) -
			      _rc4(matrix, 2, 2)) *
			2.0f;
		q.w = (_rc4(matrix, 1, 2) - _rc4(matrix, 2, 1)) / s;
		q.x = 0.25f * s;
		q.y = (_rc4(matrix, 1, 0) + _rc4(matrix, 1, 0)) / s;
		q.z = (_rc4(matrix, 2, 0) + _rc4(matrix, 0, 2)) / s;
	}

	else if (_rc4(matrix, 1, 1) > _rc4(matrix, 2, 2)) {
		f32 s = sqrtf(1.0f + _rc4(matrix, 1, 1) - _rc4(matrix, 0, 0) -
			      _rc4(matrix, 2, 2)) *
			2.0f;

		q.w = (_rc4(matrix, 2, 0) - _rc4(matrix, 0, 2)) / s;
		q.x = (_rc4(matrix, 1, 0) + _rc4(matrix, 1, 0)) / s;
		q.y = 0.25f * s;
		q.z = (_rc4(matrix, 2, 1) + _rc4(matrix, 1, 2)) / s;
	}

	else {
		f32 s = sqrtf(1.0f + _rc4(matrix, 2, 2) - _rc4(matrix, 0, 0) -
			      _rc4(matrix, 1, 1)) *
			2.0f;

		q.w = (_rc4(matrix, 0, 1) - _rc4(matrix, 1, 0)) / s;
		q.x = (_rc4(matrix, 2, 0) + _rc4(matrix, 0, 2)) / s;
		q.y = (_rc4(matrix, 2, 1) + _rc4(matrix, 1, 2)) / s;
		q.z = 0.25f * s;
	}

	return NormalizeQuat(q);
}

Mat4 DualQToMat4(DualQ d) {
	d = NormalizeDualQ(d);	// MARK: possibly extraneous

	Mat4 matrix = QuatToMat4(d.q1);
	Quat t = d.q2 * 2.0f;
	t = t * ConjugateQuat(d.q1);

	_rc4(matrix, 3, 0) = t.x;
	_rc4(matrix, 3, 1) = t.y;
	_rc4(matrix, 3, 2) = t.z;
	_rc4(matrix, 3, 3) = 1.0f;

	return matrix;
}

Vec4 WorldSpaceToClipSpaceVec4(Vec4 pos, Mat4 viewproj) {
#if !MATRIX_ROW_MAJOR

	viewproj = TransposeMat4(viewproj);

#endif
	auto ret = viewproj * pos;
	return ret / ret.w;
}

Vec4 ClipSpaceToWorldSpaceVec4(Vec4 pos, Mat4 viewproj) {
	auto inv_viewproj = InverseMat4(viewproj);

#if !MATRIX_ROW_MAJOR

	inv_viewproj = TransposeMat4(inv_viewproj);

#endif
	auto ret = inv_viewproj * pos;
	return ret / ret.w;
}

// MARK: MATH OPS

Mat4 AddMat4(Mat4 lhs, Mat4 rhs) {
	Mat4 matrix;

	_mm_store_ps(&_ac4(matrix, 0, 0), _mm_add_ps(lhs.simd[0], rhs.simd[0]));

	_mm_store_ps(&_ac4(matrix, 0, 1), _mm_add_ps(lhs.simd[1], rhs.simd[1]));

	_mm_store_ps(&_ac4(matrix, 0, 2), _mm_add_ps(lhs.simd[2], rhs.simd[2]));

	_mm_store_ps(&_ac4(matrix, 0, 3), _mm_add_ps(lhs.simd[3], rhs.simd[3]));

	return matrix;
}

Mat4 SubMat4(Mat4 lhs, Mat4 rhs) {
	Mat4 matrix;

	_mm_store_ps(&_ac4(matrix, 0, 0), _mm_sub_ps(lhs.simd[0], rhs.simd[0]));

	_mm_store_ps(&_ac4(matrix, 0, 1), _mm_sub_ps(lhs.simd[1], rhs.simd[1]));

	_mm_store_ps(&_ac4(matrix, 0, 2), _mm_sub_ps(lhs.simd[2], rhs.simd[2]));

	_mm_store_ps(&_ac4(matrix, 0, 3), _mm_sub_ps(lhs.simd[3], rhs.simd[3]));

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

		_ac4(matrix, 0, i) = r1[0] + r1[1] + r1[2] + r1[3];
		_ac4(matrix, 1, i) = r2[0] + r2[1] + r2[2] + r2[3];
		_ac4(matrix, 2, i) = r3[0] + r3[1] + r3[2] + r3[3];
		_ac4(matrix, 3, i) = r4[0] + r4[1] + r4[2] + r4[3];
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

		auto ref_matrix = matrix;

		InternalCmpMat((f32*)&ref_matrix, (f32*)&res);
	}

#endif

	return matrix;
}

Mat4 MulConstLMat4(f32 lhs, Mat4 rhs) {
	Mat4 matrix;
	__m128 k = _mm_set1_ps(lhs);

	__m128 res = _mm_mul_ps(rhs.simd[0], k);

	_mm_store_ps(&_ac4(matrix, 0, 0), res);

	res = _mm_mul_ps(rhs.simd[1], k);

	_mm_store_ps(&_ac4(matrix, 0, 1), res);

	res = _mm_mul_ps(rhs.simd[2], k);

	_mm_store_ps(&_ac4(matrix, 0, 2), res);

	res = _mm_mul_ps(rhs.simd[3], k);

	_mm_store_ps(&_ac4(matrix, 0, 3), res);

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

	_rc3(matrix, 0, 0) = ((f32*)&h)[0] + ((f32*)&h)[1] + ((f32*)&h)[2];
	_rc3(matrix, 1, 0) = ((f32*)&i)[0] + ((f32*)&i)[1] + ((f32*)&i)[2];
	_rc3(matrix, 2, 0) = ((f32*)&j)[0] + ((f32*)&j)[1] + ((f32*)&j)[2];

	_rc3(matrix, 0, 1) = ((f32*)&j)[3] + ((f32*)&k)[0] + ((f32*)&k)[1];
	_rc3(matrix, 1, 1) = ((f32*)&h)[3] + ((f32*)&l)[0] + ((f32*)&l)[1];
	_rc3(matrix, 2, 1) = ((f32*)&i)[3] + ((f32*)&m)[0] + ((f32*)&m)[1];

	_rc3(matrix, 0, 2) = ((f32*)&m)[2] + ((f32*)&m)[3] + ((f32*)&o)[0];
	_rc3(matrix, 1, 2) = ((f32*)&k)[2] + ((f32*)&k)[3] + ((f32*)&o)[1];
	_rc3(matrix, 2, 2) = ((f32*)&l)[2] + ((f32*)&l)[3] + ((f32*)&o)[2];

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


Vec4 MulMat4Vec4(Mat4 lhs,Vec4 rhs){
	auto v = rhs.simd;

	auto a = _mm_mul_ps(lhs.simd[0], v);
	auto b = _mm_mul_ps(lhs.simd[1], v);
	auto c = _mm_mul_ps(lhs.simd[2], v);
	auto d = _mm_mul_ps(lhs.simd[3], v);

	f32 x = ((f32*)&a)[0] + ((f32*)&a)[1] + ((f32*)&a)[2] + ((f32*)&a)[3];

	f32 y = ((f32*)&b)[0] + ((f32*)&b)[1] + ((f32*)&b)[2] + ((f32*)&b)[3];

	f32 z = ((f32*)&c)[0] + ((f32*)&c)[1] + ((f32*)&c)[2] + ((f32*)&c)[3];

	f32 w = ((f32*)&d)[0] + ((f32*)&d)[1] + ((f32*)&d)[2] + ((f32*)&d)[3];

	return {x, y, z, w};
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

DualQ AddDualQ(DualQ lhs, DualQ rhs) {
	lhs.q1 = lhs.q1 + rhs.q1;
	lhs.q2 = lhs.q2 + rhs.q2;
	return lhs;
}

DualQ SubDualQ(DualQ lhs, DualQ rhs) {
	lhs.q1 = lhs.q1 - rhs.q1;
	lhs.q2 = lhs.q2 - rhs.q2;
	return lhs;
}

DualQ MulDualQ(DualQ lhs, DualQ rhs) {
	DualQ d;
	d.q1 = lhs.q1 * rhs.q1;
	d.q2 = (lhs.q2 * rhs.q1) + (lhs.q1 * rhs.q2);
	return d;
}

DualQ MulConstLDualQ(f32 lhs, DualQ rhs) {
	rhs.q1 = rhs.q1 * lhs;
	rhs.q2 = rhs.q2 * lhs;
	return rhs;
}

DualQ MulConstRDualQ(DualQ lhs, f32 rhs) { return MulConstLDualQ(rhs, lhs); }

// MARK: SPECIAL MATH OPS

Mat4 SchurMat4(Mat4 a, Mat4 b) {
	Mat4 matrix;

	for (u32 i = 0; i < 4; i++) {
		matrix.simd[i] = _mm_mul_ps(a.simd[i], b.simd[i]);
	}

	return matrix;
}

Mat4 TransposeMat4(Mat4 matrix) {
	Mat4 store_matrix;

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

	_mm_storeu_ps(&_ac4(store_matrix, 0, 0), row0);

	// this is swapped
	row1 = _mm_shuffle_ps(row1, row1, _MM_SHUFFLE(1, 0, 3, 2));
	_mm_storeu_ps(&_ac4(store_matrix, 0, 1), row1);

	_mm_storeu_ps(&_ac4(store_matrix, 0, 2), row2);

	// this is swapped

	row3 = _mm_shuffle_ps(row3, row3, _MM_SHUFFLE(1, 0, 3, 2));
	_mm_storeu_ps(&_ac4(store_matrix, 0, 3), row3);

	return store_matrix;
}

Mat4 InverseMat4(Mat4 matrix) {
#if 0
	/*
	  Computes the inverse matrix using the adjugate formula
	  inverse A = 1/(det(A)) x adj(A)

	  where det is the determinant of the matrix and adj the adjoint matrix
	  of A. where 1 <= i <= n and 1 <= j <= n of an n by n matrix and (i,j)
	  corresponds to a value of matrix A adjoint_matrix of elements =
	  -(-1)^(i * j) * det(MinorMat(matrix,i,j)) adj(A) =
	  Transpose(adjoint_matrix)
	*/

	// we'll use these to get the det. Calling generic det will compute the
	// first row minor matrix det twice
	f32 first_row[] = {
	    matrix.container[_ac4(0, 0)], matrix.container[_ac4(1, 0)],
	    matrix.container[_ac4(2, 0)], matrix.container[_ac4(3, 0)]};

	// get the adjoint matrix, create matrices of cofactors

	Mat4 adj_matrix = {};

	for (u32 y = 0; y < 4; y++) {
		for (u32 x = 0; x < 4; x++) {
			Mat3 minormatrix = {};

			GetMinorMat((f32*)&matrix.container[0], 4, x, y,
				    (f32*)&minormatrix);

			adj_matrix.container[_ac4(x, y)] =
			    GenericGetDeterminant((f32*)&minormatrix, 3);
		}
	}

	Mat4 checkerboard_matrix = {
	    1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, -1, -1, 1, -1, 1,
	};

	adj_matrix = SchurMat4(adj_matrix, checkerboard_matrix);

	f32 det = 0;

	for (u32 i = 0; i < 4; i++) {
		det += first_row[i] * adj_matrix.container[_ac4(i, 0)];
	}

	adj_matrix = (1.0f / det) * TransposeMat4(adj_matrix);

	return adj_matrix;

#else

	f32 x = _ac4(matrix, 0, 3);
	f32 y = _ac4(matrix, 1, 3);
	f32 z = _ac4(matrix, 2, 3);
	f32 w = _ac4(matrix, 3, 3);

	Vec3 a = {_ac4(matrix, 0, 0), _ac4(matrix, 0, 1), _ac4(matrix, 0, 2)};
	Vec3 b = {_ac4(matrix, 1, 0), _ac4(matrix, 1, 1), _ac4(matrix, 1, 2)};
	Vec3 c = {_ac4(matrix, 2, 0), _ac4(matrix, 2, 1), _ac4(matrix, 2, 2)};
	Vec3 d = {_ac4(matrix, 3, 0), _ac4(matrix, 3, 1), _ac4(matrix, 3, 2)};

	auto s = CrossVec3(a, b);
	auto t = CrossVec3(c, d);
	auto u = (y * a) - (x * b);
	auto v = (w * c) - (z * d);

	f32 det = (DotVec3(s, v)) + (DotVec3(t, u));

	auto r0 = CrossVec3(b, v) + (y * t);
	auto r1 = CrossVec3(v, a) - (x * t);
	auto r2 = CrossVec3(d, u) + (w * s);
	auto r3 = CrossVec3(u, c) - (z * s);

	Mat4 mat = {
	    r0.x, r0.y, r0.z, -DotVec3(b, t), r1.x, r1.y, r1.z, DotVec3(a, t),
	    r2.x, r2.y, r2.z, -DotVec3(d, s), r3.x, r3.y, r3.z, DotVec3(c, s),
	};

	return (1.0f / det) * mat;
#endif
}

Mat4 InverseTransform(Mat4 matrix) {
	auto m = InverseMat3(Mat4ToMat3(matrix));
	auto r = Mat3ToMat4(m);

	Vec3 t = {_ac4(matrix, 3, 0), _ac4(matrix, 3, 1), _ac4(matrix, 3, 2)};

	m = m * -1.0f;
	t = m * t;

	_ac4(r, 3, 0) = t.x;
	_ac4(r, 3, 1) = t.y;
	_ac4(r, 3, 2) = t.z;

	return r;
}

Mat4 OuterMat4(Vec4 a_1, Vec4 b) {
	Mat4 a = {};

	a.simd[0] = _mm_set1_ps(a_1.x);
	a.simd[1] = _mm_set1_ps(a_1.y);
	a.simd[2] = _mm_set1_ps(a_1.z);
	a.simd[3] = _mm_set1_ps(a_1.w);

	a.simd[0] = _mm_mul_ps(a.simd[0], b.simd);
	a.simd[1] = _mm_mul_ps(a.simd[1], b.simd);
	a.simd[2] = _mm_mul_ps(a.simd[2], b.simd);
	a.simd[3] = _mm_mul_ps(a.simd[3], b.simd);

	return a;
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
#if 0
	// see Inverse(Mat4)

	f32 first_row[] = {_ac3(matrix,0, 0),
			   _ac3(matrix,1, 0),
			   _ac3(matrix,2, 0)

	};

	Mat3 adj_matrix = {};

	for (u32 y = 0; y < 3; y++) {
		for (u32 x = 0; x < 3; x++) {
			Mat2 minormatrix = {};

			GetMinorMat((f32*)&matrix[0], 3, x, y,
				    (f32*)&minormatrix);

			f32 a = _rc2(minormatrix,0, 0);
			f32 b = _rc2(minormatrix,1, 0);
			f32 c = _rc2(minormatrix,0, 1);
			f32 d = _rc2(minormatrix,1, 1);

			_ac3(adj_matrix,x, y) = ((a * d) - (b * c));
		}
	}

	Mat3 checkerboard_matrix = {
	    1, -1, 1, -1, 1, -1, 1, -1, 1,
	};

	adj_matrix = SchurMat3(adj_matrix, checkerboard_matrix);

	f32 det = 0;

	for (u32 i = 0; i < 3; i++) {
		det += first_row[i] * _ac3(adj_matrix,i, 0);
	}

	adj_matrix = (1.0f / det) * TransposeMat3(adj_matrix);

	return adj_matrix;
#else
	Vec3 a = {_ac3(matrix, 0, 0), _ac3(matrix, 0, 1), _ac3(matrix, 0, 2)};
	Vec3 b = {_ac3(matrix, 1, 0), _ac3(matrix, 1, 1), _ac3(matrix, 1, 2)};
	Vec3 c = {_ac3(matrix, 2, 0), _ac3(matrix, 2, 1), _ac3(matrix, 2, 2)};

	f32 det = ScalTripleVec3(a, b, c);

	auto r0 = CrossVec3(b, c);
	auto r1 = CrossVec3(c, a);
	auto r2 = CrossVec3(a, b);

	Mat3 mat = {
	    r0.x, r0.y, r0.z, r1.x, r1.y, r1.z, r2.x, r2.y, r2.z,
	};

	return (1.0f / det) * mat;
#endif
}

Mat3 OuterMat3(Vec3 a_1, Vec3 b) {
	Mat3 a = {};

	a.simd[0] = _mm_set1_ps(a_1.x);
	a.simd[1] = _mm_set1_ps(a_1.y);

	_ac3(a, 0, 1) = a_1.y;

	_ac3(a, 0, 2) = a_1.z;
	_ac3(a, 1, 2) = a_1.z;
	a.k = a_1.z;

	__m128 k = {b.x, b.y, b.z, b.x};
	auto d = _mm_shuffle_ps(k, k, _MM_SHUFFLE(1, 0, 2, 1));

	a.simd[0] = _mm_mul_ps(a.simd[0], k);
	a.simd[1] = _mm_mul_ps(a.simd[1], d);
	a.k *= b.z;

	return a;
}

Mat2 TransposeMat2(Mat2 matrix) {
	matrix.simd =
	    _mm_shuffle_ps(matrix.simd, matrix.simd, _MM_SHUFFLE(3, 1, 2, 0));

	return matrix;
}

Mat2 InverseMat2(Mat2 matrix) {
	f32 a = _rc2(matrix, 0, 0);
	f32 b = _rc2(matrix, 1, 0);
	f32 c = _rc2(matrix, 0, 1);
	f32 d = _rc2(matrix, 1, 1);

	f32 k = 1.0f / ((a * d) - (b * c));

	_rc2(matrix, 0, 0) = d;
	_rc2(matrix, 1, 0) = -b;
	_rc2(matrix, 0, 1) = -c;
	_rc2(matrix, 1, 1) = a;

	return matrix * k;
}

f32 MagnitudeVec4(Vec4 vec) {
	// m = sqrtf(x^2 + y^2 + z^2)

	f32 res = SquaredVec4(vec);
	res = sqrtf(res);

	return res;
}

f32 SquaredVec4(Vec4 vec) { return DotVec4(vec, vec); }

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

Vec4 LerpVec4(Vec4 a, Vec4 b, f32 step) {
	Vec4 v = {_intrin_fmadd_ps(_mm_sub_ps(b.simd, a.simd),
				   _mm_set1_ps(step), a.simd)};
	return v;
}

f32 MagnitudeVec3(Vec3 vec) {
	// m = sqrtf(x^2 * y^2 * z^2)

	f32 res = SquaredVec3(vec);

	res = sqrtf(res);

	return res;
}

f32 SquaredVec3(Vec3 vec) { return DotVec3(vec, vec); }

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
Vec3 VecTripleVec3(Vec3 a, Vec3 b, Vec3 c) {
	return (b * DotVec3(a, c)) - (c * DotVec3(a, b));
}

f32 ScalTripleVec3(Vec3 a, Vec3 b, Vec3 c) {
	// NOTE: a b c = c a b = b c a
	// c b a = b a c = a c b = - (a b c)
	// Think about it of a triangle in clockwise points a b c
	// When winding clockwise, it is the same
	// When winding counter clockwise, it is the negative of the
	// normal value
	return DotVec3(CrossVec3(a, b), c);
}

f32 CompVec3(Vec3 a, Vec3 b) { return DotVec3(a, NormalizeVec3(b)); }

Vec3 ProjectOntoVec3(Vec3 a, Vec3 b) {
	return CompVec3(a, b) * NormalizeVec3(b);
}

f32 DistPointToPlane(Plane plane, Vec3 point) {
	return DotVec4(plane.vec, Vec3ToVec4(point));
}

Vec3 ReflectPointPlaneVec3(Plane plane, Vec3 point) {
	f32 dist = DistPointToPlane(plane, point);

	return point - (dist * plane.norm * 2.0f);
}

Vec3 ProjectVec3OntoPlane(Vec3 vec, Plane plane) {
	auto dir = plane.norm * plane.d;
	return RejectVec3(vec, plane.norm) - dir;
}

Vec3 RejectVec3(Vec3 a, Vec3 b) { return a - ProjectOntoVec3(a, b); }

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
#if 0
	_breakpoint();
	vec.x = 2;
	vec.y = 3;
	vec.z = 4;
#endif

	auto rot_matrix = RotateMat3(rotation);

	return rot_matrix * vec;
}
Vec3 RotateAxisVec3(Vec3 vec, Vec3 axis, f32 angle) {
	axis = NormalizeVec3(axis);
	f32 s = sinf(angle);
	f32 c = cosf(angle);

	auto ret = (c * vec) + (ProjectOntoVec3(vec, axis) * (1.0f - c)) +
		   (s * CrossVec3(axis, vec));

	return ret;
}

f32 Cosf(Vec3 vec1, Vec3 vec2) {
	return DotVec3(vec1, vec2) /
	       (MagnitudeVec3(vec1) * MagnitudeVec3(vec2));
}

Vec3 ReflectVec3(Vec3 vec, Vec3 normal) {
	return SubVec3(vec, MulConstRVec3(ProjectOntoVec3(vec, normal), 2.0f));
}

Vec3 InvolVec3(Vec3 vec, Vec3 normal) {
	return ProjectOntoVec3(vec, normal) - RejectVec3(vec, normal);
}

f32 MagnitudeVec2(Vec2 a) {
	a.x = a.x * a.x;
	a.y = a.y * a.y;

	return sqrtf(a.x + a.y);
}

f32 SquaredVec2(Vec2 vec) { return DotVec2(vec, vec); }

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
	f32 c = cosf(rotation);
	f32 s = sinf(rotation);

	Vec2 v = {};

	v.x = (vec.x * c) - (vec.y * s);
	v.y = (vec.x * s) + (vec.y * c);

	return v;
}

Quat InverseQuat(Quat q) {
	/*
	 The Inverse of the quaternion is defined as the
	 ConjugateQuat(q)/(Magnitude(q))^2 However, as the quaternions used are
	 unit quaternions, the Magnitude(q) is always 1. Just insert the
	 conjugate here.

MARK: Should we handle non unit quaternions??
	 */

	return ConjugateQuat(q);
}

Quat NLerpQuat(Quat a, Quat b, f32 step) {
	b = Neighbourhood(a, b);

	Quat q = LerpQuat(a, b, step);

	q = NormalizeQuat(q);

	return q;
}

// TODO: Look at this again
Quat SLerpQuat(Quat a, Quat b, f32 step) {
	// MARK: if these two are normalized, they would already
	// be clamped between -1 and 1

	b = Neighbourhood(a, b);

	f32 dot = _clampf(DotQuat(a, b), -1.0f, 1.0f);

	f32 angle = acosf(dot) * step;

	Quat n = NormalizeQuat(b - (a * dot));

	return (n * sinf(angle)) + (a * cosf(angle));
}

Quat ConjugateQuat(Quat q) {
	q.w *= -1.0f;
	return q * -1.0f;
}

DualQ NormalizeDualQ(DualQ d) {
	f32 magnitude = MagnitudeQuat(d.q1);

	_kill("dq normalize error\n", magnitude <= 0.000001f);

	d.q1 = d.q1 / magnitude;
	d.q2 = d.q2 / magnitude;

	return d;
}

struct ScrewCoord {
	f32 angle;
	f32 d;
	Vec3 l;
	Vec3 m;
};

ScrewCoord ToScrew(DualQ dq) {
	Vec3 v_r = dq.q1.v;
	Vec3 v_d = dq.q2.v;

	f32 k = 1.0f / MagnitudeVec3(v_r);

	f32 a = 2.0f * acosf(dq.q1.w);
	f32 d = -2.0f * dq.q2.w * k;
	auto l = v_r * k;
	auto m = (v_d - (l * (d * dq.q1.w * 0.5f))) * k;

	return {a, d, l, m};
}

Quat ToQuat(Vec3 v, f32 w) {
	Quat q = {};

	q.v = v;
	q.r = w;

	return q;
}

DualQ ToDQ(ScrewCoord s) {
	f32 sn = sinf(s.angle * 0.5f);
	f32 cs = cosf(s.angle * 0.5f);

	f32 w_r = cs;
	Vec3 v_r = s.l * sn;

	f32 w_d = (s.d * -0.5f) * sn;
	Vec3 v_d = (sn * s.m) + (s.d * 0.5f * cs * s.l);

	return {ToQuat(v_r, w_r), ToQuat(v_d, w_d)};
}

DualQ PowerDQ(DualQ dq, f32 t) {
	auto s = ToScrew(dq);

	s.angle *= t;
	s.d *= t;

	return ToDQ(s);
}

DualQ ConjugateDualQ(DualQ dq) {
	dq.q1 = ConjugateQuat(dq.q1);
	dq.q2 = ConjugateQuat(dq.q2);

	return dq;
}

// TODO: test this using a visual aid
DualQ ScLerp(DualQ a, DualQ b, f32 step) {
	b = Neighbourhood(a.q1, b);

	auto k = ConjugateDualQ(a) * b;
	return NormalizeDualQ(a * PowerDQ(k, step));
}

DualQ LerpDualQ(DualQ a, DualQ b, f32 step) {
	b = Neighbourhood(a.q1, b);
	return NormalizeDualQ(a + ((b - a) * step));
}

// TODO:  Test these

// NOTE: these are the general functions that just return the t
// values
f32 IntersectLine3t(Line3 a, Line3 b) {
	auto cross_ab = NormalizeVec3(CrossVec3(a.dir, b.dir));
	auto cross_diff = NormalizeVec3(CrossVec3(b.pos - a.pos, b.dir));

	auto dot = DotVec3(cross_ab, cross_diff);

	f32 t = 0.0f;

	// the two terms are now
	// t * cross(a.dir,b.dir) = cross((b.pos - a.pos),b.dir)
	// to get t, we either get the magnitude of both sides
	// and div LHS by RHS, or
	// do what we do below and look for the first non-zero term
	// and get the ratio of the LHS and RHS of the terms instead
	for (u32 i = 0; i < 3; i++) {
		m32 fi1;

		fi1.f = cross_ab.container[i];

		if (fi1.u) {
			t = cross_diff.container[i] / cross_ab.container[i];
			break;
		}
	}

	return t;
}

f32 IntersectLine3Planet(Line3 line, Plane plane) {
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

	auto a = DotVec4(plane.vec, Vec3ToVec4(line.pos));
	auto b = DotVec4(plane.vec, Vec3ToDir4(line.dir));

	f32 t = -1.0f * (a / b);
	return t;
}

b32 IsPerpendicularLine3Plane(Line3 a, Plane b) {
	// Assuming line.dir and plane.norm and perpendicular to each
	// other, the only time they intersect is if the line is on
	// the plane. Thus any point on the line will be perpendicular
	// to the plane

	m32 fi;

	auto plane_pos = GetPlanePos(b);
	auto dir = NormalizeVec3(a.pos - plane_pos);

	fi.f = DotVec3(dir, b.norm);
	return !fi.u;
}

f32 DistPointRay3ToLine3(Line3 line, Vec3 point) {
	auto c = CrossVec3(point - line.pos, line.dir);

	return sqrtf(SquaredVec3(c));
}

f32 DistLineRay3ToLine3(Line3 a, Line3 b) {
	if (IntersectLine3(a, b)) {
		return 0.0f;
	}

	f32 d = DotVec3(a.dir, b.dir);

	f32 v1_sq = SquaredVec3(a.dir);
	f32 v2_sq = SquaredVec3(b.dir);

	f32 det = (d * d) - (v1_sq * v2_sq);

	if (det > 0.0f && det < _f32_error_offset) {
		// the lines are parallel
		auto k = CrossVec3((b.pos - a.pos), a.dir);
		return sqrtf(SquaredVec3(k));
	}

	__m128 res = {-1.0f * v2_sq, d, -1.0f * d, v1_sq};

	f32 i = DotVec3((b.pos - a.pos), a.dir);
	f32 j = DotVec3((b.pos - a.pos), b.dir);

	__m128 k = {i, j, i, j};

	res = _mm_mul_ps(_mm_mul_ps(res, k), _mm_set1_ps(1.0f / d));

	auto s = (f32*)&res;

	f32 t0 = s[0] + s[1];  
	f32 t1 = s[2] + s[3]; 

	auto dir = (a.pos + (t0 * a.dir)) - (b.pos + (t1 * b.dir));

	return MagnitudeVec3(dir);
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

	if (InternalIsParallel(a, b)) {
		return false;
	}

	auto cross_ab = NormalizeVec3(CrossVec3(a.dir, b.dir));
	auto cross_diff = NormalizeVec3(CrossVec3(b.pos - a.pos, b.dir));
	auto dot = DotVec3(cross_ab, cross_diff);

	return (u32)(fabsf(dot) + _f32_error_offset);
}

b32 IntersectOutLine3(Line3 a, Line3 b, Point3* out_point) {
	if (IntersectLine3(a, b)) {
		return false;
	}

	f32 t = IntersectLine3t(a, b);

	*out_point = (a.dir * t) + a.pos;

	return true;
}

b32 TypedIntersectLine3(Line3 a, Line3 b) {
	b32 is_intersect = IntersectLine3(a, b);
	b32 is_parallel = InternalIsParallel(a, b);
	b32 is_inline = InternalIsInline(a, b);

	if (is_inline & is_parallel) {
		return INTERSECT_INFINITE;
	}

	return is_intersect;
}

b32 IntersectLine3Plane(Line3 a, Plane b) {
	m32 fi1;

	// if they are perpendicular, f is 0 and they do not intersect
	fi1.f = DotVec3(a.dir, b.norm);

	return fi1.u != 0;
}

b32 IntersectOutLine3Plane(Line3 a, Plane b, Point3* out_point) {
	if (!IntersectLine3Plane(a, b)) {
		return false;
	}

	f32 t = IntersectLine3Planet(a, b);

	*out_point = a.pos + (t * a.dir);

	return true;
}

b32 TypedIntersectLine3Plane(Line3 a, Plane b) {
	/*
	  if the dir of a line is perpendicular to a plane normal, it will never
	  intersect unless the line is on the plane
	*/

	auto is_intersect = IntersectLine3Plane(a, b);
	auto is_perpendicular = IsPerpendicularLine3Plane(a, b);

	if (!is_intersect && is_perpendicular) {
		return INTERSECT_INFINITE;
	}

	return is_intersect;
}

b32 IntersectLine2(Line2 a, Line2 b) { return !InternalIsParallel(a, b); }

b32 IntersectOutLine2(Line2 a, Line2 b, Point2* out_point) {
	if (!IntersectLine2(a, b)) {
		return false;
	}

	// this uses y = mx + c (TODO: test this)

	auto m1 = a.dir.y / a.dir.x;
	auto m2 = b.dir.y / b.dir.x;

	auto c1 = a.pos.y - (a.pos.x * m1);
	auto c2 = b.pos.y - (b.pos.x * m2);

	auto x = (-1.0f * (c1 - c2)) / (m1 - m2);
	auto y = (m1 * x) + c1;

	*out_point = {x, y};

	return true;
}

b32 TypedIntersectLine2(Line2 a, Line2 b) {
	b32 is_parallel = InternalIsParallel(a, b);
	b32 is_intersect = IntersectLine2(a, b);
	b32 is_inline = InternalIsInline(a, b);

	if (is_parallel && is_inline) {
		return INTERSECT_INFINITE;
	}

	return is_intersect;
}

b32 IntersectRay3(Ray3 a, Ray3 b) {
	if (InternalIsParallel(a, b)) {
		return false;
	}

	f32 t = IntersectLine3t(Ray3ToLine3(a), Ray3ToLine3(b));

	// We need the k term and make sure that that is positive too

	Point3 p = (a.dir * t) + a.pos;
	f32 k_value = DotVec3(p - b.pos, b.dir);

	if (t < 0.0f || k_value < 0.0f) {
		return false;
	}

	return true;
}

b32 IntersectOutRay3(Ray3 a, Ray3 b, Point3* out_point) {
	if (InternalIsParallel(a, b)) {
		return false;
	}

	f32 t = IntersectLine3t(Ray3ToLine3(a), Ray3ToLine3(b));

	// We need the k term and make sure that that is positive too

	Point3 p = (a.dir * t) + a.pos;
	f32 k_value = DotVec3(p - b.pos, b.dir);

	if (t < 0.0f || k_value < 0.0f) {
		return false;
	}

	*out_point = p;

	return true;
}

b32 TypedIntersectRay3(Ray3 a, Ray3 b) {
	b32 is_intersect = IntersectRay3(a, b);
	b32 is_parallel = InternalIsParallel(a, b);
	b32 is_inline = InternalIsInline(a, b);

	if (is_inline && is_parallel) {
		return INTERSECT_INFINITE;
	}

	return is_intersect;
}

b32 IntersectRay3Plane(Ray3 a, Plane b) {
	if (!IntersectLine3Plane(Ray3ToLine3(a), b)) {
		return false;
	}

	f32 t = IntersectLine3Planet(Ray3ToLine3(a), b);
	return t > _f32_error_offset;
}

b32 IntersectOutRay3Plane(Ray3 a, Plane b, Point3* out_point) {
	if (!IntersectLine3Plane(Ray3ToLine3(a), b)) {
		return false;
	}

	f32 t = IntersectLine3Planet(Ray3ToLine3(a), b);

	if (t >= _f32_error_offset) {
		*out_point = a.pos + (t * a.dir);
		return true;
	}

	return false;
}

b32 TypedIntersectRay3Plane(Ray3 a, Plane b) {
	auto is_intersect = IntersectRay3Plane(a, b);
	auto is_perpendicular = IsPerpendicularLine3Plane(Ray3ToLine3(a), b);

	if (!is_intersect && is_perpendicular) {
		return INTERSECT_INFINITE;
	}

	return is_intersect;
}

b32 IntersectClosestOutLine3Sphere(Line3 line, Sphere sphere, Point3* point) {
	Vec3 line_to_sphere = SubVec3(sphere.pos, line.pos);

	f32 a = DotVec3(line.dir, line.dir);
	f32 b = -2.0f * DotVec3(line.dir, line_to_sphere);
	f32 c = DotVec3(line_to_sphere, line_to_sphere) -
		(sphere.radius * sphere.radius);
	f32 discriminant = (b * b) - (4.0f * a * c);

	if (discriminant < 0.0f) {
		return 0;
	}

	f32 root = sqrtf(discriminant);
	f32 t0 = ((-1.0f * b) + root) / (2.0f * a);
	f32 t1 = ((-1.0f * b) - root) / (2.0f * a);

	f32 abs_t0 = fabsf(t0);
	f32 abs_t1 = fabsf(t1);

	f32 error_margin = _f32_error_offset;

	b32 line_condition =
	    (abs_t0 < _f32_error_offset) || (abs_t1 < _f32_error_offset);

	if (line_condition) {
		return 0;
	}

	*point = 
		abs_t0 < abs_t1 ? AddVec3((MulConstRVec3(line.dir, t0)), line.pos) : AddVec3((MulConstRVec3(line.dir, t1)), line.pos);
	return 1;
}

b32 IntersectClosestOutRay3Sphere(Ray3 ray, Sphere sphere, Point3* point) {
	Vec3 ray_to_sphere = SubVec3(sphere.pos, ray.pos);

	f32 a = SquaredVec3(ray.dir);
	f32 b = -2.0f * DotVec3(ray.dir, ray_to_sphere);
	f32 c = SquaredVec3(ray_to_sphere) - (sphere.radius * sphere.radius);
	f32 discriminant = (b * b) - (4.0f * a * c);

	if (discriminant < 0.0f) {
		return 0;
	}

	f32 root = sqrtf(discriminant);
	f32 t0 = ((-1.0f * b) + root) / (2.0f * a);
	f32 t1 = ((-1.0f * b) - root) / (2.0f * a);

	// MARK: We do not support intersections from the inside
	if (t0 <= _f32_error_offset || t1 <= _f32_error_offset) {
		return 0;
	}

	if (t0 < t1) {
		*point = AddVec3((MulConstRVec3(ray.dir, t0)), ray.pos);
	} else {
		*point = AddVec3((MulConstRVec3(ray.dir, t1)), ray.pos);
	}

	return 1;
}

b32 IntersectRay2(Ray2 a, Ray2 b) {
	Point2 p = {};

	if (!IntersectOutLine2(Ray2ToLine2(a), Ray2ToLine2(b), &p)) {
		return false;
	}

	Vec2 p_to_a = NormalizeVec2(a.pos - p);
	Vec2 p_to_b = NormalizeVec2(b.pos - p);

	b32 is_a_valid = (u32)(DotVec2(a.dir, p_to_a) + _f32_error_offset);

	b32 is_b_valid = (u32)(DotVec2(b.dir, p_to_b) + _f32_error_offset);

	return is_a_valid && is_b_valid;
}

b32 IntersectOutRay2(Ray2 a, Ray2 b, Point2* out_point) {
	Point2 p = {};

	if (!IntersectOutLine2(Ray2ToLine2(a), Ray2ToLine2(b), &p)) {
		return false;
	}

	Vec2 p_to_a = NormalizeVec2(a.pos - p);
	Vec2 p_to_b = NormalizeVec2(b.pos - p);

	b32 is_a_valid = (u32)(DotVec2(a.dir, p_to_a) + _f32_error_offset);

	b32 is_b_valid = (u32)(DotVec2(b.dir, p_to_b) + _f32_error_offset);

	if (is_a_valid && is_b_valid) {
		*out_point = p;
		return true;
	}

	return false;
}

b32 TypedIntersectRay2(Ray2 a, Ray2 b) {
	b32 is_parallel = InternalIsParallel(a, b);
	b32 is_intersect = IntersectRay2(a, b);
	b32 is_inline = InternalIsInline(a, b);

	if (is_inline && is_parallel) {
		return INTERSECT_INFINITE;
	}

	return is_intersect;
}

b32 Intersect3Planes(Plane a, Plane b, Plane c, Vec3* out) {
	f32 k = ScalTripleVec3(a.norm, b.norm, c.norm);

	if (CmpErrorZero(k)) {
		return false;
	}

	k = 1.0f / k;

	auto d = CrossVec3(c.norm, b.norm) * a.d;
	auto e = CrossVec3(a.norm, c.norm) * b.d;
	auto f = CrossVec3(b.norm, a.norm) * c.d;

	*out = (d + e + f) * k;

	return true;
}

b32 Intersect2Planes(Plane a, Plane b, Line3* out) {
	auto dir = CrossVec3(a.norm, b.norm);
	auto dir_sq = SquaredVec3(dir);

	if (CmpErrorZero(dir_sq)) {
		return false;
	}

	auto c = CrossVec3(dir, b.norm) * a.d;
	auto d = CrossVec3(a.norm, dir) * a.d;

	auto pos = (c + d) / dir_sq;

	auto l = ConstructLine3(pos, dir);

	(Vec3) out->pos = l.pos;
	(Vec3) out->dir = l.dir;

	return true;
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

	_rc4(matrix, 0, 0) = a;
	_rc4(matrix, 1, 1) = b;
	_rc4(matrix, 2, 2) = c;
	_rc4(matrix, 3, 2) = d;
	_rc4(matrix, 2, 3) = -1.0f;

	return matrix;
}

Mat4 WorldMat4M(Mat4 position, Mat4 rotation, Mat4 scale) {
	return position * rotation * scale;
}

Mat4 WorldMat4V(Vec3 position, Vec3 rotation, Vec3 scale) {
	Mat4 matrix;

	Mat4 position_matrix4 = TranslateMat4(position);

	Mat4 scale_matrix4 = Mat3ToMat4(ScaleMat3(scale));

	Mat4 rotation_matrix4 = Mat3ToMat4(RotateMat3(rotation));

	matrix = WorldMat4M(position_matrix4, rotation_matrix4, scale_matrix4);

	return matrix;
}

Mat4 WorldMat4Q(Vec3 position, Quat rotation, Vec3 scale) {
	Mat4 matrix;

	Mat4 position_matrix4 = TranslateMat4(position);

	Mat4 scale_matrix4 = Mat3ToMat4(ScaleMat3(scale));

	Mat4 rotation_matrix4 = QuatToMat4(rotation);

	matrix = WorldMat4M(position_matrix4, rotation_matrix4, scale_matrix4);

	return matrix;
}

Mat4 TransformRelativeMat4(Mat4 transform, Vec3 new_origin) {
	// TODO: we can optimize this by just doing the math
	auto p = new_origin;
	auto q = -1.0f * p;

	Mat4 t0 = {
	    1, 0, 0, p.x, 0, 1, 0, p.y, 0, 0, 1, p.z, 0, 0, 0, 1,
	};

	Mat4 t1 = {
	    1, 0, 0, q.x, 0, 1, 0, q.y, 0, 0, 1, q.z, 0, 0, 0, 1,
	};

	return t1 * transform * t0;
}

Mat3 ConstructCrossMat3(Vec3 vec) {
	Mat3 mat = {
	    0, -vec.z, vec.y, vec.z, 0, -vec.x, -vec.y, vec.x, 0,

	};

	return mat;
}

Mat3 ConstructProjectOntoMat3(Vec3 vec) {
	f32 k = 1.0f / (SquaredVec3(vec));
	return k * OuterMat3(vec, vec);
}

Mat3 ConstructRejectMat3(Vec3 v) {
	f32 k = 1.0f / (SquaredVec3(v));

	Mat3 mat = {
	    (v.y * v.y) + (v.z * v.z), -(v.x * v.y), -(v.x * v.z), -(v.x * v.y),
	    (v.x * v.x) + (v.z * v.z), -(v.y * v.z), -(v.x * v.z), -(v.y * v.z),
	    (v.x * v.x) + (v.y * v.y),
	};

	return mat * k;
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

Quat ConstructVecQuat(Vec3 v1, Vec3 v2) {
	v1 = NormalizeVec3(v1);
	v2 = NormalizeVec3(v2);

	f32 c = Cosf(v1, v2);

	Quat q = {};
	q.v = CrossVec3(v1, v2) * (1.0f / (2 * c));
	q.r = c;

	return q;
}

DualQ ConstructDualQM(Mat4 transform) {
	DualQ d = {};

	d.q1 = Mat4ToQuat(transform);
	Vec3 translation = Mat4ToTranslationVec(transform);

	d.q2 =
	    Quat{translation.x, translation.y, translation.z, 0} * d.q1 * 0.5f;

	return NormalizeDualQ(d);
}

DualQ ConstructDualQ(Quat rotation, Vec3 translation) {
	/*
	  real = rotation
	  dual = 0.5 * translation * rotation
	*/

	DualQ d = {};
	d.q1 = rotation;
	d.q2 =
	    Quat{translation.x, translation.y, translation.z, 0} * d.q1 * 0.5f;
	return NormalizeDualQ(d);
}

Vec3 GetSphereNormalVec3(Sphere sphere, Point3 point_on_sphere) {
	Vec3 normal = SubVec3(point_on_sphere, sphere.pos);

#if 0
	{
		f32 magnitude = MagnitudeVec3(normal);
		b32 condition =
		    (sphere.radius > magnitude || sphere.radius < magnitude);

		if(condition){
			printf("ERROR: %f\n",(f64)magnitude);
		}
		//_kill("point is not on sphere\n", condition);
	}
#endif

	return NormalizeVec3(normal);
}

// MARK: deconstrutors

void DeconstructQuat(Quat quaternion, Vec3* vector, f32* angle) {
	f32 anglew = acosf(quaternion.w) * 2.0f;

	f32 scale = sinf(anglew * 0.5f);

	// we should handle the case scale == 0
	if (scale == 0.0f) {
		*vector = Vec3{1, 0, 0};
		*angle = 0;
		return;
	}
	vector->x = quaternion.x / scale;
	vector->y = quaternion.y / scale;
	vector->z = quaternion.z / scale;

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
	printf("%f   %f   %f   %f\n", (f64)vec.x, (f64)vec.y, (f64)vec.z,
	       (f64)vec.w);
}

void PrintDualQ(DualQ d) {
	printf("%f   %f   %f   %f | %f %f %f %f\n", (f64)d.q1.x, (f64)d.q1.y,
	       (f64)d.q1.z, (f64)d.q1.w, (f64)d.q2.x, (f64)d.q2.y, (f64)d.q2.z,
	       (f64)d.q2.w);
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

Mat2 operator/(Mat2 lhs, Mat2 rhs) { return DivMat2(lhs, rhs); }

Vec4 operator+(Vec4 lhs, Vec4 rhs) { return AddVec4(lhs, rhs); }

Vec4 operator-(Vec4 lhs, Vec4 rhs) { return SubVec4(lhs, rhs); }

Vec4 operator*(f32 lhs, Vec4 rhs) { return MulConstLVec4(lhs, rhs); }

Vec4 operator*(Vec4 lhs, f32 rhs) { return rhs * lhs; }

Vec4 operator/(Vec4 lhs, f32 rhs) { return DivConstRVec4(lhs, rhs); }

Vec4 operator*(Mat4 lhs, Vec4 rhs){ return MulMat4Vec4(lhs,rhs); };

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

DualQ operator+(DualQ lhs, DualQ rhs) { return AddDualQ(lhs, rhs); }

DualQ operator-(DualQ lhs, DualQ rhs) { return SubDualQ(lhs, rhs); }

DualQ operator*(DualQ lhs, DualQ rhs) { return MulDualQ(lhs, rhs); }

DualQ operator*(f32 lhs, DualQ rhs) { return MulConstLDualQ(lhs, rhs); }

DualQ operator*(DualQ lhs, f32 rhs) { return rhs * lhs; }
