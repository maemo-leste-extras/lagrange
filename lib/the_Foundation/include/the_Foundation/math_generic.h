#pragma once

/** @file the_Foundation/math_generic.h  Vector math, generic implementation.

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

typedef iFloatVec3 iFloat3;
typedef iFloatVec4 iFloat4;

iLocalDef iFloat4 zero_F4(void) {
    return (iFloat4){ .v = { 0, 0, 0, 0 } };
}

iLocalDef iFloat4 init1_F4(float x) {
    return (iFloat4){ .v = { x, x, x, x } };
}

iLocalDef iFloat4 init_F4(float x, float y, float z, float w) {
    return (iFloat4){ .v = { x, y, z, w } };
}

iLocalDef iFloat4 initi_F4(int x, int y, int z, int w) {
    return (iFloat4){ .v = { (float) x, (float) y, (float) z, (float) w } };
}

iLocalDef iFloat4 initiv_F4(const int *v) {
    return (iFloat4){ .v = { (float) v[0], (float) v[1], (float) v[2], (float) v[3] } };
}

iLocalDef iFloat4 initv_F4(const float *v) {
    return (iFloat4){ .v = { v[0], v[1], v[2], v[3] } };
}

iLocalDef float x_F4(const iFloat4 d) {
    return d.value.x;
}

iLocalDef float y_F4(const iFloat4 d) {
    return d.value.y;
}

iLocalDef float z_F4(const iFloat4 d) {
    return d.value.z;
}

iLocalDef float w_F4(const iFloat4 d) {
    return d.value.w;
}

iLocalDef void store_F4(const iFloat4 d, float *p_out) {
    p_out[0] = d.v[0];
    p_out[1] = d.v[1];
    p_out[2] = d.v[2];
    p_out[3] = d.v[3];
}

iLocalDef iFloatVec4 values_F4(const iFloat4 d) {
    return d;
}

iLocalDef iFloat4 shuffle_F4(const iFloat4 d, int x, int y, int z, int w) {
    return init_F4(d.v[x], d.v[y], d.v[z], d.v[w]);
}

iLocalDef iFloat4 xyz_F4(const iFloat4 d) {
    return init_F4(d.value.x, d.value.y, d.value.z, 0);
}

iLocalDef iFloat4 yzx_F4(const iFloat4 d) {
    return shuffle_F4(d, 1, 2, 0, 3);
}

iLocalDef iFloat4 zxy_F4(const iFloat4 d) {
    return shuffle_F4(d, 2, 0, 1, 3);
}

iLocalDef iFloat4 xzy_F4(const iFloat4 d) {
    return shuffle_F4(d, 0, 2, 1, 3);
}

iLocalDef void setX_F4(iFloat4 *d, float x) {
    d->value.x = x;
}

iLocalDef void setY_F4(iFloat4 *d, float y) {
    d->value.y = y;
}

iLocalDef void setZ_F4(iFloat4 *d, float z) {
    d->value.z = z;
}

iLocalDef void setW_F4(iFloat4 *d, float w) {
    d->value.w = w;
}

typedef iFloat4 iBool4;

iLocalDef iFloat4 add_F4    (const iFloat4 a, const iFloat4 b)  {
    return (iFloat4){ .v = { a.value.x + b.value.x, a.value.y + b.value.y, a.value.z + b.value.z, a.value.w + b.value.w } };
}

iLocalDef iFloat4 addf_F4   (const iFloat4 a, const float b) {
    return (iFloat4){ .v = { a.value.x + b, a.value.y + b, a.value.z + b, a.value.w + b } };
}

iLocalDef iFloat4 sub_F4    (const iFloat4 a, const iFloat4 b)  {
    return (iFloat4){ .v = { a.value.x - b.value.x, a.value.y - b.value.y, a.value.z - b.value.z, a.value.w - b.value.w } };
}

iLocalDef iFloat4 subf_F4   (const iFloat4 a, const float b) {
    return (iFloat4){ .v = { a.value.x - b, a.value.y - b, a.value.z - b, a.value.w - b } };
}

iLocalDef iFloat4 mul_F4    (const iFloat4 a, const iFloat4 b)  {
    return (iFloat4){ .v = { a.value.x * b.value.x, a.value.y * b.value.y, a.value.z * b.value.z, a.value.w * b.value.w } };
}

iLocalDef iFloat4 mulf_F4   (const iFloat4 a, const float b)    {
    return (iFloat4){ .v = { a.value.x * b, a.value.y * b, a.value.z * b, a.value.w * b } };
}

iLocalDef iFloat4 fmul_F4   (const float a, const iFloat4 b)    {
    return (iFloat4){ .v = { b.value.x * a, b.value.y * a, b.value.z * a, b.value.w * a } };
}

iLocalDef iFloat4 div_F4    (const iFloat4 a, const iFloat4 b)  {
    return (iFloat4){ .v = { a.value.x / b.value.x, a.value.y / b.value.y, a.value.z / b.value.z, a.value.w / b.value.w } };
}

iLocalDef iFloat4 divf_F4   (const iFloat4 a, const float b)    {
    return (iFloat4){ .v = { a.value.x / b, a.value.y / b, a.value.z / b, a.value.w / b } };
}

iLocalDef iFloat4 fdiv_F4   (const float a, const iFloat4 b)    {
    return (iFloat4){ .v = { b.value.x / a, b.value.y / a, b.value.z / a, b.value.w / a } };
}

iLocalDef iFloat4 addv_F4   (iFloat4 *a, const iFloat4 b)       {
    a->v[0] += b.v[0];
    a->v[1] += b.v[1];
    a->v[2] += b.v[2];
    a->v[3] += b.v[3];
    return *a;
}

iLocalDef iFloat4 subv_F4   (iFloat4 *a, const iFloat4 b)       {
    a->v[0] -= b.v[0];
    a->v[1] -= b.v[1];
    a->v[2] -= b.v[2];
    a->v[3] -= b.v[3];
    return *a;
}

iLocalDef iFloat4 mulv_F4   (iFloat4 *a, const iFloat4 b)       {
    a->v[0] *= b.v[0];
    a->v[1] *= b.v[1];
    a->v[2] *= b.v[2];
    a->v[3] *= b.v[3];
    return *a;
}

iLocalDef iFloat4 mulvf_F4  (iFloat4 *a, const float b)         {
    a->v[0] *= b;
    a->v[1] *= b;
    a->v[2] *= b;
    a->v[3] *= b;
    return *a;
}

iLocalDef iFloat4 divv_F4   (iFloat4 *a, const iFloat4 b)       {
    a->v[0] /= b.v[0];
    a->v[1] /= b.v[1];
    a->v[2] /= b.v[2];
    a->v[3] /= b.v[3];
    return *a;
}

iLocalDef iFloat4 divvf_F4  (iFloat4 *a, const float b)         {
    a->v[0] /= b;
    a->v[1] /= b;
    a->v[2] /= b;
    a->v[3] /= b;
    return *a;
}

iLocalDef iFloat4 leftv_F4(iFloat4 *a) {
    const float t = a->v[3];
    a->v[3] = a->v[0];
    a->v[0] = a->v[1];
    a->v[1] = a->v[2];
    a->v[2] = t;
    return *a;
}

iLocalDef iFloat4 rightv_F4(iFloat4 *a) {
    const float t = a->v[3];
    a->v[3] = a->v[2];
    a->v[2] = a->v[1];
    a->v[1] = a->v[0];
    a->v[0] = t;
    return *a;
}

#define iFloatVecBool(b)   ((b) ? 1.0f : 0.0f)

iLocalDef iBool4 equal_F4   (const iFloat4 a, const iFloat4 b)  {
    return (iFloat4){ .v = { iFloatVecBool(a.v[0] == b.v[0]),
                             iFloatVecBool(a.v[1] == b.v[1]),
                             iFloatVecBool(a.v[2] == b.v[2]),
                             iFloatVecBool(a.v[3] == b.v[3]) } };
}

iLocalDef iBool4 notEqual_F4(const iFloat4 a, const iFloat4 b)  {
    return sub_F4(init1_F4(1.f), equal_F4(a, b));
}

iLocalDef iBool4 less_F4    (const iFloat4 a, const iFloat4 b)  {
    return (iFloat4){ .v = { iFloatVecBool(a.v[0] < b.v[0]),
                             iFloatVecBool(a.v[1] < b.v[1]),
                             iFloatVecBool(a.v[2] < b.v[2]),
                             iFloatVecBool(a.v[3] < b.v[3]) } };
}

iLocalDef iBool4 greater_F4 (const iFloat4 a, const iFloat4 b)  {
    return (iFloat4){ .v = { iFloatVecBool(a.v[0] > b.v[0]),
                             iFloatVecBool(a.v[1] > b.v[1]),
                             iFloatVecBool(a.v[2] > b.v[2]),
                             iFloatVecBool(a.v[3] > b.v[3]) } };
}

iLocalDef iBool4 lessEqual_F4   (const iFloat4 a, const iFloat4 b)  {
    return (iFloat4){ .v = { iFloatVecBool(a.v[0] <= b.v[0]),
                             iFloatVecBool(a.v[1] <= b.v[1]),
                             iFloatVecBool(a.v[2] <= b.v[2]),
                             iFloatVecBool(a.v[3] <= b.v[3]) } };
}

iLocalDef iBool4 greaterEqual_F4(const iFloat4 a, const iFloat4 b)  {
    return (iFloat4){ .v = { iFloatVecBool(a.v[0] >= b.v[0]),
                             iFloatVecBool(a.v[1] >= b.v[1]),
                             iFloatVecBool(a.v[2] >= b.v[2]),
                             iFloatVecBool(a.v[3] >= b.v[3]) } };
}

iLocalDef iFloat4 min_F4    (const iFloat4 a, const iFloat4 b)  {
    return (iFloat4){ .v = { iMin(a.v[0], b.v[0]),
                             iMin(a.v[1], b.v[1]),
                             iMin(a.v[2], b.v[2]),
                             iMin(a.v[3], b.v[3]) } };
}

iLocalDef iFloat4 max_F4    (const iFloat4 a, const iFloat4 b)  {
    return (iFloat4){ .v = { iMax(a.v[0], b.v[0]),
                             iMax(a.v[1], b.v[1]),
                             iMax(a.v[2], b.v[2]),
                             iMax(a.v[3], b.v[3]) } };
}

iLocalDef iFloat4 neg_F4    (const iFloat4 a)                   {
    return sub_F4(zero_F4(), a);
}

iLocalDef iFloat4 abs_F4(const iFloat4 a) {
    return (iFloat4){ .v = { fabsf(a.v[0]),
                             fabsf(a.v[1]),
                             fabsf(a.v[2]),
                             fabsf(a.v[3]) } };
}

iLocalDef unsigned mask_F4  (const iFloat4 a)   {
    return (a.v[0] > 0? 0x1 : 0) |
           (a.v[1] > 0? 0x2 : 0) |
           (a.v[2] > 0? 0x4 : 0) |
           (a.v[3] > 0? 0x8 : 0);
}

iLocalDef iBool any_Bool4       (const iBool4 a)    { return mask_F4(a) != 0; }
iLocalDef iBool all_Bool4       (const iBool4 a)    { return mask_F4(a) == 15; }

iLocalDef iFloat4 clamp_F4  (const iFloat4 t, const iFloat4 a, const iFloat4 b) { return min_F4(max_F4(t, a), b); }
iLocalDef float sum_F4      (const iFloat4 a)   { return x_F4(a) + y_F4(a) + z_F4(a) + w_F4(a); }
iLocalDef float dot_F4      (const iFloat4 a, const iFloat4 b) { return sum_F4(mul_F4(a, b)); }
iLocalDef float lengthSq_F4 (const iFloat4 a)   { return dot_F4(a, a); }
iLocalDef float length_F4   (const iFloat4 a)   { return sqrtf(lengthSq_F4(a)); }
iLocalDef iFloat4 normalize_F4(const iFloat4 a) { return mulf_F4(a, 1.f / length_F4(a)); }
iLocalDef iFloat4 sqrt_F4   (const iFloat4 a)   {
    return (iFloat4){ .v = { sqrtf(a.v[0]), sqrtf(a.v[1]), sqrtf(a.v[2]), sqrtf(a.v[3]) } };
}

iLocalDef iFloat4 mix_F4   (const iFloat4 a, const iFloat4 b, float t) {
    return add_F4(a, mulf_F4(sub_F4(b, a), t));
}

/*-------------------------------------------------------------------------------------*/

iLocalDef iFloat3 zero_F3(void) {
    return (iFloat3){ .v = { 0, 0, 0 } };
}

iLocalDef iFloat3 init1_F3(float x) {
    return (iFloat3){ .v = { x, x, x } };
}

iLocalDef iFloat3 init_F3(float x, float y, float z) {
    return (iFloat3){ .v = { x, y, z } };
}

iLocalDef iFloat3 initi_F3(int x, int y, int z) {
    return (iFloat3){ .v = { (float) x, (float) y, (float) z } };
}

iLocalDef iFloat3 initiv_F3(const int *v) {
    return (iFloat3){ .v = { (float) v[0], (float) v[1], (float) v[2] } };
}

iLocalDef iFloat3 initiv2_F3(const int *v) {
    return (iFloat3){ .v = { (float) v[0], (float) v[1], 0.f } };
}

iLocalDef iFloat3 initv_F3(const float *v) {
    return (iFloat3){ .v = { v[0], v[1], v[2] } };
}

iLocalDef float x_F3(const iFloat3 d) {
    return d.value.x;
}

iLocalDef float y_F3(const iFloat3 d) {
    return d.value.y;
}

iLocalDef float z_F3(const iFloat3 d) {
    return d.value.z;
}

iLocalDef void store_F3(const iFloat3 d, float *p_out) {
    p_out[0] = d.v[0];
    p_out[1] = d.v[1];
    p_out[2] = d.v[2];
}

iLocalDef iFloatVec3 values_F3(const iFloat3 d) {
    return d;
}

iLocalDef iFloat3 shuffle_F3(const iFloat3 d, int x, int y, int z) {
    return init_F3(d.v[x], d.v[y], d.v[z]);
}

iLocalDef iFloat3 xyz_F3(const iFloat3 d) {
    return init_F3(d.value.x, d.value.y, d.value.z);
}

iLocalDef iFloat3 yzx_F3(const iFloat3 d) {
    return shuffle_F3(d, 1, 2, 0);
}

iLocalDef iFloat3 zxy_F3(const iFloat3 d) {
    return shuffle_F3(d, 2, 0, 1);
}

iLocalDef iFloat3 xzy_F3(const iFloat3 d) {
    return shuffle_F3(d, 0, 2, 1);
}

iLocalDef void setX_F3(iFloat3 *d, float x) {
    d->value.x = x;
}

iLocalDef void setY_F3(iFloat3 *d, float y) {
    d->value.y = y;
}

iLocalDef void setZ_F3(iFloat3 *d, float z) {
    d->value.z = z;
}

typedef iFloat3 iBool3;

iLocalDef iFloat3 add_F3    (const iFloat3 a, const iFloat3 b)  {
    return (iFloat3){ .v = { a.value.x + b.value.x, a.value.y + b.value.y, a.value.z + b.value.z } };
}

iLocalDef iFloat3 addf_F3   (const iFloat3 a, const float b) {
    return (iFloat3){ .v = { a.value.x + b, a.value.y + b, a.value.z + b } };
}

iLocalDef iFloat3 sub_F3    (const iFloat3 a, const iFloat3 b)  {
    return (iFloat3){ .v = { a.value.x - b.value.x, a.value.y - b.value.y, a.value.z - b.value.z } };
}

iLocalDef iFloat3 subf_F3   (const iFloat3 a, const float b)  {
    return (iFloat3){ .v = { a.value.x - b, a.value.y - b, a.value.z - b } };
}

iLocalDef iFloat3 mul_F3    (const iFloat3 a, const iFloat3 b)  {
    return (iFloat3){ .v = { a.value.x * b.value.x, a.value.y * b.value.y, a.value.z * b.value.z } };
}

iLocalDef iFloat3 mulf_F3   (const iFloat3 a, const float b)    {
    return (iFloat3){ .v = { a.value.x * b, a.value.y * b, a.value.z * b } };
}

iLocalDef iFloat3 fmul_F3   (const float a, const iFloat3 b)    {
    return (iFloat3){ .v = { b.value.x * a, b.value.y * a, b.value.z * a } };
}

iLocalDef iFloat3 div_F3    (const iFloat3 a, const iFloat3 b)  {
    return (iFloat3){ .v = { a.value.x / b.value.x, a.value.y / b.value.y, a.value.z / b.value.z } };
}

iLocalDef iFloat3 divf_F3   (const iFloat3 a, const float b)    {
    return (iFloat3){ .v = { a.value.x / b, a.value.y / b, a.value.z / b } };
}

iLocalDef iFloat3 fdiv_F3   (const float a, const iFloat3 b)    {
    return (iFloat3){ .v = { b.value.x / a, b.value.y / a, b.value.z / a } };
}

iLocalDef iFloat3 addv_F3   (iFloat3 *a, const iFloat3 b)       {
    a->v[0] += b.v[0];
    a->v[1] += b.v[1];
    a->v[2] += b.v[2];
    return *a;
}

iLocalDef iFloat3 subv_F3   (iFloat3 *a, const iFloat3 b)       {
    a->v[0] -= b.v[0];
    a->v[1] -= b.v[1];
    a->v[2] -= b.v[2];
    return *a;
}

iLocalDef iFloat3 mulv_F3   (iFloat3 *a, const iFloat3 b)       {
    a->v[0] *= b.v[0];
    a->v[1] *= b.v[1];
    a->v[2] *= b.v[2];
    return *a;
}

iLocalDef iFloat3 mulvf_F3  (iFloat3 *a, const float b)         {
    a->v[0] *= b;
    a->v[1] *= b;
    a->v[2] *= b;
    return *a;
}

iLocalDef iFloat3 divv_F3   (iFloat3 *a, const iFloat3 b)       {
    a->v[0] /= b.v[0];
    a->v[1] /= b.v[1];
    a->v[2] /= b.v[2];
    return *a;
}

iLocalDef iFloat3 divvf_F3  (iFloat3 *a, const float b)         {
    a->v[0] /= b;
    a->v[1] /= b;
    a->v[2] /= b;
    return *a;
}

iLocalDef iFloat3 leftv_F3(iFloat3 *a) {
    const float t = a->v[2];
    a->v[2] = a->v[0];
    a->v[0] = a->v[1];
    a->v[1] = t;
    return *a;
}

iLocalDef iFloat3 rightv_F3(iFloat3 *a) {
    const float t = a->v[2];
    a->v[2] = a->v[1];
    a->v[1] = a->v[0];
    a->v[0] = t;
    return *a;
}

iLocalDef iBool3 equal_F3   (const iFloat3 a, const iFloat3 b)  {
    return (iBool3){ .v = { iFloatVecBool(a.v[0] == b.v[0]),
                            iFloatVecBool(a.v[1] == b.v[1]),
                            iFloatVecBool(a.v[2] == b.v[2]) } };
}

iLocalDef iBool3 notEqual_F3(const iFloat3 a, const iFloat3 b)  {
    return sub_F3(init1_F3(1.f), equal_F3(a, b));
}

iLocalDef iBool3 less_F3    (const iFloat3 a, const iFloat3 b)  {
    return (iBool3){ .v = { iFloatVecBool(a.v[0] < b.v[0]),
                            iFloatVecBool(a.v[1] < b.v[1]),
                            iFloatVecBool(a.v[2] < b.v[2]) } };
}

iLocalDef iBool3 greater_F3 (const iFloat3 a, const iFloat3 b)  {
    return (iBool3){ .v = { iFloatVecBool(a.v[0] > b.v[0]),
                            iFloatVecBool(a.v[1] > b.v[1]),
                            iFloatVecBool(a.v[2] > b.v[2]) } };
}

iLocalDef iBool3 lessEqual_F3   (const iFloat3 a, const iFloat3 b)  {
    return (iBool3){ .v = { iFloatVecBool(a.v[0] <= b.v[0]),
                            iFloatVecBool(a.v[1] <= b.v[1]),
                            iFloatVecBool(a.v[2] <= b.v[2]) } };
}

iLocalDef iBool3 greaterEqual_F3(const iFloat3 a, const iFloat3 b)  {
    return (iBool3){ .v = { iFloatVecBool(a.v[0] >= b.v[0]),
                            iFloatVecBool(a.v[1] >= b.v[1]),
                            iFloatVecBool(a.v[2] >= b.v[2]) } };
}

iLocalDef iFloat3 min_F3    (const iFloat3 a, const iFloat3 b)  {
    return (iFloat3){ .v = { iMin(a.v[0], b.v[0]),
                             iMin(a.v[1], b.v[1]),
                             iMin(a.v[2], b.v[2]) } };
}

iLocalDef iFloat3 max_F3    (const iFloat3 a, const iFloat3 b)  {
    return (iFloat3){ .v = { iMax(a.v[0], b.v[0]),
                             iMax(a.v[1], b.v[1]),
                             iMax(a.v[2], b.v[2]) } };
}

iLocalDef iFloat3 neg_F3    (const iFloat3 a)                   {
    return sub_F3(zero_F3(), a);
}

iLocalDef iFloat3 abs_F3(const iFloat3 a) {
    return (iFloat3){ .v = { fabsf(a.v[0]),
                             fabsf(a.v[1]),
                             fabsf(a.v[2]) } };
}

iLocalDef unsigned mask_F3  (const iFloat3 a)   {
    return (a.v[0] > 0? 0x1 : 0) |
           (a.v[1] > 0? 0x2 : 0) |
           (a.v[2] > 0? 0x4 : 0);
}

iLocalDef iBool any_Bool3       (const iBool3 a)    { return mask_F3(a) != 0; }
iLocalDef iBool all_Bool3       (const iBool3 a)    { return mask_F3(a) == 7; }

iLocalDef iFloat3 clamp_F3  (const iFloat3 t, const iFloat3 a, const iFloat3 b) { return min_F3(max_F3(t, a), b); }
iLocalDef float sum_F3      (const iFloat3 a)   { return x_F3(a) + y_F3(a) + z_F3(a); }
iLocalDef float dot_F3      (const iFloat3 a, const iFloat3 b) { return sum_F3(mul_F3(a, b)); }
iLocalDef float lengthSq_F3 (const iFloat3 a)   { return dot_F3(a, a); }
iLocalDef float length_F3   (const iFloat3 a)   { return sqrtf(lengthSq_F3(a)); }
iLocalDef iFloat3 normalize_F3(const iFloat3 a) { return mulf_F3(a, 1.f / length_F3(a)); }
iLocalDef iFloat3 sqrt_F3   (const iFloat3 a)   {
    return (iFloat3){ .v = { sqrtf(a.v[0]), sqrtf(a.v[1]), sqrtf(a.v[2]) } };
}

iLocalDef iFloat3 mix_F3   (const iFloat3 a, const iFloat3 b, float t) {
    return add_F3(a, mulf_F3(sub_F3(b, a), t));
}

iLocalDef iFloat3 cross_F3(const iFloat3 a, const iFloat3 b) {
    return zxy_F3(sub_F3(mul_F3(zxy_F3(a), b),
                         mul_F3(a, zxy_F3(b))));
}

/*-------------------------------------------------------------------------------------*/

iDeclareType(Mat4)

struct Impl_Mat4 {
    iFloatVec4 col[4];
};

void init_Mat4  (iMat4 *);

void store_Mat4 (const iMat4 *, float *v);
void load_Mat4  (iMat4 *, const float *v);

iLocalDef void copy_Mat4(iMat4 *d, const iMat4 *other) {
    d->col[0] = other->col[0];
    d->col[1] = other->col[1];
    d->col[2] = other->col[2];
    d->col[3] = other->col[3];
}

void mul_Mat4       (iMat4 *, const iMat4 *right);
void transpose_Mat4 (iMat4 *);

iLocalDef iFloat4 row_Mat4(const iMat4 *d, int row) {
    return init_F4(d->col[0].v[row], d->col[1].v[row], d->col[2].v[row], d->col[3].v[row]);
}

iLocalDef void translate_Mat4(iMat4 *d, iFloat3 v) {
    addv_F4(&d->col[3], init_F4(x_F3(v), y_F3(v), z_F3(v), 0.0f));
}

iLocalDef void initTranslate_Mat4(iMat4 *d, iFloat3 v) {
    init_Mat4(d);
    translate_Mat4(d, v);
}

iLocalDef void initScale_Mat4(iMat4 *d, iFloat3 v) {
    d->col[0] = init_F4(x_F3(v), 0, 0, 0);
    d->col[1] = init_F4(0, y_F3(v), 0, 0);
    d->col[2] = init_F4(0, 0, z_F3(v), 0);
    d->col[3] = init_F4(0, 0, 0, 1);
}

iLocalDef void scale_Mat4(iMat4 *d, iFloat3 v) {
    d->col[0].value.x *= v.value.x;
    d->col[1].value.y *= v.value.y;
    d->col[2].value.z *= v.value.z;
}

void initRotate_Mat4(iMat4 *d, iFloat3 axis, float degrees);

iLocalDef void rotate_Mat4(iMat4 *d, iFloat3 axis, float degrees) {
    iMat4 rot; initRotate_Mat4(&rot, axis, degrees);
    mul_Mat4(d, &rot);
}

iLocalDef void scalef_Mat4(iMat4 *d, float v) {
    d->col[0].value.x *= v;
    d->col[1].value.y *= v;
    d->col[2].value.z *= v;
}

iLocalDef iFloat4 mulF4_Mat4(const iMat4 *d, const iFloat4 v) {
    return init_F4(dot_F4(row_Mat4(d, 0), v),
                   dot_F4(row_Mat4(d, 1), v),
                   dot_F4(row_Mat4(d, 2), v),
                   dot_F4(row_Mat4(d, 3), v));
}

iLocalDef iFloat3 mulF3_Mat4(const iMat4 *d, const iFloat3 v) {
    iFloat4 v4 = mulF4_Mat4(d, init_F4(v.v[0], v.v[1], v.v[2], 1.0f));
    return divf_F3(initv_F3(v4.v), v4.value.w);
}

iDeclareType(Mat3)

struct Impl_Mat3 {
    iFloat3 col[3];
};

iLocalDef void init_Mat3(iMat3 *d) {
    d->col[0] = init_F3(1, 0, 0);
    d->col[1] = init_F3(0, 1, 0);
    d->col[2] = init_F3(0, 0, 1);
}

void store_Mat3 (const iMat3 *, float *v9);

iLocalDef void load_Mat3(iMat3 *d, const float *v9) {
    d->col[0] = initv_F3(v9    );
    d->col[1] = initv_F3(v9 + 3);
    d->col[2] = initv_F3(v9 + 6);
}

iLocalDef iFloat3 mulF3_Mat3(const iMat3 *d, iFloat3 v) {
    return init_F3(dot_F3(initv_F3(d->col[0].v), v),
                   dot_F3(initv_F3(d->col[1].v), v),
                   dot_F3(initv_F3(d->col[2].v), v));
}

/*-------------------------------------------------------------------------------------*/

iLocalDef float iMinf(float a, float b) { return iMin(a, b); }
iLocalDef float iMaxf(float a, float b) { return iMax(a, b); }
iLocalDef float iClampf(float i, float low, float high) { return iClamp(i, low, high); }
