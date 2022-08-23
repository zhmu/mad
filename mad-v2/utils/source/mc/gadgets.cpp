#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "../../../source/script.h"
#include "../../../source/types.h"

/*

        stripquotes(char *source,char *dest)

        this will remove the quotes from a string, e.g. "this" will
        be come this. (without the dot of course)

*/
void
stripquotes (char *instr,char *fname) {
    _UINT i,pos;
    _UCHAR active,ch;

    /* if no quotes, just copy [instr] to [fname] and return */
    if(strchr(instr,'"')==NULL) {
        strcpy(fname,instr);
        return;
    }

	active=0; pos=0; strcpy(fname,"");
	for(i=0;i<strlen(instr);i++) {
		ch=instr[i];
		if (ch=='"') {
			if (active) {
			 fname[pos]=0;
			 return;
			} else {
				active=1;
			}
		}
		if ((active)&&(ch!='"')) {
			fname[pos]=ch;
			pos++;
		}
	}
	fname[pos]=0;
}

/*
    lineout(char *instr,UINT no)

    this will line out [instr] to [no] chars, e.g. "hello" 8 will become
    "hello   " (without the quotes)
*/
void
lineout(char *instr,_UINT no) {
    _UINT i;
    if(strlen(instr)>=no) return;
    
    while(strlen(instr)<no) {
        strcat(instr," ");
    }
}

/*
    UCHAR readline(ULONG lineno,UINT filesize,char *s,char *data)

    this will read line [lineno] from buffer [data] to [s]. it will return
    0 if everything went ok, or 1 if the line doesn't exist
*/
_UCHAR
readline(_ULONG lineno,_UINT filesize,char *s,char *data) {
    _ULONG i,line,c;

    /* scan the entire buffer */
    s[0]=0; line=0; c=0;
    for(i=0;i<filesize;i++) {
        /* are we scanning the line we search? */
        if (line==lineno) {
           /* yes, add the char to the return buffer */
           s[c]=data[i];
           c++;
        }
        /* is there a cr? */
        if (data[i]==0xd) {
            /* yes. we are done, remove newline, return */
            if (line==lineno) {
                /* next line removes the newline from the returned string */
                s[c-1]=0;
                /* we had success. return 1 */
                return 1;
            }
            /* skip lf, go to next line */
            i++;
		    line++;
        }
	}
	/* line doesn't exists. return 0 */
	return 0;
}


/*
    char
    *readfile(char *fname,ULONG *length)

    this will return a pointer to the file data of file [fname]. upon exit,
    this pointer must be freed by a call to free(). it will write an error
    message and return NULL if something went wrong. if length is not NULL,
    it will hold the size of the data read.
*/
char
*readfile(char *fname,_ULONG *length) {
    FILE *f;
    char *ptr;

    if((f=fopen(fname,"rb"))==NULL) {
        printf("unable to open '%s'\n",fname);
        return NULL;
    }
    fseek(f,0,SEEK_END);*length=ftell(f);rewind(f);
    if((ptr=(char *)malloc(*length))==NULL) {
        printf("out of memory when allocating memory for file '%s'\n",fname);
        return NULL;
    }
    if(!fread(ptr,*length,1,f)) {
        printf("read error when trying to read '%s'\n",fname);
        return NULL;
    }
    fclose(f);
    return ptr;
}

/*
    void writefile(char *fname,ULONG size,char *buf);

    this will write [size] bytes of buffer [buf] to file [fname]. it will
    print an error message and quit if anything fails
*/
void
writefile(char *fname,_ULONG size,char *buf) {
    FILE *f;

    if((f=fopen(fname,"wb"))==NULL) {
        printf("unable to create '%s'\n",fname);
        exit(1);
    }
    if(!fwrite(buf,size,1,f)) {
        printf("write error when writing to '%s'\n",fname);
        exit(1);
    }
    fclose(f);
}


/*
     void constructfilenames(char *base);

     this will construct filenames of the output, map and locator files.
     it will do this by getting base and stripping the extension and
     replacing it
*/
void
constructfilenames(char *base,char *out,char *map,char *loc) {
    char *ptr;
    _UINT i,len;

    ptr=strchr(base,'.'); len=strlen(base);
    if (ptr!=NULL) {
        i=(ptr-base);           /* i = position of the . */

        strncpy(out,base,i);
        strncpy(map,base,i);
        strncpy(loc,base,i);
   } else {
        strcpy(out,base);
        strcpy(map,base);
        strcpy(loc,base);
   }
   strcat(out,".cm");
   strcat(map,".mmp");
   strcat(loc,".mlc");
}

/*
    UCHAR resolvedirection(char *direction)

    this will try to resolve direction [direction]. it will return
    DIR_NORTH, DIR_SOUTH, DIR_EAST, DIR_WEST or DIR_UNKNOWN.
*/
_UCHAR
resolvedirection(char *direction) {
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
    UCHAR resolveboolean(char *expr)

    this will try to resolve boolean [expr]. it will return BOOL_YES, BOOL_NO
    or BOOL_UNKNOWN.
*/
_UCHAR
resolveboolean(char *expr) {
    if(!strcmp(expr,"yes")) return BOOL_TRUE;
    if(!strcmp(expr,"no")) return BOOL_FALSE;
    if(!strcmp(expr,"true")) return BOOL_TRUE;
    if(!strcmp(expr,"false")) return BOOL_FALSE;
    return BOOL_UNKNOWN;
}
