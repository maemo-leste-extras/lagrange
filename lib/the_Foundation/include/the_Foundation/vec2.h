#pragma once

/** @file the_Foundation/vec2.h  2D integer vectors.

@authors Copyright (c) 2019 Jaakko Keränen <jaakko.keranen@iki.fi>

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

#include "defs.h"
#include "random.h"
#include "stream.h"
#include <math.h>

iDeclareType(IntVec2)
iDeclareType(Stream)

struct Impl_IntVec2 {
    int x, y;
};

typedef iIntVec2 iInt2;

iLocalDef iInt2 zero_I2(void) {
    return (iInt2){ 0, 0 };
}

iLocalDef iInt2 one_I2(void) {
    return (iInt2){ 1, 1 };
}

iLocalDef iInt2 init1_I2(int x) {
    return (iInt2){ x, x };
}

iLocalDef iInt2 init_I2(int x, int y) {
    return (iInt2){ x, y };
}

iLocalDef iInt2 initu_I2(unsigned x, unsigned y) {
    return (iInt2){ (int) x, (int) y };
}

iLocalDef iInt2 initv_I2(const int *v) {
    return (iInt2){ v[0], v[1] };
}

iLocalDef void store_I2(const iInt2 d, int *p_out) {
    p_out[0] = d.x;
    p_out[1] = d.y;
}

iLocalDef iInt2 yx_I2(const iInt2 d) {
    return (iInt2){ d.y, d.x };
}

iLocalDef iInt2 addX_I2     (const iInt2 a, int dx) { return (iInt2){ a.x + dx, a.y }; }
iLocalDef iInt2 addY_I2     (const iInt2 a, int dy) { return (iInt2){ a.x, a.y + dy }; }

iLocalDef iInt2 add_I2      (const iInt2 a, const iInt2 b)  { return (iInt2){ a.x + b.x, a.y + b.y }; }
iLocalDef iInt2 addi_I2     (const iInt2 a, int b)          { return (iInt2){ a.x + b, a.y + b }; }
iLocalDef iInt2 sub_I2      (const iInt2 a, const iInt2 b)  { return (iInt2){ a.x - b.x, a.y - b.y }; }
iLocalDef iInt2 subi_I2     (const iInt2 a, int b)          { return (iInt2){ a.x - b, a.y - b }; }
iLocalDef iInt2 mul_I2      (const iInt2 a, const iInt2 b)  { return (iInt2){ a.x * b.x, a.y * b.y }; }
iLocalDef iInt2 muli_I2     (const iInt2 a, int b)          { return (iInt2){ a.x * b, a.y * b }; }
iLocalDef iInt2 mulf_I2     (const iInt2 a, float b)        { return (iInt2){ (int) (a.x * b), (int) (a.y * b) }; }
iLocalDef iInt2 div_I2      (const iInt2 a, const iInt2 b)  { return (iInt2){ a.x / b.x, a.y / b.y }; }
iLocalDef iInt2 divi_I2     (const iInt2 a, int b)          { return (iInt2){ a.x / b, a.y / b }; }
iLocalDef iInt2 divf_I2     (const iInt2 a, float b)        { return (iInt2){ (int) (a.x / b), (int) (a.y / b) }; }
iLocalDef iInt2 mod_I2      (const iInt2 a, const iInt2 b)  { return (iInt2){ a.x % b.x, a.y % b.y }; }
iLocalDef iInt2 modi_I2     (const iInt2 a, int b)          { return (iInt2){ a.x % b, a.y % b }; }

iLocalDef iInt2 addv_I2     (iInt2 *a, const iInt2 b)       { a->x += b.x; a->y += b.y; return *a; }
iLocalDef iInt2 subv_I2     (iInt2 *a, const iInt2 b)       { a->x -= b.x; a->y -= b.y; return *a; }
iLocalDef iInt2 mulv_I2     (iInt2 *a, const iInt2 b)       { a->x *= b.x; a->y *= b.y; return *a; }
iLocalDef iInt2 muliv_I2    (iInt2 *a, int b)               { a->x *= b; a->y *= b; return *a; }
iLocalDef iInt2 mulfv_I2    (iInt2 *a, float b)             { a->x *= b; a->y *= b; return *a; }
iLocalDef iInt2 divv_I2     (iInt2 *a, const iInt2 b)       { a->x /= b.x; a->y /= b.y; return *a; }
iLocalDef iInt2 diviv_I2    (iInt2 *a, int b)               { a->x /= b; a->y /= b; return *a; }
iLocalDef iInt2 divfv_I2    (iInt2 *a, float b)             { a->x /= b; a->y /= b; return *a; }
iLocalDef iInt2 modv_I2     (iInt2 *a, const iInt2 b)       { a->x %= b.x; a->y %= b.y; return *a; }
iLocalDef iInt2 modiv_I2    (iInt2 *a, int b)               { a->x %= b; a->y %= b; return *a; }

iLocalDef iInt2 min_I2      (const iInt2 a, const iInt2 b)  { return (iInt2){ iMin(a.x, b.x), iMin(a.y, b.y) }; }
iLocalDef iInt2 max_I2      (const iInt2 a, const iInt2 b)  { return (iInt2){ iMax(a.x, b.x), iMax(a.y, b.y) }; }
iLocalDef iInt2 neg_I2      (const iInt2 a)                 { return (iInt2){ -a.x, -a.y }; }
iLocalDef iInt2 negX_I2     (const iInt2 a)                 { return (iInt2){ -a.x, a.y }; }
iLocalDef iInt2 negY_I2     (const iInt2 a)                 { return (iInt2){ a.x, -a.y }; }
iLocalDef iInt2 abs_I2      (const iInt2 a)                 { return (iInt2){ iAbs(a.x), iAbs(a.y) }; }

typedef iBoolv iBool2;

iLocalDef iBool2 equal_I2   (const iInt2 a, const iInt2 b) {
    return init2_Boolv(a.x == b.x, a.y == b.y);
}

iLocalDef iBool2 notEqual_I2(const iInt2 a, const iInt2 b) {
    return init2_Boolv(a.x != b.x, a.y != b.y);
}

iLocalDef iBool2 greater_I2 (const iInt2 a, const iInt2 b) {
    return init2_Boolv(a.x > b.x, a.y > b.y);
}

iLocalDef iBool2 greaterEqual_I2 (const iInt2 a, const iInt2 b) {
    return init2_Boolv(a.x >= b.x, a.y >= b.y);
}

iLocalDef iBool2 less_I2 (const iInt2 a, const iInt2 b) {
    return init2_Boolv(a.x < b.x, a.y < b.y);
}

iLocalDef iBool2 lessEqual_I2 (const iInt2 a, const iInt2 b) {
    return init2_Boolv(a.x <= b.x, a.y <= b.y);
}

iLocalDef iBool isEqual_I2  (const iInt2 a, const iInt2 b)  { return a.x == b.x && a.y == b.y; }

iLocalDef iInt2 clamp_I2    (const iInt2 t, const iInt2 a, const iInt2 b) { return min_I2(max_I2(t, a), b); }
iLocalDef int   sum_I2      (const iInt2 a)                 { return a.x + a.y; }
iLocalDef int   prod_I2     (const iInt2 a)                 { return a.x * a.y; }
iLocalDef int   dot_I2      (const iInt2 a, const iInt2 b)  { return sum_I2(mul_I2(a, b)); }
iLocalDef float lengthSq_I2 (const iInt2 a)                 { return dot_I2(a, a); }
iLocalDef float length_I2   (const iInt2 a)                 { return hypotf(a.x, a.y); }
iLocalDef float dist_I2     (const iInt2 a, const iInt2 b)  { return length_I2(sub_I2(b, a)); }
iLocalDef int   idist_I2    (const iInt2 a, const iInt2 b)  { return (int) (length_I2(sub_I2(b, a)) + .5f); }
iLocalDef int   manhattan_I2(const iInt2 a, const iInt2 b)  { return sum_I2(abs_I2(sub_I2(b, a))); }

iLocalDef iInt2 mix_I2      (const iInt2 a, const iInt2 b, float t) {
    return add_I2(a, mulf_I2(sub_I2(b, a), t));
}

iLocalDef iInt2 random_I2   (const iInt2 a) { return (iInt2){ iRandom(0, a.x), iRandom(0, a.y) }; }

iLocalDef void writeInt2_Stream(iStream *d, const iInt2 vec) {
    write32_Stream(d, vec.x);
    write32_Stream(d, vec.y);
}

iLocalDef iInt2 readInt2_Stream(iStream *d) {
    iInt2 vec;
    vec.x = read32_Stream(d);
    vec.y = read32_Stream(d);
    return vec;
}
