# Run using make (Linux) or gmake (BSD)
# Part of the PSn00bSDK Project
# 2019-2020 Lameguy64 / Meido-Tek Productions

TOPTARGETS	= all clean

# Beginner examples
DIRS 		= beginner/hello beginner/cppdemo

# Graphics examples
DIRS	   += graphics/balls graphics/billboard graphics/fpscam \
			  graphics/gte graphics/hdtv graphics/render2tex \
			  graphics/rgb24 

# System related examples
DIRS	   += system/childexec system/console system/timer \
			  system/tty

# Low-level examples
DIRS	   += 

# CD-ROM examples
DIRS	   += cdrom/cdbrowse cdrom/cdxa

# Demos
DIRS	   += demos/n00bdemo

$(TOPTARGETS): $(DIRS)
$(DIRS):
	@$(MAKE) -C $@ $(MAKECMDGOALS)

clean: $(DIRS)

.PHONY: $(TOPTARGETS) $(DIRS)
