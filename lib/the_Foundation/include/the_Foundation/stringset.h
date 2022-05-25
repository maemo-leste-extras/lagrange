#pragma once

/** @file the_Foundation/stringset.h  Sorted array of strings.

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

#include "object.h"
#include "sortedarray.h"
#include "string.h"

iBeginPublic

iDeclareClass(StringSet)

typedef int (*iStringSetCompareFunc)(const iString *, const iString *);

struct Impl_StringSet {
    iObject object;
    iSortedArray strings;
};

iDeclareObjectConstruction(StringSet)

iStringSet *newCmp_StringSet    (iStringSetCompareFunc cmp);

iStringSet *copy_StringSet      (const iStringSet *);
iBool       contains_StringSet  (const iStringSet *, const iString *value);
iBool       locate_StringSet    (const iStringSet *, const iString *value, size_t *pos_out);

iLocalDef size_t size_StringSet(const iStringSet *d) {
    return size_SortedArray(&d->strings);
}

/**
 * Locates a range of strings in the array.
 *
 * @param str       String to be used for comparisons.
 * @param relaxed   Optional comparison function that is more relaxed than the array's
 *                  own element comparison function. This allows locating a range of
 *                  elements that match partially. The relaxed comparison must be
 *                  compatible with the array's comparison function. Set to NULL to use
 *                  the array's comparison function.
 *
 * @return Located range of elements. This is an empty range if nothing was found to match.
 */
iRanges     locateRange_StringSet (const iStringSet *, const iString *value,
                                   iStringSetCompareFunc relaxed);

#define     at_StringSet(d, pos)  at_SortedArray(&(d)->strings, pos)
#define     isEmpty_StringSet(d)  isEmpty_SortedArray(&(d)->strings)

iLocalDef const iString *constAt_StringSet(const iStringSet *d, size_t pos) {
    return constAt_SortedArray(&d->strings, pos);
}
iLocalDef const iString *constFront_StringSet(const iStringSet *d) {
    return constFront_SortedArray(&d->strings);
}
iLocalDef const iString *constBack_StringSet(const iStringSet *d) {
    return constBack_SortedArray(&d->strings);
}

void        clear_StringSet   (iStringSet *);
iBool       insert_StringSet  (iStringSet *, const iString *value); /* returns true if inserted/replaced */
iBool       remove_StringSet  (iStringSet *, const iString *value);

/**
 * Inserts a new element, or replaces an existing one if the provided predicate is true.
 *
 * @param value  Value to be inserted.
 * @param pred   Predicate callback that returns non-zero if the @a value should be inserted.
 *               Called to compare an existing value and the new value. Called as: `pred(new, old)`
 *
 * @return True if @a value was inserted to the array.
 */
iBool       insertIf_StringSet  (iStringSet *, const iString *value, iStringSetCompareFunc pred);

iLocalDef void removeRange_StringSet(iStringSet *d, iRanges range) {
    removeRange_SortedArray(&d->strings, range);
}

iString *   joinCStr_StringSet  (const iStringSet *, const char *sep);

/** @name Iterators */
///@{
iDeclareConstIterator(StringSet, const iStringSet *)

#define index_StringSetConstIterator(d)   index_ArrayConstIterator(&(d)->iter)

struct ConstIteratorImpl_StringSet {
    union {
        const iString *value;
        iArrayConstIterator iter;
    };
};
///@}

iEndPublic
