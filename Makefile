all: adsd

adsd: adsd.o smpl.o rand.o
	$(LINK.c) -o $@ -Bstatic adsd.o smpl.o rand.o -lm

smpl.o: smpl.c smpl.h
	$(COMPILE.c)  -g smpl.c

adsd.o: adsd.c smpl.h
	$(COMPILE.c) -g  adsd.c

rand.o: rand.c
	$(COMPILE.c) -g rand.c

clean:
	$(RM) *.o adsd relat saida

