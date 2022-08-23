/*
		MAD Archive Manager Version 2.0
   		    (c) 1998, 1999 The MAD Crew

	  This version only handles archives with version 2.0
*/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "archive.h"
#include "mad.h"
#include "types.h"

ARCHIVE *archive;
char filename[64];
_UCHAR create_archive;

void
parseparams(char *argv[], _UINT argc) {
	_UINT i;

	for (i=1;i<argc;i++) {
		if ((argv[i][0]=='-')||(argv[i][0]=='/')) {
			switch (tolower(argv[i][1])) {
			   case 'n': create_archive=1;
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
	strcpy(filename,argv[argc-1]);
}

int
main(int argc,char *argv[]) {
	char command[255];
	_UCHAR result,done,i;
	FILE *f;
	struct ARCHIVEFILE afile;

	printf("MAD Archive Manager Version %u.%u - (c) 1998, 1999 The MAD Crew\n\n",(ARCHIVE_VERSION/100),(ARCHIVE_VERSION%100));

	if (argc<2) {
		printf("usuage: mfile [-n] file.mad\n\n");
		printf("-n		create a new archive file\n");
		exit(1);
	}
	parseparams(argv,argc);

	if (!(archive = new ARCHIVE)) {
		printf("out of memory for a new archive object\n");
		exit(1);
	}

	if (!create_archive) {
		if ((result=archive->initreadwrite(filename))!=ARCHIVE_OK) {
			printf("unable to open %s: %s\n",filename,archive->err2msg(result));
			exit(result);
		}
	} else {
		if ((result=archive->create(filename))!=ARCHIVE_OK) {
			printf("unable to create %s: %s\n",filename,archive->err2msg(result));
			exit(result);
		}
	}
	printf("MAD file '%s' opened, %u files in archive\n",filename,archive->getnofiles());

	done=0;
	while (!done) {
		printf("choice>");
		gets(command);
		switch (tolower(command[0])) {
			case 'q': done=1; break;
			case '?':
			case 'h': printf("q         quit\n");
					  printf("a [file]  adds [file] to the archive\n");
					  printf("d         shows the directory of the archive\n");
					  printf("g [file]  retrieves [file] from the archive\n");
					  printf("i         print some header information\n");
					  printf("? or h    this short help\n");
					  break;
			case 'a': if (sscanf(command,"%c%s",&i,&filename)!=2) {
						printf("filename: ");
						gets(filename);
					  }
					  if ((f=fopen(filename,"rb"))==NULL) {
						printf("unable to open '%s'\n",filename);
					  } else {
						  if ((result=archive->addfile(filename,f))!=ARCHIVE_OK) {
							printf("unable to add '%s': %s\n",filename,archive->err2msg(result));
						  }
						  fclose(f);
					  }
					  break;
			case 'd': printf("contents of archive file:\n-------------------------\n");
					  if ((result=archive->findfirst(&afile))!=ARCHIVE_OK) {
						printf("unable to seek in archive file: %s\n",archive->err2msg(result));
						break;
					  }
					  do {
						printf("%14s, %lu bytes\n",afile.filename,afile.size);
					  } while(archive->findnext(&afile)==ARCHIVE_OK);
					  break;
			case 'i': printf("%u files in archive\n",archive->getnofiles());
					  printf("entry block starts at offset %lu\n",archive->getentrypos());
					  printf("archive file version %u.%u\n",(archive->getversion()/100),(archive->getversion()%100));
					  break;
			case 'g': if (sscanf(command,"%c%s",&i,&filename)!=2) {
						printf("filename: ");
						gets(filename);
					  }
					  if ((f=fopen(filename,"wb"))==NULL) {
						printf("unable to create '%s'\n",filename);
					  } else {
						  if ((result=archive->getfile(filename,f))!=ARCHIVE_OK) {
							printf("unable to retrieve '%s': %s\n",filename,archive->err2msg(result));
						  }
						  fclose(f);
					  }
					  break;
			 default: printf("unknown command '%c' -- use ? or h for help\n",command[0]);
					  break;
		}
	}
	archive->done();
	return 0;
}
