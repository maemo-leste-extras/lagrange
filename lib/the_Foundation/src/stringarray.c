/** @file stringarray.c  Array of strings.

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

#include "the_Foundation/stringarray.h"

#include <stdarg.h>

iDefineClass(StringArray)
iDefineObjectConstruction(StringArray)

void init_StringArray(iStringArray *d) {
    init_Array(&d->strings, sizeof(iString));
}

void deinit_StringArray(iStringArray *d) {
    clear_StringArray(d);
    deinit_Array(&d->strings);
}

iStringArray *newStrings_StringArray(const iString *str, ...) {
    iStringArray *d = new_StringArray();
    iForVarArgs(const iString *, str, pushBack_StringArray(d, str));
    return d;
}

iStringArray *newStringsCStr_StringArray(const char *cstr, ...) {
    iStringArray *d = new_StringArray();
    iForVarArgs(const char *, cstr, pushBackCStr_StringArray(d, cstr));
    return d;
}

void clear_StringArray(iStringArray *d) {
    iForEach(Array, i, &d->strings) {
        deinit_String(i.value);
    }
    clear_Array(&d->strings);
}

void resize_StringArray(iStringArray *d, size_t size) {
    const size_t oldSize = size_StringArray(d);
    if (size < oldSize) {
        for (size_t i = size; i < oldSize; ++i) {
            deinit_String(at_Array(&d->strings, i));
        }
    }
    resize_Array(&d->strings, size);
    if (size > oldSize) {
        for (size_t i = oldSize; i < size; ++i) {
            init_String(at_Array(&d->strings, i));
        }
    }
}

iString *at_StringArray(iStringArray *d, size_t pos) {
    return at_Array(&d->strings, pos);
}

const iString *constAt_StringArray(const iStringArray *d, size_t pos) {
    return constAt_Array(&d->strings, pos);
}

void set_StringArray(iStringArray *d, size_t pos, const iString *str) {
    iAssert(pos < size_StringArray(d));
    set_String(at_Array(&d->strings, pos), str);
}

void pushBack_StringArray(iStringArray *d, const iString *str) {
    iString elem;
    initCopy_String(&elem, str);
    pushBack_Array(&d->strings, &elem);
}

void pushFront_StringArray(iStringArray *d, const iString *str) {
    iString elem;
    initCopy_String(&elem, str);
    pushFront_Array(&d->strings, &elem);
}

void insert_StringArray(iStringArray *d, size_t pos, const iString *str) {
    iString elem;
    initCopy_String(&elem, str);
    insert_Array(&d->strings, pos, &elem);
}

void setCStr_StringArray(iStringArray *d, size_t pos, const char *cstr) {
    iAssert(pos < size_StringArray(d));
    setCStr_String(at_Array(&d->strings, pos), cstr);
}

void pushBackCStr_StringArray(iStringArray *d, const char *cstr) {
    iString elem;
    initCStr_String(&elem, cstr);
    pushBack_Array(&d->strings, &elem);
}

void pushBackCStrN_StringArray(iStringArray *d, const char *cstr, size_t n) {
    iString elem;
    initCStrN_String(&elem, cstr, n);
    pushBack_Array(&d->strings, &elem);
}

void pushFrontCStr_StringArray(iStringArray *d, const char *cstr) {
    iString elem;
    initCStr_String(&elem, cstr);
    pushFront_Array(&d->strings, &elem);
}

void insertCStr_StringArray(iStringArray *d, size_t pos, const char *cstr) {
    iString elem;
    initCStr_String(&elem, cstr);
    insert_Array(&d->strings, pos, &elem);
}

iString *take_StringArray(iStringArray *d, size_t pos) {
    iString elem;
    if (take_Array(&d->strings, pos, &elem)) {
        iString *str = copy_String(&elem);
        deinit_String(&elem);
        return str;
    }
    return NULL;
}

void remove_StringArray(iStringArray *d, size_t pos) {
    delete_String(take_StringArray(d, pos));
}

void move_StringArray(iStringArray *d, iRanges range, iStringArray *dest, size_t destPos) {
    move_Array(&d->strings, range, &dest->strings, destPos);
}

iString *joinCStr_StringArray(const iStringArray *d, const char *delim) {
    iString *joined = new_String();
    iConstForEach(StringArray, i, d) {
        if (delim && index_StringArrayConstIterator(&i) > 0) {
            appendCStr_String(joined, delim);
        }
        append_String(joined, i.value);
    }
    return joined;
}

/*-------------------------------------------------------------------------------------*/

void init_StringArrayIterator(iStringArrayIterator *d, iStringArray *array) {
    init_ArrayIterator(&d->iter, &array->strings);
}

void next_StringArrayIterator(iStringArrayIterator *d) {
    next_ArrayIterator(&d->iter);
}

void init_StringArrayConstIterator(iStringArrayConstIterator *d, const iStringArray *array) {
    init_ArrayConstIterator(&d->iter, &array->strings);
}

void next_StringArrayConstIterator(iStringArrayConstIterator *d) {
    next_ArrayConstIterator(&d->iter);
}
