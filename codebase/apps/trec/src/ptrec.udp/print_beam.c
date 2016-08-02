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
/****************************************************************
 * print_beam()
 */

#include "trec.h"

static void print_header(ll_params_t *beam);
static void print_summary(ll_params_t *beam);

void print_beam(ll_params_t *beam)

{

  static int header_count = 0;
  static int summary_count = 0;

  /*
   * print out as requested
   */

  if (Glob->params.print_header) {
    
    if (header_count == 0)
      print_header(beam);
	
    header_count++;
	
    if (header_count == Glob->params.header_interval)
      header_count = 0;
	
   } /* if (Glob->params.header_print) */
      
  if (Glob->params.print_summary) {
    
    if (summary_count == 0)
      
      print_summary(beam);
    
    summary_count++;
    
    if (summary_count == Glob->params.summary_interval)
      summary_count = 0;
    
  } /* if (Glob->params.summary_print) */

}
      
static void print_header(ll_params_t *beam)

{
    
  int i;
  
  fprintf(stderr, "********************************************\n");
  fprintf(stderr, "LINCOLN HEADER\n");
  fprintf(stderr, "date                    %.4d/%.2d/%.2d\n",
	  beam->year, beam->month, beam->day);
  fprintf(stderr, "time                    %.2d:%.2d:%.2d\n",
	  beam->hour,beam->min, beam->sec);
  
  fprintf(stderr, "azimuth (deg)           %g\n",
	  (double) (ui16) beam->azimuth / 100.0);
  fprintf(stderr, "elevation (deg)         %g\n",
	  (double) beam->elevation / 100.0);
  
  fprintf(stderr, "vol_num                 %ld\n",
	  (long) beam->vol_num);
  fprintf(stderr, "tilt_num                %ld\n",
	  (long) beam->tilt_num);
  fprintf(stderr, "scan_mode               %ld\n",
	  (long) beam->scan_mode);
  fprintf(stderr, "target_elev (deg)       %g\n",
	  beam->target_elev / 100.0);
  fprintf(stderr, "scan_dir                %ld\n",
	  (long) beam->scan_dir);
  fprintf(stderr, "range_to_first_gate(m)  %ld\n",
	  (long) beam->range_to_first_gate);
  fprintf(stderr, "gates_per_beam          %ld\n",
	  (long) beam->range_seg[0].gates_per_beam);
  fprintf(stderr, "gate_spacing(m)         %ld\n",
	  (long) beam->range_seg[0].gate_spacing);
  fprintf(stderr, "prf                     %ld\n",
	  (long) beam->prf);
  fprintf(stderr, "radar, site names       %s, %s\n",
	  beam->radar_name, beam->site_name);
  fprintf(stderr, "latitude (deg)          %g\n",
	  beam->latitude / 100000.0);
  fprintf(stderr, "longitude (deg)         %g\n",
	  beam->longitude / 100000.0);
  fprintf(stderr, "altitude (m)            %ld\n",
	  (long) beam->altitude);
  fprintf(stderr, "beamwidth (deg)         %g\n",
	  beam->beamwidth / 100.0);
  fprintf(stderr, "polarization            %ld\n",
	  (long) beam->polarization);
  fprintf(stderr, "power_trans             %ld\n",
	  (long) beam->xmit_pwr);
  fprintf(stderr, "frequency (Mhz)         %ld\n",
	  (long) beam->frequency);
  fprintf(stderr, "pulse width (ns)        %ld\n",
	  (long) beam->pulse_width);
  
  for (i = 0; i < LL_NFIELDS; i++) {
    
    fprintf(stderr, "parameter %d scale       %g\n", i + 1,
	    (double) beam->scale[i] /
	    (double) LL_SCALE_AND_BIAS_MULT);
    fprintf(stderr, "parameter %d bias        %g\n", i + 1,
	    (double) beam->bias[i] /
	    (double) LL_SCALE_AND_BIAS_MULT);
    
  } /* i */
  
}
    
#define HDR " Vol Tilt El_tgt El_act     Az Ngat Gspac  PRF       Date     Time"
#define FMT "%4ld %4ld %6.2f %6.2f %6.2f %4ld %5ld %4ld %.4ld/%.2ld/%.2ld %.2ld:%.2ld:%.2ld"

static void print_summary(ll_params_t *beam)

{
    
  fprintf(stderr, HDR);
  fprintf(stderr, "\n");

  fprintf(stderr, FMT,
	  (long) beam->vol_num,
	  (long) beam->tilt_num,
	  (double) beam->target_elev / 100.0,
	  (double) beam->elevation / 100.0,
	  (double) (ui16) beam->azimuth / 100.0,
	  (long) beam->range_seg[0].gates_per_beam,
	  (long) beam->range_seg[0].gate_spacing,
	  (long) beam->prf,
	  (long) beam->year,
	  (long) beam->month,
	  (long) beam->day,
	  (long) beam->hour,
	  (long) beam->min,
	  (long) beam->sec);
  fprintf(stderr, "\n");

}
    
