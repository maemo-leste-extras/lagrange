#pragma once

/** @file the_Foundation/commandline.h  Command line options parsing.

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

#include "stringlist.h"
#include "stringhash.h"

iBeginPublic

iDeclareClass(CommandLine)

iDeclareType(CommandLineArg)

struct Impl_CommandLine {
    iStringList args;
    iStringHash *defined;
    iString *execPath;
};

enum { unlimitedValues_CommandLine = -1 };

iDeclareObjectConstructionArgs(CommandLine, int argc, char **argv)

void        init_CommandLine        (iCommandLine *, int argc, char **argv);
void        deinit_CommandLine      (iCommandLine *);

void        defineValues_CommandLine  (iCommandLine *, const char *arg, int valueCount);
void        defineValuesN_CommandLine (iCommandLine *, const char *arg, int minCount, int maxCount);
iBool       isDefined_CommandLine     (const iCommandLine *, const iString *arg);

/**
 * Checks if the command line contains a specific argument.
 *
 * @param arg  Argument(s) (without any dashes). This can be a single argument or a list
 *             of semicolon-separated arguments.
 */
iBool       contains_CommandLine    (const iCommandLine *, const char *arg);

/**
 * Finds an argument and its defined values from the command line. If no values have
 * been defined for the argument, it is assumed that is has none.
 *
 * @param arg   Argument(s) to find. This can be a single argument or a list of
 *              semicolon-separated arguments. The first match is returned. Matching
 *              is done in the specified order.
 *
 * @return New CommandLineArg object with the found argument and values, or @c NULL.
 */
iCommandLineArg *checkArgument_CommandLine(const iCommandLine *, const char *arg);

/**
 * Finds an argument and its values from the command line. Any previously defined
 * values for the argument(s) are ignored.
 *
 * @param arg       Argument(s) to find. This can be a single argument or a list of
 *                  semicolon-separated arguments. The first match is returned. Matching
 *                  is done in the specified order.
 * @param minCount  Minimum number of values.
 * @param maxCount  Maximum number of values, or unlimitedValues_CommandLine.
 *
 * @return New CommandLineArg object with the found argument and values, or @c NULL.
 */
iCommandLineArg *checkArgumentValuesN_CommandLine  (const iCommandLine *, const char *arg,
                                                   int minCount, int maxCount);

iLocalDef const iString *executablePath_CommandLine(const iCommandLine *d) {
    return d->execPath;
}

iLocalDef const iStringList *args_CommandLine(const iCommandLine *d) {
    return &d->args;
}

iLocalDef const iString *at_CommandLine(const iCommandLine *d, size_t pos) {
    return constAt_StringList(&d->args, pos);
}

iLocalDef iCommandLineArg *checkArgumentValues_CommandLine(const iCommandLine *d, const char *arg, int count) {
    return checkArgumentValuesN_CommandLine(d, arg, count, count);
}

iDeclareConstIterator(CommandLine, const iCommandLine *)

enum iCommandLineArgType {
    value_CommandLineArgType,
    shortArgument_CommandLineArgType,
    longArgument_CommandLineArgType,
};

struct ConstIteratorImpl_CommandLine {
    size_t value;
    iRangecc entry;
    enum iCommandLineArgType argType;
    size_t valueCount;
    iBool isAssignedValue;
    const iCommandLine *cmdLine;
};

iCommandLineArg *   argument_CommandLineConstIterator   (iCommandLineConstIterator *);
const iString *     value_CommandLineConstIterator      (iCommandLineConstIterator *);
iBool               equal_CommandLineConstIterator      (const iCommandLineConstIterator *, const char *arg);

/*-------------------------------------------------------------------------------------*/

struct Impl_CommandLineArg {
    iStringList values;
    iString arg;
    size_t pos;
};

iDeclareClass(CommandLineArg)
iDeclareObjectConstruction(CommandLineArg)

void        init_CommandLineArg     (iCommandLineArg *);
void        deinit_CommandLineArg   (iCommandLineArg *);

iLocalDef const iString *value_CommandLineArg(const iCommandLineArg *d, size_t pos) {
    return constAt_StringList(&d->values, pos);
}

iLocalDef const iStringList *values_CommandLineArg(const iCommandLineArg *d) {
    return &d->values;
}

iEndPublic
