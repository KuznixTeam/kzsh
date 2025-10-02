AC_INIT([ksh], [0.1], [youremail@example.com])
AM_INIT_AUTOMAKE([foreign])
AC_PROG_CC
AC_PROG_CXX
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AC_PREFIX_DEFAULT(/usr/local)
AC_ARG_PROGRAM
## Build metadata: version, date, target
KSH_VERSION=[0.1]
AC_SUBST([KSH_VERSION])
AC_DEFINE([KSH_VERSION], ["${KSH_VERSION}"], [KSH version])
KSH_BUILD_DATE=`date +%Y-%m-%d`
AC_SUBST([KSH_BUILD_DATE])
AC_DEFINE([KSH_BUILD_DATE], ["${KSH_BUILD_DATE}"], [KSH build date])
# Determine a simple target string
KSH_TARGET="`uname -m`-`uname -s`"
AC_SUBST([KSH_TARGET])
AC_DEFINE([KSH_TARGET], ["${KSH_TARGET}"], [KSH build target])
AC_OUTPUT
