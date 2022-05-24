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
  gen_radar_test.c : generates test file in raw radar format
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>
#include <string.h>

#include <toolsa/umisc.h>
#include <titan/file_io.h>
#include <titan/radar.h>

#ifndef DEG_TO_RAD
#define DEG_TO_RAD 0.01745329251994372
#endif

#define PSEUDO_RADIUS 8533.0

#define NELEVATIONS 11
#define NAZIMUTHS 360
#define NGATES 660
#define NFIELDS 2
#define GATE_SPACING 225.0
#define START_RANGE 112.5
#define RING_SPACING 20000.0
#define HALF_RING 2

int main(int argc, char **argv)
{

  int filearg;
  si32 iaz, iring, i;
  si32 ifield, ielev, igate, iplane;
  si32 offset;
  si32 nrings, ring_pos;
  si32 encoded = TRUE;
  si32 n_per_plane, n_per_field, ndata;

  double ring_interval, ring_start_pos;
  double f_elevations[NELEVATIONS];
  double coselev[NELEVATIONS];
  double sinelev[NELEVATIONS];
  double twice_radius = 2.0 * PSEUDO_RADIUS;
  double slant_range;
  double beam_ht;
  
  vol_file_handle_t v_handle;
  radar_params_t *radar;


  ui08 *rdata;

  /*
   * display ucopyright message
   */

  ucopyright("gen_radar_test");

  /*
   * check no of args
   */

  if (argc == 2)
    filearg = 2;
  else if (argc == 4 && !strcmp(argv[1], "-encode"))
    filearg = 4;
  else {
    fprintf(stderr, "Usage: %s [-encode yes/no] file-name\n", argv[0]);
    exit(1);
  }

  if (argc == 4) {
    
    if (!strcmp(argv[2], "yes")) {
      encoded = TRUE;
    } else if (!strcmp(argv[2], "no")) {
      encoded = FALSE;
    } else {
      fprintf(stderr, "ERROR - gen_radar_test.\n");
      fprintf(stderr, "Invalid option '%s %s'\n", argv[1], argv[2]);
      exit(1);
    }

  }

  /*
   * initialize vol file handle
   */

  RfInitVolFileHandle(&v_handle,
		      "gen_radar_test",
		      argv[filearg - 1],
		      (FILE *) NULL);

  /*
   * allocate vol params as required
   */
  
  if (RfAllocVolParams(&v_handle, "main") != R_SUCCESS)
    exit(1);
  
  /*
   * fill in fields in header
   */

  memset ((void *) v_handle.vol_params->note,
          (int) 0, (size_t)  VOL_PARAMS_NOTE_LEN);
  strcpy(v_handle.vol_params->note,
	 "Test radar volume - radial coords");

  v_handle.vol_params->start_time.year = 1999;
  v_handle.vol_params->start_time.month = 1;
  v_handle.vol_params->start_time.day = 1;
  v_handle.vol_params->start_time.hour = 0;
  v_handle.vol_params->start_time.min = 0;
  v_handle.vol_params->start_time.sec = 0;

  v_handle.vol_params->mid_time.year = 1999;
  v_handle.vol_params->mid_time.month = 1;
  v_handle.vol_params->mid_time.day = 1;
  v_handle.vol_params->mid_time.hour = 1;
  v_handle.vol_params->mid_time.min = 1;
  v_handle.vol_params->mid_time.sec = 1;

  v_handle.vol_params->end_time.year = 1999;
  v_handle.vol_params->end_time.month = 1;
  v_handle.vol_params->end_time.day = 1;
  v_handle.vol_params->end_time.hour = 2;
  v_handle.vol_params->end_time.min = 2;
  v_handle.vol_params->end_time.sec = 2;

  radar = &v_handle.vol_params->radar;
  v_handle.vol_params->radar.radar_id = 5;
  v_handle.vol_params->radar.altitude = 1604;
  v_handle.vol_params->radar.latitude = 39880980;
  v_handle.vol_params->radar.longitude = -104761730;
  v_handle.vol_params->radar.nelevations = NELEVATIONS;
  v_handle.vol_params->radar.nazimuths = NAZIMUTHS;
  v_handle.vol_params->radar.ngates = NGATES;
  v_handle.vol_params->radar.gate_spacing = (si32) (GATE_SPACING * 1000.0);
  v_handle.vol_params->radar.start_range = (si32) (START_RANGE * 1000.0);
  v_handle.vol_params->radar.delta_azimuth = 1000000;
  v_handle.vol_params->radar.start_azimuth = 0;
  v_handle.vol_params->radar.beam_width = 950000;
  v_handle.vol_params->radar.samples_per_beam = 45;
  v_handle.vol_params->radar.pulse_width = 1270;
  v_handle.vol_params->radar.prf = 994000;
  v_handle.vol_params->radar.wavelength = 104300;
  v_handle.vol_params->radar.nmissing = 0;
  memset ((void *) v_handle.vol_params->radar.name,
          (int) 0, (size_t)  R_LABEL_LEN);
  strcpy(v_handle.vol_params->radar.name, "Test radar");

  v_handle.vol_params->cart.latitude = 39880980;
  v_handle.vol_params->cart.longitude = -104761730;
  v_handle.vol_params->cart.rotation = 0;

  v_handle.vol_params->cart.nx = NGATES;
  v_handle.vol_params->cart.ny = NAZIMUTHS;
  v_handle.vol_params->cart.nz = NELEVATIONS;

  v_handle.vol_params->cart.minx = 0;
  v_handle.vol_params->cart.miny = 0;
  v_handle.vol_params->cart.minz = 0;

  v_handle.vol_params->cart.dx = 1000;
  v_handle.vol_params->cart.dy = 1000;
  v_handle.vol_params->cart.dz = 1000;

  v_handle.vol_params->cart.radarx = 0;
  v_handle.vol_params->cart.radary = 0;
  v_handle.vol_params->cart.radarz = 0;

  v_handle.vol_params->cart.scalex = 1000;
  v_handle.vol_params->cart.scaley = 1000;
  v_handle.vol_params->cart.scalez = 1000;

  v_handle.vol_params->cart.km_scalex = 1000;
  v_handle.vol_params->cart.km_scaley = 1000;
  v_handle.vol_params->cart.km_scalez = 1000;

  v_handle.vol_params->cart.dz_constant = FALSE;

  memset ((void *) v_handle.vol_params->cart.unitsx,
          (int) 0, (size_t)  R_LABEL_LEN);
  memset ((void *) v_handle.vol_params->cart.unitsy,
          (int) 0, (size_t)  R_LABEL_LEN);
  memset ((void *) v_handle.vol_params->cart.unitsz,
          (int) 0, (size_t)  R_LABEL_LEN);

  strcpy(v_handle.vol_params->cart.unitsx, "Gate number");
  strcpy(v_handle.vol_params->cart.unitsy, "Deg");
  strcpy(v_handle.vol_params->cart.unitsz, "Deg");

  v_handle.vol_params->nfields = NFIELDS;

  /*
   * allocate the arrays for the vol file handle
   */

  if (RfAllocVolArrays(&v_handle, "main") != R_SUCCESS)
    exit(1);

  /*
   * fill in elevations
   */

  f_elevations[0] = 0.5;
  f_elevations[1] = 1.2;
  f_elevations[2] = 2.5;
  f_elevations[3] = 4.0;
  f_elevations[4] = 5.5;
  f_elevations[5] = 7.0;
  f_elevations[6] = 8.5;
  f_elevations[7] = 10.0;
  f_elevations[8] = 13.0;
  f_elevations[9] = 17.0;
  f_elevations[10] = 22.0;

  for (ielev = 0; ielev < NELEVATIONS; ielev++) {
    v_handle.radar_elevations[ielev] =
      (si32) (f_elevations[ielev] * 1000000.0 + 0.5);
    sinelev[ielev] = sin(f_elevations[ielev] * DEG_TO_RAD);
    coselev[ielev] = cos(f_elevations[ielev] * DEG_TO_RAD);
  }
  
  /*
   * set plane heights, which in this (non-cartesian) grid will
   * represent the elevation angles and their limits
   */

  for (iplane = 0; iplane < NELEVATIONS; iplane++)
    v_handle.plane_heights[iplane][PLANE_MIDDLE_INDEX] =
      f_elevations[iplane];

  for (iplane = 1; iplane < NELEVATIONS; iplane++) {
    
    v_handle.plane_heights[iplane][PLANE_BASE_INDEX] =
      (f_elevations[iplane - 1] +
       f_elevations[iplane]) / 2.0;

    v_handle.plane_heights[iplane - 1][PLANE_TOP_INDEX] =
      v_handle.plane_heights[iplane][PLANE_BASE_INDEX];
    
  } /* iplane */

  v_handle.plane_heights[0][PLANE_BASE_INDEX] =
    2.0 * v_handle.plane_heights[0][PLANE_MIDDLE_INDEX] -
      v_handle.plane_heights[0][PLANE_TOP_INDEX];
  
  v_handle.plane_heights[NELEVATIONS - 1][PLANE_TOP_INDEX] =
    2.0 * v_handle.plane_heights[NELEVATIONS - 1][PLANE_MIDDLE_INDEX] -
      v_handle.plane_heights[NELEVATIONS - 1][PLANE_BASE_INDEX];

  /*
   * allocate space for rdata array
   */

  n_per_plane = NAZIMUTHS * NGATES;
  n_per_field = n_per_plane * NELEVATIONS;
  ndata = n_per_field * NFIELDS;

  rdata = (ui08 *) umalloc ((ui32) ndata);

  memset ((void *)  rdata,
          (int) 0, (size_t)  ndata);

  for (ifield = 0; ifield < NFIELDS; ifield++)
    for (iplane = 0; iplane < NELEVATIONS; iplane++)
      v_handle.field_plane[ifield][iplane] =
	rdata + (ifield * n_per_field) + iplane * n_per_plane;

  /*
   * loop through the fields
   */

  for (ifield = 0; ifield < NFIELDS; ifield++) {

    /*
     * set up parameter structure
     */

    v_handle.field_params[ifield]->encoded = encoded;
    v_handle.field_params[ifield]->factor = 1;
    v_handle.field_params[ifield]->scale = 1;
    v_handle.field_params[ifield]->bias = 0;
    v_handle.field_params[ifield]->missing_val = 0;
    v_handle.field_params[ifield]->noise = 0;

    memset ((void *) v_handle.field_params[ifield]->transform,
            (int) 0, (size_t)  R_LABEL_LEN);
    sprintf(v_handle.field_params[ifield]->transform, "none");

    memset ((void *) v_handle.field_params[ifield]->name,
            (int) 0, (size_t)  R_LABEL_LEN);
    sprintf(v_handle.field_params[ifield]->name, "Test field %d", ifield);

    memset ((void *) v_handle.field_params[ifield]->units,
            (int) 0, (size_t)  R_LABEL_LEN);
    sprintf(v_handle.field_params[ifield]->units, "units");

    /*
     * loop through elevations
     */
    
    for (ielev = 0; ielev < NELEVATIONS; ielev++) {

      /*
       * compute the range gate interval for 20 km marks
       */
      
      ring_start_pos =
	(START_RANGE /
	 (GATE_SPACING * cos(f_elevations[ielev] * DEG_TO_RAD)));

      ring_interval = 
	(RING_SPACING /
	 (GATE_SPACING * cos(f_elevations[ielev] * DEG_TO_RAD)));

      nrings = (si32) ((double) NGATES / ring_interval + 1.0);

      /*
       * loop through azimuths
       */
      
      for (iaz = 0; iaz < NAZIMUTHS; iaz++) {

	offset = iaz * NGATES;
	
	if (iaz == 0 || iaz == 120) {

	  /*
	   * if az is 0 or 120, use different color
	   */
	  
	  for (igate = 0; igate < NGATES; igate++)
	    v_handle.field_plane[ifield][ielev][offset + igate] =
	      NELEVATIONS + 2;
	  
	} else if (iaz == ((iaz / 30) * 30)) {
	  
	  /*
	   * if az is multiple of 30, use different color
	   */
	  
	  for (igate = 0; igate < NGATES; igate++)
	    v_handle.field_plane[ifield][ielev][offset + igate] =
	      NELEVATIONS + 1;
	  
	} else {

	  if (ifield == 0) {

	    /*
	     * field 0 - load up with elevation number + 1
	     */
	  
	    for (igate = 0; igate < NGATES; igate++) {
	      v_handle.field_plane[ifield][ielev][offset + igate] =
		ielev + 1;
	    }

	  } else {

	    /*
	     * field 1 - load up with height
	     */
	    
	    slant_range = START_RANGE / 1000.0;
	    
	    for (igate = 0; igate < NGATES; igate++) {
	      
	      beam_ht =
		((radar->altitude / 1000.0) +
		 slant_range * sinelev[ielev] +
		 slant_range * slant_range / twice_radius);
	      
	      v_handle.field_plane[ifield][ielev][offset + igate] =
		(si32) (beam_ht + 0.5);
	      
	      slant_range += GATE_SPACING / 1000.0;
	      
	    } /* igate */

	  } /* if (ifield == 0) */

	} /* if (iaz ... */

	/*
	 * load up special data at each 20 km point
	 */
	
	for (iring = 0; iring < nrings; iring++) {
	  
	  ring_pos = (si32) (ring_start_pos +
			     (double) (iring + 1) * ring_interval + 0.5);
	  
	  for (i = ring_pos - HALF_RING; i <= ring_pos + HALF_RING; i++) {
	    
	    if (i < NGATES)
	      v_handle.field_plane[ifield][ielev][offset + i] =
		NELEVATIONS + 3;
	    
	  } /* i */
	  
	}  /* iring */
	
      } /* iaz */

    } /* ielev */

  } /* ifield */

  /*
   * write out the file
   */

  if (RfWriteVolume(&v_handle, "main") != R_SUCCESS)
    exit(1);

  return(0);

}
