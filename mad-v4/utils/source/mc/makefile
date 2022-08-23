.SUFFIXES       : .c .o .cpp .s

TARGET          : mc.exe
FILES		= main.cpp gadgets.cpp
OBJS            = main.o gadgets.o

CFLAGS		+=
COMPILE         = gcc $(CFLAGS) -c -O3

mc.exe		: $(OBJS)
		g++ -o mc $(OBJS)

%.o		: %.cpp
		$(COMPILE) $<
		
clean:		
		rm -f mc *.o
