#ifndef	__WRITEGIF_H
#define __WRITEGIF_H

/*
 * wrgif.h
 * 
 * Header for wrgif.C
 * 
 * 
 */

typedef long INT32;
typedef short INT16; 
typedef INT16 code_int;		/* must hold -1 .. 2**MAX_LZW_BITS */

class gifencode {
//  struct djpeg_dest_struct pub;	/* public fields */

//  j_decompress_ptr cinfo;	/* back link saves passing separate parm */

  /* State for packing variable-width codes into a bitstream */
  int n_bits;			/* current number of bits/code */
  code_int maxcode;		/* maximum code, given n_bits */
  int init_bits;		/* initial n_bits ... restored after clear */
  INT32 cur_accum;		/* holds bits not yet output */
  int cur_bits;			/* # of bits in cur_accum */

  /* LZW string construction */
  code_int waiting_code;	/* symbol not yet output; may be extendable */
  boolean first_byte;		/* if TRUE, waiting_code is not valid */

  /* State for LZW code assignment */
  code_int ClearCode;		/* clear code (doesn't change) */
  code_int EOFCode;		/* EOF code (ditto) */
  code_int free_code;		/* first not-yet-used symbol code */

  /* LZW hash table */
  code_int *hash_code;		/* => hash table of symbol codes */
  hash_entry FAR *hash_value;	/* => hash table of symbol values */

  /* GIF data packet construction buffer */
  int bytesinpkt;		/* # of bytes in current packet */
  char packetbuf[256];		/* workspace for accumulating packet */

  void flush_packet();
  void char_out(char c)
  {
    packetbuf[++(dinfo)->bytesinpkt] = c;
    if (bytesinpkt >= 255)
	      flush_packet(dinfo);
  }
  void output(code_int code);
  void clear_hash();
  void clear_block();
  void compress_init(int i_bits);
  void compress_byte(int c);
  void compress_term();
  void put_word(unsigned int w);
  void put_3bytes(int val);
  void emit_header (int num_colors, char *colormap);
  void start_output_gif(int num_colors, char *colormap);
  void put_pixel_row(char *rowdata)
  void finish_output_gif();
  void init(int width, int height, char *giffilename = 0, 
    int num_colors = 0, char *colormap = 0);
}

 
#endif	/* __WRITEGIF_H */
