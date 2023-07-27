/** @file win32/process.c  Execute and communicate with child processes.

@authors Copyright (c) 2019-2023 Jaakko Keränen <jaakko.keranen@iki.fi>

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

#include "the_Foundation/process.h"

#include "the_Foundation/block.h"
#include "the_Foundation/stringlist.h"
#include "pipe.h"
#include "wide.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

struct Impl_Process {
    iObject object;
    DWORD pid;
    iStringList *args;
    iStringList *envMods;
    iString workDir;
    iPipe pin;
    iPipe pout;
    iPipe perr;
    iBlock *bufOut;
    PROCESS_INFORMATION procInfo;
    STARTUPINFOW startInfo;
    int exitStatus;
};

iDefineObjectConstruction(Process)
iDefineClass(Process)

void init_Process(iProcess *d) {
    d->pid     = 0;
    d->args    = new_StringList();
    d->envMods = new_StringList();
    init_String(&d->workDir);
    init_Pipe(&d->pin);
    init_Pipe(&d->pout);
    init_Pipe(&d->perr);
    d->bufOut = new_Block(0);
    iZap(d->procInfo);
    d->startInfo = (STARTUPINFOW){
        .cb         = sizeof(d->startInfo),
        .hStdError  = input_Pipe(&d->perr),
        .hStdOutput = input_Pipe(&d->pout),
        .hStdInput  = output_Pipe(&d->pin),
        .dwFlags    = STARTF_USESTDHANDLES,
    };
    d->exitStatus = 0;
    /* Child must not inherit our end of the pipes. */
    SetHandleInformation(output_Pipe(&d->pout), HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(output_Pipe(&d->perr), HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(input_Pipe(&d->pin), HANDLE_FLAG_INHERIT, 0);
    /* Non-blocking stdout and stderr. */ {
        DWORD mode = PIPE_NOWAIT;
        SetNamedPipeHandleState(output_Pipe(&d->pout), &mode, NULL, NULL);
        SetNamedPipeHandleState(output_Pipe(&d->perr), &mode, NULL, NULL);
    }

}

void deinit_Process(iProcess *d) {
    if (d->pid) {
        if (d->procInfo.hProcess != INVALID_HANDLE_VALUE) {
            CloseHandle(d->procInfo.hProcess);
        }
    }
    iRelease(d->args);
    iRelease(d->envMods);
    deinit_String(&d->workDir);
    deinit_Pipe(&d->pin);
    deinit_Pipe(&d->pout);
    deinit_Pipe(&d->perr);
    delete_Block(d->bufOut);
}

void setArguments_Process(iProcess *d, const iStringList *args) {
    clear_StringList(d->args);
    iConstForEach(StringList, i, args) {
        pushBack_StringList(d->args, i.value);
    }
}

void setEnvironment_Process(iProcess *d, const iStringList *env) {
    clear_StringList(d->envMods);
    iConstForEach(StringList, i, env) {
        pushBack_StringList(d->envMods, i.value);
    }
}

void setWorkingDirectory_Process(iProcess *d, const iString *cwd) {
    set_String(&d->workDir, cwd);
}

iBool start_Process(iProcess *d) {
    LPVOID envs = NULL;
    iString *cmdLine = new_String();
    /* Build the command line, quoting arguments as needed. */ {
        iConstForEach(StringList, i, d->args) {
            iString *arg = copy_String(i.value);
            if (contains_String(arg, ' ') || contains_String(arg, '"')) {
                replace_String(arg, "\\", "\\\\");
                replace_String(arg, "\"", "\\\"");
                prependCStr_String(arg, "\"");
                appendCStr_String(arg, "\"");
            }
            if (!isEmpty_String(cmdLine)) {
                appendCStr_String(cmdLine, " ");
            }
            append_String(cmdLine, arg);
            delete_String(arg);
        }
    }
    /* The environment. */
    if (!isEmpty_StringList(d->envMods)) {
        iBlock *envUtf16 = collect_Block(new_Block(0));
        for (const wchar_t *e = GetEnvironmentStringsW(); *e; ) {
            size_t len = wcslen(e);
            appendData_Block(envUtf16, e, (len + 1) * 2);
            e += len + 1;
        }
        iConstForEach(StringList, e, d->envMods) {
            iBlock *wide = toUtf16_String(e.value);
            append_Block(envUtf16, wide);
            delete_Block(wide);
            appendData_Block(envUtf16, &(wchar_t){ 0 }, 2);
        }
        appendData_Block(envUtf16, (wchar_t[]){ 0, 0 }, 4);
        envs = (LPVOID) data_Block(envUtf16);
    }
    DWORD creationFlags = CREATE_UNICODE_ENVIRONMENT;
    BOOL ok = CreateProcessW(
        NULL,
        (LPWSTR) toWide_CStr_(cstr_String(cmdLine)),
        NULL,
        NULL,
        TRUE,
        creationFlags,
        envs,
        !isEmpty_String(&d->workDir) ? toWide_CStr_(cstr_String(&d->workDir)) : NULL,
        &d->startInfo,
        &d->procInfo
    );
    if (ok) {
        d->pid = d->procInfo.dwProcessId;
        /* These were inherited by the child, so we won't need them any longer. */
        CloseHandle(input_Pipe(&d->pout));
        CloseHandle(input_Pipe(&d->perr));
        CloseHandle(output_Pipe(&d->pin));
        CloseHandle(d->procInfo.hThread);
    }
    else {
        iWarning("[Process] spawn error: (%x) %s\n", GetLastError(), errorMessage_Windows_(GetLastError()));
        d->procInfo.hProcess = INVALID_HANDLE_VALUE;
        d->procInfo.hThread = INVALID_HANDLE_VALUE;
    }
    deinit_String(cmdLine);
    return ok ? iTrue : iFalse;
}

iProcessId pid_Process(const iProcess *d) {
    if (d) {
        return d->pid;
    }
    return GetCurrentProcessId();
}

iBool isRunning_Process(const iProcess *d) {
    if (!d->pid) return iFalse;
    if (!exists_Process(d->pid)) {
        iConstCast(iProcess *, d)->pid = 0;
        return iFalse;
    }
    return iTrue;
}

int exitStatus_Process(const iProcess *d) {
    return d->exitStatus;
}

void waitForFinished_Process(iProcess *d) {
    if (d->pid) {
        iBlock *out = readOutputUntilClosed_Process(d);
        append_Block(d->bufOut, out);
        delete_Block(out);
        DWORD exitCode = 0;
        BOOL ok = GetExitCodeProcess(d->procInfo.hProcess, &exitCode);
        if (ok) {
            d->exitStatus = exitCode;
        }
        d->pid = 0;
    }
}

size_t writeInput_Process(iProcess *d, const iBlock *data) {
    const char *ptr = constBegin_Block(data);
    DWORD remain = size_Block(data);
    while (remain) {
        DWORD num = 0;
        if (!WriteFile(input_Pipe(&d->pin), ptr, remain, &num, NULL)) {
            break;
        }
        ptr += num;
        remain -= num;
    }
    return size_Block(data) - remain;
}

static iBlock *readFromPipe_(HANDLE pipe, iBlock *readChars) {
    char buf[4096];
    DWORD num = 0;
    while (ReadFile(pipe, buf, sizeof(buf), &num, NULL)) {
        appendData_Block(readChars, buf, num);
    }
    return readChars;
}

iBlock *readOutput_Process(iProcess *d) {
    iBlock *buf = copy_Block(d->bufOut);
    clear_Block(d->bufOut);
    return readFromPipe_(output_Pipe(&d->pout), buf);
}

iBlock *readError_Process(iProcess *d) {
    return readFromPipe_(output_Pipe(&d->perr), new_Block(0));
}

void kill_Process(iProcess *d) {
    if (d->pid) {
        TerminateProcess(d->procInfo.hProcess, (UINT) -1);
    }
}

iBlock *readOutputUntilClosed_Process(iProcess *d) {
    iBlock *output = copy_Block(d->bufOut);
    clear_Block(d->bufOut);
    if (!d->pid) {
        return output;
    }
    HANDLE fd = output_Pipe(&d->pout);
    CloseHandle(input_Pipe(&d->pin)); /* no more input */
    for (;;) {
        char buf[0x20000];
        DWORD len = 0;
        if (ReadFile(fd, buf, sizeof(buf), &len, NULL)) {
            appendData_Block(output, buf, len);
            if (len > 0) {
                continue;
            }
            else {
                break;
            }
        }
        const DWORD err = GetLastError();
        //iDebug("error (%x) %s\n", err, errorMessage_Windows_(err));
        if (err == ERROR_PIPE_NOT_CONNECTED || err == ERROR_BROKEN_PIPE) {
            break;
        }
    }
    iString str;
    initBlock_String(&str, output);
    deinit_String(&str);
    return output;
}

iBool exists_Process(iProcessId pid) {
    if (!pid) return iFalse;
    HANDLE proc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (proc) {
        CloseHandle(proc);
        return iTrue;
    }
    return iFalse;
}
