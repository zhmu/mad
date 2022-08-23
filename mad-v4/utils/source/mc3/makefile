.SUFFIXES       : .c .o .cpp .s

TARGET          : mc3
FILES		= main.cpp compile.cpp gadgets.cpp
OBJS            = main.o compile.o gadgets.o

CFLAGS		+=
COMPILE         = gcc $(CFLAGS) -c -O3

mc3		: $(OBJS)
		g++ -o mc3 $(OBJS)

%.o		: %.cpp
		$(COMPILE) $<
		
clean:		
		rm -f mc3 $(OBJS)
