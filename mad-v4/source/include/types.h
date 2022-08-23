#include <sys/types.h>
#include <stdint.h>

/* are we already included? */
#ifndef __TYPES_INCLUDED__
/* no. set flag we are included, and define things */
#define __TYPES_INCLUDED__

#ifndef _UCHAR
#define _UCHAR uint8_t
#endif

#ifndef _UINT
#define _UINT uint16_t
#endif

#ifndef _ULONG
#define _ULONG uint32_t
#endif

#ifndef _SCHAR
#define _SCHAR int8_t
#endif

#ifndef _SINT
#define _SINT int16_t
#endif

#ifndef _SLONG
#define _SLONG int32_t
#endif

#endif
