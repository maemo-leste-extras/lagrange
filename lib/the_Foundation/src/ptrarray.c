/** @file ptrarray.c  Array of pointers.

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

#include "the_Foundation/ptrarray.h"

#include <stdarg.h>

iDefineTypeConstruction(PtrArray)

iPtrArray *newPointers_PtrArray(void *ptr, ...) {
    iPtrArray *d = new_PtrArray();
    iForVarArgs(void *, ptr, pushBack_PtrArray(d, ptr));
    return d;
}

void init_PtrArray(iPtrArray *d) {
    init_Array(d, sizeof(void *));
}

void deinit_PtrArray(iPtrArray *d) {
    deinit_Array(d);
}

const void **constData_PtrArray(const iPtrArray *d) {
    return *(void * const *) constData_Array(d);
}

void *at_PtrArray(iPtrArray *d, size_t pos) {
    return *(void **) at_Array(d, pos);
}

const void *constAt_PtrArray(const iPtrArray *d, size_t pos) {
    return *(void * const *) constAt_Array(d, pos);
}

void set_PtrArray(iPtrArray *d, size_t pos, const void *ptr) {
    set_Array(d, pos, &ptr);
}

void setCopy_PtrArray(iPtrArray *d, const iPtrArray *other) {
    setCopy_Array(d, other);
}

void pushBack_PtrArray(iPtrArray *d, const void *ptr) {
    pushBack_Array(d, &ptr);
}

void pushFront_PtrArray(iPtrArray *d, const void *ptr) {
    pushFront_Array(d, &ptr);
}

iBool take_PtrArray(iPtrArray *d, size_t pos, void **ptr_out) {
    return takeN_PtrArray(d, pos, ptr_out, 1) > 0;
}

size_t takeN_PtrArray(iPtrArray *d, size_t pos, void **ptr_out, size_t count) {
    return takeN_Array(d, pos, ptr_out, count);
}

void insert_PtrArray(iPtrArray *d, size_t pos, const void *value) {
    insertN_PtrArray(d, pos, &value, 1);
}

void insertN_PtrArray(iPtrArray *d, size_t pos, const void **values, size_t count) {
    insertN_Array(d, pos, values, count);
}

size_t indexOf_PtrArray(const iPtrArray *d, const void *ptr) {
    iConstForEach(PtrArray, i, d) {
        if (i.ptr == ptr) {
            return index_PtrArrayConstIterator(&i);
        }
    }
    return iInvalidPos;
}

iBool removeOne_PtrArray(iPtrArray *d, const void *ptr) {
    iForEach(PtrArray, i, d) {
        if (i.ptr == ptr) {
            remove_PtrArrayIterator(&i);
            return iTrue;
        }
    }
    return iFalse;
}

size_t removeAll_PtrArray(iPtrArray *d, const void *ptr) {
    size_t count = 0;
    iForEach(PtrArray, i, d) {
        if (i.ptr == ptr) {
            remove_PtrArrayIterator(&i);
            count++;
        }
    }
    return count;
}

/*-------------------------------------------------------------------------------------*/

#define value_PtrArrayIterator_(d)      (((d)->iter.value ? *(void **) (d)->iter.value : NULL));
#define value_PtrArrayConstIterator_(d) (((d)->iter.value ? *(void * const *) (d)->iter.value : NULL));

void init_PtrArrayIterator(iPtrArrayIterator *d, iPtrArray *array) {
    init_ArrayIterator(&d->iter, array);
    d->ptr = value_PtrArrayIterator_(d);
}

void next_PtrArrayIterator(iPtrArrayIterator *d) {
    next_ArrayIterator(&d->iter);
    d->ptr = value_PtrArrayIterator_(d);
}

void init_PtrArrayConstIterator(iPtrArrayConstIterator *d, const iPtrArray *array) {
    init_ArrayConstIterator(&d->iter, array);
    d->ptr = value_PtrArrayConstIterator_(d);
}

void next_PtrArrayConstIterator(iPtrArrayConstIterator *d) {
    next_ArrayConstIterator(&d->iter);
    d->ptr = value_PtrArrayConstIterator_(d);
}

void init_PtrArrayReverseIterator(iPtrArrayReverseIterator *d, iPtrArray *array) {
    init_ArrayReverseIterator(&d->iter, array);
    d->ptr = value_PtrArrayIterator_(d);
}

void next_PtrArrayReverseIterator(iPtrArrayReverseIterator *d) {
    next_ArrayReverseIterator(&d->iter);
    d->ptr = value_PtrArrayIterator_(d);
}

void init_PtrArrayReverseConstIterator(iPtrArrayReverseConstIterator *d, const iPtrArray *array) {
    init_ArrayReverseConstIterator(&d->iter, array);
    d->ptr = value_PtrArrayConstIterator_(d);
}

void next_PtrArrayReverseConstIterator(iPtrArrayReverseConstIterator *d) {
    next_ArrayReverseConstIterator(&d->iter);
    d->ptr = value_PtrArrayConstIterator_(d);
}
