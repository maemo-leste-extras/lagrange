#pragma once

/** @file fixed3.h  Fixed-point 48.16 3D vector.

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

iDeclareType(Fixed3)

struct Impl_Fixed3 {
    iFixed x, y, z;
};

iLocalDef iFixed3 init1_X3(const iFixed x) {
    return (iFixed3){ x, x, x };
}

iLocalDef iFixed3 initi1_X3(int x) {
    return init1_X3(initi_Fixed(x));
}

iLocalDef iFixed3 initf1_X3(float x) {
    return init1_X3(initf_Fixed(x));
}

iLocalDef iFixed3 init_X3(const iFixed x, const iFixed y, const iFixed z) {
    return (iFixed3){ x, y, z };
}

iLocalDef iFixed3 initi_X3(int x, int y, int z) {
    return init_X3(initi_Fixed(x), initi_Fixed(y), initi_Fixed(z));
}

iLocalDef iFixed3 initf_X3(float x, float y, float z) {
    return init_X3(initf_Fixed(x), initf_Fixed(y), initf_Fixed(z));
}

iLocalDef iFixed3 initv_X3(const iFixed *v) {
    return (iFixed3){ v[0], v[1], v[2] };
}

iLocalDef iFixed3 zero_X3(void) {
    return (iFixed3){ .x = { 0 } };
}

iLocalDef iFixed3 one_X3(void) {
    return init1_X3(one_Fixed());
}

iLocalDef void store_X3(const iFixed3 d, iFixed *p_out) {
    p_out[0] = d.x;
    p_out[1] = d.y;
    p_out[2] = d.z;
}

iLocalDef iFixed3 addX_X3   (const iFixed3 a, const iFixed dx)  { return (iFixed3){ add_Fixed(a.x, dx), a.y, a.z }; }
iLocalDef iFixed3 addY_X3   (const iFixed3 a, const iFixed dy)  { return (iFixed3){ a.x, add_Fixed(a.y, dy), a.z }; }
iLocalDef iFixed3 addZ_X3   (const iFixed3 a, const iFixed dz)  { return (iFixed3){ a.x, a.y, add_Fixed(a.z, dz) }; }

iLocalDef iFixed3 add_X3    (const iFixed3 a, const iFixed3 b)  { return (iFixed3){ add_Fixed(a.x, b.x), add_Fixed(a.y, b.y), add_Fixed(a.z, b.z) }; }
iLocalDef iFixed3 addi_X3   (const iFixed3 a, int b)            { return add_X3(a, initi1_X3(b)); }
iLocalDef iFixed3 addf_X3   (const iFixed3 a, float b)          { return add_X3(a, initf1_X3(b)); }
iLocalDef iFixed3 sub_X3    (const iFixed3 a, const iFixed3 b)  { return (iFixed3){ sub_Fixed(a.x, b.x), sub_Fixed(a.y, b.y), sub_Fixed(a.z, b.z) }; }
iLocalDef iFixed3 subi_X3   (const iFixed3 a, int b)            { return sub_X3(a, initi1_X3(b)); }
iLocalDef iFixed3 subf_X3   (const iFixed3 a, float b)          { return sub_X3(a, initf1_X3(b)); }
iLocalDef iFixed3 mul_X3    (const iFixed3 a, const iFixed3 b)  { return (iFixed3){ mul_Fixed(a.x, b.x), mul_Fixed(a.y, b.y), mul_Fixed(a.z, b.z) }; }
iLocalDef iFixed3 muli_X3   (const iFixed3 a, int b)            { return mul_X3(a, initi1_X3(b)); }
iLocalDef iFixed3 mulf_X3   (const iFixed3 a, float b)          { return mul_X3(a, initf1_X3(b)); }
iLocalDef iFixed3 div_X3    (const iFixed3 a, const iFixed3 b)  { return (iFixed3){ div_Fixed(a.x, b.x), div_Fixed(a.y, b.y), div_Fixed(a.z, b.z) }; }
iLocalDef iFixed3 divi_X3   (const iFixed3 a, int b)            { return div_X3(a, initi1_X3(b)); }
iLocalDef iFixed3 divf_X3   (const iFixed3 a, float b)          { return div_X3(a, initf1_X3(b)); }

iLocalDef iFixed3 addv_X3   (iFixed3 *a, const iFixed3 b)       { addv_Fixed(&a->x, b.x); addv_Fixed(&a->y, b.y); addv_Fixed(&a->z, b.z); return *a; }
iLocalDef iFixed3 addiv_X3  (iFixed3 *a, int b)                 { return addv_X3(a, initi1_X3(b)); }
iLocalDef iFixed3 addfv_X3  (iFixed3 *a, float b)               { return addv_X3(a, initf1_X3(b)); }
iLocalDef iFixed3 subv_X3   (iFixed3 *a, const iFixed3 b)       { subv_Fixed(&a->x, b.x); subv_Fixed(&a->y, b.y); subv_Fixed(&a->z, b.z); return *a; }
iLocalDef iFixed3 subiv_X3  (iFixed3 *a, int b)                 { return subv_X3(a, initi1_X3(b)); }
iLocalDef iFixed3 subfv_X3  (iFixed3 *a, float b)               { return subv_X3(a, initf1_X3(b)); }
iLocalDef iFixed3 mulv_X3   (iFixed3 *a, const iFixed3 b)       { mulv_Fixed(&a->x, b.x); mulv_Fixed(&a->y, b.y); mulv_Fixed(&a->z, b.z); return *a; }
iLocalDef iFixed3 muliv_X3  (iFixed3 *a, int b)                 { return mulv_X3(a, initi1_X3(b)); }
iLocalDef iFixed3 mulfv_X3  (iFixed3 *a, float b)               { return mulv_X3(a, initf1_X3(b)); }
iLocalDef iFixed3 divv_X3   (iFixed3 *a, const iFixed3 b)       { divv_Fixed(&a->x, b.x); divv_Fixed(&a->y, b.y); divv_Fixed(&a->z, b.z); return *a; }
iLocalDef iFixed3 diviv_X3  (iFixed3 *a, int b)                 { return divv_X3(a, initi1_X3(b)); }
iLocalDef iFixed3 divfv_X3  (iFixed3 *a, float b)               { return divv_X3(a, initf1_X3(b)); }

iLocalDef iFixed3 min_X3    (const iFixed3 a, const iFixed3 b)  { return (iFixed3){ min_Fixed(a.x, b.x), min_Fixed(a.y, b.y), min_Fixed(a.z, b.z) }; }
iLocalDef iFixed3 max_X3    (const iFixed3 a, const iFixed3 b)  { return (iFixed3){ max_Fixed(a.x, b.x), max_Fixed(a.y, b.y), max_Fixed(a.z, b.z) }; }
iLocalDef iFixed3 neg_X3    (const iFixed3 a)                   { return (iFixed3){ neg_Fixed(a.x), neg_Fixed(a.y), neg_Fixed(a.z) }; }
iLocalDef iFixed3 abs_X3    (const iFixed3 a)                   { return (iFixed3){ abs_Fixed(a.x), abs_Fixed(a.y), abs_Fixed(a.z) }; }

iLocalDef iBool isEqual_X3(const iFixed3 a, const iFixed3 b) {
    return value_Fixed(a.x) == value_Fixed(b.x) &&
           value_Fixed(a.y) == value_Fixed(b.y) &&
           value_Fixed(a.z) == value_Fixed(b.z);
}

iLocalDef iBoolv equal_X3(const iFixed3 a, const iFixed3 b) {
    return init3_Boolv(value_Fixed(a.x) == value_Fixed(b.x),
                       value_Fixed(a.y) == value_Fixed(b.y),
                       value_Fixed(a.z) == value_Fixed(b.z));
}

iLocalDef iBoolv notEqual_X3(const iFixed3 a, const iFixed3 b) {
    return init3_Boolv(value_Fixed(a.x) != value_Fixed(b.x),
                       value_Fixed(a.y) != value_Fixed(b.y),
                       value_Fixed(a.z) != value_Fixed(b.z));
}

iLocalDef iBoolv greater_X3 (const iFixed3 a, const iFixed3 b) {
    return init3_Boolv(value_Fixed(a.x) > value_Fixed(b.x),
                       value_Fixed(a.y) > value_Fixed(b.y),
                       value_Fixed(a.z) > value_Fixed(b.z));
}

iLocalDef iBoolv greaterEqual_X3 (const iFixed3 a, const iFixed3 b) {
    return init3_Boolv(value_Fixed(a.x) >= value_Fixed(b.x),
                       value_Fixed(a.y) >= value_Fixed(b.y),
                       value_Fixed(a.z) >= value_Fixed(b.z));
}

iLocalDef iBoolv less_X3 (const iFixed3 a, const iFixed3 b) {
    return init3_Boolv(value_Fixed(a.x) < value_Fixed(b.x),
                       value_Fixed(a.y) < value_Fixed(b.y),
                       value_Fixed(a.z) < value_Fixed(b.z));
}

iLocalDef iBoolv lessEqual_X3 (const iFixed3 a, const iFixed3 b) {
    return init3_Boolv(value_Fixed(a.x) <= value_Fixed(b.x),
                       value_Fixed(a.y) <= value_Fixed(b.y),
                       value_Fixed(a.z) <= value_Fixed(b.z));
}

iLocalDef iFixed3   clamp_X3        (const iFixed3 t, const iFixed3 a, const iFixed3 b) { return min_X3(max_X3(t, a), b); }
iLocalDef iFixed    sum_X3          (const iFixed3 a) { return init_Fixed(value_Fixed(a.x) + value_Fixed(a.y) + value_Fixed(a.z)); }
iLocalDef iFixed    dot_X3          (const iFixed3 a, const iFixed3 b) { return sum_X3(mul_X3(a, b)); }
iLocalDef iFixed    lengthSq_X3     (const iFixed3 a) { return dot_X3(a, a); }
iLocalDef float     lengthf_X3      (const iFixed3 a) { return sqrtf(f32_Fixed(lengthSq_X3(a))); }
iLocalDef iFixed    length_X3       (const iFixed3 a) { return initf_Fixed(lengthf_X3(a)); }
iLocalDef iFixed3   normalize_X3    (const iFixed3 a) { return div_X3(a, init1_X3(length_X3(a))); }

iLocalDef iFixed3 sqrt_X3(const iFixed3 a) {
    return (iFixed3){ initf_Fixed(sqrtf(f32_Fixed(a.x))),
                      initf_Fixed(sqrtf(f32_Fixed(a.y))),
                      initf_Fixed(sqrtf(f32_Fixed(a.z)))};
}

iLocalDef iFixed3 mix_X3(const iFixed3 a, const iFixed3 b, const iFixed t) {
    return add_X3(a, mul_X3(sub_X3(b, a), init1_X3(t)));
}
