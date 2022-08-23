.SUFFIXES       : .c .o .cpp .s

TARGET          : stripal
FILES		= stripal.cpp
OBJS            = stripal.o

CFLAGS		+=
COMPILE         = gcc $(CFLAGS) -O3 -c

stripal         : $(OBJS)
		g++ -o stripal $(OBJS)

%.o		: %.cpp
		$(COMPILE) $<
		
clean:		
		rm -f stripal $(OBJS)	
