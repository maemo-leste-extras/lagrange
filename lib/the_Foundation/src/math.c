/** @file math.c  Math routines.

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

#include "the_Foundation/math.h"

#include <stdlib.h>

int iRound(float value) {
    return (int) lroundf(value);
}

int16_t iRound16(float value) {
    return (int16_t) lroundf(value);
}

int iWrap(int value, int low, int high) {
    const int span = high - low;
    if (span <= 0) return value;
    int rem = (value - low) % span;
    if (rem < 0) {
        rem += span;
    }
    return low + rem;
}

float iWrapf(float value, float low, float high) {
    const float span = high - low;
    if (span <= 0) return value;
    return low + fmodf(value - low, span);
}

/*-------------------------------------------------------------------------------------*/

static float determinant_Mat3_(const float *mat) {
    return (mat[0] * ( mat[4] * mat[8] - mat[7] * mat[5] ) -
            mat[1] * ( mat[3] * mat[8] - mat[6] * mat[5] ) +
            mat[2] * ( mat[3] * mat[7] - mat[6] * mat[4] ));
}

static iBool inverse9_Mat3_(const float *mat, float *inverse_out) {
    const float det = determinant_Mat3_(mat);
    if (fabsf(det) < .000001f) {
        iMat3 identity;
        init_Mat3(&identity);
        store_Mat3(&identity, inverse_out);
        return iFalse;
    }
    inverse_out[0] =    mat[4] * mat[8] - mat[5] * mat[7]   / det;
    inverse_out[1] = -( mat[1] * mat[8] - mat[7] * mat[2] ) / det;
    inverse_out[2] =    mat[1] * mat[5] - mat[4] * mat[2]   / det;
    inverse_out[3] = -( mat[3] * mat[8] - mat[5] * mat[6] ) / det;
    inverse_out[4] =    mat[0] * mat[8] - mat[6] * mat[2]   / det;
    inverse_out[5] = -( mat[0] * mat[5] - mat[3] * mat[2] ) / det;
    inverse_out[6] =    mat[3] * mat[7] - mat[6] * mat[4]   / det;
    inverse_out[7] = -( mat[0] * mat[7] - mat[6] * mat[1] ) / det;
    inverse_out[8] =    mat[0] * mat[4] - mat[1] * mat[3]   / det;
    return iTrue;
}

iBool inverse_Mat3(const iMat3 *d, iMat3 *inversed_out) {
    float result[9];
    float d9[9];
    store_Mat3(d, d9);
    const iBool ok = inverse9_Mat3_(d9, result);
    load_Mat3(inversed_out, result);
    return ok;
}

static void submatrix_Mat4(const float *mat4, float *mat3, int i, int j) {
    /* Loop through 3x3 submatrix. */
    for (int di = 0; di < 3; di++) {
        for (int dj = 0; dj < 3; dj++) {
            /* Map 3x3 element (destination) to 4x4 element (source). */
            int si = di + (di >= i? 1 : 0);
            int sj = dj + (dj >= j? 1 : 0);
            /* Copy element. */
            mat3[di * 3 + dj] = mat4[si * 4 + sj];
        }
    }
}

static float determinant_Mat4(const float *mat) {
    float result = 0;
    float i = 1;
    for (int n = 0; n < 4; n++, i *= -1) {
        float sub[3*3];
        submatrix_Mat4(mat, sub, 0, n);
        result += mat[n] * determinant_Mat3_(sub) * i;
    }
    return result;
}

static iBool inverse16_Mat4_(const float *in16, float *out16) {
    const float det = determinant_Mat4(in16);
    if (fabsf(det) < .000001f) {
        iMat4 identity;
        init_Mat4(&identity);
        store_Mat4(&identity, out16);
        return iFalse;
    }
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            float sub[3*3];
            const float sign = (float) (1 - ((i + j) % 2) * 2);
            submatrix_Mat4(in16, sub, i, j);
            out16[i + j*4] = (determinant_Mat3_(sub) * sign) / det;
        }
    }
    return iTrue;
}

iBool inverse_Mat4(const iMat4 *d, iMat4 *inversed_out) {
    float result[16];
    float d16[16];
    store_Mat4(d, d16);
    const iBool ok = inverse16_Mat4_(d16, result);
    if (ok) load_Mat4(inversed_out, result);
    return ok;
}

void ortho_Mat4(iMat4 *d, float left, float right, float top, float bottom, float znear,
                float zfar) {
    float m[16] = { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0 };
    m[ 0] =  2.0f / (right - left);
    m[ 5] =  2.0f / (top - bottom);
    m[10] = -2.0f / (zfar - znear);
    m[12] = -(right + left) / (right - left);
    m[13] = -(top + bottom) / (top - bottom);
    m[14] = -(zfar + znear) / (zfar - znear);
    m[15] = 1.0f;
    load_Mat4(d, m);
}

void perspective_Mat4(iMat4 *d, float xFovDeg, float aspect, float znear, float zfar) {
    const float xFov = iMathDegreeToRadianf(xFovDeg);
    const float f    = 1.0f / tanf(0.5f * xFov);
    const float A    = zfar + znear;
    const float B    = znear - zfar;
    float m[16] = { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0 };
    m[ 0] = f;
    m[ 5] = f * aspect;
    m[10] = A / B;
    m[11] = -1.0f;
    m[14] =  2.0f * zfar * znear / B;
    load_Mat4(d, m);
}

void frame_Mat4(iMat4 *d, iFloat3 front, iFloat3 up, iBool mirror) {
    float m[16] = { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0 };
    iFloat3 f = normalize_F3(front);
    iFloat3 s = normalize_F3(cross_F3(f, up));
    iFloat3 u = normalize_F3(cross_F3(s, f));
    if (mirror) {
        s = neg_F3(s);
    }
    m[ 0] =  x_F3(s);
    m[ 1] =  x_F3(u);
    m[ 2] = -x_F3(f);
    m[ 4] =  y_F3(s);
    m[ 5] =  y_F3(u);
    m[ 6] = -y_F3(f);
    m[ 8] =  z_F3(s);
    m[ 9] =  z_F3(u);
    m[10] = -z_F3(f);
    m[15] =  1.0f;
    load_Mat4(d, m);
}

void lookAt_Mat4(iMat4 *d, iFloat3 target, iFloat3 eyePos, iFloat3 up) {
    frame_Mat4(d, sub_F3(target, eyePos), normalize_F3(up), iTrue);
    iMat4 orig;
    initTranslate_Mat4(&orig, neg_F3(eyePos));
    mul_Mat4(d, &orig);
}
