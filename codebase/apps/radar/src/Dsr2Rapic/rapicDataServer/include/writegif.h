#ifndef	__WRITEGIF_H
#define __WRITEGIF_H

/*
 * wrgif.h
 * 
 * Header for wrgif.C
 * 
 * 
 */

#include <stdio.h>

typedef long INT32;
typedef short INT16; 
typedef INT16 code_int;		/* must hold -1 .. 2**MAX_LZW_BITS */
typedef INT32 hash_entry;	/* must hold (code_int<<8) | byte */

class writegif {
  /* State for packing variable-width codes into a bitstream */
  int n_bits;			/* current number of bits/code */
  code_int maxcode;		/* maximum code, given n_bits */
  int init_bits;		/* initial n_bits ... restored after clear */
  INT32 cur_accum;		/* holds bits not yet output */
  int cur_bits;			/* # of bits in cur_accum */

  /* LZW string construction */
  code_int waiting_code;	/* symbol not yet output; may be extendable */
  int first_byte;		/* if TRUE, waiting_code is not valid */

  /* State for LZW code assignment */
  code_int ClearCode;		/* clear code (doesn't change) */
  code_int EOFCode;		/* EOF code (ditto) */
  code_int free_code;		/* first not-yet-used symbol code */

  /* LZW hash table */
  code_int *hash_code;		/* => hash table of symbol codes */
  hash_entry *hash_value;	/* => hash table of symbol values */

  /* GIF data packet construction buffer */
  int bytesinpkt;		/* # of bytes in current packet */
  char packetbuf[256];		/* workspace for accumulating packet */
  
  int error;
  char *color_map;
  int num_colors;
  int output_width, output_height;
  char filename[256];
  FILE *output_file;
  
  void output(code_int code);
  void clear_hash();
  void clear_block();
  void compress_init(int i_bits);
  void compress_byte(int c);
  void compress_term();
  void put_word(unsigned int w);
  void put_3bytes(int val);
  void emit_header ();
  void flush_packet();
  void char_out(char c)
  {
    packetbuf[++bytesinpkt] = c;
    if (bytesinpkt >= 255)
      flush_packet();
  }

public:  
  void put_pixel_row(char *rowdata);
  void finish_output_gif();
  void init(int width, int height, char *giffilename, 
    int num_colors = 0, char *colormap = 0);
  int  err() {return error;};
  writegif(int width, int height, char *giffilename, 
    int num_colors = 0, char *colormap = 0);
  ~writegif();
};

 
#endif	/* __WRITEGIF_H */
