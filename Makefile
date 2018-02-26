LIB_NAME = mx_dio
SO_NAME = libmx_dio_ctl.so
V_MAJOR = 1
V_MINOR = 1
V_REVISION = 0
VER_CODE = $(V_MAJOR).$(V_MINOR).$(V_REVISION)

CROSS_COMPILE =
CC := $(CROSS_COMPILE)gcc
STRIP := $(CROSS_COMPILE)strip
CFLAGS =
LDFLAGS =

all:
	$(CC) $(CFLAGS) $(LDFLAGS) -Wall -fPIC -c -o $(LIB_NAME).o $(LIB_NAME).c

	# static library
	ar rcs $(LIB_NAME).a $(LIB_NAME).o
	$(STRIP) -s $(LIB_NAME).a

	# shared library
	$(CC) $(CFLAGS) $(LDFLAGS) -ljson-c -lpthread -lmx_gpio_ctl -shared -Wl,-soname,$(SO_NAME).$(V_MAJOR) -o $(SO_NAME).$(VER_CODE) $(LIB_NAME).o
	$(STRIP) -s $(SO_NAME).$(VER_CODE)
	ln -sf $(SO_NAME).$(VER_CODE) $(SO_NAME).$(V_MAJOR)
	ln -sf $(SO_NAME).$(V_MAJOR) $(SO_NAME)

	# utility
	$(CC) $(CFLAGS) $(LDFLAGS) -Wall -L. -lmx_dio_ctl -o mx-dio-ctl mx-dio-ctl.c
	$(STRIP) -s mx-dio-ctl

.PHONY: clean	
clean:
	rm -f *.o *.a $(SO_NAME)* mx-dio-ctl
