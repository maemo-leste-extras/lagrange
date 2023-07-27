#include <the_Foundation/archive.h>
#include <the_Foundation/commandline.h>
#include <the_Foundation/file.h>

int main(int argc, char **argv) {
    init_Foundation();
    iCommandLine *args = iClob(new_CommandLine(argc, argv));
    defineValues_CommandLine(args, "e;extract", 1);
    defineValues_CommandLine(args, "c;create", 1);
    iArchive *create = NULL;
    iString *createPath = NULL;
    const iCommandLineArg *arg = checkArgument_CommandLine(args, "c;create");
    if (arg) {
        create = new_Archive();
        openWritable_Archive(create);
        createPath = copy_String(value_CommandLineArg(arg, 0));
    }
    iConstForEach(CommandLine, i, args) {
        if (i.argType != value_CommandLineArgType) {
            continue;
        }
        const iString *path = value_CommandLineConstIterator(&i);
        if (path) {
            if (create) {
                iFile *f = new_File(path);
                if (open_File(f, readOnly_FileMode)) {
                    iBlock *data = readAll_File(f);
                    printf("%s: %zu bytes\n", cstr_String(path), size_Block(data));
                    setData_Archive(create, path, data);
                    delete_Block(data);
                }
                else {
                    printf("%s: failed to read\n", cstr_String(path));
                }
                iRelease(f);
                continue;
            }
            iArchive *arch = new_Archive();            
            printf("opening: %s\n", cstr_String(path));
            if (openFile_Archive(arch, path)) {
                arg = iClob(checkArgument_CommandLine(args, "e;extract"));
                if (arg) {
                    const iString *entryPath = value_CommandLineArg(arg, 0);
                    printf("decompressing: %s\n", cstr_String(entryPath));
                    const iBlock *data = data_Archive(arch, entryPath);
                    printf("got %zu bytes\n", size_Block(data));
                    fwrite(constData_Block(data), size_Block(data), 1, stderr);
                    continue;
                }
                printf("%zu entries\n", numEntries_Archive(arch));
                iConstForEach(Archive, j, arch) {
                    const iArchiveEntry *entry = j.value;
                    printf("%s\n%8zu %s [%08X] (%s, at %zu:%zu)\n\n",
                           cstr_String(&entry->path),
                           entry->size,
                           cstrCollect_String(format_Time(&entry->timestamp, "%Y-%m-%d %H:%M:%S")),
                           entry->crc32,
                           entry->compression == 0 ? "not compressed" : "compressed",
                           entry->archPos,
                           entry->archSize);
                }
            }
            else {
                printf("failed to open %s\n", cstr_String(path));
            }
            iRelease(arch);
        }
    }
    if (create) {
        iFile *f = new_File(createPath);
        open_File(f, writeOnly_FileMode);
        serialize_Archive(create, stream_File(f));
        close_File(f);
        printf("%zu entries\n", numEntries_Archive(create));
        printf("wrote %s\n", cstr_String(createPath));
        delete_String(createPath);
        iRelease(create);
    }
    deinit_Foundation();
    return 0;
}
