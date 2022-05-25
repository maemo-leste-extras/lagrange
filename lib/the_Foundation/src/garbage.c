/** @file garbage.c  Garbage collector.

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

#include "the_Foundation/garbage.h"
#include "the_Foundation/list.h"
#include "the_Foundation/stdthreads.h"
#include "the_Foundation/object.h"

#include <stdio.h>
#include <stdlib.h>

iDeclareType(GarbageNode)
iDeclareType(CollectedPtr)

#define iGarbageNodeMax   32

struct Impl_CollectedPtr {
    void *ptr;
    iDeleteFunc del;
};

struct Impl_GarbageNode {
    iListNode node;
    int count;
    iCollectedPtr allocs[iGarbageNodeMax];
};

iLocalDef iBool isEmpty_GarbageNode_(const iGarbageNode *d) { return d->count == 0; }
iLocalDef iBool isFull_GarbageNode_ (const iGarbageNode *d) { return d->count == iGarbageNodeMax; }

static iGarbageNode *new_GarbageNode_(void) {
    iGarbageNode *d = iMalloc(GarbageNode);
    d->count = 0;
    return d;
}

static iBool popBack_GarbageNode_(iGarbageNode *d) {
    if (d->count > 0) {
        const iCollectedPtr *alloc = d->allocs + (--d->count);
        if (alloc->del) {
            alloc->del(alloc->ptr);
            return iTrue;
        }
    }
    return iFalse;
}

static void delete_GarbageNode_(iGarbageNode *d) {
    while (d->count > 0) {
        popBack_GarbageNode_(d);
    }
    free(d);
}

/*-------------------------------------------------------------------------------------*/

iDeclareType(Collected)

struct Impl_Collected { // Thread-specific.
    iList collected;
    iBool isRecycling;
};

static iCollected *new_Collected_(void) {
    iCollected *d = iMalloc(Collected);
    init_List(&d->collected);
    d->isRecycling = iFalse;
    return d;
}

static void recycle_Collected_(iCollected *d) {
    if (!d->isRecycling && !isEmpty_List(&d->collected)) {
        d->isRecycling = iTrue; /* avoid re-entrant recycling */
        iDebug("[Garbage] recycling %zu allocations\n", size_List(&d->collected));
        iReverseForEach(List, i, &d->collected) {
            delete_GarbageNode_((iGarbageNode *) i.value);
        }
        clear_List(&d->collected);
        d->isRecycling = iFalse;
    }
}

static void delete_Collected_(iCollected *d) {
    recycle_Collected_(d);
    deinit_List(&d->collected);
    free(d);
}

static iBool pop_Collected_(iCollected *d) {
    if (isEmpty_List(&d->collected)) {
        return iFalse;
    }
    iGarbageNode *node = back_List(&d->collected);
    if (isEmpty_GarbageNode_(node) && size_List(&d->collected) > 1) {
        popBack_List(&d->collected);
        delete_GarbageNode_(node);
    }
    return popBack_GarbageNode_(back_List(&d->collected));
}

#if !defined (NDEBUG)
static void *previous_Collected_(const iCollected *d, size_t offset) {
    const iGarbageNode *node = back_List(&d->collected);
    if (!node) return NULL;
    if (offset > (size_t) node->count) {
        if (node == front_List(&d->collected)) {
            return NULL;
        }
        offset -= node->count;
        node = (const iGarbageNode *) node->node.prev;
    }
    if (offset <= (size_t) node->count) {
        return node->allocs[node->count - offset].ptr;
    }
    return NULL;
}
#endif

static void push_Collected_(iCollected *d, iCollectedPtr colptr) {
    iGarbageNode *node = back_List(&d->collected);
    if (!node || isFull_GarbageNode_(node)) {
        pushBack_List(&d->collected, node = new_GarbageNode_());
    }
#if !defined (NDEBUG)
    /* In debug builds, try to catch recent duplicate collections.
       NULL pointers are used as scope markers, and objects are allowed
       to be released multiple times. */
    if (colptr.ptr && colptr.del != (iDeleteFunc) deref_Object) {
        iAssert(previous_Collected_(d, 1) != colptr.ptr);
        iAssert(previous_Collected_(d, 2) != colptr.ptr);
        iAssert(previous_Collected_(d, 3) != colptr.ptr);
        iAssert(previous_Collected_(d, 4) != colptr.ptr);
        iAssert(previous_Collected_(d, 5) != colptr.ptr);
        iAssert(previous_Collected_(d, 6) != colptr.ptr);
        iAssert(previous_Collected_(d, 7) != colptr.ptr);
        iAssert(previous_Collected_(d, 8) != colptr.ptr);
    }
#endif
    node->allocs[node->count++] = colptr;
}

/*-------------------------------------------------------------------------------------*/

static tss_t threadLocal_Garbage_;

void deinitForThread_Garbage_(void) {
    iCollected *d = tss_get(threadLocal_Garbage_);
    if (d) {
        delete_Collected_(d);
        tss_set(threadLocal_Garbage_, NULL);
    }
}

void init_Garbage(void) {
    tss_create(&threadLocal_Garbage_, (tss_dtor_t) delete_Collected_);
}

static iCollected *initForThread_Garbage_(void) {
    iCollected *d = tss_get(threadLocal_Garbage_);
    if (!d) {
        tss_set(threadLocal_Garbage_, d = new_Collected_());
    }
    return d;
}

static iBool pop_Garbage_(void) {
    return pop_Collected_(initForThread_Garbage_());
}

void *collect_Garbage(void *ptr, iDeleteFunc del) {
    push_Collected_(initForThread_Garbage_(), (iCollectedPtr){ ptr, del });
    return ptr;
}

void beginScope_Garbage(void) {
    collect_Garbage(NULL, NULL); // marks beginning of scope
}

void endScope_Garbage(void) {
    int count = 0;
    while (pop_Garbage_()) { count++; }
    if (count) {
        iDebug("[Garbage] recycled %i scope allocations\n", count);
    }
}

void recycle_Garbage(void) {
    iCollected *d = tss_get(threadLocal_Garbage_);
    if (d) {
        recycle_Collected_(d);
    }
}
