#include <stdio.h>
#include <stdlib.h>
#include "../../../source/types.h"

#define BLOCKSIZE 768
char blockdata[BLOCKSIZE];

int
main(int argc, char *argv[]) {
    FILE *pcx,*out;
    _ULONG l,done;
    _UINT bsize;

    printf("STRiPAL Version 2.0 - (c) 1999 The MAD Crew\n\n");
    /* verify if there are 2 parameters */
    if(argc!=3) {
        /* there arent, so die */
	printf("Usuage: STRIPAL [infile.pcx] [outfile.pal]\n");
	exit(0xff);
    }
    printf("Copying palette from %s to %s...",argv[1],argv[2]);
    /* try to open the pcx file */
    if ((pcx=fopen(argv[1],"rb"))==NULL) {
        /* couldnt open it, so die */
	printf(" unable to open %s\n",argv[1]);
	exit(1);
    }
    /* try to create the pal file */
    if ((out=fopen(argv[2],"wb"))==NULL) {
        fclose(pcx);
	printf(" unable to create %s\n",argv[2]);
	exit(2);
    }
    /* figure out the file size */
    fseek(pcx,0,SEEK_END);
    l=ftell(pcx);
    /* now seek just before the palette */
    fseek(pcx,l-768,SEEK_SET);
    /* read the palette */
    if (fread(blockdata,768,1,pcx)!=1) {
        /* this failed, so die */
   	fclose(out); fclose(pcx);
        printf(" unable to read %s\n",argv[1]);
        exit(3);
    }
    /* dump the palette to the other file */
    if (fwrite(blockdata,768,1,out)!=1) {
        /* this failed, so die */
        fclose(out); fclose(pcx);
        printf(" unable to write %s\n",argv[2]);
        exit(4);
    }
    printf(" done\n");
    fclose(out);
    fclose(pcx);
    return 0;
}
