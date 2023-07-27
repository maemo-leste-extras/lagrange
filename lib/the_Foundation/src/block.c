/** @file block.c  Byte array with copy-on-write semantics.

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

#include "the_Foundation/block.h"
#include "the_Foundation/atomic.h"
#include "the_Foundation/garbage.h"
#include "the_Foundation/string.h"
#include "the_Foundation/stream.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <strings.h>
#include <uniconv.h>
#include <unistdio.h>
#if defined (iHaveZlib)
#   include <zlib.h>
#endif

/// @todo Needs a ref-counting mutex.
static iBlockData emptyBlockData = {
    .refCount = 1,
    .data = "",
    .size = 0,
    .allocSize = 1,
};

static iBlockData *new_BlockData_(size_t size, size_t allocSize) {
    iBlockData *d = iMalloc(BlockData);
    set_Atomic(&d->refCount, 1);
    d->size = size;
    d->allocSize = iMax(size + 1, allocSize);
    d->data = malloc(d->allocSize);
    return d;
}

static iBlockData *newPrealloc_BlockData_(void *data, size_t size, size_t allocSize) {
    iBlockData *d = iMalloc(BlockData);
    set_Atomic(&d->refCount, 1);
    d->size = size;
    d->allocSize = allocSize;
    d->data = data;
    return d;
}

static iBlockData *duplicate_BlockData_(const iBlockData *d, size_t allocSize) {
    if (d->size > 1024*1024) {
        iDebug("[BlockData] duplicating %p (size:%zu)\n", d, d->size);
    }
    iBlockData *dupl = new_BlockData_(d->size, allocSize);
    memcpy(dupl->data, d->data, iMin(d->allocSize, dupl->size + 1));
    return dupl;
}

static void deref_BlockData_(iBlockData *d) {
    const int refWas = addRelaxed_Atomic(&d->refCount, -1);
    if (refWas == 1) {
        iAssert(d != &emptyBlockData);
        free(d->data);
        free(d);
    }
}

iLocalDef size_t allocSize_(size_t contentSize) {
    const size_t required = contentSize + 1; /* always null-terminated */
    size_t s = 8;
    while (s < required) {
        s <<= 1;
    }
    return s;
}

static void reserve_BlockData_(iBlockData *d, size_t size) {
    if (d->allocSize >= size + 1) return;
    iAssert(value_Atomic(&d->refCount) == 1);
    iAssert(d->allocSize > 0);
    /* Reserve increased amount of memory in powers-of-two. */
#if defined (iHaveDebugOutput)
    const size_t old = d->allocSize;
#endif
    d->allocSize = allocSize_(size);
    if (d->allocSize > 4000000) {
        /* Large reallocs should be minized. */
        iDebug("[BlockData] reallocating %p from %zu to %zu bytes\n", d->data, old, d->allocSize);
    }
    d->data = realloc(d->data, d->allocSize);
}

static void memcpyFrom_Block_(iBlock *d, const void *data, size_t size) {
    if (size) {
        memcpy(d->i->data, data, size);
        d->i->data[size] = 0;
        d->i->size = size;
    }
}

static void detach_Block_(iBlock *d, size_t allocSize) {
    if (value_Atomic(&d->i->refCount) > 1) {
        iBlockData *detached = duplicate_BlockData_(d->i, allocSize);
        deref_BlockData_(d->i);
        d->i = detached;
    }
    iAssert(value_Atomic(&d->i->refCount) == 1);
}

/*-------------------------------------------------------------------------------------*/

iDefineTypeConstructionArgs(Block, (size_t size), size)

iBlock *newCStr_Block(const char *cstr) {
    iBlock *d = new_Block(strlen(cstr));
    memcpyFrom_Block_(d, cstr, d->i->size);
    return d;
}

iBlock *newData_Block(const void *data, size_t size) {
    iBlock *d = new_Block(size);
    memcpyFrom_Block_(d, data, size);
    return d;
}

iBlock *newPrealloc_Block(void *data, size_t size, size_t allocSize) {
    iBlock *d = iMalloc(Block);
    initPrealloc_Block(d, data, size, allocSize);
    return d;
}

iBlock *copy_Block(const iBlock *d) {
    if (d) {
        iBlock *dupl = malloc(sizeof(iBlock));
        initCopy_Block(dupl, d);
        return dupl;
    }
    return NULL;
}

void init_Block(iBlock *d, size_t size) {
    if (size == 0) {
        d->i = &emptyBlockData;
        addRelaxed_Atomic(&emptyBlockData.refCount, 1);
    }
    else {
        d->i = new_BlockData_(size, 0);
    }
}

void initData_Block(iBlock *d, const void *data, size_t size) {
    if (size > 0) {
        d->i = new_BlockData_(size, 0);
        memcpyFrom_Block_(d, data, size);
    }
    else {
        init_Block(d, size);
    }
}

void initCStr_Block(iBlock *d, const char *cstr) {
    initData_Block(d, cstr, cstr ? strlen(cstr) : 0);
}

void initPrealloc_Block(iBlock *d, void *data, size_t size, size_t allocSize) {
    d->i = newPrealloc_BlockData_(data, size, allocSize);
}

void initCopy_Block(iBlock *d, const iBlock *other) {
    if (other) {
        addRelaxed_Atomic(&other->i->refCount, 1);
        d->i = other->i;
    }
    else {
        init_Block(d, 0);
    }
}

void deinit_Block(iBlock *d) {
    deref_BlockData_(d->i);
}

void serialize_Block(const iBlock *d, iStream *outs) {
    writeU32_Stream(outs, (uint32_t) d->i->size);
    if (d->i->size) {
        writeData_Stream(outs, d->i->data, d->i->size);
    }
}

void deserialize_Block(iBlock *d, iStream *ins) {
    clear_Block(d);
    const size_t len = readU32_Stream(ins);
    if (len) {
        resize_Block(d, len);
        readData_Stream(ins, len, d->i->data);
    }
}

size_t size_Block(const iBlock *d) {
    return d && d->i ? d->i->size : 0;
}

char at_Block(const iBlock *d, size_t pos) {
    iAssert(pos < d->i->size);
    return d->i->data[pos];
}

char front_Block(const iBlock *d) {
    return d->i->data[0];
}

char back_Block(const iBlock *d) {
    return d->i->data[d->i->size - 1];
}

const void *constData_Block(const iBlock *d) {
    return d->i->data;
}

const char *constBegin_Block(const iBlock *d) {
    return d->i->data;
}

const char *constEnd_Block(const iBlock *d) {
    return d->i->data + d->i->size;
}

iBlock *mid_Block(const iBlock *d, size_t start, size_t count) {
    if (start >= d->i->size) {
        return new_Block(0);
    }
    const size_t midSize = iMin(count, d->i->size - start);
    iBlock *mid = new_Block(midSize);
    memcpyFrom_Block_(mid, d->i->data + start, midSize);
    return mid;
}

void *data_Block(iBlock *d) {
    detach_Block_(d, 0);
    return d->i->data;
}

void clear_Block(iBlock *d) {
    deref_BlockData_(d->i);
    d->i = &emptyBlockData;
    addRelaxed_Atomic(&emptyBlockData.refCount, 1);
}

void reserve_Block(iBlock *d, size_t reservedSize) {
    /* If we need to detach, allocate memory with the intended headroom already included.
       Otherwise an immediate realloc() would follow. */
    detach_Block_(d, allocSize_(reservedSize));
    reserve_BlockData_(d->i, reservedSize);
}

void resize_Block(iBlock *d, size_t size) {
    if (size < size_Block(d)) {
        truncate_Block(d, size);
        return;
    }
    reserve_Block(d, size);
    const size_t oldSize = d->i->size;
    d->i->size = size;
    memset(d->i->data + oldSize, 0, d->i->size - oldSize + 1);
}

void truncate_Block(iBlock *d, size_t size) {
    if (size < size_Block(d)) {
        detach_Block_(d, 0);
        d->i->size = iMin(d->i->size, size); // note: allocated size does not change
        d->i->data[d->i->size] = 0;
    }
}

void remove_Block(iBlock *d, size_t start, size_t count) {
    detach_Block_(d, 0);
    iAssert(start <= d->i->size);
    if (count == iInvalidSize || start + count > d->i->size) {
        count = d->i->size - start;
    }
    const size_t remainder = d->i->size - start - count;
    if (remainder > 0) {
        memmove(d->i->data + start, d->i->data + start + count, remainder);
    }
    d->i->size -= count;
    d->i->data[d->i->size] = 0;
}

void printf_Block(iBlock *d, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf_Block(d, format, args);
    va_end(args);
}

void vprintf_Block(iBlock *d, const char *format, va_list args) {
    va_list args2;
    va_copy(args2, args);
    const int len = vsnprintf(NULL, 0, format, args);
    if (len < 0) {
        /* Encoding error? Possibly the implementation frowns upon UTF-8. This seems to occur
           on some older 32-bit Android releases at least. */
        size_t msgLen = 0;
        uint8_t *msg = u8_u8_vasnprintf(NULL, &msgLen, (const uint8_t *) format, args);
        setData_Block(d, msg, msgLen); /* an extra copy */
        free(msg);
    }
    else {
        reserve_Block(d, len);
        vsprintf(d->i->data, format, args2);
        d->i->size = len;
    }
    va_end(args2);
}

void fill_Block(iBlock *d, char value) {
    detach_Block_(d, 0);
    memset(d->i->data, value, d->i->size);
    d->i->data[d->i->size] = 0;
}

void pushBack_Block(iBlock *d, char value) {
    reserve_Block(d, d->i->size + 1);
    d->i->data[d->i->size++] = value;
    d->i->data[d->i->size] = 0;
}

void popBack_Block(iBlock *d) {
    detach_Block_(d, 0);
    if (d->i->size > 0) {
        d->i->data[--d->i->size] = 0;
    }
}

void set_Block(iBlock *d, const iBlock *other) {
    if (d->i != other->i) {
        addRelaxed_Atomic(&other->i->refCount, 1);
        deref_BlockData_(d->i);
        d->i = other->i;
    }
}

void setByte_Block(iBlock *d, size_t pos, char value) {
    detach_Block_(d, 0);
    iAssert(pos < d->i->size);
    d->i->data[pos] = value;
}

void setData_Block(iBlock *d, const void *data, size_t size) {
    if (size) {
        reserve_Block(d, size);
        memcpyFrom_Block_(d, data, size);
    }
    else {
        clear_Block(d);
    }
}

void setSubData_Block(iBlock *d, size_t pos, const void *data, size_t size) {
    reserve_Block(d, pos + size);
    iAssert(pos <= d->i->size);
    memcpy(d->i->data + pos, data, size);
    d->i->size = iMax(d->i->size, pos + size);
    if (d->i->size == pos + size) {
        d->i->data[d->i->size] = 0;
    }
}

void setCStr_Block(iBlock *d, const char *cstr) {
    setData_Block(d, cstr, strlen(cstr));
}

void append_Block(iBlock *d, const iBlock *other) {
    appendData_Block(d, other->i->data, other->i->size);
}

void appendData_Block(iBlock *d, const void *data, size_t size) {
    reserve_Block(d, d->i->size + size);
    memcpy(d->i->data + d->i->size, data, size);
    d->i->size += size;
    d->i->data[d->i->size] = 0;
}

void appendCStr_Block(iBlock *d, const char *cstr) {
    appendData_Block(d, cstr, strlen(cstr));
}

void insertData_Block(iBlock *d, size_t insertAt, const void *data, size_t size) {
    reserve_Block(d, d->i->size + size);
    char *start = d->i->data + insertAt;
    memmove(start + size, start, d->i->size - insertAt);
    memcpy (start,        data,  size);
    d->i->size += size;
    d->i->data[d->i->size] = 0;
}

iBlock *concat_Block(const iBlock *d, const iBlock *other) {
    iBlock *cat = new_Block(d->i->size + other->i->size);
    memcpy(cat->i->data,                  d->i->data,     d->i->size);
    memcpy(cat->i->data + d->i->size, other->i->data, other->i->size);
    cat->i->data[cat->i->size] = 0;
    return cat;
}

int cmp_Block(const iBlock *d, const iBlock *other) {
    return cmpData_Block(d, other->i->data, other->i->size);
}

int cmpCase_Block(const iBlock *d, const iBlock *other) {
    return iCmpStrCase(d->i->data, other->i->data);
}

int cmpCaseN_Block(const iBlock *d, const iBlock *other, size_t size) {
    return iCmpStrNCase(d->i->data, other->i->data, size);
}

int cmpData_Block(const iBlock *d, const char *data, size_t size) {
    return memcmp(d->i->data, data, iMin(size, d->i->size));
}

int cmpCStr_Block(const iBlock *d, const char *cstr) {
    return iCmpStr(d->i->data, cstr);
}

int cmpCStrN_Block(const iBlock *d, const char *cstr, size_t len) {
    return iCmpStrN(d->i->data, cstr, len);
}

int cmpCaseCStr_Block(const iBlock *d, const char *cstr) {
    return iCmpStrCase(d->i->data, cstr);
}

int cmpCaseCStrN_Block(const iBlock *d, const char *cstr, size_t len) {
    return iCmpStrNCase(d->i->data, cstr, len);
}

uint32_t crc32_Block(const iBlock *d) {
    return iCrc32(d->i->data, d->i->size);
}

void md5_Block(const iBlock *d, uint8_t md5_out[16]) {
    iMd5Hash(d->i->data, d->i->size, md5_out);
}

iString *decode_Block(const iBlock *d, const char *textEncoding) {
    size_t len = 0;
    uint8_t *data = u8_conv_from_encoding(textEncoding,
                                          iconveh_question_mark,
                                          constData_Block(d),
                                          size_Block(d),
                                          NULL,
                                          NULL,
                                          &len);
    data = realloc(data, len + 1);
    data[len] = 0;
    iString *str = iMalloc(String);
    initPrealloc_Block(&str->chars, data, len, len + 1);
    return str;
}

iString *hexEncode_Block(const iBlock *d) {
    static const char hexValues[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
                                        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
    iString *hex = new_String();
    for (const char *i = constBegin_Block(d), *end = constEnd_Block(d); i != end; i++) {
        const uint8_t val = *i;
        appendChar_String(hex, hexValues[val >> 4]);
        appendChar_String(hex, hexValues[val & 15]);
    }
    return hex;
}

static int fromHex_(char ch) {
    if (ch >= 'a') return ch - 'a' + 10;
    if (ch >= 'A') return ch - 'A' + 10;
    return ch - '0';
}

iBlock *hexDecode_Rangecc(iRangecc range) {
    iBlock *d = new_Block(size_Range(&range) / 2);
    size_t pos = 0;
    for (const char *i = range.start; i < range.end; i += 2) {
        const uint8_t val = (fromHex_(i[0]) << 4) | fromHex_(i[1]);
        setByte_Block(d, pos++, val);
    }
    return d;
}

iString *base64Encode_Block(const iBlock *d) {
    static const char *base64Table_ =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const unsigned char *src    = constData_Block(d);
    const size_t         len    = size_Block(d);
    size_t               olen   = 4*((len + 2) / 3); /* 3-byte blocks to 4-byte */
    iString *            output = new_String();
    if (olen < len) {
        return output; /* integer overflow */
    }
    resize_Block(&output->chars, olen);
    unsigned char *      out = data_Block(&output->chars);
    const unsigned char *end = src + len;
    const unsigned char *in  = src;
    unsigned char *      pos = out;
    while (end - in >= 3) {
        *pos++ = base64Table_[in[0] >> 2];
        *pos++ = base64Table_[((in[0] & 0x03) << 4) | (in[1] >> 4)];
        *pos++ = base64Table_[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
        *pos++ = base64Table_[in[2] & 0x3f];
        in += 3;
    }
    if (end - in) {
        *pos++ = base64Table_[in[0] >> 2];
        if (end - in == 1) {
            *pos++ = base64Table_[(in[0] & 0x03) << 4];
            *pos++ = '=';
        }
        else {
            *pos++ = base64Table_[((in[0] & 0x03) << 4) | (in[1] >> 4)];
            *pos++ = base64Table_[(in[1] & 0x0f) << 2];
        }
        *pos++ = '=';
    }
    return output;
}

iLocalDef uint8_t base64Index_(char ch) {
    /* TODO: Replace this with a lookup table. */
    if (ch == '=') return 0; /* padding */
    if (ch == '/') return 63;
    if (ch == '+') return 62;
    if (ch >= 'a') return ch - 'a' + 26;
    if (ch >= 'A') return ch - 'A';
    return ch - '0' + 52;
}

iBlock *base64Decode_Block(const iBlock *d) {
    const size_t outSize = 6 * size_Block(d) / 8;
    iBlock *     decoded = new_Block(outSize);
    uint16_t     comp    = 0;
    int          shift   = 10;
    // |-------|-------| comp
    //  7654321076543210
    //       ^10         shift
    //  aaaaaa
    //             ^4    shift
    //  aaaaaabbbbbb
    uint8_t *out = data_Block(decoded);
    for (const char *pos = constBegin_Block(d), *end = constEnd_Block(d); pos != end; pos++) {
        comp |= base64Index_(*pos) << shift;
        if (shift <= 8) {
            *out++ = comp >> 8;
            comp <<= 8;
            shift += 8;
        }
        shift -= 6;
    }
    iAssert((size_t) (out - (uint8_t *) data_Block(decoded)) == outSize); /* all written */
    return decoded;
}

size_t replace_Block(iBlock *d, char oldValue, char newValue) {
    size_t count = 0;
    detach_Block_(d, 0);
    for (char *i = d->i->data, *end = d->i->data + d->i->size; i != end; ++i) {
        if (*i == oldValue) {
            *i = newValue;
            count++;
        }
    }
    return count;
}

/*-------------------------------------------------------------------------------------*/
#if defined (iHaveZlib)

iDeclareType(ZStream)

struct Impl_ZStream {
    z_stream stream;
    iBlock *out;
};

static void init_ZStream_(iZStream *d, const iBlock *in, iBlock *out) {
    d->out = out;
    iZap(d->stream);
    d->stream.avail_in  = (uInt) in->i->size;
    d->stream.next_in   = (Bytef *) in->i->data;
    d->stream.avail_out = (uInt) out->i->size;
    d->stream.next_out  = (Bytef *) out->i->data;
}

static iBool process_ZStream_(iZStream *d, int (*process)(z_streamp, int)) {
    int opts = Z_NO_FLUSH;
    for (;;) {
        int rc = process(&d->stream, opts);
        if (rc == Z_STREAM_END) {
            break;
        }
        else if (rc != Z_OK && rc != Z_BUF_ERROR) {
            /* Something went wrong. */
            return iFalse;
        }
        if (d->stream.avail_out == 0) {
            /* Allocate more room. */
            const size_t oldSize = size_Block(d->out);
            resize_Block(d->out, oldSize * 2);
            d->stream.next_out = (Bytef *) d->out->i->data + oldSize;
            d->stream.avail_out = (uInt) (size_Block(d->out) - oldSize);
        }
        if (d->stream.avail_in == 0) {
            opts = Z_FINISH;
        }
    }
    truncate_Block(d->out, size_Block(d->out) - d->stream.avail_out);
    return iTrue;
}

iBlock *compressLevel_Block(const iBlock *d, int level) {
    iBlock *out = new_Block(1024);
    iZStream z;
    init_ZStream_(&z, d, out);
    /*
     * The deflation is done in raw mode. From zlib documentation:
     *
     * "windowBits can also be –8..–15 for raw deflate. In this case, -windowBits
     * determines the window size. deflate() will then generate raw deflate data with no
     * zlib header or trailer, and will not compute an adler32 check value."
     */
    if (deflateInit2(&z.stream, level, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY) == Z_OK) {
        if (!process_ZStream_(&z, deflate)) {
            clear_Block(out);
        }
    }
    deflateEnd(&z.stream);
    return out;
}

iBlock *decompress_Block(const iBlock *d) {
    iBlock *out = new_Block(1024);
    iZStream z;
    init_ZStream_(&z, d, out);
    if (inflateInit2(&z.stream, -MAX_WBITS) == Z_OK) {
        if (!process_ZStream_(&z, inflate)) {
            clear_Block(out);
        }
    }
    inflateEnd(&z.stream);
    return out;
}

iBlock *decompressGzip_Block(const iBlock *d) {
    iBlock *out = new_Block(1024);
    iZStream z;
    init_ZStream_(&z, d, out);
    if (inflateInit2(&z.stream, 16 + MAX_WBITS) == Z_OK) {
        if (!process_ZStream_(&z, inflate)) {
            clear_Block(out);
        }
    }
    inflateEnd(&z.stream);
    return out;
}

#endif // HaveZlib
