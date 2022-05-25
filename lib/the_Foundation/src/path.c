/** @file path.c  File path manipulation.

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

#include "the_Foundation/path.h"
#include "the_Foundation/fileinfo.h"
#include "the_Foundation/string.h"

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#if defined (iPlatformWindows)
#   include <direct.h>
#   define mkdir(path, attr) _mkdir(path)
#endif

#if defined (iPlatformCygwin) || defined (iPlatformMsys)
#   include <sys/cygwin.h>
#   define iHaveCygwinPathConversion
#endif

iString *cwd_Path(void) {
    char *cwd = getcwd(NULL, 0);
#if defined (iHaveCygwinPathConversion)
    iString *str = unixToWindows_Path(cwd);
    free(cwd);
    return str;
#else
    if (cwd) {
        iBlock block;
        const size_t len = strlen(cwd);
        initPrealloc_Block(&block, cwd, len, len + 1);
        iString *d = newBlock_String(&block);
        deinit_Block(&block);
        return d;
    }
    return new_String();
#endif
}

iBool setCwd_Path(const iString *path) {
    return !chdir(cstr_String(path));
}

iString *home_Path(void) {
    iString *home = new_String();
#if defined (iPlatformMsys) || defined (iPlatformWindows)
    format_String(home, "%s%s", getenv("HOMEDRIVE"), getenv("HOMEPATH"));
#else
    setCStr_String(home, getenv("HOME"));
#endif
    return home;
}

iBool isAbsolute_Path(const iString *d) {
#if !defined (iPlatformWindows)
    if (startsWith_String(d, "~")) {
        return iTrue;
    }
#endif
    if (startsWith_String(d, iPathSeparator)) {
        return iTrue;
    }
#if defined (iPlatformWindows) || defined (iHaveCygwinPathConversion)
    /* Also accept Unix-style paths as absolute since we can convert them. */
    if (startsWith_String(d, "/")) {
        return iTrue;
    }
    /* Check for drive letters. */
    if (size_String(d) >= 3) {
        iStringConstIterator i;
        init_StringConstIterator(&i, d);
        const iChar drive = upper_Char(i.value);
        if (drive < 'A' || drive > 'Z') {
            return iFalse;
        }
        next_StringConstIterator(&i);
        if (i.value != ':') {
            return iFalse;
        }
        next_StringConstIterator(&i);
        return i.value == '\\' || i.value == '/';
    }
#endif
    return iFalse;
}

iString *makeAbsolute_Path(const iString *d) {
    iString *abs;
    iString *path = copy_String(d);
    clean_Path(path);
    if (isAbsolute_Path(path)) {
        abs = copy_String(path);
    }
    else {
        abs = cwd_Path();
        append_Path(abs, path);
        clean_Path(abs);
    }
    delete_String(path);
    return abs;
}

iString *makeRelative_Path(const iString *d) {
    iString *rel = copy_String(d);
    iString *cwd = cwd_Path();
    if (startsWith_String(d, cstr_String(cwd))) {
        remove_Block(&rel->chars, 0, size_String(cwd));
        if (startsWith_String(rel, iPathSeparator)) {
            remove_Block(&rel->chars, 0, 1);
        }
    }
    delete_String(cwd);
    return rel;
}

#define iPathMaxSegments 128

static iBool splitSegments_Path_(const iRangecc path, iRangecc *segments,
                                 size_t *count, iBool *changed) {
    iRangecc seg = iNullRange;
    while (nextSplit_Rangecc(path, iPathSeparator, &seg)) {
        if (*count > 0 && size_Range(&seg) == 0) {
            /* Skip repeated slashes. */
            *changed = iTrue;
            continue;
        }
#if defined (iPlatformMsys) || !defined (iPlatformWindows)
        if (*count == 0 && !iCmpStrRange(seg, "~")) {
            const iString *home = collect_String(home_Path());
            if (!isEmpty_String(home)) {
                if (!splitSegments_Path_(range_String(home), segments, count, changed)) {
                    return iFalse;
                }
                *changed = iTrue;
                continue;
            }
        }
#endif
        if (!iCmpStrRange(seg, ".")) {
            *changed = iTrue;
            continue; // No change in directory.
        }
        if (!iCmpStrRange(seg, "..")) {
            if (*count > 0 && iCmpStrRange(segments[*count - 1], "..")) {
                (*count)--; // Go up a directory.
                *changed = iTrue;
                continue;
            }
        }
        if (*count == iPathMaxSegments) {
            iAssert(*count < iPathMaxSegments);
            return iFalse; // Couldn't clean it.
        }
        segments[(*count)++] = seg;
    }
    return iTrue;
}

void clean_Path(iString *d) {
    if (isEmpty_String(d)) return;
#if defined (iHaveCygwinPathConversion)
    /* Convert to a Windows path, not forcing it to an absolute path. */ {
        iString *winPath = unixToWindowsRelative_Path(cstr_String(d));
        set_String(d, winPath);
        delete_String(winPath);
    }
#elif defined (iPlatformWindows)
    /* Use the correct separators. */
    replace_Block(&d->chars, '/', '\\');
#endif
    iRangecc segments[iPathMaxSegments];
    size_t count = 0;
    iBool changed = iFalse;
    splitSegments_Path_(range_String(d), segments, &count, &changed);
    /* Recompose the remaining segments. */
    if (changed) {
        if (count == 0) {
            setCStr_String(d, ".");
            return;
        }
        iString cleaned;
        init_String(&cleaned);
        for (size_t i = 0; i < count; ++i) {
            if (i != 0 || (isAbsolute_Path(d)
#if defined (iPlatformWindows) || defined (iHaveCygwinPathConversion)
                && startsWith_String(d, iPathSeparator)
#endif
                    )) {
                appendCStr_String(&cleaned, iPathSeparator);
            }
            appendRange_String(&cleaned, segments[i]);
        }
        set_String(d, &cleaned);
        deinit_String(&cleaned);
    }
}

void append_Path(iString *d, const iString *path) {
    if (isAbsolute_Path(path)) {
        set_String(d, path);
    }
    else {
        if (!endsWith_String(d, iPathSeparator)) {
            appendCStr_String(d, iPathSeparator);
        }
        append_String(d, path);
    }
    clean_Path(d);
}

iString *concat_Path(const iString *d, const iString *path) {
    iString *cat = copy_String(d);
    append_Path(cat, path);
    return cat;
}

iString *concatCStr_Path(const iString *d, const char *path) {
    iString p;
    initCStr_String(&p, path);
    iString *cat = concat_Path(d, &p);
    deinit_String(&p);
    return cat;
}

const char *concatPath_CStr(const char *dir, const char *path) {
    iString d;
    initCStr_String(&d, dir);
    iString *cat = concatCStr_Path(&d, path);
    deinit_String(&d);
    return cstr_String(collect_String(cat));
}

iBool mkdir_Path(const iString *path) {
    return mkdir(cstr_String(path), 0755) == 0;
}

iBool rmdir_Path(const iString *path) {
    return rmdir(cstr_String(path)) == 0;
}

iRangecc baseName_Path(const iString *d) {
    return baseNameSep_Path(d, iPathSeparator);
}

iRangecc baseNameSep_Path(const iString *d, const char *separator) {
    const size_t sep = lastIndexOfCStr_String(d, separator);
    return (iRangecc){ cstr_String(d) + (sep == iInvalidSize ? 0 : (sep + 1)),
                       constEnd_String(d) };
}

iRangecc withoutExtension_Path(const iString *d) {
    iRangecc base = baseName_Path(d);
    while (base.start < base.end && *base.start != '.') {
        base.start++;
    }
    if (isEmpty_Range(&base)) {
        return range_String(d);
    }
    return (iRangecc){ constBegin_String(d), base.start };
}

iRangecc dirName_Path(const iString *d) {
    return dirNameSep_Path(d, iPathSeparator);
}

iRangecc dirNameSep_Path(const iString *d, const char *separator) {
    const size_t sep = lastIndexOfCStr_String(d, separator);
    if (sep == iInvalidSize) {
        static const char *dot = ".";
        return (iRangecc){ dot, dot + 1 };
    }
    return (iRangecc){ cstr_String(d), cstr_String(d) + sep };
}

void makeDirs_Path(const iString *path) {
    iString *clean = copy_String(path);
    clean_Path(clean);
    iString *dir = newRange_String(dirName_Path(clean));
    if (!fileExists_FileInfo(dir)) {
        makeDirs_Path(dir);
    }
    delete_String(dir);
    if (!mkdir_Path(clean)) {
        iDebug("[Path] failed to create directory: %s\n", strerror(errno));
    }
    delete_String(clean);
}

#if defined (iHaveCygwinPathConversion)
static iString *unixToWindows_(const char *cstr, iBool makeAbsolute) {
    if (!cstr) {
        return new_String();
    }
    uint16_t *winPath = NULL;
    if (!iCmpStrN(cstr, "~/", 2) || !iCmpStrN(cstr, "~\\", 2)) {
        /* Expand the home directory. */
        winPath = cygwin_create_path(CCP_POSIX_TO_WIN_W | CCP_RELATIVE, cstr + 2);
        iString *conv = newUtf16_String(winPath);
        iString *str = home_Path();
        append_Path(str, conv);
        free(winPath);
        delete_String(conv);
        return str;
    }
    winPath = cygwin_create_path(CCP_POSIX_TO_WIN_W |
                                 (makeAbsolute ? CCP_ABSOLUTE : CCP_RELATIVE),
                                 cstr);
    if (winPath) {
        iString *str = newUtf16_String(winPath);
        free(winPath);
        return str;
    }
    return newCStr_String(cstr); /* seems fine as-is? */
}

iString *unixToWindowsRelative_Path(const char *cstr) {
    return unixToWindows_(cstr, iFalse);
}

iString *unixToWindows_Path(const char *cstr) {
    return unixToWindows_(cstr, iTrue /* absolute */);
}
#endif
