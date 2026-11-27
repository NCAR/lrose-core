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
 * calc_geom.c
 *
 * calculate heights & ranges
 *
 * The cosphi array is returned
 *
 * RAP, NCAR, Boulder CO
 *
 * October 1990
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "Dsr2MdvLookup.h"

#define PSEUDO_RADIUS 8533.0
#define DEG_TO_RAD 0.01745329251994372

void calc_geom(si32 n_ext_elev,
	       double *cosphi,
	       double **beam_ht,
	       double **gnd_range,
	       radar_scan_table_t *scan_table)

{

  int ielev, igate;

  double twice_radius;
  double slant_range;
  double *sinphi;

  twice_radius = 2.0 * PSEUDO_RADIUS;
  
  /*
   * allocate sin of elevation angles
   */

  sinphi = (double *) umalloc((ui32) (n_ext_elev * sizeof(double)));

  for (ielev = 0; ielev < scan_table->nelevations + 2; ielev++) {

    sinphi[ielev] = sin(scan_table->ext_elev_angles[ielev] * DEG_TO_RAD);
    cosphi[ielev] = cos(scan_table->ext_elev_angles[ielev] * DEG_TO_RAD);

    slant_range = scan_table->start_range;

    for (igate = 0; igate < scan_table->ngates; igate++) {

      gnd_range[ielev][igate] = slant_range * cosphi[ielev];

      beam_ht[ielev][igate] =
	(Glob->radarz + slant_range * sinphi[ielev] +
	 slant_range * slant_range / twice_radius);

      slant_range += scan_table->gate_spacing;

    } /* igate */

  } /* ielev */

  ufree((char *) sinphi);

}
