.SUFFIXES       : .c .o .cpp .s

TARGET          : mfile
FILES		= main.cpp archive.cpp
OBJS            = main.o archive.o

CFLAGS		+= -I../../../source -DARCHIVE_MFILE
COMPILE         = gcc $(CFLAGS) -c -O3

mfile		: $(OBJS)
		g++ -o mfile $(OBJS)

%.o		: %.cpp
		$(COMPILE) $<
		
clean:		
		rm -f mfile $(OBJS)
