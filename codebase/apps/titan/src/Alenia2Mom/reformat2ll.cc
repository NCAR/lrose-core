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
 * Reformat ALENIA radar format to LL moments format.
 *
 * Mike Dixon RAP NCAR Boulder CO USA.
 *
 * July 1997
 *
 **************************************************************************/

#include "Alenia2Mom.h"
#include <toolsa/mem.h>
using namespace std;

/**********************
 * file scope variables
 */

static int Debug;
static char *Radar_name;
static char *Site_name;
static si32 Latitude;
static si32 Longitude;
static si32 Altitude;
static si32 Polarization;
static si32 Beam_width;
static si32 Avg_xmit_pwr;
static si32 Wavelength;
static double Noise_dbz_at_100km;

/************************************************************************/

/*
 * file scope prototypes
 */

static void load_ll_params(alenia_params_t *al_params,
			   ll_params_t *ll_params);

/*********************************
 * initialize file scope variables
 */

void 
init_reformat2ll(char *radar_name,
		 char *site_name,
		 double latitude,
		 double longitude,
		 double altitude,
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
  Polarization = polarization;
  Beam_width = (si32) floor(beam_width * 100.0 + 0.5);
  Avg_xmit_pwr = (si32) (avg_xmit_pwr + 0.5);
  Wavelength = (si32) (wavelength * 100.0 + 0.5);
  Noise_dbz_at_100km = noise_dbz_at_100km;

}

/******************************
 * main routine - reformat2ll()
 *
 * Reformat RIDDS data into LL moments format
 */

int 
reformat2ll(ui08 *read_buffer, int nread,
	    ui08 **write_buffer_p, int *nwrite_p)
{

  static ui08 *rbuffer_copy = NULL;
  static int n_copy_alloc = 0;

  static ui08 *write_buffer = NULL;
  static int n_write_alloc = 0;

  ui08 *input_data;
  ui08 *output_data;
  int nwrite;

  alenia_params_t al_params;
  alenia_header_t *al_header;
  ll_params_t *ll_params;
  
  /*
   * make copy of input buffer to prevent side-effects
   */

  if (n_copy_alloc < nread) {
    if (rbuffer_copy == NULL) {
      rbuffer_copy = (ui08 *) umalloc(nread);
    } else {
      rbuffer_copy = (ui08 *) urealloc(rbuffer_copy, nread);
    }
    n_copy_alloc = nread;
  }
  memcpy(rbuffer_copy, read_buffer, nread);

  /*
   * set input pointers
   */

  al_header = (alenia_header_t *) rbuffer_copy;
  input_data = rbuffer_copy + sizeof(alenia_header_t);

  /*
   * load up alenia params
   */
  
  load_alenia_params(al_header, &al_params);
  
  /*
   * Alloc write buffer
   */

  nwrite = sizeof(ll_params_t) + Glob->nfields_out * al_params.ngates;

  if (n_write_alloc < nwrite) {
    if (write_buffer == NULL) {
      write_buffer = (ui08 *) umalloc(nwrite);
    } else {
      write_buffer = (ui08 *) urealloc(write_buffer, nwrite);
    }
    n_write_alloc = nwrite;
  }

  /*
   * set output pointers
   */
  
  ll_params = (ll_params_t *) write_buffer;
  output_data = write_buffer + sizeof(ll_params_t);
  
  /*
   * load up lincoln params
   */

  load_ll_params(&al_params, ll_params);

  /*
   * load up data
   */

  load_output_data(&al_params, input_data, output_data);

  *nwrite_p = nwrite;
  *write_buffer_p = write_buffer;
  
  return (0);

}

static void load_ll_params(alenia_params_t *al_params,
			   ll_params_t *ll_params)

{

  int i;
  date_time_t beam_time;

  MEM_zero(*ll_params);

  beam_time.unix_time = al_params->time;
  uconvert_from_utime(&beam_time);

  ll_params->year = beam_time.year;
  ll_params->month = beam_time.month;
  ll_params->day = beam_time.day;
  ll_params->hour = beam_time.hour;
  ll_params->min = beam_time.min;
  ll_params->sec = beam_time.sec;

  STRncopy(ll_params->radar_name, Radar_name, 16);
  STRncopy(ll_params->site_name, Site_name, 16);
  
  ll_params->latitude = Latitude;	/* deg X 100000 */
  ll_params->longitude = Longitude;	/* deg X 100000 */
  ll_params->altitude = Altitude;

  if (al_params->scan_mode == GATE_DATA_SURVEILLANCE_MODE) {
    ll_params->scan_mode = LL_SURVEILLANCE_MODE;
  } else if (al_params->scan_mode == GATE_DATA_RHI_MODE) {
    ll_params->scan_mode = LL_RHI_MODE;
  } else if (al_params->scan_mode == GATE_DATA_SECTOR_MODE) {
    ll_params->scan_mode = LL_SECTOR_MODE;
  } else {
    ll_params->scan_mode = LL_SURVEILLANCE_MODE;
  }

  ll_params->tilt_num = al_params->tilt_num;
  ll_params->vol_num = al_params->vol_num;

  ll_params->azimuth = (int) (al_params->azimuth * 100.0 + 0.5);
  ll_params->elevation = (int) (al_params->elevation * 100.0 + 0.5);
  ll_params->target_elev = (int) (al_params->elev_target * 100.0 + 0.5);

  ll_params->prf = al_params->prf;

  /* only scans in one direction */
  ll_params->scan_dir = 1;

  /* constant gate spacing */

  ll_params->num_spacing_segs = 1;
  ll_params->range_seg[0].gate_spacing =
    (int) (al_params->gate_spacing * 1000.0 + 0.5);
  ll_params->range_seg[0].gates_per_beam = al_params->ngates;
  ll_params->range_to_first_gate =
    (int) (al_params->start_range * 1000.0 + 0.5);

  ll_params->ref_gpb = al_params->ngates;
  ll_params->vel_gpb = al_params->ngates;

  ll_params->polarization = Polarization;
  ll_params->beamwidth = Beam_width;

  ll_params->xmit_pwr
    = (10 * (short) log10((double) Avg_xmit_pwr)) * 256;

  ll_params->frequency = (unsigned short) (3000000 / Wavelength);

  ll_params->pulse_width = (si16) al_params->pulse_width;

  for (i = 0; i < LL_NFIELDS; i++) {
    ll_params->scale[i] = (si16) (al_params->scale[i] * 100.0 + 0.5);
    ll_params->bias[i] = (si16) (al_params->bias[i] * 100.0 + 0.5);
  }

  return;

}


