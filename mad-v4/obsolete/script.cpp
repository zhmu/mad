#include <stdlib.h>
#include <string.h>
#include "anm.h"
#include "archive.h"
#include "bar.h"
#include "gadgets.h"
#include "gfx.h"
#include "mad.h"
#include "controls.h"
#include "script.h"

void
SCRIPT::init(char *fname) {
    /* say no procedures, labels or variables found */
    nofprocs=0;noflabels=0;nofvars=0;
    /* say no script loaded, and set stack pointer to end of stack */
    scriptdata=NULL;sp=SCRIPT_STACKSIZE;
	/* load the script file in memory */
	scriptdata=(char *)archive->readfile(fname,&scriptsize);
}

void
SCRIPT::pushl(ULONG value) {
    char tempstr[MAD_TEMPSTR_SIZE];

    /* put the value in the array */
    script_stack[sp]=value;
    /* decrement stack pointer */
    sp-=4;
    /* no stack left? */
    if (!sp) {
        /* no, so bomb out */
        sprintf(tempstr,"script_pushl(%lu): stack overflow\n",value);
        die(tempstr);
    }

void
SCRIPT::popl(ULONG *value) {
    char tempstr[MAD_TEMPSTR_SIZE];

    /* anything ever pushed?  */
    if (sp==SCRIPT_STACKSIZE) {
        /* no, so bomb out */
        sprintf(tempstr,"script_popl(): stack underflow\n");
        die(tempstr);
    }


    /* put the value in the array */
    *value=script_stack[sp];
    /* increment stack pointer */
    sp+=4;
}

UCHAR
SCRIPT::readline(ULONG lineno,char *s) {
	ULONG i,line,c;

	/* scan the entire buffer */
	s[0]=0; line=0; c=0;
	for(i=0;i<scriptsize;i++) {
        /* are we scanning the line we search? */
		if (line==lineno) {
     	    /* yes, add the char to the return buffer */
			s[c]=scriptdata[i];
			c++;
		}
		/* is there a cr? */
		if (scriptdata[i]==0xd) {
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

void
SCRIPT::parse() {
	ULONG line;
	UINT i;
	char s[SCRIPT_MAXLINESIZE];
    char s2[SCRIPT_MAXLINESIZE];
	char s3[SCRIPT_MAXLINESIZE];
    char *ptr;

	line=0;
	/* next loop reads all lines in the script file until
	   readline returns an error */
	while (readline(line,s)) {
        /* first zero out all fields */
        strcpy(s2,"");
        strcpy(s3,"");
        /* if sscanf able to fill in 2 variables? */
		if (sscanf(s,"%s %s",&s2,&s3)==2) {
		    /* yes. check if s3 is the SCRIPT_STARTPROC */
			if (!strcmp(s3,SCRIPT_STARTPROC)) {
                /* yes. we found a procedure, let's check if it was already defined */
                if ((i=findproc(s2))!=SCRIPT_NOIDENT) {
                    /* yes, bomb out with an error message */
                    sprintf(s,"script_parse(): procedure '%s' in line %lu is duplicated in line %lu\n",s2,proc[i].name,proc[i].startline,line);
                    die(s);
                }
                /* no, so add it to the list. is the name short enough? */
				if (strlen(s2)>SCRIPT_MAXIDENTLEN) {
                    /* no, bomb out with an error message (friendlier than a segmentation fault :-) */
					sprintf(s,"script_parse(): procedure name cannot exceed %u chars, and '%s' does\n",SCRIPT_MAXIDENTLEN,s2);
					die(s);
                }
                /* yes. let's go! */
                strcpy(proc[nofprocs].name,s2);
                proc[nofprocs].startline=line;
                proc[nofprocs].endline=SCRIPT_NOLINE;
                nofprocs++;
            }
            /* no, is s2 a SCRIPT_ENDPROC? */
            if (!strcmp(s2,SCRIPT_ENDPROC)) {
                /* yes. we found a procedure end. was it ever started? */
                if ((i=findproc(s3))==SCRIPT_NOIDENT) {
                    /* yes, so bomb out with an error message */
                    sprintf(s,"script_parse(): procedure '%s' ended in line %lu but never started\n",s2,line);
                    die(s);
                }
                /* no, so edit the endline field */
                proc[i].endline=line;
            }
            /* no, is s2 a SCRIPT_DEFINEVAR? */
            if (!strcmp(s2,SCRIPT_DEFINEVAR)) {
                /* yes. we found a variable declarator. check if it was ever declared before */
                if (findvariable(s3)!=SCRIPT_NOIDENT) {
                    /* yes, so bomb out with an error message */
                    sprintf(s,"script_parse(): variable '%s' was already defined somewhere else\n",s3);
                    die(s);
                }
                /* fill in the new variable info */
                strcpy(variable[nofvars].name,s3);
                variable[nofvars].value=0L;
                nofvars++;
            }
        }
        /* no, so it might be a label. check for a colon */
        if ((ptr=strchr(s2,':'))!=NULL) {
            /* it's a label, we found a colon. copy the name in s3 */
            for (i=0;i<(ptr-s2);i++) {
                s3[i]=s2[i];
                s3[i+1]=0;
            }
            /* check if this label was already declared elsewhere */
            if ((i=findlabel(s3))!=SCRIPT_NOIDENT) {
                /* yes, so bomb out with an error */
                sprintf(s,"error in line %lu: label '%s' already defined in line %lu\n",line,s3,label[i].line);
                die(s);
            }
            /* no, so add the label to the list */
            strcpy(label[noflabels].name,s3);
            label[noflabels].line=line;
            noflabels++;
        }
		line++;
	}
    /* now check all procedures for unterminated ones */
    for(i=0;i<nofprocs;i++) {
        /* is this one unterminated? */
        if (proc[i].endline==SCRIPT_NOLINE) {
            /* yes, bomb out with an message */
            sprintf(s,"script_parse(): procedure '%s' started in line %lu never ends\n",proc[i].name,proc[i].startline);
            die(s);
        }
    }
}

UINT
SCRIPT::findproc(char *name) {
        UINT i;

        /* trace all procedures */
        for(i=0;i<nofprocs;i++) {
                /* if they match, return the number */
                if (!strcmp(name,proc[i].name)) return i;
        }
        /* no matches found, so return SCRIPT_NOIDENT */
        return SCRIPT_NOIDENT;
}

UINT
SCRIPT::findlabel(char *name) {
        UINT i;

        /* trace all labels */
        for(i=0;i<noflabels;i++) {
                /* if they match, return the number */
                if (!strcmp(name,label[i].name)) return i;
        }
        /* no matches found, so return SCRIPT_NOIDENT */
        return SCRIPT_NOIDENT;
}

UINT
SCRIPT::findvariable(char *name) {
        UINT i;

        /* trace all variables */
        for(i=0;i<nofvars;i++) {
                /* if they match, return the number */
                if (!strcmp(name,variable[i].name)) return i;
        }
        /* no matches found, so return SCRIPT_NOIDENT */
        return SCRIPT_NOIDENT;
}


UCHAR
SCRIPT::goline(char *script_line,char *script_errmsg,ULONG *line) {
        UCHAR quit;
        UINT i,ok,objectno,j,c,pos;
        char *ptr;
        char script_s1[SCRIPT_MAXLINESIZE];
        char script_s2[SCRIPT_MAXLINESIZE];
        char script_s3[SCRIPT_MAXLINESIZE];
        char script_s4[SCRIPT_MAXLINESIZE];
        char script_s5[SCRIPT_MAXLINESIZE];
        char script_s6[SCRIPT_MAXLINESIZE];
        char script_temp[SCRIPT_MAXLINESIZE];

        char script_object[SCRIPT_MAXIDENTLEN];
        char script_method[SCRIPT_MAXIDENTLEN];

        /* empty previously read strings */
        strcpy(script_s1,"");strcpy(script_s2,"");strcpy(script_s3,"");
        strcpy(script_s3,"");strcpy(script_s4,"");strcpy(script_s5,"");
        strcpy(script_s6,"");strcpy(script_object,"");strcpy(script_method,"");

        /* divide the script line into field script_s[number] */
        sscanf(script_line,"%s %s %s %s %s %s",&script_s1,&script_s2,&script_s3,&script_s4,&script_s5,&script_s6);

        ok=0;
        /* check for a dot in the string, but no assign char */
        if (((ptr=strchr(script_s1,'.'))!=NULL)&&(strchr(script_s1,'=')==NULL)) {
             /* script_object is going to be the object name before the dot */
             for (i=0;i<ptr-script_s1;i++) {
                  script_object[i]=script_s1[i];
                  script_object[i+1]=0;
             }
             /* script_method is the method after the dot */
             for (i=0;i<(strlen(script_s1)-(ptr-script_s1))-1;i++) {
                 script_method[i]=script_s1[i+1+ptr-script_s1];
                 script_method[i+1]=0;
             }
             /* find the object */
             if ((objectno=gfx->findobject(script_object))==GFX_NOSPRITE) {
                 /* we cant find the object. report and die */
                 sprintf(script_errmsg,"object '%s' was never declared\n",script_object);
                 return SCRIPT_ERROR;
             }
             /* it is a statement of the form object.method. lets process it */
             if (!strcmp(script_method,SCRIPT_MOVE)) {
                 /* user wants to move the object. check if it is a internal object */
                 if (gfx->object[objectno].internal) {
                     /* it is internal. report and die */
                     sprintf(script_errmsg,"cannot move internal object '%s'\n",script_object);
                     return SCRIPT_ERROR;
                 }
                 /* try to resolve the x coordinate */
                 if ((gfx->object[objectno].x=resolvexpression(script_s2))==SCRIPT_NORESOLV) {
                     /* we cant, so bomb out */
                     sprintf(script_errmsg,"cannot resolve '%s'\n",script_s2);
                     return SCRIPT_ERROR;
                 }
                 /* try to resolve the y coordinate */
                 if ((gfx->object[objectno].y=resolvexpression(script_s3))==SCRIPT_NORESOLV) {
                     /* we cant, so bomb out */
                     sprintf(script_errmsg,"cannot resolve '%s'\n",script_s3);
                     return SCRIPT_ERROR;
                 }
                 ok=1;
             }
             if (!strcmp(script_method,SCRIPT_SHOW)) {
                 /* user wants to show the object. check if it is a internal object */
                 if (gfx->object[objectno].internal) {
                     /* it is internal. report and die */
                     sprintf(script_errmsg,"cannot show internal object '%s'\n",script_object);
                     return SCRIPT_ERROR;
                 }
                 gfx->object[objectno].visible=1;
                 ok=1;
             }
             if (!strcmp(script_method,SCRIPT_HIDE)) {
                 /* user wants to hide the object. check if it is a internal object */
                 if (gfx->object[objectno].internal) {
                     /* it is internal. report and die */
                     sprintf(script_errmsg,"cannot hide internal object '%s'\n",script_object);
                     return SCRIPT_ERROR;
                 }
                 gfx->object[objectno].visible=0;
                 ok=1;
             }
             if (!strcmp(script_method,SCRIPT_LOADSPR)) {
                 /* user wants to load a new sprite image */
                 /* script_s2 has quotes, so get rid of them */
                 stripquotes(script_s2,script_s6);
                 gfx->setobjectsource(objectno,gfx->loadsprite(script_s6));
                 ok=1;
             }
             if (!strcmp(script_method,SCRIPT_ENABLE)) {
                 /* user wants to enable the object. go for it! */
                 gfx->object[objectno].enabled=1;
                 ok=1;
             }
             if (!strcmp(script_method,SCRIPT_DISABLE)) {
                 /* user wants to disable the object. go for it! */
                 gfx->object[objectno].enabled=0;
                 ok=1;
             }
             if (!strcmp(script_method,SCRIPT_BAR_ADDICON)) {
                 /* user wishes to add an icon to the bar. check if it is the bar object */
                 if (objectno!=GFX_BAR_SPRITENO) {
                     /* it is not the bar object. report and quit */
                     sprintf(script_errmsg,"can only add icons to a bar object, not to a '%s' object\n",script_object);
                     return SCRIPT_ERROR;
                 }
                 /* script_s2, script_s3, script_s4 and script_s5 have quotes, so get rid of them */
                 stripquotes(script_s2,script_temp);strcpy(script_s2,script_temp);
                 stripquotes(script_s3,script_temp);strcpy(script_s3,script_temp);
                 stripquotes(script_s4,script_temp);strcpy(script_s4,script_temp);
                 stripquotes(script_s5,script_temp);strcpy(script_s5,script_temp);

                 bar->addicon(script_s2,script_s3,script_s4,script_s5,atoi(script_s6));
                 ok=1;
             }
             if (!strcmp(script_method,SCRIPT_CONTROLS_WAITACTION)) {
                 /* user wishes to wait until the user pushes a key or clicks
                    a button is it the controls object? */
                 if (objectno!=GFX_CONTROLS_SPRITENO) {
                     /* it is not the controls object. report and quit */
                     sprintf(script_errmsg,"can only wait for an action with the controls object, not from a '%s' object",script_object);
                     return SCRIPT_ERROR;
                 }
                 controls->waitevent();
                 ok=1;
             }
             if (!strcmp(script_method,SCRIPT_CONTROLS_WAITIDLE)) {
                 /* user wishes to wait until user stops touching anything. is it
                    the controls object? */
                 if (objectno!=GFX_CONTROLS_SPRITENO) {
                     /* it is not the controls object. report and quit */
                     sprintf(script_errmsg,"can only wait for an action with the controls object, not from a '%s' object",script_object);
                     return SCRIPT_ERROR;
                 }
                 controls->waitnobutton();
                 ok=1;
             }
             if (!strcmp(script_method,SCRIPT_DESTROY)) {
                /* user wishes to destroy a sprite. first check if user tries
                   to elimate a internal object */
                if (gfx->object[objectno].internal) {
                   /* yes. report and die */
                   sprintf(script_errmsg,"cannot destroy internal object '%s'\n",script_object);
                   return SCRIPT_ERROR;
                }
                /* destroy the object */
                gfx->destroyobject(objectno);
                ok=1;
             }
             if (!strcmp(script_method,SCRIPT_UNLOADSPR)) {
                /* user wishes to unload and destroy an object. first check if
                   user tries to elimate a internal object */
                if (gfx->object[objectno].internal) {
                   /* yes. report and die */
                   sprintf(script_errmsg,"cannot unload internal object '%s'\n",script_object);
                   return SCRIPT_ERROR;
                }
                gfx->unloadobject(objectno);
                ok=1;
             }
             if (!strcmp(script_method,SCRIPT_EXIT)) {
                /* user wishes to exit the interprinter. was this called from
                   the system object? */
                if (objectno!=GFX_SYSTEM_SPRITENO) {
                    /* it is not the system object. report and quit */
                    sprintf(script_errmsg,"method '%s' is only applicable for the system object",SCRIPT_EXIT);
                    return SCRIPT_ERROR;
                }
                /* return with exit code SCRIPT_QUIT */
                return SCRIPT_QUIT;
             }
             if (!strcmp(script_method,SCRIPT_SETANIM)) {
                /* user wants to set the animation of an object. check if it
                   is an animation object */
                if (gfx->object[objectno].type!=GFX_TYPE_ANM) {
                    /* it isnt. report and die */
                     sprintf(script_errmsg,"cannot set animation sequence for non-animation object '%s'\n",script_object);
                     return SCRIPT_ERROR;
                }
                /* script_s2 has quotes, so get rid of them */
                stripquotes(script_s2,script_temp);strcpy(script_s2,script_temp);

                i=gfx->findanmsequence(gfx->object[objectno].sourceno,script_s2);
                if (i==GFX_NOBJECT) {
                   /* we couldnt find the animation, so die with this error */
                   sprintf(script_errmsg,"couldn't find animation sequence '%s' for object '%s'\n",script_s2,script_object);
                   return SCRIPT_ERROR;
                }
                gfx->object[objectno].animno=i;
                ok=1;
             }
             if (!strcmp(script_method,SCRIPT_SETFRAME)) {
                /* user wants to set the animation frame of an object. check if it
                   is an animation object */
                if (gfx->object[objectno].type!=GFX_TYPE_ANM) {
                    /* it isnt. report and die */
                    sprintf(script_errmsg,"cannot set animation frame number for non-animation object '%s'\n",script_object);
                    return SCRIPT_ERROR;
                }
                i=atoi(script_s2);
                /* check if the object actually has that much frames */
                if(i>=gfx->getnoframes(objectno)) {
                    /* it doesnt. report and die */
                    sprintf(script_errmsg,"cannot set frame number for object '%s' to %u, since it only has %u frame\n",script_object,i,gfx->getnoframes(objectno));
                    return SCRIPT_ERROR;
                }
                gfx->object[objectno].frameno=i;
                ok=1;
             }
             if (!strcmp(script_method,SCRIPT_SETMOVE)) {
                /* user wants to set move sequence for the script. check if
                   it is an animation object */
                if (gfx->object[objectno].type!=GFX_TYPE_ANM) {
                    /* it isnt. report and die */
                    sprintf(script_errmsg,"cannot set move sequence for non-animation object '%s'\n",script_object);
                    return SCRIPT_ERROR;
                }
                /* script_s2 has quotes, so get rid of them */
                stripquotes(script_s2,script_temp);strcpy(script_s2,script_temp);

                strcat(script_temp,SCRIPT_MOVE_SEPARATOR);
                strcat(script_temp,SCRIPT_MOVE_NORTH);
                if ((i=gfx->findanmsequence(gfx->object[objectno].sourceno,script_temp))==GFX_NOBJECT) {
                    sprintf(script_errmsg,"couldn't find north move sequence for object '%s'\n",script_method,script_object);
                    return SCRIPT_ERROR;
                }
                gfx->object[objectno].move_sequence[SCRIPT_MOVE_NORTH_NO]=i;

                strcpy(script_temp,script_s2);
                strcat(script_temp,SCRIPT_MOVE_SEPARATOR);
                strcat(script_temp,SCRIPT_MOVE_EAST);
                if ((i=gfx->findanmsequence(gfx->object[objectno].sourceno,script_temp))==GFX_NOBJECT) {
                    sprintf(script_errmsg,"couldn't find east move sequence for object '%s'\n",script_method,script_object);
                    return SCRIPT_ERROR;
                }
                gfx->object[objectno].move_sequence[SCRIPT_MOVE_EAST_NO]=i;

                strcpy(script_temp,script_s2);
                strcat(script_temp,SCRIPT_MOVE_SEPARATOR);
                strcat(script_temp,SCRIPT_MOVE_SOUTH);
                if ((i=gfx->findanmsequence(gfx->object[objectno].sourceno,script_temp))==GFX_NOBJECT) {
                    sprintf(script_errmsg,"couldn't find south move sequence for object '%s'\n",script_method,script_object);
                    return SCRIPT_ERROR;
                }
                gfx->object[objectno].move_sequence[SCRIPT_MOVE_SOUTH_NO]=i;

                strcpy(script_temp,script_s2);
                strcat(script_temp,SCRIPT_MOVE_SEPARATOR);
                strcat(script_temp,SCRIPT_MOVE_WEST);
                if ((i=gfx->findanmsequence(gfx->object[objectno].sourceno,script_temp))==GFX_NOBJECT) {
                    sprintf(script_errmsg,"couldn't find west move sequence for object '%s'\n",script_method,script_object);
                    return SCRIPT_ERROR;
                }
                gfx->object[objectno].move_sequence[SCRIPT_MOVE_WEST_NO]=i;

                ok=1;
             }
             if (!strcmp(script_method,SCRIPT_SETHALT)) {
                /* user wants to set halt sequence for the object. check if
                   it is an animation object */
                if (gfx->object[objectno].type!=GFX_TYPE_ANM) {
                    /* it isnt. report and die */
                    sprintf(script_errmsg,"cannot set halt sequence for non-animation object '%s'\n",script_object);
                    return SCRIPT_ERROR;
                }
                /* script_s2 has quotes, so get rid of them */
                stripquotes(script_s2,script_temp);strcpy(script_s2,script_temp);

                if ((i=gfx->findanmsequence(gfx->object[objectno].sourceno,script_temp))==GFX_NOBJECT) {
                    sprintf(script_errmsg,"couldn't find halt sequence for object '%s'\n",script_method,script_object);
                    return SCRIPT_ERROR;
                }
                gfx->object[objectno].move_sequence[SCRIPT_MOVE_HALT]=i;
                ok=1;
             }
             if (!strcmp(script_method,SCRIPT_MOVETO)) {
                /* user wants the object to nicely move to somewhere. find out
                   if it is an animation object */
                if (gfx->object[objectno].type!=GFX_TYPE_ANM) {
                    /* it isnt. report and die */
                    sprintf(script_errmsg,"cannot set moveto coordinates for non-animation object '%s'\n",script_object);
                    return SCRIPT_ERROR;
                }
                /* try to resolve the x coordinate */
                if ((gfx->object[objectno].destx=resolvexpression(script_s2))==SCRIPT_NORESOLV) {
                    /* we cant, so bomb out */
                    sprintf(script_errmsg,"cannot resolve '%s'\n",script_s2);
                    return SCRIPT_ERROR;
                }
                /* try to resolve the y coordinate */
                if ((gfx->object[objectno].desty=resolvexpression(script_s3))==SCRIPT_NORESOLV) {
                    /* we cant, so bomb out */
                    sprintf(script_errmsg,"cannot resolve '%s'\n",script_s3);
                    return SCRIPT_ERROR;
                }
                gfx->object[objectno].move=1;
                ok=1;
             }
             if (!strcmp(script_method,SCRIPT_SETSPEED)) {
                /* user wants to set the speed for a move. find out if it is
                   an animation object */
                if (gfx->object[objectno].type!=GFX_TYPE_ANM) {
                    /* it isnt. report and die */
                    sprintf(script_errmsg,"cannot set moveto coordinates for non-animation object '%s'\n",script_object);
                    return SCRIPT_ERROR;
                }
                ok=1;
             }
             if (!strcmp(script_method,SCRIPT_FACE)) {
                /* user wants to look at an object. check if it is an
                   animation object */
                if (gfx->object[objectno].type!=GFX_TYPE_ANM) {
                    /* it isnt. report and die */
                     sprintf(script_errmsg,"cannot use face an non-animation object '%s'\n",script_object);
                     return SCRIPT_ERROR;
                }
                /* script_s2 has quotes, so get rid of them */
                gfx->object[objectno].animno=gfx->object[objectno].move_sequence[SCRIPT_MOVE_HALT];

                /* set the frame number */
                /* is it west? */
                if(!strcmp(script_s2,SCRIPT_MOVE_WEST)) {
                    /* yeah, set west frame */
                    gfx->object[objectno].frameno=SCRIPT_MOVE_WEST_NO;
                    ok=1;
                }
                /* no? is it east? */
                if(!strcmp(script_s2,SCRIPT_MOVE_EAST)) {
                    /* yeah, set east frame */
                    gfx->object[objectno].frameno=SCRIPT_MOVE_EAST_NO;
                    ok=1;
                }
                /* no? it north? */
                if(!strcmp(script_s2,SCRIPT_MOVE_NORTH)) {
                    /* yeah, set north frame */
                    gfx->object[objectno].frameno=SCRIPT_MOVE_NORTH_NO;
                    ok=1;
                }
                /* no? it south? */
                if(!strcmp(script_s2,SCRIPT_MOVE_SOUTH)) {
                    /* yeah, set south frame */
                    gfx->object[objectno].frameno=SCRIPT_MOVE_SOUTH_NO;
                    ok=1;
                }
                /* did we process the action? */
                if (!ok) {
                    /* no, print error and die */
                    sprintf(script_errmsg,"cannot halt object '%s' in unknown direction '%s'\n",script_object,script_s2);
                    return SCRIPT_ERROR;
                }
             }
             if(!strcmp(script_method,SCRIPT_STARTANIM)) {
                 /* user wants the object to start animating. it this an
                    animation object? */
                if (gfx->object[objectno].type!=GFX_TYPE_ANM) {
                    /* it isnt. report and die */
                     sprintf(script_errmsg,"cannot start animation of non-animation object '%s'\n",script_object);
                     return SCRIPT_ERROR;
                }
                /* set animation frame to first animation and enable animating
                   flag */
                gfx->object[objectno].frameno=0;
                gfx->object[objectno].animating=1;
                ok=1;
             }
             if(!strcmp(script_method,SCRIPT_SETLOOP)) {
                 /* user wants to say whether the object has to loop. is this
                    an animation object? */
                if (gfx->object[objectno].type!=GFX_TYPE_ANM) {
                    /* it isnt. report and die */
                     sprintf(script_errmsg,"cannot start looping flag of non-animation object '%s'\n",script_object);
                     return SCRIPT_ERROR;
                }
                /* set the looping flag */
                i=resolvexpression(script_s2);
                if(i==SCRIPT_NORESOLV) {
                     sprintf(script_errmsg,"cannot to resolve '%s'",script_s2);
                     exit(1);
                }
                gfx->object[objectno].looping=i;
                ok=1;
             }
             if (!strcmp(script_method,SCRIPT_LOADSCRIPT)) {
                 /* user wishes to load a script. is it the script object? */
                 if (objectno!=GFX_SCRIPT_SPRITENO) {
                     /* it is not the script object. report and quit */
                     sprintf(script_errmsg,"can only load scripts from script object, not from a '%s' object",script_object);
                     return SCRIPT_ERROR;
                 }
                 /* check if we are the main script */
                 if(this!=mainscript) {
                     /* we arnt. report and die */
                     sprintf(script_errmsg,"can only load scripts from main script");
                     return SCRIPT_ERROR;
                 }

                 /* script_s2 has quotes, so get rid of them */
                 stripquotes(script_s2,script_temp);strcpy(script_s2,script_temp);

                 script->init(script_s2);
                 script->parse();
                 ok=1;
             }
             if (!strcmp(script_method,SCRIPT_RUNSCRIPT)) {
                 /* user wishes to run a script. is it the script object? */
                 if (objectno!=GFX_SCRIPT_SPRITENO) {
                     /* it is not the script object. report and quit */
                     sprintf(script_errmsg,"can only run scripts from script object, not from a '%s' object",script_object);
                     return SCRIPT_ERROR;
                 }
                 /* check if we are the main script */
                 if(this!=mainscript) {
                     /* we arnt. report and die */
                     sprintf(script_errmsg,"can only run scripts from main script");
                     return SCRIPT_ERROR;
                 }
                 script->goproc(SCRIPT_PROC_INIT);
                 script->running=1;
                 while(script->running) {
                     script->goproc(SCRIPT_PROC_RUN);
                     mainscript->goproc(SCRIPT_PROC_RUN);
                     gfx->redraw();
                 }
                 ok=1;
             }
             if (!strcmp(script_method,SCRIPT_DISPOSESCRIPT)) {
                 /* user wishes to dispose a script. is it the script object? */
                 if (objectno!=GFX_SCRIPT_SPRITENO) {
                     /* it is not the script object. report and quit */
                     sprintf(script_errmsg,"can only dispose scripts from script object, not from a '%s' object",script_object);
                     return SCRIPT_ERROR;
                 }
                 /* check if we are the main script */
                 if(this!=mainscript) {
                     /* we arnt. report and die */
                     sprintf(script_errmsg,"can only dispose scripts from main script");
                     return SCRIPT_ERROR;
                 }
                 script->goproc(SCRIPT_PROC_DONE);
                 script->done();
                 ok=1;
             }
             if(!strcmp(script_method,SCRIPT_SETMASK)) {
                 /* user wants to set the mask direction. is this an animation
                    object? */
                if (gfx->object[objectno].type!=GFX_TYPE_ANM) {
                    /* it isnt. report and die */
                     sprintf(script_errmsg,"cannot set masking direction of non-animation object '%s'\n",script_object);
                     return SCRIPT_ERROR;
                }
                i=0xff;
                /* was it a top mask? */
                if(!strcmp(script_s2,SCRIPT_MASK_TOP)) {
                    /* yeah. lets do it! */
                    i=GFX_MASK_TOP;
                }
                /* was it a bottom mask? */
                if(!strcmp(script_s2,SCRIPT_MASK_BOTTOM)) {
                    /* yeah. lets do it! */
                    i=GFX_MASK_BOTTOM;
                }
                /* was a valid mask supplied? */
                if(i==0xff) {
                    /* no, so die with an error */
                    sprintf(script_errmsg,"mask parameter '%s' not understood\n",script_s2);
                    return SCRIPT_ERROR;
                }
                gfx->object[objectno].mask_dir=i;
                ok=1;
             }
             if(!strcmp(script_method,SCRIPT_SETCLIP)) {
                 /* user wants to set the clip flag. is this an animation
                    object? */
                if (gfx->object[objectno].type!=GFX_TYPE_ANM) {
                    /* it isnt. report and die */
                     sprintf(script_errmsg,"cannot set clipping flag of non-animation object '%s'\n",script_object);
                     return SCRIPT_ERROR;
                }
                i=0xff;
                /* was it true? */
                if(!strcmp(script_s2,SCRIPT_TRUE)) {
                    /* yeah. */
                    i=1;
                }
                /* was it false? */
                if(!strcmp(script_s2,SCRIPT_FALSE)) {
                    /* yeah. */
                    i=0;
                }
                /* was a valid flag supplied? */
                if(i==0xff) {
                    /* no, so die with an error */
                    sprintf(script_errmsg,"clip parameter '%s' not understood\n",script_s2);
                    return SCRIPT_ERROR;
                }
                gfx->object[objectno].clip=i;
                ok=1;
             }
             if(!strcmp(script_method,SCRIPT_STOP)) {
                 /* user wants to make an object stop moving. is this an
                    animation object? */
                if (gfx->object[objectno].type!=GFX_TYPE_ANM) {
                    /* it isnt. report and die */
                     sprintf(script_errmsg,"cannot set clipping flag of non-animation object '%s'\n",script_object);
                     return SCRIPT_ERROR;
                }
                gfx->object[objectno].move=0;
                ok=1;
             }
             /* add new methods here
                like this:

                if (!strcmp(script_method,SCRIPT_DUMMY)) {
                   // user wants to call the dummy function

                   ...code...

                   ok=1;
                }
             */
             /* check if we processed it */
             if (!ok) {
                 /* no? bomb out with an error */
                 sprintf(script_errmsg,"unknown method '%s' for object '%s'\n",script_method,script_object);
                 return SCRIPT_ERROR;
             }
             return SCRIPT_OK;
        }
        ok=0;
        /* check what the user wants to do */
        if (!strcmp(script_s1,SCRIPT_JUMP)) {
             /* user wants to jump to label in [script_s2]. is line NULL? */
             if (line==NULL) {
                /* yes, so return an error */
                sprintf(script_errmsg,"script wants to jump but line is null\n");
                return SCRIPT_ERROR;
             }
             if ((i=findlabel(script_s2))==SCRIPT_NOIDENT) {
                 /* script label was never declared. report and quit */
                 sprintf(script_errmsg,"label '%s' never declared",script_s2);
                 return SCRIPT_ERROR;
             }
             *line=label[i].line;
             ok=1;
        }
        if (!strcmp(script_s1,SCRIPT_REDRAW)) {
            /* user wants to redraw the screen, lets do it! */
            gfx->redraw();
            ok=1;
        }
        if (!strcmp(script_s1,SCRIPT_DEFINEVAR)) {
            /* this is a variable declaration. ignore it */
            ok=1;
        }
        if (!strcmp(script_s1,SCRIPT_IF)) {
            /* user wishes to check a variable for a value */
            ok=0;
            for(i=(strchr(script_line,')')-script_line)+1;i<strlen(script_line);i++) {
                script_method[ok]=script_line[i];
                script_method[ok+1]=0;
                ok++;
            }
            ok=processexpression(script_s2);
            /* was the condition true? */
            if (ok) {
                /* yes, so execute script_method */
                return (goline(script_method,script_errmsg,line));
            }
            /* no, so just continue */
            ok=1;
        }
        if (!strcmp(script_s1,SCRIPT_ASSIGN)) {
            /* user wishes to assign something. check what object it wants to assign */
            if (!strcmp(script_s4,SCRIPT_LOADSPR)) {
                /* user wishes to assign a sprite to [script_s2]. does
                   an sprite named like that already exist? */
                if (gfx->findobject(script_s2)!=SCRIPT_NOIDENT) {
                    /* yes, so bomb out with an error message */
                    sprintf(script_errmsg,"sprite '%s' already defined somewhere else\n",script_s2);
                    return SCRIPT_ERROR;
                }
                /* is there an active sprite unused */
                if ((i=gfx->findfreeobject())==SCRIPT_NOIDENT) {
                    /* no. quit with an error message */
                    sprintf(script_errmsg,"out of active sprites\n");
                    return SCRIPT_ERROR;
                }
                /* initialize the sprite */
                gfx->object[i].type=GFX_TYPE_SPRITE;
                gfx->object[i].x=0;
                gfx->object[i].y=0;
                gfx->object[i].enabled=0;
                strcpy(gfx->object[i].name,script_s2);
                /* script_s5 has quotes, so get rid of them */
                stripquotes(script_s5,script_s6);
                gfx->object[i].sourceno=gfx->loadsprite(script_s6);
                ok=1;
            } else {
                /* maybe a anm object? */
                if (!strcmp(script_s4,SCRIPT_LOADANM)) {
                    /* user wishes to assign a animation to [script_s2]. does
                       an object named like that already exist? */
                    if (gfx->findobject(script_s2)!=SCRIPT_NOIDENT) {
                        /* yes, so bomb out with an error message */
                        sprintf(script_errmsg,"animation '%s' already defined somewhere else\n",script_s2);
                        return SCRIPT_ERROR;
                    }
                    /* is there an active sprite unused */
                    if ((i=gfx->findfreeobject())==SCRIPT_NOIDENT) {
                        /* no. quit with an error message */
                        sprintf(script_errmsg,"out of active sprites\n");
                        return SCRIPT_ERROR;
                    }
                    /* initialize the sprite */
                    gfx->object[i].type=GFX_TYPE_ANM;
                    gfx->object[i].x=0;
                    gfx->object[i].y=0;
                    gfx->object[i].enabled=0;
                    strcpy(gfx->object[i].name,script_s2);
                    /* script_s5 has quotes, so get rid of them */
                    stripquotes(script_s5,script_s6);
                    gfx->object[i].sourceno=gfx->loadanm(script_s6);
                    ok=1;
                }
            }
            if (!ok) {
                /* no known object type. we cant assign this, so quit */
                sprintf(script_errmsg,"what kind of object type is '%s'?\n",script_s4);
                return SCRIPT_ERROR;
            }
        }
        if (!strcmp(script_s1,SCRIPT_LOADBACKGROUND)) {
            /* user wants to load background picture in [script_s2] */
            /* first remove the quotes left and right of the script_s2 */
            stripquotes(script_s2,script_s6);
            /* and load it */
            gfx->loadbackground(script_s6);
            ok=1;
        }
        if (!strcmp(script_s1,SCRIPT_LOADPRIORITY)) {
            /* user wants to load a new priority screen. let's do it! */
            /* first remove the quotes left and right of the script_s2 */
            stripquotes(script_s2,script_s6);
            /* and load it */
            gfx->loadpriorityscreen(script_s6);
            ok=1;
        }
        if (!strcmp(script_s1,SCRIPT_LOADMASK)) {
            /* user wants to load a new mask screen. let's do it! */
            /* first remove the quotes left and right of the script_s2 */
            stripquotes(script_s2,script_s6);
            /* and load it */
            gfx->loadmaskscreen(script_s6);
            ok=1;
        }
        if (!strcmp(script_s1,SCRIPT_RETURN)) {
            /* user wants to exit this script. lets do it */
            return SCRIPT_LEFT;
        }
        if (!strcmp(script_s1,SCRIPT_LEAVE)) {
            /* user wishes to leave. is this the main script? */
            if(this==mainscript) {
                /* we are. report and die */
                sprintf(script_errmsg,"can not leave the main script");
                return SCRIPT_ERROR;
            }
            script->running=0;
            return SCRIPT_LEFT;
        }
        if (!strcmp(script_s1,SCRIPT_WHILE)) {
            /* user wishes to loop while something is true */
            /* copy everything behind parenthese to script_method */
            ok=0;
            for(i=(strchr(script_line,')')-script_line)+1;i<strlen(script_line);i++) {
                script_method[ok]=script_line[i];
                script_method[ok+1]=0;
                ok++;
            }
            ok=processexpression(script_s2);
            /* was the condition true? */
            if (ok) {
                /* yes, so execute script_method */
                i=goline(script_method,script_errmsg,NULL);
                if(i!=SCRIPT_OK) return i;

                *line--;
                return SCRIPT_SETLINE;
            }
            /* no, so just continue */
            ok=1;
        }        
        /* add new things here
           like:

           if(!strcmp(script_s1,SCRIPT_DUMMY)) {
               // user wants to call the dummy function. let's do it!

               ...code...
           }
        */
        if ((ptr=strchr(script_s1,'='))!=NULL) {
            /* script_object is the value name before the equal sign */
             for (i=0;i<ptr-script_s1;i++) {
                  script_object[i]=script_s1[i];
                  script_object[i+1]=0;
             }
             /* script_temp is the number after the equal sign */
             for (i=0;i<(strlen(script_s1)-(ptr-script_s1));i++) {
                  script_temp[i]=script_s1[i+1+ptr-script_s1];
                  script_temp[i+1]=0;
             }
             /* was this variable ever declared in this script? */
             if ((i=findvariable(script_object))!=SCRIPT_NOIDENT) {
                 /* yeah, try to resolve this */
                 if((variable[i].value=resolvexpression(script_temp))==SCRIPT_NORESOLV) {
                     /* we cant. bomb out */
                     sprintf(script_errmsg,"cannot resolve '%s'\n",script_temp);
                     return SCRIPT_ERROR;
                 }
             } else {
                 /* maybe in the main script? */
                 if ((i=mainscript->findvariable(script_object))==SCRIPT_NOIDENT) {
                     /* no, so bomb out */
                     sprintf(script_errmsg,"variable %s not defined\n",script_object);
                     return SCRIPT_ERROR;
                 }
                 /* yeah, try to resolve this */
                 if((mainscript->variable[i].value=resolvexpression(script_temp))==SCRIPT_NORESOLV) {
                     /* we cant. bomb out */
                     sprintf(script_errmsg,"cannot resolve '%s'\n",script_temp);
                     return SCRIPT_ERROR;
                 }
             }
             ok=1;
        }
        if (!strcmp(script_s1,SCRIPT_DELAY)) {
            /* user wishes to delay for a period of time */
            delay_ms(atol(script_s2));
            ok=1;
        }
        /* add new commands here */
        /* if line is empty, say it's ok */
        if (!strcmp(script_line,"")) ok=1;
        /* if line was commented, say it's ok */
        if (script_s1[0]==SCRIPT_COMMENTCHAR) ok=1;
        /* if this was a procedure start marker, say it's ok */
        if (!strcmp(script_s2,SCRIPT_STARTPROC)) ok=1;
        /* if this was a procedure end marker, say it's ok */
        if (!strcmp(script_s1,SCRIPT_ENDPROC)) ok=1;
        /* was the event processed? */
        if ((!ok)&&(strchr(script_s1,':')==NULL)) {
            /* no, so bomb out */
            sprintf(script_errmsg,"unknown command '%s'\n",script_line);
            return SCRIPT_ERROR;
        }
        return SCRIPT_OK;
}

void
SCRIPT::goproc(char *procname) {
        UINT procno,quit,i;
        ULONG line;
        char script_line[SCRIPT_MAXLINESIZE];
        char script_temp[SCRIPT_MAXLINESIZE];

        /* search the procedure */
        if ((procno=findproc(procname))==SCRIPT_NOIDENT) {
                /* procedure was not found, so quit */
                sprintf(script_temp,"script_goproc(): procedure '%s' not found\n",procname);
                die(script_temp);
        }

        /* set the right line number */
        line=proc[procno].startline+1;

        quit=0;
        while(!quit) {
            /* try to read a line (shouldn't fail) */
            if (!readline(line,script_line)) {
                /* we were unable to read the line. bomb out with an error */
                sprintf(script_temp,"script_goproc(%s): unable to read line %lu\n",procname,line);
                die(script_temp);
            }
            /* handle the script line */
            if ((i=goline(script_line,script_temp,&line))==SCRIPT_ERROR) {
                /* an error happend. report and die */
                sprintf(script_line,"script_goproc(%s)[%lu]: %s\n",procname,(line+1),script_temp);
                die(script_line);
            }
            /* did the goline function tell us to quit? */
            if (i==SCRIPT_QUIT) {
                /* yes, so die */
                mad_exit();
            }
            /* did the script itself want to end? */
            if (i==SCRIPT_LEFT) {
                /* yes, just return */
                quit=1;
            }
            /* only advance line if not SCRIPT_SETLINE returned */
            if (i!=SCRIPT_SETLINE) {
                /* next line */
                line++;
            }
            gfx->object[GFX_MOUSE_SPRITENO].x=controls->getxpos();
            gfx->object[GFX_MOUSE_SPRITENO].y=controls->getypos();
            /* show mouse pointer if mouse object is enabled, hide otherwise */
            gfx->object[GFX_MOUSE_SPRITENO].visible=gfx->object[GFX_MOUSE_SPRITENO].enabled;
            /* handle the events of the bar, if it is enabled */
            if (gfx->object[GFX_BAR_SPRITENO].enabled) { bar->handlevent(); }
            /* check if button 1 is pushed. if so, check for collisions */
            if (controls->button1pressed()) {
                /* only check if bar icon has no special flags */
                if(!bar->getflags(bar->getselno())) {
                    /* scan all sprites for a collision with the mouse pointer */
                    for (i=0;i<GFX_MAXOBJECTS;i++) {
                        /* only check non-internal sprites and enabled sprites */
                        if ((!gfx->object[i].internal)&&(gfx->object[i].enabled)) {
                            if(gfx->objectscollide(GFX_MOUSE_SPRITENO,i)) {
                                /* wait until user stops clicking */
                                controls->waitnobutton();
                                /* initialize the procedure name */
                                /* if the bar is enabled add <bar_method>
                                   and seperator bar to the object name */
                                if(gfx->object[GFX_BAR_SPRITENO].enabled) {
                                    strcpy(script_temp,bar->getname(bar->getselno()));
                                    strcat(script_temp,SCRIPT_EVENTSEPERATOR);
                                    strcat(script_temp,gfx->object[i].name);
                                } else {
                                    /* otherwise just copy the object name */
                                    strcpy(script_temp,gfx->object[i].name);
                                }
                                /* call the procedure */
                                goproc(script_temp);
                            }
                        }
                    }
                }
                /* check if bit 2 is set (move ego object to) */
                if((bar->getflags(bar->getselno())&BAR_MOVETO)&&
                  (!bar->mouseinreach())&&
                  (gfx->object[GFX_CONTROLS_SPRITENO].enabled)) {
                    /* check if the ego object exists */
                    if ((i=gfx->findobject(BAR_EGONAME))==GFX_NOBJECT) {
                        /* no, so die with an error */
                        sprintf(script_temp,"script_goproc(%s): cannot find ego object",procname);
                        die(script_temp);
                    }
                    gfx->changecoords((gfx->object[GFX_MOUSE_SPRITENO].x-(gfx->getspritewidth(gfx->object[GFX_MOUSE_SPRITENO].sourceno))),
                                       gfx->object[GFX_MOUSE_SPRITENO].y-(gfx->getspriteheight(gfx->object[GFX_MOUSE_SPRITENO].sourceno)),i);
                    gfx->object[i].move=1;
                 }
            }
            /* if controls have anything to report, quit */
            if (controls->poll()) {
                quit=1;
            }
            /* scan if we have passed the last line of the script file */
            if (line>=proc[procno].endline) {
                /* yes, so exit this procedure */
                quit=1;
            }
        }
}

void
SCRIPT::done() {
    UINT i;

    if (scriptdata!=NULL) { free((void *)scriptdata); }
    noflabels=0;nofprocs=0;nofvars=0;
}

UINT
SCRIPT::resolvexpression(char *expr) {
    UINT no,i;
    UCHAR ok;
    char script_object[SCRIPT_MAXIDENTLEN];
    char script_value[SCRIPT_MAXIDENTLEN];
    char tempstr[MAD_TEMPSTR_SIZE];
    char *ptr;

    /* check if there is an dot in it */
    if((ptr=strchr(expr,'.'))!=NULL) {
        /* it got a dot. copy everthing before the dot to [script_object] */
        for (i=0;i<ptr-expr;i++) {
            script_object[i]=expr[i];
            script_object[i+1]=0;
        }
        /* [script_value] is the method after the dot */
        for (i=0;i<(strlen(expr)-(ptr-expr))-1;i++) {
            script_value[i]=expr[i+1+ptr-expr];
            script_value[i+1]=0;
        }
        /* try to find the object */
        if((no=gfx->findobject(script_object))==GFX_NOBJECT) {
            /* we cant. return SCRIPT_NORESOLV */
            return SCRIPT_NORESOLV;
        }
        ok=0;
        if(!strcmp(script_value,SCRIPT_EXPR_X)) {
            /* user wants to know the x coordinate of the object. give it */
            no=gfx->object[no].x;
            ok=1;
        }
        if(!strcmp(script_value,SCRIPT_EXPR_Y)) {
            /* user wants to know the y coordinate of the object. give it */
            no=gfx->object[no].y;
            ok=1;
        }
        if(!strcmp(script_value,SCRIPT_EXPR_PRIORITYCODE)) {
            /* user wants to know the priority code of the object. give it */
            no=gfx->object[no].prioritycode;
            ok=1;
        }
        if((gfx->object[no].type==GFX_TYPE_ANM)&&(!ok)) {
            /* it is an animation object. also check animation stuff */
            if(!strcmp(script_value,SCRIPT_EXPR_FRAMENO)) {
                /* user wants to know the frame number of the object. give it */
                no=gfx->object[no].frameno;
                ok=1;
            }
            if(!strcmp(script_value,SCRIPT_EXPR_MOVING)) {
                /* user wants to know the if the object is moving. tell him */
                no=gfx->object[no].move;
                ok=1;
            }
            if(!strcmp(script_value,SCRIPT_EXPR_ANIMATING)) {
                /* user wants to know the if the object is animating. tell him */
                no=gfx->object[no].animating;
                ok=1;
            }
        }
        /* did we process this? */
        if(!ok) {
            /* no, so return SCRIPT_NORESOLV */
            return SCRIPT_NORESOLV;
        }
        /* return the value */
        return no;
    }
    /* try to find the variable */
    if ((no=findvariable(expr))!=SCRIPT_NOIDENT) {
        /* wow, we found a variable! return the value of this one */
        return variable[no].value;
    }
    if ((no=mainscript->findvariable(expr))!=SCRIPT_NOIDENT) {
        /* wow, we found a variable! return the value of this one */
        return mainscript->variable[no].value;
    }
    /* try to convert it to a value */
    no=strtoul(expr,&ptr,0);
    if(ptr==expr) {
        /* we cannot resolv this. return SCRIPT_NORESOLV */
        return SCRIPT_NORESOLV;
    }
    return no;
}

UCHAR
SCRIPT::processexpression(char *expression) {
    UINT i,j,pos,ok,c,val1,val2;
    char expr1name[SCRIPT_MAXLINESIZE];
    char expr2name[SCRIPT_MAXLINESIZE];
    char tempstr[MAD_TEMPSTR_SIZE];
    char *ptr;

    /* search for an operator */
    j=SCRIPT_NOCHECK; pos=0; ok=0; c=strchr(expression,')')-expression-1;
    ptr=strchr(expression,'=');
    if (ptr!=NULL) {
        /* we found an equal sign. it the next char also an
           also an equal sign? */
        i=(ptr-expression);
        if (expression[i+1]=='=') {
            /* it is an check for equal sign. */
            j=SCRIPT_CHECKEQUAL;
            pos=i+2;
            c=i-1;
        } else {
            /* no, maybe the previous char is an exclamation
               mark. */
            if (expression[i-1]=='!') {
                /* it is! to the user wants to check if it
                   is not equal to something */
                j=SCRIPT_CHECKNOTEQUAL;
                pos=i+1;
                c=i-2;
            }
        }
    } else {
        /* is it greater than? */
        if((ptr=strchr(expression,'>'))!=NULL) {
            /* it is! user wants to check if something is greater than
               something */
            j=SCRIPT_CHECKGREATER;
            i=(ptr-expression);
            pos=i+1;
            c=i-1;
        }
        /* is it smaller than? */
        if((ptr=strchr(expression,'<'))!=NULL) {
            /* it is! user wants to check if something is smaller than
               something */
            j=SCRIPT_CHECKSMALLER;
            i=(ptr-expression);
            pos=i+1;
            c=i-1;
        }
    }
    /* check for a parenthese open */
    if ((ptr=strchr(expression,'('))==NULL) {
        /* not found, so bomb out */
        sprintf(tempstr,"no parenthese open found in '%s'\n",expression);
        die(tempstr);
    }
    ok=0;
    for(i=(ptr-expression)+1;i<(c+1);i++) {
        expr1name[ok]=expression[i];
        expr1name[ok+1]=0;
        ok++;
    }
    /* check for a parenthese close */
    if ((ptr=strchr(expression,')'))==NULL) {
        /* not found, so bomb out */
        sprintf(tempstr,"no parenthese close found in '%s'\n",expression);
        die(tempstr);
    }
    /* try to resolve the first part */
    if ((val1=resolvexpression(expr1name))==SCRIPT_NORESOLV) {
        /* it wasnt resolved. report and die */
        sprintf(tempstr,"cannot resolve expression 1 '%s'\n",expr1name);
        die(tempstr);
    }
    /* was an operator found? */
    if (j==SCRIPT_NOCHECK) {
        /* no, so go to a default of != 0 */
        val2=0;
        j=SCRIPT_CHECKNOTEQUAL;
    } else {
        /* copy everything at [pos] and past it until a parenthese to
           expr2name */
        ok=0;
        for(i=pos;i<(ptr-expression);i++) {
            expr2name[ok]=expression[i];
            expr2name[ok+1]=0;
            ok++;
        }
        /* try to resolve the second part */
        if ((val2=resolvexpression(expr2name))==SCRIPT_NORESOLV) {
            /* it wasnt resolved. report and die */
            sprintf(tempstr,"cannot resolve expression 2 '%s'\n",expr2name);
            die(tempstr);
        }
    }
    switch(j) {
          case SCRIPT_CHECKEQUAL: if (val1==val2) { ok=1; } else { ok=0; }; break;
       case SCRIPT_CHECKNOTEQUAL: if (val1!=val2) { ok=1; } else { ok=0; }; break;
        case SCRIPT_CHECKGREATER: if (val1>val2) { ok=1; } else { ok=0; }; break;
        case SCRIPT_CHECKSMALLER: if (val1<val2) { ok=1; } else { ok=0; }; break;
                         default: /* we dont know what to check, so die (shouldnt happend) */
                                  sprintf(tempstr,"unknown check value '%i'\n",j);
                                  die(tempstr);
    }
   return ok;
}
