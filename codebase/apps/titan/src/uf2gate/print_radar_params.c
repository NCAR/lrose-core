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
 * print_radar_params.c
 *
 * prints details of a radar parameter buffer to stdout
 *
 * Nancy Rehak RAP NCAR Boulder CO USA
 *
 * Mar 1995
 *
 **************************************************************************/

#include "uf2gate.h"

void print_radar_params (ui08 *param_data)

{
  int i;
  
  gate_data_radar_params_t  *radar_params;
  gate_data_field_params_t  *field_params;
  
  /*
   * Position the header pointers within the buffer.
   */

  radar_params = (gate_data_radar_params_t *)param_data;
  field_params = (gate_data_field_params_t *)
    (param_data + sizeof(gate_data_radar_params_t));
  
  /*
   * Print out the radar parameter information.
   */

  fprintf(stdout, "\n");
  fprintf(stdout, "***************************************\n");
  fprintf(stdout, "RADAR PARAMETERS\n");
  fprintf(stdout, "\n");
  
  fprintf(stdout, "radar id:                     %d\n",
	 radar_params->radar_id);
  fprintf(stdout, "altitude:                     %d m\n",
	 radar_params->altitude);
  fprintf(stdout, "latitude:                     %f degrees\n",
	 (float)radar_params->latitude / 1000000.0);
  fprintf(stdout, "longitude:                    %f degrees\n",
	 (float)radar_params->longitude / 1000000.0);
  fprintf(stdout, "number of gates:              %d\n",
	 radar_params->ngates);
  fprintf(stdout, "gate spacing:                 %d mm\n",
	 radar_params->gate_spacing);
  fprintf(stdout, "start range:                  %d mm\n",
	 radar_params->start_range);
  fprintf(stdout, "beam width:                   %f degrees\n",
	 (float)radar_params->beam_width / 1000000.0);
  fprintf(stdout, "samples per beam:             %d\n",
	 radar_params->samples_per_beam);
  fprintf(stdout, "pulse width:                  %d nano-seconds\n",
	 radar_params->pulse_width);
  fprintf(stdout, "pulse rep freq:               %f\n",
	 (float)radar_params->prf / 1000.0);
  fprintf(stdout, "wavelength:                   %d micro-meters\n",
	 radar_params->wavelength);
  fprintf(stdout, "number of fields:             %d\n",
	 radar_params->nfields);
  fprintf(stdout, "scan type:                    %d\n",
	 radar_params->scan_type);
  fprintf(stdout, "scan mode:                    %d\n",
	 radar_params->scan_mode);
  fprintf(stdout, "data field by field?:         %d\n",
	 radar_params->data_field_by_field);
  fprintf(stdout, "num fields current:           %d\n",
	 radar_params->nfields_current);
  fprintf(stdout, "field flag:                   %d\n",
	 radar_params->field_flag);
  
  /*
   * Print out the field parameters
   */
  
  fprintf(stdout, "\n");
  fprintf(stdout, "***************************************\n");
  fprintf(stdout, "FIELD PARAMETERS\n");
  fprintf(stdout, "\n");
  
  for (i = 0; i < radar_params->nfields; i++)
  {
    fprintf(stdout, "Information for field %d:\n", i);
    fprintf(stdout, "   factor:                    %d\n",
	   field_params[i].factor);
    fprintf(stdout, "   scale:                     %d\n",
	   field_params[i].scale);
    fprintf(stdout, "   bias:                      %d\n",
	   field_params[i].bias);
    fprintf(stdout, "\n");
  }
  
  fprintf(stdout, "\n\n");
  
  fflush(stdout);
  
}
