#pragma once

/** @file the_Foundation/objectlist.h  List of objects.

ObjectList is itself an Object.

ObjectList owns its nodes, so deleting the list will delete all the nodes and
release references to the corresponding objects.

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

#include "list.h"
#include "object.h"

iBeginPublic

iDeclareClass(ObjectList)

iDeclareType(ObjectListNode)

struct Impl_ObjectList {
    iObject object;
    iList list;
};

struct Impl_ObjectListNode {
    iListNode node;
    iObject *object;
};

#define next_ObjectListNode(d)      ((iObjectListNode *) ((d) ? (d)->node.next : NULL))
#define prev_ObjectListNode(d)      ((iObjectListNode *) ((d) ? (d)->node.prev : NULL))
#define object_ObjectListNode(d)    ((iAnyObject *) ((const iObjectListNode *) (d))->object)

iDeclareObjectConstruction(ObjectList)

iObjectList *       copy_ObjectList     (const iObjectList *);

iLocalDef iBool     isEmpty_ObjectList  (const iObjectList *d) { return d == NULL || isEmpty_List(&d->list); }
iLocalDef size_t    size_ObjectList     (const iObjectList *d) { return d ? size_List(&d->list) : 0; }
#define             list_ObjectList(d)  (&(d)->list)

iObject *       front_ObjectList        (const iObjectList *);
iObject *       back_ObjectList         (const iObjectList *);

#define         begin_ObjectList(d)         ((iObjectListNode *) (d)->list.root.next)
#define         end_ObjectList(d)           ((iObjectListNode *) &(d)->list.root)
#define         constBegin_ObjectList(d)    ((const iObjectListNode *) (d)->list.root.next)
#define         constEnd_ObjectList(d)      ((const iObjectListNode *) &(d)->list.root)

void            clear_ObjectList        (iObjectList *);

iAnyObject *    pushBack_ObjectList     (iObjectList *, iAnyObject *object);
iAnyObject *    pushFront_ObjectList    (iObjectList *, iAnyObject *object);
iAnyObject *    insertAfter_ObjectList  (iObjectList *, iObjectListNode *after, iObject *object);
iAnyObject *    insertBefore_ObjectList (iObjectList *, iObjectListNode *before, iObject *object);
void            removeNode_ObjectList   (iObjectList *, iObjectListNode *node);
void            popFront_ObjectList     (iObjectList *);
void            popBack_ObjectList      (iObjectList *);

/**
 * Pops the front object. Caller is responsible for releasing the returned object.
 */
iLocalDef iAnyObject *takeFront_ObjectList(iObjectList *d) {
    iAnyObject *obj = ref_Object(front_ObjectList(d));
    if (obj) iAssertIsObject(d);
    popFront_ObjectList(d);
    return obj;
}

/**
 * Pops the back object. Caller is responsible for releasing the returned object.
 */
iLocalDef iAnyObject *takeBack_ObjectList(iObjectList *d) {
    iAnyObject *obj = ref_Object(back_ObjectList(d));
    popBack_ObjectList(d);
    return obj;
}

/** @name Iterators */
///@{
iDeclareIterator(ObjectList, iObjectList *)
void            remove_ObjectListIterator(iObjectListIterator *);
struct IteratorImpl_ObjectList {
    iObjectListNode *value;
    iObjectListNode *next;
    iAnyObject *object;
    iObjectList *list;
};

iDeclareConstIterator(ObjectList, const iObjectList *)
struct ConstIteratorImpl_ObjectList {
    const iObjectListNode *value;
    const iAnyObject *object;
    const iObjectList *list;
};
///@}

iEndPublic
