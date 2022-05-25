#pragma once

/** @file the_Foundation/future.h  Future value.

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

#include "thread.h"
#include "objectlist.h"

iBeginPublic

iDeclareClass(Future)

iDeclareType(ThreadPool)

typedef void (*iFutureResultAvailable)(iFuture *, iThread *);

struct Impl_Future {
    iObject object;
    iObjectList *threads;
    iMutex mutex;
    iCondition ready;
    iAtomicInt pendingCount;
    iFutureResultAvailable resultAvailable;
};

iDeclareObjectConstruction(Future)

void        init_Future         (iFuture *);
void        initHandler_Future  (iFuture *, iFutureResultAvailable resultAvailable);

void        deinit_Future   (iFuture *);

/**
 * Adds a thread to the set of pending results. One thread may be a result is multiple
 * Futures.
 *
 * @param thread  Thread to add. Must not be started yet.
 */
void        add_Future      (iFuture *, iThread *thread);

/**
 * Adds a thread to the future and starts the thread in a thread pool.
 *
 * @param thread    Thread to run.
 * @param pool      Thread pool where to run the thread.
 *
 * @return The added thread.
 */
iThread *   runPool_Future(iFuture *, iThread *thread, iThreadPool *pool);

iBool       isReady_Future  (const iFuture *);
void        wait_Future     (iFuture *);

/**
 * Returns the next complete result. If nothing is ready, waits until a result is
 * available. Caller gets a reference to the returned thread.
 */
iThread *   nextResult_Future (iFuture *);

iBool       isEmpty_Future  (const iFuture *d);

iLocalDef const iObjectList *threads_Future(const iFuture *d) {
    return d->threads;
}

iEndPublic
