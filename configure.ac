##
## This file is part of the libsigrokflow project.
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

# We require at least autoconf 2.63 (AC_INIT format changed there).
AC_PREREQ([2.63])

AC_INIT([libsigrokflow], [0.1.0],
	[sigrok-devel@lists.sourceforge.net],
	[libsigrokflow], [http://www.sigrok.org])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([autostuff])
AC_CONFIG_HEADERS([config.h])

# We require at least automake 1.11 (needed for 'silent rules').
AM_INIT_AUTOMAKE([1.11 -Wall -Werror no-define nostdinc subdir-objects check-news color-tests])
AM_SILENT_RULES([yes])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S

# Get compiler versions.
SR_PROG_VERSION([$CXX], [srf_cxx_version])

# Check for optional make features.
SR_PROG_MAKE_ORDER_ONLY

# Initialize libtool.
LT_INIT

# Set up the libsigrokflow version defines.
SR_PKG_VERSION_SET([SRF_PACKAGE_VERSION], [AC_PACKAGE_VERSION])

# Library version for libsigrokflow (NOT the same as the package version).
# Carefully read the libtool docs before updating these numbers!
# The algorithm for determining which number to change (and how) is nontrivial!
# http://www.gnu.org/software/libtool/manual/libtool.html#Updating-version-info
# Format: current:revision:age.
SR_LIB_VERSION_SET([SRF_LIB_VERSION], [0:0:0])

# Initialize pkg-config.
# We require at least 0.22, as "Requires.private" behaviour changed there.
PKG_PROG_PKG_CONFIG([0.22])

AC_LANG([C++])
SR_ARG_ENABLE_WARNINGS([SRF_WXXFLAGS], [-Wall], [-Wall -Wextra])

# Check if the C++ compiler supports the C++11 standard.
AX_CXX_COMPILE_STDCXX([11], [noext], [mandatory])

##############################
##  Finalize configuration  ##
##############################

AC_CONFIG_FILES([Makefile libsigrokflow.pc])

AC_OUTPUT

cat >&AS_MESSAGE_FD <<_EOF

libsigrokflow configuration summary:
 - Package version................. $SRF_PACKAGE_VERSION
 - Library ABI version............. $SRF_LIB_VERSION
 - Prefix.......................... $prefix
 - Building on..................... $build
 - Building for.................... $host
 - Building shared / static........ $enable_shared / $enable_static

Compile configuration:
 - C++ compiler.................... $CXX
 - C++ compiler version............ $srf_cxx_version
 - C++ compiler flags.............. $CXXFLAGS
 - C++ compiler warnings........... $SRF_WXXFLAGS
 - Linker flags.................... $LDFLAGS

_EOF
