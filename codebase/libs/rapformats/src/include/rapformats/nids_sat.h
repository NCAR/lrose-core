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
#ifndef nids_sat_h
#define nids_sat_h

#ifdef __cplusplus
 extern "C" {
#endif
/******************************************************************
 * nids_sast.h
 *
 * Structures and defines for NIDS Satellite Attribute Table
 * information.
 *
 * TIME FORMAT
 *
 * All times are passed as longs, and are to be interpreted as the number of
 * seconds since Jan 1 1970 GMT. (see time(3V))
 *
 ***********************************************************************/
   
#include <dataport/port_types.h>

#define MAX_NDA_STORMS 20

#define NDA_LAT_LON_SCALE  1000.0
#define NDA_HEIGHT_SCALE    100.0
#define NDA_STORM_TOP_SCALE  10.0


typedef struct {
  char site[4];
  long radar_lat;
  long radar_lon;
  short radar_ht;
  short vol_scan;
  unsigned long vol_scan_time;
  short vol_scan_date;
  char reserved[8];
  short nstorms;
} nda_header_t;

typedef struct {
  char storm_id[3];
  char tvs;
  char meso;
  char hail;
  short azimuth;
  short range;
  short max_dbz;
  short height_of_max_dbz;
  short vlow;
  short storm_top;
  short fcast_azimuth;
  short fcast_speed;
  unsigned short mw_vol;
} nda_storm_t;

typedef struct {
  nda_header_t header;
  nda_storm_t storms[MAX_NDA_STORMS];
} nda_data_t;



#ifdef __cplusplus
}
#endif

#endif

