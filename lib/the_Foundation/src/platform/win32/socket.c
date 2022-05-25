/** @file win32/socket.c  TCP socket.

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

#include "the_Foundation/socket.h"
#include "the_Foundation/buffer.h"
#include "the_Foundation/mutex.h"
#include "the_Foundation/thread.h"
// #include "pipe.h"

// #include <errno.h>
// #include <unistd.h>
// #include <sys/socket.h>
// #include <sys/types.h>

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>

// address.c
// void getSockAddr_Address(const iAddress *  d,
//                          struct sockaddr **addr_out,
//                          socklen_t *       addrSize_out,
//                          int               family);

iDeclareType(SocketThread)

struct Impl_Socket {
    iStream stream;
    iBuffer *output;
    iBuffer *input;
    enum iSocketStatus status;
    iAddress *address;
    // int fd;
    iThread *connecting;
    iSocketThread *thread;
    iCondition allSent;
    iMutex mutex;
    // Audiences:
    iAudience *connected;
    iAudience *disconnected;
    iAudience *error;
    iAudience *readyRead;
    iAudience *bytesWritten;
    iAudience *writeFinished;
};

static iSocketClass Class_Socket;
static void shutdown_Socket_(iSocket *d);

iDefineAudienceGetter(Socket, connected)
iDefineAudienceGetter(Socket, disconnected)
iDefineAudienceGetter(Socket, error)
iDefineAudienceGetter(Socket, readyRead)
iDefineAudienceGetter(Socket, bytesWritten)
iDefineAudienceGetter(Socket, writeFinished)

//---------------------------------------------------------------------------------------

enum iSocketThreadMode {
    run_SocketThreadMode,
    stop_SocketThreadMode,
};

iDeclareClass(SocketThread)

struct Impl_SocketThread {
    iThread thread;
    iSocket *socket;
    // iPipe wakeup;
    volatile enum iSocketThreadMode mode;
};

static iThreadResult run_SocketThread_(iThread *thread) {
    iSocketThread *d = (iAny *) thread;
    // iMutex *smx = &d->socket->mutex;
    // iBlock *inbuf = collect_Block(new_Block(0x20000));
    while (d->mode == run_SocketThreadMode) {
        // if (bytesToSend_Socket(d->socket) > 0) {
        //     // Make sure we won't block on select() when there's still data to send.
        //     writeByte_Pipe(&d->wakeup, 0);
        // }
        // // Wait for activity.
        // fd_set reads, errors; {
        //     FD_ZERO(&reads);
        //     FD_ZERO(&errors);
        //     FD_SET(output_Pipe(&d->wakeup), &reads);
        //     FD_SET(d->socket->fd, &reads);
        //     FD_SET(d->socket->fd, &errors);
        //     int maxfd = iMax(output_Pipe(&d->wakeup), d->socket->fd);
        //     int ready = select(maxfd + 1, &reads, NULL, &errors, NULL);
        //     if (ready == -1) {
        //         return errno;
        //     }
        // }
        // if (FD_ISSET(output_Pipe(&d->wakeup), &reads)) {
        //     readByte_Pipe(&d->wakeup);
        // }
        // // Problem with the socket?
        // if (FD_ISSET(d->socket->fd, &errors)) {
        //     if (status_Socket(d->socket) == connected_SocketStatus) {
        //         iWarning("[Socket] error when receiving: %s\n", strerror(errno));
        //         shutdown_Socket_(d->socket);
        //         return errno;
        //     }
        //     return 0;
        // }
        // /* Check for data to send. */ {
        //     iBlock *data = NULL;
        //     size_t remaining = 0;
        //     iGuardMutex(smx, {
        //         if (d->mode != stop_SocketThreadMode) {
        //             if (!isEmpty_Buffer(d->socket->output)) {
        //                 data = consumeBlock_Buffer(d->socket->output, 0x10000);
        //             }
        //         }
        //     });
        //     if (data) {
        //         const char *ptr = constData_Block(data);
        //         const size_t totalToSend = remaining = size_Block(data);
        //         while (remaining > 0) {
        //             ssize_t sent = send(d->socket->fd, ptr, remaining, 0);
        //             if (sent == -1) {
        //                 // Error!
        //                 delete_Block(data);
        //                 shutdown_Socket_(d->socket);
        //                 return errno;
        //             }
        //             remaining -= sent;
        //             ptr += sent;
        //         }
        //         delete_Block(data);
        //         iNotifyAudienceArgs(d->socket, bytesWritten, SocketBytesWritten, totalToSend);
        //         iGuardMutex(smx, {
        //             if (isEmpty_Buffer(d->socket->output)) {
        //                 signal_Condition(&d->socket->allSent);
        //                 if (d->socket->writeFinished) {
        //                     unlock_Mutex(smx);
        //                     iNotifyAudience(d->socket, writeFinished, SocketWriteFinished);
        //                     lock_Mutex(smx);
        //                 }
        //             }
        //         }
        //     });
        // }
        // // Check for incoming data.
        // if (FD_ISSET(d->socket->fd, &reads)) {
        //     ssize_t readSize = recv(d->socket->fd, data_Block(inbuf), size_Block(inbuf), 0);
        //     if (readSize == 0) {
        //         iWarning("[Socket] peer closed the connection\n");
        //         shutdown_Socket_(d->socket);
        //         return 0;
        //     }
        //     if (readSize == -1) {
        //         if (status_Socket(d->socket) == connected_SocketStatus) {
        //             iWarning("[Socket] error when receiving: %s\n", strerror(errno));
        //             shutdown_Socket_(d->socket);
        //             return errno;
        //         }
        //         // This was expected.
        //         return 0;
        //     }
        //     iGuardMutex(smx, {
        //         writeData_Buffer(d->socket->input, constData_Block(inbuf), readSize);
        //         signal_Condition(&d->socket->input->dataAvailable);
        //     });
        //     iNotifyAudience(d->socket, readyRead, SocketReadyRead);
        // }
    }
    return 0;
}

static void init_SocketThread(iSocketThread *d, iSocket *socket,
                              enum iSocketThreadMode mode) {
    init_Thread(&d->thread, run_SocketThread_); {
        iString name;
        init_String(&name);
        format_String(&name, "SocketThread (%p)", NULL /*handle*/);
        setName_Thread(&d->thread, cstr_String(&name));
        deinit_String(&name);
    }
    // init_Pipe(&d->wakeup);
    d->socket = socket;
    d->mode = mode;
}

static void deinit_SocketThread(iSocketThread *d) {
    // deinit_Pipe(&d->wakeup);
}

static void exit_SocketThread_(iSocketThread *d) {
    d->mode = stop_SocketThreadMode;
    // writeByte_Pipe(&d->wakeup, 1); // select() will exit
    join_Thread(&d->thread);
}

iDefineSubclass(SocketThread, Thread)
iDefineObjectConstructionArgs(SocketThread,
                              (iSocket *socket, enum iSocketThreadMode mode),
                              socket, mode)

static inline void start_SocketThread(iSocketThread *d) { start_Thread(&d->thread); }

//---------------------------------------------------------------------------------------

iDefineObjectConstructionArgs(Socket,
                              (const char *hostName, uint16_t port),
                              hostName, port)

static iBool setStatus_Socket_(iSocket *d, enum iSocketStatus status) {
    if (d->status != status) {
        d->status = status;
        iDebug("[Socket] %p: state changed to %i (in thread %p)\n", d, status,
               current_Thread());
        return iTrue;
    }
    return iFalse;
}

static void init_Socket_(iSocket *d) {
    init_Stream(&d->stream);
    d->output = new_Buffer();
    d->input = new_Buffer();
    openEmpty_Buffer(d->output);
    openEmpty_Buffer(d->input);
    // d->fd = -1;
    d->address = NULL;
    d->connecting = NULL;
    d->thread = NULL;
    init_Condition(&d->allSent);
    init_Mutex(&d->mutex);
    d->connected = NULL;
    d->disconnected = NULL;
    d->error = NULL;
    d->readyRead = NULL;
    d->bytesWritten = NULL;
    d->writeFinished = NULL;
}

void deinit_Socket(iSocket *d) {
    close_Socket(d);
    iGuardMutex(&d->mutex, {
        iReleasePtr(&d->output);
        iReleasePtr(&d->input);
        iReleasePtr(&d->address);
    });
    deinit_Mutex(&d->mutex);
    deinit_Condition(&d->allSent);
    delete_Audience(d->connected);
    delete_Audience(d->disconnected);
    delete_Audience(d->error);
    delete_Audience(d->readyRead);
    delete_Audience(d->bytesWritten);
    delete_Audience(d->writeFinished);
}

static void startThread_Socket_(iSocket *d) {
    d->thread = new_SocketThread(d, run_SocketThreadMode);
    start_SocketThread(d->thread);
}

static void stopThread_Socket_(iSocket *d) {
    if (d->thread) {
        iAssert(current_Thread() != &d->thread->thread);
        exit_SocketThread_(d->thread);
        iReleasePtr(&d->thread);
    }
}

static void shutdown_Socket_(iSocket *d) {
    iGuardMutex(&d->mutex, {
        setStatus_Socket_(d, disconnecting_SocketStatus);
        // if (d->fd >= 0) {
        //     shutdown(d->fd, SHUT_RD);
        // }
    });
    iBool notify = iFalse;
    iGuardMutex(&d->mutex, {
        // if (d->fd >= 0) {
        //     close(d->fd);
        //     d->fd = -1;
        // }
        notify = setStatus_Socket_(d, disconnected_SocketStatus);
        // iAssert(d->fd < 0);
        iAssert(!d->connecting);
    });
    if (notify) {
        iNotifyAudience(d, disconnected, SocketDisconnected);
    }
}

// static iThreadResult connectAsync_Socket_(iThread *thd) {
//     iSocket *d = userData_Thread(thd);
//     struct sockaddr *addr;
//     socklen_t addrSize;
//     getSockAddr_Address(d->address, &addr, &addrSize, AF_UNSPEC);
//     const int rc = connect(d->fd, addr, addrSize);
//     iGuardMutex(&d->mutex, {
//         if (d->status == connecting_SocketStatus) {
//             if (rc == 0) {
//                 setStatus_Socket_(d, connected_SocketStatus);
//                 startThread_Socket_(d);
//                 if (d->connected) {
//                     unlock_Mutex(&d->mutex);
//                     iNotifyAudience(d, connected, SocketConnected);
//                     lock_Mutex(&d->mutex);
//                 }
//             }
//             else {
//                 setStatus_Socket_(d, disconnected_SocketStatus);
//                 const int errNum = errno;
//                 const char *msg = strerror(errNum);
//                 iWarning("[Socket] connection failed: %s\n", msg);
//                 if (d->error) {
//                     unlock_Mutex(&d->mutex);
//                     iNotifyAudienceArgs(d, error, SocketError, errNum, msg);
//                     lock_Mutex(&d->mutex);
//                 }
//             }
//         }
//         d->connecting = NULL;
//     });
//     iRelease(thd);
//     return rc;
// }

static iBool open_Socket_(iSocket *d) {
    // The socket is assumed to be locked already.
    if (isPending_Address(d->address)) {
        setStatus_Socket_(d, connecting_SocketStatus);
        return iTrue; // when address is resolved, the socket will be opened.
    }
    else if (!isValid_Address(d->address)) {
        return iFalse;
    }
    else {
        // const iSocketParameters sp = socketParameters_Address(d->address, AF_UNSPEC);
        // d->fd = socket(sp.family, sp.type, sp.protocol);
        // /// @todo On Linux, set the O_NONBLOCK flag on the socket.
        // d->connecting = new_Thread(connectAsync_Socket_);
        // setStatus_Socket_(d, connecting_SocketStatus);
        // setUserData_Thread(d->connecting, d);
        // start_Thread(d->connecting);
        return iTrue;
    }
}

static void addressLookedUp_Socket_(iAny *any, const iAddress *address) {
    iUnused(address);
    iSocket *d = any;
    // This is being called from another thread.
    iGuardMutex(&d->mutex, {
        if (d->status == addressLookup_SocketStatus) {
            setStatus_Socket_(d, initialized_SocketStatus);
        }
        else if (d->status == connecting_SocketStatus) {
            open_Socket_(d);
        }
    });
}

iSocket *newAddress_Socket(const iAddress *address) {
    iSocket *d = iNew(Socket);
    init_Socket_(d);
    waitForFinished_Address(address);
    d->address = ref_Object(address);
    setStatus_Socket_(d, initialized_SocketStatus);
    return d;
}

iSocket *newExisting_Socket(int fd, const void *sockAddr, size_t sockAddrSize) {
    iSocket *d = iNew(Socket);
    init_Socket_(d);
    // d->fd = fd;
    // d->address = newSockAddr_Address(sockAddr, sockAddrSize, tcp_SocketType);
    setStatus_Socket_(d, connected_SocketStatus);
    startThread_Socket_(d);
    return d;
}

void init_Socket(iSocket *d, const char *hostName, uint16_t port) {
    init_Socket_(d);
    d->address = new_Address();
    setStatus_Socket_(d, addressLookup_SocketStatus);
    iConnect(Address, d->address, lookupFinished, d, addressLookedUp_Socket_);
    lookupTcpCStr_Address(d->address, hostName, port);
}

iBool open_Socket(iSocket *d) {
    iBool ok;
    iGuardMutex(&d->mutex, {
        if (isOpen_Socket(d)) {
            ok = iFalse;
        }
        else {
            ok = open_Socket_(d);
        }
    });
    return ok;
}

void close_Socket(iSocket *d) {
    stopThread_Socket_(d);
    iGuardMutex(&d->mutex, {
        if (d->status == disconnected_SocketStatus ||
            d->status == disconnecting_SocketStatus) {
            unlock_Mutex(&d->mutex);
            return;
        }
        if (d->status == connecting_SocketStatus) {
            // shutdown(d->fd, SHUT_WR);
        }
        else if (d->status == connected_SocketStatus) {
            flush_Socket(d);
        }
        setStatus_Socket_(d, disconnecting_SocketStatus);
    });
    guardJoin_Thread(d->connecting, &d->mutex);
    iReleasePtr(&d->connecting);
    shutdown_Socket_(d);
}

iBool isOpen_Socket(const iSocket *d) {
    iBool open;
    iGuardMutex(&d->mutex, {
        open = (d->status == connecting_SocketStatus ||
                d->status == connected_SocketStatus);
    });
    return open;
}

enum iSocketStatus status_Socket(const iSocket *d) {
    enum iSocketStatus status;
    iGuardMutex(&d->mutex, status = d->status);
    return status;
}

size_t bytesToSend_Socket(const iSocket *d) {
    size_t n;
    iGuardMutex(&d->mutex, n = size_Buffer(d->output));
    return n;
}

size_t receivedBytes_Socket(const iSocket *d) {
    size_t n;
    iGuardMutex(&d->mutex, n = size_Buffer(d->input));
    return n;
}

const iAddress *address_Socket(const iSocket *d) {
    return d->address;
}

static size_t seek_Socket_(iStream *d, size_t pos) {
    iUnused(d, pos);
    iAssert(false); // not allowed
    return 0;
}

static size_t read_Socket_(iSocket *d, size_t size, void *data_out) {
    size_t readSize = 0;
    iGuardMutex(&d->mutex, {
        readSize = consume_Buffer(d->input, size, data_out);
    });
    return readSize;
}

static size_t write_Socket_(iSocket *d, const void *data, size_t size) {
    iGuardMutex(&d->mutex, {
        writeData_Stream(stream_Buffer(d->output), data, size);
        if (d->thread) {
            // writeByte_Pipe(&d->thread->wakeup, 0); // wake up the I/O thread
        }
    });
    return size;
}

static void flush_Socket_(iSocket *d) {
    iGuardMutex(&d->mutex, {
        if (!isEmpty_Buffer(d->output)) {
            wait_Condition(&d->allSent, &d->mutex);
        }
    });
}

static iBeginDefineSubclass(Socket, Stream)
    .seek   = seek_Socket_,
    .read   = (size_t (*)(iStream *, size_t, void *))       read_Socket_,
    .write  = (size_t (*)(iStream *, const void *, size_t)) write_Socket_,
    .flush  = (void   (*)(iStream *))                       flush_Socket_,
iEndDefineClass(Socket)
