#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "textman.h"
#include "types.h"
#include "txfgen.h"

#define MAX_FILE_NAME_LEN 64
#define MAX_TEMPSTR_LEN 512
#define MAX_STRINGS 1024

#define TXFGEN_GLOBAL_SECTION "[global]"
#define TXFGEN_GLOBAL_OUTFILE "out="

#define TXFGEN_STRINGS_SECTION "[strings]"

#define TXFGEN_ENCRYPTSTRING "The User Of InCredible power"

#define NO_GFX_SCR 1
#define NO_MSK_SCR 2
#define NO_PRI_SCR 4

#define NO_FILE_NAME "/dev/null desktop->trashcan"

char filename[MAX_FILE_NAME_LEN];
char out_filename[MAX_FILE_NAME_LEN];
char* scriptdata;
_ULONG scriptlength;
_ULONG offset[MAX_STRINGS];
_ULONG nofoffsets,curoffset;

/*
 * usuage()
 *
 * This will show the user some help about the switches and stuff.
 *
 */
void
usuage() {
    printf("Usuage: TXFGEN [input_file]\n\n");
    printf("Switches can be:\n");
    printf("    -? or -h       this help\n\n");
    printf("As you might have guessed, the parameters are cAsE sEnSiTiVe\n");
}

/*
 * parseparams(char *argv[], _UINT argc)
 *
 * This will parse the parameters. [argv] is assumed to be the argv from
 * main(), and [argc] the argc from main(). This will quit if anything is not
 * understood.
 *
 */
void
parseparams(char *argv[], _UINT argc) {
    _UINT i,j;

    strcpy(filename,NO_FILE_NAME);
    for (i=1;i<argc;i++) {
        if ((argv[i][0]=='-')||(argv[i][0]=='/')) {
            switch (argv[i][1]) {
                case 'h':
                case '?': usuage();
                          exit(1);
                 default: printf("unknown option -- %c\n",argv[i][1]);
                          exit(1);
            }
        } else {
            if(strcmp(filename,NO_FILE_NAME)) {
                printf("Filename already given!\n");
                exit(1);
            }
            strcpy(filename,argv[i]);
	}
    }
}

/*
 * init_script(char* name)
 *
 * This will initialize the text script. It will exit if anything fails.
 *
 */
void
init_script(char* name) {
    FILE *f;
    _UCHAR active;
    _ULONG lineno;
    char tempstr[MAX_TEMPSTR_LEN];

    /* Make sure done_script() knows there is not script data right now */
    scriptdata=NULL;

    /* Make sure out_filename is a nonsense string */
    strcpy(out_filename,NO_FILE_NAME);

    /* Try to open the file */
    if((f=fopen(name,"rb"))==NULL) {
        /* This failed. Die */
        printf("unable to open file '%s' for reading\n");
        exit(1);
    }
    /* Figure out the length */
    fseek(f,0,SEEK_END);
    scriptlength=ftell(f);
    rewind(f);

    /* Allocate memory for the script data */
    if((scriptdata=(char*)malloc(scriptlength))==NULL) {
        /* This failed. Die */
        printf("unable to allocate %lu bytes of memory\n",scriptlength);
        exit(1);
    }

    /* Read the script */
    if(!fread(scriptdata,scriptlength,1,f)) {
        /* This failed. Die */
        printf("unable to read script file\n");
        exit(1);
    }

    /* Close the file */
    fclose(f);

    /* Now figure out the [global] section */
    active=0;lineno=0;

    while(readline(lineno,scriptlength,tempstr,scriptdata)) {
        /* Are we active? */
        if (active) {
            if (tempstr[0]=='[') {
                /* This is another section. Get outta here! */
                lineno=0xfffffffe;
            } else {
                /* is it the out= thingy? */
                if(!strncmp(tempstr,TXFGEN_GLOBAL_OUTFILE,strlen(TXFGEN_GLOBAL_OUTFILE))) {
                    /* Yeah. Gotcha! */
                    strcpy(out_filename,(char*)(tempstr+strlen(TXFGEN_GLOBAL_OUTFILE)));
                } else {
                    /* Is this something other than a empty line and not a
                       comment? */
                    if(strlen(tempstr)&&(tempstr[0]!='#')&&(!((tempstr[0]=='/')&&(tempstr[1]=='/')))) {
                        /* Yeah. We don't know this. Die */
                        printf("Unknown global parameter in line %lu\n",lineno+1);
                        exit(2);
                    }
                }
            }
        } else {
            /* Is the current line the [global] section? */
            if (!strcmp(tempstr,TXFGEN_GLOBAL_SECTION)) {
                /* Yeah. Make ourselves active */
                active=1;
            }
        }
        lineno++;
    }
}

/*
 * done_script()
 *
 * This will deinitialize the text script.
 *
 */
void
done_script() {
    /* Was memory allocated for the script data? */
    if(scriptdata!=NULL) {
        /* Yeah, set it free */
        free(scriptdata);
        /* And make sure we don't do this twice */
        scriptdata=NULL;
    }
}

/*
 * encrypt_string(char* in,char* out)
 *
 * This will encrypt asciiz string [in] and dump the result in [out].
 *
 */
void
encrypt_string(char* in,char* out) {
    _UINT i,key;
    char keyring[MAX_TEMPSTR_LEN];

    strcpy(keyring,TXFGEN_ENCRYPTSTRING);
    strcpy(out,in);

    i=~(strlen(in));

    out[0]=(i&0xff);
    out[1]=(i>>8);

    key=0;
    for(i=0;i<strlen(in);i++) {
        out[i+2]=(char)in[i]^(char)keyring[key];

        key++;
        if(key==strlen(keyring)) key=0;
    }
}

/*
 * create_output()
 *
 * This will do the actual creating of the input file.
 *
 */
void
create_output() {
    FILE *f;
    _UCHAR active;
    _ULONG lineno,i;
    char tempstr[MAX_TEMPSTR_LEN];
    char tempstr2[MAX_TEMPSTR_LEN];
    struct TEXTHEADER hdr;

    /* Create the file */
    if((f=fopen(out_filename,"wb"))==NULL) {
        /* This failed. Die */
        printf("unable to open file '%s'\n",out_filename);
        exit(2);
    }

    /* Write a mumbo-jumbo header */
    memset(&hdr,0,sizeof(hdr));

    hdr.magic=TEXTMAN_MAGICID;
    hdr.version=TEXTMAN_VERSIONO;

    if(!fwrite(&hdr,sizeof(hdr),1,f)) {
        /* This failed. Die */
        printf("unable to write header to file '%s'\n",out_filename);
        exit(2);
    }

    /* Now figure out the [strings] section */
    active=0;lineno=0;curoffset=0;nofoffsets=0;

    while(readline(lineno,scriptlength,tempstr,scriptdata)) {
        /* Are we active? */
        if (active) {
            if (tempstr[0]=='[') {
                /* This is another section. Get outta here! */
                lineno=0xfffffffe;
            } else {
                /* don't add string if it's a comment (starting with //, ; or #)
                   or a blank line */
                if(strlen(tempstr)&&(tempstr[0]!='#')&&(!((tempstr[0]=='/')&&(tempstr[1]=='/')))) {
                    /* Add the string to the file */
                    encrypt_string(tempstr,tempstr2);

                    /* Dump it in the file */
                    if(!fwrite(tempstr2,strlen(tempstr)+2,1,f)) {
                        /* This failed. Die */
                        printf("unable to write to file '%s'\n",out_filename);
                        exit(2);
                    }

                    /* save the offset */
                    offset[nofoffsets]=curoffset;

                    nofoffsets++;
                    curoffset+=(strlen(tempstr)+2);
                }
            }
        } else {
            /* Is the current line the [strings] section? */
            if (!strcmp(tempstr,TXFGEN_STRINGS_SECTION)) {
                /* Yeah. Make ourselves active */
                active=1;
            }
        }
        lineno++;
    }
    /* Write the offset table */
    for(i=0;i<nofoffsets;i++) {
        if(!fwrite(&offset[i],4,1,f)) {
            /* This failed. Die */
            printf("unable to write to file '%s'\n",out_filename);
            exit(2);
        }
    }

    /* Edit the header */
    rewind(f);

    memset(&hdr,0,sizeof(hdr));
    hdr.magic=TEXTMAN_MAGICID;
    hdr.version=TEXTMAN_VERSIONO;
    hdr.nofstrings=nofoffsets;
    hdr.offset=curoffset;

    if(!fwrite(&hdr,sizeof(hdr),1,f)) {
        /* This failed. Die */
        printf("unable to write header to file '%s'\n",out_filename);
        exit(2);
    }


    /* Close the file */
    fclose(f);
}

/*
 * main(int argc,char *argv[])
 *
 * This will be executed first.
 *
 */
int
main(int argc,char *argv[]) {
    printf("MAD Text File Generator Version 1.0 - (c) 1999 The MAD Crew\n\n");
    if (argc<2) {
        usuage();
        exit(1);
    }
    parseparams(argv,argc);
    printf("Parsing '%s'...",filename); fflush (stdout);
    init_script(filename);
    /* Do we have a valid output filename? */
    if(!strcmp(out_filename,NO_FILE_NAME)) {
        /* No. Die */
        printf("No output file specified. Check your 'out' stuff in the [global] header\n");
        exit(1);
    }
    printf(" done\n"); fflush (stdout);

    printf("Creating file '%s' from script '%s'...",out_filename,filename); fflush(stdout);
    create_output();
    printf(" done\n"); fflush (stdout);

    done_script();
    return 0;
}
