# Run using make (Linux) or gmake (BSD)
# Part of the PSn00bSDK Project
# 2019 Lameguy64 / Meido-Tek Productions

TOPTARGETS	= all clean

DIRS 		= balls gte n00bdemo fpscam childexec render2tex rgb24 timer billboard


$(TOPTARGETS): $(DIRS)
$(DIRS):
	@$(MAKE) -C $@ $(MAKECMDGOALS)

clean: $(DIRS)

.PHONY: $(TOPTARGETS) $(DIRS)
