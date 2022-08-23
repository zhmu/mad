TARGET          : prjedit
FILES		= main.cpp project.cpp
OBJS            = project.o main.o

CFLAGS		+= -DPROJECT_PRJEDIT -I../../../source
COMPILE         = gcc $(CFLAGS) -c -O3

prjedit:	$(OBJS)
		g++ -o prjedit $(OBJS)

%.o		: %.cpp
		$(COMPILE) $<
		
clean:
		rm -f prjedit $(OBJS)
		
release:	clean prjedit
