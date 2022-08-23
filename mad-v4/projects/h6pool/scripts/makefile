SCRIPTS		= gl_dial.cm pool.cm startup.cm title.cm

MC		= ../../../utils/mc4
ML		= ../../../utils/ml4

all	        : $(SCRIPTS)
		$(ML) startup

%.cm		: %.s
		$(MC) $<
		
clean:		
		rm -f *.cm *.m

rebuild:	clean
rebuild:	all
