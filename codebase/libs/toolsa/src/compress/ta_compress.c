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
 * ta_compress.c
 *
 * Generic compression utilities.
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, USA
 *
 * July 1999
 *
 **********************************************************************/

#include <toolsa/compress.h>
#include <toolsa/umisc.h>
#include <dataport/bigend.h>

/**********************************************************************
 * ta_is_compressed() - tests whether buffer is compressed using toolsa
 *                      compression
 *
 * Safer than ta_compressed(). Use recommended.
 *
 * Returns TRUE or FALSE
 *
 **********************************************************************/

int ta_is_compressed(const void *compressed_buffer,
                     int compressed_len)
     
{

  if (compressed_len < sizeof(compress_buf_hdr_t)) {
    return FALSE;
  }

  /*
   * check the magic cookie
   */

  ui32 magic_cookie;
  memcpy(&magic_cookie, compressed_buffer, sizeof(ui32));
  BE_to_array_32(&magic_cookie, sizeof(ui32));

  /*
   * Note that we also test for several NOT_COMPRESSED
   * cases, which may seem a little odd, but what we're
   * really testing for is if this is a compression header, and
   * if the magic cookie is one of these NOT_COMPRESSED cases, then
   * it is (otherwise we won't strip the header off on decompress).
   */

  if (magic_cookie != LZO_COMPRESSED &&
      magic_cookie != BZIP_COMPRESSED &&
      magic_cookie != GZIP_COMPRESSED &&
      magic_cookie != LZO_NOT_COMPRESSED &&
      magic_cookie != BZIP_NOT_COMPRESSED &&
      magic_cookie != GZIP_NOT_COMPRESSED &&
      magic_cookie != RLE_COMPRESSED &&
      magic_cookie != _RLE_COMPRESSED &&
      magic_cookie != __RLE_COMPRESSED &&
      magic_cookie != ZLIB_COMPRESSED &&
      magic_cookie != ZLIB_NOT_COMPRESSED) {
    return FALSE;
  }

  if (magic_cookie == RLE_COMPRESSED) {

    if (compressed_len < 20) {
      return FALSE;
    }
    
    ui32 key;
    memcpy(&key, (char *) compressed_buffer + sizeof(ui32), sizeof(ui32));
    BE_to_array_32(&key, sizeof(ui32));
    if (key != 255) {
      return FALSE;
    }

  } else {

    if (compressed_len < sizeof(compress_buf_hdr_t)) {
      return FALSE;
    }

    compress_buf_hdr_t hdr;
    memcpy(&hdr, compressed_buffer, sizeof(hdr));
    BE_to_array_32(&hdr, sizeof(hdr));

    if (hdr.nbytes_compressed - hdr.nbytes_coded != 24) {
      return FALSE;
    }
    if (hdr.spare[0] != 0 || hdr.spare[1] != 0) {
      return FALSE;
    }

  }

  return TRUE;
    
}

/**********************************************************************
 * ta_compression_method() - returns type of ta compression
 *
 * Use after calling ta_is_compressed() to get the compression type.
 *
 **********************************************************************/

ta_compression_method_t ta_compression_method(const void *compressed_buffer,
                                              int compressed_len)
     
{

  if (compressed_len < sizeof(compress_buf_hdr_t)) {
    return TA_COMPRESSION_NA;
  }

  /*
   * check the magic cookie
   */

  ui32 magic_cookie;
  memcpy(&magic_cookie, compressed_buffer, sizeof(ui32));
  BE_to_array_32(&magic_cookie, sizeof(ui32));
  
  if (magic_cookie == TA_NOT_COMPRESSED) {
    return TA_COMPRESSION_NONE;
  }

  if (magic_cookie == RLE_COMPRESSED ||
      magic_cookie == _RLE_COMPRESSED ||
      magic_cookie == __RLE_COMPRESSED) {
    return TA_COMPRESSION_RLE;
  }

  if (magic_cookie == LZO_COMPRESSED ||
      magic_cookie == LZO_NOT_COMPRESSED) {
    return TA_COMPRESSION_LZO;
  }

  if (magic_cookie == ZLIB_COMPRESSED ||
      magic_cookie == ZLIB_NOT_COMPRESSED) {
    return TA_COMPRESSION_ZLIB;
  }

  if (magic_cookie == GZIP_COMPRESSED ||
      magic_cookie == GZIP_NOT_COMPRESSED) {
    return TA_COMPRESSION_GZIP;
  }

  if (magic_cookie == BZIP_COMPRESSED ||
      magic_cookie == BZIP_NOT_COMPRESSED) {
    return TA_COMPRESSION_BZIP;
  }

  return TA_COMPRESSION_NA;

}

/**********************************************************************
 * ta_compressed() - tests whether buffer is compressed using toolsa
 *                   compression
 *
 * Not safe - assumes there are at least 4 bytes in the buffer.
 * Only checks the magic cookie.
 *
 * deprecated - use ta_is_compressed() instead.
 *
 * Returns TRUE or FALSE
 *
 **********************************************************************/

int ta_compressed(const void *compressed_buffer)
     
{

  ui32 magic_cookie;

  memcpy(&magic_cookie, compressed_buffer, sizeof(ui32));
  BE_to_array_32(&magic_cookie, sizeof(ui32));

  /*
   * Note that we also test for several NOT_COMPRESSED
   * cases, which may seem a little odd, but what we're
   * really testing for is if this is a compression header, and
   * if the magic cookie is one of these NOT_COMPRESSED cases, then
   * it is (otherwise we won't strip the header off on decompress).
   *
   */

  if (magic_cookie == LZO_COMPRESSED ||
      magic_cookie == BZIP_COMPRESSED ||
      magic_cookie == GZIP_COMPRESSED ||
      magic_cookie == LZO_NOT_COMPRESSED ||
      magic_cookie == BZIP_NOT_COMPRESSED ||
      magic_cookie == GZIP_NOT_COMPRESSED ||
      magic_cookie == RLE_COMPRESSED ||
      magic_cookie == _RLE_COMPRESSED ||
      magic_cookie == __RLE_COMPRESSED ||
      magic_cookie == ZLIB_COMPRESSED ||
      magic_cookie == ZLIB_NOT_COMPRESSED) {

    return TRUE;

  } else {

    return FALSE;

  }
    
}

/**********************************************************************
 * ta_compression_debug() - prints info to stderr about a buffer.
 *
 * void, no values returned. Info includes compression type, length.
 *
 **********************************************************************/

extern void ta_compression_debug(const void *compressed_buffer){

  ui32 magic_cookie;

  memcpy(&magic_cookie, compressed_buffer, sizeof(ui32));
  BE_to_array_32(&magic_cookie, sizeof(ui32));

  switch ( magic_cookie) {

  case LZO_COMPRESSED :
    fprintf(stderr, "Compression type : LZO_COMPRESSED\n");
    break;

  case LZO_NOT_COMPRESSED :
    fprintf(stderr, "Compression type : LZO_NOT_COMPRESSED\n");
    break;

  case BZIP_COMPRESSED :
    fprintf(stderr, "Compression type : BZIP_COMPRESSED\n");
    break;

  case BZIP_NOT_COMPRESSED :
    fprintf(stderr, "Compression type : BZIP_NOT_COMPRESSED\n");
    break;

  case GZIP_COMPRESSED :
    fprintf(stderr, "Compression type : GZIP_COMPRESSED\n");
    break;

  case GZIP_NOT_COMPRESSED :
    fprintf(stderr, "Compression type : GZIP_NOT_COMPRESSED\n");
    break;

  case RLE_COMPRESSED :
    fprintf(stderr, "Compression type : RLE_COMPRESSED\n");
    break;

  case _RLE_COMPRESSED :
    fprintf(stderr, "Compression type : _RLE_COMPRESSED\n");
    break;

  case __RLE_COMPRESSED :
    fprintf(stderr, "Compression type : __RLE_COMPRESSED\n");
    break;

  case ZLIB_COMPRESSED :
    fprintf(stderr, "Compression type : ZLIB_COMPRESSED\n");
    break;

  case ZLIB_NOT_COMPRESSED :
    fprintf(stderr, "Compression type : ZLIB_NOT_COMPRESSED\n");
    break;

  default :
    fprintf(stderr, "Compression type : UNKOWN\n");
    return;
    break;

  }

  /*
   * Print header info - depends on if we're RLE or not.
   */

  if (
      (magic_cookie == __RLE_COMPRESSED) ||
      (magic_cookie == _RLE_COMPRESSED) ||
      (magic_cookie == RLE_COMPRESSED)
      ){

    ui32 key;
    ui32 nbytes_buffer;
    ui32 nbytes_uncompressed;
    ui32 nbytes_compressed;

    memcpy(&key, 
	   (unsigned char *) compressed_buffer + sizeof(ui32),
	   sizeof(ui32));
    BE_to_array_32(&key, sizeof(ui32));


    memcpy(&nbytes_buffer, 
	   (unsigned char *) compressed_buffer + 2*sizeof(ui32),
	   sizeof(ui32));
    BE_to_array_32(&nbytes_buffer, sizeof(ui32));

    memcpy(&nbytes_uncompressed, 
	   (unsigned char *) compressed_buffer + 3*sizeof(ui32),
	   sizeof(ui32));
    BE_to_array_32(&nbytes_uncompressed, sizeof(ui32));

    memcpy(&nbytes_compressed, 
	   (unsigned char *) compressed_buffer + 4*sizeof(ui32),
	   sizeof(ui32));
    BE_to_array_32(&nbytes_compressed, sizeof(ui32));


    fprintf(stderr,"key : %d\n", key);
    fprintf(stderr,"nbytes buffer : %d\n", nbytes_buffer);
    fprintf(stderr,"nbytes uncompressed : %d\n", nbytes_uncompressed);
    fprintf(stderr,"nbytes compressed : %d\n", nbytes_compressed);
    
  } else {
    
    ui32 nbytes_uncompressed;
    ui32 nbytes_compressed;
    ui32 nbytes_coded;
    memcpy(&nbytes_uncompressed, 
	   (unsigned char *) compressed_buffer + sizeof(ui32),
	   sizeof(ui32));
    BE_to_array_32(&nbytes_uncompressed, sizeof(ui32));

    memcpy(&nbytes_compressed, 
	   (unsigned char *) compressed_buffer + 2*sizeof(ui32),
	   sizeof(ui32));
    BE_to_array_32(&nbytes_compressed, sizeof(ui32));

    memcpy(&nbytes_coded, 
	   (unsigned char *) compressed_buffer + 3*sizeof(ui32),
	   sizeof(ui32));
    BE_to_array_32(&nbytes_coded, sizeof(ui32));

    fprintf(stderr,"nbytes uncompressed : %d\n", nbytes_uncompressed);
    fprintf(stderr,"nbytes compressed : %d\n", nbytes_compressed);
    fprintf(stderr,"nbytes coded : %d\n", nbytes_coded);

  }

  return;
}

/**********************************************************************
 * ta_gzip_buffer() - tests whether buffer is gzip type
 *
 * Returns TRUE or FALSE
 **********************************************************************/

int ta_gzip_buffer(const void *compressed_buffer)
     
{

  ui32 magic_cookie;
  memcpy(&magic_cookie, compressed_buffer, sizeof(ui32));
  if (magic_cookie == GZIP_COMPRESSED ||
      magic_cookie == GZIP_NOT_COMPRESSED) {
    return TRUE;
  } else {
    return FALSE;
  }

}

/**********************************************************************
 * ta_compress()
 *
 * Compress according to the compression method.
 *
 * The memory for the encoded buffer is allocated by this routine,
 * and passed back to the caller.
 * This should be freed by the calling routine using ta_compress_free().
 *
 * The length of the compressed data buffer (*nbytes_compressed_p) is set.
 *
 * Returns pointer to the encoded buffer, NULL on error.
 *
 **********************************************************************/

void *ta_compress(ta_compression_method_t method,
		  const void *uncompressed_buffer,
		  unsigned int nbytes_uncompressed,
		  unsigned int *nbytes_compressed_p)

{

  switch (method) {

  case TA_COMPRESSION_NONE:
    return (_ta_no_compress(TA_NOT_COMPRESSED,
			    uncompressed_buffer,
			    nbytes_uncompressed,
			    nbytes_compressed_p));
    break;
    
  case TA_COMPRESSION_RLE:
    // uze gzip
    return (zlib_compress(uncompressed_buffer,
                          nbytes_uncompressed, nbytes_compressed_p));
    break;
    
#ifndef EXCLUDE_LZO
  case TA_COMPRESSION_LZO:
    // uze gzip
    return (zlib_compress(uncompressed_buffer,
			 nbytes_uncompressed, nbytes_compressed_p));
    break;
#endif

  case TA_COMPRESSION_ZLIB:
    // uze gzip
    return (zlib_compress(uncompressed_buffer,
			  nbytes_uncompressed, nbytes_compressed_p));
    break;
    
  case TA_COMPRESSION_BZIP:
    return (bzip_compress(uncompressed_buffer,
			  nbytes_uncompressed, nbytes_compressed_p));
    break;
    
  case TA_COMPRESSION_GZIP:
    return (gzip_compress(uncompressed_buffer,
			  nbytes_uncompressed, nbytes_compressed_p));
    break;

  default:
    fprintf(stderr, "ERROR - ta_compress\n");
    fprintf(stderr, "  Unsupported compression method: %d\n", method);
    return NULL;

  }

}

/**********************************************************************
 * ta_decompress() - toolsa generic decompression
 *
 * Perform generic decompression on buffer created using
 *   rle_compress(), lzo_compress() or bzip_compress().
 *
 * Switches on the magic cookie in the header.
 *
 * The memory for the uncompressed data buffer is allocated by this routine.
 * This should be freed by the calling routine using ta_decompress_free();
 *
 * On success, returns pointer to the uncompressed data buffer.
 * Also, *nbytes_uncompressed_p is set.
 *
 * On failure, returns NULL.
 *
 **********************************************************************/

void *ta_decompress(const void *compressed_buffer,
		    unsigned int *nbytes_uncompressed_p)
     
{

  ui32 magic_cookie;

  memcpy(&magic_cookie, compressed_buffer, sizeof(ui32));
  BE_to_array_32(&magic_cookie, sizeof(ui32));

  if (magic_cookie == TA_NOT_COMPRESSED) {

    compress_buf_hdr_t hdr;
    char *uncompressed_data;
    char *compressed_data;
    
    memcpy(&hdr, compressed_buffer, sizeof(compress_buf_hdr_t));
    BE_to_array_32(&hdr, sizeof(compress_buf_hdr_t));
    
    uncompressed_data = (char *) umalloc_min_1 (hdr.nbytes_uncompressed);
    *nbytes_uncompressed_p = hdr.nbytes_uncompressed;
    
    compressed_data = (char *) compressed_buffer + sizeof(compress_buf_hdr_t);
    memcpy(uncompressed_data, compressed_data, hdr.nbytes_uncompressed);
    return (uncompressed_data);

#ifndef EXCLUDE_LZO

  } else if (magic_cookie == LZO_COMPRESSED ||
	     magic_cookie == LZO_NOT_COMPRESSED) {
    
    /* fprintf(stderr, "LZO decompression\n"); */
    
    return (lzo_decompress(compressed_buffer, nbytes_uncompressed_p));
    
#endif

  } else if (magic_cookie == BZIP_COMPRESSED ||
	     magic_cookie == BZIP_NOT_COMPRESSED) {
    
    /* fprintf(stderr, "BZIP decompression\n"); */

    return (bzip_decompress(compressed_buffer, nbytes_uncompressed_p));
    
  } else if (magic_cookie == GZIP_COMPRESSED ||
	     magic_cookie == GZIP_NOT_COMPRESSED) {
    
    /* fprintf(stderr, "GZIP decompression\n"); */

    return (gzip_decompress(compressed_buffer, nbytes_uncompressed_p));
    
  } else if (magic_cookie == RLE_COMPRESSED ||
	     magic_cookie == _RLE_COMPRESSED ||
	     magic_cookie == __RLE_COMPRESSED) {
    
    /* fprintf(stderr, "RLE decompression\n"); */

    return (rle_decompress(compressed_buffer, nbytes_uncompressed_p));

  } else if (magic_cookie == ZLIB_COMPRESSED ||
	     magic_cookie == ZLIB_NOT_COMPRESSED) {
    
    /* fprintf(stderr, "ZLIB decompression\n"); */

    return (zlib_decompress(compressed_buffer, nbytes_uncompressed_p));
    
  }

  *nbytes_uncompressed_p = 0;
  return (NULL);

}

/**********************************************************************
 * ta_compress_free() - free up buffer allocated by any ta_compress
 * routines
 *
 */

void ta_compress_free(void *buffer)

{
  ufree(buffer);
}


/*****************************************************
 * _ta_no_compress()
 *
 * Load up compressed_buffer with uncompressed data
 *
 * returns output buffer of header plus original data
 *
 * private routine for use only by other compression routines.
 */

void *_ta_no_compress(unsigned int magic_cookie,
		      const void *uncompressed_buffer,
		      unsigned int nbytes_uncompressed,
		      unsigned int *nbytes_compressed_p)
     
{
 
  unsigned int nbytes_buffer =
    nbytes_uncompressed + sizeof(compress_buf_hdr_t);
  void *out_buffer = umalloc_min_1(nbytes_buffer);
  compress_buf_hdr_t *hdr = (compress_buf_hdr_t *) out_buffer;
  
  /*
   * load header and swap
   */
  
  MEM_zero(*hdr);
  hdr->magic_cookie = magic_cookie;
  hdr->nbytes_uncompressed = nbytes_uncompressed;
  hdr->nbytes_compressed = nbytes_buffer;
  hdr->nbytes_coded = nbytes_uncompressed;
  BE_from_array_32(hdr, sizeof(compress_buf_hdr_t));
  
  if (nbytes_compressed_p != NULL) {
    *nbytes_compressed_p = nbytes_buffer;
  }
  
  /*
   * load buffer
   */
  
  memcpy((char *) out_buffer + sizeof(compress_buf_hdr_t),
	 uncompressed_buffer, nbytes_uncompressed);
  
  /*
   * swap header
   */
  
  return (out_buffer);

}

