#
# Font Manager 2.0 makefile
#
# 19991111	rink		made this file
# 20070120	rink		fixed for GCC3+
# 20080502	rink		fixed for GCC4+
#
.SUFFIXES       : .cpp

DEFAULT		= debug
TARGET          : $(DEFAULT)

PLATFORM	= DOS

OBJS            = main.o font.o

CFLAGS          = -O -g -D${PLATFORM}
COMPILE         = gcc $(CFLAGS) -c

fntman2     : $(OBJS)
		g++ -o fntman2 $(OBJS)
		
.cpp.o		: 
		$(COMPILE) $<
		
clean:		
		rm -f fntman2 $(OBJS)
		

debug:		CFLAGS += -DDEBUG
debug:	        fntman2

rebuild:	clean
rebuild:	$(TARGET)
