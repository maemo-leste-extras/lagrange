/** @file objectlist.c  List of objects.

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

#include "the_Foundation/objectlist.h"

#include <stdlib.h>

iDefineClass(ObjectList)
iDefineObjectConstruction(ObjectList)

void init_ObjectList(iObjectList *d) {
    init_List(&d->list);
}

void deinit_ObjectList(iObjectList *d) {
    clear_ObjectList(d);
}

iObjectList *copy_ObjectList(const iObjectList *d) {
    iObjectList *copy = new_ObjectList();
    iConstForEach(ObjectList, i, d) {
        pushBack_ObjectList(copy, iConstCast(iObject *, i.object));
    }
    return copy;
}

iObject *front_ObjectList(const iObjectList *d) {
    if (isEmpty_ObjectList(d)) return NULL;
    return ((const iObjectListNode *) front_List(&d->list))->object;
}

iObject *back_ObjectList(const iObjectList *d) {
    if (isEmpty_ObjectList(d)) return NULL;
    return ((const iObjectListNode *) back_List(&d->list))->object;
}

void clear_ObjectList(iObjectList *d) {
    iReverseForEach(ObjectList, i, d) {
        removeNode_ObjectList(d, i.value);
    }
}

static iObjectListNode *new_ObjectListNode_(iAnyObject *object) {
    iObjectListNode *d = iMalloc(ObjectListNode);
    d->object = ref_Object(object);
    return d;
}

static void delete_ObjectListNode_(iAny *any) {
    if (any) {
        iObjectListNode *d = any;
        deref_Object(d->object);
        free(d);
    }
}

iAnyObject *pushBack_ObjectList(iObjectList *d, iAnyObject *object) {
    pushBack_List(&d->list, new_ObjectListNode_(object));
    return object;
}

iAnyObject *pushFront_ObjectList(iObjectList *d, iAnyObject *object) {
    pushFront_List(&d->list, new_ObjectListNode_(object));
    return object;
}

iAnyObject *insertAfter_ObjectList(iObjectList *d, iObjectListNode *after, iObject *object) {
    insertAfter_List(&d->list, after, new_ObjectListNode_(object));
    return object;
}

iAnyObject *insertBefore_ObjectList(iObjectList *d, iObjectListNode *before, iObject *object) {
    insertBefore_List(&d->list, before, new_ObjectListNode_(object));
    return object;
}

void removeNode_ObjectList(iObjectList *d, iObjectListNode *node) {
    delete_ObjectListNode_(remove_List(&d->list, node));
}

void popFront_ObjectList(iObjectList *d) {
    delete_ObjectListNode_(popFront_List(&d->list));
}

void popBack_ObjectList(iObjectList *d) {
    delete_ObjectListNode_(popBack_List(&d->list));
}

/*-------------------------------------------------------------------------------------*/

void init_ObjectListIterator(iObjectListIterator *d, iObjectList *list) {
    iAssert(!list || isInstance_Object(list, &Class_ObjectList));
    d->list = list;
    d->value = (list ? front_List(&list->list) : NULL);
    d->object = (d->value ? d->value->object : NULL);
    d->next = next_ObjectListNode(d->value);
}

void next_ObjectListIterator(iObjectListIterator *d) {
    d->value = d->next;
    d->object = (d->value? d->value->object : NULL);
    if (d->value == end_ObjectList(d->list)) {
        d->value = NULL;
    }
    else {
        d->next = (iObjectListNode *) d->value->node.next;
    }
}

void remove_ObjectListIterator(iObjectListIterator *d) {
    removeNode_ObjectList(d->list, d->value);
}

void init_ObjectListReverseIterator(iObjectListReverseIterator *d, iObjectList *list) {
    iAssert(!list || isInstance_Object(list, &Class_ObjectList));
    d->list = list;
    d->value = (list ? back_List(&list->list) : NULL);
    d->object = (d->value ? d->value->object : NULL);
    d->next = prev_ObjectListNode(d->value);
}

void next_ObjectListReverseIterator(iObjectListReverseIterator *d) {
    d->value = d->next;
    d->object = (d->value? d->value->object : NULL);
    if (d->value == end_ObjectList(d->list)) {
        d->value = NULL;
    }
    else {
        d->next = (iObjectListNode *) d->value->node.prev;
    }
}

void remove_ObjectListReverseIterator(iObjectListReverseIterator *d) {
    removeNode_ObjectList(d->list, d->value);
}

/*-------------------------------------------------------------------------------------*/

void init_ObjectListConstIterator(iObjectListConstIterator *d, const iObjectList *list) {
    iAssert(!list || isInstance_Object(list, &Class_ObjectList));
    d->list = list;
    d->value = (list ? front_List(&list->list) : NULL);
    d->object = (d->value ? d->value->object : NULL);
}

void next_ObjectListConstIterator(iObjectListConstIterator *d) {
    d->value = (const iObjectListNode *) d->value->node.next;
    if (d->value == constEnd_ObjectList(d->list)) {
        d->value = NULL;
    }
    d->object = (d->value? d->value->object : NULL);
}

void init_ObjectListReverseConstIterator(iObjectListReverseConstIterator *d, const iObjectList *list) {
    iAssert(!list || isInstance_Object(list, &Class_ObjectList));
    d->list = list;
    d->value = (list ? back_List(&list->list) : NULL);
    d->object = (d->value ? d->value->object : NULL);
}

void next_ObjectListReverseConstIterator(iObjectListReverseConstIterator *d) {
    d->value = (const iObjectListNode *) d->value->node.prev;
    if (d->value == constEnd_ObjectList(d->list)) {
        d->value = NULL;
    }
    d->object = (d->value? d->value->object : NULL);
}
