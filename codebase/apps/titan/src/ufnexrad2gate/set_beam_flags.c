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
 * set_beam_flags.c
 *
 * Sets the flags which indicate if the beam is the last in a tilt
 * or the last in a volume.
 *
 * returns TRUE if tilt has changed, FALSE otherwise
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * Sept 1992
 *
 * Copied from rdata_to_socket for use in ufnexrad2gate by Nancy Rehak,
 * Mar 1995.
 **************************************************************************/

#include "ufnexrad2gate.h"

int set_beam_flags(ui08 *gate_data_pkt,
		   ui08 *prev_gate_data_pkt)

{
  gate_data_beam_header_t *prev, *current;

  /*
   * set pointers
   */

  prev = (gate_data_beam_header_t *) prev_gate_data_pkt;
  current = (gate_data_beam_header_t *) gate_data_pkt;

  /*
   * check for change in tilt
   */

  if (prev->tilt_num != current->tilt_num)
  {
    prev->end_of_tilt = TRUE;
      
    if ((Glob->params.summary_print && Glob->params.summary_interval <= 90)
	|| Glob->params.header_print)
      printf ("End of tilt\n");
      
  }
  else
  {
    prev->end_of_tilt = FALSE;
  }

  /*
   * check for change in volume
   */

  if (prev->vol_num != current->vol_num)
  {
    prev->end_of_volume = TRUE;
      
    if (Glob->params.summary_print || Glob->params.header_print)
      printf ("End of volume\n");
      
  }
  else
  { 
    prev->end_of_volume = FALSE;
  }

  return ((int) prev->end_of_tilt);
  
}
