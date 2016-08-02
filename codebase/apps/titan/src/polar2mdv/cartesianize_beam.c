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
 * cartesianize_beam.c
 *
 * processes a beam record in shared memory, converting it into
 * cartesian coords
 *
 * Mike Dixon RAP NCAR Boulder CO November 1990
 *
 **************************************************************************/

#include "polar2mdv.h"
#include <dataport/bigend.h>
#include <toolsa/sockutil.h>
#include <toolsa/xdru.h>
#include <time.h>
#include <netinet/in.h>

#define NOISE_GATES 20

/*
 * prototypes for static functions in this file
 */

static void cartesianize_beam (si32 az_num,
			       si32 elev_num,
			       si32 time,
			       int end_of_tilt,
			       int end_of_volume,
			       int new_scan_limits,
			       ui08 *field_data);

static void cart_initialize(vol_file_handle_t *v_handle,
			    rc_table_file_handle_t *rc_handle,
			    clutter_table_file_handle_t *clutter_handle);

static void interpolate_beam(ui08 *field_data,
			     ui08 *prev_field_data,
			     ui08 *interp_field_data,
			     int nbytes_interp);

static void transmit_initialize(rdata_shmem_beam_header_t *bhdr);

static void transmit(ui08 *data,
		     si32 len,
		     si32 product_id,
		     si32 seq_no);

static void write_beam_packet(void);

/*
 * file scope variables
 */

static ui08 **Rdata, **Cdata;
static ui08 **Ray_check;
static int *Dbz_threshold_level;

static int New_volume_starts = TRUE;
static int Data_field_by_field;
static int Use_max_on_repeat;
static int Remove_clutter;

static si32 Nelev, Naz, Ngates;
static si32 Nplanes, Nfields, Nfields_in;
static si32 Ncdata, npoints_plane;
static si32 Nbeams_array, Nbeams_target, Npoints_vol;
static si32 Nscan, Nrays, Nnoise;
static si32 Noise_gate_start;
static si32 Vol_params_pkt_seq_no = 0;
static si32 Field_params_pkt_seq_no = 0;
static si32 Beam_pkt_seq_no = 0;
static si32 End_of_volume_pkt_seq_no = 0;
static si32 Nbytes_vol_params_pkt;
static si32 Nbytes_field_params_pkt;
static si32 Nbytes_end_of_volume_pkt;
static si32 Nbeams_pkt, Nbytes_pkt_hdrs, Nbytes_pkt_data;
static si32 Max_npoints;
static si32 Min_valid_run;
static si32 Dbz_field_num, Dbz_field_pos;
static si32 *Sum_noise;
static si32 *Field_positions;
static rdata_shmem_header_t *Shdr;

static rc_table_index_t **Rc_table_index;
static clutter_table_index_t **Clutter_table_index;
static vol_file_handle_t *Vol_index;
static scan_table_t *Scan_table;

static ui08 *Vol_params_pkt;
static ui08 *Field_params_pkt;
static ui08 *End_of_volume_pkt;
static ui08 *Beam_pkt;
static beam_packet_header_t *Beam_pkt_hdr;
static beam_subpacket_header_t *Beam_subhdr;
static ui08 *pkt_data, *Pkt_data_ptr;

/*
 * main routine
 */

void process_beam (rdata_shmem_beam_header_t *bhdr,
		   ui08 *field_data,
		   vol_file_handle_t *v_handle,
		   rc_table_file_handle_t *rc_handle,
		   clutter_table_file_handle_t *clutter_handle)

{
  
  si32 nbeams_missing;
  si32 elev_num, az_num;
  scan_table_t *scan_table = rc_handle->scan_table;

  static si32 first_call = TRUE;
  static si32 prev_elev_num = -1;
  static si32 prev_az_num;
  static si32 nbytes_interp;
  static double prev_azimuth;
  static ui08 *prev_field_data = NULL;
  static ui08 *interp_field_data = NULL;
  
  /*
   * if first call, setup the arrays for the lookup table, read the
   * lookup table file, check the geometry and load up the relevant
   * parts of the Vparams structure
   */
  
  PMU_auto_register("Processing beam");

  if (first_call) {
    
    /*
     * initialize function
     */
    
    cart_initialize(v_handle, rc_handle, clutter_handle);
    
    /*
     * if transmit option in effect, setup the transmit mode
     */
    
    if(Glob->transmit_cart_data)
      transmit_initialize(bhdr);

    /*
     * allocate space for prev_field_data and interp_field_data,
     *  for use in interpolation of single missing beams
     */

    nbytes_interp = Nfields_in * Ngates;
    prev_field_data = (ui08 *) umalloc ((ui32) nbytes_interp);
    interp_field_data = (ui08 *) umalloc ((ui32) nbytes_interp);
    
    /*
     * set first_call flag
     */
    
    first_call = FALSE;
    
  } /* if (first_call) */
  
  /*
   * set start date in vol params if start of new volume
   */
  
  if (New_volume_starts) {

    if (Glob->override_table_latlon) {
      /* set the radar lat and lon */
      rdata_shmem_header_t *shmem_header = get_shmem_header();
      radar_params_t *rparams = &shmem_header->radar;
      Glob->radar_lat = ((double) rparams->latitude / DEG_FACTOR);
      Glob->radar_lon = ((double) rparams->longitude / DEG_FACTOR);
      if (Glob->debug) {
	fprintf(stderr, "Setting radar posn from data stream\n");
	fprintf(stderr, ", lat,lon: %g,%g\n",
		Glob->radar_lat, Glob->radar_lon);
      }
    }
    
    Rfdtime2rtime(udate_time(bhdr->beam_time),
		  &v_handle->vol_params->start_time);
    
    New_volume_starts = FALSE;
    
  }
  
  /*
   * determine elevation index
   */
  
  elev_num = elev_index(bhdr->target_elev, Scan_table);
  
  /*
   * determine azimuth index
   */
  
  if (Scan_table->use_azimuth_table) {

    az_num = RfScanTableAng2AzNum(Scan_table->elevs + elev_num,
				  bhdr->azimuth);

  } else {

    az_num = (si32)
      floor ((bhdr->azimuth +
	      Glob->azimuth_offset - scan_table->start_azimuth) /
	     scan_table->delta_azimuth);
    
    if (az_num >= scan_table->nazimuths)
      az_num -= scan_table->nazimuths;
    else if (az_num < 0)
      az_num += scan_table->nazimuths;

  }

  if (prev_elev_num != elev_num) {
    if (Glob->timestamp_at_start_of_tilt != -1  &&
        Glob->timestamp_at_start_of_tilt == bhdr->tilt_num) {
      Glob->volume_timestamp = bhdr->beam_time;    
    }
  }

  else {

  /*
   * we're on the same elev_num as last beam
   * check for missing beams
   */
  
    nbeams_missing = az_num - prev_az_num - 1;

    if (nbeams_missing == 1) {

      /*
       * interpolate missing beam
       */

      interpolate_beam(field_data, prev_field_data,
		       interp_field_data, nbytes_interp);
      
      /*
       * cartesianize the interpolated beam
       */

      cartesianize_beam(az_num - 1L,
			elev_num,
			bhdr->beam_time,
			FALSE, FALSE, FALSE,
			interp_field_data);

    } else if (nbeams_missing > 1) {
      
      if (Glob->report_missing_beams) {
      
	fprintf(stderr, "WARNING - %s:cartesianize_beam\n",
		Glob->prog_name);
	fprintf(stderr,
		"%ld beam(s) missing, elev %g, between azs %g and %g\n",
		(long) nbeams_missing, bhdr->elevation,
		prev_azimuth, bhdr->azimuth);

      } /* if (Glob->report_missing_beams) */
      
    } /* if (nbeams_missing == 1) */ 
    
  } /* if (prev_elev_num == elev_num) */

  cartesianize_beam(az_num,
		    elev_num,
		    bhdr->beam_time,
		    bhdr->end_of_tilt,
		    bhdr->end_of_volume,
		    bhdr->new_scan_limits,
		    field_data);

  /*
   * set values for use next time through
   */

  prev_elev_num = elev_num;
  prev_az_num = az_num;
  prev_azimuth = bhdr->azimuth;
  memcpy((void *) prev_field_data, (void *) field_data,
	 (int) (Nfields_in * Ngates));
  
  return;

}

/************************************************************************
 * cartesianize_beam()
 */

static void cartesianize_beam (si32 az_num,
			       si32 elev_num,
			       si32 time,
			       int end_of_tilt,
			       int end_of_volume,
			       int new_scan_limits,
			       ui08 *field_data)

{

  register ui08 rvalue, *dbz_byte, *fd_ptr, *fdata;
  register long igate, ifield, ipoint, jpoint;
  
  ui08 *fptr;
  
  si32 nclut_points, nbytes_run;
  long index;
  si32 new_packet_length;
  si32 run_length;
  si32 npoints;

  rc_table_entry_t *rc_entry;
  clutter_table_entry_t *clutter_entry;
  
  /*
   * set end date in v_handle.vol_params
   */

  Rfdtime2rtime(udate_time(time), &Vol_index->vol_params->end_time);
  
  /*
   * set latest data time
   */

  Glob->latest_data_time = time;

  /*
   * process the ray
   */
  
  /*
   * if elevation has changed and transmit_cart_data is true,
   * send a vol params packet and a field params packet
   */
  
  if (end_of_tilt) {

    if(Glob->transmit_cart_data) {
      
      transmit(Vol_params_pkt,
	       Nbytes_vol_params_pkt,
	       (si32) VOL_PARAMS_PACKET_CODE,
	       Vol_params_pkt_seq_no);
      
      Vol_params_pkt_seq_no++;
      
      transmit(Field_params_pkt,
	       Nbytes_field_params_pkt,
	       (si32) FIELD_PARAMS_PACKET_CODE,
	       Field_params_pkt_seq_no);
      
      Field_params_pkt_seq_no++;
      
    } /* if(Glob->transmit_cart_data) */
    
  } /* if (Shdr->end_of_tilt) */
  
  if (elev_num >= 0 && elev_num < Nelev &&
      az_num >= 0 && az_num < Naz) {
    
    /*
     * if ray slot not already filled, update counter. If
     * repeated elevation data is not to be used, return
     * now
     */
    
    if (Ray_check[az_num][elev_num] == 0) {
      
      Ray_check[az_num][elev_num] = 1;
      Nrays++;
      
    } else {
      
      if (!Glob->use_repeated_elevations) {
	if (end_of_volume) {
	  handle_end_of_volume();
	}
	return;
      }
      
    }
    
    /*
     * set number of cartesian points associated with this ray
     */
    
    npoints = Rc_table_index[elev_num][az_num].npoints;

    if (npoints > 0) {
      
      /*
       * filter the data to ensure run lengths of non-noise at least
       * as si32 as Min_valid_run
       */
      
      if (Data_field_by_field) {
	
	/*
	 * data is stored on a field-by-field basis
	 */
	
	run_length = 0;
	fd_ptr = field_data;
	dbz_byte = fd_ptr + Dbz_field_pos * Ngates;
	
	for (igate = 0;
	     igate < (si32) Rc_table_index[elev_num][az_num].last_gate_active;
	     igate++) {
	  
	  if (*dbz_byte > Dbz_threshold_level[igate]) {
	    
	    /*
	     * increment counter for number of valid points in this run
	     */
	    
	    run_length++;
	    
	  } else {
	    
	    /*
	     * if valid run is too short, zero out the short run
	     */
	    
	    if (run_length > 0 && run_length < Min_valid_run) {
	      
	      fptr = fd_ptr - run_length;
	      for (ifield = 0; ifield < Nfields_in; ifield++) {
		memset ((void *)  fptr,
			(int) 0, (size_t) run_length);
		fptr += Ngates;
	      } /* ifield */
	      
	    } /* if (run_length ... */
	    
	    run_length = 0;
	    
	  } /* if (dbz_byte ... */
	  
	  fd_ptr++;
	  dbz_byte++;
	  
	} /* igate */
	
      } else {
	
	/*
	 * data is stored on a gate-by-gate basis
	 */
	
	run_length = 0;
	fd_ptr = field_data;
	dbz_byte = fd_ptr + Dbz_field_pos;
	
	for (igate = 0;
	     igate < (si32) Rc_table_index[elev_num][az_num].last_gate_active;
	     igate++) {
	  
	  if (*dbz_byte > Dbz_threshold_level[igate]) {
	    
	    /*
	     * increment counter for number of valid points in this run
	     */
	    
	    run_length++;
	    
	  } else {
	    
	    /*
	     * if valid run is too short, zero out the short run
	     */
	    
	    if (run_length > 0 && run_length < Min_valid_run) {
	      
	      nbytes_run = run_length * Nfields_in;
	      memset ((void *)  (fd_ptr - nbytes_run),
		      (int) 0, (size_t) nbytes_run);
	      
	    }
	    
	    run_length = 0;
	    
	  } /* if (dbz_byte ... */
	  
	  fd_ptr += Nfields_in;
	  dbz_byte += Nfields_in;
	  
	} /* igate */
	
      } /* if (Data_field_by_field) */
      
    } /* if (npoints > 0) */
    
    /*
     * initialize rdata array with the missing data flag
     */
    
    memset((char *) *Rdata,
	   (int) MISSING_DATA_VAL,
	   (int) (Max_npoints * Nfields));
    
    /*
     * initialize the rc_entry pointer
     */
    
    rc_entry = Rc_table_index[elev_num][az_num].u.entry;
    
    /*
     * loop through the points
     */
    
    if (Data_field_by_field) {
      
      /*
       * data is stored on a field-by-field basis
       */
      
      for (ipoint = 0; ipoint < npoints; ipoint++) {
	
	fdata = field_data + rc_entry->gate;
	
	/*
	 * check the signal-to-noise ratio exceeds threshold
	 */
	
	dbz_byte = fdata + Dbz_field_pos * Ngates;
	
	if (*dbz_byte > Dbz_threshold_level[rc_entry->gate]) {
	  
	  /*
	   * s/n ratio OK
	   */
	  
	  for (ifield = 0; ifield < Nfields; ifield++) {
	    
	    rvalue = *(fdata + Field_positions[ifield] * Ngates);
	    
	    /*
	     * Value of 0 reserved for bad data.
	     * Therefore set to 1 if the value is 0.
	     * This will avoid confusion between bad data and data which was
	     * actually 0.
	     */
	    
#ifdef NOTNOW
	    if (rvalue == 0) {
	      rvalue = 1;
	    }
#endif
	    
	    if (Use_max_on_repeat) {
	      Rdata[ifield][ipoint] = MAX(Rdata[ifield][ipoint], rvalue);
	    } else {
	      Rdata[ifield][ipoint] = rvalue;
	    } /* if (Use_max_on_repeat) */
	    
	  } /* ifield */
	  
	}  /* if (dbz_byte > ....  */
	
	rc_entry++;
	
      } /* ipoint */
      
    } else {
      
      /*
       * data is stored on a gate-by-gate basis
       */
      
      for (ipoint = 0; ipoint < npoints; ipoint++) {
	
	fdata = field_data + rc_entry->gate * Nfields_in;
	
	/*
	 * check the signal-to-noise ratio exceeds threshold
	 */
	
	dbz_byte = fdata + Dbz_field_pos;
	
	if (*dbz_byte > Dbz_threshold_level[rc_entry->gate]) {
	  
	  /*
	   * s/n ratio OK
	   */
	  
	  for (ifield = 0; ifield < Nfields; ifield++) {
	    
	    rvalue = *(fdata + Field_positions[ifield]);
	    
	    /*
	     * Value of 0 reserved for bad data.
	     * Therefore set to 1 if the value is 0.
	     * This will avoid confusion between bad data and data which was
	     * actually 0. Also divide the data value by 2 (right shift 1)
	     * so that run-length encoding will work properly - the 
	     * encoding algorithm expects data only 7 bits wide
	     */

	    if (rvalue == 0) {
               Rdata[ifield][ipoint] = 0;

               /* Rdata[ifield][ipoint] = 1; */

	    } else {

	      Rdata[ifield][ipoint] = rvalue;
	    
	      if (Use_max_on_repeat) {
		Rdata[ifield][ipoint] = MAX(Rdata[ifield][ipoint], rvalue);
	      } else {
		Rdata[ifield][ipoint] = rvalue;
	      } /* if (Use_max_on_repeat) */
	      
	    } /* if (rvalue == 0) */

	  } /* ifield */
	  
	}  /* if (dbz_byte > ....  */
	
	rc_entry++;
	
      } /* ipoint */
      
    } /* if (Data_field_by_field) */
    
    /*
     * remove clutter from rdata array if required
     */
    
    if (Remove_clutter) {
      
      /*
       * get number of clutter points, and the address of the clutter
       * entries in the clutter list
       */
      
      nclut_points = Clutter_table_index[elev_num][az_num].nclut_points;
      clutter_entry = Clutter_table_index[elev_num][az_num].u.entry;
      
      /*
       * loop through the points associated with this beam
       */
      
      for (jpoint = 0; jpoint < nclut_points; jpoint++) {
	
	ipoint = clutter_entry->ipoint;
	
	if (Rdata[Dbz_field_num][ipoint] <= clutter_entry->dbz) {
	  
	  /*
	   * if dbz val is below clutter value, set all fields to
	   * the missing data value
	   */
	  
	  for (ifield = 0; ifield < Nfields; ifield++)
	    Rdata[ifield][ipoint] = MISSING_DATA_VAL;
	  
	}
	
	clutter_entry++;
	
      } /* jpoint */
      
    } /* if (Remove_clutter) */
    
    /*
     * put data into the cartesian array
     */
    
    rc_entry = Rc_table_index[elev_num][az_num].u.entry;
    
    for (ipoint = 0; ipoint < npoints; ipoint++) {
      
      index = rc_entry->index;
      
      for (ifield = 0; ifield < Nfields; ifield++)
	Cdata[ifield][index] = Rdata[ifield][ipoint];
      
      rc_entry++;
      
    } /* ipoint */
    
    /*
     * if highest elevation scan, accumulate noise data from last few
     * gates
     */
    
    if (elev_num == Nelev - 1) {
      
      if (Data_field_by_field) {
	
	fdata = field_data + Noise_gate_start;
	
	for (igate = Noise_gate_start; igate < Ngates; igate++) {
	  
	  for (ifield = 0; ifield < Nfields; ifield++)
	    Sum_noise[ifield] +=
	      *(fdata + Field_positions[ifield] * Ngates);
	  
	  Nnoise++;
	  fdata++;
	  
	}
	
      } else {
	
	fdata = field_data + Noise_gate_start * Nfields_in;
	
	for (igate = Noise_gate_start; igate < Ngates; igate++) {
	  
	  for (ifield = 0; ifield < Nfields; ifield++)
	    Sum_noise[ifield] += *(fdata + Field_positions[ifield]);
	  
	  Nnoise++;
	  fdata += Nfields_in;
	  
	}
	
      } /* if (Data_field_by_field) */
      
    } /* if (elev_num == Nelev - 1) */
    
    /*
     * if transmit_cart_data is true, deal with packet data
     */
    
    if (Glob->transmit_cart_data) {
      
      new_packet_length = (Nbytes_pkt_hdrs +
			   sizeof(beam_subpacket_header_t) +
			   Nbytes_pkt_data +
			   Nfields * npoints);
      
      /*
       * check to see if new beam's data will fit into this packet
       */
      
      if (new_packet_length > Glob->max_packet_length)
	write_beam_packet();
      
      /*
       * load up sub header
       */
      
      Beam_subhdr = (beam_subpacket_header_t *)
	((char *) Beam_pkt + Nbytes_pkt_hdrs);
      Nbytes_pkt_hdrs += sizeof(beam_subpacket_header_t);
      
      Beam_subhdr->new_scan_limits = new_scan_limits;
      Beam_subhdr->az_num = az_num;
      Beam_subhdr->elev_num = elev_num;
      Beam_subhdr->npoints = npoints;
      Beam_subhdr->data_offset = Nbytes_pkt_data;
      
      /*
       * copy rdata arrays to beam data buffer
       */
      
      for (ifield = 0; ifield < Nfields; ifield++) {
	memcpy ((void *) Pkt_data_ptr,
		(void *) Rdata[ifield],
		(size_t) npoints);
	Pkt_data_ptr += npoints;
      }
      
      /*
       * set counters
       */
      
      Nbytes_pkt_data += Nfields * npoints;
      Nbeams_pkt++;
      
    } /* if (Glob->transmit_cart_data) */
    
  }  /*  if (elev_num >= 0 && elev_num < Nelev &&
      *  az_num >= 0 && az_num < Naz) */
  
  /*
   * if end of volume, write file
   */
  
  if (end_of_volume) {
    handle_end_of_volume();
  }
  
}

/****************************************************************************
 * cart_initialize()
 *
 * initialize data used in the routines in this file
 */

static void cart_initialize(vol_file_handle_t *v_handle,
			    rc_table_file_handle_t *rc_handle,
			    clutter_table_file_handle_t *clutter_handle)

{
  
  int dbz_field_found;
  
  long iaz, ielev, ifield, igate;
  si32 dbz_level;
  
  double dbz_scale, dbz_bias;
  double range;
  double noise_dbz;
  
  radar_params_t *rparams;
  field_params_t *fparams;
  
  /*
   * set up index pointers
   */

  Vol_index = v_handle;
  Rc_table_index = rc_handle->table_index;
  Clutter_table_index = clutter_handle->table_index;
  Scan_table = rc_handle->scan_table;
  
  /*
   * set dimensions
   */
  
  rparams = &v_handle->vol_params->radar;
  Shdr = get_shmem_header();
  
  Nfields = Glob->nfields_processed;
  Nfields_in = Shdr->nfields_in;
  Data_field_by_field = Shdr->data_field_by_field;

  Field_positions = (si32 *) umalloc ((ui32) (Nfields * sizeof(si32)));
  memcpy((void *) Field_positions,
	 (void *) Glob->field_positions,
	 (int) (Nfields * sizeof(si32)));
  
  Nelev = rparams->nelevations;
  Naz = rparams->nazimuths;
  Ngates = rparams->ngates;
  Noise_gate_start = Ngates - NOISE_GATES;
  
  Nplanes = rc_handle->table_params->cart.nz;
  npoints_plane = (rc_handle->table_params->cart.nx *
		   rc_handle->table_params->cart.ny);
  Ncdata = npoints_plane * Nplanes;
  
  Nbeams_array = Naz * Nelev;
  if (Scan_table->use_azimuth_table) {
    Nbeams_target = Scan_table->nbeams_vol;
  } else {
    Nbeams_target = Nbeams_array;
  }

  Npoints_vol = Ncdata * Nfields;

  Min_valid_run = Glob->min_valid_run;
  Use_max_on_repeat = Glob->use_max_on_repeat;
  Remove_clutter = Glob->remove_clutter;
  
  Sum_noise = (si32 *) umalloc ((ui32) (Nfields * sizeof(si32)));
  
  Ray_check = (ui08 **) ucalloc2
    ((ui32) Naz, (ui32) Nelev, (ui32) sizeof(ui08));
  
  Cdata = (ui08 **) umalloc ((ui32) (Nfields * sizeof(ui08 *)));
  
  for (ifield = 0; ifield < Nfields; ifield++)
    Cdata[ifield] = *v_handle->field_plane[ifield];
  
  /*
   * determine the max number of cartesian points associated
   * with any beam
   */
  
  Max_npoints = 0;
  
  for (ielev = 0; ielev < Nelev; ielev++) {
    for (iaz = 0; iaz < Naz; iaz++) {
      
      if ((si32) Rc_table_index[ielev][iaz].npoints > Max_npoints)
	Max_npoints = Rc_table_index[ielev][iaz].npoints;
      
    } /* iaz */
  } /* ielev */
  
  /*
   * allocate the rdata array to handle this max number of points -
   * this array is used to temporarily store the radial data
   */
  
  Rdata = (ui08 **) ucalloc2
    ((ui32) Nfields, (ui32) Max_npoints, (ui32) sizeof(ui08));
  
  /*
   * Set dbz test level according to the signal/noise
   * threshold.
   */
  
  dbz_field_found = FALSE;
  
  for (ifield = 0; ifield < Nfields; ifield++) {
    
    if (Field_positions[ifield] == Shdr->dbz_field_pos) {
      
      Dbz_field_num = ifield;
      dbz_field_found = TRUE;
      break;
      
    }
    
  } /* ifield */
  
  if (!dbz_field_found) {
    
    fprintf(stderr, "ERROR - %s:cartesianize_beam\n", Glob->prog_name);
    fprintf(stderr, "Dbz field is in position %d in the raw data.\n",
	    Shdr->dbz_field_pos);
    fprintf(stderr, "You must select this field for processing.\n");
    fprintf(stderr, "Remember, field numbers start at 0.\n");
    tidy_and_exit(-1);
    
  }
  
  fparams = v_handle->field_params[Dbz_field_num];
  
  dbz_scale = ((double) fparams->scale /
	       (double) fparams->factor);
  
  dbz_bias = ((double) fparams->bias /
	      (double) fparams->factor);
  
  Dbz_threshold_level = (int *)
    umalloc ((ui32) (Ngates * sizeof(int)));
  
  for (igate = 0; igate < Ngates; igate++) {
    
    range = (rparams->start_range +
	     (double) igate * (double) rparams->gate_spacing) / 1000000.0;
    
    noise_dbz = (Glob->sn_threshold + Shdr->noise_dbz_at_100km +
		 20.0 * (log10(range) - log10(100.0)));
    
    dbz_level = (si32) ((noise_dbz - dbz_bias) / dbz_scale + 0.5);
    
    if (dbz_level < 0)
      dbz_level = 0;
    if (dbz_level > 255)
      dbz_level = 255;

    if (Glob->check_sn) {
      Dbz_threshold_level[igate] = dbz_level;
    } else {
      Dbz_threshold_level[igate] = -1;
    }
    
  } /* igate */
  
  /*
   * set local pointers for speed reasons
   */
  
  Dbz_field_pos = Shdr->dbz_field_pos;
  
  /*
   * zero out counters and arrays
   */
  
  Nscan = 0;
  Nrays = 0;
  Nnoise = 0;
  
  memset ((void *)  Sum_noise,
          (int) 0, (size_t) (Nfields * sizeof(si32)));
  memset ((void *)  *Ray_check,
          (int) 0, (size_t) (Nbeams_array * sizeof(ui08)));
  
  /*
   * initialize data array with MISSING_DATA_VAL
   */
  
  memset((char *) *Cdata,
	 (int) MISSING_DATA_VAL,
	 (int) (Npoints_vol * sizeof(ui08)));
  
}

/****************************************************************************
 * handle_end_of_volume()
 * 
 * deal with end of scan condition
 */

void handle_end_of_volume(void)

{
  
  fprintf(stdout, "polar2mdv: end of volume\n");

  /*
   * increment scan number
   */
  
  Nscan++;
  
  /*
   * transmit beam packet if there are beams in it
   */
  
  if (Glob->transmit_cart_data) {
    
    if (Nbeams_pkt > 0)
      write_beam_packet();
    
    transmit(End_of_volume_pkt,
	     Nbytes_end_of_volume_pkt,
	     (si32) END_OF_VOLUME_PACKET_CODE,
	     End_of_volume_pkt_seq_no);
    
    End_of_volume_pkt_seq_no++;
    
  }
  
  /*
   * write to file
   */
  
  write_volume(Vol_index,
	       Nscan,
	       Nbeams_target, Nrays,
	       Nnoise, Sum_noise);
  
  /*
   * zero out counters and arrays
   */
  
  Nrays = 0;
  Nnoise = 0;
  memset ((void *)  Sum_noise,
          (int) 0, (size_t) (Nfields * sizeof(si32)));
  memset ((void *)  *Ray_check,
          (int) 0, (size_t) (Nbeams_array * sizeof(ui08)));
  
  /*
   * initialize data array with MISSING_DATA_VAL
   */
  
  memset((char *) *Cdata,
	 (int) MISSING_DATA_VAL,
	 (int) (Npoints_vol * sizeof(ui08)));

  /*
   * set New_volume_starts flag
   */

  New_volume_starts = TRUE;

}

/****************************************************************************
 * interpolate_beam()
 *
 * interpolates a single missing beam
 * 
 */

static void interpolate_beam(ui08 *field_data,
			     ui08 *prev_field_data,
			     ui08 *interp_field_data,
			     int nbytes_interp)

{

  long i;

  for (i = 0; i < nbytes_interp; i++) {

    *interp_field_data = (*field_data + *prev_field_data) / 2;

    field_data++;
    prev_field_data++;
    interp_field_data++;

  }

}


/****************************************************************************
 * transmit_initialize()
 *
 * initialize for data packet transmission
 */

static void transmit_initialize(rdata_shmem_beam_header_t *bhdr)

{
  
  long ifield;
  si32 nheights;
  si32 *pkt_elevations, *pkt_heights;
  
  vol_params_t *vol_params;
  field_params_t *fparams;
  
  /*
   * load vol params packet - the long integers are coded
   * into network byte order
   */
  
  nheights = Nplanes * N_PLANE_HEIGHT_VALUES;
  
  Nbytes_vol_params_pkt = (sizeof(vol_params_t) +
			   Nelev * sizeof(si32) +
			   nheights * sizeof(si32));
  
  if (Nbytes_vol_params_pkt > Glob->max_packet_length) {
    
    fprintf(stderr, "ERROR - %s:cartesianize_beam/n", Glob->prog_name);
    fprintf(stderr,
	    "Size of vol params packet exceeds max packet length\n");
    fprintf(stderr, "Size of vol params packet is %ld\n",
	    (long) Nbytes_vol_params_pkt);
    fprintf(stderr, "Max packet length set to %ld\n",
	    (long) Glob->max_packet_length);
    fprintf(stderr, "Reset %s*max_packet_length in params file '%s'\n",
	    Glob->prog_name, Glob->params_path_name);
    fprintf(stderr, "Check client max_packet_length parameters too.\n");
    tidy_and_exit(-1);
    
  }
  
  Vol_params_pkt =
    (ui08 *) umalloc((ui32) Nbytes_vol_params_pkt);
  
  vol_params = (vol_params_t *) Vol_params_pkt;
  
  /*
   * copy the vol params
   */
  
  memcpy ((void *) vol_params,
          (void *)  Vol_index->vol_params,
          (size_t) sizeof(vol_params_t));
  
  /*
   * set the mid time to the current time, zero out the
   * other times
   */
  
  Rfdtime2rtime(udate_time(bhdr->beam_time), &vol_params->mid_time);
  
  memset(&vol_params->file_time, 0, sizeof(radtim_t));
  memset(&vol_params->start_time, 0, sizeof(radtim_t));
  memset(&vol_params->end_time, 0, sizeof(radtim_t));

  /*
   * code the vol params into network byte order
   */
  
  BE_from_array_32((ui32 *) &vol_params->mid_time,
		   (ui32) (sizeof(radtim_t)));
  
  vol_params->radar.nbytes_char =
    N_RADAR_PARAMS_LABELS * R_LABEL_LEN;
  
  BE_from_array_32((ui32 *) &vol_params->radar,
		   (ui32) (sizeof(radar_params_t) -
			   vol_params->radar.nbytes_char));
  
  vol_params->cart.nbytes_char =
    N_CART_PARAMS_LABELS * R_LABEL_LEN;
  
  BE_from_array_32((ui32 *) &vol_params->cart,
		   (ui32) (sizeof(cart_params_t) -
			   vol_params->cart.nbytes_char));
  
  BE_from_array_32((ui32 *) &vol_params->nfields, (si32) sizeof(si32));
  
  /*
   * radar elevations
   */
  
  pkt_elevations = (si32 *) ((char *) vol_params + sizeof(vol_params_t));
  
  memcpy ((void *) pkt_elevations,
          (void *)  Vol_index->radar_elevations,
          (size_t) (Nelev * sizeof(si32)));
  
  BE_from_array_32((ui32 *) pkt_elevations,
		   (ui32) (Nelev * sizeof(si32)));
  
  /*
   * plane limit heights
   */
  
  pkt_heights = pkt_elevations + Nelev;
  
  memcpy ((void *) pkt_heights,
          (void *)  *Vol_index->plane_heights,
          (size_t) (nheights * sizeof(si32)));
  
  BE_from_array_32((ui32 *) pkt_heights,
		   (ui32) (nheights * sizeof(si32)));
  
  /*
   * load field params packet
   */
  
  Nbytes_field_params_pkt = Nfields * sizeof(field_params_t);
  
  if (Nbytes_field_params_pkt > Glob->max_packet_length) {
    
    fprintf(stderr, "ERROR - %s:cartesianize_beam/n", Glob->prog_name);
    fprintf(stderr,
	    "Size of field params packet exceeds max packet length\n");
    fprintf(stderr, "Size of field params packet is %ld\n",
	    (long) Nbytes_field_params_pkt);
    fprintf(stderr, "Max packet length set to %ld\n",
	    (long) Glob->max_packet_length);
    fprintf(stderr, "Reset %s*max_packet_length in params file '%s'\n",
	    Glob->prog_name, Glob->params_path_name);
    fprintf(stderr, "Check client max_packet_length parameters too.\n");
    tidy_and_exit(-1);
    
  }
  
  Field_params_pkt = (ui08 *)
    umalloc((ui32) (Nbytes_field_params_pkt));
  
  /*
   * load up field parameters
   */
  
  for (ifield = 0; ifield < Nfields; ifield++) {
    
    fparams = (field_params_t *) Field_params_pkt + ifield;
    
    memcpy ((void *) fparams,
            (void *)  Vol_index->field_params[ifield],
            (size_t) sizeof(field_params_t));
    
    fparams->nbytes_char = N_FIELD_PARAMS_LABELS * R_LABEL_LEN;
    fparams->encoded = FALSE;
    fparams->noise = 0;
    
    /*
     * encode into network byte order
     */
    
    BE_from_array_32((ui32 *) fparams,
		     (ui32) (sizeof(field_params_t) -
			     N_FIELD_PARAMS_LABELS * R_LABEL_LEN));
    
  } /* ifield */
  
  /*
   * load up end of scan packet
   */
  
  End_of_volume_pkt = (ui08 *) umalloc((ui32) sizeof(si32));
  Nbytes_end_of_volume_pkt = sizeof(si32);
  
  /*
   * allocate space for beam packets
   */
  
  Beam_pkt = (ui08 *) umalloc ((ui32) (Glob->max_packet_length));
  pkt_data = (ui08 *) umalloc ((ui32) (Glob->max_packet_length));
  Beam_pkt_hdr = (beam_packet_header_t *) Beam_pkt;
  
  /*
   * initialize parameters
   */
  
  Nbeams_pkt = 0;
  Nbytes_pkt_data = 0;
  Pkt_data_ptr = pkt_data;
  Nbytes_pkt_hdrs = sizeof(beam_packet_header_t);
  
}

/*********************************************************************
 * write_beam_packet()
 *
 * writes a beam packet to the connected sockets
 */

static void write_beam_packet(void)

{
  
  long ibeam;
  si32 nbytes_pkt;
  beam_subpacket_header_t *bm_subhdr;
  
  /*
   * set message header
   */
  
  nbytes_pkt = Nbytes_pkt_hdrs + Nbytes_pkt_data;
  Beam_pkt_seq_no++;
  
  /*
   * set the time as the latest time read
   */
  
  Beam_pkt_hdr->beam_time =
    Rfrtime2utime(&Vol_index->vol_params->end_time);
  
  BE_from_array_32((ui32 *) &Beam_pkt_hdr->beam_time,
		   (ui32) sizeof(si32));
  
  /*
   * set the number of beams in the packet
   */
  
  Beam_pkt_hdr->nbeams = BE_from_si32((ui32) Nbeams_pkt);
  
  /*
   * adjust the field data offsets to account for the headers
   */
  
  bm_subhdr = (beam_subpacket_header_t *)
    ((char *) Beam_pkt + sizeof(beam_packet_header_t));
  
  for (ibeam = 0; ibeam < Nbeams_pkt; ibeam++) {
    bm_subhdr->data_offset += Nbytes_pkt_hdrs;
    bm_subhdr++;
  }
  
  /*
   * encode the sub-headers
   */
  
  BE_from_array_32((ui32 *) ((char *) Beam_pkt + sizeof(beam_packet_header_t)),
		   (ui32) (Nbeams_pkt * sizeof(beam_subpacket_header_t)));
  
  /*
   * copy the beam data into the packet buffer
   */
  
  memcpy ((void *) ((char *) Beam_pkt + Nbytes_pkt_hdrs),
          (void *) pkt_data,
          (size_t) Nbytes_pkt_data);
  
  /*
   * send the packet
   */
  
  transmit(Beam_pkt,
	   nbytes_pkt,
	   (si32) CART_DATA_PACKET_CODE,
	   Beam_pkt_seq_no);

  /*
   * set latest request time
   */

  Glob->latest_request_time = time((time_t *) NULL);

  /*
   * reset parameters
   */
  
  Nbeams_pkt = 0;
  Nbytes_pkt_data = 0;
  Pkt_data_ptr = pkt_data;
  Nbytes_pkt_hdrs = sizeof(beam_packet_header_t);
  
}

/*************************************************************************
 * transmit()
 *
 * sends data to client (cart_slave)
 */

static void transmit(ui08 *data,
		     si32 len,
		     si32 product_id,
		     si32 seq_no)

{
  
  static int listening = FALSE;
  static int sock_open = FALSE;
  static int proto_fd, client_fd;
  
  /*
   * listen on socket
   */
  
  if (!listening) {
    
    if ((proto_fd = SKU_open_server(Glob->port)) < 0) {
      
      fprintf(stderr, "ERROR - %s:cartesianize_beam:transmit\n",
	      Glob->prog_name);
      tidy_and_exit(-1);
      
    }
    
    listening = TRUE;
    
    if (Glob->debug)
      printf("listening on port %d\n", Glob->port);
    
  } /* if (!listening) */
  
  /*
   * if necessary, open socket to client
   */
  
  if (!sock_open) {
    
    if ((client_fd = SKU_get_client_timed(proto_fd, 0L)) < 0) {
      if (Glob->debug)
	printf("trying to get client - not successful\n");
      return;
    } else {
      if (Glob->debug)
	printf("got client - successful\n");
      SKU_set_headers_to_new();
      sock_open = TRUE;
    }
    
  } /* if (!socket_open) */
  
  /*
   * write the data - on failure, close the socket. It will then
   * be reopened on the next attempt at a write
   */
  
  if (SKU_writeh(client_fd, (char *) data, len,
		 product_id, seq_no) < 0) {
    
    SKU_close(client_fd);
    sock_open = FALSE;
    
  }

}


