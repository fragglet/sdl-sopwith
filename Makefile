CC=gcc
CFLAGS=`sdl-config --cflags` -I.
LDFLAGS=`sdl-config --libs`
EXE=sopwith
OBJS =               	\
	bmblib.o 	\
	pcsound.o	\
	timer.o		\
	cgavideo.o	\
	swasynio.o	\
	swauto.o	\
	swcollsn.o	\
	swdisp.o	\
	swend.o		\
	swgames.o	\
	swground.o	\
	swgrpha.o	\
	swinit.o	\
	swmain.o	\
	swmisc.o	\
	swmove.o	\
	swmultio.o	\
	swobject.o	\
	swplanes.o	\
	swsound.o	\
	swsymbol.o	\
	swtitle.o	\
	swutil.o

$(EXE): depends $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

depends:
	$(CC) $(CFLAGS) -MM *.c > depends
clean:
	rm -f $(EXE)
	rm -f *.o
	rm -f depends
	rm -f *~

include depends

