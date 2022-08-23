.SUFFIXES       : .c .o .cpp .s

TARGET          : anman
FILES		= main.cpp anm.cpp
OBJS            = anm.o main.o

CFLAGS		+= -DANMAN -I../../../source
COMPILE         = gcc $(CFLAGS) -c -O3

anman: 		$(OBJS)
		g++ -o anman $(OBJS)

%.o		: %.cpp
		$(COMPILE) $<
		
clean:
		rm -f anman $(OBJS)
		
release:	clean anman
