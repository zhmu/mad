#ifndef __GAME_DEFINED__
#define __GAME_DEFINED__

#include <stdio.h>
#include "mad.h"
#include "types.h"

#define MAD_SAVEGAME_MAX_SAVED_GAMES      128

#define MAD_SAVEGAME_ID 0x21475653        /* savegame magic value */
#define MAD_SAVEGAME_VERSION 100          /* savegame version */
#define MAD_SAVEGAME_PRJID_LEN 8          /* project id length */
#define MAD_SAVEGAME_TITLE_LEN 64         /* saved game title length */
#define MAD_SAVEGAME_SCRIPT_LEN 16        /* script filename length */
#define MAD_SAVEGAME_PIC_LEN 16           /* picture filename length */

struct SAVEGAME {
    _ULONG id;                            /* id, should be MAD_SAVEGAME_ID */
    _UINT  version;                       /* version, should be MAD_SAVEGAME_VERSION */
    char   projectid[MAD_SAVEGAME_PRJID_LEN]; /* project id */
    char   title[MAD_SAVEGAME_TITLE_LEN];     /* saved game title */
    _ULONG checksum;                      /* checksum value */
    _ULONG nofobjects;                    /* number of objects stored */
    char   scriptfile[MAD_SAVEGAME_SCRIPT_LEN]; /* secondary script file name */
    _ULONG rootscript_eip;                /* EIP of root script */
    _ULONG script_eip;                    /* EIP of secondary script */
    char   picturefile[MAD_SAVEGAME_PIC_LEN]; /* current picture file name */
    char   reserved[896];                 /* unused */
};

void save_game();
void load_game();
void do_save(char*,char*);
void do_load(char*);

_UINT do_conversation(_UINT,_UINT,_UINT,_UINT,_ULONG);

#endif
