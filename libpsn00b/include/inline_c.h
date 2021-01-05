/* Inline GTE macros for the GNU C compiler.
 *
 * Part of the PSn00bSDK Project by Lameguy64.
 * 2019 Meido-Tek Production
 *
 * All GTE commands can be used without having to pass your object file
 * through some stupid tool such as DMPSX. Perhaps it was Sony's attempt
 * to prevent people from quickly discovering the GTE commands from the 
 * official SDK easily? Though people could just extract the cop2 opcodes
 * of an object file after it has been passed through DMPSX.
 *
 * Todo: A couple of GTE operation macros are still missing such as
 *  gte_rtv*() though they appear to be just variants of gte_mvmva more or
 *  less (gte_rtv0() is actually gte_mvmva(1, 0, 0, 3, 0) for example).
 *
 */

#ifndef _INLINE_C_H
#define _INLINE_C_H

/*
 *	GTE load macros
 */

/* Load a SVECTOR (passed as a pointer) to GTE V0
 */
#define gte_ldv0( r0 ) __asm__ volatile ( \
	"lwc2	$0 , 0( %0 );"	\
	"lwc2	$1 , 4( %0 );"	\
	:						\
	: "r"( r0 )				\
	: "$t0" )

/* Load a SVECTOR (passed as a pointer) to GTE V1
 */
#define gte_ldv1( r0 ) __asm__ volatile ( \
	"lwc2	$2 , 0( %0 );"	\
	"lwc2	$3 , 4( %0 );"	\
	:						\
	: "r"( r0 )				\
	: "$t0" )

/* Load a SVECTOR (passed as a pointer) to GTE V2
 */
#define gte_ldv2( r0 ) __asm__ volatile ( \
	"lwc2	$4 , 0( %0 );"	\
	"lwc2	$5 , 4( %0 );"	\
	:						\
	: "r"( r0 )				\
	: "$t0" )

/* Load three SVECTORs (passed as a pointer) to the GTE at once
 */
#define gte_ldv3( r0, r1, r2 ) __asm__ volatile ( \
	"lwc2	$0 , 0( %0 );"	\
	"lwc2	$1 , 4( %0 );"	\
	"lwc2	$2 , 0( %1 );"	\
	"lwc2	$3 , 4( %1 );"	\
	"lwc2	$4 , 0( %2 );"	\
	"lwc2	$5 , 4( %2 );"	\
	:						\
	: "r"( r0 ), "r"( r1 ), "r"( r2 ) )
	
#define gte_ldrgb( r0 ) __asm__ volatile ( \
	"lwc2	$6 , 0( %0 );"	\
	:						\
	: "r"( r0 ) )
	
#define gte_ldopv2( r0 ) __asm__ volatile ( \
	"lwc2	$11, 8( %0 );"	\
	"lwc2	$9 , 0( %0 );"	\
	"lwc2	$10, 4( %0 );"	\
	:						\
	: "r"( r0 ) )

#define gte_lddp( r0 ) __asm__ volatile ( \
	"mtc2	%0, $8;"		\
	:						\
	: "r"( r0 ) )
	
/* Sets the GTE offset
 */
#define gte_SetGeomOffset( r0, r1 ) __asm__ volatile ( \
	"sll	$t0, %0, 16;"	\
	"sll	$t1, %1, 16;"	\
	"ctc2 	$t0, $24;"		\
	"ctc2	$t1, $25;"		\
	:						\
	: "r"( r0 ), "r"( r1 )	\
	: "$t0", "$t1" )
	
#define gte_SetGeomScreen( r0 ) __asm__ volatile ( \
	"ctc2	%0, $26;"		\
	:						\
	: "r"( r0 ) )

#define gte_SetTransMatrix( r0 ) __asm__ volatile ( \
	"lw		$t0, 20( %0 );"	\
	"lw		$t1, 24( %0 );"	\
	"ctc2	$t0, $5;"		\
	"lw		$t2, 28( %0 );"	\
	"ctc2	$t1, $6;"		\
	"ctc2	$t2, $7;"		\
	:						\
	: "r"( r0 )				\
	: "$t2" )
	
#define gte_SetRotMatrix( r0 ) __asm__ volatile ( \
	"lw		$t0, 0( %0 );"	\
	"lw		$t1, 4( %0 );"	\
	"ctc2	$t0, $0;"		\
	"ctc2	$t1, $1;"		\
	"lw		$t0, 8( %0 );"	\
	"lw		$t1, 12( %0 );"	\
	"lhu	$t2, 16( %0 );"	\
	"ctc2	$t0, $2;"		\
	"ctc2	$t1, $3;"		\
	"ctc2	$t2, $4;"		\
	:						\
	: "r"( r0 )				\
	: "$t2" )

#define gte_SetLightMatrix( r0 ) __asm__ volatile ( \
	"lw		$t0, 0( %0 );"	\
	"lw		$t1, 4( %0 );"	\
	"ctc2	$t0, $8;"		\
	"ctc2	$t1, $9;"		\
	"lw		$t0, 8( %0 );"	\
	"lw		$t1, 12( %0 );"	\
	"lhu	$t2, 16( %0 );"	\
	"ctc2	$t0, $10;"		\
	"ctc2	$t1, $11;"		\
	"ctc2	$t2, $12;"		\
	:						\
	: "r"( r0 )				\
	: "$t2" )
	
#define gte_SetColorMatrix( r0 ) __asm__ volatile ( \
	"lw		$t0, 0( %0 );"	\
	"lw		$t1, 4( %0 );"	\
	"ctc2	$t0, $16;"		\
	"ctc2	$t1, $17;"		\
	"lw		$t0, 8( %0 );"	\
	"lw		$t1, 12( %0 );"	\
	"lhu	$t2, 16( %0 );"	\
	"ctc2	$t0, $18;"		\
	"ctc2	$t1, $19;"		\
	"ctc2	$t2, $20;"		\
	:						\
	: "r"( r0 )				\
	: "$t2" )
	
#define gte_SetBackColor( r0, r1, r2 ) __asm__ volatile ( \
	"sll	$t0, %0, 4;"	\
	"sll	$t1, %1, 4;"	\
	"sll	$t2, %2, 4;"	\
	"ctc2	$t0, $13;"		\
	"ctc2	$t1, $14;"		\
	"ctc2	$t2, $15;"		\
	:						\
	: "r"( r0 ), "r"( r1 ), "r"( r2 )	\
	: "$t0", "$t1", "$t2" )
	
/*
 *	GTE store macros
 */
	
#define gte_otz( r0 ) __asm__ volatile ( \
	"swc2	$7, 0( %0 );"	\
	:						\
	: "r"( r0 )				\
	: "memory" )
	
#define gte_stflg( r0 ) __asm__ volatile ( \
	"cfc2	$t0, $31;"		\
	"nop;"					\
	"sw		$t0, 0( %0 );"	\
	:						\
	: "r"( r0 )				\
	: "memory" )
	
#define gte_stsxy( r0 ) __asm__ volatile ( \
	"swc2	$14, 0( %0 );"	\
	:						\
	: "r"( r0 )				\
	: "memory" )
	
#define gte_stsxy0( r0 ) __asm__ volatile ( \
	"swc2	$12, 0( %0 );"	\
	:						\
	: "r"( r0 )				\
	: "memory" )

#define gte_stsxy1( r0 ) __asm__ volatile ( \
	"swc2	$13, 0( %0 );"	\
	:						\
	: "r"( r0 )				\
	: "memory" )

#define gte_stsxy2( r0 ) __asm__ volatile ( \
	"swc2	$14, 0( %0 );"	\
	:						\
	: "r"( r0 )				\
	: "memory" )

#define gte_stsxy3( r0, r1, r2 ) __asm__ volatile ( \
	"swc2	$12, 0( %0 );"	\
	"swc2	$13, 0( %1 );"	\
	"swc2	$14, 0( %2 );"	\
	:						\
	: "r"( r0 ), "r"( r1 ), "r"( r2 ) \
	: "memory" )

#define gte_stsz( r0 ) __asm__ volatile ( \
	"swc2	$19, 0( %0 );"	\
	:						\
	: "r"( r0 )				\
	: "memory" )
	
#define gte_stotz( r0 ) __asm__ volatile ( \
	"swc2	$7, 0( %0 );"	\
	:						\
	: "r"( r0 ) 			\
	: "memory" )
	
#define gte_stopz( r0 ) __asm__ volatile ( \
	"swc2	$24, 0( %0 );"	\
	:						\
	: "r"( r0 )				\
	: "memory" )
	
#define gte_strgb( r0 ) __asm__ volatile ( \
	"swc2	$22, 0( %0 );"	\
	:						\
	: "r"( r0 )				\
	: "memory" )
	
#define gte_strgb3( r0, r1, r2 ) __asm__ volatile ( \
	"swc2	$20, 0( %0 );"	\
	"swc2	$21, 0( %1 );"	\
	"swc2	$22, 0( %2 );"	\
	:						\
	: "r"( r0 ), "r"( r1 ), "r" ( r2 )	\
	: "memory" )

#define gte_stsv( r0 ) __asm__ volatile ( \
	"mfc2	$t0, $9;"		\
	"mfc2	$t1, $10;"		\
	"mfc2	$t2, $11;"		\
	"sh		$t0, 0( %0 );"	\
	"sh		$t1, 2( %0 );"	\
	"sh		$t2, 4( %0 );"	\
	:						\
	: "r"( r0 )				\
	: "memory" )
	
#define gte_stlvnl( r0 ) __asm__ volatile ( \
	"swc2	$25, 0( %0 );"	\
	"swc2	$26, 4( %0 );"	\
	"swc2	$27, 8( %0 );"	\
	:						\
	: "r"( r0 )				\
	: "memory" )
	
	
/*
 *	GTE operation macros (does not need a stupid tool such as dmpsx!)
 */
 
#define gte_rtps() __asm__ volatile ( \
	"nop;"					\
	"nop;"					\
	"cop2 0x0180001;" )

#define gte_rtpt() __asm__ volatile ( \
	"nop;"					\
	"nop;"					\
	"cop2 0x0280030;" )
	
#define gte_nclip() __asm__ volatile ( \
	"nop;"					\
	"nop;"					\
	"cop2 0x1400006;" )
	
#define gte_avsz3() __asm__ volatile ( \
	"nop;"					\
	"nop;"					\
	"cop2 0x158002D;" )
	
#define gte_avsz4() __asm__ volatile ( \
	"nop;"					\
	"nop;"					\
	"cop2 0x168002E;" )
	
#define gte_sqr0() __asm__ volatile ( \
	"nop;"					\
	"nop;"					\
	"cop2	0x0A00428;" )
	
#define gte_sqr12() __asm__ volatile ( \
	"nop;"					\
	"nop;"					\
	"cop2	0x0A80428;" )
	
#define gte_op0() __asm__ volatile ( \
	"nop;"					\
	"nop;"					\
	"cop2	0x170000C;"	)

#define gte_op12() __asm__ volatile ( \
	"nop;"					\
	"nop;"					\
	"cop2	0x178000C;"	)
	
#define gte_ncs() __asm__ volatile ( \
	"nop;"					\
	"nop;"					\
	"cop2	0x0C8041E;" )
	
#define gte_nct() __asm__ volatile ( \
	"nop;"					\
	"nop;"					\
	"cop2	0x0D80420;" )
	
#define gte_nccs() __asm__ volatile ( \
	"nop;"					\
	"nop;"					\
	"cop2	0x108041B;" )	\
	
#define gte_ncct() __asm__ volatile ( \
	"nop;"					\
	"nop;"					\
	"cop2	0x118043F;"	)
	
#define gte_ncds() __asm__ volatile ( \
	"nop;"					\
	"nop;"					\
	"cop2	0x0E80413;"	)
	
#define gte_ncdt() __asm__ volatile ( \
	"nop;"					\
	"nop;"					\
	"cop2	0x0F80416;" )
	
#define gte_cc() __asm__ volatile ( \
	"nop;"					\
	"nop;"					\
	"cop2	0x138041C;" )
	
#define gte_cdp() __asm__ volatile ( \
	"nop;"					\
	"nop;"					\
	"cop2	0x1280414;" )
	
#define gte_dcpl() __asm__ volatile ( \
	"nop;"					\
	"nop;"					\
	"cop2	0x0680029;"	)
	
#define gte_dpcs() __asm__ volatile ( \
	"nop;"					\
	"nop;"					\
	"cop2	0x0780010;" )
	
#define gte_dpct() __asm__ volatile ( \
	"nop;"					\
	"nop;"					\
	"cop2	0x0180001;" )

#define gte_intpl() __asm__ volatile ( \
	"nop;"					\
	"nop;"					\
	"cop2	0x0980011;" )

#define gte_gpf0() __asm__ volatile ( \
	"nop;"					\
	"nop;"					\
	"cop2	0x190003D;"	)

#define gte_gpf12() __asm__ volatile ( \
	"nop;"					\
	"nop;"					\
	"cop2	0x198003D;"	)
	
#define gte_gpl0() __asm__ volatile ( \
	"nop;"					\
	"nop;"					\
	"cop2	0x1A0003E;" )

#define gte_gpl12() __asm__ volatile ( \
	"nop;"					\
	"nop;"					\
	"cop2	0x1A8003E;" )

#define gte_mvmva_core( r0 ) __asm__ volatile ( \
	"nop;"					\
	"nop;"					\
	"cop2 %0"				\
	:						\
	: "g"( r0 ) )
	
#define gte_mvmva(sf, mx, v, cv, lm) gte_mvmva_core( 0x0400012 | \
	((sf)<<19) | ((mx)<<17) | ((v)<<15) | ((cv)<<13) | ((lm)<<10) )
	
	
/*
 *	GTE operation macros without leading nops
 *
 *	Checking assembler output when using these is advised.
 */
 
#define gte_rtps_b()	__asm__ volatile ( "cop2 0x0180001;" )
#define gte_rtpt_b()	__asm__ volatile ( "cop2 0x0280030;" )
#define gte_nclip_b()	__asm__ volatile ( "cop2 0x1400006;" )
#define gte_avsz3_b()	__asm__ volatile ( "cop2 0x158002D;" )
#define gte_avsz4_b()	__asm__ volatile ( "cop2 0x168002E;" )
#define gte_sqr0_b()	__asm__ volatile ( "cop2 0x0A00428;" )
#define gte_sqr12_b()	__asm__ volatile ( "cop2 0x0A80428;" )
#define gte_op0_b()		__asm__ volatile ( "cop2 0x170000C;" )
#define gte_op12_b()	__asm__ volatile ( "cop2 0x178000C;" )
#define gte_ncs_b()		__asm__ volatile ( "cop2 0x0C8041E;" )
#define gte_nct_b()		__asm__ volatile ( "cop2 0x0D80420;" )
#define gte_nccs_b()	__asm__ volatile ( "cop2 0x108041B;" )
#define gte_ncct_b()	__asm__ volatile ( "cop2 0x118043F;" )
#define gte_ncds_b()	__asm__ volatile ( "cop2 0x0E80413;" )
#define gte_ncdt_b()	__asm__ volatile ( "cop2 0x0F80416;" )
#define gte_cc_b()		__asm__ volatile ( "cop2 0x138041C;" )
#define gte_cdp_b()		__asm__ volatile ( "cop2 0x1280414;" )
#define gte_dcpl_b()	__asm__ volatile ( "cop2 0x0680029;" )
#define gte_dpcs_b()	__asm__ volatile ( "cop2 0x0780010;" )
#define gte_dpct_b()	__asm__ volatile ( "cop2 0x0180001;" )
#define gte_intpl_b()	__asm__ volatile ( "cop2 0x0980011;" )
#define gte_gpf0_b()	__asm__ volatile ( "cop2 0x190003D;" )
#define gte_gpf12_b()	__asm__ volatile ( "cop2 0x198003D;" )
#define gte_gpl0_b()	__asm__ volatile ( "cop2 0x1A0003E;" )
#define gte_gpl12_b()	__asm__ volatile ( "cop2 0x1A8003E;" )
#define gte_mvmva_core_b( r0 ) __asm__ volatile ( \
	"cop2 %0"				\
	:						\
	: "g"( r0 ) )	
#define gte_mvmva_b(sf, mx, v, cv, lm) gte_mvmva_core_b( 0x0400012 | \
	((sf)<<19) | ((mx)<<17) | ((v)<<15) | ((cv)<<13) | ((lm)<<10) )
	
#endif // _INLINE_C_H