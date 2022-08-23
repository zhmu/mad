SCRIPTS		= 154.cm 169.cm startup.cm

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
