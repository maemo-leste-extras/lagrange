/** @file string.c  UTF-8 string with copy-on-write semantics.

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

#include "the_Foundation/string.h"
#include "the_Foundation/stringlist.h"
#include "the_Foundation/range.h"
#include "the_Foundation/stdthreads.h"

#if defined (iHaveRegExp)
#   include "the_Foundation/regexp.h"
#endif

#include <stdlib.h>
#include <stdarg.h>
#include <strings.h>
#include <unicase.h>
#include <unictype.h>
#include <uniconv.h>
#include <uninorm.h>
#include <unistr.h>
#include <uniwidth.h>
#include <ctype.h>

#if !defined (iHaveStrnstr)
#   include "platform/strnstr.h"
#endif

static char localeCharSet_[64];

iLocalDef const char *currentLocaleLanguage_(void) {
    static iString *loc = NULL;
    if (!loc) {
        loc = newCStr_String(uc_locale_language());
    }
    return cstr_String(loc);
}

iChar upper_Char(iChar d) {
    return uc_toupper(d);
}

iChar lower_Char(iChar d) {
    return uc_tolower(d);
}

iBool isSpace_Char(iChar d) {
    return uc_is_space(d) ? iTrue : iFalse;
}

iBool isAlpha_Char(iChar d) {
    return uc_is_alpha(d) ? iTrue : iFalse;
}

iBool isNumeric_Char(iChar d) {
    return uc_is_digit(d) ? iTrue : iFalse;
}

iBool isAlphaNumeric_Char(iChar d) {
    return uc_is_alnum(d) ? iTrue : iFalse;
}

iBool isPunct_Char(iChar d) {
    return uc_is_punct(d) ? iTrue : iFalse;
}

const char *script_Char(iChar d) {
    const uc_script_t *scr = uc_script(d);
    return scr ? scr->name : "";
}

int width_Char(iChar d) {
    if (isEmoji_Char(d)) {
        return 2;
    }
    return uc_width(d, localeCharSet_);
}

void setLocaleCharSet_String(const char *charSet) {
    const size_t n = sizeof(localeCharSet_);
    strncpy(localeCharSet_, charSet, n);
    localeCharSet_[n - 1] = 0;
}

iString *new_String(void) {
    iString *d = iMalloc(String);
    init_String(d);
    return d;
}

iString *newCStr_String(const char *cstr) {
    return newCStrN_String(cstr, strlen(cstr));
}

iString *newCStrN_String(const char *cstr, size_t n) {
    iString *d = iMalloc(String);
    initData_Block(&d->chars, cstr, n);
    return d;
}

iString *newUtf16_String(const uint16_t *utf16Str) {
    iString *d = iMalloc(String);
    initUtf16_String(d, utf16Str);
    return d;
}

iString *newUtf16N_String(const uint16_t *utf16Str, size_t n) {
    iString *d = iMalloc(String);
    initUtf16N_String(d, utf16Str, n);
    return d;
}

iString *newLocalCStr_String(const char *localCStr) {
    return newLocalCStrN_String(localCStr, strlen(localCStr));
}

iString *newLocalCStrN_String(const char *localCStr, size_t n) {
    iString *d = iMalloc(String);
    initLocalCStrN_String(d, localCStr, n);
    return d;
}

iString *newUnicode_String(const iChar *ucs) {
    return newUnicodeN_String(ucs, u32_strlen(ucs));
}

iString *newUnicodeN_String(const iChar *ucs, size_t n) {
    iString *d = iMalloc(String);
    initUnicodeN_String(d, ucs, n);
    return d;
}

iString *newFormat_String(const char *format, ...) {
    iString *d = new_String();
    va_list args;
    va_start(args, format);
    vprintf_Block(&d->chars, format, args);
    va_end(args);
    return d;
}

iString *collectNewFormat_String(const char *format, ...) {
    iString *d = collectNew_String();
    va_list args;
    va_start(args, format);
    vprintf_Block(&d->chars, format, args);
    va_end(args);
    return d;
}

const char *format_CStr(const char *format, ...) {
    iString *d = collectNew_String();
    va_list args;
    va_start(args, format);
    vprintf_Block(&d->chars, format, args);
    va_end(args);
    return cstr_String(d);
}

iString *newBlock_String(const iBlock *data) {
    iString *d = iMalloc(String);
    initCopy_Block(&d->chars, data);
    return d;
}

iString *copy_String(const iString *d) {
    if (d) {
        iString *copy = iMalloc(String);
        initCopy_Block(&copy->chars, &d->chars);
        return copy;
    }
    return new_String();
}

void delete_String(iString *d) {
    if (d) {
        deinit_String(d);
        free(d);
    }
}

void init_String(iString *d) {
    init_Block(&d->chars, 0);
}

void initBlock_String(iString *d, const iBlock *chars) {
    initCopy_Block(&d->chars, chars);
}

void initCStr_String(iString *d, const char *cstr) {
    initCStrN_String(d, cstr, strlen(cstr));
}

void initCStrN_String(iString *d, const char *cstr, size_t size) {
    initData_Block(&d->chars, cstr, size);
}

void initUtf16_String(iString *d, const uint16_t *utf16Str) {
    initUtf16N_String(d, utf16Str, u16_strlen(utf16Str));
}

void initUtf16N_String(iString *d, const uint16_t *utf16Str, size_t n) {
    size_t len = 0;
    uint8_t *data = u16_to_u8(utf16Str, n, NULL, &len);
    data = realloc(data, len + 1);
    data[len] = 0; // terminate
    initPrealloc_Block(&d->chars, data, len, len + 1);
}

void initLocalCStr_String(iString *d, const char *localCStr) {
    initLocalCStrN_String(d, localCStr, strlen(localCStr));
}

void initLocalCStrN_String(iString *d, const char *localCStr, size_t size) {
    initBlockEncoding_String(d, &iBlockLiteral(localCStr, size, size), localeCharSet_);
}

void initBlockEncoding_String(iString *d, const iBlock *chars, const char *encoding) {
    size_t len = 0;
    uint8_t *data = u8_conv_from_encoding(encoding,
                                          iconveh_question_mark,
                                          constData_Block(chars),
                                          size_Block(chars),
                                          NULL,
                                          NULL,
                                          &len);
    data = realloc(data, len + 1);
    data[len] = 0;
    initPrealloc_Block(&d->chars, data, len, len + 1);
}

void initUnicode_String(iString *d, const iChar *ucs) {
    initUnicodeN_String(d, ucs, u32_strlen(ucs));
}

void initUnicodeN_String(iString *d, const iChar *ucs, size_t n) {
    size_t len = 0;
    uint8_t *str = u32_to_u8(ucs, n, NULL, &len);
    str = realloc(str, len + 1);
    str[len] = 0;
    iBlock chars;
    initPrealloc_Block(&chars, str, len, len + 1);
    initBlock_String(d, &chars);
    deinit_Block(&chars);
}

void initCopy_String(iString *d, const iString *other) {
    initCopy_Block(&d->chars, &other->chars);
}

void deinit_String(iString *d) {
    deinit_Block(&d->chars);
}

void serialize_String(const iString *d, iStream *outs) {
    serialize_Block(&d->chars, outs);
}

void deserialize_String(iString *d, iStream *ins) {
    deserialize_Block(&d->chars, ins);
}

void clear_String(iString *d) {
    clear_Block(&d->chars);
}

void truncate_String(iString *d, size_t charCount) {
    const char *start = constData_Block(&d->chars);
    const char *pos = start;
    iConstForEach(String, i, d) {
        if (charCount-- == 0) break;
        pos = i.next;
    }
    truncate_Block(&d->chars, (size_t) (pos - start));
}

void removeEnd_String(iString *d, size_t charCount) {
    if (charCount > 0) {
        const size_t len = length_String(d);
        if (charCount < len) {
            truncate_String(d, len - charCount);
        }
        else {
            clear_String(d);
        }
    }
}

void trimStart_String(iString *d) {
    if (!isEmpty_String(d)) {
        iRangecc range = range_String(d);
        const char *start = range.start;
        trimStart_Rangecc(&range);
        remove_Block(&d->chars, 0, (size_t) (range.start - start));
    }
}

void trimStart_Rangecc(iRangecc *d) {
    const uint8_t *pos = (const uint8_t *) d->start;
    while (pos != (const uint8_t *) d->end) {
        iChar ch;
        pos = u8_next(&ch, pos);
        /* Variation selectors follow the main codepoint, so if one is found at the beginning
           it should be ignored. */
        if (!isSpace_Char(ch) && !isVariationSelector_Char(ch)) break;
        d->start = (const char *) pos;
    }
}

void trimEnd_String(iString *d) {
    if (!isEmpty_String(d)) {
        iRangecc range = range_String(d);
        trimEnd_Rangecc(&range);
        truncate_Block(&d->chars, (size_t) (range.end - range.start));
    }
}

void trimEnd_Rangecc(iRangecc *d) {
    while (d->end != d->start) {
        iAssert(d->end > d->start);
        /* Skip over any extra NULL characters. */
        if (d->end[-1] == 0) {
            d->end--;
        }
        else {
            iChar ch = 0;
            const uint8_t *pos = u8_prev(&ch, (const uint8_t *) d->end, (const uint8_t *) d->start);
            if (!pos) {
                /* `pos` is NULL when beginning of the string is reached (but we're already
                   checking for that so the loop ends before that happens), or if there's
                   an invalid codepoint. We'll trim the invalid ones, too. */
                d->end--;
            }
            else if (isSpace_Char(ch)) {
                d->end = (const char *) pos;
            }
            else break;
        }
    }
}

void trim_Rangecc(iRangecc *d) {
    trimStart_Rangecc(d);
    trimEnd_Rangecc(d);
}

void trim_String(iString *d) {
    trimStart_String(d);
    trimEnd_String(d);
}

iString *trimmed_String(const iString *d) {
    iString *str = copy_String(d);
    trim_String(str);
    return str;
}

void replace_String(iString *d, const char *src, const char *dst) {
    const size_t srcLen = strlen(src);
    const size_t dstLen = strlen(dst);
    for (size_t pos = indexOfCStr_String(d, src); pos != iInvalidPos;
         pos = indexOfCStrFrom_String(d, src, pos)) {
        remove_Block(&d->chars, pos, srcLen);
        insertData_Block(&d->chars, pos, dst, dstLen);
        pos += dstLen;
    }
}

#if defined (iHaveRegExp)
int replaceRegExp_String(iString *d, const iRegExp *regexp, const char *replacement,
                         void (*matchHandler)(void *, const iRegExpMatch *),
                         void *context) {
    iRegExpMatch m;
    iString      result;
    int          numMatches = 0;
    const char  *pos        = constBegin_String(d);
    init_RegExpMatch(&m);
    init_String(&result);
    while (matchString_RegExp(regexp, d, &m)) {
        appendRange_String(&result, (iRangecc){ pos, begin_RegExpMatch(&m) });
        /* Replace any capture group back-references. */
        for (const char *ch = replacement; *ch; ch++) {
            if (*ch == '\\') {
                ch++;
                if (*ch == '\\') {
                    appendCStr_String(&result, "\\");
                }
                else if (*ch >= '0' && *ch <= '9') {
                    appendRange_String(&result, capturedRange_RegExpMatch(&m, *ch - '0'));
                }
            }
            else {
                appendData_Block(&result.chars, ch, 1);
            }
        }
        if (matchHandler) {
            matchHandler(context, &m);
        }
        pos = end_RegExpMatch(&m);
        numMatches++;
    }
    appendRange_String(&result, (iRangecc){ pos, constEnd_String(d) });
    set_String(d, &result);
    deinit_String(&result);
    return numMatches;
}
#endif

void normalize_String(iString *d) {
    size_t len = 0;
    uint8_t *nfc =
        u8_normalize(UNINORM_NFC, constData_Block(&d->chars), size_Block(&d->chars), NULL, &len);
    /* Ensure it's null-terminated. */
    nfc = realloc(nfc, len + 1);
    nfc[len] = 0;
    iBlock data;
    initPrealloc_Block(&data, nfc, len, len + 1);
    set_Block(&d->chars, &data);
    deinit_Block(&data);
}

const char *cstr_String(const iString *d) {
    return constData_Block(&d->chars);
}

size_t length_String(const iString *d) {
    return u8_mbsnlen((const uint8_t *) cstr_String(d), size_String(d));
}

iBool isUtf8_Rangecc(iRangecc d) {
    return u8_check((const uint8_t *) d.start, size_Range(&d)) == NULL;
}

size_t length_Rangecc(const iRangecc d) {
    /*
    size_t n = 0;
    for (const char *i = d.start; i < d.end; ) {
        iChar ch;
        const int chLen = decodeBytes_MultibyteChar(i, d.end, &ch);
        if (chLen <= 0) break;
        i += chLen;
        n++;
    }
    return n;*/
    return u8_mbsnlen((const uint8_t *) d.start, size_Range(&d));
}

size_t size_String(const iString *d) {
    return d ? size_Block(&d->chars) : 0;
}

iString *mid_String(const iString *d, size_t charStartPos, size_t charCount) {
    if (charCount == 0) return new_String();
    const char *chars = constData_Block(&d->chars);
    iRanges range = { 0, size_Block(&d->chars) };
    size_t pos = 0;
    iConstForEach(String, i, d) {
        if (pos > charStartPos && pos == charStartPos + charCount) {
            range.end = i.pos - chars;
            break;
        }
        else if (pos == charStartPos) {
            range.start = i.pos - chars;
            if (charCount == iInvalidSize) break;
        }
        pos++;
    }
    iBlock *midChars = midRange_Block(&d->chars, range);
    iString *mid = newBlock_String(midChars);
    delete_Block(midChars);
    return mid;
}

iString *upper_String(const iString *d) {
    return upperLang_String(d, currentLocaleLanguage_());
}
    
iString *upperLang_String(const iString *d, const char *langCode) {
    size_t len = 0;
    uint8_t *str = u8_toupper((const uint8_t *) cstr_String(d),
                              size_String(d),
                              langCode,
                              NULL,
                              NULL,
                              &len);
    str = realloc(str, len + 1);
    str[len] = 0;
    iBlock data;
    initPrealloc_Block(&data, str, len, len + 1);
    if (cmp_Block(&data, &d->chars) == 0) {
        /* Memory optimization: nothing changed so just use a reference. */
        deinit_Block(&data);
        return copy_String(d);
    }
    iString *up = newBlock_String(&data);
    deinit_Block(&data);
    return up;
}

iString *lower_String(const iString *d) {
    return lowerLang_String(d, currentLocaleLanguage_());
}

iString *lowerLang_String(const iString *d, const char *langCode) {
    size_t len = 0;
    uint8_t *str = u8_tolower((const uint8_t *) cstr_String(d),
                              size_String(d),
                              langCode,
                              NULL,
                              NULL,
                              &len);
    str = realloc(str, len + 1);
    str[len] = 0;
    iBlock data;
    initPrealloc_Block(&data, str, len, len + 1);
    if (cmp_Block(&data, &d->chars) == 0) {
        /* Memory optimization: nothing changed so just use a reference. */
        deinit_Block(&data);
        return copy_String(d);
    }
    iString *lwr = newBlock_String(&data);
    deinit_Block(&data);
    return lwr;
}

iStringList *split_String(const iString *d, const char *separator) {
    const iRangecc range = range_String(d);
    return split_Rangecc(range, separator);
}

iString *urlEncodeExclude_String(const iString *d, const char *excluded) {
    iString *enc = maybeUrlEncodeExclude_String(d, excluded);
    return enc ? enc : copy_String(d);
}

iString *urlEncode_String(const iString *d) {
    return urlEncodeExclude_String(d, "");
}

iString *maybeUrlEncodeExclude_String(const iString *d, const char *excluded) {
    /* TODO: Return NULL if nothing to encode. */
    iString *encoded = new_String();
    /* Note: Any UTF-8 code points are encoded as multiple %NN sequences. */
    for (const char *i = constBegin_String(d), *end = constEnd_String(d); i != end; ++i) {
        char ch = *i;
        if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') ||
            ch == '-' || ch == '_' || ch == '.' || ch == '~' || strchr(excluded, ch)) {
            appendData_Block(&encoded->chars, i, 1);
        }
        else {
            static const char hex[16] = {
                '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
            char escaped[3] = {'%', hex[(ch >> 4) & 0xf], hex[ch & 0xf]};
            appendCStrN_String(encoded, escaped, 3);
        }
    }
    return encoded;
}

static int fromHex_(char ch) {
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'A' && ch <= 'F') return 10 + ch - 'A';
    if (ch >= 'a' && ch <= 'f') return 10 + ch - 'a';
    return -1;
}

iString *urlDecode_String(const iString *d) {
    return urlDecodeExclude_String(d, "");
}

iString *maybeUrlDecodeExclude_String(const iString *d, const char *excluded) {
    if (indexOf_String(d, '%') == iInvalidPos) {
        return NULL;
    }
    iString *decoded = new_String();
    for (const char *i = constBegin_String(d), *end = constEnd_String(d); i != end; ++i) {
        if (*i == '%' && i + 3 <= end) {
            const int values[2] = { fromHex_(i[1]), fromHex_(i[2]) };
            if (values[0] >= 0 && values[1] >= 0) {
                const char ch = (char) ((values[0] << 4) | values[1]);
                if (!strchr(excluded, ch)) {
                    appendData_Block(&decoded->chars, &ch, 1);
                    i += 2;
                    continue;
                }
            }
        }
        appendData_Block(&decoded->chars, i, 1);
    }
    return decoded;
}

iString *urlDecodeExclude_String(const iString *d, const char *excluded) {
    iString *dec = maybeUrlDecodeExclude_String(d, excluded);
    return dec ? dec : copy_String(d);
}

iChar first_String(const iString *d) {
    iStringConstIterator iter;
    init_StringConstIterator(&iter, d);
    return iter.value;
}

iChar last_String(const iString *d) {
    iStringReverseConstIterator iter;
    init_StringReverseConstIterator(&iter, d);
    return iter.value;
}

iBlock *toLocal_String(const iString *d) {
    size_t len = 0;
    char * str = u8_conv_to_encoding(localeCharSet_,
                                     iconveh_question_mark,
                                     (const uint8_t *) cstr_String(d),
                                     size_String(d),
                                     NULL,
                                     NULL,
                                     &len);
    str = realloc(str, len + 1);
    str[len] = 0;
    return newPrealloc_Block(str, len, len + 1);
}

iBlock *toUtf16_String(const iString *d) {
    size_t len = 0;
    uint16_t *u16 = u8_to_u16((const uint8_t *) cstr_String(d),
                              size_String(d),
                              NULL,
                              &len);
    /* Make it null-terminated. */
    const size_t bytes = 2 * len;
    u16 = realloc(u16, bytes + 2);
    u16[len] = 0;
    return newPrealloc_Block(u16, bytes, bytes + 2);
}

iBlock *toUnicode_String(const iString *d) {
    size_t len = 0;
    uint32_t *u32 = u8_to_u32((const uint8_t *) cstr_String(d),
                              size_String(d),
                              NULL,
                              &len);
    /* Make it null-terminated. */
    const size_t bytes = 4 * len;
    u32 = realloc(u32, bytes + 4);
    u32[len] = 0;
    return newPrealloc_Block(u32, bytes, bytes + 4);
}

int cmpSc_String(const iString *d, const char *cstr, const iStringComparison *sc) {
    return sc->cmp(constData_Block(&d->chars), cstr);
}

int cmpNSc_String(const iString *d, const char *cstr, size_t n, const iStringComparison *sc) {
    return sc->cmpN(constData_Block(&d->chars), cstr, n);
}

iBool startsWithSc_String(const iString *d, const char *cstr, const iStringComparison *sc) {
    const iRangecc rc = range_String(d);
    return startsWithSc_Rangecc(rc, cstr, sc);
}

iBool startsWithSc_Rangecc(const iRangecc d, const char *cstr, const iStringComparison *sc) {
    const size_t len = strlen(cstr);
    if (size_Range(&d) < len) return iFalse;
    return !sc->cmpN(d.start, cstr, len);
}

iBool endsWithSc_Rangecc(const iRangecc d, const char *cstr, const iStringComparison *sc) {
    const size_t len = strlen(cstr);
    if (size_Range(&d) < len) return iFalse;
    return !sc->cmpN(d.end - len, cstr, len);
}

iBool endsWithSc_String(const iString *d, const char *cstr, const iStringComparison *sc) {
    const size_t len = strlen(cstr);
    if (size_String(d) < len) return iFalse;
    return !sc->cmp(constEnd_Block(&d->chars) - len, cstr);
}

void set_String(iString *d, const iString *other) {
    set_Block(&d->chars, &other->chars);
}

void setCStr_String(iString *d, const char *cstr) {
    setCStr_Block(&d->chars, cstr);
}

void setCStrN_String(iString *d, const char *cstr, size_t n) {
    setData_Block(&d->chars, cstr, n);
}

void setBlock_String(iString *d, const iBlock *block) {
    set_Block(&d->chars, block);
}

void format_String(iString *d, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf_Block(&d->chars, format, args);
    va_end(args);
}

void appendFormat_String(iString *d, const char *format, ...) {
    iBlock chars;
    init_Block(&chars, 0); {
        va_list args;
        va_start(args, format);
        vprintf_Block(&chars, format, args);
        va_end(args);
    }
    append_Block(&d->chars, &chars);
    deinit_Block(&chars);
}

size_t indexOf_String(const iString *d, iChar ch) {
    iMultibyteChar mb;
    init_MultibyteChar(&mb, ch);
    return indexOfCStr_String(d, mb.bytes);
}

size_t indexOfCStr_String(const iString *d, const char *cstr) {
    return indexOfCStrFromSc_String(d, cstr, 0, &iCaseSensitive);
}

size_t indexOfCStrFrom_String(const iString *d, const char *cstr, size_t from) {
    return indexOfCStrFromSc_String(d, cstr, from, &iCaseSensitive);
}

size_t indexOfCStrSc_String(const iString *d, const char *cstr, const iStringComparison *sc) {
    return indexOfCStrFromSc_String(d, cstr, 0, sc);
}

size_t indexOfCStrFromSc_String(const iString *d, const char *cstr, size_t from,
                                const iStringComparison *sc) {
    if (from >= size_String(d)) return iInvalidPos;
    const char *chars = cstr_String(d) + from;
    const char *found = sc->locate(chars, cstr);
    if (found) {
        return found - chars + from;
    }
    return iInvalidPos;
}

size_t lastIndexOf_String(const iString *d, iChar ch) {
    iMultibyteChar mb;
    init_MultibyteChar(&mb, ch);
    return lastIndexOfCStr_String(d, mb.bytes);
}

size_t lastIndexOfCStr_Rangecc(const iRangecc d, const char *cstr) {
    const size_t len = strlen(cstr);
    if (len > size_Range(&d)) return iInvalidPos;
    for (const char *i = d.end - len; i >= d.start; --i) {
        if (iCmpStrN(i, cstr, len) == 0) {
            return i - d.start;
        }
    }
    return iInvalidPos;
}

size_t lastIndexOfCStr_String(const iString *d, const char *cstr) {
    return lastIndexOfCStr_Rangecc((iRangecc){ constBegin_String(d), constEnd_String(d) }, cstr);
}

void append_String(iString *d, const iString *other) {
    append_Block(&d->chars, &other->chars);
}

void appendCStr_String(iString *d, const char *cstr) {
    appendCStr_Block(&d->chars, cstr);
}

void appendCStrN_String(iString *d, const char *cstr, size_t size) {
    appendData_Block(&d->chars, cstr, size);
}

void appendChar_String(iString *d, iChar ch) {
    iMultibyteChar mb;
    init_MultibyteChar(&mb, ch);
    appendCStr_String(d, mb.bytes);
}

void appendRange_String(iString *d, const iRangecc range) {
    appendData_Block(&d->chars, range.start, size_Range(&range));
}

void prepend_String(iString *d, const iString *other) {
    iString pre;
    initCopy_String(&pre, other);
    append_String(&pre, d);
    set_String(d, &pre);
    deinit_String(&pre);
}

void prependChar_String(iString *d, iChar ch) {
    iMultibyteChar mb;
    init_MultibyteChar(&mb, ch);
    insertData_Block(&d->chars, 0, mb.bytes, strlen(mb.bytes));
}

void prependCStr_String(iString *d, const char *cstr) {
    iString pre;
    initCStr_String(&pre, cstr);
    append_String(&pre, d);
    set_String(d, &pre);
    deinit_String(&pre);
}

iBool nextSplit_Rangecc(const iRangecc str, const char *separator, iRangecc *range) {
    iAssert(range->start == NULL || contains_Range(&str, range->start));
    const size_t separatorSize = strlen(separator);
    iAssert(separatorSize > 0);
    if (range->start == NULL) {
        if (separatorSize > size_Range(&str)) {
            /* Doesn't fit in the string. */
            return iFalse;
        }
        if (!cmpCStrSc_Rangecc(str, separator, &iCaseSensitive)) {
            return iFalse;
        }
        range->start = range->end = str.start;
        if (!iCmpStrN(range->start, separator, separatorSize)) {
            /* Skip the first separator. */
            range->start += separatorSize;
        }
    }
    else if (range->start == str.end) {
        return iFalse;
    }
    else {
        range->start = range->end + separatorSize;
        if (range->start >= str.end) {
            return iFalse;
        }
    }
    const char *found = strstr(range->start, separator);
    range->end = (found && found < str.end ? found : str.end);
    iAssert(range->start <= range->end);
    return iTrue;
}

const char *cstr_Rangecc(iRangecc range) {
    const size_t len  = size_Range(&range);
    char *       copy = malloc(len + 1);
    memcpy(copy, range.start, len);
    copy[len] = 0;
    return iCollectMem(copy);
}

const iString *string_Rangecc(iRangecc range) {
    return collect_String(newRange_String(range));
}

iLocalDef int cmpNullRange_(const char *cstr) {
    return (cstr == NULL || *cstr == 0 ? 0 : -1);
}

int cmpCStrSc_Rangecc(const iRangecc d, const char *cstr, const iStringComparison *sc) {
    if (isNull_Rangecc(d)) {
        return cmpNullRange_(cstr);
    }
    return cmpCStrNSc_Rangecc(d, cstr, strlen(cstr), sc);
}

int cmpCStrNSc_Rangecc(const iRangecc d, const char *cstr, size_t n, const iStringComparison *sc) {
    if (isNull_Rangecc(d)) {
        return cmpNullRange_(cstr);
    }
    const size_t size = size_Range(&d);
    int cmp = sc->cmpN(d.start, cstr, iMin(n, size));
    if (cmp == 0) {
        if (n == size) {
            return 0;
        }
        return size < n ? -1 : 1;
    }
    return cmp;
}

iStringList *split_Rangecc(const iRangecc d, const char *separator) {
    iStringList *parts = new_StringList();
    iRangecc range = iNullRange;
    while (nextSplit_Rangecc(d, separator, &range)) {
        pushBackRange_StringList(parts, range);
    }
    return parts;
}

int toInt_String(const iString *d) {
    if (startsWith_String(d, "0x") || startsWith_String(d, "0X")) {
        return (int) strtol(cstr_String(d), NULL, 16);
    }
    return atoi(cstr_String(d));
}

float toFloat_String(const iString *d) {
    return strtof(cstr_String(d), NULL);
}

double toDouble_String(const iString *d) {
    return strtod(cstr_String(d), NULL);
}

iString *quote_String(const iString *d, iBool numericUnicode) {
    iString *quot = new_String();
    iConstForEach(String, i, d) {
        const iChar ch = i.value;
        if (ch == '"') {
            appendCStr_String(quot, "\\\"");
        }
        else if (ch == '\\') {
            appendCStr_String(quot, "\\\\");
        }
        else if (ch == '\n') {
            appendCStr_String(quot, "\\n");
        }
        else if (ch == '\r') {
            appendCStr_String(quot, "\\r");
        }
        else if (ch == '\t') {
            appendCStr_String(quot, "\\t");
        }
        else if (numericUnicode && ch >= 0x80) {
            if ((ch >= 0xD800 && ch < 0xE000) || ch >= 0x10000) {
                /* TODO: Add a helper function? */
                /* UTF-16 surrogate pair */
                iString *chs = newUnicodeN_String(&ch, 1);
                iBlock *u16 = toUtf16_String(chs);
                delete_String(chs);
                const uint16_t *ch16 = constData_Block(u16);
                appendFormat_String(quot, "\\u%04x\\u%04x", ch16[0], ch16[1]);
            }
            else {
                appendFormat_String(quot, "\\u%04x", ch);
            }
        }
        else {
            appendChar_String(quot, ch);
        }
    }
    return quot;
}

iString *unquote_String(const iString *d) {
    iString *unquot = new_String();
    iConstForEach(String, i, d) {
        const iChar ch = i.value;
        if (ch == '\\') {
            next_StringConstIterator(&i);
            const iChar esc = i.value;
            if (esc == '\\') {
                appendChar_String(unquot, esc);
            }
            else if (esc == 'n') {
                appendChar_String(unquot, '\n');
            }
            else if (esc == 'r') {
                appendChar_String(unquot, '\r');
            }
            else if (esc == 't') {
                appendChar_String(unquot, '\t');
            }
            else if (esc == '"') {
                appendChar_String(unquot, '"');
            }
            else if (esc == 'u') {
                char digits[5];
                iZap(digits);
                for (size_t j = 0; j < 4; j++) {
                    next_StringConstIterator(&i);
                    digits[j] = *i.pos;
                }
                uint16_t ch16[2] = { strtoul(digits, NULL, 16), 0 };
                if (ch16[0] < 0xD800 || ch16[0] >= 0xE000) {
                    appendChar_String(unquot, ch16[0]);
                }
                else {
                    /* UTF-16 surrogate pair */
                    next_StringConstIterator(&i);
                    next_StringConstIterator(&i);
                    iZap(digits);
                    for (size_t j = 0; j < 4; j++) {
                        next_StringConstIterator(&i);
                        digits[j] = *i.pos;
                    }
                    ch16[1] = strtoul(digits, NULL, 16);
                    iString *u16 = newUtf16N_String(ch16, 2);
                    append_String(unquot, u16);
                    delete_String(u16);
                }
            }
            else {
                iAssert(0);
            }
        }
        else {
            appendChar_String(unquot, ch);
        }
    }
    return unquot;
}

const char *skipSpace_CStr(const char *cstr) {
    while (*cstr && isspace((int) *cstr)) {
        cstr++;
    }
    return cstr;
}

const char *findAscii_Rangecc(const iRangecc str, char ch) {
    const char *pos = strchr(str.start, ch);
    if (!pos || pos >= str.end) return NULL;
    return pos;
}

iStringList *split_CStr(const char *cstr, const char *separator) {
    return split_Rangecc((iRangecc){ cstr, cstr + strlen(cstr) }, separator);
}

/*-------------------------------------------------------------------------------------*/

static void decodeNextMultibyte_StringConstIterator_(iStringConstIterator *d) {
    d->value = 0;
    /* u8_next() returns NULL when end is reached. */
    d->next = (const char *) u8_next(&d->value, (const uint8_t *) d->next);
}

static void decodePrecedingMultibyte_StringConstIterator_(iStringConstIterator *d) {
    d->value = 0;
    d->next = (const char *) u8_prev(
        &d->value, (const uint8_t *) d->next, constData_Block(&d->str->chars));
}

void init_StringConstIterator(iStringConstIterator *d, const iString *str) {
    d->str = str;
    d->value = 0;
    if (str) {
        d->pos = d->next = constData_Block(&str->chars);
        /* Decode the first character. */
        decodeNextMultibyte_StringConstIterator_(d);
    }
    else {
        d->pos = d->next = NULL;
    }
}

void next_StringConstIterator(iStringConstIterator *d) {
    d->pos = d->next;
    decodeNextMultibyte_StringConstIterator_(d);
}

void init_StringReverseConstIterator(iStringConstIterator *d, const iString *str) {
    d->str = str;
    d->value = 0;
    d->pos = d->next = constEnd_Block(&str->chars);
    /* Decode the first (last) character. */
    decodePrecedingMultibyte_StringConstIterator_(d);
}

void next_StringReverseConstIterator(iStringConstIterator *d) {
    d->pos = d->next;
    decodePrecedingMultibyte_StringConstIterator_(d);
}

/*-------------------------------------------------------------------------------------*/

void init_MultibyteChar(iMultibyteChar *d, iChar ch) {
    int len = u8_uctomb((uint8_t *) d->bytes, ch, sizeof(d->bytes));
    d->bytes[iMax(0, len)] = 0;
}

int decodeBytes_MultibyteChar(const char *bytes, const char *end, iChar *ch_out) {
    int rc = u8_mbtouc(ch_out, (const uint8_t *) bytes, end - bytes);
    if (*ch_out == 0xfffd) {
        rc = -1; /* Decode failed. */
    }
    return rc;
}

int decodePrecedingBytes_MultibyteChar(const char *bytes, const char *start, iChar *ch_out) {
    *ch_out = 0;
    const char *precPos =
        (const char *) u8_prev(ch_out, (const uint8_t *) bytes, (const uint8_t *) start);
    if (!precPos) {
        return 0;
    }
    return (int) (bytes - precPos);
}

static char *threadLocalCharBuffer_(void) {
    static tss_t bufKey = 0;
    if (!bufKey) {
        tss_create(&bufKey, free);
    }
    char *buf = tss_get(bufKey);
    if (!buf) {
        tss_set(bufKey, buf = malloc(iMultibyteCharMaxSize + 1));
    }
    return buf;
}

const char *cstrLocal_Char(iChar ch) {
    char *chBuf = threadLocalCharBuffer_();
    const iChar ucs[2] = { ch, 0 };
    size_t len = iMultibyteCharMaxSize;
    u32_conv_to_encoding(localeCharSet_, iconveh_question_mark, ucs, 1, NULL, chBuf, &len);
    chBuf[len] = 0;
    return chBuf;
}

int iCmpStrRange(const iRangecc range, const char *cstr) {
    const size_t clen = strlen(cstr);
    const int cmp = iCmpStrN(range.start, cstr, size_Range(&range));
    if (clen == size_Range(&range)) {
        return cmp;
    }
    if (cmp == 0) return (size_Range(&range) < clen? -1 : 1);
    return cmp;
}

int iCmpStrCase(const char *a, const char *b) {
    int rc = 0;
    u8_casecmp((const uint8_t *) a,
               strlen(a),
               (const uint8_t *) b,
               strlen(b),
               currentLocaleLanguage_(),
               NULL,
               &rc);
    return rc;
}

int iCmpStrNCase(const char *a, const char *b, size_t len) {
    int rc = 0;
    u8_casecmp((const uint8_t *) a,
               strnlen(a, len),
               (const uint8_t *) b,
               strnlen(b, len),
               currentLocaleLanguage_(),
               NULL,
               &rc);
    return rc;
}

static char *strcasestr_(const char *haystack, const char *needle) {
    const iString hay = iStringLiteral(haystack);
    const iString ndl = iStringLiteral(needle);
    const iChar ndlFirstChar = lower_Char(first_String(&ndl));
    if (size_String(&ndl) > size_String(&hay)) {
        /* Too long to be able to find it. */
        return NULL;
    }
    iConstForEach(String, i, &hay) {
        if (lower_Char(i.value) == ndlFirstChar) {
            /* Check if the full needle matches. */
            iStringConstIterator hayStart;
            memcpy(&hayStart, &i, sizeof(i));
            iStringConstIterator j;
            init_StringConstIterator(&j, &ndl);
            for (;;) {
                next_StringConstIterator(&j);
                next_StringConstIterator(&i);
                if (!j.value) return iConstCast(char *, hayStart.pos); // Matched full needle.
                if (!i.value) return NULL; // Not long enough for needle.
                if (lower_Char(i.value) != lower_Char(j.value)) {
                    /* Must match all need characters. */
                    break;
                }
            }
            memcpy(&i, &hayStart, sizeof(i));
        }
    }
    return NULL;
}

int iCmpStr(const char *a, const char *b) {
    return u8_strcmp((const uint8_t *) a, (const uint8_t *) b);
}

int iCmpStrN(const char *a, const char *b, size_t n) {
    const size_t n1 = strnlen(a, n);
    const size_t n2 = strnlen(b, n);
    return u8_cmp2((const uint8_t *) a, n1, (const uint8_t *) b, n2);
}

iStringComparison iCaseSensitive = {
    .cmp    = iCmpStr,
    .cmpN   = iCmpStrN,
    .locate = strstr,
};

iStringComparison iCaseInsensitive = {
    .cmp    = iCmpStrCase,
    .cmpN   = iCmpStrNCase,
    .locate = strcasestr_,
};

char *iDupStr(const char *a) {
    return strdup(a);
}

char *iStrStrN(const char *a, const char *b, size_t n) {
    return strnstr(a, b, n);
}
