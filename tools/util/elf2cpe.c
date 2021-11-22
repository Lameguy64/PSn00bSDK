#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "elf.h"

#ifdef WIN32
#define strcasecmp _stricmp
#endif

#define	MAX_prg_entry_count	128

#ifndef false
#define false	0
#endif

#ifndef true
#define true	1
#endif

char *in_file = NULL;
char *out_file = NULL;

int quiet;

int copydata( FILE *in, FILE *out, int len )
{
	char buff[8192];
	int i,readlen;
	
	while( len > 0 )
	{
		readlen = len;
		if( readlen > 8192 )
		{
			readlen = 8192;
		}
		
		i = fread( buff, 1, readlen, in );
		fwrite( buff, 1, i, out );
		
		if( i < readlen )
		{
			break;
		}
		len -= readlen;
	}
	
	return( 0 );
	
} /* copydata */

int convertELF( void )
{
	ELF_HEADER head;
	PRG_HEADER prg_heads[MAX_prg_entry_count];
	
	int i;
	char *output_name;
	
	FILE *in_fp;
	FILE *out_fp;
	
	/* Generate output filename if no output is specified */
	if( out_file )
	{
		output_name = out_file;
	}
	else
	{
		char	*ptr;

		output_name = strdup( in_file );

		ptr = &output_name[strlen(output_name)];
		while (ptr != output_name)
		{
			if (*ptr == '.')
				break;
			else
				ptr--;
		}

		if( ptr != output_name )
		{
			strcpy( ptr, ".cpe" );
		}
		else
		{
			strcat( ptr, ".cpe" );
		}
	}
	
	if( !out_file )
	{
		free( output_name );
	}
	
	if( !( in_fp = fopen( in_file, "rb" ) ) )
	{
		printf( "Cannot open input file.\n" );
		return( -1 );
	}
	
	/* read ELF header */
	fread( &head, 1, sizeof(head), in_fp );
	
	/* test header to make sure it is valid */
	if( head.magic != 0x464c457f )
	{
		printf( "File is not an ELF file.\n" );
		fclose( in_fp );
		return( -1 );
	}

	if( head.type != 2 )
	{
		printf( "Only executable ELF files are supported.\n" );
		fclose( in_fp );
		return( -1 );
	}

	if( head.instr_set != 8 )
	{
		printf( "ELF file is not a MIPS binary.\n" );
		fclose( in_fp );
		return( -1 );
	}

	if( head.word_size != 1 )
	{
		printf( "Only 32-bit ELF files are supported.\n" );
		fclose( in_fp );
		return( -1 );
	}

	if( head.endianness != 1 )
	{
		printf( "Only little endian ELF files are supported.\n" );
		fclose( in_fp );
		return( -1 );
	}
	
	/* read program headers */
	fseek( in_fp, head.prg_head_pos, SEEK_SET );
	fread( prg_heads, sizeof( PRG_HEADER ), head.prg_entry_count, in_fp );
	
	if( !quiet )
	{
		printf( "pc:%08x\n", head.prg_entry_addr );
	}
	
	/* create the CPE file */
	printf( "%s\n", output_name );
	if( !( out_fp = fopen( output_name, "wb" ) ) )
	{
		printf( "Cannot create output file.\n" );
		return( -1 );
	}
	
	/* create file header */
	fputs( "CPE", out_fp );
	fputc( 0x01, out_fp );
	
	/* select unit 0 */
	fputc( 0x08, out_fp );	/* chunk ID */
	fputc( 0x00, out_fp );	/* unit number */
	
	/* set entrypoint */
	fputc( 0x03, out_fp );	/* chunk ID */
	fputc( 0x90, out_fp );	/* entry point type */
	fputc( 0x00, out_fp );
	fwrite( &head.prg_entry_addr, 1, 4, out_fp );
	
	/* copy the program chunks */
	for( i=0; i<head.prg_entry_count; i++ )
	{
		if( prg_heads[i].flags == 4 )
		{
			continue;
		}
		
		/* seek to location of program chunk */
		fseek( in_fp, prg_heads[i].p_offset, SEEK_SET );
		
		/* create load chunk */
		fputc( 0x01, out_fp );	/* chunk ID */
		fwrite( &prg_heads[i].p_vaddr, 1, 4, out_fp );	/* load address */
		fwrite( &prg_heads[i].p_filesz, 1, 4, out_fp );	/* load length */
		
		/* copy the chunk data */
		copydata( in_fp, out_fp, prg_heads[i].p_filesz );
	}	
	
	/* end of file chunk */
	fputc( 0x00, out_fp );
	
	/* close files */
	fclose( in_fp );
	fclose( out_fp );
	
	return( 0 );
	
} /* convertELF */

int main( int argc, char *argv[] )
{
	int i;
	
	quiet = false;
	
	/* parse program arguments */
	for( i=1; i<argc; i++ )
	{
		if( strcasecmp( "-q", argv[i] ) == 0 )
		{
			quiet = true;
		}
		else
		{
			if( in_file == NULL )
			{
				in_file = argv[i];
			}
			else if( out_file == NULL )
			{
				out_file = argv[i];
			}
		}
	}
	
	/* print banner if not in quiet mode */
	if( !quiet )
	{
		printf( "PSn00bSDK elf2cpe - Experimental ELF to CPE executable "
			"converter\n" );
		printf( "2021 Meido-Tek Productions\n\n" );
	}
	
	/* print usage parameters if no arguments specified */
	if( argc == 1 )
	{
		printf( "Usage:\n" );
		printf( "  elf2cpe [-q] <elf_file> [cpe_file]\n" );
		return( 0 );
	}
	
	if( !in_file )
	{
		printf( "No input file specified.\n" );
		return( 0 );
	}
	
	return( convertELF() );
	
} /* main */