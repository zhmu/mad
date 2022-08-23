/*
                Font Manager Version 2.1
                 (c) 1999, 2000 The MAD Crew

   This version only handles fonts with version 2.0
*/

#define FONTMAN_VERSION "2.1"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "font.h"
#include "../../../source/include/mad.h"
#include "../../../source/include/types.h"

char   filename[MAD_TEMPSTR_SIZE];
_UCHAR create_font;
FONT   *font;

void
parseparams(char *argv[], _UINT argc) {
    _UINT i;

    create_font=0;
    for (i=1;i<argc;i++) {
        if ((argv[i][0]=='-')||(argv[i][0]=='/')) {
            switch (tolower(argv[i][1])) {
                case 'n': create_font=1;
                          break;
                 default: printf("unknown option -- %c\n",argv[i][1]);
                          exit(0xff);
            }
        } else {
	    if (i!=(argc-1)) {
                printf("filename must be last argument\n");
                exit(0xff);
            }
        }
    }
    /* get the filename */
    strcpy(filename,argv[argc-1]);
}

int
main(int argc,char *argv[]) {
    char   command[MAD_TEMPSTR_SIZE];
    _UCHAR done;
    int    i,d,e;

    printf("MAD Font Manager Version "FONTMAN_VERSION" - (c) 1999, 2000 The MAD Crew\n\n");

    if(argc<2) {
        printf("usuage: fontman [-n] file.fnt\n\n");
        printf("-n		create a new font file\n");
        exit(1);
    }
    /* figure out the parameter stuff */
    parseparams(argv,argc);

    /* dump the name of the font file used */
    printf ("Font file used: '%s'\n",filename);

    /* create the font object */
    if (!(font = new FONT)) {
        /* that failed. we're out of memory */
        printf("Out of memory for font object\n");
        return 1;
    }

    /* open/create the font file */
    if(create_font) {
        i=font->create(filename);
    } else {
        i=font->init(filename);
    }

    if (i!=FONT_ERROR_OK) {
        printf ("Error: %s\n",font->err2msg(i));
        return 2;
    }

    done=0;
    while (!done) {
        printf("choice>");
        gets(command);
        switch (tolower(command[0])) {
            case 'q': done=1; break;
            case '?': printf("q                                  quit\n");
                      printf("s [0..255 file opaquecolor]        sets image for character\n");
                      printf("w 1..15                            sets base font width\n");
                      printf("h 1..15                            sets base font height\n");
                      printf("?                                  this short help\n");
		      break;
            case 'h': /* set height */
                      if (sscanf(command,"%c%d",&i,&d)!=2) {
                          printf("Incorrect usage - please use '?' for help\n");
                      } else {
                          /* already set? */
                          if(!(font->header.height_width>>4)) {
                              /* nope. set it */
                              font->header.height_width=(d<<4)+(font->header.height_width&0xf);
                          } else {
                              printf("The height is already set to %u\n",font->header.height_width>>4);
                          }
                      }
                      break;
            case 'w': /* set width */
                      if (sscanf(command,"%c%d",&i,&d)!=2) {
                          printf("Incorrect usage - please use '?' for help\n");
                      } else {
                          /* already set? */
                          if(!(font->header.height_width&0xf)) {
                              /* nope. set it */
                              font->header.height_width=((font->header.height_width>>4)<<4)+d;
                          } else {
                              printf("The width is already set to %u\n",font->header.height_width&0xf);
                          }
                      }
                      break;
            case 's': /* set width */
                      if (sscanf(command,"%c%u%s%u",&i,&d,&filename,&e)!=4) {
                          printf("Incorrect usage - please use '?' for help\n");
                      } else {
                          /* add the char */
                          if ((i=font->addchar(filename,d,e))!=FONT_ERROR_OK) {
                              /* this failed. show the error */
                              printf("Error: %s\n",font->err2msg(i));
                          }
                      }
                      break;
            case ';':
            case '#': /* comment, ignore it */
                      break;
             default: printf("unknown command '%c' -- use '?' for help\n",command[0]);
                      break;
	}
    }
    /* clean it up */
    i=font->done();

    /* if this failed, inform user */
    if (i!=FONT_ERROR_OK) {
        /* it *did* fail. inform user */
        printf ("Error: %s\n",font->err2msg(i));
        return 2;
    }

    /* nuke the font object */
    delete font;

    /* goodbye, blue sky! :-) */
    return 0;
}
