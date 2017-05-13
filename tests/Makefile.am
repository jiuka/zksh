## Process this file with automake to produce Makefile.in

TESTS = check_zksh_join_path check_zksh_check_nodepath
check_PROGRAMS = check_zksh_join_path check_zksh_check_nodepath

AM_CFLAGS = @CHECK_CFLAGS@
LDADD = $(top_builddir)/src/libzksh.a @CHECK_LIBS@ -lzookeeper_mt
