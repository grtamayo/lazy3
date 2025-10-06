/* BITSBYTE.H */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>  /* C99 */

#if !defined( BITSBYTE_H )
	#define BITSBYTE_H

#define pset_bit() *pbuf |= (1<<p_cnt)

/* ---- writes a ONE (1) bit. ---- */
#define put_ONE() { pset_bit(); advance_buf(); }

/* ---- writes a ZERO (0) bit. ---- */
#define put_ZERO() advance_buf()

/* just increment the pbuf buffer for faster processing. */
#define advance_buf()		\
{                          \
	if ( (++p_cnt) == 8 ) { \
		p_cnt = 0; \
		if ( (++pbuf_count) == pBUFSIZE ){ \
			pbuf = pbuf_start; \
			fwrite( pbuf, pBUFSIZE, 1, pOUT ); \
			memset( pbuf, 0, pBUFSIZE ); \
			pbuf_count = 0; \
			nbytes_out += pBUFSIZE; \
		} \
		else pbuf++; \
	} \
}

/* increment the gbuf buffer. */
#define advance_gbuf()   \
{                        \
	if ( (++g_cnt) == 8 ){   \
		g_cnt = 0;   \
		if ( ++gbuf == gbuf_end ) {   \
			nbytes_read += nfread;   \
			gbuf = gbuf_start;   \
			nfread = fread ( gbuf, 1, gBUFSIZE, gIN );   \
			gbuf_end = (unsigned char *) (gbuf + nfread);   \
		}   \
	}   \
}

extern FILE *gIN, *pOUT;
extern unsigned int pBUFSIZE, gBUFSIZE;
extern unsigned char *pbuf, *pbuf_start, p_cnt;
extern unsigned char *gbuf, *gbuf_start, *gbuf_end, g_cnt;
extern unsigned int pbuf_count, nfread, bit_read;
extern int64_t nbytes_out, nbytes_read;

int open_input_file( char arg[] );
int open_output_file( char arg[] );
void init_buffer_sizes( unsigned int size );
void init_put_buffer( void );
void init_get_buffer( void );
void free_put_buffer( void );
void free_get_buffer( void );
void flush_put_buffer( void );
static inline int  get_bit( void );
static inline int  gfgetc( void );
static inline void pfputc( int c );
int64_t get_nbytes_out( void );
int64_t get_nbytes_read( void );

#endif
