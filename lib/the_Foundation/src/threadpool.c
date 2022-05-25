/** @file the_Foundation/threadpool.h  Thread pool.

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

#include "the_Foundation/threadpool.h"

void finish_Thread_(iThread *); // thread.c

iDeclareClass(PooledThread)

struct Impl_PooledThread {
    iThread thread;
    iThreadPool *pool;
};

static iThreadResult run_PooledThread_(iThread *thread) {
    iPooledThread *d = (iAny *) thread;
    while (yield_ThreadPool(d->pool, 0.0)) { /* Keep going. */ }
    return 0;
}

static void init_PooledThread(iPooledThread *d, iThreadPool *pool) {
    init_Thread(&d->thread, run_PooledThread_);
    setName_Thread(&d->thread, "PooledThread");
    d->pool = pool;
}

static void deinit_PooledThread(iPooledThread *d) {
    iUnused(d);
}

iDefineSubclass(PooledThread, Thread)
iDefineObjectConstructionArgs(PooledThread, (iThreadPool *pool), pool)

iLocalDef void start_PooledThread(iPooledThread *d) { start_Thread(&d->thread); }
iLocalDef void join_PooledThread (iPooledThread *d) { join_Thread(&d->thread); }

/*-------------------------------------------------------------------------------------*/

iDefineClass(ThreadPool)
iDefineObjectConstruction(ThreadPool)

static void startThreads_ThreadPool_(iThreadPool *d, int minThreads, int reservedCores) {
    const int count = iMaxi(iMaxi(1, minThreads), idealConcurrentCount_Thread() - reservedCores);
    for (int i = 0; i < count; ++i) {
        iPooledThread *pt = new_PooledThread(d);
        pushBack_ObjectList(d->threads, pt);
        start_PooledThread(pt);
        iRelease(pt);
    }
}

static void stopThreads_ThreadPool_(iThreadPool *d) {
    for (size_t count = size_ObjectList(d->threads); count; count--) {
        put_Queue(&d->queue, d);
    }
    iForEach(ObjectList, i, d->threads) {
        join_PooledThread((iPooledThread *) i.value->object);
        remove_ObjectListIterator(&i);
    }
}

iThreadPool *newLimits_ThreadPool(int minThreads, int reservedCores) {
    iThreadPool *d = iNew(ThreadPool);
    initLimits_ThreadPool(d, minThreads, reservedCores);
    return d;
}

void init_ThreadPool(iThreadPool *d) {
    initLimits_ThreadPool(d, 0, 0);
}

void initLimits_ThreadPool(iThreadPool *d, int minThreads, int reservedCores) {
    init_Queue(&d->queue);
    d->threads = new_ObjectList();
    startThreads_ThreadPool_(d, minThreads, reservedCores);
}

void deinit_ThreadPool(iThreadPool *d) {
    stopThreads_ThreadPool_(d);
    iRelease(d->threads);
    deinit_Queue(&d->queue);
}

iThread *run_ThreadPool(iThreadPool *d, iThread *thread) {
    if (thread) {
        put_Queue(&d->queue, thread);
    }
    return thread;
}

iBool yield_ThreadPool(iThreadPool *d, double timeoutSeconds) {
    iThread *job = NULL;
    if (timeoutSeconds <= 0.0) {
        job = (iAny *) take_Queue(&d->queue);
    }
    else {
        job = (iAny *) takeTimeout_Queue(&d->queue, timeoutSeconds);
    }
    if (job == NULL || job == (void *) d) {
        /* Terminated. */
        return iFalse;
    }
    /* Run in the calling thread. */
    iAssert(job->state == created_ThreadState);
    iGuardMutex(&job->mutex, job->state = running_ThreadState);
    job->result = job->run(job);
    finish_Thread_(job);
    iRelease(job);
    return iTrue;
}
