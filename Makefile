all: vcube

vcube: vcube.o smpl.o rand.o
	$(LINK.c) -o $@ -Bstatic vcube.o smpl.o rand.o -lm

smpl.o: smpl.c smpl.h
	$(COMPILE.c)  -g smpl.c

vcube.o: vcube.c smpl.h cisj.o
	$(COMPILE.c) -g vcube.c cisj.o

cisj.o: cisj.c
	$(COMPILE.c) -g cisj.c

rand.o: rand.c
	$(COMPILE.c) -g rand.c

clean:
	$(RM) *.o vscube relat saida

