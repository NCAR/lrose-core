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
 * get_error.c: get the minimum error distance between the desired point
 *              and any data point on this elevation
 *
 * RAP, NCAR, Boulder CO
 *
 * October 1990
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "Dsr2MdvLookup.h"

/*
 * get_error_regular_azs()
 *
 * Gets error for case with regular azimuth scan
 */

double get_error_regular_azs(si32 elev_index,
			     double range,
			     double z,
			     si32 *range_index_p,
			     double *cosphi,
			     double **beam_ht,
			     double **gnd_range,
			     radar_scan_table_t *scan_table)
{

  si32 i;
  si32 range_index;
  si32 irange;
  si32 shorter_successful = FALSE;
  double error_min, error;
  double error_z, error_r;

  /*
   * return -1 if range is not within limits
   */

  if (range < gnd_range[elev_index][0] ||
      range > gnd_range[elev_index][scan_table->ngates - 1]) {
    return (-1.0);
  }
  
  /*
   * compute the range index at which to begin the search
   */

  irange =
    (si32) (((range / cosphi[elev_index]) - scan_table->start_range) /
	    scan_table->gate_spacing);

  /*
   * compute the distance from cartesian point to radar point
   */

  error_z = z - beam_ht[elev_index][irange];
  error_r = range - gnd_range[elev_index][irange];
  error = sqrt(error_z * error_z + error_r * error_r);
  error_min = error;
  range_index = irange;

  /*
   * now search for shorter ranges which may have lower error values
   */

  for (i = irange - 1; i >= 0; i--) {

    /*
     * compute error
     */

    error_z = z - beam_ht[elev_index][i];
    error_r = range - gnd_range[elev_index][i];
    error = sqrt(error_z * error_z + error_r * error_r);

    if (error < error_min) {

      /*
       * if error is lower than before, we're on the right track
       * store the min error and range index
       */

      error_min = error;
      range_index = i;
      shorter_successful = TRUE;

    } else {

      /*
       * if error not lower, break out
       */

      break;

    }

  }

  /*
   * if the search at shorter ranges was not successful, try longer
   * ranges
   */

  if (shorter_successful == FALSE) {

    for (i = irange + 1; i < scan_table->ngates; i++) {

      /*
       * compute error
       */

      error_z = z - beam_ht[elev_index][i];
      error_r = range - gnd_range[elev_index][i];
      error = sqrt(error_z * error_z + error_r * error_r);

      if (error < error_min) {
	
	/*
	 * if error is lower than before, we're on the right track
	 * store the min error and range index
	 */

	error_min = error;
	range_index = i;

      } else {

	/*
	 * if error not lower, break out
	 */

	break;

      }

    }

  } /* if (shorter_successful........... */

  /*
   * return the min error value found, and the range index
   */

  *range_index_p = range_index;
  return (error_min);

}

/*
 * get_error_irregular_azs()
 *
 * Gets error for case with irregular azimuth scan specified
 * by azimith table
 */

double get_error_irregular_azs(si32 az_index,
			       si32 elev_index,
			       double range,
			       double x,
			       double y,
			       double z,
			       si32 *range_index_p,
			       double *cosphi,
			       double **beam_ht,
			       double **gnd_range,
			       radar_scan_table_t *scan_table)
{

  si32 i;
  si32 range_index;
  si32 irange;
  si32 shorter_successful = FALSE;
  double err_min = 1.0e99;
  double err;
  double err_x, err_y, err_z;
  double azimuth;
  double gate_x, gate_y, gate_z;
  double sinaz, cosaz;
  radar_scan_table_elev_t *elev;

  /*
   * return -1 if range is not within limits
   */

  if (range < gnd_range[elev_index][0] ||
      range > gnd_range[elev_index][scan_table->ngates - 1]) {
    return (-1.0);
  }
  
  /*
   * get azimuth and trig vals
   */

  elev = scan_table->ext_elevs + elev_index;
  azimuth = elev->azs[az_index].angle;
  
  sinaz = sin(azimuth * DEG_TO_RAD);
  cosaz = cos(azimuth * DEG_TO_RAD);

  /*
   * compute the range index at which to begin the search
   */

  irange =
    (si32) (((range / cosphi[elev_index]) - scan_table->start_range) /
	    scan_table->gate_spacing);

  /*
   * compute the distance from cartesian point to radar point
   */

  gate_x = gnd_range[elev_index][irange] * sinaz;
  gate_y = gnd_range[elev_index][irange] * cosaz;
  gate_z = beam_ht[elev_index][irange];
  err_x = x - gate_x;
  err_y = y - gate_y;
  err_z = z - gate_z;
  err = sqrt(err_x * err_x + err_y * err_y + err_z * err_z);

  range_index = irange;

  /*
   * now search for shorter ranges which may have lower error values
   */

  for (i = irange - 1; i >= 0; i--) {

    /*
     * compute error
     */

    gate_x = gnd_range[elev_index][irange] * sinaz;
    gate_y = gnd_range[elev_index][irange] * cosaz;
    gate_z = beam_ht[elev_index][irange];
    err_x = x - gate_x;
    err_y = y - gate_y;
    err_z = z - gate_z;
    err = sqrt(err_x * err_x + err_y * err_y + err_z * err_z);
    
    if (err < err_min) {

      /*
       * if error is lower than before, we're on the right track
       * store the min error and range index
       */

      err_min = err;
      range_index = i;
      shorter_successful = TRUE;

    } else {

      /*
       * if error not lower, break out
       */

      break;

    }

  }

  /*
   * if the search at shorter ranges was not successful, try longer
   * ranges
   */

  if (shorter_successful == FALSE) {

    for (i = irange + 1; i < scan_table->ngates; i++) {

      /*
       * compute error
       */

      gate_x = gnd_range[elev_index][irange] * sinaz;
      gate_y = gnd_range[elev_index][irange] * cosaz;
      gate_z = beam_ht[elev_index][irange];
      err_x = x - gate_x;
      err_y = y - gate_y;
      err_z = z - gate_z;
      err = sqrt(err_x * err_x + err_y * err_y + err_z * err_z);

      if (err < err_min) {
	
	/*
	 * if error is lower than before, we're on the right track
	 * store the min error and range index
	 */

	err_min = err;
	range_index = i;

      } else {

	/*
	 * if error not lower, break out
	 */

	break;

      }

    }

  } /* if (shorter_successful........... */

  /*
   * return the min error value found, and the range index
   */

  *range_index_p = range_index;
  return (err_min);

}




