#pragma once

/** @file the_Foundation/ptrarray.h  Array of pointers.

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

#include "array.h"

iBeginPublic

typedef iArray iPtrArray;

iDeclareTypeConstruction(PtrArray)

iPtrArray * newPointers_PtrArray    (void *ptr, ...); // NULL-terminated

#define     isEmpty_PtrArray(d)     isEmpty_Array(d)
#define     size_PtrArray(d)        size_Array(d)

void **         data_PtrArray       (iPtrArray *);
const void **   constData_PtrArray  (const iPtrArray *);
void *          at_PtrArray         (iPtrArray *, size_t pos);
const void *    constAt_PtrArray    (const iPtrArray *, size_t pos);
void            set_PtrArray        (iPtrArray *, size_t pos, const void *ptr);
void            setCopy_PtrArray    (iPtrArray *, const iPtrArray *other);

void        pushBack_PtrArray       (iPtrArray *, const void *ptr);
void        pushFront_PtrArray      (iPtrArray *, const void *ptr);

iLocalDef void *front_PtrArray  (iPtrArray *d) { return !isEmpty_PtrArray(d) ? at_PtrArray(d, 0) : NULL; }

iLocalDef const void *constFront_PtrArray(const iPtrArray *d) {
    return !isEmpty_PtrArray(d) ? constAt_PtrArray(d, 0) : NULL;
}

iBool       take_PtrArray           (iPtrArray *, size_t pos, void **ptr_out);
size_t      takeN_PtrArray          (iPtrArray *, size_t pos, void **ptr_out, size_t count);
void        insert_PtrArray         (iPtrArray *, size_t pos, const void *value);
void        insertN_PtrArray        (iPtrArray *, size_t pos, const void **values, size_t count);

iLocalDef iBool takeFront_PtrArray(iPtrArray *d, void **ptr_out) {
    return take_PtrArray(d, 0, ptr_out);
}
iLocalDef iBool takeBack_PtrArray(iPtrArray *d, void **ptr_out) {
    if (isEmpty_PtrArray(d)) return iFalse;
    return take_PtrArray(d, size_PtrArray(d) - 1, ptr_out);
}

#define     resize_PtrArray(d, s)   resize_Array(d, s)
#define     clear_PtrArray(d)       clear_Array(d)

size_t      indexOf_PtrArray        (const iPtrArray *, const void *ptr); /* O(n) */
iBool       removeOne_PtrArray      (iPtrArray *, const void *ptr); /* O(n) */
size_t      removeAll_PtrArray      (iPtrArray *, const void *ptr); /* O(n) */

/** @name Iterators */
///@{
iDeclareIterator(PtrArray, iPtrArray *)
iDeclareConstIterator(PtrArray, const iPtrArray *)

#define index_PtrArrayIterator(d)               index_ArrayIterator(&(d)->iter)
#define index_PtrArrayConstIterator(d)          index_ArrayConstIterator(&(d)->iter)
#define index_PtrArrayReverseIterator(d)        index_ArrayReverseIterator(d)
#define index_PtrArrayReverseConstIterator(d)   index_ArrayReverseConstIterator(d)

struct IteratorImpl_PtrArray {
    union {
        void **value; // pointer to array element
        iArrayIterator iter;
    };
    void *ptr; // array element
};
struct ConstIteratorImpl_PtrArray {
    union {
        void * const *value; // pointer to array element
        iArrayConstIterator iter;
    };
    void *ptr; // array element
};
struct ReverseIteratorImpl_PtrArray {
    union {
        void **value; // pointer to array element
        iArrayReverseIterator iter;
    };
    void *ptr; // array element
};
struct ReverseConstIteratorImpl_PtrArray {
    union {
        void * const *value; // pointer to array element
        iArrayReverseConstIterator iter;
    };
    void *ptr; // array element
};
///@}

iLocalDef void remove_PtrArrayIterator(iPtrArrayIterator *d) {
    remove_ArrayIterator(&d->iter);
}

iEndPublic
