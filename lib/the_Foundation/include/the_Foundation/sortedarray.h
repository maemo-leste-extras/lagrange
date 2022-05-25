#pragma once

/** @file the_Foundation/sortedarray.h  SortedArray of sorted unique values.

Elements of an Array are stored sequentially in a single block of memory. When
elements are inserted, the data is copied into the Array.

SortedArray is an Array whose elements are sorted according to a comparison function.
It is best suited when there is a relatively small amount of insertions/removals
but lots of searches.

@par Complexity

- Insert: O(n)
- Removal: O(n)
- Search: O(log n)

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

iDeclareType(SortedArray)

typedef int (*iSortedArrayCompareElemFunc)(const void *, const void *);

struct Impl_SortedArray {
    iArray values;
    iSortedArrayCompareElemFunc cmp;
};

iDeclareTypeConstructionArgs(SortedArray, size_t elementSize, iSortedArrayCompareElemFunc cmp)

iSortedArray *  copy_SortedArray    (const iSortedArray *);
iBool           contains_SortedArray(const iSortedArray *, const void *value);
iBool           locate_SortedArray  (const iSortedArray *, const void *value, size_t *pos_out);

iLocalDef size_t size_SortedArray(const iSortedArray *d) {
    return size_Array(&d->values);
}

/**
 * Locates a range of elements in the array.
 *
 * @param value     Value to be used for comparisons.
 * @param relaxed   Optional comparison function that is more relaxed than the array's
 *                  own element comparison function. This allows locating a range of
 *                  elements that match partially. The relaxed comparison must be
 *                  compatible with the array's comparison function. Set to NULL to use
 *                  the array's comparison function.
 *
 * @return Located range of elements. This is an empty range if nothing was found to match.
 */
iRanges     locateRange_SortedArray (const iSortedArray *, const void *value, iSortedArrayCompareElemFunc relaxed);

#define     at_SortedArray(d, pos)  at_Array(&(d)->values, pos)
#define     isEmpty_SortedArray(d)  isEmpty_Array(&(d)->values)

iLocalDef const void * constAt_SortedArray      (const iSortedArray *d, size_t pos) { return constAt_Array(&d->values, pos); }
iLocalDef const void * constFront_SortedArray   (const iSortedArray *d) { return constFront_Array(&d->values); }
iLocalDef const void * constBack_SortedArray    (const iSortedArray *d) { return constBack_Array(&d->values); }

void        clear_SortedArray   (iSortedArray *);
iBool       insert_SortedArray  (iSortedArray *, const void *value); /* returns true if inserted/replaced */
iBool       remove_SortedArray  (iSortedArray *, const void *value);

/**
 * Inserts a new element, or replaces an existing one if the provided predicate is true.
 *
 * @param value  Value to be inserted.
 * @param pred   Predicate callback that returns non-zero if the @a value should be inserted.
 *               Called to compare an existing value and the new value. Called as: `pred(new, old)`
 *
 * @return True if @a value was inserted to the array.
 */
iBool       insertIf_SortedArray(iSortedArray *, const void *value, iSortedArrayCompareElemFunc pred);

iLocalDef void removeRange_SortedArray(iSortedArray *d, iRanges range) {
    removeRange_Array(&d->values, range);
}

iEndPublic
