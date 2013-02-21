OBJS=			\
	cgavideo.o	\
	pcsound.o	\
	timer.o
CC=gcc
LD=ld
CFLAGS=-g -I.. -Wall 
LDFLAGS=
EXE=sopwith

$(EXE): $(OBJS) ../sopwith.o
	$(CC) $(CFLAGS) $(OBJS) ../sopwith.o -o $@ $(LDFLAGS)

../sopwith.o: 
	make -C .. sopwith.o
%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

depends:
	$(CC) $(CFLAGS) -MM *.c > depends

clean : 
	rm -f $(EXE)
	rm -f $(OBJS)
	rm -f *~	
	rm -f depends

include depends

