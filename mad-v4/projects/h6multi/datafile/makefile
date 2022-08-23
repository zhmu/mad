LANG=		english

data.mad:	anm m fnt project.prj text.txf inv.inv
		../../../utils/mfile2 -n data.mad < mfile.in

project.prj:	project.in
		../../../utils/prjedit -n project.prj < project.in

text.txf:	text.in
		../../../utils/txfgen text.in

inv.inv:	../../h6pool/datafile/invman.in
		../../../utils/invman ../../h6pool/datafile/invman.in inv.inv

anm:
		cd ../../h6pool/anm && $(MAKE)

m:
		cd ../scripts && $(MAKE)

fnt:
		cd ../../h6pool/font && $(MAKE)

clean:
		rm -f data.mad project.prj text.txf inv.inv sysfnt.fnt
		cd ../../h6pool/anm && $(MAKE) clean
		cd ../scripts && $(MAKE) clean
		cd ../../h6pool/font && $(MAKE) clean
