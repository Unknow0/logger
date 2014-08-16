#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([logger], [1.0])
LT_INIT
AM_INIT_AUTOMAKE([foreign])

AC_CONFIG_SRCDIR([src/logger.c])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.
AC_CHECK_LIB([cfg], [cfg_init])
AC_CHECK_LIB([container], [chunked_list_create])
AC_CHECK_LIB([json-c], [json_object_new_object])

# Checks for header files.
AC_CHECK_HEADERS([langinfo.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_CHECK_FUNCS([localtime_r memset nl_langinfo strdup])

AC_CONFIG_FILES([
	Makefile
	logger.pc
	])
AC_OUTPUT
