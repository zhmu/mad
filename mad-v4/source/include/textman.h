/* are we already included ? */
#ifndef __TEXT_INCLUDED__
/* no. Set flag we are included and define things */
#define __TEXT_INCLUDED__

#include "types.h"

#define TEXTMAN_VERSION      "1.0"          /* font manager version */
#define TEXTMAN_VERSIONO     100            /* font manager version */
#define TEXTMAN_MAGICID      0x21545854     /* font file magic number */

#define TEXTMAN_ENCRYPTSTRING "The User Of InCredible power"

struct TEXTHEADER {
    _ULONG   magic;             /* magic value, should be MAGICID */
    _UINT    version;		/* version number, 100 for 1.00, 201 for 2.01 etc. */
    _ULONG   nofstrings;        /* number of strings */
    _ULONG   offset;            /* offset table offset */
};

class TEXTMAN {
public:
       TEXTMAN();
       void   init(char*);
       void   done();

       void   getstring(_ULONG,char*,_ULONG);

private:
       struct TEXTHEADER hdr;
       char   *textdata,*offsetdata;
};
#endif  /* __TEXT_INCLUDED__ */
