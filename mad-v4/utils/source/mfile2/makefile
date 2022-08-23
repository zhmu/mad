.SUFFIXES       : .cpp

DEFAULT		= mfile2
TARGET          : $(DEFAULT)

PLATFORM	= UNIX

OBJS            = main.o archive.o

CFLAGS          = -O -g -D${PLATFORM} -DMFILE
COMPILE         = gcc $(CFLAGS) -c

mfile2:		$(OBJS)
		g++ -o mfile2 $(OBJS) -lz
		
.cpp.o		: 
		$(COMPILE) $<
		
clean:		
		rm -f mfile2 *.o

rebuild:	clean
rebuild:	$(TARGET)
