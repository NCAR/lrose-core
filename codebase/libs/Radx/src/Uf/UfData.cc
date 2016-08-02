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

#include <Radx/UfData.hh>
#include <Radx/ByteOrder.hh>
#include <Radx/RadxTime.hh>
using namespace std;

/////////////////////////////////////////////////////////////////
// printing

// mandatory header

void UfData::print(ostream &out,
                   const UfData::mandatory_header_t &hdr)
  
{

  out << "===========================================================" << endl;
  out << "UF mandatory header:" << endl;
  out << "    record_length, bytes: " << hdr.record_length
      << ", " << (hdr.record_length) * sizeof(Radx::si16) << endl;
  out << "    optional_header_pos, byte: " << hdr.optional_header_pos
      << ", " << (hdr.optional_header_pos - 1) * sizeof(Radx::si16) << endl;
  out << "    local_use_header_pos, byte: " << hdr.local_use_header_pos
      << ", " << (hdr.local_use_header_pos - 1) * sizeof(Radx::si16) << endl;
  out << "    data_header_pos, byte: " << hdr.data_header_pos
      << ", " << (hdr.data_header_pos - 1) * sizeof(Radx::si16) << endl;
  out << "    record_num: " << hdr.record_num << endl;
  out << "    volume_scan_num: " << hdr.volume_scan_num << endl;
  out << "    ray_num: " << hdr.ray_num << endl;
  out << "    ray_record_num: " << hdr.ray_record_num << endl;
  out << "    sweep_num: " << hdr.sweep_num << endl;
  out << "    radar_name: "<< label(hdr.radar_name, 8) << endl;
  out << "    site_name: " << label(hdr.site_name, 8) << endl;
  out << "    lat_degrees: " << hdr.lat_degrees << endl;
  out << "    lat_minutes: " << hdr.lat_minutes << endl;
  out << "    lat_seconds: " << hdr.lat_seconds / 64.0 << endl;
  double lat = hdr.lat_degrees + hdr.lat_minutes / 60.0 +
    hdr.lat_seconds / (3600.0 * 64.0);
  out << "    lat: " << lat << endl;
  out << "    lon_degrees: " << hdr.lon_degrees << endl;
  out << "    lon_minutes: " << hdr.lon_minutes << endl;
  out << "    lon_seconds: " << hdr.lon_seconds / 64.0 << endl;
  double lon = hdr.lon_degrees + hdr.lon_minutes / 60.0 +
    hdr.lon_seconds / (3600.0 * 64.0);
  out << "    lon: " << lon << endl;
  out << "    antenna_height: " << hdr.antenna_height << endl;
  out << "    year: " << hdr.year << endl;
  out << "    month: " << hdr.month << endl;
  out << "    day: " << hdr.day << endl;
  out << "    hour: " << hdr.hour << endl;
  out << "    minute: " << hdr.minute << endl;
  out << "    second: " << hdr.second << endl;
  out << "    time_zone: " << label(hdr.time_zone, 2) << endl;
  out << "    azimuth: " << hdr.azimuth / 64.0 << endl;
  out << "    elevation: " << hdr.elevation / 64.0 << endl;
  out << "    sweep_mode: " << hdr.sweep_mode << endl;
  out << "    fixed_angle: " << hdr.fixed_angle / 64.0 << endl;
  out << "    sweep_rate: " << hdr.sweep_rate / 64.0 << endl;
  out << "    gen_year: " << hdr.gen_year << endl;
  out << "    gen_month: " << hdr.gen_month << endl;
  out << "    gen_day: " << hdr.gen_day << endl;
  out << "    gen_facility: " << label(hdr.gen_facility, 8) << endl;
  out << "    missing_data_val: " << hdr.missing_data_val << endl;
  out << "===========================================================" << endl;

}

// optional header

void UfData::print(ostream &out,
                   const UfData::optional_header_t &hdr)
  
{

  out << "-----------------------------------------------------------" << endl;
  out << "UF optional header:" << endl;
  out << "    project_name: "<< label(hdr.project_name, 8) << endl;
  out << "    baseline_azimuth: " << hdr.baseline_azimuth << endl;
  out << "    baseline_elevation: " << hdr.baseline_elevation << endl;
  out << "    hour: " << hdr.hour << endl;
  out << "    minute: " << hdr.minute << endl;
  out << "    second: " << hdr.second << endl;
  out << "    tape_name: "<< label(hdr.tape_name, 8) << endl;
  out << "    flag: " << hdr.flag << endl;
  out << "-----------------------------------------------------------" << endl;

}

// data header

void UfData::print(ostream &out,
                   const UfData::data_header_t &hdr)
  
{

  out << "-----------------------------------------------------------" << endl;
  out << "UF data header:" << endl;
  out << "    num_ray_fields: " << hdr.num_ray_fields << endl;
  out << "    num_ray_records: " << hdr.num_ray_records << endl;
  out << "    num_record_fields: " << hdr.num_record_fields << endl;
  out << "-----------------------------------------------------------" << endl;

}

// field info

void UfData::print(ostream &out,
                   int field_num,
                   const UfData::field_info_t &info)
  
{

  out << "-----------------------------------------------------------" << endl;
  out << "UF field info: - field num " << field_num << endl;
  out << "    field_name: "<< label(info.field_name, 2) << endl;
  out << "    field_pos, byte: " << info.field_pos
      << ", " << (info.field_pos - 1) * sizeof(Radx::si16) << endl;
  out << "-----------------------------------------------------------" << endl;

}

// field header

void UfData::print(ostream &out,
                   const string &field_name,
                   int field_num,
                   const UfData::field_header_t &hdr)
  
{

  out << "-----------------------------------------------------------" << endl;
  out << "UF field header" << endl;
  out << "    field name " << field_name << endl;
  out << "    field num " << field_num << endl;
  out << "    data_pos, bytes: " << hdr.data_pos
      << ", " << (hdr.data_pos - 1) * sizeof(Radx::si16) << endl;
  out << "    scale_factor: " << hdr.scale_factor << endl;
  out << "    start_range: " << hdr.start_range << endl;
  out << "    start_center: " << hdr.start_center << endl;
  out << "    volume_spacing: " << hdr.volume_spacing << endl;
  out << "    num_volumes, bytes: " << hdr.num_volumes
      << ", " << hdr.num_volumes * sizeof(Radx::si16) << endl;
  out << "    volume_depth: " << hdr.volume_depth << endl;
  out << "    horiz_beam_width: " << hdr.horiz_beam_width / 64.0 << endl;
  out << "    vert_beam_width: " << hdr.vert_beam_width / 64.0 << endl;
  out << "    receiver_bandwidth: " << hdr.receiver_bandwidth << endl;
  out << "    polarization: " << hdr.polarization << endl;
  out << "    wavelength: " << hdr.wavelength / 64.0 << endl;
  out << "    num_samples: " << hdr.num_samples << endl;
  out << "    threshold_field: " << label(hdr.threshold_field, 2) << endl;
  out << "    threshold_val: " << hdr.threshold_val << endl;
  out << "    scale for nyquist, power and noise: " << hdr.scale << endl;
  double scale = hdr.scale;
  if (scale <= 0) {
    scale = 100.0; // if missing set to 100
    out << "      note: scale not set, using 100.0 instead" << endl;
  }
  out << "    edit_code: " << label(hdr.edit_code, 2) << endl;
  out << "    pulse_rep_time: " << hdr.pulse_rep_time << endl;
  out << "    volume_bits: " << hdr.volume_bits << endl;
  if (field_name[0] == 'V') {
    out << "    nyquist_vel: " << hdr.word20.nyquist_vel / scale << endl;
    out << "    fl_string: " << label(hdr.word21.fl_string, 2) << endl;
  } else {
    out << "    dbz0: " << hdr.word20.dbz0 << endl;
    out << "    noise_power: " << hdr.word21.noise_power / scale << endl;
    out << "    receiver_gain: " << hdr.receiver_gain / scale << endl;
    out << "    peak_power: " << hdr.peak_power / scale << endl;
    out << "    antenna_gain: " << hdr.antenna_gain / scale << endl;
    out << "    pulse_duration: " << hdr.pulse_duration / 64.0 << endl;
  }
  out << "-----------------------------------------------------------" << endl;

}

// print field data

void UfData::printFieldData(ostream &out,
                              const string &field_name,
                              int field_num,
                              int ngates,
                              double scale_factor,
                              int missing_val,
                              const Radx::si16 *data)
  
{
  
  out << "###########################################################" << endl;
  out << "UF field data:" << endl;
  out << "  field name " << field_name << endl;
  out << "  field num " << field_num << endl;
  out << "  scale factor " << scale_factor << endl;
  int nMiss = 0;
  for (int i = 0; i < ngates; i++) {
    if (data[i] == missing_val) {
      nMiss++;
    } else {
      if (nMiss > 0) {
	out << " " << nMiss << "*miss";
	nMiss = 0;
      }
      out << " " << data[i] / scale_factor;
    }
  }
  if (nMiss > 0) {
    out << " " << nMiss << "*miss";
  }
  out << endl;
  out << "###########################################################" << endl;

}

////////////////////////////////////////////////
// get a label - takes care of null termination

string UfData::label(const char *str, int maxLen)

{

  // null terminate

  char *copy = new char[maxLen + 1];
  memset(copy, 0, maxLen + 1);
  memcpy(copy, str, maxLen);
  
  // remove blanks

  int startPos = 0;
  for (int ii = 0; ii <= maxLen; ii++) {
    if (copy[ii] == ' ') {
      startPos++;
    } else {
      break;
    }
  }

  for (int ii = maxLen-1; ii >= 0; ii--) {
    if (copy[ii] == ' ') {
      copy[ii] = '\0';
    } else {
      break;
    }
  }

  string label = copy + startPos;
  delete[] copy;
  return label;

}

////////////////////////////////////////////////////////////////
// Byte swapping

void UfData::swap(UfData::mandatory_header_t &hdr,
                  bool force /* = false */)
{

  ByteOrder::swap16(&hdr.record_length, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.optional_header_pos, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.local_use_header_pos, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.data_header_pos, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.record_num, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.volume_scan_num, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.ray_num, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.ray_record_num, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.sweep_num, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.lat_degrees, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.lat_minutes, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.lat_seconds, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.lon_degrees, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.lon_minutes, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.lon_seconds, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.antenna_height, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.year, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.month, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.day, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.hour, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.minute, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.second, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.azimuth, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.elevation, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.sweep_mode, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.fixed_angle, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.sweep_rate, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.gen_year, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.gen_month, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.gen_day, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.missing_data_val, sizeof(Radx::si16), force);

}

void UfData::swap(UfData::optional_header_t &hdr,
                  bool force /* = false */)
{
  ByteOrder::swap16(&hdr.baseline_azimuth, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.baseline_elevation, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.hour, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.minute, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.second, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.flag, sizeof(Radx::si16), force);
}

void UfData::swap(UfData::data_header_t &hdr,
                  bool force /* = false */)
{
  ByteOrder::swap16(&hdr.num_ray_fields, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.num_ray_records, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.num_record_fields, sizeof(Radx::si16), force);
}

void UfData::swap(UfData::field_info_t &info,
                  bool force /* = false */)
{
  ByteOrder::swap16(&info.field_pos, sizeof(Radx::si16), force);
}

void UfData::swap(UfData::field_header_t &hdr,
                  const string &field_name,
                  bool force /* = false */)
{
  ByteOrder::swap16(&hdr.data_pos, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.scale_factor, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.start_range, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.start_center, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.volume_spacing, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.num_volumes, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.volume_depth, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.horiz_beam_width, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.vert_beam_width, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.receiver_bandwidth, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.polarization, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.wavelength, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.num_samples, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.threshold_val, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.scale, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.pulse_rep_time, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.volume_bits, sizeof(Radx::si16), force);
  if (field_name.c_str()[0] == 'V') {
    ByteOrder::swap16(&hdr.word20.nyquist_vel, sizeof(Radx::si16), force);
  } else {
    ByteOrder::swap16(&hdr.word20.dbz0, sizeof(Radx::si16), force);
    ByteOrder::swap16(&hdr.word21.noise_power, sizeof(Radx::si16), force);
  }
  ByteOrder::swap16(&hdr.receiver_gain, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.peak_power, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.antenna_gain, sizeof(Radx::si16), force);
  ByteOrder::swap16(&hdr.pulse_duration, sizeof(Radx::si16), force);
}

