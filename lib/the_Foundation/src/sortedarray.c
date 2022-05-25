/** @file sortedarray.c  SortedArray of unique integer values.

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

#include "the_Foundation/sortedarray.h"

#include <stdlib.h>

iDefineTypeConstructionArgs(SortedArray,
                            (size_t elementSize, iSortedArrayCompareElemFunc cmp),
                            elementSize, cmp)

void init_SortedArray(iSortedArray *d, size_t elementSize, iSortedArrayCompareElemFunc cmp) {
    init_Array(&d->values, elementSize);
    d->cmp = cmp;
}

void deinit_SortedArray(iSortedArray *d) {
    deinit_Array(&d->values);
}

iSortedArray *copy_SortedArray(const iSortedArray *other) {
    iSortedArray *d = iMalloc(SortedArray);
    initCopy_Array(&d->values, &other->values);
    d->cmp = other->cmp;
    return d;
}

iBool contains_SortedArray(const iSortedArray *d, const void *value) {
    return locate_SortedArray(d, value, NULL);
}

iBool locate_SortedArray(const iSortedArray *d, const void *value, size_t *pos_out) {
    /* We will narrow down the span until the pointer is found or we'll know where
     * it would be if it were inserted. */
    iRanges span = { 0, size_Array(&d->values) };
    iAssert(span.end == 0 || d->values.data != NULL);
    while (!isEmpty_Range(&span)) {
        /* Narrow down the search by a half. */
        const size_t mid = (span.start + span.end) / 2;
        const int cmp = d->cmp(value, constAt_SortedArray(d, mid));
        if (cmp == 0) {
            /* Oh, it's here. */
            if (pos_out) *pos_out = mid;
            return iTrue;
        }
        else if (cmp > 0) {
            span.start = mid + 1;
        }
        else {
            span.end = mid;
        }
    }
    if (pos_out) *pos_out = span.start;
    return iFalse;
}

iRanges locateRange_SortedArray(const iSortedArray *d, const void *value,
                                iSortedArrayCompareElemFunc relaxed) {
    const iSortedArrayCompareElemFunc cmpFunc = (relaxed ? relaxed : d->cmp);
    if (isEmpty_SortedArray(d) ||
        cmpFunc(value, constFront_SortedArray(d)) < 0) {
        return (iRanges){ 0, 0 };
    }
    if (cmpFunc(value, constBack_SortedArray(d)) > 0) {
        return (iRanges){ size_SortedArray(d), size_SortedArray(d) };
    }
    iRanges range;
    /* Find the beginning of the range. */ {
        iRanges span = { 0, size_SortedArray(d) };
        while (!isEmpty_Range(&span)) {
            const size_t mid = (span.start + span.end) / 2;
            if (cmpFunc(constAt_SortedArray(d, mid), value) >= 0) {
                span.end = mid;
            }
            else {
                span.start = mid + 1;
            }
        }
        range.start = span.end;
    }
    /* Find the end of the range. */ {
        iRanges span = { 0, size_SortedArray(d) };
        while (!isEmpty_Range(&span)) {
            const size_t mid = (span.start + span.end) / 2;
            if (cmpFunc(constAt_SortedArray(d, mid), value) > 0) {
                span.end = mid;
            }
            else {
                span.start = mid + 1;
            }
        }
        range.end = span.start;
    }
    return range;
}

void clear_SortedArray(iSortedArray *d) {
    clear_Array(&d->values);
}

iBool insert_SortedArray(iSortedArray *d, const void *value) {
    return insertIf_SortedArray(d, value, NULL);
}

iBool insertIf_SortedArray(iSortedArray *d, const void *value, iSortedArrayCompareElemFunc pred) {
    size_t pos;
    if (locate_SortedArray(d, value, &pos)) {
        if (!pred || pred(value, at_SortedArray(d, pos))) {
            set_Array(&d->values, pos, value);
            return iTrue;
        }
        return iFalse;
    }
    insert_Array(&d->values, pos, value);
    return iTrue;
}

iBool remove_SortedArray(iSortedArray *d, const void *value) {
    size_t pos;
    if (locate_SortedArray(d, value, &pos)) {
        remove_Array(&d->values, pos);
        return iTrue;
    }
    return iFalse;
}
