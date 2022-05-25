/** @file c_plus.c  Library initialization.

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

#include "the_Foundation/defs.h"
#include "the_Foundation/string.h"
#include "the_Foundation/time.h"
#include "the_Foundation/version.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>

static iBool hasBeenInitialized_ = iFalse;

void deinitForThread_Garbage_(void); /* garbage.c */
void deinit_DatagramThreads_(void);  /* datagram.c */
void deinit_Address_(void);          /* address.c */
void deinit_Threads_(void);          /* thread.c */
void init_DatagramThreads_(void);    /* datagram.c */
void init_Locale(void);              /* locale */
void init_Threads(void);             /* thread.c */

void init_Foundation(void) {
    init_Threads();
    init_Garbage();
    iDebug("[the_Foundation] version: %i.%i.%i%s cstd:%li\n",
           version_Foundation.major, version_Foundation.minor, version_Foundation.patch,
           strlen(iFoundationLibraryGitTag) ? format_CStr("-%s", iFoundationLibraryGitTag) : "",
           __STDC_VERSION__);
    /* Locale. */ {
        setLocale_Foundation();
    }
    hasBeenInitialized_ = iTrue;
    atexit(deinit_Foundation); /* should be manually called, though */
}

void deinit_Foundation(void) {
    if (isInitialized_Foundation()) {
        hasBeenInitialized_ = iFalse;
        deinit_DatagramThreads_();
        deinit_Address_();
        deinitForThread_Garbage_();
        deinit_Threads_();
    }
}

iBool isInitialized_Foundation(void) {
    return hasBeenInitialized_;
}

void setLocale_Foundation(void) {
    init_Locale();
}

void printMessage_Foundation(FILE *output, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(output, format, args);
    va_end(args);
}
