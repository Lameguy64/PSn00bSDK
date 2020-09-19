# Run using make (Linux) or gmake (BSD)
# Part of the PSn00bSDK Project
# 2019 - 2020 Lameguy64 / Meido-Tek Productions

TOPTARGETS	= all clean

LIBS 		= libc psxgpu psxapi psxgte psxcd psxetc psxsio psxspu lzp   

$(TOPTARGETS): $(LIBS)

install: $(LIBS)
ifdef PSN00BSDK_LIBS
ifneq ($(CURDIR),$(PSN00BSDK_LIBS))				# needs a better method
	cp -R include $(PSN00BSDK_LIBS)/include
endif
endif

clean: $(LIBS)

$(LIBS):
	@$(MAKE) -C $@ $(MAKECMDGOALS)
	
.PHONY: $(TOPTARGETS) $(LIBS)
