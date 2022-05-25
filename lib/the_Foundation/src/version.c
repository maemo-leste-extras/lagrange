/** @file version.c  Version numbers.

@authors Copyright (c) 2017-2021 Jaakko Keränen <jaakko.keranen@iki.fi>

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

#include "the_Foundation/version.h"

const iVersion version_Foundation = iFoundationLibraryVersion;

iBool init_Version(iVersion *d, iRangecc text) {
    iZap(*d);
    int      i   = 0;
    int     *dst = &d->major;
    iRangecc seg = iNullRange;
    iString  segStr;
    iBool    ok = iTrue;
    init_String(&segStr);
    while (nextSplit_Rangecc(text, ".", &seg)) {
        if (i < 3) {
            setRange_String(&segStr, seg);
            dst[i] = toInt_String(&segStr);
            if (dst[i] < 0) {
                dst[i] = 0;
                ok = iFalse;
            }
        }
        i++;
    }
    deinit_String(&segStr);
    return ok && i > 0 && i <= 3;
}

int cmp_Version(const iVersion *d, const iVersion *other) {
    for (int i = 0; i < 3; ++i) {
        const int cmp = iCmp((&d->major)[i], (&other->major)[i]);
        if (cmp) return cmp;
    }
    return 0;
}
