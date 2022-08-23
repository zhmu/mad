#
# Makefile for MAD (UNIX version only)
#
# 19990610	WvE		created
# 19991118	rink		updated
# 19991119 	rink		edited it so it will make MAD/X and MAL/VGL.
# 20070121	rink		fixed for GCC 3.3+
#

.SUFFIXES	: .cpp

XHOME		= /usr/X11R6
DEFAULT         : x

PLATFORM	= UNIX

OBJS		= main.o archive.o bar.o gadgets.o gfx.o project.o script.o \
		  anm.o debugmad.o fontman.o objman.o dlgman.o parser.o \
		  textman.o game.o controls.o inv.o

CFLAGS		= -D${PLATFORM} -O1 -I$(XHOME)/include
CFLAGS		+= -DDEBUG -g

COMPILE		= gcc $(CFLAGS) -c
LINK		= g++

x:		
		@ $(MAKE) $(OBJS) gfx_x.o ctrl_x.o CFLAGS='$(CFLAGS) -DX'
		$(LINK) -o mad $(OBJS) gfx_x.o ctrl_x.o -lX11 -L$(XHOME)/lib

vgl:		
		@ $(MAKE) $(OBJS) gfx_vgl.o ctrl_vgl.o CFLAGS='$(CFLAGS) -DVGL -DUNIX_R00T_REQUIRED'
		$(LINK) -o mad $(OBJS) gfx_vgl.o ctrl_vgl.o -lvgl

ggi:		
		@ $(MAKE) $(OBJS) gfx_ggi.o ctrl_gii.o CFLAGS='$(CFLAGS) -DGGI -I/usr/local/include'
		$(LINK) -o mad $(OBJS) gfx_ggi.o ctrl_gii.o -lggi -lgii -lgg -L/usr/local/lib -lpthread

.cpp.o :
		$(COMPILE) $<

clean:
		rm -f *.o *.core mad
