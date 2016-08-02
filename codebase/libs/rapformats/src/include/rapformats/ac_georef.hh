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
/**********************************************************************
 * ac_georef.hh
 *
 * structs and defines for aircraft georeference struct
 *
 * Mike Dixon, EOL, NCAR, Boulder, CO
 *
 * March 2016
 */

#ifndef ac_georef_hh
#define ac_georef_hh

#include <dataport/port_types.h>
#include <string>
#include <cmath>

#define AC_GEOREF_NBYTES_64 32
#define AC_GEOREF_NBYTES_32 200
#define AC_GEOREF_NBYTES_CALLSIGN 24
#define AC_GEOREF_N_UNUSED 10
#define AC_GEOREF_N_CUSTOM 10
#define AC_GEOREF_MISSING_VAL -9999.0

typedef struct ac_georef {
  
  si64 time_secs_utc;       /**< UTC time in secs since Jan 1 1970 */
  si64 time_nano_secs;      /**< partial secs - nanosecs */

  fl64 longitude;           /**< Antenna longitude (Eastern
                             * Hemisphere is positive, West
                             * negative) in degrees */
  fl64 latitude;            /**< Antenna Latitude (Northern
                             * Hemisphere is positive, South
                             * Negative) in degrees */
  
  fl32 altitude_msl_km;     /**< Antenna Altitude above mean sea
                             * level (MSL) in km */
  fl32 altitude_agl_km;     /**< Antenna Altitude above ground level
                             * (AGL) in km */
  
  fl32 altitude_pres_m;     /**< pressure altitude (m) */

  fl32 ew_velocity_mps;     /**< Antenna east-west ground speed
                             * (towards East is positive) in m/sec */
  fl32 ns_velocity_mps;     /**< Antenna north-south ground speed
                             * (towards North is positive) in m/sec */
  fl32 vert_velocity_mps;   /**< Antenna vertical velocity (Up is
                             * positive) in m/sec */
  
  fl32 ew_horiz_wind_mps;   /**< east - west wind velocity at the
                             * platform (towards East is positive)
                             * in m/sec */
  fl32 ns_horiz_wind_mps;   /**< North - South wind velocity at the
                             * platform (towards North is 
                             * positive) in m/sec */
  fl32 vert_wind_mps;       /**< Vertical wind velocity at the
                             * platform (up is positive) in m/sec */
  
  fl32 heading_deg;         /**< Antenna heading (angle between
                             * rotodome rotational axis and true
                             * North, clockwise (looking down)
                             * positive) in degrees */
  
  fl32 drift_angle_deg;     /**< Antenna drift Angle. (angle between
                             * platform true velocity and heading,
                             * positive is drift more clockwise
                             * looking down) in degrees */
  fl32 track_deg;           /** track over the ground */
  
  fl32 roll_deg;            /**< Roll angle of aircraft tail section
                             * (Horizontal zero, Positive left wing up)
                             * in degrees */
  fl32 pitch_deg;           /**< Pitch angle of platform (Horizontal
                             * is zero positive front up) in degrees*/
  
  fl32 heading_rate_dps;    /**< Heading change rate in degrees/second. */
  fl32 pitch_rate_dps;      /**< Pitch change rate in degrees/second. */
  fl32 roll_rate_dps;       /**< roll angle rate in degrees/second. */
  
  fl32 drive_angle_1_deg;   /**< angle of motor drive 1 (degrees) */
  fl32 drive_angle_2_deg;   /**< angle of motor drive 2 (degrees) */
  
  fl32 temp_c;              /**< Ambient air temperature - deg C */
  fl32 pressure_hpa;        /**< Ambient air pressure - Hpa */
  fl32 rh_percent;          /**< Ambient air relative humidity (%) */
  fl32 density_kg_m3;       /**< Ambient air density (kg/m3) */

  fl32 angle_of_attack_deg; /**< aircraft angle of attack (deg) */
  fl32 ind_airspeed_mps;    /**< aircraft indicated airpseed (m/s) */
  fl32 true_airspeed_mps;   /**< aircraft true airpseed (m/s) */

  fl32 accel_normal;        /**< acceleration normal to platform (G) */
  fl32 accel_latitudinal;   /**< acceleration latitudinal (G) */
  fl32 accel_longitudinal;  /**< acceleration longitudinal (G) */

  fl32 flight_time_secs;    /**< flight time so far (secs) */

  fl32 unused[AC_GEOREF_N_UNUSED]; /**< for future expansion */

  fl32 custom[AC_GEOREF_N_CUSTOM]; /**< for custom fields */
  
  char callsign[AC_GEOREF_NBYTES_CALLSIGN];
  
} ac_georef_t;

/*
 * prototypes
 */

///////////////////////////////////////////
// load XML string
// returns XML string

extern string ac_georef_load_as_xml(const ac_georef_t *georef,
                                    int startIndentLevel = 0);

///////////////
// print as XML

extern void ac_georef_print_as_xml(const ac_georef_t *georef,
                                   ostream &out, int startIndentLevel = 0);

extern void BE_from_ac_georef(ac_georef_t *georef);

extern void BE_to_ac_georef(ac_georef_t *georef);

extern void ac_georef_init(ac_georef_t *georef);

extern void ac_georef_print(FILE *out,
                            const char *spacer,
                            const ac_georef_t *georef);

#endif

