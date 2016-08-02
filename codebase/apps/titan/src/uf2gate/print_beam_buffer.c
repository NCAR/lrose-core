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
 * print_beam_buffer.c
 *
 * prints details of a radar beam buffer to stdout
 *
 * Nancy Rehak RAP NCAR Boulder CO USA
 *
 * Mar 1995
 *
 **************************************************************************/

#include "uf2gate.h"

void print_beam_buffer (ui08 *beam_data)

{

  gate_data_beam_header_t   *beam_hdr;
  
  /*
   * Position the header pointers within the buffer.
   */

  beam_hdr = (gate_data_beam_header_t *)beam_data;
  
  /*
   * Print out the beam header information.
   */

  fprintf(stdout, "\n");
  fprintf(stdout, "***************************************\n");
  fprintf(stdout, "BEAM HEADER\n");
  fprintf(stdout, "\n");
  
  fprintf(stdout, "time:                  %d secs\n",
	 beam_hdr->time);
  fprintf(stdout, "                       %s", utimstr(beam_hdr->time));
  fprintf(stdout, "azimuth:               %f degrees\n",
	 (float)beam_hdr->azimuth / 1000000.0);
  fprintf(stdout, "elevation:             %f degrees\n",
	 (float)beam_hdr->elevation / 1000000.0);
  fprintf(stdout, "target elevation:      %f degrees\n",
	 (float)beam_hdr->target_elev / 1000000.0);
  fprintf(stdout, "volume number:         %d\n",
	 beam_hdr->vol_num);
  fprintf(stdout, "tilt number:           %d\n",
	 beam_hdr->tilt_num);
  fprintf(stdout, "new scan limits?:      %d\n",
	 beam_hdr->new_scan_limits);
  fprintf(stdout, "end of tilt?:          %d\n",
	 beam_hdr->end_of_tilt);
  fprintf(stdout, "end of volume?:        %d\n",
	 beam_hdr->end_of_volume);
  
  fprintf(stdout, "\n\n");
  
  fflush(stdout);
  
}
