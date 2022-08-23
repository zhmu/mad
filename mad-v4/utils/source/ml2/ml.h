#include "../mc2/mc.h"

#define ML_MAX_FILES 128
#define ML_MAX_FILENAME_LEN 16

#define ML_MAX_SHARED_OBJECTS 64
#define ML_MAX_LOCAL_OBJECTS 64
#define ML_MAX_PROCS 64

#define ML_NAME_CSCRIPT "*"MC_EXT_CSCRIPT

#define ML_NOBJECT 0xffffffff

#define ML_VERSIONO 200

void getbasename(char*,char*);

struct ML_OBJECT {
    char         name[MC_MAX_OBJECT_LEN];
    _UCHAR       type;
    _ULONG       flags;
    _ULONG       nofchanges;
};

struct ML_PROC {
    char         name[MC_MAX_PROCNAME_LEN];
    _ULONG       startbyte;
    _ULONG       endbyte;
};
