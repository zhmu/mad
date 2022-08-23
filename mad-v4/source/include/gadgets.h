/* are we already defined? */
#ifndef __GADGETS_DEFINED__
/* no, say we are, and define things */
#define __GADGETS_DEFINED__

#include "types.h"

void stripquotes (char *instr,char *fname);
void delay_ms(_UINT timeout);
int pow(int a, int b);

void box(_SINT,_SINT,_UINT,_UINT,_ULONG,char*);
void grayoutbox(_SINT,_SINT,_UINT,_UINT,char*);
void hline(_SINT,_SINT,_SINT,_ULONG,char*);
void vline(_SINT,_SINT,_SINT,_ULONG,char*);

#endif
