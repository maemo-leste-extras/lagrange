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

#if !defined (iHavePcre) && !defined (iHavePcre2)
#   error libpcre or libpcre2 is required for regular expressions
#endif

#include <stdio.h>

iBool isSyntaxChar_RegExp(iChar ch) {
    return strchr("|()[]{}.\\", ch) != NULL;
}

#if defined (iHavePcre2) /* PCRE version 10 */

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

struct Impl_RegExp {
    iObject object;
    pcre2_code *re;
};

iRegExp *new_RegExp(const char *pattern, enum iRegExpOption options) {
    uint32_t opts = PCRE2_UTF | PCRE2_UCP | PCRE2_NO_UTF_CHECK;
    if (options & caseInsensitive_RegExpOption) {
        opts |= PCRE2_CASELESS;
    }
    if (options & multiLine_RegExpOption) {
        opts |= PCRE2_MULTILINE;
    }
    int errorCode = 0;
    PCRE2_SIZE errorOffset = 0;
    iRegExp *d = iNew(RegExp);
    d->re = pcre2_compile((PCRE2_SPTR) pattern, PCRE2_ZERO_TERMINATED, opts,
                          &errorCode, &errorOffset, NULL);
    if (!d->re) {
        PCRE2_UCHAR errorMsg[256];
        pcre2_get_error_message(errorCode, errorMsg, sizeof(errorMsg));
        iDebug("new_RegExp: \"%s\" %s (at offset %i)\n", pattern, errorMsg, errorOffset);
    }
    return d;
}

void deinit_RegExp(iRegExp *d) {
    if (d->re) {
        pcre2_code_free(d->re);
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
    pcre2_match_data *matchData = pcre2_match_data_create_from_pattern(d->re, NULL);
    int rc = pcre2_match(d->re, (PCRE2_SPTR) subject, (PCRE2_SIZE) len, (PCRE2_SIZE) match->pos,
                         0, matchData, NULL);
    if (rc >= 1) {
        PCRE2_SIZE *output = pcre2_get_ovector_pointer(matchData);
        uint32_t count = pcre2_get_ovector_count(matchData);
        iRangei *mrange = &match->range;
        for (uint32_t i = 0; i < count && i <= iRegExpMaxSubstrings; i++, mrange++) {
            mrange->start = (int) output[2 * i];
            mrange->end   = (int) output[2 * i + 1];
        }
        match->pos = match->range.end;
    }
    pcre2_match_data_free(matchData);
    return rc >= 1;
}

#else /* PCRE version 8 */

#include <pcre.h>

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

#endif /* defined (iHavePcre) */

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
