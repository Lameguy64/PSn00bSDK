#
# LibPSn00b Example Programs
#
# Drawing Tile-maps with Assembler Routines
# 2022 Meido-Tek Productions / PSn00bSDK Project
#
# Example by John "Lameguy" Wilbert Villamor (Lameguy64)
#
# This assembler file is used to include the file tiles.tim as an array named
# 'tim_tileset' for use in this example program. Note how the variable name
# itself is leading with an underscore (_) in this file. This is because
# GNU C requires leading underscores for global variables, perhaps to prevent
# function names and variable names from mixing up during the linking stage.

# Tell assembler that the contents that follow must be in the .data section
.section .data

# This directive define the 'tim_tileset' label as a global symbol so that
# main.c and other program modules can see this symbol during linking
.global tim_tileset

# This directive is not really required, but its best to define symbols
# not pointing to program code as an object to help identify it as a
# variable in debuggers
.type tim_tileset, @object

# The following line defines the variable 'tim_tileset' itself filled with the
# contents of the file 'tiles.tim' by using the .incbin directive
#
# Remember the variable type of a symbol is always governed by how it is
# declared in the C code
#
tim_tileset:
	.incbin "../tiles_256.tim"
	