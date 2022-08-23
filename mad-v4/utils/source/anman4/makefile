.SUFFIXES       : .c .o .cpp .s

TARGET          : anman4
FILES		= main.cpp
OBJS            = main.o

CFLAGS		+= -I../../../source/include
COMPILE         = gcc $(CFLAGS) -c -O3

anman4:		$(OBJS)
		g++ -o anman4 $(OBJS)

%.o		: %.cpp
		$(COMPILE) $<
		
clean:
		rm -f anman4 *.o
		
release:	clean
release:	anman
