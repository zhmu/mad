#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../source/types.h"

/*
    char
    *readfile(char *fname,_ULONG *length)

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
        printf("unable to read '%s'\n",fname);
        return NULL;
    }
    fclose(f);
    return ptr;
}

/*
    void writefile(char *fname,_ULONG size,char *buf);

    this will write [size] bytes of buffer [buf] to file [fname]. it will
    print an error message and quit if anything fails
*/
void
writefile(char *fname,_ULONG size,char *buf) {
    FILE *f;

    if((f=fopen(fname,"wb"))==NULL) {
        printf("unable to create file '%s'\n",fname);
        exit(1);
    }
    if(!fwrite(buf,size,1,f)) {
        printf("unable to write '%s'\n",fname);
        exit(1);
    }
    fclose(f);
}

/*
    void getbasename(char *fname,char *dest)

    this will get the base (everything before the dot) of filename [fname].
    the result will be put in [dest]
*/
void
getbasename(char *fname,char *dest) {
    char *ptr;
    _UINT i,len;

    ptr=strchr(fname,'.'); len=strlen(fname);
    if (ptr!=NULL) {
        i=(ptr-fname);           /* i = position of the . */

        memset(dest,0,12);
        strncpy(dest,fname,i);
    } else {
        strcpy(dest,fname);
    }
}
