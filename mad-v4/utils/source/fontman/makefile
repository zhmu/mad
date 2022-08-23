.SUFFIXES       : .c .o .cpp .s

TARGET          : fontman.exe
FILES		= main.cpp font.cpp
OBJS            = main.o font.o

CFLAGS		+= -O3
COMPILE         = gcc -g $(CFLAGS) -c

fontman.exe	: $(OBJS)
		g++ -g -o fontman $(OBJS)

%.o		: %.cpp
		$(COMPILE) $<
		
clean:		
		rm -f fontman *.o
