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
 * print_summary.c
 *
 * prints a summary of a radar beam record stdout
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * December 1991
 *
 **************************************************************************/

#include "polar_ingest.h"

#define HDR " Vol Tilt El_tgt El_act     Az Ngat Gspac  PRF       Date     Time"
#define FMT "%4ld %4ld %6.2f %6.2f %6.2f %4ld %5ld %4ld %.4ld/%.2ld/%.2ld %.2ld:%.2ld:%.2ld"

static void print_lincoln_summary(void);
static void print_rp7_summary(void);
static void print_bprp_summary(void);
static void print_gate_data_summary(void);

void print_summary (void)

{
  

  switch (Glob->header_type) {
    
  case LINCOLN_HEADER:
    print_lincoln_summary();
    break;
    
  case RP7_HEADER:
    print_rp7_summary();
    break;
    
  case BPRP_HEADER:
    print_bprp_summary();
    break;
    
  case GATE_DATA_HEADER:
    print_gate_data_summary();
    break;
    
  } /* switch */

  fflush(stdout);
  
}

static void print_lincoln_summary(void)

{
    
  ui08 *ll_data;
  ll_params_t *ll_params;

  get_lincoln_ptrs(&ll_params, &ll_data);
  
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
    
static void print_rp7_summary(void)

{
    
  rp7_params_t *rp7_params;
  ui08 *rp7_data;
  double cf = 360.0 / 65536.0;

  get_rp7_ptrs(&rp7_params, &rp7_data);

  fprintf(stderr, HDR);
  fprintf(stderr, "\n");
  
  fprintf(stderr, FMT,
	  (long) rp7_params->vol_num,
	  (long) rp7_params->tilt_num,
	  (double) rp7_params->target_elev * cf,
	  (double) rp7_params->elevation * cf,
	  (double) rp7_params->azimuth * cf,
	  (long) rp7_params->gates_per_beam,
	  (long) rp7_params->gate_spacing,
	  (long) ((double) rp7_params->prfx10 / 10.0 + 0.5),
	  (long) rp7_params->year,
	  (long) rp7_params->month,
	  (long) rp7_params->day,
	  (long) rp7_params->hour,
	  (long) rp7_params->min,
	  (long) rp7_params->sec);
  fprintf(stderr, "\n");
  
}

static void print_gate_data_summary(void)

{

  ui08 *gate_data;
  date_time_t dt;
  gate_data_radar_params_t *gate_rparams;
  gate_data_field_params_t *gate_fparams;
  gate_data_beam_header_t *gate_header;

  get_gate_data_ptrs(&gate_rparams, &gate_fparams, &gate_header, &gate_data);

  dt.unix_time = gate_header->time;
  uconvert_from_utime(&dt);
  
  fprintf(stderr, HDR);
  fprintf(stderr, "  Id Scan\n");
  
  fprintf(stderr, FMT,
	  (long) gate_header->vol_num,
	  (long) gate_header->tilt_num,
	  (double) gate_header->target_elev / 1000000.0,
	  (double) gate_header->elevation / 1000000.0,
	  (double) gate_header->azimuth / 1000000.0,
	  (long) gate_rparams->ngates,
	  (long) (gate_rparams->gate_spacing / 1000.0 + 0.5),
	  (long) (gate_rparams->prf / 1000.0 + 0.5),
	  (long) dt.year, (long) dt.month, (long) dt.day,
	  (long) dt.hour, (long) dt.min, (long) dt.sec);
  fprintf(stderr, "%4ld %4ld\n",
	  (long)gate_rparams->radar_id,
	  (long)gate_rparams->scan_type);

}

static void print_bprp_summary(void)

{

  bprp_params_t *bprp_params;
  bprp_data_t *bprp_data;
  
  get_bprp_ptrs(&bprp_params, &bprp_data);

  printf(HDR);
  printf("\n");
  
  fprintf(stderr, FMT,
	  (long) bprp_params->vol_num,
	  (long) bprp_params->tilt_num,
	  bprp_params->target_elevation,
	  bprp_params->elevation,
	  bprp_params->azimuth,
	  (long) BPRP_GATES_PER_BEAM,          
	  (long) (BPRP_GATE_SPACING * 1000.0 + 0.5),        
	  (long) (double) BPRP_PRF_NOMINAL,
	  (long) bprp_params->year,
	  (long) bprp_params->month,
	  (long) bprp_params->day,
	  (long) bprp_params->hour,
	  (long) bprp_params->min,
	  (long) bprp_params->sec);
  fprintf(stderr, "\n");
  
}
