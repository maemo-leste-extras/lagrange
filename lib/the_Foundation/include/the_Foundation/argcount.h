#pragma once

/** @file the_Foundation/argcount.h  Macro for counting arguments.

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

#define iArgCountSeq        30, \
                            29, 28, 27, 26, 25, 24, 23, 22, 21, 20, \
                            19, 18, 17, 16, 15, 14, 13, 12, 11, 10 \
                             9,  8,  7,  6,  5,  4,  3,  2,  1,  0

#define iArgCountN_(_01, _02, _03, _04, _05, _06, _07, _08, _09, _10, \
                    _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                    _21, _22, _23, _24, _25, _26, _27, _28, _29,   N, ...) N

#define iArgCount_(...)     iArgCountN_(__VA_ARGS__)
#define iArgCount(...)      iArgCount_(__VA_ARGS__, iArgCountSeq)
