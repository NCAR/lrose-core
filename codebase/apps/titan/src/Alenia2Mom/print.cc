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
 * print.c
 *
 * prints summaries and headers of a radar beam record
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * May 1997
 *
 **************************************************************************/

#include "Alenia2Mom.h"
using namespace std;

#define HDR " Vol Tilt El_tgt El_act     Az Ngat Gspac  PRF     Date     Time"
#define FMT "%4ld %4ld %6.2f %6.2f %6.2f %4ld %5ld %4ld " \
"%.2ld/%.2ld/%.2ld %.2ld:%.2ld:%.2ld"

static void print_lincoln_summary(ui08 *buffer);
static void print_ds_summary(ui08 *buffer);
static void print_native_summary(ui08 *buffer);

static void print_lincoln_header(ui08 *buffer);
static void print_ds_header(ui08 *buffer);
static void print_native_header(ui08 *buffer);

void print_summary (ui08 *buffer)

{

  static int count = 0;

  if (count == 0) {
    if (Glob->params.output_format == LL_FORMAT) {
      print_lincoln_summary(buffer);
    } else if (Glob->params.output_format == DS_FORMAT) {
      print_ds_summary(buffer);
    } else if (Glob->params.output_format == NATIVE_FORMAT) {
      print_native_summary(buffer);
    }
  }

  count++;
  if (count == Glob->params.summary_interval) {
    count = 0;
  }

  return;

}

static void print_lincoln_summary(ui08 *buffer)

{
    
  ll_params_t *ll_params;

  ll_params = (ll_params_t *) buffer;

  fprintf(stderr, HDR);
  fprintf(stderr, "\n");

  fprintf(stderr, FMT,
	  (long) ll_params->vol_num,
	  (long) ll_params->tilt_num,
	  (double) ll_params->target_elev / 100.0,
	  (double) ll_params->elevation / 100.0,
	  (double) (ui16) ll_params->azimuth / 100.0,
	  (long) ll_params->range_seg[0].gates_per_beam,
	  (long) ll_params->range_seg[0].gate_spacing,
	  (long) ll_params->prf,
	  (long) ll_params->year,
	  (long) ll_params->month,
	  (long) ll_params->day,
	  (long) ll_params->hour,
	  (long) ll_params->min,
	  (long) ll_params->sec);
  fprintf(stderr, "\n");

}
    
static void print_ds_summary(ui08 *buffer)

{
  fprintf(stderr, "%d ds_beams read\n",
	  (int) Glob->params.summary_interval);
}

static void print_native_summary(ui08 *buffer)

{
  fprintf(stderr, "%d native_beams read\n",
	  (int) Glob->params.summary_interval);
}

void print_header (ui08 *buffer)

{
  
  static int count = 0;

  if (count == 0) {
    if (Glob->params.output_format == LL_FORMAT) {
      print_lincoln_header(buffer);
    } else if (Glob->params.output_format == DS_FORMAT) {
      print_ds_header(buffer);
    } else if (Glob->params.output_format == NATIVE_FORMAT) {
      print_native_header(buffer);
    }
  }

  count++;
  if (count == Glob->params.header_interval) {
    count = 0;
  }

  return;

}

static void print_lincoln_header(ui08 *buffer)

{
    
  int i;
  ll_params_t *ll_params;

  ll_params = (ll_params_t *) buffer;

  fprintf(stderr, "********************************************\n");
  fprintf(stderr, "LINCOLN HEADER\n");
  fprintf(stderr, "date                    %.2d/%.2d/%.2d\n",
	  ll_params->year, ll_params->month, ll_params->day);
  fprintf(stderr, "time                    %.2d:%.2d:%.2d\n",
	  ll_params->hour,ll_params->min, ll_params->sec);
  
  fprintf(stderr, "azimuth (deg)           %g\n",
	  (double) (ui16) ll_params->azimuth / 100.0);
  fprintf(stderr, "elevation (deg)         %g\n",
	  (double) ll_params->elevation / 100.0);
  
  fprintf(stderr, "vol_num                 %ld\n",
	  (long) ll_params->vol_num);
  fprintf(stderr, "tilt_num                %ld\n",
	  (long) ll_params->tilt_num);
  fprintf(stderr, "scan_mode               %ld\n",
	  (long) ll_params->scan_mode);
  fprintf(stderr, "target_elev (deg)       %g\n",
	  ll_params->target_elev / 100.0);
  fprintf(stderr, "scan_dir                %ld\n",
	  (long) ll_params->scan_dir);
  fprintf(stderr, "range_to_first_gate(m)  %ld\n",
	  (long) ll_params->range_to_first_gate);
  fprintf(stderr, "gates_per_beam          %ld\n",
	  (long) ll_params->range_seg[0].gates_per_beam);
  fprintf(stderr, "gate_spacing(m)         %ld\n",
	  (long) ll_params->range_seg[0].gate_spacing);
  fprintf(stderr, "prf                     %ld\n",
	  (long) ll_params->prf);
  fprintf(stderr, "radar, site names       %s, %s\n",
	  ll_params->radar_name, ll_params->site_name);
  fprintf(stderr, "latitude (deg)          %g\n",
	  ll_params->latitude / 100000.0);
  fprintf(stderr, "longitude (deg)         %g\n",
	  ll_params->longitude / 100000.0);
  fprintf(stderr, "altitude (m)            %ld\n",
	  (long) ll_params->altitude);
  fprintf(stderr, "beamwidth (deg)         %g\n",
	  ll_params->beamwidth / 100.0);
  fprintf(stderr, "polarization            %ld\n",
	  (long) ll_params->polarization);
  fprintf(stderr, "power_trans             %ld\n",
	  (long) ll_params->xmit_pwr);
  fprintf(stderr, "frequency (Mhz)         %ld\n",
	  (long) ll_params->frequency);
  fprintf(stderr, "pulse width (ns)        %ld\n",
	  (long) ll_params->pulse_width);
  
  for (i = 0; i < LL_NFIELDS; i++) {
    
    fprintf(stderr, "parameter %d scale       %g\n", i + 1,
	    (double) ll_params->scale[i] /
	    (double) LL_SCALE_AND_BIAS_MULT);
    fprintf(stderr, "parameter %d bias        %g\n", i + 1,
	    (double) ll_params->bias[i] /
	    (double) LL_SCALE_AND_BIAS_MULT);
    
  } /* i */
  
}
    
static void print_ds_header(ui08 *buffer)

{
}
    
static void print_native_header(ui08 *buffer)

{
}
    
