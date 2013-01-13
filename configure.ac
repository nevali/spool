## Copyright 2013 Mo McRoberts.
##
##  Licensed under the Apache License, Version 2.0 (the "License");
##  you may not use this file except in compliance with the License.
##  You may obtain a copy of the License at
##
##      http://www.apache.org/licenses/LICENSE-2.0
##
##  Unless required by applicable law or agreed to in writing, software
##  distributed under the License is distributed on an "AS IS" BASIS,
##  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
##  See the License for the specific language governing permissions and
##  limitations under the License.

AC_INIT([spool],[master])

AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE

AC_CONFIG_HEADER([config.h])

AC_PROG_CC

LT_INIT

AC_MSG_CHECKING([whether to enable compiler warnings])
if test x"$GCC" = x"yes" ; then
   AC_MSG_RESULT([yes, -W -Wall])
   AM_CPPFLAGS="$_CPPFLAGS -W -Wall"
else
   AC_MSG_RESULT([no])
fi
AC_SUBST([AM_CPPFLAGS])

AC_CHECK_HEADERS([unistd.h dirent.h uuid/uuid.h uuid.h sys/types.h sys/stat.h fcntl.h])

AC_CHECK_FUNC([uuid_generate],,[
		AC_CHECK_LIB([uuid],[uuid_generate],,[
				AC_MSG_ERROR([cannot locate the library containing uuid_generate(); please install libuuid])
				])
		])

INCLUDES="$INCLUDES -I\${top_srcdir}/iniparser/src"
AC_SUBST([INCLUDES])

buildroot="`pwd`"
AC_SUBST([buildroot])

AC_CONFIG_FILES([
Makefile
source/Makefile
identify/Makefile
storage/Makefile
spoold.conf
])

AC_OUTPUT
