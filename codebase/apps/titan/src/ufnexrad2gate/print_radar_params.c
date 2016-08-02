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

#include "ufnexrad2gate.h"

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

  printf("\n");
  printf("***************************************\n");
  printf("RADAR PARAMETERS\n");
  printf("\n");
  
  printf("radar id:                     %d\n",
	 radar_params->radar_id);
  printf("altitude:                     %d m\n",
	 radar_params->altitude);
  printf("latitude:                     %f degrees\n",
	 (float)radar_params->latitude / 1000000.0);
  printf("longitude:                    %f degrees\n",
	 (float)radar_params->longitude / 1000000.0);
  printf("number of gates:              %d\n",
	 radar_params->ngates);
  printf("gate spacing:                 %d mm\n",
	 radar_params->gate_spacing);
  printf("start range:                  %d mm\n",
	 radar_params->start_range);
  printf("beam width:                   %f degrees\n",
	 (float)radar_params->beam_width / 1000000.0);
  printf("samples per beam:             %d\n",
	 radar_params->samples_per_beam);
  printf("pulse width:                  %d nano-seconds\n",
	 radar_params->pulse_width);
  printf("pulse rep freq:               %f\n",
	 (float)radar_params->prf / 1000.0);
  printf("wavelength:                   %d micro-meters\n",
	 radar_params->wavelength);
  printf("number of fields:             %d\n",
	 radar_params->nfields);
  printf("scan type:                    %d\n",
	 radar_params->scan_type);
  printf("scan mode:                    %d\n",
	 radar_params->scan_mode);
  printf("data field by field?:         %d\n",
	 radar_params->data_field_by_field);
  printf("num fields current:           %d\n",
	 radar_params->nfields_current);
  printf("field flag:                   %d\n",
	 radar_params->field_flag);
  
  /*
   * Print out the field parameters
   */
  
  printf("\n");
  printf("***************************************\n");
  printf("FIELD PARAMETERS\n");
  printf("\n");
  
  for (i = 0; i < radar_params->nfields; i++)
  {
    printf("Information for field %d:\n", i);
    printf("   factor:                    %d\n",
	   field_params[i].factor);
    printf("   scale:                     %d\n",
	   field_params[i].scale);
    printf("   bias:                      %d\n",
	   field_params[i].bias);
    printf("\n");
  }
  
  printf("\n\n");
  
  fflush(stdout);
  
}
