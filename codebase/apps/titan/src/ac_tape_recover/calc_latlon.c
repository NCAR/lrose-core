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
 * Function:       calc_latlon
 *
 * Description:
 *    calculates a latitude and longitude in degrees 
 *
 * Input:  
 *    an integer which stores sign, degrees, minutes and seconds
 *    in each of it's 4 bytes.
 * Output: 
 *    the latitude or longitude
 *    a units string with the units (degrees) 
 **********************************************************************/

#include "ac_tape_recover.h"

#define DEG2RAD .017453292


fl64 calc_latlon(si32 *int_val)
{

  si32 native_int_val;
  si32 nswap;
  fl64 latlon;
  ui08 *latlon_ptr = NULL;
  fl32 degrees;
  fl32 minutes;
  fl32 dec_min;
  fl32 sign;
/**********************************************************************/

   native_int_val = *int_val;

   /* Since data is stored in one byte pieces, data was swapped earlier
    * when it didn't need to be...  Swap it back now.. */

   if (Glob->swap_data) {
      nswap = SWAP_array_32((ui32 *) &native_int_val,sizeof(si32)); 
      if (nswap != sizeof(si32)) 
         exit_str("Could not swap lat/lon information");
   }

   latlon_ptr = (ui08 *)&native_int_val;

/* determine the sign of the value */
   sign = (fl32)(*latlon_ptr);
   if (sign == 0) 
      sign = 1.0;
   else
      sign = -1.0;

   latlon_ptr ++;

/* degrees */
   degrees = (fl32)(*latlon_ptr);
   latlon_ptr ++;

/* minutes */
   minutes = (fl32)(*latlon_ptr);
   latlon_ptr ++;

/* decimal minutes */
   dec_min = (fl32)(*latlon_ptr);

/* stuff into double */
   latlon = (fl64)((sign)*(degrees + (minutes/60.) + (dec_min/6000.)));

   return(latlon);

}
