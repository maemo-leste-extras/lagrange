#pragma once

/** @file the_Foundation/address.h  Network address.

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

#include "object.h"
#include "audience.h"
#include "string.h"

iBeginPublic

iDeclareType(String)
iDeclareType(ObjectList)

iDeclareClass(Address)

iDeclareConstNotifyFunc(Address, LookupFinished)

iDeclareObjectConstruction(Address)

enum iSocketType {
    tcp_SocketType,
    udp_SocketType,
};

iAddress *  newBroadcast_Address(uint16_t port);
iAddress *  newSockAddr_Address (const void *sockAddr, size_t sockAddrSize, enum iSocketType socketType);

void        init_Address        (iAddress *);
void        deinit_Address      (iAddress *);

iBool       equal_Address       (const iAddress *, const iAddress *other);
iBool       isHostFound_Address (const iAddress *);
iBool       isValid_Address     (const iAddress *);
int         count_Address       (const iAddress *);

iDeclareType(SocketParameters)

struct Impl_SocketParameters {
    int family;
    int type;
    int protocol;
};

enum iSocketStringFlags {
    noPort_SocketStringFlag = 0x1,
};

iSocketParameters   socketParametersIndex_Address   (const iAddress *d, int index);
iSocketParameters   socketParametersFamily_Address  (const iAddress *, int family);
iString *           toString_Address        (const iAddress *);
iString *           toStringFlags_Address   (const iAddress *, int flags, int family); /* `family`==0: unspecified */
const iString *     hostName_Address        (const iAddress *);
uint16_t            port_Address            (const iAddress *);
iBool               isPending_Address       (const iAddress *);

/**
 * Starts an asynchronous address lookup. The lookupFinished audience will be notified when
 * the operation is complete. Alternatively, one can use waitForFinished_Address() to block
 * until the lookup is done.
 *
 * @param hostName    Hostname to look up. This can be a numerical IP address or a host name
 *                    that will be looked up via DNS. If the hostname is an empty string or NULL,
 *                    the address will represent any available local network interface.
 * @param port        TCP/UDP port number.
 * @param socketType  Type of the socket (TCP/UDP).
 */
void    lookupCStr_Address      (iAddress *, const char *hostName, uint16_t port, enum iSocketType socketType);

void    waitForFinished_Address (const iAddress *);

iLocalDef void lookupTcpCStr_Address(iAddress *d, const char *hostName, uint16_t port) {
    lookupCStr_Address(d, hostName, port, tcp_SocketType);
}

iLocalDef void lookupTcp_Address(iAddress *d, const iString *hostName, uint16_t port) {
    lookupTcpCStr_Address(d, cstr_String(hostName), port);
}

iDeclareAudienceGetter(Address, lookupFinished)

iObjectList *   networkInterfaces_Address(void); // list of iAddress objects

iEndPublic
