TOPTARGETS	= all install clean

TOOLDIRS 	= lzpack smxlink util

$(TOPTARGETS): $(TOOLDIRS)
$(TOOLDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

clean: $(LIBDIRS)
	rm -Rf bin

.PHONY: $(TOPTARGETS) $(TOOLDIRS)
