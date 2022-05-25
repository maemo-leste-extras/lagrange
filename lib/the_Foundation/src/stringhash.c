/** @file stringhash.c  Hash that uses strings for keys.

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

#include "the_Foundation/stringhash.h"
#include "the_Foundation/garbage.h"

#include <stdlib.h>
#include <stdarg.h>

iDefineBlockHash(StringHash, String, AnyObject)

const iString *key_StringHashNode(const iStringHashNode *d) {
    return (const iString *) &d->keyBlock; // iString derived from iBlock
}

void initKey_StringHashNode(const iStringHashNode *d, iString *key) {
    initBlock_String(key, &d->keyBlock);
}

void initBlock_StringHashKey(const iString *d, iBlock *block) {
    initCopy_Block(block, &d->chars);
}

const iAnyObject *constValueRange_StringHash(const iStringHash *d, const iRangecc key) {
    return constValue_BlockHash(d, &iBlockLiteral(key.start, size_Range(&key), size_Range(&key)));
}

iBool insertCStr_StringHash(iStringHash *d, const char *key, iAnyObject *value) {
    return insertCStrN_StringHash(d, key, strlen(key), value);
}

iBool insertCStrN_StringHash(iStringHash *d, const char *key, size_t size, iAnyObject *value) {
    iString keyStr;
    initCStrN_String(&keyStr, key, size);
    const iBool res = insert_StringHash(d, &keyStr, value);
    deinit_String(&keyStr);
    return res;
}

void insertValues_StringHash(iStringHash *d, const iString *key, iAnyObject *value, ...) {
    insert_StringHash(d, key, value);
    va_list args;
    for (va_start(args, value);;) {
        key = va_arg(args, const iString *);
        if (!key) break;
        value = va_arg(args, iAnyObject *);
        insert_StringHash(d, key, value);
    }
    va_end(args);
}

void insertValuesCStr_StringHash(iStringHash *d, const char *key, iAnyObject *value, ...) {
    insertCStr_StringHash(d, key, value);
    va_list args;
    for (va_start(args, value);;) {
        key = va_arg(args, const char *);
        if (!key) break;
        value = va_arg(args, iAnyObject *);
        insertCStr_StringHash(d, key, value);
    }
    va_end(args);
}
