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
 * Function:       calc_palt
 *
 * Description :
 *    calculates pressure altitude (altitude derived from pressure)
 *
 * Input:  
 *    static pressure
 * Output: 
 *    Altitude in meters
 *    a units string with the units (meters) 
 **********************************************************************/

#include "ac_tape_recover.h"

#define F2M 0.3048  /* feet to meters */

fl64 calc_palt(fl64 static_pres) 

{

   fl64 palt;

/**********************************************************************/

/* make sure static pressure is okay */
   if (static_pres == BAD_VAL) {
      fprintf(stdout,"Cannot compute palt since static_pres is bad\n");
      fprintf(stdout,"static_pres = %f \n",static_pres);
      return(BAD_VAL);
   }

   if (fabs(static_pres - 0.0) < SMALL_VAL) {
      fprintf(stdout,"Cannot comput stemp since input data bad\n");
      fprintf(stdout,"static_pres = %f\n", static_pres);
      return(BAD_VAL);
   }
    
/* pressure altitude */
   palt = 145450.0*(1.0 - pow(static_pres/1013.25,.1903));

/* convert feet to meters */
   palt = palt*F2M;

   return(palt);

}
