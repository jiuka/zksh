## Process this file with automake to produce Makefile.in

bin_PROGRAMS = zkls zkcat zktouch zkrm zktee zkgetacl zklock

LDADD = libzksh.a -lzookeeper_mt

noinst_LIBRARIES  = libzksh.a

libzksh_a_SOURCES = zksh.c zksh.h \
										zkeproxy.c zkeproxy.h
