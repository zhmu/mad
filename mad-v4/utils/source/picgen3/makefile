.SUFFIXES       : .c .o .cpp .s

TARGET          : picgen3
FILES		= main.cpp
OBJS            = main.o

CFLAGS		+= -O3
COMPILE         = gcc -g $(CFLAGS) -c

picgen3:	$(OBJS)
		g++ -o picgen3 $(OBJS)

%.o		: %.cpp
		$(COMPILE) $<
		
clean:		
		rm -f picgen3 *.o
