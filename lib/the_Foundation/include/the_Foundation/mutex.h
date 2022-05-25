#pragma once

/** @file the_Foundation/mutex.h  Mutual exclusion.

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

#include "time.h"
#include "stdthreads.h"

iBeginPublic

/**
 * Defines a lockable object type. In practice, this is an object paired with a mutex.
 * Both the objcet and the mutex are created when the lockable object is initialized.
 * @param typeName  Type of the value object.
 */
#define iDeclareLockableObject(typeName) \
    iDeclareType(Lockable##typeName) \
    struct Impl_Lockable##typeName { \
        iMutex *mutex; \
        i##typeName *value; \
    }; \
    iDeclareTypeConstruction(Lockable##typeName)

#define iDefineLockableObject(typeName) \
    iDefineTypeConstruction(Lockable##typeName) \
    void init_Lockable##typeName(iLockable##typeName *d) { \
        d->mutex = new_Mutex(); \
        d->value = new_##typeName(); \
    } \
    void deinit_Lockable##typeName(iLockable##typeName *d) { \
        delete_Mutex(d->mutex); \
        deref_Object(d->value); \
    }

#define iLock(d)                lock_Mutex((d)->mutex)
#define iUnlock(d)              unlock_Mutex((d)->mutex)
#define iGuard(d, stmt)         {iLock(d); stmt; iUnlock(d);}

#define iGuardMutex(d, stmt)    {lock_Mutex  (iConstCast(iMutex *, d)); stmt; \
                                 unlock_Mutex(iConstCast(iMutex *, d));}
iDeclareType(Mutex)

struct Impl_Mutex {
    mtx_t mtx;
};

enum iMutexType {
    nonRecursive_MutexType  = 0,
    recursive_MutexType     = 1,
};

iDeclareTypeConstruction(Mutex)

iMutex *    newType_Mutex(enum iMutexType type);

iBool       lock_Mutex      (iMutex *);
iBool       tryLock_Mutex   (iMutex *);
void        unlock_Mutex    (iMutex *);

/*-------------------------------------------------------------------------------------*/

iDeclareType(Condition)

struct Impl_Condition {
    cnd_t cnd;
};

iDeclareTypeConstruction(Condition)

iLocalDef void signal_Condition(iCondition *d) {
    cnd_signal(&d->cnd);
}

iLocalDef void signalAll_Condition(iCondition *d) {
    cnd_broadcast(&d->cnd);
}

iLocalDef void wait_Condition(iCondition *d, iMutex *mutex) {
    cnd_wait(&d->cnd, &mutex->mtx);
}

iLocalDef int waitTimeout_Condition(iCondition *d, iMutex *mutex, const iTime *timeout) {
    return cnd_timedwait(&d->cnd, &mutex->mtx, &timeout->ts);
}

iEndPublic
