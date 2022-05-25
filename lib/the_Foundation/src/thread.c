/** @file thread.c  Thread object.

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

#include "the_Foundation/thread.h"
#include "the_Foundation/blockhash.h"
#include "the_Foundation/mutex.h"

/* garbage.c */
void deinitForThread_Garbage_(void);

iDefinePlainKeyBlockHash(ThreadHash, ThreadId, Thread)

/*-------------------------------------------------------------------------------------*/

iDefineLockableObject(ThreadHash)

static iLockableThreadHash *runningThreads_;

void deinit_Threads_(void) {
    delete_LockableThreadHash(runningThreads_);
    runningThreads_ = NULL;
}

static iLockableThreadHash *init_Threads_(void) {
    if (!runningThreads_) {
        runningThreads_ = new_LockableThreadHash();
    }
    return runningThreads_;
}

void init_Threads(void) {
    init_Threads_();
}

void finish_Thread_(iThread *d) { /* called from threadpool.c as well */
    iGuardMutex(&d->mutex, {
        d->state = finished_ThreadState;
        signalAll_Condition(&d->finishedCond);
    });
    iNotifyAudience(d, finished, ThreadFinished);
    iRecycle();
}

static int run_Threads_(void *arg) {
    iThread *d = (iThread *) arg;
    ref_Object(d);
    if (!isEmpty_String(&d->name)) {
#if defined (iPlatformApple)
        pthread_setname_np(cstr_String(&d->name));
#endif
#if defined (iPlatformLinux)
        pthread_setname_np(d->id, cstr_String(&d->name));
#endif
    }
    if (d->flags & terminationEnabled_ThreadFlag) {
#if defined (iHavePThreadCancel)
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
#endif
    }
    d->result = d->run(d);
    /* Deregister the thread since it's stopping. */
    iGuard(runningThreads_, remove_ThreadHash(runningThreads_->value, &d->id));
    /* Notify observers that the thread is done. */
    finish_Thread_(d);
    deref_Object(d);
    deinitForThread_Garbage_();
    thrd_exit(0); // thread-local data gets deleted
    return 0;
}

/*-------------------------------------------------------------------------------------*/

iDefineClass(Thread)
iDefineObjectConstructionArgs(Thread, (iThreadRunFunc run), run)

void init_Thread(iThread *d, iThreadRunFunc run) {
    init_Mutex(&d->mutex);
    init_Condition(&d->finishedCond);
    init_String(&d->name);
    d->result = 0;
    d->run = run;
    d->id = 0;
    d->flags = 0;
    d->userData = NULL;
    d->state = created_ThreadState;
    d->finished = NULL;
}

void deinit_Thread(iThread *d) {
    iAssert(d->state != running_ThreadState);
    if (d->state == running_ThreadState) {
        iWarning("[Thread] thread %p is being destroyed while still running\n", d);
    }
    delete_Audience(d->finished);
    deinit_Condition(&d->finishedCond);
    deinit_Mutex(&d->mutex);
    deinit_String(&d->name);
}

void start_Thread(iThread *d) {
    iLockableThreadHash *threads = init_Threads_();
    iGuardMutex(&d->mutex, {
        iAssert(d->state == created_ThreadState);
        d->state = running_ThreadState;
        thrd_create(&d->id, run_Threads_, d);
        iDebug("[Thread] created thread ID %p (%s)\n", d->id, cstr_String(&d->name));
    });
    /* Register this thread as a running thread. */
    iGuard(threads, insert_ThreadHash(threads->value, &d->id, d));
}

void setName_Thread(iThread *d, const char *name) {
    iAssert(d->state == created_ThreadState);
    setCStr_String(&d->name, name);
}

void setUserData_Thread(iThread *d, void *userData) {
    iGuardMutex(&d->mutex, d->userData = userData);
}

void setTerminationEnabled_Thread(iThread *d, iBool enable) {
    iChangeFlags(d->flags, terminationEnabled_ThreadFlag, enable);
}

iBool isRunning_Thread(const iThread *d) {
    iBool ret;
    iGuardMutex(&d->mutex, ret = (d->state == running_ThreadState));
    return ret;
}

iBool isFinished_Thread(const iThread *d) {
    iBool ret;
    iGuardMutex(&d->mutex, ret = (d->state == finished_ThreadState));
    return ret;
}

const iString *name_Thread(const iThread *d) {
    return &d->name;
}

void *userData_Thread(const iThread *d) {
    return d->userData;
}

iThreadResult result_Thread(const iThread *d) {
    join_Thread(iConstCast(iThread *, d));
    iAssert(d->state == finished_ThreadState);
    return d->result;
}

void join_Thread(iThread *d) {
    if (!d) return;
    iAssert(d->id != thrd_current());
    if (d->id == thrd_current()) return;
    lock_Mutex(&d->mutex);
    if (d->state == running_ThreadState) {
        wait_Condition(&d->finishedCond, &d->mutex);
    }
    unlock_Mutex(&d->mutex);
    thrd_join(d->id, NULL);
}

void terminate_Thread(iThread *d) {
    iAssert(d->flags & terminationEnabled_ThreadFlag);
#if defined (iHavePThreadCancel)
    pthread_cancel(d->id);
#endif
}

void sleep_Thread(double seconds) {
    iTime dur;
    initSeconds_Time(&dur, seconds);
    thrd_sleep(&dur.ts, NULL);
}

iThread *current_Thread(void) {
    iThread *d = NULL;
    const iThreadId cur = thrd_current();
    const iLockableThreadHash *threads = init_Threads_();
    iGuard(threads, d = value_ThreadHash(threads->value, &cur));
    return d;
}

iBool isCurrent_Thread(const iThread *d) {
    iAssert(d);
    return d->id == thrd_current();
}
