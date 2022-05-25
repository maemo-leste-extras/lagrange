/** @file xml.c  Minimal non-validating XML parser.

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

#include "the_Foundation/xml.h"

iDeclareType(XmlParser)

enum iXmlToken {
    none_XmlToken,
    headerOpen_XmlToken,  /* <? */
    headerClose_XmlToken, /* ?> */
    open_XmlToken,        /* <  */
    openSlash_XmlToken,   /* </ */
    close_XmlToken,       /* >  */
    closeSlash_XmlToken,  /* /> */
    name_XmlToken,
    assignment_XmlToken,
    stringLiteral_XmlToken,
    content_XmlToken,
};

#if 0
static const char *tokenTypeStr_[] = {
    "headerOpen",
    "headerClose",
    "open",
    "openSlash",
    "close",
    "closeSlash",
    "name",
    "assignment",
    "stringLiteral",
    "content",
};
#endif

struct Impl_XmlParser {
    iXmlDocument *doc;
    const iString *src;
    iRangecc token;
    iBool inTag;
    enum iXmlToken tokenType;
    iStringConstIterator iter;
};

static void init_XmlParser_(iXmlParser *d, iXmlDocument *doc, const iString *source) {
    d->doc = doc;
    d->src = source;
    d->token = iNullRange;
    d->inTag = iFalse;
    d->tokenType = none_XmlToken;
    init_StringConstIterator(&d->iter, source);
}

iLocalDef iBool advance_XmlParser_(iXmlParser *d) {
    if (!d->iter.value) {
        return iFalse;
    }
    next_StringConstIterator(&d->iter);
    return iTrue;
}

static void skipSpace_XmlParser_(iXmlParser *d) {
    while (d->iter.value) {
        if (isSpace_Char(d->iter.value)) {
            advance_XmlParser_(d);
            continue;
        }
        if (!iCmpStrN(d->iter.pos, "<!--", 4)) {
            while (advance_XmlParser_(d)) {
                if (!iCmpStrN(d->iter.pos, "-->", 3)) {
                    advance_XmlParser_(d);
                    advance_XmlParser_(d);
                    advance_XmlParser_(d);
                    break;
                }
            }
            continue;
        }
        break;
    }
}

static iBool isFirstNameChar_(iChar c) {
    /* TODO: XML spec has the list of chars to allow. */
    return isAlpha_Char(c);
}

static iBool isNameChar_(iChar c) {
    return isAlphaNumeric_Char(c) || c == ':';
}

static iBool nextToken_XmlParser_(iXmlParser *d) {
    d->tokenType = none_XmlToken;
    skipSpace_XmlParser_(d);
    if (!d->iter.value) {
        return iFalse; /* Nothing more to parse. */
    }
    d->token.start = d->iter.pos;
    if (!d->inTag) {
        if (!iCmpStrN(d->iter.pos, "<?", 2)) {
            d->tokenType = headerOpen_XmlToken;
            advance_XmlParser_(d);
            advance_XmlParser_(d);
            d->token.end = d->iter.pos;
            d->inTag = iTrue;
            return iTrue;
        }
        else if (!iCmpStrN(d->iter.pos, "</", 2)) {
            d->tokenType = openSlash_XmlToken;
            advance_XmlParser_(d);
            advance_XmlParser_(d);
            d->token.end = d->iter.pos;
            d->inTag = iTrue;
            return iTrue;
        }
        else if (d->iter.value == '<' && iCmpStrN(d->iter.pos, "<![CDATA[", 9)) {
            d->tokenType = open_XmlToken;
            advance_XmlParser_(d);
            d->token.end = d->iter.pos;
            d->inTag = iTrue;
            return iTrue;
        }
        d->tokenType = content_XmlToken;
        while (d->iter.value) {
            if (!iCmpStrN(d->iter.pos, "<!--", 4)) {
                skipSpace_XmlParser_(d);
                continue;
            }
            else if (!iCmpStrN(d->iter.pos, "<![CDATA[", 9)) {
                while (advance_XmlParser_(d)) {
                    if (!iCmpStrN(d->iter.pos, "]]>", 3)) {
                        advance_XmlParser_(d);
                        advance_XmlParser_(d);
                        advance_XmlParser_(d);
                        break;
                    }
                }
                continue;
            }
            else if (d->iter.value == '<') {
                break;
            }
            advance_XmlParser_(d);
        }
        d->token.end = d->iter.pos;
        return iTrue;
    }
    else {
        if (!iCmpStrN(d->iter.pos, "?>", 2)) {
            d->tokenType = headerClose_XmlToken;
            advance_XmlParser_(d);
            advance_XmlParser_(d);
            d->token.end = d->iter.pos;
            d->inTag = iFalse;
            return iTrue;
        }
        else if (!iCmpStrN(d->iter.pos, "/>", 2)) {
            d->tokenType = closeSlash_XmlToken;
            advance_XmlParser_(d);
            advance_XmlParser_(d);
            d->token.end = d->iter.pos;
            d->inTag = iFalse;
            return iTrue;
        }
        else if (d->iter.value == '>') {
            d->tokenType = close_XmlToken;
            advance_XmlParser_(d);
            d->token.end = d->iter.pos;
            d->inTag = iFalse;
            return iTrue;
        }
        else if (isFirstNameChar_(d->iter.value)) {
            d->tokenType = name_XmlToken;
            advance_XmlParser_(d);
            while (isNameChar_(d->iter.value) && advance_XmlParser_(d)) {}
            d->token.end = d->iter.pos;
            return iTrue;
        }
        else if (d->iter.value == '=') {
            d->tokenType = assignment_XmlToken;
            advance_XmlParser_(d);
            d->token.end = d->iter.pos;
            return iTrue;
        }
        else if (d->iter.value == '\'' || d->iter.value == '"') {
            d->tokenType = stringLiteral_XmlToken;
            d->token.start++; /* omit the quotes */
            const iChar delim = d->iter.value;
            advance_XmlParser_(d);
            while (d->iter.value != delim && advance_XmlParser_(d)) {}
            d->token.end = d->iter.pos;
            advance_XmlParser_(d); /* closing delim */
            return iTrue;
        }
        d->tokenType = none_XmlToken;
        return iFalse;
    }
}

static iBool expect_XmlParser_(iXmlParser *d, enum iXmlToken type) {
    if (d->tokenType != type) {
        return iFalse;
    }
    nextToken_XmlParser_(d);
    return iTrue;
}

static iBool parseTree_XmlParser_(iXmlParser *d, iXmlElement *elem) {
    /* Iterator is assumed to be at the opening token of the element. */
    if (!expect_XmlParser_(d, open_XmlToken)) return iFalse;
    if (d->tokenType != name_XmlToken) {
        return iFalse;
    }
    elem->name = d->token;
    nextToken_XmlParser_(d);
    /* Parse the attributes. */
    while (d->tokenType != close_XmlToken && d->tokenType != closeSlash_XmlToken) {
        if (d->tokenType != name_XmlToken) {
            return iFalse;
        }
        iXmlAttribute attr = { .name = d->token };
        nextToken_XmlParser_(d);
        if (!expect_XmlParser_(d, assignment_XmlToken)) return iFalse;
        attr.value = d->token;
        pushBack_Array(&elem->attribs, &attr);
        //printf("%s.%s = %s\n", cstr_Rangecc(elem->name), cstr_Rangecc(attr.name),
        //       cstr_Rangecc(attr.value)); fflush(stdout);
        nextToken_XmlParser_(d);
    }
    if (d->tokenType == closeSlash_XmlToken) {
        nextToken_XmlParser_(d);
        return iTrue; /* no children */
    }
    iAssert(d->tokenType == close_XmlToken);
    elem->content.start = elem->content.end = d->token.end;
    /* Parse all child elements. */
    nextToken_XmlParser_(d);
    while (d->tokenType != none_XmlToken) {
        if (d->tokenType == open_XmlToken) {
            iXmlElement *child = new_XmlElement();
            if (!parseTree_XmlParser_(d, child)) {
                delete_XmlElement(child);
                return iFalse;
            }
            pushBack_PtrArray(&elem->children, child);
        }
        else if (d->tokenType == openSlash_XmlToken) {
            elem->content.end = d->token.start;
            nextToken_XmlParser_(d);
            if (d->tokenType != name_XmlToken || !equalRange_Rangecc(d->token, elem->name)) {
                return iFalse;
            }
            nextToken_XmlParser_(d);
            break;
        }
        else if (!expect_XmlParser_(d, content_XmlToken)) {
            return iFalse;
        }
    }
    //printf("content of '%s': [%s]\n", cstr_Rangecc(elem->name), cstr_Rangecc(elem->content));
    if (!expect_XmlParser_(d, close_XmlToken)) return iFalse;
    return iTrue;
}

/*----------------------------------------------------------------------------------------------*/

iDefineTypeConstruction(XmlElement)
iDefineTypeConstruction(XmlDocument)

void init_XmlElement(iXmlElement *d) {
    d->name    = iNullRange;
    d->content = iNullRange;
    init_Array(&d->attribs, sizeof(iXmlAttribute));
    init_PtrArray(&d->children);
}

void deinit_XmlElement(iXmlElement *d) {
    deinit_Array(&d->attribs);
    iForEach(PtrArray, i, &d->children) {
        delete_XmlElement(i.ptr);
    }
    deinit_PtrArray(&d->children);
}

const iXmlElement *child_XmlElement(const iXmlElement *d, const char *name) {
    iConstForEach(PtrArray, i, &d->children) {
        const iXmlElement *child = i.ptr;
        if (equal_Rangecc(child->name, name)) {
            return child;
        }
    }
    return NULL;
}

iRangecc attribute_XmlElement(const iXmlElement *d, const char *name) {
    iConstForEach(Array, i, &d->attribs) {
        const iXmlAttribute *attr = i.value;
        if (equal_Rangecc(attr->name, name)) {
            return attr->value;
        }
    }
    return iNullRange;
}

iString *decodedContent_XmlElement(const iXmlElement *d) {
    iString *str = new_String();
    if (!d) return str;
    iBool isCData = iFalse;
    iBool wasSpace = iFalse;
    for (const char *pos = d->content.start; pos < d->content.end; ) {
        if (!isCData && *pos == '&') {
            if (!iCmpStrN(pos, "&quot;", 6)) {
                appendChar_String(str, '"');
                wasSpace = iFalse;
                pos += 6;
            }
            else if (!iCmpStrN(pos, "&apos;", 6)) {
                appendChar_String(str, '\'');
                wasSpace = iFalse;
                pos += 6;
            }
            else if (!iCmpStrN(pos, "&amp;", 5)) {
                appendChar_String(str, '&');
                wasSpace = iFalse;
                pos += 5;
            }
            else if (!iCmpStrN(pos, "&lt;", 4)) {
                appendChar_String(str, '<');
                wasSpace = iFalse;
                pos += 4;
            }
            else if (!iCmpStrN(pos, "&gt;", 4)) {
                appendChar_String(str, '>');
                wasSpace = iFalse;
                pos += 4;
            }
            else if (!iCmpStrN(pos, "&#", 2)) {
                pos += 2;
                iBool isHex = iFalse;
                if (*pos == 'x') {
                    isHex = iTrue;
                    pos++;
                }
                char digits[5];
                iZap(digits);
                for (size_t idx = 0; idx < 4; idx++) {
                    if (*pos == ';') break;
                    digits[idx] = *pos++;
                }
                pos++;
                const iChar codepoint = strtoul(digits, NULL, isHex ? 16 : 10);
                if (codepoint) {
                    appendChar_String(str, codepoint);
                }
            }
            else {
                /* Just skip it, maybe the document is not well-formed. Must not hang! */
                pos++;
            }
        }
        else if (!isCData && !iCmpStrN(pos, "<!--", 4)) {
            pos += 4;
            while (pos <= d->content.end - 3 && iCmpStrN(pos, "-->", 3)) {
                pos++;
            }
            pos += 3;
        }
        else if (!isCData && !iCmpStrN(pos, "<![CDATA[", 9)) {
            pos += 9;
            isCData = iTrue;
        }
        else {
            if (isCData && !iCmpStrN(pos, "]]>", 3)) {
                isCData = iFalse;
                pos += 3;
                continue;
            }
            iChar ch = 0;
            int n = decodeBytes_MultibyteChar(pos, d->content.end, &ch);
            if (n <= 0) {
                return str;
            }
            pos += n;
            /* Normalize whitespace. */
            const iBool isSpace = isSpace_Char(ch);
            if (isSpace) {
                ch = ' ';
                if (wasSpace) continue;
            }
            appendChar_String(str, ch);
            wasSpace = isSpace;
        }
    }
    return str;
}

/*----------------------------------------------------------------------------------------------*/

void init_XmlDocument(iXmlDocument *d) {
    init_String(&d->source);
    init_XmlElement(&d->root);
}

void deinit_XmlDocument(iXmlDocument *d) {
    deinit_XmlElement(&d->root);
    deinit_String(&d->source);
}

iBool parse_XmlDocument(iXmlDocument *d, const iString *source) {
    iXmlParser par;
    init_XmlParser_(&par, d, source);
    /* Must begin with the header. */
    nextToken_XmlParser_(&par);
    if (par.tokenType != headerOpen_XmlToken) {
        return iFalse;
    }
    nextToken_XmlParser_(&par);
    if (par.tokenType != name_XmlToken || !equal_Rangecc(par.token, "xml")) {
        return iFalse;
    }
    while (par.tokenType != headerClose_XmlToken) {
        /* Header must say version 1.0 and UTF-8. */
        nextToken_XmlParser_(&par);
        if (par.tokenType == name_XmlToken) {
            if (equal_Rangecc(par.token, "version")) {
                nextToken_XmlParser_(&par);
                if (!expect_XmlParser_(&par, assignment_XmlToken)) {
                    return iFalse;
                }
                if (par.tokenType != stringLiteral_XmlToken || !equal_Rangecc(par.token, "1.0")) {
                    return iFalse;
                }
            }
            else if (equal_Rangecc(par.token, "encoding")) {
                nextToken_XmlParser_(&par);
                if (!expect_XmlParser_(&par, assignment_XmlToken)) {
                    return iFalse;
                }
                if (par.tokenType != stringLiteral_XmlToken ||
                    !equalCase_Rangecc(par.token, "UTF-8")) {
                    return iFalse;
                }
            }
        }
    }
    nextToken_XmlParser_(&par);
    /* This should now be the root element. */
    if (!parseTree_XmlParser_(&par, &d->root)) {
        return iFalse;
    }
    return par.tokenType == none_XmlToken;
}
