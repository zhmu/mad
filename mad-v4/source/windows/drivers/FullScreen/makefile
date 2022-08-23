#
# Makefile for the FSCREEN.DLL graphics driver
#
# 20000619	rink		created this file
# 20000725	rink		altered it for FSCREEN.DLL
#

DEFAULT         : fscreen.dll

OBJS		= fscreen.o

CFLAGS		= -O1 -I../../../include

COMPILE		= gcc $(CFLAGS) -c -DWINDOWS -fvtable-thunks

fscreen.dll:	fscreen.o
		dllwrap --export-all-symbols -o fscreen.dll fscreen.o -lgdi32 -lddraw -ldinput -ldxguid

fscreen.o:	fscreen.cpp
		$(COMPILE) $<

clean:
		rm -f *.o *.core *.dll
