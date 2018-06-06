#  _ _ _
# | (_) |__ ___ ___ _ __  __ _   SEPA library · www.libsepa.com
# | | | '_ (_-</ -_) '_ \/ _` |  Copyright (c) 2013-2014 Keppler IT GmbH.
# |_|_|_.__/__/\___| .__/\__,_|_____________________________________________
#                  |_|
# $Id: config.m4 214 2014-04-13 08:41:57Z kk $

PHP_ARG_ENABLE(sepa, for SEPA support,
[  --enable-sepa           enable libsepa (SEPA XML library)])
 
if test "$PHP_SEPA" = "yes"; then
  AC_DEFINE(HAVE_SEPA, 1, [Whether you have sepa support])

  case "`uname -m`" in
    i*86) ARCH=32bit;;
    *_64|amd64) ARCH=64bit;;
    *) ARCH=unknown;;
  esac

  if test -f "../../`uname -s`/$ARCH/lib/libsepa.a"; then
    # source distribution
    SEPA_SHARED_LIBADD="../../`uname -s`/$ARCH/lib/libsepa.a $SEPA_SHARED_LIBADD"
    PHP_ADD_INCLUDE(../include)
  else
    # Jenkins build environment
    SEPA_SHARED_LIBADD="../libsepa/lib/libsepa.a $SEPA_SHARED_LIBADD"
    PHP_ADD_INCLUDE(../libsepa/include)
  fi

  PHP_SUBST(SEPA_SHARED_LIBADD)

  PHP_NEW_EXTENSION(sepa, sepa.c stmt.c, $ext_shared)
fi

# <EOF>_____________________________________________________________________
