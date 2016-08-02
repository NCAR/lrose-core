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
/*********************************************************************
 *
 * ac_georef.cc
 *
 * Aircraft georeference data
 *
 * Mike Dixon
 *
 * EOL, NCAR, Boulder, Colorado, USA
 *
 * March 2016
 *
 *********************************************************************/

#include <cstdio>
#include <cstring>
#include <dataport/bigend.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaXml.hh>
#include <rapformats/ac_georef.hh>

/********************************************************************
 *
 * BE_from_ac_georef()
 *
 * Gets BE format from ac georef struct
 *
 *********************************************************************/

void BE_from_ac_georef(ac_georef_t *georef)

{
  BE_from_array_64(georef, AC_GEOREF_NBYTES_64);
  BE_from_array_32((char *) georef + AC_GEOREF_NBYTES_64,
                   AC_GEOREF_NBYTES_32);
}

/********************************************************************
 *
 * BE_to_ac_georef()
 *
 * Converts BE format to ac_georef struct
 *
 *********************************************************************/

void BE_to_ac_georef(ac_georef_t *georef)

{
  BE_to_array_64(georef, AC_GEOREF_NBYTES_64);
  BE_to_array_32((char *) georef + AC_GEOREF_NBYTES_64,
                 AC_GEOREF_NBYTES_32);
}

/****************************
 * Initialize ac_georef struct
 */

void ac_georef_init(ac_georef_t *georef)

{

  memset(georef, 0, sizeof(ac_georef_t));

  fl64 missingFl64 = (fl64) AC_GEOREF_MISSING_VAL;
  fl32 missingFl32 = (fl32) AC_GEOREF_MISSING_VAL;

  georef->longitude = missingFl64;
  georef->latitude = missingFl64;
  georef->altitude_msl_km = missingFl32;
  georef->altitude_agl_km = missingFl32;
  georef->altitude_pres_m = missingFl32;
  georef->ew_velocity_mps = missingFl32;
  georef->ns_velocity_mps = missingFl32;
  georef->vert_velocity_mps = missingFl32;
  georef->ew_horiz_wind_mps = missingFl32;
  georef->ns_horiz_wind_mps = missingFl32;
  georef->vert_wind_mps = missingFl32;
  georef->heading_deg = missingFl32;
  georef->drift_angle_deg = missingFl32;
  georef->track_deg = missingFl32;
  georef->roll_deg = missingFl32;
  georef->pitch_deg = missingFl32;
  georef->heading_rate_dps = missingFl32;
  georef->pitch_rate_dps = missingFl32;
  georef->roll_rate_dps = missingFl32;
  georef->drive_angle_1_deg = missingFl32;
  georef->drive_angle_2_deg = missingFl32;
  georef->temp_c = missingFl32;
  georef->pressure_hpa = missingFl32;
  georef->rh_percent = missingFl32;
  georef->density_kg_m3 = missingFl32;
  georef->angle_of_attack_deg = missingFl32;
  georef->ind_airspeed_mps = missingFl32;
  georef->true_airspeed_mps = missingFl32;
  georef->accel_normal = missingFl32;
  georef->accel_latitudinal = missingFl32;
  georef->accel_longitudinal = missingFl32;
  georef->flight_time_secs = missingFl32;

  for (int ii = 0; ii < AC_GEOREF_N_UNUSED; ii++) {
    georef->unused[ii] = missingFl32;
  }
  for (int ii = 0; ii < AC_GEOREF_N_CUSTOM; ii++) {
    georef->custom[ii] = missingFl32;
  }

}

/****************************
 * ac_georef_print
 *
 * Prints out ac_georef struct
 */

void ac_georef_print(FILE *out,
                     const char *spacer,
                     const ac_georef_t *georef)

{
  
  fl64 missingFl64 = (fl64) AC_GEOREF_MISSING_VAL;
  fl32 missingFl32 = (fl32) AC_GEOREF_MISSING_VAL;
  
  fprintf(out, "%s ac_georef_t struct:\n", spacer);
  fprintf(out, "%s callsign: %s\n", spacer, georef->callsign);

  DateTime dtime(georef->time_secs_utc, true, 
                 (double) georef->time_nano_secs / 1.0e9);

  fprintf(out, "%s time: %s\n", spacer, dtime.asString(6).c_str());

  if (georef->longitude != missingFl64) {
    fprintf(out, "%s   longitude: %.6f\n", spacer, georef->longitude);
  }
  if (georef->latitude != missingFl64) {
    fprintf(out, "%s   latitude: %.6f\n", spacer, georef->latitude);
  }

  if (georef->altitude_msl_km != missingFl32) {
    fprintf(out, "%s   altitude_msl_km: %.6f\n", 
            spacer, georef->altitude_msl_km);
  }
  if (georef->altitude_agl_km != missingFl32) {
    fprintf(out, "%s   altitude_agl_km: %.6f\n", 
            spacer, georef->altitude_agl_km);
  }
  if (georef->altitude_pres_m != missingFl32) {
    fprintf(out, "%s   altitude_pres_m: %.6f\n", 
            spacer, georef->altitude_pres_m);
  }
  if (georef->ew_velocity_mps != missingFl32) {
    fprintf(out, "%s   ew_velocity_mps: %.6f\n", 
            spacer, georef->ew_velocity_mps);
  }
  if (georef->ns_velocity_mps != missingFl32) {
    fprintf(out, "%s   ns_velocity_mps: %.6f\n", 
            spacer, georef->ns_velocity_mps);
  }
  if (georef->vert_velocity_mps != missingFl32) {
    fprintf(out, "%s   vert_velocity_mps: %.6f\n", 
            spacer, georef->vert_velocity_mps);
  }
  if (georef->ew_horiz_wind_mps != missingFl32) {
    fprintf(out, "%s   ew_horiz_wind_mps: %.6f\n", 
            spacer, georef->ew_horiz_wind_mps);
  }
  if (georef->ns_horiz_wind_mps != missingFl32) {
    fprintf(out, "%s   ns_horiz_wind_mps: %.6f\n", 
            spacer, georef->ns_horiz_wind_mps);
  }
  if (georef->vert_wind_mps != missingFl32) {
    fprintf(out, "%s   vert_wind_mps: %.6f\n", 
            spacer, georef->vert_wind_mps);
  }
  if (georef->heading_deg != missingFl32) {
    fprintf(out, "%s   heading_deg: %.6f\n", 
            spacer, georef->heading_deg);
  }
  if (georef->drift_angle_deg != missingFl32) {
    fprintf(out, "%s   drift_angle_deg: %.6f\n", 
            spacer, georef->drift_angle_deg);
  }
  if (georef->track_deg != missingFl32) {
    fprintf(out, "%s   track_deg: %.6f\n", 
            spacer, georef->track_deg);
  }
  if (georef->roll_deg != missingFl32) {
    fprintf(out, "%s   roll_deg: %.6f\n", 
            spacer, georef->roll_deg);
  }
  if (georef->pitch_deg != missingFl32) {
    fprintf(out, "%s   pitch_deg: %.6f\n", 
            spacer, georef->pitch_deg);
  }
  if (georef->heading_rate_dps != missingFl32) {
    fprintf(out, "%s   heading_rate_dps: %.6f\n", 
            spacer, georef->heading_rate_dps);
  }
  if (georef->pitch_rate_dps != missingFl32) {
    fprintf(out, "%s   pitch_rate_dps: %.6f\n", 
            spacer, georef->pitch_rate_dps);
  }
  if (georef->roll_rate_dps != missingFl32) {
    fprintf(out, "%s   roll_rate_dps: %.6f\n", 
            spacer, georef->roll_rate_dps);
  }
  if (georef->drive_angle_1_deg != missingFl32) {
    fprintf(out, "%s   drive_angle_1_deg: %.6f\n", 
            spacer, georef->drive_angle_1_deg);
  }
  if (georef->drive_angle_2_deg != missingFl32) {
    fprintf(out, "%s   drive_angle_2_deg: %.6f\n", 
            spacer, georef->drive_angle_2_deg);
  }
  if (georef->temp_c != missingFl32) {
    fprintf(out, "%s   temp_c: %.6f\n", 
            spacer, georef->temp_c);
  }
  if (georef->pressure_hpa != missingFl32) {
    fprintf(out, "%s   pressure_hpa: %.6f\n", 
            spacer, georef->pressure_hpa);
  }
  if (georef->rh_percent != missingFl32) {
    fprintf(out, "%s   rh_percent: %.6f\n", 
            spacer, georef->rh_percent);
  }
  if (georef->density_kg_m3 != missingFl32) {
    fprintf(out, "%s   density_kg_m3: %.6f\n", 
            spacer, georef->density_kg_m3);
  }
  if (georef->angle_of_attack_deg != missingFl32) {
    fprintf(out, "%s   angle_of_attack_deg: %.6f\n", 
            spacer, georef->angle_of_attack_deg);
  }
  if (georef->ind_airspeed_mps != missingFl32) {
    fprintf(out, "%s   ind_airspeed_mps: %.6f\n", 
            spacer, georef->ind_airspeed_mps);
  }
  if (georef->true_airspeed_mps != missingFl32) {
    fprintf(out, "%s   true_airspeed_mps: %.6f\n", 
            spacer, georef->true_airspeed_mps);
  }
  if (georef->accel_normal != missingFl32) {
    fprintf(out, "%s   accel_normal: %.6f\n", 
            spacer, georef->accel_normal);
  }
  if (georef->accel_latitudinal != missingFl32) {
    fprintf(out, "%s   accel_latitudinal: %.6f\n", 
            spacer, georef->accel_latitudinal);
  }
  if (georef->accel_longitudinal != missingFl32) {
    fprintf(out, "%s   accel_longitudinal: %.6f\n", 
            spacer, georef->accel_longitudinal);
  }
  if (georef->flight_time_secs != missingFl32) {
    fprintf(out, "%s   flight_time_secs: %.6f\n", 
            spacer, georef->flight_time_secs);
  }

  for (int ii = 0; ii < AC_GEOREF_N_UNUSED; ii++) {
    if (georef->unused[ii] != missingFl32) {
      fprintf(out, "%s   unused[%d]: %.6f\n", 
              spacer, ii, georef->unused[ii]);
    }
  }

  for (int ii = 0; ii < AC_GEOREF_N_CUSTOM; ii++) {
    if (georef->custom[ii] != missingFl32) {
      fprintf(out, "%s   custom[%d]: %.6f\n", 
              spacer, ii, georef->custom[ii]);
    }
  }

}

///////////////////////////////////////////
// load XML string
// returns XML string

string ac_georef_load_as_xml(const ac_georef_t *georef,
                             int startIndentLevel /* = 0 */)
  
{
 
  int sil = startIndentLevel;

  // print object to string as XML
  
  string xml;
  
  xml += TaXml::writeStartTag("AcGeoref", sil+0);
  xml += TaXml::writeString("callsign", sil+1, georef->callsign);

  DateTime dtime(georef->time_secs_utc, true, 
                 (double) georef->time_nano_secs / 1.0e9);
  xml += TaXml::writeString("time", sil+1, dtime.asString(9));

  fl64 missingFl64 = (fl64) AC_GEOREF_MISSING_VAL;
  fl32 missingFl32 = (fl32) AC_GEOREF_MISSING_VAL;

  if (georef->longitude != missingFl64) {
    xml += TaXml::writeDouble("longitude", sil+1, georef->longitude, "%.9f");
  }
  if (georef->latitude != missingFl64) {
    xml += TaXml::writeDouble("latitude", sil+1, georef->latitude, "%.9f");
  }

  if (georef->altitude_msl_km != missingFl32) {
    xml += TaXml::writeDouble("altitude_msl_km", sil+1, georef->altitude_msl_km, "%.6f");
  }
  if (georef->altitude_agl_km != missingFl32) {
    xml += TaXml::writeDouble("altitude_agl_km", sil+1, georef->altitude_agl_km, "%.6f");
  }
  if (georef->altitude_pres_m != missingFl32) {
    xml += TaXml::writeDouble("altitude_pres_m", sil+1, georef->altitude_pres_m, "%.6f");
  }
  if (georef->ew_velocity_mps != missingFl32) {
    xml += TaXml::writeDouble("ew_velocity_mps", sil+1, georef->ew_velocity_mps, "%.6f");
  }
  if (georef->ns_velocity_mps != missingFl32) {
    xml += TaXml::writeDouble("ns_velocity_mps", sil+1, georef->ns_velocity_mps, "%.6f");
  }
  if (georef->vert_velocity_mps != missingFl32) {
    xml += TaXml::writeDouble("vert_velocity_mps", sil+1, georef->vert_velocity_mps, "%.6f");
  }
  if (georef->ew_horiz_wind_mps != missingFl32) {
    xml += TaXml::writeDouble("ew_horiz_wind_mps", sil+1, georef->ew_horiz_wind_mps, "%.6f");
  }
  if (georef->ns_horiz_wind_mps != missingFl32) {
    xml += TaXml::writeDouble("ns_horiz_wind_mps", sil+1, georef->ns_horiz_wind_mps, "%.6f");
  }
  if (georef->vert_wind_mps != missingFl32) {
    xml += TaXml::writeDouble("vert_wind_mps", sil+1, georef->vert_wind_mps, "%.6f");
  }
  if (georef->heading_deg != missingFl32) {
    xml += TaXml::writeDouble("heading_deg", sil+1, georef->heading_deg, "%.6f");
  }
  if (georef->drift_angle_deg != missingFl32) {
    xml += TaXml::writeDouble("drift_angle_deg", sil+1, georef->drift_angle_deg, "%.6f");
  }
  if (georef->track_deg != missingFl32) {
    xml += TaXml::writeDouble("track_deg", sil+1, georef->track_deg, "%.6f");
  }
  if (georef->roll_deg != missingFl32) {
    xml += TaXml::writeDouble("roll_deg", sil+1, georef->roll_deg, "%.6f");
  }
  if (georef->pitch_deg != missingFl32) {
    xml += TaXml::writeDouble("pitch_deg", sil+1, georef->pitch_deg, "%.6f");
  }
  if (georef->heading_rate_dps != missingFl32) {
    xml += TaXml::writeDouble("heading_rate_dps", sil+1, georef->heading_rate_dps, "%.6f");
  }
  if (georef->pitch_rate_dps != missingFl32) {
    xml += TaXml::writeDouble("pitch_rate_dps", sil+1, georef->pitch_rate_dps, "%.6f");
  }
  if (georef->roll_rate_dps != missingFl32) {
    xml += TaXml::writeDouble("roll_rate_dps", sil+1, georef->roll_rate_dps, "%.6f");
  }
  if (georef->drive_angle_1_deg != missingFl32) {
    xml += TaXml::writeDouble("drive_angle_1_deg", sil+1, georef->drive_angle_1_deg, "%.6f");
  }
  if (georef->drive_angle_2_deg != missingFl32) {
    xml += TaXml::writeDouble("drive_angle_2_deg", sil+1, georef->drive_angle_2_deg, "%.6f");
  }
  if (georef->temp_c != missingFl32) {
    xml += TaXml::writeDouble("temp_c", sil+1, georef->temp_c, "%.6f");
  }
  if (georef->pressure_hpa != missingFl32) {
    xml += TaXml::writeDouble("pressure_hpa", sil+1, georef->pressure_hpa, "%.6f");
  }
  if (georef->rh_percent != missingFl32) {
    xml += TaXml::writeDouble("rh_percent", sil+1, georef->rh_percent, "%.6f");
  }
  if (georef->density_kg_m3 != missingFl32) {
    xml += TaXml::writeDouble("density_kg_m3", sil+1, georef->density_kg_m3, "%.6f");
  }
  if (georef->angle_of_attack_deg != missingFl32) {
    xml += TaXml::writeDouble("angle_of_attack_deg", sil+1, georef->angle_of_attack_deg, "%.6f");
  }
  if (georef->ind_airspeed_mps != missingFl32) {
    xml += TaXml::writeDouble("ind_airspeed_mps", sil+1, georef->ind_airspeed_mps, "%.6f");
  }
  if (georef->true_airspeed_mps != missingFl32) {
    xml += TaXml::writeDouble("true_airspeed_mps", sil+1, georef->true_airspeed_mps, "%.6f");
  }
  if (georef->accel_normal != missingFl32) {
    xml += TaXml::writeDouble("accel_normal", sil+1, georef->accel_normal, "%.6f");
  }
  if (georef->accel_latitudinal != missingFl32) {
    xml += TaXml::writeDouble("accel_latitudinal", sil+1, georef->accel_latitudinal, "%.6f");
  }
  if (georef->accel_longitudinal != missingFl32) {
    xml += TaXml::writeDouble("accel_longitudinal", sil+1, georef->accel_longitudinal, "%.6f");
  }
  if (georef->flight_time_secs != missingFl32) {
    xml += TaXml::writeDouble("flight_time_secs", sil+1, georef->flight_time_secs, "%.6f");
  }
  
  for (int ii = 0; ii < AC_GEOREF_N_UNUSED; ii++) {
    if (georef->unused[ii] != missingFl32) {
      char label[128];
      sprintf(label, "unused[%d]", ii);
      xml += TaXml::writeDouble(label, sil+1, georef->unused[ii], "%.6f");
    }
  }

  for (int ii = 0; ii < AC_GEOREF_N_CUSTOM; ii++) {
    if (georef->custom[ii] != missingFl32) {
      char label[128];
      sprintf(label, "custom[%d]", ii);
      xml += TaXml::writeDouble(label, sil+1, georef->custom[ii], "%.6f");
    }
  }

  xml += TaXml::writeEndTag("AcGeoref", sil+0);

  return xml;

}

///////////////////////////////////////////
// Print as XML string
  
void ac_georef_print_as_xml(const ac_georef_t *georef,
                            ostream &out, int startIndentLevel /* = 0 */)

{
  
  string xml = ac_georef_load_as_xml(georef, startIndentLevel);
  out << xml;

}

 
