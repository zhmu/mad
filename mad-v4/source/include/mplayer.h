#ifdef MULTIPLAYER

#include "debugmad.h"
#include "mad.h"
#include "types.h"
#ifdef WINDOWS
#include <winsock2.h>
#endif
#ifdef DOS
#include <socket.h>
#endif
#ifdef UNIX
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

/* are we already included? */
#ifndef __MPLAYER_INCLUDED__
/* no. set flag we are included, and define things */
#define __MPLAYER_INCLUDED__

#define MPLAYER_DEFAULT_NAME    "Player"

#define MPLAYER_MAX_NODES       64
#define MPLAYER_NONODE          0xffff
#define MPLAYER_SOCKETBUF_LEN   1024

#define MPLAYER_MAX_PLAYERNAME  16

#define MPLAYER_CMD_NOP         0       /* does nothing */
#define MPLAYER_CMD_JOIN        1       /* join a game */
#define MPLAYER_CMD_LEAVE       2       /* leave a game */
#define MPLAYER_CMD_MESSAGE     3       /* message */
#define MPLAYER_CMD_MOD         4       /* modification */
#define MPLAYER_CMD_INFO        5       /* info packet */


#define MPLAYER_CMD_JOIN_RESP   0x80    /* join response */

#define MPLAYER_JOIN_RES_OK     0       /* you're in! */
#define MPLAYER_JOIN_RES_BADID  1       /* wrong project id */
#define MPLAYER_JOIN_RES_FULL   2       /* game full */

#define MPLAYER_STAT_CONN       0       /* connected, but nothing else */
#define MPLAYER_STAT_JOINED     1       /* joined in game */

#define MPLAYER_WHAT_ROOMNO     0       /* room number changes */
#define MPLAYER_WHAT_COORDS     1       /* coordinates */
#define MPLAYER_WHAT_DESTCOORDS 2       /* destination coordinates */
#define MPLAYER_WHAT_SPEED      3       /* speed */
#define MPLAYER_WHAT_VISIBLE    4       /* visibility */
#define MPLAYER_WHAT_MOVESEQ    5       /* move sequence */
#define MPLAYER_WHAT_ANIMNO     6       /* animation number */
#define MPLAYER_WHAT_FRAMENO    7       /* frame number */
#define MPLAYER_WHAT_ANIMATING  8       /* animating flag */
#define MPLAYER_WHAT_CLIP       9       /* clipping */
#define MPLAYER_WHAT_MASKDIR    10      /* mask direction */
#define MPLAYER_WHAT_SPEEDS     11      /* all speeds */
#define MPLAYER_WHAT_MOVING     12      /* moving flag */

#ifdef UNIX
#define	SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define ioctlsocket ioctl
#define closesocket close
#define getsocketerror errno
#endif

#ifdef WINDOWS
#define getsocketerror WSAGetLastError()
#endif

struct NODE {
    SOCKET      socketno;       /* socket number or MPLAYER_NONODE is unused */
    struct sockaddr_in addr;           /* node address */
    _UINT       status;         /* node status */
    _ULONG      time_joined;    /* time joined */
    char        name[MPLAYER_MAX_PLAYERNAME]; /* node name */
    _UINT       obj_no;         /* ego object number */
};

struct PACKET {
    char        projectid[8];   /* the game id code */
    _ULONG      time;           /* time on which this packet has been sent */
    _UCHAR      command;        /* the command */
};

struct JOIN_PACKET {
    char        name[MPLAYER_MAX_PLAYERNAME];
};

struct JOIN_PACKET_RESPONSE {
    _UCHAR      result;         /* result of joining */
    _UINT       nof_nodes;      /* number of nodes */
    char        name[MPLAYER_MAX_PLAYERNAME]; /* other box's name */
};

struct MOD_PACKET {
    _UINT       what;           /* what has changed? */
    _ULONG      old_value;      /* old value */
    _ULONG      old_value2;     /* old value2 */
    _ULONG      new_value;      /* new value */
    _ULONG      new_value2;     /* new value2 */
};

struct INFO_PACKET {
    _SINT  x,y;                         /* x and y coordinates */
    _UINT  frameno;                     /* current frame number */
    _UCHAR move;                        /* nonzero if the object must move */
    _UCHAR animating;                   /* nonzero if the object is animating */
    _ULONG roomno;                      /* room number */
    _UCHAR visible;                     /* nonzero if visible */
    _SINT  destx,desty;                 /* destination x and y */
    _UINT  speed;                       /* speed of the animation */
    _UINT  h_speed,v_speed;             /* increment values */
    _UCHAR mask_dir;                    /* direction to check masking */
    _UCHAR clip;                        /* nonzero if object must be clipped */
};

class MPLAYER {
public:
    MPLAYER();

    void    init();
    void    done();
    void    poll();

    void    joingame(char*,struct sockaddr_in*);
    void    leavegame();

    void    joingamedlg();
    void    talkdlg();

    void    send_change(_UINT,_ULONG,_ULONG);
    void    send_change_ext(_UINT,_ULONG,_ULONG,_ULONG,_ULONG);

private:
    _UCHAR  in_game;            /* are we currently in a game? */

    SOCKET  s;

    struct  NODE node[MPLAYER_MAX_NODES];

    _UINT   findfreenode();

    void    sendtoall(_UCHAR,char*,_UCHAR);
    void    sendtonode(_UINT,_UCHAR,char*,_UCHAR);

    void    process_packet(_UCHAR,char*,_UCHAR);

    void    send_ego_info(_UINT);
    void    apply_ego_info(_UINT,struct INFO_PACKET*);
};
#endif
#endif /* MULTIPLAYER */
