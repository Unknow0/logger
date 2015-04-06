EXTRA_DIST=README.md

lib_LTLIBRARIES=liblogger.la
liblogger_la_SOURCES=src/logger.c

include_HEADERS=src/logger.h

EXTRA_PROGRAMS=test
test_SOURCES=src/main.c
test_LDADD=liblogger.la
test_LDFLAGS=-pthread

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA=logger.pc
