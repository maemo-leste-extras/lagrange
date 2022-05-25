/** @file ptrset.c  Set of unique integers.

@authors Copyright (c) 2020 Jaakko Keränen <jaakko.keranen@iki.fi>

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

#include "the_Foundation/intset.h"
#include "the_Foundation/stream.h"

static int cmp_IntSet_(const void *a, const void *b) {
    return iCmp(*(const int *) a, *(const int *) b);
}

iDefineTypeConstruction(IntSet)

iIntSet *newCmp_IntSet(iIntSetCompareFunc cmp) {
    iIntSet *d = new_IntSet();
    if (cmp) d->cmp = (iSortedArrayCompareElemFunc) cmp;
    return d;
}

void init_IntSet(iIntSet *d) {
    init_SortedArray(d, sizeof(int), cmp_IntSet_);
}

void initCmp_IntSet(iIntSet *d, iIntSetCompareFunc cmp) {
    init_SortedArray(d, sizeof(int), cmp ? (iSortedArrayCompareElemFunc) cmp : cmp_IntSet_);
}

void deinit_IntSet(iIntSet *d) {
    deinit_SortedArray(d);
}

iIntSet *copy_IntSet(const iIntSet *d) {
    return d ? copy_SortedArray(d) : NULL;
}

iBool contains_IntSet(const iIntSet *d, int value) {
    return d && contains_SortedArray(d, &value);
}

iBool locate_IntSet(const iIntSet *d, int value, size_t *pos_out) {
    return locate_SortedArray(d, &value, pos_out);
}

iBool insert_IntSet(iIntSet *d, int value) {
    return insert_SortedArray(d, &value);
}

iBool remove_IntSet(iIntSet *d, int value) {
    iAssert(d);
    return remove_SortedArray(d, &value);
}

int at_IntSet(const iIntSet *d, size_t pos) {
    return *(const int *) constAt_SortedArray(d, pos);
}

void serialize_IntSet(const iIntSet *d, iStream *outs) {
    writeU32_Stream(outs, (uint32_t) size_IntSet(d));
    iConstForEach(IntSet, i, d) {
        write32_Stream(outs, *i.value);
    }
}

void deserialize_IntSet(iIntSet *d, iStream *ins) {
    clear_IntSet(d);
    uint32_t count = readU32_Stream(ins);
    while (count--) {
        insert_IntSet(d, read32_Stream(ins));
    }
}

/*-------------------------------------------------------------------------------------*/

void init_IntSetIterator(iIntSetIterator *d, iIntSet *set) {
    init_ArrayIterator(&d->iter, &set->values);
}

void next_IntSetIterator(iIntSetIterator *d) {
    next_ArrayIterator(&d->iter);
}

void remove_IntSetIterator(iIntSetIterator *d) {
    remove_ArrayIterator(&d->iter);
}

void init_IntSetConstIterator(iIntSetConstIterator *d, const iIntSet *set) {
    init_ArrayConstIterator(&d->iter, set ? &set->values : NULL);
}

void next_IntSetConstIterator(iIntSetConstIterator *d) {
    next_ArrayConstIterator(&d->iter);
}

void init_IntSetReverseConstIterator(iIntSetReverseConstIterator *d, const iIntSet *set) {
    init_ArrayReverseConstIterator(&d->iter, set ? &set->values : NULL);
}

void next_IntSetReverseConstIterator(iIntSetReverseConstIterator *d) {
    next_ArrayReverseConstIterator(&d->iter);
}
