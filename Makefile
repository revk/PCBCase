all:	clean case tracklen PCB/Test/Test.scad PCB/Test/Test1.kicad_pcb PCB/Test/Test2.kicad_pcb

case:	case.c pcb.c models case.scad
	gcc -O -o $@ $< pcb.c -lpopt -lm -g

clean:	clean.c pcb.c
	gcc -O -o $@ $< pcb.c -lpopt -lm -g

tracklen:	tracklen.c
	gcc -O -o $@ $< -lpopt -lm -g

PCB/Test/Test.scad:	PCB/Test/Test.kicad_pcb case Makefile
	./case -o $@ $< -M models

PCB/Test/Test1.kicad_pcb: PCB/Test/Test.kicad_pcb clean Makefile
	./clean -o $@ $<

PCB/Test/Test2.kicad_pcb: PCB/Test/Test.kicad_pcb clean Makefile
	./clean -o $@ $< --case=2
