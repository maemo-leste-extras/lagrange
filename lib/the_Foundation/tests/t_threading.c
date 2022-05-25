/**
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

#include <the_Foundation/future.h>
#include <the_Foundation/threadpool.h>
#include <the_Foundation/math.h>

static atomic_int thrCounter;

static iThreadResult run_Worker_(iThread *d) {
    iBeginCollect();
    int value = 0;
    for (int i = 0; i < 10000; ++i) {
        value = iRandom(0, 1000000);
    }
    printf("%6i : Job %p on thread %p: value %i\n", ++thrCounter, d, current_Thread(), value);
    iEndCollect();
    return value;
}

int main(int argc, char *argv[]) {
    iUnused(argc, argv);
    init_Foundation();
    /* Run a few threads in a pool. */ {
        iThreadPool *pool = new_ThreadPool();
        iFuture *future = new_Future();
        for (int i = 0; i < 100; ++i) {
            iRelease(runPool_Future(future, new_Thread(run_Worker_), pool));
        }
        puts("Waiting for threads to finish...");
#if 0
        wait_Future(future);
#else
        while (!isEmpty_Future(future)) {
            iThread *result = nextResult_Future(future);
            iAssert(isFinished_Thread(result));
            printf("Result from future %p: %li\n", result, result_Thread(result));
            iRelease(result);
        }
#endif
        iRelease(future);
        iRelease(pool);
    }
    return 0;
}
