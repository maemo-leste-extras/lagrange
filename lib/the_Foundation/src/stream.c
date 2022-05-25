/** @file stream.c

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

#include "the_Foundation/stream.h"
#include "the_Foundation/block.h"
#include "the_Foundation/mutex.h"
#include "the_Foundation/stringlist.h"
#include "the_Foundation/buffer.h"

iDefineClass(Stream)

#define class_Stream(d)         ((const iStreamClass *) (d)->object.classObj)

iDeclareType(ByteOrder)

struct Impl_ByteOrder {
    uint16_t (*order16)(uint16_t);
    uint32_t (*order32)(uint32_t);
    uint64_t (*order64)(uint64_t);
};

iLocalDef uint16_t nop16_(uint16_t v) { return v; }
iLocalDef uint32_t nop32_(uint32_t v) { return v; }
iLocalDef uint64_t nop64_(uint64_t v) { return v; }

iLocalDef uint16_t swap16_(uint16_t v) { return (uint16_t) ((v >> 8) | ((v & 0xff) << 8)); }
iLocalDef uint32_t swap32_(uint32_t v) { return (v >> 24) | ((v & 0xff0000) >> 8) | ((v & 0xff00) << 8) | ((v & 0xff) << 24); }
iLocalDef uint64_t swap64_(uint64_t v) { return swap32_(v >> 32) | ((uint64_t) (swap32_(v & 0xffffffff)) << 32); }

#if defined (iBigEndian)
static uint16_t order16le_(uint16_t v) { return swap16_(v); }
static uint32_t order32le_(uint32_t v) { return swap32_(v); }
static uint64_t order64le_(uint64_t v) { return swap64_(v); }

static uint16_t order16be_(uint16_t v) { return nop16_(v); }
static uint32_t order32be_(uint32_t v) { return nop32_(v); }
static uint64_t order64be_(uint64_t v) { return nop64_(v); }
#else
static uint16_t order16le_(uint16_t v) { return nop16_(v); }
static uint32_t order32le_(uint32_t v) { return nop32_(v); }
static uint64_t order64le_(uint64_t v) { return nop64_(v); }

static uint16_t order16be_(uint16_t v) { return swap16_(v); }
static uint32_t order32be_(uint32_t v) { return swap32_(v); }
static uint64_t order64be_(uint64_t v) { return swap64_(v); }
#endif

static const iByteOrder byteOrder_[2] = {
    /* Little-endian */ { .order16 = order16le_, .order32 = order32le_, .order64 = order64le_ },
    /* Big-endian */    { .order16 = order16be_, .order32 = order32be_, .order64 = order64be_ }
};

#define ord_Stream(d)   (byteOrder_[(d)->flags & bigEndianByteOrder_StreamFlag])

enum iStreamFlags {
    bigEndianByteOrder_StreamFlag = 1,
    versionMask_StreamFlag        = 0xfff00,
    versionShift_StreamFlag       = 8,
};

void init_Stream(iStream *d) {
    d->size = 0;
    d->pos = 0;
    d->flags = 0; // little-endian
    d->mtx = new_Mutex();
}

void deinit_Stream(iStream *d) {
    delete_Mutex(d->mtx);
}

void setByteOrder_Stream(iStream *d, enum iStreamByteOrder byteOrder) {
    iChangeFlags(d->flags, bigEndianByteOrder_StreamFlag, byteOrder == bigEndian_StreamByteOrder);
}

void setVersion_Stream(iStream *d, int version) {
    d->flags &= ~versionMask_StreamFlag;
    d->flags |= (version << versionShift_StreamFlag) & versionMask_StreamFlag;
}

void setSize_Stream(iStream *d, size_t size) {
    iGuardMutex(d->mtx, {
        d->size = size;
        d->pos = iMin(d->pos, size);
    });
}

enum iStreamByteOrder byteOrder_Stream(const iStream *d) {
    return d->flags & bigEndianByteOrder_StreamFlag ? bigEndian_StreamByteOrder
                                                    : littleEndian_StreamByteOrder;
}

int version_Stream(const iStream *d) {
    return (d->flags & versionMask_StreamFlag) >> versionShift_StreamFlag;
}

void seek_Stream(iStream *d, size_t offset) {
    iGuardMutex(d->mtx, d->pos = class_Stream(d)->seek(d, offset));
}

iBlock *read_Stream(iStream *d, size_t size) {
    iBlock *data = new_Block(0);
    readBlock_Stream(d, size, data);
    return data;
}

size_t readData_Stream(iStream *d, size_t size, void *data_out) {
    size_t readSize = 0;
    iGuardMutex(d->mtx, {
        readSize = class_Stream(d)->read(d, size, data_out);
        d->pos += readSize;
        d->size = iMax(d->size, d->pos); // update successfully read size
    });
    return readSize;
}

size_t readBlock_Stream(iStream *d, size_t size, iBlock *data_out) {
    resize_Block(data_out, size);
    const size_t readSize = readData_Stream(d, size, data_Block(data_out));
    truncate_Block(data_out, readSize);
    return readSize;
}

iBlock *readAll_Stream(iStream *d) {
    iBlock *data = new_Block(0);
    iBlock *chunk = new_Block(0);
    for (;;) {
        size_t readSize = readBlock_Stream(d, 128 * 1024, chunk);
        if (!readSize) break;
        append_Block(data, chunk);
    }
    delete_Block(chunk);
    return data;
}

size_t write_Stream(iStream *d, const iBlock *data) {
    return writeData_Stream(d, constData_Block(data), size_Block(data));
}

size_t writeData_Stream(iStream *d, const void *data, size_t size) {
    size_t n = 0;
    iGuardMutex(d->mtx, {
        n = class_Stream(d)->write(d, data, size);
        d->pos += n;
        d->size = iMax(d->pos, d->size);
    });
    return n;
}

size_t writeBuffer_Stream(iStream *d, const iBuffer *buf) {
    return write_Stream(d, data_Buffer(buf));
}

iStringList *readLines_Stream(iStream *d) {
    iBlock *data = readAll_Stream(d);
    iStringList *lines = split_String((iString *) data, "\n");
    delete_Block(data);
    return lines;
}

size_t printf_Stream(iStream *d, const char *format, ...) {
    va_list args;
    va_start(args, format);
    iBlock str;
    init_Block(&str, 0);
    vprintf_Block(&str, format, args);
    const size_t len = write_Stream(d, &str);
    deinit_Block(&str);
    va_end(args);
    return len;
}

void flush_Stream(iStream *d) {
    class_Stream(d)->flush(d);
}

iString *readString_Stream(iStream *d) {
    iBlock *chars = readAll_Stream(d);
    iString *str = newBlock_String(chars);
    delete_Block(chars);
    return str;
}

size_t writeObject_Stream(iStream *d, const iAnyObject *object) {
    iAssert(class_Object(object)->serialize != NULL);
    const size_t start = d->pos;
    class_Object(object)->serialize(object, d);
    iAssert(d->pos >= start);
    return d->pos - start;
}

iAnyObject *readObject_Stream(iStream *d, const iAnyClass *class) {
    const iClass *cls = (const iClass *) class;
    iAssert(cls->newObject);
    iAssert(cls->deserialize);
    iAnyObject *obj = cls->newObject();
    cls->deserialize(obj, d);
    return obj;
}

void write16_Stream(iStream *d, int16_t value) {
    const uint16_t data = ord_Stream(d).order16(value);
    writeData_Stream(d, &data, 2);
}

void write32_Stream(iStream *d, int32_t value) {
    const uint32_t data = ord_Stream(d).order32(value);
    writeData_Stream(d, &data, 4);
}

void write64_Stream(iStream *d, int64_t value) {
    const uint64_t data = ord_Stream(d).order64(value);
    writeData_Stream(d, &data, 8);
}

int8_t read8_Stream(iStream *d) {
    int8_t value = 0;
    readData_Stream(d, 1, &value);
    return value;
}

int16_t read16_Stream(iStream *d) {
    uint16_t data = 0;
    readData_Stream(d, 2, &data);
    return ord_Stream(d).order16(data);
}

int32_t read32_Stream(iStream *d) {
    uint32_t data = 0;
    readData_Stream(d, 4, &data);
    return ord_Stream(d).order32(data);
}

int64_t read64_Stream(iStream *d) {
    uint64_t data = 0;
    readData_Stream(d, 8, &data);
    return ord_Stream(d).order64(data);
}
