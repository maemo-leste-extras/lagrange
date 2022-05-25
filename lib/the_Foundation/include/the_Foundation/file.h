#pragma once

/** @file the_Foundation/file.h  File stream.

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

#include "stream.h"

#include <stdio.h>

iBeginPublic

typedef iStreamClass iFileClass;

iDeclareType(File)
iDeclareType(String)

enum iFileMode {
    read_FileMode       = 0x1,
    readOnly_FileMode   = 0x1,
    write_FileMode      = 0x2,
    writeOnly_FileMode  = 0x2,
    append_FileMode     = 0x4,
    text_FileMode       = 0x8,

    readWrite_FileMode  = read_FileMode | write_FileMode,
};

struct Impl_File {
    iStream stream;
    iString *path;
    int flags;
    void *file; /* native handle */
};

iDeclareObjectConstructionArgs(File, const iString *path)

iFile *     newCStr_File    (const char *path);

iBool       open_File       (iFile *, int mode);
void        close_File      (iFile *);
iBool       isOpen_File     (const iFile *);

iLocalDef int    mode_File   (const iFile *d) { return d->flags ;}
iLocalDef size_t pos_File    (const iFile *d) { return pos_Stream(&d->stream); }
iLocalDef size_t size_File   (const iFile *d) { return size_Stream(&d->stream); }
iLocalDef iBool  atEnd_File  (const iFile *d) { return atEnd_Stream(&(d)->stream); }
iLocalDef const  iString *path_File(const iFile *d) { return d->path; }

iLocalDef iStream *     stream_File     (iFile *d) { return &d->stream; }
iLocalDef void          seek_File       (iFile *d, size_t offset) { seek_Stream(&d->stream, offset); }
iLocalDef iBlock *      read_File       (iFile *d, size_t size) { return read_Stream(&d->stream, size); }
iLocalDef size_t        readData_File   (iFile *d, size_t size, void *data_out) { return readData_Stream(&d->stream, size, data_out); }
iLocalDef iBlock *      readAll_File    (iFile *d) { return readAll_Stream(&d->stream); }
iLocalDef iString *     readString_File (iFile *d) { return readString_Stream(&d->stream); }
iLocalDef iStringList * readLines_File  (iFile *d) { return readLines_Stream(&d->stream); }
iLocalDef int8_t        read8_File      (iFile *d) { return read8_Stream(&d->stream); }
iLocalDef int16_t       read16_File     (iFile *d) { return read16_Stream(&d->stream); }
iLocalDef int32_t       read32_File     (iFile *d) { return read32_Stream(&d->stream); }
iLocalDef uint8_t       readU8_File     (iFile *d) { return readU8_Stream(&d->stream); }
iLocalDef uint16_t      readU16_File    (iFile *d) { return readU16_Stream(&d->stream); }
iLocalDef uint32_t      readU32_File    (iFile *d) { return readU32_Stream(&d->stream); }

iLocalDef size_t        write_File      (iFile *d, const iBlock *data) { return write_Stream(&d->stream, data); }
iLocalDef size_t        writeData_File  (iFile *d, const void *data, size_t size) { return writeData_Stream(&d->stream, data, size); }
iLocalDef void          write8_File     (iFile *d, int8_t value) { write8_Stream(&d->stream, value); }
iLocalDef void          write16_File    (iFile *d, int16_t value) { write16_Stream(&d->stream, value); }
iLocalDef void          write32_File    (iFile *d, int32_t value) { write32_Stream(&d->stream, value); }
iLocalDef void          writeU8_File    (iFile *d, uint8_t value) { writeU8_Stream(&d->stream, value); }
iLocalDef void          writeU16_File   (iFile *d, uint16_t value) { writeU16_Stream(&d->stream, value); }
iLocalDef void          writeU32_File   (iFile *d, uint32_t value) { writeU32_Stream(&d->stream, value); }

iEndPublic
