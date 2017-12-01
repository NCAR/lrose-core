// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
////////////////////////////////////////////////////////////////////////////////
//
//  Mike Dixon, EOL, NCAR, Boulder, CO, 80307, USA
//  Jan 2013
//
// Based on IWRF message structs - will be merged into IWRF moments later
//
////////////////////////////////////////////////////////////////////////////////

#include <toolsa/umisc.h>
#include <toolsa/DateTime.hh>
#include <dataport/bigend.h>
#include <rapformats/DsPlatformGeoref.hh>
#include <iostream>
using namespace std;

#define BOOL_STR(x) (x? "TRUE" : "FALSE")

DsPlatformGeoref::DsPlatformGeoref()
{
  clear();
}

DsPlatformGeoref::~DsPlatformGeoref()
{

}

DsPlatformGeoref&
  DsPlatformGeoref::operator=(const DsPlatformGeoref &rhs)
{
  copy(rhs);
  return(*this);
}

void
  DsPlatformGeoref::copy(const DsPlatformGeoref& rhs)
{
  
  if (this == &rhs)
    return;

  _georef = rhs._georef;

}

bool
  DsPlatformGeoref::operator==(const DsPlatformGeoref& rhs) const
{
  if (memcmp(&_georef, &rhs._georef, sizeof(_georef))) {
    return false;
  }
  return true;
}

void
  DsPlatformGeoref::clear()
{
   
  MEM_zero(_georef);
  _georef.packet.id = DS_IWRF_PLATFORM_GEOREF_ID;
  _georef.packet.len_bytes = sizeof(_georef);
  _georef.packet.version_num = 1;
  struct timeval time;
  gettimeofday(&time, NULL);
  _georef.packet.time_secs_utc = time.tv_sec;
  _georef.packet.time_nano_secs = time.tv_usec * 1000;

}

//////////////////////////////////////////////////////
// Convert to BE

void DsPlatformGeoref::toBe(ds_iwrf_platform_georef_t &val)
{
  BE_from_array_32(&val.packet.id, 2 * sizeof(si32));
  BE_from_array_64(&val.packet.seq_num, sizeof(si64));
  BE_from_array_32(&val.packet.version_num, 2 * sizeof(si32));
  BE_from_array_64(&val.packet.time_secs_utc, sizeof(si64));
  BE_from_array_32(&val.packet.time_nano_secs, 6 * sizeof(si32));
  BE_from_array_32(&val.unit_num, 20 * sizeof(fl32));
  BE_from_array_64(&val.longitude, 2 * sizeof(fl64));
  BE_from_array_32(&val.track_deg, 2 * sizeof(fl32));
}

//////////////////////////////////////////////////////
// Convert from BE

void DsPlatformGeoref::fromBe(ds_iwrf_platform_georef_t &val)
{
  BE_to_array_32(&val.packet.id, 2 * sizeof(si32));
  BE_to_array_64(&val.packet.seq_num, sizeof(si64));
  BE_to_array_32(&val.packet.version_num, 2 * sizeof(si32));
  BE_to_array_64(&val.packet.time_secs_utc, sizeof(si64));
  BE_to_array_32(&val.packet.time_nano_secs, 6 * sizeof(si32));
  BE_to_array_32(&val.unit_num, 20 * sizeof(fl32));
  BE_to_array_64(&val.longitude, 2 * sizeof(fl64));
  BE_to_array_32(&val.track_deg, 2 * sizeof(fl32));
}

//////////////////////////////////////////////////////
// Encode/decode to/from BE for message passing

void
  DsPlatformGeoref::decode(const ds_iwrf_platform_georef_t &val)

{
  _georef = val;
  fromBe(_georef);
}

void
  DsPlatformGeoref::encode(ds_iwrf_platform_georef_t &val)
{
  val = _georef;
  toBe(val);
}

//////////////////////////////////////////////////////
// print platform_georef

void
  DsPlatformGeoref::print(FILE *out) const
{
   
  fprintf(out, "PLATFORM GEOREF\n");
   
  fprintf(out, "  id: 0x%x (%d)\n", _georef.packet.id, _georef.packet.id);
  fprintf(out, "  len_bytes: %d\n", _georef.packet.len_bytes);
  fprintf(out, "  seq_num: %lld\n", (long long) _georef.packet.seq_num);
  fprintf(out, "  version_num: %d\n", _georef.packet.version_num);
  fprintf(out, "  radar_id: %d\n", _georef.packet.radar_id);
  
  time_t ptime = _georef.packet.time_secs_utc;
  fprintf(out, "  time UTC: %s.%.9d\n",
	  DateTime::strm(ptime).c_str(), _georef.packet.time_nano_secs);

  fprintf(out, "  unit_num: %d\n", _georef.unit_num);
  fprintf(out, "  unit_id: %d\n", _georef.unit_id);
  fprintf(out, "  longitude: %g\n", _georef.longitude);
  fprintf(out, "  latitude: %g\n", _georef.latitude);
  fprintf(out, "  altitude_msl_km: %g\n", _georef.altitude_msl_km);
  fprintf(out, "  altitude_agl_km: %g\n", _georef.altitude_agl_km);
  fprintf(out, "  ew_velocity_mps: %g\n", _georef.ew_velocity_mps);
  fprintf(out, "  ns_velocity_mps: %g\n", _georef.ns_velocity_mps);
  fprintf(out, "  vert_velocity_mps: %g\n", _georef.vert_velocity_mps);
  fprintf(out, "  heading_deg: %g\n", _georef.heading_deg);
  fprintf(out, "  track_deg: %g\n", _georef.track_deg);
  fprintf(out, "  roll_deg: %g\n", _georef.roll_deg);
  fprintf(out, "  pitch_deg: %g\n", _georef.pitch_deg);
  fprintf(out, "  drift_angle_deg: %g\n", _georef.drift_angle_deg);
  fprintf(out, "  rotation_angle_deg: %g\n", _georef.rotation_angle_deg);
  fprintf(out, "  tilt_angle_deg: %g\n", _georef.tilt_angle_deg);
  fprintf(out, "  ew_horiz_wind_mps: %g\n", _georef.ew_horiz_wind_mps);
  fprintf(out, "  ns_horiz_wind_mps: %g\n", _georef.ns_horiz_wind_mps);
  fprintf(out, "  vert_wind_mps: %g\n", _georef.vert_wind_mps);
  fprintf(out, "  heading_rate_dps: %g\n", _georef.heading_rate_dps);
  fprintf(out, "  pitch_rate_dps: %g\n", _georef.pitch_rate_dps);
  fprintf(out, "  roll_rate_dps: %g\n", _georef.roll_rate_dps);
  fprintf(out, "  drive_angle_1_deg: %g\n", _georef.drive_angle_1_deg);
  fprintf(out, "  drive_angle_2_deg: %g\n", _georef.drive_angle_2_deg);

  fprintf(out, "\n");
}

void
  DsPlatformGeoref::print(ostream &out) const
{
   
  out << "FLATFORM GEOREF" << endl;
   
  out << "  packet.id: " << _georef.packet.id << endl;
  out << "  packet.len_bytes: " << _georef.packet.len_bytes << endl;
  out << "  packet.seq_num: " << _georef.packet.seq_num << endl;
  out << "  packet.version_num: " << _georef.packet.version_num << endl;
  out << "  packet.radar_id: " << _georef.packet.radar_id << endl;

  time_t ptime = _georef.packet.time_secs_utc;
  char timestr[128];
  sprintf(timestr, "  time UTC: %s.%.9d\n",
	  DateTime::strm(ptime).c_str(), _georef.packet.time_nano_secs);
  out << "  packet_t time UTC: " << timestr << endl;

  out << "  unit_num: " << _georef.unit_num << endl;
  out << "  unit_id: " << _georef.unit_id << endl;
  out << "  longitude: " << _georef.longitude << endl;
  out << "  latitude: " << _georef.latitude << endl;
  out << "  altitude_msl_km: " << _georef.altitude_msl_km << endl;
  out << "  altitude_agl_km: " << _georef.altitude_agl_km << endl;
  out << "  ew_velocity_mps: " << _georef.ew_velocity_mps << endl;
  out << "  ns_velocity_mps: " << _georef.ns_velocity_mps << endl;
  out << "  vert_velocity_mps: " << _georef.vert_velocity_mps << endl;
  out << "  heading_deg: " << _georef.heading_deg << endl;
  out << "  track_deg: " << _georef.track_deg << endl;
  out << "  roll_deg: " << _georef.roll_deg << endl;
  out << "  pitch_deg: " << _georef.pitch_deg << endl;
  out << "  drift_angle_deg: " << _georef.drift_angle_deg << endl;
  out << "  rotation_angle_deg: " << _georef.rotation_angle_deg << endl;
  out << "  tilt_angle_deg: " << _georef.tilt_angle_deg << endl;
  out << "  ew_horiz_wind_mps: " << _georef.ew_horiz_wind_mps << endl;
  out << "  ns_horiz_wind_mps: " << _georef.ns_horiz_wind_mps << endl;
  out << "  vert_wind_mps: " << _georef.vert_wind_mps << endl;
  out << "  heading_rate_dps: " << _georef.heading_rate_dps << endl;
  out << "  pitch_rate_dps: " << _georef.pitch_rate_dps << endl;
  out << "  roll_rate_dps: " << _georef.roll_rate_dps << endl;
  out << "  drive_angle_1_deg: " << _georef.drive_angle_1_deg << endl;
  out << "  drive_angle_2_deg: " << _georef.drive_angle_2_deg << endl;

  out << endl;

}

