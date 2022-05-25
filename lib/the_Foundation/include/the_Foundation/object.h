#pragma once

/** @file the_Foundation/object.h  Reference-counted object.

Object is reference-counted and gets deleted only after all references are gone.
Object is used as a base class for many of the objects in the_Foundation.

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

#include "defs.h"
#include "class.h"
#include "atomic.h"

iBeginPublic

#define iNew(typeName)      ((i##typeName *) new_Object(&Class_##typeName))

#define iDeclareObjectConstruction(typeName) \
    i##typeName *new_##typeName(void); \
    iLocalDef i##typeName *collect_##typeName(i##typeName *d) { \
        return iCollectDel(d, (iDeleteFunc) deref_Object); \
    } \
    void init_##typeName(i##typeName *d); \
    void deinit_##typeName(i##typeName *d); \
    iLocalDef const i##typeName##Class *class_##typeName(const i##typeName *d) { \
        return (const i##typeName##Class *) class_Object(d);\
    }

#define iDeclareObjectConstructionArgs(typeName, ...) \
    i##typeName *new_##typeName(__VA_ARGS__); \
    iLocalDef i##typeName *collect_##typeName(i##typeName *d) { \
        return iCollectDel(d, (iDeleteFunc) deref_Object); \
    } \
    void init_##typeName(i##typeName *d, __VA_ARGS__); \
    void deinit_##typeName(i##typeName *d); \
    iLocalDef const i##typeName##Class *class_##typeName(const i##typeName *d) { \
        return (const i##typeName##Class *) class_Object(d);\
    }

#define iDefineObjectConstruction(typeName) \
    i##typeName *new_##typeName(void) { \
        i##typeName *d = iNew(typeName); \
        init_##typeName(d); \
        return d; \
    } \

#define iDefineObjectConstructionArgs(typeName, newArgs, ...) \
    i##typeName *new_##typeName newArgs { \
        i##typeName *d = iNew(typeName); \
        init_##typeName(d, __VA_ARGS__); \
        return d; \
    }

#define iDeclareObjectSerialization(typeName) \
    void serialize_##typeName(const i##typeName *, iStream *); \
    void deserialize_##typeName(i##typeName *, iStream *);

iDeclareType(Object)
iDeclareType(AudienceMember)

struct Impl_Object {
    const iClass *classObj;
    iAtomicInt refCount;
    iAudienceMember *memberOf;
    void *user; /* custom user contextual data */
    uint32_t __sig; /* validity checks in debug builds */
};

typedef void iAnyObject;

/**
 * Constructs a new object.
 *
 * @param class  Object class. Determines how much memory is allocated for the object.
 */
iAnyObject *    new_Object      (const iAnyClass *);

void            deinit_Object   (iAnyObject *);

iAnyObject *    ref_Object      (const iAnyObject *);
void            deref_Object    (const iAnyObject *);
const iClass *  class_Object    (const iAnyObject *);

iLocalDef iBool isInstance_Object(const iAnyObject *d, const iAnyClass *pClass) {
    return class_Object(d) == pClass || isDerived_Class(class_Object(d), pClass);
}

void            setUserData_Object  (iAnyObject *, void *user);
void *          userData_Object     (const iAnyObject *);

iAudienceMember * audienceMember_Object (const iAnyObject *);

int             totalCount_Object       (void);
void            checkSignature_Object   (const iAnyObject *);

#if !defined (NDEBUG)
#define iAssertIsObject(d)  checkSignature_Object(d)
#else
#define iAssertIsObject(d)
#endif

iLocalDef iAnyObject *collect_Object(const iAnyObject *d) {
    if (d) {
        iAssertIsObject(d);
        return collect_Garbage(iConstCast(iAnyObject *, d), (iDeleteFunc) deref_Object);
    }
    return NULL;
}

iLocalDef void iRelease(const iAnyObject *d) {
    deref_Object(d);
}

#define iChangeRef(d, ptr)   { iRelease(d); (d) = ref_Object(ptr); }

#define iReleasePtr(d)       { iAssert((d) != NULL); if (d) { deref_Object(*(d)); *(d) = NULL; } } 

iLocalDef iAnyObject *iReleaseLater(const iAnyObject *d) {
    return collect_Object(d);
}

#define iClob(d)     iReleaseLater(d)   // clob == collect object

iEndPublic

#if defined (__cplusplus)
namespace tF {

template <typename T>
class ref {
    T *_ptr;
public:
    ref() : _ptr(nullptr) {}
    ref(const T *p) : _ptr(static_cast<T *>(ref_Object(p))) {}
    ref(const ref &other) : _ptr(static_cast<T *>(ref_Object(other._ptr))) {}
    ref(ref &&moved) : _ptr(moved._ptr) {
        moved._ptr = nullptr;
    }
    ~ref() { deref_Object(_ptr); }
    void reset(const T *p = nullptr) {
        if (_ptr != p) {
            deref_Object(_ptr);
            _ptr = static_cast<T *>(ref_Object(p));
        }
    }
    ref &operator=(const ref &other) {
        reset(other._ptr);
        return *this;
    }
    ref &operator=(ref &&moved) {
        reset();
        std::swap(_ptr, moved._ptr);
        return *this;
    }
    explicit operator bool() const { return _ptr != nullptr; }
    operator T *() const { return _ptr; }
    operator const T *() const { return _ptr; }
    T *operator->() const { return _ptr; }
    T &operator*() const { return *_ptr; }
    static ref take(T *p) {
        ref<T> r(p);
        deref_Object(p);
        return r;
    }
};

template <typename T> ref<T> make_ref(T *p) {
    ref<T> r(p);
    iRelease(p);
    return r;
}

} // namespace tF
#endif
