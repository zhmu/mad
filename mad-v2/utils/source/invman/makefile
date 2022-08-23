.SUFFIXES       : .c .o .cpp .s

TARGET          : invman.exe
FILES		= main.cpp
OBJS            = main.o

CFLAGS		+= -O3
COMPILE         = gcc $(CFLAGS) -c

invman.exe	: $(OBJS)
		g++ -o invman $(OBJS)

%.o		: %.cpp
		$(COMPILE) $<
		
clean:		
		rm -f invman *.o
