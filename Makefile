all:	case tracklen

case:	case.c models models/final.scad
	gcc -I/usr/local/include -L/usr/local/lib -O -o $@ $< -lpopt -lm -g

tracklen:	tracklen.c
	gcc -I/usr/local/include -L/usr/local/lib -O -o $@ $< -lpopt -lm -g
