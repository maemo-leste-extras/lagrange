/** @file the_Foundation/stringset.c  Sorted array of strings.

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

#include "the_Foundation/stringset.h"

iDefineObjectConstruction(StringSet)

static int cmp_StringSet_(const iString *a, const iString *b) {
    return cmpString_String(a, b);
}

iStringSet *newCmp_StringSet(iStringSetCompareFunc cmp) {
    iStringSet *d = new_StringSet();
    d->strings.cmp = (iSortedArrayCompareElemFunc) cmp;
    return d;
}

void init_StringSet(iStringSet *d) {
    init_SortedArray(&d->strings, sizeof(iString), (iSortedArrayCompareElemFunc) cmp_StringSet_);
}

void deinit_StringSet(iStringSet *d) {
    clear_StringSet(d);
    deinit_SortedArray(&d->strings);
}

iStringSet *copy_StringSet(const iStringSet *d) {
    iStringSet *copy = newCmp_StringSet((iStringSetCompareFunc) d->strings.cmp);
    resize_Array(&copy->strings.values, size_StringSet(d));
    for (size_t i = 0; i < size_StringSet(d); i++) {
        initCopy_String(at_StringSet(copy, i), constAt_StringSet(d, i));
    }
    return copy;
}

iBool contains_StringSet(const iStringSet *d, const iString *value) {
    return contains_SortedArray(&d->strings, value);
}

iBool locate_StringSet(const iStringSet *d, const iString *value, size_t *pos_out) {
    return locate_SortedArray(&d->strings, value, pos_out);
}

iRanges locateRange_StringSet(const iStringSet *d, const iString *value,
                              iStringSetCompareFunc relaxed) {
    return locateRange_SortedArray(&d->strings, value, (iSortedArrayCompareElemFunc) relaxed);
}

void clear_StringSet(iStringSet *d) {
    iForEach(Array, i, &d->strings.values) {
        deinit_String(i.value);
    }
    clear_SortedArray(&d->strings);
}

iBool insert_StringSet(iStringSet *d, const iString *value) {
    iString elem;
    initCopy_String(&elem, value);
    if (!insert_SortedArray(&d->strings, &elem)) {
        deinit_String(&elem);
        return iFalse;
    }
    return iTrue;
}

iBool remove_StringSet(iStringSet *d, const iString *value) {
    size_t pos;
    if (locate_SortedArray(&d->strings, value, &pos)) {
        deinit_String(at_StringSet(d, pos));
        remove_Array(&d->strings.values, pos);
        return iTrue;
    }
    return iFalse;
}

iBool insertIf_StringSet(iStringSet *d, const iString *value, iStringSetCompareFunc pred) {
    return insertIf_SortedArray(&d->strings, value, (iSortedArrayCompareElemFunc) pred);
}

iString *joinCStr_StringSet(const iStringSet *d, const char *sep) {
    iString *joined = new_String();
    iConstForEach(StringSet, i, d) {
        if (sep && index_StringSetConstIterator(&i) > 0) {
            appendCStr_String(joined, sep);
        }
        append_String(joined, i.value);
    }
    return joined;
}

iDefineClass(StringSet)

/*----------------------------------------------------------------------------------------------*/

void init_StringSetConstIterator(iStringSetConstIterator *d, const iStringSet *ssa) {
    init_ArrayConstIterator(&d->iter, &ssa->strings.values);
}

void next_StringSetConstIterator(iStringSetConstIterator *d) {
    next_ArrayConstIterator(&d->iter);
}

void init_StringSetReverseConstIterator(iStringSetReverseConstIterator *d, const iStringSet *set) {
    init_ArrayReverseConstIterator(&d->iter, &set->strings.values);
}

void next_StringSetReverseConstIterator(iStringSetReverseConstIterator *d) {
    next_ArrayReverseConstIterator(&d->iter);
}
