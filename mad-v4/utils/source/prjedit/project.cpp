#include <stdio.h>
#include <string.h>
#include "../../../source/include/project.h"
#include "../../../source/include/types.h"

char
*PROJECT::err2msg(_UCHAR errno) {
    switch (errno) {
            case PROJECT_OK: return "everything is ok";
       case PROJECT_OPENERR: return "unable to open project file";
       case PROJECT_READERR: return "read error";
       case PROJECT_WRITERR: return "write error";
  case PROJECT_NOMADPROJECT: return "file is not a mad project";
  case PROJECT_WRONGVERSION: return "this is a wrong version";
      case PROJECT_CREATERR: return "unable to create project file";
    }
    return "unknown error. help!";
}

_UCHAR
PROJECT::init(char *fname) {
    FILE *f;

    /* set projectfile to NULL, so it will not be updated by done */
    projectfile=NULL;

    /* try to open the file */
    if ((f=fopen(fname,"rb"))==NULL) {
        /* unable to open it, so return PROJECT_OPENERR */
        return PROJECT_OPENERR;
    }
    /* read the project info into info */
    if(!fread(&info,sizeof(struct PROJECTINFO),1,f)) {
        /* couldnt read it, so return PROJECT_READERR */
        return PROJECT_READERR;
    }
    /* be a neat program and close the file */
    fclose(f);

    /* is this a mad project file? */
    if(info.idcode!=PROJECT_MAGICVALUE) {
        /* no, so return PROJECT_NOMADPROJECT */
        return PROJECT_NOMADPROJECT;
    }
    /* is the version ok? */
    if (info.versiono!=PROJECT_VERSIONO) {
        /* no, so return PROJECT_WRONGVERSION */
        return PROJECT_WRONGVERSION;
    }
    /* everything went ok, so return PROJECT_OK */
    return PROJECT_OK;
}

_UCHAR
PROJECT::initreadwrite(char *fname) {
    /* try to open the file */
    if ((projectfile=fopen(fname,"r+b"))==NULL) {
        /* unable to open it, so return PROJECT_OPENERR */
        return PROJECT_OPENERR;
    }
    /* read the project info into info */
    if(!fread(&info,sizeof(struct PROJECTINFO),1,projectfile)) {
        /* couldnt read it, so return PROJECT_READERR */
        return PROJECT_READERR;
    }

    /* is this a mad project file? */
    if(info.idcode!=PROJECT_MAGICVALUE) {
        /* no, so return PROJECT_NOMADPROJECT */
        return PROJECT_NOMADPROJECT;
    }
    /* is the version ok? */
    if (info.versiono!=PROJECT_VERSIONO) {
        /* no, so return PROJECT_WRONGVERSION */
        return PROJECT_WRONGVERSION;
    }
    /* everything went ok, so return PROJECT_OK */
    return PROJECT_OK;
}

_UCHAR
PROJECT::create(char *fname) {
    _ULONG i;

    /* try to create the file */
    if ((projectfile=fopen(fname,"wb"))==NULL) {
        /* unable to open it, so return PROJECT_OPENERR */
	return PROJECT_OPENERR;
    }

    /* zero out the project data */
    memset(&info,0,sizeof(struct PROJECTINFO));

    /* everything went ok, so return PROJECT_OK */
    return PROJECT_OK;
}

void
PROJECT::done() {
    /* if projectfile is NULL, the file was opened read only and no
       update is necessary */
    if(projectfile==NULL) return;

    /* first go to the beginning of the file so any previous data will
       be overwritten */
    rewind(projectfile);

    /* fill in the magic value so other programs will identify this file
       as a mad project file */
    info.idcode=PROJECT_MAGICVALUE;
    /* and set the correct version number */
    info.versiono=PROJECT_VERSIONO;
    /* now write the file info buffer */
    if(!fwrite(&info,sizeof(struct PROJECTINFO),1,projectfile)) {
        /* couldnt write it, so display PROJECT_WRITERR */
	printf ("project.done: %s\n",err2msg(PROJECT_WRITERR));
	return;
    }

    /* close the file */
    fclose(projectfile);
}
