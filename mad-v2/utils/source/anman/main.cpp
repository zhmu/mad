/*
		 MAD Animation File Manager Version 2.0
                         (c) 1999 The MAD Crew
*/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "anm.h"
#include "types.h"

#define VERSION "1.0"

#define FILES_START "[files]"
#define ANIMS_START "[anims]"

_UINT flagtable[16] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048,
					   4096, 8192, 16384, 32768 };

ANM *anm;
FILE *scriptfile;

_UCHAR quiet;
char filename[64],scriptname[64];
char buf[1024];

void
usuage() {
    printf("MAD Animation Manager Version "VERSION" - (c) 1999 The MAD Crew\n\n");
    printf("usuage: anman [-h] [-q] anmfile.anm script.scr\n\n");
    printf("anmfile.anm is the animation file you want to create\n");
    printf("script.scr is the script file you want me to use\n\n");
    printf("-q is quiet, don't output anything\n");
    printf("-h or -? is this help\n");
    exit(2);
}

void
parseparams(char *argv[], _UINT argc) {
    _UINT i;

    quiet=0;
    for (i=1;i<argc;i++) {
        if ((argv[i][0]=='-')||(argv[i][0]=='/')) {
            switch (tolower(argv[i][1])) {
               case 'h':
               case '?': usuage();
               case 'q': quiet=1;
                         break;
                default: printf("unknown option -- %c\n",argv[i][1]);
                         exit(0xff);
            }
        }
    }
    if (argc<3) {
        printf("I need at least a anm file name and a script file name!\n");
        exit(1);
    }
    strcpy(filename,argv[argc-2]);
    strcpy(scriptname,argv[argc-1]);
}

int
main(int argc,char *argv[]) {
	_UCHAR mode,i,donesprites,flags;
	char animname[64],pos,frameno;
	char realbuf[1024];

	char	temp[64];
	_UCHAR	count;

	if (argc<2) {
            usuage();
	    return 1;
	}
	parseparams(argv,argc);

        if(!quiet) {
   	    printf("MAD Animation Manager Version "VERSION" - (c) 1999 The MAD Crew\n\n");
            printf("using anm file '%s', script file '%s'\n",filename,scriptname);
        }
	if (!(anm = new ANM)) {
		printf("out of memory for a new animation object\n");
		exit(1);
	}
	if((scriptfile=fopen(scriptname,"rt"))==NULL) {
		printf("unable to open '%s'\n",scriptname);
		exit(1);
	}
	if (anm->createanm(filename)!=ANM_OK) {
		printf("unable to create '%s'\n",filename);
		exit(1);
	}
	mode=0;donesprites=0;
	while(fgets(buf,1024,scriptfile)) {
		strcpy(realbuf,buf);
		/* chop the newline off */
		buf[strlen(buf)-1]=0;

		if((mode==1)&&(buf[0]!=0)&&(buf[0]!='[')&&(buf[0]!=';')) {
			/* process entry as a file */
                        if(!quiet) printf("%s...",buf);
			if((i=anm->addsprite(buf))!=ANM_OK) {
				/* something went wrong. report this */
				printf(" failed, %s\n",anm->err2msg(i));
			} else {
				if(!quiet) printf(" done\n");
			}
		}

		if((mode==2)&&(buf[0]!=0)&&(buf[0]!='[')&&(buf[0]!=';')) {
			/* process entry as an animation */
			sscanf(buf,"%s",animname);
			if(!quiet) printf("%s...",animname);

			pos=strlen(animname)+1;flags=0;
			if(strlen(animname)>(ANM_MAXANMLEN-1)) {
				printf(" ignored %s, name may not exceed %u chars\n",animname,ANM_MAXANMLEN-1);
			} else {
				if(!quiet) printf(" ");
				count=0;
				while(pos<=strlen(buf)) {
					frameno=atoi((char *)(realbuf+pos));
					if (!strncasecmp((char *)(realbuf+pos),"f",1)) {
						if(!quiet) printf("<<Flag%u>> ",atoi((char *)(realbuf+pos+1)));
						flags+=flagtable[atoi((char *)(realbuf+pos+1))];
						pos+=3;
					} else {
						if(frameno>9) {
							pos+=3;
						} else {
							pos+=2;
						}
						temp[count]=frameno;
						if(!quiet) printf("%u ",frameno);
						count++;
					}
				}
				if((i=anm->addanm(animname,temp,count,flags))!=ANM_OK) {
					printf(" failed adding %s, %s\n",animname,anm->err2msg(i));
				}
			}
			if(!quiet) printf("done\n");
		}
		if(!strncmp(buf,FILES_START,sizeof(FILES_START))) {
			if(!donesprites) {
				mode=1;
				if(!quiet) printf(">> processing sprites...\n");
			} else {
				printf(">> cannot process sprites: already happened\n");
			}
		}
		if(!strncmp(buf,ANIMS_START,sizeof(ANIMS_START))) {
			if(mode==1) donesprites=1;
			if (donesprites) {
				mode=2;
				if(!quiet) printf(">> processing animations...\n");
			} else {
				printf(">> cannot process animations: sprites must go first\n");
			}
		}
	}
	anm->done();
	fclose(scriptfile);
	return 0;
}
