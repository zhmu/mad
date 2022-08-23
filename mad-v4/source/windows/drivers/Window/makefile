#
# Makefile for the WINDOW.DLL graphics driver
#
# 20000619	rink		created this file
#

DEFAULT         : window.dll

OBJS		= window.o

CFLAGS		= -g -O1 -I../../../include

COMPILE		= gcc $(CFLAGS) -c -DWINDOWS

window.dll:	window.o
		dllwrap --export-all-symbols -o window.dll window.o -lgdi32

window.o:	window.cpp
		$(COMPILE) $<

clean:
		rm -f *.o *.core mad
