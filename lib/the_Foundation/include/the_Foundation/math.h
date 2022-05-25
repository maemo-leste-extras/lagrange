#pragma once

/** @file the_Foundation/math.h  Math routines and constants.

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

#include "defs.h"
#include "random.h"
#include "stream.h"

iBeginPublic

#define iMathPi                 3.14159265358979323846
#define iMathPif                3.14159265358979323846f
#define iMathDegreeToRadianf(v) ((v) * iMathPif / 180.f)
#define iMathRadianToDegreef(v) ((v) * 180.f / iMathPif)

int         iRound  (float value);
int16_t     iRound16(float value);
int         iWrap   (int value, int low, int high); /* `high` is not inclusive */
float       iWrapf  (float value, float low, float high); /* `high` is not inclusive */

iDeclareType(FloatVec3)
iDeclareType(FloatVec4)

struct Impl_FloatVec3 {
    union {
        float v[3];
        struct {
            float x;
            float y;
            float z;
        } value;
    };
};

struct Impl_FloatVec4 {
    union {
        float v[4];
        struct {
            float x;
            float y;
            float z;
            float w;
        } value;
    };
};

#if defined (iHaveSSE4_1)
#   include "math_sse.h"
#else
#   include "math_generic.h"
#endif

#define initI2_F3(v2)     initiv2_F3(&(v2).x)
#define initF3_I2(f3)     init_I2((int) x_F3(f3), (int) y_F3(f3))

iLocalDef void writeFloat3_Stream(iStream *d, const iFloat3 vec) {
    writef_Stream(d, x_F3(vec));
    writef_Stream(d, y_F3(vec));
    writef_Stream(d, z_F3(vec));
}

iLocalDef iFloat3 readFloat3_Stream(iStream *d) {
    const float v[3] = { readf_Stream(d), readf_Stream(d), readf_Stream(d) };
    return initv_F3(v);
}

iBool   inverse_Mat3    (const iMat3 *d, iMat3 *inversed_out);
iBool   inverse_Mat4    (const iMat4 *d, iMat4 *inversed_out);

void    ortho_Mat4      (iMat4 *, float left, float right, float top, float bottom, float znear, float zfar);
void    perspective_Mat4(iMat4 *, float xFovDeg, float aspect, float znear, float zfar);
void    frame_Mat4      (iMat4 *, iFloat3 front, iFloat3 up, iBool mirror);
void    lookAt_Mat4     (iMat4 *, iFloat3 target, iFloat3 eyePos, iFloat3 up);

#include "vec2.h"

iEndPublic
