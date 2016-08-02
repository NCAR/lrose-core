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

#include "polar2gate.h"

#define HDR " Vol Tilt El_tgt El_act     Az Ngat Gspac  PRF       Date     Time"
#define FMT "%4ld %4ld %6.2f %6.2f %6.2f %4ld %5ld %4ld %.4ld/%.2ld/%.2ld %.2ld:%.2ld:%.2ld"

void print_summary (ui08 *beam_data)

{

  static double cf = 360.0 / 65535.0;
  
  date_time_t dtime;
  ll_params_t *ll_params;
  rp7_params_t *rp7_params;
  chill_params_t *ch_params;
  chldat1_t *chldat1;
  lass_params_t * lass_params;
  alenia_params_t *al_params;
  
  switch (Glob->header_type) {

  case ALENIA_HEADER:
    al_params = (alenia_params_t *) beam_data;
    dtime.unix_time = al_params->time;
    uconvert_from_utime(&dtime);
    printf(HDR);
    printf("\n");
    printf(FMT,
	   (long) al_params->vol_num, 
	   (long) al_params->tilt_num,
	   (double) al_params->elevation,
	   (double) al_params->elevation,
	   (double) al_params->azimuth,
	   (long) al_params->ngates,
	   (long) (al_params->gate_spacing * 1000.0 + 0.5),
	   (long) al_params->prf,
	   (long) dtime.year,
	   (long) dtime.month,
	   (long) dtime.day,
	   (long) dtime.hour,
	   (long) dtime.min,
	   (long) dtime.sec);
    printf("\n");
    break;

  case LASS_HEADER:
    lass_params = ( lass_params_t *) beam_data;
    printf(HDR);
    printf("\n");
    printf(FMT,
        (long)lass_params->volSummary.volume, 
        (long)lass_params->sweep,
        (double)lass_params->targetAngle,
        (double)lass_params->myElevation,
        (double)lass_params->myAzymuth,
        (long)360,
        (long)lass_params->volSummary.gatewid,
        (long)lass_params->volSummary.prf,
        (long)lass_params->volSummary.year+1900,
        (long)lass_params->volSummary.month,
        (long)lass_params->volSummary.day,
        (long)lass_params->volSummary.shour,
        (long)lass_params->volSummary.sminute,
        (long)lass_params->volSummary.ssecond);
        printf("\n");
        break;

  case LINCOLN_HEADER:

    ll_params = (ll_params_t *) beam_data;

    printf(HDR);
    printf("\n");

    printf(FMT,
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
    printf("\n");

    break;

  case RP7_HEADER:

    rp7_params = (rp7_params_t *) beam_data;

    printf(HDR);
    printf("\n");

    printf(FMT,
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
    printf("\n");

    break;

  case CHILL_HEADER:

    ch_params = (chill_params_t *) beam_data;
    chldat1 = (chldat1_t *) &ch_params->chldat1;
    
    printf(HDR);
    
    if (Glob->chill_extended_hsk) {
      printf(" Scantype Fields\n");
    } else {
      printf(" Fields\n");
    }
    
    printf(FMT,
	   (long) chldat1->volnum,
	   (long) chldat1->sweepnum,
	   ch_params->target_el,
	   (double) chldat1->el * CHILL_DEG_CONV,
	   (double) chldat1->az * CHILL_DEG_CONV,
	   (long) ch_params->ngates,
	   (long) (ch_params->gate_spacing * 1000.0 + 0.5),
	   (long) (ch_params->prf + 0.5),
	   (long) chldat1->year,
	   (long) chldat1->month,
	   (long) chldat1->day,
	   (long) chldat1->hour,
	   (long) chldat1->min,
	   (long) chldat1->sec);

    if (Glob->chill_extended_hsk) {
      printf("%9s", ch_params->segname);
    }

    printf(" %s\n", ch_params->field_str);

    break;

  } /* switch */

  fflush(stdout);

}
