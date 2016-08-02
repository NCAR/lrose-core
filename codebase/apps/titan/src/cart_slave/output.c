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
 * output.c
 *
 * Handles setup and writing of output file.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 1997
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "cart_slave.h"

/*
 * file scope
 */

static int Nfields;
static int Nplanes;
static int Nbytes_per_plane;
static ui08 **Fields;
static char tmp_output_path[MAX_PATH_LEN];

static vol_file_handle_t V_Handle; /* output vol file handle */

static slave_table_file_handle_t slave_handle;   /* radar to cart slave
						* table file handle */

/**************************
 * init volume file handle
 */

void init_output(void)

{

  sprintf(tmp_output_path, "%s.%s",
	  Glob->params.output_file_path, "tmp");

  RfInitVolFileHandle(&V_Handle,
		      Glob->prog_name,
		      tmp_output_path,
		      (FILE *) NULL);
  
  if (RfAllocVolParams(&V_Handle,
		       "init_output"))
    tidy_and_exit(-1);
  
}

/********************
 * read_slave_table()
 */

void read_slave_table(void)

{

  RfInitSlaveTableHandle(&slave_handle,
			 Glob->prog_name,
			 Glob->params.slave_table_path,
			 (FILE *) NULL);
  
  if (RfReadSlaveTable(&slave_handle,
		       "read_in_slave_table") != R_SUCCESS) {
    tidy_and_exit(-1);
  }
  
}

/**********************************
 * set up the output volume params
 */

void setup_volume(vol_params_t *vol_params,
		  si32 *radar_elevations,
		  si32 *plane_heights)

{
  
  long ifield, iplane;
  si32 nheights;

  /*
   * vol params
   */

  *V_Handle.vol_params = *vol_params;
  
  /*
   * alloc index arrays
   */
  
  if (RfAllocVolArrays(&V_Handle, "setup_volume"))
    tidy_and_exit(-1);

  /*
   * copy over radar elevation angles
   */
  
  umalloc_verify();

  memcpy (V_Handle.radar_elevations, radar_elevations,
          vol_params->radar.nelevations * sizeof(si32));
  
  umalloc_verify();
  /*
   * copy over plane height limits
   */

  Nplanes = vol_params->cart.nz;
  nheights = Nplanes * N_PLANE_HEIGHT_VALUES;

  memcpy (*V_Handle.plane_heights, plane_heights,
          nheights * sizeof(si32));
  umalloc_verify();
  
  /*
   * alloc planes
   */
  
  Nbytes_per_plane =
    vol_params->cart.nx * vol_params->cart.ny;

  umalloc_verify();

  Nfields = vol_params->nfields;

  Fields = (ui08 **) umalloc(Nfields * sizeof(ui08*));

  for (ifield = 0; ifield < Nfields; ifield++) {

    Fields[ifield] = (ui08 *) umalloc(Nbytes_per_plane * Nplanes);
    
    for (iplane = 0; iplane < Nplanes; iplane++) {
      V_Handle.field_plane[ifield][iplane] =
	Fields[ifield] + iplane * Nbytes_per_plane;
    }

  } /* ifield */
  
}

/*************************
 * Set up the field params
 */

void setup_field_params(field_params_t *field_param_array)

{

  int nbytes_char;
  int ifield;

  field_params_t *fparams = field_param_array;
  
  for (ifield = 0; ifield < V_Handle.vol_params->nfields;
       ifield++, fparams++) {
    
    /*
     * decode from network byte order
     */
    
    nbytes_char = BE_to_si32(fparams->nbytes_char);
    
    BE_to_array_32((ui32 *) fparams,
		   sizeof(field_params_t) - nbytes_char);

    /*
     * store
     */
    
    *V_Handle.field_params[ifield] = *fparams;

  } /* ifield */

}

/***************************
 * load_beam
 *
 * load beam into Cart space
 */

void load_beam(ui08 *packet)

{

  ui08 *slave_data;

  long ibeam, ielev, iaz, ifield, ipoint, iplane;
  si32 npoints, nbeams;
  si32 *cart_index;

  date_time_t btime;

  slave_table_index_t **s_t_index;

  beam_packet_header_t *bhdr;
  beam_subpacket_header_t *bsubhdr;

  s_t_index = slave_handle.table_index;

  bhdr = (beam_packet_header_t *) packet;
  
  /*
   * decode the header from network byte order
   */

  BE_to_array_32((ui32 *) bhdr,
		 sizeof(beam_packet_header_t));
	
  nbeams = bhdr->nbeams;
	
  /*
   * set the time
   */
  
  btime.unix_time = bhdr->beam_time;
  uconvert_from_utime(&btime);
  Rfdtime2rtime(&btime, &V_Handle.vol_params->start_time);
  Rfdtime2rtime(&btime, &V_Handle.vol_params->mid_time);
  Rfdtime2rtime(&btime, &V_Handle.vol_params->end_time);
	
  /*
   * decode the subpacket headers
   */

  bsubhdr = (beam_subpacket_header_t *)
    ((char *) bhdr + sizeof(beam_packet_header_t));

  BE_to_array_32((ui32 *) bsubhdr,
		 (ui32) (nbeams * sizeof(beam_subpacket_header_t)));
	
  /*
   * loop through beams in packet
   */
  
  for (ibeam = 0; ibeam < nbeams; ibeam++) {

    /*
     * if new azimuth limits are in effect, clear the
     * data area
     */
    
    if (bsubhdr->new_scan_limits) {
      for (ifield = 0; ifield < Nfields; ifield++) {
	for (iplane = 0; iplane < Nplanes; iplane++) {
	  memset (V_Handle.field_plane[ifield][iplane],
		  0, Nbytes_per_plane);
	}
      }
    }

    npoints = bsubhdr->npoints;
    ielev =  bsubhdr->elev_num;
    iaz = bsubhdr->az_num;

    /*
     * check that the number of points sent is what is expected
     */

    if (s_t_index[ielev][iaz].npoints != npoints) {
	    
      fprintf(stderr, "ERROR - %s:read_beams\n", Glob->prog_name);
      fprintf(stderr,
	      "Incoming packets do not conform to slave table.\n");
      fprintf(stderr, "Table file is '%s'\n",
	      Glob->params.slave_table_path);
      tidy_and_exit(-1);
      
    }
    
    /*
     * stuff the data into the cartesian grid
     */
    
    slave_data = packet + bsubhdr->data_offset;
    
    for (ifield = 0; ifield < Nfields; ifield++) {

      cart_index = s_t_index[ielev][iaz].u.index;
 
      for (ipoint = 0; ipoint < npoints; ipoint++) {

	Fields[ifield][*cart_index] = *slave_data;
	
	slave_data++;
	cart_index++;

      } /* ipoint */

    } /* ifield */

    bsubhdr++;

  } /* ibeam */

}

/*************************************
 * write the file
 *
 * Returns 0 on success, -1 on failure
 */

int write_output(void)

{

  char call_str[BUFSIZ];

  if (Glob->params.debug) {
    fprintf(stderr, "Writing out file %s\n", V_Handle.vol_file_path);
  }

  if (RfWriteVolume(&V_Handle, "write_output") != R_SUCCESS) {
    return(-1);
  }

  /*
   * change the name
   */

  /*
   * move to correct location
   */
  
  sprintf(call_str, "/bin/mv -f %s %s",
	  tmp_output_path, Glob->params.output_file_path);
  
  if (Glob->params.debug) {
    fprintf(stderr, "%s\n", call_str);
  }
  usystem_call(call_str);
  
  return (0);

}
