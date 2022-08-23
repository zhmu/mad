ALL =		d_but dhero gleaf hero poolstr tears

all:
		for i in $(ALL); do \
			cd $$i && ../../../../utils/anman4 $$i.asr ../$$i.anm && cd ..; \
		done

clean:
		for i in $(ALL); do \
			rm -f $$i.anm; \
		done
