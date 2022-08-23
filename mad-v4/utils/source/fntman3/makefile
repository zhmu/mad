#
# Font Manager 3.0 makefile
#
# 19991111	rink		made this file
# 20000724	rink		fixed it to work again
# 20080504	rink		various fixes to make it work
#
.SUFFIXES       : .cpp

DEFAULT		= fntman3
TARGET          : $(DEFAULT)

PLATFORM	= DOS

OBJS            = main.o font.o

CFLAGS          = -O -D${PLATFORM}
COMPILE         = gcc $(CFLAGS) -c

fntman3: 	$(OBJS)
		g++ -o fntman3 $(OBJS)
		
.cpp.o		: 
		$(COMPILE) $<
		
clean:		
		rm -f fntman3 *.o

rebuild:	clean
rebuild:	$(TARGET)
