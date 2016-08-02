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
 * compute_lookup.c
 *
 * Performs the lookup computations
 *
 * Mike Dixon
 * Marion Mittermaier
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * Aug 1998
 *
 ****************************************************************************/

#include "dva_mklookup.h"

void compute_lookup(int iz, int nlookup,
		    dva_rdas_cal_t *cal,
		    dva_grid_t *grid,
		    dva_position_t *pos,
		    float *range,
		    dva_lookup_t *lookup)
     
{

  double az, az_plus_one;
  double vipdiff, dbzdiff, vips_per_dB, delta_l, factor;
  double maxx, maxy;
  double min_range, max_range;
  double gate_spacing;
  double xx, yy, rd, ad, ed, rng, radius;
  double *elev = Glob->params.radar_elevations.val;

  int i, j, l, rc;
  int n, ngates;
  int nelev = Glob->params.radar_elevations.len;
  
  vipdiff = cal->viphi - cal->viplo;
  dbzdiff = cal->dbzhi - cal->dbzlo;
  vips_per_dB  = vipdiff/dbzdiff;

  if (Glob->params.debug >= DEBUG_VERBOSE) {
    fprintf (stderr, "Vips per dB = %f \n", vips_per_dB);
  }
	
  if (Glob->params.debug >= DEBUG_VERBOSE) {
    fprintf (stderr, "Cappi %d - Grid point matching in progress ... \n", iz); 
  }

  /*
   * set up min and max values
   */
  
  maxx = grid->minx + ((double) grid->nx + 0.5) * grid->dx;
  maxy = grid->miny + ((double) grid->ny + 0.5) * grid->dy;

  min_range = range[1];
  max_range = range[cal->ngates] + cal->gate_spacing;
  gate_spacing = cal->gate_spacing;
  ngates = cal->ngates;

  if (Glob->params.debug >= DEBUG_VERBOSE) {
    fprintf(stderr, "maxx, maxy: %g, %g\n", maxx, maxy);
    fprintf(stderr, "min_range, max_range: %g, %g\n", min_range, max_range);
  }

  /*
   * Calculation of weighting factors
   */
  
  for ( n = 0; n < nlookup; n ++ ) {
    
    if (Glob->params.debug > DEBUG_VERBOSE) {
      fprintf (stderr, " %d %f %u %u %f %f %f\n",
	       n, pos[n].z, pos[n].y, pos[n].x, pos[n].range,
	       pos[n].elev, pos[n].azim);
    }
    
    xx = maxx - (double) pos[n].x;
    yy = maxy - (double) pos[n].y;
    rng = pow ( xx, 2.) + pow ( yy, 2.);
    radius = sqrt ( rng );
    
    if (radius > min_range && radius < max_range) {

      /* Range bin matching */
      
      for ( i = 1; i < ngates; i ++) {
	
	if ( pos[n].range > range[i] && pos[n].range <= range[i+1]) {
	  
	  rd = (2./gate_spacing) * (pos[n].range - range[i]);
	  
	  if ( rd < 0.00000001 ) {
	    lookup[n].rd1 = vips_per_dB * 10. * log10(2.) + 0.5;
	    lookup[n].rd2 = vips_per_dB * -80. + 0.5;
	    lookup[n].ii = i;
	  } else if ( rd > 1.99999999 )	{
	    lookup[n].rd1 = vips_per_dB * -80. + 0.5;
	    lookup[n].rd2 = vips_per_dB * 10. * log10(2.) + 0.5;
	    lookup[n].ii = i;
	  } else {
	    lookup[n].rd1 = vips_per_dB * 10. * log10(2. - rd) + 0.5;
	    lookup[n].rd2 = vips_per_dB * 10. * log10(rd) + 0.5;
	    lookup[n].ii = i;
	  }

	}
	
      } /* i */

      /* Azimuth matching */

      for (j = 1; j < 361; j ++) {

	az = (double) j;
	az_plus_one = az + 1.0;
	
	if ( pos[n].azim == 360.) {
	  lookup[n].ad1 = vips_per_dB * 10. * log10(0.00000001) + .5;
	  lookup[n].ad2 = vips_per_dB * 10. * log10(2.) + .5;
	  lookup[n].a1 = 360;
	} else if ( pos[n].azim < 1. ) {
	  lookup[n].ad1 = vips_per_dB * 10. * log10(2.-2. * pos[n].azim) + .5;
	  lookup[n].ad2 = vips_per_dB * 10. * log10(2.*pos[n].azim) + .5;
	  lookup[n].a1 = 360;
	} else if ( pos[n].azim > az && pos[n].azim <= az_plus_one ) {

	  ad = 2. * (pos[n].azim - az);
	  
	  if ( ad < 0.00000001 ) {
	    lookup[n].ad1 = vips_per_dB * 10. * log10(2.) + 0.5;
	    lookup[n].ad2 = vips_per_dB * -80. + 0.5;
	    lookup[n].a1 = j;
	  } else if ( ad > 1.99999999 )	{
	    lookup[n].ad1 = vips_per_dB * -80. + 0.5;
	    lookup[n].ad2 = vips_per_dB * 10. * log10(2.) + 0.5;
	    lookup[n].a1 = j;
	  } else {
	    lookup[n].ad1 = vips_per_dB * 10. * log10(2. - ad) + 0.5;
	    lookup[n].ad2 = vips_per_dB * 10. * log10(ad) + 0.5;
	    lookup[n].a1 = j;
	  }
	}
      }

      /* Elevation step matching */
      
      for ( l = 1; l < nelev; l ++) {

	if ( pos[n].elev < elev[0] ) {

	  lookup[n].ed1 = vips_per_dB * -80. + .5;
	  lookup[n].ed2 = vips_per_dB * -80. + .5;
	  lookup[n].ll = 0; /* mark for not to be used */

	} else if ( pos[n].elev > elev[nelev-1] ) {

	  lookup[n].ed1 = vips_per_dB * -80. + .5;
	  lookup[n].ed2 = vips_per_dB * -80. + .5;
	  lookup[n].ll = 0; /* mark for not to be used */

	} else if ( pos[n].elev > elev[l-1] && pos[n].elev <= elev[l] ) {
	  delta_l = elev[l] - elev[l-1];
	  factor = 2. / delta_l;
	  ed = factor * (pos[n].elev - elev[l-1]);
			
	  if ( ed < 0.00000001 ) {
	    lookup[n].ed1 = vips_per_dB * 10. * log10(2.) + 0.5;
	    lookup[n].ed2 = vips_per_dB * -80. + 0.5;
	    lookup[n].ll = l;
	  } else if ( ed > 1.99999999 )	{
	    lookup[n].ed1 = vips_per_dB * -80. + 0.5;
	    lookup[n].ed2 = vips_per_dB * 10. * log10(2.) + 0.5;
	    lookup[n].ll = l;
	  } else {
	    lookup[n].ed1 = vips_per_dB * 10. * log10(2. - ed) + 0.5;
	    lookup[n].ed2 = vips_per_dB * 10. * log10(ed) + 0.5;
	    lookup[n].ll = l;
	  }
	  
	}

      } /* l */

      /* Create range dependent noise threshold */

      rc = (short) (20. * log10 (pos[n].range / 100.) * vips_per_dB);
      lookup[n].noise = rc + cal->mus;

      if (Glob->params.debug > DEBUG_VERBOSE) {
	if (radius < 10.) {
	  fprintf(stderr, "%f %f %f %f %d %d %d \n",
		  xx,yy,rng,radius,rc,cal->mus,lookup[n].noise);
	}
      }

      lookup[n].z = (short) pos[n].z;
      lookup[n].x1 = pos[n].x;
      lookup[n].y1 = pos[n].y;

      lookup[n].x2 = pos[n].y;
      lookup[n].y2 = grid->ny + 1 - pos[n].x;

      lookup[n].x3 = grid->nx + 1 - pos[n].x;
      lookup[n].y3 = grid->ny + 1 - pos[n].y;
      
      lookup[n].x4 = grid->nx + 1 - pos[n].y;
      lookup[n].y4 = pos[n].x;

      if ( lookup[n].a1 == 0 ) lookup[n].a1 = 360;

      if ( lookup[n].a1 == 360 )	{
	lookup[n].a2 = 90;
	lookup[n].a3 = 180;
	lookup[n].a4 = 270;
      } else {
	lookup[n].a2 =  lookup[n].a1 + 90;
	lookup[n].a3 =  lookup[n].a1 + 180;
	lookup[n].a4 =  lookup[n].a1 + 270;
      }

    } else { /* else for when outside range boundaries */

      lookup[n].z = (short) pos[n].z;

      lookup[n].x1 = pos[n].x;
      lookup[n].y1 = pos[n].y;

      lookup[n].x2 = pos[n].y;
      lookup[n].y2 = grid->ny + 1 - pos[n].x;

      lookup[n].x3 = grid->nx + 1 - pos[n].x;
      lookup[n].y3 = grid->ny + 1 - pos[n].y;

      lookup[n].x4 = grid->nx + 1 - pos[n].y;
      lookup[n].y4 = pos[n].x;
		
      lookup[n].ii = 0;

      lookup[n].a1 = 0;
      lookup[n].a2 = 0;
      lookup[n].a3 = 0;
      lookup[n].a4 = 0;

      lookup[n].ll = 0;

      lookup[n].rd1 = 0;
      lookup[n].rd2 = 0;
      lookup[n].ed1 = 0;
      lookup[n].ed2 = 0;
      lookup[n].ad1 = 0;
      lookup[n].ad2 = 0;
      lookup[n].noise = cal->mus; /* to make sure skip stays "clean" */

    }	/* end BIG range if */

    if (Glob->params.debug >= DEBUG_VERBOSE) {
      fprintf (stderr,
	       "%4d %4d %4d %4d %4d %4d %4d %4d %4d "
	       "%4d %4d %4d %4d %4d %6d %6d %6d %6d %6d %6d %6d\n",
	       lookup[n].x1, lookup[n].y1, lookup[n].x2, lookup[n].y2,
	       lookup[n].x3, lookup[n].y3, lookup[n].x4, lookup[n].y4,
	       lookup[n].ii, lookup[n].a1, lookup[n].a2, lookup[n].a3,
	       lookup[n].a4, lookup[n].ll, lookup[n].rd1, lookup[n].rd2,
	       lookup[n].ed1, lookup[n].ed2, lookup[n].ad1, lookup[n].ad2,
	       lookup[n].noise);
    }
    
  } /* n */ /* end n loop */

  return;

}



