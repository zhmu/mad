/*
                              SCRIPT.CPP

                MAD Script Engine - Handles the scripts

                   (c) 1998, 1999, 2000 The MAD Crew

  Description:
  This will handle all MAD scripts, and the VM.

  Todo: Nothing

  Portability notes: Fully portable, no changes required
 */
#ifdef WINDOWS
#include <windows.h>
#endif
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "archive.h"
#include "bar.h"
#include "controls.h"
#include "debugmad.h"
#include "dlgman.h"
#include "fontman.h"
#include "gadgets.h"
#include "inv.h"
#include "maderror.h"
#include "objman.h"
#include "opcodes.h"
#include "parser.h"
#include "game.h"
#include "gfx.h"
#include "mad.h"
#include "script.h"
#include "textman.h"
#include "types.h"

#define  MAD_ERROR_100 "Error 100: script instruction pointer out of range at %s/$%x"
#define  MAD_ERROR_101 "Error 101: file '%s' is not a mad script file"
#define  MAD_ERROR_102 "Error 102: file '%s' is of a wrong version"
#define  MAD_ERROR_103 "Error 103: stack overflow"
#define  MAD_ERROR_104 "Error 104: stack underflow"
#define  MAD_ERROR_105 "Error 105: division by zero"
#define  MAD_ERROR_106 "Error 106: invalid opcode $%x at %s/$%x"
#define  MAD_ERROR_107 "Error 107: cannot find animation sequence '%s' for object $%x"
#define  MAD_ERROR_108 "Error 108: kernel function $%x is unknown"
#define  MAD_ERROR_109 "Error 109: object function $%x is unknown"
#define  MAD_ERROR_110 "Error 110: method offset %s/$%x/$%x ($%x) is out of range"

/*
 * SCRIPT::SCRIPT()
 *
 * This is the constructor of the SCRIPT class. It will reset most general
 * variables.
 *
 */
SCRIPT::SCRIPT() {
    /* clear all variables */
    hdr=NULL; code=NULL; data=NULL; buffer=NULL; object=NULL; filename[0]=0; dialogno = DLGMAN_NODIALOG;
    dialog_objid=OBJMAN_NOBJECT;dialog_resulthandler=SCRIPT_NOADDRESS;
    dialog_result=0;

    /* reset the VM */
    stack=NULL; a=0; ip=0; sp=SCRIPT_STACK_SIZE; flag=0;
}

/*
 * SCRIPT::init(char* fname,_UCHAR rootscript)
 *
 * This will load script [fname]. If [rootscript] is non-zero, it will attempt
 * to load the script as a root script. It will die on any error.
 *
 */
void
SCRIPT::init(char *fname,_UCHAR rootscript) {
    char tempstr[MAD_TEMPSTR_SIZE];
    _ULONG script_size;
    _ULONG i,j;
    char*  ptr;
    struct SCRIPT_OBJECT* obj;

    /* clear all variables */
    hdr=NULL; code=NULL; data=NULL; buffer=NULL; object=NULL; filename[0]=0; dialogno = DLGMAN_NODIALOG;
    dialog_objid=OBJMAN_NOBJECT;dialog_resulthandler=SCRIPT_NOADDRESS;
    dialog_result=0;

    /* reset the VM */
    stack=NULL; a=0; ip=0; sp=SCRIPT_STACK_SIZE; flag=0;

    /* load the script */
    buffer=(char *)archive_readfile(fname,NULL);
    if(buffer==NULL) {
        /* this failed, so die (error 1: unable to load) */
        sprintf(tempstr,MAD_ERROR_1,fname);
        die(tempstr);
    }

    /* create a pointer to the script header */
    hdr=(struct SCRIPTHEADER*)buffer;

    /* check the magic number */
    if(hdr->magicno!=SCRIPT_MAGICNO) {
        /* its not. die (error 116: file is not a mad script file) */
        sprintf(tempstr,MAD_ERROR_101,fname);
        die(tempstr);
    }
    /* check the version */
    if(hdr->version!=SCRIPT_VERSIONO) {
        /* its not. die (error 117: file is of a wrong version) */
        sprintf(tempstr,MAD_ERROR_102,fname);
        die(tempstr);
    }

    /* create a pointer to the script code and data */
    code=(char*)(buffer+sizeof(SCRIPTHEADER));
    data=(char*)(buffer+sizeof(SCRIPTHEADER)+hdr->code_size);

    /* allocate memory for the script objects */
    if((object=(SCRIPT_OBJECT*)malloc(hdr->nofobjects*sizeof(SCRIPT_OBJECT)))==NULL) {
        /* this failed. die (error 2: out of memory */
        sprintf(tempstr,MAD_ERROR_2,hdr->nofobjects*sizeof(SCRIPT_OBJECT));
        die(tempstr);
    }

    /* handle all objects */
    ptr=(char*)(buffer+sizeof(SCRIPTHEADER)+hdr->code_size+hdr->data_size);
    for(i=0;i<hdr->nofobjects;i++) {
        /* make a pointer to the internal object structure */
        obj=(struct SCRIPT_OBJECT*)&(object[i]);

        /* set the pointer */
        obj->hdr=(OBJECTHEADER*)ptr;
        ptr+=sizeof(OBJECTHEADER);

        /* do it too for the methods */
        obj->method=(_ULONG*)ptr;

/*        fprintf(stderr,"OBJ $%x: ",i);
        for(j=0;j<obj->hdr->nofmethods;j++) {
            fprintf(stderr,"[$%x=$%x] ",j,obj->method[j]);
        }
        fprintf(stderr,"\n");*/

        /* next entry! */
        ptr+=sizeof(_ULONG)*obj->hdr->nofmethods;
    }

    /* allocate a stack */
    if((stack=(char*)malloc(SCRIPT_STACK_SIZE))==NULL) {
        /* this failed. die (error 2: out of memory) */
        sprintf(tempstr,MAD_ERROR_2,SCRIPT_STACK_SIZE);
        die(tempstr);
    }

    /* store filename */
    strcpy(filename,fname);
}

/*
 * SCRIPT::code_getbyte()
 *
 * This will return the next script code byte.
 *
 */
_UCHAR
SCRIPT::code_getbyte() {
    _UCHAR i;
    char tempstr[MAD_TEMPSTR_SIZE];

    /* get the byte */
    i=code[ip];

    /* increment the ip */
    ip++;

    /* are we beyond bounds? */
    if(ip>hdr->code_size) {
        /* yup. die (error 100: script instruction pointer out of range at) */
        sprintf(tempstr,MAD_ERROR_100,filename,ip);
        die(tempstr);
    }

    return i;
}

/*
 * SCRIPT::code_getword()
 *
 * This will return the next script code word.
 *
 */
_UINT
SCRIPT::code_getword() {
    _UINT i,j;
    i=code_getbyte(); j=code_getbyte();

    return i+(j<<8);
}

/*
 * SCRIPT::code_getlong()
 *
 * This will return the next script code long.
 *
 */
_ULONG
SCRIPT::code_getlong() {
    return code_getword()+(code_getword()<<16);
}

/*
 * SCRIPT::data_getstring(_ULONG offset)
 *
 * This will return a pointer to the string at data offset [offset].
 *
 */
char*
SCRIPT::data_getstring(_ULONG offset) {
    return (char*)(data+offset);
}

/*
 * SCRIPT::push(_ULONG i)
 *
 * This will store value [i] on the script stack.
 *
 */
void
SCRIPT::push(_ULONG i) {
    _ULONG* ptr;

    /* is all stack gone? */
    if(sp<sizeof(_ULONG)) {
        /* yup. die (error 103: stack overflow) */
        die(MAD_ERROR_103);
    }

    /* decrement stack pointer */
    sp-=sizeof(_ULONG);

    /* push it */
    ptr=(_ULONG*)(stack+sp);
    *ptr=i;
}

/*
 * SCRIPT::pop()
 *
 * This will retrieve a value from the script stack.
 *
 */
_ULONG
SCRIPT::pop() {
    _ULONG i;
    _ULONG* ptr;

    /* pop it */
    ptr=(_ULONG*)(stack+sp);
    i=*ptr;

    /* increment stack pointer */
    sp+=sizeof(_ULONG);

    /* is all stack gone? */
    if(sp>SCRIPT_STACK_SIZE) {
        /* yup. die (error 104: stack underflow) */
        die(MAD_ERROR_104);
    }

    return i;
}

/*
 * SCRIPT::get_real_objid(_UINT objno)
 *
 * This will return the real object id of object [objno].
 *
 */
_UINT
SCRIPT::get_real_objid(_UINT objno) {
    /* if the object is invalid, return the same thing */
    if (objno==OBJMAN_NOBJECT) return objno;

    return object[objno].hdr->obj_mapcode;
}

/*
 * SCRIPT::get_fake_objid(_UINT objno)
 *
 * This will return the fake object id of object [objno].
 *
 */
_UINT
SCRIPT::get_fake_objid(_UINT objno) {
    _UINT i;

    for(i=0;i<hdr->nofobjects;i++) {
        if(object[i].hdr->obj_mapcode==objno) {
            return i;
        }
    }

    /* default to zero */
    return 0;
}

/*
 * SCRIPT::get_property(_UINT objno,_UINT propno)
 *
 * This will retrieve the value of object number [objno], property number
 * [propno].
 *
 */
_ULONG
SCRIPT::get_property(_UINT objno,_UINT propno) {
    return(objman->get_property(get_real_objid(objno),propno));
}

/*
 * SCRIPT::set_property(_UINT objno,_UINT propno,_ULONG value)
 *
 * This will set the value of object number [objno], property number [propno]
 * to [value].
 *
 */
void
SCRIPT::set_property(_UINT objno,_UINT propno,_ULONG value) {
    objman->set_property (get_real_objid (objno),propno,value);
}                                             

/*
 * SCRIPT::call_kernel(_UCHAR func)
 *
 * This will call kernel function [func].
 *
 */
void
SCRIPT::call_kernel(_UCHAR func) {
    #ifdef DOS
    struct timeval timeval;
    #else
        #ifdef WINDOWS
            ULONG timeval;
        #endif
    #endif
    char tempstr[MAD_TEMPSTR_SIZE];
    _ULONG i, j, a, b, c, d, e;

    switch(func) {
        case SCRIPT_KFUNC_LEAVE: /* leave this script */
                                 left=1;
                                 break;
       case SCRIPT_KFUNC_INVOKE: /* invoke a script */
                                 script->done();
                                 script->init(data_getstring(pop()),0);
                                 script->go();
                                 break;
         case SCRIPT_KFUNC_EXIT: /* exit */
                                 mad_exit();
                                 /* NOTREACHED */
                                 break;
   case SCRIPT_KFUNC_LOADSCREEN: /* load a screen */
                                 i=pop();
                                 gfx->loadscreen(data_getstring(i));
                                 break;
      case SCRIPT_KFUNC_REFRESH: /* refresh the display */
                                 gfx->redraw(1);
                                 break;
        case SCRIPT_KFUNC_DELAY: /* delay */
                                 i=pop();
                                 #ifdef DOS
                                 gettimeofday(&timeval,NULL);
                                 i+=timeval.tv_usec;
                                 #else
                                     #ifdef WINDOWS
                                         i+=GetTickCount();
                                     #endif
                                 #endif
                                 #ifdef DOS
                                 while (timeval.tv_usec<i) {
                                     gettimeofday(&timeval,NULL);
                                     gfx->redraw(0);
                                     controls->poll();
                                     idler();
                                 }
                                 #elif WINDOWS
                                     while (GetTickCount()<i) {
                                         gfx->redraw(0);
                                         controls->poll();
                                         idler();
                                     }
                                 #endif
                                 break;
#ifdef DEBUG_AVAILABLE
        case SCRIPT_KFUNC_BREAK: /* debug break */
                                 if(this==rootscript) {
                                     debug_step=1;
                                 } else {
                                     debug_step=2;
                                 }
                                 break;
#endif
      case SCRIPT_KFUNC_LOADPAL: /* load palette */
                                 gfx->loadpalette(data_getstring(pop()));
                                 break;
      case SCRIPT_KFUNC_ADDICON: /* add bar icon */
                                 e=pop(); /* procedure */
                                 d=pop(); /* object id */
                                 c=pop(); /* flags */
                                 b=pop(); /* y coordinate */
                                 a=pop(); /* x coordinate */
                                 j=pop(); /* &spritefile */
                                 i=pop(); /* &name */
                                 bar->addicon(data_getstring(i),data_getstring(j),a,b,c,get_real_objid(d),e);
                                 break;
      case SCRIPT_KFUNC_SHOWBAR: /* show icon bar */
                                 bar->setenabled(1);
                                 break;
      case SCRIPT_KFUNC_HIDEBAR: /* hide icon bar */
                                 bar->setenabled(0);
                                 break;
      case SCRIPT_KFUNC_BINDBAR: /* bind a bar icon to a default handler */
                                 c=pop(); b=pop(); a=pop();
                                 bar->bindefault(a,b,c);
                                 break;
       case SCRIPT_KFUNC_RETURN: /* return */
                                 ip=pop();
                                 if(ip==SCRIPT_NOADDRESS) {
                                     running=0;
                                 }
                                 break;
       case SCRIPT_KFUNC_RANDOM: /* return a random number */
                                 i=(rand()%pop());
                                 push(i);
                                 break;
    case SCRIPT_KFUNC_DLGCHOICE: /* dialog choice */
                                 push (dialog_result);
                                 break;
      case SCRIPT_KFUNC_TOUCHED: /* touched by mouse? (XXX) */
                                 break;
     case SCRIPT_KFUNC_SHOWTEXT: /* show text */
                                 b = pop(); a = pop(); j = pop(); i = pop();

                                 /* get the text from the datafile */
                                 textman->getstring (a, tempstr, MAD_TEMPSTR_SIZE);

                                 /* get the text height and width (XXX) */
                                 c = fontman->getextheight (0, tempstr) + 2;
                                 d = fontman->getextwidth (0, tempstr) + 2;

                                 dialogno = dlgman->createdialog (0, i, j, c, d);
                                 dlgman->dialog[dialogno]->createcontrol (DLGMAN_CONTROL_TYPE_LABEL);
                                 dlgman->dialog[dialogno]->resize (c, d);
                                 dlgman->dialog[dialogno]->control[0]->setcaption (tempstr);

                                 /* do the loop until the delay runs out */
                                 while (b) {
                                     rootscript->go_offset(rootscript->hdr->run_offset);
                                     go_offset(hdr->run_offset);
                                     gfx->redraw(1);

//                                     if (controls->button1pressed() || controls->button2pressed()) break;
                                     if (controls->button1pressed()) break;
                                     if (controls->getch()) break;

                                     /* poll the controls! */
                                     idler ();
                                     controls->poll();

                                     /* decrement timeout */
                                     b--;
                                 }

                                 /* kill the dialog */
                                 dlgman->destroydialog (dialogno);
                                 dialogno = DLGMAN_NODIALOG;
                                 break;
     case SCRIPT_KFUNC_CONVERSE: /* converse */
                                 c = pop(); b = pop(); a = pop(); j = pop(); i = pop();
                                 dialog_result = do_conversation (i, j, a, b, c);

                                 /* do we have a valid dialog result handler object id? */
                                 if (dialog_objid != OBJMAN_NOBJECT) {
                                     /* yup. call it */
                                     go_method (get_fake_objid (dialog_objid), dialog_resulthandler);
                                 }
                                 break;
  case SCRIPT_KFUNC_EMPTYCHOICE: /* empty dialog choice */
                                 dialog_result = 0xffff;
                                 break;
        case SCRIPT_KFUNC_ALERT: /* alert the user */
                                 dlgman->alert(pop());
                                 break;
   case SCRIPT_KFUNC_INV_ENABLE: /* enable inventory dialog */
                                 inv->setstatus(1);
                                 break;
  case SCRIPT_KFUNC_INV_DISABLE: /* disable inventory dialog */
                                 inv->setstatus(0);
                                 break;
     case SCRIPT_KFUNC_INV_GIVE: /* give inventory item */
                                 a=pop(); /* quantity */
                                 b=pop(); /* &name */
                                 inv->give_item(data_getstring(b),a);
                                 break;
     case SCRIPT_KFUNC_INV_DROP: /* drop inventory item */
                                 b=pop(); /* &name */
                                 inv->drop_item(data_getstring(b));
                                 break;
     case SCRIPT_KFUNC_INV_SETQ: /* set inventory item quantity */
                                 a=pop(); /* quantity */
                                 b=pop(); /* &name */
                                 inv->change_quantity(data_getstring(b),a);
                                 break;
     case SCRIPT_KFUNC_INV_OPEN: /* open inventory window */
                                 inv->show_inventory();
                                 break;
     case SCRIPT_KFUNC_INV_GETQ: /* get inventory item quantity */
                                 a=pop(); /* &name */
                                 i=inv->get_quantity(data_getstring(a));
                                 push(i);
                                 break;
   case SCRIPT_KFUNC_LOADCURSOR: /* load a cursor */
                                 a=pop();  /* &fname */
                                 controls->loadcursor(data_getstring(a));
                                 break;
                        default: /* die (error 108: kernel function is unknown) */
                                 sprintf(tempstr,MAD_ERROR_108,func);
                                 die(tempstr);
                                 /* NOTREACHED */
   }
}

/*
 * SCRIPT::call_object(_UINT objno,_UCHAR func)
 *
 * This will call object function [func].
 *
 */
void
SCRIPT::call_object(_UINT objno,_UCHAR func) {
    _UINT objid;
    _ULONG i,j,a;
    char*  ptr;
    char tempstr[MAD_TEMPSTR_SIZE];
    char   objman_text[OBJMAN_MAX_TEXT_LEN];

    /* get the real object id */
    objid=get_real_objid(objno);

    switch(func) {
        case SCRIPT_OFUNC_SETANIM: /* set animation sequence */
                                   ptr=data_getstring(pop());
                                   i=gfx->findanmsequence(objman->getsourceno(objid),ptr);
                                   if (i==GFX_NOBJECT) {
                                       /* we couldnt find the animation, so die with this error (error 107: cannot found animation sequence for object ) */
                                       sprintf(tempstr,MAD_ERROR_107,ptr,objid);
                                       die(tempstr);
                                   }
                                   objman->setanimno(objid,i);
                                   objman->setframeno(objid,0);
                                   break;
       case SCRIPT_OFUNC_SETFRAME: /* set frame number */
                                   objman->setframeno(objid,pop());
                                   break;
        case SCRIPT_OFUNC_SETMOVE: /* set move sequence */
                                   ptr=data_getstring(pop());
                                   gfx->setmovesequence(objid,ptr);
                                   break;
       case SCRIPT_OFUNC_SETSPEED: /* set speed */
                                   i=pop();j=pop();a=pop();
                                   objman->setobjectspeed(objid,i,j,a);
                                   break;
        case SCRIPT_OFUNC_SETHALT: /* set halt sequence */
                                   ptr=data_getstring(pop());
                                   gfx->sethaltsequence(objid,ptr);
                                   break;
           case SCRIPT_OFUNC_FACE: /* face */
                                   objman->setdirection(objid,pop());
                                   break;
        case SCRIPT_OFUNC_SETLOOP: /* set loop value */
                                   objman->setlooping(objid,pop());
                                   break;
        case SCRIPT_OFUNC_SETMASK: /* set mask value */
                                   objman->setmaskdir(objid,pop());
                                   break;
        case SCRIPT_OFUNC_SETCLIP: /* set clip value */
                                   objman->setclipping(objid,pop());
                                   break;
           case SCRIPT_OFUNC_STOP: /* stop animating */
                                   objman->setmove(objid,0);
                                   break;
        case SCRIPT_OFUNC_ANIMATE: /* start animating */
                                   objman->setframeno(objid,0);
                                   objman->setanimating(objid,1);
                                   break;
           case SCRIPT_OFUNC_MOVE: /* move */
                                   i=pop();j=pop();
                                   objman->setcoords(objid,j,i);
                                   break;
         case SCRIPT_OFUNC_MOVETO: /* move to */
                                   i=pop();j=pop();
                                   objman->setdestcoords(objid,j,i);
                                   objman->setmove(objid,1);
                                   break;
           case SCRIPT_OFUNC_SHOW: /* show */
                                   objman->setvisible(objid,1);
                                   break;
           case SCRIPT_OFUNC_HIDE: /* hide */
                                   objman->setvisible(objid,0);
                                   break;
         case SCRIPT_OFUNC_UNLOAD: /* unload */
                                   objman->unloadobject(objid);
                                   break;
         case SCRIPT_OFUNC_ENABLE: /* enable */
                                   objman->setenabled(objid,1);
                                   break;
        case SCRIPT_OFUNC_DISABLE: /* disable */
                                   objman->setenabled(objid,0);
                                   break;
        case SCRIPT_OFUNC_SETPRIO: /* set priority */
                                   objman->setbasepriority(objid,pop());
                                   break;
       case SCRIPT_OFUNC_TELEPORT: /* teleport */
                                   objman->setroomno(objid,pop());
                                   break;
        case SCRIPT_OFUNC_LOADANM: /* load animation */
                                   ptr=data_getstring(pop());
                                   objman->setanmobject(objid,ptr,"");
                                   break;
     case SCRIPT_OFUNC_SETMOVEOBJ: /* set move object */
                                   ego_object=objid;
                                   break;
     case SCRIPT_OFUNC_BINDACTION: /* bind an action to an object */
                                   i=pop();j=pop();
                                   objman->bindaction(objid,j,i);
                                   break;
       case SCRIPT_OFUNC_LOADTEXT: /* load text */
                                   textman->getstring(pop(),objman_text,OBJMAN_MAX_TEXT_LEN);
                                   objman->setext(objid,objman_text);
                                   break;
       case SCRIPT_OFUNC_SETALIGN: /* set text alignment */
                                   i=pop();
                                   objman->setalign(objid,i);
                                   break;
        case SCRIPT_OFUNC_INITEXT: /* initialize text object */
                                   objman->setextobject(objid,"",data_getstring(pop()));
                                   break;
       case SCRIPT_OFUNC_SETCOLOR: /* set text color */
                                   objman->setcolor(objid,pop());
                                   break;
    case SCRIPT_OFUNC_DLG_BINDRES: /* bind dialog manager result */
                                   dialog_objid = objid;
                                   dialog_resulthandler=pop();
                                   break;                                   
                          default: /* die (error 109: object function is unknown) */
                                   sprintf(tempstr,MAD_ERROR_109,func);
                                   die(tempstr);
    }
}

/*
 * SCRIPT::run_instruction()
 *
 * This will return the instruction at [ip].
 *
 */
void
SCRIPT::run_instruction() {
    char tempstr[MAD_TEMPSTR_SIZE];
    _ULONG i,j;


    #ifdef DEBUG_AVAILABLE
    if(debug_step) {
        /* don't trace the root script */
        if((debug_step==2)&&(this!=rootscript)) debug_launchdebugger(this);
        if(debug_step==1) debug_launchdebugger(this);
    }
    #endif

    i=code_getbyte();
    switch (i) {
         case OPCODE_NOP: /* nop */
                          break;
        case OPCODE_PUSH: /* push immediate */
                          push(code_getlong());
                          break;
       case OPCODE_PUSHA: /* push accumulator to stack */
                          push(a);
                          break;
        case OPCODE_TOSS: /* toss top stack code away */
                          pop();
                          break;
        case OPCODE_POPA: /* pop accumulator */
                          a=pop();
                          break;
       case OPCODE_PUSHW: /* push 16-bit word */
                          i=(_ULONG)code_getword();
                          push(i);
                          break;
         case OPCODE_RET: /* return */
                          ip=pop();
                          if(ip==SCRIPT_NOADDRESS) {
                              running=0;
                          }
                          break;
         case OPCODE_JMP: /* jump */
                          ip=code_getlong();
                          break;
         case OPCODE_AND: /* logical and */
                          i=pop(); j=(i&pop()); push (j);
                          break;
          case OPCODE_OR: /* logical or */
                          i=pop(); j=(i|pop()); push (j);
                          break;
         case OPCODE_XOR: /* logical xor */
                          i=pop(); j=(i^pop()); push (j);
                          break;
         case OPCODE_NOT: /* logical not */
                          i=~pop(); push (i);
                          break;
         case OPCODE_ADD: /* add */
                          i=pop();j=pop();push (i+j);
                          break;
         case OPCODE_SUB: /* subtract */
                          i=pop();j=pop();push (j-i);
                          break;
         case OPCODE_MUL: /* multiply */
                          i=pop();j=pop();push (i*j);
                          break;
         case OPCODE_DIV: /* divide */
                          i=pop(); j=pop();
                          /* trap division by zero */
                          if(!j) {
                              /* die (Error 105: division by zero) */
                              die(MAD_ERROR_105);
                          }
                          push (i/j);
                          break;
         case OPCODE_INC: /* increment */
                          i=pop();i++;push(i);
                          break;
         case OPCODE_DEC: /* decrement */
                          i=pop();i--;push(i);
                          break;
          case OPCODE_EQ: /* equal? */
                          flag=0;i=pop();
                          if (i==pop()) flag=1;
                          break;
          case OPCODE_GT: /* greater than? */
                          flag=0;j=pop();i=pop();
                          if (i>j) flag=1;
                          break;
          case OPCODE_LT: /* less than? */
                          flag=0;i=pop();
                          if (i<pop()) flag=1;
                          break;
          case OPCODE_GE: /* greater or equal than? */
                          flag=0;i=pop();
                          if (i>=pop()) flag=1;
                          break;
          case OPCODE_LE: /* lesser or equal than? */
                          flag=0;i=pop();
                          if (i<=pop()) flag=1;
                          break;
          case OPCODE_JS: /* jump set */
                          i=code_getlong();
                          if(flag) ip=i;
                          break;
          case OPCODE_JC: /* jump clear */
                          i=code_getlong();
                          if(!flag) ip=i;
                          break;
         case OPCODE_LDA: /* load accumulator */
                          a=code_getlong();
                          break;
         case OPCODE_LAP: /* load property in accumulator */
                          i=code_getword();j=code_getword();
                          push(get_property(i,j));
                          break;
         case OPCODE_LDP: /* load accumulator in property */
                          i=code_getword();j=code_getword();
                          set_property(i,j,pop());
                          break;
         case OPCODE_CLK: /* call kernel function */
                          call_kernel(code_getbyte());
                          break;
         case OPCODE_CLO: /* call kernel function */
                          i=code_getword(); j=code_getbyte();
                          call_object(i,j);
                          break;
                 default: /* unknown opcode. die (error 106: invalid opcode at /) */
                          sprintf(tempstr,MAD_ERROR_106,i,filename,ip);
                          die(tempstr);
                          /* NOTREACHED */
    }
}

#ifdef DEBUG_AVAILABLE
/*
 * SCRIPT::disassemble(_ULONG the_ip,char* dest)
 *
 * This will disassemble the instruction at ip [the_ip] and return it in
 * dest. It will return the number of bytes disassembled.
 *
 */
_ULONG
SCRIPT::disassemble(_ULONG the_ip,char* dest) {
    _ULONG old_ip,old_sp;
    _ULONG i,j;

    /* save old ip and sp and activate new ip */
    old_sp=sp;old_ip=ip;ip=the_ip;

    /* figure out what it is! */
    switch(code_getbyte()) {
         case OPCODE_NOP: /* nop */
                          sprintf(dest,"nop");
                          break;
        case OPCODE_PUSH: /* push immediate */
                          sprintf(dest,"push $%x",code_getlong());
                          break;
       case OPCODE_PUSHA: /* push accumulator to stack */
                          sprintf(dest,"push a");
                          break;
        case OPCODE_TOSS: /* toss top stack code away */
                          sprintf(dest,"toss");
                          break;
        case OPCODE_POPA: /* pop accumulator */
                          sprintf(dest,"pop a");
                          break;
       case OPCODE_PUSHW: /* push 16-bit word */
                          sprintf(dest,"pushw $%x",code_getword());
                          break;
         case OPCODE_RET: /* return */
                          sprintf(dest,"ret");
                          break;
         case OPCODE_JMP: /* jump */
                          sprintf(dest,"jmp $%x",code_getlong());
                          break;
         case OPCODE_AND: /* logical and */
                          sprintf(dest,"and");
                          break;
          case OPCODE_OR: /* logical or */
                          sprintf(dest,"or");
                          break;
         case OPCODE_XOR: /* logical xor */
                          sprintf(dest,"xor");
                          break;
         case OPCODE_NOT: /* logical not */
                          sprintf(dest,"not");
                          break;
         case OPCODE_ADD: /* add */
                          sprintf(dest,"add $%x,%$%x",pop(),pop());
                          break;
         case OPCODE_SUB: /* subtract */
                          sprintf(dest,"sub $%x,$%x",pop(),pop());
                          break;
         case OPCODE_MUL: /* multiply */
                          sprintf(dest,"mul $%x,$%x",pop(),pop());
                          break;
         case OPCODE_DIV: /* divide */
                          sprintf(dest,"div $%x,$%x",pop(),pop());
                          break;
         case OPCODE_INC: /* increment */
                          sprintf(dest,"inc");
                          break;
         case OPCODE_DEC: /* decrement */
                          sprintf(dest,"dec");
                          break;
          case OPCODE_EQ: /* equal? */
                          sprintf(dest,"eq? (%s)",(pop()==pop()) ? "yes" : "no");
                          break;
          case OPCODE_GT: /* greater than? */
                          sprintf(dest,"gt? (%s)",(pop()>pop()) ? "yes" : "no");
                          break;
          case OPCODE_LT: /* less than? */
                          sprintf(dest,"lt? (%s)",(pop()<pop()) ? "yes" : "no");
                          break;
          case OPCODE_GE: /* greater or equal than? */
                          sprintf(dest,"ge? (%s)",(pop()>=pop()) ? "yes" : "no");
                          break;
          case OPCODE_LE: /* lesser or equal than? */
                          sprintf(dest,"le? (%s)",(pop()<=pop()) ? "yes" : "no");
                          break;
          case OPCODE_JS: /* jump set */
                          i=code_getlong();
                          sprintf(dest,"js $%x (%s)",i,(flag) ? "yes" : "no");
                          break;
          case OPCODE_JC: /* jump clear */
                          i=code_getlong();
                          sprintf(dest,"jc $%x (%s)",i,(!flag) ? "yes" : "no");
                          break;
         case OPCODE_LDA: /* load accumulator */
                          sprintf(dest,"lda $%x",code_getlong());
                          break;
         case OPCODE_LAP: /* load property in accumulator */
                          sprintf(dest,"lap $%x,$%x",code_getword(),code_getword());
                          break;
         case OPCODE_LDP: /* load accumulator in property */
                          i=code_getword();j=code_getword();
                          sprintf(dest,"ldp $%02x,$%02x,$%x",i,j,pop());
                          break;
         case OPCODE_CLK: /* call kernel function */
                          sprintf(dest,"clk $%x",code_getbyte());
                          break;
         case OPCODE_CLO: /* call object function */
                          i=code_getword(); j=code_getbyte();
                          sprintf(dest,"clo $%02x,$%02x",i,j);
                          break;
                 default: /* unknown opcode */
                          sprintf(dest,"???");
                          break;
    }

    /* restore the ip */
    i=ip; ip=old_ip; sp=old_sp;

    return (i-the_ip);
}
#endif

/*
 * SCRIPT::go_offset(_ULONG offset)
 *
 * This will run a certain offset.
 *
 */
void
SCRIPT::go_offset(_ULONG offset) {
    _ULONG old_ip;
    _UCHAR old_run;

    /* save the old offset, activate the correct ip, and start running! */
    old_run=running; old_ip=ip; running=1; ip=offset;

    /* push the special address */
    push(SCRIPT_NOADDRESS);

    /* do while running */
    do {
        /* do the opcodes! */
        run_instruction();

        /* poll the rest */
        controls->poll();
        idler();
        parser->handlevent();
    } while (running);

    /* fix the running flag and ip */
    running=old_run; ip=old_ip;
}

/*
 * SCRIPT::go()
 *
 * This will run the script.
 *
 */
void
SCRIPT::go() {
    if (this==rootscript) {
        go_offset(hdr->init_offset);
    } else {
        force_left=0;
        go_offset(hdr->init_offset);
        if (!force_left) {
            left=0;
            while(!left) {
                rootscript->go_offset(rootscript->hdr->run_offset);
                go_offset(hdr->run_offset);
                gfx->redraw(1);
            }
        }
        go_offset(hdr->done_offset);
    }
}

/*
 * SCRIPT::done()
 *
 * This will deinitialize the script manager and VM.
 *
 */
void
SCRIPT::done() {
    /* do we have a dialog assigned to us? */
    if (dialogno != DLGMAN_NODIALOG) {
        /* yup. free it (XXX) */
//        dlgman->destroydialog (dialogno);
        /* make sure we don't do this twice */
//        dialogno = DLGMAN_NODIALOG;
    }
    /* is there data allocated? */
    if(buffer!=NULL) {
        /* yup. free it */
        free(buffer);
        /* make sure we don't do it twice */
        buffer=NULL;
    }

    /* do we have object data allocted? */
    if(object==NULL) {
        /* yup. zap it */
        free(object);
        /* make sure we don't do it twice */
        object=NULL;
    }

    /* do we have stack allocted? */
    if(stack==NULL) {
        /* yup. zap it */
        free(stack);
        /* make sure we don't do it twice */
        stack=NULL;
    }
    /* clear filename */
    filename[0]=0;
}

/*
 * SCRIPT::getfilename()
 *
 * This will return the filename of the currently loaded script or NULL if
 * nothing is loaded.
 *
 */
char*
SCRIPT::getfilename() {
    if (filename[0]) return filename;

    return NULL;
}

/*
 * SCRIPT::setactive(_UCHAR isactive)
 *
 * This will set the active flag to [isactive].
 *
 */
void
SCRIPT::setactive(_UCHAR isactive) {
    active=isactive;
}

/*
 * SCRIPT::isactive()
 *
 * This will return the active flag.
 *
 */
_UCHAR
SCRIPT::isactive() {
    return active;
}

/*
 * SCRIPT::get_ip()
 *
 * This will return the instruction pointer.
 *
 */
_ULONG
SCRIPT::get_ip() {
    return ip;
}

/*
 * SCRIPT::get_a()
 *
 * This will return the accumulator.
 *
 */
_ULONG
SCRIPT::get_a() {
    return a;
}

/*
 * SCRIPT::get_sp()
 *
 * This will return the stack pointer.
 *
 */
_UINT
SCRIPT::get_sp() {
    return sp;
}

/*
 * SCRIPT::get_flag()
 *
 * This will return the flag.
 *
 */
_UCHAR
SCRIPT::get_flag() {
    return flag;
}

/*
 * SCRIPT::get_code_size()
 *
 * This will return the length of the script code.
 *
 */
_ULONG
SCRIPT::get_code_size() {
    return hdr->code_size;
}

#ifdef DEBUG_AVAILABLE
/*
 * SCRIPT::get_tos_value()
 *
 * This will return the top-of-stack value.
 *
 */
_ULONG
SCRIPT::get_tos_value() {
    _ULONG* ptr;

    ptr=(_ULONG*)(stack+sp);
    return *ptr;
}
#endif

/*
 * SCRIPT::go_method(_ULONG objno,methodno)
 *
 * This will run a certain method of a certain object.
 *
 */
void
SCRIPT::go_method(_ULONG objno,_ULONG methodno) {
    char tempstr[MAD_TEMPSTR_SIZE];
    _ULONG offs;
    struct SCRIPT_OBJECT* obj;

    /* get the offset */
    obj=(struct SCRIPT_OBJECT*)&(object[objno]);
    offs=obj->method[methodno];

    /* if we need to inherit it, do it */
    if(offs==0xffffffff) {
        /* FIXME */
        return;
    }

    /* if it's out of range, complain */
    if(offs>hdr->code_size) {
        /* it is. die (error 110: method offset is out of range) */
        sprintf(tempstr,MAD_ERROR_110,filename,objno,methodno,offs);
        die(tempstr);
    }

    /* run it */
    go_offset (offs);
}

/*
 * SCRIPT::getdialogno()
 *
 * This will return the initially used dialog number of the script.
 *
 */
_UINT
SCRIPT::getdialogno() {
    return dialogno;
}

/*
 * SCRIPT::setdialogno(_UINT no)
 *
 * This will set the initially used dialog number of the script to
 * [no].
 *
 */
void
SCRIPT::setdialogno(_UINT no) {
    dialogno = no;
}

#ifdef DEBUG_AVAILABLE
/*
 * SCRIPT::forceleave()
 *
 * This will force the script to leave
 *
 *
 */
void
SCRIPT::forceleave() {
    left=1; running=0; force_left=1;
}
#endif
