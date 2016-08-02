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

#include "test2gate.h"

void print_radar_params (ui08 *param_data, FILE *out)
{
  int i;
  
  gate_data_radar_params_t  radar_params, *rparams;
  gate_data_field_params_t  field_params, *fparams;
  
  /*
   * Position the header pointers within the buffer.
   */

  rparams = (gate_data_radar_params_t *)param_data;
  radar_params = *rparams;

  /*
   * Print out the radar parameter information.
   */

  BE_to_gate_data_radar_params(&radar_params);

  fprintf(out, "\n");
  fprintf(out, "***************************************\n");
  fprintf(out, "RADAR PARAMETERS\n");
  fprintf(out, "\n");
  
  fprintf(out, "radar id:                     %d\n",
	 radar_params.radar_id);
  fprintf(out, "altitude:                     %d m\n",
	 radar_params.altitude);
  fprintf(out, "latitude:                     %f degrees\n",
	 (float)radar_params.latitude / 1000000.0);
  fprintf(out, "longitude:                    %f degrees\n",
	 (float)radar_params.longitude / 1000000.0);
  fprintf(out, "number of gates:              %d\n",
	 radar_params.ngates);
  fprintf(out, "gate spacing:                 %d mm\n",
	 radar_params.gate_spacing);
  fprintf(out, "start range:                  %d mm\n",
	 radar_params.start_range);
  fprintf(out, "beam width:                   %f degrees\n",
	 (float)radar_params.beam_width / 1000000.0);
  fprintf(out, "samples per beam:             %d\n",
	 radar_params.samples_per_beam);
  fprintf(out, "pulse width:                  %d nano-seconds\n",
	 radar_params.pulse_width);
  fprintf(out, "pulse rep freq:               %f\n",
	 (float)radar_params.prf / 1000.0);
  fprintf(out, "wavelength:                   %d micro-meters\n",
	 radar_params.wavelength);
  fprintf(out, "number of fields:             %d\n",
	 radar_params.nfields);
  fprintf(out, "scan type:                    %d\n",
	 radar_params.scan_type);
  fprintf(out, "scan mode:                    %d\n",
	 radar_params.scan_mode);
  fprintf(out, "data field by field?:         %d\n",
	 radar_params.data_field_by_field);
  fprintf(out, "num fields current:           %d\n",
	 radar_params.nfields_current);
  fprintf(out, "field flag:                   %d\n",
	 radar_params.field_flag);
  
  /*
   * Print out the field parameters
   */
  
  fparams = (gate_data_field_params_t *)
    (param_data + sizeof(gate_data_radar_params_t));
  
  fprintf(out, "\n");
  fprintf(out, "***************************************\n");
  fprintf(out, "FIELD PARAMETERS\n");
  fprintf(out, "\n");
  
  for (i = 0; i < radar_params.nfields; i++)
  {
    field_params = fparams[i];
    BE_to_gate_data_field_params(&field_params);
    fprintf(out, "Information for field %d:\n", i);
    fprintf(out, "   factor:                    %d\n",
	   field_params.factor);
    fprintf(out, "   scale:                     %d\n",
	   field_params.scale);
    fprintf(out, "   bias:                      %d\n",
	   field_params.bias);
    fprintf(out, "\n");
  }
  
  fprintf(out, "\n\n");
  
  fflush(out);
  
}
