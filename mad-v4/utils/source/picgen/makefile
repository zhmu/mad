.SUFFIXES       : .c .o .cpp .s

TARGET          : picgen

CFLAGS		+= -I../../../source
COMPILE         = gcc $(CFLAGS) -c -O3

picgen:
		g++ -o picgen main.cpp $(CFLAGS)

%.o		: %.cpp
		$(COMPILE) $<
		
clean:
		rm -f picgen
		
release:	clean picgen
