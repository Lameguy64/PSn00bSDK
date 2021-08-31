.set noreorder

.section .text

.global FlushCache
.type FlushCache, @function
FlushCache:
	addiu	$t2, $0, 0xa0
	jr		$t2
	addiu	$t1, $0, 0x44
