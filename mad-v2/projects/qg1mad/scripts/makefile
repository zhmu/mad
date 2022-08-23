SCRIPTS		= b_healer.lm castle.lm centry.lm frontc.lm stables.lm \
		  startup.lm healer.lm fields.lm btown.lm town.lm road1.lm \
		  forest1.lm cemetary.lm road2.lm road3.lm forest2.lm \
		  forest3.lm forest4.lm forest5.lm forest6.lm forest7.lm \
		  snowy.lm forest8.lm archery.lm forest9.lm forest10.lm \
		  forest11.lm

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
