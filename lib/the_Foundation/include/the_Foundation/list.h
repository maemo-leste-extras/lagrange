#pragma once

/** @file the_Foundation/list.h  Doubly linked list.

List is doubly-linked, and uses a sentinel node representing the start/end of the list.

    Sentinel <--> Node
       ʌ           ʌ
       |           |
       v           v
      Node <----> Node

List does not have ownership of the nodes. This means the nodes can be any type of object
as long as they are derived from ListNode.

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

iBeginPublic

iDeclareType(List)
iDeclareType(ListNode)

struct Impl_ListNode {
    iListNode *next;
    iListNode *prev;
};

struct Impl_List {
    iListNode root;
    size_t size;
};

iDeclareTypeConstruction(List)

size_t      size_List   (const iList *);
iAny *      front_List  (const iList *);
iAny *      back_List   (const iList *);

#define     begin_List(d)       (&(d)->root.next)
#define     end_List(d)         (&(d)->root)

#define     isEmpty_List(d)     (size_List(d) == 0)

void        clear_List          (iList *);

iAny *      pushBack_List       (iList *, iAny *node);
iAny *      pushFront_List      (iList *, iAny *node);
iAny *      insertAfter_List    (iList *, iAny *after, iAny *node);
iAny *      insertBefore_List   (iList *, iAny *before, iAny *node);
iAny *      remove_List         (iList *, iAny *node);
iAny *      popFront_List       (iList *);
iAny *      popBack_List        (iList *);

typedef int (*iListCompareFunc)(const iAny *, const iAny *);

void        sort_List           (iList *, iListCompareFunc cmp);

/** @name Iterators */
///@{
iDeclareIterator(List, iList *)
iDeclareConstIterator(List, const iList *)
struct IteratorImpl_List {
    iListNode *value;
    iList *list;
    iListNode *next;
};
struct ConstIteratorImpl_List {
    const iListNode *value;
    const iList *list;
    const iListNode *next;
};
///@}

iEndPublic
