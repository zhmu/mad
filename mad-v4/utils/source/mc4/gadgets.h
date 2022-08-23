/*
 *                             The MAD Compiler
 *                                Version 4.0
 *
 *                        (c) 1999, 2000 The MAD Crew
 *
 *                          Gadgets include file
 *
 */
#include "../../../source/include/types.h"
#include "../../../source/include/script.h"

_UCHAR gad_resolvetype(char*);

void gad_copynochar(char*,char*,_UCHAR);
void gad_copyuntil(char*,char*,char);
void gad_striptrailingspaces(char*);
void gad_copynostring(char*,char*,char,char);
char* gad_scanlastfrombegin(char*,char);
_UINT gad_resolveparamstring(char*);
char* gad_scanforoperator(char*,_UCHAR*);
_UCHAR gad_resolvedirection(char*);
void gad_getbasename(char*,char*);
_UCHAR gad_getnamebetween(char*,char*,_UCHAR,_UCHAR);
_UINT gad_resolveparamstring(char*);
_UCHAR gad_resolveboolean(char*);
char* gad_scanforcomperator(char*,_UCHAR*,_UCHAR*);
