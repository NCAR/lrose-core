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
 * Module: swap.c
 *
 * Author: Mike Dixon
 *	
 * Jan 1996.
 *
 */

#include <dataport/swap.h>
#include <string.h>

/**********************************************************************
 * SWAP_array_64()
 *
 * Performs an in-place 64-bit value byte swap.
 *
 * Array must be aligned.
 *
 * Returns the number of bytes converted.
 *
 */

ui32 SWAP_array_64(void *array, ui32 nbytes)
     
{
  char *ptr = (char*) array;
  ui32 i, l1,l2, ndoubles;

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
 * SWAP_array_32()
 *
 * Performs an in-place 32-bit word byte swap.
 *
 * Array must be aligned.
 *
 * Returns the number of bytes converted.
 *
 */

ui32 SWAP_array_32(void *array, ui32 nbytes)
     
{

  ui32 i, l, nlongs;
  ui32 *this_long;

  nlongs = nbytes / sizeof(ui32);
  this_long = array;
  
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
 * SWAP_array_16()
 *
 * Performs an in-place 16-bit word byte swap.
 *
 * Array must be aligned.
 *
 * Returns the number of bytes converted.
 *
 */

ui32 SWAP_array_16(void *array, ui32 nbytes)
     
{

  ui32 i, l, nlongs, nshorts;
  ui32 *this_long;
  ui16 *uarray = (ui16 *) array; 
  ui16 s;

  nlongs = nbytes / sizeof(ui32);
  this_long = (ui32 *)array;

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
    s = uarray[nshorts-1];
    uarray[nshorts-1]= (((s & (ui16) 0xff00) >> 8) | ((s & (ui16) 0x00ff) << 8));
  }

  return (nbytes);
  
}

/* See dataport/port_types.h for whish OS'es to include */
/* si64 and ui64 not defined for the others (yet) */
#if defined(IRIX6) || defined(DECOSF1) || defined(SUNOS5_64) 
/********************************
 *  SWAP_si64
 *  Swaps a single si64
 */

si64 SWAP_si64(si64 x)

{
  SWAP_array_64((void *) &x, sizeof(si64));
  return (x);
}    

/********************************
 *  SWAP_ui64
 *  Swaps a single ui64
 */

ui64 SWAP_ui64(ui64 x)

{
  SWAP_array_64((void *) &x, sizeof(ui64));
  return (x);
}    

#endif

/********************************
 *  SWAP_fl64
 *  Swaps a single fl64
 */

fl64 SWAP_fl64(fl64 x)

{
  SWAP_array_64((void *) &x, sizeof(fl64));
  return (x);
}    


/********************************
 *  SWAP_si32
 *  Swaps a single si32
 */

si32 SWAP_si32(si32 x)

{
  SWAP_array_32((ui32 *) &x, sizeof(si32));
  return (x);
}    

/********************************
 *  SWAP_ui32
 *  Swaps a single ui32
 */

ui32 SWAP_ui32(ui32 x)

{
  SWAP_array_32(&x, sizeof(ui32));
  return (x);
}    

/********************************
 *  SWAP_fl32
 *  Swaps a single fl32
 */

fl32 SWAP_fl32(fl32 x)

{
  SWAP_array_32((ui32 *) &x, sizeof(fl32));
  return (x);
}    

/********************************
 *  SWAP_si16
 *  Swaps single si16
 */

si16 SWAP_si16(si16 x)

{
  SWAP_array_16((ui16 *) &x, sizeof(si16));
  return (x);
}    

/********************************
 *  SWAP_ui16
 *  Swaps single ui16
 */

ui16 SWAP_ui16(ui16 x)

{
  SWAP_array_16((ui16 *) &x, sizeof(ui16));
  return (x);
}    

