#pragma once

/** @file the_Foundation/toml.h  TOML (subset) parser.

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

#include "the_Foundation/string.h"

iDeclareType(TomlParser)
iDeclareTypeConstruction(TomlParser)

enum iTomlType {
    string_TomlType,
    int64_TomlType,
    float64_TomlType,
    boolean_TomlType,
};

iDeclareType(TomlValue)

struct Impl_TomlValue {
    enum iTomlType type;
    union {
        const iString *string;
        int64_t        int64;
        double         float64;
        iBool          boolean;
    } value;
};

iLocalDef double number_TomlValue(const iTomlValue *d) {
    return d->type == float64_TomlType   ? d->value.float64
           : d->type == int64_TomlType   ? d->value.int64
           : d->type == boolean_TomlType ? (d->value.boolean ? 1.0 : 0.0)
                                         : 0;
}

typedef void (*iTomlTableFunc)(void *context, const iString *table, iBool isStart);
typedef void (*iTomlKeyValueFunc)(void *context, const iString *table, const iString *key,
                                  const iTomlValue *value);

void    setHandlers_TomlParser   (iTomlParser *, iTomlTableFunc table, iTomlKeyValueFunc kv, void *);

iBool   parse_TomlParser         (iTomlParser *, const iString *toml); /* returns true if no errors found */

