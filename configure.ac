#AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS], [TAR-NAME], [URL])
AC_INIT([zksh], [0.0.1], [marius.rieder@durchmesser.ch], [zksh], [https://github.com/jiuka/zksh])

# Force autoconf to be at least this version number.
AC_PREREQ([2.68])

# Put autotools auxiliary files in a subdir, so they don't clutter top dir.
# I am not actually doing this at the moment because there should be relatively
# little in the top directory anyway.
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

# Enable "automake" to simplify creating makefiles. foreign relaxes some GNU
# checks. -Wall and -Werror are instructions to Automake, not gcc.
# Because this is a non-recursive automake, we use subdir-objects to tell
# automake to create objects such as .o in sub-directories rather than placing
# all artifacts in the top level directory.
AM_INIT_AUTOMAKE([subdir-objects foreign -Wall -Werror])

# These are the files to be generated.
AC_CONFIG_FILES([Makefile src/Makefile man/Makefile tests/Makefile])

# Safety check - list a source file that wouldn't be in other directories.
AC_CONFIG_SRCDIR([src/zksh.h])

# Put configuration results in this file for us to #include in our program.
AC_CONFIG_HEADERS([src/config.h])

AC_USE_SYSTEM_EXTENSIONS
AC_GNU_SOURCE
AC_CHECK_FUNCS([__secure_getenv secure_getenv])

# Check for a C compiler.
AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_RANLIB
AM_PROG_AR

# Check for libraries, headers, data etc here.

AC_USE_SYSTEM_EXTENSIONS
AC_CHECK_FUNCS([__secure_getenv secure_getenv])
AC_CHECK_FUNCS([asprintf],,AC_MSG_ERROR([asprintf not found.]))


PKG_CHECK_MODULES([LIBCONFIG], [libconfig >= 1.3],
                  CFLAGS="$CFLAGS $LIBCONFIG_CFLAGS"
                  LIBS="$LIBS $LIBCONFIG_LIBS"
                  ,
                  AC_MSG_ERROR([libconfig 1.3 or newer not found.])
                  )

ACX_PTHREAD
CC="$PTHREAD_CC"

## Check
AC_ARG_WITH([check], AS_HELP_STRING([--without-check], [Build without the check library]))
AS_IF([test "x$with_check" != "xno"],
      [PKG_CHECK_MODULES([CHECK], [check >= 0.9.4],
                         [have_check=yes], [have_check=no])],
                         [have_check=disable])
AM_CONDITIONAL([HAVE_CHECK], [test "x$CHECK_LIBS" != x])

AC_PATH_PROG([RONN], [ronn])
AM_CONDITIONAL([HAVE_RONN], [test "x$RONN" != x])

# Last line: actually generate all required output files.
AC_OUTPUT
