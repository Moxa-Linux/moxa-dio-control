CC ?= gcc

all:
	$(CC) -Wall -lpthread -lmx_dio_ctl dio-test.c -o dio-test

.PHONY: clean
clean:
	rm -f dio-test
