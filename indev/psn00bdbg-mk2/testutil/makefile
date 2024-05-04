CC      = wcc386
LNK     = wlink

CFLAGS  = -q -bt=nt -w3 -d2 -odhl

all : psdbtest.exe

psdbtest.exe : main.obj prompt.obj comms.obj serial.obj mips_disassembler.obj exec.obj
	wlink name psdbtest system nt f main,prompt,comms,serial,mips_disassembler,exec

main.obj : main.c main.h cmdefs.h
	$(CC) $(CFLAGS) main.c
	
comms.obj : comms.c main.h cmdefs.h
	$(CC) $(CFLAGS) comms.c
	
prompt.obj : prompt.c main.h cmdefs.h
	$(CC) $(CFLAGS) prompt.c
	
serial.obj : serial.c
	$(CC) $(CFLAGS) serial.c

mips_disassembler.obj : mips_disassembler.c mips_disassembler.h
	$(CC) $(CFLAGS) mips_disassembler.c

exec.obj : exec.c exec.h
	$(CC) $(CFLAGS) exec.c

clean : .SYMBOLIC
	del psdbtest.exe
	del *.obj