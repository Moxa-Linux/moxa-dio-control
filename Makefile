FNAME=mx-dio-ctl
#CC=/usr/libexec/icecc/bin/gcc
#CC=arm-linux-gnueabihf-gcc
#STRIP=arm-linux-gnueabihf-strip
#CC=gcc
release:
	$(CC) $(CFLAGS) $(LDFLAGS) $(FNAME).c dio.c -DDEBUG -L. -lpthread -ljson-c -o $(FNAME)

clean:
	/bin/rm -f $(FNAME) $(FNAME)-dbg *.o
