#pragma once

/** @file the_Foundation/process.h  Execute and communicate with child processes.

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
#include <sys/types.h>

#if defined (iPlatformWindows)
typedef int iProcessId;
#else
typedef pid_t iProcessId;
#endif

iBeginPublic

iDeclareType(String)
iDeclareType(StringList)
iDeclareType(Block)

iDeclareClass(Process)
iDeclareObjectConstruction(Process)

void        setArguments_Process        (iProcess *, const iStringList *args);
void        setEnvironment_Process      (iProcess *, const iStringList *env); /* additions to environ; "name=value" */
void        setWorkingDirectory_Process (iProcess *, const iString *cwd);

iBool       start_Process           (iProcess *);
void        kill_Process            (iProcess *);
void        waitForFinished_Process (iProcess *);
size_t      writeInput_Process      (iProcess *, const iBlock *data);
iBlock *    readOutput_Process      (iProcess *);
iBlock *    readError_Process       (iProcess *);

iBlock *    readOutputUntilClosed_Process   (iProcess *); /* blocking */

iProcessId  pid_Process             (const iProcess *); /* NULL for current process */
iBool       isRunning_Process       (const iProcess *);
int         exitStatus_Process      (const iProcess *); /* valid after waitForFinished returns */

iLocalDef iProcessId currentId_Process(void) {
    return pid_Process(NULL);
}

iBool       exists_Process          (iProcessId pid);

iEndPublic
