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
 * compress.c
 *
 * Compression utilities
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <memory.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <dataport/bigend.h>

#define RL8_FLAG 0xfe0103fdU
#define NBYTES_WORD 4
#define RL7_NBYTES_EXTRA (3 * NBYTES_WORD)
#define RL8_NBYTES_EXTRA (5 * NBYTES_WORD)

/**********************************************************************
 * RLDecode7() - performs run-length decoding on byte data which was
 *               compressed using RLEncode7
 *
 * Returns the full data array. The size of the array is passed back via
 * nbytes_full.
 *
 * utility routine
 *
 **********************************************************************/

ui08 *RLDecode7(ui08 *coded_data, unsigned int *nbytes_full)

{

  int runcount;
  unsigned int nbytes_coded;
  unsigned int nbytes_extra;

  ui08 byteval;
  ui08 *full_data;
  ui08 *last_data;
  ui08 *fdata, *cdata;

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

    full_data = (ui08 *) malloc(*nbytes_full);
    
    fdata = full_data;
    cdata = coded_data + nbytes_extra;

    last_data = cdata + (nbytes_coded - 1);

    while (cdata < last_data) {

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

	memset(fdata, byteval, runcount);
	fdata += runcount;

      } else {

	/*
	 * if most significant bit not set, set single byte
	 */

	*fdata = byteval;
	fdata++;

      } /* if ((byteval & 0x80) == 0x80) */

      cdata++;

    } /* while (cdata < last_data) */

    return (full_data);

  } else {

    return ((ui08 *) NULL);

  }

}

/**********************************************************************
 * RLDecode8() - performs run-length decoding on byte data which was
 *               compressed using RLEncode8
 *
 * Returns the full data array. The size of the array is passed back via
 * nbytes_full.
 *
 **********************************************************************/

ui08 *RLDecode8(ui08 *coded_data,
                unsigned int *nbytes_full)

{

  int runcount;
  
  ui32 *lptr;
  ui32 compress_flag;
  ui32 key;

  unsigned int nbytes_coded;
  unsigned int nbytes_extra;

  ui08 byteval;
  ui08 *full_data;
  ui08 *last_data;
  ui08 *fdata, *cdata;

  if (coded_data != NULL) {

    lptr = (ui32 *) coded_data;
    compress_flag = BE_to_ui32(*lptr);
    lptr++;

    if (compress_flag != RL8_FLAG) {

      /*
       * not compressed, return NULL
       */

      return ((ui08 *) NULL);

    }

    /*
     * get number of bytes for the coded and full data
     */

    key = BE_to_ui32(*lptr);
    lptr += 2;                         /* skip nbytes_array */
    *nbytes_full = BE_to_ui32(*lptr);
    lptr++;
    nbytes_coded = BE_to_ui32(*lptr);

    nbytes_extra = RL8_NBYTES_EXTRA;

    /*
     * get space for full data
     */

    full_data = (ui08 *) malloc(*nbytes_full);
    
    fdata = full_data;
    cdata = coded_data + nbytes_extra;

    last_data = cdata + (nbytes_coded - 1);

    while (cdata < last_data) {

      byteval = *cdata;

      if (byteval == key) {

	/*
	 * if RLE flag value, the next byte contains the run count and
	 * the byte after contains the byte value
	 */

	cdata++;
	runcount = *cdata;
	cdata++;
	byteval  = *cdata;

	/*
	 * set runcount values
	 */

	memset(fdata, byteval, runcount);
	fdata += runcount;

      } else {

	/*
	 * if not RLE flag value, set single byte
	 */

	*fdata = byteval;
	fdata++;

      } /* if (byteval == key) */

      cdata++;

    } /* while (cdata < last_data) */

    return (full_data);

  } else {

    return ((ui08 *) NULL);

  }

}

/**********************************************************************
 * RLEncode7() - performs run-length encoding on byte data which uses
 *               only the lower 7 bits
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
 **********************************************************************/

ui08 *RLEncode7(ui08 *full_data, unsigned int nbytes_full,
                unsigned int *nbytes_array)

{

  ui08 byteval;
  ui08 *coded_data;
  ui08 *fdata, *cdata, *last_data;

  unsigned int runcount;

  unsigned int nbytes_coded;
  unsigned int nbytes_extra;
  unsigned int nbytes_unpadded;

  /*
   * full_data is original array
   * fdata is pointer into original array
   * cdata is pointer into coded array
   * last_data is pointer to last byte in original array
   */

  /*
   * initial allocation of encoded array, the size of the original array
   * plus the extra bytes at the start for the nbyte values, plus enough
   * bytes to pass a word boundary. This will be sufficient for the
   * worst case in which there is no compression
   */

  nbytes_extra = RL7_NBYTES_EXTRA;
  
  coded_data = (ui08 *) malloc(nbytes_full + nbytes_extra + NBYTES_WORD);

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

  last_data = fdata + (nbytes_full - 1);

  if (full_data != NULL) {

    while (fdata < last_data) {

      /*
       * get byte value
       */

      byteval = *fdata;

      /*
       * return with NULL pointer if data exceeds 127
       */

      if (byteval > 127) {

	fprintf(stderr, "ERROR - RLEncode\n");
	fprintf(stderr, "Byte value exceeds 127.\n");
	return ((ui08 *) NULL);

      } /* if (byteval .... */
  
      runcount = 1;

      while ((fdata < last_data) && (runcount < 127) &&
	     (*(fdata + 1) == byteval)) {

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
	 *  count > 1, store as count then byte value byte
	 */

	*cdata = 0x80 | runcount;
	*(cdata + 1) = byteval;
	cdata += 2;

      }

      fdata++;

    } /* while (fdata < last_data) */

    /*
     * compute the number of bytes in the encoded data, including the 
     * leading 8 bytes and the padding to go to a word boundary
     */

    nbytes_coded = cdata - coded_data - nbytes_extra;
    nbytes_unpadded = nbytes_coded + nbytes_extra;
    *nbytes_array =
      ((unsigned int) ((nbytes_unpadded - 1) / NBYTES_WORD) + 1) * NBYTES_WORD;

    /*
     * realloc the coded_data array
     */

    coded_data = (ui08 *) realloc(coded_data, *nbytes_array);

    /*
     * set the bytes counts
     */

    *((ui32 *) coded_data) = BE_from_ui32(*nbytes_array);
    *((ui32 *) coded_data + 1) = BE_from_ui32(nbytes_full);
    *((ui32 *) coded_data + 2) = BE_from_ui32(nbytes_coded);

    return (coded_data);

  } else {

    return ((ui08 *) NULL);

  } /* if (full_data != NULL) */

}

/**********************************************************************
 * RLEncode8() - performs run-length encoding on byte data which uses
 *               all 8 bits
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
 **********************************************************************/

ui08 *RLEncode8(ui08 *full_data,
                unsigned int nbytes_full,
                unsigned int key,
                unsigned int *nbytes_array)

{

  ui08 byteval;
  ui08 *coded_data;
  ui08 *fdata, *cdata, *last_data;

  unsigned int runcount;

  ui32 *lptr;

  unsigned int nbytes_coded;
  unsigned int nbytes_extra;
  unsigned int nbytes_unpadded;

  int i;

  /*
   * full_data is original array
   * fdata is pointer into original array
   * cdata is pointer into coded array
   * last_data is pointer to last byte in original array
   */

  /*
   * initial allocation of encoded array, the size of the original array
   * plus the extra bytes at the start for the nbyte values, plus enough
   * bytes to pass a word boundary. This will be sufficient for the
   * worst case in which there is no compression
   */

  nbytes_extra = RL8_NBYTES_EXTRA;
  
  coded_data = (ui08 *) malloc(nbytes_full + nbytes_extra + NBYTES_WORD);

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

  last_data = fdata + (nbytes_full - 1);

  if (full_data != NULL) {

    while (fdata < last_data) {

      /*
       * get byte value
       */

      byteval = *fdata;

      /*
       * if data has key val, subtract 1
       */

      if (byteval == key)
	byteval--;
  
      runcount = 1;

      while ((fdata < last_data) && (runcount < 255) &&
	     (*(fdata + 1) == byteval)) {

	/*
	 * count up adjacent bytes of same value
	 */

	fdata++;
	runcount++;
	
      }

      if (runcount <= 3) {

	/*
	 * count <= 3, store as single bytes because there is no
	 * advantage to RLE
	 */

	for (i = 0; i < runcount; i++)
	  {
	    *cdata = byteval;
	    cdata++;
	  } /* endfor - i */

      } else {

	/*
	 *  count > 3, store as RLE indicator, count then byte value
	 */

	*cdata = key;
	*(cdata + 1) = runcount;
	*(cdata + 2) = byteval;
	cdata += 3;

      }

      fdata++;

    } /* while (fdata < last_data) */

    /*
     * compute the number of bytes in the encoded data, including the 
     * leading 8 bytes and the padding to go to a word boundary
     */

    nbytes_coded = cdata - coded_data - nbytes_extra;
    nbytes_unpadded = nbytes_coded + nbytes_extra;
    *nbytes_array =
      ((unsigned int) ((nbytes_unpadded - 1) / NBYTES_WORD) + 1) * NBYTES_WORD;

    /*
     * realloc the coded_data array
     */

    coded_data = (ui08 *) realloc(coded_data, *nbytes_array);

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
