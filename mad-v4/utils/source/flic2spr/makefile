.SUFFIXES       : .c .o .cpp .s

TARGET          : flic2spr
FILES		= main.cpp
OBJS            = main.o

COMPILE         = gcc $(CFLAGS) -c -O3

flic2spr:	$(OBJS)
		g++ -o flic2spr $(OBJS)

%.o		: %.cpp
		$(COMPILE) $<
		
clean:
		rm -f flic2spr $(OBJS)
		
release:	clean flic2spr
