#pragma once

/** @file the_Foundation/intset.h  Set of unique integers.

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

#include "sortedarray.h"

iBeginPublic

typedef iSortedArray iIntSet;
typedef int (*iIntSetCompareFunc)(const int *, const int *);

iDeclareType(Stream)

iDeclareTypeConstruction(IntSet)
iDeclareTypeSerialization(IntSet)

iIntSet *   copy_IntSet     (const iIntSet *);

/** Constructs an IntSet with a custom comparison function. The default comparison
    function is used if @a cmp is NULL. */
iIntSet *   newCmp_IntSet   (iIntSetCompareFunc cmp);

void        initCmp_IntSet  (iIntSet *, iIntSetCompareFunc cmp);

iBool       contains_IntSet (const iIntSet *, int value);
iBool       locate_IntSet   (const iIntSet *, int value, size_t *pos_out);
int         at_IntSet       (const iIntSet *, size_t pos);

#define     isEmpty_IntSet(d)   isEmpty_SortedArray(d)
#define     size_IntSet(d)      size_SortedArray(d)

iLocalDef int   front_IntSet  (const iIntSet *d) { return at_IntSet(d, 0); }
iLocalDef int   back_IntSet   (const iIntSet *d) { return at_IntSet(d, size_IntSet(d) - 1); }
iLocalDef void  clear_IntSet  (iIntSet *d) { clear_SortedArray(d); }

iBool       insert_IntSet   (iIntSet *, int value);
iBool       remove_IntSet   (iIntSet *, int value);

/** @name Iterators */
///@{
iDeclareIterator(IntSet, iIntSet *)
iDeclareConstIterator(IntSet, const iIntSet *)

struct IteratorImpl_IntSet {
    union {
        int *value;
        iArrayIterator iter;
    };
};

void    remove_IntSetIterator   (iIntSetIterator *);

struct ConstIteratorImpl_IntSet {
    union {
        const int *value;
        iArrayConstIterator iter;
    };
};
///@}

iEndPublic
