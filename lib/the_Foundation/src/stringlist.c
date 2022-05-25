/** @file stringlist.c  List of strings.

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

#include "the_Foundation/stringlist.h"
#include "the_Foundation/stringarray.h"

#include <stdlib.h>
#include <stdarg.h>

#define iStringListMaxStringsPerNode    1024

iDeclareType(StringListNode)

struct Impl_StringListNode {
    iListNode node;
    iStringArray strings;
};

static void init_StringListNode_(iStringListNode *d) {
    init_StringArray(&d->strings);
}

static void deinit_StringListNode_(iStringListNode *d) {
    if (d) {
        deinit_StringArray(&d->strings);
    }
}

static size_t size_StringListNode_(const iStringListNode *d) {
    return size_StringArray(&d->strings);
}

#define next_StringListNode_(d)         ((iStringListNode *) ((const iStringListNode *) (d))->node.next)
#define prev_StringListNode_(d)         ((iStringListNode *) ((const iStringListNode *) (d))->node.prev)
#define constNext_StringListNode_(d)    ((const iStringListNode *) ((const iStringListNode *) (d))->node.next)
#define constPrev_StringListNode_(d)    ((const iStringListNode *) ((const iStringListNode *) (d))->node.prev)

iDefineStaticTypeConstruction(StringListNode)

/*-------------------------------------------------------------------------------------*/

iDefineClass(StringList)
iDefineObjectConstruction(StringList)

#define isEnd_StringList_(d, node)  ((node) == (const iStringListNode *) end_List(&(d)->list))

iStringList *newStrings_StringList(const iString *str, ...) {
    iStringList *d = new_StringList();
    iForVarArgs(const iString *, str, pushBack_StringList(d, str));
    return d;
}

iStringList *newStringsCStr_StringList(const char *cstr, ...) {
    iStringList *d = new_StringList();
    iForVarArgs(const char *, cstr, pushBackCStr_StringList(d, cstr));
    return d;
}

void init_StringList(iStringList *d) {
    init_List(&d->list);
    d->size = 0;
}

void deinit_StringList(iStringList *d) {
    clear_StringList(d);
    deinit_List(&d->list);
}

void clear_StringList(iStringList *d) {
    while (!isEmpty_List(&d->list)) {
        delete_StringListNode_(popBack_List(&d->list));
    }
    d->size = 0;
}

static iStringListNode *locateNode_StringList_(const iStringList *d, size_t pos, size_t *start_out) {
    iAssert(pos < d->size);
    if (isEmpty_List(&d->list)) {
        return NULL;
    }
    const iBool forwards = (pos < d->size / 2);
    iStringListNode *node = (forwards? front_List(&d->list) : back_List(&d->list));
    iRanges range;
    range.start = (forwards? 0 : (d->size - size_StringListNode_(node)));
    setSize_Range(&range, size_StringListNode_(node));
    while (!contains_Range(&range, pos)) {
        /* Advance the front node. */
        if (forwards) {
            range.start = range.end;
            node = next_StringListNode_(node);
            setSize_Range(&range, size_StringListNode_(node));
        }
        else {
            range.end = range.start;
            node = prev_StringListNode_(node);
            range.start = range.end - size_StringListNode_(node);
        }
    }
    *start_out = range.start;
    return node;
}

const iString *constAt_StringList(const iStringList *d, size_t pos) {
    size_t start;
    const iStringListNode *node = locateNode_StringList_(d, pos, &start);
    if (node) {
        return constAt_StringArray(&node->strings, pos - start);
    }
    return NULL;
}

iString *at_StringList(iStringList *d, size_t pos) {
    return iConstCast(iString *, constAt_StringList(d, pos));
}

static iStringListNode *backNode_StringList_(iStringList *d) {
    if (isEmpty_List(&d->list)) {
        pushBack_List(&d->list, new_StringListNode_());
    }
    return back_List(&d->list);
}

static iStringListNode *frontNode_StringList_(iStringList *d) {
    if (isEmpty_List(&d->list)) {
        pushFront_List(&d->list, new_StringListNode_());
    }
    return front_List(&d->list);
}

static void debug_StringList_(const iStringList *d) {
    printf("%4zu", size_StringList(d));
    iConstForEach(List, i, &d->list) {
        const iStringListNode *node = (const iAny *) i.value;
        printf(" [%2zu]", size_StringArray(&node->strings));
    }
    printf("\n");
}

static void splitNode_StringList_(iStringList *d, iStringListNode *node) {
    const size_t count = size_StringListNode_(node);
    if (count > iStringListMaxStringsPerNode) {
        iStringListNode *half = new_StringListNode_();
        move_StringArray(&node->strings, (iRanges){ count / 2, count }, &half->strings, 0);
        insertAfter_List(&d->list, node, half);
        debug_StringList_(d);
    }
}

static void mergeIntoAndRemoveNode_StringList_(iStringList *d, iStringListNode *from,
                                               iStringListNode *to) {
    /* Move all the strings to the previous node. */
    move_StringArray(&from->strings,
                     (iRanges){ 0, size_StringListNode_(from) },
                     &to->strings,
                     to == prev_StringListNode_(from)? size_StringListNode_(to) : 0);
    remove_List(&d->list, from);
    delete_StringListNode_(from);
    debug_StringList_(d);
}

static void mergeNode_StringList_(iStringList *d, iStringListNode *node) {
    const size_t count = size_StringListNode_(node);
    if (count < iStringListMaxStringsPerNode / 2) {
        for (int side = 0; side < 2; ++side) {
            iStringListNode *adjacent = (side? prev_StringListNode_(node) : next_StringListNode_(node));
            if (!isEnd_StringList_(d, adjacent)) {
                if (size_StringListNode_(adjacent) < iStringListMaxStringsPerNode / 2) {
                    mergeIntoAndRemoveNode_StringList_(d, node, adjacent);
                    return;
                }
            }
        }
    }
}

void pushBack_StringList(iStringList *d, const iString *str) {
    iStringListNode *node = backNode_StringList_(d);
    pushBack_StringArray(&node->strings, str);
    d->size++;
    splitNode_StringList_(d, node);
}

void pushBackCStr_StringList(iStringList *d, const char *cstr) {
    pushBackCStrN_StringList(d, cstr, strlen(cstr));
}

void pushBackCStrN_StringList(iStringList *d, const char *cstr, size_t size) {
    iString str; initCStrN_String(&str, cstr, size);
    pushBack_StringList(d, &str);
    deinit_String(&str);
}

void pushFront_StringList(iStringList *d, const iString *str) {
    iStringListNode *node = frontNode_StringList_(d);
    pushFront_StringArray(&node->strings, str);
    d->size++;
    splitNode_StringList_(d, node);
}

void pushFrontCStr_StringList(iStringList *d, const char *cstr) {
    iString str; initCStr_String(&str, cstr);
    pushFront_StringList(d, &str);
    deinit_String(&str);
}

void insert_StringList(iStringList *d, size_t pos, const iString *str) {
    if (pos == 0) {
        pushFront_StringList(d, str);
    }
    else if (pos >= size_StringList(d)) {
        pushBack_StringList(d, str);
    }
    else {
        size_t start = 0;
        iStringListNode *node = locateNode_StringList_(d, pos, &start);
        insert_StringArray(&node->strings, pos - start, str);
        d->size++;
        splitNode_StringList_(d, node);
    }
}

void insertCStr_StringList(iStringList *d, size_t pos, const char *cstr) {
    iString str; initCStr_String(&str, cstr);
    insert_StringList(d, pos, &str);
    deinit_String(&str);
}

void popBack_StringList(iStringList *d) {
    if (!isEmpty_StringList(d)) {
        remove_StringList(d, size_StringList(d) - 1);
    }
}

void popFront_StringList(iStringList *d) {
    if (!isEmpty_StringList(d)) {
        remove_StringList(d, 0);
    }
}

void remove_StringList(iStringList *d, size_t pos) {
    delete_String(take_StringList(d, pos));
}

iString *take_StringList(iStringList *d, size_t pos) {
    size_t start;
    iStringListNode *node = locateNode_StringList_(d, pos, &start);
    if (!node) return NULL;
    iString *str = take_StringArray(&node->strings, pos - start);
    mergeNode_StringList_(d, node);
    d->size--;
    return str;
}

iString *joinCStr_StringList(const iStringList *d, const char *delim) {
    iString *joined = new_String();
    iConstForEach(StringList, i, d) {
        if (delim && i.pos > 0) {
            appendCStr_String(joined, delim);
        }
        append_String(joined, i.value);
    }
    return joined;
}

/*-------------------------------------------------------------------------------------*/

#define updateValue_StringListIterator(d)   \
    {(d)->value = at_StringArray(&((iStringListNode *) (d)->node)->strings, (d)->nodePos);}

#define constUpdateValue_StringListIterator(d)   \
    {(d)->value = constAt_StringArray(&((const iStringListNode *) (d)->node)->strings, (d)->nodePos);}

void init_StringListIterator(iStringListIterator *d, iStringList *list) {
    d->node = front_List(&list->list);
    d->list = list;
    d->pos = 0;
    if (size_StringList(list) == 0) {
        d->value = NULL;
    }
    else {
        d->nodePos = 0;
        updateValue_StringListIterator(d);
    }
}

void next_StringListIterator(iStringListIterator *d) {
    d->pos++;
    d->nodePos++;
    if (d->nodePos == size_StringListNode_(d->node)) {
        d->nodePos = 0;
        d->node = next_StringListNode_(d->node);
        if (isEnd_StringList_(d->list, d->node)) {
            d->value = NULL;
            return;
        }
    }
    updateValue_StringListIterator(d);
}

void init_StringListConstIterator(iStringListConstIterator *d, const iStringList *list) {
    d->node = front_List(&list->list);
    d->list = list;
    d->pos = 0;
    if (size_StringList(list) == 0) {
        d->value = NULL;
    }
    else {
        d->nodePos = 0;
        constUpdateValue_StringListIterator(d);
    }
}

void next_StringListConstIterator(iStringListConstIterator *d) {
    d->pos++;
    d->nodePos++;
    if (d->nodePos == size_StringListNode_(d->node)) {
        d->nodePos = 0;
        d->node = next_StringListNode_(d->node);
        if (isEnd_StringList_(d->list, d->node)) {
            d->value = NULL;
            return;
        }
    }
    constUpdateValue_StringListIterator(d);
}

