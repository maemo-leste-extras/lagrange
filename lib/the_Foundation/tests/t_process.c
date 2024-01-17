/**
@authors Copyright (c) 2023 Jaakko Keränen <jaakko.keranen@iki.fi>

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

#include <the_Foundation/garbage.h>
#include <the_Foundation/process.h>
#include <the_Foundation/stringlist.h>
#include <the_Foundation/thread.h>
#include <the_Foundation/math.h>

#define TEST_MULTITHREAD 0

#if TEST_MULTITHREAD
iMutex mtx_; /* avoid pipe issues */

static iThreadResult run_(iThread *d) {
    iStringList *args = newStringsCStr_StringList(
#if defined (iPlatformWindows)
        "c:\\msys64\\usr\\bin\\ls.exe",
#else
        "/bin/cat",
#endif
        NULL
    );
    iString inputData;
    initCStr_String(&inputData, "Hello world, this is some TEXT for testing.\n");
    for (int n = 0; n < 100; n++) {
        iBeginCollect();
        lock_Mutex(&mtx_);
        iProcess *proc = iClob(new_Process());
        setArguments_Process(proc, args);
        printf("[%p] Output #%d:\n", d, n);
        if (start_Process(proc)) {
            writeInput_Process(proc, utf8_String(&inputData));
            iBlock *out = collect_Block(readOutputUntilClosed_Process(proc));
            printf("[%p] %s\n", d, cstr_Block(out));
        }
        else {
            printf("[%p] FAILED TO START\n", d);
        }
        unlock_Mutex(&mtx_);
        iEndCollect();
    }
    deinit_String(&inputData);
    iRelease(args);
    return 0;
}
#endif

int main(int argc, char *argv[]) {
    iUnused(argc, argv);    
    init_Foundation();
    /* Test GUI launch. */ {
        iProcess *proc = new_Process();
        setArguments_Process(proc, collect_StringList(newStringsCStr_StringList(
#if defined (iPlatformWindows)
            "c:\\windows\\notepad.exe",
#elif defined (iPlatformApple)
            "/System/Applications/TextEdit.app/Contents/MacOS/TextEdit",                                  
#endif
            NULL)));
        //start_Process(proc);
        waitForFinished_Process(proc);
        iRelease(proc);
    }
    /* Test arguments and environment. */ {
        iProcess *proc = new_Process();
        setArguments_Process(proc, collect_StringList(newStringsCStr_StringList(
#if defined (iPlatformWindows)
            "c:\\msys64\\usr\\bin\\printenv.exe",
#else
            "/usr/bin/printenv",
#endif
            NULL)));
        setEnvironment_Process(proc,
                               collect_StringList(newStringsCStr_StringList(
                                   "TESTVALUE=Keränen", "OTHER=something with space", NULL)));
        if (start_Process(proc)) {
            waitForFinished_Process(proc);
            iBlock *out = collect_Block(readOutput_Process(proc));
            iString *outs = collect_String(newBlock_String(out));
            printf("Child environment:\n%s\n", cstrLocal_String(outs));
        }
        else {
            printf("Failed to start\n");
        }
        iRelease(proc);
    }
#if TEST_MULTITHREAD
    init_Mutex(&mtx_);
    iThread *thds[8];
    iForIndices(i, thds) {
        thds[i] = new_Thread(run_);
        start_Thread(thds[i]);
    }
    iForIndices(i, thds) {
        join_Thread(thds[i]);
        iRelease(thds[i]);
    }
#endif
    deinit_Foundation();
    return 0;
}
