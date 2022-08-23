#include "../../../source/types.h"

void stripquotes (char *instr,char *fname);
void lineout(char *instr,_UINT no);

_UCHAR readline(_ULONG lineno,_UINT filesize,char *s,char *data);
char *readfile(char *fname,_ULONG *length);
void writefile(char *fname,_ULONG size,char *buf);
void constructfilenames(char *base,char *out,char *map,char *loc);
_UCHAR resolvedirection(char *direction);
_UCHAR resolveboolean(char *expr);
