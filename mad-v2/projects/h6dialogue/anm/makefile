ALL =		llyette

all:
		for i in $(ALL); do \
			cd $$i && ../../../../utils/anman ../$$i.anm $$i.scr && cd ..; \
		done

clean:
		for i in $(ALL); do \
			rm -f $$i.anm; \
		done
