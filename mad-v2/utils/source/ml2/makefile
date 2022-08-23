.SUFFIXES       : .c .o .cpp .s

TARGET          : ml2
FILES		= main.cpp gadgets.cpp
OBJS            = main.o gadgets.o

CFLAGS		+= -O3
COMPILE         = gcc $(CFLAGS) -c -DUNIX

ml2    		: $(OBJS)
		g++ -o ml2 $(OBJS)

%.o		: %.cpp
		$(COMPILE) $<
		
clean:		
		rm -f ml2 $(OBJS)
