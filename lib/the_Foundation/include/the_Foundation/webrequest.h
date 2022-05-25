#pragma once

/** @file the_Foundation/webrequest.h  HTTP(S)/FTP requests (using CURL).

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
#include "string.h"
#include "audience.h"

iBeginPublic

iDeclareType(StringArray)

iDeclareClass(WebRequest)
iDeclareObjectConstruction(WebRequest)

iDeclareNotifyFuncArgs(WebRequest, Progress, size_t currentBytes, size_t totalBytes)
iDeclareNotifyFunc    (WebRequest, ReadyRead)
iDeclareAudienceGetter(WebRequest, progress)
iDeclareAudienceGetter(WebRequest, readyRead)

void    clear_WebRequest        (iWebRequest *);

void    setUrl_WebRequest       (iWebRequest *, const iString *url);
void    setUserAgent_WebRequest (iWebRequest *, const iString *userAgent);
void    setPostData_WebRequest  (iWebRequest *, const char *contentType, const iBlock *data);

iBool   get_WebRequest          (iWebRequest *);
iBool   post_WebRequest         (iWebRequest *);

iBlock *read_WebRequest         (iWebRequest *);

const iBlock *          result_WebRequest       (const iWebRequest *);
size_t                  contentLength_WebRequest(const iWebRequest *);
const iStringArray *    headers_WebRequest      (const iWebRequest *);
const iString *         errorMessage_WebRequest (const iWebRequest *);

/**
 * Finds the value of an HTTP header from the result.
 *
 * @param header      Header to find, for example "Content-Type:".
 * @param value_out   Value of the header is returned here. Must be an initialized iString
 *                    or NULL for checking if the header exists.
 *
 * @return @c iTrue, if the header was found; otherwise @c iFalse.
 */
iBool   headerValue_WebRequest  (const iWebRequest *, const char *header, iString *value_out);

iEndPublic
