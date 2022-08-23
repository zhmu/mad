/*
 *                             The MAD Compiler
 *                                Version 3.0
 *
 *                          (c) 1999 The MAD Crew
 *
 *                           Gadgets include file
 *
 */
#include "../../../source/types.h"
#include "../../../source/script.h"

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
