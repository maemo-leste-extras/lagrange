/** @file audience.c  Observer audience.

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

#include "the_Foundation/audience.h"
#include "the_Foundation/object.h"

static int cmpObject_Observer_(const void *a, const void *b) {
    const iObserver *x = a, *y = b;
    return iCmp(x->object, y->object);
}

static int cmp_Observer_(const void *a, const void *b) {
    const int cmp = cmpObject_Observer_(a, b);
    if (cmp != 0) return cmp;
    /* Same object. */
    const iObserver *x = a, *y = b;
    return iCmp((intptr_t) x->func, (intptr_t) y->func);
}

iDefineTypeConstruction(Audience)

void init_Audience(iAudience *d) {
    init_SortedArray(&d->observers, sizeof(iObserver), cmp_Observer_);
    init_Mutex(&d->mutex);
}

void deinit_Audience(iAudience *d) {
    /* Tells members of this audience that the audience is going away. */
    iGuardMutex(&d->mutex, {
        iConstForEach(Audience, i, d) {
            iAudienceMember *memberOf = ((const iObject *) i.value->object)->memberOf;
            iAssert(memberOf != NULL);
            remove_PtrSet(&memberOf->audiences, d);
        }
        deinit_SortedArray(&d->observers);
    });
    deinit_Mutex(&d->mutex);
}

iBool insert_Audience(iAudience *d, iAnyObject *object, iObserverFunc func) {
    /* This object becomes an audience member. */
    iAssert(object != NULL);
    iBool inserted;
    iGuardMutex(&d->mutex, {
        insert_AudienceMember(audienceMember_Object(object), d);
        inserted = insert_SortedArray(&d->observers, &(iObserver){ object, func });
    });
    return inserted;
}

static iBool removeObject_Audience_(iAudience *d, const iAnyObject *object) {
    iBool removed;
    iGuardMutex(&d->mutex, {
        const iRanges range = locateRange_SortedArray(
            &d->observers, &(iObserver){ iConstCast(void *, object), NULL }, cmpObject_Observer_);
        removeRange_SortedArray(&d->observers, range);
        removed = !isEmpty_Range(&range);
    });
    return removed;
}

iBool remove_Audience(iAudience *d, iAnyObject *object, iObserverFunc func) {
    /* This object is no longer an audience member. */
    iBool removed;
    iGuardMutex(&d->mutex, {
        remove_AudienceMember(audienceMember_Object(object), d);
        if (func) {
            removed = remove_SortedArray(&d->observers, &(iObserver){ object, func });
        }
        else {
            removed = removeObject_Audience_(d, object);
        }
    });
    return removed;
}

/*-------------------------------------------------------------------------------------*/

void init_AudienceConstIterator(iAudienceConstIterator *d, const iAudience *audience) {
    if (audience) {
        init_ArrayConstIterator(&d->iter, &audience->observers.values);
    }
    else {
        iZap(d->iter);
    }
}

void next_AudienceConstIterator(iAudienceConstIterator *d) {
    next_ArrayConstIterator(&d->iter);
}

/*-------------------------------------------------------------------------------------*/

iDefineTypeConstructionArgs(AudienceMember, (iAnyObject *object), object)

void init_AudienceMember(iAudienceMember *d, iAnyObject *object) {
    init_PtrSet(&d->audiences);
    d->object = object;
}

void deinit_AudienceMember(iAudienceMember *d) {
    iForEach(PtrSet, i, &d->audiences) {
        removeObject_Audience_(*(iAudience **) i.value, d->object);
    }
    deinit_PtrSet(&d->audiences);
}

void insert_AudienceMember(iAudienceMember *d, iAudience *audience) {
    insert_PtrSet(&d->audiences, audience);
}

void remove_AudienceMember(iAudienceMember *d, iAudience *audience) {
    remove_PtrSet(&d->audiences, audience);
}
