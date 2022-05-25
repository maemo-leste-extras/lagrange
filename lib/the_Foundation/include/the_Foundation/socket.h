#pragma once

/** @file the_Foundation/socket.h  TCP socket.

Socket is a bidirectional non-random-access Stream where data gets written by a
background I/O thread.

Every time something gets written to the Socket's output buffer, the I/O thread is woken
up and the pending data is sent. You may wish to use another Buffer to queue up data to
send in larger chunks.

Socket is derived from Stream, so all Stream methods can be used on it for writing and
reading data. Both reading and writing is non-blocking. When writing, a copy of the data
is made into an internal buffer for the I/O thread. When reading, only data already
received and waiting in the input buffer will be returned. To wait for incoming data you
can join the `readyRead` audience.

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

#include "stream.h"
#include "address.h"

iBeginPublic

typedef iStreamClass iSocketClass;

iDeclareType(Socket)
iDeclareType(Mutex)

iDeclareNotifyFunc    (Socket, Connected)
iDeclareNotifyFunc    (Socket, Disconnected)
iDeclareNotifyFuncArgs(Socket, Error, int error, const char *msg)
iDeclareNotifyFunc    (Socket, ReadyRead)
iDeclareNotifyFuncArgs(Socket, BytesWritten, size_t numBytes)
iDeclareNotifyFunc    (Socket, WriteFinished)
    
iDeclareAudienceGetter(Socket, connected)
iDeclareAudienceGetter(Socket, disconnected)
iDeclareAudienceGetter(Socket, error)
iDeclareAudienceGetter(Socket, readyRead)
iDeclareAudienceGetter(Socket, bytesWritten)
iDeclareAudienceGetter(Socket, writeFinished)

enum iSocketStatus {
    addressLookup_SocketStatus,
    initialized_SocketStatus,
    connecting_SocketStatus,
    connected_SocketStatus,
    disconnecting_SocketStatus,
    disconnected_SocketStatus,
};

iDeclareObjectConstructionArgs(Socket, const char *hostName, uint16_t port)

iSocket *   newAddress_Socket   (const iAddress *address);
iSocket *   newExisting_Socket  (int fd, const void *sockAddr, size_t sockAddrSize);

iBool       open_Socket     (iSocket *);
void        close_Socket    (iSocket *);

iBool               isOpen_Socket           (const iSocket *);
enum iSocketStatus  status_Socket           (const iSocket *);
size_t              receivedBytes_Socket    (const iSocket *);
size_t              bytesToSend_Socket      (const iSocket *);
const iAddress *    address_Socket          (const iSocket *);

iLocalDef void      flush_Socket        (iSocket *d) { flush_Stream((iStream *) d); }
iLocalDef iBlock *  readAll_Socket      (iSocket *d) { return readAll_Stream((iStream *) d); }
iLocalDef size_t    writeData_Socket    (iSocket *d, const void *data, size_t size) {
    return writeData_Stream((iStream *) d, data, size);
}
iLocalDef size_t    write_Socket        (iSocket *d, const iBlock *data) {
    return writeData_Socket(d, constData_Block(data), size_Block(data));
}

iEndPublic
