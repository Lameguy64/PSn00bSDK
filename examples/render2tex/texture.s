# Assembler file for including the texture file in a more elegant manner

.section .data

.global tim_blendpattern
.type tim_blendpattern, @object
tim_blendpattern:
	.incbin "blendpattern-16c.tim"
	