/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/**********************************************************************
 * lzo_compress.c
 *
 * Compression utilities using LZO compression
 *
 * See doc/README.LZO
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, USA
 *
 * July 1999
 *
 **********************************************************************/

#include <toolsa/compress.h>
#include <toolsa/umisc.h>
#include <dataport/bigend.h>
#include "minilzo.h"

/* #define DEBUG_PRINT */

/* Work-memory needed for compression. Allocate memory in units
 * of `long' (instead of `char') to make sure it is properly aligned.
 */

#define HEAP_ALLOC(var,size) \
	long __LZO_MMODEL var [ ((size) + (sizeof(long) - 1)) / sizeof(long) ]

static HEAP_ALLOC(Wrkmem, LZO1X_1_MEM_COMPRESS);

/**********************************************************************
 * lzo_compress()
 *
 * In the compressed data, the first 24 bytes are a header as follows:
 *
 *   (ui32) magic_cookie - LZO_COMPRESSED or LZO_NOT_COMPRESSED
 *   (ui32) nbytes_uncompressed
 *   (ui32) nbytes_compressed - including this header
 *   (ui32) nbytes_coded - (nbytes_compressed - sizeof header)
 *   (ui32) spare
 *   (ui32) spare
 *
 * The header is in BE byte order.
 *
 * If the buffer is not compressed, magic_cookie is set to LZO_NOT_COMPRESSED.
 *
 * The compressed data follows the header.
 *
 * The memory for the encoded buffer is allocated by this routine,
 * and passed back to the caller.
 * This should be freed by the calling routine using ta_compress_free();
 *
 * The length of the compressed data buffer (*nbytes_compressed_p) is set.
 *
 * Returns pointer to the encoded buffer.
 *
 **********************************************************************/

void *lzo_compress(const void *uncompressed_buffer,
		   unsigned int nbytes_uncompressed,
		   unsigned int *nbytes_compressed_p)
     
{

  int iret;
  unsigned int nbytes_buffer;
  unsigned char *compressed_buffer;
  unsigned char *truncated_buffer;
  unsigned int nbytes_alloc;
  compress_buf_hdr_t *hdr;
  lzo_uint out_len;
 
  /*
   * initial allocation of encoded buffer, the size of the original buffer
   * plus the extra bytes needed for working space.
   */
  
  nbytes_alloc = (nbytes_uncompressed + sizeof(compress_buf_hdr_t) +
		  nbytes_uncompressed / 64 + 16 + 3);
  
  compressed_buffer = (unsigned char *) umalloc_min_1 (nbytes_alloc);
  
  /*
   * initialize the LZO library
   */

  if (lzo_init() != LZO_E_OK) {
    
    /*
     * load buffer with uncompressed data
     */

#ifdef DEBUG_PRINT
  fprintf(stderr, "LZO init failed\n");
#endif

    ufree(compressed_buffer);
    return(_ta_no_compress(LZO_NOT_COMPRESSED,
			   uncompressed_buffer,
			   nbytes_uncompressed,
			   nbytes_compressed_p));
    
  }

  /*
   * compress with LZO1X-1
   */
  
  iret = lzo1x_1_compress(uncompressed_buffer,
			  nbytes_uncompressed,
			  compressed_buffer + sizeof(compress_buf_hdr_t),
			  &out_len,
			  Wrkmem);

  if (iret != LZO_E_OK || out_len >= nbytes_uncompressed) {
    
#ifdef DEBUG_PRINT
    if (iret != LZO_E_OK) {
      fprintf(stderr, "LZO compress failed\n");
    }
    if (out_len >= nbytes_uncompressed) {
      fprintf(stderr, "LZO failed to reduce size\n");
      fprintf(stderr, "  Uncompressed size: %d\n", nbytes_uncompressed);
      fprintf(stderr, "  Compressed size: %d\n", out_len);
    }
#endif

    /*
     * compression failed or data not compressible
     */
    
    ufree(compressed_buffer);
    return(_ta_no_compress(LZO_NOT_COMPRESSED,
			   uncompressed_buffer,
			   nbytes_uncompressed,
			   nbytes_compressed_p));
    
  }

  /*
   * compression worked
   */

  /*
   * truncate buffer
   */
  
  nbytes_buffer = sizeof(compress_buf_hdr_t) + out_len;
  truncated_buffer = urealloc(compressed_buffer, nbytes_buffer);

#ifdef DEBUG_PRINT
  fprintf(stderr, "LZO compress succeeded\n");
  fprintf(stderr, "  Uncompressed size: %d\n", nbytes_uncompressed);
  fprintf(stderr, "  Compressed size: %d\n", out_len);
#endif

  /*
   * load hdr and swap
   */

  hdr = (compress_buf_hdr_t *) truncated_buffer;
  MEM_zero(*hdr);
  hdr->magic_cookie = LZO_COMPRESSED;
  hdr->nbytes_uncompressed = nbytes_uncompressed;
  hdr->nbytes_compressed = nbytes_buffer;
  hdr->nbytes_coded = out_len;
  BE_from_array_32(hdr, sizeof(compress_buf_hdr_t));
  
  if (nbytes_compressed_p != NULL) {
    *nbytes_compressed_p = nbytes_buffer;
  }
  
  return (truncated_buffer);

}

/**********************************************************************
 * lzo_decompress()
 *
 * Perform LZO decompression on buffer created using lzo_compress();
 *
 * The memory for the uncompressed data buffer is allocated by this routine.
 * This should be freed by the calling routine using ta_compress_free();
 *
 * On success, returns pointer to the uncompressed data buffer.
 * Also, *nbytes_uncompressed_p is set.
 *
 * On failure, returns NULL.
 *
 **********************************************************************/

void *lzo_decompress(const void *compressed_buffer,
		     unsigned int *nbytes_uncompressed_p)
     
{

  void *compressed_copy;
  unsigned char *uncompressed_data;
  unsigned char *compressed_data;
  int iret;
  lzo_uint out_len;
  compress_buf_hdr_t hdr;

  if (compressed_buffer == NULL) {
    *nbytes_uncompressed_p = 0;
    return (NULL);
  }

  /*
   * decode header
   */
  
  memcpy(&hdr, compressed_buffer, sizeof(compress_buf_hdr_t));
  BE_to_array_32(&hdr, sizeof(compress_buf_hdr_t));
  
  if (hdr.magic_cookie != LZO_COMPRESSED &&
      hdr.magic_cookie != LZO_NOT_COMPRESSED) {
    *nbytes_uncompressed_p = 0;
    return (NULL);
  }

  uncompressed_data =
    (unsigned char *) umalloc_min_1 (hdr.nbytes_uncompressed);
  *nbytes_uncompressed_p = hdr.nbytes_uncompressed;

  if (hdr.magic_cookie == LZO_NOT_COMPRESSED) {

#ifdef DEBUG_PRINT
    fprintf(stderr, "LZO decompress: data not compressed\n");
    fprintf(stderr, "  Returning uncompressed data\n");
    fprintf(stderr, "  Uncompressed size: %d\n", hdr.nbytes_uncompressed);
#endif
    
    compressed_data = (unsigned char *) compressed_buffer + sizeof(compress_buf_hdr_t);
    memcpy(uncompressed_data, compressed_data, hdr.nbytes_uncompressed);
    return (uncompressed_data);

  } else {

    /*
     * create compressed copy - for byte alignment
     */
    
    compressed_copy = umalloc_min_1(hdr.nbytes_coded);
    memcpy(compressed_copy,
	   (char *) compressed_buffer + sizeof(compress_buf_hdr_t),
	   hdr.nbytes_coded);

    iret = lzo1x_decompress(compressed_copy,
			    hdr.nbytes_coded,
			    uncompressed_data,
			    &out_len, NULL);

    ufree(compressed_copy);

    if (iret == LZO_E_OK && out_len == hdr.nbytes_uncompressed) {
#ifdef DEBUG_PRINT
      fprintf(stderr, "LZO decompress: success\n");
      fprintf(stderr, "  compressed_size: %d\n", hdr.nbytes_coded);
      fprintf(stderr, "  uncompressed_size: %d\n", out_len);
#endif
      return(uncompressed_data);
    } else {
#ifdef DEBUG_PRINT
      fprintf(stderr, "LZO decompress: failure\n");
#endif
      *nbytes_uncompressed_p = 0;
      ufree(uncompressed_data);
      return (NULL);
    }

  }

}
