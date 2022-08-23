SCRIPTS		= startup.lm llyette.lm

MC		= ../../../utils/mc2
ML		= ../../../utils/ml2

all	        : $(SCRIPTS)
		$(ML)

%.lm		: %.scr
		$(MC) $<
		
clean:		
		rm -f *.cm *.lm

rebuild:	clean
rebuild:	all
