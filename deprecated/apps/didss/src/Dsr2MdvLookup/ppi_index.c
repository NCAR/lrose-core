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
 * ppi_index.c
 *
 * Get the index for a position in the table for PPI
 *
 * RAP, NCAR, Boulder CO
 *
 * October 1990
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "Dsr2MdvLookup.h"

static int ppi_gate_index(double range,
			  int ielev,
			  double *cosphi,
			  double **gnd_range,
			  radar_scan_table_t *scan_table);

/*
 * ppi_index_regular_azs()
 *
 * Deals with a regular azimuth pattern scan
 */

si32 ppi_index_regular_azs(double azimuth,
			   double range,
			   int ielev,
			   double *cosphi,
			   double **gnd_range,
			   radar_scan_table_t *scan_table)

{

  si32 az_index, gate_index;
  
  /*
   * compute the azimuth index (i.e. pos in array in azimuth direction)
   * from the azimuth
   */

  az_index = (si32) floor((azimuth - scan_table->start_azimuth) /
			  scan_table->delta_azimuth + 0.5);
  
  if (az_index >= scan_table->nazimuths) {
    az_index -= scan_table->nazimuths;
  } else if (az_index < 0) {
    az_index += scan_table->nazimuths;
  }

  /*
   * compute the index in range
   */

  gate_index = ppi_gate_index(range, ielev, cosphi, gnd_range, scan_table);
  
  if (gate_index < 0) {
    
    /*
     * the point is outside radar data space, return missing data index
     */
    
    return(scan_table->missing_data_index);

  } else {

    /*
     * the point is within the radar data space, so compute
     * the table index and return
     */

    return(ielev * scan_table->nazimuths * scan_table->ngates +
	   az_index * scan_table->ngates +
	   gate_index);
    
  }

}

/*
 * ppi_index_irregular_azs()
 *
 * Deals with a irregular azimuth pattern scan specified by
 * an azimuth table
 */

si32 ppi_index_irregular_azs(double azimuth,
			     double range,
			     int ielev,
			     double *cosphi,
			     double **gnd_range,
			     radar_scan_table_t *scan_table)

{

  si32 index;
  si32 az_index, beam_index;
  si32 gate_index, elev_index;
  
  /*
   * compute the azimuth index (i.e. pos in array in azimuth direction)
   * from the azimuth
   */

  az_index = RadarScanTableAng2AzNum(scan_table->ext_elevs + ielev,
				     azimuth);
  
  if (az_index < 0) {
    
    /*
     * azimuth is out of limits
     */
    
    return (scan_table->missing_data_index);
    
  }

  /*
   * compute the index in range
   */

  gate_index = ppi_gate_index(range, ielev, cosphi, gnd_range, scan_table);

  if (gate_index < 0) {

    /*
     * the point is outside radar data space, return missing data index
     */

    return(scan_table->missing_data_index);

  } else {

    /*
     * the point is within the radar data space, so compute
     * the table index and return
     */

    beam_index = scan_table->elevs[elev_index].azs[az_index].beam_num;
    index = beam_index * scan_table->ngates + gate_index;
    return (index);
    
  }

  return (index);


}

#ifdef NOTNOW

/***********************************************
 * ppi_gate_index()
 *
 * Get index of range in range array.
 *
 * returns range index on success, -1 on failure
 */

static int ppi_gate_index(double range,
			  int ielev,
			  double *cosphi,
			  double **gnd_range,
			  radar_scan_table_t *scan_table)
     
{

  int igate;
  si32 gate_index;
  double error;
  double error_min = 1.0e99;

  /*
   * compute the gate index at which to begin the search
   */
  
  gate_index =
    (si32) (((range / cosphi[ielev]) - scan_table->start_range) /
	    scan_table->gate_spacing);

  /*
   * return -1 if gate_index is not within limits
   */

  if (gate_index < 0 || gate_index >= scan_table->ngates) {
    return (-1.0);
  }

  for (igate = 0; igate < scan_table->ngates; igate++) {

    error = fabs(range - gnd_range[ielev][igate]);

    if (error < error_min) {
      error_min = error;
    } else {
      return (igate - 1);
    }

  } /* igate */
  
  return (-1);

}

#endif

/***********************************************
 * ppi_gate_index()
 *
 * Get index of gate in gate array.
 *
 * returns gate index on success, -1 on failure
 */

static int ppi_gate_index(double range,
			  int ielev,
			  double *cosphi,
			  double **gnd_range,
			  radar_scan_table_t *scan_table)

{

  si32 gate_index;
  si32 igate;
  si32 shorter_successful = FALSE;
  double error;
  double error_min;

  /*
   * return -1 if range is not within limits
   */

  if (range < gnd_range[ielev][0] ||
      range > gnd_range[ielev][scan_table->ngates - 1]) {
    return (-1.0);
  }
  
  /*
   * compute the gate index at which to begin the search
   */
  
  gate_index =
    (si32) (((range / cosphi[ielev]) - scan_table->start_range) /
	    scan_table->gate_spacing);

  error_min = fabs(range - gnd_range[ielev][gate_index]);
  
  /*
   * now search for shorter ranges which may have lower error values
   */
  
  for (igate = gate_index - 1; igate >= 0; igate--) {

    /*
     * compute error
     */

    error = fabs(range - gnd_range[ielev][igate]);

    if (error < error_min) {

      /*
       * if error is lower than before, we're on the right track
       * store the min error and gate index
       */

      error_min = error;
      gate_index = igate;
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

    for (igate = gate_index + 1; igate < scan_table->ngates; igate++) {

      /*
       * compute error
       */
      
      error = fabs(range - gnd_range[ielev][igate]);

      if (error < error_min) {
	
	/*
	 * if error is lower than before, we're on the right track
	 * store the min error and gate index
	 */

	error_min = error;
	gate_index = igate;

      } else {

	/*
	 * if error not lower, break out
	 */

	break;

      }

    }

  } /* if (shorter_successful........... */

  /*
   * return the gate index
   */

  return (gate_index);

}


