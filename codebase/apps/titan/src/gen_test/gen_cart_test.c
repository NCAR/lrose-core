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
  gen_cart_test.c : generates test file in cartesian format
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
#define DEG_TO_RAD 0.017453293
#endif

#define NFIELDS 2
#define RING_SPACING 20.0
#define HALF_RING 2

static void parse_args(int argc,
		       char **argv,
		       char *prog_name,
		       int *debug_p,
		       int *encode_p,
		       char **lut_name_p,
		       char **out_name_p);

int
main(int argc, char **argv)

{
  
  ui08 *rdata, *cdata;

  char *prog_name;
  char *out_name;
  char *lut_name;
  
  int encode;
  int debug;

  si32 ifield, ielev, iaz, igate, ipoint, iring, i;
  si32 nplanes, iplane, npoints_per_plane;
  si32 nelevations, nazimuths, ngates;
  si32 npoints;
  si32 nrings, ring_pos;
  
  double ring_interval, ring_start_pos;
  double start_range, gate_spacing;
  
  rc_table_params_t *rcparams;
  rc_table_index_t **rc_table_index;
  rc_table_entry_t *rc_entry;
  
  rc_table_file_handle_t rc_handle;
  vol_file_handle_t v_handle;
  
  /*
   * display ucopyright message
   */
  
  prog_name = "gen_cart_test";
  ucopyright(prog_name);

  parse_args(argc, argv, prog_name,
	     &debug, &encode,
	     &lut_name, &out_name);
  
  /*
   * initialize rc_table file handle
   */
  
  RfInitRcTableHandle(&rc_handle,
		      prog_name,
		      lut_name,
		      (FILE *) NULL);
  
  /*
   * read in the radar-to-cart table
   */
  
  if (RfReadRcTable(&rc_handle,
		    "main") != R_SUCCESS)
    exit(-1);
  
  rcparams = rc_handle.table_params;
  rc_table_index = rc_handle.table_index;
  
  nelevations = rcparams->nelevations;
  nazimuths = rcparams->nazimuths;
  ngates = rcparams->ngates;
  start_range = (double) rcparams->start_range / 1000000.0;
  gate_spacing = (double) rcparams->gate_spacing / 1000000.0;
  
  /*
   * initialize vol file handle
   */
  
  RfInitVolFileHandle(&v_handle,
		      "gen_cart_test",
		      out_name,
		      (FILE *) NULL);
  
  /*
   * allocate vol params as required
   */
  
  if (RfAllocVolParams(&v_handle, "main") != R_SUCCESS)
    exit(-1);
  
  /*
   * fill in fields in header
   */
  
  memset ((void *) v_handle.vol_params->note,
          (int) 0, (size_t)  VOL_PARAMS_NOTE_LEN);
  strcpy(v_handle.vol_params->note,
	 "Test radar volume - cartesian coords");
  
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
  
  v_handle.vol_params->radar.radar_id = 5;
  v_handle.vol_params->radar.altitude = 1604;
  v_handle.vol_params->radar.latitude = 39880980;
  v_handle.vol_params->radar.longitude = -104761730;
  v_handle.vol_params->radar.nelevations = nelevations;
  v_handle.vol_params->radar.nazimuths = nazimuths;
  v_handle.vol_params->radar.ngates = ngates;
  v_handle.vol_params->radar.gate_spacing = rcparams->gate_spacing;
  v_handle.vol_params->radar.start_range = rcparams->start_range;
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
  
  memcpy ((void *)  &v_handle.vol_params->cart,
          (void *)  &rcparams->cart,
          (size_t) 
	  sizeof(cart_params_t));
  
  v_handle.vol_params->nfields = NFIELDS;
  
  /*
   * allocate the arrays for the vol file handle
   */
  
  if (RfAllocVolArrays(&v_handle, "main") != R_SUCCESS)
    exit(-1);
  
  /*
   * copy elevation array from rc_table_file
   */
  
  for (ielev = 0; ielev < rc_handle.scan_table->nelevations; ielev++)
    v_handle.radar_elevations[ielev] =
      (si32) (rc_handle.scan_table->elev_angles[ielev] * DEG_FACTOR + 0.5);

  /*
   * copy plane_heights array from rc_table_file
   */
  
  nplanes = rcparams->cart.nz;
  
  memcpy ((void *)  *v_handle.plane_heights,
          (void *)  *rc_handle.plane_heights,
          (size_t)  (nplanes * N_PLANE_HEIGHT_VALUES * sizeof(si32)));
  
  /*
   * allocate space for rdata array - one beam of data
   */
  
  rdata = (ui08 *) umalloc ((ui32) (ngates * sizeof(ui08)));
  
  /*
   * allocate memory for field data
   */
  
  npoints_per_plane =
    v_handle.vol_params->cart.nx * v_handle.vol_params->cart.ny;
  
  cdata = (ui08 *) ucalloc
    ((ui32) (npoints_per_plane * nplanes * NFIELDS),
     (ui32) sizeof(ui08));
  
  for (ifield = 0; ifield < NFIELDS; ifield++) {
    
    for (iplane = 0; iplane < nplanes; iplane++) {
      
      v_handle.field_plane[ifield][iplane] = cdata;
      
      cdata += npoints_per_plane;
      
    } /* iplane */
    
  } /* ifield */
  
  /*
   * loop through the fields
   */
  
  for (ifield = 0; ifield < NFIELDS; ifield++) {
    
    /*
     * set up parameter structure
     */
    
    v_handle.field_params[ifield]->encoded = encode;
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
     * clear rdata array
     */
    
    memset ((void *)  rdata,
            (int) 0, (size_t)  (ngates * sizeof(ui08)));
    
    /*
     * set up pointer to start of field array
     */
    
    cdata = *v_handle.field_plane[ifield];
    
    /*
     * loop through elevations
     */
    
    for (ielev = 0; ielev < nelevations; ielev++) {
      
      /*
       * compute the range gate interval for 20 km marks
       */
      
      ring_start_pos =
	(start_range /
	 (gate_spacing *
	  cos(rc_handle.scan_table->elev_angles[ielev] * DEG_TO_RAD)));
      
      ring_interval = 
	(RING_SPACING /
	 (gate_spacing *
	  cos(rc_handle.scan_table->elev_angles[ielev] * DEG_TO_RAD)));
      
      nrings = (si32) ((double) ngates / ring_interval + 1.0);
      
      /*
       * loop through azimuths
       */
      
      for (iaz = 0; iaz < nazimuths; iaz++) {
	
	memset ((void *)  rdata,
		(int) 0, (size_t)  ngates);
	
	if (iaz == 0 || iaz == 120 || iaz == 200) {
	  
	  /*
	   * if az is 0 or 120, use different color
	   */
	  
	  for (igate = 0;
	       igate <= (si32) rc_table_index[ielev][iaz].last_gate_active;
	       igate++)
	    rdata[igate] = nelevations + 11;
	  
	} else if (iaz == ((iaz / 30) * 30)) {
	  
	  /*
	   * if az is multiple of 30, use different color
	   */
	  
	  for (igate = 0;
	       igate <= (si32) rc_table_index[ielev][iaz].last_gate_active;
	       igate++)
	    rdata[igate] = nelevations + 12;
	  
	} else {
	  
	  /*
	   * load up with elevation number + 2 * field number + 1
	   */
	  
	  for (igate = 0;
	       igate <= (si32) rc_table_index[ielev][iaz].last_gate_active;
	       igate++)
	    rdata[igate] = 2 * ifield + ielev + 1;
	  
	} /* iaz */
	
	/*
	 * load up special data at each 20 km point
	 */
	
	for (iring = 0; iring < nrings; iring++) {
	  
	  ring_pos = (si32) (ring_start_pos +
			     (double) (iring + 1) * ring_interval + 0.5);
	  
	  for (i = ring_pos - HALF_RING; i <= ring_pos + HALF_RING; i++) {
	    
	    if (i < ngates) {
	      
	      if (iring == 3)
		rdata[i] = nelevations + 11;
	      else
		rdata[i] = nelevations + 9;
	      
	    }
	    
	  } /* i */
	  
	}  /* iring */
	
	/*
	 * where applicable, place rdata values into cdata array
	 */
	
	npoints = rc_table_index[ielev][iaz].npoints;
	
	if (npoints > 0) {
	  
	  rc_entry = rc_table_index[ielev][iaz].u.entry;
	  
	  for (ipoint = 0; ipoint < npoints; ipoint++)
	    cdata[rc_entry[ipoint].index] = rdata[rc_entry[ipoint].gate];
	  
	} /* if (npoints > 0) */
	
      } /* iaz */
      
    } /* ielev */
    
  } /* ifield */
  
  /*
   * write out the file
   */
  
  if (RfWriteVolume(&v_handle, "main") != R_SUCCESS)
    exit(-1);
  
  return(0);
  
}

static void parse_args(int argc,
		       char **argv,
		       char *prog_name,
		       int *debug_p,
		       int *encode_p,
		       char **lut_name_p,
		       char **out_name_p)

{

  int error_flag = 0;
  int i;

  char usage[BUFSIZ];

  /*
   * set usage string
   */

  sprintf(usage, "%s%s%s",
	  "Usage: ",
	  prog_name,
	  " [options as below] [-lut ?] [-out ?]\n"
	  "options:\n"
	  "       [ --, -h, -help, -man, -usage] produce this list.\n"
	  "       [ -debug] print debug messages\n"
	  "       [ -encode] run_length encode output\n"
	  "       [ -lut ?] lookup table path (default rc_table)\n"
	  "       [ -out ?] output file path (default cart.out)\n"
	  "\n");

  /*
   * initialize
   */

  *debug_p = FALSE;
  *encode_p = FALSE;
  *lut_name_p = "rc_table";
  *out_name_p = "cart.out";
  
  /*
   * look for command options
   */

  for (i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man") ||
	!strcmp(argv[i], "-usage")) {
      
      fprintf(stderr, "%s", usage);
      exit(0);
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      *debug_p = TRUE;
      
    } else if (!strcmp(argv[i], "-encode")) {
      
      *encode_p = TRUE;
      
    } else if (!strcmp(argv[i], "-lut")) {
	
      if (i < argc - 1) {
	*lut_name_p = argv[++i];
      } else {
	error_flag = TRUE;
      }
	
    } else if (!strcmp(argv[i], "-out")) {
	
      if (i < argc - 1) {
	*out_name_p = argv[++i];
      } else {
	error_flag = TRUE;
      }
	
    } /* if */
    
  } /* i */

  /*
   * print message if error flag set
   */

  if(error_flag) {
    fprintf(stderr, "%s", usage);
    exit(-1);
  }

  return;

}


