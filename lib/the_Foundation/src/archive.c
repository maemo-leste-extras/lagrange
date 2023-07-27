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

#include "the_Foundation/archive.h"
#include "the_Foundation/array.h"
#include "the_Foundation/buffer.h"
#include "the_Foundation/file.h"
#include "the_Foundation/path.h"
#include "the_Foundation/sortedarray.h"

/* Marker signatures. */
#define SIG_LOCAL_FILE_HEADER   0x04034b50
#define SIG_CENTRAL_FILE_HEADER 0x02014b50
#define SIG_END_OF_CENTRAL_DIR  0x06054b50
#define SIG_DIGITAL_SIGNATURE   0x05054b50

/* Maximum tolerated size of the comment. */
#define MAXIMUM_COMMENT_SIZE    2048

/* This is the length of the central directory end record (without the
   comment, but with the signature). */
#define CENTRAL_END_SIZE        22

/* File header flags. */
#define ZFH_ENCRYPTED           0x1
#define ZFH_COMPRESSION_OPTS    0x6
#define ZFH_DESCRIPTOR          0x8
#define ZFH_COMPRESS_PATCHED    0x20    /* not supported */

/* Compression methods. */
enum iCompression {
    none_Compression = 0,       /* supported */
    shrunk_Compression,
    reduced1_Compression,
    reduced2_Compression,
    reduced3_Compression,
    reduced4_Compression,
    imploded_Compression,
    deflated_Compression = 8,   /* supported (zlib) */
    deflated64_Compression,
    pkwareDCLImploded_Compression
};

iDeclareType(DOSTime)
iDeclareType(DOSDate)
iDeclareType(LocalFileHeader)
iDeclareType(CentralFileHeader)
iDeclareType(CentralEnd)

/* MS-DOS time:
   - 0..4:   Second/2
   - 5..10:  Minute
   - 11..15: Hour
*/
struct Impl_DOSTime {
    uint16_t seconds;
    uint16_t minutes;
    uint16_t hours;
};

static void init_DOSTime_(iDOSTime *d, uint16_t packed) {
    d->seconds = (packed & 0x1f) * 2;
    d->minutes = (packed >> 5) & 0x3f;
    d->hours   = packed >> 11;
}

static uint16_t packed_DOSTime_(const iDOSTime *d) {
    return ((d->seconds / 2) & 0x1f) | ((d->minutes & 0x3f) << 5) | (d->hours << 11);
}

/* MS-DOS date:
   - 0..4:  Day of the month (1-31)
   - 5..8:  Month (1=Jan)
   - 9..15: Year since 1980
*/
struct Impl_DOSDate {
    uint16_t dayOfMonth;
    uint16_t month;
    uint16_t year;
};

static void init_DOSDate_(iDOSDate *d, uint16_t packed) {
    d->dayOfMonth = packed & 0x1f;
    d->month      = (packed >> 5) & 0xf;
    d->year       = packed >> 9;
}

static uint16_t packed_DOSDate_(const iDOSDate *d) {
    return (d->dayOfMonth & 0x1f) | ((d->month & 0xf) << 5) | (d->year << 9);
}

struct Impl_LocalFileHeader {
    uint32_t signature;
    uint16_t requiredVersion;
    uint16_t flags;
    uint16_t compression;
    uint16_t lastModTime;
    uint16_t lastModDate;
    uint32_t crc32;
    uint32_t compressedSize;
    uint32_t size;
    uint16_t fileNameSize;
    uint16_t extraFieldSize;
};

static void read_LocalFileHeader_(iLocalFileHeader *d, iStream *stream) {
    d->signature       = readU32_Stream(stream);
    d->requiredVersion = readU16_Stream(stream);
    d->flags           = readU16_Stream(stream);
    d->compression     = readU16_Stream(stream);
    d->lastModTime     = readU16_Stream(stream);
    d->lastModDate     = readU16_Stream(stream);
    d->crc32           = readU32_Stream(stream);
    d->compressedSize  = readU32_Stream(stream);
    d->size            = readU32_Stream(stream);
    d->fileNameSize    = readU16_Stream(stream);
    d->extraFieldSize  = readU16_Stream(stream);
}

static void write_LocalFileHeader_(const iLocalFileHeader *d, iStream *stream) {
    writeU32_Stream(stream, d->signature);
    writeU16_Stream(stream, d->requiredVersion);
    writeU16_Stream(stream, d->flags);
    writeU16_Stream(stream, d->compression);
    writeU16_Stream(stream, d->lastModTime);
    writeU16_Stream(stream, d->lastModDate);
    writeU32_Stream(stream, d->crc32);
    writeU32_Stream(stream, d->compressedSize);
    writeU32_Stream(stream, d->size);
    writeU16_Stream(stream, d->fileNameSize);
    writeU16_Stream(stream, d->extraFieldSize);
}

struct Impl_CentralFileHeader {
    uint32_t signature;
    uint16_t version;
    uint16_t requiredVersion;
    uint16_t flags;
    uint16_t compression;
    uint16_t lastModTime;
    uint16_t lastModDate;
    uint32_t crc32;
    uint32_t compressedSize;
    uint32_t size;
    uint16_t fileNameSize;
    uint16_t extraFieldSize;
    uint16_t commentSize;
    uint16_t diskStart;
    uint16_t internalAttrib;
    uint32_t externalAttrib;
    uint32_t relOffset;

    /* Followed by:
       - file name (variable size)
       - extra field (variable size)
       - file comment (variable size) */
};

static void read_CentralFileHeader_(iCentralFileHeader *d, iStream *stream) {
    d->signature       = readU32_Stream(stream);
    d->version         = readU16_Stream(stream);
    d->requiredVersion = readU16_Stream(stream);
    d->flags           = readU16_Stream(stream);
    d->compression     = readU16_Stream(stream);
    d->lastModTime     = readU16_Stream(stream);
    d->lastModDate     = readU16_Stream(stream);
    d->crc32           = readU32_Stream(stream);
    d->compressedSize  = readU32_Stream(stream);
    d->size            = readU32_Stream(stream);
    d->fileNameSize    = readU16_Stream(stream);
    d->extraFieldSize  = readU16_Stream(stream);
    d->commentSize     = readU16_Stream(stream);
    d->diskStart       = readU16_Stream(stream);
    d->internalAttrib  = readU16_Stream(stream);
    d->externalAttrib  = readU32_Stream(stream);
    d->relOffset       = readU32_Stream(stream);
}

static void write_CentralFileHeader_(const iCentralFileHeader *d, iStream *stream) {
    writeU32_Stream(stream, d->signature);
    writeU16_Stream(stream, d->version);
    writeU16_Stream(stream, d->requiredVersion);
    writeU16_Stream(stream, d->flags);
    writeU16_Stream(stream, d->compression);
    writeU16_Stream(stream, d->lastModTime);
    writeU16_Stream(stream, d->lastModDate);
    writeU32_Stream(stream, d->crc32);
    writeU32_Stream(stream, d->compressedSize);
    writeU32_Stream(stream, d->size);
    writeU16_Stream(stream, d->fileNameSize);
    writeU16_Stream(stream, d->extraFieldSize);
    writeU16_Stream(stream, d->commentSize);
    writeU16_Stream(stream, d->diskStart);
    writeU16_Stream(stream, d->internalAttrib);
    writeU32_Stream(stream, d->externalAttrib);
    writeU32_Stream(stream, d->relOffset);
}

struct Impl_CentralEnd {
    uint16_t disk;
    uint16_t centralStartDisk;
    uint16_t diskEntryCount;
    uint16_t totalEntryCount;
    uint32_t size;
    uint32_t offset;
    uint16_t commentSize;
};

static void read_CentralEnd_(iCentralEnd *d, iStream *stream) {
    d->disk             = readU16_Stream(stream);
    d->centralStartDisk = readU16_Stream(stream);
    d->diskEntryCount   = readU16_Stream(stream);
    d->totalEntryCount  = readU16_Stream(stream);
    d->size             = readU32_Stream(stream);
    d->offset           = readU32_Stream(stream);
    d->commentSize      = readU16_Stream(stream);
}

static void write_CentralEnd_(const iCentralEnd *d, iStream *stream) {
    writeU16_Stream(stream, d->disk);
    writeU16_Stream(stream, d->centralStartDisk);
    writeU16_Stream(stream, d->diskEntryCount);
    writeU16_Stream(stream, d->totalEntryCount);
    writeU32_Stream(stream, d->size);
    writeU32_Stream(stream, d->offset);
    writeU16_Stream(stream, d->commentSize);
}

static iBool seekToCentralEnd_(iStream *stream) {
    /* Locate the central directory. Start from the earliest location where
     * the signature might be. */
    for (size_t pos = CENTRAL_END_SIZE; pos < MAXIMUM_COMMENT_SIZE + CENTRAL_END_SIZE; pos++) {
        seek_Stream(stream, size_Stream(stream) - pos);
        uint32_t signature = readU32_Stream(stream);
        if (signature == SIG_END_OF_CENTRAL_DIR) {
            return iTrue;
        }
    }
    return iFalse;
}

/*----------------------------------------------------------------------------------------------*/

iDefineTypeConstruction(ArchiveEntry)

void init_ArchiveEntry(iArchiveEntry *d) {
    init_String(&d->path);
    d->size = 0;
    iZap(d->timestamp);
    d->archPos = 0;
    d->archSize = 0;
    d->compression = 0;
    d->data = NULL;
}

void deinit_ArchiveEntry(iArchiveEntry *d) {
    delete_Block(d->data);
    deinit_String(&d->path);
}

static int cmp_ArchiveEntry_(const void *a, const void *b) {
    const iArchiveEntry *e1 = a, *e2 = b;
    return cmpString_String(&e1->path, &e2->path);
}

/*----------------------------------------------------------------------------------------------*/

struct Impl_Archive {
    iObject       object;
    iFile *       sourceFile;
    iBuffer *     sourceBuffer;
    iBool         isWritable;
    iSortedArray *entries; /* sorted by path */
};

iDefineObjectConstruction(Archive)

static iStream *source_Archive_(const iArchive *d) {
    if (d->sourceFile) {
        return stream_File(d->sourceFile);
     }
     if (d->sourceBuffer) {
         return stream_Buffer(d->sourceBuffer);
     }
     return NULL;
}

static iBool readDirectory_Archive_(iArchive *d) {
    iStream *is = source_Archive_(d);
    /* Is this a ZIP archive? */
    seek_Stream(is, 0);
    const uint32_t magic = readU32_Stream(is);
    if (magic != SIG_LOCAL_FILE_HEADER) {
        /* Does not look like a ZIP file. */
        return iFalse;
    }
    if (!seekToCentralEnd_(is)) {
        iDebug("[Archive] central directory not found\n");
        return iFalse;
    }
    /* Read the central directory. */
    iCentralEnd cend;
    read_CentralEnd_(&cend, is);
    /* Only one-part ZIPs are supported. */
    const size_t entryCount = cend.totalEntryCount;
    if (entryCount != cend.diskEntryCount) {
        iDebug("[Archive] multipart ZIPs not supported\n");
        return iFalse;
    }
    seek_Stream(is, cend.offset);
    iBool ok = iTrue;
    iString path;
    init_String(&path);
    for (size_t index = 0; index < entryCount; index++) {
        iCentralFileHeader header;
        read_CentralFileHeader_(&header, is);
        if (header.signature != SIG_CENTRAL_FILE_HEADER) {
            iDebug("[Archive] corrupt central directory\n");
            ok = iFalse;
            break;
        }
        resize_Block(&path.chars, header.fileNameSize);
        readData_Stream(is, header.fileNameSize, data_Block(&path.chars));
        seek_Stream(is, pos_Stream(is) + header.extraFieldSize + header.commentSize);
        /* Skip directories. */
        if (!endsWith_String(&path, "/") || header.size > 0) {
            if (header.compression != none_Compression &&
                header.compression != deflated_Compression) {
                iDebug("[Archive] unsupported type of compression\n");
                ok = iFalse;
                break;
            }
            if (header.flags & ZFH_ENCRYPTED) {
                iDebug("[Archive] archive uses encryption\n");
                ok = iFalse;
                break;
            }
            iArchiveEntry entry;
            init_ArchiveEntry(&entry);
            set_String(&entry.path, &path);
            entry.size        = header.size;
            entry.archSize    = header.compressedSize;
            entry.compression = header.compression;
            entry.crc32       = header.crc32;
            /* Last modified time. */ {
                iDOSDate lastModDate;
                iDOSTime lastModTime;
                init_DOSDate_(&lastModDate, header.lastModDate);
                init_DOSTime_(&lastModTime, header.lastModTime);
                init_Time(&entry.timestamp,
                          &(iDate){ .year   = lastModDate.year + 1980,
                                    .month  = lastModDate.month,
                                    .day    = lastModDate.dayOfMonth,
                                    .hour   = lastModTime.hours,
                                    .minute = lastModTime.minutes,
                                    .second = lastModTime.seconds });
            }
            size_t oldPos = pos_Stream(is);
            /* Read the local header to find out where the data is. */ {
                iLocalFileHeader localHeader;
                seek_Stream(is, header.relOffset);
                read_LocalFileHeader_(&localHeader, is);
                entry.archPos =
                    pos_Stream(is) + localHeader.fileNameSize + localHeader.extraFieldSize;
            }
            seek_Stream(is, oldPos);
            insert_SortedArray(d->entries, &entry);
        }
    }
    deinit_String(&path);
    return ok;
}

static size_t findPath_Archive_(const iArchive *d, const iString *path) {
    iArchiveEntry entry;
    initCopy_String(&entry.path, path);
    replace_String(&entry.path, "\\", "/"); /* in case it's a Windows-style path */
    size_t pos;
    if (!locate_SortedArray(d->entries, &entry, &pos)) {
        pos = iInvalidPos;
    }
    deinit_String(&entry.path);
    return pos;
}

static size_t findOrAddEntry_Archive_(iArchive *d, const iString *path) {
    iAssert(d->isWritable);
    iArchiveEntry entry;
    iZap(entry);
    initCopy_String(&entry.path, path);
    replace_String(&entry.path, "\\", "/"); /* in case it's a Windows-style path */
    if (!contains_SortedArray(d->entries, &entry)) {
        iArchiveEntry newEntry;
        init_ArchiveEntry(&newEntry);
        set_String(&newEntry.path, &entry.path);
        insert_SortedArray(d->entries, &newEntry);
    }
    size_t pos;
    locate_SortedArray(d->entries, &entry, &pos);
    deinit_String(&entry.path);
    return pos;
}

static iArchiveEntry *loadEntry_Archive_(const iArchive *d, size_t index) {
    iArchiveEntry *entry = at_SortedArray(d->entries, index);
    if (!entry->data || size_Block(entry->data) != entry->size) {
        if (entry->data) {
            delete_Block(entry->data);
            entry->data = NULL;
        }
        /* Load it now. */
        iStream *is = source_Archive_(d);
        seek_Stream(is, entry->archPos);
        iBlock *arch = read_Stream(is, entry->archSize);
        if (entry->compression == deflated_Compression) {
            entry->data = decompress_Block(arch);
            delete_Block(arch);
        }
        else {
            entry->data = arch;
        }
#if defined (iHaveDebugOutput)
        const uint32_t checksum = crc32_Block(entry->data);
        if (checksum != entry->crc32) {
            iWarning("[Archive] failed checksum on entry: %s\n", cstr_String(&entry->path));
        }
#endif
    }
    return entry;
}

void init_Archive(iArchive *d) {
    d->sourceFile   = NULL;
    d->sourceBuffer = NULL;
    d->isWritable   = iFalse;
    d->entries      = new_SortedArray(sizeof(iArchiveEntry), cmp_ArchiveEntry_);
}

void deinit_Archive(iArchive *d) {
    close_Archive(d);
    delete_SortedArray(d->entries);
}

iBool openData_Archive(iArchive *d, const iBlock *data) {
    close_Archive(d);
    d->sourceBuffer = new_Buffer();
    open_Buffer(d->sourceBuffer, data);
    return readDirectory_Archive_(d);
}

iBool openFile_Archive(iArchive *d, const iString *path) {
    close_Archive(d);
    d->sourceFile = new_File(path);
    if (!open_File(d->sourceFile, readOnly_FileMode)) {
        iReleasePtr(&d->sourceFile);
        return iFalse;
    }
    return readDirectory_Archive_(d);
}

void openWritable_Archive(iArchive *d) {
    close_Archive(d);
    d->isWritable = iTrue;
}

void close_Archive(iArchive *d) {
    iForEach(Array, i, &d->entries->values) {
        deinit_ArchiveEntry(i.value);
    }
    clear_SortedArray(d->entries);
    iReleasePtr(&d->sourceBuffer);
    iReleasePtr(&d->sourceFile);
    d->isWritable = iFalse;
}

iBool isOpen_Archive(const iArchive *d) {
    return d && (d->isWritable || source_Archive_(d) != NULL);
}

size_t numEntries_Archive(const iArchive *d) {
    return size_SortedArray(d->entries);
}

size_t sourceSize_Archive(const iArchive *d) {
    if (source_Archive_(d)) {
        return size_Stream(source_Archive_(d));
    }
    return 0;
}

static const iString *normalize_Path_(const iString *path) {
#if defined (iPlatformMsys) || defined (iPlatformWindows)
    /* Normalize separators. */
    iString *norm = copy_String(path);
    replace_String(norm, "\\", "/");
    return collect_String(norm);
#else
    return path;
#endif
}

iBool isDirectory_Archive(const iArchive *d, const iString *path) {
    if (isEmpty_String(path)) {
        return iTrue; /* root */
    }
    path = normalize_Path_(path);
    size_t pos = iInvalidPos;
    iArchiveEntry entry;
    initCopy_String(&entry.path, path);
    locate_SortedArray(d->entries, &entry, &pos);
    deinit_String(&entry.path);
    if (pos < size_SortedArray(d->entries)) {
        const iArchiveEntry *match = constAt_SortedArray(d->entries, pos);
        if (size_String(&match->path) > size_String(path)) {
            return startsWith_String(&match->path, cstr_String(path));
        }
    }
    return iFalse;
}

iStringSet *listDirectory_Archive(const iArchive *d, const iString *dirPath) {
    iStringSet *paths = new_StringSet();
    dirPath = normalize_Path_(dirPath);
    iString path;
    init_String(&path);
    const iBool isRoot = isEmpty_String(dirPath);
    iConstForEach(Array, i, &d->entries->values) {
        const iArchiveEntry *entry = i.value;
        iRangecc entryDir = dirNameSep_Path(&entry->path, "/");
        if (*entryDir.end == '/') {
            entryDir.end++; /* include the slash */
        }
        if (!cmp_Rangecc(entryDir, cstr_String(dirPath)) ||
            (isRoot && equal_Rangecc(entryDir, "."))) {
            insert_StringSet(paths, &entry->path);
        }
        else if (startsWith_Rangecc(entryDir, cstr_String(dirPath))) {
            /* A subdirectory. */
            size_t nextSlash = indexOfCStrFrom_String(&entry->path, "/", size_String(dirPath));
            if (nextSlash != iInvalidPos) {
                set_String(&path, dirPath);
                appendRange_String(
                    &path,
                    (iRangecc){ constBegin_String(&entry->path) + size_String(dirPath),
                                constBegin_String(&entry->path) + nextSlash + 1 });
                insert_StringSet(paths, &path);
            }
        }
    }
    deinit_String(&path);
    return paths;
}

const iArchiveEntry *entryAt_Archive(const iArchive *d, size_t index) {
    if (index >= size_SortedArray(d->entries)) {
        return NULL;
    }
    return constAt_SortedArray(d->entries, index);
}

static iArchiveEntry *writableEntryAt_Archive_(iArchive *d, size_t index) {
    iAssert(d->isWritable);
    if (index >= size_SortedArray(d->entries)) {
        return NULL;
    }
    return at_SortedArray(d->entries, index);
}

const iArchiveEntry *entry_Archive(const iArchive *d, const iString *path) {
    return entryAt_Archive(d, findPath_Archive_(d, path));
}

const iArchiveEntry *entryCStr_Archive(const iArchive *d, const char *pathCStr) {
    return entry_Archive(d, &iStringLiteral(pathCStr)); /* string used for lookup; not retained */
}

const iBlock *dataAt_Archive(const iArchive *d, size_t index) {
    if (index >= size_SortedArray(d->entries)) {
        return NULL;
    }
    return loadEntry_Archive_(d, index)->data;
}

const iBlock *data_Archive(const iArchive *d, const iString *path) {
    return dataAt_Archive(d, findPath_Archive_(d, path));
}

const iBlock *dataCStr_Archive(const iArchive *d, const char *pathCStr) {
    return data_Archive(d, &iStringLiteral(pathCStr)); /* string used for lookup; not retained */
}

void setData_Archive(iArchive *d, const iString *path, const iBlock *data) {
    if (d->isWritable) {
        iArchiveEntry *entry = writableEntryAt_Archive_(d, findOrAddEntry_Archive_(d, path));
        initCurrent_Time(&entry->timestamp);
        if (!entry->data) {
            entry->data = copy_Block(data);
        }
        else {
            set_Block(entry->data, data);
        }
        entry->crc32 = crc32_Block(data);
        entry->size  = size_Block(data);
        /* The data is compressed when the Archive is serialized. */
    }
}

void setDataCStr_Archive(iArchive *d, const char *path, const iBlock *data) {
    iString pathStr;
    initCStr_String(&pathStr, path);
    setData_Archive(d, &pathStr, data);
    deinit_String(&pathStr);
}

void serialize_Archive(const iArchive *d, iStream *out) {
    /* Structure:       
            LocalFileHeader + fileName + data, ...
            CentralFileHeader + fileName, ...
            CentralEnd */    
    const size_t numEntries = size_SortedArray(d->entries);
    iArray centralDir;
    init_Array(&centralDir, sizeof(iCentralFileHeader));
    resize_Array(&centralDir, numEntries);
    iConstForEach(Array, i, &d->entries->values) {
        const iArchiveEntry *entry = i.value;
        iLocalFileHeader local;
        iDate ts;
        init_Date(&ts, &entry->timestamp);
        iZap(local);
        local.signature   = SIG_LOCAL_FILE_HEADER;
        local.crc32       = entry->crc32;
        local.size        = (uint32_t) entry->size;
        local.lastModDate = packed_DOSDate_(&(iDOSDate){ .dayOfMonth = ts.day,
                                                         .month      = ts.month,
                                                         .year       = ts.year - 1980 });
        local.lastModTime = packed_DOSTime_(&(iDOSTime){ .hours      = ts.hour,
                                                         .minutes    = ts.minute,
                                                         .seconds    = ts.second });
        local.fileNameSize = size_String(&entry->path);
        iAssert(entry->data);
        iBlock *comp = compress_Block(entry->data);
        if (size_Block(comp) < entry->size) {
            local.compression = deflated_Compression;
            local.compressedSize = (uint32_t) size_Block(comp);
        }
        else {
            local.compression = none_Compression;
            local.compressedSize = (uint32_t) entry->size;
            set_Block(comp, entry->data);
        }
        /* Also prepare the central file header with the same information. */
        iCentralFileHeader *central = at_Array(&centralDir, index_ArrayConstIterator(&i));
        iZap(*central);
        central->signature = SIG_CENTRAL_FILE_HEADER;
        central->compressedSize = local.compressedSize;
        central->compression = local.compression;
        central->crc32 = local.crc32;
        central->fileNameSize = local.fileNameSize;
        central->lastModDate = local.lastModDate;
        central->lastModTime = local.lastModTime;
        central->size = local.size;
        central->relOffset = (uint32_t) pos_Stream(out);
        write_LocalFileHeader_(&local, out);
        write_Stream(out, utf8_String(&entry->path));
        write_Stream(out, comp);
        delete_Block(comp);
    }
    const size_t centralStartOffset = pos_Stream(out);
    iForEach(Array, j, &centralDir) {
        iCentralFileHeader *central = j.value;
        const iArchiveEntry *entry = constAt_SortedArray(d->entries, index_ArrayIterator(&j));
        write_CentralFileHeader_(central, out);
        write_Stream(out, utf8_String(&entry->path));
    }
    writeU32_Stream(out, SIG_END_OF_CENTRAL_DIR);
    write_CentralEnd_(
        &(iCentralEnd){
            .diskEntryCount  = numEntries,
            .totalEntryCount = numEntries,
            .size            = (uint32_t) (pos_Stream(out) - centralStartOffset - 4),
            .offset          = (uint32_t) centralStartOffset,
        },
        out);
    deinit_Array(&centralDir);
}

/*----------------------------------------------------------------------------------------------*/

void init_ArchiveConstIterator(iArchiveConstIterator *d, const iArchive *archive) {
    if (archive) {
        d->archive = archive;
        d->index   = 0;
        d->value   = entryAt_Archive(archive, 0);
    }
    else {
        iZap(*d);
    }
}

void next_ArchiveConstIterator(iArchiveConstIterator *d) {
    if (d->archive && d->value) {
        d->value = entryAt_Archive(d->archive, ++d->index);
    }
}

iDefineClass(Archive)
