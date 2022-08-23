/*
                           MPLAYER.CPP

                    Multiplayer Engine for MAD

                     (c) 2000 The MAD Crew

  Description:
  This will handle the multiplayer issues.

  Todo: Nothing

  Portability notes: Use #ifdef's when code must differ for other platforms.
*/
#ifdef WINDOWS
#include <winsock2.h>
#endif
#ifdef UNIX
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "controls.h"
#include "dlgman.h"
#include "fontman.h"
#include "gfx.h"
#include "objman.h"
#include "parser.h"
#include "mad.h"
#include "maderror.h"
#include "mplayer.h"
#include "project.h"
#include "types.h"

#define MAD_ERROR_900 "Error 900: Unable to create socket"
#define MAD_ERROR_901 "Error 901: Unable to bind address to socket"
#define MAD_ERROR_902 "Error 902: Socket refuses to listen"
#define MAD_ERROR_903 "Error 903: ioctlsocket() failed"
#define MAD_ERROR_950 "Error 950: WSAStartup() failed"

#define MAD_MPLAYER_JOINGAME_TITLE "Join game"
#define MAD_MPLAYER_JOININGAME_TITLE "Joining game"

#define MAD_MPLAYER_JOIN_TEXT "Joining game at %s, connecting"
#define MAD_MPLAYER_RESOLV_TEXT "Resolving %s"
#define MAD_MPLAYER_JOIN_REP_TEXT "Joining game at %s, waiting for reply"

#define MAD_MPLAYER_OK " OK "
#define MAD_MPLAYER_CANCEL " Cancel "
#define MAD_MPLAYER_ADDRESS "Address"
#define MAD_MPLAYER_ADDR "255.255.255.255"

#define MPLAYER_CONNFAILED "Connection failed"
#define MPLAYER_LOOKUPFAILED "Host lookup failed"

#ifdef MULTIPLAYER
/*
 * MPLAYER::MPLAYER()
 *
 * This is the constructor.
 *
 */
MPLAYER::MPLAYER() {
    _UINT i;

    /* mark the socket as invalid */
    s=INVALID_SOCKET;

    /* kill all nodes */
    for(i=0;i<MPLAYER_MAX_NODES;i++) {
        node[i].socketno=INVALID_SOCKET;
    }

    /* we haven't joined a game yet */
    in_game=0;
}

/*
 * MPLAYER::findfreenode()
 *
 * This will search for a free node. It will return MPLAYER_NONODE if nothing
 * can be found.
 *
 */
_UINT
MPLAYER::findfreenode() {
    _UINT i;

    /* scan them all */
    for (i=0;i<MPLAYER_MAX_NODES;i++) {
        /* node in use? */
        if(node[i].socketno==INVALID_SOCKET) {
            /* no. this is the one */
            return i;
        }
    }

    /* no nodes available */
    return MPLAYER_NONODE;
}

/*
 * MPLAYER::init()
 *
 * This will initialize the multiplayer services.
 *
 */
void
MPLAYER::init() {
    struct sockaddr_in sin;
    struct in_addr     myaddr;
    #ifdef WINDOWS
    WSADATA wsaData;
    #endif
    _ULONG i;

    /* do we run under Windows? */
    #ifdef WINDOWS
        /* yup. initialize the sockets first */
        if (WSAStartup (MAKEWORD (2, 0), &wsaData) != 0) {
            /* this failed. die (error 950: WSAStartup() failed) */
            die(MAD_ERROR_950);
        }
    #endif

    /* try to allocate a socket */
    if((s=socket(AF_INET, SOCK_STREAM, 0))==INVALID_SOCKET) {
        /* this failed. die (error 900: unable to create socket) */
        die(MAD_ERROR_900);
    }

    /* set up the binding structure */
    myaddr.s_addr = htonl (INADDR_ANY);

    sin.sin_family = AF_INET;
    sin.sin_addr = myaddr;
    sin.sin_port = htons (MAD_SOCKETNO);

    /* bind the correct address to the socket */
    if (bind (s, (struct sockaddr*)&sin, sizeof (sin))==SOCKET_ERROR) {
        /* this failed. die (error 901: unable to create socket) */
        die(MAD_ERROR_901);
    }

    /* listen for connections on this socket */
    if (listen (s, 32) == SOCKET_ERROR) {
        /* this failed. die (error 902: socket refuses to listen) */
        die(MAD_ERROR_902);
    }

    /* toss the socket into non-blocking mode */
    i=1;
    if (ioctlsocket (s, FIONBIO, &i) == SOCKET_ERROR) {
        /* this failed. die (error 903: ioctlsocket() failed) */
        die(MAD_ERROR_903);
    }
}

/*
 * MPLAYER::done()
 *
 * This will initialize the multiplayer services.
 *
 */
void
MPLAYER::done() {
    _UINT i;

    /* leave the current game */
    leavegame();

    /* kill all nodes */
    for(i=0;i<MPLAYER_MAX_NODES;i++) {
        /* in use? */
        if (node[i].socketno!=INVALID_SOCKET) {
            /* yup. kill it */
            closesocket(node[i].socketno);
            node[i].socketno=INVALID_SOCKET;
        }
    }

    /* do we have a valid socket? */
    if (s!=INVALID_SOCKET) {
        /* yup. kill it */
        closesocket(s);
        /* make sure we never do this again */
        s=INVALID_SOCKET;
    }

    #ifdef WINDOWS
        /* deinitialize the sockets */
        WSACleanup();
    #endif
}

/*
 * MPLAYER::poll()
 *
 * This will poll the multiplayer services.
 *
 */
void
MPLAYER::poll() {
    SOCKET i;
    struct sockaddr_in sa;
    char tempstr[MAD_TEMPSTR_SIZE];
    _UINT no;
    unsigned int j;
    _ULONG len;
    char   socketbuf[MPLAYER_SOCKETBUF_LEN];

    /* scan all nodes */
    for (no=0;no<MPLAYER_MAX_NODES;no++) {
        /* is this node in use? */
        if (node[no].socketno != INVALID_SOCKET) {
            /* yup. check for a message */
            while (((len = recv (node[no].socketno, socketbuf, MPLAYER_SOCKETBUF_LEN, 0)) != 0) && (len != SOCKET_ERROR)) {
                /* now, we have [len] bytes in [socketbuf] received from node
                   [no]. process them */
                process_packet (no, socketbuf, len);
            }
        }
    }

    /* do we have new connections that need to be approved? */
    j = sizeof (sockaddr);
    while ((i = accept (s, (struct sockaddr*)&sa, &j)) != INVALID_SOCKET) {
        /* scan for a free node */
        if ((no = findfreenode()) == MPLAYER_NONODE) {
            /* there isn't one. tell the box we are full (XXX) */
            closesocket(no);
        } else {
            /* use this node */
            node[no].socketno = i;
            node[no].status = MPLAYER_STAT_CONN;
            memcpy((void*)&node[no].addr,(void*)&sa,sizeof(sa));
        }
    }
}

/*
 * MPLAYER::sendtoall(_UCHAR cmd,char* buf,_UCHAR size)
 *
 * This will send [size] bytes of [buf] to all nodes connected. The packet will
 * get a header before it will be sent, and the command field will be set to
 * [cmd].
 *
 */
void
MPLAYER::sendtoall(_UCHAR cmd,char* buf,_UCHAR size) {
    _UINT i;

    /* scan all nodes */
    for (i=0;i<MPLAYER_MAX_NODES;i++) {
        /* send it (sendtonode() will check for connected/unconnected nodes) */
        sendtonode (i, cmd, buf, size);
    }
}

/*
 * MPLAYER::sendtonode(_UINT no,_UCHAR cmd,char* buf,_UCHAR size)
 *
 * This will send [size] bytes of [buf] to node [no]. The packet will get a
 * header before it will be sent, and the command field will be set to [cmd]. If
 * [buf] is NULL, only a header will be sent.
 *
 */
void
MPLAYER::sendtonode(_UINT no,_UCHAR cmd,char* buf,_UCHAR size) {
    char* ptr;
    char  tempstr[MAD_TEMPSTR_SIZE];
    struct PACKET* p;

    /* is this node connected? */
    if(node[no].socketno==INVALID_SOCKET) {
        /* no. just return */
        return;
    }

    /* if there's a NULL buffer, the size is zero too */
    if (buf==NULL) {
        size=0;
    }

    /* allocate memory for the packet */
    if ((ptr = (char*)malloc (size + sizeof (struct PACKET))) == NULL) {
        /* this failed. die (error 2: out of memory) */
        sprintf(tempstr,MAD_ERROR_2,size+sizeof(struct PACKET));
        die(tempstr);
    }

    /* generate a header */
    p=(struct PACKET*)ptr;

    /* fill it in */
    memcpy((char*)p->projectid,(char*)project->info.projectid,sizeof(project->info.projectid));
    p->command=cmd;

    /* don't copy a NULL buffer! */
    if (buf!=NULL) {
        /* add the raw data */
        memcpy((ptr+sizeof(struct PACKET)),buf,size);
    }

    /* send the data */
    send (node[no].socketno, ptr, (size + sizeof (struct PACKET)), 0);
}

/*
 * MPLAYER::joingame(char* addr,struct SOCKADDR* sa)
 *
 * This will attempt to join the game at host [addr]. If [sa] is not NULL, it
 * will be used as address.
 *
 */
void
MPLAYER::joingame(char* addr,struct sockaddr_in* sa) {
    _UINT    dialogno,height,width,ok,cancel,i,text;
    _ULONG   j;
    DIALOG*  dlg;
    char     tempstr[MAD_TEMPSTR_SIZE];
    char*    displaydata;
    struct   sockaddr_in sin;
    struct   in_addr defaddr;
    struct   hostent* hp;
    struct   hostent def;
    fd_set   fd;
    struct   timeval tv;
    struct   JOIN_PACKET jp;

    /* leave the current game first */
    leavegame();

    /* set height and width */
    height=4*fontman->getfontheight(fontman->getsysfontno()); width=250;

    /* create a dialog */
    dialogno=dlgman->createdialog(DLGMAN_DIALOG_FLAG_TITLEBAR,0,0,height,width);
    dlg=dlgman->dialog[dialogno];

    /* set the title */
    dlg->setitle(MAD_MPLAYER_JOININGAME_TITLE);

    /* center it */
    dlg->centerdialog();

    /* create the 'cancel' button */
    cancel=dlg->createcontrol(DLGMAN_CONTROL_TYPE_BUTTON);

    /* set it up */
    dlg->control[cancel]->move((width-fontman->getextwidth(fontman->getsysfontno(),MAD_MPLAYER_CANCEL))/2,2*fontman->getfontheight(fontman->getsysfontno())+4);
    dlg->control[cancel]->resize(fontman->getfontheight(fontman->getsysfontno()),fontman->getextwidth(fontman->getsysfontno(),MAD_MPLAYER_CANCEL));
    dlg->control[cancel]->setcaption(MAD_MPLAYER_CANCEL);
    dlg->control[cancel]->setfocus(1);

    /* create the text */
    text=dlg->createcontrol(DLGMAN_CONTROL_TYPE_LABEL);

    /* set it up */
    sprintf(tempstr,MAD_MPLAYER_RESOLV_TEXT,addr);
    dlg->control[text]->move((width-fontman->getextwidth(fontman->getsysfontno(),tempstr))/2,fontman->getfontheight(fontman->getsysfontno())/2);
    dlg->control[text]->resize(0,width);
    dlg->control[text]->setcaption(tempstr);

    /* allocate memory for a temponary display buffer */
    if((displaydata=(char*)malloc(GFX_DRIVER_HRES*GFX_DRIVER_VRES*GFX_BPP))==NULL) {
        /* out of memory. die (Error 2: out of memory (allocation of bytes failed) */
        sprintf(tempstr,MAD_ERROR_2,(GFX_DRIVER_HRES*GFX_DRIVER_VRES*GFX_BPP));
        die(tempstr);
    }

    dlgman->drawdialog(dialogno,GFX_DRIVER_HRES,GFX_DRIVER_VRES,displaydata);

    if (sa==NULL) {
        /* try to resolve the host by IP */
        if ((defaddr.s_addr = inet_addr (addr)) == INADDR_NONE) {
           /* resolve the host name */
            if ((hp = gethostbyname (addr)) == NULL) {
                /* this failed. complain */
                free (displaydata); dlgman->destroydialog (dialogno);
                gfx->console_print(MPLAYER_LOOKUPFAILED);
                return;
            }
            i = hp->h_length;
        } else {
            def.h_addr = (char*)&defaddr;
            hp = &def;
            i = sizeof (sin.sin_addr);
        }
        memcpy ((char*)&sin.sin_addr, hp->h_addr, i);
    } else {
        memcpy ((char*)&sin.sin_addr, (char*)&sa, sizeof (sin.sin_addr));
    }

    /* refresh the text (to: joining game at x.x.x.x) */
    sprintf(tempstr,MAD_MPLAYER_JOIN_TEXT,inet_ntoa (sin.sin_addr));
    dlg->control[text]->move((width-fontman->getextwidth(fontman->getsysfontno(),tempstr))/2,fontman->getfontheight(fontman->getsysfontno())/2);
    dlg->control[text]->resize(0,width);
    dlg->control[text]->setcaption(tempstr);

    /* redraw the dialog */
    dlgman->drawdialog(dialogno,GFX_DRIVER_HRES,GFX_DRIVER_VRES,displaydata);

    /* create a socket */
    if ((node[0].socketno = socket (AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        /* this failed. die (error 900: unable to create socket) */
        die(MAD_ERROR_900);
    }
    node[0].status = MPLAYER_STAT_CONN;

    tv.tv_sec = 1;
    tv.tv_usec = 0;

    /* connect */
    sin.sin_family = AF_INET;
    sin.sin_port = htons (MAD_SOCKETNO);

    /* toss the socket into non-blocking mode */
    j=1;
    if (ioctlsocket (node[0].socketno, FIONBIO, &j) == SOCKET_ERROR) {
        /* this failed. die (error 903: ioctlsocket() failed) */
        die(MAD_ERROR_903);
    }

    if (connect (node[0].socketno, (struct sockaddr *)&sin, sizeof (struct sockaddr_in)) == SOCKET_ERROR) {
        /* if the error is EWOULDBLOCK, ignore it (it's supposed to say this) */
        if (getsocketerror != EWOULDBLOCK) {
            /* the connection failed */
            gfx->console_print(MPLAYER_CONNFAILED);
            return;
        }
    }

    /* now, wait until the user joined */
    FD_ZERO (&fd);
    FD_SET (node[0].socketno, &fd);
    while (1) {
        if (select (node[0].socketno + 1, NULL, &fd, NULL, &tv) == 1) break;

        idler();
        controls->poll();
    }

    /* we have a connection! send the 'hello' packet */
    strcpy (jp.name,my_name);
    sendtonode(0,MPLAYER_CMD_JOIN,(char*)&jp,sizeof (jp));

    /* refresh the text (to: joining game at x.x.x.x, waiting for reply) */
    sprintf(tempstr,MAD_MPLAYER_JOIN_REP_TEXT,inet_ntoa (sin.sin_addr));
    dlg->control[text]->move((width-fontman->getextwidth(fontman->getsysfontno(),tempstr))/2,fontman->getfontheight(fontman->getsysfontno())/2);
    dlg->control[text]->resize(0,width);
    dlg->control[text]->setcaption(tempstr);

    /* redraw the dialog */
    dlgman->drawdialog(dialogno,GFX_DRIVER_HRES,GFX_DRIVER_VRES,displaydata);

    /* wait until we have joined */
    while (node[0].status != MPLAYER_STAT_JOINED) {
        idler();
        controls->poll();
        poll();
    }

    gfx->console_print("Game joined");
    gfx->redraw(0);

    /* set the 'game joined' flag */
    in_game=1;

    /* free the temp. memory */
    free(displaydata);

    /* destroy the dialog */
    dlgman->destroydialog(dialogno);
}

/*
 * MPLAYER::leavegame()
 *
 * This will leave the currently joined game.
 *
 */
void
MPLAYER::leavegame() {
    /* are we inside a game? */
    if (in_game) {
        /* yup. leave it */
        sendtoall(MPLAYER_CMD_LEAVE,NULL,0);

        /* we're no longer in a game now */
        in_game=0;
    }
}

/*
 * MPLAYER::joingamedlg()
 *
 * This will show the 'join game' dialog.
 *
 */
void
MPLAYER::joingamedlg() {
    _UINT dialogno,height,width,ok,cancel,i,addr,text;
    DIALOG* dlg;
    char    tempstr[MAD_TEMPSTR_SIZE];

    /* set height and width */
    height=4*fontman->getfontheight(fontman->getsysfontno()); width=200;

    /* create a dialog */
    dialogno=dlgman->createdialog(DLGMAN_DIALOG_FLAG_TITLEBAR|DLGMAN_DIALOG_FLAG_MOVEABLE,0,0,height,width);
    dlg=dlgman->dialog[dialogno];

    /* set the title */
    dlg->setitle(MAD_MPLAYER_JOININGAME_TITLE);

    /* center it */
    dlg->centerdialog();

    /* create the OK button */
    ok=dlg->createcontrol(DLGMAN_CONTROL_TYPE_BUTTON);

    /* set it up */
    dlg->control[ok]->move(fontman->getfontwidth(fontman->getsysfontno()),2*fontman->getfontheight(fontman->getsysfontno())+4);
    dlg->control[ok]->resize(fontman->getfontheight(fontman->getsysfontno()),fontman->getextwidth(fontman->getsysfontno(),MAD_MPLAYER_OK));
    dlg->control[ok]->setcaption(MAD_MPLAYER_OK);

    /* create the ok button */
    cancel=dlg->createcontrol(DLGMAN_CONTROL_TYPE_BUTTON);

    /* set it up */
    dlg->control[cancel]->move(width-(4*DLGMAN_BORDER_WIDTH)-fontman->getextwidth(fontman->getsysfontno(),MAD_MPLAYER_CANCEL),2*fontman->getfontheight(fontman->getsysfontno())+4);
    dlg->control[cancel]->resize(fontman->getfontheight(fontman->getsysfontno()),fontman->getextwidth(fontman->getsysfontno(),MAD_MPLAYER_CANCEL));
    dlg->control[cancel]->setcaption(MAD_MPLAYER_CANCEL);

    /* create the 'address' label */
    addr=dlg->createcontrol(DLGMAN_CONTROL_TYPE_LABEL);

    /* set it up */
    dlg->control[addr]->move(fontman->getfontwidth(fontman->getsysfontno()),4);
    dlg->control[addr]->resize(fontman->getfontheight(fontman->getsysfontno()),fontman->getextwidth(fontman->getsysfontno(),MAD_MPLAYER_ADDRESS));
    dlg->control[addr]->setcaption(MAD_MPLAYER_ADDRESS);

    /* create the 'room no' input field */
    text=dlg->createcontrol(DLGMAN_CONTROL_TYPE_TEXTINPUT);

    /* set it up */
    dlg->control[text]->move(width-(4*DLGMAN_BORDER_WIDTH)-fontman->getextwidth(fontman->getsysfontno(),MAD_MPLAYER_ADDR),4);
    dlg->control[text]->resize(fontman->getfontheight(fontman->getsysfontno()),fontman->getextwidth(fontman->getsysfontno(),MAD_MPLAYER_ADDR));
    dlg->control[text]->setcaption("");
    dlg->control[text]->setfocus(1);

    /* handle the dialog */
    i=dlgman->dodialog(dialogno);

    /* get the address */
    strcpy(tempstr,dlg->control[text]->getcaption());

    /* destroy the dialog */
    dlgman->destroydialog(dialogno);

    /* did the user click the 'cancel' button? */
    if(i==cancel) {
        /* yup. just leave */
        return;
    }

    /* join the game! */
    joingame(tempstr,NULL);
}

/*
 * MPLAYER::process_packet (_UCHAR no, char* buf, _UCHAR len)
 *
 * This will process [len] bytes of packet [buf]. The packet should come from
 * node [no].
 *
 */
void
MPLAYER::process_packet (_UCHAR no, char* buf,_UCHAR len) {
    struct PACKET* p;
    char*  data;
    char   tempstr[MAD_TEMPSTR_SIZE];

    /* create some pointers */
    p=(struct PACKET*)buf;
    data=(char*)(buf+sizeof(struct PACKET));

    /* create some neato pointers */
    struct JOIN_PACKET* pjp = (struct JOIN_PACKET*)data;
    struct JOIN_PACKET_RESPONSE* pjpr=(struct JOIN_PACKET_RESPONSE*)data;
    struct MOD_PACKET* mp=(struct MOD_PACKET*)data;
    struct JOIN_PACKET jp;

    /* yup. is it a 'I WANNA JOIN!' packet? */
    switch (p->command) {
        case MPLAYER_CMD_JOIN: /* I WANNA JOIN(TM) packet */
                               printf("> '%s' WANTS TO JOIN<\n",pjp->name);
                               strcpy(node[no].name,pjp->name);

                               /* add the console */
                               sprintf(tempstr,"Player %s joined the game",pjp->name);
                               gfx->console_print(tempstr);

                               /* he has joined now */
                               node[no].time_joined=time(NULL);
                               node[no].obj_no=(OBJMAN_MAXOBJECTS-1)-no;

                               objman->setanmobject(node[no].obj_no,objman->getdatafile(ego_object),node[no].name);
                               objman->setnonotify(node[no].obj_no,1);

                               /* send a packet */
                               struct JOIN_PACKET_RESPONSE jpr;

                               jpr.result = MPLAYER_JOIN_RES_OK;
                               jpr.nof_nodes = 0;
                               strcpy(jpr.name,my_name);

                               sendtonode (no, MPLAYER_CMD_JOIN_RESP, (char*)&jpr, sizeof (jpr));

                               /* we're in a game! */
                               in_game = 1;

                               /* send our ego info */
                               send_ego_info(no);
                               break;
   case MPLAYER_CMD_JOIN_RESP: /* I WANNA JOIN(TM) response */
                               /* are we allowed? */
                               if(pjpr->result==MPLAYER_JOIN_RES_OK)  {
                                   /* yup! hand out the champaign :) */
                                   node[no].status=MPLAYER_STAT_JOINED;
                                   strcpy(node[no].name,pjpr->name);

                                   /* XXX: we need to join the other box! */
                                   strcpy (jp.name,my_name);

                                   /* JOIN RESPONSE */
                                   printf(">JOIN RESPONSE SENT\n");
                                   sendtonode (no, MPLAYER_CMD_JOIN, (char*)&jp,sizeof(jp));
                               }
                               break;
     case MPLAYER_CMD_MESSAGE: /* message */
                               sprintf (tempstr, "%s: %s", node[no].name, data);
                               gfx->console_print(tempstr);
                               break;
       case MPLAYER_CMD_LEAVE: /* leave the game */
                               sprintf(tempstr,"Player %s left the game",node[no].name);
                               gfx->console_print(tempstr);

                               /* kill him */
                               closesocket (node[no].socketno);

                               /* shoot his ego charachter */
                               objman->unloadobject(node[no].obj_no);

                               /* mark socket as unused */
                               node[no].socketno=INVALID_SOCKET;
                               break;
         case MPLAYER_CMD_MOD: /* modification */
                               switch(mp->what) {
                                   case MPLAYER_WHAT_ROOMNO: /* change room no */
                                                             objman->setroomno(node[no].obj_no,mp->new_value);
                                                             break;
                                   case MPLAYER_WHAT_COORDS: /* coordinates */
                                                             objman->setcoords(node[no].obj_no,(mp->new_value>>16),(mp->new_value&0xffff));
                                                             break;
                                case MPLAYER_WHAT_DESTCOORDS: /* destination coordinates */
                                                             objman->setdestcoords(node[no].obj_no,(mp->new_value>>16),(mp->new_value&0xffff));
                                                             break;
                                    case MPLAYER_WHAT_SPEED: /* speed */
                                                             objman->setspeed(node[no].obj_no,mp->new_value);
                                                             break;
                                  case MPLAYER_WHAT_VISIBLE: /* visibility */
                                                             objman->setvisible(node[no].obj_no,mp->new_value);
                                                             break;
                                  case MPLAYER_WHAT_MOVESEQ: /* move sequence*/
                                                             objman->setmovesequence(node[no].obj_no,mp->new_value&0xff,(mp->new_value>>8));
                                                             break;
                                   case MPLAYER_WHAT_ANIMNO: /* animation number */
                                                             objman->setanimno(node[no].obj_no,mp->new_value);
                                                             break;
                                  case MPLAYER_WHAT_FRAMENO: /* frame number */
                                                             objman->setframeno(node[no].obj_no,mp->new_value);
                                                             break;
                                case MPLAYER_WHAT_ANIMATING: /* animating number */
                                                             objman->setanimating(node[no].obj_no,mp->new_value);
                                                             break;
                                     case MPLAYER_WHAT_CLIP: /* clipping */
                                                             objman->setclipping(node[no].obj_no,mp->new_value);
                                                             break;
                                  case MPLAYER_WHAT_MASKDIR: /* mask direction */
                                                             objman->setmaskdir(node[no].obj_no,mp->new_value);
                                                             break;
                                   case MPLAYER_WHAT_SPEEDS: /* speeds */
                                                             objman->setobjectspeed(node[no].obj_no,mp->new_value2,mp->new_value>>16,mp->new_value&0xffff);
                                                             break;
                                   case MPLAYER_WHAT_MOVING: /* moving */
                                                             objman->setmove(node[no].obj_no,mp->new_value);
                                                             break;
                               }
                               break;
        case MPLAYER_CMD_INFO: /* apply ego info */
                              apply_ego_info (node[no].obj_no,(struct INFO_PACKET*)data);
                              break;
    }
}

/*
 * MPLAYER::talkdlg()
 *
 * This will pop up the talk dialog.
 *
 */
void
MPLAYER::talkdlg() {
    _UINT i,flags,dialog_no,text_no;
    _UCHAR mulfact;
    char tempstr[MAD_TEMPSTR_SIZE];

    i=fontman->getfontheight(fontman->getsysfontno());

    /* create a dialog */
    flags=DLGMAN_DIALOG_FLAG_GRAYBACK|DLGMAN_DIALOG_FLAG_TITLEBAR|DLGMAN_DIALOG_FLAG_MOVEABLE;
    mulfact=3;

    dialog_no=dlgman->createdialog(flags|DLGMAN_DIALOG_FLAG_NODEFAULT,PARSER_SPAREX,GFX_DRIVER_VRES-(i*mulfact),i,GFX_DRIVER_HRES-(2*PARSER_SPAREX));

    /* create the text box */
    text_no=dlgman->dialog[dialog_no]->createcontrol(DLGMAN_CONTROL_TYPE_TEXTINPUT);

    dlgman->dialog[dialog_no]->setitle("Say");

    /* initialize the control */
    dlgman->dialog[dialog_no]->control[text_no]->move(0,0);
    dlgman->dialog[dialog_no]->control[text_no]->resize(i-DLGMAN_BORDER_HEIGHT,dlgman->dialog[dialog_no]->getwidth()-DLGMAN_BORDER_WIDTH-2);
    dlgman->dialog[dialog_no]->control[text_no]->setfocus(1);
    dlgman->dialog[dialog_no]->control[text_no]->setcaption("");

    /* handle the dialog */
    if(dlgman->dodialog(dialog_no)!=DLGMAN_DIALOG_NOHIT) {
        /* send it! */
        sendtoall(MPLAYER_CMD_MESSAGE,dlgman->dialog[dialog_no]->control[text_no]->getcaption(),strlen(dlgman->dialog[dialog_no]->control[text_no]->getcaption())+1);

        /* do it ourselves too */
        sprintf (tempstr, "%s: %s", my_name, dlgman->dialog[dialog_no]->control[text_no]->getcaption());
        gfx->console_print(tempstr);
    }

    /* destroy it */
    dlgman->destroydialog(dialog_no);

}

/*
 * MPLAYER::send_change_ext(_UINT what,_ULONG old_val,_ULONG old_val_2,_ULONG new_val,_ULONG new_val_2)
 *
 * This will send a change to everyone who needs to know it. The type is [what],
 * the old value is [old_val] and the new one is [new_val]
 *
 */
void
MPLAYER::send_change_ext(_UINT what,_ULONG old_val,_ULONG old_val_2,_ULONG new_val,_ULONG new_val_2) {
    _UINT i;
    struct MOD_PACKET mp;

    /* build the struct */
    mp.what=what;
    mp.old_value=old_val;
    mp.new_value=new_val;
    mp.old_value2=old_val_2;
    mp.new_value2=new_val_2;

    /* is it a teleport? */
    if (what==MPLAYER_WHAT_ROOMNO) {
        /* yup. tell it to everyone */
        sendtoall(MPLAYER_CMD_MOD,(char*)&mp,sizeof(mp));

        return;
    }

    /* only send it to the guys with the same room number as ourselves */
    for(i=0;i<MPLAYER_MAX_NODES;i++) {
        /* node in use? */
        if(node[i].socketno!=INVALID_SOCKET) {
            /* yup. same room no? */
//            if (objman->getroomno(node[i].obj_no)==objman->getroomno(ego_object)) {
                /* yup. send it! */
                sendtonode(i,MPLAYER_CMD_MOD,(char*)&mp,sizeof(mp));
  //          }
        }
    }
}

/*
 * MPLAYER::send_change(_UINT what,_ULONG old_val,_ULONG new_val)
 *
 * This will send a change to everyone who needs to know it. The type is [what],
 * the old value is [old_val] and the new one is [new_val].
 *
 */
void
MPLAYER::send_change(_UINT what,_ULONG old_val,_ULONG new_val) {
    send_change_ext(what,old_val,0,new_val,0);
}

/*
 * MPLAYER::send_ego_info(_UINT node)
 *
 * This will send everything the other side needs to know about the ego to node
 * [node].
 *
 */
void
MPLAYER::send_ego_info(_UINT node) {
    struct INFO_PACKET ip;

    /* build the packet */
    ip.x=objman->getobjectx(ego_object); ip.y=objman->getobjecty(ego_object);
    ip.frameno=objman->getframeno(ego_object); ip.move=objman->get_property(ego_object,OBJMAN_PROP_MOVING);
    ip.animating=objman->isanimating(ego_object); ip.roomno=objman->getroomno(ego_object);
    ip.visible=objman->isvisible(ego_object); ip.destx=objman->getobjectdestx(ego_object);
    ip.desty=objman->getobjectdesty(ego_object); ip.speed=objman->getspeed(ego_object);
    ip.h_speed=objman->getobjecthspeed(ego_object);ip.v_speed=objman->getobjectvspeed(ego_object);
    ip.mask_dir=objman->getmaskdir(ego_object);ip.clip=objman->isclipping(ego_object);

    /* send it */
    sendtonode(node,MPLAYER_CMD_INFO,(char*)&ip,sizeof(ip));
}

/*
 * MPLAYER::apply_ego_info(_UINT no,struct INFO_PACKET* ip)
 *
 * This will apply ego information to the ego from node [no].
 *
 */
void
MPLAYER::apply_ego_info(_UINT no,struct INFO_PACKET* ip) {
    _UINT id=node[no].obj_no;

    objman->setobjectspeed(id,ip->speed,ip->h_speed,ip->v_speed);
    objman->setcoords(id,ip->x,ip->y);
    objman->setframeno(id,ip->frameno);
    objman->setmove(id,ip->move);
    objman->setanimating(id,ip->animating);
    objman->setroomno(id,ip->roomno);
    objman->setvisible(id,ip->visible);
    objman->setdestcoords(id,ip->destx,ip->desty);
    objman->setmaskdir(id,ip->mask_dir);
    objman->setclipping(id,ip->clip);
}
#endif
