PREFIX		= mipsel-unknown-elf-

CC			= $(PREFIX)gcc
AS			= $(PREFIX)as
LD			= $(PREFIX)ld

all: rom.o
	$(LD) --oformat binary -T rom.ld -o cartrom.rom rom.o
	
%.o: %.s
	$(AS) -msoft-float --warn $< -o $@
	
clean:
	rm -f rom.o cartrom.rom