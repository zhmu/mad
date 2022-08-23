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
#include "mc.h"
#include "gadgets.h"
#include "../../../source/script.h"
#include "../../../source/types.h"

/*
 * scanstring(char* string,char* scanfor)
 *
 * This will scan string [string] for string [scanfor]. If it is found,
 * this function will return the position of it in [string], otherwise
 * GADGETS_NONUM is returned.
 *
 *
 */
_ULONG
scanstring(char* string,char* scanfor) {
    _ULONG i,j,count;

    /* scan for it */
    for(i=0;i<=strlen(string)-strlen(scanfor);i++) {
        /* check it */
        count=0;
        for(j=0;j<strlen(scanfor);j++) {
            if(string[i+j]==scanfor[j]) {
                count++;
            }
        }
        /* did we have a match? */
        if(count==j) {
             /* yeah, return this position */
             return i;
        }
    }
    return GADGETS_NONUM;
}

/*
 * stripquotes(char* source,char* dest)
 *
 * This will remove the quotes from [source] and dump the result in [dest].
 * e.g. "this" will become this. (but without the dot :-)
 *
 */
void
stripquotes(char* source,char* dest) {
    _UINT i,pos;
    _UCHAR active,ch;

    /* if no quotes, just copy [source] to [dest] and return */
    if(strchr(source,'"')==NULL) {
        strcpy(dest,source);
        return;
    }

    active=0; pos=0; strcpy(dest,"");
    for(i=0;i<strlen(source);i++) {
        ch=source[i];
        if (ch=='"') {
            if (active) {
                dest[pos]=0;
                return;
            } else {
                active=1;
            }
        }
        if ((active)&&(ch!='"')) {
            dest[pos]=ch;
            pos++;
        }
    }
    dest[pos]=0;
}

/*
 * resolveparamtype(char* param)
 *
 * This will resolve [param] into a number, one of the MC_TYPE_[xxx] defines.
 *
 */
_UINT
resolveparamtype(char* param) {
    /* check string */
    if(!strcmp(param,"\%s")) return MC_TYPE_STRING;
    /* check unsigned integer */
    if(!strcmp(param,"\%u")) return MC_TYPE_UINT;
    /* check signed integer */
    if(!strcmp(param,"\%i")) return MC_TYPE_SINT;
    /* check evaluated unsigned integer */
    if(!strcmp(param,"\%e")) return MC_TYPE_EUINT;
    /* check quoted string */
    if(!strcmp(param,"\%q")) return MC_TYPE_QSTRING;
    /* check direction */
    if(!strcmp(param,"\%d")) return MC_TYPE_DIRECTION;
    /* check boolean */
    if(!strcmp(param,"\%b")) return MC_TYPE_BOOLEAN;
    /* check text block */
    if(!strcmp(param,"\%t")) return MC_TYPE_TEXTBLOCK;
    /* check label */
    if(!strcmp(param,"\%l")) return MC_TYPE_LABEL;
    /* check double word */
    if(!strcmp(param,"\%w")) return MC_TYPE_DWORD;

    /* we don't know this type. return MC_TYPE_UNKNOWN */
    return MC_TYPE_UNKNOWN;
}

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

/*
 *  _UCHAR resolvedirection(char* direction)
 *
 *  this will try to resolve direction [direction]. it will return
 *  one of the DIR_xxx defines.
 *
 */
_UCHAR
resolvedirection(char* direction) {
    if(!strcmp(direction,"north")) return DIR_NORTH;
    if(!strcmp(direction,"east")) return DIR_EAST;
    if(!strcmp(direction,"south")) return DIR_SOUTH;
    if(!strcmp(direction,"west")) return DIR_WEST;

    if(!strcmp(direction,"top")) return DIR_NORTH;
    if(!strcmp(direction,"bottom")) return DIR_SOUTH;
    if(!strcmp(direction,"no")) return DIR_NONE;
    return DIR_UNKNOWN;
}

/*
 *  _UCHAR resolveboolean(char* expr)
 *
 *  this will try to resolve boolean [expr]. it will return one of the
 *  MC_BOOL_xxx defines., BOOL_NO
 *
 */
_UCHAR
resolveboolean(char* expr) {
    if(!strcmp(expr,"yes")) return MC_BOOL_TRUE;
    if(!strcmp(expr,"no")) return MC_BOOL_FALSE;
    if(!strcmp(expr,"true")) return MC_BOOL_TRUE;
    if(!strcmp(expr,"false")) return MC_BOOL_FALSE;
    return MC_BOOL_UNKNOWN;
}

/*
 * insert(char* s,int pos,_UCHAR ch)
 *
 * This will insert char [ch] in string [s] at position [pos].
 *
 */
void
insert(char* s,int pos,_UCHAR ch) {
    int i;
    char temp[MC_MAX_SCRIPTLINE_LEN];

    strcpy(temp,s);

    for(i=pos+1;i<strlen(s)+1;i++) {
        s[i]=temp[i-1];
    }
    s[pos]=ch;
}
