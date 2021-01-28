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
 * rle_compress.c
 *
 * RLE (Run Length Encoding) Compression utilities
 *
 **********************************************************************/

#include <toolsa/mem.h>
#include <toolsa/compress.h>
#include <dataport/bigend.h>
#include <memory.h>
#include <netinet/in.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* #define DEBUG_PRINT */

/**********************************************************************
 * rle_compress()
 *
 * In the compressed data, the first 20 bytes are a header as follows:
 *
 *   (ui32) Magic cookie - RL8_FLAG
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
 * This should be freed by the calling routine using ta_compress_free();
 *
 * The length of the compressed buffer (*nbytes_compressed_p) is set.
 * This length is for the header plus the compressed data.
 *
 * Returns pointer to the encoded buffer.
 *
 **********************************************************************/

void *rle_compress(const void *uncompressed_buffer,
		   unsigned int nbytes_uncompressed,
		   unsigned int *nbytes_compressed_p)

{

  ui08 *compressed_buffer;
  ui32 nbytes_compressed;

  compressed_buffer = uRLEncode8(uncompressed_buffer,
				 nbytes_uncompressed,
				 255,
				 &nbytes_compressed);

  
  *nbytes_compressed_p = nbytes_compressed;

#ifdef DEBUG_PRINT
  fprintf(stderr, "RLE compress successful\n");
  fprintf(stderr, "  compressed_size: %d\n", nbytes_compressed);
  fprintf(stderr, "  uncompressed_size: %d\n", nbytes_uncompressed);
#endif

  return (compressed_buffer);

}

     
/**********************************************************************
 * rle_decompress()
 *
 * Perform RLE decompression on buffer created using rle_compress();
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

void *rle_decompress(const void *compressed_buffer,
		     unsigned int *nbytes_uncompressed_p)

{

  ui08 *uncompressed_buffer;
  ui32 nbytes_uncompressed;

  uncompressed_buffer = uRLDecode8(compressed_buffer,
				   &nbytes_uncompressed);

  *nbytes_uncompressed_p = nbytes_uncompressed;

#ifdef DEBUG_PRINT
  fprintf(stderr, "RLE decompress successful\n");
  fprintf(stderr, "  uncompressed_size: %d\n", nbytes_uncompressed);
#endif

  return (uncompressed_buffer);

}

     
/**********************************************************************
 * uRLEncode8() - deprecated function interface
 *
 * Performs run-length encoding on byte data which uses all 8 bits
 *
 * In the coded data, the first 20 bytes are as follows:
 *
 * (ui32) Magic cookie - RL8_FLAG
 * (ui32) key
 * (ui32) nbytes_array
 * (ui32) nbytes_full
 * (ui32) nbytes_coded
 *
 * The coded data follows these 20 bytes. The coded data is padded out
 * to end on a 4-byte boundary.
 *
 * The memory for the encoded array is allocated by this routine.
 *
 * Returns pointer to the encoded array. The number of bytes in the
 * encodeded data array is returned via nbytes_array.
 *
 * utility routine
 *
 * N. Rehak RAP, NCAR, Boulder CO 5 Oct 1990
 *
 * hacked from uRLEncode.c by Mike Dixon
 *
 **********************************************************************/

ui08 *uRLEncode8(const ui08 *full_data,
		 ui32 nbytes_full,
		 ui32 key,
		 ui32 *nbytes_array)
     
{

  ui08 byteval;
  ui08 *coded_data;
  const ui08 *fdata, *end;
  ui08 *cdata;

  ui32 runcount;

  ui32 *lptr;

  ui32 nbytes_coded;
  ui32 nbytes_extra;
  ui32 nbytes_alloc;
  ui32 nbytes_unpadded;

  int i;

  /*
   * full_data is original array
   * fdata is pointer into original array
   * cdata is pointer into coded array
   * end is pointer to last byte in original array
   */

  /*
   * initial allocation of encoded array, the size of the original array
   * plus the extra bytes at the start for the nbyte values, plus enough
   * bytes to pass a word boundary. Twice this will be sufficient for the
   * worst case in which the pattern is
   *    key non-key key non-key key ...
   * which expands to twice the original size.
   */

  nbytes_extra = RL8_NBYTES_EXTRA;
  nbytes_alloc = 2 * (nbytes_full + nbytes_extra + RL_NBYTES_WORD);
  coded_data = ucalloc_min_1 (nbytes_alloc, 1);

  /*
   * set the number of bytes in the full data, and the pointer to the
   * number of encoded bytes
   */

  /*
   * set pointers to data arrays
   */

  fdata = full_data;
  cdata = coded_data + nbytes_extra;
  
  /*
   * set pointer to last data byte
   */

  end = fdata + nbytes_full;

  if (full_data != NULL) {

    while (fdata < end) {

      /*
       * get byte value
       */

      byteval = *fdata;
      runcount = 1;
      fdata++;
      
      while ((fdata < end) &&
	     (runcount < 255) &&
	     (*fdata == byteval)) {
	
	/*
	 * count up adjacent bytes of same value
	 */

	fdata++;
	runcount++;
	
      }

      if (runcount <= 3 && byteval != key) {
	
	/*
	 * count <= 3, and byteval is not key,
	 * so store as single bytes because there is no
	 * advantage to RLE
	 */
	
	for (i = 0; i < runcount; i++) {
	  *cdata = byteval;
	  cdata++;
	}
	
      } else {

	/*
	 *  count > 3, store as RLE indicator, count then byte value
	 */

	*cdata = key;
	*(cdata + 1) = runcount;
	*(cdata + 2) = byteval;
	cdata += 3;

      }

    } /* while (fdata < end) */

    /*
     * compute the number of bytes in the encoded data, including the 
     * leading 8 bytes and the padding to go to a word boundary
     */

    nbytes_coded = cdata - coded_data - nbytes_extra;
    nbytes_unpadded = nbytes_coded + nbytes_extra;
    *nbytes_array =
      ((ui32) ((nbytes_unpadded - 1) / RL_NBYTES_WORD) + 1) * RL_NBYTES_WORD;

    /*
     * realloc the coded_data array
     */

    coded_data = (ui08 *) urealloc
      ((char *) coded_data, (ui32) *nbytes_array);

    /*
     * set the bytes counts
     */

    lptr = (ui32 *) coded_data;

    *lptr = BE_from_ui32(RL8_FLAG);
    lptr++;
    *lptr = BE_from_ui32(key);
    lptr++;
    *lptr = BE_from_ui32(*nbytes_array);
    lptr++;
    *lptr = BE_from_ui32(nbytes_full);
    lptr++;
    *lptr = BE_from_ui32(nbytes_coded);

    return (coded_data);

  } else {

    return ((ui08 *) NULL);

  } /* if (full_data != NULL) */

}

/**********************************************************************
 * uRLDecode8() - deprecated function interface
 *
 * Performs run-length decoding on byte data which was
 * compressed using uRLEncode8
 *
 * Returns the full data array. The size of the array is passed back via
 * nbytes_full.
 *
 * utility routine
 *
 * N. Rehak RAP, NCAR, Boulder CO 6 October 1992
 *
 * hacked from "uRLDecode.c" by Mike Dixon
 *
 **********************************************************************/

ui08 *uRLDecode8(const ui08 *coded_data,
		 ui32 *nbytes_full)
     
{

  int runcount;
  int swap;
  
  ui32 *lptr;
  ui32 compress_flag;
  ui32 key;

  ui32 nbytes_coded;
  ui32 nbytes_extra;

  ui08 byteval;
  ui08 *full_data;
  const ui08 *end;
  const ui08 *cdata;
  ui08 *fdata;

  if (coded_data != NULL) {

    lptr = (ui32 *) coded_data;
    compress_flag = *lptr;
    lptr++;

    if (compress_flag == RL8_FLAG ||
	compress_flag == MDV_RL8_FLAG) {
      swap = FALSE;
    } else if (BE_to_ui32(compress_flag) == RL8_FLAG ||
	       BE_to_ui32(compress_flag) == MDV_RL8_FLAG) {
      swap = TRUE;
    } else {
      /*
       * not compressed with this alg, return NULL
       */
      return ((ui08 *) NULL);
    }

    /*
     * get number of bytes for the coded and full data
     */

    if (swap) {
      key = BE_to_ui32(*lptr);
    } else {
      key = *lptr;
    }

    lptr += 2;
    if (swap) {
      *nbytes_full = BE_to_ui32(*lptr);
    } else {
      *nbytes_full = *lptr;
    }

    lptr++;
    if (swap) {
      nbytes_coded = BE_to_ui32(*lptr);
    } else {
      nbytes_coded = *lptr;
    }

    nbytes_extra = RL8_NBYTES_EXTRA;

    /*
     * get space for full data
     */

    full_data = ucalloc_min_1(*nbytes_full, 1);
    
    fdata = full_data;
    cdata = coded_data + nbytes_extra;

    end = cdata + nbytes_coded;

    while (cdata < end) {

      byteval = *cdata;

      if (byteval == key) {

	/*
	 * if RLE flag value, the next byte contains the run count and
	 * the byte after contains the byte value
	 */

	cdata ++;
	runcount = *cdata;
	cdata++;
	byteval  = *cdata;

	/*
	 * set runcount values
	 */

	memset((char *) fdata, (int) byteval, (int) runcount);
	umalloc_verify();
	fdata += runcount;

      } else {

	/*
	 * if not RLE flag value, set single byte
	 */

	*fdata = byteval;
	fdata++;

      } /* if (byteval == key) */

      cdata++;

    } /* while (cdata < end) */

    return (full_data);

  } else {

    return ((ui08 *) NULL);

  }

}

/**********************************************************************
 * uRLCheck() - deprecated function interface
 *
 * Checks for compression type, and number of bytes in compressed
 * array.
 *
 * Returns 0 on success, -1 on error
 *
 * utility routine
 *
 * Mike Dixon, RAP, NCAR, Boulder CO
 *
 * Feb 1994
 *
 **********************************************************************/

int uRLCheck(const ui08 *coded_data,
	     ui32 nbytes_passed,
	     int *eight_bit,
	     ui32 *nbytes_compressed)
     
{

  ui32 *lptr;
  ui32 first_int, third_int;
  ui32 compress_flag;
  
  if (coded_data == NULL)
    return (-1);
  
  if (nbytes_passed < sizeof(ui32))
    return (-1);
  
  lptr = (ui32 *) coded_data;
  first_int = BE_to_ui32(*lptr);

  compress_flag = first_int;

  if (compress_flag == RL8_FLAG ||
      compress_flag == MDV_RL8_FLAG) {
    
    *eight_bit = TRUE;

    if (nbytes_passed < 3 * sizeof(ui32))
      return (-1);
  
    lptr += 2;
    third_int = BE_to_ui32(*lptr);
    *nbytes_compressed = third_int;

  } else {

    *eight_bit = FALSE;
    *nbytes_compressed = first_int;

  }

  return (0);

}

/**********************************************************************
 * uRLEncode() - deprecated function interface
 *
 * Performs run-length encoding on byte data which uses only
 * the lower 7 bits
 *
 * In the coded data, the first 12 bytes are as follows:
 *
 * (ui32) nbytes_array, (ui32) nbytes_full, (ui32) nbytes_coded.
 *
 * The coded data follows these 12 bytes. The coded data is padded out
 * to end on a 4-byte boundary.
 *
 * The memory for the encoded array is allocated by this routine.
 *
 * Returns pointer to the encoded array. The number of bytes in the
 * encodeded data array is returned via nbytes_array.
 *
 * utility routine
 *
 * Mike Dixon RAP, NCAR, Boulder CO November 1990
 *
 **********************************************************************/

ui08 *uRLEncode(const ui08 *full_data, ui32 nbytes_full,
		ui32 *nbytes_array)
     
{

  ui08 byteval;
  ui08 *coded_data;
  const ui08 *fdata;
  ui08 *cdata;
  const ui08 *end;

  ui32 runcount;

  ui32 nbytes_coded;
  ui32 nbytes_extra;
  ui32 nbytes_unpadded;

  /*
   * full_data is original array
   * fdata is pointer into original array
   * cdata is pointer into coded array
   * end is pointer to last byte in original array
   */

  /*
   * initial allocation of encoded array, the size of the original array
   * plus the extra bytes at the start for the nbyte values, plus enough
   * bytes to pass a word boundary. This will be sufficient for the
   * worst case in which there is no compression
   */

  nbytes_extra = RL7_NBYTES_EXTRA;
  
  coded_data = (ui08 *) umalloc_min_1
    ((ui32) (nbytes_full + nbytes_extra + RL_NBYTES_WORD));

  /*
   * set the number of bytes in the full data, and the pointer to the
   * number of encoded bytes
   */

  /*
   * set pointers to data arrays
   */

  fdata = full_data;
  cdata = coded_data + nbytes_extra;
  
  /*
   * set pointer to last data byte
   */

  end = full_data + nbytes_full;

  if (full_data != NULL) {

    while (fdata < end) {

      /*
       * get byte value
       */

      byteval = *fdata;
      fdata++;

      /*
       * return with NULL pointer if data exceeds 127
       */

      if (byteval > 127) {

	fprintf(stderr, "ERROR - uRLEncode\n");
	fprintf(stderr, "Byte value exceeds 127.\n");
	ufree(coded_data);
	return ((ui08 *) NULL);

      } /* if (byteval .... */
  
      runcount = 1;
      
      while ((fdata < end) &&
	     (runcount < 127) &&
	     (*fdata == byteval)) {

	/*
	 * count up adjacent bytes of same value
	 */

	fdata++;
	runcount++;
	
      }

      if (runcount == 1) {

	/*
	 * count = 1, store as single byte
	 */

	*cdata = byteval;
	cdata++;

      } else {

	/*
	 *  count > 1, store as count then byte value
	 */

	*cdata = 0x80 | runcount;
	*(cdata + 1) = byteval;
	cdata += 2;

      }

    } /* while (fdata < end) */

    /*
     * compute the number of bytes in the encoded data, including the 
     * leading 8 bytes and the padding to go to a word boundary
     */

    nbytes_coded = cdata - coded_data - nbytes_extra;
    nbytes_unpadded = nbytes_coded + nbytes_extra;
    *nbytes_array =
      ((ui32) ((nbytes_unpadded - 1) / RL_NBYTES_WORD) + 1) * RL_NBYTES_WORD;

    /*
     * realloc the coded_data array
     */

    coded_data = (ui08 *) urealloc
      ((char *) coded_data, (ui32) *nbytes_array);

    /*
     * set the bytes counts
     */

    *((ui32 *) coded_data) = BE_from_ui32(*nbytes_array);
    *((ui32 *) coded_data + 1) = BE_from_ui32(nbytes_full);
    *((ui32 *) coded_data + 2) = BE_from_ui32(nbytes_coded);

    return (coded_data);

  } else {

    ufree(coded_data);
    return ((ui08 *) NULL);

  } /* if (full_data != NULL) */

}

/**********************************************************************
 * uRLDecode() - deprecated function interface
 *
 * Performs run-length decoding on byte data which was
 * compressed using uRLEncode
 *
 * Returns the full data array. The size of the array is passed back via
 * nbytes_full.
 *
 * utility routine
 *
 * Mike Dixon RAP, NCAR, Boulder CO November 1990
 *
 **********************************************************************/

ui08 *uRLDecode(const ui08 *coded_data, ui32 *nbytes_full)
     
{

  int runcount;
  ui32 nbytes_coded;
  ui32 nbytes_extra;

  ui08 byteval;
  ui08 *full_data;
  const ui08 *end;
  ui08 *fdata;
  const ui08 *cdata;

  if (coded_data != NULL) {

    /*
     * get number of bytes for the coded and full data
     */

    *nbytes_full = BE_to_ui32(*((ui32 *) coded_data + 1));
    nbytes_coded = BE_to_ui32(*((ui32 *) coded_data + 2));
    nbytes_extra = RL7_NBYTES_EXTRA;

    /*
     * get space for full data
     */

    full_data = (ui08 *) umalloc_min_1(*nbytes_full);
    
    fdata = full_data;
    cdata = coded_data + nbytes_extra;

    end = cdata + nbytes_coded;

    while (cdata < end) {

      byteval = *cdata;

      if ((byteval & 0x80) == 0x80) {

	/*
	 * if most significant bit set, mask off lower 7 bits and
	 * use as the count on the next byte value
	 */

	runcount = byteval & 0x7f;
	cdata++;
	byteval  = *cdata;

	/*
	 * set runcount values
	 */

	memset((char *) fdata, (int) byteval, (int) runcount);
	fdata += runcount;

      } else {

	/*
	 * if most significant bit not set, set single byte
	 */

	*fdata = byteval;
	fdata++;

      } /* if ((byteval & 0x80) == 0x80) */

      cdata++;

    } /* while (cdata < end) */

    return (full_data);

  } else {

    return ((ui08 *) NULL);

  }

}

