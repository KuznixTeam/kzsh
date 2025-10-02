AC_INIT([kzsh], [0.1.1], [krzysztofdemirkuzniak@gmail.com])
AM_INIT_AUTOMAKE([foreign])
AC_PROG_CC
AC_PROG_CXX
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AC_PREFIX_DEFAULT(/usr/local)
AC_ARG_PROGRAM
## Build metadata: version, date, target
KZSH_VERSION=[0.1.1]
AC_SUBST([KZSH_VERSION])
AC_DEFINE([KZSH_VERSION], ["${KZSH_VERSION}"], [KZSH version])
KZSH_BUILD_DATE=`date +%Y-%m-%d`
AC_SUBST([KZSH_BUILD_DATE])
AC_DEFINE([KZSH_BUILD_DATE], ["${KZSH_BUILD_DATE}"], [KZSH build date])
# Copyright year (configure-time)
KZSH_COPYRIGHT_YEAR=`date +%Y`
AC_SUBST([KZSH_COPYRIGHT_YEAR])
AC_DEFINE([KZSH_COPYRIGHT_YEAR], ["${KZSH_COPYRIGHT_YEAR}"], [KZSH copyright year])
# Provide backward-compatible macro name KSH_COPYRIGHT_YEAR for sources that expect KSH_ prefix
AC_SUBST([KSH_COPYRIGHT_YEAR], [${KZSH_COPYRIGHT_YEAR}])
AC_DEFINE([KSH_COPYRIGHT_YEAR], ["${KZSH_COPYRIGHT_YEAR}"], [KSH copyright year])
# Determine a simple target string
KZSH_TARGET="`uname -m`-`uname -s`"
AC_SUBST([KZSH_TARGET])
AC_DEFINE([KZSH_TARGET], ["${KZSH_TARGET}"], [KZSH build target])
AC_OUTPUT
