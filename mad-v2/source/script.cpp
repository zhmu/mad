/*
                              SCRIPT.CPP

                MAD Script Engine - Handles the scripts

                     (c) 1998, 1999 The MAD Crew

  Description:
  This will handle all MAD scripts. It will convert opcodes as neccesary.

  Todo: Nothing

  Portability notes: Fully portable, no changes required
*/
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "archive.h"
#include "bar.h"
#include "controls.h"
#include "debugmad.h"
#include "dlgman.h"
#include "fontman.h"
#include "maderror.h"
#include "objman.h"
#include "parser.h"
#include "game.h"
#include "gfx.h"
#include "mad.h"
#include "script.h"
#include "textman.h"
#include "types.h"

/*
 * SCRIPT::SCRIPT()
 *
 * This is the constructor of the script class.
 *
 */
SCRIPT::SCRIPT() {
    /* constructor: set scriptcode to NULL since libc will crash if we try to
       free a NULL pointer */
    scriptcode=NULL;
    /* and reset the filename to a single zero char */
    filename[0]='\0';
    dialogno=DLGMAN_NODIALOG;
}


/*
 * SCRIPT::init(char* fname)
 *
 * This will load script [fname]. It will die on any error.
 *
 */
void
SCRIPT::init(char *fname) {
    char tempstr[MAD_TEMPSTR_SIZE];
    char *ptr;
    _ULONG script_size;
    _UINT i;

    scriptcode=NULL;
    /* reset the filename to a single zero char */
    filename[0]='\0';
    /*
     * try to load the script
     *
     */
    ptr=(char *)archive->readfile(fname,&script_size);
    if(ptr==NULL) {
        /* this failed, so die (error 1: unable to load) */
        sprintf(tempstr,MAD_ERROR_1,tempstr);
        die(tempstr);
    }
    /*
     * script file is loaded. now fill in some useful structures
     *
     */
    memcpy(&header,ptr,sizeof(struct SCRIPTHEADER));

    /* check whether the header is correct */
    /* check the magic number */
    if(header.magicno!=SCRIPT_MAGICNO) {
        /* its not. die (error 116: file is not a mad script file) */
        sprintf(tempstr,MAD_ERROR_116,fname);
        die(tempstr);
    }
    /* check the version */
    if(header.version!=SCRIPT_VERSIONO) {
        /* its not. die (error 117: file is of a wrong version) */
        sprintf(tempstr,MAD_ERROR_117,fname);
        die(tempstr);
    }

    for(i=0;i<header.nofprocs;i++) {
        memcpy(&proc[i],(void *)(ptr+header.script_size+sizeof(struct SCRIPTHEADER)+(i*sizeof(struct PROCHEADER))),sizeof(struct PROCHEADER));
    }
    scriptcodesize=header.script_size;
    /*
     * copy the actual script code to [scriptcode]
     *
     */
    if((scriptcode=(char *)malloc(scriptcodesize))==NULL) {
        /* memory allocation failed, so die (error 2: out of memory, bytes) */
        sprintf(tempstr,MAD_ERROR_2,scriptcodesize);
        die(tempstr);
    }
    memcpy(scriptcode,(void *)(ptr+sizeof(struct SCRIPTHEADER)),scriptcodesize);
    free(ptr);

    /* and save the file name */
    strcpy(filename,fname);

    #ifdef DEBUG_AVAILABLE
        /* nuke all breakpoints */
        for(i=0;i<DEBUG_MAX_BREAKPOINTS;i++) {
            breakpoint[i]=SCRIPT_NOBREAKPOINTADDRESS;
        }
    #endif
}

/*
 * SCRIPT::parse();
 *
 * This will parse the script. It will ensure us the procedure table is valid.
 *
 */
void
SCRIPT::parse() {
    char tempstr[MAD_TEMPSTR_SIZE];
    _UINT i;

    /*
     * check whether all procedures are valid
     *
     */
    for(i=0;i<header.nofprocs;i++) {
        if((proc[i].start_pos>scriptcodesize)||(proc[i].end_pos>scriptcodesize)) {
            /* it is not valid, so die (error 101: script procedure hasn't got valid offsets) */
            sprintf(tempstr,MAD_ERROR_101,proc[i].name);
            die(tempstr);
        }
    }
}

/*
 * SCRIPT::findproc(char* name);
 *
 * This will try to find procedure [name] in the script. If it is found, the
 * proc[] number is returned, otherwise SCRIPT_NOIDENT will be returned.
 *
 */
_UINT
SCRIPT::findproc(char *name) {
    _UINT i;

    /*
     * scan the proc[] array structure for [name]
     *
     */
    for(i=0;i<header.nofprocs;i++) {
        if(!strcmp(proc[i].name,name)) {
            /* this is the correct procedure. return the number */
            return i;
        }
    }
    /* no procedure found. return SCRIPT_NOIDENT */
    return SCRIPT_NOIDENT;
}

/*
 * SCRIPT::getbyte()
 *
 * This will return the next byte in the script an advance the script's
 * instruction pointer by 1.
 *
 */
_UCHAR
SCRIPT::getbyte() {
    _UCHAR i;

    i=scriptcode[eip++];

    /* Log this if nessecary */
    #ifdef DEBUG_AVAILABLE
//        LOG("Got byte $%x\n",i);
    #endif

    /* just return the next byte */
    return (i);
}

/*
 * SCRIPT::getbyte()
 *
 * This will return the next word in the script and advance the script's
 * instruction pointer by 2.
 *
 */
_UINT
SCRIPT::getword() {
    _UINT i;

    /* just return the next 2 script code bytes as a word */
    memcpy(&i,(void *)(scriptcode+eip),2);
    eip=eip+2;
    return i;
}

/*
 * SCRIPT::getbyte()
 *
 * This will return the next doubleword in the script and advance the script's
 * instruction pointer by 4.
 *
 */
_ULONG
SCRIPT::getdword() {
    _UINT i,j;

    /* get 2 words */
    i=getword();j=getword();

    return (i+(j<<16));
}

/*
 * SCRIPT::getstring(char* dest,_UCHAR maxlen)
 *
 * This will copy a string from the script to [dest]. If the length of the
 * string exceedes [maxlen], this function will die.
 *
 */
void
SCRIPT::getstring(char *dest,_UCHAR maxlen) {
    char tempstr[MAD_TEMPSTR_SIZE];
    _UINT len;

    /* get a string from the script code */
    /* first get the length */
    len=getword();
    /* verify length */
    if(len>maxlen) {
        /* string is too long. die (error 102: length of string at offset is bytes and maximum is set to */
        sprintf(tempstr,MAD_ERROR_102,eip,len,maxlen);
        die(tempstr);
        /* NOTREACHED */
    }
    /* first clear out [dest] */
    memset(dest,0,maxlen);
    /* now just copy [len] bytes of the script code to [dest] */
    memcpy(dest,(char *)(scriptcode+eip),len);
    /* and increment eip */
    eip+=len;
}

_ULONG
SCRIPT::getval(_UCHAR valtype,_ULONG valno,_UCHAR *error) {
    *error=0;
    switch(valtype) {
        case TYPE_STATIC: /* just a static number */
                          return valno;
      case TYPE_VARIABLE: /* variable */
 case TYPE_OBJECT_CHOICE: /* dialog choice */
                          return objman->getsourceno(valno);
      case TYPE_OBJECT_X: /* object x position */
                          return objman->getobjectx(valno);
      case TYPE_OBJECT_Y: /* object y position */
                          return objman->getobjecty(valno);
case TYPE_OBJECT_FRAMENO: /* object frame number */
                          return objman->getframeno(valno);
 case TYPE_OBJECT_MOVING: /* object is moving flag */
                          return objman->objmustmove(valno);
case TYPE_OBJECT_ANIMATING: /* object is animating flag */
                          return objman->isanimating(valno);
case TYPE_OBJECT_PRIORITY: /* object priority code */
                          return objman->getprioritycode(valno);
case TYPE_SPECIAL_RANDOM: /* random value */
                          return rand() % valno;
case TYPE_OBJECT_FINISHED: /* dialog finished flag (XXX: TODO: implement it) */
                          return 0;
    }
    *error=1;
    return 0xffff;
}

#ifdef DEBUG_AVAILABLE
char
*SCRIPT::resolvecondition(_UCHAR condition) {
    /* change condition number [condition] to a human-readable string */
    switch(condition) {
           case CHECK_EQUAL: return "==";
        case CHECK_NOTEQUAL: return "!=";
       case CHECK_GREATEREQ: return ">=";
       case CHECK_SMALLEREQ: return "<=";
         case CHECK_GREATER: return ">";
         case CHECK_SMALLER: return "<";
    }
    /* we don't understand this. return question marks */
    return "???";
}

void
SCRIPT::resolvevar(_UCHAR valtype,_UINT valno,char* dest) {
    /* this will resolve var [valno], type [valtype] into human-readable text */
    /* first default message */
    sprintf(dest,"(???)val %u no %u",valtype,valno);
    switch(valtype) {
        case TYPE_STATIC: /* just a static number */
                          sprintf(dest,"(static)%u",valno);
                          break;
      case TYPE_VARIABLE: /* variable (doesn't have a known name) */
                          sprintf(dest,"(variable)%u",valno);
                          break;
      case TYPE_OBJECT_X: /* object x position */
                          sprintf(dest,"(object)%s.x",objman->getobjectname(valno));
                          break;
      case TYPE_OBJECT_Y: /* object y position */
                          sprintf(dest,"(object)%s.y",objman->getobjectname(valno));
                          break;
case TYPE_OBJECT_FRAMENO: /* object frame number */
                          sprintf(dest,"(object)%s.frameno",objman->getobjectname(valno));
                          break;
 case TYPE_OBJECT_MOVING: /* object is moving flag */
                          sprintf(dest,"(object)%s.moving",objman->getobjectname(valno));
                          break;
case TYPE_OBJECT_ANIMATING: /* object is animating flag */
                          sprintf(dest,"(object)%s.animating",objman->getobjectname(valno));
                          break;
case TYPE_OBJECT_PRIORITY: /* object priority code */
                          sprintf(dest,"(object)%s.prioritycode",objman->getobjectname(valno));
                          break;
case TYPE_SPECIAL_RANDOM: /* random value */
                          sprintf(dest,"(value)random[%u]",valno);
                          break;
    }
}

void
SCRIPT::resolvexpression(char* dest) {
    char v1[MAD_TEMPSTR_SIZE];
    char v2[MAD_TEMPSTR_SIZE];
    _UCHAR condition;

    /* first get all values */
    condition=getbyte();
    resolvevalue(v1);
    resolvevalue(v2);

    sprintf(dest,"%s %s %s",v1,resolvecondition(condition),v2);
}

/*
 * SCRIPT::resolvevalue(char* dest)
 *
 * This will resolve the value opcodes to human-readable text in [dest].
 *
 */
void
SCRIPT::resolvevalue(char* dest) {
    _UCHAR type,expr;
    _ULONG value;
    char   tempstr[MAD_TEMPSTR_SIZE];

    /* get the values */
    type=getbyte();
    if(type==TYPE_STATIC) {
        value=getdword();
    } else {
        value=getword();
    }
    expr=getbyte();

    /* do the first step of the resolving */
    resolvevar(type,value,dest);

    /* check whether this is an expression */
    if(expr!=EXPR_NONE) {
        /* it's an expression! add it */
        switch(expr) {
            case EXPR_ADD: /* addition */
                           strcat(dest," + ");
                           break;
            case EXPR_SUB: /* subtraction */
                           strcat(dest," - ");
                           break;
                  default: /* unknown */
                           strcat(dest," ? ");
                           break;
        }
        /* now the actual value */
        resolvevalue(tempstr);
        strcat(dest,tempstr);
    }
}

/*
 * SCRIPT::disassembleobj(_ULONG codeip,char* dest,_UINT objno)
 *
 * This will disassemble script object code at the current eip to [dest]. It
 * expects object [objno] to hold the object number.
 *
 */
void
SCRIPT::disassembleobj(char* dest,_UINT objno) {
    _UCHAR t1,t2;
    _UINT a,b,c,d;
    _ULONG i,j;
    char objname[SCRIPT_MAXIDENTLEN];
    char objtemp1[SCRIPT_MAXIDENTLEN];
    char objtemp2[SCRIPT_MAXIDENTLEN];
    char objtemp3[SCRIPT_MAXIDENTLEN];
    char objtemp4[SCRIPT_MAXIDENTLEN];
    char tempstr[MAD_TEMPSTR_SIZE];
    _ULONG saveip,w;

    /* get the object name */
    strcpy(objname,objman->getobjectname(objno));

    /* get the instruction */
    switch(getbyte()) {
        case OPCODE_OBJ_MOVE: /* object wants to move */
                              /* save the eip */
                              saveip=eip;
                              resolvevalue(objtemp1);
                              resolvevalue(objtemp2);
                              /* restore the eip */

                              eip=saveip;
                              i=getevalue();j=getevalue();
                              sprintf(dest,"%s.move %s %s (%u,%u)",objname,objtemp1,objtemp2,i,j);
                              return;
     case OPCODE_OBJ_SETHALT: /* set object halt sequence */
                              getstring(objtemp1,SCRIPT_MAXIDENTLEN);
                              sprintf(dest,"%s.sethalt '%s'",objname,objtemp1);
                              return;
     case OPCODE_OBJ_SETMOVE: /* set object move sequence */
                              getstring(objtemp1,SCRIPT_MAXIDENTLEN);
                              sprintf(dest,"%s.setmove '%s'",objname,objtemp1);
                              return;
    case OPCODE_OBJ_SETSPEED: /* set speed of object */
                              i=getword(); j=getword(); a=getword();
                              sprintf(dest,"%s.setspeed x=%u y=%u frame=%u\n",objname,i,j,a);
                              return;
        case OPCODE_OBJ_FACE: /* object must face a direction */
                              i=getbyte();
                              sprintf(dest,"%s.setdirection %u",objname,i&0xff);
                              return;
        case OPCODE_OBJ_SHOW: /* show the object */
                              sprintf(dest,"%s.show",objname);
                              return;
        case OPCODE_OBJ_HIDE: /* hide the object */
                              sprintf(dest,"%s.hide",objname);
                              return;
      case OPCODE_OBJ_ENABLE: /* enable the object */
                              sprintf(dest,"%s.enable",objname);
                              return;
     case OPCODE_OBJ_DISABLE: /* disable the object */
                              sprintf(dest,"%s.disable",objname);
                              return;
     case OPCODE_OBJ_LOADSPR: /* load a sprite for an object */
                              getstring(objtemp1,SCRIPT_MAXIDENTLEN);
                              sprintf(dest,"%s.loadsprite '%s'",objname,objtemp1);
                              return;
     case OPCODE_OBJ_ADDICON: /* add an icon to the button bar */
                              getstring(objtemp1,SCRIPT_MAXIDENTLEN);
                              getstring(objtemp2,SCRIPT_MAXIDENTLEN);
                              getstring(objtemp3,SCRIPT_MAXIDENTLEN);
                              getstring(objtemp4,SCRIPT_MAXIDENTLEN);
                              a=getword();
                              sprintf(dest,"%s.addicon '%s' '%s' '%s' '%s' flags $%x",objname,objtemp1,objtemp2,objtemp3,objtemp4,a);
                              return;
        case OPCODE_OBJ_LOAD: /* load a script */
                              getstring(objtemp1,SCRIPT_MAXIDENTLEN);
                              sprintf(dest,"%s.load '%s'",objname,objtemp1);
                              return;
          case OPCODE_OBJ_GO: /* run the loaded script */
                              sprintf(dest,"%s.go",objname);
                              return;
     case OPCODE_OBJ_DISPOSE: /* dispose the loaded script */
                              sprintf(dest,"%s.dispose",objname);
                              return;
     case OPCODE_OBJ_SETCLIP: /* set clipping to yes or no */
                              a=getbyte()&0xff;
                              sprintf(dest,"%s.setclip %u",objname,a);
                              return;
      case OPCODE_OBJ_MOVETO: /* move object to somewhere */
                              /* save the eip */
                              saveip=eip;
                              /* get the values */
                              resolvevalue(objtemp1);
                              resolvevalue(objtemp2);
                              /* restore the eip */
                              eip=saveip;
                              i=getevalue();j=getevalue();
                              sprintf(dest,"%s.moveto %s %s (%u,%u)",objname,objtemp1,objtemp2,i,j);
                              return;
     case OPCODE_OBJ_SETMASK: /* set object mask */
                              i=getbyte()&0xff;
                              sprintf(dest,"%s.setmask %u",objname,i);
                              return;
        case OPCODE_OBJ_EXIT: /* exit */
                              sprintf(dest,"%s.exit",objname);
                              return;
  case OPCODE_OBJ_PROMPTEXIT: /* exit with prompt */
                              sprintf(dest,"%s.promptexit",objname);
                              return;
  case OPCODE_OBJ_SETMOVEOBJ: /* set object to let bar move */
                              sprintf(dest,"%s.setmoveobj",objname);
                              return;
     case OPCODE_OBJ_SETANIM: /* set object animation */
                              getstring(objtemp1,SCRIPT_MAXIDENTLEN);
                              sprintf(dest,"%s.setanim '%s'",objname,objtemp1);
                              return;
     case OPCODE_OBJ_ANIMATE: /* start animating the object */
                              sprintf(dest,"%s.animate",objname);
                              return;
      case OPCODE_OBJ_UNLOAD: /* unload an object */
                              sprintf(dest,"%s.unload",objname);
                              return;
        case OPCODE_OBJ_STOP: /* stop moving */
                              sprintf(dest,"%s.stop",objname);
                              return;
      case OPCODE_OBJ_SETPRI: /* set object priority */
                              /* save the eip and get the type and number values */
                              saveip=eip;
                              resolvevalue(objtemp1);
                              /* restore the eip */
                              eip=saveip;
                              i=getevalue();
                              sprintf(dest,"%s.setpriority %s (%u)",objname,objtemp1,i);
                              return;
     case OPCODE_OBJ_SETLOOP: /* set looping */
                              i=getword()&0xff;
                              sprintf(dest,"%s.setloop %u",objname,i);
                              return;
   case OPCODE_OBJ_DARKENPAL: /* darken palette */
                              i=getword();
                              sprintf(dest,"%s.darkenpal %u",objname,i);
                              return;
  case OPCODE_OBJ_RESTOREPAL: /* restore palette */
                              sprintf(dest,"%s.restorepal",objname);
                              return;
     case OPCODE_OBJ_SETTEXT: /* change text of text object */
                              getstring(tempstr,MAD_TEMPSTR_SIZE);
                              sprintf(dest,"%s.settext '%s'",objname,tempstr);
                              return;
    case OPCODE_OBJ_SETCOLOR: /* set color of text */
                              /* save the eip and get the type and number values */
                              saveip=eip;
                              resolvevalue(objtemp1);
                              /* restore the eip */
                              eip=saveip;
                              i=getevalue();
                              sprintf(dest,"%s.setcolor %s (%u)",objname,objtemp1,i);
                              return;
     case OPCODE_OBJ_FADEOUT: /* fade out */
                              sprintf(dest,"%s.fadeout",objname);
                              return;
      case OPCODE_OBJ_FADEIN: /* fade in */
                              sprintf(dest,"%s.fadein",objname);
                              return;
    case OPCODE_OBJ_LOADTEXT: /* load text */
                              w=getevalue();
                              textman->getstring(w,tempstr,MAD_TEMPSTR_SIZE);
                              if((strlen(tempstr)*fontman->getfontwidth(fontman->getsysfontno()))>=(GFX_DRIVER_HRES)) {
                                  /* it's too long to fit on the screen */
                                  strcpy(tempstr,"...");
                              }
                              sprintf(dest,"%s.loadtext %lu (%s)",objname,w,tempstr);
                              return;
case OPCODE_OBJ_CREATETXTDLG: /* create a dialog */
                              i=getword();j=getword();
                              sprintf(dest,"%s.createdialog (%u,%u)",objname,i,j);
                              return;
  case OPCODE_OBJ_LOADBCKDRP: /* load backdrop for the dialog */
                              getstring(objtemp1,SCRIPT_MAXIDENTLEN);
                              sprintf(dest,"%s.loadbackdrop '%s'",objname,objtemp1);
                              return;
  case OPCODE_OBJ_LOADBORDER: /* load border for a dialog */
                              getstring(objtemp1,SCRIPT_MAXIDENTLEN);
                              getstring(objtemp2,SCRIPT_MAXIDENTLEN);
                              getstring(objtemp3,SCRIPT_MAXIDENTLEN);
                              getstring(objtemp4,SCRIPT_MAXIDENTLEN);
                              sprintf(dest,"%s.loadborder '%s' '%s' '%s' '%s'",objname,objtemp1,objtemp2,objtemp3,objtemp4);
                              return;
 case OPCODE_OBJ_SHOWMESSAGE: /* show a message */
                              w=getevalue();
                              textman->getstring(w,tempstr,MAD_TEMPSTR_SIZE);
                              if((strlen(tempstr)*fontman->getfontwidth(fontman->getsysfontno()))>=(GFX_DRIVER_HRES)) {
                                  /* it's too long to fit on the screen */
                                  strcpy(tempstr,"...");
                              }
                              sprintf(dest,"%s.showmessage %lu (%s)",objname,w,tempstr);
                              return;
    case OPCODE_OBJ_CONVERSE: /* converse */
                              /* save the eip and get the type and number values */
                              a=getword();b=getword();c=getword();d=getword();
                              saveip=eip;
                              resolvevalue(objtemp1);
                              /* restore the eip */
                              eip=saveip;
                              i=getevalue();
                              sprintf(dest,"%s.converse %u %u %u %u %s (%u)",objname,a,b,c,d,objtemp1,i);
                              return;
    }
    /* this is unknown. return some question marks */
    sprintf(dest,"???");
}

/*
 * SCRIPT::disassemble(_ULONG codeip,char* dest,_UCHAR* noskip)
 *
 * This will disassemble script code at [codeip] to [dest]. It will
 * return the number of bytes used. [noskip] should be a pointer to a byte,
 * which will be set to zero if the instruction can be skipped, and zero
 * otherwise.
 *
 */
_ULONG
SCRIPT::disassemble(_ULONG codeip,char* dest,_UCHAR* noskip) {
    _ULONG saveip,l,saveip2;
    char tempstr[MAD_TEMPSTR_SIZE];
    char tempstr2[MAD_TEMPSTR_SIZE];
    char objtemp[SCRIPT_MAXIDENTLEN];
    _UINT objno,temp;
    _ULONG val1,val2,temp2,temp3,i;
    _UCHAR type1,type2;

    /* set skippable to yes */
    *noskip=0;

    /* save the current eip */
    saveip=eip;

    /* set the new eip */
    eip=codeip;

    /* get the opcode */
    switch(getbyte()) {
        case OPCODE_OBJECT: /* it is an object opcode */
                            objno=getword();
                            disassembleobj(dest,objno);
                            break;
       case OPCODE_REFRESH: /* refresh the screen */
                            sprintf(dest,"refresh");
                            break;
          case OPCODE_JUMP: /* jump in the script */
                            l=getdword();
                            sprintf(dest,"jump $%x",l);
                            /* better not skip this */
                            *noskip=1;
                            break;
  case OPCODE_OBJ_ACTIVATE: /* active the object. now the object actually exists */
                            objno=getword();
                            /* now get the object type */
                            i=getbyte();

                            /* get filename & object name */
                            getstring(tempstr,SCRIPT_MAXIDENTLEN);   /* filename */
                            getstring(objtemp,SCRIPT_MAXIDENTLEN);   /* object name */

                            sprintf(dest,"%s.activate '%s'",objtemp,tempstr);
                            break;
        case OPCODE_SETVAR: /* set variable number */
                            objno=getword();
                            /* save the eip and get the type and number values */
                            saveip2=eip;
                            /* resolve it into a string */
                            resolvevalue(tempstr);
                            /* restore the eip */
                            eip=saveip2;

                            i=getevalue();

                            sprintf(dest,"$%x.setvalue %s (%u)",objno,tempstr,i);
                            break;
            case OPCODE_IF: /* compare two things */
                            saveip2=eip;
                            i=getbyte();                /* condition */
                            val1=getevalue();           /* value 1 */
                            val2=getevalue();           /* value 2 */
                            /* restore the eip */
                            eip=saveip2;
                            /* resolve the expression */
                            resolvexpression(tempstr2);
                            temp2=getdword();
                            /* and print it */
                            sprintf(dest,"if (%s): ",tempstr2);
                            if(!expressiontrue(i,val1,val2)) {
                                strcat (dest,"false");
                            } else {
                                strcat (dest,"true");
                            }
                            break;
    case OPCODE_LOADSCREEN: /* load screens */
                            getstring(objtemp,SCRIPT_MAXIDENTLEN);

                            sprintf(dest,"loadscreen '%s'",objtemp);
                            break;
        case OPCODE_RETURN: /* return from this script */
                            sprintf(dest,"return");
                            /* better not skip this */
                            *noskip=1;
                            break;
         case OPCODE_WHILE: /* compare two things */
                            saveip2=eip;
                            i=getbyte();                /* condition */
                            val1=getevalue();           /* value 1 */
                            val2=getevalue();           /* value 2 */
                            /* restore the eip */
                            eip=saveip2;
                            /* resolve the expression */
                            resolvexpression(tempstr2);
                            temp2=getdword();

                            sprintf(dest,"while (%s): ",tempstr2);
                            if(!expressiontrue(i,val1,val2)) {
                                strcat (dest,"false");
                            } else {
                                strcat (dest,"true");
                            }
                            break;
	      case OPCODE_LEAVE: /* leave the script */
                            sprintf(dest,"leave");
                            /* better not skip this */
                            *noskip=1;
                            break;
           case OPCODE_NOP: /* no operation */
                            sprintf(dest,"noop");
                            break;
                   default: /* this is unknown. return question marks */
                            sprintf(dest,"???");
                            break;
    }

    /* calculate the number of bytes processed */
    l=eip-codeip;

    /* restore the old ip */
    eip=saveip;

    /* and return the number of bytes processed */
    return l;
}
#endif

/*
 * SCRIPT::expressiontrue(_UCHAR condition,_UINT result1,_UINT result2)
 *
 * This will return zero if the condition is false, otherwise non-zero.
 *
 */
_UCHAR
SCRIPT::expressiontrue(_UCHAR condition,_UINT result1,_UINT result2) {
    char tempstr[MAD_TEMPSTR_SIZE];
    _UCHAR err;

    switch(condition) {
           case CHECK_EQUAL: /* check whether [v1] == [v2] */
                             return (result1==result2);
        case CHECK_NOTEQUAL: /* check whether [v1] != [v2] */
                             return (result1!=result2);
       case CHECK_GREATEREQ: /* check whether [v1] >= [v2] */
                             return (result1>=result2);
       case CHECK_SMALLEREQ: /* check whether [v1] <= [v2] */
                             return (result1<=result2);
         case CHECK_GREATER: /* check whether [v1] > [v2] */
                             return (result1>result2);
         case CHECK_SMALLER: /* check whether [v1] < [v2] */
                             return (result1<result2);
    }
    /* unknown condition. die (error 105: condition is unknown) */
    sprintf(tempstr,MAD_ERROR_105,condition);
    die(tempstr);
    /* NOTREACHED */
    return 0;
}

/*
 * SCRIPT::getevalue()
 *
 * This will return the next bytes as an evaluated value. It will die on all
 * errors.
 *
 */
_ULONG
SCRIPT::getevalue() {
    char tempstr[MAD_TEMPSTR_SIZE];
    _UCHAR type,error,expr;
    _ULONG val,i,v;

    type=getbyte();            /* type of expression */
    if(type==TYPE_STATIC) {
        val=getdword();        /* expression (dword) */
    } else {
        val=getword();         /* expression (word) */
    }

    v=getval(type,val,&error);
    if(error) {
       /* error 106: variable has type, which is unknown (offset) */
       sprintf(tempstr,MAD_ERROR_106,type,eip);
       die(tempstr);
    }
    /* now check the expression stuff (like foo = foo + bar */
    expr=getbyte();
    if(expr!=EXPR_NONE) {
        /* it's a real expression! */
        i=getevalue();
        switch(expr) {
            case EXPR_ADD: /* addition */
                           v+=i;
                           break;
            case EXPR_SUB: /* subtraction */
                           v-=i;
                           break;
                  default: /* unknown value. die (error 118: unknown expression type value) */
                           sprintf(tempstr,MAD_ERROR_118,expr);
                           die(tempstr);
        }
    }

    return v;
}

void
SCRIPT::handleobjectopcode() {
    char tempstr[MAD_TEMPSTR_SIZE];
    char objtemp1[SCRIPT_MAXIDENTLEN];
    char objtemp2[SCRIPT_MAXIDENTLEN];
    char objtemp3[SCRIPT_MAXIDENTLEN];
    char objtemp4[SCRIPT_MAXIDENTLEN];
    _UINT objno,a,b,c,d;
    _ULONG w,i,j;

    /* was the previous opcode actually an OPCODE_OBJECT */
    if(scriptcode[eip-1]!=OPCODE_OBJECT) {
        /* no, report and die (error 107: handeobjectopcode called with previous opcode) */
        sprintf(tempstr,MAD_ERROR_107,scriptcode[eip-1]);
        die(tempstr);
        /* NOTREACHED */
    }
    /* figure out the object number */
    objno=getword();
    if(objno>=OBJMAN_MAXOBJECTS) {
        /* this object number is not valid. die (error 108: tried to access object, is the maximum) */
        sprintf(tempstr,MAD_ERROR_108,objno,OBJMAN_MAXOBJECTS);
        die(tempstr);
        /* NOTREACHED */
    }
    /* lets rock! */
    switch(getbyte()) {
        case OPCODE_OBJ_MOVE: /* object wants to move. do it */
                              i=getevalue();j=getevalue();
                              objman->setcoords(objno,(_SINT)i,(_SINT)j);
                              break;
     case OPCODE_OBJ_SETHALT: /* set object halt sequence */
                              getstring(objtemp1,SCRIPT_MAXIDENTLEN);

                              if(!gfx->sethaltsequence(objno,objtemp1)) {
                                  /* sethaltsequence failed. aargh! (error 109: unable to set halt sequence at */
                                  sprintf(tempstr,MAD_ERROR_109,objtemp1,eip);
                                  die(tempstr);
                                  /* NOTREACHED */
                              }
                              break;
     case OPCODE_OBJ_SETMOVE: /* set object move sequence */
                              getstring(objtemp1,SCRIPT_MAXIDENTLEN);
                              gfx->setmovesequence(objno,objtemp1);
                              break;
    case OPCODE_OBJ_SETSPEED: /* set speed of object */
                              i=getword(); j=getword(); a=getword();
                              objman->setobjectspeed(objno,a,i,j);
                              break;
        case OPCODE_OBJ_FACE: /* object must face a direction */
                              objman->setdirection(objno,getbyte());
                              break;
        case OPCODE_OBJ_SHOW: /* show the object */
                              objman->setvisible(objno,1);
                              break;
        case OPCODE_OBJ_HIDE: /* hide the object */
                              objman->setvisible(objno,0);
                              break;
      case OPCODE_OBJ_ENABLE: /* enable the object */
                              objman->setenabled(objno,1);
                              break;
     case OPCODE_OBJ_DISABLE: /* disable the object */
                              objman->setenabled(objno,0);
                              break;
     case OPCODE_OBJ_LOADSPR: /* load a sprite for an object */
                              getstring(objtemp1,SCRIPT_MAXIDENTLEN);
                              objman->setsourceno(objno,gfx->loadsprite(objtemp1));
                              break;
     case OPCODE_OBJ_ADDICON: /* add an icon to the button bar */
                              getstring(objtemp1,SCRIPT_MAXIDENTLEN);
                              getstring(objtemp2,SCRIPT_MAXIDENTLEN);
                              getstring(objtemp3,SCRIPT_MAXIDENTLEN);
                              getstring(objtemp4,SCRIPT_MAXIDENTLEN);
                              bar->addicon(objtemp1,objtemp2,objtemp3,objtemp4,getword());
                              break;
        case OPCODE_OBJ_LOAD: /* load a script */
                              getstring(objtemp1,SCRIPT_MAXIDENTLEN);
                              script->init(objtemp1);
                              script->parse();
                              break;
          case OPCODE_OBJ_GO: /* run the loaded script */
                              script->goproc(SCRIPT_PROC_INIT);
                              script->running=1;
                              while(script->running) {
                                  script->goproc(SCRIPT_PROC_RUN);
                                  mainscript->goproc(SCRIPT_PROC_RUN);
                                  gfx->redraw(1);
                                  controls->poll();
                                  idler();
                                  parser->handlevent();
                              }
                              break;
     case OPCODE_OBJ_DISPOSE: /* dispose the loaded script */
                              if(this!=mainscript) {
                                  /* we arnt. report and die (error 110: can only dispose scripts from main script) */
                                  sprintf(tempstr,MAD_ERROR_110);
                                  die(tempstr);
                              }
                              script->goproc(SCRIPT_PROC_DONE);
                              script->done();
                              break;
     case OPCODE_OBJ_SETCLIP: /* set clipping to yes or no */
                              objman->setclipping(objno,getbyte());
                              break;
      case OPCODE_OBJ_MOVETO: /* move object to somewhere */
                              i=getevalue();j=getevalue();
                              objman->setdestcoords(objno,(_SINT)i,(_SINT)j);
                              objman->setmove(objno,1);
                              break;
     case OPCODE_OBJ_SETMASK: /* set object mask */
                              i=getbyte();
                              objman->setmaskdir(objno,i);
                              break;
        case OPCODE_OBJ_EXIT: /* exit */
                              mad_exit();
                              /* NOTREACHED */
  case OPCODE_OBJ_PROMPTEXIT: /* exit with prompt */
                              if(dlgman->askyesno(DLGMAN_DIALOG_ASKEXIT,DLGMAN_DIALOG_ASKEXITITLE)) {
                                  mad_exit();
                                  /* NOTREACHED */
                              }
                              break;
  case OPCODE_OBJ_SETMOVEOBJ: /* set object to let bar move */
                              bar->setobjtomove(objno);
                              break;
     case OPCODE_OBJ_SETANIM: /* set object animation */
                              getstring(objtemp1,SCRIPT_MAXIDENTLEN);
   			                  i=gfx->findanmsequence(objman->getsourceno(objno),objtemp1);
                			      if (i==GFX_NOBJECT) {
                                  /* we couldnt find the animation, so die with this error (error 111: couldn't find animation sequence for object) */
                    				    sprintf(tempstr,MAD_ERROR_111,objtemp1,objno);
              				          die(tempstr);
                              }
                              objman->setanimno(objno,i);
                              objman->setframeno(objno,0);
			                     break;
     case OPCODE_OBJ_ANIMATE: /* start animating the object */
                              objman->setframeno(objno,0);
                              objman->setanimating(objno,1);
			                     break;
      case OPCODE_OBJ_UNLOAD: /* unload an object */
                              objman->unloadobject(objno);
			                     break;
        case OPCODE_OBJ_STOP: /* stop moving */
                              objman->setmove(objno,0);
	                           break;
      case OPCODE_OBJ_SETPRI: /* set object priority */
                              i=getevalue();
                              if(i>GFX_MAX_PRIORITIES) {
                                  /* this priority cannot be possible. die (error 112: cannot set priority level to when there are priorities possible) */
 				                      sprintf(tempstr,MAD_ERROR_112,i,GFX_MAX_PRIORITIES);
				                      die(tempstr);
                                  /* NOTREACHED */
                              }
                              objman->setbasepriority(objno,i);
                              break;
     case OPCODE_OBJ_SETLOOP: /* set looping */
                              i=getword();
                              objman->setlooping(objno,(_UCHAR)(i&0xff));
                              break;
   case OPCODE_OBJ_DARKENPAL: /* darken palette */
                              i=getword();
                              gfx->darkenpalette((_UCHAR)(i&0xff));
                              break;
  case OPCODE_OBJ_RESTOREPAL: /* restore palette */
                              gfx->restorepalette();
                              break;
     case OPCODE_OBJ_SETTEXT: /* change text of text object */
                              getstring(tempstr,MAD_TEMPSTR_SIZE);
                              objman->setext(objno,tempstr);
                              break;
    case OPCODE_OBJ_SETCOLOR: /* set color of text */
                              i=getevalue();
                              objman->setcolor(objno,(_UCHAR)(i&0xff));
                              break;
     case OPCODE_OBJ_FADEOUT: /* fade out */
//                              gfx->fadeout();
                              break;
      case OPCODE_OBJ_FADEIN: /* fade in */
//                              gfx->fadein();
                              break;
    case OPCODE_OBJ_LOADTEXT: /* load text */
                              w=getevalue();
                              /* is this the internal dialog object? */
                              if(objno==OBJMAN_DIALOG_OBJNO) {
                                  /* yeah. resize the dialog */
                                  textman->getstring(w,tempstr,MAD_TEMPSTR_SIZE);

                                  dlgman->dialog[mainscript->dialogno]->resize(
                                      fontman->getextheight(fontman->getsysfontno(),tempstr)+4,
                                      fontman->getextwidth(fontman->getsysfontno(),tempstr)+4);

                                  dlgman->dialog[mainscript->dialogno]->control[0]->setcaption(tempstr);
                              } else {
                                  /* nope, its just a text object */
                                  textman->getstring(w,tempstr,MAD_TEMPSTR_SIZE);
                                  objman->setext(objno,tempstr);
                              }
                              break;
case OPCODE_OBJ_CREATETXTDLG: /* create a text dialog (will always contain a
                                 label control) */
                              i=getword();j=getword();
                              mainscript->dialogno=dlgman->createdialog(0,i,j,0,0);

                              /* create a label control in it */
                              dlgman->dialog[mainscript->dialogno]->createcontrol(DLGMAN_CONTROL_TYPE_LABEL);

                              /* and set the label properties */
                              dlgman->dialog[mainscript->dialogno]->resize(0,0);
                              /* XXX: TODO: This should be done better */
                              dlgman->dialog[mainscript->dialogno]->control[0]->setcolor(0);
                              break;
  case OPCODE_OBJ_LOADBCKDRP: /* load a sprite for an object */
                              getstring(objtemp1,SCRIPT_MAXIDENTLEN);
                              dlgman->dialog[mainscript->dialogno]->loadbackdrop(objtemp1);
                              break;
  case OPCODE_OBJ_LOADBORDER: /* load border for a dialog */
                              getstring(objtemp1,SCRIPT_MAXIDENTLEN);
                              getstring(objtemp2,SCRIPT_MAXIDENTLEN);
                              getstring(objtemp3,SCRIPT_MAXIDENTLEN);
                              getstring(objtemp4,SCRIPT_MAXIDENTLEN);
                              dlgman->dialog[mainscript->dialogno]->loadborder(objtemp1,objtemp2,objtemp3,objtemp4);
                              break;
    case OPCODE_OBJ_DODIALOG: /* do the dialog */
                              i=getword();
                              a=0;
                              while((a!=i)&&(i)) {
                                  /* redraw the screen */
                                  script->goproc(SCRIPT_PROC_RUN);
                                  mainscript->goproc(SCRIPT_PROC_RUN);

                                  gfx->redraw(1);
                                  controls->poll();
                                  idler();

                                  if(controls->button1pressed()||controls->button2pressed()) break;
                                  if(controls->getch()) break;

                                  a++;
                              }
                              /* make sure no buttons are pressed */
                              controls->waitnobutton();

                              /* call the 'do dialog finished' procedure */
                              goproc(SCRIPT_PROC_DODIALOG_DONE);
                              break;
 case OPCODE_OBJ_SHOWMESSAGE: /* show a message */
                              w=getevalue();
                              textman->getstring(w,tempstr,MAD_TEMPSTR_SIZE);
                              dlgman->showmessage(tempstr);
                              break;
    case OPCODE_OBJ_CONVERSE: /* converse */
                              a=getword();b=getword();c=getword();d=getword();
                              i=getevalue();
                              /* do it */
                              objman->setsourceno(OBJMAN_DIALOG_OBJNO,do_conversation(a,b,c,d,i));
                              return;
                     default: /* no known opcode found. die (error 100: unknown object opcode at offset) */
                              sprintf(tempstr,MAD_ERROR_100,scriptcode[eip-1],eip-1);
                              die(tempstr);
                              /* NOTREACHED */
    }
}

_UCHAR
SCRIPT::handleopcode() {
    char tempstr[MAD_TEMPSTR_SIZE];
    char tempstr2[MAD_TEMPSTR_SIZE];
    char objtemp[SCRIPT_MAXIDENTLEN];
    _UINT objno,val1,val2,temp;
    _ULONG temp2,temp3;
    _UCHAR type1,type2,i;

    #ifdef DEBUG_AVAILABLE
        if(debug_step) {
            /* don't trace the main script */
            if((debug_step==2)&&(this!=mainscript)) debug_launchdebugger();
            if(debug_step==1) debug_launchdebugger();
        }
        /* if there's a breakpoint here, launch the debugger */
        if((isbreakpoint(eip)!=SCRIPT_NOBREAKPOINT)&&(!debug_step)) {
            /* launch the debugger */
            debug_launchdebugger();
        }
    #endif
    switch(getbyte()) {
        case OPCODE_OBJECT: /* it is an object opcode */
                            handleobjectopcode();
                            break;
       case OPCODE_REFRESH: /* refresh the screen */
                            gfx->redraw(1);
                            break;
          case OPCODE_JUMP: /* jump in the script */
                            eip=getword();
                            break;
  case OPCODE_OBJ_ACTIVATE: /* active the object. now the object actually exists */
                            objno=getword();
                            if(objno>=OBJMAN_MAXOBJECTS) {
                                /* this object number is not valid. die (error 108: tried to access object, is the maximum) */
                                sprintf(tempstr,MAD_ERROR_108,objno);
                                die(tempstr);
                                /* NOTREACHED */
                            }
                            /* now get the object type */
                            i=getbyte();

                            /* get filename & object name */
                            getstring(tempstr,SCRIPT_MAXIDENTLEN);   /* filename */
                            getstring(objtemp,SCRIPT_MAXIDENTLEN);   /* object name */
                            /* create the object */
                            switch(i) {
                                case OBJMAN_TYPE_ANM: /* it's an animation object */
                                                      objman->setanmobject(objno,tempstr,objtemp);
                                                      break;
                               case OBJMAN_TYPE_TEXT: /* is's a text object */
                                                      objman->setextobject(objno,objtemp,tempstr);
                                                      break;
                                             default: /* it's an unknown object. die (error 115: activate with invalid object type) */
                                                      sprintf(tempstr,MAD_ERROR_115,i);
                                                      die(tempstr);
                            }
                            break;
        case OPCODE_SETVAR: /* set variable number */
                            objno=getword();
                            objman->setsourceno(objno,getevalue());
                            break;
            case OPCODE_IF: /* compare two things */
                            i=getbyte();                /* condition */
                            val1=getevalue();           /* value 1 */
                            val2=getevalue();           /* value 2 */
                            temp2=getdword();           /* location to jump to if false */
                            if(!expressiontrue(i,val1,val2)) {
                                eip=temp2;
                                return 0;
                            }
                            break;
    case OPCODE_LOADSCREEN: /* load screens */
                            getstring(objtemp,SCRIPT_MAXIDENTLEN);
                            gfx->loadscreen(objtemp);
                            break;
        case OPCODE_RETURN: /* return from this script */
                            return 1;
         case OPCODE_WHILE: /* compare two things */
                            temp2=eip;          /* save my current eip */

                            i=getbyte();        /* condition */
                            val1=getevalue();   /* expr 1 */
                            val2=getevalue();   /* expr 2 */
                            temp3=getdword();   /* location to jump to */
                            if(expressiontrue(i,val1,val2)) {
                                handleopcode();
                                eip=temp2-1;
                                return 0;
                            }
                            break;
         case OPCODE_LEAVE: /* leave the script */
                            /* does the user try to leave the root script? */
                            if(this==mainscript) {
                                /* yeah. die (error 119: cannot leave root script) */
                                die(MAD_ERROR_119);
                                /* NOTREACHED */
                            }
                            script->running=0;
                            return 1;
           case OPCODE_NOP: /* no operation. do nothing */
                            break;
          case OPCODE_CALL: /* call another procedure */
                            getstring(objtemp,SCRIPT_MAXIDENTLEN);
                            /* call it */
                            goproc(objtemp);
                            break;
                   default: /* no known opcode found. die (error 113: unknown opcode at offset) */
                            sprintf(tempstr,MAD_ERROR_113,scriptcode[eip-1],eip-1);
                            die(tempstr);
                            /* NOTREACHED */
    }
    return 0;
}

void
SCRIPT::goproc(char *procname) {
    char tempstr[MAD_TEMPSTR_SIZE];
    _UINT procno;
    _ULONG save_eip;

    #ifdef DEBUG_AVAILABLE
    /* set the flag */
    mainscript->mainactive=(this==mainscript);
    #endif

    /* does this procedure actually exist? */
    if((procno=findproc(procname))==SCRIPT_NOIDENT) {
        /* no, so die (error 114: procedure never declared) */
        sprintf(tempstr,MAD_ERROR_114,procname);
        die(tempstr);
        /* NOTREACHED */
    }
    /* if no code, return */
    if(proc[procno].start_pos==proc[procno].end_pos) return;
    /* save eip */
    save_eip=eip;
    /* and set eip to the new address */
    eip=proc[procno].start_pos;
    while(1) {
        idler();
        if (handleopcode()) break;
        if(eip>=proc[procno].end_pos) break;
        controls->poll();
    }
    /* restore eip */
    eip=save_eip;

    #ifdef DEBUG_AVAILABLE
    /* set the flag */
    mainscript->mainactive=0;
    #endif
}

void
SCRIPT::done() {
    if(scriptcode!=NULL) {
        free(scriptcode);
    	scriptcode=NULL;
    }
    /* make sure there is no filename */
    filename[0]='\0';
}

_UCHAR
SCRIPT::proc_exists(char *procname) {
    return (script->findproc(procname)!=SCRIPT_NOIDENT);
}

/*
 * SCRIPT::getfilename()
 *
 * This will return a pointer to the filename of the current script, or NULL
 * if no script has been loaded.
 *
 */
char*
SCRIPT::getfilename() {
    if(filename[0]=='\0') return NULL;
    return filename;
}

/*
 * SCRIPT::geteip()
 *
 * This will return the enhanced instruction pointer (EIP) of the script.
 *
 */
_ULONG
SCRIPT::geteip() {
    return eip;
}

/*
 * SCRIPT::seteip(_ULONG neweip)
 *
 * This will set the enhanced instruction pointer (EIP) of the script to
 * [neweip].
 *
 */
void
SCRIPT::seteip(_ULONG neweip) {
    eip=neweip;
}

/*
 * SCRIPT::mainscriptactive()
 *
 * This will return zero if the secondary script is running, otherwise
 * non-zero.
 *
 */
_UCHAR
SCRIPT::mainscriptactive() {
    return mainactive;
}

#ifdef DEBUG_AVAILABLE
/*
 * SCRIPT::getscriptsize()
 *
 * This will return the size of the script.
 *
 */
_ULONG
SCRIPT::getscriptsize() {
    return scriptcodesize;
}

/*
 * SCRIPT::changescriptcode(_ULONG pos,_UCHAR newbyte)
 *
 * This will change script code byte at [pos] to [newbyte].
 *
 */
void
SCRIPT::changescriptcode(_ULONG pos,_UCHAR newbyte) {
    scriptcode[pos]=newbyte;
}

/*
 * SCRIPT::addbreakpoint(_ULONG pos)
 *
 * This will add a breakpoint whenever the code at position [pos] is about to
 * be executed. If it could not add a breakpoint, it will return
 * SCRIPT_NOBREAKPOINT otherwise the breakpoint id number.
 *
 */
_UCHAR
SCRIPT::addbreakpoint(_ULONG pos) {
    _UCHAR i;

    /* is there's already a breakpoint here? */
    if((i=isbreakpoint(pos))!=SCRIPT_NOBREAKPOINT) {
        /* yeah, return its id */
        return i;
    }

    /* scan all breakpoints */
    for(i=0;i<DEBUG_MAX_BREAKPOINTS;i++) {
        /* is this one free? */
        if(breakpoint[i]==SCRIPT_NOBREAKPOINTADDRESS) {
            /* yeah. gotcha! */
            breakpoint[i]=pos;
            return i;
        }
    }
    /* nope, no luck. return SCRIPT_NOBREAKPOINT */
    return SCRIPT_NOBREAKPOINT;
}

/*
 * SCRIPT::isbreakpoint(_ULONG pos)
 *
 * This will return SCRIPT_NOBREAKPOINT if there is no breakpoint at position
 * [pos], otherwise it will return the breakpoint number.
 *
 */
_UCHAR
SCRIPT::isbreakpoint(_ULONG pos) {
    _UCHAR i;

    /* scan all breakpoints */
    for(i=0;i<DEBUG_MAX_BREAKPOINTS;i++) {
        /* is this the one? */
        if(breakpoint[i]==pos) {
            /* yeah. gotcha! */
            return i;
        }
    }
    /* nope, no luck. return SCRIPT_NOBREAKPOINT */
    return SCRIPT_NOBREAKPOINT;
}

/*
 * SCRIPT::getbreakpoint(_UCHAR no)
 *
 * This will return the address of breakpoint [no].
 *
 */
_ULONG
SCRIPT::getbreakpoint(_UCHAR no) {
    return breakpoint[no];
}

/*
 * SCRIPT::clearbreakpoint(_UCHAR no)
 *
 * This will clear breakpoint [no].
 *
 */
void
SCRIPT::clearbreakpoint(_UCHAR no) {
    breakpoint[no]=SCRIPT_NOBREAKPOINTADDRESS;
}

/*
 * SCRIPT::getnofprocs()
 *
 * This will return the number of procedures.
 *
 */
_ULONG
SCRIPT::getnofprocs() {
    return header.nofprocs;
}

/*
 * SCRIPT::getprocinfo(_ULONG procno)
 *
 * This will return the procedure header of [procno].
 *
 */
PROCHEADER*
SCRIPT::getprocinfo(_ULONG procno) {
    return &proc[procno];
}
#endif
