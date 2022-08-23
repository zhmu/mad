data.mad:	anm lm fnt project.prj text.txf inv.inv
		../../../utils/mfile -n data.mad < mfile.in

project.prj:	project.in
		../../../utils/prjedit -n project.prj < project.in

text.txf:	txfgen.in
		../../../utils/txfgen txfgen.in

inv.inv:	invman.in
		../../../utils/invman invman.in inv.inv

anm:
		cd ../anm && $(MAKE)

lm:
		cd ../scripts && $(MAKE)

fnt:
		cd ../gfx/sysfnt && $(MAKE)

clean:
		rm -f data.mad project.prj text.txf inv.inv sysfnt.fnt
		cd ../anm && $(MAKE) clean
		cd ../scripts && $(MAKE) clean
		cd ../gfx/sysfnt && $(MAKE) clean
