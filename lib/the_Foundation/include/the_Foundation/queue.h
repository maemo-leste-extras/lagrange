#pragma once

/** @file the_Foundation/queue.h  Thread-safe queue of objects.

Queue is derived from ObjectList, so it keeps a reference to each object in the queue.
When objects are taken from the queue the reference is passed to the caller, so they are
responsible for releasing taken objects.

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
#include "mutex.h"
#include "objectlist.h"
#include "stdthreads.h"

iBeginPublic

iDeclareClass(Queue)

iDeclareObjectConstruction(Queue)

struct Impl_Queue {
    iObjectList items;
    iMutex mutex;
    iCondition cond;
};

typedef iAnyObject iQueueItem;

void        init_Queue          (iQueue *);
void        deinit_Queue        (iQueue *);

void        put_Queue           (iQueue *, iQueueItem *item);

iQueueItem *take_Queue          (iQueue *);
iQueueItem *takeTimeout_Queue   (iQueue *, double timeoutSeconds);
iQueueItem *tryTake_Queue       (iQueue *);
void        waitForItems_Queue  (iQueue *);

size_t      size_Queue          (const iQueue *d);

iLocalDef iBool isEmpty_Queue(const iQueue *d) {
    return size_Queue(d) == 0;
}

iEndPublic
