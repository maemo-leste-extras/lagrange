/** @file mutex.c  Mutual exclusion.

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

#include "the_Foundation/mutex.h"

static void init_Mutex_(iMutex *d, enum iMutexType type) {
    mtx_init(&d->mtx, type == recursive_MutexType? mtx_recursive : mtx_plain);
}

iDefineTypeConstruction(Mutex)

iMutex *newType_Mutex(enum iMutexType type) {
    iMutex *d = iMalloc(Mutex);
    init_Mutex_(d, type);
    return d;
}

void init_Mutex(iMutex *d) {
    init_Mutex_(d, recursive_MutexType);
}

void deinit_Mutex(iMutex *d) {
    mtx_destroy(&d->mtx);
}

iBool lock_Mutex(iMutex *d) {
    return mtx_lock(&d->mtx) == thrd_success;
}

iBool tryLock_Mutex(iMutex *d) {
    return mtx_trylock(&d->mtx) == thrd_success;
}

void unlock_Mutex(iMutex *d) {
    mtx_unlock(&d->mtx);
}

/*-------------------------------------------------------------------------------------*/

iDefineTypeConstruction(Condition)

void init_Condition(iCondition *d) {
    cnd_init(&d->cnd);
}

void deinit_Condition(iCondition *d) {
    cnd_destroy(&d->cnd);
}
