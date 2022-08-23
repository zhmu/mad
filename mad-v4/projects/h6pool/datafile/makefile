LANG=		english

data.mad:	anm fnt m project.prj text.txf inv.inv
		../../../utils/mfile2 -n data.mad < mfile.in

project.prj:	project.in
		../../../utils/prjedit -n project.prj < project.in

text.txf:	../text/$(LANG).in
		../../../utils/txfgen ../text/$(LANG).in

inv.inv:	invman.in
		../../../utils/invman invman.in inv.inv

anm:
		cd ../anm && $(MAKE)

m:
		cd ../scripts && $(MAKE)

fnt:
		cd ../font && $(MAKE)

clean:
		rm -f data.mad project.prj text.txf inv.inv sysfnt.fnt
		cd ../anm && $(MAKE) clean
		cd ../scripts && $(MAKE) clean
		cd ../font && $(MAKE) clean
