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
 * load_alenia_params.c
 *
 * Loads param struct from alenia header.
 *
 * On success, returns 0.
 * On failure returns -1
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307-3000
 *
 * July 1997
 *
 **************************************************************************/

#include "Alenia2Mom.h"
using namespace std;

static int VolNum = 0;
static int TiltNum = 0;
static int PrevTiltNum = 0;

static int get_tilt_num(double elevation);

int load_alenia_params(alenia_header_t *al_header,
		       alenia_params_t *al_params)

{

  int z_range;
  int ifield = 0;
  date_time_t *beam_time;

  beam_time = get_beam_time();
  al_params->time = beam_time->unix_time;

  /*
   * azimuth and elevation
   */
  
  al_params->azimuth = (double) ((int) al_header->azim_h * 256 +
			     al_header->azim_l) * ALENIA_ANGLE_CONV;

  al_params->elevation = (double) ((int) al_header->elev_h * 256 +
			       al_header->elev_l) * ALENIA_ANGLE_CONV;
  
  TiltNum = get_tilt_num(al_params->elevation);
  if (TiltNum != PrevTiltNum && TiltNum == 0) {
    VolNum++;
  }
  PrevTiltNum = TiltNum;

  al_params->tilt_num = TiltNum;
  al_params->vol_num = VolNum;
  al_params->elev_target = Glob->params.elev_table.val[TiltNum];

  /*
   * pulse width
   */
  
  switch (al_header->scan_mode & 3) {
    
  case 0:
    al_params->pulse_width = 3.0;
    break;
    
  case 1:
    al_params->pulse_width = 1.5;
    break;
    
  case 2:
    al_params->pulse_width = 0.5;
    break;
    
  default:
    fprintf(stderr, "ERROR - %s:read_alenia_record\n", Glob->prog_name);
    fprintf(stderr, "pulse_width bits incorrect\n");
    
  } /* switch (al_header->scan_mode & 3) */
  
  
  /*
   * scan mode
   */
  
  switch ((al_header->scan_mode >> 2) & 3) {

  case 0:
    al_params->scan_mode = GATE_DATA_SURVEILLANCE_MODE;
    break;
    
  case 1:
    al_params->scan_mode = GATE_DATA_SURVEILLANCE_MODE;
    break;
    
  case 2:
    al_params->scan_mode = GATE_DATA_SECTOR_MODE;
    break;
    
  case 3:
    al_params->scan_mode = GATE_DATA_RHI_MODE;
    break;
    
  } /* switch (al_header->scan_mode & 3) */

  /*
   * dual_prf?
   */

  al_params->dual_prf = (al_header->parameters >> 3) & 1;

  /*
   * available fields
   */
  
  al_params->dbz_avail = al_header->parameters & 1;
  al_params->zdr_avail = (al_header->parameters >> 1) & 1;
  al_params->vel_avail = (al_header->parameters >> 2) & 1;
  al_params->width_avail = (al_header->parameters >> 7) & 1;
  
  al_params->nfields = 0;
  al_params->nfields += al_params->dbz_avail;
  al_params->nfields += al_params->zdr_avail;
  al_params->nfields += al_params->vel_avail;
  al_params->nfields += al_params->width_avail;
  
  /*
   * gate spacing
   */
  
  switch ((al_header->parameters >> 4) & 7) {
    
  case 0:
    al_params->gate_spacing = 0.0625;
    break;
    
  case 1:
    al_params->gate_spacing = 0.125;
    break;
    
  case 2:
    al_params->gate_spacing = 0.25;
    break;

  case 3:
    al_params->gate_spacing = 0.5;
    break;

  case 4:
    al_params->gate_spacing = 1.0;
    break;

  case 5:
    al_params->gate_spacing = 2.0;
    break;

  default:
    fprintf(stderr, "ERROR - %s:read_alenia_record\n", Glob->prog_name);
    fprintf(stderr, "gate_spacing bits incorrect\n");
    
  } /* switch ((al_header->parameters >> 4) & 7) */
  
  al_params->start_range = al_params->gate_spacing / 2.0;
  
  /*
   * number of gates
   */
  
  al_params->ngates =
    ((int) (al_header->avarie >> 6) * 256) + al_header->num_bin_l;

  /*
   * freqency code
   */

  al_params->freq_num = (al_header->avarie >> 3) & 7;

  /*
   * filters and clutter maps
   */

  al_params->clutter_map = al_header->clutter & 1;
  al_params->rejected_by_filter = !((al_header->clutter > 1) & 1);
  al_params->filter_num = (al_header->clutter > 2) & 15;
  al_params->clutter_filter = (al_header->clutter > 6) & 1;
  al_params->clutter_correction = (al_header->clutter > 7);
  
  /*
   * number of pulses
   */
  
  al_params->npulses =
    ((int) (al_header->num_pulses_h >> 6) * 256) + al_header->num_pulses_l;

  /*
   * dbz
   */

  ifield = 0;
    
  if (Glob->params.output_dbz) {

    z_range = al_header->num_pulses_h & 3;
    
    if (Glob->params.rescale_dbz) {
      
      al_params->scale[ifield] = Glob->params.final_dbz_scale;
      al_params->bias[ifield] = Glob->params.final_dbz_bias;
      rescale_set_z_range(z_range);
      
    } else {
      
      al_params->scale[ifield] = ALENIA_DBZ_SCALE;
      if (z_range == ALENIA_Z_RANGE_MED) {
	al_params->bias[ifield] = ALENIA_DBZ_BIAS_MED;
      } else if (z_range == ALENIA_Z_RANGE_HIGH) {
	al_params->bias[ifield] = ALENIA_DBZ_BIAS_HIGH;
      } else {
	al_params->bias[ifield] = ALENIA_DBZ_BIAS_LOW;
      }
    }
    
    ifield++;

  }

  /*
   * zdr
   */

  if (Glob->params.output_zdr) {
    al_params->scale[ifield] = ALENIA_ZDR_SCALE;
    al_params->bias[ifield] = ALENIA_ZDR_BIAS;
    ifield++;
  }

  /*
   * vel
   */

  if (Glob->params.output_vel) {
    if (al_params->dual_prf) {
      al_params->prf = 1200;
      al_params->scale[ifield] = ALENIA_VEL_SCALE_DUAL;
      al_params->bias[ifield] = ALENIA_VEL_BIAS_DUAL;
    } else {
      al_params->prf = 1000;
      al_params->scale[ifield] = ALENIA_VEL_SCALE;
      al_params->bias[ifield] = ALENIA_VEL_BIAS;
    }
    ifield++;
  }

  /*
   * width
   */

  if (Glob->params.output_width) {
    al_params->scale[ifield] = ALENIA_SPW_SCALE;
    al_params->bias[ifield] = ALENIA_SPW_BIAS;
    ifield++;
  }

  return (0);

}

static int get_tilt_num(double elevation)

{

  int i;
  si32 tilt_num;
  double min_diff = 1.0e99;
  double diff;

  for (i = 0; i < Glob->params.elev_table.len; i++) {

    diff = fabs(Glob->params.elev_table.val[i] - elevation);
    
    if (diff < min_diff) {
      tilt_num = i;
      min_diff = diff;
    }

  } /* i */

  return (tilt_num);

}
