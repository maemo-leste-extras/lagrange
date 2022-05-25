/** @file regexp.c  Perl-compatible regular expressions.

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

#include "the_Foundation/regexp.h"
#include "the_Foundation/string.h"
#include "the_Foundation/range.h"
#include "the_Foundation/object.h"

#if !defined (iHavePcre)
#   error libpcre is required for regular expressions
#endif

#include <stdio.h>
#include <pcre.h>

iBool isSyntaxChar_RegExp(iChar ch) {
    return strchr("|()[]{}.\\", ch) != NULL;
}

struct Impl_RegExp {
    iObject object;
    pcre *re;
};

iRegExp *new_RegExp(const char *pattern, enum iRegExpOption options) {
    int opts = PCRE_UTF8 | PCRE_UCP;
    if (options & caseInsensitive_RegExpOption) {
        opts |= PCRE_CASELESS;
    }
    if (options & multiLine_RegExpOption) {
        opts |= PCRE_MULTILINE;
    }
    const char *errorMsg = NULL;
    int errorOffset;
    iRegExp *d = iNew(RegExp);
    d->re = pcre_compile(pattern, opts, &errorMsg, &errorOffset, NULL);
    if (!d->re) {
        iDebug("new_RegExp: \"%s\" %s (at offset %i)\n", pattern, errorMsg, errorOffset);
    }
    return d;
}

void deinit_RegExp(iRegExp *d) {
    if (d->re) {
        pcre_free(d->re);
    }
}

iBool match_RegExp(const iRegExp *d, const char *subject, size_t len, iRegExpMatch *match) {
    if (!d->re || !subject) return iFalse;
    if (!match->subject) {
        /* The match object is uninitialized, so initialize it now. */
        match->subject = subject;        
    }
    iAssert(match->subject == subject); /* first or subsequent match */
    if (match->pos > len) return iFalse;
    int rc = pcre_exec(d->re, NULL,
                       subject, (int) len,
                       (int) match->pos, 0,
                       &match->range.start,
                       iRegExpMaxSubstrings + 1);
    if (rc > 0) {
        match->pos = match->range.end;
        return iTrue;
    }
    return iFalse;
}

void init_RegExpMatch(iRegExpMatch *d) {
    iZap(*d);
}

iString *captured_RegExpMatch(const iRegExpMatch *d, int index) {
    return newRange_String(capturedRange_RegExpMatch(d, index));
}

iRangecc capturedRange_RegExpMatch(const iRegExpMatch *d, int index) {
    iAssert(index <= iRegExpMaxSubstrings);
    const iRangei *cap = &d->range + index; // Full range at index zero.
    return (iRangecc){ d->subject + cap->start, d->subject + cap->end };
}

iDefineClass(RegExp)
