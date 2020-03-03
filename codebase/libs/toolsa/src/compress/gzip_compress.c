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
 * gzip_compress.c
 *
 * Compression utilities using GZIP compression - based on ZLIB
 *
 * See doc/README.ZLIB
 *
 * Mike Dixon / Steve Sullivan, RAP, NCAR, P.O.Box 3000, Boulder, CO, USA
 *
 * April 2004
 *
 **********************************************************************/

#include <toolsa/toolsa_macros.h>
#include <toolsa/compress.h>
#include <toolsa/mem.h>
#include <dataport/bigend.h>
#include <zlib.h>
#include <stdio.h>

/* #define DEBUG_PRINT */

static int zlib_gz_magic[2] = {0x1f, 0x8b}; /* gzip magic header */

/* 
 * file scope functions
 */

static uLong _mkHeader(unsigned char *cmpbuf);

static uLong _mkTrailer(uLong inlen,
			const unsigned char *inbuf,
			uLong totcmplen,
			unsigned char *cmpbuf);

static int _checkHeader(uLong totcmplen,
			const unsigned char * cmpbuf);

static void _insertLong(uLong pos,
			uLong val,
			unsigned char *cmpbuf);

static uLong _getLong(uLong pos,
		      const unsigned char * cmpbuf);

/**********************************************************************
 * gzip_compress()
 *
 * In the compressed data, the first 24 bytes are a header as follows:
 *
 *   (ui32) Magic cookie - GZIP_COMPRESSED or GZIP_NOT_COMPRESSED
 *   (ui32) nbytes_uncompressed
 *   (ui32) nbytes_compressed - including this header
 *   (ui32) nbytes_coded - (nbytes_compressed - sizeof header)
 *   (ui32) spare
 *   (ui32) spare
 *
 * The header is in BE byte order.
 *
 * If the buffer is not compressed, magic_cookie is set to GZIP_NOT_COMPRESSED.
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

void *gzip_compress(const void *uncompressed_buffer,
		    unsigned int nbytes_uncompressed,
		    unsigned int *nbytes_compressed_p)
     
{
  
  int iret = 0;
  int gzHdrLen;

  unsigned int nbytes_alloc;
  unsigned int nbytes_used;
  uLongf datacmplen, totcmplen;

  unsigned char *working_buffer;
  unsigned char *final_buffer;
  unsigned char *gzbuf;

  compress_buf_hdr_t *compHdr;
  z_stream zstm;
 
  /*
   * initial allocation of encoded buffer, the size of the original buffer
   * plus the extra bytes needed for working space.
   */
  
  nbytes_alloc = (nbytes_uncompressed + sizeof(compress_buf_hdr_t) +
		  nbytes_uncompressed / 10 + 1024);
  
  working_buffer = (unsigned char *) umalloc_min_1 (nbytes_alloc);
  gzbuf = working_buffer + sizeof(compress_buf_hdr_t);
  
  /*
   * put header into buffer
   */
  
  gzHdrLen = _mkHeader(gzbuf);
  
  /*
   * Set up the z_stream
   */
  
  memset(&zstm, 0, sizeof(z_stream));
  zstm.next_in = (Bytef*) uncompressed_buffer;
  zstm.avail_in = (uInt) nbytes_uncompressed;
  zstm.next_out = gzbuf + gzHdrLen;
  zstm.avail_out = (uInt) (nbytes_alloc - sizeof(compress_buf_hdr_t) - gzHdrLen);
  
  /*
   * compress
   */
  
  if (deflateInit2(&zstm,
                   Z_DEFAULT_COMPRESSION, /* compression level = 6.  0 <= x <= 9 */
                   Z_DEFLATED,            /* method */
                   -MAX_WBITS,            /* neg max window bits: suppress wrapper */
                   8,                     /* internal state memory level.
                                           *  1 <= x <= 9.
                                           * See doc zconf.h, MAX_MEM_LEVEL.
                                           * default: DEF_MEM_LEVEL, in zutil.h */
                   Z_DEFAULT_STRATEGY)) { /* compression strategy */
    iret = -1;
  }

  if (deflate(&zstm, Z_FINISH) != Z_STREAM_END) {
    iret = -1;
  }
  datacmplen = zstm.total_out;
  if (deflateEnd(&zstm)) {
    iret = -1;
  }
  
  if (iret) {

#ifdef DEBUG_PRINT
    fprintf(stderr, "GZIP compress failed\n");
    fprintf(stderr, "  Uncompressed size: %d\n", nbytes_uncompressed);
#endif

    /*
     * compression failed
     */
    ufree(working_buffer);
    return (_ta_no_compress(GZIP_NOT_COMPRESSED,
			    uncompressed_buffer,
			    nbytes_uncompressed,
			    nbytes_compressed_p));
  }
  
  /*
   * Make trailer
   */
  
  totcmplen = _mkTrailer(nbytes_uncompressed,
			 uncompressed_buffer,
			 gzHdrLen + datacmplen, /* output len used so far */
			 gzbuf);              /* output gz buf */
  
  if (iret || totcmplen >= nbytes_uncompressed) {

#ifdef DEBUG_PRINT
    fprintf(stderr, "GZIP failed\n");
    fprintf(stderr, "  Uncompressed size: %d\n", nbytes_uncompressed);
    fprintf(stderr, "  Compressed size: %ld\n", totcmplen);
#endif
    
    /*
     * compression failed or data not compressible
     */
    ufree(working_buffer);
    return (_ta_no_compress(GZIP_NOT_COMPRESSED,
			    uncompressed_buffer,
			    nbytes_uncompressed,
			    nbytes_compressed_p));
  }
  
  /*
   * compression worked - truncate working buffer to final buffer
   */

  nbytes_used = sizeof(compress_buf_hdr_t) + totcmplen;
  final_buffer = (unsigned char *) urealloc(working_buffer, nbytes_used);

#ifdef DEBUG_PRINT
    fprintf(stderr, "GZIP succeeded\n");
    fprintf(stderr, "  Uncompressed size: %d\n", nbytes_uncompressed);
    fprintf(stderr, "  Compressed size: %ld\n", totcmplen);
#endif
    
  /*
   * load hdr and swap
   */

  compHdr = (compress_buf_hdr_t *) final_buffer;
  MEM_zero(*compHdr);
  compHdr->magic_cookie = GZIP_COMPRESSED;
  compHdr->nbytes_uncompressed = nbytes_uncompressed;
  compHdr->nbytes_compressed = nbytes_used;
  compHdr->nbytes_coded = totcmplen;
  BE_from_array_32(compHdr, sizeof(compress_buf_hdr_t));
  
  if (nbytes_compressed_p != NULL) {
    *nbytes_compressed_p = nbytes_used;
  }
  
  return final_buffer;

}

/**********************************************************************
 * gzip_decompress()
 *
 * Perform GZIP decompression on buffer created using gzip_compress();
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

void *gzip_decompress(const void *compressed_buffer,
		      unsigned int *nbytes_uncompressed_p)
     
{

  int iret = 0;
  int ic;
  int gzHdrLen;
  uLongf decmplen;
  uLong crcdecomp;
  uLong crcfile;

  unsigned char *gzbuf;
  unsigned char *uncompressed_data;

  compress_buf_hdr_t hdr;
  z_stream zstm;

  if (compressed_buffer == NULL) {
    *nbytes_uncompressed_p = 0;
    return (NULL);
  }
  
  /*
   * decode header
   */
  
  memcpy(&hdr, compressed_buffer, sizeof(compress_buf_hdr_t));
  BE_to_array_32(&hdr, sizeof(compress_buf_hdr_t));
  
  if (hdr.magic_cookie != GZIP_COMPRESSED &&
      hdr.magic_cookie != GZIP_NOT_COMPRESSED) {
    *nbytes_uncompressed_p = 0;
    return (NULL);
  }
  
  /*
   * allocate uncompressed buffer
   */

  uncompressed_data =
    (unsigned char *) umalloc_min_1 (hdr.nbytes_uncompressed);
  *nbytes_uncompressed_p = hdr.nbytes_uncompressed;
  
  if (hdr.magic_cookie == GZIP_NOT_COMPRESSED) {
    
#ifdef DEBUG_PRINT
    fprintf(stderr, "GZIP decompress: data not compressed\n");
    fprintf(stderr, "  Returning uncompressed data\n");
    fprintf(stderr, "  Uncompressed size: %d\n", hdr.nbytes_uncompressed);
#endif
    
    gzbuf = (unsigned char *) compressed_buffer + sizeof(compress_buf_hdr_t);
    memcpy(uncompressed_data, gzbuf, hdr.nbytes_uncompressed);
    return (uncompressed_data);

  } else {
    
    /*
     * create compressed copy - for byte alignment
     */
    
    gzbuf = (unsigned char *) umalloc_min_1(hdr.nbytes_coded);
    memcpy(gzbuf,
	   (char *) compressed_buffer + sizeof(compress_buf_hdr_t),
	   hdr.nbytes_coded);

    /*
     * check gzip header
     */

    gzHdrLen = _checkHeader(hdr.nbytes_coded, gzbuf);
    if (gzHdrLen < 0) {
      ufree(uncompressed_data);
      ufree(gzbuf);
      *nbytes_uncompressed_p = 0;
      return (NULL);
    }
    
    /* Set up the z_stream */
    
    memset( &zstm, 0, sizeof(z_stream));
    zstm.next_in = (Bytef*) gzbuf + gzHdrLen;
    zstm.avail_in = (uInt) hdr.nbytes_coded - gzHdrLen;
    zstm.next_out = uncompressed_data;
    zstm.avail_out = (uInt) hdr.nbytes_uncompressed;

    /*
     * initialize inflate - neg max window bits: suppress wrapper
     */
    
    if (inflateInit2(&zstm, -MAX_WBITS)) {
      iret = -1;
    }

    /*
     * perform inflation
     */

    if (inflate(&zstm, Z_FINISH) != Z_STREAM_END) {
      iret = -1;
    }
    decmplen = zstm.total_out;
    if (inflateEnd(&zstm)) {
      iret = -1;
    }

    /*
     * check trailer
     */
    
    crcdecomp = crc32(0L, NULL, 0);
    crcdecomp = crc32(crcdecomp, uncompressed_data, decmplen);
	
    ic = gzHdrLen + zstm.total_in;
    crcfile = _getLong(ic, gzbuf);
    ic += 4;
    if (crcfile != crcdecomp) {
      iret = -1;
    }
    
    ufree(gzbuf);
    
    if (iret == 0 && decmplen == hdr.nbytes_uncompressed) {
#ifdef DEBUG_PRINT
      fprintf(stderr, "GZIP decompress: success\n");
      fprintf(stderr, "  compressed_size: %d\n", hdr.nbytes_coded);
      fprintf(stderr, "  uncompressed_size: %ld\n", hdr.nbytes_uncompressed);
#endif
      return(uncompressed_data);
    } else {
#ifdef DEBUG_PRINT
      fprintf(stderr, "GZIP decompress: failure\n");
#endif
      *nbytes_uncompressed_p = 0;
      ufree(uncompressed_data);
      return (NULL);
    }

  }

}

/**********************************************************************
 * gunzip_known_len()
 *
 * Perform GZIP decompression on a standard GZIP buffer.
 *
 * The length of the uncompressed length is known a-prior and is passed in.
 *
 * The memory for the uncompressed data buffer is allocated by this routine.
 * This should be freed by the calling routine using ta_compress_free();
 *
 * On success, returns pointer to the uncompressed data buffer.
 *
 * On failure, returns NULL.
 *
 **********************************************************************/

void *gunzip_known_len(const void *compressed_buffer,
                       unsigned int nbytes_compressed,
                       unsigned int nbytes_uncompressed)
     
{

  int iret = 0;
  int ic;
  int gzHdrLen = 0;
  uLongf decmplen;
  uLong crcdecomp;
  uLong crcfile;

  unsigned char *gzbuf = NULL;
  unsigned char *uncompressed_data;

  compress_buf_hdr_t hdr;
  MEM_zero(hdr);
  z_stream zstm;
  
  if (compressed_buffer == NULL) {
    return (NULL);
  }
  
  /*
   * allocate uncompressed buffer
   */

  uncompressed_data = (unsigned char *) umalloc_min_1 (nbytes_uncompressed);
  
  /* Set up the z_stream */
  
  memset(&zstm, 0, sizeof(z_stream));
  zstm.next_in = (Bytef*) gzbuf + gzHdrLen;
  zstm.avail_in = (uInt) nbytes_compressed - gzHdrLen;
  zstm.next_out = uncompressed_data;
  zstm.avail_out = (uInt) nbytes_uncompressed;

  /*
   * initialize inflate - neg max window bits: suppress wrapper
   */
  
  if (inflateInit2(&zstm, -MAX_WBITS)) {
    iret = -1;
  }

  /*
   * perform inflation
   */
  
  if (inflate(&zstm, Z_FINISH) != Z_STREAM_END) {
    iret = -1;
  }
  decmplen = zstm.total_out;
  if (inflateEnd(&zstm)) {
    iret = -1;
  }
  
  /*
   * check trailer
   */
  
  crcdecomp = crc32(0L, NULL, 0);
  crcdecomp = crc32(crcdecomp, uncompressed_data, decmplen);
	
  ic = gzHdrLen + zstm.total_in;
  crcfile = _getLong(ic, gzbuf);
  ic += 4;
  if (crcfile != crcdecomp) {
    iret = -1;
  }
  
  ufree(gzbuf);
  
  if (iret == 0 && decmplen == hdr.nbytes_uncompressed) {
#ifdef DEBUG_PRINT
    fprintf(stderr, "gunzip_known_len: success\n");
    fprintf(stderr, "  compressed_size: %ld\n", nbytes_compressed);
    fprintf(stderr, "  uncompressed_size: %ld\n", nbytes_uncompressed);
#endif
    return(uncompressed_data);
  } else {
#ifdef DEBUG_PRINT
    fprintf(stderr, "gunzip_known_len: failure\n");
#endif
    ufree(uncompressed_data);
    return (NULL);
  }

}

/******************************************
 * Insert zlib header into start of gzbuf.
 * Returns header len.
 */

static uLong _mkHeader(unsigned char * gzbuf)

{
  
  /*
   * The zlib header is variable length with optional fields.
   * Here we create the simplest header, 10 bytes long.
   */

  int zlib_OS_CODE = 0x03; /* say Unix */

  uLong ic = 0;
  gzbuf[ic++] = zlib_gz_magic[0]; /* magic code */
  gzbuf[ic++] = zlib_gz_magic[1]; /* magic code */
  gzbuf[ic++] = Z_DEFLATED;
  gzbuf[ic++] = 0; /* flags */
  gzbuf[ic++] = 0; /* time: always 0 */
  gzbuf[ic++] = 0; /* time */
  gzbuf[ic++] = 0; /* time */
  gzbuf[ic++] = 0; /* time */
  gzbuf[ic++] = 0; /* xflags: always 0 */
  gzbuf[ic++] = zlib_OS_CODE;	/* ignored */

  return ic;	/* hdrlen == 10 */

}

/***********************************************************
 * Insert zlib trailer after data in gzbuf.
 * Returns total buf len, including header, data, and trailer.
 */

static uLong _mkTrailer(uLong inlen,            /* len of inbuf */
			const unsigned char *inbuf,  /* original uncompressed data */
			uLong totcmplen,        /* len of header + compressed data */
			unsigned char *gzbuf) /* output buffer */
     
{
  uLong pos = totcmplen;
  uLong crc = crc32(0L, NULL, 0);
  crc = crc32(crc, inbuf, inlen);
  _insertLong(pos, crc, gzbuf); /* insert CRC */
  pos += 4;
  _insertLong(pos, inlen & 0xffffffff, gzbuf); /* insert uncomp len */
  pos += 4;
  return pos; /* return total len: header + cmpdata + trailer */
}

/****************************************************
 * Check a header for validity.
 *
 * Returns header len on success, -1 on error
 */

static int _checkHeader(uLong totcmplen,
			const unsigned char * gzbuf)
     
{

  long ic = 0; /* cursor */
  int flags;

  /* Parse flag byte and skip over optional fields. */
  
  /* int ASCII_FLAG= 0x01;*/    /* bit 0 set: file probably ascii text */
  int  HEAD_CRC     = 0x02;	/* bit 1 set: header CRC present */
  int  EXTRA_FIELD  = 0x04;	/* bit 2 set: extra field present */
  int  ORIG_NAME    = 0x08;	/* bit 3 set: original file name present */
  int  COMMENT      = 0x10;	/* bit 4 set: file comment present */
  /* int RESERVED   = 0xE0;*/	/* bits 5..7: reserved */
  
  if (ic >= totcmplen || gzbuf[ic++] != zlib_gz_magic[0]) {
    return -1;
  }
  if (ic >= totcmplen || gzbuf[ic++] != zlib_gz_magic[1]) {
    return -1;
  }
  if (ic >= totcmplen || gzbuf[ic++] != Z_DEFLATED) {
    return -1;
  }
  if (ic >= totcmplen) {
    return -1;
  }

  flags = gzbuf[ic++];
  ic += 4; /* skip time */
  ic += 1; /* skip xflags */
  ic += 1; /* skip os_code */
  if (ic >= totcmplen) {
    return -1;
  }

  if ((flags & EXTRA_FIELD) != 0) {		/* field has specified len */
    if (ic >= totcmplen) {
      return -1;
    }
    {
      long fldlen = 0xff & gzbuf[ic++];
      ic += fldlen;
    }
  }
  if ((flags & ORIG_NAME) != 0) {		/* null delimited field */
    while (TRUE) {
      if (ic >= totcmplen) {
	return -1;
      }
      {
	char cc = gzbuf[ic++];
	if (cc == 0) break;
      }
    }
  }
  if ((flags & COMMENT) != 0) {			/* null delimited field */
    while (TRUE) {
      if (ic >= totcmplen) {
	return -1;
      }
      {
	char cc = gzbuf[ic++];
	if (cc == 0) break;
      }
    }
  }
  if ((flags & HEAD_CRC) != 0) {			/* field has len = 2 */
    /* Ignore header CRC.  We check trailer CRC. */
    ic += 2;
  }
  if (ic >= totcmplen) {
    return -1;
  }
  return ic; /* hdrlen */
}

/**********************************
 * Insert long int into output buf.
 *
 * pos: position in gzbuf
 * val: value to insert
 * gzbuf: ouput compressed buffer
 */

static void _insertLong(uLong pos,
			uLong val,
			unsigned char *gzbuf)
{
  int ii;
  for (ii = 0; ii < 4; ii++) {
    gzbuf[pos + ii] = (val & 0xff);
    val >>= 8;
  }
}

/*********************************
 * Retrieve long int from buffer.
 *
 * pos:  position in gzbuf
 * gzbuf: input compressed buffer
 */

static uLong _getLong(uLong pos,
		      const unsigned char * gzbuf)
{
  uLong ii, val = 0;
  for (ii = 0; ii < 4; ii++) {
    val <<= 8;
    val |= (gzbuf[ pos + 4 - 1 - ii] & 0xff);
  }
  return val;
}

