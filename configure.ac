# moxa-dio-control version
m4_define([version_major], [1])
m4_define([version_minor], [1])
m4_define([version_revision], [3])
m4_define([version_code], [version_major.version_minor.version_revision])

# autoconf, automake, libtool initial
AC_PREREQ(2.69)
AC_INIT([moxa-dio-control], [version_code])
AM_INIT_AUTOMAKE

AM_SILENT_RULES([yes])

AC_PROG_CC
AC_PROG_MKDIR_P
AC_PROG_INSTALL

LT_PREREQ(2.4)
LT_INIT([disable-static])

VERSION_CODE=version_code
AC_SUBST(VERSION_CODE)

# check libraries & headers
AC_DEFUN([FUNC_NOT_FOUND_LIB],
	[AC_MSG_ERROR([library $1 not found])])
AC_DEFUN([HEADER_NOT_FOUND_LIB],
	[AC_MSG_ERROR([header $1 not found])])

AC_CHECK_HEADERS([stdio.h], [], [HEADER_NOT_FOUND_LIB([stdio.h])])
AC_CHECK_HEADERS([unistd.h], [], [HEADER_NOT_FOUND_LIB([unistd.h])])
AC_CHECK_HEADERS([string.h], [], [HEADER_NOT_FOUND_LIB([string.h])])
AC_CHECK_HEADERS([fcntl.h], [], [HEADER_NOT_FOUND_LIB([fcntl.h])])
AC_CHECK_HEADERS([pthread.h], [], [HEADER_NOT_FOUND_LIB([pthread.h])])
AC_CHECK_HEADERS([sys/time.h], [], [HEADER_NOT_FOUND_LIB([sys/time.h])])
AC_CHECK_HEADERS([sys/file.h], [], [HEADER_NOT_FOUND_LIB([sys/file.h])])
AC_CHECK_HEADERS([sys/ioctl.h], [], [HEADER_NOT_FOUND_LIB([sys/ioctl.h])])
AC_CHECK_HEADERS([json-c/json.h], [], [HEADER_NOT_FOUND_LIB([json-c/json.h])])
AC_CHECK_HEADERS([moxa/mx_gpio.h], [], [HEADER_NOT_FOUND_LIB([moxa/mx_gpio.h])])

AC_CHECK_LIB(json-c, json_object_from_file, [], [FUNC_NOT_FOUND_LIB([json-c])])
AC_CHECK_LIB(mx_gpio_ctl, mx_gpio_is_exported, [], [FUNC_NOT_FOUND_LIB([mx_gpio_ctl])])

# output
AC_CONFIG_FILES([
	include/Makefile
	lib/Makefile
	tools/Makefile
	Makefile
])
AC_OUTPUT
