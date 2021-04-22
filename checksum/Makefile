APPS = crc32_csum md5_csum ip_csum

all clean:
	@for i in $(APPS) ; do \
        make -f Makefile.$$i $@ ; \
    done

.PHONY: all clean
