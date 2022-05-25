/** @file win32/datagram.c  UDP socket.

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

#include "the_Foundation/datagram.h"
#include "the_Foundation/queue.h"

struct Impl_Datagram {
    iObject object;
    iMutex mutex;
    uint16_t port;
    // int fd;
    iAddress *address;
    iAddress *destination;
    iCondition allSent;
    iCondition messageReceived;
    iQueue *output;
    iQueue *input;
    // Audiences:
    iAudience *error;
    iAudience *message;
    iAudience *writeFinished;
};

//---------------------------------------------------------------------------------------

void init_DatagramThreads_(void) {

}

void deinit_DatagramThreads_(void) {

}

//---------------------------------------------------------------------------------------

iDefineObjectConstruction(Datagram)
iDefineClass(Datagram)
iDefineAudienceGetter(Datagram, error)
iDefineAudienceGetter(Datagram, message)
iDefineAudienceGetter(Datagram, writeFinished)

void init_Datagram(iDatagram *d) {
    iUnused(d);
}

void deinit_Datagram(iDatagram *d) {
    iUnused(d);
}

iBool open_Datagram(iDatagram *d, uint16_t port) {
    iUnused(d, port);
    return iFalse;
}

void close_Datagram(iDatagram *d) {
    iUnused(d);
}

iBool isOpen_Datagram(const iDatagram *d) {
    iUnused(d);
    return iFalse;
}

uint16_t port_Datagram(const iDatagram *d) {
    iUnused(d);
    return 0;
}

void send_Datagram(iDatagram *d, const iBlock *data, const iAddress *to) {
    iUnused(d, data, to);
}

void sendData_Datagram(iDatagram *d, const void *data, size_t size, const iAddress *to) {
    iUnused(d, data, size, to);
}

iBlock *receive_Datagram(iDatagram *d, iAddress **from_out) {
    iUnused(d, from_out);
    return NULL;
}

void connect_Datagram(iDatagram *d, const iAddress *address) {
    iUnused(d, address);
}

void write_Datagram(iDatagram *d, const iBlock *data) {
    iUnused(d, data);
}

void writeData_Datagram(iDatagram *d, const void *data, size_t size) {
    iUnused(d, data, size);
}

void disconnect_Datagram (iDatagram *d) {
    iUnused(d);
}

void flush_Datagram(iDatagram *d) {
    iUnused(d);
}
