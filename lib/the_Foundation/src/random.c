/** @file the_Foundation/random.c  Random number generators.

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

#include "the_Foundation/random.h"
#include "the_Foundation/time.h"
#include "the_Foundation/atomic.h"

#include <stdlib.h>

#define iRandMax (1u << 24)

static uint32_t rand24_(void) {
    /* For reference, see: https://en.wikipedia.org/wiki/Linear_congruential_generator */
    static atomic_uint seed;
    static iBool inited = iFalse;
    const uint32_t multiplier = 1103515245;
    const uint32_t increment = 12345;
    const uint32_t modulus = 0x7fffffff;
    if (!inited) {
        inited = iTrue;
        const iTime now = now_Time();
        const uint32_t val = (uint32_t) (nanoSeconds_Time(&now) ^ (integralSeconds_Time(&now) % 1000));
        iDebug("[the_Foundation] random seed: %ld\n", val);
        set_Atomic(&seed, val & modulus);
    }
    return (exchange_Atomic(&seed, (multiplier * value_Atomic(&seed) + increment) & modulus) >> 6) &
           (iRandMax - 1);
}

float iRandomf(void) {
    return (float) rand24_() / (float) iRandMax;
}

int iRandom(int start, int end) {
    if (end <= start) return start;
    const unsigned range = end - start;
    if (range >= iRandMax) {
        return (int) (start + iRandomf() * range);
    }
    return start + rand24_() % range;
}

unsigned iRandomu(unsigned start, unsigned end) {
    if (end <= start) return start;
    const unsigned range = end - start;
    if (range >= iRandMax) {
        return (unsigned) (start + iRandomf() * range);
    }
    return start + rand24_() % range;
}

size_t iRandoms(size_t start, size_t end) {
    if (end <= start) return start;
    const size_t range = end - start;
    if (range >= iRandMax) {
        return (size_t) (start + iRandomf() * range);
    }
    return start + rand24_() % range;
}
