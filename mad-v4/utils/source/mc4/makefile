.SUFFIXES       : .c .o .cpp .s

TARGET          : mc4
FILES		= main.cpp compile.cpp gadgets.cpp
OBJS            = main.o compile.o gadgets.o

CFLAGS		+=
COMPILE         = gcc $(CFLAGS) -c -O3

mc4		: $(OBJS)
		g++ -o mc4 $(OBJS)

%.o		: %.cpp
		$(COMPILE) $<
		
clean:		
		rm -f mc4 $(OBJS)
