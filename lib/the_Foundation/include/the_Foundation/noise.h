#pragma once

/** @file the_Foundation/noise.h  2D Perlin noise.

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

#include "math.h"

/* Forward declarations */
iDeclareType(Stream)

iDeclareType(Noise)
iDeclareTypeConstructionArgs(Noise, iInt2 size)
iDeclareTypeSerialization(Noise)

float   eval_Noise  (const iNoise *, float normX, float normY);

/*-----------------------------------------------------------------------------------------------*/

iDeclareType(NoiseComponent)

struct Impl_NoiseComponent {
    iInt2 size;
    float weight;
    float offset;
};

iDeclareType(CombinedNoise)
iDeclareTypeConstructionArgs(CombinedNoise, const iNoiseComponent *components, size_t count)
iDeclareTypeSerialization(CombinedNoise)

float       eval_CombinedNoise          (const iCombinedNoise *, float normX, float normY);
iFloat3     randomCoord_CombinedNoise   (const iCombinedNoise *, iBool (*rangeCheck)(float));

void        setOffset_CombinedNoise     (iCombinedNoise *, size_t index, float offset);
void        setPointOffset_CombinedNoise(iCombinedNoise *, float normX, float normY, float offset);

