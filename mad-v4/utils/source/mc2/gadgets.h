/*
 *                             The MAD Compiler
 *                                Version 2.0
 *
 *                              Useful gadgets
 *
 *                          (c) 1999 The MAD Crew
 *
 */
#include "../../../source/types.h"

#define GADGETS_NONUM   0xffffffff

_ULONG scanstring(char*,char*);
void stripquotes(char*,char*);
_UINT resolveparamtype(char*);
void getbasename(char*,char*);
_UCHAR resolvedirection(char*);
_UCHAR resolveboolean(char*);
void insert(char*,int,_UCHAR);
