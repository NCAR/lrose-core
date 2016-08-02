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
#ifdef __cplusplus
extern "C" {
#endif
  /**********************************************************************
   * gate_data.h
   *
   * structs and defines for sending radar gate data via TCP/IP
   *
   * Mike Dixon, RAP, NCAR, Boulder, CO, Sept 1992
   *
   *
   * A params packet should be sent at the start of each tilt, and
   * whenever any of the parameters change.
   *
   * The params packet consists of:
   *
   *    8 bytes of HEX f0f0f0f0f0f0f0f0	
   *    SKU_header_t - packet header (sockutil.h)
   *                   id = GATE_PARAMS_PACKET_CODE
   *                   len = sizeof(gate_data_radar_params_t) +
   *                         nfields * sizeof(gate_data_field_params_t) 
   *
   *    gate_data_radar_params_t - radar parameters
   *    nfields * gate_data_field_params_t - field parameters
   *
   * The data for each beam is sent in a packet consisting of:
   *
   *    8 bytes of HEX f0f0f0f0f0f0f0f0	
   *    SKU_header_t - packet header (sockutil.h)
   *                   id set to GATE_DATA_PACKET_CODE
   *    gate_data_beam_header_t - data header
   *    nfields * (ngates * 1 byte per gate) - field data
   */

#ifndef gate_data_h
#define gate_data_h

#include <dataport/port_types.h>

#define GATE_PARAMS_PACKET_CODE 0
#define GATE_DATA_PACKET_CODE 1

#define GATE_DATA_UNKNOWN_MODE -1
#define GATE_DATA_SECTOR_MODE 1
#define GATE_DATA_RHI_MODE 3
#define GATE_DATA_SURVEILLANCE_MODE 8

  typedef struct {

    si32 radar_id;		/* unique number */
    si32 altitude;		/* meters */
    si32 latitude;		/* degrees * 1000000 */
    si32 longitude;		/* degrees * 1000000 */
    si32 ngates;
    si32 gate_spacing;		/* millimeters */
    si32 start_range;		/* millimeters */
    si32 beam_width;		/* degrees * 1000000 */
    si32 samples_per_beam;
    si32 pulse_width;		/* nano-seconds */
    si32 prf;			/* pulse repitition freq. * 1000 */
    si32 wavelength;		/* micro-meters */
    si32 nfields;
  
    si32 scan_type;		/* the current scan strategy -
				 * not yet implemented */
  
    si32 scan_mode;		/* GATE_DATA_SURVEILLANCE_MODE or
				   GATE_DATE_SECTOR_MODE or
				   GATE_DATE_RHI_MODE or
				   GATE_DATA_UNKNOWN_MODE */
  
    si32 data_field_by_field;	/* TRUE (1) if radar field data is stored
				 * as contiguous blocks, FALSE (0) if each
				 * field for a given gate is stored
				 * consecutively followed by the fields
				 * for the next gate etc. */
  
    si32 nfields_current;	/* the number of fields currently being
				 * sent - the positions of the fields
				 * are indicated by the
				 * bits set in the field_flag */
  
    si32 field_flag;		/* for each field included in the beam data,
				 * the relevant bit is set in this long.
				 * For example, suppose there are a total
				 * of 6 fields referred to in the params
				 * struct, and only fields
				 * 0, 1, 3, and 5 are currently in
				 * the data stream.
				 * Then, field_flag = 00.....0101011 */

  } gate_data_radar_params_t;

  typedef struct {

    si32 factor;		/* scale and bias values are multiplied by this
				 * factor before being stored */

    si32 scale;			/* gain of the data */
    si32 bias;			/* offset of zero value */

  } gate_data_field_params_t;

  /*
   * struct for header for gate data packet
   */

  typedef struct {

    si32 time;			/* secs since Jan 1 1970 */
    si32 azimuth;		/* deg * 1000000 */
    si32 elevation;		/* deg * 1000000 */
    si32 target_elev;		/* deg * 1000000 */
    si32 vol_num;		/* the volume scan number */
    si32 tilt_num;		/* the tilt number in the volume scan */

    si32 new_scan_limits;	/* TRUE (1) if scan limits have
				 * changed, FALSE (0) otherwise */

    si32 end_of_tilt;		/* flag - TRUE(1) or FALSE (0) - not essential
				 * set to (-1)  if not operational */

    si32 end_of_volume;		/* flag - TRUE(1) or FALSE (0) - not essential
				 * set to (-1)  if not operational */

  } gate_data_beam_header_t;

#endif
#ifdef __cplusplus
}
#endif
