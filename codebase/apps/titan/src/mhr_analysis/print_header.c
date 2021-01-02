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
 * print_header.c
 *
 * prints details of a radar beam header to stdout
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * December 1993
 *
 **************************************************************************/

#include "mhr_analysis.h"

void print_header (char *beam_buffer)

{
  
  static double	cf = 360.0 / 65535.0;

  rp7_params_t *rp7_params;
  
  int i;
  char radar_name[3];
  
  rp7_params = (rp7_params_t *) beam_buffer;
  
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
  
  fflush(stdout);
  
}
