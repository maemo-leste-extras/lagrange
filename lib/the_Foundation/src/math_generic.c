/** @file math_generic.c  Vector math, generic implementation.

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

void init_Mat4(iMat4 *d) {
    d->col[0] = init_F4(1, 0, 0, 0);
    d->col[1] = init_F4(0, 1, 0, 0);
    d->col[2] = init_F4(0, 0, 1, 0);
    d->col[3] = init_F4(0, 0, 0, 1);
}

void store_Mat4(const iMat4 *d, float *v) {
    store_F4(d->col[0], v);
    store_F4(d->col[1], v + 4);
    store_F4(d->col[2], v + 8);
    store_F4(d->col[3], v + 12);
}

void load_Mat4(iMat4 *d, const float *v) {
    d->col[0] = initv_F4(v);
    d->col[1] = initv_F4(v + 4);
    d->col[2] = initv_F4(v + 8);
    d->col[3] = initv_F4(v + 12);
}

void mul_Mat4(iMat4 *d, const iMat4 *right) {
    iMat4 result;
    for (int i = 0; i < 4; ++i) {
        const iFloat4 rCol = right->col[i];
        result.col[i]         = mul_F4(d->col[0], init1_F4(x_F4(rCol)));
        addv_F4(&result.col[i], mul_F4(d->col[1], init1_F4(y_F4(rCol))));
        addv_F4(&result.col[i], mul_F4(d->col[2], init1_F4(z_F4(rCol))));
        addv_F4(&result.col[i], mul_F4(d->col[3], init1_F4(w_F4(rCol))));
    }
    copy_Mat4(d, &result);
}

void transpose_Mat4(iMat4 *d) {
    const iFloat4 rows[4] = {
        row_Mat4(d, 0), row_Mat4(d, 1), row_Mat4(d, 2), row_Mat4(d, 3)
    };
    d->col[0] = rows[0];
    d->col[1] = rows[1];
    d->col[2] = rows[2];
    d->col[3] = rows[3];
}

void initRotate_Mat4(iMat4 *d, iFloat3 axis, float degrees) {
    const float   ang      = iMathDegreeToRadianf(degrees);
    const float   c        = cosf(ang);
    const float   s        = sinf(ang);
    const iFloat3 normAxis = normalize_F3(axis);
    const float * av       = normAxis.v;
    const iFloat4 omc      = init1_F4(1 - c);
    d->col[0] = mul_F4(omc, init_F4(av[0] * av[0], av[0] * av[1], av[0] * av[2], 0.0f));
    d->col[1] = mul_F4(omc, init_F4(av[1] * av[0], av[1] * av[1], av[1] * av[2], 0.0f));
    d->col[2] = mul_F4(omc, init_F4(av[2] * av[0], av[2] * av[1], av[2] * av[2], 0.0f));
    addv_F4(&d->col[0], init_F4(+c,       +av[2]*s,   -av[1]*s,   0));
    addv_F4(&d->col[1], init_F4(-av[2]*s, +c,         +av[0]*s,   0));
    addv_F4(&d->col[2], init_F4(+av[1]*s, -av[0]*s,   +c,         0));
    d->col[3] = init_F4(0, 0, 0, 1);
}

void store_Mat3(const iMat3 *d, float *v9) {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            v9[3*i + j] = d->col[i].v[j];
        }
    }
}
