/** @file wide.c  UTF-16 (i.e., Win32 wchar) utilities

@authors Copyright (c) 2021-2023 Jaakko Keränen <jaakko.keranen@iki.fi>

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

#include "wide.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

const wchar_t *toWide_CStr_(const char *u8) {
    const iString str = iStringLiteral(u8);
    iBlock *u16 = toUtf16_String(&str);
    return (wchar_t *) data_Block(collect_Block(u16));
}

const char *fromWide_CStr_(const wchar_t *ws) {
    return cstrLocal_String(collect_String(newUtf16_String((const uint16_t *) ws)));
}

const char *errorMessage_Windows_(uint32_t systemErrorNumber) {
    LPWSTR msg = NULL;
    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, 
        systemErrorNumber, 
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR) &msg, 
        0,
        NULL
    );
    if (msg) {
        iString str;
        initUtf16_String(&str, msg);
        LocalFree(msg);
        trimEnd_String(&str);
        const char *cstr = cstrLocal_String(&str);
        deinit_String(&str);
        return cstr;
    }
    iWarning("[Windows] FormatMessage failed: %x\n", GetLastError());
    return "";
}
