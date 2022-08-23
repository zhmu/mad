.SUFFIXES       : .c .o .cpp .s

TARGET          : pcxgrab.exe
FILES		= main.cpp
OBJS            = main.o

CFLAGS		+=
COMPILE         = gcc $(CFLAGS) -O3 -c

pcxgrab.exe     : $(OBJS)
		gxx -o pcxgrab.exe $(OBJS)
		strip pcxgrab.exe
		djp pcxgrab.exe
		move pcxgrab.exe ...

%.o		: %.cpp
		$(COMPILE) $<
		
clean:		
		del pcxgrab.exe
		del *.o
