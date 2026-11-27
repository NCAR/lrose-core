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
 * elev_index_below.c
 *
 * obtains the ext elevation index number below the point
 *                     
 * if successful returns the index, 0-based
 *
 * if unsuccessful, returns -1
 *
 * Mike Dixon
 *
 **************************************************************************/

#include "Dsr2MdvLookup.h"

#define OUT_OF_RANGE (si32) -1

si32 elev_index_below (double range,
		       double z,
		       double *cosphi,
		       double **beam_ht,
		       radar_scan_table_t *scan_table)

{

  si32 gate_num;
  si32 ielev;

  for (ielev = 0; ielev < scan_table->nelevations + 2; ielev++) {

    /*
     * get gate num
     */
    
    gate_num =
      (si32) (((range / cosphi[ielev]) - scan_table->start_range) /
	      scan_table->gate_spacing);

    if (gate_num < 0 || gate_num >= scan_table->ngates) {
      return (-1);
    }

    if (z < beam_ht[ielev][gate_num]) {
      return (ielev - 1);
    }

  } /* ielev */

  return (-1);

}

