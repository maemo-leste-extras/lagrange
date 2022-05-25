/** @file the_Foundation/webrequest.c  HTTP(S)/FTP requests (using CURL).

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

#include "the_Foundation/webrequest.h"
#include "the_Foundation/stringarray.h"
#include "the_Foundation/buffer.h"
#include "the_Foundation/mutex.h"

#include <curl/curl.h>

#define iWebRequestProgressMinSize 0x10000

struct Impl_WebRequest {
    iObject object;
    iMutex mutex;
    CURL *curl;
    iBlock postData;
    iString postContentType;
    iBuffer *result;
    size_t receivedSize;
    size_t contentLength;
    size_t lastNotifySize;
    iString errorMessage;
    iStringArray *headers;
    /* Audiences: */
    iAudience *progress;
    iAudience *readyRead;
};

static size_t headerCallback_WebRequest_(char *ptr, size_t size, size_t nmemb, void *userdata) {
    iWebRequest *d = (iWebRequest *) userdata;
    const size_t len = size * nmemb;
    iString *hdr = newCStrN_String(ptr, len);
    trim_String(hdr);
    if (!isEmpty_String(hdr)) {
        iDebug("[WebRequest] Header received: `%s`\n", cstr_String(hdr));
        pushBack_StringArray(d->headers, hdr);
    }
    if (startsWith_String(hdr, "Content-Length:")) {
        d->contentLength = strtoul(cstr_String(hdr) + 15, NULL, 10);
    }
    delete_String(hdr);
    return len;
}

static size_t dataCallback_WebRequest_(char *ptr, size_t size, size_t nmemb, void *userdata) {
    iWebRequest *d = (iWebRequest *) userdata;
    const size_t len = size * nmemb;
    iGuardMutex(&d->mutex, {
        writeData_Buffer(d->result, ptr, len);
        d->receivedSize += len;
    });
    if (d->receivedSize - d->lastNotifySize > iWebRequestProgressMinSize) {
        d->lastNotifySize = d->receivedSize;
        iNotifyAudienceArgs(d, progress, WebRequestProgress, d->receivedSize, d->contentLength);
    }
    iNotifyAudience(d, readyRead, WebRequestReadyRead);
    return len;
}

void configure_WebRequest_(iWebRequest *d) {
    curl_easy_setopt(d->curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(d->curl, CURLOPT_TIMEOUT, 10);
    curl_easy_setopt(d->curl, CURLOPT_HEADERFUNCTION, headerCallback_WebRequest_);
    curl_easy_setopt(d->curl, CURLOPT_HEADERDATA, d);
    curl_easy_setopt(d->curl, CURLOPT_WRITEFUNCTION, dataCallback_WebRequest_);
    curl_easy_setopt(d->curl, CURLOPT_WRITEDATA, d);
}

void init_WebRequest(iWebRequest *d) {
    iAssertIsObject(d);
    init_Mutex(&d->mutex);
    d->curl = curl_easy_init();
    init_Block(&d->postData, 0);
    init_String(&d->postContentType);
    d->result = new_Buffer();
    openEmpty_Buffer(d->result);
    init_String(&d->errorMessage);
    d->headers = new_StringArray();
    d->progress = NULL;
    d->readyRead = NULL;
    configure_WebRequest_(d);
}

void deinit_WebRequest(iWebRequest *d) {
    curl_easy_cleanup(d->curl);
    delete_Audience(d->progress);
    iRelease(d->headers);
    deinit_String(&d->errorMessage);
    iRelease(d->result);
    deinit_String(&d->postContentType);
    deinit_Block(&d->postData);
    deinit_Mutex(&d->mutex);
}

void clear_WebRequest(iWebRequest *d) {
    curl_easy_reset(d->curl);
    configure_WebRequest_(d);
    clear_Block(&d->postData);
    clear_Buffer(d->result);
    clear_String(&d->errorMessage);
    clear_StringArray(d->headers);
}

void setUrl_WebRequest(iWebRequest *d, const iString *url) {
    curl_easy_setopt(d->curl, CURLOPT_URL, cstr_String(url));
}

void setUserAgent_WebRequest(iWebRequest *d, const iString *userAgent) {
    curl_easy_setopt(d->curl, CURLOPT_USERAGENT, cstr_String(userAgent));
}

void setPostData_WebRequest(iWebRequest *d, const char *contentType, const iBlock *data) {
    set_Block(&d->postData, data);
    format_String(&d->postContentType, "Content-Type: %s", contentType);
}

static iBool execute_WebRequest_(iWebRequest *d) {
    char errorMsg[CURL_ERROR_SIZE];
    d->contentLength = 0;
    d->receivedSize = 0;
    d->lastNotifySize = 0;
    clear_String(&d->errorMessage);
    clear_Buffer(d->result);
    clear_StringArray(d->headers);
    curl_easy_setopt(d->curl, CURLOPT_ERRORBUFFER, errorMsg);
    const iBool ok = (curl_easy_perform(d->curl) == CURLE_OK);
    if (!ok) {
        setCStr_String(&d->errorMessage, errorMsg);
        iWarning("[WebRequest] %s\n", errorMsg);
    }
    return ok;
}

iBool get_WebRequest(iWebRequest *d) {
    return execute_WebRequest_(d);
}

iBool post_WebRequest(iWebRequest *d) {
    struct curl_slist *headers = curl_slist_append(NULL, cstr_String(&d->postContentType));
    curl_easy_setopt(d->curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(d->curl, CURLOPT_POSTFIELDS, data_Block(&d->postData));
    curl_easy_setopt(d->curl, CURLOPT_POSTFIELDSIZE, size_Block(&d->postData));
    const iBool ok = execute_WebRequest_(d);
    curl_slist_free_all(headers);
    return ok;
}

const iBlock *result_WebRequest(const iWebRequest *d) {
    return data_Buffer(d->result);
}

size_t contentLength_WebRequest(const iWebRequest *d) {
    return d->contentLength;
}

const iStringArray *headers_WebRequest(const iWebRequest *d) {
    return d->headers;
}

const iString *errorMessage_WebRequest(const iWebRequest *d) {
    return &d->errorMessage;
}

iBool headerValue_WebRequest(const iWebRequest *d, const char *header, iString *value_out) {
    iBool found = iFalse;
    iConstForEach(StringArray, i, d->headers) {
        const iString *j = i.value;
        if (startsWith_String(j, header)) {
            found = iTrue;
            if (value_out) {
                setCStr_String(value_out, cstr_String(j) + strlen(header));
                trimStart_String(value_out);
            }
            break;
        }
    }
    return found;
}

iBlock *read_WebRequest(iWebRequest *d) {
    iBlock *bytes;
    iGuardMutex(&d->mutex, bytes = consumeAll_Buffer(d->result));
    return bytes;
}

iDefineClass(WebRequest)
iDefineObjectConstruction(WebRequest)
iDefineAudienceGetter(WebRequest, progress)
iDefineAudienceGetter(WebRequest, readyRead)
