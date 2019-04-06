CFLAGS	= -O2 -s

CC		= gcc

ifeq "$(OS)" "Windows_NT"
EXE_SUFFIX	= .exe
else
EXE_SUFFIX	=
endif

all:
	$(CC) $(CFLAGS) elf2x.c -o elf2x$(EXE_SUFFIX)
	
install:
	mkdir -p ../bin
	cp elf2x$(EXE_SUFFIX) ../bin/elf2x$(EXE_SUFFIX)
	
clean:
	rm -f elf2x$(EXE_SUFFIX)
