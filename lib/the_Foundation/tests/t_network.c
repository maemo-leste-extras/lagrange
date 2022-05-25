/**
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

#include <the_Foundation/address.h>
#include <the_Foundation/audience.h>
#include <the_Foundation/commandline.h>
#include <the_Foundation/string.h>
#include <the_Foundation/objectlist.h>
#include <the_Foundation/service.h>
#include <the_Foundation/socket.h>
#include <the_Foundation/thread.h>
#if defined (iHaveWebRequest)
#  include <the_Foundation/webrequest.h>
#endif
#if defined (iHaveTlsRequest)
#  include <the_Foundation/tlsrequest.h>
#endif

static void logConnected_(iAny *d, iSocket *sock) {
    iUnused(d);
    printf("Socket %p: connected\n", sock);
}

static void logDisonnected_(iAny *d, iSocket *sock) {
    iUnused(d);
    printf("Socket %p: disconnected\n", sock);
}

static void logWriteFinished_(iAny *d, iSocket *sock) {
    iUnused(d);
    printf("Socket %p: write finished\n", sock);
}

static void logReadyRead_(iAny *d, iSocket *sock) {
    iUnused(d);
    printf("Socket %p: data ready for reading (%zu bytes)\n", sock, receivedBytes_Socket(sock));
}

static void logError_(iAny *d, iSocket *sock, int code, const char *msg) {
    iUnused(d);
    printf("Socket %p: error %i: %s\n", sock, code, msg);
}

#if defined (iHaveWebRequest)
static void logWebRequestProgress_(iAny *d, iWebRequest *web, size_t currentSize, size_t totalSize) {
    iUnused(d);
    printf("WebRequest %p: downloaded %zu/%zu bytes\n", web, currentSize, totalSize);
}
#endif

static void observeSocket_(iSocket *sock) {
    iConnect(Socket, sock, connected, sock, logConnected_);
    iConnect(Socket, sock, disconnected, sock, logDisonnected_);
    iConnect(Socket, sock, error, sock, logError_);
    iConnect(Socket, sock, readyRead, sock, logReadyRead_);
    iConnect(Socket, sock, writeFinished, sock, logWriteFinished_);
}

static void hostLookedUp(iAny *d, const iAddress *address) {
    iUnused(d);
    if (isValid_Address(address)) {
        iString *ip = toString_Address(address);
        printf("%i IP addresses for %s; chosen: %s \n",
               count_Address(address),
               cstr_String(hostName_Address(address)),
               cstr_String(ip));
        delete_String(ip);
    }
}

static void printMessage_(iAny *any, iSocket *sock) {
    iUnused(any);
    iBlock *data = readAll_Socket(sock);
    printf("%s", constData_Block(data));
    delete_Block(data);
}

static iThreadResult messageReceiver_(iThread *thread) {
    iSocket *sock = userData_Thread(thread);
    iConnect(Socket, sock, readyRead, sock, printMessage_);
    printMessage_(NULL, sock);
    while (isOpen_Socket(sock)) {
        sleep_Thread(0.1);
    }
    iRelease(sock);
    iRelease(thread);
    return 0;
}

static void communicate_(iAny *d, iService *sv, iSocket *sock) {
    iUnused(d, sv);
    iString *addr = toString_Address(address_Socket(sock));
    printf("incoming connecting from %s\n", cstr_String(addr));
    delete_String(addr);
    /* Start a new thread to communicate through the socket. */
    iThread *receiver = new_Thread(messageReceiver_);
    setUserData_Thread(receiver, ref_Object(sock));
    observeSocket_(sock);
    start_Thread(receiver);
}

static bool connectTo_(const char *address) {
    iSocket *sock = iClob(new_Socket(address, 14666));
    observeSocket_(sock);
    if (!open_Socket(sock)) {
        puts("Failed to connect");
        return false;
    }
    puts("Type to send a message (empty to quit):");
    for (;;) {
        char buf[200];
        if (!fgets(buf, sizeof(buf), stdin)) {
            break;
        }
        if (strlen(buf) <= 1) break;
        writeData_Socket(sock, buf, strlen(buf));
    }
    puts("Good day!");
    return true;
}

#if defined (iHaveTlsRequest)
void printTlsRequestProgress_(iAnyObject *obj) {
    iTlsRequest *d = obj;
    printf("TlsRequest progress: %zu bytes received so far\n", receivedBytes_TlsRequest(d));
}

void printTlsRequestResult_(iAnyObject *obj) {
    iTlsRequest *d = obj;
    iBlock *result = collect_Block(readAll_TlsRequest(d));
    printf("Request %s\n", status_TlsRequest(d) == finished_TlsRequestStatus ?
           "succeeded." : "failed!");
    printf("--------TlsRequest-Result--------\n%s\n--------End-of-Result--------\n",
           cstr_Block(result));
}
#endif

int main(int argc, char *argv[]) {
    init_Foundation();
    /* List network interface addresses. */ {
        iObjectList *ifs = networkInterfaces_Address();
        printf("%zu network interfaces:\n", size_ObjectList(ifs));
        iConstForEach(ObjectList, i, ifs) {
            iString *str = toString_Address(i.object);
            printf("- %s\n", cstr_String(str));
            delete_String(str);
        }
        iRelease(ifs);
    }
    /* Check the arguments. */
    iCommandLine *cmdline = iClob(new_CommandLine(argc, argv)); {
#if defined (iHaveWebRequest)
        iCommandLineArg *getUrl = iClob(checkArgumentValues_CommandLine(cmdline, "g", 1));
        if (getUrl) {
            iWebRequest *web = iClob(new_WebRequest());
            setUrl_WebRequest(web, value_CommandLineArg(getUrl, 0));
            iConnect(WebRequest, web, progress, web, logWebRequestProgress_);
            printf("Getting URL \"%s\"...\n", cstr_String(value_CommandLineArg(getUrl, 0)));
            iBool ok = get_WebRequest(web);
            if (ok) {
                printf("Success! Received %zu bytes\n", size_Block(result_WebRequest(web)));
                iString *ctype = collect_String(new_String());
                iString *clen = collect_String(new_String());
                headerValue_WebRequest(web, "Content-Type:", ctype);
                headerValue_WebRequest(web, "Content-Length:", clen);
                printf("Content type: %s\n", cstr_String(ctype));
                if (!isEmpty_String(clen)) {
                    printf("Content length according to header: %s bytes\n", cstr_String(clen));
                }
                else {
                    puts("Content length omitted");
                }
            }
            else {
                printf("Failure! CURL says: %s\n", cstr_String(errorMessage_WebRequest(web)));
            }
            return 0;
        }
#endif
    }
#if defined (iHaveTlsRequest)
    /* Perform a TLS request. */ {
        if (contains_CommandLine(cmdline, "cert")) {
            iDate expiry;
            initCurrent_Date(&expiry);
            expiry.year++;
            iTlsCertificateName names[] = {
                { issuerCommonName_TlsCertificateNameType, collectNewCStr_String("t_network.c") },
                { subjectCommonName_TlsCertificateNameType, collectNewCStr_String("jaakko.keranen@iki.fi") },
                { subjectUserId_TlsCertificateNameType, collectNewCStr_String("skyjake") },
                { subjectDomain_TlsCertificateNameType, collectNewCStr_String("skyjake.fi") },
                { subjectCountry_TlsCertificateNameType, collectNewCStr_String("FI") },
                { 0, NULL }
            };
            iTlsCertificate *cert = newSelfSignedRSA_TlsCertificate(2048, expiry, names);
            iString *crt = collect_String(pem_TlsCertificate(cert));
            iString *key = collect_String(privateKeyPem_TlsCertificate(cert));
            printf("%s\n", cstr_String(crt));
            printf("Issuer: %s\n", cstrLocal_String(issuer_TlsCertificate(cert)));
            printf("Subject: %s\n", cstrLocal_String(subject_TlsCertificate(cert)));
            iDate until;
            validUntil_TlsCertificate(cert, &until);
            printf("Expires on: %s\n", cstrCollect_String(format_Date(&until, "%Y-%m-%d %H:%M:%S")));
            printf("%s\n", cstr_String(key));
            printf("Fingerprint 1: %s\n",
                   cstrCollect_String(
                       hexEncode_Block(collect_Block(fingerprint_TlsCertificate(cert)))));
            delete_TlsCertificate(cert);
            /* Try to recreate it. */
            cert = newPemKey_TlsCertificate(crt, key);
            printf("Fingerprint 2: %s\n",
                   cstrCollect_String(
                       hexEncode_Block(collect_Block(fingerprint_TlsCertificate(cert)))));
            printf("Recreated private key:\n%s", cstrCollect_String(privateKeyPem_TlsCertificate(cert)));
            delete_TlsCertificate(cert);
            return 0;
        }
        iCommandLineArg *tlsArgs = iClob(checkArgumentValues_CommandLine(cmdline, "t;tls", 2));
        if (tlsArgs) {
            iTlsRequest *tls = iClob(new_TlsRequest());
            setHost_TlsRequest(tls,
                               value_CommandLineArg(tlsArgs, 0),
                               toInt_String(value_CommandLineArg(tlsArgs, 1)));
            iConnect(TlsRequest, tls, readyRead, tls, printTlsRequestProgress_);
            iConnect(TlsRequest, tls, finished, tls, printTlsRequestResult_);
            iString *content = collectNew_String();
            format_String(content, "gemini://%s/\r\n", cstr_String(value_CommandLineArg(tlsArgs, 0)));
            setContent_TlsRequest(tls, utf8_String(content));
            submit_TlsRequest(tls);
            waitForFinished_TlsRequest(tls);
            printf("We are done.\n");
            return 0;
        }
    }
#endif
    if (contains_CommandLine(cmdline, "s;server")) {
        iService *sv = iClob(new_Service(14666));
        iConnect(Service, sv, incomingAccepted, sv, communicate_);
        if (!open_Service(sv)) {
            puts("Failed to start service");
            return 1;
        }
        puts("Press Enter to quit..."); {
            char line[2];
            if (!fgets(line, sizeof(line), stdin)) {
                iWarning("fgets failed\n");
            }
        }
    }
    else if (contains_CommandLine(cmdline, "c;client")) {
        connectTo_("localhost");
    }
    else {
        iCommandLineArg *arg = checkArgumentValuesN_CommandLine(cmdline, "h;host", 1, 1);
        if (arg) {
            connectTo_(cstr_String(value_CommandLineArg(arg, 0)));
            iRelease(arg);
        }
        iConstForEach(CommandLine, i, cmdline) {
            if (i.argType == value_CommandLineArgType) {
                printf("\nLooking up \"%s\"...\n", cstr_String(value_CommandLineConstIterator(&i)));
                iAddress *addr = new_Address();
                iConnect(Address, addr, lookupFinished, addr, hostLookedUp);
                lookupTcp_Address(addr, value_CommandLineConstIterator(&i), 0);
                waitForFinished_Address(addr);
                iRelease(addr);
            }
        }
    }
    return 0;
}
