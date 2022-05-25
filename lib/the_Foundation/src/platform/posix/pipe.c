/** @file posix/pipe.c  Pipe.

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

#include "pipe.h"
#include <unistd.h>
#include <errno.h>

iDefineTypeConstruction(Pipe)

void init_Pipe(iPipe *d) {
    if (pipe(d->fds)) {
        iWarning("pipe: (%i) %s\n", errno, strerror(errno));
        d->fds[0] = d->fds[1] = -1;
    }
}

void deinit_Pipe(iPipe *d) {
    close(d->fds[0]);
    close(d->fds[1]);
}

size_t write_Pipe(const iPipe *d, const void *data, size_t size) {
    return write(input_Pipe(d), data, size);
}

size_t read_Pipe(const iPipe *d, size_t size, void *data_out) {
    return read(output_Pipe(d), data_out, size);
}
