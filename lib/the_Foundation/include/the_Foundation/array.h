#pragma once

/** @file the_Foundation/array.h  Array of sequential fixed-size elements.

Array elements are packed sequentially inside a single block of memory. New elements
can be efficiently added to or removed from the start/end of the array.

                 start                          end
                   v                             v
    | ....... | Element | Element | Element | ....... |

@par Complexity

- Push back: amortized O(1)
- Push front: amortized O(1)
- Insert: O(n)
- Pop back: O(1)
- Pop front: O(1)

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

iBeginPublic

struct Impl_Array {
    char *data;
    iRanges range; // elements
    size_t allocSize; // elements
    size_t elementSize; // bytes
};

iDeclareType(Array)

iDeclareTypeConstructionArgs(Array, size_t elementSize)

iArray *    newN_Array          (size_t elementSize, const void *value, size_t count);
iArray *    newElements_Array   (size_t elementSize, const void *value, ...);
iArray *    copy_Array          (const iArray *);
iArray *    collectNew_Array    (size_t elementSize);

void        initCopy_Array      (iArray *, const iArray *other);

void *       data_Array      (iArray *);
const void * constData_Array (const iArray *);
void *       at_Array        (iArray *, size_t pos);
const void * constAt_Array   (const iArray *, size_t pos);
void *       end_Array       (iArray *);
const void * constEnd_Array  (const iArray *);
size_t       indexOf_Array   (const iArray *, const void *element);

#define value_Array(d, index, type)         (*(type *) at_Array((d), (index)))
#define constValue_Array(d, index, type)    (*(const type *) constAt_Array((d), (index)))

size_t       size_Array      (const iArray *);
iBool        equal_Array     (const iArray *, const iArray *other);

iLocalDef iBool     isEmpty_Array   (const iArray *d) { return d == NULL || isEmpty_Range(&d->range); }
iLocalDef void *    front_Array     (iArray *d) { return at_Array(d, 0); }
iLocalDef void *    back_Array      (iArray *d) { return at_Array(d, size_Array(d) - 1); }
iLocalDef const void * constFront_Array (const iArray *d) { return constAt_Array(d, 0); }
iLocalDef const void * constBack_Array  (const iArray *d) { return constAt_Array(d, size_Array(d) - 1); }

void        reserve_Array   (iArray *, size_t reservedSize);
void        clear_Array     (iArray *);
void        resize_Array    (iArray *, size_t size);
void        fill_Array      (iArray *, char value);
void        sort_Array      (iArray *, int (*cmp)(const void *, const void *));

void        setN_Array      (iArray *, size_t pos, const void *value, size_t count);
void        pushBackN_Array (iArray *, const void *value, size_t count);
void        pushFrontN_Array(iArray *, const void *value, size_t count);
size_t      popBackN_Array  (iArray *, size_t count);
size_t      popFrontN_Array (iArray *, size_t count);
size_t      takeN_Array     (iArray *, size_t pos, void *value_out, size_t count);
void        insertN_Array   (iArray *, size_t pos, const void *value, size_t count);
void        removeN_Array   (iArray *, size_t pos, size_t count);
void        move_Array      (iArray *, iRanges range, iArray *dest, size_t destPos);

iLocalDef void  set_Array           (iArray *d, size_t pos, const void *value)  { setN_Array(d, pos, value, 1); }
iLocalDef void  setCopy_Array       (iArray *d, const iArray *other)            { deinit_Array(d); initCopy_Array(d, other); }
iLocalDef void  pushBack_Array      (iArray *d, const void *value)              { pushBackN_Array(d, value, 1); }
iLocalDef void  pushFront_Array     (iArray *d, const void *value)              { pushFrontN_Array(d, value, 1); }
iLocalDef iBool popBack_Array       (iArray *d)                                 { return popBackN_Array(d, 1) > 0; }
iLocalDef iBool popFront_Array      (iArray *d)                                 { return popFrontN_Array(d, 1); }
iLocalDef void  insert_Array        (iArray *d, size_t pos, const void *value)  { insertN_Array(d, pos, value, 1); }
iLocalDef void  remove_Array        (iArray *d, size_t pos)                     { removeN_Array(d, pos, 1); }
iLocalDef void  removeRange_Array   (iArray *d, iRanges range)                  { removeN_Array(d, range.start, size_Range(&range)); }
iLocalDef iBool take_Array          (iArray *d, size_t pos, void *value_out)    { return takeN_Array(d, pos, value_out, 1) > 0; }

/** @name Iterators */
///@{
iDeclareIterator(Array, iArray *)
iDeclareConstIterator(Array, const iArray *)

size_t index_ArrayIterator(const iArrayIterator *);
size_t index_ArrayConstIterator(const iArrayConstIterator *);
size_t index_ArrayReverseIterator(const iArrayReverseIterator *);
size_t index_ArrayReverseConstIterator(const iArrayReverseConstIterator *);

void   remove_ArrayIterator (iArrayIterator *);

struct IteratorImpl_Array {
    void *value; // address of element
    iArray *array;
    size_t pos;
};
struct ConstIteratorImpl_Array {
    const void *value; // address of element
    const iArray *array;
    const void *end;
};
///@}

iEndPublic
