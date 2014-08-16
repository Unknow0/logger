EXTRA_DIST=README.md

lib_LTLIBRARIES=liblogger.la
liblogger_la_SOURCES=src/logger.c

include_HEADERS=src/logger.h

EXTRA_PROGRAMS=test
test_SOURCES=src/main.c
test_LDADD=liblogger.la

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA=logger.pc

distclean-local:
	-rm -rf Makefile.in aclocal.m4 autom4te.cache/ config.guess config.sub depcomp install-sh ltmain.sh missing
