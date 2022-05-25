/** @file win32/process.c  Execute and communicate with child processes.

@authors Copyright (c) 2019 Jaakko Keränen <jaakko.keranen@iki.fi>

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
#include "the_Foundation/stringlist.h"
#include "the_Foundation/path.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// #include <spawn.h>
// #include <poll.h>
// #include <unistd.h>
// #include <signal.h>
// #include <sys/wait.h>

struct Impl_Process {
    iObject object;
    iStringList *args;
    iString workDir;
    PROCESS_INFORMATION procInfo;
    // iPipe pout;
    // iPipe perr;
};

iDefineObjectConstruction(Process)
iDefineClass(Process)

extern char **environ; // The environment variables.

void init_Process(iProcess *d) {
    d->args = new_StringList();
    init_String(&d->workDir);
    iZap(d->procInfo);
    // init_Pipe(&d->pout);
    // init_Pipe(&d->perr);
}

void deinit_Process(iProcess *d) {
    iRelease(d->args);
    deinit_String(&d->workDir);
    // deinit_Pipe(&d->pout);
    // deinit_Pipe(&d->perr);
}

void setArguments_Process(iProcess *d, const iStringList *args) {
    clear_StringList(d->args);
    iConstForEach(StringList, i, args) {
        pushBack_StringList(d->args, i.value);
    }
}

void setWorkingDirectory_Process(iProcess *d, const iString *cwd) {
    set_String(&d->workDir, cwd);
}

iBool start_Process(iProcess *d) {
    // posix_spawn_file_actions_t facts;
    // int rc;
    // const char **argv;
    // iString *oldCwd = cwd_Path();
    // setCwd_Path(&d->workDir);
    // argv = malloc(sizeof(char *) * (size_StringList(d->args) + 1));
    // for (size_t i = 0; i < size_StringList(d->args); ++i) {
    //     argv[i] = cstr_String(at_StringList(d->args, i));
    // }
    // argv[size_StringList(d->args)] = NULL;
    // // Use pipes to redirect the child's stdout/stderr to us.
    // posix_spawn_file_actions_init(&facts);
    // posix_spawn_file_actions_addclose(&facts, output_Pipe(&d->pout));
    // posix_spawn_file_actions_addclose(&facts, output_Pipe(&d->perr));
    // posix_spawn_file_actions_adddup2 (&facts, input_Pipe(&d->pout), 1); // stdout
    // posix_spawn_file_actions_adddup2 (&facts, input_Pipe(&d->perr), 2); // stderr
    // posix_spawn_file_actions_addclose(&facts, input_Pipe(&d->pout));
    // posix_spawn_file_actions_addclose(&facts, input_Pipe(&d->perr));
    // // Start the child process.
    // rc = posix_spawn(&d->pid, argv[0], &facts, NULL, iConstCast(char **, argv), environ);
    // free(argv);
    // setCwd_Path(oldCwd);
    // delete_String(oldCwd);
    // posix_spawn_file_actions_destroy(&facts);
    // return rc == 0;
    return iFalse;
}

iProcessId pid_Process(const iProcess *d) {
    // if (d) {
    //     return d->pid;
    // }
    // return getpid();
    return 0;
}

iBool isRunning_Process(const iProcess *d) {
    return iFalse;
    // if (!d->pid) return iFalse;
    // int status = 0;
    // pid_t res = wait4(d->pid, &status, WNOHANG, NULL);
    // if (res == 0) {
    //     return iTrue;
    // }
    // iConstCast(iProcess *, d)->pid = 0;
    // return iFalse;
}

void waitForFinished_Process(iProcess *d) {
    // if (!d->pid) return;
    // waitpid(d->pid, NULL, 0);
    // d->pid = 0;
}

// static iString *readFromPipe_(int fd, iString *readChars) {
//     char buf[4096];
//     struct pollfd pfd = {.fd = fd, .events = POLLIN};
//     while (poll(&pfd, 1, 0) == 1) { // non-blocking
//         if (pfd.revents & POLLIN) {
//             ssize_t num = read(fd, buf, sizeof(buf));
//             if (num > 0) {
//                 appendData_Block(&readChars->chars, buf, num);
//             }
//             else break;
//         }
//         else break;
//     }
//     return readChars;
// }

size_t writeInput_Process(iProcess *d, const iBlock *data) {
    return 0;
}

iBlock *readOutput_Process(iProcess *d) {
    // return readFromPipe_(output_Pipe(&d->pout), new_String());
    return new_Block(0);
}

iBlock *readError_Process(iProcess *d) {
    // return readFromPipe_(output_Pipe(&d->perr), new_String());
    return new_Block(0);
}

iBlock *readOutputUntilClosed_Process(iProcess *d) {
    return new_Block(0);
}

void kill_Process(iProcess *d) {
    // if (d->pid) {
    //     kill(d->pid, SIGTERM);
    // }
}
