#pragma once

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

#include "thread.h"
#include "queue.h"

iBeginPublic

iDeclareClass(ThreadPool)

struct Impl_ThreadPool {
    iQueue queue;
    iObjectList *threads;
};

iDeclareObjectConstruction(ThreadPool)

/**
 * Constructs a thread pool with a limited number of threads.
 *
 * @param minThreads     Minimum number of threads to create.
 * @param reservedCores  Number of logical CPU cores to be considered reserved for other uses
 *                       and therefore not included in the pool, making the pool smaller.
 */
iThreadPool *   newLimits_ThreadPool    (int minThreads, int reservedCores);

void        init_ThreadPool         (iThreadPool *);
void        initLimits_ThreadPool   (iThreadPool *, int minThreads, int reservedCores);
void        deinit_ThreadPool       (iThreadPool *);

iThread *   run_ThreadPool          (iThreadPool *, iThread *thread);

/**
 * Use the calling thread to run another queud thread. Returns immediately after a queued thread
 * has finished executing. Use this to sleep in pooled threads; regular sleeping in a pooled
 * thread would potentially block the pool from doing any work, if all the workers are sleeping.
 *
 * @param timeoutSeconds  Maximum duration to wait until queued threads become available.
 *
 * @return iTrue, if a queued thread was executed. iFalse, if timed out without running
 * anything.
 */
iBool       yield_ThreadPool        (iThreadPool *, double timeoutSeconds);

iEndPublic
