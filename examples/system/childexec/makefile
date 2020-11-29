include ../../examples-setup.mk

INCLUDE	 	+= 
LIBDIRS		+= 

LIBS		= -lpsxgpu -lpsxgte -lpsxspu -lpsxetc -lpsxapi -lc

CFLAGS		= -g -O2 -fno-builtin -fdata-sections -ffunction-sections
CPPFLAGS	= $(CFLAGS) -fno-exceptions
AFLAGS		= -g -msoft-float
LDFLAGS		= -g -gc-sections -T $(GCC_BASE)/mipsel-unknown-elf/lib/ldscripts/elf32elmip.x

LDFLAGS_P	= $(LDFLAGS) -Ttext=0x80010000
LDFLAGS_C	= $(LDFLAGS) -Ttext=0x80030000

all: child parent

child: build/child.o
	$(LD) $(LDFLAGS_C) $(LIBDIRS) build/child.o $(LIBS) -o child.elf
	elf2x -q child.elf

parent: build/parent.o build/child_exe.o
	$(LD) $(LDFLAGS_P) $(LIBDIRS) build/parent.o build/child_exe.o $(LIBS) -o parent.elf
	elf2x -q parent.elf
	
build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@
	
build/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(AFLAGS) $(INCLUDE) -c $< -o $@
	
clean:
	rm -rf build parent.elf parent.exe child.elf child.exe
