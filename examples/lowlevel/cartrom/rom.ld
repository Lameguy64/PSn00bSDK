MEMORY {
	ROM : ORIGIN = 0x1f000000, LENGTH = 256K
}

ENTRY (entry)

SECTIONS {
	.text :{
		*(.text)
	} >ROM
}
