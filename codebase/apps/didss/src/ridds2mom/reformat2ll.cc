// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/***************************************************************************
 * reformat2ll.c
 *
 * Reformat RIDDS radar format to LL moments format.
 *
 * Mike Dixon RAP NCAR Boulder CO USA.
 *
 * Code mostly copied from nexrad.c, in reformatter by Gary Blackburn.
 *
 * May 1997
 *
 **************************************************************************/

#include "ridds2mom.h"
#include <rapformats/ds_radar.h>
#include <rapformats/swap.h>
using namespace std;

typedef struct {
  ll_params_t hdr;
  ui08 data[LL_NFIELDS * LL_MAX_NGATES];
} ll_beam_buf_t;

/**********************
 * file scope variables
 */

static int Debug;
static char *Radar_name;
static char *Site_name;
static si32 Latitude;
static si32 Longitude;
static si32 Altitude;
static si32 Time_correction;
static si32 Polarization;
static si32 Beam_width;
static si32 Avg_xmit_pwr;
static si32 Wavelength;

/************************************************************************/

/*
 * file scope prototypes
 */

static int process_beam(RIDDS_data_hdr * data_hdr,
			ll_beam_buf_t * ll_buf,
			NEXRAD_vcp_set * vol_cntrl_patterns,
			int *nwrite);

static void init_headers(ll_beam_buf_t * ll_buf);

static void 
reset_tilt_spec_hdr(ll_beam_buf_t * ll_buf,
		    RIDDS_data_hdr * radar_beam,
		    NEXRAD_vcp_set * vol_cntrl_patterns);

static void 
reset_beam_spec_hdr(ll_beam_buf_t * ll_buf,
		    RIDDS_data_hdr * radar_beam);

static void print_data(ll_params_t * header);

/************************************************************************/

/*********************************
 * initialize file scope variables
 */

void 
init_reformat2ll(char *radar_name,
		 char *site_name,
		 double latitude,
		 double longitude,
		 double altitude,
		 int time_correction,
		 int polarization,
		 double beam_width,
		 double avg_xmit_pwr,
		 double wavelength,
		 double noise_dbz_at_100km,
		 int debug)
{

  Debug = debug;
  Radar_name = STRdup(radar_name);
  Site_name = STRdup(site_name);
  Latitude = (si32) floor(latitude * 100000.0 + 0.5);
  Longitude = (si32) floor(longitude * 100000.0 + 0.5);
  Altitude = (si32) floor(altitude * 1000.0 + 0.5);
  Time_correction = time_correction;
  Polarization = polarization;
  Beam_width = (si32) floor(beam_width * 100.0 + 0.5);
  Avg_xmit_pwr = (si32) (avg_xmit_pwr + 0.5);
  Wavelength = (si32) (wavelength * 100.0 + 0.5);


  init_moments( noise_dbz_at_100km );

}

/******************************
 * main routine - reformat2ll()
 *
 * Reformat RIDDS data into LL moments format
 */

int 
reformat2ll(ui08 *read_buffer, int nread,
	    NEXRAD_vcp_set * vcp_set,
	    ui08 **write_buffer,
	    int *nwrite)
{

  static ui08 *buffer_copy = NULL;
  static int n_copy_alloc = 0;

  static ll_beam_buf_t ll_beam;
  RIDDS_data_hdr *data_hdr;

  /*
   * make copy of buffer
   */

  if (n_copy_alloc < nread) {
    if (buffer_copy == NULL) {
      buffer_copy = (ui08 *) umalloc(nread);
    } else {
      buffer_copy = (ui08 *) urealloc(buffer_copy, nread);
    }
    n_copy_alloc = nread;
  }
  memcpy(buffer_copy, read_buffer, nread);

  /*
   * swap header
   */
  
  data_hdr = (RIDDS_data_hdr *) buffer_copy;
  BE_to_RIDDS_data_hdr(data_hdr);
  
  if (process_beam(data_hdr, &ll_beam, vcp_set, nwrite)) {

    return (-1);

  } else {

    *write_buffer = (ui08 *) &ll_beam;
    return (0);

  }

}

/************************************************************************/

static int process_beam(RIDDS_data_hdr * data_hdr,
			ll_beam_buf_t * ll_buf,
			NEXRAD_vcp_set * vol_cntrl_patterns,
			int *nwrite)
{

  static int initial_proc_call = TRUE;
  static int beam_count = 0;
  static int radial_num = 0;
  static int last_elev_num = 0;
  static int vol_num = 0;

  int iret;
  int new_vol = FALSE;
  int data_len = 0;
  int hour = 0;
  int min = 0;
  int seconds = 0;

  date_time_t ttime;

  /* Determine if this is the start of a new 360 degree tilt */

  if ((data_hdr->radial_status == START_OF_NEW_ELEVATION) ||
      (data_hdr->radial_status == BEGINNING_OF_VOL_SCAN) ||
      (data_hdr->elev_num != last_elev_num) ||
      initial_proc_call) {

    new_vol = TRUE;
    get_beam_time( data_hdr, Time_correction, &ttime );

    if (Debug > 1)
      fprintf(stderr, "\nradial status = %d, elev number = %d\n",
		    data_hdr->radial_status, data_hdr->elev_num);

    if (data_hdr->radial_status == START_OF_NEW_ELEVATION) {
      radial_num = data_hdr->radial_num;

    } else {

      if (Debug > 1)
	if (data_hdr->radial_num != radial_num + 1)
	  fprintf(stderr, "dropped beam\n");

      radial_num = data_hdr->radial_num;

    }

    /*
     * A new sweep will begin to be processed; if a previous
     * packet has been defined mark this beam as the last beam in
     * the sweep and send the packets to the network
     */

    if (!initial_proc_call) {

      end_of_tilt( last_elev_num );
      ll_buf->hdr.end_tilt_flag = 1;

      data_len = ll_buf->hdr.range_seg[0].gates_per_beam * LL_NFIELDS
	+ sizeof(ll_params_t);

      if (Debug > 1) {
	print_data(&ll_buf->hdr);
      }

      ll_buf->hdr.end_tilt_flag = 0;

    } else {

      init_headers(ll_buf);
      initial_proc_call = FALSE;

    }

    /*
     * Begin processing a new sweep first by initializing aspects
     * of the header specific to all beams common to this sweep
     */

    if (Debug > 1) {
      fprintf(stderr, "%d beams - \n", beam_count);
    }

    if (data_hdr->radial_status == BEGINNING_OF_VOL_SCAN
	&& data_hdr->elev_num == 1) {

      seconds = data_hdr->millisecs_past_midnight / 1000;
      hour = seconds / 3600;
      min = (seconds - hour * 3600) / 60;
      
      if (Debug) {
	fprintf(stderr, "\n*********************************************\n");
	fprintf(stderr, "New Volume %2d :%2d :%2d", hour, min,
		seconds - hour * 3600 - min * 60);
	fprintf(stderr, " - NEXRAD radar\n\n");
      }

      if (!initial_proc_call) {
         end_of_volume( vol_num );
      }
      vol_num++;
      start_of_volume( vol_num, ttime.unix_time );
    }

    start_of_tilt( data_hdr->elev_num, ttime.unix_time );
    reset_tilt_spec_hdr(ll_buf, data_hdr, vol_cntrl_patterns);

    if (Debug > 1) {
      fprintf(stderr, "          - fixed ang %5.2f - prf %d - vcp %d\r",
	      (float) (ll_buf->hdr.target_elev)
	      / 100., ll_buf->hdr.prf, ll_buf->hdr.vcp);
    }

    beam_count = 0;

  } else {
    
    /*
     * Send the moments formatted beam created by the previous
     * iteration of this procedure so new beam processing can
     * proceed
     */

    data_len = ll_buf->hdr.range_seg[0].gates_per_beam * LL_NFIELDS
      + sizeof(ll_params_t);

    if (Debug > 1) {
      print_data(&ll_buf->hdr);
    }

  }

  /*
   * get new beam specif info
   */
  
  reset_beam_spec_hdr(ll_buf, data_hdr);
  last_elev_num = data_hdr->elev_num;
  
  /*
   *  move the radar data into the MOM buffer
   */

  iret = store_data(data_hdr, ll_buf->data,
		    &ll_buf->hdr.azimuth, new_vol);

  beam_count++;

  if (iret || data_len <= 0) {
    return (-1);
  } else {
    *nwrite = data_len;
    return (0);
  }

}

/************************************************************************/

static void 
reset_tilt_spec_hdr(ll_beam_buf_t * ll_buf,
		    RIDDS_data_hdr * radar_beam,
		    NEXRAD_vcp_set * vol_cntrl_patterns)
     /* update a MOM buffer with the MHR tilt specific data */

{
  static short    vol_num = 0;
  short           tilt_number = 0;

  (void) strcpy(ll_buf->hdr.radar_name, Radar_name);

  (void) strcpy(ll_buf->hdr.site_name, Site_name);

  ll_buf->hdr.latitude = Latitude;	/* deg X 100000 */

  ll_buf->hdr.longitude = Longitude;	/* deg X 100000 */

  ll_buf->hdr.altitude = Altitude;

  /* modification to LL format - notion of volume control pattern */
  ll_buf->hdr.vcp = radar_beam->vol_coverage_pattern;


  /* default to surveillance (PPI) */
  ll_buf->hdr.scan_mode = 1;

  ll_buf->hdr.tilt_num = radar_beam->elev_num;

  /***********************************************
    check to see if the elev_num starts at 0 or 1*/
  if (radar_beam->elev_num == 1)
    vol_num++;

  ll_buf->hdr.vol_num = vol_num;


  tilt_number = ll_buf->hdr.tilt_num;
  ll_buf->hdr.target_elev = get_target_elev(&ll_buf->hdr.vcp,
						 &tilt_number, vol_cntrl_patterns);
  if (ll_buf->hdr.target_elev < 0)
    exit(-1);

  /*
   * ll_buf->hdr.target_elev  = (short) ((float)
   * (radar_beam->elevation) 18000. / 32768.);
   * ll_buf->hdr.target_elev  = ((short) (float)
   * ((ll_buf->hdr.target_elev + 5.) / 10.)) * 10;
   */

  /* NEXRAD only scans in one direction */
  ll_buf->hdr.scan_dir = 1;

  /* ? leaving the same as MHR */
  ll_buf->hdr.num_spacing_segs = 1;

  ll_buf->hdr.prf = (short) ((float) SPD_OF_LITE /
				  (2 * 100 * radar_beam->unamb_range_x10));

  /*
   * NEXRAD velocity and reflectivity fields have different gate
   * spacing and number of gates per beam - during the formatting
   * process everything is transformed to reflect the higher resolution
   * velocity values except low prf long range beams
   *
   * We ignore the first gate (short range scan) and first 4 gates
   * (long range scan) because these have neg ranges which RDI does
   * not like. LL start range is to start of gate.
   */

  if (ll_buf->hdr.prf < 500) {

    ll_buf->hdr.range_seg[0].gate_spacing = radar_beam->ref_gate_width;
    ll_buf->hdr.range_seg[0].gates_per_beam = radar_beam->ref_num_gates;

    ll_buf->hdr.range_to_first_gate =
      (short) (radar_beam->ref_gate1 +
	       0.5 * ll_buf->hdr.range_seg[0].gate_spacing);
    
    if (Debug) {
      fprintf(stderr, "LR: ngates, spacing, start: %d, %d, %d\n",
	      ll_buf->hdr.range_seg[0].gates_per_beam,
	      ll_buf->hdr.range_seg[0].gate_spacing,
	      radar_beam->ref_gate1);
    }

  } else {

    ll_buf->hdr.range_seg[0].gate_spacing = radar_beam->vel_gate_width;
    ll_buf->hdr.range_seg[0].gates_per_beam = radar_beam->vel_num_gates;
    ll_buf->hdr.range_to_first_gate =
      (short) (radar_beam->vel_gate1 +
	      (Glob->gate_ratio - 0.5) * ll_buf->hdr.range_seg[0].gate_spacing);

    if (Debug) {
      fprintf(stderr, "SR: ngates, spacing, start: %d, %d, %d\n",
	      ll_buf->hdr.range_seg[0].gates_per_beam,
	      ll_buf->hdr.range_seg[0].gate_spacing,
	      radar_beam->vel_gate1);
    }

  }

  /*
   * if (ll_buf->hdr.range_seg[0].gates_per_beam > MAX_GATES_RDI)
   * ll_buf->hdr.range_seg[0].gates_per_beam = MAX_GATES_RDI;
   */

  ll_buf->hdr.ref_gpb = radar_beam->ref_num_gates;
  ll_buf->hdr.vel_gpb = radar_beam->vel_num_gates;


  /*
   * NEXRAD documentation states that early radars have circular
   * polarization later ones horizontal.  Denver's is circular - code =
   * 3
   */
  ll_buf->hdr.polarization = Polarization;
  /* from NEXRAD documentation */
  ll_buf->hdr.beamwidth = Beam_width;

  /*
   * the average transmitted power - 700 watts - obtained from Den
   * NEXRAD to convert to dbM - 10 x log10(watt x 1000). xmit_pwr is
   * dbM x 256
   */
  ll_buf->hdr.xmit_pwr
    = (10 * (short) log10((double) Avg_xmit_pwr)) * 256;

  /*
   * the wavelength was obtained from the Denver NEXRAD staff based on
   * a wavelength of 10.38cm the frequency is 2890
   */
  ll_buf->hdr.frequency = (unsigned short) (3000000 / Wavelength);


  /* dependent on the VCP  - units nanosecs */
  if (radar_beam->vol_coverage_pattern == 31)
    ll_buf->hdr.pulse_width = 470;
  else
    ll_buf->hdr.pulse_width = 157;

  ll_buf->hdr.scale[0] = DBZ_SCALE;
  ll_buf->hdr.bias[0] = DBZ_BIAS;

  ll_buf->hdr.scale[1] = DBZ_SCALE;
  ll_buf->hdr.bias[1] = DBZ_BIAS;


  /*
   * handle the dynamic nature of velocity resolution (1 m/s & .5)
   */
  if (radar_beam->velocity_resolution == 4) {
    ll_buf->hdr.bias[2] = VEL_BIAS_1;
    ll_buf->hdr.scale[2] = VEL_SCALE_1;
  } else {
    ll_buf->hdr.bias[2] = VEL_BIAS_HALF;
    ll_buf->hdr.scale[2] = VEL_SCALE_HALF;
  }

  ll_buf->hdr.scale[3] = SW_SCALE;
  ll_buf->hdr.bias[3] = SW_BIAS;

  return;

}

/*---------------------------------------------------------------*/

static void 
reset_beam_spec_hdr(ll_beam_buf_t * ll_buf,
		    RIDDS_data_hdr * radar_beam)
     /* update a Lincoln Lab buffer with the MHR radial specific data */

{
  date_time_t ttime;

  ll_buf->hdr.vel_gate1 = radar_beam->vel_gate1;
  ll_buf->hdr.ref_gate1 = radar_beam->ref_gate1;

  /*
   * the current method used to integrated low prf reflectivity with
   * velocity only tilts may leave gaps in the radial numbers.  Nexrad
   * has a beam width of .99 - currently we are leaving off 5 beams per
   * tilt
   */
  ll_buf->hdr.rad_seq_num = radar_beam->radial_num;

  ll_buf->hdr.vel_spacing = radar_beam->vel_gate_width;
  ll_buf->hdr.ref_spacing = radar_beam->ref_gate_width;

  /*
   * get time
   */

  get_beam_time( radar_beam, Time_correction, &ttime );

  ll_buf->hdr.year = (ttime.year % 100);
  ll_buf->hdr.month = ttime.month;
  ll_buf->hdr.day = ttime.day;
  ll_buf->hdr.hour = ttime.hour;
  ll_buf->hdr.min = ttime.min;
  ll_buf->hdr.sec = ttime.sec;

  ll_buf->hdr.elevation = (short) (((float) radar_beam->elevation / 8.)
					* (18000. / 4096.));

  ll_buf->hdr.azimuth = (short) (((float) radar_beam->azimuth / 8.)
				      * (18000. / 4096.));

}

/*-----------------------------------------------------------*/

static void 
init_headers(ll_beam_buf_t * ll_buf)
     /* set the static sections of the Lincoln headers */

{

  char           *var1 = "SN      ";
  char           *var2 = "DZ      ";
  char           *var3 = "V       ";
  char           *var4 = "SW      ";

  ll_buf->hdr.num_spacing_segs = 1;

  (void) strncpy(ll_buf->hdr.prod_name[0], var1, 8);
  (void) strncpy(ll_buf->hdr.prod_name[1], var2, 8);
  (void) strncpy(ll_buf->hdr.prod_name[2], var3, 8);
  (void) strncpy(ll_buf->hdr.prod_name[3], var4, 8);

  ll_buf->hdr.min_behind_uct = 420;

  /* assume 1 byte vel data until data indicates differently */
  ll_buf->hdr.vel_field_size = 0;

}

/*-----------------------------------------------------------*/

static void 
print_data(ll_params_t * header)
{
  static int vol_num = 0;
  static int count = 0;


  if (header->vol_num != vol_num) {
    vol_num = header->vol_num;

    fprintf(stderr, "\n\nvolume  radar     site   latitude  longitude  "
	    "altitude  beamwidth  polarization  power trans   freq  "
	    "pulse width");
    fprintf(stderr, "\n  %d    %s   %s  %.2f N   %.2f W    %d       "
	    "%.2f         %d           %.2f      %d      %d\n",
	    header->vol_num,
	    header->radar_name, header->site_name,
	    (float) (header->latitude) / 100000.,
	    (float) (header->longitude) / 100000., header->altitude,
	    header->beamwidth / 100.,
	    header->polarization,
	    (float) (header->xmit_pwr) / 256.,
	    header->frequency, header->pulse_width);
  }

  if (count % 20 == 0) {
  
    if (count % 200 == 0) {

      fprintf(stderr, "\nvolume  radar     site   latitude  longitude  "
	      "altitude  beamwidth  polarization  power trans   freq  "
	      "pulse width");
      fprintf(stderr, "\n  %d    %s   %s  %.2f N   %.2f W    %d       "
	      "%.2f         %d           %.2f      %d      %d\n",
	      header->vol_num,
	      header->radar_name, header->site_name,
	      (float) (header->latitude) / 100000.,
	      (float) (header->longitude) / 100000., header->altitude,
	      header->beamwidth / 100.,
	      header->polarization,
	      (float) (header->xmit_pwr) / 256.,
	      header->frequency, header->pulse_width);

      fprintf(stderr, "\ntilt num  elevation  vel gpb   ref gpb  vel "
	      "spacing ref spacing   prf       date       time     "
	      "azimuth   VCP");
    }

    fprintf(stderr, "\n   %2d       %5.2f      %3d      %3d        %3d"
	    "         %3d       %d  %2d/%02d/%02d   %2d:%02d:%02d    "
	    "%6.2f    %d\n",
	    header->tilt_num, (float) header->target_elev / 100.,
	    header->vel_gpb, header->ref_gpb, header->vel_spacing,
	    header->ref_spacing, header->prf,
	    header->month, header->day, header->year, header->hour,
	    header->min, header->sec,
	    (float) (header->azimuth) / 100., header->vcp);
  }
  count++;

}
