/** @file posix/address.c  Network address.

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

#include "the_Foundation/address.h"
#include "the_Foundation/mutex.h"
#include "the_Foundation/string.h"
#include "the_Foundation/objectlist.h"
#include "the_Foundation/queue.h"
#include "the_Foundation/thread.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ifaddrs.h>

enum iAddressFlag {
    finished_AddressFlag = 0x1,
};

struct Impl_Address {
    iObject object;
    iMutex *mutex;
    iString hostName;
    iString service;
    int socktype;
    int flags;
    int count;
    struct addrinfo *info;
    iAudience *lookupFinished;
    iCondition *lookupDidFinish;
};

iDefineAudienceGetter(Address, lookupFinished)

#if defined (iPlatformHaiku) || defined (iPlatformAndroid) || defined (iPlatformOther)
#   if defined (AI_V4MAPPED_CFG)
#       undef AI_V4MAPPED_CFG
#   endif
#   define AI_V4MAPPED_CFG  0
#elif defined (iPlatformLinux) || defined (iPlatformMsys) || defined (iPlatformCygwin)
#   define AI_V4MAPPED_CFG  AI_V4MAPPED
#endif

static iThread *lookupThread_;
static iQueue * lookupQueue_;

static iThreadResult runAddressLookup_(iThread *thd) {
    iUnused(thd);
    iDebug("[Address] lookup thread started\n");
    while (lookupThread_) {
        waitForItems_Queue(lookupQueue_);
        iAddress *d = tryTake_Queue(lookupQueue_);
        if (!lookupThread_ || !d) {
            break;
        }
        /* Perform the lookup. */
        /* TODO: hostName/service accessed without locking... */
        const int hintFlags = AI_V4MAPPED_CFG | AI_ADDRCONFIG | (isEmpty_String(&d->hostName) ? AI_PASSIVE : 0);
        const struct addrinfo hints = {
            .ai_socktype = d->socktype,
            .ai_family   = (d->socktype == SOCK_DGRAM ? AF_INET     : AF_UNSPEC /* v4 or v6 */),
            .ai_protocol = (d->socktype == SOCK_DGRAM ? IPPROTO_UDP : IPPROTO_TCP),
            .ai_flags    = hintFlags,
        };
        int rc = getaddrinfo(!isEmpty_String(&d->hostName) ? cstr_String(&d->hostName) : NULL,
                             !isEmpty_String(&d->service)  ? cstr_String(&d->service)  : NULL,
                             &hints,
                             &d->info);
        iGuardMutex(d->mutex,
            d->count = 0;
            if (rc == 0) {
                for (const struct addrinfo *at = d->info; at; at = at->ai_next, d->count++) {}
            }
            else {
                iWarning("[Address] host lookup failed with error: %s\n", gai_strerror(rc));
            }
            d->flags |= finished_AddressFlag;
        );
        iNotifyAudience(d, lookupFinished, AddressLookupFinished);
        signalAll_Condition(d->lookupDidFinish);
        iRelease(d); /* ref was added by Queue */
    }
    iDebug("[Address] lookup thread exited\n");
    return 0;
}

static void startLookupThread_Address_(void) {
    /* Address lookup is done asynchronously because it may involve blocking for unknown
       periods of time. */
    if (!lookupThread_) {
        lookupQueue_  = new_Queue();
        lookupThread_ = new_Thread(runAddressLookup_);
        setName_Thread(lookupThread_, "runAddressLookup_");
        start_Thread(lookupThread_);
    }
}

void deinit_Address_(void) {
    if (lookupThread_) {
        iThread *thd = lookupThread_;
        lookupThread_ = NULL;
        signal_Condition(&lookupQueue_->cond);
        join_Thread(thd);
        iRelease(thd);
        iReleasePtr(&lookupQueue_);
    }
}

iLocalDef socklen_t sockAddrSize_addrinfo_(const struct addrinfo *d) {
    if (d->ai_family == AF_INET) {
        return sizeof(struct sockaddr_in);
    }
    return sizeof(struct sockaddr_in6);
}

iDefineObjectConstruction(Address)

iAddress *newBroadcast_Address(uint16_t port) {
    iAddress *d = new_Address();
    lookupCStr_Address(d, "255.255.255.255", port, udp_SocketType);
    return d;
}

iAddress *newSockAddr_Address(const void *     sockAddr,
                              size_t           sockAddrSize,
                              enum iSocketType socketType) {
    iAddress *d = iNew(Address);
    init_Address(d);
    d->socktype = (socketType == udp_SocketType ? SOCK_DGRAM : SOCK_STREAM);
    d->count = 1;
    d->info = calloc(1, sizeof(struct addrinfo));
    d->info->ai_addrlen = (socklen_t) sockAddrSize;
    d->info->ai_addr = malloc(sockAddrSize);
    d->info->ai_socktype = d->socktype;
    d->info->ai_family = (sockAddrSize == sizeof(struct sockaddr_in6) ? AF_INET6 : AF_INET);
    memcpy(d->info->ai_addr, sockAddr, sockAddrSize);
    return d;
}

void init_Address(iAddress *d) {
    d->mutex = new_Mutex();
    init_String(&d->hostName);
    init_String(&d->service);
    d->socktype = SOCK_STREAM;
    d->info = NULL;
    d->count = -1;
    d->flags = finished_AddressFlag;
    d->lookupFinished = NULL;
    d->lookupDidFinish = new_Condition();
}

void deinit_Address(iAddress *d) {
    /* Note: This is never called when lookup is pending because `lookupQueue_` holds a ref. */
    lock_Mutex(d->mutex);
    delete_Condition(d->lookupDidFinish);
    if (d->info) freeaddrinfo(d->info);
    deinit_String(&d->service);
    deinit_String(&d->hostName);
    unlock_Mutex(d->mutex);
    delete_Audience(d->lookupFinished);
    delete_Mutex(d->mutex);
}

const iString *hostName_Address(const iAddress *d) {
    if (isEmpty_String(&d->hostName)) {
        char hbuf[NI_MAXHOST];
        if (!getnameinfo(d->info->ai_addr,
                         sockAddrSize_addrinfo_(d->info),
                         hbuf, sizeof(hbuf),
                         NULL, 0,
                         NI_NUMERICHOST)) {
            setCStr_String(&iConstCast(iAddress *, d)->hostName, hbuf);
        }
    }
    return &d->hostName;
}

uint16_t port_Address(const iAddress *d) {
    if (!d->info) return 0;
    char sbuf[NI_MAXSERV];
    if (!getnameinfo(d->info->ai_addr,
                     sockAddrSize_addrinfo_(d->info),
                     NULL, 0,
                     sbuf, sizeof(sbuf),
                     NI_NUMERICSERV)) {
        return (uint16_t) atoi(sbuf);
    }
    return 0;
}

int count_Address(const iAddress *d) {
    int count;
    iGuardMutex(d->mutex, count = d->count);
    return count;
}

iSocketParameters socketParametersIndex_Address(const iAddress *d, int index) {
    iSocketParameters sp = { .family = 0 };
    iGuardMutex(d->mutex, {
        for (const struct addrinfo *i = d->info; i; i = i->ai_next, index--) {
            if (index == 0) {
                sp.family   = i->ai_family;
                sp.type     = i->ai_socktype;
                sp.protocol = i->ai_protocol;
                break;
            }
        }
    });
    return sp;
}

iSocketParameters socketParametersFamily_Address(const iAddress *d, int family) {
    iSocketParameters sp = { .family = 0 };
    iGuardMutex(d->mutex, {
        for (const struct addrinfo *i = d->info; i; i = i->ai_next) {
            if (family == AF_UNSPEC || i->ai_family == family) {
                sp.family   = i->ai_family;
                sp.type     = i->ai_socktype;
                sp.protocol = i->ai_protocol;
                break;
            }
        }
    });
    return sp;
}

iBool isValid_Address(const iAddress *d) {
    return count_Address(d) >= 0;
}

iBool isHostFound_Address(const iAddress *d) {
    return count_Address(d) > 0;
}

iBool isPending_Address(const iAddress *d) {
    iBool isFinished;
    iGuardMutex(d->mutex, isFinished = (d->flags & finished_AddressFlag) != 0);
    return !isFinished;
}

iBool equal_Address(const iAddress *d, const iAddress *other) {
    waitForFinished_Address(d);
    waitForFinished_Address(other);
    /* Compare the addresses with each other. */
    for (const struct addrinfo *i = d->info; i; i = i->ai_next) {
        for (const struct addrinfo *j = other->info; j; j = j->ai_next) {
            if (i->ai_family == j->ai_family && i->ai_protocol == j->ai_protocol &&
                i->ai_addrlen == j->ai_addrlen && !memcmp(i->ai_addr, j->ai_addr, i->ai_addrlen)) {
                return iTrue;
            }
        }
    }
    return iFalse;
}

void lookupCStr_Address(iAddress *d, const char *hostName, uint16_t port, enum iSocketType socketType) {
    waitForFinished_Address(d);
    if (d->info) {
        freeaddrinfo(d->info);
        d->info = NULL;
    }
    d->flags &= ~finished_AddressFlag;
    d->count = -1;
    d->socktype = (socketType == udp_SocketType ? SOCK_DGRAM : SOCK_STREAM);
    if (hostName) {
        setCStr_String(&d->hostName, hostName);
    }
    else {
        clear_String(&d->hostName);
    }
    if (port) {
        format_String(&d->service, "%i", port);
    }
    else {
        clear_String(&d->service);
    }
    startLookupThread_Address_();
    put_Queue(lookupQueue_, d);
}

void waitForFinished_Address(const iAddress *d) {
    lock_Mutex(d->mutex);
    if (~d->flags & finished_AddressFlag) {
        wait_Condition(d->lookupDidFinish, d->mutex);
    }
    unlock_Mutex(d->mutex);
}

/* internal use only */
int getSockAddr_Address(const iAddress *d, struct sockaddr **addr_out, socklen_t *addrSize_out,
                        int family, int indexInFamily) {
    const iBool byIndex = indexInFamily >= 0;
    waitForFinished_Address(d);
    *addr_out = NULL;
    *addrSize_out = 0;
    int resultIndex = 0;
    iGuardMutex(d->mutex, {
        for (const struct addrinfo *i = d->info; i; i = i->ai_next, resultIndex++) {
            if (family == AF_UNSPEC || i->ai_family == family) {
                if (!byIndex || indexInFamily-- == 0) {
                    *addr_out = i->ai_addr;
                    *addrSize_out = i->ai_addrlen;
                    break;
                }
            }
        }
    });
    return resultIndex;
}

/* internal use only */
iString *toString_SockAddr(const struct sockaddr *addr) {
    char hbuf[NI_MAXHOST];
    char sbuf[NI_MAXSERV];
    iString *str = new_String();
    if (!getnameinfo(addr,
                     addr->sa_family == AF_INET6 ? sizeof(struct sockaddr_in6)
                                                 : sizeof(struct sockaddr),
                     hbuf,
                     sizeof(hbuf),
                     sbuf,
                     sizeof(sbuf),
                     NI_NUMERICHOST | NI_NUMERICSERV)) {
        if (iCmpStr(sbuf, "0")) {
            format_String(str, addr->sa_family == AF_INET6 ? "[%s]:%s" : "%s:%s", hbuf, sbuf);
        }
        else {
            setCStr_String(str, hbuf);
        }
    }
    return str;
}

iString *toString_Address(const iAddress *d) {
    return toStringFlags_Address(d, 0, AF_UNSPEC);
}

iString *toStringFlags_Address(const iAddress *d, int flags, int family) {
    waitForFinished_Address(d);
    iString *str = new_String();
    if (!d) return str;
    iGuardMutex(d->mutex, {
        for (const struct addrinfo *i = d->info; i; i = i->ai_next) {
            if (family == AF_UNSPEC || i->ai_family == family) {
                char hbuf[NI_MAXHOST];
                char sbuf[NI_MAXSERV];
                if (!getnameinfo(i->ai_addr,
                                 sockAddrSize_addrinfo_(i),
                                 hbuf, sizeof(hbuf),
                                 sbuf, sizeof(sbuf),
                                 NI_NUMERICHOST | NI_NUMERICSERV)) {
                    if (iCmpStr(sbuf, "0") && ~flags & noPort_SocketStringFlag) {
                        format_String(str, i->ai_family == AF_INET6? "[%s]:%s" : "%s:%s", hbuf, sbuf);
                    }
                    else {
                        setCStr_String(str, hbuf);
                    }
                }
                break;
            }
        }
    });
    return str;
}

iObjectList *networkInterfaces_Address(void) {
    iObjectList *list = new_ObjectList();
#if !defined (iPlatformAndroid) /* not supported; see https://github.com/oliviertilmans/android-ifaddrs */
    struct ifaddrs *addrs = NULL;
    if (!getifaddrs(&addrs)) {
        for (struct ifaddrs *i = addrs; i; i = i->ifa_next) {
            char hbuf[NI_MAXHOST];
            struct sockaddr *sockAddr = i->ifa_addr;
            /* Only IPv4 and IPv6 addresses. */
            if (sockAddr->sa_family != AF_INET && sockAddr->sa_family != AF_INET6) continue;
            const socklen_t size = sockAddr->sa_family == AF_INET6 ? sizeof(struct sockaddr_in6)
                                                                   : sizeof(struct sockaddr_in);
            if (!getnameinfo(sockAddr, size, hbuf, sizeof(hbuf), NULL, 0, NI_NUMERICHOST)) {
                if (strlen(hbuf)) {
                    iAddress *addr = newSockAddr_Address(sockAddr, size, tcp_SocketType);
                    /* We also have a text version of the host address. */
                    setCStr_String(&addr->hostName, hbuf);
                    pushBack_ObjectList(list, addr);
                }
            }
        }
        freeifaddrs(addrs);
    }
#endif
    return list;
}

iDefineClass(Address)
