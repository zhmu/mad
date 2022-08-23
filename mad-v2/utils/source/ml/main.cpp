#define RHIDE_COMPITABLE
#include <dir.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gadgets.h"
#include "../../../source/script.h"
#include "../../../source/types.h"

#define MAX_FILE_LEN 64
#define MAX_IDENTS 512
#define MAX_REFERENCES 512
#define NO_IDENTIFIER 0xffff
#define MAX_FILES 64
#define NO_FILE 0xffff

struct REFERENCE {
    _UINT  offset;
    _UINT  filenum;
};

struct IDENTIFIER {
    char  name[SCRIPT_MAXIDENTLEN];
    _UINT  nofreferences;
    struct REFERENCE reference[MAX_REFERENCES];
    _UCHAR type;
    _UCHAR internal;
    _UCHAR external;
    char  filename[MAX_FILE_LEN];
};

struct FILENTRY {
    char name[MAX_FILE_LEN];
};

struct IDENTIFIER identifier[MAX_IDENTS];
struct FILENTRY filentry[MAX_FILES];
_ULONG curident,curfilentry;

char *compiled_script;
_ULONG compiled_len;
char *locate_data;
_ULONG locate_len;

_ULONG
search_identifier(char *name) {
    _ULONG i;

    for(i=0;i<curident;i++) {
        if(!strcmp(name,identifier[i].name)) return i;
    }
    return NO_IDENTIFIER;
}

void
add_identifier(char *name,_UCHAR type,_UCHAR internal,_UCHAR external,_UINT filenum,_UCHAR nofoffsets,_UINT *offsets,char *filename) {
    _UINT i,no;

    if ((no=search_identifier(name))==NO_IDENTIFIER) {
        no=curident;
        curident++;
    }
    if(curident>=MAX_IDENTS) {
        printf("Error: cannot add identifier '%s', there are too many identifiers. %u is the maximum\n",name,MAX_IDENTS);
        exit(1);
    }
    strcpy(identifier[no].name,name);
//    printf("ADD %u: %s\n",no,name);
    identifier[no].type=type;
    identifier[no].internal=internal;

    if((external)&&(!identifier[no].external)) {
        identifier[no].external=external;
    }
    if((!external)&&(identifier[no].external)) {
        identifier[no].external=0;
    }
    if(nofoffsets>=MAX_REFERENCES) {
        printf("Error: cannot add identifier '%s', it has too many references. %u is the maximum, and it has %u\n",MAX_REFERENCES,nofoffsets);
        exit(1);
    }
    for(i=identifier[no].nofreferences;i<identifier[no].nofreferences+nofoffsets;i++) {
        identifier[no].reference[i].filenum=filenum;
        identifier[no].reference[i].offset=offsets[i-identifier[no].nofreferences];
    }
    identifier[no].nofreferences+=nofoffsets;
    strcpy(identifier[no].filename,filename);
}

void
go_linker(_UINT filenum) {
    _ULONG i;
    _UCHAR type,internal,external;
    _UINT nofrefs;
    char namestr[SCRIPT_MAXIDENTLEN];
    char filestr[MAX_FILE_LEN];
    char offsetab[MAX_REFERENCES];

    for(i=0;i<locate_len;i++) {
        memset(namestr,0,SCRIPT_MAXIDENTLEN);
        strcpy(namestr,(char *)(locate_data+i));

        i=i+strlen(namestr)+1;
        nofrefs=locate_data[i]; i++;
        memcpy(offsetab,(char *)(locate_data+i),nofrefs*2);
        i=i+(nofrefs*2);
        type=locate_data[i]; i++;
        internal=locate_data[i]; i++;
        external=locate_data[i]; i++;

        strcpy(filestr,(char *)(locate_data+i));
        i=i+strlen(filestr);

        add_identifier(namestr,type,internal,external,filenum,nofrefs,(_UINT *)&offsetab,filestr);
    }
/*    for (i=0;i<curident;i++) {
        printf("NAME >%s< TYPE %u INT %u EXT %u REFS %u = {",identifier[i].name,identifier[i].type,identifier[i].internal,identifier[i].external,identifier[i].nofreferences);
        for(nofrefs=0;nofrefs<identifier[i].nofreferences;nofrefs++) {
            printf(" %u",identifier[i].reference[nofrefs]);
        }
        printf(" }\n");
    }*/
}

void
add_file(char *name) {
    if(curfilentry>=MAX_FILES) {
        /* too many file entries. bomb out */
        printf("Error: Cannot add '%s' to the file entries buffer. %u is the maximum amount of names that can be there\n",MAX_FILES);
        exit(1);
    }
    getbasename(name,filentry[curfilentry].name);
    curfilentry++;
}

_UINT
get_file_no(char *name) {
    _UINT i;

    for(i=0;i<curfilentry;i++) {
        if(!strcmp(name,filentry[i].name)) {
            return i;
        }
    }
    return NO_FILE;
}

void
process_all_locs() {
    _UINT i;
    struct ffblk ff;

    i=findfirst("*.mlc",&ff,FA_ARCH);
    while(!i) {
        #ifndef RHIDE_COMPITABLE
            printf("þ Reading locating file %s...",ff.ff_name);
        #endif
        locate_data=readfile(ff.ff_name,&locate_len);
        #ifndef RHIDE_COMPITABLE
            printf(" done\n");
            printf("> Processing %s...",ff.ff_name);
        #endif
        go_linker(curfilentry);
        #ifndef RHIDE_COMPITABLE
            printf(" done\n");
        #endif
        free(locate_data);
        add_file(ff.ff_name);

        i=findnext(&ff);
    }
}

void
go_compiler(char *fname) {
    _UINT fileno,j,count;
    char base[512];

    getbasename(fname,base);
    if((fileno=get_file_no(base))==NO_FILE) {
        printf("Error: No location file for '%s'\n",base);
        exit(1);
    }
    for(count=0;count<curident;count++) {
        for(j=0;j<identifier[count].nofreferences;j++) {
            if (identifier[count].reference[j].filenum==fileno) {
//                printf("obj %s in file %s @ %x gets %x\n",identifier[count].name,fname,identifier[count].reference[j].offset,count);
                /* this belongs to us. tweak the correct number */
                compiled_script[identifier[count].reference[j].offset]=count&0xff;
                compiled_script[identifier[count].reference[j].offset+1]=count>>8;
            }
        }
    }
}

void
process_all_scripts() {
    _UINT i;
    _UINT filenum;
    struct ffblk ff;
    char tmp[MAX_FILE_LEN];
    char *ptr;
    struct SCRIPTHEADER *hdr;
    char base[512];

    i=findfirst("*.cm",&ff,FA_ARCH);
    while(!i) {
        #ifndef RHIDE_COMPITABLE
            printf("þ Reading compiled file %s...",ff.ff_name);
        #endif
        ptr=readfile(ff.ff_name,&compiled_len);
        hdr=(SCRIPTHEADER *)ptr;
        compiled_script=(char *)(ptr+sizeof(struct SCRIPTHEADER));
        #ifndef RHIDE_COMPITABLE
            printf(" done\n");
            printf("> Processing %s...",ff.ff_name);
        #endif
        if(hdr->magicno!=SCRIPT_MAGICNO) {
            printf("Error: '%s' is not a compiled script\n",ff.ff_name);
            exit(1);
        }
        if(hdr->version) {
            printf("Error: script '%s' was already compiled\n",ff.ff_name);
            exit(1);
        }
        go_compiler(ff.ff_name);
        #ifndef RHIDE_COMPITABLE
            printf(" done\n");
            printf("> Writing destination file...");
        #endif
        /* set correct header version number */
        hdr->version=SCRIPT_VERSIONO;
        getbasename(ff.ff_name,base);
        strcat(base,".LM");
        writefile(base,compiled_len,ptr);
        #ifndef RHIDE_COMPITABLE
            printf(" done\n");
        #endif
        free(ptr);

        i=findnext(&ff);
    }
}

void
check_locs() {
    _UINT i;

/*    for(i=0;i<curident;i++) {
        if(identifier[i].external) {
             printf("external identifier '%s' was never internal\n",identifier[i].name);
        }
    }*/
}

void
main(_UINT argc,char *argv[]) {
    char *locate_filename;

    if(argc!=1) {
        printf("The MAD Linker Version 1.0 - (c) 1999 The MAD Crew\n\n");
        printf("Usuage: ML\n");
        exit(1);
    }
    memset(identifier,0,MAX_IDENTS*sizeof(struct IDENTIFIER));
    curident=0;curfilentry=0;
    #ifndef RHIDE_COMPITABLE
        printf("> Linking...\n");
    #endif
    process_all_locs();
    check_locs();
    process_all_scripts();
    free(locate_data);
    free(compiled_script);
}
