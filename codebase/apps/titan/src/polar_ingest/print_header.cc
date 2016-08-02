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
 * print_header.c
 *
 * prints details of a radar beam header to stdout
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * December 1991
 *
 **************************************************************************/

#include "polar_ingest.h"

static void print_lincoln_header(void);
static void print_rp7_header(void);
static void print_bprp_header(void);
static void print_gate_data_header(void);

void print_header (void)

{
  

  switch (Glob->header_type) {
    
  case LINCOLN_HEADER:
    print_lincoln_header();
    break;
    
  case RP7_HEADER:
    print_rp7_header();
    break;
    
  case BPRP_HEADER:
    print_bprp_header();
    break;
    
  case GATE_DATA_HEADER:
    print_gate_data_header();
    break;
    
  } /* switch */

  fflush(stdout);
  
}

static void print_lincoln_header(void)

{
    
  ui08 *ll_data;
  int i;
  ll_params_t *ll_params;

  get_lincoln_ptrs(&ll_params, &ll_data);
  
  fprintf(stderr, "********************************************\n");
  fprintf(stderr, "LINCOLN HEADER\n");
  fprintf(stderr, "date                    %.4d/%.2d/%.2d\n",
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
    
static void print_rp7_header(void)

{
    
  int i;
  rp7_params_t *rp7_params;
  ui08 *rp7_data;
  char radar_name[3];
  double cf = 360.0 / 65536.0;

  get_rp7_ptrs(&rp7_params, &rp7_data);

  strncpy(radar_name, (char *) &(rp7_params->rs_id), 2);
  radar_name[2] = '\0';
  
  fprintf(stderr, "********************************************\n");
  fprintf(stderr, "RP7 HEADER\n");
  fprintf(stderr, "logical seq num         %ld\n",
	  (long) rp7_params->rec_num);
  fprintf(stderr, "field tape sequence num %ld\n",
	  (long) rp7_params->field_tape_seq);
  fprintf(stderr, "record type             %ld\n",
	  (long) rp7_params->rec_type);
  fprintf(stderr, "date                    %.4d/%.2d/%.2d\n",
	  rp7_params->year, rp7_params->month, rp7_params->day);
  fprintf(stderr, "time                    %.2d:%.2d:%.2d\n",
	  rp7_params->hour,rp7_params->min, rp7_params->sec);
  
  fprintf(stderr, "azimuth (deg)           %g\n",
	  (double) rp7_params->azimuth * cf);
  fprintf(stderr, "elevation (deg)         %g\n",
	  (double) rp7_params->elevation * cf);
  
  fprintf(stderr, "range to first gate     %ld\n",
	  (long) rp7_params->rhozero1);
  fprintf(stderr, "gate spacing (m)        %ld\n",
	  (long) rp7_params->gate_spacing);
  fprintf(stderr, "number of gates         %ld\n",
	  (long) rp7_params->gates_per_beam);
  fprintf(stderr, "samples per beam        %ld\n",
	  (long) rp7_params->samples_per_beam);
  
  fprintf(stderr, "avg_xmit_pwr            %ld\n",
	  (long) rp7_params->avg_xmit_pwr);
  fprintf(stderr, "pulse width (nanosec)   %ld\n",
	  (long) rp7_params->pulse_width);
  fprintf(stderr, "prf                     %g\n",
	  (double) rp7_params->prfx10 / 10.0);
  fprintf(stderr, "wavelength (cm)         %g\n",
	  (double) rp7_params->wavelength / 100.0);
  fprintf(stderr, "tilt seq                %ld\n",
	  (long) rp7_params->tilt_seq);
  fprintf(stderr, "scan mode               %ld\n",
	  (long) rp7_params->scan_mode);
  
  fprintf(stderr, "cw_az_lim   (deg)       %g\n",
	  (double) rp7_params->cw_az_lim * cf);
  fprintf(stderr, "ccw_az_lim   (deg)      %g\n",
	  (double) rp7_params->ccw_az_lim * cf);
  
  fprintf(stderr, "target elev (deg)       %g\n",
	  (double) rp7_params->target_elev * cf);
  
  fprintf(stderr, "beamwidth (deg)         %g\n",
	  (double) (rp7_params->beamwidth) / 100.);
  fprintf(stderr, "scan_rate               %ld\n",
	  (long) rp7_params->scan_rate);
  fprintf(stderr, "vol num                 %ld\n",
	  (long) rp7_params->vol_num);
  fprintf(stderr, "polarization            %ld\n",
	  (long) rp7_params->polarization);
  fprintf(stderr, "prf1                    %g\n",
	  (double) rp7_params->prf1 / 10.);
  fprintf(stderr, "prf2                    %g\n",
	  (double) rp7_params->prf2 / 10.);
  fprintf(stderr, "prf3                    %g\n",
	  (double) rp7_params->prf3 / 10.);
  fprintf(stderr, "prf4                    %g\n",
	  (double) rp7_params->prf4 / 10.);
  fprintf(stderr, "prf5                    %g\n",
	  (double) rp7_params->prf5 / 10.);
  
  fprintf(stderr, "radar altitude (m)      %ld\n",
	  (long) rp7_params->altitude);
  fprintf(stderr, "radar latitude (deg)    %g N\n",
	  (double) (rp7_params->latitude) * (90.0 / 65536.0));
  fprintf(stderr, "radar longitude (deg)   %g W\n",
	  (double) (rp7_params->longitude) * (180.0 / 65536.0));
  
  fprintf(stderr, "size of param header    %ld\n",
	  (long) rp7_params->size_rp7_params);
  fprintf(stderr, "logical record size     %ld\n",
	  (long) rp7_params->rec_size * 2);
  fprintf(stderr, "logical record number   %ld\n",
	  (long) rp7_params->num_log_rcd);
  fprintf(stderr, "nfields                 %ld\n",
	  (long) rp7_params->nfields);
  
  fprintf(stderr, "radar name              %s\n", radar_name);
  fprintf(stderr, "project number          %ld\n",
	  (long) rp7_params->proj_num);
  
  for (i = 0; i < (si32) rp7_params->nfields; i++) {
    
    fprintf(stderr, "parameter %d scale       %g\n", i,
	    (double) rp7_params->lscale[i].scale /
	    (double) RP7_SCALE_AND_BIAS_MULT);
    fprintf(stderr, "parameter %d bias        %g\n", i,
	    (double) rp7_params->lscale[i].bias / 
	    (double) RP7_SCALE_AND_BIAS_MULT);
    
  }
  
}

static void print_gate_data_header(void)

{

  ui08 *gate_data;
  int i;
  date_time_t dt;
  gate_data_radar_params_t *gate_rparams;
  gate_data_field_params_t *gate_fparams;
  gate_data_beam_header_t *gate_header;

  get_gate_data_ptrs(&gate_rparams, &gate_fparams, &gate_header, &gate_data);

  dt.unix_time = gate_header->time;
  uconvert_from_utime(&dt);

  fprintf(stderr, "********************************************\n");
  fprintf(stderr, "GATE DATA HEADER\n");
  
  fprintf(stderr, "time                    %s\n", utimestr(&dt));
  
  fprintf(stderr, "azimuth                %g\n",
	  (double) gate_header->azimuth / 1000000.0);
  fprintf(stderr, "elevation              %g\n",
	  (double) gate_header->elevation / 1000000.0);
  fprintf(stderr, "target_elev            %g\n",
	  (double) gate_header->target_elev / 1000000.0);
  fprintf(stderr, "vol_num                %ld\n",
	  (long) gate_header->vol_num);
  fprintf(stderr, "tilt_num               %ld\n",
	  (long) gate_header->tilt_num);
  fprintf(stderr, "scan_mode              %ld\n",
	  (long) gate_rparams->scan_mode);
  fprintf(stderr, "end_of_tilt            %ld\n",
	  (long) gate_header->end_of_tilt);
  fprintf(stderr, "end_of_volume          %ld\n",
	  (long) gate_header->end_of_volume);

  fprintf(stderr, "\n");

  fprintf(stderr, "radar_id                %ld\n",
	  (long) gate_rparams->radar_id);
  fprintf(stderr, "radar altitude          %g\n",
	  gate_rparams->altitude / 1000.0);
  fprintf(stderr, "radar latitude          %g\n",
	  gate_rparams->latitude / 1000000.0);
  fprintf(stderr, "radar longitude         %g\n",
	  gate_rparams->longitude / 1000000.0);
  fprintf(stderr, "ngates                  %ld\n",
	  (long) gate_rparams->ngates);
  fprintf(stderr, "gate_spacing            %g\n",
	  gate_rparams->gate_spacing / 1000000.0);
  fprintf(stderr, "start_range             %g\n",
	  gate_rparams->start_range / 1000000.0);
  fprintf(stderr, "beam_width              %g\n",
	  gate_rparams->beam_width / 1000000.0);
  fprintf(stderr, "samples_per_beam        %ld\n",
	  (long) gate_rparams->samples_per_beam);
  fprintf(stderr, "pulse_width             %g\n",
	  gate_rparams->pulse_width / 1000.0);
  fprintf(stderr, "prf                     %g\n",
	  gate_rparams->prf / 1000.0);
  fprintf(stderr, "wavelength              %g\n",
	  gate_rparams->wavelength / 10000.0);
  fprintf(stderr, "nfields                 %ld\n",
	 (long)  gate_rparams->nfields);
  
  for (i = 0; i < gate_rparams->nfields; i++) {
    
    fprintf(stderr, "parameter %d scale       %g\n", i,
	    (double) gate_fparams[i].scale /
	    (double) gate_fparams[i].factor);
    
    fprintf(stderr, "parameter %d bias        %g\n", i,
	    (double) gate_fparams[i].bias /
	    (double) gate_fparams[i].factor);
    
  }
  
}

static void print_bprp_header(void)

{

  bprp_params_t *bprp_params;
  bprp_data_t *bprp_data;
  
  get_bprp_ptrs(&bprp_params, &bprp_data);
  
  fprintf(stderr, "********************************************\n");
  fprintf(stderr, "BPRP HEADER\n");

  fprintf(stderr, "date                    %d/%d/%d\n", 
	 bprp_params->year, bprp_params->month, bprp_params->day);
  fprintf(stderr, "time                    %02d:%02d:%02d\n", 
	 bprp_params->hour, bprp_params->min, bprp_params->sec);
  fprintf(stderr, "scan_mode               %d\n", bprp_params->scan_mode);
  fprintf(stderr, "vol_num                 %02d\n", bprp_params->vol_num);
  fprintf(stderr, "tilt_num                %02d\n", bprp_params->tilt_num);
  fprintf(stderr, "start_azimuth           %g\n", bprp_params->start_azimuth);
  fprintf(stderr, "end_azimuth             %g\n", bprp_params->end_azimuth);
  fprintf(stderr, "azimuth                 %g\n", bprp_params->azimuth);
  fprintf(stderr, "elevation               %g\n", bprp_params->elevation);
  fprintf(stderr, "target elevation        %g\n",
	  bprp_params->target_elevation);
  fprintf(stderr, "raycount                %d\n", bprp_params->raycount);
  fprintf(stderr, "mds                     %g\n", bprp_params->mds);
  fprintf(stderr, "noise                   %g\n", bprp_params->noise);
  fprintf(stderr, "viphi                   %g\n", bprp_params->viphi);
  fprintf(stderr, "viplo                   %g\n", bprp_params->viplo);
  fprintf(stderr, "phi                     %g\n", bprp_params->phi);
  fprintf(stderr, "plo                     %g\n", bprp_params->plo);
  fprintf(stderr, "azcwlim                 %d\n", bprp_params->azcwlim);   
  fprintf(stderr, "azccwlim                %d\n", bprp_params->azccwlim);   
  fprintf(stderr, "xmt                     %g\n", bprp_params->xmt);
  fprintf(stderr, "site                    %d\n", bprp_params->site);
  fprintf(stderr, "skip                    %d\n", bprp_params->skip);
  fprintf(stderr, "binwidth                %d\n", bprp_params->binwidth);
  fprintf(stderr, "ints                    %d\n", bprp_params->ints);
  
}

    

