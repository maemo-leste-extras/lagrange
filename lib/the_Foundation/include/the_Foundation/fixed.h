#pragma once

/** @file fixed.h  Fixed-point 48.16 math routines.

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

#include "defs.h"
#include "random.h"

#include <math.h>

iDeclareType(Fixed)

typedef int64_t     iFixed64;
#if defined(__SIZEOF_INT128__)
typedef __int128_t  iFixedLong;
#else
typedef int64_t     iFixedLong; /* oops, will overflow */
#endif

struct Impl_Fixed {
    union {
        iFixed64 v;
        struct {
            uint64_t frac : 16;
            uint64_t wnum : 47;
            uint64_t sign : 1;
        } comp;
    };
};

#define iFixedFracBits  16
#define iFixedUnit      (1 << iFixedFracBits)
#define iFixedMaxWNum   ((((iFixed64) 1) << 47) - 1)

iLocalDef iFixed zero_Fixed(void) {
    return (iFixed){ .v = 0 };
}

iLocalDef iFixed one_Fixed(void) {
    return (iFixed){ .v = iFixedUnit };
}

iLocalDef iFixed half_Fixed(void) {
    return (iFixed){ .v = iFixedUnit >> 1 };
}

iLocalDef iFixed init_Fixed(iFixed64 fp) {
    return (iFixed){ .v = fp };
}

iLocalDef iFixed initi_Fixed(int32_t i) {
    return (iFixed){ .v = i << iFixedFracBits };
}

iLocalDef iFixed initf_Fixed(float f) {
    return (iFixed){ .v = (iFixed64) (f * iFixedUnit) };
}

iLocalDef iFixed initd_Fixed(double d) {
    return (iFixed){ .v = (iFixed64) (d * iFixedUnit) };
}

iLocalDef iFixed add_Fixed(const iFixed a, const iFixed b) {
    return (iFixed){ .v = a.v + b.v };
}

iLocalDef void addv_Fixed(iFixed *a, const iFixed b) {
    a->v += b.v;
}

iLocalDef iFixed sub_Fixed(const iFixed a, const iFixed b) {
    return (iFixed){ .v = a.v - b.v };
}

iLocalDef void subv_Fixed(iFixed *a, const iFixed b) {
    a->v -= b.v;
}

iLocalDef iFixed mul_Fixed(const iFixed a, const iFixed b) {
    return init_Fixed((((iFixedLong) a.v * (iFixedLong) b.v) >> iFixedFracBits));
}

iLocalDef void mulv_Fixed(iFixed *a, const iFixed b) {
    *a = mul_Fixed(*a, b);
}

iLocalDef iFixed muli_Fixed(const iFixed a, int i) {
    return init_Fixed(a.v * i);
}

iLocalDef iFixed mulf_Fixed(const iFixed a, float f) {
    return init_Fixed((iFixed64) (a.v * f));
}

iLocalDef iFixed div_Fixed(const iFixed a, const iFixed b) {
    return init_Fixed(((iFixedLong) a.v << iFixedFracBits) / (iFixedLong) b.v);
}

iLocalDef void divv_Fixed(iFixed *a, const iFixed b) {
    *a = div_Fixed(*a, b);
}

iLocalDef iFixed divi_Fixed(const iFixed a, int i) {
    return div_Fixed(a, initi_Fixed(i));
}

iLocalDef iFixed divf_Fixed(const iFixed a, float f) {
    return div_Fixed(a, initf_Fixed(f));
}

iLocalDef iFixed64 value_Fixed(const iFixed a) {
    return a.v;
}

iLocalDef int64_t i64_Fixed(const iFixed a) {
    return a.v >> iFixedFracBits;
}

iLocalDef int32_t i32_Fixed(const iFixed a) {
    return (int32_t) a.v >> iFixedFracBits;
}

iLocalDef double f64_Fixed(const iFixed a) {
    return (double) a.v / iFixedUnit;
}

iLocalDef float f32_Fixed(const iFixed a) {
    return (float) f64_Fixed(a);
}

iLocalDef iFixed min_Fixed(const iFixed a, const iFixed b)  { return init_Fixed(iMin(a.v, b.v)); }
iLocalDef iFixed max_Fixed(const iFixed a, const iFixed b)  { return init_Fixed(iMax(a.v, b.v)); }
iLocalDef iFixed neg_Fixed(const iFixed a)                  { return init_Fixed(-a.v); }
iLocalDef iFixed abs_Fixed(const iFixed a)                  { if (a.comp.sign) return neg_Fixed(a); else return a; }

iLocalDef iFixed mix_Fixed(const iFixed a, const iFixed b, const iFixed t) {
    return add_Fixed(a, mul_Fixed(sub_Fixed(b, a), t));
}

iLocalDef iFixed random_Fixed(void) {
    return initf_Fixed(iRandomf());
}
