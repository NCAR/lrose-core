# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
# ** Copyright UCAR (c) 1992 - 2010 */
# ** University Corporation for Atmospheric Research(UCAR) */
# ** National Center for Atmospheric Research(NCAR) */
# ** Research Applications Laboratory(RAL) */
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA */
# ** 2010/10/7 23:12:36 */
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
###############################################
# template for configure
###############################################

AC_PREREQ([2.50])
AC_INIT([libRadx], [1.0], [http://www.ral.ucar.edu/projects/titan/docs])
AM_INIT_AUTOMAKE([-Wall foreign])
AC_CONFIG_SRCDIR([src/include/Radx/Radx.hh])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([sys/time.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

# Checks for library functions.
AC_HEADER_STDC
AC_FUNC_STAT
AC_FUNC_UTIME_NULL
AC_CHECK_FUNCS([floor gettimeofday memset modf pow strerror])

# create makefiles
AC_OUTPUT(makefile src/makefile)

