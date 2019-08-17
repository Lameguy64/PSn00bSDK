.section .data

.global child_exe			# Insert spoopypasta
.type child_exe, @object
child_exe:
	.incbin "child.exe"