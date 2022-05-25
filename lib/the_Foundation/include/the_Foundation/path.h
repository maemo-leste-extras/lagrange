#pragma once

/** @file the_Foundation/path.h  File path manipulation, directory listing.

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
#include "string.h"

iBeginPublic

#if defined (iPlatformWindows) || defined (iPlatformCygwin) || defined (iPlatformMsys)
#   define iPathSeparator   "\\"
#else
#   define iPathSeparator   "/"
#endif

iString *   cwd_Path        (void);
iBool       setCwd_Path     (const iString *path);
iString *   home_Path       (void);

iBool       mkdir_Path      (const iString *path);
void        makeDirs_Path   (const iString *path);
iBool       rmdir_Path      (const iString *path);

iBool       isAbsolute_Path     (const iString *);
iString *   makeAbsolute_Path   (const iString *);
iString *   makeRelative_Path   (const iString *);
iString *   concat_Path         (const iString *, const iString *path);
iString *   concatCStr_Path     (const iString *, const char *path);

const char *concatPath_CStr  (const char *, const char *);

void        append_Path     (iString *, const iString *path);
void        clean_Path      (iString *);

iLocalDef iString *cleaned_Path(const iString *d) {
    iString *clean = copy_String(d);
    clean_Path(clean);
    return clean;
}
iLocalDef iString *cleanedCStr_Path(const char *cstr) {
    return cleaned_Path(collectNewCStr_String(cstr));
}
iLocalDef const char *cleanedPath_CStr(const char *cstr) {
    return cstrCollect_String(cleanedCStr_Path(cstr));
}

iRangecc    baseName_Path           (const iString *); /* returns address of last component */
iRangecc    baseNameSep_Path        (const iString *, const char *separator);
iRangecc    withoutExtension_Path   (const iString *);
iRangecc    dirName_Path            (const iString *); /* native path separator */
iRangecc    dirNameSep_Path         (const iString *, const char *separator);

#if defined (iPlatformCygwin) || defined (iPlatformMsys)
iString *   unixToWindows_Path         (const char *cstr);
iString *   unixToWindowsRelative_Path (const char *cstr);
#endif

iEndPublic
