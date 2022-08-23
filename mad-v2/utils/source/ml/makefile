.SUFFIXES       : .c .o .cpp .s

TARGET          : ml
FILES		= main.cpp gadgets.cpp
OBJS            = main.o gadgets.o

CFLAGS		+= -O3
COMPILE         = gcc $(CFLAGS) -c

ml    		: $(OBJS)
		g++ -o ml $(OBJS)

%.o		: %.cpp
		$(COMPILE) $<
		
clean:		
		rm -f ml $(OBJS)
