AM_CFLAGS = -I$(top_srcdir)/include/
AM_CFLAGS += -Wall -Wextra -g
LDADD = $(top_builddir)/lib/libmx_dio_ctl.la -ljson-c -lpthread -lmx_gpio_ctl
sbin_PROGRAMS = mx-dio-ctl
mx_dio_ctl_SOURCES = mx-dio-ctl.c
