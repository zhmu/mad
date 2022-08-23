.SUFFIXES       : .c .o .cpp .s

TARGET          : anmtst4
FILES		= main.cpp
OBJS            = main.o

CFLAGS		= -DUNIX -I/usr/local/include
LIBS		= -L/usr/local/lib -lX11
COMPILE         = gcc $(CFLAGS) -c -O3 -I../../../source/include -I../../../source/unix/include

anmtst4:	$(OBJS)
		g++ -o anmtst4 $(OBJS) $(LIBS)

%.o		: %.cpp
		$(COMPILE) $<
		
clean:		
		rm -f anmtst4 *.o
		
release:	clean
release:	anmtst4
