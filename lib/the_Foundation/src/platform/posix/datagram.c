/** @file posix/datagram.c  UDP socket.

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
#include "the_Foundation/mutex.h"
#include "the_Foundation/address.h"
#include "the_Foundation/queue.h"
#include "the_Foundation/thread.h"
#include "the_Foundation/ptrset.h"
#include "pipe.h"

#include <errno.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

/* address.c */
int getSockAddr_Address(const iAddress *  d,
                        struct sockaddr **addr_out,
                        socklen_t *       addrSize_out,
                        int               family,
                        int               indexInFamily);

iDeclareClass(Message)

struct Impl_Message {
    iObject object;
    iAddress *address;
    iBlock data;
};

static void init_Message(iMessage *d) {
    d->address = NULL;
    init_Block(&d->data, 0);
}

static void deinit_Message(iMessage *d) {
    iRelease(d->address);
    deinit_Block(&d->data);
}

iDefineObjectConstruction(Message)
iDefineClass(Message)

/*-------------------------------------------------------------------------------------*/

struct Impl_Datagram {
    iObject object;
    iMutex mutex;
    uint16_t port;
    int fd;
    iAddress *address;
    iAddress *destination;
    iCondition allSent;
    iCondition messageReceived;
    iQueue *output;
    iQueue *input;
    /* Audiences: */
    iAudience *error;
    iAudience *message;
    iAudience *writeFinished;
};

iDeclareClass(DatagramThread)

enum iDatagramThreadMode {
    run_DatagramThreadMode,
    stop_DatagramThreadMode,
};

struct Impl_DatagramThread {
    iThread thread;
    iPipe wakeup;
    iMutex mutex;
    iPtrSet datagrams;
    iAtomicInt mode;
};

#define iMessageMaxDataSize 4096

static iThreadResult run_DatagramThread_(iThread *thread) {
    iDatagramThread *d = (iAny *) thread;
    iMutex *mtx = &d->mutex;
    while (d->mode == run_DatagramThreadMode) {
        /* Wait for activity. */
        fd_set reads, errors; {
            FD_ZERO(&reads);
            FD_ZERO(&errors);
            FD_SET(output_Pipe(&d->wakeup), &reads);
            int maxfd = output_Pipe(&d->wakeup);
            iGuardMutex(mtx, {
                iConstForEach(PtrSet, i, &d->datagrams) {
                    const iDatagram *dgm = *i.value;
                    FD_SET(dgm->fd, &reads);
                    FD_SET(dgm->fd, &errors);
                    maxfd = iMax(maxfd, dgm->fd);
                }
            });
            int ready = select(maxfd + 1, &reads, NULL, &errors, NULL);
            if (ready == -1) {
                return errno;
            }
        }
        /* Clear the wakeup. */
        if (FD_ISSET(output_Pipe(&d->wakeup), &reads)) {
            readByte_Pipe(&d->wakeup);
        }
        lock_Mutex(mtx); { // thread locked during datagram iteration
            iForEach(PtrSet, i, &d->datagrams) {
                iDatagram *dgm = *i.value;
                /* Problem with the socket? */
                if (FD_ISSET(dgm->fd, &errors)) {
                    iWarning("[Datagram] socket %i has exception status\n", dgm->fd);
                }
                /* Check for incoming data. */
                if (FD_ISSET(dgm->fd, &reads)) {
                    char buf[iMessageMaxDataSize];
                    struct sockaddr_storage addr;
                    socklen_t addrSize = sizeof(addr);
                    ssize_t dataSize = recvfrom(
                        dgm->fd, buf, iMessageMaxDataSize - 1, 0, (struct sockaddr *) &addr, &addrSize);
                    if (dataSize == -1) {
                        iWarning("[Datagram] socket %i: error %i while receiving: %s\n",
                                 dgm->fd, errno, strerror(errno));
                        iNotifyAudienceArgs(dgm, error, DatagramError, errno, strerror(errno));
                        /* Maybe remove the datagram from the set? */
                    }
                    /* Keep the data as a message. */ {
                        iMessage *msg = new_Message();
                        msg->address = newSockAddr_Address(&addr, addrSize, udp_SocketType);
                        setData_Block(&msg->data, buf, dataSize);
                        put_Queue(dgm->input, msg);
                        iRelease(msg);
                    }
                    iGuardMutex(&dgm->mutex, signal_Condition(&dgm->messageReceived));
                    if (dgm->message) {
                        iNotifyAudience(dgm, message, DatagramMessage);
                    }
                }
            }
        }
        unlock_Mutex(mtx);
        /* Now that received messages have been handled, check for outgoing messages. */
        lock_Mutex(mtx); {  // thread locked during datagram iteration
            iForEach(PtrSet, i, &d->datagrams) {
                iDatagram *dgm = *i.value;
                iMessage *msg = NULL;
                iBool didSend = iFalse;
                while ((msg = tryTake_Queue(dgm->output)) != NULL) {
                    socklen_t destLen;
                    struct sockaddr *destAddr;
                    getSockAddr_Address(msg->address, &destAddr, &destLen, AF_INET, 0);
                    ssize_t rc = sendto(dgm->fd,
                                        data_Block(&msg->data),
                                        size_Block(&msg->data),
                                        0,
                                        destAddr,
                                        destLen);
                    if (rc != (ssize_t) size_Block(&msg->data)) {
                        iWarning("[Datagram] socket %i: error %i while sending %zu bytes: %s\n",
                                 dgm->fd,
                                 errno,
                                 size_Block(&msg->data),
                                 strerror(errno));
                        iNotifyAudienceArgs(dgm, error, DatagramError, errno, strerror(errno));
                        /* Maybe remove the datagram from the set? */
                    }
                    iRelease(msg);
                    didSend = iTrue;
                }
                if (didSend) {
                    iGuardMutex(&dgm->mutex, signal_Condition(&dgm->allSent));
                    if (dgm->writeFinished) {
                        iNotifyAudience(dgm, writeFinished, DatagramWriteFinished);
                    }
                }
            }
        }
        unlock_Mutex(mtx);
    }
    return 0;
}

static void init_DatagramThread(iDatagramThread *d) {
    init_Thread(&d->thread, run_DatagramThread_);
    setName_Thread(&d->thread, "DatagramThread");
    init_Pipe(&d->wakeup);
    init_Mutex(&d->mutex);
    init_PtrSet(&d->datagrams);
    d->mode = run_DatagramThreadMode;
}

static void deinit_DatagramThread(iDatagramThread *d) {
    iGuardMutex(&d->mutex, {
        deinit_PtrSet(&d->datagrams);
        deinit_Mutex(&d->mutex);
        deinit_Pipe(&d->wakeup);
    });
}

iDefineObjectConstruction(DatagramThread)

iLocalDef void start_DatagramThread_(iDatagramThread *d) { start_Thread(&d->thread); }

static void exit_DatagramThread_(iDatagramThread *d) {
    d->mode = stop_DatagramThreadMode;
    writeByte_Pipe(&d->wakeup, 1);
    join_Thread(&d->thread);
}

static iDatagramThread *datagramIO_ = NULL;

void init_DatagramThreads_(void) {
    iAssert(datagramIO_ == NULL);
    datagramIO_ = new_DatagramThread();
    start_DatagramThread_(datagramIO_);
}

void deinit_DatagramThreads_(void) { /* called from deinit_Foundation */
    if (datagramIO_) {
        exit_DatagramThread_(datagramIO_);
        iRelease(datagramIO_);
        datagramIO_ = NULL;
    }
}

iDefineSubclass(DatagramThread, Thread)

/*-------------------------------------------------------------------------------------*/

iDefineObjectConstruction(Datagram)
iDefineClass(Datagram)
iDefineAudienceGetter(Datagram, error)
iDefineAudienceGetter(Datagram, message)
iDefineAudienceGetter(Datagram, writeFinished)

void init_Datagram(iDatagram *d) {
    init_Mutex(&d->mutex);
    d->port = 0;
    d->fd = -1;
    d->address = NULL;
    d->destination = NULL;
    init_Condition(&d->allSent);
    init_Condition(&d->messageReceived);
    d->output = new_Queue();
    d->input = new_Queue();
    d->error = NULL;
    d->message = NULL;
    d->writeFinished = NULL;
}

iBool isOpen_Datagram(const iDatagram *d) {
    return d->fd != -1;
}

uint16_t port_Datagram(const iDatagram *d) {
    return d->port;
}

iBool open_Datagram(iDatagram *d, uint16_t port) {
    if (isOpen_Datagram(d)) {
        return iFalse;
    }
    iAssert(port);
    if (d->address) iRelease(d->address);
    d->address = new_Address();
    d->port = port;
    lookupCStr_Address(d->address, NULL, port, udp_SocketType);
    waitForFinished_Address(d->address);
    /* Create and bind a socket for listening to incoming messages. */ {
        socklen_t sockLen;
        struct sockaddr *sockAddr;
        iSocketParameters sp = socketParametersFamily_Address(d->address, AF_INET);
        d->fd = socket(sp.family, sp.type, sp.protocol);
        if (d->fd == -1) {
            iWarning("[Datagram] error creating socket\n");
            iReleasePtr(&d->address);
            return iFalse;
        }
        /* Enable broadcasting. */ {
            const int broadcast = 1;
            setsockopt(d->fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
        }
        getSockAddr_Address(d->address, &sockAddr, &sockLen, AF_INET, 0 /* first one */);
        if (bind(d->fd, sockAddr, sockLen) == -1) {
            iReleasePtr(&d->address);
            close(d->fd);
            d->fd = -1;
            iWarning("[Datagram] error binding socket (port %u): %s\n", port, strerror(errno));
            return iFalse;
        }
    }
    /* All open datagrams share the I/O thread. */ {
        if (!datagramIO_) {
            init_DatagramThreads_();
        }
        iGuardMutex(&datagramIO_->mutex, insert_PtrSet(&datagramIO_->datagrams, d));
        writeByte_Pipe(&datagramIO_->wakeup, 1); // update the set of waiting datagrams
    }
    return iTrue;
}

void close_Datagram(iDatagram *d) {
    flush_Datagram(d);
    /* Remove from the I/O thread. */
    if (datagramIO_) {
        iGuardMutex(&datagramIO_->mutex, remove_PtrSet(&datagramIO_->datagrams, d));
        writeByte_Pipe(&datagramIO_->wakeup, 1); // update the set of waiting datagrams
    }
    iGuardMutex(&d->mutex, {
        if (isOpen_Datagram(d)) {
            close(d->fd);
            d->fd = -1;
        }
    });
}

void deinit_Datagram(iDatagram *d) {
    close_Datagram(d);
    iGuardMutex(&d->mutex, {
        iRelease(d->address);
        iRelease(d->destination);
        iRelease(d->output);
        iRelease(d->input);
        deinit_Condition(&d->allSent);
        deinit_Condition(&d->messageReceived);
        delete_Audience(d->error);
        delete_Audience(d->message);
        delete_Audience(d->writeFinished);
    });
    deinit_Mutex(&d->mutex);
}

void send_Datagram(iDatagram *d, const iBlock *data, const iAddress *to) {
    iAssert(to != NULL);
    iMessage *msg = new_Message();
    /* Block here until the address is resolved. We cannot block the datagram I/O thread because */
    /* it handles multiple sockets at once. */
    waitForFinished_Address(to);
    msg->address = ref_Object(to);
    set_Block(&msg->data, data);
    put_Queue(d->output, msg);
    iRelease(msg);
    writeByte_Pipe(&datagramIO_->wakeup, 1);
}

void sendData_Datagram(iDatagram *d, const void *data, size_t size, const iAddress *to) {
    iBlock buf;
    initData_Block(&buf, data, size);
    send_Datagram(d, &buf, to);
    deinit_Block(&buf);
}

iBlock *receive_Datagram(iDatagram *d, iAddress **from_out) {
    iMessage *msg = tryTake_Queue(d->input);
    iBlock *data = NULL;
    if (msg) {
        data = copy_Block(&msg->data);
        if (from_out) *from_out = ref_Object(msg->address);
        iRelease(msg);
    }
    else {
        if (from_out) *from_out = NULL;
    }
    return data;
}

void connect_Datagram(iDatagram *d, const iAddress *address) {
    iRelease(d->destination);
    d->destination = ref_Object(address);
}

void write_Datagram(iDatagram *d, const iBlock *data) {
    send_Datagram(d, data, d->destination);
}

void writeData_Datagram(iDatagram *d, const void *data, size_t size) {
    iBlock buf;
    initData_Block(&buf, data, size);
    write_Datagram(d, &buf);
    deinit_Block(&buf);
}

void disconnect_Datagram(iDatagram *d) {
    iRelease(d->destination);
    d->destination = NULL;
}

void flush_Datagram(iDatagram *d) {
    iGuardMutex(&d->mutex, {
        if (isOpen_Datagram(d) && !isEmpty_Queue(d->output)) {
            wait_Condition(&d->allSent, &d->mutex);
        }
    });
}
