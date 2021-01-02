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
/*********************************************************************
 * transform.c: perform the transformation
 *
 * RAP, NCAR, Boulder CO
 *
 * October 1990
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "cart_convert.h"

void transform(vol_file_handle_t *rv_handle,
	       vol_file_handle_t *cv_handle,
	       rc_table_file_handle_t *rc_handle)

{

  ui08 **cdata, **beam;

  si32 ifield, ielev, iaz, ipoint;
  si32 nfields;
  si32 nelevations, nazimuths, ngates;
  si32 npoints;

  rc_table_index_t **rc_table_index;
  rc_table_entry_t *rc_entry;

  /*
   * set constants
   */

  nfields = cv_handle->vol_params->nfields;
  nelevations = cv_handle->vol_params->radar.nelevations;
  nazimuths = cv_handle->vol_params->radar.nazimuths;
  ngates = cv_handle->vol_params->radar.ngates;
  rc_table_index = rc_handle->table_index;

  /*
   * allocate cdata array, and set to it to
   * point to the cart-coord data
   */

  cdata = (ui08 **) umalloc
    ((ui32) (nfields * sizeof(ui08 *)));

  beam = (ui08 **) umalloc
    ((ui32) (nfields * sizeof(ui08 *)));

  for (ifield = 0; ifield < nfields; ifield++)
    cdata[ifield] = *cv_handle->field_plane[ifield];

  /*
   * loop through elevations
   */

  for (ielev = 0; ielev < nelevations; ielev++) {

    /*
     * loop through azimuths
     */
    
    for (iaz = 0; iaz < nazimuths; iaz++) {

      for (ifield = 0; ifield < nfields; ifield++)
	beam[ifield] =
	  rv_handle->field_plane[ifield][ielev] + (iaz * ngates);

      /*
       * where applicable, place rdata values into cdata array
       */

      npoints = rc_table_index[ielev][iaz].npoints;

      if (npoints > 0) {
	  
	rc_entry = rc_table_index[ielev][iaz].u.entry;

	for (ipoint = 0; ipoint < npoints; ipoint++) {

	  for (ifield = 0; ifield < nfields; ifield++)
	    cdata[ifield][rc_entry->index] =
	      beam[ifield][rc_entry->gate];

	  rc_entry++;

	} /* ipoint */

      } /* if (npoints > 0) */

    } /* iaz */

  } /* ielev */

  /*
   * free up resources
   */

  ufree((char *) cdata);
  ufree((char *) beam);

}
