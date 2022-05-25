/** @file the_Foundation/toml.c  TOML (subset) parser.

@authors Copyright (c) 2021 Jaakko Keränen <jaakko.keranen@iki.fi>

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

#include "the_Foundation/toml.h"

struct Impl_TomlParser {
    iTomlTableFunc    tableFunc;
    iTomlKeyValueFunc keyValueFunc;
    void *            context;
};

iDefineTypeConstruction(TomlParser)

static void trimComment_(iString *line) {
    iBool isQuoted = iFalse;
    iBool isEscape = iFalse;
    iConstForEach(String, i, line) {
        const iChar ch = i.value;
        if (isQuoted) {
            if (isEscape) {
                isEscape = iFalse;
            }
            else if (ch == '\\') {
                isEscape = iTrue;
            }
            else if (ch == '"') {
                isQuoted = iFalse;
            }
        }
        else {
            if (ch == '"') {
                isQuoted = iTrue;
            }
            else if (ch == '#') {
                truncate_Block(&line->chars, i.pos - cstr_String(line));
                trim_String(line);
                return;
            }
        }
    }
}

static iBool parseNumber_(iRangecc value, int base, iTomlValue *tv_out) {
    iString str;
    initRange_String(&str, value);
    char *endp = NULL;
    tv_out->value.int64 = strtoll(cstr_String(&str), &endp, base);
    tv_out->type = int64_TomlType;
    iBool ok = !*endp;
    if (!ok && (*endp == '.' || *endp == ',')) {
        tv_out->value.float64 = strtod(cstr_String(&str), &endp);
        if (!*endp) {
            tv_out->type = float64_TomlType;
            ok = iTrue;
        }
    }
    deinit_String(&str);
    return ok;
}

void init_TomlParser(iTomlParser *d) {
    d->tableFunc = NULL;
    d->keyValueFunc = NULL;
    d->context = NULL;
}

void deinit_TomlParser(iTomlParser *d) {
    iUnused(d);
}

void setHandlers_TomlParser(iTomlParser *d, iTomlTableFunc table, iTomlKeyValueFunc kv,
                            void *context) {
    d->tableFunc = table;
    d->keyValueFunc = kv;
    d->context = context;
}

static void notifyTable_TomlParser_(const iTomlParser *d, const iString *table, iBool isStart) {
    if (!isEmpty_String(table) && d->tableFunc) {
        d->tableFunc(d->context, table, isStart);
    }    
}

iBool parse_TomlParser(iTomlParser *d, const iString *toml) {
    iBool allOk = iTrue;
    iString line;
    iString table;
    iString key;
    init_String(&line);
    init_String(&table);
    init_String(&key);
    iRangecc srcLine = iNullRange;
    while (nextSplit_Rangecc(range_String(toml), "\n", &srcLine)) {
        setRange_String(&line, srcLine);
        trim_String(&line);
        if (isEmpty_String(&line) || *cstr_String(&line) == '#') {
            continue; /* skip empty/comment lines without futher ado */
        }
        trimComment_(&line);
        iRangecc rgLine = range_String(&line);
        if (startsWith_String(&line, "[") && endsWith_String(&line, "]")) {
            notifyTable_TomlParser_(d, &table, iFalse);
            setRange_String(&table, (iRangecc){ rgLine.start + 1, rgLine.end - 1 });
            trim_String(&table);
            notifyTable_TomlParser_(d, &table, iTrue);
            continue;
        }
        const size_t eqlPos = indexOfCStr_String(&line, "=");
        if (eqlPos == iInvalidPos) {
            allOk = iFalse;
            continue;
        }
        iRangecc rgKey = { rgLine.start, rgLine.start + eqlPos };
        iRangecc rgValue = { rgLine.start + eqlPos + 1, rgLine.end };
        trim_Rangecc(&rgKey);
        trim_Rangecc(&rgValue);
        if (isEmpty_Range(&rgKey) || isEmpty_Range(&rgValue)) {
            allOk = iFalse;
            continue;
        }
        setRange_String(&key, rgKey);        
        /* String value. */
        if (*rgValue.start == '"' && rgValue.end[-1] == '"') {
            rgValue.start++;
            rgValue.end--;
            iString quot;
            initRange_String(&quot, rgValue);
            iString *value = unquote_String(&quot);
            deinit_String(&quot);
            if (d->keyValueFunc) {
                d->keyValueFunc(
                    d->context,
                    &table,
                    &key,
                    &(iTomlValue){ .type = string_TomlType, .value = { .string = value } });
            }
            delete_String(value);
        }
        /* Boolean. */
        else if (equal_Rangecc(rgValue, "true") || equal_Rangecc(rgValue, "false")) {
            if (d->keyValueFunc) {
                d->keyValueFunc(
                    d->context,
                    &table,
                    &key,
                    &(iTomlValue){ .type  = boolean_TomlType,
                                   .value = { .boolean = equal_Rangecc(rgValue, "true") } });
            }
        }
        /* Hexadecimal value. */
        else if (size_Range(&rgValue) >= 3 && rgValue.start[0] == '0' && rgValue.start[1] == 'x') {
            iTomlValue tv;
            if (parseNumber_(rgValue, 16, &tv)) {
                if (d->keyValueFunc) {
                    d->keyValueFunc(d->context, &table, &key, &tv);
                }
            }
            else {
                allOk = iFalse;
            }
        }
        /* Decimal value. */
        else if (isNumeric_Char(*rgValue.start) || *rgValue.start == '-' || *rgValue.start == '+') {
            iTomlValue tv;
            if (parseNumber_(rgValue, 10, &tv)) {
                if (d->keyValueFunc) {
                    d->keyValueFunc(d->context, &table, &key, &tv);
                }
            }
            else {
                allOk = iFalse;
            }            
        }
        else {
            allOk = iFalse;
        }
    }
    notifyTable_TomlParser_(d, &table, iFalse);
    deinit_String(&key);
    deinit_String(&table);
    deinit_String(&line);
    return allOk;
}
