#pragma once

/** @file the_Foundation/datagram.h  UDP socket.

Datagram is an IPv4 UDP network socket that can send and receive short messages.

@authors Copyright (c) 2018 Jaakko Keränen <jaakko.keranen@iki.fi>

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

iBeginPublic

iDeclareClass(Datagram)

iDeclareObjectConstruction(Datagram)

iDeclareNotifyFuncArgs  (Datagram, Error, int error, const char *msg)
iDeclareNotifyFunc      (Datagram, Message)
iDeclareNotifyFunc      (Datagram, WriteFinished)
iDeclareAudienceGetter  (Datagram, error)
iDeclareAudienceGetter  (Datagram, message)
iDeclareAudienceGetter  (Datagram, writeFinished)

iDeclareType(Address)
iDeclareType(Block)

iBool       open_Datagram       (iDatagram *, uint16_t port);
void        close_Datagram      (iDatagram *);

iBool       isOpen_Datagram     (const iDatagram *);
uint16_t    port_Datagram       (const iDatagram *);

void        send_Datagram       (iDatagram *, const iBlock *data, const iAddress *to);
void        sendData_Datagram   (iDatagram *, const void *data, size_t size, const iAddress *to);
iBlock *    receive_Datagram    (iDatagram *, iAddress **from_out);

void        connect_Datagram    (iDatagram *, const iAddress *address);
void        write_Datagram      (iDatagram *, const iBlock *data);
void        writeData_Datagram  (iDatagram *, const void *data, size_t size);
void        disconnect_Datagram (iDatagram *);

void        flush_Datagram      (iDatagram *);

iEndPublic
