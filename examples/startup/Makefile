.PHONY: clean

startup: startup.C
	gcc startup.C -o startup -I../.. -L../.. -lreconverse -Wl,-rpath,../..

clean: 
	rm -rf startup
