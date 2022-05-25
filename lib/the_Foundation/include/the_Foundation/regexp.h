#pragma once

/** @file the_Foundation/regexp.h  Perl-compatible regular expressions.

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

#include "defs.h"
#include "range.h"
#include "class.h"
#include "string.h"

iBeginPublic

iDeclareClass(RegExp)

iDeclareType(RegExpMatch)

#define iRegExpMaxSubstrings  32

enum iRegExpOption {
    caseSensitive_RegExpOption      = 0,
    caseInsensitive_RegExpOption    = 0x1,
    multiLine_RegExpOption          = 0x2,
};

iBool   isSyntaxChar_RegExp (iChar);

/**
 * Compiles a regular expression.
 *
 * Avoid repeatedly compiling and releasing RegExp objects: always compile first and then
 * use multiple times.
 *
 * @param pattern  Regular expression.
 * @param options  Options affecting the matching behavior.
 *
 * @return RegExp object.
 *
 * @see [Perl compatible regular expressions](https://en.wikipedia.org/wiki/Perl_Compatible_Regular_Expressions)
 */
iRegExp *   new_RegExp(const char *pattern, enum iRegExpOption options);

void        deinit_RegExp(iRegExp *);

iBool       match_RegExp(const iRegExp *, const char *subject, size_t len, iRegExpMatch *match);

iLocalDef iBool matchString_RegExp(const iRegExp *d, const iString *str, iRegExpMatch *match) {
    return match_RegExp(d, cstr_String(str), size_String(str), match);
}
iLocalDef iBool matchRange_RegExp(const iRegExp *d, iRangecc subject, iRegExpMatch *match) {
    return match_RegExp(d, subject.start, size_Range(&subject), match);
}

/*----------------------------------------------------------------------------------------------*/

struct Impl_RegExpMatch {
    const char *subject;
    size_t      pos;
    iRangei     range;
    iRangei     substring[iRegExpMaxSubstrings];
    int         data_[iRegExpMaxSubstrings + 1];
};

void        init_RegExpMatch            (iRegExpMatch *);

iString *   captured_RegExpMatch        (const iRegExpMatch *, int index);
iRangecc    capturedRange_RegExpMatch   (const iRegExpMatch *, int index);

iLocalDef const char *  begin_RegExpMatch   (const iRegExpMatch *d) { return d->subject + d->range.start; }
iLocalDef const char *  end_RegExpMatch     (const iRegExpMatch *d) { return d->subject + d->range.end; }

iEndPublic
