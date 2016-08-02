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
 * prints a summary of a radar beam record to stdout
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * December 1991
 *
 * Copied from rdata_to_socket for use in uf2gate by Nancy Rehak,
 * Mar 1995.
 **************************************************************************/

#include "uf2gate.h"

#define HDR " Vol Tilt El_tgt El_act     Az Ngat Gspac  PRF       Date     Time"
#define FMT "%4ld %4ld %6.2f %6.2f %6.2f %4ld %5ld %4ld %.4d/%.2d/%.2d %.2d:%.2d:%.2d"

void print_summary (ui08 *beam_data, ui08 *param_data)
{
  gate_data_beam_header_t  *beam_hdr;
  gate_data_radar_params_t *radar_hdr;

  date_time_t    date_struct;
  
  beam_hdr = (gate_data_beam_header_t *)beam_data;
  radar_hdr = (gate_data_radar_params_t *)param_data;
  
  date_struct.unix_time = beam_hdr->time;
  uconvert_from_utime(&date_struct);
  
  fprintf(stdout, HDR);
  fprintf(stdout, "\n");
  fprintf(stdout, FMT,
	  (long) beam_hdr->vol_num,
	  (long) beam_hdr->tilt_num,
	  (float)beam_hdr->target_elev / 1000000.0,
	  (float)beam_hdr->elevation / 1000000.0,
	  (float)beam_hdr->azimuth / 1000000.0,
	  (long) radar_hdr->ngates,
	  (long)((float)radar_hdr->gate_spacing / 1000.0 + 0.5),
	  (long)((float)radar_hdr->prf / 1000.0 + 0.5),
	  date_struct.year,
	  date_struct.month,
	  date_struct.day,
	  date_struct.hour,
	  date_struct.min,
	  date_struct.sec);
  fprintf(stdout, "\n");
  
  fflush(stdout);

}
