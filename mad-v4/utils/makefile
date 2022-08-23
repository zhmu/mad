UTILS =		anman4 anmtst4 fntman3 invman mc4 mfile2 ml4 picgen3 prjedit \
		stripal txfgen flic2spr

all:
		for i in $(UTILS); do \
			cd source/$$i && $(MAKE) && cp $$i ../../ && cd ../..; \
		done

objclean:
		for i in $(UTILS); do \
			cd source/$$i && $(MAKE) clean && cd ../..; \
		done

clean:		objclean
		rm -f $(UTILS)

