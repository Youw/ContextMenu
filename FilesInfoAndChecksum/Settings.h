#ifndef Settings_h
#define Settings_h

#define MenuText_CalcChecksum "&Make log of file(s) info"
#define Verb_CalcChecksum "makeloginfochecksum"
#define VerbCanonicalName_CalcChecksum "MakeLogInfoChecksum"
#define VerbHelpText_CalcChecksum "Save checksums of files to log"

#define HandlerFullName "FilesInfoAndChecksum.FiacsContextMenuHlr"

//textual concationation
#define CONCAT(x, y) x##y
//to convert some X , where X - '#define x "some string"' to L"some string"
#define MAKEWIDE(x) CONCAT(L,x)

#endif //Settings_h