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
 * bzip_compress.c
 *
 * Compression utilities using BZIP compression
 *
 * See doc/README.BZIP2
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, USA
 *
 * July 1999
 *
 **********************************************************************/

#include <toolsa/compress.h>
#include <toolsa/umisc.h>
#include <dataport/bigend.h>
#include <bzlib.h>

/* #define DEBUG_PRINT */

/***********************************
 * compute 64-bit bz integer size
 ***********************************/

static ui64 _compute64bitSize(ui64 lo32, ui64 hi32)
{
  return (hi32 << 32) + lo32;
}

/**********************************************************************
 * bzip_compress()
 *
 * In the compressed data, the first 24 bytes are a header as follows:
 *
 *   (ui32) Magic cookie - BZIP_COMPRESSED or BZIP_NOT_COMPRESSED
 *   (ui32) nbytes_uncompressed
 *   (ui32) nbytes_compressed - including this header
 *   (ui32) nbytes_coded - (nbytes_compressed - sizeof header)
 *   (ui32) spare
 *   (ui32) spare
 *
 * The header is in BE byte order.
 *
 * If the buffer is not compressed, magic_cookie is set to BZIP_NOT_COMPRESSED.
 *
 * The compressed data follows the header.
 *
 * The memory for the encoded buffer is allocated by this routine,
 * and passed back to the caller.
 * This should be freed by the calling routine using ta_compress_free();
 *
 * The length of the compressed data buffer (*nbytes_compressed_p) is set.
 *
 * Returns pointer to the encoded buffer on success, NULL on failure.
 *
 **********************************************************************/

void *bzip_compress(const void *uncompressed_buffer,
		    unsigned int nbytes_uncompressed,
		    unsigned int *nbytes_compressed_p)
     
{

  int iret;
  unsigned int nbytes_buffer;
  char *comp_buf;
  char *out_buf;
  unsigned int nbytes_alloc;
  compress_buf_hdr_t *hdr;
  unsigned int out_len;
 
  /*
   * initial allocation of encoded buffer, the size of the original buffer
   * plus the extra bytes needed for working space.
   */
  
  nbytes_alloc = (nbytes_uncompressed + sizeof(compress_buf_hdr_t) +
		  nbytes_uncompressed / 64 + 1024);
  
  comp_buf = (char *) umalloc_min_1 (nbytes_alloc);
  
  /*
   * compress with BZIP2
   */
  
  out_len = nbytes_alloc - sizeof(compress_buf_hdr_t);
  iret = BZ2_bzBuffToBuffCompress(comp_buf + sizeof(compress_buf_hdr_t),
                                  &out_len,
                                  (void *) uncompressed_buffer,
                                  nbytes_uncompressed,
                                  1, 0, 0);

  if (iret != BZ_OK || out_len >= nbytes_uncompressed) {
    
#ifdef DEBUG_PRINT
    if (iret != BZ_OK) {
      fprintf(stderr, "BZIP compress failed\n");
    }
    if (out_len >= nbytes_uncompressed) {
      fprintf(stderr, "BZIP failed to reduce size\n");
      fprintf(stderr, "  Uncompressed size: %d\n", nbytes_uncompressed);
      fprintf(stderr, "  Compressed size: %d\n", out_len);
    }
#endif

    /*
     * compression failed or data not compressible
     */

    ufree(comp_buf);
    return NULL;
    /* return (_ta_no_compress(BZIP_NOT_COMPRESSED, */
    /*     		    uncompressed_buffer, */
    /*     		    nbytes_uncompressed, */
    /*     		    nbytes_compressed_p)); */
    
  }
  
  /*
   * compression worked
   */

  /*
   * truncate buffer
   */
  
  nbytes_buffer = sizeof(compress_buf_hdr_t) + out_len;
  out_buf = urealloc(comp_buf, nbytes_buffer);

#ifdef DEBUG_PRINT
  fprintf(stderr, "BZIP compress succeeded\n");
  fprintf(stderr, "  Uncompressed size: %d\n", nbytes_uncompressed);
  fprintf(stderr, "  Compressed size: %d\n", out_len);
#endif

  /*
   * load hdr and swap
   */

  hdr = (compress_buf_hdr_t *) out_buf;
  MEM_zero(*hdr);
  hdr->magic_cookie = BZIP_COMPRESSED;
  hdr->nbytes_uncompressed = nbytes_uncompressed;
  hdr->nbytes_compressed = nbytes_buffer;
  hdr->nbytes_coded = out_len;
  compress_buf_hdr_to_BE(hdr);
  
  if (nbytes_compressed_p != NULL) {
    *nbytes_compressed_p = nbytes_buffer;
  }
  
  return out_buf;

}

/**********************************************************************
 * bzip_decompress()
 *
 * Perform BZIP decompression on buffer created using bzip_compress();
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

void *bzip_decompress(const void *compressed_buffer,
		      unsigned int *nbytes_uncompressed_p)
     
{

  void *compressed_copy;
  char *uncompressed_data;
  char *compressed_data;
  int iret;
  unsigned int out_len;
  compress_buf_hdr_t hdr;

  if (compressed_buffer == NULL) {
    *nbytes_uncompressed_p = 0;
    return (NULL);
  }

  /*
   * decode header
   */
  
  memcpy(&hdr, compressed_buffer, sizeof(compress_buf_hdr_t));
  compress_buf_hdr_from_BE(&hdr);

  if (hdr.magic_cookie != BZIP_COMPRESSED &&
      hdr.magic_cookie != BZIP_NOT_COMPRESSED) {
    *nbytes_uncompressed_p = 0;
    return (NULL);
  }

  uncompressed_data = (char *) umalloc_min_1 (hdr.nbytes_uncompressed);
  *nbytes_uncompressed_p = hdr.nbytes_uncompressed;

  if (hdr.magic_cookie == BZIP_NOT_COMPRESSED) {

#ifdef DEBUG_PRINT
    fprintf(stderr, "BZIP decompress: data not compressed\n");
    fprintf(stderr, "  Returning uncompressed data\n");
    fprintf(stderr, "  Uncompressed size: %d\n", hdr.nbytes_uncompressed);
#endif
    
    compressed_data = (char *) compressed_buffer + sizeof(compress_buf_hdr_t);
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

    out_len = hdr.nbytes_uncompressed;
    iret = BZ2_bzBuffToBuffDecompress(uncompressed_data,
				  &out_len,
				  compressed_copy,
				  hdr.nbytes_compressed,
				  0, 0);

    ufree(compressed_copy);
    
    if (iret == BZ_OK && out_len == hdr.nbytes_uncompressed) {
#ifdef DEBUG_PRINT
      fprintf(stderr, "BZIP decompress: success\n");
      fprintf(stderr, "  compressed_size: %d\n", hdr.nbytes_coded);
      fprintf(stderr, "  uncompressed_size: %d\n", out_len);
#endif
      return(uncompressed_data);
    } else {
#ifdef DEBUG_PRINT
      fprintf(stderr, "BZIP decompress: failure\n");
#endif
      *nbytes_uncompressed_p = 0;
      return (NULL);
    }

  }

}


/**********************************************************************
 * bzip_compress64()
 *
 * The compressed buffer will contain the following in order:
 *
 *   (1a)  compress_buf_hdr_t    (toolsa 32-bit header, 24 bytes)
 *     or for large buffers (len > INT_MAX)
 *   (1b)  compress_buf_hdr_64_t (toolsa 64-bit header, 48 bytes)
 *
 *   (2)  gzip header (10 bytes)
 *   (3)  compressed data
 *   (4)  gzip trailer (8 bytes)
 *   
 * The toolsa header is in BE byte order.
 *
 * If the buffer is not compressed, the magic_cookie is set to GZIP_NOT_COMPRESSED.
 *
 * The memory for the encoded buffer is allocated by this routine,
 * and passed back to the caller.
 * This should be freed by the calling routine using ta_compress_free();
 *
 * The length of the compressed data buffer (*nbytes_compressed_p) is set.
 *
 * Returns pointer to the compressed buffer on success, NULL on failure.
 *
 **********************************************************************/

void *bzip_compress64(const void *uncompressed_buffer,
                      ui64 nbytes_uncompressed,
                      ui64 *nbytes_compressed_p)
  
{

  /* return value */

  int iret = 0;

  /* toolsa compressed header */
  
  compress_buf_hdr_64_t toolsaHdr64;
  compress_buf_hdr_64_t *toolsaHdr64Ptr;

  /* working buffer, for temp storage of compressed output */
  
  ui64 workLen = 256 * 1024 * 1024; /* 256 MB working buffer size */
  MEMbuf *workBuf = NULL;
  char *work = NULL;

  /* final buffer, for the full output */

  MEMbuf *finalBuf = NULL;
  char *final = NULL;

  /* counters */

  ui64 nLeftIn = nbytes_uncompressed;
  ui64 nOutThisCycle = 0;
  ui64 nCompressed = 0;
  
  /* bz stream, and flags to indicate flush or finish */

  bz_stream strm;
  int action = BZ_RUN; /* initilize */
  
  /* create the memory buffers */
  
  workBuf = MEMbufCreate();
  work = (char*) MEMbufPrepare(workBuf, workLen);

  finalBuf = MEMbufCreate();
  final = (char*) MEMbufPtr(finalBuf);
  
  /*
   * add toolsa compression header to start final buffer
   * we will return and fill this out at the end
   */
  
  memset(&toolsaHdr64, 0, sizeof(toolsaHdr64));
  MEMbufAdd(finalBuf, &toolsaHdr64, sizeof(toolsaHdr64));
  
  /* Set up the bz_stream handle */
  
  memset(&strm, 0, sizeof(strm));
  strm.bzalloc = NULL; /* use default allocator */
  strm.bzfree = NULL; /* use default free */
  strm.opaque = NULL; /* default */
  strm.next_in = (char *) uncompressed_buffer;
  if (nLeftIn > workLen) {
    strm.avail_in = workLen;
  } else {
    strm.avail_in = nLeftIn;
  }
  strm.next_out = work;
  strm.avail_out = workLen;
  
  /*
   * Initialize with deflateInit2 with windowBits negative
   * this suppresses the zlib header and trailer.
   * We manually add the gzip header and trailer instead.
   */

  if (BZ2_bzCompressInit(&strm,   /* stream pointer */
                         9,       /* block size 9 * 100000 bytes */
                         1,       /* verbosity - 0 to 4 */
                         30)) {   /* default work factor */
    iret = -1;
  }
  if (iret != BZ_OK) {
    MEMbufFree(workBuf);
    MEMbufFree(finalBuf);
    return NULL;
  }
  
  /*
   * compress in a while loop using the smaller work buffer
   * for the actual compression
   * and then concatenate to form the final buffer
   */

  do {

    if (nLeftIn == 0) {
      action = BZ_FINISH;
    } else {
      action = BZ_RUN;
    }

    /* compress */
    
    iret = BZ2_bzCompress(&strm, action);
    if (iret < 0) {
      MEMbufFree(workBuf);
      MEMbufFree(finalBuf);
#ifdef DEBUG_PRINT
      fprintf(stderr, "ERROR - BZ2_bzCompress, iret: %d\n", iret);
#endif
      return NULL;
    }

#ifdef DEBUG_PRINT
    fprintf(stderr, "SUCCESS - BZ2_bzCompress, iret: %d\n", iret);
#endif

    /* add output to final buffer */
    
    nOutThisCycle = workLen - strm.avail_out;
    MEMbufAdd(finalBuf, work, nOutThisCycle);
    nCompressed += nOutThisCycle;

    /* update zstream for next pass */

    {
      ui64 totalIn = _compute64bitSize(strm.total_in_lo32, strm.total_in_hi32);
      strm.avail_out = workLen;
      strm.next_out = work;
      nLeftIn = nbytes_uncompressed - totalIn;
      if (nLeftIn > workLen) {
        strm.avail_in = workLen;
      } else {
        strm.avail_in = nLeftIn;
      }
      strm.next_in = (char *) uncompressed_buffer + totalIn;
    }
    
  } while (iret != BZ_STREAM_END);
  
  /* finalize the compression */

  if (BZ2_bzCompressEnd(&strm)) {
#ifdef DEBUG_PRINT
    fprintf(stderr, "BZIP compress failed\n");
    fprintf(stderr, "  Uncompressed size: %ld\n", nbytes_uncompressed);
#endif
    MEMbufFree(workBuf);
    MEMbufFree(finalBuf);
    return NULL;
  }

#ifdef DEBUG_PRINT
  fprintf(stderr, "bzip_compress64 succeeded\n");
  fprintf(stderr, "  Uncompressed size: %ld\n", nbytes_uncompressed);
  fprintf(stderr, "  Compressed size: %ld\n", nCompressed);
#endif
  
  /* set the compressed size in the return args */

  if (nbytes_compressed_p != NULL) {
    *nbytes_compressed_p = MEMbufLen(finalBuf);
  }
  
  /* fill out toolsa hdr at the start of the buffer, and swap */
  
  final = (char*) MEMbufPtr(finalBuf);
  toolsaHdr64Ptr = (compress_buf_hdr_64_t *) final;
  toolsaHdr64Ptr->flag_64 = TA_COMPRESS_FLAG_64;
  toolsaHdr64Ptr->magic_cookie = BZIP_COMPRESSED;
  toolsaHdr64Ptr->nbytes_uncompressed = nbytes_uncompressed;
  toolsaHdr64Ptr->nbytes_compressed = MEMbufLen(finalBuf);
  toolsaHdr64Ptr->nbytes_coded = toolsaHdr64Ptr->nbytes_compressed - sizeof(compress_buf_hdr_64_t);
  toolsaHdr64Ptr->spare[0] = 0;
  compress_buf_hdr_64_to_BE(toolsaHdr64Ptr);

  /* free up working buffer */

  MEMbufDelete(workBuf);

  /*
   * Free up handle to final buffer but save the pointer to the
   * compressed data, to return to the caller.
   * This will need to be freed by the caller.
   */

  {
    void *compBuf = MEMbufPtr(finalBuf);
    MEMbufDeleteHandle(finalBuf);
    /* return compressed buffer */
    return compBuf;
  }
  
}
  
/**********************************************************************
 * bzip_decompress64()
 *
 * Perform BZIP decompression on buffer created using bzip_compress();
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

void *bzip_decompress64(const void *compressed_buffer,
                        ui64 *nbytes_uncompressed_p)
  
{

  /* return code */

  int iret = 0;

  /* temporary working buffer, for the immediate output */
  /* final buffer, for the full output */
  
  ui64 workLen = 256 * 1024 * 1024; /* 256 MB working buffer size */
  MEMbuf *workBuf = NULL;
  MEMbuf *finalBuf = NULL;
  
  /* working pointers and lengths */
  
  char *bzBuf;
  char *uncompressed_data;
  ui64 nbytes_uncompressed;
  
  ui64 nLeftIn = 0;
  ui64 nOutThisCycle = 0;
  ui64 totalOut;
  ui64 availIn = 0;
  ui64 totalIn;

  ui64 nInTotal;
  ui64 nInBzip;

  /* toolsa header */
  
  compress_buf_hdr_64_t toolsaHdr64;
  
  /* stream */

  bz_stream strm;

  /* sanity check */

  if (compressed_buffer == NULL) {
    *nbytes_uncompressed_p = 0;
    return NULL;
  }
  
  /* decode and check toolsa header */
  
  memcpy(&toolsaHdr64, compressed_buffer, sizeof(compress_buf_hdr_64_t));
  compress_buf_hdr_64_from_BE(&toolsaHdr64);
  
  if (toolsaHdr64.magic_cookie != BZIP_COMPRESSED &&
      toolsaHdr64.magic_cookie != BZIP_NOT_COMPRESSED) {
    *nbytes_uncompressed_p = 0;
    return NULL;
  }
  
  nInTotal = toolsaHdr64.nbytes_compressed;
  nInBzip = nInTotal - sizeof(compress_buf_hdr_64_t);
  
  /* start of bzip data */
  
  bzBuf = (char *) compressed_buffer + sizeof(compress_buf_hdr_64_t);
  
  /* deal with non-compressed case */
  
  if (toolsaHdr64.magic_cookie == BZIP_NOT_COMPRESSED) {
    
#ifdef DEBUG_PRINT
    fprintf(stderr, "BZIP decompress: data not compressed\n");
    fprintf(stderr, "  Returning uncompressed data\n");
    fprintf(stderr, "  Uncompressed size: %ld\n", toolsaHdr64.nbytes_uncompressed);
#endif
    
    uncompressed_data =
      (char *) umalloc_min_1 (toolsaHdr64.nbytes_uncompressed);
    *nbytes_uncompressed_p = toolsaHdr64.nbytes_uncompressed;
    
    memcpy(uncompressed_data, bzBuf, toolsaHdr64.nbytes_uncompressed);
    return uncompressed_data;

  }
    
  /* create the memory buffers for uncompressed output */
  
  finalBuf = MEMbufCreate();
  workBuf = MEMbufCreate();
  MEMbufPrepare(workBuf, workLen);

  /* Set up the bz_stream handle  */

  memset(&strm, 0, sizeof(strm));
  strm.bzalloc = NULL; /* use default allocator */
  strm.bzfree = NULL; /* use default free */
  strm.opaque = NULL; /* default */
  strm.next_in = (char *) bzBuf;
  availIn = nInBzip;
  if (availIn > workLen) {
    availIn = workLen;
  }
  strm.avail_in = availIn;
  strm.next_out = (char*) MEMbufPtr(workBuf);
  strm.avail_out = workLen;
  
  /* initialize inflate - neg max window bits: suppress wrapper */
  
  if (BZ2_bzDecompressInit(&strm, 0, 0) != BZ_OK) {
    MEMbufFree(workBuf);
    MEMbufFree(finalBuf);
    return NULL;
  }
  
  /*
   * perform inflation in while loop, using buffers that may be
   * shorter than the data
   */

  nLeftIn = nInBzip;
  
  do {

    /* uncompress */
    
    iret = BZ2_bzDecompress(&strm);
    if (iret < 0) {
#ifdef DEBUG_PRINT
      fprintf(stderr, "ERROR - BZ2_bzDecompress() in GzipTest::bzip_decompress64\n");
      fprintf(stderr, "  iret: %d\n", iret);
#endif
      MEMbufFree(workBuf);
      MEMbufFree(finalBuf);
      return NULL;
    }


    /* add data from work buffer to final buffer  */

    totalOut = _compute64bitSize(strm.total_out_lo32, strm.total_out_hi32);
    nOutThisCycle = totalOut - MEMbufLen(finalBuf);
    MEMbufAdd(finalBuf, MEMbufPtr(workBuf), nOutThisCycle);

    /* update zstream for next pass */

    totalIn = _compute64bitSize(strm.total_in_lo32, strm.total_in_hi32);
    nLeftIn = nInBzip - totalIn;
    
    strm.next_in = (char *) bzBuf + totalIn;
    availIn = nLeftIn;
    if (availIn > workLen) {
      availIn = workLen;
    }
    strm.avail_in = availIn;

    strm.next_out = (char*) MEMbufPtr(workBuf);
    strm.avail_out = workLen;

  } while (iret != BZ_STREAM_END);
    
  /* finish */
  
  if (BZ2_bzDecompressEnd(&strm)) {
    MEMbufFree(workBuf);
    MEMbufFree(finalBuf);
    return NULL;
  }

  uncompressed_data = (char*) MEMbufPtr(finalBuf);
  nbytes_uncompressed = MEMbufLen(finalBuf);
  *nbytes_uncompressed_p = nbytes_uncompressed;
  
  /* check sizes */
  
  if (nbytes_uncompressed != toolsaHdr64.nbytes_uncompressed) {
#ifdef DEBUG_PRINT
    fprintf(stderr, "BZIP decompress: failure\n");
    fprintf(stderr, "  nCompressed: %ld\n", toolsaHdr64.nbytes_coded);
    fprintf(stderr, "  uncompressed_size: %ld\n", nbytes_uncompressed);
    fprintf(stderr, "  expecting: %ld\n", toolsaHdr64.nbytes_uncompressed);
#endif
    MEMbufFree(workBuf);
    MEMbufFree(finalBuf);
    return NULL;
  }

  /* success */
  
#ifdef DEBUG_PRINT
  fprintf(stderr, "BZIP decompress: success\n");
  fprintf(stderr, "  nCompressed: %ld\n", toolsaHdr64.nbytes_coded);
  fprintf(stderr, "  uncompressed_size: %ld\n", toolsaHdr64.nbytes_uncompressed);
#endif

  MEMbufFree(workBuf);
  MEMbufDeleteHandle(finalBuf);
  
  return uncompressed_data;

}

