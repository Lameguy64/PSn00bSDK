patch.bin: patchinst.bin
	copy /B patchinst.bin+patchcode.bin patch.bin

patchinst.bin: monitor.asm cmdefs.inc sio.inc
	armips -temp monitor.lst monitor.asm 
	
clean: .SYMBOLIC
	del patch.bin
	del patchinst.bin
	del patchcode.bin
