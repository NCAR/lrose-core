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
 * create_param_buffer.c
 *
 * creates the test radar parameter buffer
 *
 * Nancy Rehak RAP NCAR Boulder CO USA
 *
 * September 1995
 *
 **************************************************************************/

#include "test2gate.h"

ui08 *create_param_buffer(int *buffer_size)

{
  gate_data_radar_params_t *param_buffer;
  gate_data_field_params_t *field_params;
  
  vol_file_handle_t *v_handle;
  field_params_t *dbz_params;
  field_params_t *vel_params;
  
  *buffer_size = sizeof(gate_data_radar_params_t) +
    Glob->nfields * sizeof(gate_data_field_params_t);
  
  param_buffer = (gate_data_radar_params_t *)umalloc((ui32)(*buffer_size));
  
  param_buffer->radar_id = Glob->params.radar_params.radar_id;
  param_buffer->altitude = Glob->params.radar_params.altitude;
  param_buffer->latitude =
    (si32)(Glob->params.radar_params.latitude * 1000000.0 + 0.5);
  param_buffer->longitude =
    (si32)(Glob->params.radar_params.longitude * 1000000.0 + 0.5);
  param_buffer->ngates = Glob->params.radar_params.num_gates;
  param_buffer->gate_spacing =
    (si32)(Glob->params.radar_params.gate_spacing * 1000.0 + 0.5);
  param_buffer->start_range =
    (si32)(Glob->params.radar_params.start_range * 1000.0 + 0.5);
  param_buffer->beam_width =
    (si32)(Glob->params.radar_params.beam_width * 1000000.0 + 0.5);
  param_buffer->samples_per_beam = Glob->params.radar_params.samples_per_beam;
  param_buffer->pulse_width = Glob->params.radar_params.pulse_width;
  param_buffer->prf =
    (si32)(Glob->params.radar_params.prf * 1000.0 + 0.5);
  param_buffer->wavelength =
    (si32)(Glob->params.radar_params.wavelength * 10000.0 + 0.5);
  param_buffer->nfields = Glob->nfields;
  param_buffer->scan_type = -1;
  param_buffer->scan_mode = GATE_DATA_SURVEILLANCE_MODE;
  param_buffer->data_field_by_field = TRUE;
  param_buffer->nfields_current = Glob->nfields;
  param_buffer->field_flag = 3;
  
  BE_from_gate_data_radar_params(param_buffer);

  field_params =
    (gate_data_field_params_t *)
      ((ui08 *)param_buffer + sizeof(gate_data_radar_params_t));

  v_handle = get_sampling_vol_index();
  dbz_params = v_handle->field_params[Glob->params.sample_dbz_field];
  vel_params = v_handle->field_params[Glob->params.sample_vel_field];

  field_params[0].factor = dbz_params->factor;
  field_params[0].scale = dbz_params->scale;
  field_params[0].bias = dbz_params->bias;
  BE_from_gate_data_field_params(field_params + 0);

  if (Glob->params.output_vel_field) {
    field_params[1].factor = vel_params->factor;
    field_params[1].scale = vel_params->scale;
    field_params[1].bias = vel_params->bias;
    BE_from_gate_data_field_params(field_params + 1);

  }

  if (Glob->params.output_geom_fields) {

    field_params[2].factor = 1000;
    field_params[2].scale = 1000;
    field_params[2].bias = 0;
    BE_from_gate_data_field_params(field_params + 2);
    
    field_params[3].factor = 1000;
    field_params[3].scale = 1000;
    field_params[3].bias = 0;
    BE_from_gate_data_field_params(field_params + 3);

    field_params[4].factor = 1000;
    field_params[4].scale = 1000;
    field_params[4].bias = 0;
    BE_from_gate_data_field_params(field_params + 4);

  }
  
  return((ui08 *)param_buffer);
}
