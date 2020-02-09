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
 * compress.h
 *
 * Compression utilities
 *
 **********************************************************************/

#ifndef ta_compress_h
#define ta_compress_h

#ifdef __cplusplus
extern "C" {
#endif

#include <dataport/port_types.h>

/*
 * common header for most all compression types except RLE.
 * RLE uses a 20-byte header for backward compatibility reasons
 */
  
typedef struct {
  ui32 magic_cookie;
  ui32 nbytes_uncompressed;
  ui32 nbytes_compressed; /* including this header */
  ui32 nbytes_coded; /* nbytes_compressed - sizeof(hdr) */
  ui32 spare[2];
} compress_buf_hdr_t;

typedef struct {
  ui32 flag_64;
  ui32 magic_cookie;
  ui64 nbytes_uncompressed;
  ui64 nbytes_compressed; /* including this header */
  ui64 nbytes_coded; /* nbytes_compressed - sizeof(hdr) */
  ui64 spare[2];
} compress_buf_hdr_64_t;

typedef enum {
  TA_COMPRESSION_NA =   -1,  /* not applicable
                              * raw data, no compression, no header */
  TA_COMPRESSION_NONE =  0,  /* no compression, but header included */
  TA_COMPRESSION_RLE =   1,  /* run-length encoding */
  TA_COMPRESSION_LZO =   2,  /* Lempel-Ziv-Oberhaumer */
  TA_COMPRESSION_ZLIB =  3,  /* Lempel-Ziv */
  TA_COMPRESSION_BZIP =  4,  /* bzip2 */
  TA_COMPRESSION_GZIP =  5   /* Lempel-Ziv in gzip format */
} ta_compression_method_t;

/*
 * magic cookies for various compression states
 */

#define TA_COMPRESS_FLAG_64 0x64646464U
#define TA_NOT_COMPRESSED 0x2f2f2f2fU
#define LZO_COMPRESSED 0xf1f1f1f1U
#define LZO_NOT_COMPRESSED 0xf2f2f2f2U
#define BZIP_COMPRESSED 0xf3f3f3f3U
#define BZIP_NOT_COMPRESSED 0xf4f4f4f4U
#define GZIP_COMPRESSED 0xf7f7f7f7U
#define GZIP_NOT_COMPRESSED 0xf8f8f8f8U
#define RLE_COMPRESSED 0xfe0103fdU
#define _RLE_COMPRESSED 0xfe0104fdU /* used in some early mdv files */
#define __RLE_COMPRESSED 0xfd0301fe /* used in some early mdv files */
#define ZLIB_COMPRESSED 0xf5f5f5f5U
#define ZLIB_NOT_COMPRESSED 0xf6f6f6f6U

/**********************************************************************
 * ta_is_compressed() - tests whether buffer is compressed using toolsa
 *                      compression
 *
 * Safer than ta_compressed(). Use recommended.
 *
 * Returns TRUE or FALSE
 *
 **********************************************************************/

extern int ta_is_compressed(const void *compressed_buffer,
                            ui64 compressed_len);

/**********************************************************************
 * ta_is_compressed_64() - tests whether buffer is compressed
 * using toolsa, compression, with 64-bit headers
 *
 * Returns TRUE or FALSE
 *
 **********************************************************************/

extern int ta_is_compressed_64(const void *compressed_buffer,
                               ui64 compressed_len);
     
/**********************************************************************
 * ta_compression_method() - returns type of ta compression
 *
 * Use after calling ta_is_compressed() to get the compression type.
 *
 **********************************************************************/

extern ta_compression_method_t
  ta_compression_method(const void *compressed_buffer,
                        ui64 compressed_len);
     
/**********************************************************************
 * ta_compressed() - tests whether buffer is compressed using toolsa
 *                   compression
 *
 * deprecated - use ta_is_compressed() instead.
 *
 * Assumes there are at least 4 bytes in the buffer
 *
 * Returns TRUE or FALSE
 *
 **********************************************************************/

extern int ta_compressed(const void *compressed_buffer);
     

/**********************************************************************
 * ta_compression_debug() - prints info to stderr about a buffer.
 *
 * void, no values returned. Info includes compression type, length.
 *
 **********************************************************************/

extern void ta_compression_debug(const void *compressed_buffer);
     

/**********************************************************************
 * ta_gzip_buffer() - tests whether buffer is gzip type
 *
 * Returns TRUE or FALSE
 **********************************************************************/

extern int ta_gzip_buffer(const void *compressed_buffer);
     
/***********************
 * compression
 ***********************/

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
 * Returns pointer to the encoded buffer.
 *
 **********************************************************************/

extern void *ta_compress(ta_compression_method_t method,
			 const void *uncompressed_buffer,
			 ui64 nbytes_uncompressed,
			 ui64 *nbytes_compressed_p);

/***********************
 * generic decompression
 ***********************/

/**********************************************************************
 * ta_decompress() - toolsa generic decompression
 *
 * Perform generic decompression on buffer created using
 *   rle_compress(), lzo_compress() or bzip_compress().
 *
 * Switches on the magic cookie in the header.
 *
 * The memory for the uncompressed data buffer is allocated by this routine.
 * This should be freed by the calling routine using ta_compress_free().
 *
 * On success, returns pointer to the uncompressed data buffer.
 * Also, *nbytes_uncompressed_p is set.
 *
 * On failure, returns NULL.
 *
 **********************************************************************/

extern void *ta_decompress(const void *compressed_buffer,
			   ui64 *nbytes_uncompressed_p);
     
/**********************************************************************
 * ta_compress_free() - free up buffer allocated and returned by
 * any of the compression or decompression routines.
 *
 */

extern void ta_compress_free(void *buffer);

/***************
 * LZO routines
 ***************/
#ifndef EXCLUDE_LZO

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
 * This should be freed by the calling routine using ta_compress_free().
 *
 * The length of the compressed data buffer (*nbytes_compressed_p) is set.
 *
 * Returns pointer to the encoded buffer.
 *
 **********************************************************************/

extern void *lzo_compress(const void *uncompressed_buffer,
			  ui32 nbytes_uncompressed,
			  ui32 *nbytes_compressed_p);

/**********************************************************************
 * lzo_decompress()
 *
 * Perform LZO decompression on buffer created using lzo_compress();
 *
 * The memory for the uncompressed data buffer is allocated by this routine.
 * This should be freed by the calling routine using ta_compress_free().
 *
 * On success, returns pointer to the uncompressed data buffer.
 * Also, *nbytes_uncompressed_p is set.
 *
 * On failure, returns NULL.
 *
 **********************************************************************/

extern void *lzo_decompress(const void *compressed_buffer,
			    ui32 *nbytes_uncompressed_p);
     
#endif
     
/***************
 * BZIP routines
 ***************/

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
 * This should be freed by the calling routine using ta_compress_free().
 *
 * The length of the compressed data buffer (*nbytes_compressed_p) is set.
 *
 * Returns pointer to the encoded buffer.
 *
 **********************************************************************/

extern void *bzip_compress(const void *uncompressed_buffer,
			   ui32 nbytes_uncompressed,
			   ui32 *nbytes_compressed_p);

extern void *bzip_compress64(const void *uncompressed_buffer,
                             ui64 nbytes_uncompressed,
                             ui64 *nbytes_compressed_p);

/**********************************************************************
 * bzip_decompress()
 *
 * Perform BZIP decompression on buffer created using bzip_compress();
 *
 * The memory for the uncompressed data buffer is allocated by this routine.
 * This should be freed by the calling routine using ta_compress_free().
 *
 * On success, returns pointer to the uncompressed data buffer.
 * Also, *nbytes_uncompressed_p is set.
 *
 * On failure, returns NULL.
 *
 **********************************************************************/

extern void *bzip_decompress(const void *compressed_buffer,
			     ui32 *nbytes_uncompressed_p);

extern void *bzip_decompress64(const void *compressed_buffer,
                               ui64 *nbytes_uncompressed_p);

/***************
 * GZIP routines
 ***************/

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
 * This should be freed by the calling routine using ta_compress_free().
 *
 * The length of the compressed data buffer (*nbytes_compressed_p) is set.
 *
 * Returns pointer to the encoded buffer.
 *
 **********************************************************************/

extern void *gzip_compress(const void *uncompressed_buffer,
			   ui32 nbytes_uncompressed,
			   ui32 *nbytes_compressed_p);

extern void *gzip_compress64(const void *uncompressed_buffer,
                             ui64 nbytes_uncompressed,
                             ui64 *nbytes_compressed_p);

/**********************************************************************
 * gzip_decompress()
 *
 * Perform GZIP decompression on buffer created using gzip_compress();
 *
 * The memory for the uncompressed data buffer is allocated by this routine.
 * This should be freed by the calling routine using ta_compress_free().
 *
 * On success, returns pointer to the uncompressed data buffer.
 * Also, *nbytes_uncompressed_p is set.
 *
 * On failure, returns NULL.
 *
 **********************************************************************/

extern void *gzip_decompress(const void *compressed_buffer,
			     ui32 *nbytes_uncompressed_p);
     
extern void *gzip_decompress64(const void *compressed_buffer,
                               ui64 *nbytes_uncompressed_p);
     
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

extern void *gunzip_known_len(const void *compressed_buffer,
                              ui32 nbytes_compressed,
                              ui32 nbytes_uncompressed);
     
/******************************************************************
 * RLE routines
 *
 * These routines create and read buffers in the same format as the
 * deprecated routines.
 *******************************************************************/

/**********************************************************************
 * rle_compress()
 *
 * In the compressed data, the first 20 bytes are a header as follows:
 *
 *   (ui32) Magic cookie - RLE_COMPRESSED
 *   (ui32) key for compression
 *   (ui32) nbytes_buffer (nbytes_compressed + sizeof header)
 *   (ui32) nbytes_uncompressed
 *   (ui32) nbytes_compressed
 *
 * The header is in BE byte order.
 *
 * The compressed data follows the header.
 *
 * The memory for the encoded buffer is allocated by this routine,
 * and passed back to the caller.
 * This should be freed by the calling routine using ta_compress_free().
 *
 * The length of the compressed buffer (*nbytes_compressed_p) is set.
 * This length is for the header plus the compressed data.
 *
 * Returns pointer to the encoded buffer.
 *
 **********************************************************************/

extern void *rle_compress(const void *uncompressed_buffer,
			  ui32 nbytes_uncompressed,
			  ui32 *nbytes_compressed_p);

/**********************************************************************
 * rle_decompress()
 *
 * Perform RLE decompression on buffer created using rle_compress();
 *
 * The memory for the uncompressed data buffer is allocated by this routine.
 * This should be freed by the calling routine using ta_compress_free().
 *
 * On success, returns pointer to the uncompressed data buffer.
 * Also, *nbytes_uncompressed_p is set.
 *
 * On failure, returns NULL.
 *
 **********************************************************************/

extern void *rle_decompress(const void *compressed_buffer,
			    ui32 *nbytes_uncompressed_p);

/***************
 * ZLIB routines
 ***************/

/**********************************************************************
 * zlib_compress()
 *
 * In the compressed data, the first 24 bytes are a header as follows:
 *
 *   (ui32) Magic cookie - ZLIB_COMPRESSED or ZLIB_NOT_COMPRESSED
 *   (ui32) nbytes_uncompressed
 *   (ui32) nbytes_compressed - including this header
 *   (ui32) nbytes_coded - (nbytes_compressed - sizeof header)
 *   (ui32) spare
 *   (ui32) spare
 *
 * The header is in BE byte order.
 *
 * If the buffer is not compressed, magic_cookie is set to ZLIB_NOT_COMPRESSED.
 *
 * The compressed data follows the header.
 *
 * The memory for the encoded buffer is allocated by this routine,
 * and passed back to the caller.
 * This should be freed by the calling routine using ta_compress_free().
 *
 * The length of the compressed data buffer (*nbytes_compressed_p) is set.
 *
 * Returns pointer to the encoded buffer.
 *
 **********************************************************************/

extern void *zlib_compress(const void *uncompressed_buffer,
			   ui32 nbytes_uncompressed,
			   ui32 *nbytes_compressed_p);

/**********************************************************************
 * zlib_decompress()
 *
 * Perform ZLIB decompression on buffer created using zlib_compress();
 *
 * The memory for the uncompressed data buffer is allocated by this routine.
 * This should be freed by the calling routine using ta_compress_free().
 *
 * On success, returns pointer to the uncompressed data buffer.
 * Also, *nbytes_uncompressed_p is set.
 *
 * On failure, returns NULL.
 *
 **********************************************************************/

extern void *zlib_decompress(const void *compressed_buffer,
			     ui32 *nbytes_uncompressed_p);

/*
 * private routines for use internally by the compression routines
 */

extern void *_ta_no_compress(ui32 magic_cookie,
			     const void *uncompressed_buffer,
			     ui32 nbytes_uncompressed,
			     ui32 *nbytes_compressed_p);

extern void *_ta_no_compress64(ui32 magic_cookie,
                               const void *uncompressed_buffer,
                               ui64 nbytes_uncompressed,
                               ui64 *nbytes_compressed_p);

/*************************
 * Header byte swapping
 *************************/

extern void compress_buf_hdr_64_to_BE(compress_buf_hdr_64_t *hdr);
     
extern void compress_buf_hdr_64_from_BE(compress_buf_hdr_64_t *hdr);

extern void compress_buf_hdr_to_BE(compress_buf_hdr_t *hdr);
     
extern void compress_buf_hdr_from_BE(compress_buf_hdr_t *hdr);

/*************************
 * Deprecated RLE routines
 *************************/

#define RL_NBYTES_WORD 4
#define RL7_NBYTES_EXTRA (3 * RL_NBYTES_WORD)
#define RL8_NBYTES_EXTRA (5 * RL_NBYTES_WORD)

#define RL8_FLAG RLE_COMPRESSED
#define MDV_RL8_FLAG 0xfe0104fdU   /* This flag was used by early MDV */
                                   /* files.  Now, MDV files use the */
                                   /* RL8_FLAG, but we must keep this */
                                   /* flag also so the early MDV data can */
                                   /* still be decoded.  This is only used */
                                   /* in the check at the beginning of the */
                                   /* decode routine. */

extern ui08 *uRLEncode8(const ui08 *full_data,
			ui32 nbytes_full,
			ui32 key,
			ui32 *nbytes_array);

extern ui08 *uRLDecode8(const ui08 *coded_data,
			ui32 *nbytes_full);
     
extern int uRLCheck(const ui08 *coded_data,
		    ui32 nbytes_passed,
		    int *eight_bit,
		    ui32 *nbytes_compressed);

extern ui08 *uRLEncode(const ui08 *full_data,
		       ui32 nbytes_full,
		       ui32 *nbytes_array);

extern ui08 *uRLDecode(const ui08 *coded_data,
		       ui32 *nbytes_full);

#ifdef __cplusplus
}
#endif

#endif
