UTILS =		anman fontman fntman2 invman mc mc2 mc3 mc4 mfile ml2 picgen \
		prjedit stripal txfgen

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

