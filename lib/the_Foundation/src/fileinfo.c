/** @file fileinfo.c  File information.

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

#include "the_Foundation/fileinfo.h"
#include "the_Foundation/file.h"
#include "the_Foundation/time.h"
#include "the_Foundation/path.h"

#include <stdlib.h>
#if defined (iPlatformWindows) || defined (iPlatformMsys)
#   define iHaveMsdirent 1
#   include "platform/win32/wide.h"
#   include "platform/win32/msdirent.h"

#   if defined (iPlatformMsys)
#       define R_OK 0
#       define W_OK 1

static int access(const char *path, int mode) {
    const iString str = iStringLiteral(path);
    iBlock *wpath = toUtf16_String(&str);
    const DWORD attr = GetFileAttributesW(data_Block(wpath));
    delete_Block(wpath);
    if (attr == INVALID_FILE_ATTRIBUTES) {
        return -1;
    }
    if (mode == W_OK) {
        return (attr & FILE_ATTRIBUTE_READONLY) ? -1 : 0;
    }
    return 0;
}
#   endif /* defined (iPlatformMsys) */

#else
#   include <sys/stat.h>
#   include <sys/types.h>
#   if defined (iHaveSysDirent)
#       include <sys/dirent.h>
#   elif defined (iPlatformAndroid)
#       include <dirent.h>
#   elif !defined (iPlatformHaiku)
#       include <sys/dir.h>
#   endif
#   include <unistd.h>
#   include <dirent.h>
#endif

#if defined (iPlatformLinux) || defined (iPlatformMsys) || defined (iPlatformCygwin) || defined (iPlatformHaiku)
#   define st_mtimespec st_mtim
#endif

enum FileInfoFlags {
    exists_FileInfoFlag     = iBit(1),
    directory_FileInfoFlag  = iBit(2),
    writable_FileInfoFlag   = iBit(3),
};

struct Impl_FileInfo {
    iObject object;
    iString *path;
    iTime lastModified;
    size_t size;
    uint32_t flags;
};

iDefineClass(FileInfo)
iDefineObjectConstructionArgs(FileInfo, (const iString *path), path)

iFileInfo *newCStr_FileInfo(const char *path) {
    iString str; initCStr_String(&str, path);
    iFileInfo *info = new_FileInfo(&str);
    deinit_String(&str);
    return info;
}

iFileInfo *new_FileInfo_(void) {
    iFileInfo *d = iNew(FileInfo);
    d->path = new_String();
    iZap(d->lastModified);
    d->size = 0;
    d->flags = 0;
    return d;
}

void init_FileInfo(iFileInfo *d, const iString *path) {    
    d->path = copy_String(path);
    d->flags = 0;
    struct stat st;
    if (!stat(cstr_String(d->path), &st)) {
#if defined (iPlatformWindows)
        d->lastModified.ts = (struct timespec){ .tv_sec = st.st_mtime };
#else
        d->lastModified.ts = st.st_mtimespec;
#endif
        d->size = st.st_size;
        d->flags |= exists_FileInfoFlag;
        if (st.st_mode & S_IFDIR) d->flags |= directory_FileInfoFlag;
    }
    else {
        iZap(d->lastModified);
        d->size = iInvalidSize;
    }
}

static iBool initDirEntry_FileInfo_(iFileInfo *d, const iString *dirPath, struct dirent *ent) {
    iString entryName;
#if defined (iPlatformApple)
    initLocalCStrN_String(&entryName, ent->d_name, ent->d_namlen);
#elif defined (iHaveMsdirent)
    initCStrN_String(&entryName, ent->d_name, ent->d_namlen); /* UTF-8 name */
#else
    initLocalCStr_String(&entryName, ent->d_name);
#endif
    /* Check for ignored entries. */
    if (!cmp_String(&entryName, "..") || !cmp_String(&entryName, ".")) {
        deinit_String(&entryName);
        return iFalse;
    }
    iString *full = concat_Path(dirPath, &entryName);
    clean_Path(full);
    set_String(d->path, full);
    delete_String(full);
    deinit_String(&entryName);
    d->flags = exists_FileInfoFlag;
    if (access(cstr_String(d->path), W_OK) == 0) {
        d->flags |= writable_FileInfoFlag;
    }
#if defined (iPlatformHaiku)
    struct stat s;
    stat(ent->d_name, &s);
    if (S_ISDIR(s.st_mode)) {
#else
    if (ent->d_type == DT_DIR) {
#endif
        d->flags |= directory_FileInfoFlag;
        d->size = 0;
    }
    else {
        d->size = iInvalidSize; // Unknown at this time.
    }
    return iTrue;
}

void deinit_FileInfo(iFileInfo *d) {
    delete_String(d->path);
}

iBool exists_FileInfo(const iFileInfo *d) {
    return (d->flags & exists_FileInfoFlag) != 0;
}

const iString *path_FileInfo(const iFileInfo *d) {
    return d->path;
}

size_t size_FileInfo(const iFileInfo *d) {
    if (d->size == iInvalidSize) {
        iConstCast(iFileInfo *, d)->size = fileSize_FileInfo(d->path);
    }
    return d->size;
}

iBool isDirectory_FileInfo(const iFileInfo *d) {
    return (d->flags & directory_FileInfoFlag) != 0;
}
    
iBool isWritable_FileInfo(const iFileInfo *d) {
    return (d->flags & writable_FileInfoFlag) != 0;
}

iTime lastModified_FileInfo(const iFileInfo *d) {
    if (!isValid_Time(&d->lastModified)) {
        struct stat st;
        if (!stat(cstr_String(d->path), &st)) {
#if defined (iPlatformWindows)
            iConstCast(iFileInfo *, d)->lastModified.ts = (struct timespec){ 
                .tv_sec = st.st_mtime };
#else
            iConstCast(iFileInfo *, d)->lastModified.ts = st.st_mtimespec;
#endif
        }
    }
    return d->lastModified;
}

iDirFileInfo *directoryContents_FileInfo(const iFileInfo *fileInfo) {
    iDirFileInfo *d = iNew(DirFileInfo);
    initInfo_DirFileInfo(d, fileInfo);
    return d;
}

iFile *open_FileInfo(const iFileInfo *d, int modeFlags) {
    iFile *f = new_File(path_FileInfo(d));
    open_File(f, modeFlags);
    return f;
}

iBool fileExists_FileInfo(const iString *path) {
    return fileExistsCStr_FileInfo(cstr_String(path));
}

iBool fileExistsCStr_FileInfo(const char *path) {
    return !access(path, R_OK);
}

size_t fileSize_FileInfo(const iString *path) {
    return fileSizeCStr_FileInfo(cstr_String(path));
}

size_t fileSizeCStr_FileInfo(const char *path) {
    size_t size = iInvalidSize;
#if defined (iHaveMsdirent)
    iBeginCollect();
    const wchar_t *wpath = toWide_CStr_(path);
    HANDLE f = CreateFileW(wpath,
                           FILE_READ_ATTRIBUTES,
                           FILE_SHARE_READ,
                           NULL,
                           OPEN_EXISTING,
                           0,
                           NULL);
    if (f != INVALID_HANDLE_VALUE) {
        BY_HANDLE_FILE_INFORMATION info;
        GetFileInformationByHandle(f, &info);
        size = (size_t) info.nFileSizeLow | (((size_t) info.nFileSizeHigh) << 32);
        CloseHandle(f);
    }
    else {
        if (GetFileAttributesW(wpath) & FILE_ATTRIBUTE_DIRECTORY) {
            size = 0;
        }
    }
    iEndCollect();
#else
    struct stat st;
    if (!stat(path, &st)) {
        size = (size_t) st.st_size;
    }
#endif
    return size;
}

/*-------------------------------------------------------------------------------------*/

struct Impl_DirFileInfo {
    iObject object;
    const iFileInfo *dirInfo;
    DIR *fd;
    iFileInfo *entry;
};

iDefineClass(DirFileInfo)
iDefineObjectConstructionArgs(DirFileInfo, (const iString *path), path)

iDirFileInfo *newCStr_DirFileInfo(const char *path) {
    iString str; initCStr_String(&str, path);
    iDirFileInfo *d = new_DirFileInfo(&str);
    deinit_String(&str);
    return d;
}

void init_DirFileInfo(iDirFileInfo *d, const iString *path) {
    iFileInfo *fileInfo = new_FileInfo(path);
    initInfo_DirFileInfo(d, fileInfo);
    iRelease(fileInfo);
    d->entry = NULL;
}

void initInfo_DirFileInfo(iDirFileInfo *d, const iFileInfo *dir) {
    if (isDirectory_FileInfo(dir)) {
        d->fd = opendir(cstr_String(path_FileInfo(dir)));
        d->dirInfo = ref_Object(dir);
    }
    else {
        d->fd = NULL;
        d->dirInfo = NULL;
    }
    d->entry = NULL;
}

void deinit_DirFileInfo(iDirFileInfo *d) {
    if (d->fd) {
        closedir(d->fd);
        d->fd = NULL;
    }
    iRelease(d->entry);
    deref_Object(d->dirInfo);
}

static iBool readNextEntry_DirFileInfo_(iDirFileInfo *d) {
    iReleasePtr(&d->entry);
    if (!d->fd) {
        return iFalse;
    }
    for (;;) {
        struct dirent *result = NULL;
#if defined (iPlatformApple)
        struct dirent ent;
        readdir_r(d->fd, &ent, &result);
#else
        result = readdir(d->fd);
#endif
        if (result) {
            iReleasePtr(&d->entry);
            d->entry = new_FileInfo_();
            if (!initDirEntry_FileInfo_(d->entry, path_FileInfo(d->dirInfo), result)) {
                continue;
            }
            return iTrue;
        }
        return iFalse;
    }
}

void init_DirFileInfoIterator(iDirFileInfoIterator *d, iDirFileInfo *info) {
    d->dir = info;
    next_DirFileInfoIterator(d);
}

void next_DirFileInfoIterator(iDirFileInfoIterator *d) {
    if (readNextEntry_DirFileInfo_(d->dir)) {
        d->value = d->dir->entry;
    }
    else {
        d->value = NULL;
    }
}
