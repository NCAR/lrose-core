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
/*
 * Module: smallend.c
 *
 * Author: Nancy Rehak
 *	
 * Date:   Feb 3, 1998
 *
 * Copied from bigend.c
 *
 * See smallend.h for details
 *
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <dataport/smallend.h>

static int SmallEnd = 1;

/***********************************************
 * SE_reverse()
 *
 * Reverses the sense of this library. Therefore,
 * is called once, BigEndian values are set.
 * If called twice, goes back to SmallEndian.
 */

void SE_reverse(void)

{
  SmallEnd = !SmallEnd;
}

/************************************************
 * Return 1 if host is small_endian and 0 otherwise
 *
 * For debugging, if FORCE_SWAP is set, this routine will
 * always return FALSE, forcing a swap.
 */

int
SE_is_small_endian()
{
  
#ifdef FORCE_SWAP

  return (0);

#else

  union 
    {
      ui16    d;
      ui08     bytes[2];
    }
  short_int;

  short_int.d = 1;
  if (short_int.bytes[0] != 0)
    return (SmallEnd);
  else
    return (!SmallEnd);
  
#endif

}

/****************************************************
 * SE_swap_array_64()
 *
 * Performs an in-place 64-bit value byte swap.
 *
 * Array must be aligned.
 *
 * Returns the number of bytes converted.
 */

si32
SE_swap_array_64(void *array, ui32 nbytes)

{
  static int small_endian;
  static int one_time = 0;
  char *ptr = (char*) array;
  ui32 i, l1,l2, ndoubles;

  /* check for little or big endian */
  if (one_time == 0) {
    small_endian = SE_is_small_endian ();
    one_time = 1;
  }
  
  if (small_endian) {
    return (0);
  }
  
  ndoubles = nbytes / 8;

  for (i = 0; i < ndoubles; i++) {

    /* Copy the 8 bytes to 2 ui32's - Reversing 1st & 2nd */
    /* PTR                 L1      L2      */
    /* 1 2 3 4 5 6 7 8 ->  5 6 7 8 1 2 3 4 */
    memcpy((void*)&l2,(void*)ptr,4);
    memcpy((void*)&l1,(void*)(ptr+4),4);


    /* Reverse the 4 bytes of each ui32 */
    /* 5 6 7 8  -> 8 7 6 5  */
    l1 = (((l1 & 0xff000000) >> 24) |
	  ((l1 & 0x00ff0000) >> 8) |
	  ((l1 & 0x0000ff00) << 8) |
	  ((l1 & 0x000000ff) << 24));

    /* 1 2 3 4 -> 4 3 2 1 */
    l2 = (((l2 & 0xff000000) >> 24) |
	  ((l2 & 0x00ff0000) >> 8) |
	  ((l2 & 0x0000ff00) << 8) |
	  ((l2 & 0x000000ff) << 24));


    /* Copy the reversed value back into place */
    memcpy((void*)ptr,(void*)&l1,4);
    memcpy((void*)(ptr+4),(void*)&l2,4);

    ptr+=8;  /* Move to the next 8 byte value */
  }

  return (nbytes);
}

/**********************************************************************
 * SE_swap_array_32()
 *
 * Performs an in-place 32-bit word byte swap, if necessary, to produce
 * SE representation from machine representation, or vice-versa.
 *
 * Array must be aligned.
 *
 * Returns the number of bytes converted.
 *
 */

si32
SE_swap_array_32(void *array, ui32 nbytes)
     
{

  static int small_endian;
  static int one_time = 0;
  ui32 i, l, nlongs;
  ui32 *this_long;
  ui32 *array32 = array;

  /* check for little or big endian */
  if (one_time == 0) {
    small_endian = SE_is_small_endian ();
    one_time = 1;
  }
  
  if (small_endian) {
    return (0);
  }
  
  nlongs = nbytes / sizeof(ui32);
  this_long = array32;
  
  for (i = 0; i < nlongs; i++) {

    l = *this_long;
    
    *this_long = (((l & 0xff000000) >> 24) |
		  ((l & 0x00ff0000) >> 8) |
		  ((l & 0x0000ff00) << 8) |
		  ((l & 0x000000ff) << 24));
    
    this_long++;

  }

  return (nbytes);

}

/**********************************************************************
 * SE_swap_array_16()
 *
 * Performs an in-place 16-bit word byte swap, if necessary, to produce
 * SE representation from machine representation, or vice-versa.
 *
 * Array must be aligned.
 *
 * Returns the number of bytes converted.
 *
 */

si32
SE_swap_array_16(void *array, ui32 nbytes)
     
{

  static int small_endian;
  static int one_time = 0;
  ui32 i, l, nlongs, nshorts;
  ui32 *this_long;
  ui16 *array16 = array;
  ui16 s;

  /* check for little or big endian */
  if (one_time == 0) {
    small_endian = SE_is_small_endian ();
    one_time = 1;
  }
  
  if (small_endian) {
    return (0);
  }
  
  nlongs = nbytes / sizeof(ui32);
  this_long = (ui32 *)array16;

  for (i = 0; i < nlongs; i++) {
    
    l = *this_long;
    
    *this_long = (((l & 0xff000000) >> 8) |
		   ((l & 0x00ff0000) << 8) |
		   ((l & 0x0000ff00) >> 8) |
		   ((l & 0x000000ff) << 8));

    this_long++;
  }
  
  if (nlongs * sizeof(ui32) != nbytes) {
    nshorts = nbytes / sizeof(ui16);
    s = array16[nshorts-1];
    array16[nshorts-1]= (((s & 0xff00) >> 8) | ((s & 0x00ff) << 8));
  }

  return (nbytes);
  
}

/*****************************************************
 * the to- and from- routines are identical,
 * and may be implemented in terms of SE_swap routines
 *
 * Later we may duplicate the code for effiency, to
 * avoid the calling overhead.
 */

/********************
 * SE_from_array_64()
 * Converts an array of 64s
 */

si32 SE_from_array_64(void *array, ui32 nbytes)

{
  return (SE_swap_array_64(array, nbytes));
}
     
/******************
 * SE_to_array_64()
 * Converts an array of 64s
 */

si32 SE_to_array_64(void *array, ui32 nbytes)

{
  return (SE_swap_array_64(array, nbytes));
}
     
/********************
 * SE_from_array_32()
 * Converts an array of 32s
 */

si32 SE_from_array_32(void *array, ui32 nbytes)

{
  return (SE_swap_array_32(array, nbytes));
}
     
/******************
 * SE_to_array_32()
 * Converts an array of 32s
 */

si32 SE_to_array_32(void *array, ui32 nbytes)

{
  return (SE_swap_array_32(array, nbytes));
}
     
/********************
 * SE_from_array_16()
 * Converts an array of 16s
 */

si32 SE_from_array_16(void *array, ui32 nbytes)

{
  return (SE_swap_array_16(array, nbytes));
}
     
/******************
 * SE_to_array_16()
 * Converts an array of 16s
 */

si32 SE_to_array_16(void *array, ui32 nbytes)

{
  return (SE_swap_array_16(array, nbytes));
}

/********************************
 *  SE_from_si32 replaces htonl()
 *  Converts a single si32
 */

si32 SE_from_si32(si32 x)

{
  SE_swap_array_32((void *) &x, sizeof(si32));
  return (x);
}    

/******************************
 *  SE_to_si32 replaces ntohl()
 *  Converts a single si32
 */

si32 SE_to_si32(si32 x)

{
  SE_swap_array_32(&x, sizeof(si32));
  return (x);
}    

/********************************
 *  SE_from_si16 replaces htons()
 *  Converts a single si16
 */

si16 SE_from_si16(si16 x)

{
  SE_swap_array_16(&x, sizeof(si16));
  return (x);
}    

/******************************
 *  SE_to_si16 replaces ntohs()
 *  Converts a single si16
 */

si16 SE_to_si16(si16 x)

{
  SE_swap_array_16(&x, sizeof(si16));
  return (x);
}    

/********************************
 *  SE_from_ui32
 *  Converts a single ui32
 */

ui32 SE_from_ui32(ui32 x)

{
  SE_swap_array_32(&x, sizeof(ui32));
  return (x);
}    

/******************************
 *  SE_to_ui32
 *  Converts a single ui32
 */

ui32 SE_to_ui32(ui32 x)

{
  SE_swap_array_32(&x, sizeof(ui32));
  return (x);
}    

/********************************
 *  SE_from_fl32
 *  Converts a single fl32
 */

fl32 SE_from_fl32(fl32 x)

{
  SE_swap_array_32(&x, sizeof(fl32));
  return (x);
}    

/******************************
 *  SE_to_fl32
 *  Converts a single fl32
 */

fl32 SE_to_fl32(fl32 x)

{
  SE_swap_array_32(&x, sizeof(fl32));
  return (x);
}    

/********************************
 *  SE_from_ui16
 *  Converts a single ui16
 */

ui16 SE_from_ui16(ui16 x)

{
  SE_swap_array_16(&x, sizeof(ui16));
  return (x);
}    

/******************************
 *  SE_to_ui16
 *  Converts a single ui16
 */

ui16 SE_to_ui16(ui16 x)

{
  SE_swap_array_16(&x, sizeof(ui16));
  return (x);
}    

