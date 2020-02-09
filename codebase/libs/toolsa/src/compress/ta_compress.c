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
#define SI32_MAX 2147483647L
#define UI32_MAX 4294967295U

/**********************************************************************
 * Is this a valid magic cookie?
 * Note that we also test for several NOT_COMPRESSED
 * cases, which may seem a little odd, but what we're
 * really testing for is if this is a compression header, and
 * if the magic cookie is one of these NOT_COMPRESSED cases, then
 * it is (otherwise we won't strip the header off on decompress).
 **********************************************************************/

static int _cookie_is_valid(ui32 magic_cookie)
     
{
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
 * ta_is_compressed() - tests whether buffer is compressed using toolsa
 *                      compression
 *
 * Safer than ta_compressed(). Use recommended.
 *
 * Returns TRUE or FALSE
 *
 **********************************************************************/

int ta_is_compressed(const void *compressed_buffer,
                     ui64 compressed_len)
     
{

  ui32 magic_cookie;

  if (ta_is_compressed_64(compressed_buffer, compressed_len)) {
    return TRUE;
  }

  if (compressed_len < sizeof(compress_buf_hdr_t)) {
    return FALSE;
  }

  /*
   * check the magic cookie
   */

  memcpy(&magic_cookie, compressed_buffer, sizeof(ui32));
  BE_to_array_32(&magic_cookie, sizeof(ui32));
  if (!_cookie_is_valid(magic_cookie)) {
    return FALSE;
  }
  
  if (magic_cookie == RLE_COMPRESSED) {

    ui32 key;

    if (compressed_len < 20) {
      return FALSE;
    }
    
    memcpy(&key, (char *) compressed_buffer + sizeof(ui32), sizeof(ui32));
    BE_to_array_32(&key, sizeof(ui32));
    if (key != 255) {
      return FALSE;
    }

  } else {

    compress_buf_hdr_t hdr;

    if (compressed_len < sizeof(compress_buf_hdr_t)) {
      return FALSE;
    }

    memcpy(&hdr, compressed_buffer, sizeof(hdr));
    compress_buf_hdr_from_BE(&hdr);

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
  if (_cookie_is_valid(magic_cookie)) {
    return TRUE;
  } else {
    return FALSE;
  }
}

/**********************************************************************
 * ta_is_compressed_64() - tests whether buffer is compressed
 * using toolsa, compression, with 64-bit headers
 *
 * Returns TRUE or FALSE
 *
 **********************************************************************/

int ta_is_compressed_64(const void *compressed_buffer,
                        ui64 compressed_len)
     
{

  /* buffers with 64-bit compression will start with the
   * compress_buf_hdr_64_t struct:
   * 
   * ui32 flag_64; (0x64646464U)
   * ui32 magic_cookie;
   * ui64 nbytes_uncompressed;
   * ui64 nbytes_compressed;
   * ui64 nbytes_coded;
   * ui64 spare[2];
   */

  if (compressed_len < sizeof(compress_buf_hdr_64_t)) {
    return FALSE;
  }
  
  /*
   * check the 64-bit flag
   */

  {
    ui32 flag_64;
    memcpy(&flag_64, compressed_buffer, sizeof(ui32));
    BE_to_array_32(&flag_64, sizeof(ui32));
    if (flag_64 != TA_COMPRESS_FLAG_64) {
      return FALSE;
    }
  }

  /*
   * check the magic cookie
   */

  {
    ui32 magic_cookie;
    memcpy(&magic_cookie,
           (char *) compressed_buffer + sizeof(ui32),
           sizeof(ui32));
    BE_to_array_32(&magic_cookie, sizeof(ui32));
    if (_cookie_is_valid(magic_cookie)) {
      return TRUE;
    } else {
      return FALSE;
    }
  }

}

/**********************************************************************
 * ta_compression_method() - returns type of ta compression
 *
 * Use after calling ta_is_compressed() to get the compression type.
 *
 **********************************************************************/

ta_compression_method_t ta_compression_method(const void *compressed_buffer,
                                              ui64 compressed_len)
  
{

  ui32 magic_cookie;

  if (compressed_len < sizeof(compress_buf_hdr_t)) {
    return TA_COMPRESSION_NA;
  }

  /*
   * check the magic cookie
   */

  if (ta_is_compressed_64(compressed_buffer, compressed_len)) {
    memcpy(&magic_cookie, (char *) compressed_buffer + sizeof(ui32), sizeof(ui32));
  } else {
    memcpy(&magic_cookie, compressed_buffer, sizeof(ui32));
  }
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
 * ta_compression_debug() - prints info to stderr about a buffer.
 *
 * void, no values returned. Info includes compression type, length.
 *
 **********************************************************************/

extern void ta_compression_debug(const void *compressed_buffer){

  ui32 magic_cookie;
  int is64bit = FALSE;

  memcpy(&magic_cookie, compressed_buffer, sizeof(ui32));
  if (magic_cookie == TA_COMPRESS_FLAG_64) {
    /* 64-bit compression, magic cookie is second in header */
    is64bit = TRUE;
    memcpy(&magic_cookie, (char *) compressed_buffer + sizeof(ui32), sizeof(ui32));
  }
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

  if ((magic_cookie == __RLE_COMPRESSED) ||
      (magic_cookie == _RLE_COMPRESSED) ||
      (magic_cookie == RLE_COMPRESSED)){

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
    
  } else if (is64bit) {

    /* 64-bit headers */

    ui64 nbytes_uncompressed;
    ui64 nbytes_compressed;
    ui64 nbytes_coded;
    memcpy(&nbytes_uncompressed, 
	   (unsigned char *) compressed_buffer + sizeof(ui64),
	   sizeof(ui64));
    BE_to_array_64(&nbytes_uncompressed, sizeof(ui64));

    memcpy(&nbytes_compressed, 
	   (unsigned char *) compressed_buffer + 2*sizeof(ui64),
	   sizeof(ui64));
    BE_to_array_64(&nbytes_compressed, sizeof(ui64));

    memcpy(&nbytes_coded, 
	   (unsigned char *) compressed_buffer + 3*sizeof(ui64),
	   sizeof(ui64));
    BE_to_array_64(&nbytes_coded, sizeof(ui64));

    fprintf(stderr,"nbytes uncompressed : %ld\n", nbytes_uncompressed);
    fprintf(stderr,"nbytes compressed : %ld\n", nbytes_compressed);
    fprintf(stderr,"nbytes coded : %ld\n", nbytes_coded);

  } else {
    
    /* 32-bit headers */

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
 * For compressing, we have deprecated all methods
 * except gzip and bzip2.
 *
 * The memory for the encoded buffer is allocated by this routine,
 * and passed back to the caller.
 * This should be freed by the calling routine using ta_compress_free().
 *
 * The length of the compressed data buffer (*nbytes_compressed_p) is set.
 *
 * Returns pointer to the encoded buffer.
 * If compression fails, a buffer with uncompressed data is returned.
 * Guaranteed to return a valid buffer.
 *
 **********************************************************************/

void *ta_compress(ta_compression_method_t method,
		  const void *uncompressed_buffer,
		  ui64 nbytes_uncompressed,
		  ui64 *nbytes_compressed_p)

{

  /* check whether we need to use 64-bit headers */
  
  int use64bit = FALSE;
  if (nbytes_uncompressed >= UI32_MAX) {
    use64bit = TRUE;
  }
  
  if (use64bit) {
    
    /* for 64-bit, only use bzip and gzip */
    
    if (method == TA_COMPRESSION_NONE) {
      
      /*
       * create uncompressed buffer with ta_compress header 
       */

      ui64 nbytes_compressed_64;
      void *buf = _ta_no_compress64(TA_NOT_COMPRESSED,
                                    uncompressed_buffer,
                                    nbytes_uncompressed,
                                    &nbytes_compressed_64);
      *nbytes_compressed_p = nbytes_compressed_64;
      return buf;

    } else if (method == TA_COMPRESSION_BZIP) {

      /* bzip 64 compression for large buffers */
      
      ui64 nbytes_compressed_64;
      void *buf = bzip_compress64(uncompressed_buffer,
                                  nbytes_uncompressed,
                                  &nbytes_compressed_64);
      
      if (buf == NULL) {
        /*
         * compression failed
         * create uncompressed buffer with ta_compress header 
         */
        buf = _ta_no_compress64(BZIP_NOT_COMPRESSED,
                                uncompressed_buffer,
                                nbytes_uncompressed,
                                &nbytes_compressed_64);
      }

      *nbytes_compressed_p = nbytes_compressed_64;
      return buf;
      
    } else {
      
      /* gzip 64 compression for large buffers */
      
      ui64 nbytes_compressed_64;
      void *buf = gzip_compress64(uncompressed_buffer,
                                  nbytes_uncompressed,
                                  nbytes_compressed_p);
      
      if (buf == NULL) {
        /*
         * compression failed
         * create uncompressed buffer with ta_compress header 
         */
        buf = _ta_no_compress64(BZIP_NOT_COMPRESSED,
                                uncompressed_buffer,
                                nbytes_uncompressed,
                                &nbytes_compressed_64);
      }

      *nbytes_compressed_p = nbytes_compressed_64;
      return buf;

    }
    
  } else {
    
    /* 32 bit compression, buf size < UI32_MAX */
    
    if (method == TA_COMPRESSION_NONE) {
      
      /* no compression, just add header */
      
      ui32 nbytes_compressed_32;
      void *buf = _ta_no_compress(TA_NOT_COMPRESSED,
                                  uncompressed_buffer,
                                  (ui32) nbytes_uncompressed,
                                  &nbytes_compressed_32);
      *nbytes_compressed_p = nbytes_compressed_32;
      return buf;

    } else if (method == TA_COMPRESSION_BZIP) {
      
      /* bzip compression */

      ui32 nbytes_compressed_32;
      void *buf = bzip_compress(uncompressed_buffer,
                                (ui32) nbytes_uncompressed,
                                &nbytes_compressed_32);

      if (buf == NULL) {
        /*
         * compression failed
         * create uncompressed buffer with ta_compress header 
         */
        buf = _ta_no_compress(BZIP_NOT_COMPRESSED,
                              uncompressed_buffer,
                              nbytes_uncompressed,
                              &nbytes_compressed_32);
        
      }
      
      *nbytes_compressed_p = nbytes_compressed_32;
      return buf;
      
    } else {

      /* gzip compression */

      ui32 nbytes_compressed_32;
      void *buf = gzip_compress(uncompressed_buffer,
                                (ui32) nbytes_uncompressed,
                                &nbytes_compressed_32);

      if (buf == NULL) {
        /*
         * compression failed
         * create uncompressed buffer with ta_compress header 
         */
        buf = _ta_no_compress(GZIP_NOT_COMPRESSED,
                              uncompressed_buffer,
                              nbytes_uncompressed,
                              &nbytes_compressed_32);
        
      }
      
      *nbytes_compressed_p = nbytes_compressed_32;
      return buf;
      
    }
    
  } /* if (use64bit) */

  /* should not reach here */

  return NULL;

}

/**********************************************************************
 * ta_decompress() - toolsa generic decompression
 *
 * Perform generic decompression on buffer created using
 *   one of the toolsa compression methods.
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
		    ui64 *nbytes_uncompressed_p)
     
{

  ui32 magic_cookie;
  int is64bit = FALSE;

  memcpy(&magic_cookie, compressed_buffer, sizeof(ui32));
  if (magic_cookie == TA_COMPRESS_FLAG_64) {
    /* 64-bit compression, magic cookie is second in header */
    is64bit = TRUE;
    memcpy(&magic_cookie, (char *) compressed_buffer + sizeof(ui32), sizeof(ui32));
  }
  BE_to_array_32(&magic_cookie, sizeof(ui32));

  if (magic_cookie == TA_NOT_COMPRESSED ||
      magic_cookie == LZO_NOT_COMPRESSED ||
      magic_cookie == BZIP_NOT_COMPRESSED ||
      magic_cookie == GZIP_NOT_COMPRESSED ||
      magic_cookie == ZLIB_NOT_COMPRESSED) {
    
    /* buf has toolsa header, but is not compressed */
    /* strip off header, return data */
    
    if (is64bit) {
      /* 64-bit header */
      compress_buf_hdr_64_t hdr;
      char *decomp;
      char *compressed_data;
      memcpy(&hdr, compressed_buffer, sizeof(hdr));
      compress_buf_hdr_64_from_BE(&hdr);
      decomp = (char *) umalloc_min_1 (hdr.nbytes_uncompressed);
      *nbytes_uncompressed_p = hdr.nbytes_uncompressed;
      compressed_data = (char *) compressed_buffer + sizeof(hdr);
      memcpy(decomp, compressed_data, hdr.nbytes_uncompressed);
      return decomp;
    } else {
      /* 32-bit header */
      compress_buf_hdr_t hdr;
      char *decomp;
      char *compressed_data;
      memcpy(&hdr, compressed_buffer, sizeof(hdr));
      compress_buf_hdr_from_BE(&hdr);
      decomp = (char *) umalloc_min_1 (hdr.nbytes_uncompressed);
      *nbytes_uncompressed_p = hdr.nbytes_uncompressed;
      compressed_data = (char *) compressed_buffer + sizeof(hdr);
      memcpy(decomp, compressed_data, hdr.nbytes_uncompressed);
      return decomp;
    }

  } else if (magic_cookie == BZIP_COMPRESSED) {
    
    /* bzip */

    if (is64bit) {
      ui64 nbytes_uncompressed;
      void *decomp = bzip_decompress64(compressed_buffer, &nbytes_uncompressed);
      *nbytes_uncompressed_p = nbytes_uncompressed;
      return decomp;
    } else {
      ui32 nbytes_uncompressed;
      void *decomp = bzip_decompress(compressed_buffer, &nbytes_uncompressed);
      *nbytes_uncompressed_p = nbytes_uncompressed;
      return decomp;
    }

  } else if (magic_cookie == GZIP_COMPRESSED) {

    /* gzip */
    
    if (is64bit) {
      ui64 nbytes_uncompressed;
      void *decomp = gzip_decompress64(compressed_buffer, &nbytes_uncompressed);
      *nbytes_uncompressed_p = nbytes_uncompressed;
      return decomp;
    } else {
      ui32 nbytes_uncompressed;
      void *decomp = gzip_decompress(compressed_buffer, &nbytes_uncompressed);
      *nbytes_uncompressed_p = nbytes_uncompressed;
      return decomp;
    }

  } else if (magic_cookie == ZLIB_COMPRESSED) {
    
    /* only 32-bit compression for ZLIB */

    ui32 nbytes_uncompressed;
    void *decomp = zlib_decompress(compressed_buffer, &nbytes_uncompressed);
    *nbytes_uncompressed_p = nbytes_uncompressed;
    return decomp;
    
  } else if (magic_cookie == LZO_COMPRESSED) {

    /* only 32-bit compression for LZO */

    ui32 nbytes_uncompressed;
    void *decomp = lzo_decompress(compressed_buffer, &nbytes_uncompressed);
    *nbytes_uncompressed_p = nbytes_uncompressed;
    return decomp;
    
  } else if (magic_cookie == RLE_COMPRESSED ||
	     magic_cookie == _RLE_COMPRESSED ||
	     magic_cookie == __RLE_COMPRESSED) {
    
    /* only 32-bit compression for RLE */

    ui32 nbytes_uncompressed;
    void *decomp = rle_decompress(compressed_buffer, &nbytes_uncompressed);
    *nbytes_uncompressed_p = nbytes_uncompressed;
    return decomp;

  }

  *nbytes_uncompressed_p = 0;
  return NULL;

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

void *_ta_no_compress(ui32 magic_cookie,
		      const void *uncompressed_buffer,
		      ui32 nbytes_uncompressed,
		      ui32 *nbytes_compressed_p)
  
{
 
  ui32 nbytes_buffer =
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
  compress_buf_hdr_to_BE(hdr);
  
  if (nbytes_compressed_p != NULL) {
    *nbytes_compressed_p = nbytes_buffer;
  }
  
  /*
   * load buffer
   */
  
  memcpy((char *) out_buffer + sizeof(compress_buf_hdr_t),
	 uncompressed_buffer, nbytes_uncompressed);
  
  return out_buffer;

}

/*****************************************************
 * _ta_no_compress_64 - 64-bit version
 *
 * Load up compressed_buffer with uncompressed data
 *
 * returns output buffer of header plus original data
 *
 * private routine for use only by other compression routines.
 */

void *_ta_no_compress64(ui32 magic_cookie,
                        const void *uncompressed_buffer,
                        ui64 nbytes_uncompressed,
                        ui64 *nbytes_compressed_p)
  
{
  
  ui64 nbytes_buffer =
    nbytes_uncompressed + sizeof(compress_buf_hdr_64_t);
  void *out_buffer = umalloc_min_1(nbytes_buffer);
  compress_buf_hdr_64_t *hdr = (compress_buf_hdr_64_t *) out_buffer;
  
  /*
   * load header and swap
   */
  
  MEM_zero(*hdr);
  hdr->flag_64 = TA_COMPRESS_FLAG_64;
  hdr->magic_cookie = magic_cookie;
  hdr->nbytes_uncompressed = nbytes_uncompressed;
  hdr->nbytes_compressed = nbytes_buffer;
  hdr->nbytes_coded = nbytes_uncompressed;
  compress_buf_hdr_64_to_BE(hdr);
  
  if (nbytes_compressed_p != NULL) {
    *nbytes_compressed_p = nbytes_buffer;
  }
  
  /*
   * load buffer
   */
  
  memcpy((char *) out_buffer + sizeof(compress_buf_hdr_64_t),
	 uncompressed_buffer, nbytes_uncompressed);
  
  return out_buffer;

}

/*****************************************************
 * Convert header buffer to/from BigEndian.
 * 64-bit.
 */

void compress_buf_hdr_64_to_BE(compress_buf_hdr_64_t *hdr)
     
{
  BE_from_array_32(hdr, 2 * sizeof(ui32));
  BE_from_array_64(&hdr->nbytes_uncompressed, 5 * sizeof(ui64));
}

void compress_buf_hdr_64_from_BE(compress_buf_hdr_64_t *hdr)
     
{
  BE_to_array_32(hdr, 2 * sizeof(ui32));
  BE_to_array_64(&hdr->nbytes_uncompressed, 5 * sizeof(ui64));
}

/*****************************************************
 * Convert header buffer to/from BigEndian.
 * 32-bit.
 */

void compress_buf_hdr_to_BE(compress_buf_hdr_t *hdr)
     
{
  BE_from_array_32(hdr, sizeof(compress_buf_hdr_t));
}

void compress_buf_hdr_from_BE(compress_buf_hdr_t *hdr)
     
{
  BE_to_array_32(hdr, sizeof(compress_buf_hdr_t));
}

