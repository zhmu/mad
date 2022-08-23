/*
 *                             The MAD Compiler
 *                                Version 2.0
 *
 *                              Useful gadgets
 *
 *                          (c) 1999 The MAD Crew
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ml.h"
#include "../../../source/include/script.h"
#include "../../../source/include/types.h"

/*
 * getbasename(char* fname,char* dest)
 *
 * This will get the base filename of [fname] and dump it in [dest].
 *
 */
void
getbasename(char* fname,char* dest) {
    _UINT i;
    char* ptr;

    /* scan for a dot in [fname] */
    ptr=strchr(fname,'.');

    if(ptr==NULL) {
        /* no dot, so just copy [fname] to [dest] */
        strcpy(dest,fname);
        return;
    }
    for(i=0;i<(ptr-fname);i++) {
        dest[i]=fname[i];
        dest[i+1]=0;
    }
}
