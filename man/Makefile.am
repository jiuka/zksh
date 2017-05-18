man_MANS = zkls.1 zkrm.1 zkcat.1 zktouch.1 zktee.1 zkgetacl.1 zklock.1 zkwait.1
EXTRA_DIST = $(man_MANS)

.ronn.1:
if HAVE_RONN
		$(RONN) --roff $<
else
	@echo "Error: ronn not found! Can not build manpages"
endif
