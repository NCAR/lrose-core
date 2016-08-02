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

#include "polar2gate.h"

#define BOOL_STR(a) ((a)? "true" : "false")

void print_header (ui08 *beam_data)

{
  
  static double	cf = 360.0 / 65535.0;

  int i;
  char str[12];
  char radar_name[3];
  ll_params_t *ll_params;
  rp7_params_t *rp7_params;
  chill_params_t *ch_params;
  chldat1_t *chldat1;
  lass_params_t *lass_params;
  alenia_params_t *al_params;

  switch (Glob->header_type) {
    
 case ALENIA_HEADER:

    al_params = (alenia_params_t *) beam_data;

    printf("********************************************\n");
    printf("ALENIA HEADER\n");
    
    printf("Time:                   %s\n",
	   utimstr(al_params->time));
    
    printf("azimuth   (deg)         %g\n", al_params->azimuth);
    printf("elevation (deg)         %g\n", al_params->elevation);
    printf("gate_spacing(m)         %g\n", al_params->gate_spacing);
    printf("range_mid_first_gate(m) %g\n", al_params->start_range);
    printf("pulse_width (us)        %g\n", al_params->pulse_width);
    
    printf("tilt_num                %ld\n", (long) al_params->tilt_num);
    printf("vol_num                 %ld\n", (long) al_params->vol_num);
    printf("nfields                 %ld\n", (long) al_params->nfields);
    printf("ngates                  %ld\n", (long) al_params->ngates);
    printf("npulses                 %ld\n", (long) al_params->npulses);
    printf("ndata                   %ld\n", (long) al_params->ndata);
    printf("prf                     %ld\n", (long) al_params->prf);

    printf("dbz_avail                 %s\n",
	   BOOL_STR(al_params->dbz_avail));
    printf("zdr_avail               %s\n",
	   BOOL_STR(al_params->zdr_avail));
    printf("vel_avail               %s\n",
	   BOOL_STR(al_params->vel_avail));
    printf("width_avail             %s\n",
	   BOOL_STR(al_params->width_avail));
    printf("dual_prf                %s\n",
	   BOOL_STR(al_params->dual_prf));
    printf("freq_num                %d\n", al_params->freq_num);
    printf("clutter_map             %s\n",
	   BOOL_STR(al_params->clutter_map));
    printf("rejected_by_filter      %s\n",
	   BOOL_STR(al_params->rejected_by_filter));
    printf("filter_num              %d\n", al_params->filter_num);
    printf("clutter_filter          %s\n",
	   BOOL_STR(al_params->clutter_filter));
    printf("clutter_correction      %s\n",
	   BOOL_STR(al_params->clutter_correction));

    switch (al_params->scan_mode) {
    case GATE_DATA_SURVEILLANCE_MODE:
      printf("scan_mode               surveillance\n");
      break;
    case GATE_DATA_SECTOR_MODE:
      printf("scan_mode               sector\n");
      break;
    case GATE_DATA_RHI_MODE:
      printf("scan_mode               rhi\n");
      break;
    case GATE_DATA_UNKNOWN_MODE:
      printf("scan_mode               unknown\n");
      break;
    } /* switch */

    for (i = 0; i < N_ALENIA_FIELDS; i++) {
      printf("parameter %d scale       %g\n", i + 1,
	     al_params->scale[i]);
      printf("parameter %d bias        %g\n", i + 1,
	     al_params->bias[i]);
    } /* i */
      
    break;

  case LASS_HEADER:
    lass_params = ( lass_params_t *) beam_data;
    printf("********************************************\n");
    printf("LASSEN HEADER\n");
    
    printf("Version                %-4ld \n",
	   (long) lass_params->volSummary.version);
    
    printf("Date                   %2ld/%2ld/%2ld\n",
	   (long) lass_params->volSummary.month,
	   (long) lass_params->volSummary.day,
	   (long) lass_params->volSummary.year); 
    
    printf("Time                   %2ld:%2ld:%2ld\n",
	   (long) lass_params->volSummary.shour,
	   (long) lass_params->volSummary.sminute,
	   (long) lass_params->volSummary.ssecond); 
    
    printf("vol_num                %-ld\n"  ,
	   (long) lass_params->volSummary.volume);
    
    printf("scanmode               %-ld\n"  ,
	   (long) lass_params->volSummary.sweep_type);  
    
    printf("range_to_first_gate    %-ld\n",
	   (long) lass_params->rangeTo1stGate);
    
    printf("gate_spacing(m)        %-ld\n",
	   (long) lass_params->volSummary.gatewid);
    
    printf("prf                    %-ld\n",
	   (long) lass_params->volSummary.prf);
    
    printf("radar, site name       %s, %s\n",
	   lass_params->radinfo.radar_name,
	   lass_params->radinfo.site_name);
    
    printf("latitude               %3ld.%2ld.%2ld\n",   
	   (long) lass_params->radinfo.latdegree,
	   (long) lass_params->radinfo.latminute,
	   (long) lass_params->radinfo.latsecond);
    
    printf("longitude              %3ld.%2ld.%2ld\n",   
	   (long) lass_params->radinfo.londegree,
	   (long) lass_params->radinfo.lonminute,
	   (long) lass_params->radinfo.lonsecond);
    
    printf("altitude               %-ld\n",
	   (long) lass_params->radinfo.antenna_height);
    
    break;

  case LINCOLN_HEADER:
    
    ll_params = (ll_params_t *) beam_data;
    
    printf("********************************************\n");
    printf("RP7 HEADER\n");
    printf("date                    %.4d/%.2d/%.2d\n",
	   ll_params->year, ll_params->month, ll_params->day);
    printf("time                    %.2d:%.2d:%.2d\n",
	   ll_params->hour,ll_params->min, ll_params->sec);
    
    printf("azimuth (deg)           %g\n",
	     (double) (ui16) ll_params->azimuth / 100.0);
    printf("elevation (deg)         %g\n",
	   (double) ll_params->elevation / 100.0);

    printf("vol_num                 %ld\n", (long) ll_params->vol_num);
    printf("tilt_num                %ld\n", (long) ll_params->tilt_num);
    printf("scan_mode               %ld\n", (long) ll_params->scan_mode);
    printf("target_elev (deg)       %g\n", ll_params->target_elev / 100.0);
    printf("scan_dir                %ld\n", (long) ll_params->scan_dir);
    printf("range_to_first_gate(m)  %ld\n",
	   (long) ll_params->range_to_first_gate);
    printf("gates_per_beam          %ld\n",
	   (long) ll_params->range_seg[0].gates_per_beam);
    printf("gate_spacing(m)         %ld\n",
	   (long) ll_params->range_seg[0].gate_spacing);
    printf("prf                     %ld\n", (long) ll_params->prf);
    printf("radar, site names       %s, %s\n",
	   ll_params->radar_name, ll_params->site_name);
    printf("latitude (deg)          %g\n", ll_params->latitude / 100000.0);
    printf("longitude (deg)         %g\n", ll_params->longitude / 100000.0);
    printf("altitude (m)            %ld\n", (long) ll_params->altitude);
    printf("beamwidth (deg)         %g\n", ll_params->beamwidth / 100.0);
    printf("polarization            %ld\n", (long) ll_params->polarization);
    printf("power_trans             %ld\n", (long) ll_params->xmit_pwr);
    printf("frequency (Mhz)         %ld\n", (long) ll_params->frequency);
    printf("pulse width (ns)        %ld\n", (long) ll_params->pulse_width);
    
    for (i = 0; i < LL_NFIELDS; i++) {
      
      printf("parameter %d scale       %g\n", i + 1,
	     (double) ll_params->scale[i] /
	     (double) LL_SCALE_AND_BIAS_MULT);
      printf("parameter %d bias        %g\n", i + 1,
	     (double) ll_params->bias[i] /
	     (double) LL_SCALE_AND_BIAS_MULT);

    } /* i */
      
    break;
    
  case RP7_HEADER:
    
    rp7_params = (rp7_params_t *) beam_data;
    
    strncpy(radar_name, (char *) &(rp7_params->rs_id), 2);
    radar_name[2] = '\0';
    
    printf("********************************************\n");
    printf("RP7 HEADER\n");
    printf("logical seq num         %ld\n",
	   (long) rp7_params->rec_num);
    printf("field tape sequence num %ld\n",
	   (long) rp7_params->field_tape_seq);
    printf("record type             %ld\n",
	   (long) rp7_params->rec_type);
    printf("date                    %.4d/%.2d/%.2d\n",
	   rp7_params->year, rp7_params->month, rp7_params->day);
    printf("time                    %.2d:%.2d:%.2d\n",
	   rp7_params->hour,rp7_params->min, rp7_params->sec);
    
    printf("azimuth (deg)           %g\n",
	   (double) rp7_params->azimuth * cf);
    printf("elevation (deg)         %g\n",
	   (double) rp7_params->elevation * cf);
    
    printf("range to first gate     %ld\n",
	   (long) rp7_params->rhozero1);
    printf("gate spacing (m)        %ld\n",
	   (long) rp7_params->gate_spacing);
    printf("number of gates         %ld\n",
	   (long) rp7_params->gates_per_beam);
    printf("samples per beam        %ld\n",
	   (long) rp7_params->samples_per_beam);
    
    printf("avg_xmit_pwr            %ld\n",
	   (long) rp7_params->avg_xmit_pwr);
    printf("pulse width (nanosec)   %ld\n",
	   (long) rp7_params->pulse_width);
    printf("prf                     %g\n",
	   (double) rp7_params->prfx10 / 10.0);
    printf("wavelength (cm)         %g\n",
	   (double) rp7_params->wavelength / 100.0);
    printf("tilt seq                %ld\n",
	   (long) rp7_params->tilt_seq);
    printf("scan mode               %ld\n",
	   (long) rp7_params->scan_mode);
    
    printf("cw_az_lim   (deg)       %g\n",
	   (double) rp7_params->cw_az_lim * cf);
    printf("ccw_az_lim   (deg)      %g\n",
	   (double) rp7_params->ccw_az_lim * cf);
    
    printf("target elev (deg)       %g\n",
	   (double) rp7_params->target_elev * cf);
    
    printf("beamwidth (deg)         %g\n",
	   (double) (rp7_params->beamwidth) / 100.);
    printf("scan_rate               %ld\n",
	   (long) rp7_params->scan_rate);
    printf("vol num                 %ld\n",
	   (long) rp7_params->vol_num);
    printf("polarization            %ld\n",
	   (long) rp7_params->polarization);
    printf("prf1                    %g\n", (double) rp7_params->prf1 / 10.);
    printf("prf2                    %g\n", (double) rp7_params->prf2 / 10.);
    printf("prf3                    %g\n", (double) rp7_params->prf3 / 10.);
    printf("prf4                    %g\n", (double) rp7_params->prf4 / 10.);
    printf("prf5                    %g\n", (double) rp7_params->prf5 / 10.);
    
    printf("radar altitude (m)      %ld\n",
	   (long) rp7_params->altitude);
    printf("radar latitude (deg)    %g N\n",
	   (double) (rp7_params->latitude) * (90.0 / 65535.0));
    printf("radar longitude (deg)   %g W\n",
	   (double) (rp7_params->longitude) * (180.0 / 65535.0));
    
    printf("size of param header    %ld\n",
	   (long) rp7_params->size_rp7_params);
    printf("logical record size     %ld\n",
	   (long) rp7_params->rec_size * 2);
    printf("logical record number   %ld\n",
	   (long) rp7_params->num_log_rcd);
    printf("nfields                 %ld\n",
	   (long) rp7_params->nfields);
    
    printf("radar name              %s\n", radar_name);
    printf("project number          %ld\n",
	   (long) rp7_params->proj_num);
    
    for (i = 0; i < (si32) rp7_params->nfields; i++) {
      
      printf("parameter %d scale       %g\n", i,
	     (double) rp7_params->lscale[i].scale /
	     (double) RP7_SCALE_AND_BIAS_MULT);
      printf("parameter %d bias        %g\n", i,
	     (double) rp7_params->lscale[i].bias / 
	     (double) RP7_SCALE_AND_BIAS_MULT);
      
    }
    
    break;
    
  case CHILL_HEADER:

    ch_params = (chill_params_t *) beam_data;
    chldat1 = (chldat1_t *) &ch_params->chldat1;

    printf("********************************************\n");
    printf("CHILL HEADER\n");
    printf("raylen                 %ld\n", (long) chldat1->raylen);
    printf("offset1                %ld\n", (long) chldat1->offset1);
    printf("elevation              %g\n",
	   (double) chldat1->el * CHILL_DEG_CONV);
    printf("azimuth                %g\n",
	   (double) chldat1->az * CHILL_DEG_CONV);
    printf("raynum                 %ld\n", (long) chldat1->raynum);
    printf("date                   %.4d/%.2d/%.2d\n",
	   chldat1->year, chldat1->month, chldat1->day);
    printf("time                   %.2d:%.2d:%.2d.%2d\n",
	   chldat1->hour,chldat1->min,
	   chldat1->sec, chldat1->tenths);

    printf("antstat                %ld\n", (long) chldat1->antstat);
    printf("volnum                 %ld\n", (long) chldat1->volnum);
    printf("sweepnum               %ld\n", (long) chldat1->sweepnum);

    printf("target az              %g\n",
	   (double) chldat1->azprogpos * CHILL_FIXED_CONV);

    printf("target el              %g\n", ch_params->target_el);

    if (Glob->chill_extended_hsk) {

      printf("cw az limit            %g\n",
	     (double) chldat1->azcwlim * CHILL_FIXED_CONV);
      printf("ccm az limit           %g\n",
	     (double) chldat1->azccwlim * CHILL_FIXED_CONV);
      printf("prt                    %ld\n", (long) chldat1->prt);
      printf("sweep rate             %g\n",
	     (double) chldat1->swprate / 100.0);
      printf("hits                   %ld\n", (long) chldat1->hits);
      printf("scanmode               %ld\n", (long) chldat1->scanmode);
      printf("pulse_len              %ld\n",
	     (long) chldat1->pulse_len / 1024L);
      printf("gate_space             %ld\n",
	     (long) (ch_params->gate_spacing * 1000.0 + 0.5));
      printf("txbin                  %ld\n", (long) chldat1->txbin);
      printf("txpwr                  %g\n",
	     (double) chldat1->txpwr / 256.0);
      printf("maxtop                 %ld\n", (long) chldat1->maxtop);
      printf("el_down                %g\n",
	     (double) chldat1->el_down * CHILL_DEG_CONV);
      printf("el_up                  %g\n",
	     (double) chldat1->el_up * CHILL_DEG_CONV);
      printf("step_opt               %ld\n", (long) chldat1->step_opt);
      printf("bypass                 %ld\n", (long) chldat1->bypass);
      printf("opt_rmax               %g\n",
	     (double) chldat1->opt_rmax / 100.0);
      printf("opt_htmax              %g\n",
	     (double) chldat1->opt_htmax / 100.0);
      printf("opt_res                %g\n",
	     (double) chldat1->opt_res / 100.0);
      printf("filt_end               %ld\n", (long) chldat1->filt_end);
      printf("filt_num               %ld\n", (long) chldat1->filt_num);
      printf("nyqvel                 %g\n",
	     (double) chldat1->nyqvel / 256.0);

      strncpy(str, chldat1->segname, 8);
      str[8] = '\0';
      printf("segname                %s\n", str);
      
      strncpy(str, chldat1->sp20prog, 8);
      str[8] = '\0';
      printf("sp20prog               %s\n", str);
      
      strncpy(str, chldat1->polseq, 8);
      str[8] = '\0';
      printf("polseq                 %s\n", str);

    } /* if (Glob->chill_extended_hsk) */

    break;

  } /* switch */

  fflush(stdout);
  
}
