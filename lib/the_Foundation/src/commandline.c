/** @file commandline.c  Command line options parser.

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

#include "the_Foundation/commandline.h"
#include "the_Foundation/file.h"
#include "the_Foundation/block.h"
#include "the_Foundation/path.h"

#include <ctype.h>

iDeclareType(DefinedArg)
iDeclareStaticClass(DefinedArg)

iDefinedArg *new_DefinedArg(int minCount, int maxCount);

struct Impl_DefinedArg {
    iObject object;
    int minCount;
    int maxCount;
};

iDefineClass(CommandLine)
iDefineObjectConstructionArgs(CommandLine, (int argc, char **argv), argc, argv)

static void loadArgumentsFile_CommandLine_(iCommandLine *d, const char *path) {
    iBeginCollect();
    const iString *input;
    /* Read the argument file contents. */ {
        iFile *file = iClob(newCStr_File(path));
        if (!open_File(file, readOnly_FileMode | text_FileMode)) {
            iWarning("[CommandLine] arguments file \"%s\" not found\n", path);
            iEndCollect();
            return;
        }
        input = collect_String(readString_File(file));
        close_File(file);
    }
    /* Parse into separate arguments. */
    const char *i = cstr_String(input);
    iBool isDone = iFalse;
    iBool inQuote = iFalse;
    iBlock *word = collect_Block(new_Block(0));
    while (i != constEnd_String(input) && !isDone) {
        /* Skip initial whitespace. */
        i = skipSpace_CStr(i);
        /* Check for a nested argument file. */
        iBool isResponse = iFalse;
        if (*i == '@') {
            isResponse = iTrue;
            i = skipSpace_CStr(i + 1);
        }
        clear_Block(word);
        while (*i && (inQuote || !isspace((int) *i))) {
            iBool copyChar = iTrue;
            if (!inQuote) {
                if (*i == '"') {
                    inQuote = iTrue;
                    copyChar = iFalse;
                }
            }
            else { // Inside quotes.
                if (*i == '"') { // Quote ends.
                    if (i[1] == '"') { // Doubled?
                        /* Normal processing, but output only one quote. */
                        i++;
                    }
                    else {
                        inQuote = iFalse;
                        copyChar = iFalse;
                    }
                }
            }
            if (copyChar) pushBack_Block(word, *i);
            i++;
        }
        /* Word has been extracted, examine it. */
        if (isResponse) {
            loadArgumentsFile_CommandLine_(d, constData_Block(word));
        }
        else if (!cmpCStr_Block(word, "--")) { // End of arguments.
            isDone = iTrue;
        }
        else if (!isEmpty_Block(word)) {
            pushBackCStr_StringList(&d->args, constData_Block(word));
        }
    }
    iEndCollect();
}

static int cmpArg_(const iRangecc entry, const iRangecc arg) {
    iAssert(*arg.start != '-');
    iAssert(*entry.start != '-');
    if (size_Range(&arg) == 1) {
        /* Single-letter arguments can be joined in a longer entry. */
        return findAscii_Rangecc(entry, *arg.start)? 0 : 1;
    }
    const char *eql = findAscii_Rangecc(entry, '=');
    const size_t entryLen = (eql ? eql - entry.start : size_Range(&entry));
    if (size_Range(&arg) != entryLen) {
        return iCmp(size_Range(&arg), entryLen);
    }
    return iCmpStrN(entry.start, arg.start, size_Range(&arg));
}

static size_t findArg_CommandLine_(const iCommandLine *d, const iRangecc arg) {
    iAssert(*arg.start != '-');
    iConstForEach(StringList, i, &d->args) {
        iRangecc entry = range_String(i.value);
        if (*entry.start != '-') continue;
        entry.start++;
        if (size_Range(&arg) > 1) { // Long args must have two dashes.
            if (*entry.start != '-') continue;
            entry.start++;
        }
        else if (*entry.start == '-') {
            continue; // Want short, got long.
        }
        if (!cmpArg_(entry, arg)) {
            return i.pos;
        }
    }
    return iInvalidPos;
}

void init_CommandLine(iCommandLine *d, int argc, char **argv) {
    init_StringList(&d->args);
    d->defined = NULL;
    for (int i = 0; i < argc; ++i) {
        /* Load response files. */
        iString *arg = newLocalCStr_String(argv[i]);
        if (startsWith_String(arg, "@")) {
            loadArgumentsFile_CommandLine_(d, cstr_String(arg) + 1);
        }
        else {
            pushBack_StringList(&d->args, arg);
        }
        delete_String(arg);
    }
#if defined (iPlatformCygwin) || defined (iPlatformMsys)
    d->execPath = unixToWindows_Path(cstr_String(constFront_StringList(&d->args)));
    /* Cygwin strips the .exe extension from the executable name. */
    if (!endsWithCase_String(d->execPath, ".exe")) {
        appendCStr_String(d->execPath, ".exe");
    }
#else
    /* TODO: This does not work if the executable was started via PATH. It
       would be more reliable to use platform-specific means to determine the
       path. See: https://stackoverflow.com/a/1024937 */
    d->execPath = makeAbsolute_Path(constFront_StringList(&d->args));
#endif
}

void deinit_CommandLine(iCommandLine *d) {
    delete_String(d->execPath);
    iRelease(d->defined);
    deinit_StringList(&d->args);
}

void defineValues_CommandLine(iCommandLine *d, const char *arg, int valueCount) {
    defineValuesN_CommandLine(d, arg, valueCount == unlimitedValues_CommandLine ? 0 : valueCount, valueCount);
}

void defineValuesN_CommandLine(iCommandLine *d, const char *arg, int minCount, int maxCount) {
    if (!d->defined) {
        d->defined = new_StringHash();
    }
    iDefinedArg *def = new_DefinedArg(minCount, maxCount);
    const iRangecc args = range_CStr(arg);
    iRangecc range = iNullRange;
    while (nextSplit_Rangecc(args, ";", &range)) {
        insertCStrN_StringHash(d->defined, range.start, size_Range(&range), def);
    }
    iRelease(def);
}

iBool isDefined_CommandLine(const iCommandLine *d, const iString *arg) {
    if (!d->defined) {
        return iFalse;
    }
    iRangecc key = range_String(arg);
    /* Ignore one or two dashes in the beginning. */
    if (*key.start == '-') key.start++;
    if (*key.start == '-') key.start++;
    return constValueRange_StringHash(d->defined, key) != NULL;
}

iBool contains_CommandLine(const iCommandLine *d, const char *arg) {
    const iRangecc args = range_CStr(arg);
    iRangecc range = iNullRange;
    while (nextSplit_Rangecc(args, ";", &range)) {
        if (findArg_CommandLine_(d, range) != iInvalidPos)
            return iTrue;
    }
    return iFalse;
}

static iCommandLineArg *checkArgumentPosValuesN_CommandLine_(const iCommandLine *d, size_t pos,
                                                             size_t minCount, size_t maxCount) {
    const iString *lineEntry = constAt_StringList(&d->args, pos);
    size_t equalPos;
    if ((equalPos = indexOf_String(lineEntry, '=')) != iInvalidPos) {
        if (minCount > 1 || maxCount == 0) return NULL;
        /* There is a single value included in the entry. */
        iCommandLineArg *clArg = new_CommandLineArg();
        clArg->pos = pos;
        set_String(&clArg->arg, lineEntry);
        pushBackCStr_StringList(&clArg->values, cstr_String(lineEntry) + equalPos + 1);
        truncate_String(&clArg->arg, equalPos);
        return clArg;
    }
    /* Check how many values are provided. */
    size_t endPos;
    for (endPos = pos + 1;
         endPos < size_StringList(&d->args) &&
             !startsWith_String(constAt_StringList(&d->args, endPos), "-");
         endPos++) {}
    size_t valueCount = endPos - pos - 1;
    if (valueCount < minCount) {
        return NULL;
    }
    if (maxCount != (size_t) unlimitedValues_CommandLine) {
        valueCount = iMin(valueCount, maxCount);
    }
    iCommandLineArg *clArg = new_CommandLineArg();
    clArg->pos = pos;
    set_String(&clArg->arg, lineEntry);
    for (size_t i = pos + 1; i < pos + 1 + valueCount; ++i) {
        pushBack_StringList(&clArg->values, constAt_StringList(&d->args, i));
    }
    return clArg;
}

static iCommandLineArg *checkArgumentValuesN_CommandLine_(const iCommandLine *d,
                                                          const iRangecc arg, size_t minCount,
                                                          size_t maxCount) {
    const size_t pos = findArg_CommandLine_(d, arg);
    if (pos == iInvalidPos) return NULL;
    return checkArgumentPosValuesN_CommandLine_(d, pos, minCount, maxCount);
}

iCommandLineArg *checkArgumentValuesN_CommandLine(const iCommandLine *d, const char *arg,
                                                  int minCount, int maxCount) {
    const iRangecc args = range_CStr(arg);
    iRangecc range = iNullRange;
    while (nextSplit_Rangecc(args, ";", &range)) {
        iCommandLineArg *clArg = checkArgumentValuesN_CommandLine_(d, range, minCount, maxCount);
        if (clArg) return clArg;
    }
    return NULL;
}

static enum iCommandLineArgType argumentType_CommandLine_(const iCommandLine *d, size_t pos) {
    const iString *arg = constAt_StringList(&d->args, pos);
    if (startsWith_String(arg, "--") && size_String(arg) > 2) {
        return longArgument_CommandLineArgType;
    }
    if (startsWith_String(arg, "-")  && size_String(arg) > 1) {
        return shortArgument_CommandLineArgType;
    }
    return value_CommandLineArgType;
}

static size_t valueCountForArgument_CommandLine_(const iCommandLine *d, const iRangecc arg,
                                                 size_t pos) {
    if (!d->defined) return 0;
    const iDefinedArg *def = constValueRange_StringHash(d->defined, arg);
    if (def) {
        size_t count = 0;
        while (pos < size_StringList(&d->args) &&
               argumentType_CommandLine_(d, pos) == value_CommandLineArgType) {
            pos++;
            count++;
        }
        if (def->maxCount != unlimitedValues_CommandLine) {
            count = iMin(count, (size_t) def->maxCount);
        }
        return count;
    }
    return 0;
}

iCommandLineArg *checkArgument_CommandLine(const iCommandLine *d, const char *arg) {
    int minCount = 0, maxCount = 0; // By default, no values expected.
    iRangecc key = iNullRange;
    const iRangecc args = range_CStr(arg);
    while (d->defined && nextSplit_Rangecc(args, ";", &key)) {
        const iDefinedArg *defined = constValueRange_StringHash(d->defined, key);
        if (defined) {
            minCount = defined->minCount;
            maxCount = defined->maxCount;
            break;
        }
    }
    return checkArgumentValuesN_CommandLine(d, arg, minCount, maxCount);
}

/*-------------------------------------------------------------------------------------*/

void init_CommandLineConstIterator(iCommandLineConstIterator *d, const iCommandLine *cmdLine) {
    d->value = 0;
    d->argType = value_CommandLineArgType;
    d->cmdLine = cmdLine;
    d->valueCount = 0;
    d->isAssignedValue = iFalse;
    d->entry = (iRangecc){ NULL, NULL };
    next_CommandLineConstIterator(d);
}

static void updateShortArgumentValueCount_CommandLineConstIterator_
(iCommandLineConstIterator *d) {
    if (!*d->entry.end) {
        d->valueCount = valueCountForArgument_CommandLine_(d->cmdLine, d->entry, d->value + 1);
    }
    else {
        d->isAssignedValue = (*d->entry.end == '=');
        d->valueCount = d->isAssignedValue? 1 : 0;
    }
}

void next_CommandLineConstIterator(iCommandLineConstIterator *d) {
    if (d->argType == shortArgument_CommandLineArgType) {
        /* Go to the next short argument, if there is one. */
        if (*d->entry.end && *d->entry.end != '=') {
            shift_Range(&d->entry, 1);
            /* The last short option may have arguments. */
            updateShortArgumentValueCount_CommandLineConstIterator_(d);
            return;
        }
    }
    /* Advance to the next entry. */
    d->value += d->valueCount + 1 + (d->isAssignedValue ? -1 : 0);
    if (d->value >= size_StringList(&d->cmdLine->args)) { // Done?
        d->value = 0;
        return;
    }
    const iString *arg = constAt_StringList(&d->cmdLine->args, d->value);
    d->entry = range_String(arg);
    d->argType = argumentType_CommandLine_(d->cmdLine, d->value);
    d->isAssignedValue = iFalse;
    switch (d->argType) {
        case value_CommandLineArgType:
            d->valueCount = 0;
            break;
        case shortArgument_CommandLineArgType:
            d->entry.start++;
            d->entry.end = d->entry.start + 1;
            updateShortArgumentValueCount_CommandLineConstIterator_(d);
            break;
        case longArgument_CommandLineArgType: {
            d->entry.start += 2;
            const char *eql = findAscii_Rangecc(d->entry, '=');
            if (eql) {
                d->isAssignedValue = iTrue;
                d->entry.end = eql;
                d->valueCount = 1;
            }
            else {
                d->valueCount = valueCountForArgument_CommandLine_(d->cmdLine, d->entry, d->value + 1);
            }
            break;
        }
    }
}

iCommandLineArg *argument_CommandLineConstIterator(iCommandLineConstIterator *d) {
    if (d->argType == value_CommandLineArgType) return NULL;
    return checkArgumentPosValuesN_CommandLine_(d->cmdLine, d->value,
                                                d->valueCount, d->valueCount);
}

const iString *value_CommandLineConstIterator(iCommandLineConstIterator *d) {
    if (d->argType == value_CommandLineArgType) {
        return at_CommandLine(d->cmdLine, d->value);
    }
    return NULL;
}

iBool equal_CommandLineConstIterator(const iCommandLineConstIterator *d, const char *arg) {
    iRangecc entry = iNullRange;
    const iRangecc args = range_CStr(arg);
    while (nextSplit_Rangecc(args, ";", &entry)) {
        if (equalRange_Rangecc(d->entry, entry)) {
            return iTrue;
        }
    }
    return iFalse;
}

/*-------------------------------------------------------------------------------------*/

void init_DefinedArg(iDefinedArg *d, int min, int max) {
    d->minCount = min;
    d->maxCount = max;
}

void deinit_DefinedArg(iDefinedArg *d) {
    iUnused(d);
}

static iDefineClass(DefinedArg)

iDefineObjectConstructionArgs(DefinedArg, (int min, int max), min, max)

/*-------------------------------------------------------------------------------------*/

iDefineSubclass(CommandLineArg, StringList)
iDefineObjectConstruction(CommandLineArg)

void init_CommandLineArg(iCommandLineArg *d) {
    init_StringList(&d->values);
    init_String(&d->arg);
}

void deinit_CommandLineArg(iCommandLineArg *d) {
    deinit_String(&d->arg);
    deinit_StringList(&d->values);
}
