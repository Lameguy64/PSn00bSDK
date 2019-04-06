// Originally written in C++ by Lameguy64
// Ported to plain C by Orion

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define	MAX_prg_entry_count	128
#define	true	(1)
#define	false	(0)

#pragma pack(push, 1)

typedef struct {

	unsigned int magic;				// 0-3
	unsigned char word_size;		// 4
	unsigned char endianness;		// 5
	unsigned char elf_version;		// 6
	unsigned char os_abi;			// 7
	unsigned int unused[2];			// 8-15

	unsigned short type;			// 16-17
	unsigned short instr_set;		// 18-19
	unsigned int elf_version2;		// 20-23

	unsigned int prg_entry_addr;	// 24-27
	unsigned int prg_head_pos;		// 28-31
	unsigned int sec_head_pos;		// 32-35
	unsigned int flags;				// 36-39
	unsigned short head_size;		// 40-41
	unsigned short prg_entry_size;	// 42-23
	unsigned short prg_entry_count;	// 44-45
	unsigned short sec_entry_size;	// 46-47
	unsigned short sec_entry_count;	// 48-49
	unsigned short sec_names_index;	// 50-51

} ELF_HEADER;

typedef struct {
	unsigned int seg_type;
	unsigned int p_offset;
	unsigned int p_vaddr;
	unsigned int undefined;
	unsigned int p_filesz;
	unsigned int p_memsz;
	unsigned int flags;
	unsigned int alignment;
} PRG_HEADER;

#pragma pack(pop)

typedef struct {
	unsigned int pc0;
	unsigned int gp0;
	unsigned int t_addr;
	unsigned int t_size;
	unsigned int d_addr;
	unsigned int d_size;
	unsigned int b_addr;
	unsigned int b_size;
	unsigned int sp_addr;
	unsigned int sp_size;
	unsigned int sp;
	unsigned int fp;
	unsigned int gp;
	unsigned int ret;
	unsigned int base;
} EXEC;

typedef struct {
	char header[8];
	char pad[8];
	EXEC params;
	char license[64];
	char pad2[1908];
} PSEXE;

int main(int argc, char** argv) {

	char* in_file = NULL;
	char* out_file = NULL;
	int quiet = false;
	int	i;
	FILE* fp;
	ELF_HEADER head;
	PRG_HEADER prg_heads[MAX_prg_entry_count];
	unsigned int exe_taddr = 0xffffffff;
	unsigned int exe_haddr = 0;
	unsigned int exe_tsize = 0;
	unsigned char* binary;
	PSEXE exe;
	char *output_name;

	for( i=1; i<argc; i++ ) {

		if( strcasecmp( "-q", argv[i] ) == 0 ) {

			quiet = true;

		} else {

			if( in_file == NULL ) {
				in_file = argv[i];
			} else if( out_file == NULL ) {
				out_file = argv[i];
			}

		}

	}

	if( !quiet ) {
		printf( "PSn00bSDK elf2x - ELF to PS-EXE Converter\n" );
		printf( "2018-2019 Meido-Tek Productions\n\n" );
	}

	if( argc == 1 ) {
		printf( "Usage:\n" );
		printf( "  elf2x [-q] <elf_file> [exe_file]\n" );
		return 0;
	}

	if( in_file == NULL ) {
		printf( "No input file specified.\n" );
		return EXIT_FAILURE;
	}


	fp = fopen( in_file, "rb" );

	if( fp == NULL ) {
		printf( "Cannot open file %s.\n", in_file );
		return EXIT_FAILURE;
	}

	fread( &head, 1, sizeof(head), fp );


	// Check header
	if( head.magic != 0x464c457f ) {
		printf( "File is not an ELF file.\n" );
		return EXIT_FAILURE;
	}

	if( head.type != 2 ) {
		printf( "Only executable ELF files are supported.\n" );
		fclose( fp );
		return EXIT_FAILURE;
	}

	if( head.instr_set != 8 ) {
		printf( "ELF file is not a MIPS binary.\n" );
		fclose( fp );
		return EXIT_FAILURE;
	}

	if( head.word_size != 1 ) {
		printf( "Only 32-bit ELF files are supported.\n" );
		fclose( fp );
		return EXIT_FAILURE;
	}

	if( head.endianness != 1 ) {
		printf( "Only little endian ELF files are supported.\n" );
		fclose( fp );
		return EXIT_FAILURE;
	}


	// Load program headers and determine binary size and load address

	fseek( fp, head.prg_head_pos, SEEK_SET );
	for( i=0; i<head.prg_entry_count; i++ ) {

		fread( &prg_heads[i], 1, sizeof(PRG_HEADER), fp );

		if( prg_heads[i].flags == 4 ) {
			continue;
		}

		if( prg_heads[i].p_vaddr < exe_taddr ) {
			exe_taddr = prg_heads[i].p_vaddr;
		}

		if( prg_heads[i].p_vaddr > exe_haddr ) {
			exe_haddr = prg_heads[i].p_vaddr;
		}

	}

	exe_tsize = (exe_haddr-exe_taddr);
	exe_tsize += prg_heads[head.prg_entry_count-1].p_filesz;

	if( !quiet ) {

		printf( "pc:%08x t_addr:%08x t_size:%d\n",
			head.prg_entry_addr, exe_taddr, exe_tsize );

	}

	// Check if load address is appropriate in main RAM locations
	if( ( ( exe_taddr>>24 ) == 0x0 ) || ( ( exe_taddr>>24 ) == 0x80 ) ||
		( ( exe_taddr>>24 ) == 0xA0 ) ) {

		if( ( exe_taddr&0x00ffffff ) < 65536 ) {

			printf( "Warning: Program text address overlaps kernel area!\n" );

		}

	}


	// Pad out the size to multiples of 2KB
	exe_tsize = 2048*((exe_tsize+2047)/2048);

	// Load the binary data
	binary = (unsigned char*)malloc( exe_tsize );
	memset( binary, 0x0, exe_tsize );

	for( i=0; i<head.prg_entry_count; i++ ) {

		if( prg_heads[i].flags == 4 ) {
			continue;
		}

		fseek( fp, prg_heads[i].p_offset, SEEK_SET );
		fread( &binary[(int)(prg_heads[i].p_vaddr-exe_taddr)],
			1, prg_heads[i].p_filesz, fp );

	}

	fclose( fp );


	if( out_file ) {

		output_name = out_file;

	} else {
		char	*ptr;

		// Generate output filename if no output is specified
		output_name = in_file;

		ptr = &output_name[strlen(output_name)];
		while (ptr != output_name)
		{
			if (*ptr == '.')
				break;
			else
				ptr--;
		}

		if (ptr != output_name)
		{
			strcpy(ptr, ".exe");
		}
		else
		{
			strcat(ptr, ".exe");
		}
	}


	// Prepare PS-EXE header
	memset( &exe, 0, sizeof(PSEXE) );

	exe.params.sp_addr = 0x801FFFF0;
	exe.params.t_addr = exe_taddr;
	exe.params.t_size = exe_tsize;
	exe.params.pc0 = head.prg_entry_addr;

	strncpy( exe.header, "PS-X EXE", 8 );
	strcpy( exe.license,
		"Not Licensed or Endorsed by Sony Computer Entertainment Inc." );
	strcpy( exe.pad2, "Built using GCC and PSn00bSDK libraries" );


	// Write file
	fp = fopen( output_name, "wb" );

	if( !fp ) {
		printf( "Cannot write output: %s\n", output_name );
		free( binary );
		fclose( fp );
	}

	fwrite( &exe, 1, sizeof( PSEXE ), fp );
	fwrite( binary, 1, exe_tsize, fp );

	fclose( fp );


	free( binary );

	return 0;
}

