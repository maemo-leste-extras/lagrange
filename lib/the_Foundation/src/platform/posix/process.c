/** @file posix/process.c  Execute and communicate with child processes.

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

#include "the_Foundation/process.h"

#include "the_Foundation/array.h"
#include "the_Foundation/block.h"
#include "the_Foundation/stringlist.h"
#include "the_Foundation/path.h"
#include "pipe.h"

#include <fcntl.h>
#include <spawn.h>
#include <poll.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#if defined (iPlatformOther)
#   include <sys/select.h>
#endif

struct Impl_Process {
    iObject object;
    pid_t pid;
    iStringList *args;
    iStringList *envMods;
    iString workDir;
    iPipe pin;
    iPipe pout;
    iPipe perr;
    int exitStatus;
};

iDefineObjectConstruction(Process)
iDefineClass(Process)

extern char **environ; /* The environment variables. */

void init_Process(iProcess *d) {
    d->pid     = 0;
    d->args    = new_StringList();
    d->envMods = new_StringList();
    init_String(&d->workDir);
    init_Pipe(&d->pin);
    init_Pipe(&d->pout);
    init_Pipe(&d->perr);
    d->exitStatus = 0;
}

void deinit_Process(iProcess *d) {
    iRelease(d->args);
    iRelease(d->envMods);
    deinit_String(&d->workDir);
    deinit_Pipe(&d->pin);
    deinit_Pipe(&d->pout);
    deinit_Pipe(&d->perr);
    if (d->pid) {
        /* Allow zombies to terminate. */
        int status = 0;
        waitpid(d->pid, &status, WNOHANG);
    }
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
#if defined (iPlatformAndroid)
    /* posix_spawn requires API level 28 */
    return iFalse;
#else
    posix_spawn_file_actions_t facts;
    int rc;
    const char **argv;
    iString *oldCwd = cwd_Path();
    setCwd_Path(&d->workDir);
    argv = malloc(sizeof(char *) * (size_StringList(d->args) + 1));
    for (size_t i = 0; i < size_StringList(d->args); ++i) {
        argv[i] = cstr_String(at_StringList(d->args, i));
    }
    argv[size_StringList(d->args)] = NULL;
    /* Use pipes to redirect the child's stdout/stderr to us. */
    posix_spawn_file_actions_init(&facts);
    posix_spawn_file_actions_addclose(&facts, input_Pipe(&d->pin)); /* these are used by parent */
    posix_spawn_file_actions_addclose(&facts, output_Pipe(&d->pout));
    posix_spawn_file_actions_addclose(&facts, output_Pipe(&d->perr));
    posix_spawn_file_actions_adddup2 (&facts, output_Pipe(&d->pin), 0); /* child's stdin */
    posix_spawn_file_actions_addclose(&facts, output_Pipe(&d->pin));
    posix_spawn_file_actions_adddup2 (&facts, input_Pipe(&d->pout), 1); /* child's stdout */
    posix_spawn_file_actions_addclose(&facts, input_Pipe(&d->pout));
    posix_spawn_file_actions_adddup2 (&facts, input_Pipe(&d->perr), 2); /* child's stderr */
    posix_spawn_file_actions_addclose(&facts, input_Pipe(&d->perr));
    char **envs = environ;
    /* The environment. */
    if (!isEmpty_StringList(d->envMods)) {
        /* TODO: This doesn't handle changes to previously set variables. */
        iArray *env = collectNew_Array(sizeof(char *));
        for (char **e = environ; *e; e++) {
            pushBack_Array(env, e);
        }
        iConstForEach(StringList, e, d->envMods) {
            pushBack_Array(env, &(const char *){ cstr_String(e.value) });
        }
        pushBack_Array(env, &(const char *){ NULL });
        envs = data_Array(env);
    }
    /* Start the child process. */
    rc = posix_spawn(&d->pid, argv[0], &facts, NULL, iConstCast(char **, argv), envs);
    if (rc != 0) {
        iWarning("[Process] spawn error: %d %s\n", errno, strerror(errno));
    }
    free(argv);
    setCwd_Path(oldCwd);
    delete_String(oldCwd);
    posix_spawn_file_actions_destroy(&facts);
    close(output_Pipe(&d->pin));
    close(input_Pipe(&d->pout));
    close(input_Pipe(&d->perr));
    return rc == 0;
#endif
}

iProcessId pid_Process(const iProcess *d) {
    if (d) {
        return d->pid;
    }
    return getpid();
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
    if (!d->pid) return;
    close(input_Pipe(&d->pin)); /* no more input */
    int ws = 0;
    waitpid(d->pid, &ws, 0);
    if (WIFEXITED(ws)) {
        d->exitStatus = WEXITSTATUS(ws);
    }
    d->pid = 0;
}

size_t writeInput_Process(iProcess *d, const iBlock *data) {
    const char *ptr = constBegin_Block(data);
    size_t remain = size_Block(data);
    while (remain) {
        ssize_t num = write(input_Pipe(&d->pin), ptr, remain);
        if (num > 0) {
            ptr += num;
            remain -= num;
        }
        else break;
    }
    return size_Block(data) - remain;
}

static iBlock *readFromPipe_(int fd, iBlock *readChars) {
    char buf[4096];
    struct pollfd pfd = { .fd = fd, .events = POLLIN };
    while (poll(&pfd, 1, 0) == 1) { /* non-blocking */
        if (pfd.revents & POLLIN) {
            ssize_t num = read(fd, buf, sizeof(buf));
            if (num > 0) {
                appendData_Block(readChars, buf, num);
            }
            else break;
        }
        else break;
    }
    return readChars;
}

iBlock *readOutput_Process(iProcess *d) {
    return readFromPipe_(output_Pipe(&d->pout), new_Block(0));
}

iBlock *readError_Process(iProcess *d) {
    return readFromPipe_(output_Pipe(&d->perr), new_Block(0));
}

void kill_Process(iProcess *d) {
    if (d->pid) {
        kill(d->pid, SIGTERM);
    }
}

iBlock *readOutputUntilClosed_Process(iProcess *d) {
    iBlock *output = new_Block(0);
    const int fd = output_Pipe(&d->pout);
    close(input_Pipe(&d->pin)); /* no more input */
    for (;;) {
        char buf[0x20000];
        ssize_t len = read(fd, buf, sizeof(buf));
        if (len > 0) {
            appendData_Block(output, buf, len);
            continue;
        }
        if (len < 0 && isEmpty_Block(output)) {
            iWarning("[Process] failed to read output: %s\n", strerror(errno));
        }
        break; /* EOF */
    }
    return output;
}

iBool exists_Process(iProcessId pid) {
    if (!pid) return iFalse;
    return kill(pid, 0) == 0;
}
