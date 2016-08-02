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
 * cappi_index.c
 *
 * Get the index for a position in the table for CAPPI
 *
 * RAP, NCAR, Boulder CO
 *
 * October 1990
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "rctable_generate.h"

/*
 * cappi_index_regular_azs()
 *
 * Deals with a regular azimuth pattern scan
 */

si32 cappi_index_regular_azs(double azimuth,
			     double range,
			     double z,
			     double *cosphi,
			     double **beam_ht,
			     double **gnd_range,
			     scan_table_t *scan_table)

{

  si32 azimuth_index, range_index, elev_index;
  si32 ielev_below, ielev_above;
  si32 irange_below, irange_above;
  double error_below, error_above;
  
  /*
   * get elevation index below the computed elevation
   */

  if ((ielev_below = elev_index_below(range, z, cosphi,
				      beam_ht, scan_table)) == -1) {
    return (scan_table->missing_data_index);
  }
  ielev_above = ielev_below + 1;

  /*
   * compute the azimuth index (i.e. pos in array in azimuth direction)
   * from the azimuth
   */

  azimuth_index = (si32) floor((azimuth - scan_table->start_azimuth) /
			       scan_table->delta_azimuth + 0.5);

  if (azimuth_index >= scan_table->nazimuths) {
    azimuth_index -= scan_table->nazimuths;
  } else if (azimuth_index < 0) {
    azimuth_index += scan_table->nazimuths;
  }

  /*
   * get the error distance for the elevations above and below
   * the point
   */

  error_below = get_error_regular_azs(ielev_below, range, z, &irange_below,
				      cosphi, beam_ht, gnd_range,
				      scan_table);
  
  error_above = get_error_regular_azs(ielev_above, range, z, &irange_above,
				      cosphi, beam_ht, gnd_range,
				      scan_table);

  /*
   * if either error is -1, outside scan limits, return missing data index
   */
  
  if (error_below < 0 || error_above < 0) {
    return(scan_table->missing_data_index);
  }

  /*
   * choose the least error
   */

  if (error_below < error_above) {
    range_index = irange_below;
    elev_index = ielev_below - 1;
  } else {
    range_index = irange_above;
    elev_index = ielev_above - 1;
  }

  if (elev_index >= 0 && elev_index < scan_table->nelevations) {

    /*
     * the point is within the radar data space, so compute
     * the table index and return
     */

    return(elev_index * scan_table->nazimuths * scan_table->ngates +
	   azimuth_index * scan_table->ngates +
	   range_index);

  } else {

    /*
     * the point is outside radar data space, return missing data index
     */

    return(scan_table->missing_data_index);

  }

}

/*
 * cappi_index_irregular_azs()
 *
 * Deals with a irregular azimuth pattern scan specified by
 * an azimuth table
 */

si32 cappi_index_irregular_azs(double azimuth,
			       double range,
			       double x,
			       double y,
			       double z,
			       double *cosphi,
			       double **beam_ht,
			       double **gnd_range,
			       scan_table_t *scan_table)

{

  si32 index;
  si32 az_index, beam_index;
  si32 range_index, elev_index;
  si32 ielev_below, ielev_above;
  si32 iaz_below, iaz_above;
  si32 irange_below, irange_above;
  double error_below, error_above;
  
  /*
   * get elevation index below the computed elevation
   */

  if ((ielev_below = elev_index_below(range, z, cosphi,
				      beam_ht, scan_table)) == -1) {
    return (scan_table->missing_data_index);
  }
  ielev_above = ielev_below + 1;

  /*
   * compute the azimuth index (i.e. pos in array in azimuth direction)
   * from the azimuth
   */

  iaz_below = RfScanTableAng2AzNum(scan_table->ext_elevs + ielev_below,
				   azimuth);
  iaz_above = RfScanTableAng2AzNum(scan_table->ext_elevs + ielev_above,
				   azimuth);

  if (iaz_below < 0 || iaz_above < 0) {

    /*
     * azimuth is out of limits
     */

    return (scan_table->missing_data_index);

  }

  /*
   * get the error distance for the elevations above and below
   * the point
   */

  error_below =
    get_error_irregular_azs(iaz_below, ielev_below, range, x, y, z,
			    &irange_below,
			    cosphi, beam_ht, gnd_range,
			    scan_table);
  
  error_above =
    get_error_irregular_azs(iaz_above, ielev_above, range, x, y, z,
			    &irange_above,
			    cosphi, beam_ht, gnd_range,
			    scan_table);

  /*
   * if either error is -1, outside scan limits, return missing data index
   */
  
  if (error_below < 0 || error_above < 0) {
    return(scan_table->missing_data_index);
  }

  /*
   * choose the least error
   */

  if (error_below < error_above) {
    az_index = iaz_below;
    range_index = irange_below;
    elev_index = ielev_below - 1;
  } else {
    az_index = iaz_above;
    range_index = irange_above;
    elev_index = ielev_above - 1;
  }

  if (elev_index >= 0 && elev_index < scan_table->nelevations) {

    /*
     * the point is within the radar data space, so compute
     * the table index and return
     */

    beam_index = scan_table->elevs[elev_index].azs[az_index].beam_num;
    index = beam_index * scan_table->ngates + range_index;

  } else {

    /*
     * the point is outside radar data space, return missing data index
     */

    index = scan_table->missing_data_index;

  }

  return (index);


}





