#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../source/include/game.h"
#include "../../../source/include/inv.h"
#include "../../../source/include/textman.h"
#include "../../../source/include/types.h"

#define MAX_FILE_NAME_LEN 64

#define INVMAN_VALUE_MAXQ    "max_quantity"
#define INVMAN_VALUE_FLAGS   "flags"
#define INVMAN_VALUE_INVICON "icon"

char filename[MAX_FILE_NAME_LEN];
char out_filename[MAX_FILE_NAME_LEN];

/*
 * usuage()
 *
 * This will show the user some help about the switches and stuff.
 *
 */
void
usuage() {
    printf("Usuage: INVMGR [input_file] [out_file.inv]\n\n");
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

    for (i=1;i<argc;i++) {
        if ((argv[i][0]=='-')||(argv[i][0]=='/')) {
            switch (argv[i][1]) {
                case 'h':
                case '?': usuage();
                          exit(1);
                 default: printf("unknown option -- %c\n",argv[i][1]);
                          exit(1);
            }
	}
    }
    strcpy(filename,argv[argc-2]);
    strcpy(out_filename,argv[argc-1]);
}

/*
 * create_inv_file(FILE* f,FILE* in)
 *
 * This will write the actual data to inventory file [f]. The input comes
 * from [in].
 *
 */
void
create_inv_file(FILE* f,FILE* in) {
    struct INV_HEADER hdr;
    _ULONG item_count,line_count,i;
    char   tempstr[MAD_TEMPSTR_SIZE];
    char   object_name[MAD_TEMPSTR_SIZE];
    char   value[MAD_TEMPSTR_SIZE];
    char*  ptr;
    char*  ptr2;
    struct INV_ITEM item;

    /* set up a header */
    memset(&hdr,0,sizeof(hdr));
    hdr.id=MAD_INV_ID;
    hdr.version=MAD_INV_VERSION;
    hdr.nofitems=0;
    /* dump it in the file */
    if(!fwrite(&hdr,sizeof(hdr),1,f)) {
        /* This failed. die */
        printf("Unable to write header to output file\n");
        exit(1);
    }

//struct INV_ITEM {
//    char   name[MAD_INV_NAME_LENGTH];     /* name */
//    _ULONG max_quantity;                  /* maximum quantity */
//    char   inv_icon[MAD_INV_ICONAME_LENGTH]; /* inventory icon file */
//    _ULONG flags;                         /* flags */
//    char   reserved[88];                  /* unused */
//};

    item_count=0;line_count=1;memset(object_name,0,MAD_TEMPSTR_SIZE);
    memset(&item,0,sizeof(item));
    while(!feof(in)) {
        /* get the line and nuke the \n */
        fgets(tempstr,MAD_TEMPSTR_SIZE,in);
        if(tempstr[strlen(tempstr)-1]=='\n') {
            tempstr[strlen(tempstr)-1]=0;
        }

        /* figure out what it is */
        if((ptr=strchr(tempstr,'['))!=NULL) {
            /* It seems to be a [xxx] subsection! */
            item_count++;
            /* If we are currently doing something, save it */
            if(object_name[0]!=0) {
                /* Save it */
                if(!fwrite(&item,sizeof(item),1,f)) {
                    /* This failed. Die */
                    printf("Unable to write output file\n");
                    exit(2);
                }
                /* And finish message */
                printf(" done\n");
            }
            /* Find the ']' */
            if((ptr2=strchr(tempstr,']'))==NULL) {
                /* We could not find it. die */
                printf("%s(%lu): found [ but not ]\n",filename,line_count);
                exit(2);
            }
            /* Get the name of the object */
            memset(object_name,0,MAD_TEMPSTR_SIZE);
            for(i=(ptr-tempstr)+1;i<(ptr2-tempstr);i++) {
                object_name[i-(ptr-tempstr)-1]=tempstr[i];
            }
            if(strlen(object_name)>MAD_INV_NAME_LENGTH) {
                 /* Object name is too long. Die */
                printf("%s(%lu): object name '%s' is too long, it can be maximum %u charachters\n",filename,line_count,object_name,MAD_INV_NAME_LENGTH);
                exit(2);
            }
            printf(">> Processing object '%s'...",object_name);

            /* Nuke the current item data */
            memset(&item,0,sizeof(item));
            /* Fill in the name */
            strcpy(item.name,object_name);
        } else {
            /* If it contains a =, it is a property */
            if((ptr=strchr(tempstr,'='))!=NULL) {
                 /* It is a property! Split it to [object_name]=[value] */
                 memset(object_name,0,sizeof(object_name));
                 for(i=0;i<(ptr-tempstr);i++) {
                     object_name[i]=tempstr[i];
                 }
                 memset(value,0,sizeof(value));
                 for(i=(ptr-tempstr)+1;i<strlen(tempstr);i++) {
                     value[i-((ptr-tempstr)+1)]=tempstr[i];
                 }

                 /* do we recognize this value? */
                 if(!strcmp(object_name,INVMAN_VALUE_MAXQ)) {
                      /* it is the maximum quantity value! resolve it */
                      item.max_quantity=strtoul(value,&ptr,0);
                      if(ptr==value) {
                          /* We could not resolve this. Die */
                          printf("%s(%lu): cannot resolve string '%s' to an integer\n",filename,line_count,value);
                          exit(2);
                      }
                 } else {
                      /* maybe the flags? */
                      if(!strcmp(object_name,INVMAN_VALUE_FLAGS)) {
                          /* it is the flags value! resolve it */
                          item.flags=strtoul(value,&ptr,0);
                          if(ptr==value) {
                              /* We could not resolve this. Die */
                              printf("%s(%lu): cannot resolve string '%s' to an integer\n",filename,line_count,value);
                              exit(2);
                          }
                      } else {
                          /* maybe the inventory icon? */
                          if(!strcmp(object_name,INVMAN_VALUE_INVICON)) {
                              /* yeah. copy it */
                              strcpy(item.inv_icon,value);
                          } else {
                              /* we don't know it. die */
                              printf("%s(%lu): unknown property '%s'\n",filename,line_count,object_name);
                              exit(2);
                          }
                      }
                 }
            } else {
                 /* maybe a comment? */
                 if(strlen(tempstr)&&(tempstr[0]!='#')&&(!((tempstr[0]=='/')&&(tempstr[1]=='/')))) {
                     /* nope. we're out of choices. die */
                     printf("%s(%lu): line '%s' not understood\n",filename,line_count,tempstr);
                     exit(2);
                 }
            }
        }
        line_count++;
    }
    /* If we are currently doing something, save it */
    if(object_name[0]!=0) {
        /* Save it */
        if(!fwrite(&item,sizeof(item),1,f)) {
            /* This failed. Die */
            printf("Unable to write output file\n");
            exit(2);
        }
        /* And finish message */
        printf(" done\n");
    }
    /* update the header */
    rewind(f);
    hdr.nofitems=item_count;
    /* dump it in the file */
    if(!fwrite(&hdr,sizeof(hdr),1,f)) {
        /* This failed. die */
        printf("Unable to write header to output file\n");
        exit(1);
    }
}

/*
 * main(int argc,char *argv[])
 *
 * This will be executed first.
 *
 */
int
main(int argc,char *argv[]) {
    FILE *in;
    FILE *f;

    printf("MAD Inventory Manager Version 1.0 - (c) 1999 The MAD Crew\n\n");
    if (argc<3) {
        usuage();
        exit(1);
    }
    parseparams(argv,argc);
    printf("Creating inventory file '%s' from '%s'...\n",out_filename,filename);

    if((f=fopen(out_filename,"wb"))==NULL) {
        printf("Unable to create output file '%s'\n",out_filename);
        exit(1);
    }
    if((in=fopen(filename,"rt"))==NULL) {
        printf("Unable to open input file '%s'\n",filename);
        exit(1);
    }
    /* process it all */
    create_inv_file(f,in);
    fclose(in);
    fclose(f);
    return 0;
}
