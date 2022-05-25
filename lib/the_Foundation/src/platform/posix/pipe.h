#pragma once

/** @file posix/pipe.h  Pipe.

@authors Copyright (c) 2018 Jaakko Keränen <jaakko.keranen@iki.fi>

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

#include "the_Foundation/defs.h"

iBeginPublic

iDeclareType(Pipe)

struct Impl_Pipe {
    int fds[2];
};

iDeclareTypeConstruction(Pipe)

iLocalDef int input_Pipe  (const iPipe *d) { return d->fds[1]; }
iLocalDef int output_Pipe (const iPipe *d) { return d->fds[0]; }

size_t  write_Pipe  (const iPipe *, const void *data, size_t size);
size_t  read_Pipe   (const iPipe *, size_t size, void *data_out);

iLocalDef void writeByte_Pipe(const iPipe *d, uint8_t value) {
    write_Pipe(d, &(char){ (char) value }, 1);
}

iLocalDef uint8_t readByte_Pipe(const iPipe *d) {
    uint8_t value = 0;
    read_Pipe(d, 1, &value);
    return value;
}

iEndPublic
