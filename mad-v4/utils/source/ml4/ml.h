#include "../mc4/mc.h"


#define ML_MAX_CLASSNAME_LEN MC_MAX_SUPERCLASS_LEN
#define ML_MAX_FILES 128
#define ML_MAX_FILENAME_LEN 16

#define ML_MAX_SHARED_OBJECTS 64
#define ML_MAX_LOCAL_OBJECTS 64
#define ML_MAX_PROCS 64

#define ML_NAME_SCREXT  ".cm"
#define ML_NAME_OBJEXT  ".m"
#define ML_NAME_CSCRIPT "*"ML_NAME_SCREXT

#define ML_NOBJECT 0xffffffff

#define ML_VERSIONO 400

void getbasename(char*,char*);

struct ML_METHOD {
     _ULONG offset;
     _ULONG flags;
};

struct ML_SUPERCLASS {
    char       name[ML_MAX_CLASSNAME_LEN];
    _ULONG     nofmethods;
    ML_METHOD* method;
};
