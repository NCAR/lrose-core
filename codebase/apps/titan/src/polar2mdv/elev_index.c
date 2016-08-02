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
/***************************************************************************
 * elev_index.c: obtains the elevation index number given the 
 *               elevation angle
 *
 * if successful returns the index, 0-based
 *
 * if unsuccessful, returns -1
 *
 * Mike Dixon
 *
 **************************************************************************/

#include "polar2mdv.h"

#define CHECK_DELTA 0.01

si32 elev_index (double elev,
		 scan_table_t *scan_table)

{

  static si32 prev_index = -1L;
  static double prev_elev = -1000.0;

  si32 index;
  double *elev_limits;

  elev_limits = scan_table->elev_limits;

  if (fabs(elev - prev_elev) < CHECK_DELTA) {

    /*
     * same elevation as before, return the same index
     */

    return(prev_index);

  } else {

    /*
     * elev has changed, find new index
     */

    index = RfScanTableAng2ElNum(scan_table, elev);
    prev_elev = elev;
    prev_index = index;
    
    return(index);

  }

}

