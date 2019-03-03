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
/*********************************************************************
 * ArcBeamMsg : Class representing a VIRAQ header read from a HiQ
 *               data stream.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2006
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <dataport/smallend.h>
#include <toolsa/DateTime.hh>

#include "ArcBeamMsg.hh"
using namespace std;


// Define globals

const int ArcBeamMsg::HEADER_SIZE = 189;

const int ArcBeamMsg::PIRAQ_CLOCK_FREQ = 10000000;    // 10 Mhz
const double ArcBeamMsg::HALF_LIGHT_SPEED = 2.99792458e8 / 2.0;    // m/s


ArcBeamMsg::ArcBeamMsg(const bool debug) :
  HiqMsg(debug),
  _abp(0),
  _abpSize(0)
{
}

ArcBeamMsg::ArcBeamMsg(const ArcBeamMsg &rhs) :
  HiqMsg(rhs._debug)
{
  // Copy the message header information

  memcpy(&_msgHeader, &rhs._msgHeader, sizeof(_msgHeader_t));
  
  // Copy the beam data

  _abp = new si16[_abpSize];
  memcpy(_abp, rhs._abp, _abpSize * sizeof(si16));
}

ArcBeamMsg::~ArcBeamMsg() 
{
  delete [] _abp;
}


bool ArcBeamMsg::init(const char *buffer)
{
  char *buffer_ptr = (char *)buffer;

  // Copy the message header

  memcpy(&_msgHeader, buffer_ptr, sizeof(_msgHeader_t));
  buffer_ptr += sizeof(_msgHeader_t);
  
  // Byte swap each of the numical fields.  Do each field individually since
  // the types are all mixed up in the structure.  Character fields don't need
  // swapping.

  SE_to_array_32(&_msgHeader.record_len, sizeof(ui32));
  SE_to_array_32(&_msgHeader.channel, sizeof(ui32));
  SE_to_array_32(&_msgHeader.rev, sizeof(ui32));
  SE_to_array_32(&_msgHeader.one, sizeof(ui32));
  SE_to_array_32(&_msgHeader.byte_offset_to_data, sizeof(ui32));
  SE_to_array_32(&_msgHeader.data_format, sizeof(ui32));
  SE_to_array_32(&_msgHeader.type_of_compression, sizeof(ui32));
  SE_to_array_64(&_msgHeader.pulse_num, sizeof(ui64));
  SE_to_array_64(&_msgHeader.beam_num, sizeof(ui64));
  SE_to_array_32(&_msgHeader.gates, sizeof(ui32));
  SE_to_array_32(&_msgHeader.start_gate, sizeof(ui32));
  SE_to_array_32(&_msgHeader.hits, sizeof(ui32));
  SE_to_array_32(&_msgHeader.ctrl_flags, sizeof(ui32));
  SE_to_array_32(&_msgHeader.bytes_per_gate, sizeof(ui32));
  SE_to_array_32(&_msgHeader.rcvr_pulse_width, sizeof(fl32));
  SE_to_array_32(_msgHeader.prt, ARC_NUM_PRT * sizeof(fl32));
  SE_to_array_32(&_msgHeader.meters_to_first_gate, sizeof(fl32));
  SE_to_array_32(&_msgHeader.num_segments, sizeof(ui32));
  SE_to_array_32(_msgHeader.gate_spacing_meters,
		 ARC_MAX_SEGMENTS * sizeof(fl32));
  SE_to_array_32(_msgHeader.gates_in_segment, ARC_MAX_SEGMENTS * sizeof(ui32));
  SE_to_array_32(_msgHeader.clutter_start,
		 ARC_NUM_CLUTTER_REGIONS * sizeof(ui32));
  SE_to_array_32(_msgHeader.clutter_end,
		 ARC_NUM_CLUTTER_REGIONS * sizeof(ui32));
  SE_to_array_32(_msgHeader.clutter_type,
		 ARC_NUM_CLUTTER_REGIONS * sizeof(ui32));
  SE_to_array_32(&_msgHeader.secs, sizeof(ui32));
  SE_to_array_32(&_msgHeader.nanoseconds, sizeof(ui32));
  SE_to_array_32(&_msgHeader.az, sizeof(fl32));
  SE_to_array_32(&_msgHeader.az_off_ref, sizeof(fl32));
  SE_to_array_32(&_msgHeader.el, sizeof(fl32));
  SE_to_array_32(&_msgHeader.el_off_ref, sizeof(fl32));
  SE_to_array_32(&_msgHeader.radar_longitude, sizeof(fl32));
  SE_to_array_32(&_msgHeader.radar_latitude, sizeof(fl32));
  SE_to_array_32(&_msgHeader.radar_altitude, sizeof(fl32));
  SE_to_array_32(&_msgHeader.ts_start_gate, sizeof(ui32));
  SE_to_array_32(&_msgHeader.ts_end_gate, sizeof(ui32));
  SE_to_array_32(&_msgHeader.ew_velocity, sizeof(fl32));
  SE_to_array_32(&_msgHeader.ns_velocity, sizeof(fl32));
  SE_to_array_32(&_msgHeader.vert_velocity, sizeof(fl32));
  SE_to_array_32(&_msgHeader.fxd_angle, sizeof(fl32));
  SE_to_array_32(&_msgHeader.true_scan_rate, sizeof(fl32));
  SE_to_array_32(&_msgHeader.scan_type, sizeof(ui32));
  SE_to_array_32(&_msgHeader.scan_num, sizeof(ui32));
  SE_to_array_32(&_msgHeader.vol_num, sizeof(ui32));
  SE_to_array_32(&_msgHeader.transition, sizeof(ui32));
  SE_to_array_32(&_msgHeader.h_xmit_power, sizeof(fl32));
  SE_to_array_32(&_msgHeader.yaw, sizeof(fl32));
  SE_to_array_32(&_msgHeader.pitch, sizeof(fl32));
  SE_to_array_32(&_msgHeader.roll, sizeof(fl32));
  SE_to_array_32(&_msgHeader.track, sizeof(fl32));
  SE_to_array_32(&_msgHeader.gate0_mag, sizeof(fl32));
  SE_to_array_32(&_msgHeader.dacv, sizeof(fl32));
  SE_to_array_32(&_msgHeader.packet_flag, sizeof(ui32));
  SE_to_array_32(&_msgHeader.year, sizeof(ui32));
  SE_to_array_32(&_msgHeader.julian_day, sizeof(ui32));
  SE_to_array_32(&_msgHeader.test_pulse_pwr, sizeof(fl32));
  SE_to_array_32(&_msgHeader.test_pulse_frq, sizeof(fl32));
  SE_to_array_32(&_msgHeader.frequency, sizeof(fl32));
  SE_to_array_32(&_msgHeader.stalo_freq, sizeof(fl32));
  SE_to_array_32(&_msgHeader.noise_figure, sizeof(fl32));
  SE_to_array_32(&_msgHeader.noise_power, sizeof(fl32));
  SE_to_array_32(&_msgHeader.receiver_gain, sizeof(fl32));
  SE_to_array_32(&_msgHeader.e_plane_angle, sizeof(fl32));
  SE_to_array_32(&_msgHeader.h_plane_angle, sizeof(fl32));
  SE_to_array_32(&_msgHeader.data_sys_sat, sizeof(fl32));
  SE_to_array_32(&_msgHeader.antenna_gain, sizeof(fl32));
  SE_to_array_32(&_msgHeader.h_beam_width, sizeof(fl32));
  SE_to_array_32(&_msgHeader.v_beam_width, sizeof(fl32));
  SE_to_array_32(&_msgHeader.xmit_pulse_width, sizeof(fl32));
  SE_to_array_32(&_msgHeader.rconst, sizeof(fl32));
  SE_to_array_32(&_msgHeader.phase_offset, sizeof(fl32));
  SE_to_array_32(&_msgHeader.zdr_fudge_factor, sizeof(fl32));
  SE_to_array_32(&_msgHeader.mismatch_loss, sizeof(fl32));
  SE_to_array_32(&_msgHeader.rcvr_const, sizeof(fl32));
  SE_to_array_32(_msgHeader.test_pulse_rngs_km,
		 ARC_NUM_TEST_PULSE_RINGS * sizeof(fl32));
  SE_to_array_32(&_msgHeader.antenna_rotation_angle, sizeof(fl32));
  SE_to_array_32(&_msgHeader.i_norm, sizeof(fl32));
  SE_to_array_32(&_msgHeader.q_norm, sizeof(fl32));
  SE_to_array_32(&_msgHeader.i_compand, sizeof(fl32));
  SE_to_array_32(&_msgHeader.q_compand, sizeof(fl32));
  SE_to_array_32(_msgHeader.transform_matrix,
		 ARC_TRANSFORM_NUMX * ARC_TRANSFORM_NUMY *
		 ARC_TRANSFORM_NUMZ * sizeof(fl32));
  SE_to_array_32(_msgHeader.stokes, ARC_NUM_STOKES * sizeof(fl32));
  SE_to_array_32(&_msgHeader.v_xmit_power, sizeof(fl32));
  SE_to_array_32(&_msgHeader.v_receiver_gain, sizeof(fl32));
  SE_to_array_32(&_msgHeader.v_antenna_gain, sizeof(fl32));
  SE_to_array_32(&_msgHeader.v_noise_power, sizeof(fl32));
  SE_to_array_32(&_msgHeader.h_measured_xmit_power, sizeof(fl32));
  SE_to_array_32(&_msgHeader.v_measured_xmit_power, sizeof(fl32));
  SE_to_array_32(&_msgHeader.angle_source, sizeof(ui32));
  SE_to_array_32(&_msgHeader.timing_mode, sizeof(ui32));
  SE_to_array_32(&_msgHeader.tp_width, sizeof(fl32));
  SE_to_array_32(&_msgHeader.tp_delay, sizeof(fl32));
  SE_to_array_32(&_msgHeader.delay, sizeof(fl32));
  SE_to_array_32(&_msgHeader.pll_freq, sizeof(fl32));
  SE_to_array_32(&_msgHeader.pll_alpha, sizeof(fl32));
  SE_to_array_32(&_msgHeader.afc_lo, sizeof(fl32));
  SE_to_array_32(&_msgHeader.afc_hi, sizeof(fl32));
  SE_to_array_32(&_msgHeader.afc_gain, sizeof(fl32));
  SE_to_array_32(&_msgHeader.vel_sign, sizeof(fl32));
  SE_to_array_32(&_msgHeader.xmit_coupler, sizeof(fl32));
  SE_to_array_32(_msgHeader.spare, ARC_NUM_SPARES * sizeof(fl32));

  // Extract the data from the message buffer

  int data_bytes = _msgHeader.record_len - sizeof(_msgHeader_t);
  int array_size = data_bytes / sizeof(si16);

  if (_debug)
    cerr << "   Allocating " << data_bytes << " bytes for data pointer" << endl;

  delete [] _abp;
  _abp = new si16[array_size];
  memcpy(_abp, buffer_ptr, data_bytes);
  _abpSize = array_size;
  
  return true;
}


void ArcBeamMsg::print(ostream &stream) const
{
  stream << "sizeof(_msgHeader) = " << sizeof(_msgHeader) << endl;
  stream << "Input Header" << endl;
  stream << "------------" << endl;
  stream << "record_len = " << _msgHeader.record_len << endl;
  _printString(stream, "desc", _msgHeader.description, ARC_DESC_LEN);
  stream << "channel = " << _msgHeader.channel << endl;
  stream << "rev = " << _msgHeader.rev << endl;
  stream << "one = " << _msgHeader.one << endl;
  stream << "byte_offset_to_data = " << _msgHeader.byte_offset_to_data << endl;
  stream << "data_format = " << _msgHeader.data_format << endl;
  stream << "type_of_compression = " << _msgHeader.type_of_compression << endl;
  stream << "pulse_num = " << _msgHeader.pulse_num << endl;
  stream << "beam_num = " << _msgHeader.beam_num << endl;
  stream << "gates = " << _msgHeader.gates << endl;
  stream << "start_gate = " << _msgHeader.start_gate << endl;
  stream << "hits = " << _msgHeader.hits << endl;
  stream << "ctrl_flags = " << _msgHeader.ctrl_flags << endl;
  stream << "bytes_per_gate = " << _msgHeader.bytes_per_gate << endl;
  stream << "rcvr_pulse_width = " << _msgHeader.rcvr_pulse_width << endl;
  for (int i = 0; i < ARC_NUM_PRT; ++i)
    stream << "prt[" << i << "] = " << _msgHeader.prt[i] << endl;
  stream << "meters_to_first_gate = " << _msgHeader.meters_to_first_gate << endl;
  stream << "num_segments = " << _msgHeader.num_segments << endl;
  for (int i = 0; i < ARC_MAX_SEGMENTS; ++i)
  {
    stream << "gate_spacing_meters[" << i << "] = "
	   << _msgHeader.gate_spacing_meters[i] << endl;
    stream << "gates_in_segment[" << i << "] = "
	   << _msgHeader.gates_in_segment[i] << endl;
  }
  for (int i = 0; i < ARC_NUM_CLUTTER_REGIONS; ++i)
  {
    stream << "clutter_start[" << i << "] = "
	   << _msgHeader.clutter_start[i] << endl;
    stream << "clutter_end[" << i << "] = "
	   << _msgHeader.clutter_end[i] << endl;
    stream << "clutter_type[" << i << "] = "
	   << _msgHeader.clutter_type[i] << endl;
  }
  stream << "secs = " << _msgHeader.secs << endl;
  stream << "nanoseconds = " << _msgHeader.nanoseconds << endl;
  stream << "az = " << _msgHeader.az << endl;
  stream << "az_off_ref = " << _msgHeader.az_off_ref << endl;
  stream << "el = " << _msgHeader.el << endl;
  stream << "el_off_ref = " << _msgHeader.el_off_ref << endl;
  stream << "radar_lon = " << _msgHeader.radar_longitude << endl;
  stream << "radar_lat = " << _msgHeader.radar_latitude << endl;
  stream << "radar_alt = " << _msgHeader.radar_altitude << endl;
  _printString(stream, "gps_datum", _msgHeader.gps_datum, ARC_MAX_GPS_DATUM);
  stream << "ts_start_gate = " << _msgHeader.ts_start_gate << endl;
  stream << "ts_end_gate = " << _msgHeader.ts_end_gate << endl;
  stream << "ew_velocity = " << _msgHeader.ew_velocity << endl;
  stream << "ns_velocity = " << _msgHeader.ns_velocity << endl;
  stream << "vert_velocity = " << _msgHeader.vert_velocity << endl;
  stream << "fxd_angle = " << _msgHeader.fxd_angle << endl;
  stream << "true_scan_rate = " << _msgHeader.true_scan_rate << endl;
  stream << "scan_type = " << _msgHeader.scan_type << endl;
  stream << "scan_num = " << _msgHeader.scan_num << endl;
  stream << "vol_num = " << _msgHeader.vol_num << endl;
  stream << "transition = " << _msgHeader.transition << endl;
  stream << "h_xmit_power = " << _msgHeader.h_xmit_power << endl;
  stream << "yaw = " << _msgHeader.yaw << endl;
  stream << "pitch = " << _msgHeader.pitch << endl;
  stream << "roll = " << _msgHeader.roll << endl;
  stream << "track = " << _msgHeader.track << endl;
  stream << "gate0_mag = " << _msgHeader.gate0_mag << endl;
  stream << "dacv = " << _msgHeader.dacv << endl;
  stream << "packet_flag = " << _msgHeader.packet_flag << endl;
  stream << "year = " << _msgHeader.year << endl;
  stream << "julian_day = " << _msgHeader.julian_day << endl;
  _printString(stream, "radar_name",
	       _msgHeader.radar_name, ARC_MAX_RADAR_NAME);
  _printString(stream, "channel_name",
	       _msgHeader.channel_name, ARC_MAX_CHANNEL_NAME);
  _printString(stream, "project_name",
	       _msgHeader.project_name, ARC_MAX_PROJECT_NAME);
  _printString(stream, "operator_name",
	       _msgHeader.operator_name, ARC_MAX_OPERATOR_NAME);
  _printString(stream, "site_name", _msgHeader.site_name, ARC_MAX_SITE_NAME);
  _printString(stream, "polarization", _msgHeader.polarization,
	       ARC_POLARIZATION_STR_LEN);
  stream << "test_pulse_pwr = " << _msgHeader.test_pulse_pwr << endl;
  stream << "test_pulse_frq = " << _msgHeader.test_pulse_frq << endl;
  stream << "frequency = " << _msgHeader.frequency << endl;
  stream << "stalo_freq = " << _msgHeader.stalo_freq << endl;
  stream << "noise_figure = " << _msgHeader.noise_figure << endl;
  stream << "noise_power = " << _msgHeader.noise_power << endl;
  stream << "receiver_gain = " << _msgHeader.receiver_gain << endl;
  stream << "e_plane_angle = " << _msgHeader.e_plane_angle << endl;
  stream << "h_plane_angle = " << _msgHeader.h_plane_angle << endl;
  stream << "data_sys_sat = " << _msgHeader.data_sys_sat << endl;
  stream << "antenna_gain = " << _msgHeader.antenna_gain << endl;
  stream << "h_beam_width = " << _msgHeader.h_beam_width << endl;
  stream << "v_beam_width = " << _msgHeader.v_beam_width << endl;
  stream << "xmit_pulse_width = " << _msgHeader.xmit_pulse_width << endl;
  stream << "rconst = " << _msgHeader.rconst << endl;
  stream << "phase_offset = " << _msgHeader.phase_offset << endl;
  stream << "zdr_fudge_factor = " << _msgHeader.zdr_fudge_factor << endl;
  stream << "mismatch_loss = " << _msgHeader.mismatch_loss << endl;
  stream << "rcvr_const = " << _msgHeader.rcvr_const << endl;
  for (int i = 0; i < ARC_NUM_TEST_PULSE_RINGS; ++i)
    stream << "test_pulse_rngs_km[" << i << "] = "
	   << _msgHeader.test_pulse_rngs_km[i] << endl;
  stream << "antenna_rot_angle = " << _msgHeader.antenna_rotation_angle << endl;
  _printString(stream, "comment", _msgHeader.comment, ARC_SZ_COMMENT);
  stream << "i_norm = " << _msgHeader.i_norm << endl;
  stream << "q_norm = " << _msgHeader.q_norm << endl;
  stream << "i_compand = " << _msgHeader.i_compand << endl;
  stream << "q_compand = " << _msgHeader.q_compand << endl;
  for (int x = 0; x < ARC_TRANSFORM_NUMX; ++x)
  {
    for (int y = 0; y < ARC_TRANSFORM_NUMY; ++y)
    {
      for (int z = 0; z < ARC_TRANSFORM_NUMZ; ++z)
	stream << "transform_matrix[" << x << "][" << y << "][" << z << "] = "
	       << _msgHeader.transform_matrix[x][y][z] << endl;
    }
  }
  for (int i = 0; i < ARC_NUM_STOKES; ++i)
    stream << "stokes[" << i << "] = " << _msgHeader.stokes[i] << endl;
  stream << "v_xmit_power = " << _msgHeader.v_xmit_power << endl;
  stream << "v_receiver_gain = " << _msgHeader.v_receiver_gain << endl;
  stream << "v_antenna_gain = " << _msgHeader.v_antenna_gain << endl;
  stream << "v_noise_power = " << _msgHeader.v_noise_power << endl;
  stream << "h_measured_xmit_power = " << _msgHeader.h_measured_xmit_power << endl;
  stream << "v_measured_xmit_power = " << _msgHeader.v_measured_xmit_power << endl;
  stream << "angle_source = " << _msgHeader.angle_source << endl;
  stream << "timing_mode = " << _msgHeader.timing_mode << endl;
  stream << "tp_width = " << _msgHeader.tp_width << endl;
  stream << "tp_delay = " << _msgHeader.tp_delay << endl;
  stream << "delay = " << _msgHeader.delay << endl;
  stream << "pll_freq = " << _msgHeader.pll_freq << endl;
  stream << "pll_alpha = " << _msgHeader.pll_alpha << endl;
  stream << "afc_lo = " <<  _msgHeader.afc_lo << endl;
  stream << "afc_hi = " << _msgHeader.afc_hi << endl;
  stream << "afc_gain = " << _msgHeader.afc_gain << endl;
  stream << "vel_sign = " << _msgHeader.vel_sign << endl;
  stream << "xmit_coupler = " << _msgHeader.xmit_coupler << endl;
  stream << endl;
}


void ArcBeamMsg::printSummary(ostream &stream) const
{
  stream << "beam: elev = " << _msgHeader.el << ", az = " << _msgHeader.az
	 << ", time = " << DateTime::str(_msgHeader.secs) << endl;
}


void ArcBeamMsg::_printString(ostream &stream, const string &label,
			      const ui08 *value, const int num_chars)
{
  stream << label << " = <";
  for (int i = 0; i < num_chars; ++i)
  {
    if (isprint(value[i]))
      stream << value[i];
    else
      stream << "*";
  }
  stream << ">" << endl;
}
