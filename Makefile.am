##
## This file is part of the libsigrok project.
##
## Copyright (C) 2018 Uwe Hermann <uwe@hermann-uwe.de>
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.
##

ACLOCAL_AMFLAGS = -I m4
AM_LIBTOOLFLAGS = --silent
GNUMAKEFLAGS = --no-print-directory

AM_CPPFLAGS = -Iinclude -I$(srcdir)/include -I$(srcdir)/src -I.

AM_CXXFLAGS = $(SRF_WXXFLAGS)

lib_LTLIBRARIES = libsigrokflow.la

libsigrokflow_la_SOURCES = \
	src/main.cpp

libsigrokflow_la_LDFLAGS = -version-info $(SRF_LIB_VERSION) -no-undefined

library_includedir = $(includedir)/libsigrokflow
library_include_HEADERS = \
	include/libsigrokflow/libsigrokflow.hpp

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libsigrokflow.pc

.PHONY: dist-changelog

dist-hook: dist-changelog

dist-changelog:
	$(AM_V_at)if test ! -d '$(top_srcdir)/.git'; then \
		cp -f '$(top_srcdir)/ChangeLog' "$(top_distdir)/ChangeLog"; \
	elif git -C '$(top_srcdir)' log >.ChangeLog.tmp; then \
		mv -f .ChangeLog.tmp "$(top_distdir)/ChangeLog"; \
	else \
		rm -f .ChangeLog.tmp; exit 1; \
	fi
