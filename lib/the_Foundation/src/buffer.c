/** @file buffer.c  Memory stream.

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

#include "the_Foundation/buffer.h"
#include "the_Foundation/string.h"

static iBufferClass Class_Buffer;

iDefineObjectConstruction(Buffer)

void init_Buffer(iBuffer *d) {
    iAssertIsObject(d);
    init_Stream(&d->stream);
    init_Block(&d->block, 0);
    d->data = NULL;
    d->mode = readWrite_BufferMode;
}

void deinit_Buffer(iBuffer *d) {
    deinit_Block(&d->block);
}

iBool open_Buffer(iBuffer *d, const iBlock *data) {
    if (isOpen_Buffer(d) || !data) return iFalse;
    set_Block(&d->block, data); // this copy of the data will be accessed
    d->data = &d->block;
    d->mode = readOnly_BufferMode;
    setSize_Stream(&d->stream, size_Block(data));
    return iTrue;
}

iBool openData_Buffer(iBuffer *d, iBlock *data) {
    if (isOpen_Buffer(d) || !data) return iFalse;
    clear_Block(&d->block);
    d->data = data;
    d->mode = readWrite_BufferMode;
    setSize_Stream(&d->stream, size_Block(data));
    return iTrue;
}

iBool openEmpty_Buffer(iBuffer *d) {
    if (isOpen_Buffer(d)) return iFalse;
    clear_Block(&d->block);
    d->data = &d->block;
    d->mode = readWrite_BufferMode;
    setSize_Stream(&d->stream, 0);
    return iTrue;
}

void close_Buffer(iBuffer *d) {
    if (isOpen_Buffer(d)) {
        d->data = NULL;
        clear_Block(&d->block);
    }
}

void clear_Buffer(iBuffer *d) {
    clear_Block(&d->block);
    if (isOpen_Buffer(d)) {
        clear_Block(d->data);
        setSize_Stream(&d->stream, 0);
    }
}

static size_t seek_Buffer_(iBuffer *d, size_t offset) {
    if (isOpen_Buffer(d)) {
        return iMin(offset, size_Block(d->data));
    }
    return pos_Stream(&d->stream);
}

static size_t read_Buffer_(iBuffer *d, size_t size, void *data_out) {
    if (isOpen_Buffer(d)) {
        if (atEnd_Buffer(d)) return 0;
        const iRanges range = { pos_Buffer(d), iMin(pos_Buffer(d) + size, size_Block(d->data)) };
        memcpy(data_out, constBegin_Block(d->data) + range.start, size_Range(&range));
        return size_Range(&range);
    }
    return 0;
}

static size_t write_Buffer_(iBuffer *d, const void *data, size_t size) {
    if (isOpen_Buffer(d) && (~d->mode & readOnly_BufferMode)) {
        setSubData_Block(d->data, pos_Buffer(d), data, size);
        return size;
    }
    return 0;
}

static void flush_Buffer_(iBuffer *d) {
    iUnused(d);
}

const iBlock *data_Buffer(const iBuffer *d) {
    return d->data;
}

size_t consume_Buffer(iBuffer *d, size_t size, void *data_out) {
    iAssert(~d->mode & readOnly_BufferMode);
    size_t consumedSize;
    iGuardMutex(d->stream.mtx, {
        size_t spos = pos_Stream(&d->stream);
        rewind_Buffer(d);
        consumedSize = readData_Buffer(d, size, data_out);
        remove_Block(d->data, 0, consumedSize);
        setSize_Stream(&d->stream, size_Block(d->data));
        d->stream.pos = spos - consumedSize;
    });
    return consumedSize;
}

iBlock *consumeBlock_Buffer(iBuffer *d, size_t size) {
    iBlock *consumed = new_Block(size);
    const size_t count = consume_Buffer(d, size, data_Block(consumed));
    truncate_Block(consumed, count);
    return consumed;
}

iBlock *consumeAll_Buffer(iBuffer *d) {
    iAssert(~d->mode & readOnly_BufferMode);
    iBlock *data;
    iGuardMutex(d->stream.mtx, {
        rewind_Buffer(d);
        data = readAll_Buffer(d);
        clear_Buffer(d);
    });
    return data;
}

static iBeginDefineSubclass(Buffer, Stream)
    .seek   = (size_t (*)(iStream *, size_t))               seek_Buffer_,
    .read   = (size_t (*)(iStream *, size_t, void *))       read_Buffer_,
    .write  = (size_t (*)(iStream *, const void *, size_t)) write_Buffer_,
    .flush  = (void   (*)(iStream *))                       flush_Buffer_,
iEndDefineClass(Buffer)
