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
 * create_beam_buffer.c
 *
 * creates the test beam buffer
 *
 * Nancy Rehak RAP NCAR Boulder CO USA
 *
 * September 1995
 *
 **************************************************************************/

#include "test2gate.h"
#include <time.h>

int create_beam_buffer(gate_data_beam_header_t *beam_buffer)

{
  static int first_time = TRUE;
  static scan_table_t scan_table;
  static int curr_elev = 0;
  static int curr_az = 0;
  static int vol_num = 0;
  static int tilt_num = 0;
  
  int end_of_tilt = FALSE;
  int end_of_volume = FALSE;
  int new_tilt = FALSE;
  
  int i;
  ui08 *byte_ptr;
  
  /*
   * Read in the scan table.
   */

  if (first_time)
  {
    setup_scan(&scan_table);
    calc_sampling_geom(&scan_table);
    first_time = FALSE;
  }

  /*
   * See if we are at the end of the current tilt.
   */

  if (curr_az >= scan_table.elevs[curr_elev].naz)
  {
    curr_az = 0;
    new_tilt = TRUE;
    tilt_num++;
    
    curr_elev++;
    if (curr_elev >= scan_table.nelevations)
    {
      curr_elev = 0;
      tilt_num = 0;
      vol_num++;
      sleep(Glob->params.vol_wait_secs);
    }
  }
  
  if (curr_az == scan_table.elevs[curr_elev].naz - 1)
  {
    end_of_tilt = TRUE;

    if (curr_elev == scan_table.nelevations - 1)
      end_of_volume = TRUE;
  }

  /*
   * Fill in the header information.
   */

  beam_buffer->time = time(NULL);
  beam_buffer->azimuth = 
    (si32)(scan_table.elevs[curr_elev].azs[curr_az].angle * 1000000.0 + 0.5);
  beam_buffer->elevation =
    (si32)(scan_table.elev_angles[curr_elev] * 1000000.0 + 0.5);
  beam_buffer->target_elev = beam_buffer->elevation;
  beam_buffer->vol_num = vol_num;
  beam_buffer->tilt_num = tilt_num;
  beam_buffer->new_scan_limits = FALSE;
  beam_buffer->end_of_tilt = end_of_tilt;
  beam_buffer->end_of_volume = end_of_volume;

  BE_from_gate_data_beam_header(beam_buffer);
  
  byte_ptr = (ui08 *)beam_buffer + sizeof(gate_data_beam_header_t);

  /*
   * initialize to missing data
   */

  memset(byte_ptr, 0, Glob->nfields * Glob->params.radar_params.num_gates);

  /*
   * field 0 and 1 are dbz and velocity sampled from the
   * dobson file
   */

  sample_dbz_and_vel(byte_ptr, &scan_table, curr_elev, curr_az);

  /*
   * if needed, fields 3, 4 and 5 are elev, az and range bin #
   */

  if (Glob->params.output_geom_fields) {

    byte_ptr += (2 * Glob->params.radar_params.num_gates);

    /*
     * field 2 is elev, field 3 is azimuth, field 4 is range
     */
    
    /*
     * elev
     */
    
    for (i = 0; i < Glob->params.radar_params.num_gates; i++)
      {
	*byte_ptr = (ui08) (curr_elev + 1);
	byte_ptr++;
      }
    
    /*
     * azimuth
     */
    
    for (i = 0; i < Glob->params.radar_params.num_gates; i++)
      {
	*byte_ptr = (ui08) ((curr_az % 10) + 1);
	byte_ptr++;
      }
    
    /*
     * range
     */
    
    for (i = 0; i < Glob->params.radar_params.num_gates; i++)
      {
	*byte_ptr = (ui08) ((i % 10) + 1);
	byte_ptr++;
      }

  } /* if (Glob->nfields >= 5) */
  
  curr_az++;
  
  return(new_tilt);
}
