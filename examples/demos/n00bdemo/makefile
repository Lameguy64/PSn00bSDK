include ../../sdk-common.mk

TARGET		= demo.elf

CFILES		= $(notdir $(wildcard *.c))
AFILES		= $(notdir $(wildcard *.s))

OFILES		= $(addprefix build/,$(CFILES:.c=.o) $(AFILES:.s=.o))

INCLUDE	 	+= -I../../../libpsn00b/lzp

LIBS		= -llzp -lc -lpsxetc -lpsxgpu -lpsxgte -lpsxspu -lpsxapi -lc

CFLAGS		= -g -O2 -fno-builtin -fdata-sections -ffunction-sections
CPPFLAGS	= $(CFLAGS) -fno-exceptions
AFLAGS		= -g -msoft-float
LDFLAGS		= -g -Ttext=0x80010000 -gc-sections -T $(GCC_BASE)/mipsel-unknown-elf/lib/ldscripts/elf32elmip.x

CC			= $(PREFIX)gcc
CXX			= $(PREFIX)g++
AS			= $(PREFIX)as
LD			= $(PREFIX)ld

all: resources $(OFILES)
	$(LD) $(LDFLAGS) $(LIBDIRS) $(OFILES) $(LIBS) -o $(TARGET)
	elf2x -q $(TARGET)
	
build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@
	
build/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(AFLAGS) $(INCLUDE) -c $< -o $@
	
resources:
	lzpack data.xml
	touch data.s
	
iso:
	mkpsxiso -y -q -o demo.iso iso.xml
	
clean:
	rm -rf build *.lzp *.qlp $(TARGET) $(TARGET:.elf=.exe) $(TARGET:.elf=.iso)
