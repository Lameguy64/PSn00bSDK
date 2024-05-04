#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "exec.h"

#define	MAX_prg_entry_count	128

#pragma pack(push, 1)

typedef struct _ELF_HEADER {

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

typedef struct _PRG_HEADER {
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

typedef struct _PSEXE {
	char header[8];
	char pad[8];
	EXEC params;
	char license[64];
	char pad2[1908];
} PSEXE;

typedef struct _EXEPARAM {
	EXEC params;
	unsigned int crc32;
	unsigned int flags;
} EXEPARAM;

typedef struct _BINPARAM {
    int size;
    unsigned int addr;
    unsigned int crc32;
} BINPARAM;


void* loadELF(FILE* fp, EXEC* param)
{
	int i;
	ELF_HEADER head;
	PRG_HEADER prg_heads[MAX_prg_entry_count];
	unsigned int exe_taddr = 0xffffffff;
	unsigned int exe_haddr = 0;
	unsigned int exe_tsize = 0;
	unsigned char* binary;
	size_t read_n;
	
	fseek(fp, 0, SEEK_SET);
	read_n = fread(&head, 1, sizeof(head), fp);

	// Check header
	if( head.magic != 0x464c457f ) {
		
		printf("File is neither a PS-EXE, CPE or ELF binary.\n");
		return NULL;
		
	}

	if( head.type != 2 ) {
		
		printf("Only executable ELF files are supported.\n");
		return NULL;
		
	}

	if( head.instr_set != 8 ) {
		
		printf("ELF file is not a MIPS binary.\n");
		return NULL;
		
	}

	if( head.word_size != 1 ) {
		
		printf("Only 32-bit ELF files are supported.\n");
		return NULL;
		
	}

	if( head.endianness != 1 ) {
		
		printf("Only little endian ELF files are supported.\n");
		return NULL;
		
	}


	// Load program headers and determine binary size and load address

	fseek(fp, head.prg_head_pos, SEEK_SET);
	for( i=0; i<head.prg_entry_count; i++ ) {

		read_n = fread( &prg_heads[i], 1, sizeof(PRG_HEADER), fp );

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
	

	// Check if load address is appropriate in main RAM locations
	if( ( ( exe_taddr>>24 ) == 0x0 ) || ( ( exe_taddr>>24 ) == 0x80 ) ||
		( ( exe_taddr>>24 ) == 0xA0 ) ) {

		if( ( exe_taddr&0x00ffffff ) < 65536 ) {

			printf( "WARNING: Program text address overlaps kernel area!\n" );

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
		read_n = fread( &binary[(int)(prg_heads[i].p_vaddr-exe_taddr)],
			1, prg_heads[i].p_filesz, fp );

	}
	
	memset(param, 0, sizeof(EXEC));
	param->pc0 = head.prg_entry_addr;
	param->t_addr = exe_taddr;
	param->t_size = exe_tsize;
	
	return binary;
	
}

void* loadCPE(FILE* fp, EXEC* param) {
	
	int i, val, exe_size = 0;
	unsigned int uv, exe_entry = 0;
	unsigned int *addr_list = NULL;
	int addr_list_len = 0;	
	char* exe_buff;
	unsigned int addr_upper=0;
	unsigned int addr_lower=0;
	size_t read_n;
	
	// Check CPE magic word
	fseek(fp, 0, SEEK_SET);
	read_n = fread(&val, 1, 4, fp);
	
	if( val != 0x01455043 )
	{
		return NULL;
	}
	
	// Clear EXEC parameters
	memset(param, 0x0, sizeof(EXEC));
	
	// Begin parsing CPE data
	val = 0;
	read_n = fread(&val, 1, 1, fp);
	
	while( val )
	{
		switch( val )
		{
		case 0x1:	// Binary chunk
				
			read_n = fread(&uv, 1, 4, fp);
			read_n = fread(&val, 1, 4, fp);
				
			addr_list_len++;
			addr_list = (unsigned int*)realloc(addr_list, 
				sizeof(unsigned int)*addr_list_len);
				
			addr_list[addr_list_len-1] = uv;
			exe_size += val;
				
			fseek(fp, val, SEEK_CUR);
			break;
				
		case 0x3:	// Set register, ignored
				
			val = 0;
			read_n = fread(&val, 1, 2, fp);
				
			if( val != 0x90 )
			{
				printf("Warning: Unknown SETREG code: %d\n", val);
			}
				
			read_n = fread(&exe_entry, 1, 4, fp);
			break;
				
		case 0x8:	// Select unit, ignored
				
			val = 0;
			read_n = fread(&val, 1, 1, fp);
			break;
				
		default:
			
			printf("Unknown chunk found: %d\n", val);
			free(addr_list);
			return NULL;
				
		}
		
		// Read next code
		val = 0;
		read_n = fread(&val, 1, 1, fp);
		
	}
	
	// Begin loading the executable data
	
	// Find the highest chunk address
	for(i=0; i<addr_list_len; i++) {
		
		if( addr_list[i] > addr_upper ) {
			
			addr_upper = addr_list[i];
			
		}
		
	}
	
	// Find the lowest chunk address
	addr_lower = addr_upper;
	
	for(i=0; i<addr_list_len; i++) {
		
		if ( addr_list[i] < addr_lower ) {
			
			addr_lower = addr_list[i];
			
		}
		
	}
	
	// Pad executable size to multiples of 2KB and allocate
	exe_size = 2048*((exe_size+2047)/2048);
	
	exe_buff = (char*)malloc(exe_size);
	memset(exe_buff, 0x0, exe_size);
	
	// Load binary chunks
	val = 0;
	fseek(fp, 4, SEEK_SET);
	read_n = fread(&val, 1, 1, fp);
	
	while( val ) {
		
		switch( val ) {
			case 0x1:	// Binary chunk
				
				read_n = fread(&uv, 1, 4, fp);
				read_n = fread(&val, 1, 4, fp);
				
				read_n = fread(exe_buff+(uv-addr_lower), 1, val, fp);
				
				break;
				
			case 0x3:	// Set register (skipped)
				
				val = 0;
				read_n = fread(&val, 1, 2, fp);
				
				if( val == 0x90 ) {
					
					fseek(fp, 4, SEEK_CUR);
					
				}
				
				break;
				
			case 0x8:	// Select unit (skipped)
				
				fseek(fp, 1, SEEK_CUR);
				
				break;
		}
		
		val = 0;
		read_n = fread(&val, 1, 1, fp);
		
	}
	
	// Set program text and entrypoint
	param->pc0 = exe_entry;
	param->t_addr = addr_lower;
	param->t_size = exe_size;
	
	return exe_buff;
	
}

void *loadExecutable(const char* exefile, EXEC *param) {
	
	PSEXE exe;
	unsigned char* buffer;
	
	FILE* fp = fopen(exefile, "rb");
	
	if( fp == NULL )
	{
		printf("File not found.\n");
		return(NULL);
	}
	
	// Load PS-EXE header
	if( fread(&exe, 1, sizeof(PSEXE), fp) != sizeof(PSEXE) )
	{
		printf("Read error or invalid file.\n");
		fclose(fp);
		return(NULL);
	}
	
	// Check file ID
	if( strcmp(exe.header, "PS-X EXE") ) {
		
		// If not PS-EXE, try loading it as CPE
		buffer = (unsigned char*)loadCPE(fp, param);
		
		if( buffer == NULL ) {
			
			// If not CPE, try loading it as ELF
			buffer = (unsigned char*)loadELF(fp, param);
			
			if( buffer == NULL ) {
			
				fclose(fp);
				return(NULL);
				
			}
			
		}
		
	} else {
		
		// Load PS-EXE body, simple
		buffer = (unsigned char*)malloc(param->t_size);
	
		if( fread(buffer, 1, param->t_size, fp) != param->t_size ) {
			
			printf("Incomplete file or read error.\n");
			free(buffer);
			fclose(fp);
			
			return(NULL);
			
		}
		
		memcpy(param, &exe.params, sizeof(EXEC));
		
	}
	
	// Done with the file
	fclose(fp);
	
	return(buffer);
}
