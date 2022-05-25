/** @file array.c  Array of sequential fixed-size elements.

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

#include "the_Foundation/array.h"

#include <stdlib.h>

#define iArrayMinAlloc 4

#define element_Array_(d, index) (&(d)->data[(d)->elementSize * (index)])

static void moveElements_Array_(iArray *d, const iRanges *moved, long delta) {
    if (delta != 0) {
        char *ptr = element_Array_(d, moved->start);
        iAssert(moved->start + delta >= 0);
        iAssert(moved->end   + delta <= d->allocSize);
        memmove(ptr + (long) (delta * d->elementSize), ptr, d->elementSize * size_Range(moved));
    }
}

static void shift_Array_(iArray *d, long delta) {
    if (delta != 0) {
        iAssert(d->range.start + delta >= 0);
        iAssert(d->range.end   + delta <= d->allocSize);
        moveElements_Array_(d, &d->range, delta);
        shift_Range(&d->range, delta);
    }
}

static long imbalance_Array_(const iArray *d) {
    /* Calculates the shift required to balance the list elements. */
    const long left  = d->range.start;
    const long right = d->allocSize - d->range.end;
    if (left <= iArrayMinAlloc && right <= iArrayMinAlloc) {
        return 0; /* Too tight, shouldn't move just a little bit. */
    }
    /* The imbalance must be significant. */
    if (left > 2 * right || right > 2 * left) {
        return (right - left) / 2;
    }
    return 0;
}

static void rebalance_Array_(iArray *d) {
    /* Rebalancing occurs if the elements are touching the start or end of the buffer,
       which means elements cannot be added without moving existing ones. */
    if (!isEmpty_Range(&d->range)) {
        const long imbalance = imbalance_Array_(d);
        if (d->range.end == d->allocSize || d->range.start == 0) {
            shift_Array_(d, imbalance);
        }
    }
}

iDefineTypeConstructionArgs(Array, (size_t elemSize), elemSize)

iArray *collectNew_Array(size_t elementSize) {
    return collect_Array(new_Array(elementSize));
}

iArray *copy_Array(const iArray *other) {
    iArray *d = iMalloc(Array);
    initCopy_Array(d, other);
    return d;
}

void init_Array(iArray *d, size_t elementSize) {
    iZap(*d);
    d->elementSize = elementSize;
}

void initCopy_Array(iArray *d, const iArray *other) {
    d->range.start = 0;
    d->range.end = size_Array(other);
    d->allocSize = d->range.end + 1;
    d->elementSize = other->elementSize;
    d->data = malloc(d->allocSize * d->elementSize);
    memcpy(d->data, constData_Array(other), size_Array(other) * d->elementSize);
}

void deinit_Array(iArray *d) {
    free(d->data);
}

void *data_Array(iArray *d) {
    if (isEmpty_Array(d)) return NULL;
    return element_Array_(d, d->range.start);
}

const void *constData_Array(const iArray *d) {
    if (isEmpty_Array(d)) return NULL;
    return element_Array_(d, d->range.start);
}

size_t size_Array(const iArray *d) {
    return size_Range(&d->range);
}

iBool equal_Array(const iArray *d, const iArray *other) {
    if (size_Array(d) != size_Array(other) || d->elementSize != other->elementSize) {
        return iFalse;
    }
    return memcmp(constData_Array(d), constData_Array(other), size_Array(d) * d->elementSize) == 0;
}

void *at_Array(iArray *d, size_t pos) {
    iAssert(pos < size_Range(&d->range));
    return element_Array_(d, d->range.start + pos);
}

const void *constAt_Array(const iArray *d, size_t pos) {
    iAssert(pos < size_Range(&d->range));
    return element_Array_(d, d->range.start + pos);
}

void *end_Array(iArray *d) {
    return element_Array_(d, d->range.end);
}

const void *constEnd_Array(const iArray *d) {
    return element_Array_(d, d->range.end);
}

size_t indexOf_Array(const iArray *d, const void *element) {
    const size_t index =
        (size_t) ((const char *) element - (const char *) constData_Array(d)) / d->elementSize;
    if (index < size_Array(d)) return index;
    return iInvalidPos;
}

void reserve_Array(iArray *d, size_t reservedSize) {
    size_t newSize = (d->allocSize == 0? iArrayMinAlloc : d->allocSize);
    while (newSize < reservedSize) {
        newSize *= 2;
    }
    if (newSize > d->allocSize) {
        d->data = realloc(d->data, d->elementSize * newSize);
        d->allocSize = newSize;
    }
}

void clear_Array(iArray *d) {
    d->range.start = d->range.end = d->allocSize / 2;
}

void resize_Array(iArray *d, size_t size) {
    const size_t oldSize = size_Range(&d->range);
    if (size == oldSize) return;
    if (size < oldSize) {
        setSize_Range(&d->range, size);
        return;
    }
    reserve_Array(d, size);
    if (d->range.start + size > d->allocSize) {
        /* Rebalance according to the new size. */
        const long leftSide = (d->allocSize - size) / 2;
        shift_Array_(d, leftSide - (int) d->range.start);
    }
    setSize_Range(&d->range, size);
    /* Zero newly added elements. */
    memset(element_Array_(d, d->range.start + oldSize), 0, d->elementSize * (size - oldSize));
}

void setN_Array(iArray *d, size_t pos, const void *value, size_t count) {
    iAssert(pos + count <= size_Array(d));
    memcpy(at_Array(d, pos), value, count * d->elementSize);
}

void pushBackN_Array(iArray *d, const void *value, size_t count) {
    insertN_Array(d, size_Array(d), value, count);
}

void pushFrontN_Array(iArray *d, const void *value, size_t count) {
    insertN_Array(d, 0, value, count);
}

size_t popBackN_Array(iArray *d, size_t count) {
    count = iMin(count, size_Array(d));
    removeN_Array(d, size_Array(d) - count, count);
    return count;
}

size_t popFrontN_Array(iArray *d, size_t count) {
    count = iMin(count, size_Array(d));
    removeN_Array(d, 0, count);
    return count;
}

size_t takeN_Array(iArray *d, size_t pos, void *value_out, size_t count) {
    if (pos == iInvalidPos) return 0;
    count = iMin(count, size_Array(d) - pos);
    memcpy(value_out, at_Array(d, pos), count * d->elementSize);
    removeN_Array(d, pos, count);
    return count;
}

void insertN_Array(iArray *d, size_t pos, const void *value, size_t count) {
    if (!count) return;
    iAssert(pos <= size_Array(d));
    reserve_Array(d, size_Array(d) + count);
    /* Map to internal range. */
    pos += d->range.start;
    /* Check for non-splitting insertions first. */
    if (pos == d->range.end)  { /* At the end. */
        if (d->range.end + count > d->allocSize) {
            size_t overrun = d->range.end + count - d->allocSize;
            shift_Array_(d, -(int) overrun);
        }
        memcpy(element_Array_(d, d->range.end), value, count * d->elementSize);
        d->range.end += count;
    }
    else if (pos == d->range.start) { /* At the beginning. */
        if (d->range.start < count) {
            size_t overrun = count - d->range.start;
            shift_Array_(d, overrun);
        }
        d->range.start -= count;
        memcpy(element_Array_(d, d->range.start), value, count * d->elementSize);
    }
    else {
        /* Some of the existing elements must be moved. Below is an
           example of inserting two elements in the middle, at the ^ mark.

           Case A) Left side first:

               [ . A A A B B B B . . . ]
                         ^
               [ A A A . B B B B . . . ]
                       ^ *
               [ A A A . . B B B B . . ]
                       ^ *

           Case B) Right side first:

               [ . . . A A A A B B B . ]
                               ^
               [ . . . A A A A . B B B ]
                               ^
               [ . . A A A A . . B B B ]
                             ^ *
        */
        const size_t splitPos = pos;
        int part[2];
        if (splitPos - d->range.start <= d->range.end - splitPos &&
            count < d->range.start) {
            /* First half is smaller, move it first. However, we require that there is
               some empty space remaning in the beginning to reduce need for balancing. */
            part[0] = 0;
            part[1] = 1;
        }
        else {
            part[0] = 1;
            part[1] = 0;
        }
        size_t needed = count;
        for (int i = 0; i < 2 && needed > 0; ++i) {
            if (part[i] == 0) { /* Moving the first half. */
                long avail = iMin(needed, d->range.start);
                moveElements_Array_(d, &(iRanges){ d->range.start, splitPos }, -avail);
                d->range.start -= avail;
                pos -= avail;
                needed -= avail;
            }
            else if (part[i] == 1) { /* Moving the second half. */
                long avail = iMin(needed, d->allocSize - d->range.end);
                moveElements_Array_(d, &(iRanges) { splitPos, d->range.end }, avail);
                d->range.end += avail;
                needed -= avail;
            }
        }
        iAssert(needed == 0);
        memcpy(element_Array_(d, pos), value, count * d->elementSize);
    }
    rebalance_Array_(d);
}

void removeN_Array(iArray *d, size_t pos, size_t count) {
    if (count == 0) return;
    if (count == iInvalidSize) {
        count = size_Array(d) - pos;
    }
    iAssert(pos < size_Array(d));
    iAssert(pos + count <= size_Array(d));
    pos += d->range.start;
    if (pos == d->range.end - count) {
        d->range.end -= count;
    }
    else if (pos == d->range.start) {
        d->range.start += count;
    }
    else if (pos - d->range.start > d->range.end - pos) {
        moveElements_Array_(d, &(iRanges){ pos + count, d->range.end }, -(int)count);
        d->range.end -= count;
    }
    else {
        moveElements_Array_(d, &(iRanges){ d->range.start, pos }, count);
        d->range.start += count;
    }
}

void fill_Array(iArray *d, char value) {
    memset(front_Array(d), value, d->elementSize * size_Array(d));
}

void move_Array(iArray *d, iRanges range, iArray *dest, size_t destPos) {
    iAssert(d);
    iAssert(dest);
    iAssert(d != dest);
    iAssert(d->elementSize == dest->elementSize);
    const long count = size_Range(&range);
    const long oldDestSize = size_Array(dest);
    resize_Array(dest, oldDestSize + count); // dest size increased
    /* Make room for the moved elements. */
    moveElements_Array_(dest, &(iRanges){ dest->range.start + destPos,
                                          dest->range.start + oldDestSize }, count);
    memcpy(element_Array_(dest, dest->range.start + destPos),
           element_Array_(   d,    d->range.start + range.start),
           d->elementSize * count);
    removeRange_Array(d, range);
}

void sort_Array(iArray *d, int (*cmp)(const void *, const void *)) {
    if (size_Array(d) > 1) {
        qsort(front_Array(d), size_Array(d), d->elementSize, cmp);
    }
}

/*-------------------------------------------------------------------------------------*/

void init_ArrayIterator(iArrayIterator *d, iArray *array) {
    d->array = array;
    d->pos = 0;
    d->value = (array && !isEmpty_Array(array) ? at_Array(array, 0) : NULL);
}

void next_ArrayIterator(iArrayIterator *d) {
    if (++d->pos < size_Array(d->array)) {
        d->value = at_Array(d->array, d->pos);
    }
    else {
        d->value = NULL;
    }
}

size_t index_ArrayIterator(const iArrayIterator *d) {
    return d->pos;
}

void remove_ArrayIterator(iArrayIterator *d) {
    remove_Array(d->array, d->pos--);
}

void init_ArrayConstIterator(iArrayConstIterator *d, const iArray *array) {
    if (array) {
        d->array = array;
        if (!isEmpty_Array(array)) {
            d->value = constFront_Array(array);
            d->end   = element_Array_(d->array, d->array->range.end);
        }
        else {
            d->value = d->end = NULL;
        }
    }
    else {
        iZap(*d);
    }
}

void next_ArrayConstIterator(iArrayConstIterator *d) {
    iAssert(d->value);
    d->value = (const char *) d->value + d->array->elementSize;
    if (d->value >= d->end) {
        d->value = NULL;
    }
}

size_t index_ArrayConstIterator(const iArrayConstIterator *d) {
    return ((const char *) d->value - (const char *) constData_Array(d->array)) /
            d->array->elementSize;
}

void init_ArrayReverseIterator(iArrayIterator *d, iArray *array) {
    d->array = array;
    d->pos   = array ? size_Array(array) - 1 : iInvalidPos;
    d->value = (array && !isEmpty_Array(array) ? at_Array(array, d->pos) : NULL);
}

void next_ArrayReverseIterator(iArrayIterator *d) {
    if (d->pos > 0) {
        d->value = at_Array(d->array, --d->pos);
    }
    else {
        d->pos = iInvalidPos;
        d->value = NULL;
    }
}

size_t index_ArrayReverseIterator(const iArrayIterator *d) {
    return d->pos;
}

void init_ArrayReverseConstIterator(iArrayConstIterator *d, const iArray *array) {
    if (array) {
        d->array = array;
        if (!isEmpty_Array(array)) {
            d->value = constBack_Array(array);
            d->end = element_Array_(d->array, d->array->range.start);
        }
        else {
            d->value = NULL;
            d->end = NULL;
        }
    }
    else {
        iZap(*d);
    }
}

void next_ArrayReverseConstIterator(iArrayConstIterator *d) {
    iAssert(d->value);
    d->value = (const char *) d->value - d->array->elementSize;
    if (d->value < d->end) {
        d->value = NULL;
    }
}

size_t index_ArrayReverseConstIterator(const iArrayReverseConstIterator *d) {
    return index_ArrayConstIterator(d);
}
