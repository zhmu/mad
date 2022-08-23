/*
 *                             The MAD Compiler
 *                         Some handy gadget routines
 *                               Version 4.0
 *
 *                          (c) 1999 The MAD Crew
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mc.h"
#include "../../../source/include/objman.h"
#include "../../../source/include/script.h"
#include "../../../source/include/types.h"

/*
 * gad_copynochar(char* source,char* dest,_UCHAR ch)
 *
 * This will copy [source] to [dest], ignoring char [ch]. It will stop at a
 * nul.
 *
 */
void
gad_copynochar(char* source,char* dest,_UCHAR ch) {
    _ULONG n,i;

    n=0;i=0;
    while(source[i]!='\0') {
        /* not [ch]? */
        if(source[i]!=ch) {
            /* yeh, add it */
            dest[n]=source[i];
            n++;
        }
        /* next char */
        i++;
    }
    dest[n]=0;
}

/*
 * gad_copyuntil(char* source,char* dest,char ch)
 *
 * This will copy [source] to [dest], stopping at either [ch] or nul.
 *
 */
void
gad_copyuntil(char* source,char* dest,char ch) {
    _ULONG i;

    i=0;
    while((source[i]!=ch)&&(source[i]!=0)) {
        /* it's not [ch] or nul. add it */
        dest[i]=source[i];
        i++;
    }
    /* add a null (just in case) */
    dest[i]=0;
}

/*
 * gad_striptrailingspaces(char* dest)
 *
 * This will string any trailing spaces from [dest].
 *
 */
void
gad_striptrailingspaces(char* dest) {
    _UINT i;

    /* if the line is empty, just return */
    if(!strlen(dest)) return;

    i=strlen(dest)-1;
    while (dest[i]==' ') {
        /* there's a space. nuke it */
        dest[i]=0;

        /* next char (actually previous one) */
        i--;
    }

    while(dest[0]==' ') {
        /* there's a space in the beginning! nuke it */
        memmove(dest,dest+1,strlen(dest));
    }
}

/*
 * gad_copynostring(char* source,char* dest,char inchar,char outchar)
 *
 * This will copy everything from [source] to [dest], but it will change each
 * [inchar] char to [outchar]. It will not do this in a " pair.
 *
 */
void
gad_copynostring(char* source,char* dest,char inchar,char outchar) {
    _ULONG i,n;
    _UCHAR instr;

    instr=0;n=0;
    for(i=0;i<strlen(source);i++) {
        /* is it a string char (")? */
        if(source[i]=='"') {
            /* yeh. toggle flag */
            instr^=1;
        }
        if(!instr) {
            if(source[i]==inchar) {
                dest[n]=outchar;
            } else {
                dest[n]=source[i];
            }
            n++;
        } else {
            dest[n]=source[i];
            n++;
        }
    }
    /* add the nul */
    dest[n]=0;
}

/*
 * gad_scanlastfrombegin(char* source,char ch)
 *
 * This will return a pointer to the last occourence of [ch], scanning from
 * the beginning. it will stop at a char that is not [ch].
 *
 */
char*
gad_scanlastfrombegin(char* source,char ch) {
    _UINT i;

    for(i=0;i<strlen(source);i++) {
        if(source[i]!=ch) {
            return (source+i);
        }
    }
    /* the complete string is of [ch]'s! */
    return source;
}

/*
 * gad_resolveparamstring(char* paramtype)
 *
 * This will resolve parameter type string [paramtype] to a MC_TYPE_xxx
 * constant. It will return MC_TYPE_UNKNOWN if it does not recognise the type.
 *
 */
_UINT
gad_resolveparamstring(char* paramtype) {
    /* check string */
    if(!strcmp(paramtype,"\%s")) return MC_TYPE_STRING;
    /* check evaluated long */
    if(!strcmp(paramtype,"\%e")) return MC_TYPE_ELONG;
    /* check quoted string */
    if(!strcmp(paramtype,"\%q")) return MC_TYPE_QSTRING;
    /* check direction */
    if(!strcmp(paramtype,"\%d")) return MC_TYPE_DIRECTION;
    /* check boolean */
    if(!strcmp(paramtype,"\%b")) return MC_TYPE_BOOLEAN;
    /* check text block */
    if(!strcmp(paramtype,"\%t")) return MC_TYPE_TEXTBLOCK;
    /* check label */
    if(!strcmp(paramtype,"\%l")) return MC_TYPE_LABEL;
    /* check double word */
    if(!strcmp(paramtype,"\%w")) return MC_TYPE_DWORD;
    /* check property */
    if(!strcmp(paramtype,"\%p")) return MC_TYPE_PROP;
    /* check object-property */
    if(!strcmp(paramtype,"\%f")) return MC_TYPE_OBJPROP;

    /* we don't know this type. return MC_TYPE_UNKNOWN */
    return MC_TYPE_UNKNOWN;
}

/*
 * gad_scanforoperator(char* string,char* expr)
 *
 * This will scan for an operator. It will return a pointer to it or NULL of
 * nothing was found. [expr] will be set to the type of EXPR_xxx or EXPR_NONE
 * if nothing was found.
 *
 */
char*
gad_scanforoperator(char* string,_UCHAR *expr) {
    char* ptr;

    /* scan for a plus */
    ptr=strchr(string,'+');
    if(ptr!=NULL) {
        *expr=EXPR_ADD;
        return ptr;
    }
    /* scan for a minus */
    ptr=strchr(string,'-');
    if(ptr!=NULL) {
        *expr=EXPR_SUB;
        return ptr;
    }
    /* scan for a multiply */
    ptr=strchr(string,'*');
    if(ptr!=NULL) {
        *expr=EXPR_MUL;
        return ptr;
    }
    /* scan for a division */
    ptr=strchr(string,'/');
    if(ptr!=NULL) {
        *expr=EXPR_DIV;
        return ptr;
    }

    /* nothing was found */
    *expr=EXPR_NONE;
    return NULL;
}

/*
 * gad_scanforcomperator(char* string,_UCHAR* comp,_UCHAR len)
 *
 * This will scan for a comperator in [string]. It'll return a pointer to it,
 * and set [comp] to the value of it. If nothing was found, [comp] will be
 * MC_COMP_UNKNOWN and the return value will be null. [len] will be set to
 * the length of the operator string.
 *
 */
char*
gad_scanforcomperator(char* string,_UCHAR* comp,_UCHAR* len) {
    char* ptr;

    /* scan for a '==' */
    if((ptr=strchr(string,'='))!=NULL) {
        /* we found a single '='. Maybe there's a '=' after it? */
        if(*(ptr+1)=='=') {
            /* yup. it's equal! */
            *comp=MC_COMP_EQUAL; *len=2;
            return ptr;
        }
        /* maybe a '!' before it? */
        if(*(ptr-1)=='!') {
            /* yup. it's not equal! */
            *comp=MC_COMP_NOTEQUAL; *len=2;
            return ptr;
        }
        /* maybe a '<' before it? */
        if(*(ptr-1)=='<') {
            /* yup. it's not equal! */
            *comp=MC_COMP_GE; *len=2;
            return ptr;
        }
        /* maybe a '>' before it? */
        if(*(ptr-1)=='>') {
            /* yup. it's not equal! */
            *comp=MC_COMP_LE; *len=2;
            return ptr;
        }
    }

    /* maybe '>'? */
    if((ptr=strchr(string,'>'))!=NULL) {
        /* yup. it's greater */
        *comp=MC_COMP_GT; *len=1;
        return ptr;
    }

    /* maybe '<'? */
    if((ptr=strchr(string,'<'))!=NULL) {
        /* yup. it's lesser */
        *comp=MC_COMP_LT; *len=1;
        return ptr;
    }

    /* nothing was found */
    *comp=MC_COMP_UNKNOWN; *len=0;
    return NULL;
}

/*
 * gad_resolvedirection(char* direction)
 *
 * this will try to resolve direction [direction]. it will return one of the
 * DIR_xxx constants;
 *
 */
_UCHAR
gad_resolvedirection(char* direction) {
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
 * gad_getbasename(char* source,char* dest)
 *
 * This will get the base name (complete filename without extension) from
 * [source] and put it in [dest].
 *
 */
void
gad_getbasename(char* source,char* dest) {
    gad_copyuntil(source,dest,'.');
}

/*
 * gad_getnamebetween(char* source,char* dest,_UCHAR startch,_UCHAR endch)
 *
 * This will return the name between [startch] and [endch] in string [source].
 * It will return the result in [dest]. It will return non-zero if successful
 * or zero if unsuccessful.
 *
 */
_UCHAR
gad_getnamebetween(char* source,char* dest,_UCHAR startch,_UCHAR endch) {
    char* ptr;

    /* first, scan for a [startch] in [source] */
    if((ptr=strchr(source,startch))==NULL) {
        /* not found. return zero */
        return 0;
    }
    /* scan for the end char */
    if(strchr((char*)(ptr+1),endch)==NULL) {
        /* not found. return zero */
        return 0;
    }
    /* copy everything until the end char to [dest] */
    gad_copyuntil((char*)(ptr+1),dest,endch);

    /* it went all ok! */
    return 1;
}

/*
 * gad_resolveboolean(char* expr)
 *
 * this will try to resolve boolean [expr]. it will return one of the
 * MC_BOOL_xxx defines.
 *
 */
_UCHAR
gad_resolveboolean(char* expr) {
    if(!strcmp(expr,"yes")) return MC_BOOL_TRUE;
    if(!strcmp(expr,"no")) return MC_BOOL_FALSE;
    if(!strcmp(expr,"true")) return MC_BOOL_TRUE;
    if(!strcmp(expr,"false")) return MC_BOOL_FALSE;
    return MC_BOOL_UNKNOWN;
}
