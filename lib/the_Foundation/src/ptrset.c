/** @file ptrset.c  Set of unique pointers.

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

#include "the_Foundation/ptrset.h"
#include "the_Foundation/stream.h"

typedef void * iPtr;

static int cmp_PtrSet_(const void *a, const void *b) {
    return iCmp(*(const iPtr *) a, *(const iPtr *) b);
}

iDefineTypeConstruction(PtrSet)

iPtrSet *newCmp_PtrSet(iSortedArrayCompareElemFunc cmp) {
    iPtrSet *d = new_PtrSet();
    if (cmp) d->cmp = cmp;
    return d;
}

void init_PtrSet(iPtrSet *d) {
    init_SortedArray(d, sizeof(iPtr), cmp_PtrSet_);
}

void initCmp_PtrSet(iPtrSet *d, iSortedArrayCompareElemFunc cmp) {
    init_SortedArray(d, sizeof(iPtr), cmp ? cmp : cmp_PtrSet_);
}

void deinit_PtrSet(iPtrSet *d) {
    deinit_SortedArray(d);
}

iPtrSet *copy_PtrSet(const iPtrSet *d) {
    return d ? copy_SortedArray(d) : NULL;
}

iBool contains_PtrSet(const iPtrSet *d, const void *ptr) {
    return d && contains_SortedArray(d, &ptr);
}

iBool locate_PtrSet(const iPtrSet *d, const void *ptr, size_t *pos_out) {
    return locate_SortedArray(d, &ptr, pos_out);
}

iBool insert_PtrSet(iPtrSet *d, const void *ptr) {
    return insert_SortedArray(d, &ptr);
}

iBool remove_PtrSet(iPtrSet *d, const void *ptr) {
    iAssert(d);
    return remove_SortedArray(d, &ptr);
}

void *at_PtrSet(const iPtrSet *d, size_t pos) {
    return *(void * const *) constAt_SortedArray(d, pos);
}

void serializeObjects_PtrSet(const iPtrSet *d, iStream *outs) {
    if (!d) {
        writeU32_Stream(outs, 0);
        return;
    }
    writeU32_Stream(outs, (uint32_t) size_PtrSet(d));
    iConstForEach(PtrSet, i, d) {
        writeObject_Stream(outs, *i.value);
    }
}

void deserializeObjects_PtrSet(iPtrSet *d, iStream *ins, const iAnyClass *class) {
    iAssert(d);
    iAssert(isEmpty_PtrSet(d));
    uint32_t count = readU32_Stream(ins);
    while (count--) {
        insert_PtrSet(d, readObject_Stream(ins, class));
    }
}

iPtrSet *newStreamObjects_PtrSet(iStream *ins, const iAnyClass *class) {
    iPtrSet *d = NULL;
    uint32_t count = readU32_Stream(ins);
    if (count) {
        d = new_PtrSet();
        while (count--) {
            insert_PtrSet(d, readObject_Stream(ins, class));
        }
    }
    return d;
}

/*-------------------------------------------------------------------------------------*/

void init_PtrSetIterator(iPtrSetIterator *d, iPtrSet *set) {
    init_ArrayIterator(&d->iter, set ? &set->values : NULL);
}

void next_PtrSetIterator(iPtrSetIterator *d) {
    next_ArrayIterator(&d->iter);
}

void remove_PtrSetIterator(iPtrSetIterator *d) {
    remove_ArrayIterator(&d->iter);
}

void init_PtrSetConstIterator(iPtrSetConstIterator *d, const iPtrSet *set) {
    init_ArrayConstIterator(&d->iter, set ? &set->values : NULL);
}

void next_PtrSetConstIterator(iPtrSetConstIterator *d) {
    next_ArrayConstIterator(&d->iter);
}
