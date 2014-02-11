#ifndef SETTINGS_H
#define SETTINGS_H

#include <cstdint>

#define MenuText_CalcChecksum "&Make log of file(s) info"
#define Verb_CalcChecksum "makeloginfochecksum"
#define VerbCanonicalName_CalcChecksum "MakeLogInfoChecksum"
#define VerbHelpText_CalcChecksum "Save checksums of files to log"

#if UINTPTR_MAX==UINT64_MAX
#define HandlerFullName "FilesInfoAndChecksum.ContextMenuHlr_x64"
#else
#define HandlerFullName "FilesInfoAndChecksum.ContextMenuHlr_x32"
#endif

#define LogFileName "SelectedFilesInfo.log"

#define RUN_ASYNC true

//textual concationation
#define CONCAT(x, y) x##y
//to convert some X , where X - '#define x "some string"' to L"some string"
#define MAKEWIDE(x) CONCAT(L,x)

#endif //SETTINGS_H