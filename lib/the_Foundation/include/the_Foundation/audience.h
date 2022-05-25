#pragma once

/** @file the_Foundation/audience.h  Observer audience.

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

#include "mutex.h"
#include "sortedarray.h"
#include "ptrset.h"

iBeginPublic

#define iDeclareNotifyFunc(typeName, audienceName) \
    typedef void (*iNotify##typeName##audienceName)(iAny *, i##typeName *);

#define iDeclareNotifyFuncArgs(typeName, audienceName, ...) \
    typedef void (*iNotify##typeName##audienceName)(iAny *, i##typeName *, __VA_ARGS__);

#define iDeclareConstNotifyFunc(typeName, audienceName) \
    typedef void (*iNotify##typeName##audienceName)(iAny *, const i##typeName *);

#define iDeclareConstNotifyFuncArgs(typeName, audienceName, ...) \
    typedef void (*iNotify##typeName##audienceName)(iAny *, const i##typeName *, __VA_ARGS__);

#define iDeclareAudienceGetter(typeName, audienceName) \
    iAudience *audienceName##_##typeName(i##typeName *d);

#define iDefineAudienceGetter(typeName, audienceName) \
    iAudience *audienceName##_##typeName(i##typeName *d) { \
        if (!d->audienceName) { d->audienceName = new_Audience(); }  \
        return d->audienceName; \
    }

#define iDefineInlineAudienceGetter(typeName, audienceName) \
    iLocalDef iDefineAudienceGetter(typeName, audienceName)

#define iNotifyAudience(d, audienceName, notifyName) { \
    if ((d)->audienceName) { \
        iGuardMutex(&(d)->audienceName->mutex, \
            iConstForEach(Audience, i, (d)->audienceName) { \
                iFunctionCast(iNotify##notifyName, i.value->func)(i.value->object, d); \
            } \
        ); \
    } \
}

#define iNotifyAudienceArgs(d, audienceName, notifyName, ...) { \
    if ((d)->audienceName) { \
        iGuardMutex(&(d)->audienceName->mutex, \
            iConstForEach(Audience, i, (d)->audienceName) { \
                iFunctionCast(iNotify##notifyName, i.value->func)(i.value->object, d, __VA_ARGS__); \
            } \
        ); \
    } \
}

#define iConnect(typeName, src, audienceName, dest, function) \
    insert_Audience(audienceName##_##typeName(src), (dest), iFunctionCast(iObserverFunc, function))

#define iDisconnect(typeName, src, audienceName, dest, function) \
    remove_Audience(audienceName##_##typeName(src), (dest), iFunctionCast(iObserverFunc, function))

#define iDisconnectObject(typeName, src, audienceName, dest) \
    removeObject_Audience(audienceName##_##typeName(src), (dest))

iDeclareType(Audience)
iDeclareType(Observer)
iDeclareType(Object)

typedef void (*iObserverFunc)(iAnyObject *);

struct Impl_Observer {
    iAnyObject *object;
    iObserverFunc func;
};

struct Impl_Audience {
    iSortedArray observers;
    iMutex mutex;
};

iDeclareTypeConstruction(Audience)

void    init_Audience   (iAudience *);
void    deinit_Audience (iAudience *);

iBool   insert_Audience (iAudience *d, iAnyObject *object, iObserverFunc func);
iBool   remove_Audience (iAudience *d, iAnyObject *object, iObserverFunc func);

iLocalDef iBool removeObject_Audience(iAudience *d, iAnyObject *object) {
    return remove_Audience(d, object, NULL);
}

/** @name Iterators */
///@{
iDeclareConstIterator(Audience, const iAudience *)

struct ConstIteratorImpl_Audience {
    union {
        const iObserver *value;
        iArrayConstIterator iter;
    };
};
///@}

/*-------------------------------------------------------------------------------------*/

iDeclareType(AudienceMember)

struct Impl_AudienceMember {
    iPtrSet audiences;
    iObject *object;
};

iDeclareTypeConstructionArgs(AudienceMember, iAnyObject *object)

void    init_AudienceMember     (iAudienceMember *, iAnyObject *object);
void    deinit_AudienceMember   (iAudienceMember *);

void    insert_AudienceMember   (iAudienceMember *, iAudience *audience);
void    remove_AudienceMember   (iAudienceMember *, iAudience *audience);

iEndPublic
