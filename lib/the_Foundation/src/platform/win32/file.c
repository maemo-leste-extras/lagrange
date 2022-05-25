/** @file file.c  File stream using Win32 API

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

#include "the_Foundation/file.h"
#include "the_Foundation/fileinfo.h"
#include "the_Foundation/path.h"
#include "the_Foundation/string.h"
#include "wide.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static iFileClass Class_File; /* Note: alternative implementation, cf. src/file.c */

enum iFileFlag {
    readEndedAtCarriageReturn_FileFlag = iBit(17),
};

iFile *new_File(const iString *path) {
    iFile *d = new_Object(&Class_File);
    init_File(d, path);
    return d;
}

iFile *newCStr_File(const char *path) {
    iString str;
    initCStr_String(&str, path);
    clean_Path(&str);
    iFile *d = new_File(&str);
    deinit_String(&str);
    return d;
}

void init_File(iFile *d, const iString *path) {
    iAssertIsObject(d);
    init_Stream(&d->stream);
    d->path = copy_String(path);
    clean_Path(d->path);
    d->flags = readOnly_FileMode;
    d->file = INVALID_HANDLE_VALUE;
}

void deinit_File(iFile *d) {
    if (isOpen_File(d)) {
        close_File(d);
    }
    delete_String(d->path);
}

iBool open_File(iFile *d, int modeFlags) {
    if (isOpen_File(d)) return iFalse;
    d->flags = modeFlags;
    if ((d->flags & (readWrite_FileMode | append_FileMode)) == 0) {
        /* Default to read. */
        d->flags |= read_FileMode;
    }
    DWORD desiredAccess = 0;
    DWORD creation      = OPEN_EXISTING;
    DWORD shareMode     = FILE_SHARE_READ;
    DWORD flagsAttribs  = FILE_ATTRIBUTE_NORMAL;
    if (d->flags & (write_FileMode | append_FileMode)) {
        desiredAccess = GENERIC_WRITE;
        creation      = OPEN_ALWAYS;
    }
    if (d->flags & read_FileMode) {
        desiredAccess |= GENERIC_READ;
    }
    d->file = CreateFileW(toWide_CStr_(cstr_String(d->path)),
                          desiredAccess,
                          shareMode,
                          NULL,
                          creation,
                          flagsAttribs,
                          NULL);
    if (isOpen_File(d) && d->flags & (read_FileMode | append_FileMode)) {
        SetFilePointer(d->file, 0, NULL, FILE_END);
        LARGE_INTEGER endPos;
        LARGE_INTEGER zero;
        iZap(zero);
        SetFilePointerEx(d->file, zero, &endPos, FILE_CURRENT);
        d->stream.size = (size_t) endPos.QuadPart;
        if (d->flags & append_FileMode) {
            d->stream.pos = d->stream.size;
        }
        else {
            d->stream.pos = 0;
            SetFilePointer(d->file, 0, NULL, FILE_BEGIN);
        }
    }
    return isOpen_File(d);
}

void close_File(iFile *d) {
    if (isOpen_File(d)) {
        CloseHandle(d->file);
        d->file = INVALID_HANDLE_VALUE;
    }
}

iBool isOpen_File(const iFile *d) {
    return d->file != INVALID_HANDLE_VALUE;
}

static size_t seek_File_(iFile *d, size_t offset) {
    if (isOpen_File(d)) {
        LARGE_INTEGER newPos;
        SetFilePointerEx(d->file, (LARGE_INTEGER){ .QuadPart = offset }, &newPos, FILE_BEGIN);
        return (size_t) newPos.QuadPart;
    }
    return pos_Stream(&d->stream);
}

static size_t read_File_(iFile *d, size_t size, void *data_out) {
    if (isOpen_File(d) && size > 0) {
        DWORD numRead = 0;
        if (~d->flags & text_FileMode) {
            ReadFile(d->file, data_out, size, &numRead, NULL);
            return numRead;
        }
        /* Read to a temporary buffer first and convert newlines. */
        char         buf[1024];
        const size_t bufLen = sizeof(buf) - 1;
        size_t       avail  = size;
        char *       outBuf = data_out;
        DWORD        numSkipped = 0;
        while (avail) {
            ReadFile(d->file, buf, iMin(avail, bufLen), &numRead, NULL);
            if (!numRead) break;           
            char *end = buf + numRead;
            if (end[-1] == '\r') {
                /* Need to know if the following character is a newline. */
                DWORD extra = 0;
                ReadFile(d->file, end, 1, &extra, NULL);
                if (extra) {
                    SetFilePointer(d->file, -1, NULL, FILE_CURRENT);
                }
            }
            for (const char *ch = buf; ch != end; ch++) {
                if (ch[0] == '\r' && ch[1] == '\n') {
                    numSkipped++;
                }
                else {
                    *outBuf++ = *ch;
                    iAssert(avail != 0);
                    avail--;
                }
            }
        }
        /* Stream position must reflect all bytes in the file. */
        d->stream.pos += numSkipped;
        return size - avail;
    }
    return 0;
}

static size_t write_File_(iFile *d, const void *data, size_t size) {
    if (isOpen_File(d)) {
        DWORD numWritten = 0;
        iBlock buf;
        if (d->flags & text_FileMode) {
            /* Must convert any line endings. */
            init_Block(&buf, 0);
            for (const char *ch = data; ch != (const char *) data + size; ch++) {
                if (*ch == '\n') {
                    pushBack_Block(&buf, '\r');
                }
                pushBack_Block(&buf, *ch);
            }
            WriteFile(d->file, data_Block(&buf), size_Block(&buf), 
                      &numWritten, NULL);
            deinit_Block(&buf);
        }
        else {
            WriteFile(d->file, data, size, &numWritten, NULL);
        }
        return numWritten;
    }
    return 0;
}

static void flush_File_(iFile *d) {
    if (isOpen_File(d)) {
        FlushFileBuffers(d->file);
    }
}

static iBeginDefineSubclass(File, Stream)
    .seek   = (size_t (*)(iStream *, size_t))               seek_File_,
    .read   = (size_t (*)(iStream *, size_t, void *))       read_File_,
    .write  = (size_t (*)(iStream *, const void *, size_t)) write_File_,
    .flush  = (void   (*)(iStream *))                       flush_File_,
iEndDefineClass(File)
