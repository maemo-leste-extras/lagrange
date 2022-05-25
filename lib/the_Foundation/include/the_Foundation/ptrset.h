#pragma once

/** @file the_Foundation/ptrset.h  Set of unique pointers.

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

#include "sortedarray.h"
#include "class.h"

iBeginPublic

typedef iSortedArray iPtrSet;

iDeclareTypeConstruction(PtrSet)
iDeclareType(Stream)

iPtrSet *   copy_PtrSet     (const iPtrSet *);

/** Constructs a PtrSet with a custom comparison function. The default comparison
    function is used if @a cmp is NULL. */
iPtrSet *   newCmp_PtrSet   (iSortedArrayCompareElemFunc cmp);

void        initCmp_PtrSet  (iPtrSet *, iSortedArrayCompareElemFunc cmp);

iBool       contains_PtrSet (const iPtrSet *, const void *ptr);
iBool       locate_PtrSet   (const iPtrSet *, const void *ptr, size_t *pos_out);
void *      at_PtrSet       (const iPtrSet *, size_t pos);

#define     isEmpty_PtrSet(d)   isEmpty_SortedArray(d)
#define     size_PtrSet(d)      size_SortedArray(d)

iLocalDef void *    front_PtrSet  (const iPtrSet *d) { return at_PtrSet(d, 0); }
iLocalDef void *    back_PtrSet   (const iPtrSet *d) { return at_PtrSet(d, size_PtrSet(d) - 1); }

iBool       insert_PtrSet   (iPtrSet *, const void *ptr);
iBool       remove_PtrSet   (iPtrSet *, const void *ptr);

iLocalDef void  clear_PtrSet    (iPtrSet *d) { clear_SortedArray(d); }

void        serializeObjects_PtrSet     (const iPtrSet *, iStream *);
void        deserializeObjects_PtrSet   (iPtrSet *, iStream *ins, const iAnyClass *);
iPtrSet *   newStreamObjects_PtrSet     (iStream *ins, const iAnyClass *);

/** @name Iterators */
///@{
iDeclareIterator(PtrSet, iPtrSet *)
iDeclareConstIterator(PtrSet, const iPtrSet *)
struct IteratorImpl_PtrSet {
    union {
        void **value;
        iArrayIterator iter;
    };
};

void    remove_PtrSetIterator   (iPtrSetIterator *);

struct ConstIteratorImpl_PtrSet {
    union {
        const void **value;
        iArrayConstIterator iter;
    };
};
///@}

iEndPublic
