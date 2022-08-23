#ifdef DOS
#include <dpmi.h>
#endif

#include "mad.h"
#include "types.h"

/* are we already included? */
#ifndef __CONTROLS_INCLUDED__
/* no. set flag we are included, and define things */
#define __CONTROLS_INCLUDED__

#define CONTROLS_KEYUP    72
#define CONTROLS_KEYDOWN  80
#define CONTROLS_KEYLEFT  75
#define CONTROLS_KEYRIGHT 77
#define CONTROLS_KEYSTOP  76         /* numberic 5 */

#define CONTROLS_KEY_ESCAPE 1        /* escape */
#define CONTROLS_KEY_1     2         /* 1 */
#define CONTROLS_KEY_2     3         /* 2 */
#define CONTROLS_KEY_3     4         /* 3 */
#define CONTROLS_KEY_4     5         /* 4 */
#define CONTROLS_KEY_5     6         /* 5 */
#define CONTROLS_KEY_6     7         /* 6 */
#define CONTROLS_KEY_7     8         /* 7 */
#define CONTROLS_KEY_8     9         /* 8 */
#define CONTROLS_KEY_9     10        /* 9 */
#define CONTROLS_KEY_0     11        /* 0 */
#define CONTROLS_KEY_MINUS 12        /* -/_ */
#define CONTROLS_KEY_EQUAL 13        /* =/+ */
#define CONTROLS_KEY_BSPACE 14       /* backspace */
#define CONTROLS_KEY_TAB   15        /* enter */
#define CONTROLS_KEY_Q     16        /* q */
#define CONTROLS_KEY_W     17        /* w */
#define CONTROLS_KEY_E     18        /* e */
#define CONTROLS_KEY_R     19        /* r */
#define CONTROLS_KEY_T     20        /* t */
#define CONTROLS_KEY_Y     21        /* y */
#define CONTROLS_KEY_U     22        /* u */
#define CONTROLS_KEY_I     23        /* i */
#define CONTROLS_KEY_O     24        /* o */
#define CONTROLS_KEY_P     25        /* p */
#define CONTROLS_KEY_LBRACKET 26     /* [/{ */
#define CONTROLS_KEY_RBRACKET 27     /* ]/} */
#define CONTROLS_KEY_ENTER 28        /* enter */
#define CONTROLS_KEY_A     30        /* a */
#define CONTROLS_KEY_S     31        /* s */
#define CONTROLS_KEY_D     32        /* d */
#define CONTROLS_KEY_F     33        /* f */
#define CONTROLS_KEY_G     34        /* g */
#define CONTROLS_KEY_H     35        /* h */
#define CONTROLS_KEY_J     36        /* j */
#define CONTROLS_KEY_K     37        /* k */
#define CONTROLS_KEY_L     38        /* l */
#define CONTROLS_KEY_COLON 39        /* ;/: */
#define CONTROLS_KEY_TILDE 41        /* tilde */
#define CONTROLS_KEY_LSHIFT 42       /* left shift */
#define CONTROLS_KEY_Z     44        /* z */
#define CONTROLS_KEY_X     45        /* x */
#define CONTROLS_KEY_C     46        /* c */
#define CONTROLS_KEY_V     47        /* v */
#define CONTROLS_KEY_B     48        /* b */
#define CONTROLS_KEY_N     49        /* n */
#define CONTROLS_KEY_M     50        /* m */
#define CONTROLS_KEY_COMMA 51        /* ,/< */
#define CONTROLS_KEY_DOT   52        /* ./> */
#define CONTROLS_KEY_SLASH 53        /* / / ? */
#define CONTROLS_KEY_RSHIFT 54       /* right shift */
#define CONTROLS_KEY_ALT   56        /* alt key */
#define CONTROLS_KEY_SPACE 57        /* space bar */
#define CONTROLS_KEY_F1    59        /* f1 */
#define CONTROLS_KEY_F2    60        /* f2 */
#define CONTROLS_KEY_F3    61        /* f3 */
#define CONTROLS_KEY_F4    62        /* f4 */
#define CONTROLS_KEY_F5    63        /* f5 */
#define CONTROLS_KEY_F6    64        /* f6 */
#define CONTROLS_KEY_F7    65        /* f7 */
#define CONTROLS_KEY_F8    66        /* f8 */
#define CONTROLS_KEY_F9    67        /* f9 */
#define CONTROLS_KEY_F10   68        /* f10 */

#define CONTROLS_KEY_HOME  71        /* home */
#define CONTROLS_KEY_PAGEUP 73       /* page up */
#define CONTROLS_KEY_END    79       /* end */
#define CONTROLS_KEY_PAGEDOWN 81     /* page down */
#define CONTROLS_KEY_INSERT 82       /* insert */
#define CONTROLS_KEY_DELETE 83       /* delete */

#define CONTROLS_KEY_KPSUB  74       /* keypad - */
#define CONTROLS_KEY_KPADD  78       /* keypad + */

#define CONTROLS_KEY_F11   87        /* f11 */
#define CONTROLS_KEY_F12   88        /* f12 (emergency exit) */

#define CONTROLS_HOTKEY_SAVEGAME    CONTROLS_KEY_F5      /* f5 = save game */
#define CONTROLS_HOTKEY_LOADGAME    CONTROLS_KEY_F7      /* f7 = load game */
#define CONTROLS_HOTKEY_INVENTORY   CONTROLS_KEY_TAB     /* tab = inventory */

#define CONTROLS_QUIT      CONTROLS_KEY_ESCAPE  /* escape quits */
#define CONTROLS_CONSOLE   CONTROLS_KEY_TILDE   /* tilde activates console */

#define CONTROLS_BUTTON1   1	       /* left mouse button */
#define CONTROLS_BUTTON2   2   	    /* right mouse button */

class CONTROLS {
public:
        void   init();
        void   done();
        void   poll();

        _UINT  getxpos();
        _UINT  getypos();
        _UCHAR button1pressed();
        _UCHAR button2pressed();
        _UCHAR iskeydown(_UCHAR no);

        char   resolvecode(_UCHAR);

        void   waitnobutton();
        void   waitevent();

        _UCHAR getbuttonstatus();
        _UCHAR getch();
        _UCHAR peekcode();

        void   clearkey(_UCHAR);

        _UCHAR keydown();

        _UINT  sysmouse_sprno;
};
#endif
