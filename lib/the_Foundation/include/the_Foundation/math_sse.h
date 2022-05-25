#pragma once

/** @file math_sse.h  Vector math using SSE.

@authors Copyright (c) 2017 Jaakko Keränen <jaakko.keranen@iki.fi>

@par License

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

<small>THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.</small>
*/

#include "math.h"

#include <math.h>
#include <smmintrin.h> // SSE 4.1

iDeclareType(Float4)
iDeclareType(Float3)

struct Impl_Float4 {
    __m128 m;
};

iLocalDef iFloat4 zero_F4(void) {
    return (iFloat4){ _mm_setzero_ps() };
}

iLocalDef iFloat4 init1_F4(float x) {
    return (iFloat4){ _mm_set1_ps(x) };
}

iLocalDef iFloat4 init_F4(float x, float y, float z, float w) {
    return (iFloat4){ _mm_set_ps(z, y, x, w) };
}

iLocalDef iFloat4 initi_F4(int x, int y, int z, int w) {
    return init_F4((float) x, (float) y, (float) z, (float) w);
}

iLocalDef iFloat4 initiv_F4(const int *v) {
    return init_F4((float) v[0], (float) v[1], (float) v[2], (float) v[3]);
}

iLocalDef iFloat4 initv_F4(const float *v) {
    return (iFloat4){ _mm_loadu_ps(v) };
}

iLocalDef iFloat4 initmm_F4(__m128 m) {
    return (iFloat4){ m };
}

iLocalDef float w_F4(const iFloat4 d) {
    return _mm_cvtss_f32(d.m);
}

iLocalDef float x_F4(const iFloat4 d) {
    return _mm_cvtss_f32(_mm_shuffle_ps(d.m, d.m, _MM_SHUFFLE(1, 1, 1, 1)));
}

iLocalDef float y_F4(const iFloat4 d) {
    return _mm_cvtss_f32(_mm_shuffle_ps(d.m, d.m, _MM_SHUFFLE(2, 2, 2, 2)));
}

iLocalDef float z_F4(const iFloat4 d) {
    return _mm_cvtss_f32(_mm_shuffle_ps(d.m, d.m, _MM_SHUFFLE(3, 3, 3, 3)));
}

iLocalDef void store_F4(const iFloat4 d, float *p_out) {
    _mm_storeu_ps(p_out, _mm_shuffle_ps(d.m, d.m, _MM_SHUFFLE(0, 3, 2, 1)));
}

iLocalDef iFloatVec4 values_F4(const iFloat4 d) {
    iFloatVec4 vals;
    store_F4(d, vals.v);
    return vals;
}

#define iFloat4Shuffle3(d, X, Y, Z)     (iFloat4){ _mm_shuffle_ps((d).m, (d).m, _MM_SHUFFLE(Z, Y, X, 0)) }
#define iFloat4Bitmask(x, y, z, w)      (((union { __m128 m; uint32_t m32[4]; }){ .m32 = { z, y, x, w } }).m)
#define iFloat4High92Bits               iFloat4Bitmask(0xffffffff, 0xffffffff, 0xffffffff, 0)

iLocalDef iFloat4 xyz_F4(const iFloat4 d) {
    return (iFloat4){ _mm_and_ps(d.m, iFloat4High92Bits) };
}

iLocalDef iFloat4 yzx_F4(const iFloat4 d) {
    return iFloat4Shuffle3(d, 2, 3, 1);
}

iLocalDef iFloat4 zxy_F4(const iFloat4 d) {
    return iFloat4Shuffle3(d, 3, 1, 2);
}

iLocalDef iFloat4 xzy_F4(const iFloat4 d) {
    return iFloat4Shuffle3(d, 1, 3, 2);
}

iLocalDef void setW_F4(iFloat4 *d, float w) {
    d->m = _mm_move_ss(d->m, _mm_set_ss(w));
}

iLocalDef void setX_F4(iFloat4 *d, float x) {
    __m128 t = _mm_move_ss(d->m, _mm_set_ss(x));
    t = _mm_shuffle_ps(t, t, _MM_SHUFFLE(3, 2, 0, 0));
    d->m = _mm_move_ss(t, d->m);
}

iLocalDef void setY_F4(iFloat4 *d, float y) {
    __m128 t = _mm_move_ss(d->m, _mm_set_ss(y));
    t = _mm_shuffle_ps(t, t, _MM_SHUFFLE(3, 0, 1, 0));
    d->m = _mm_move_ss(t, d->m);
}

iLocalDef void setZ_F4(iFloat4 *d, float z) {
    __m128 t = _mm_move_ss(d->m, _mm_set_ss(z));
    t = _mm_shuffle_ps(t, t, _MM_SHUFFLE(0, 2, 1, 0));
    d->m = _mm_move_ss(t, d->m);
}

typedef iFloat4 iBool4;

iLocalDef iFloat4 add_F4    (const iFloat4 a, const iFloat4 b)  { return (iFloat4){ _mm_add_ps(a.m, b.m) }; }
iLocalDef iFloat4 addf_F4   (const iFloat4 a, const float b)    { return (iFloat4){ _mm_add_ps(a.m, _mm_set1_ps(b)) }; }
iLocalDef iFloat4 sub_F4    (const iFloat4 a, const iFloat4 b)  { return (iFloat4){ _mm_sub_ps(a.m, b.m) }; }
iLocalDef iFloat4 subf_F4   (const iFloat4 a, const float b)    { return (iFloat4){ _mm_sub_ps(a.m, _mm_set1_ps(b)) }; }
iLocalDef iFloat4 mul_F4    (const iFloat4 a, const iFloat4 b)  { return (iFloat4){ _mm_mul_ps(a.m, b.m) }; }
iLocalDef iFloat4 mulf_F4   (const iFloat4 a, const float b)    { return (iFloat4){ _mm_mul_ps(a.m, _mm_set1_ps(b)) }; }
iLocalDef iFloat4 fmul_F4   (const float a, const iFloat4 b)    { return (iFloat4){ _mm_mul_ps(_mm_set1_ps(a), b.m) }; }
iLocalDef iFloat4 div_F4    (const iFloat4 a, const iFloat4 b)  { return (iFloat4){ _mm_div_ps(a.m, b.m) }; }
iLocalDef iFloat4 divf_F4   (const iFloat4 a, const float b)    { return (iFloat4){ _mm_div_ps(a.m, _mm_set1_ps(b)) }; }
iLocalDef iFloat4 fdiv_F4   (const float a, const iFloat4 b)    { return (iFloat4){ _mm_div_ps(_mm_set1_ps(a), b.m) }; }

iLocalDef iFloat4 addv_F4   (iFloat4 *a, const iFloat4 b)       { a->m = _mm_add_ps(a->m, b.m); return *a; }
iLocalDef iFloat4 subv_F4   (iFloat4 *a, const iFloat4 b)       { a->m = _mm_sub_ps(a->m, b.m); return *a; }
iLocalDef iFloat4 mulv_F4   (iFloat4 *a, const iFloat4 b)       { a->m = _mm_mul_ps(a->m, b.m); return *a; }
iLocalDef iFloat4 mulvf_F4  (iFloat4 *a, const float b)         { a->m = _mm_mul_ps(a->m, _mm_set1_ps(b)); return *a; }
iLocalDef iFloat4 divv_F4   (iFloat4 *a, const iFloat4 b)       { a->m = _mm_div_ps(a->m, b.m); return *a; }
iLocalDef iFloat4 divvf_F4  (iFloat4 *a, const float b)         { a->m = _mm_div_ps(a->m, _mm_set1_ps(b)); return *a; }

iLocalDef iFloat4 leftv_F4(iFloat4 *a) {
    a->m = _mm_shuffle_ps(a->m, a->m, _MM_SHUFFLE(0, 3, 2, 1));
    return *a;
}

iLocalDef iFloat4 rightv_F4(iFloat4 *a) {
    a->m = _mm_shuffle_ps(a->m, a->m, _MM_SHUFFLE(2, 1, 0, 3));
    return *a;
}

iLocalDef iBool4 equal_F4   (const iFloat4 a, const iFloat4 b)  { return (iFloat4){ _mm_cmpeq_ps(a.m, b.m) }; }
iLocalDef iBool4 notEqual_F4(const iFloat4 a, const iFloat4 b)  { return (iFloat4){ _mm_cmpneq_ps(a.m, b.m) }; }
iLocalDef iBool4 less_F4    (const iFloat4 a, const iFloat4 b)  { return (iFloat4){ _mm_cmplt_ps(a.m, b.m) }; }
iLocalDef iBool4 greater_F4 (const iFloat4 a, const iFloat4 b)  { return (iFloat4){ _mm_cmpgt_ps(a.m, b.m) }; }
iLocalDef iBool4 lessEqual_F4   (const iFloat4 a, const iFloat4 b)  { return (iFloat4){ _mm_cmple_ps(a.m, b.m) }; }
iLocalDef iBool4 greaterEqual_F4(const iFloat4 a, const iFloat4 b)  { return (iFloat4){ _mm_cmpge_ps(a.m, b.m) }; }

iLocalDef iFloat4 min_F4    (const iFloat4 a, const iFloat4 b)  { return (iFloat4){ _mm_min_ps(a.m, b.m) }; }
iLocalDef iFloat4 max_F4    (const iFloat4 a, const iFloat4 b)  { return (iFloat4){ _mm_max_ps(a.m, b.m) }; }

iLocalDef iFloat4 neg_F4    (const iFloat4 a)                   { return (iFloat4){ _mm_sub_ps(_mm_setzero_ps(), a.m) }; }

#define iFloat4SignBits     iFloat4Bitmask(0x80000000, 0x80000000, 0x80000000, 0x80000000)

iLocalDef iFloat4 abs_F4(const iFloat4 a) {
    return (iFloat4){ _mm_andnot_ps(iFloat4SignBits, a.m) };
}

iLocalDef unsigned mask_F4  (const iFloat4 a)   { return _mm_movemask_ps(a.m) & 15; }
iLocalDef iBool any_Bool4   (const iBool4 a)    { return mask_F4(a) != 0; }
iLocalDef iBool all_Bool4   (const iBool4 a)    { return mask_F4(a) == 15; }

iLocalDef iFloat4 clamp_F4  (const iFloat4 t, const iFloat4 a, const iFloat4 b) { return min_F4(max_F4(t, a), b); }
iLocalDef float sum_F4      (const iFloat4 a)   { return _mm_cvtss_f32(_mm_dp_ps(a.m, _mm_set1_ps(1.f), 0xf1)); }
iLocalDef float dot_F4      (const iFloat4 a, const iFloat4 b) { return _mm_cvtss_f32(_mm_dp_ps(a.m, b.m, 0xf1)); }
iLocalDef float lengthSq_F4 (const iFloat4 a)   { return dot_F4(a, a); }
iLocalDef float length_F4   (const iFloat4 a)   { return sqrtf(lengthSq_F4(a)); }
iLocalDef iFloat4 normalize_F4(const iFloat4 a) { return mulf_F4(a, 1.f / length_F4(a)); }
iLocalDef iFloat4 sqrt_F4   (const iFloat4 a)   { return (iFloat4){ _mm_sqrt_ps(a.m) }; }

iLocalDef iFloat4 mix_F4   (const iFloat4 a, const iFloat4 b, float t) {
    return add_F4(a, mulf_F4(sub_F4(b, a), t));
}

/*-------------------------------------------------------------------------------------*/

struct Impl_Float3 {
    __m128 m;
};

iLocalDef iFloat3 zero_F3(void) {
    return (iFloat3){ _mm_setzero_ps() };
}

iLocalDef iFloat3 init1_F3(float x) {
    return (iFloat3){ _mm_set1_ps(x) };
}

iLocalDef iFloat3 init_F3(float x, float y, float z) {
    return (iFloat3){ _mm_set_ps(z, y, x, 0.f) };
}

iLocalDef iFloat3 initi_F3(int x, int y, int z) {
    return init_F3((float) x, (float) y, (float) z);
}

iLocalDef iFloat3 initv_F3(const float *v) {
    return (iFloat3){ _mm_set_ps(v[2], v[1], v[0], 0.f) };
}

iLocalDef iFloat3 initiv_F3(const int *v) {
    return init_F3((float) v[0], (float) v[1], (float) v[2]);
}

iLocalDef iFloat3 initiv2_F3(const int *v) {
    return init_F3((float) v[0], (float) v[1], 0.f);
}

iLocalDef iFloat3 initmm_F3(const __m128 m) {
    return (iFloat3){ m };
}

iLocalDef float x_F3(const iFloat3 d) {
    return _mm_cvtss_f32(_mm_shuffle_ps(d.m, d.m, _MM_SHUFFLE(1, 1, 1, 1)));
}

iLocalDef float y_F3(const iFloat3 d) {
    return _mm_cvtss_f32(_mm_shuffle_ps(d.m, d.m, _MM_SHUFFLE(2, 2, 2, 2)));
}

iLocalDef float z_F3(const iFloat3 d) {
    return _mm_cvtss_f32(_mm_shuffle_ps(d.m, d.m, _MM_SHUFFLE(3, 3, 3, 3)));
}

iLocalDef void store_F3(const iFloat3 d, float *p_out) {
    _Alignas(16) float t[4];
    _mm_store_ps(t, d.m);
    p_out[0] = t[1];
    p_out[1] = t[2];
    p_out[2] = t[3];
}

iLocalDef iFloatVec3 values_F3(const iFloat3 d) {
    iFloatVec3 vec3;
    store_F3(d, vec3.v);
    return vec3;
}

#define iFloat3Shuffle3(d, X, Y, Z)  (iFloat3){ _mm_shuffle_ps((d).m, (d).m, _MM_SHUFFLE(Z, Y, X, 0)) }

iLocalDef iFloat3 yzx_F3(const iFloat3 d) {
    return iFloat3Shuffle3(d, 2, 3, 1);
}

iLocalDef iFloat3 zxy_F3(const iFloat3 d) {
    return iFloat3Shuffle3(d, 3, 1, 2);
}

iLocalDef iFloat3 xzy_F3(const iFloat3 d) {
    return iFloat3Shuffle3(d, 1, 3, 2);
}

iLocalDef void setX_F3(iFloat3 *d, float x) {
    __m128 t = _mm_move_ss(d->m, _mm_set_ss(x));
    t = _mm_shuffle_ps(t, t, _MM_SHUFFLE(3, 2, 0, 0));
    d->m = _mm_move_ss(t, d->m);
}

iLocalDef void setY_F3(iFloat3 *d, float y) {
    __m128 t = _mm_move_ss(d->m, _mm_set_ss(y));
    t = _mm_shuffle_ps(t, t, _MM_SHUFFLE(3, 0, 1, 0));
    d->m = _mm_move_ss(t, d->m);
}

iLocalDef void setZ_F3(iFloat3 *d, float z) {
    __m128 t = _mm_move_ss(d->m, _mm_set_ss(z));
    t = _mm_shuffle_ps(t, t, _MM_SHUFFLE(0, 2, 1, 0));
    d->m = _mm_move_ss(t, d->m);
}

typedef iFloat3 iBool3;

iLocalDef iFloat3 add_F3    (const iFloat3 a, const iFloat3 b)  { return (iFloat3){ _mm_add_ps(a.m, b.m) }; }
iLocalDef iFloat3 addf_F3   (const iFloat3 a, const float b)    { return (iFloat3){ _mm_add_ps(a.m, _mm_set1_ps(b)) }; }
iLocalDef iFloat3 sub_F3    (const iFloat3 a, const iFloat3 b)  { return (iFloat3){ _mm_sub_ps(a.m, b.m) }; }
iLocalDef iFloat3 subf_F3   (const iFloat3 a, const float b)    { return (iFloat3){ _mm_sub_ps(a.m, _mm_set1_ps(b)) }; }
iLocalDef iFloat3 mul_F3    (const iFloat3 a, const iFloat3 b)  { return (iFloat3){ _mm_mul_ps(a.m, b.m) }; }
iLocalDef iFloat3 mulf_F3   (const iFloat3 a, const float b)    { return (iFloat3){ _mm_mul_ps(a.m, _mm_set1_ps(b)) }; }
iLocalDef iFloat3 fmul_F3   (const float a, const iFloat3 b)    { return (iFloat3){ _mm_mul_ps(_mm_set1_ps(a), b.m) }; }
iLocalDef iFloat3 div_F3    (const iFloat3 a, const iFloat3 b)  { return (iFloat3){ _mm_div_ps(a.m, b.m) }; }
iLocalDef iFloat3 divf_F3   (const iFloat3 a, const float b)    { return (iFloat3){ _mm_div_ps(a.m, _mm_set1_ps(b)) }; }
iLocalDef iFloat3 fdiv_F3   (const float a, const iFloat3 b)    { return (iFloat3){ _mm_div_ps(_mm_set1_ps(a), b.m) }; }

iLocalDef iFloat3 addv_F3   (iFloat3 *a, const iFloat3 b)       { a->m = _mm_add_ps(a->m, b.m); return *a; }
iLocalDef iFloat3 subv_F3   (iFloat3 *a, const iFloat3 b)       { a->m = _mm_sub_ps(a->m, b.m); return *a; }
iLocalDef iFloat3 mulv_F3   (iFloat3 *a, const iFloat3 b)       { a->m = _mm_mul_ps(a->m, b.m); return *a; }
iLocalDef iFloat3 mulvf_F3  (iFloat3 *a, const float b)         { a->m = _mm_mul_ps(a->m, _mm_set1_ps(b)); return *a; }
iLocalDef iFloat3 divv_F3   (iFloat3 *a, const iFloat3 b)       { a->m = _mm_div_ps(a->m, b.m); return *a; }
iLocalDef iFloat3 divvf_F3  (iFloat3 *a, const float b)         { a->m = _mm_div_ps(a->m, _mm_set1_ps(b)); return *a; }

iLocalDef iBool3 equal_F3   (const iFloat3 a, const iFloat3 b)  { return (iFloat3){ _mm_cmpeq_ps(a.m, b.m) }; }
iLocalDef iBool3 notEqual_F3(const iFloat3 a, const iFloat3 b)  { return (iFloat3){ _mm_cmpneq_ps(a.m, b.m) }; }
iLocalDef iBool3 less_F3    (const iFloat3 a, const iFloat3 b)  { return (iFloat3){ _mm_cmplt_ps(a.m, b.m) }; }
iLocalDef iBool3 greater_F3 (const iFloat3 a, const iFloat3 b)  { return (iFloat3){ _mm_cmpgt_ps(a.m, b.m) }; }
iLocalDef iBool3 lessEqual_F3   (const iFloat3 a, const iFloat3 b)  { return (iFloat3){ _mm_cmple_ps(a.m, b.m) }; }
iLocalDef iBool3 greaterEqual_F3(const iFloat3 a, const iFloat3 b)  { return (iFloat3){ _mm_cmpge_ps(a.m, b.m) }; }

iLocalDef iFloat3 min_F3    (const iFloat3 a, const iFloat3 b)  { return (iFloat3){ _mm_min_ps(a.m, b.m) }; }
iLocalDef iFloat3 max_F3    (const iFloat3 a, const iFloat3 b)  { return (iFloat3){ _mm_max_ps(a.m, b.m) }; }

iLocalDef iFloat3 neg_F3    (const iFloat3 a)                   { return (iFloat3){ _mm_sub_ps(_mm_setzero_ps(), a.m) }; }

iLocalDef iFloat3 abs_F3(const iFloat3 a) {
    return (iFloat3){ _mm_andnot_ps(iFloat4SignBits, a.m) };
}

iLocalDef iFloat3 cross_F3(const iFloat3 a, const iFloat3 b) {
    return zxy_F3(sub_F3(mul_F3(zxy_F3(a), b),
                         mul_F3(a, zxy_F3(b))));
}

iLocalDef unsigned mask_F3  (const iFloat3 a)   { return _mm_movemask_ps(a.m) & (8|4|2); }
iLocalDef iBool any_Bool3   (const iBool3 a)    { return mask_F3(a) != 0; }
iLocalDef iBool all_Bool3   (const iBool3 a)    { return mask_F3(a) == (8|4|2); }

iLocalDef iFloat3 clamp_F3  (const iFloat3 t, const iFloat3 a, const iFloat3 b) { return min_F3(max_F3(t, a), b); }
iLocalDef float sum_F3      (const iFloat3 a)       { return _mm_cvtss_f32(_mm_dp_ps(a.m, _mm_set1_ps(1.f), 0xe1)); }
iLocalDef float dot_F3      (const iFloat3 a, const iFloat3 b) { return _mm_cvtss_f32(_mm_dp_ps(a.m, b.m, 0xe1)); }
iLocalDef float lengthSq_F3 (const iFloat3 a)       { return dot_F3(a, a); }
iLocalDef float length_F3   (const iFloat3 a)       { return sqrtf(lengthSq_F3(a)); }
iLocalDef iFloat3 normalize_F3(const iFloat3 a)     { return mulf_F3(a, 1.f / length_F3(a)); }
iLocalDef iFloat3 sqrt_F3   (const iFloat3 a)       { return (iFloat3){ _mm_sqrt_ps(a.m) }; }

iLocalDef iFloat3 mix_F3   (const iFloat3 a, const iFloat3 b, float t) {
    return add_F3(a, mulf_F3(sub_F3(b, a), t));
}

/*-------------------------------------------------------------------------------------*/

iDeclareType(Mat4)

struct Impl_Mat4 {
    __m128 col[4];
};

iLocalDef void init_Mat4(iMat4 *d) {
    d->col[0] = init_F4(1, 0, 0, 0).m;
    d->col[1] = init_F4(0, 1, 0, 0).m;
    d->col[2] = init_F4(0, 0, 1, 0).m;
    d->col[3] = init_F4(0, 0, 0, 1).m;
}

void store_Mat4     (const iMat4 *, float *v);
void transpose_Mat4 (iMat4 *);

iLocalDef void load_Mat4(iMat4 *d, const float *v) {
    for (int i = 0; i < 4; ++i) {
        d->col[i] = initv_F4(v + 4*i).m;
        d->col[i] = _mm_shuffle_ps(d->col[i], d->col[i], _MM_SHUFFLE(2, 1, 0, 3));
    }
}

iLocalDef void copy_Mat4(iMat4 *d, const iMat4 *other) {
    d->col[0] = other->col[0];
    d->col[1] = other->col[1];
    d->col[2] = other->col[2];
    d->col[3] = other->col[3];
}

void    mul_Mat4    (iMat4 *, const iMat4 *b);
iFloat4 row_Mat4    (const iMat4 *, int row);

iLocalDef void translate_Mat4(iMat4 *d, iFloat3 v) {
    d->col[3] = _mm_add_ps(d->col[3],
                           /* Ensure w is really zero. */ _mm_move_ss(v.m, _mm_set_ss(0.0f)));
}

iLocalDef void initTranslate_Mat4(iMat4 *d, iFloat3 v) {
    init_Mat4(d);
    translate_Mat4(d, v);
}

iLocalDef void initScale_Mat4(iMat4 *d, iFloat3 v) {
    d->col[0] = _mm_set_ps(0, 0, x_F3(v), 0);
    d->col[1] = _mm_set_ps(0, y_F3(v), 0, 0);
    d->col[2] = _mm_set_ps(z_F3(v), 0, 0, 0);
    d->col[3] = _mm_set_ps(0, 0, 0, 1);
}

iLocalDef void scale_Mat4(iMat4 *d, iFloat3 v) {
    d->col[0] = _mm_mul_ps(d->col[0], _mm_set_ps(1, 1, x_F3(v), 1));
    d->col[1] = _mm_mul_ps(d->col[1], _mm_set_ps(1, y_F3(v), 1, 1));
    d->col[2] = _mm_mul_ps(d->col[2], _mm_set_ps(z_F3(v), 1, 1, 1));
}

iLocalDef void scalef_Mat4(iMat4 *d, float v) {
    d->col[0] = _mm_mul_ps(d->col[0], _mm_set_ps(1, 1, v, 1));
    d->col[1] = _mm_mul_ps(d->col[1], _mm_set_ps(1, v, 1, 1));
    d->col[2] = _mm_mul_ps(d->col[2], _mm_set_ps(v, 1, 1, 1));
}

void initRotate_Mat4(iMat4 *d, iFloat3 axis, float degrees);

iLocalDef void rotate_Mat4(iMat4 *d, iFloat3 axis, float degrees) {
    iMat4 rot; initRotate_Mat4(&rot, axis, degrees);
    mul_Mat4(d, &rot);
}

iLocalDef iFloat4 mulF4_Mat4(const iMat4 *d, iFloat4 v) {
    return init_F4(dot_F4(row_Mat4(d, 0), v),
                   dot_F4(row_Mat4(d, 1), v),
                   dot_F4(row_Mat4(d, 2), v),
                   dot_F4(row_Mat4(d, 3), v));
}

iLocalDef iFloat3 mulF3_Mat4(const iMat4 *d, const iFloat3 v) {
    const iFloat4 i = mulF4_Mat4(d, initmm_F4(_mm_move_ss(v.m, _mm_set1_ps(1.f))));
    return (iFloat3){ _mm_div_ps(i.m, _mm_set1_ps(_mm_cvtss_f32(i.m))) };
}

iDeclareType(Mat3)

struct Impl_Mat3 {
    __m128 col[3];
};

iLocalDef void init_Mat3(iMat3 *d) {
    d->col[0] = init_F3(1, 0, 0).m;
    d->col[1] = init_F3(0, 1, 0).m;
    d->col[2] = init_F3(0, 0, 1).m;
}

void store_Mat3 (const iMat3 *, float *v9);

iLocalDef void load_Mat3(iMat3 *d, const float *v9) {
    d->col[0] = initv_F3(v9    ).m;
    d->col[1] = initv_F3(v9 + 3).m;
    d->col[2] = initv_F3(v9 + 6).m;
}

iLocalDef iFloat3 mulF3_Mat3(const iMat3 *d, iFloat3 v) {
    return init_F3(dot_F3(initmm_F3(d->col[0]), v),
                   dot_F3(initmm_F3(d->col[1]), v),
                   dot_F3(initmm_F3(d->col[2]), v));
}

/*-------------------------------------------------------------------------------------*/

iLocalDef float iMinf(float a, float b) {
    return _mm_cvtss_f32(_mm_min_ss(_mm_set_ss(a), _mm_set_ss(b)));
}

iLocalDef float iMaxf(float a, float b) {
    return _mm_cvtss_f32(_mm_max_ss(_mm_set_ss(a), _mm_set_ss(b)));
}

iLocalDef float iClampf(float i, float low, float high) {
    return _mm_cvtss_f32(_mm_min_ss(_mm_max_ss(_mm_set_ss(i), _mm_set_ss(low)), _mm_set_ss(high)));
}
