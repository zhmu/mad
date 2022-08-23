/*
			MAD Font Manager Version 1.0
				 (c) 1999 The MAD Crew

	  This version only handles fonts with version 1.0
*/

#define FONTMAN_VERSION "1.0"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "font.h"
#include "../../../source/mad.h"
#include "../../../source/types.h"

FONT *font;
char filename[64], file[64];
_UCHAR create_font=0;

void
parseparams(char *argv[], _UINT argc) {
	_UINT i;

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
	strcpy(file,argv[argc-1]);
}

int
main(int argc,char *argv[]) {
	char command[255];
	_UCHAR result,done,i;
	FILE *fil;
        int c;
	_UINT d,q,e,z,f,y;

	printf("MAD Font Manager Version "FONTMAN_VERSION" - (c) 1999 The MAD Crew\n\n");

	if (argc<2) {
		printf("usuage: fontman [-n] file.fnt\n\n");
		printf("-n		create a new font file\n");
		exit(1);
	}
	parseparams(argv,argc);

	if (!(font = new FONT)) {
		printf("out of memory for a new font object\n");
		exit(1);
	}

	if (!create_font) {
		if ((result=font->initreadwrite(file))!=FONT_OK) {
			printf("unable to open %s: %s\n",file,font->err2msg(result));
			exit(result);
		}
	} else {
		if ((result=font->create(file))!=FONT_OK) {
			printf("unable to create %s: %s\n",file,font->err2msg(result));
			exit(result);
		}
	}
	printf("Font file '%s' opened\n",file);

	done=0;
	while (!done) {
		printf("choice>");
		gets(command);
                printf(">>%s<<\n",command);
		switch (tolower(command[0])) {
			case 'q': done=1; break;
			case '?': printf("q                                  quit\n");
				  printf("s [0..255 file opaquecolor]        sets image for character\n");
				  printf("g [0..255 file color opaqcol]      retrieves image of character into [file]\n");
                                  printf("d 0..255                           removes a character from the font\n");
				  printf("i                                  print some header information\n");
                                  printf("w 0..65535                         sets font width\n");
                                  printf("h 0..65535                         sets font height\n");
				  printf("?                                  this short help\n");
				  break;

                        case 'w': if (sscanf(command,"%c%d",&i,&d)!=2) {
                                      printf("Incorrect usage - please use '?' for help\n");
                                  } else {
                                      if (font->header->width==0) {
                                          font->header->width = d;
                                      } else {
                                          printf("Can only initialize width once\n");
                                      }
                                  }
                                  break;

                        case 'h': if (sscanf(command,"%c%d",&i,&d)!=2) {
                                      printf("Incorrect usage - please use '?' for help\n");
                                  } else {
                                      if (font->header->height==0) {
                                          font->header->height = d;
                                      } else {
                                          printf("Can only initialize height once\n");
                                      }
                                  }
                                  break;

                        case 'i': printf("Font version                %.4x",font->header->version);
                                  printf("\nFont height                 %d",font->header->height);
                                  printf("\nFont width                  %d",font->header->width);
                                  printf("\nUsed font entries:\n");
                                  for (d=0;d<256;d++) {
                                      if (font->charexists(d)) {
                                          printf("%.3d ",d);
                                      }
                                  }
                                  printf("\n");
                                  break;

                        case 'd': if (sscanf(command,"%c%d",&i,&c)!=2) {
                                      printf("Incorrect usage - please use '?' for help\n");
                                  } else {
                                      if (c<0 || c>255 ) {
                                          printf("character must be between 0 and 255");
                                      } else {
                                          if ((result=font->deletechar(c))!=FONT_OK) {
                                              printf("unable to delete character %d : %s\n",c,font->err2msg(result));
                                          }
                                      }
                                  }
                                  break;


			case 's': if (sscanf(command,"%c%u%s%u",&i,&d,&filename,&e)!=4) {
                                                printf("   character: ");
                                                scanf("%u",&d);
						printf("    filename: ");
						scanf("%s",&filename);
                                                printf("opaque color: ");
                                                scanf("%u",&e);
					  }
                                          c=(char)d;
                                          if (c>255 || c<0 || e<0 || e>255) {
                                                printf("character and color must be between 0 and 255\n");
                                          } else {
                                              if ((fil=fopen(filename,"rb"))==NULL) {
                                                  printf("unable to open '%s'\n",filename);
                                              } else {
                                                  fseek(fil,0,SEEK_SET);
					          if ((result=font->setchar(fil,c,e))!=FONT_OK) {
                                                      printf("unable to add '%s': %s\n",filename,font->err2msg(result));
                                                  }
                                              fclose(fil);
                                              }
					  }
					  break;

			case 'g': if (sscanf(command,"%c%d%s%d%d",&i,&d,&filename,&e,&f)!=5) {
                                                printf("       character: ");
                                                scanf("%d",&d);
						printf("        filename: ");
						gets(filename);
                                                printf("      font color: ");
                                                scanf("%d",&e);
                                                printf("background color: ");
                                                scanf("%d",&f);
					  }
                                          c=(char)d;
                                          if (c>255 || c<0 || f>255 || f<0 || e>255 || e<0) {
                                                printf("character and color must be between 0 and 255\n");
                                          } else {
                                              if ((fil=fopen(filename,"wb"))==NULL) {
						    printf("unable to create '%s'\n",filename);
                                              } else {
						  if ((result=font->getchr(fil,c,(_UCHAR)e,(_UCHAR)f))!=FONT_OK) {
							printf("unable to retrieve '%s': %s\n",filename,font->err2msg(result));
						  }
						  fclose(fil);
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
        if ((result=font->save(file))!=FONT_OK) {
            printf("Could not save file %s: %s",file,font->err2msg(result));
        }
        font->done();
	return 0;
}
