#pragma once

/** @file the_Foundation/block.h  Byte array with copy-on-write semantics.

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

#include "defs.h"
#include "range.h"
#include "atomic.h"

#include <stdarg.h>

iBeginPublic

iDeclareType(Stream)
iDeclareType(String)

iDeclareType(Block)
iDeclareType(BlockData)

struct Impl_Block {
    iBlockData *i;
};

struct Impl_BlockData {
    iAtomicInt refCount;
    char *data;
    size_t size;
    size_t allocSize;
};

/**
 * @warning When used outside the global scope (i.e., inside a function), note that BlockData
 * gets deallocated when the scope is exited, because it is stored on the stack. In this case,
 * make sure that the BlockData is only read and not copy-referenced.
 */
#define iBlockLiteral(ptr, sz, allocSz) \
    (iBlock){ &(iBlockData){ .refCount = 2, .data = iConstCast(char *, ptr), .size = (sz), .allocSize = (allocSz) } }

iDeclareTypeConstructionArgs(Block, size_t size)
iDeclareTypeSerialization(Block)

iBlock *        newCStr_Block       (const char *cstr);
iBlock *        newData_Block       (const void *data, size_t size);
iBlock *        newPrealloc_Block   (void *data, size_t size, size_t allocSize);
iBlock *        copy_Block          (const iBlock *);

iLocalDef iBlock *newRange_Block(iRangecc range) {
    return newData_Block(range.start, size_Range(&range));
}

void            initCStr_Block      (iBlock *, const char *cstr);
void            initData_Block      (iBlock *, const void *data, size_t size);
void            initPrealloc_Block  (iBlock *, void *data, size_t size, size_t allocSize);
void            initCopy_Block      (iBlock *, const iBlock *other);

size_t          size_Block          (const iBlock *);
char            at_Block            (const iBlock *, size_t pos);
char            front_Block         (const iBlock *);
char            back_Block          (const iBlock *);
iBlock *        mid_Block           (const iBlock *, size_t start, size_t count);
iBlock *        concat_Block        (const iBlock *, const iBlock *other);

const void *    constData_Block     (const iBlock *);
const char *    constBegin_Block    (const iBlock *);
const char *    constEnd_Block      (const iBlock *);

iLocalDef const char *cstr_Block(const iBlock *d) { return constBegin_Block(d); }
iLocalDef iRangecc range_Block  (const iBlock *d) { iRangecc r = { constBegin_Block(d), constEnd_Block(d) }; return r; }
iLocalDef iBool isEmpty_Block   (const iBlock *d) { return size_Block(d) == 0; }
iLocalDef iBlock *midRange_Block(const iBlock *d, iRanges range) { return mid_Block(d, range.start, size_Range(&range)); }

int             cmp_Block           (const iBlock *, const iBlock *other);
int             cmpData_Block       (const iBlock *, const char *data, size_t size);

int             cmpCase_Block       (const iBlock *, const iBlock *other);
int             cmpCaseN_Block      (const iBlock *, const iBlock *other, size_t size);
int             cmpCStr_Block       (const iBlock *, const char *cstr);
int             cmpCStrN_Block      (const iBlock *, const char *cstr, size_t len);
int             cmpCaseCStr_Block   (const iBlock *, const char *cstr);
int             cmpCaseCStrN_Block  (const iBlock *, const char *cstr, size_t len);

void *          data_Block          (iBlock *);
void            fill_Block          (iBlock *, char value);
void            clear_Block         (iBlock *);
void            reserve_Block       (iBlock *, size_t reservedSize);
void            resize_Block        (iBlock *, size_t size);
void            truncate_Block      (iBlock *, size_t size);
void            remove_Block        (iBlock *, size_t start, size_t count);
size_t          replace_Block       (iBlock *, char oldValue, char newValue);
void            printf_Block        (iBlock *, const char *format, ...);
void            vprintf_Block       (iBlock *, const char *format, va_list args);

void            pushBack_Block      (iBlock *, char value);
void            popBack_Block       (iBlock *);

void            set_Block           (iBlock *, const iBlock *other);
void            setByte_Block       (iBlock *, size_t pos, char value);
void            setData_Block       (iBlock *, const void *data, size_t size);
void            setSubData_Block    (iBlock *, size_t pos, const void *data, size_t size);
void            setCStr_Block       (iBlock *, const char *cstr);

void            append_Block        (iBlock *, const iBlock *other);
void            appendData_Block    (iBlock *, const void *data, size_t size);
void            appendCStr_Block    (iBlock *, const char *cstr);

void            insertData_Block    (iBlock *, size_t insertAt, const void *data, size_t size);

uint32_t        crc32_Block         (const iBlock *);
void            md5_Block           (const iBlock *, uint8_t md5_out[16]);

iString *       decode_Block        (const iBlock *, const char *textEncoding);
iString *       hexEncode_Block     (const iBlock *);
iBlock *        hexDecode_Rangecc   (iRangecc);
iBlock *        base64Decode_Block  (const iBlock *);

#define iBlockDefaultCompressionLevel   6

iBlock *        compressLevel_Block (const iBlock *, int level);
iBlock *        decompress_Block    (const iBlock *);
iBlock *        decompressGzip_Block(const iBlock *);

iLocalDef iBlock * compress_Block(const iBlock *d) {
    return compressLevel_Block(d, iBlockDefaultCompressionLevel);
}

iEndPublic
