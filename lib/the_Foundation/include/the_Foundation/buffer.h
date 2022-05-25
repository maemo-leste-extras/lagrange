#pragma once

/** @file the_Foundation/buffer.h  Memory stream.

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

#include "stream.h"
#include "block.h"
#include "mutex.h"

#include <stdio.h>
#include "stdthreads.h"

iBeginPublic

typedef iStreamClass iBufferClass;

iDeclareType(Buffer)
iDeclareType(String)

enum iBufferMode {
    readWrite_BufferMode = 0,
    readOnly_BufferMode  = 0x1,
};

struct Impl_Buffer {
    iStream stream;
    iBlock block;
    iBlock *data;
    enum iBufferMode mode;
};

iDeclareObjectConstruction(Buffer)

iBool       open_Buffer     (iBuffer *, const iBlock *data);
iBool       openData_Buffer (iBuffer *, iBlock *data);
iBool       openEmpty_Buffer(iBuffer *);
void        close_Buffer    (iBuffer *);

const iBlock *data_Buffer   (const iBuffer *);

iLocalDef iStream *     stream_Buffer   (iBuffer *d) { return &d->stream; }

void        clear_Buffer    (iBuffer *);

iLocalDef iBool  isOpen_Buffer (const iBuffer *d) { return d->data != NULL; }
iLocalDef int    mode_Buffer   (const iBuffer *d) { return d->mode ;}
iLocalDef size_t pos_Buffer    (const iBuffer *d) { return pos_Stream(&d->stream); }
iLocalDef size_t size_Buffer   (const iBuffer *d) { return size_Stream(&d->stream); }
iLocalDef iBool  isEmpty_Buffer(const iBuffer *d) { return size_Buffer(d) == 0; }
iLocalDef iBool  atEnd_Buffer  (const iBuffer *d) { return atEnd_Stream(&(d)->stream); }

iLocalDef void          rewind_Buffer       (iBuffer *d) { seek_Stream(&d->stream, 0); }
iLocalDef void          seek_Buffer         (iBuffer *d, size_t offset) { seek_Stream(&d->stream, offset); }
iLocalDef void          write32_Buffer      (iBuffer *d, int32_t value) { write32_Stream(&d->stream, value); }
iLocalDef size_t        writeData_Buffer    (iBuffer *d, const void *data, size_t size) { return writeData_Stream(&d->stream, data, size); }

iLocalDef size_t        readData_Buffer     (iBuffer *d, size_t size, void *data_out) { return readData_Stream(&d->stream, size, data_out); }
iLocalDef iBlock *      readAll_Buffer      (iBuffer *d) { return readAll_Stream(&d->stream); }
iLocalDef iString *     readString_Buffer   (iBuffer *d) { return readString_Stream(&d->stream); }
iLocalDef iStringList * readLines_Buffer    (iBuffer *d) { return readLines_Stream(&d->stream); }

size_t      consume_Buffer      (iBuffer *d, size_t size, void *data_out);
iBlock *    consumeBlock_Buffer (iBuffer *d, size_t size);
iBlock *    consumeAll_Buffer   (iBuffer *d);

iEndPublic
