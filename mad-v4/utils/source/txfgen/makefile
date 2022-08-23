.SUFFIXES       : .c .o .cpp .s

TARGET          : txfgen
FILES		= main.cpp gadgets.cpp
OBJS            = main.o gadgets.o

CFLAGS		+= -O3 -I../../../source
COMPILE         = gcc $(CFLAGS) -c

txfgen	        : $(OBJS)
		g++ -o txfgen $(OBJS)

%.o		: %.cpp
		$(COMPILE) $<
		
clean:		
		rm -f txfgen $(OBJS)	
