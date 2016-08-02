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

#include <rapformats/UfRadar.hh>
#include <dataport/bigend.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
using namespace std;

/////////////////////////////////////////////////////////////////
// printing

// print mandatory header

void UfRadar::print_mandatory_header(ostream &out,
				     const UF_mandatory_header_t &hdr)

{

  out << "===========================================================" << endl;
  out << "UF mandatory header:" << endl;
  out << "    record_length, bytes: " << hdr.record_length
      << ", " << (hdr.record_length) * sizeof(si16) << endl;
  out << "    optional_header_pos, byte: " << hdr.optional_header_pos
      << ", " << (hdr.optional_header_pos - 1) * sizeof(si16) << endl;
  out << "    local_use_header_pos, byte: " << hdr.local_use_header_pos
      << ", " << (hdr.local_use_header_pos - 1) * sizeof(si16) << endl;
  out << "    data_header_pos, byte: " << hdr.data_header_pos
      << ", " << (hdr.data_header_pos - 1) * sizeof(si16) << endl;
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
  double lat = hdr.lat_degrees + hdr.lat_minutes / 60.0 + hdr.lat_seconds / (3600.0 * 64.0);
  out << "    lat: " << lat << endl;
  out << "    lon_degrees: " << hdr.lon_degrees << endl;
  out << "    lon_minutes: " << hdr.lon_minutes << endl;
  out << "    lon_seconds: " << hdr.lon_seconds / 64.0 << endl;
  double lon = hdr.lon_degrees + hdr.lon_minutes / 60.0 + hdr.lon_seconds / (3600.0 * 64.0);
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

// print data header

void UfRadar::print_data_header(ostream &out,
				const UF_data_header_t &hdr)

{

  out << "-----------------------------------------------------------" << endl;
  out << "UF data header:" << endl;
  out << "    num_ray_fields: " << hdr.num_ray_fields << endl;
  out << "    num_ray_records: " << hdr.num_ray_records << endl;
  out << "    num_record_fields: " << hdr.num_record_fields << endl;
  out << "-----------------------------------------------------------" << endl;

}

// print field info

void UfRadar::print_field_info(ostream &out,
			  int field_num,
			  const UF_field_info_t &info)

{

  out << "-----------------------------------------------------------" << endl;
  out << "UF field info: - field num " << field_num << endl;
  out << "    field_name: "<< label(info.field_name, 2) << endl;
  out << "    field_pos, byte: " << info.field_pos
      << ", " << (info.field_pos - 1) * sizeof(si16) << endl;
  out << "-----------------------------------------------------------" << endl;

}

// print field header

void UfRadar::print_field_header(ostream &out,
				 const string &field_name,
				 int field_num,
				 const UF_field_header_t &hdr)
  
{

  out << "-----------------------------------------------------------" << endl;
  out << "UF field header" << endl;
  out << "    field name " << field_name << endl;
  out << "    field num " << field_num << endl;
  out << "    data_pos, bytes: " << hdr.data_pos
      << ", " << (hdr.data_pos - 1) * sizeof(si16) << endl;
  out << "    scale_factor: " << hdr.scale_factor << endl;
  out << "    start_range: " << hdr.start_range << endl;
  out << "    start_center: " << hdr.start_center << endl;
  out << "    volume_spacing: " << hdr.volume_spacing << endl;
  out << "    num_volumes, bytes: " << hdr.num_volumes
      << ", " << hdr.num_volumes * sizeof(si16) << endl;
  out << "    volume_depth: " << hdr.volume_depth << endl;
  out << "    horiz_beam_width: " << hdr.horiz_beam_width / 64.0 << endl;
  out << "    vert_beam_width: " << hdr.vert_beam_width / 64.0 << endl;
  out << "    receiver_bandwidth: " << hdr.receiver_bandwidth << endl;
  out << "    polarization: " << hdr.polarization << endl;
  out << "    wavelength: " << hdr.wavelength / 64.0 << endl;
  out << "    num_samples: " << hdr.num_samples << endl;
  out << "    threshold_field: " << label(hdr.threshold_field, 2) << endl;
  out << "    threshold_val: " << hdr.threshold_val << endl;
  out << "    scale for power and noise: " << hdr.scale << endl;
  out << "    edit_code: " << label(hdr.edit_code, 2) << endl;
  out << "    pulse_rep_time: " << hdr.pulse_rep_time << endl;
  out << "    volume_bits: " << hdr.volume_bits << endl;
  if (field_name.c_str()[0] == 'V') {
    out << "    nyquist_vel: "
	<< hdr.word20.nyquist_vel / (double) hdr.scale_factor << endl;
    out << "    fl_string: " << label(hdr.word21.fl_string, 2) << endl;
  } else {
    out << "    radar_const: " << hdr.word20.radar_const << endl;
    out << "    noise_power: " << hdr.word21.noise_power / (double) hdr.scale << endl;
    out << "    receiver_gain: " << hdr.receiver_gain / (double) hdr.scale << endl;
    out << "    peak_power: " << hdr.peak_power / (double) hdr.scale << endl;
    out << "    antenna_gain: " << hdr.antenna_gain / (double) hdr.scale << endl;
    out << "    pulse_duration: " << hdr.pulse_duration / 64.0 << endl;
  }
  out << "-----------------------------------------------------------" << endl;

}

// print field data

void UfRadar::print_field_data(ostream &out,
			       const string &field_name,
			       int field_num,
			       int ngates,
			       double scale_factor,
			       int missing_val,
			       const si16 *data)

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

string UfRadar::label(const char *str, int maxLen)

{

  // null terminate

  TaArray<char> copy_;
  char *copy = copy_.alloc(maxLen + 1);
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

  return copy + startPos;

}

////////////////////////////////////////////////////////////////
// Byte swapping

// mandatory header to BigEndian

void UfRadar::BE_from_mandatory_header(UF_mandatory_header_t &hdr)
{

  BE_from_array_16(&hdr.record_length, sizeof(si16));
  BE_from_array_16(&hdr.optional_header_pos, sizeof(si16));
  BE_from_array_16(&hdr.local_use_header_pos, sizeof(si16));
  BE_from_array_16(&hdr.data_header_pos, sizeof(si16));
  BE_from_array_16(&hdr.record_num, sizeof(si16));
  BE_from_array_16(&hdr.volume_scan_num, sizeof(si16));
  BE_from_array_16(&hdr.ray_num, sizeof(si16));
  BE_from_array_16(&hdr.ray_record_num, sizeof(si16));
  BE_from_array_16(&hdr.sweep_num, sizeof(si16));
  BE_from_array_16(&hdr.lat_degrees, sizeof(si16));
  BE_from_array_16(&hdr.lat_minutes, sizeof(si16));
  BE_from_array_16(&hdr.lat_seconds, sizeof(si16));
  BE_from_array_16(&hdr.lon_degrees, sizeof(si16));
  BE_from_array_16(&hdr.lon_minutes, sizeof(si16));
  BE_from_array_16(&hdr.lon_seconds, sizeof(si16));
  BE_from_array_16(&hdr.antenna_height, sizeof(si16));
  BE_from_array_16(&hdr.year, sizeof(si16));
  BE_from_array_16(&hdr.month, sizeof(si16));
  BE_from_array_16(&hdr.day, sizeof(si16));
  BE_from_array_16(&hdr.hour, sizeof(si16));
  BE_from_array_16(&hdr.minute, sizeof(si16));
  BE_from_array_16(&hdr.second, sizeof(si16));
  BE_from_array_16(&hdr.azimuth, sizeof(si16));
  BE_from_array_16(&hdr.elevation, sizeof(si16));
  BE_from_array_16(&hdr.sweep_mode, sizeof(si16));
  BE_from_array_16(&hdr.fixed_angle, sizeof(si16));
  BE_from_array_16(&hdr.sweep_rate, sizeof(si16));
  BE_from_array_16(&hdr.gen_year, sizeof(si16));
  BE_from_array_16(&hdr.gen_month, sizeof(si16));
  BE_from_array_16(&hdr.gen_day, sizeof(si16));
  BE_from_array_16(&hdr.missing_data_val, sizeof(si16));

}

// BigEndian to mandatory header

void UfRadar::BE_to_mandatory_header(UF_mandatory_header_t &hdr)
{

  BE_to_array_16(&hdr.record_length, sizeof(si16));
  BE_to_array_16(&hdr.optional_header_pos, sizeof(si16));
  BE_to_array_16(&hdr.local_use_header_pos, sizeof(si16));
  BE_to_array_16(&hdr.data_header_pos, sizeof(si16));
  BE_to_array_16(&hdr.record_num, sizeof(si16));
  BE_to_array_16(&hdr.volume_scan_num, sizeof(si16));
  BE_to_array_16(&hdr.ray_num, sizeof(si16));
  BE_to_array_16(&hdr.ray_record_num, sizeof(si16));
  BE_to_array_16(&hdr.sweep_num, sizeof(si16));
  BE_to_array_16(&hdr.lat_degrees, sizeof(si16));
  BE_to_array_16(&hdr.lat_minutes, sizeof(si16));
  BE_to_array_16(&hdr.lat_seconds, sizeof(si16));
  BE_to_array_16(&hdr.lon_degrees, sizeof(si16));
  BE_to_array_16(&hdr.lon_minutes, sizeof(si16));
  BE_to_array_16(&hdr.lon_seconds, sizeof(si16));
  BE_to_array_16(&hdr.antenna_height, sizeof(si16));
  BE_to_array_16(&hdr.year, sizeof(si16));
  BE_to_array_16(&hdr.month, sizeof(si16));
  BE_to_array_16(&hdr.day, sizeof(si16));
  BE_to_array_16(&hdr.hour, sizeof(si16));
  BE_to_array_16(&hdr.minute, sizeof(si16));
  BE_to_array_16(&hdr.second, sizeof(si16));
  BE_to_array_16(&hdr.azimuth, sizeof(si16));
  BE_to_array_16(&hdr.elevation, sizeof(si16));
  BE_to_array_16(&hdr.sweep_mode, sizeof(si16));
  BE_to_array_16(&hdr.fixed_angle, sizeof(si16));
  BE_to_array_16(&hdr.sweep_rate, sizeof(si16));
  BE_to_array_16(&hdr.gen_year, sizeof(si16));
  BE_to_array_16(&hdr.gen_month, sizeof(si16));
  BE_to_array_16(&hdr.gen_day, sizeof(si16));
  BE_to_array_16(&hdr.missing_data_val, sizeof(si16));

}

// data header to BigEndian

void UfRadar::BE_from_data_header(UF_data_header_t &hdr)
{
  BE_from_array_16(&hdr.num_ray_fields, sizeof(si16));
  BE_from_array_16(&hdr.num_ray_records, sizeof(si16));
  BE_from_array_16(&hdr.num_record_fields, sizeof(si16));
}

// BigEndian to data header

void UfRadar::BE_to_data_header(UF_data_header_t &hdr)
{
  BE_to_array_16(&hdr.num_ray_fields, sizeof(si16));
  BE_to_array_16(&hdr.num_ray_records, sizeof(si16));
  BE_to_array_16(&hdr.num_record_fields, sizeof(si16));
}

// field info to BigEndian

void UfRadar::BE_from_field_info(UF_field_info_t &info)
{
  BE_from_array_16(&info.field_pos, sizeof(si16));
}

// BigEndian to field info

void UfRadar::BE_to_field_info(UF_field_info_t &info)
{
  BE_to_array_16(&info.field_pos, sizeof(si16));
}

// field header to BigEndian

void UfRadar::BE_from_field_header(UF_field_header_t &hdr, const string &field_name)
{
  BE_from_array_16(&hdr.data_pos, sizeof(si16));
  BE_from_array_16(&hdr.scale_factor, sizeof(si16));
  BE_from_array_16(&hdr.start_range, sizeof(si16));
  BE_from_array_16(&hdr.start_center, sizeof(si16));
  BE_from_array_16(&hdr.volume_spacing, sizeof(si16));
  BE_from_array_16(&hdr.num_volumes, sizeof(si16));
  BE_from_array_16(&hdr.volume_depth, sizeof(si16));
  BE_from_array_16(&hdr.horiz_beam_width, sizeof(si16));
  BE_from_array_16(&hdr.vert_beam_width, sizeof(si16));
  BE_from_array_16(&hdr.receiver_bandwidth, sizeof(si16));
  BE_from_array_16(&hdr.polarization, sizeof(si16));
  BE_from_array_16(&hdr.wavelength, sizeof(si16));
  BE_from_array_16(&hdr.num_samples, sizeof(si16));
  BE_from_array_16(&hdr.threshold_val, sizeof(si16));
  BE_from_array_16(&hdr.scale, sizeof(si16));
  BE_from_array_16(&hdr.pulse_rep_time, sizeof(si16));
  BE_from_array_16(&hdr.volume_bits, sizeof(si16));
  if (field_name.c_str()[0] == 'V') {
    BE_from_array_16(&hdr.word20.nyquist_vel, sizeof(si16));
  } else {
    BE_from_array_16(&hdr.word20.radar_const, sizeof(si16));
    BE_from_array_16(&hdr.word21.noise_power, sizeof(si16));
  }
  BE_from_array_16(&hdr.receiver_gain, sizeof(si16));
  BE_from_array_16(&hdr.peak_power, sizeof(si16));
  BE_from_array_16(&hdr.antenna_gain, sizeof(si16));
  BE_from_array_16(&hdr.pulse_duration, sizeof(si16));
}

// BigEndian to field header

void UfRadar::BE_to_field_header(UF_field_header_t &hdr, const string &field_name)
{
  BE_to_array_16(&hdr.data_pos, sizeof(si16));
  BE_to_array_16(&hdr.scale_factor, sizeof(si16));
  BE_to_array_16(&hdr.start_range, sizeof(si16));
  BE_to_array_16(&hdr.start_center, sizeof(si16));
  BE_to_array_16(&hdr.volume_spacing, sizeof(si16));
  BE_to_array_16(&hdr.num_volumes, sizeof(si16));
  BE_to_array_16(&hdr.volume_depth, sizeof(si16));
  BE_to_array_16(&hdr.horiz_beam_width, sizeof(si16));
  BE_to_array_16(&hdr.vert_beam_width, sizeof(si16));
  BE_to_array_16(&hdr.receiver_bandwidth, sizeof(si16));
  BE_to_array_16(&hdr.polarization, sizeof(si16));
  BE_to_array_16(&hdr.wavelength, sizeof(si16));
  BE_to_array_16(&hdr.num_samples, sizeof(si16));
  BE_to_array_16(&hdr.threshold_val, sizeof(si16));
  BE_to_array_16(&hdr.scale, sizeof(si16));
  BE_to_array_16(&hdr.pulse_rep_time, sizeof(si16));
  BE_to_array_16(&hdr.volume_bits, sizeof(si16));
  if (field_name.c_str()[0] == 'V') {
    BE_to_array_16(&hdr.word20.nyquist_vel, sizeof(si16));
  } else {
    BE_to_array_16(&hdr.word20.radar_const, sizeof(si16));
    BE_to_array_16(&hdr.word21.noise_power, sizeof(si16));
  }
  BE_to_array_16(&hdr.receiver_gain, sizeof(si16));
  BE_to_array_16(&hdr.peak_power, sizeof(si16));
  BE_to_array_16(&hdr.antenna_gain, sizeof(si16));
  BE_to_array_16(&hdr.pulse_duration, sizeof(si16));
}

