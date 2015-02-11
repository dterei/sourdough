#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([sourdough], [0.01], [keithw@cs.stanford.edu])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([src/address.hh])
AC_CONFIG_HEADERS([config.h])

# Set default CXXFLAGS
DEF_CXXFLAGS="-std=c++11 -g -O2"
AC_SUBST([AM_CXXFLAGS], [DEF_CXXFLAGS])
# Add picky CXXFLAGS
PICKY_CXXFLAGS="$DEF_CXXFLAGS -pedantic -Wall -Wextra -Weffc++ -Werror"
AC_SUBST([PICKY_CXXFLAGS])

# Checks for programs.
AC_PROG_CXX
AC_PROG_RANLIB

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UINT16_T

# Checks for library functions.

AC_CONFIG_FILES([Makefile src/Makefile examples/Makefile datagrump/Makefile])
AC_OUTPUT
