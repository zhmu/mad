/*
			MAD Project Editor Version 1.0
                           (c) 1998, 1999 The MAD Crew

	This version only handles projects files with version 1.0
*/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../source/include/project.h"
#include "../../../source/include/types.h"

char filename[64];
_UCHAR create_project;

PROJECT *project;

void
parseparams(char *argv[], _UINT argc) {
    _UINT i;

    for (i=1;i<argc;i++) {
        if ((argv[i][0]=='-')||(argv[i][0]=='/')) {
            switch (argv[i][1]) {
                case 'n': create_project=1;
                          break;
                 default: printf("unknown option -- %c\n",argv[i][1]);
                          exit(0xfe);
            }
        } else {
            if (i!=(argc-1)) {
                printf("filename must be last argument\n");
                exit(0xfe);
            }
        }
    }
    strcpy(filename,argv[argc-1]);
}

void
printhelp() {
    printf("f...            set name of system font to ...\n");
    printf("p...            set palette file to ...\n");
    printf("s...            set script file to autoload to ...\n");
    printf("c...            set script procedure to autostart to ...\n");
    printf("t...            set text file to use to ...\n");
    printf("d...            set project id to ...\n");
    printf("i...            set inventory datafile to ...\n");
    printf("b...            set default dialog backdrop to ...\n");
    printf("o...            set default dialog border to ...\n");
    printf("m...            set system mouse cursor to ...\n");
    printf("l...            set number of bytes per pixel to ...\n");
    printf("r               show current project configuration\n");
    printf("B               toggle disabling of bar\n");
    printf("I               toggle using inventory file\n");
    printf("D               toggle usuage default dialog stuff\n");
    printf("P               toggle disabling of parser\n");
    printf("q               quit\n\n");
}

void
printconf() {
    _UCHAR i,j;

    printf("System font file: '%s'\n",project->info.sysfont);
    printf("Script file which will be autoloaded: '%s'\n",project->info.scriptfile);
    printf("Script procedure which will be autostarted: '%s'\n",project->info.scriptproc);
    printf("Palette file: '%s'\n",project->info.palettefile);
    printf("Bar is currently %sabled\n",(project->info.flags&PROJECT_FLAG_NOBAR)?"dis":"en");
    printf("Inventory file is currently %sabled\n",(project->info.flags&PROJECT_FLAG_NOINV)?"dis":"en");
    printf("Dialog defaults usuage is currently %sabled\n",(project->info.flags&PROJECT_FLAG_NODLG)?"dis":"en");
    printf("Parser is currently %sabled\n",(project->info.flags&PROJECT_FLAG_NOPARSER)?"dis":"en");
    printf("Project ID: '%s'\n",project->info.projectid);
    printf("Inventory file: '%s'\n",project->info.invfile);
    printf("Text file: '%s'\n",project->info.textfile);
    printf("Dialog border file: '%s'\n",project->info.border);
    printf("Dialog backdrop file: '%s'\n",project->info.backdrop);
    printf("System mouse cursor: '%s'\n",project->info.sysmouse);
    printf("Number of bytes per pixel: %u\n",project->info.bpp);
}

void
copystr(char* source,char* dest,_UINT maxlen) {
    _UINT i,j;

    j=strlen(source);
    if(maxlen>j) j=maxlen;

    dest[maxlen-1]=0;

    for(i=0;i<=strlen(source);i++) {
        dest[i]=source[i];
    }
}

int
main(int argc,char *argv[]) {
    _UINT i,j;
    _UCHAR quit;
    char command[255];

    printf("MAD Project Editor Version "PROJECT_VERSION" - (c) 1998, 1999 The MAD Crew\n\n");
    if (argc<2) {
        printf("usuage: prjedit [-n] file.prj\n\n");
        printf("-n		create a new project file\n\n");
        printf("As you might have guessed, the parameters are cAsE sEnSiTiVe\n");
        exit(1);
    }
    parseparams(argv,argc);

    printf("using file '%s'\n",filename);
    if (!(project=new PROJECT)) {
       printf("unable to create project object\n");
       exit(1);
    }
    if (!create_project) {
 	i=project->initreadwrite(filename);
	if(i!=PROJECT_OK) {
	    printf("Unable to open project file '%s': %s\n",filename,project->err2msg(i));
            exit(i);
	}
    } else {
        i=project->create(filename);
	if(i!=PROJECT_OK) {
	    printf("Unable to create project file '%s': %s\n",filename,project->err2msg(i));
	    exit(i);
	}
    }

    quit=0;
    while(!quit) {
        printf("choice>");
	gets(command);
	memset(filename,0,64);
        switch (command[0]) {
            case '?':
            case 'h': printhelp(); break;
            case 'r': printconf(); break;
	    case 'p': if (sscanf(command,"%c%s",&i,&filename)!=2) {
                          printf("filename: ");
                          gets(filename);
                      }
                      copystr(filename,project->info.palettefile,12);
                      break;
            case 's': if (sscanf(command,"%c%s",&i,&filename)!=2) {
                          printf("filename: ");
			  gets(filename);
                      }
                      copystr(filename,project->info.scriptfile,12);
                      break;
            case 'c': if (sscanf(command,"%c%s",&i,&filename)!=2) {
			  printf("filename: ");
                          gets(filename);
                      }
                      copystr(filename,project->info.scriptproc,32);
                      break;
            case 'f': if (sscanf(command,"%c%s",&i,&filename)!=2) {
			  printf("filename: ");
                          gets(filename);
		      }
                      copystr(filename,project->info.sysfont,12);
                      break;
            case 't': if (sscanf(command,"%c%s",&i,&filename)!=2) {
                          printf("filename: ");
			  gets(filename);
                      }
                      copystr(filename,project->info.textfile,12);
                      break;
            case 'B': printf("Bar is now ");
                      if(project->info.flags&PROJECT_FLAG_NOBAR) {
                          project->info.flags&=~PROJECT_FLAG_NOBAR;
                          printf("en");
                      } else {
                          project->info.flags|=PROJECT_FLAG_NOBAR;
                          printf("dis");
                      };
                      printf("abled\n");
                      break;
            case 'I': printf("Inventory is now ");
                      if(project->info.flags&PROJECT_FLAG_NOINV) {
                          project->info.flags&=~PROJECT_FLAG_NOINV;
                          printf("en");
                      } else {
                          project->info.flags|=PROJECT_FLAG_NOINV;
                          printf("dis");
                      };
                      printf("abled\n");
                      break;
            case 'D': printf("Dialog default usuage is now ");
                      if(project->info.flags&PROJECT_FLAG_NODLG) {
                          project->info.flags&=~PROJECT_FLAG_NODLG;
                          printf("en");
                      } else {
                          project->info.flags|=PROJECT_FLAG_NODLG;
                          printf("dis");
                      };
                      printf("abled\n");
                      break;
            case 'P': printf("Parser is now ");
                      if(project->info.flags&PROJECT_FLAG_NOPARSER) {
                          project->info.flags&=~PROJECT_FLAG_NOPARSER;
                          printf("en");
                      } else {
                          project->info.flags|=PROJECT_FLAG_NOPARSER;
                          printf("dis");
                      };
                      printf("abled\n");
                      break;
            case 'd': if (sscanf(command,"%c%s",&i,&filename)!=2) {
                          printf("Game ID: ");
                          gets(filename);
                      }
                      copystr(filename,project->info.projectid,8);
                      break;
            case 'b': if (sscanf(command,"%c%s",&i,&filename)!=2) {
                          printf("Backdrop: ");
                          gets(filename);
                      }
                      copystr(filename,project->info.backdrop,12);
                      break;
            case 'o': if (sscanf(command,"%c%s",&i,&filename)!=2) {
                          printf("Border: ");
                          gets(filename);
                      }
                      copystr(filename,project->info.border,12);
                      break;
            case 'i': if (sscanf(command,"%c%s",&i,&filename)!=2) {
                          printf("filename: ");
                          gets(filename);
                      }
                      copystr(filename,project->info.invfile,12);
                      break;
            case 'm': if (sscanf(command,"%c%s",&i,&filename)!=2) {
	                  printf("filename: ");
                          gets(filename);
                      }
                      copystr(filename,project->info.sysmouse,12);
                      break;
            case 'l': if (sscanf(command,"%c%s",&i,&filename)!=2) {
	                  printf("number of colors: ");
                          gets(filename);
                      }
                      project->info.bpp=atoi(filename);
                      break;
            case 'q': quit=1; break;
             default: printf("unknown command '%c' -- use ? or h for help\n",command[0]);
	       	      break;
        }
    }
    project->done();
    delete project;
    return 0;
}
