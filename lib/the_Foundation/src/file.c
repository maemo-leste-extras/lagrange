/** @file file.c  File stream.

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

#include "the_Foundation/file.h"
#include "the_Foundation/fileinfo.h"
#include "the_Foundation/path.h"
#include "the_Foundation/string.h"

static iFileClass Class_File;

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
    d->file = NULL;
}

void deinit_File(iFile *d) {
    if (isOpen_File(d)) {
        close_File(d);
    }
    delete_String(d->path);
}

iBool open_File(iFile *d, int modeFlags) {
    if (isOpen_File(d)) return iFalse;
    d->stream.pos = 0;
    d->flags = modeFlags;
    if ((d->flags & (readWrite_FileMode | append_FileMode)) == 0) {
        /* Default to read. */
        d->flags |= read_FileMode;
    }
    if (d->flags & (read_FileMode | append_FileMode)) {
        setSize_Stream(&d->stream, fileSize_FileInfo(d->path));
    }
    char mode[4], *m = mode;
    if (d->flags & append_FileMode) {
        *m++ = 'a';
        d->stream.pos = d->stream.size;
        if (d->flags & read_FileMode) { *m++ = '+'; }
    }
    else {
        if (d->flags & read_FileMode) { *m++ = 'r'; }
        if (d->flags & write_FileMode) {
            if (d->flags & read_FileMode) { *m++ = '+'; }
            else { *m++ = 'w'; }
        }
    }
    if (d->flags & text_FileMode) { *m++ = 't'; } else { *m++ = 'b'; }
    *m = 0;
    d->file = fopen(cstr_String(d->path), mode);
    return isOpen_File(d);
}

void close_File(iFile *d) {
    if (isOpen_File(d)) {
        fclose(d->file);
        d->file = NULL;
    }
}

iBool isOpen_File(const iFile *d) {
    return d->file != NULL;
}

static size_t seek_File_(iFile *d, size_t offset) {
    if (isOpen_File(d)) {
        fseek(d->file, offset, SEEK_SET);
        return ftell(d->file);
    }
    return pos_Stream(&d->stream);
}

static size_t read_File_(iFile *d, size_t size, void *data_out) {
    if (isOpen_File(d)) {
        const size_t oldPos = d->stream.pos;
        size_t numRead = fread(data_out, 1, size, d->file);
#if defined (iPlatformWindows) || defined (iPlatformMsys)
        if (d->flags & text_FileMode) {
            /* We may have skipped over some carriage returns. */
            const size_t numActualRead = ftell(d->file) - oldPos;
            d->stream.pos += numActualRead - numRead;
        }
#else
        iUnused(oldPos);
#endif
        return numRead;
    }
    return 0;
}

static size_t write_File_(iFile *d, const void *data, size_t size) {
    if (isOpen_File(d)) {
        return fwrite(data, 1, size, d->file);
    }
    return 0;
}

static void flush_File_(iFile *d) {
    if (isOpen_File(d)) {
        fflush(d->file);
    }
}

static iBeginDefineSubclass(File, Stream)
    .seek   = (size_t (*)(iStream *, size_t))               seek_File_,
    .read   = (size_t (*)(iStream *, size_t, void *))       read_File_,
    .write  = (size_t (*)(iStream *, const void *, size_t)) write_File_,
    .flush  = (void   (*)(iStream *))                       flush_File_,
iEndDefineClass(File)
