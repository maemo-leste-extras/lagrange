#pragma once

/* Copyright 2021 Jaakko Keränen <jaakko.keranen@iki.fi>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#include "block.h"
#include "string.h"
#include "stringset.h"
#include "time.h"

#if defined (iHaveZlib)

iBeginPublic

iDeclareType(ArchiveEntry)
iDeclareTypeConstruction(ArchiveEntry)

struct Impl_ArchiveEntry {
    iString  path;
    size_t   size;
    iTime    timestamp;
    uint32_t crc32;
    size_t   archPos;
    size_t   archSize;
    int      compression;
    iBlock * data; /* NULL until uncompressed with `data_Archive()` */
};

iDeclareClass(Archive)
iDeclareObjectConstruction(Archive)

iBool   openData_Archive    (iArchive *, const iBlock *data);
iBool   openFile_Archive    (iArchive *, const iString *path);
void    openWritable_Archive(iArchive *);
void    close_Archive       (iArchive *);

iBool   isOpen_Archive      (const iArchive *);
size_t  numEntries_Archive  (const iArchive *);
size_t  sourceSize_Archive  (const iArchive *);
iBool   isDirectory_Archive (const iArchive *, const iString *path);

iStringSet *    listDirectory_Archive       (const iArchive *, const iString *dirPath);

const iArchiveEntry *   entry_Archive       (const iArchive *, const iString *path);
const iArchiveEntry *   entryCStr_Archive   (const iArchive *, const char *pathCStr);
const iArchiveEntry *   entryAt_Archive     (const iArchive *, size_t index);

const iBlock *          data_Archive        (const iArchive *, const iString *path);
const iBlock *          dataCStr_Archive    (const iArchive *d, const char *pathCStr);
const iBlock *          dataAt_Archive      (const iArchive *, size_t index);

void    setData_Archive     (iArchive *, const iString *path, const iBlock *data);
void    setDataCStr_Archive (iArchive *, const char *path, const iBlock *data);
void    serialize_Archive   (const iArchive *, iStream *);

/** @name Iterators */
///@{
iDeclareConstIterator(Archive, const iArchive *)
struct ConstIteratorImpl_Archive {
    const iArchiveEntry *value;
    size_t               index;
    const iArchive *     archive;
};
///@}

iEndPublic

#endif /* defined (iHaveZlib) */
