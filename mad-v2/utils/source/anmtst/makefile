.SUFFIXES       : .c .o .cpp .s

TARGET          : anmtst.exe
FILES		= anmtst.cpp
OBJS            = anmtst.o

CFLAGS		+=
COMPILE         = gcc $(CFLAGS) -c -O3

anmtst.exe      : $(OBJS)
		gxx -o anmtst.exe $(OBJS)
		strip anmtst.exe
		djp anmtst.exe
		move anmtst.exe ...

%.o		: %.cpp
		$(COMPILE) $<
		
clean:		
		del anmtst.exe
		del *.o
		
release:	clean
release:	anmtst.exe
