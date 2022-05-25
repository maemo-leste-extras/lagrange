#pragma once

/** @file the_Foundation/range.h  Numeric ranges.

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

#include "random.h"

iDeclareType(Rangei)
iDeclareType(Rangeui)
iDeclareType(Ranges)
iDeclareType(Rangef)
iDeclareType(Ranged)
iDeclareType(Rangecc)

struct Impl_Rangei {
    int start;
    int end;
};

struct Impl_Rangeui {
    unsigned int start;
    unsigned int end;
};

struct Impl_Ranges {
    size_t start;
    size_t end;
};

struct Impl_Rangef {
    float start;
    float end;
};

struct Impl_Ranged {
    double start;
    double end;
};

struct Impl_Rangecc {
    const char *start;
    const char *end;
};

#define iNullRange                  (iRangecc){ NULL, NULL }

#define size_Range(d)               ((size_t) ((d)->end - (d)->start))
#define isEmpty_Range(d)            ((d)->end == (d)->start)
#define contains_Range(d, value)    ((value) >= (d)->start && (value) < (d)->end)

#define shift_Range(d, delta)       {(d)->start += (delta); (d)->end += (delta);}
#define setSize_Range(d, ns)        {(d)->end = (d)->start + (ns);}

iLocalDef float random_Rangef(iRangef range) {
    return range.start + iRandomf() * (range.end - range.start);
}
