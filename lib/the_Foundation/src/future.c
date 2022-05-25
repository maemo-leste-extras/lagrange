/** @file future.c  Future value.

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

#include "the_Foundation/future.h"
#include "the_Foundation/threadpool.h"

iDefineClass(Future)
iDefineObjectConstruction(Future)

static void threadFinished_Future_(iAny *any, iThread *thread) {
    iFuture *d = any;
    if (d->resultAvailable) {
        d->resultAvailable(d, thread);
    }
    iGuardMutex(&d->mutex, {
        addRelaxed_Atomic(&d->pendingCount, -1);
        iAssert(value_Atomic(&d->pendingCount) >= 0);
        signalAll_Condition(&d->ready);
    });
}

void init_Future(iFuture *d) {
    initHandler_Future(d, NULL);
}

void initHandler_Future(iFuture *d, iFutureResultAvailable resultAvailable) {
    init_Mutex(&d->mutex);
    init_Condition(&d->ready);
    d->threads = new_ObjectList();
    set_Atomic(&d->pendingCount, 0);
    d->resultAvailable = resultAvailable;
}

void deinit_Future(iFuture *d) {
    wait_Future(d);
    /* Stop observing the remaining threads. */
    iForEach(ObjectList, i, d->threads) {
        iDisconnect(Thread, i.object, finished, d, threadFinished_Future_);
    }
    iRelease(d->threads);
    deinit_Condition(&d->ready);
    deinit_Mutex(&d->mutex);
}

void add_Future(iFuture *d, iThread *thread) {
    iAssert(!isRunning_Thread(thread));
    iGuardMutex(&d->mutex, {
        addRelaxed_Atomic(&d->pendingCount, 1);
        iConnect(Thread, thread, finished, d, threadFinished_Future_);
        pushBack_ObjectList(d->threads, thread);
    });
}

iThread *runPool_Future(iFuture *d, iThread *thread, iThreadPool *pool) {
    add_Future(d, thread);
    run_ThreadPool(pool, thread);
    return thread;
}

iBool isReady_Future(const iFuture *d) {
    iBool ready = iFalse;
    iGuardMutex(&d->mutex, ready = (value_Atomic(&iConstCast(iFuture *, d)->pendingCount) == 0));
    return ready;
}

void wait_Future(iFuture *d) {
    iGuardMutex(&d->mutex, {
        while (value_Atomic(&d->pendingCount) > 0) {
            wait_Condition(&d->ready, &d->mutex);
        }
    });
}

iBool isEmpty_Future(const iFuture *d) {
    iBool empty;
    iGuardMutex(&d->mutex, empty = isEmpty_ObjectList(d->threads));
    return empty;
}

iThread *nextResult_Future(iFuture *d) {
    iThread *result = NULL;
    iGuardMutex(&d->mutex, {
        while (!isEmpty_ObjectList(d->threads)) {
            /* Check for a finished thread. */
            iForEach(ObjectList, i, d->threads) {
                iThread *thread = i.object;
                if (isFinished_Thread(thread)) {
                    result = ref_Object(thread);
                    remove_ObjectListIterator(&i);
                    unlock_Mutex(&d->mutex);
                    iDisconnect(Thread, thread, finished, d, threadFinished_Future_);
                    return result;
                }
            }
            if (result) break;
            wait_Condition(&d->ready, &d->mutex);
        }
    });
    return result;
}
