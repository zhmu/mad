.SUFFIXES       : .c .o .cpp .s

TARGET          : mc2
FILES		= main.cpp gadgets.cpp
OBJS            = main.o gadgets.o

CFLAGS		+=
COMPILE         = gcc $(CFLAGS) -c -O3

mc2		: $(OBJS)
		g++ -o mc2 $(OBJS)

%.o		: %.cpp
		$(COMPILE) $<
		
clean:		
		rm -f mc2 $(OBJS)
