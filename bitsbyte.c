/*
	Filename:  BITSBYTE.C (buffered bits-byte i/o)
	Author:    Gerald R. Tamayo
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>  /* C99 */
#include "bitsbyte.h"

FILE *gIN = NULL, *pOUT = NULL;
unsigned int pBUFSIZE = 8192, gBUFSIZE = 8192;
unsigned char *pbuf = NULL, *pbuf_start = NULL, p_cnt = 0;
unsigned char *gbuf = NULL, *gbuf_start = NULL, *gbuf_end = NULL, g_cnt = 0;
unsigned int pbuf_count = 0, nfread = 0, bit_read = 0;
int64_t nbytes_out = 0, nbytes_read = 0;

/* Opens the input file. */
int open_input_file( char arg[] )
{
	if ( (gIN = fopen( arg, "rb" )) == NULL ) {
		fprintf(stderr, "\nError opening input file, %s.", arg );
		return 0;
	}
	else return 1;
}

/* Opens the output file. */
int open_output_file( char arg[] )
{
	if ( (pOUT = fopen( arg, "wb" )) == NULL ) {
		fprintf(stderr, "\nError opening output file, %s.", arg );
		return 0;
	}
	else return 1;
}

void init_buffer_sizes( unsigned int size )
{
	pBUFSIZE = gBUFSIZE = size;
}

void init_put_buffer( void )
{
	pbuf = NULL;
	pbuf_start = NULL;
	pbuf_count = 0;
	nbytes_out = 0;
	
	/* Allocate MEMORY for BUFFERS. */
	while ( 1 ) {
		pbuf = (unsigned char *) malloc( sizeof(char) * pBUFSIZE );
		if ( pbuf ) {
			pbuf_start = pbuf;
			break;
		}
		else {
			pBUFSIZE -= 1024;
			if ( pBUFSIZE == 0 ) {
				fprintf(stderr, "\nmemory allocation error!");
				exit(0);
			}
		}
	}
	memset( pbuf, 0, pBUFSIZE );
}

void init_get_buffer( void )
{
	gbuf = NULL;
	gbuf_start = NULL;
	gbuf_end = NULL;
	nfread = 0;
	nbytes_read = 0;
	
	/* Allocate MEMORY for BUFFERS. */
	while ( 1 ) {
		gbuf = (unsigned char *) malloc( sizeof(char) * gBUFSIZE );
		if ( gbuf ) {
			gbuf_start = gbuf;
			break;
		}
		else {
			gBUFSIZE -= 1024;
			if ( gBUFSIZE == 0 ) {
				fprintf(stderr,"\nmemory allocation error!");
				exit(0);
			}
		}
	}
	nfread = fread ( gbuf, 1, gBUFSIZE, gIN );
	gbuf_end = (unsigned char *) (gbuf + nfread);
}

void free_put_buffer( void )
{
	pbuf = pbuf_start;
	if ( pbuf ) free( pbuf );
}

void free_get_buffer( void )
{
	gbuf = gbuf_start;
	if ( gbuf ) free( gbuf );
}

void flush_put_buffer( void )
{
	if ( pbuf_count || p_cnt ) {
		fwrite( pbuf_start, pbuf_count+(p_cnt?1:0), 1, pOUT );
		nbytes_out += (pbuf_count+(p_cnt?1:0));
		pbuf = pbuf_start; pbuf_count = 0; p_cnt = 0;
		memset( pbuf, 0, pBUFSIZE );
	}
}

/* Gets a bit (1/0) */
static inline int get_bit( void )
{
	if ( nfread ){
		if ( (*gbuf) & (1<<(g_cnt++)) ) bit_read = 1;
		else bit_read = 0;

		if ( g_cnt == 8 ) { /* finished 8 bits? */
			g_cnt = 0;        /* reset to zero. */
			if ( (++gbuf) == gbuf_end ) { /* end of buffer? */
				nbytes_read += nfread;
				/* then fill buffer again. */
				gbuf = gbuf_start;
				nfread = fread ( gbuf, 1, gBUFSIZE, gIN );
				gbuf_end = (unsigned char *) (gbuf + nfread);
			}
		}
	}
	else return EOF;

	/* allows further processing of the previous bit. */
	return bit_read;
}

/* Gets a byte from the input buffer.

	NOTE:

	Do not mix gfgetc() with the get_bit() function in one program.
	Call get_symbol(8) or get_nbits(8) instead to get an 8-bit byte value.
	Unless you are in a byte boundary or you force it, call advance_gbuf():
	
	if ( g_cnt > 0 && g_cnt < 8 ) {
		g_cnt = 7;
		advance_gbuf();
	}
	
	Same as in mixing pfputc() with put_ONE() and put_ZERO().
	Force byte boundary. Write current byte *pbuf by advancing the pbuf buffer:
	
	if ( p_cnt > 0 && p_cnt < 8 ) {
		p_cnt = 7;
		advance_buf();
	}
*/
static inline int gfgetc( void )
{
	int c;
	
	if ( nfread ){
		c = (int) (*gbuf++);
		if ( gbuf == gbuf_end ) {
			nbytes_read += nfread;
			gbuf = gbuf_start;
			nfread = fread ( gbuf, 1, gBUFSIZE, gIN );
			gbuf_end = (unsigned char *) (gbuf + nfread);
		}
		return c;
	}
	else return EOF;
}

/* Puts a byte into the output buffer. */
static inline void pfputc( int c )
{
	*pbuf++ = (unsigned char) c;
	if ( (++pbuf_count) == pBUFSIZE ){
		fwrite( pbuf_start, pBUFSIZE, 1, pOUT );
		nbytes_out += pBUFSIZE;
		pbuf_count = 0;
		pbuf = pbuf_start;
		memset( pbuf, 0, pBUFSIZE );
	}
}

int64_t get_nbytes_out( void )
{
	return ( nbytes_out + pbuf_count+(p_cnt?1:0) );
}
/* nbytes_out = get_nbytes_out(); */

int64_t get_nbytes_read( void )
{
	return ( nbytes_read + nfread );
}
/* nbytes_read = get_nbytes_read(); */
