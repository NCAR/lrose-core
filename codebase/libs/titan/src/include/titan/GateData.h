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
/*************************************************************************
 *
 * GateData.h
 *
 * Gate format for radar data
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * March 1997
 *
 **************************************************************************/

#include <rapformats/gate_data.h>

/*************************************************************************
 *
 * BE_from_gate_data_radar_params()
 *
 * Gets BE format from gate_data_radar_params struct
 *
 **************************************************************************/

extern void BE_from_gate_data_radar_params(gate_data_radar_params_t *params);

/*************************************************************************
 *
 * BE_to_gate_data_radar_params()
 *
 * Puts BE format to gate_data_radar_params struct
 *
 **************************************************************************/

extern void BE_to_gate_data_radar_params(gate_data_radar_params_t *params);

/*************************************************************************
 *
 * BE_from_gate_data_field_params()
 *
 * Gets BE format from gate_data_field_params struct
 *
 **************************************************************************/

extern void BE_from_gate_data_field_params(gate_data_field_params_t *params);

/*************************************************************************
 *
 * BE_to_gate_data_field_params()
 *
 * Puts BE format to gate_data_field_params struct
 *
 **************************************************************************/

extern void BE_to_gate_data_field_params(gate_data_field_params_t *params);

/*************************************************************************
 *
 * BE_from_gate_data_beam_header()
 *
 * Gets BE format from gate_data_beam_header struct
 *
 **************************************************************************/

extern void BE_from_gate_data_beam_header(gate_data_beam_header_t *header);

/*************************************************************************
 *
 * BE_to_gate_data_beam_header()
 *
 * Puts BE format to gate_data_beam_header struct
 *
 **************************************************************************/

extern void BE_to_gate_data_beam_header(gate_data_beam_header_t *header);


