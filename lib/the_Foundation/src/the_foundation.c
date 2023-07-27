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

#if defined (iPlatformAndroid)
#  include <android/log.h>
#endif

#if defined (iPlatformWindows)
#  define WIN32_LEAN_AND_MEAN
#  include <Windows.h>
#  include "platform/win32/wide.h"
void init_Windows_(void);
void deinit_Windows_(void);
#endif

void deinitForThread_Garbage_(void); /* garbage.c */
void deinit_DatagramThreads_(void);  /* datagram.c */
void deinit_Address_(void);          /* address.c */
void deinit_Threads_(void);          /* thread.c */
void init_DatagramThreads_(void);    /* datagram.c */
void init_Locale(void);              /* locale */
void init_Threads(void);             /* thread.c */

static iBool hasBeenInitialized_ = iFalse;

static void checkDeinit_Foundation_(void) {
    iAssert(!isInitialized_Foundation());
    /* If deinit happens during atexit(), things may not work as expected, e.g., the program
       may hang on Windows. This may be because worker threads have already been forcibly
       terminated by the OS at this time. */
    deinit_Foundation();
}

void init_Foundation(void) {
    init_Threads();
    init_Garbage();
    iDebug("[the_Foundation] version:" iFoundationLibraryVersionCStr " cstd:%li\n",
           __STDC_VERSION__);
    /* Locale. */ {
        setLocale_Foundation();
    }
#if defined (iPlatformWindows)
    init_Windows_();
#endif
    hasBeenInitialized_ = iTrue;
    atexit(checkDeinit_Foundation_); /* deinit must be manually called (order undefined) */
}

void deinit_Foundation(void) {
    if (isInitialized_Foundation()) {
        hasBeenInitialized_ = iFalse;
        deinit_DatagramThreads_();
        deinit_Address_();
        deinitForThread_Garbage_();
        deinit_Threads_();
#if defined (iPlatformWindows)
        deinit_Windows_();
#endif
    }
}

iBool isInitialized_Foundation(void) {
    return hasBeenInitialized_;
}

void setLocale_Foundation(void) {
    init_Locale();
}

void printMessage_Foundation(FILE *output, const char *format, ...) {
#if defined (iPlatformAndroid)
    iBlock msg;
    init_Block(&msg, 0);
    va_list args;
    va_start(args, format);
    vprintf_Block(&msg, format, args);
    va_end(args);
    __android_log_print(ANDROID_LOG_DEBUG, "tfdn", "%s", cstr_Block(&msg));
#elif defined (iPlatformWindows)
    iUnused(output);
    iBlock msg;
    init_Block(&msg, 0);
    va_list args;
    va_start(args, format);
    vprintf_Block(&msg, format, args);
    va_end(args);
    OutputDebugStringW(toWide_CStr_(cstr_Block(&msg)));
#else
    va_list args;
    va_start(args, format);
    vfprintf(output, format, args);
    va_end(args);
#endif
}
