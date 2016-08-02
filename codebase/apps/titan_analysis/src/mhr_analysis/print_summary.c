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
 * print_summary.c
 *
 * prints a summary of a radar beam record stdout
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * December 1993
 *
 **************************************************************************/

#include "mhr_analysis.h"

#define HDR " Vol Tilt El_tgt El_act     Az Ngat Gspac  PRF       Date     Time"
#define FMT "%4ld %4ld %6.2f %6.2f %6.2f %4ld %5ld %4ld %.4ld/%.2ld/%.2ld %.2ld:%.2ld:%.2ld"

void print_summary (char *beam_buffer)

{

  static double cf = 360.0 / 65535.0;
  rp7_params_t *rp7_params;

  rp7_params = (rp7_params_t *) beam_buffer;

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
  
  fflush(stdout);

}
