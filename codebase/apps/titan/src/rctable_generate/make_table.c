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
 * make_table.c: makes look-up table
 *
 * RAP, NCAR, Boulder CO
 *
 * October 1990
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "rctable_generate.h"

/*******************
 * make_table_cart()
 *
 * CartMode
 */

void make_table_cart(si32 *rindex_table,
		     double *cosphi,
		     double **beam_ht,
		     double **gnd_range,
		     scan_table_t *scan_table)
     
{

  si32 ix, iy, iz;
  si32 pos = 0, index;
  double delx, dely, range;
  double azimuth;
  double x, y, z;
  
  /*
   * loop through height planes - z dimension
   */

  for (iz = 0; iz < Glob->nz; iz++) {

    z = Glob->minz + iz * Glob->dz;

    printf("make_table_cart: iz, z = %d, %g\n", iz, z);

    /*
     * loop through y dimension
     */

    for (iy = 0; iy < Glob->ny; iy++) {

      y = Glob->miny + iy * Glob->dy;

      dely = y - Glob->radary;

      /*
       * loop through x dimension
       */

      for (ix = 0; ix < Glob->nx; ix++, pos++) {

	x = Glob->minx + ix * Glob->dx;

	delx = x - Glob->radarx;

	/*
	 * if cartesian point is over radar, missing data
	 */

	if (delx == 0 && dely == 0) {

	  rindex_table[pos] = scan_table->missing_data_index;

	} else {

	  azimuth = ((atan2(delx, dely) * RAD_TO_DEG) +
		     Glob->rotation_at_radar);

	  range = sqrt(delx * delx + dely * dely);

	  if (scan_table->use_azimuth_table) {
	    
	    index = cappi_index_irregular_azs(azimuth, range,
					      delx, dely, z,
					      cosphi, beam_ht,
					      gnd_range, scan_table);
	  } else {

	    index = cappi_index_regular_azs(azimuth, range, z,
					    cosphi, beam_ht,
					    gnd_range, scan_table);
	    
	  } /* if (scan_table->use_azimuth_table)*/
	  
	  rindex_table[pos] = index;

	} /* if (delx == 0 && dely == 0) */

      } /* ix */

    } /* iy */

  } /* iz */

}

/******************
 * make_table_ppi()
 *
 * PpiMode
 */

void make_table_ppi(si32 *rindex_table,
		    double *cosphi,
		    double **beam_ht,
		    double **gnd_range,
		    scan_table_t *scan_table)
     
{

  si32 ix, iy, iz;
  si32 pos = 0, index;
  double delx, dely, range;
  double azimuth;
  double x, y;
  
  /*
   * z - loop through height planes (CartMode) or
   * elevation angles (PpiMode)
   */

  for (iz = 0; iz < Glob->nz; iz++) {

    printf("make_table: iz = %d\n", iz);

    /*
     * loop through lines (y)
     */

    for (iy = 0; iy < Glob->ny; iy++) {

      y = Glob->miny + iy * Glob->dy;

      dely = y - Glob->radary;

      /*
       * loop through samples (x)
       */
      
      for (ix = 0; ix < Glob->nx; ix++, pos++) {

	x = Glob->minx + ix * Glob->dx;

	delx = x - Glob->radarx;

	/*
	 * if cartesian point is over radar, missing data
	 */

	if (delx == 0 && dely == 0) {

	  rindex_table[pos] = scan_table->missing_data_index;

	} else {

	  azimuth = ((atan2(delx, dely) * RAD_TO_DEG) +
		     Glob->rotation_at_radar);

	  range = sqrt(delx * delx + dely * dely);

	  if (scan_table->use_azimuth_table) {

	    index = ppi_index_irregular_azs(azimuth, range, iz,
					    cosphi, gnd_range, scan_table);
	    
	  } else {

	    index = ppi_index_regular_azs(azimuth, range, iz,
					  cosphi, gnd_range, scan_table);

	  } /* if (scan_table->use_azimuth_table)*/
	  
	  rindex_table[pos] = index;

	} /* if (delx == 0 && dely == 0) */

      } /* ix */

    } /* iy */

  } /* iz */

}

void make_table_polar(si32 *rindex_table)

{
  
  si32 ix, iy, iz;
  si32 pos = 0;
  
  /*
   * loop through elevation angles (z)
   */
  
  for (iz = 0; iz < Glob->nz; iz++) {

    printf("make_table_polar: iz = %d\n", iz);

    /*
     * loop through azimuths (y)
     */

    for (iy = 0; iy < Glob->ny; iy++) {

      /*
       * loop through gates (x)
       */

      for (ix = 0; ix < Glob->nx; ix++, pos++) {

	rindex_table[pos] = pos;

      } /* ix */

    } /* iy */

  } /* i */

}

