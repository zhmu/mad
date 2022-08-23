#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../source/include/types.h"

/*
    _UCHAR readline(_ULONG lineno,_UINT filesize,char *s,char *data)

    this will read line [lineno] from buffer [data] to [s]. it will return
    0 if everything went ok, or 1 if the line doesn't exist
*/
_UCHAR
readline(_ULONG lineno,_UINT filesize,char *s,char *data) {
    _ULONG i,line,c;

    /* scan the entire buffer */
    s[0]=0; line=0; c=0;
    for(i=0;i<filesize;i++) {
        /* are we scanning the line we search? */
        if (line==lineno) {
           /* yes, add the char to the return buffer */
           s[c]=data[i];
           c++;
        }
        /* is there a cr? */
        if (data[i]==0xd) {
            /* yes. we are done, remove newline, return */
            if (line==lineno) {
                /* next line removes the newline from the returned string */
                s[c-1]=0;
                /* we had success. return 1 */
                return 1;
            }
            /* skip lf, go to next line */
            i++;
            line++;
        }
    }
    /* line doesn't exists. return 0 */
    return 0;
}
