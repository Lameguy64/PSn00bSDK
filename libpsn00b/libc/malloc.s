# Custom first-fit malloc routines by Lameguy64
# Part of the PSn00bSDK Project
#
# NOTE: there reportedly is a GCC bug which messes up .weak functions written
# in assembly if LTO is enabled. I haven't tested but, according to the
# internet, this bug has never been fixed.
# https://gcc.gnu.org/legacy-ml/gcc-help/2019-10/msg00092.html

.set noreorder

.set ND_PREV,	0	# Address to previous block (NULL if starting block)
.set ND_NEXT,	4	# Address to next block (NULL if end block)
.set ND_SIZE,	8	# Size of block
.set ND_HSIZ,	12

.section .text

# Stupid small function just to get bss end
# due to GCC insisting externs to be gp relative
.global GetBSSend
.type GetBSSend, @function
GetBSSend:
	la		$v0, _end
	jr		$ra
	nop
	

# Initializes the heap for malloc
#   a0 - Starting address of heap
#   a1 - Size of memory heap
#
.global InitHeap
.type InitHeap, @function
.weak InitHeap
InitHeap:
	la		$v0, _malloc_addr
	sw		$a0, 0($v0)
	la		$v0, _malloc_size
	sw		$a1, 0($v0)
	
	sw		$0 , ND_PREV($a0)		# Set heap header
	sw		$0 , ND_NEXT($a0)
	jr		$ra
	sw		$0 , ND_SIZE($a0)
	
	
# Changes the heap size without clearing or relocating the heap
#   a0 - Size of memory heap in bytes
.global SetHeapSize
.type SetHeapSize, @function
.weak SetHeapSize
SetHeapSize:
	la		$v1, _malloc_size
	lw		$v0, 0($v1)
	jr		$ra
	sw		$a1, 0($v1)


# Allocates a block of memory in the heap
#   a0 - Size of memory block to allocate.
#
.global malloc
.type malloc, @function
.weak malloc
malloc:
	addiu	$a0, 3					# Round size to a multiple of 4
	srl		$a0, 2
	
	la		$a2, _malloc_addr
	lw		$a2, 0($a2)
	sll		$a0, 2
	
.Lfind_next:
	
	move	$a1, $a2
	
	lw		$a2, ND_NEXT($a1)		# Get block header
	lw		$v1, ND_SIZE($a1)
	
	subu	$v0, $a2, $a1			# Compute space between current and next
	
	beqz	$v1, .Lempty_block		# Occupy empty block (if size = 0)
	nop
	
	beqz	$a2, .Lnew_block		# Allocate a new block (if no next)
	nop
	
	addiu	$v0, -(ND_HSIZ*2)		# Compute remaining space of block
	subu	$v0, $v1
	
	blt		$v0, $a0, .Lfind_next	# Search for the next block if space is not big enough
	nop
	
	# Perform a block split using remaining space of current block

	addiu	$v0, $a1, ND_HSIZ		# Compute address for new header
	addu	$v0, $v1
	
	sw		$a1, ND_PREV($v0)		# Set the new block header
	sw		$a2, ND_NEXT($v0)
	sw		$a0, ND_SIZE($v0)
	
	sw		$v0, ND_NEXT($a1)		# Update previous and next blocks
	sw		$v0, ND_PREV($a2)
	
	jr		$ra
	addiu	$v0, ND_HSIZ
	
.Lempty_block:						# Occupy an empty block

	beqz	$a2, .Lno_next			# Skip size calculation if there's no next
	nop
	
	addiu	$v0, -ND_HSIZ
	blt		$v0, $a0, .Lfind_next
	nop
	
	b		.Lskip_space_check
	nop
	
.Lno_next:

	la		$v1, _malloc_addr		# Check if there's enough space for a block
	lw		$v1, 0($v1)
	la		$v0, _malloc_size
	lw		$v0, 0($v0)
	
	subu	$v1, $a1, $v1
	addu	$v1, $a0
	addiu	$v1, ND_HSIZ
	
	bgt		$v1, $v0, .Lno_space
	nop
	
.Lskip_space_check:

	sw		$a0, ND_SIZE($a1)
	jr		$ra						# Return address
	addiu	$v0, $a1, ND_HSIZ
	
.Lnew_block:						# Create a new block

	addu	$a2, $a1, $v1			# Compute address for new block
	addiu	$a2, ND_HSIZ
	
	la		$v1, _malloc_addr
	lw		$v1, 0($v1)
	la		$v0, _malloc_size
	lw		$v0, 0($v0)
	
	subu	$v1, $a2, $v1
	addu	$v1, $a0
	addiu	$v1, ND_HSIZ
	
	bgt		$v1, $v0, .Lno_space	# Reject if it exceeds specified size
	nop
	
	sw		$a1, ND_PREV($a2)
	sw		$0 , ND_NEXT($a2)
	sw		$a0, ND_SIZE($a2)
	
	sw		$a2, ND_NEXT($a1)
	
	jr		$ra						# Return address
	addiu	$v0, $a2, ND_HSIZ
	
.Lno_space:							# Return a null if no space can be found

	jr		$ra
	move	$v0, $0

	
# Allocates a block of memory in block units and zero fills the
# allocated block.
#   a0 - Block size.
#	a1 - Number of blocks to allocate
#
.global calloc
.type calloc, @function
.weak calloc
calloc:
	mult	$a0, $a1
	addiu	$sp, -4
	sw		$ra, 0($sp)
	
	jal		malloc
	mflo	$a0
	
	move	$a0, $v0
	mflo	$a1

.Lclear_loop:

	sw		$0 , 0($a0)
	addi	$a1, 4
	bgtz	$a1, .Lclear_loop
	addiu	$a0, 4
	
	lw		$ra, 0($sp)
	addiu	$sp, 4
	jr		$ra
	nop

	
# Deallocates an allocated block
#	a0 - An address returned by malloc to deallocate
#
.global free
.type free, @function
.weak free
free:

	addiu	$a0, -ND_HSIZ
	lw		$a1, ND_PREV($a0)
	lw		$a2, ND_NEXT($a0)
	
	beqz	$a1, .Lis_start			# Check if block is a starting block
	nop
	
	beqz	$a2, .Lis_end
	nop

	# Unlink
	
	sw		$a2, ND_NEXT($a1)
	jr		$ra
	sw		$a1, ND_PREV($a2)
	
.Lis_end:							# Unlinks the ending block

	jr		$ra
	sw		$0 , ND_NEXT($a1)
	
.Lis_start:							# Simply set size to 0 if starting block

	jr		$ra
	sw		$0 , ND_SIZE($a0)


# Internal variables
.comm _malloc_addr, 4, 4
.comm _malloc_size, 4, 4
