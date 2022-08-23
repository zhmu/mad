.SUFFIXES       : .c .o .cpp .s

TARGET          : ml4
FILES		= main.cpp gadgets.cpp
OBJS            = main.o gadgets.o

CFLAGS		+= -O3 -DUNIX
COMPILE         = gcc $(CFLAGS) -c

ml4:		$(OBJS)
		g++ -o ml4 $(OBJS)

%.o		: %.cpp
		$(COMPILE) $<
		
clean:		
		rm -f ml4 *.o
