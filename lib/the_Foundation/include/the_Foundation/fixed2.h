#pragma once

/** @file fixed2.h  Fixed-point 48.16 2D vector.

@authors Copyright (c) 2020 Jaakko Keränen <jaakko.keranen@iki.fi>

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

#include "fixed.h"

iDeclareType(Fixed2)

struct Impl_Fixed2 {
    iFixed x, y;
};

iLocalDef iFixed2 init1_X2(const iFixed x) {
    return (iFixed2){ x, x };
}

iLocalDef iFixed2 initi1_X2(int x) {
    return init1_X2(initi_Fixed(x));
}

iLocalDef iFixed2 initf1_X2(float x) {
    return init1_X2(initf_Fixed(x));
}

iLocalDef iFixed2 init_X2(const iFixed x, const iFixed y) {
    return (iFixed2){ x, y };
}

iLocalDef iFixed2 initi_X2(int x, int y) {
    return init_X2(initi_Fixed(x), initi_Fixed(y));
}

iLocalDef iFixed2 initf_X2(float x, float y) {
    return init_X2(initf_Fixed(x), initf_Fixed(y));
}

iLocalDef iFixed2 initv_X2(const iFixed *v) {
    return (iFixed2){ v[0], v[1] };
}

iLocalDef iFixed2 zero_X2(void) {
    return (iFixed2){ .x = zero_Fixed(), .y = zero_Fixed() };
}

iLocalDef iFixed2 one_X2(void) {
    return init1_X2(one_Fixed());
}

iLocalDef void store_X2(const iFixed2 d, iFixed *p_out) {
    p_out[0] = d.x;
    p_out[1] = d.y;
}

iLocalDef iFixed2 yx_X2(const iFixed2 d) {
    return (iFixed2){ d.y, d.x };
}

iLocalDef iFixed2 addX_X2   (const iFixed2 a, const iFixed dx)  { return (iFixed2){ add_Fixed(a.x, dx), a.y }; }
iLocalDef iFixed2 addY_X2   (const iFixed2 a, const iFixed dy)  { return (iFixed2){ a.x, add_Fixed(a.y, dy) }; }

iLocalDef iFixed2 add_X2    (const iFixed2 a, const iFixed2 b)  { return (iFixed2){ add_Fixed(a.x, b.x), add_Fixed(a.y, b.y) }; }
iLocalDef iFixed2 addi_X2   (const iFixed2 a, int b)            { return add_X2(a, initi1_X2(b)); }
iLocalDef iFixed2 addf_X2   (const iFixed2 a, float b)          { return add_X2(a, initf1_X2(b)); }
iLocalDef iFixed2 sub_X2    (const iFixed2 a, const iFixed2 b)  { return (iFixed2){ sub_Fixed(a.x, b.x), sub_Fixed(a.y, b.y) }; }
iLocalDef iFixed2 subi_X2   (const iFixed2 a, int b)            { return sub_X2(a, initi1_X2(b)); }
iLocalDef iFixed2 subf_X2   (const iFixed2 a, float b)          { return sub_X2(a, initf1_X2(b)); }
iLocalDef iFixed2 mul_X2    (const iFixed2 a, const iFixed2 b)  { return (iFixed2){ mul_Fixed(a.x, b.x), mul_Fixed(a.y, b.y) }; }
iLocalDef iFixed2 muli_X2   (const iFixed2 a, int b)            { return mul_X2(a, initi1_X2(b)); }
iLocalDef iFixed2 mulf_X2   (const iFixed2 a, float b)          { return mul_X2(a, initf1_X2(b)); }
iLocalDef iFixed2 div_X2    (const iFixed2 a, const iFixed2 b)  { return (iFixed2){ div_Fixed(a.x, b.x), div_Fixed(a.y, b.y) }; }
iLocalDef iFixed2 divi_X2   (const iFixed2 a, int b)            { return div_X2(a, initi1_X2(b)); }
iLocalDef iFixed2 divf_X2   (const iFixed2 a, float b)          { return div_X2(a, initf1_X2(b)); }

iLocalDef iFixed2 addv_X2   (iFixed2 *a, const iFixed2 b)       { addv_Fixed(&a->x, b.x); addv_Fixed(&a->y, b.y); return *a; }
iLocalDef iFixed2 addiv_X2  (iFixed2 *a, int b)                 { return addv_X2(a, initi1_X2(b)); }
iLocalDef iFixed2 addfv_X2  (iFixed2 *a, float b)               { return addv_X2(a, initf1_X2(b)); }
iLocalDef iFixed2 subv_X2   (iFixed2 *a, const iFixed2 b)       { subv_Fixed(&a->x, b.x); subv_Fixed(&a->y, b.y); return *a; }
iLocalDef iFixed2 subiv_X2  (iFixed2 *a, int b)                 { return subv_X2(a, initi1_X2(b)); }
iLocalDef iFixed2 subfv_X2  (iFixed2 *a, float b)               { return subv_X2(a, initf1_X2(b)); }
iLocalDef iFixed2 mulv_X2   (iFixed2 *a, const iFixed2 b)       { mulv_Fixed(&a->x, b.x); mulv_Fixed(&a->y, b.y); return *a; }
iLocalDef iFixed2 muliv_X2  (iFixed2 *a, int b)                 { return mulv_X2(a, initi1_X2(b)); }
iLocalDef iFixed2 mulfv_X2  (iFixed2 *a, float b)               { return mulv_X2(a, initf1_X2(b)); }
iLocalDef iFixed2 divv_X2   (iFixed2 *a, const iFixed2 b)       { divv_Fixed(&a->x, b.x); divv_Fixed(&a->y, b.y); return *a; }
iLocalDef iFixed2 diviv_X2  (iFixed2 *a, int b)                 { return divv_X2(a, initi1_X2(b)); }
iLocalDef iFixed2 divfv_X2  (iFixed2 *a, float b)               { return divv_X2(a, initf1_X2(b)); }

iLocalDef iFixed2 min_X2    (const iFixed2 a, const iFixed2 b)  { return (iFixed2){ min_Fixed(a.x, b.x), min_Fixed(a.y, b.y) }; }
iLocalDef iFixed2 max_X2    (const iFixed2 a, const iFixed2 b)  { return (iFixed2){ max_Fixed(a.x, b.x), max_Fixed(a.y, b.y) }; }
iLocalDef iFixed2 neg_X2    (const iFixed2 a)                   { return (iFixed2){ neg_Fixed(a.x), neg_Fixed(a.y) }; }
iLocalDef iFixed2 abs_X2    (const iFixed2 a)                   { return (iFixed2){ abs_Fixed(a.x), abs_Fixed(a.y) }; }

iLocalDef iBool   isEqual_X2(const iFixed2 a, const iFixed2 b)  { return value_Fixed(a.x) == value_Fixed(b.x) && value_Fixed(a.y) == value_Fixed(b.y); }

iLocalDef iBoolv equal_X2   (const iFixed2 a, const iFixed2 b) {
    return init2_Boolv(value_Fixed(a.x) == value_Fixed(b.x), value_Fixed(a.y) == value_Fixed(b.y));
}

iLocalDef iBoolv notEqual_X2(const iFixed2 a, const iFixed2 b) {
    return init2_Boolv(value_Fixed(a.x) != value_Fixed(b.x), value_Fixed(a.y) != value_Fixed(b.y));
}

iLocalDef iBoolv greater_X2 (const iFixed2 a, const iFixed2 b) {
    return init2_Boolv(value_Fixed(a.x) > value_Fixed(b.x), value_Fixed(a.y) > value_Fixed(b.y));
}

iLocalDef iBoolv greaterEqual_X2 (const iFixed2 a, const iFixed2 b) {
    return init2_Boolv(value_Fixed(a.x) >= value_Fixed(b.x), value_Fixed(a.y) >= value_Fixed(b.y));
}

iLocalDef iBoolv less_X2 (const iFixed2 a, const iFixed2 b) {
    return init2_Boolv(value_Fixed(a.x) < value_Fixed(b.x), value_Fixed(a.y) < value_Fixed(b.y));
}

iLocalDef iBoolv lessEqual_X2 (const iFixed2 a, const iFixed2 b) {
    return init2_Boolv(value_Fixed(a.x) <= value_Fixed(b.x), value_Fixed(a.y) <= value_Fixed(b.y));
}

iLocalDef iFixed2   clamp_X2        (const iFixed2 t, const iFixed2 a, const iFixed2 b) { return min_X2(max_X2(t, a), b); }
iLocalDef iFixed    sum_X2          (const iFixed2 a) { return init_Fixed(value_Fixed(a.x) + value_Fixed(a.y)); }
iLocalDef iFixed    dot_X2          (const iFixed2 a, const iFixed2 b) { return sum_X2(mul_X2(a, b)); }
iLocalDef iFixed    lengthSq_X2     (const iFixed2 a) { return dot_X2(a, a); }
iLocalDef float     lengthf_X2      (const iFixed2 a) { return sqrtf(f32_Fixed(lengthSq_X2(a))); }
iLocalDef iFixed    length_X2       (const iFixed2 a) { return initf_Fixed(lengthf_X2(a)); }
iLocalDef iFixed2   normalize_X2    (const iFixed2 a) { return div_X2(a, init1_X2(length_X2(a))); }

iLocalDef iFixed2 sqrt_X2(const iFixed2 a) {
    return (iFixed2){ initf_Fixed(sqrtf(f32_Fixed(a.x))),
                     initf_Fixed(sqrtf(f32_Fixed(a.y))) };
}

iLocalDef iFixed2 mix_X2(const iFixed2 a, const iFixed2 b, const iFixed t) {
    return add_X2(a, mul_X2(sub_X2(b, a), init1_X2(t)));
}
