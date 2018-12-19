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
/////////////////////////////////////////////////////////////
// DoradeData.cc
//
// Definitions for DoradeData data
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2010
//
///////////////////////////////////////////////////////////////

#include <Radx/DoradeData.hh>
#include <Radx/RadxTime.hh>
#include <Radx/ByteOrder.hh>

#include <cstring>
#include <cstdio>
#include <iostream>
using namespace std;

const double DoradeData::missingDouble = -9999.0;
const float DoradeData::missingFloat = -9999.0f;
const int DoradeData::missingInt = -9999;

///////////////////////////////////////////////////////////////
// constructor

DoradeData::DoradeData()

{

}

///////////////////////////////////////////////////////////////
// destructor

DoradeData::~DoradeData()

{

}

///////////////////////////////////////////////////////////////
// initialization routines - set initial state for structs

void DoradeData::init(DoradeData::comment_t &val)
{
  memset(&val, 0, sizeof(val));
  string id = "COMM";
  memcpy(&val, id.c_str(), id.size());
  val.nbytes = sizeof(val);
}

void DoradeData::init(DoradeData::super_SWIB_t &val)
{
  memset(&val, 0, sizeof(val));
  string id = "SSWB";
  memcpy(&val, id.c_str(), id.size());
  val.nbytes = sizeof(val);
}

void DoradeData::init(DoradeData::super_SWIB_32bit_t &val)
{
  memset(&val, 0, sizeof(val));
  string id = "SSWB";
  memcpy(&val, id.c_str(), id.size());
  val.nbytes = sizeof(val);
}

void DoradeData::copy(const DoradeData::super_SWIB_t &val,
                      DoradeData::super_SWIB_32bit_t &val32)
{
  memcpy(&val32.id, &val.id, 44);
  memcpy(val32.d_start_time, &val.d_start_time, 152);
}

void DoradeData::copy(const DoradeData::super_SWIB_32bit_t &val32,
                      DoradeData::super_SWIB_t &val)
{
  memcpy(&val.id, &val32.id, 44);
  memcpy(&val.d_start_time, val32.d_start_time, 152);
}

void DoradeData::init(DoradeData::volume_t &val)
{
  memset(&val, 0, sizeof(val));
  string id = "VOLD";
  memcpy(&val, id.c_str(), id.size());
  val.nbytes = sizeof(val);
  val.format_version = 2;
}

void DoradeData::init(DoradeData::radar_t &val)
{
  memset(&val, 0, sizeof(val));
  string id = "RADD";
  memcpy(&val, id.c_str(), id.size());
  val.nbytes = sizeof(val);
  val.radar_const = missingDouble;
  val.peak_power = missingDouble;
  val.receiver_gain = missingDouble;
  val.antenna_gain = missingDouble;
  val.system_gain = missingDouble;
  val.horz_beam_width = missingDouble;
  val.vert_beam_width = missingDouble;
  val.radar_type = RADAR_GROUND;
  val.scan_mode = SCAN_MODE_SUR;
  val.req_rotat_vel = missingDouble;
  val.scan_mode_pram0 = missingDouble;
  val.scan_mode_pram1 = missingDouble;
  val.num_parameter_des = 0;
  val.total_num_des = 0;
  val.data_compress = 0;
  val.data_reduction = 0;
  val.data_red_parm0 = missingDouble;
  val.data_red_parm1 = missingDouble;
  val.radar_longitude = missingDouble;
  val.radar_latitude = missingDouble;
  val.radar_altitude = missingDouble;
  val.eff_unamb_vel = missingDouble;
  val.eff_unamb_range = missingDouble;
  val.freq1 = missingDouble;
  val.freq2 = missingDouble;
  val.freq3 = missingDouble;
  val.freq4 = missingDouble;
  val.freq5 = missingDouble;
  val.prt1 = missingDouble;
  val.prt2 = missingDouble;
  val.prt3 = missingDouble;
  val.prt4 = missingDouble;
  val.prt5 = missingDouble;
  val.aperture_size = missingDouble;
  val.field_of_view = missingDouble;
  val.aperture_eff = missingDouble;
  for (int ii = 0; ii < 11; ii++) {
    val.aux_freq[ii] = missingDouble;
    val.aux_prt[ii] = missingDouble;
  }
  val.pulse_width = missingDouble;
  val.primary_cop_baseln = missingDouble;
  val.secondary_cop_baseln = missingDouble;
  val.pc_xmtr_bandwidth = missingDouble;
  val.pc_waveform_type = 0;
}

void DoradeData::init(DoradeData::lidar_t &val)
{
  memset(&val, 0, sizeof(val));
  string id = "LIDR";
  memcpy(&val, id.c_str(), id.size());
  val.nbytes = sizeof(val);
  val.lidar_const = missingDouble;
  val.pulse_energy = missingDouble;
  val.peak_power = missingDouble;
  val.pulse_width = missingDouble;
  val.aperture_size = missingDouble;
  val.field_of_view = missingDouble;
  val.aperture_eff = missingDouble;
  val.beam_divergence = missingDouble;
  val.lidar_type = LIDAR_GROUND;
  val.scan_mode = SCAN_MODE_SUR;
  val.req_rotat_vel = missingDouble;
  val.eff_unamb_vel = missingDouble;
  val.eff_unamb_range = missingDouble;
  val.num_wvlen_trans = 1;
  val.prf = missingDouble;
  for (int ii = 0; ii < 10; ii++) {
    val.wavelength[ii] = missingDouble;
  }
}

void DoradeData::init(DoradeData::correction_t &val)
{
  memset(&val, 0, sizeof(val));
  string id = "CFAC";
  memcpy(&val, id.c_str(), id.size());
  val.nbytes = sizeof(val);
}

void DoradeData::init(DoradeData::parameter_t &val)
{
  memset(&val, 0, sizeof(val));
  string id = "PARM";
  memcpy(&val, id.c_str(), id.size());
  val.nbytes = sizeof(val);
  val.recvr_bandwidth = missingDouble;
  val.pulse_width = 150;
  val.polarization = POLARIZATION_HORIZONTAL;
  val.num_samples = 0;
  val.binary_format = BINARY_FORMAT_INT16;
  val.threshold_value = missingDouble;
  val.parameter_scale = 1.0;
  val.parameter_bias = 0.0;
  val.bad_data = -32767;
  val.mks_conversion = 0.0;
  val.number_cells = 0;
  val.meters_to_first_cell = 75.0;
  val.meters_between_cells = 150.0;
  val.eff_unamb_vel = missingDouble;
}

void DoradeData::init(DoradeData::cell_vector_t &val)
{
  memset(&val, 0, sizeof(val));
  string id = "CELV";
  memcpy(&val, id.c_str(), id.size());
  val.nbytes = sizeof(val);
}

void DoradeData::init(DoradeData::cell_spacing_fp_t &val)
{
  memset(&val, 0, sizeof(val));
  string id = "CSFD";
  memcpy(&val, id.c_str(), id.size());
  val.nbytes = sizeof(val);
}

void DoradeData::init(DoradeData::sweepinfo_t &val)
{
  memset(&val, 0, sizeof(val));
  string id = "SWIB";
  memcpy(&val, id.c_str(), id.size());
  val.nbytes = sizeof(val);
}

void DoradeData::init(DoradeData::platform_t &val)
{
  memset(&val, 0, sizeof(val));
  string id = "ASIB";
  memcpy(&val, id.c_str(), id.size());
  val.nbytes = sizeof(val);
}

void DoradeData::init(DoradeData::ray_t &val)
{
  memset(&val, 0, sizeof(val));
  string id = "RYIB";
  memcpy(&val, id.c_str(), id.size());
  val.nbytes = sizeof(val);
}

void DoradeData::init(DoradeData::paramdata_t &val)
{
  memset(&val, 0, sizeof(val));
  string id = "RDAT";
  memcpy(&val, id.c_str(), id.size());
  val.nbytes = sizeof(val);
}

void DoradeData::init(DoradeData::qparamdata_t &val)
{
  memset(&val, 0, sizeof(val));
  string id = "QDAT";
  memcpy(&val, id.c_str(), id.size());
  val.nbytes = sizeof(val);
}

void DoradeData::init(DoradeData::extra_stuff_t &val)
{
  memset(&val, 0, sizeof(val));
  string id = "XSTF";
  memcpy(&val, id.c_str(), id.size());
  val.nbytes = sizeof(val);
}

void DoradeData::init(DoradeData::null_block_t &val)
{
  memset(&val, 0, sizeof(val));
  string id = "NULL";
  memcpy(&val, id.c_str(), id.size());
  val.nbytes = sizeof(val);
}

void DoradeData::init(DoradeData::rot_angle_table_t &val)
{
  memset(&val, 0, sizeof(val));
  string id = "RKTB";
  memcpy(&val, id.c_str(), id.size());
  val.nbytes = sizeof(val);
}

void DoradeData::init(DoradeData::rot_table_entry_t &val)
{
  memset(&val, 0, sizeof(val));
}

void DoradeData::init(DoradeData::radar_test_status_t &val)
{
  memset(&val, 0, sizeof(val));
  string id = "FRAD";
  memcpy(&val, id.c_str(), id.size());
  val.nbytes = sizeof(val);
}

void DoradeData::init(DoradeData::field_radar_t &val)
{
  memset(&val, 0, sizeof(val));
  string id = "FRIB";
  memcpy(&val, id.c_str(), id.size());
  val.nbytes = sizeof(val);
}

void DoradeData::init(DoradeData::field_lidar_t &val)
{
  memset(&val, 0, sizeof(val));
  string id = "FLIB";
  memcpy(&val, id.c_str(), id.size());
  val.nbytes = sizeof(val);
}

void DoradeData::init(DoradeData::insitu_descript_t &val)
{
  memset(&val, 0, sizeof(val));
  string id = "SITU";
  memcpy(&val, id.c_str(), id.size());
  val.nbytes = sizeof(val);
}

void DoradeData::init(DoradeData::insitu_data_t &val)
{
  memset(&val, 0, sizeof(val));
  string id = "ISIT";
  memcpy(&val, id.c_str(), id.size());
  val.nbytes = sizeof(val);
}

void DoradeData::init(DoradeData::indep_freq_t &val)
{
  memset(&val, 0, sizeof(val));
  string id = "INDF";
  memcpy(&val, id.c_str(), id.size());
  val.nbytes = sizeof(val);
}

void DoradeData::init(DoradeData::minirims_data_t &val)
{
  memset(&val, 0, sizeof(val));
  string id = "MINI";
  memcpy(&val, id.c_str(), id.size());
  val.nbytes = sizeof(val);
}

void DoradeData::init(DoradeData::nav_descript_t &val)
{
  memset(&val, 0, sizeof(val));
  string id = "NDDS";
  memcpy(&val, id.c_str(), id.size());
  val.nbytes = sizeof(val);
}

void DoradeData::init(DoradeData::time_series_t &val)
{
  memset(&val, 0, sizeof(val));
  string id = "TIME";
  memcpy(&val, id.c_str(), id.size());
  val.nbytes = sizeof(val);
}

void DoradeData::init(DoradeData::waveform_t &val)
{
  memset(&val, 0, sizeof(val));
  string id = "WAVE";
  memcpy(&val, id.c_str(), id.size());
  val.nbytes = sizeof(val);
}

///////////////////////////////////////////////////////////////
// check if a dorade structure has a valid ID

bool DoradeData::isValid(const void *ddStruct)
{
  string idStr = Radx::makeString((const char *) ddStruct, 4);
  if (idStr == "COMM" || idStr == "SSWB" || idStr == "VOLD" ||
      idStr == "RADD" || idStr == "CFAC" || idStr == "PARM" ||
      idStr == "CELV" || idStr == "CSFD" || idStr == "SWIB" ||
      idStr == "RYIB" || idStr == "RDAT" || idStr == "QDAT" ||
      idStr == "XSTF" || idStr == "FRAD" || idStr == "FRIB" ||
      idStr == "LIDR" || idStr == "FLIB" || idStr == "SITU" ||
      idStr == "ISIT" || idStr == "INDF" || idStr == "MINI" ||
      idStr == "NDDS" || idStr == "TIME" ||  idStr == "WAVE") {
    return true;
  } else {
    return false;
  }
}

bool DoradeData::isValid(const DoradeData::comment_t &val)
{
  string idStr = Radx::makeString(val.id, 4);
  if (idStr == "COMM") {
    return true;
  } else {
    return false;
  }
}

bool DoradeData::isValid(const DoradeData::super_SWIB_t &val)
{
  string idStr = Radx::makeString(val.id, 4);
  if (idStr == "SSWB") {
    return true;
  } else {
    return false;
  }
}

bool DoradeData::isValid(const DoradeData::super_SWIB_32bit_t &val)
{
  string idStr = Radx::makeString(val.id, 4);
  if (idStr == "SSWB") {
    return true;
  } else {
    return false;
  }
}

bool DoradeData::isValid(const DoradeData::volume_t &val)
{
  string idStr = Radx::makeString(val.id, 4);
  if (idStr == "VOLD") {
    return true;
  } else {
    return false;
  }
}

bool DoradeData::isValid(const DoradeData::radar_t &val)
{
  string idStr = Radx::makeString(val.id, 4);
  if (idStr == "RADD") {
    return true;
  } else {
    return false;
  }
}

bool DoradeData::isValid(const DoradeData::correction_t &val)
{
  string idStr = Radx::makeString(val.id, 4);
  if (idStr == "CFAC") {
    return true;
  } else {
    return false;
  }
}

bool DoradeData::isValid(const DoradeData::parameter_t &val)
{
  string idStr = Radx::makeString(val.id, 4);
  if (idStr == "PARM") {
    return true;
  } else {
    return false;
  }
}

bool DoradeData::isValid(const DoradeData::cell_vector_t &val)
{
  string idStr = Radx::makeString(val.id, 4);
  if (idStr == "CELV") {
    return true;
  } else {
    return false;
  }
}

bool DoradeData::isValid(const DoradeData::cell_spacing_fp_t &val)
{
  string idStr = Radx::makeString(val.id, 4);
  if (idStr == "CSFD") {
    return true;
  } else {
    return false;
  }
}

bool DoradeData::isValid(const DoradeData::sweepinfo_t &val)
{
  string idStr = Radx::makeString(val.id, 4);
  if (idStr == "SWIB") {
    return true;
  } else {
    return false;
  }
}

bool DoradeData::isValid(const DoradeData::platform_t &val)
{
  string idStr = Radx::makeString(val.id, 4);
  if (idStr == "ASIB") {
    return true;
  } else {
    return false;
  }
}

bool DoradeData::isValid(const DoradeData::ray_t &val)
{
  string idStr = Radx::makeString(val.id, 4);
  if (idStr == "RYIB") {
    return true;
  } else {
    return false;
  }
}

bool DoradeData::isValid(const DoradeData::paramdata_t &val)
{
  string idStr = Radx::makeString(val.id, 4);
  if (idStr == "RDAT") {
    return true;
  } else {
    return false;
  }
}

bool DoradeData::isValid(const DoradeData::qparamdata_t &val)
{
  string idStr = Radx::makeString(val.id, 4);
  if (idStr == "QDAT") {
    return true;
  } else {
    return false;
  }
}

bool DoradeData::isValid(const DoradeData::extra_stuff_t &val)
{
  string idStr = Radx::makeString(val.id, 4);
  if (idStr == "XSTF") {
    return true;
  } else {
    return false;
  }
}

bool DoradeData::isValid(const DoradeData::null_block_t &val)
{
  string idStr = Radx::makeString(val.id, 4);
  if (idStr == "NULL") {
    return true;
  } else {
    return false;
  }
}

bool DoradeData::isValid(const DoradeData::rot_angle_table_t &val)
{
  string idStr = Radx::makeString(val.id, 4);
  if (idStr == "RKTB") {
    return true;
  } else {
    return false;
  }
}

bool DoradeData::isValid(const DoradeData::radar_test_status_t &val)
{
  string idStr = Radx::makeString(val.id, 4);
  if (idStr == "FRAD") {
    return true;
  } else {
    return false;
  }
}

bool DoradeData::isValid(const DoradeData::field_radar_t &val)
{
  string idStr = Radx::makeString(val.id, 4);
  if (idStr == "FRIB") {
    return true;
  } else {
    return false;
  }
}

bool DoradeData::isValid(const DoradeData::lidar_t &val)
{
  string idStr = Radx::makeString(val.id, 4);
  if (idStr == "LIDR") {
    return true;
  } else {
    return false;
  }
}

bool DoradeData::isValid(const DoradeData::field_lidar_t &val)
{
  string idStr = Radx::makeString(val.id, 4);
  if (idStr == "FLIB") {
    return true;
  } else {
    return false;
  }
}

bool DoradeData::isValid(const DoradeData::insitu_descript_t &val)
{
  string idStr = Radx::makeString(val.id, 4);
  if (idStr == "SITU") {
    return true;
  } else {
    return false;
  }
}

bool DoradeData::isValid(const DoradeData::insitu_data_t &val)
{
  string idStr = Radx::makeString(val.id, 4);
  if (idStr == "ISIT") {
    return true;
  } else {
    return false;
  }
}

bool DoradeData::isValid(const DoradeData::indep_freq_t &val)
{
  string idStr = Radx::makeString(val.id, 4);
  if (idStr == "INDF") {
    return true;
  } else {
    return false;
  }
}

bool DoradeData::isValid(const DoradeData::minirims_data_t &val)
{
  string idStr = Radx::makeString(val.id, 4);
  if (idStr == "MINI") {
    return true;
  } else {
    return false;
  }
}

bool DoradeData::isValid(const DoradeData::nav_descript_t &val)
{
  string idStr = Radx::makeString(val.id, 4);
  if (idStr == "NDDS") {
    return true;
  } else {
    return false;
  }
}

bool DoradeData::isValid(const DoradeData::time_series_t &val)
{
  string idStr = Radx::makeString(val.id, 4);
  if (idStr == "TIME") {
    return true;
  } else {
    return false;
  }
}

bool DoradeData::isValid(const DoradeData::waveform_t &val)
{
  string idStr = Radx::makeString(val.id, 4);
  if (idStr == "WAVE") {
    return true;
  } else {
    return false;
  }
}

///////////////////////////////////////////////////////////////
// printing routines

// print comment block

void DoradeData::print(const DoradeData::comment_t &comm, ostream &out)
  
{
  
  out << "=============== DoradeData comment ===============" << endl;
  out << "  id: " << Radx::makeString(comm.id, 4) << endl;
  out << "  len: " << comm.nbytes << endl;
  out << "  comment text: " << comm.comment << endl;
  out << "==================================================" << endl;

}

// print super sweep indentification block

void DoradeData::print(const DoradeData::super_SWIB_t &val, ostream &out)
  
{
  
  out << "=============== DoradeData super_SWIB ===============" << endl;
  out << "  id: " << Radx::makeString(val.id, 4) << endl;
  out << "  nbytes: " << val.nbytes << endl;
  out << "  sizeof(super_SWIB_t): " << sizeof(val) << endl;
  out << "  last_used: " << RadxTime::strm(val.last_used) << endl;
  out << "  start_time: " << RadxTime::strm(val.start_time) << endl;
  out << "  stop_time: " << RadxTime::strm(val.stop_time) << endl;
  out << "  sizeof_file: " << val.sizeof_file << endl;
  out << "  compression_flag: " << val.compression_flag << endl;
  out << "  volume_time_stamp: " << val.volume_time_stamp << endl;
  out << "  num_params: " << val.num_params << endl;
  out << "  radar_name: " << Radx::makeString(val.radar_name, 8) << endl;
  out << "  d_start_time: " << val.d_start_time << endl;
  out << "  d_stop_time: " << val.d_stop_time << endl;

  /*
   * "last_used" is an age off indicator where > 0 implies Unix time
   * of the last access and
   * 0 implies this sweep should not be aged off
   */
  
  out << "  version_num: " << val.version_num << endl;
  out << "  num_key_tables: " << val.num_key_tables << endl;
  out << "  status: " << val.status << endl;

  int nTables = val.num_key_tables;
  if (nTables > MAX_KEYS) {
    nTables = MAX_KEYS;
  }
  for (int ii = 0; ii < nTables; ii++) {
    out << "  Key table num: " << ii << endl;
    out << "    offset: " << val.key_table[ii].offset << endl;
    out << "    size: " << val.key_table[ii].size << endl;
    out << "    type: " << val.key_table[ii].type << endl;
  }
  out << "=====================================================" << endl;

}

void DoradeData::print(const DoradeData::super_SWIB_32bit_t &val, ostream &out)
  
{
  
  out << "=============== DoradeData super_SWIB_32bit ==========" << endl;
  out << "  id: " << Radx::makeString(val.id, 4) << endl;
  out << "  nbytes: " << val.nbytes << endl;
  out << "  sizeof(super_SWIB_32bit_t): " << sizeof(val) << endl;
  out << "  last_used: " << RadxTime::strm(val.last_used) << endl;
  out << "  start_time: " << RadxTime::strm(val.start_time) << endl;
  out << "  stop_time: " << RadxTime::strm(val.stop_time) << endl;
  out << "  sizeof_file: " << val.sizeof_file << endl;
  out << "  compression_flag: " << val.compression_flag << endl;
  out << "  volume_time_stamp: " << val.volume_time_stamp << endl;
  out << "  num_params: " << val.num_params << endl;
  out << "  radar_name: " << Radx::makeString(val.radar_name, 8) << endl;

  Radx::fl64 d_start_time, d_stop_time;
  memcpy(&d_start_time, val.d_start_time, sizeof(d_start_time));
  memcpy(&d_stop_time, val.d_stop_time, sizeof(d_stop_time));
  out << "  d_start_time: " << d_start_time << endl;
  out << "  d_stop_time: " << d_stop_time << endl;

  /*
   * "last_used" is an age off indicator where > 0 implies Unix time
   * of the last access and
   * 0 implies this sweep should not be aged off
   */
  
  out << "  version_num: " << val.version_num << endl;
  out << "  num_key_tables: " << val.num_key_tables << endl;
  out << "  status: " << val.status << endl;

  int nTables = val.num_key_tables;
  if (nTables > MAX_KEYS) {
    nTables = MAX_KEYS;
  }
  for (int ii = 0; ii < nTables; ii++) {
    out << "  Key table num: " << ii << endl;
    out << "    offset: " << val.key_table[ii].offset << endl;
    out << "    size: " << val.key_table[ii].size << endl;
    out << "    type: " << val.key_table[ii].type << endl;
  }
  out << "=====================================================" << endl;

}

// print volume block

void DoradeData::print(const DoradeData::volume_t &val, ostream &out)
  
{
  
  out << "=============== DoradeData volume ===============" << endl;
  out << "  id: " << Radx::makeString(val.id, 4) << endl;
  out << "  nbytes: " << val.nbytes << endl;
  out << "  sizeof(volume_t): " << sizeof(val) << endl;
  out << "  format_version: " << val.format_version << endl;
  out << "  volume_num: " << val.volume_num << endl;
  out << "  maximum_bytes: " << val.maximum_bytes << endl;
  out << "  proj_name: " << Radx::makeString(val.proj_name, 20) << endl;
  out << "  year: " << val.year << endl;
  out << "  month: " << val.month << endl;
  out << "  day: " << val.day << endl;
  out << "  data_set_hour: " << val.data_set_hour << endl;
  out << "  data_set_minute: " << val.data_set_minute << endl;
  out << "  data_set_second: " << val.data_set_second << endl;
  out << "  flight_num: " << Radx::makeString(val.flight_num, 8) << endl;
  out << "  gen_facility: " << Radx::makeString(val.gen_facility, 8) << endl;
  out << "  gen_year: " << val.gen_year << endl;
  out << "  gen_month: " << val.gen_month << endl;
  out << "  gen_day: " << val.gen_day << endl;
  out << "  number_sensor_des: " << val.number_sensor_des << endl;
  out << "=================================================" << endl;

}

// print radar block

void DoradeData::print(const DoradeData::radar_t &val, ostream &out)
  
{
  
  out << "=============== DoradeData radar ===============" << endl;
  out << "  id: " << Radx::makeString(val.id, 4) << endl;
  out << "  nbytes: " << val.nbytes << endl;
  out << "  sizeof(radar_t): " << sizeof(val) << endl;
  out << "  radar_name: " << Radx::makeString(val.radar_name, 8) << endl;
  out << "  radar_const: " << val.radar_const << endl;
  out << "  peak_power: " << val.peak_power << endl;
  out << "  noise_power: " << val.noise_power << endl;
  out << "  receiver_gain: " << val.receiver_gain << endl;
  out << "  antenna_gain: " << val.antenna_gain << endl;
  out << "  system_gain: " << val.system_gain << endl;
  out << "  horz_beam_width: " << val.horz_beam_width << endl;
  out << "  vert_beam_width: " << val.vert_beam_width << endl;
  out << "  radar_type: " << radarTypeToStr((radar_type_t) val.radar_type) << endl;
  out << "  scan_mode: " << scanModeToStr((scan_mode_t) val.scan_mode) << endl;
  out << "  req_rotat_vel: " << val.req_rotat_vel << endl;
  out << "  scan_mode_pram0: " << scanModeToStr((scan_mode_t) val.scan_mode_pram0) << endl;
  out << "  scan_mode_pram1: " << scanModeToStr((scan_mode_t) val.scan_mode_pram1) << endl;
  out << "  num_parameter_des: " << val.num_parameter_des << endl;
  out << "  total_num_des: " << val.total_num_des << endl;
  out << "  data_compress: " << val.data_compress << endl;
  out << "  data_reduction: " << val.data_reduction << endl;
  out << "  data_red_parm0: " << val.data_red_parm0 << endl;
  out << "  data_red_parm1: " << val.data_red_parm1 << endl;
  out << "  radar_longitude: " << val.radar_longitude << endl;
  out << "  radar_latitude: " << val.radar_latitude << endl;
  out << "  radar_altitude: " << val.radar_altitude << endl;
  out << "  eff_unamb_vel: " << val.eff_unamb_vel << endl;
  out << "  eff_unamb_range: " << val.eff_unamb_range << endl;
  out << "  num_freq_trans: " << val.num_freq_trans << endl;
  out << "  num_ipps_trans: " << val.num_ipps_trans << endl;
  out << "  freq1: " << val.freq1 << endl;
  out << "  freq2: " << val.freq2 << endl;
  out << "  freq3: " << val.freq3 << endl;
  out << "  freq4: " << val.freq4 << endl;
  out << "  freq5: " << val.freq5 << endl;
  out << "  prt1: " << val.prt1 << endl;
  out << "  prt2: " << val.prt2 << endl;
  out << "  prt3: " << val.prt3 << endl;
  out << "  prt4: " << val.prt4 << endl;
  out << "  prt5: " << val.prt5 << endl;
  out << "  extension_num: " << val.extension_num << endl;
  out << "  config_name: " << Radx::makeString(val.config_name, 8) << endl;
  out << "  config_num: " << val.config_num << endl;
  out << "  aperture_size: " << val.aperture_size << endl;
  out << "  field_of_view: " << val.field_of_view << endl;
  out << "  aperture_eff: " << val.aperture_eff << endl;
  for (int ii = 0; ii < 11; ii++) {
    out << "  aux_freq[" << ii << "]: " << val.aux_freq[ii] << endl;
    out << "  aux_prt[" << ii << "]: " << val.aux_prt[ii] << endl;
  }
  out << "  pulse_width (us): " << val.pulse_width << endl;
  out << "  primary_cop_baseln: " << val.primary_cop_baseln << endl;
  out << "  secondary_cop_baseln: " << val.secondary_cop_baseln << endl;
  out << "  pc_xmtr_bandwidth: " << val.pc_xmtr_bandwidth << endl;
  out << "  pc_waveform_type: " << val.pc_waveform_type << endl;
  out << "  site_name: " << Radx::makeString(val.site_name, 20) << endl;

  out << "================================================" << endl;

}

// print correction factor block

void DoradeData::print(const DoradeData::correction_t &val, ostream &out)
  
{
  
  out << "=============== DoradeData correction ===============" << endl;
  out << "  id: " << Radx::makeString(val.id, 4) << endl;
  out << "  nbytes: " << val.nbytes << endl;
  out << "  sizeof(correction_t): " << sizeof(val) << endl;
  out << "  azimuth_corr: " << val.azimuth_corr << endl;
  out << "  elevation_corr: " << val.elevation_corr << endl;
  out << "  range_delay_corr: " << val.range_delay_corr << endl;
  out << "  longitude_corr: " << val.longitude_corr << endl;
  out << "  latitude_corr: " << val.latitude_corr << endl;
  out << "  pressure_alt_corr: " << val.pressure_alt_corr << endl;
  out << "  radar_alt_corr: " << val.radar_alt_corr << endl;
  out << "  ew_gndspd_corr: " << val.ew_gndspd_corr << endl;
  out << "  ns_gndspd_corr: " << val.ns_gndspd_corr << endl;
  out << "  vert_vel_corr: " << val.vert_vel_corr << endl;
  out << "  heading_corr: " << val.heading_corr << endl;
  out << "  roll_corr: " << val.roll_corr << endl;
  out << "  pitch_corr: " << val.pitch_corr << endl;
  out << "  drift_corr: " << val.drift_corr << endl;
  out << "  rot_angle_corr: " << val.rot_angle_corr << endl;
  out << "  tilt_corr: " << val.tilt_corr << endl;
  out << "=====================================================" << endl;

}

// print parameter block

void DoradeData::print(const DoradeData::parameter_t &val, ostream &out)
  
{
  
  out << "=============== DoradeData parameter ===============" << endl;
  out << "  id: " << Radx::makeString(val.id, 4) << endl;
  out << "  nbytes: " << val.nbytes << endl;
  out << "  sizeof(paramter_t): " << sizeof(val) << endl;
  out << "  parameter_name: " << Radx::makeString(val.parameter_name, 8) << endl;
  out << "  param_description: " << Radx::makeString(val.param_description, 40) << endl;
  out << "  param_units: " << Radx::makeString(val.param_units, 8) << endl;
  out << "  interpulse_time: " << val.interpulse_time << endl;
  out << "  xmitted_freq: " << val.xmitted_freq << endl;
  out << "  recvr_bandwidth: " << val.recvr_bandwidth << endl;
  out << "  pulse_width(m): " << val.pulse_width << endl;
  out << "  polarization: " << val.polarization << endl;
  out << "  num_samples: " << val.num_samples << endl;
  out << "  binary_format: " << binaryFormatToStr((binary_format_t) val.binary_format) << endl;
  out << "  threshold_field: " << Radx::makeString(val.threshold_field, 8) << endl;
  out << "  threshold_value: " << val.threshold_value << endl;
  out << "  parameter_scale: " << val.parameter_scale << endl;
  out << "  parameter_bias: " << val.parameter_bias << endl;
  out << "  bad_data: " << val.bad_data << endl;
  out << "  extension_num: " << val.extension_num << endl;
  out << "  config_name: " << Radx::makeString(val.config_name, 8) << endl;
  out << "  config_num: " << val.config_num << endl;
  out << "  offset_to_data: " << val.offset_to_data << endl;
  out << "  mks_conversion: " << val.mks_conversion << endl;
  out << "  num_qnames: " << val.num_qnames << endl;
  out << "  qdata_names: " << Radx::makeString(val.qdata_names, 32) << endl;
  out << "  num_criteria: " << val.num_criteria << endl;
  out << "  criteria_names: " << Radx::makeString(val.criteria_names, 32) << endl;
  out << "  number_cells: " << val.number_cells << endl;
  out << "  meters_to_first_cell: " << val.meters_to_first_cell << endl;
  out << "  meters_between_cells: " << val.meters_between_cells << endl;
  out << "  eff_unamb_vel: " << val.eff_unamb_vel << endl;
  out << "====================================================" << endl;
  
}

// print cell block

void DoradeData::print(const DoradeData::cell_vector_t &val, ostream &out)
  
{
  
  out << "=============== DoradeData cell ===============" << endl;
  out << "  id: " << Radx::makeString(val.id, 4) << endl;
  out << "  nbytes: " << val.nbytes << endl;
  out << "  sizeof(cell_vector_t): " << sizeof(val) << endl;
  out << "  number_cells: " << val.number_cells << endl;
  int nn = val.number_cells;
  if (nn > MAXCVGATES) {
    nn = MAXCVGATES;
  }
  for (int ii = 0; ii < nn; ii++) {
    out << "  cell distance[" << ii << "]: " << val.dist_cells[ii] << endl;
  }
  out << "===============================================" << endl;

}

// print cell spacing block

void DoradeData::print(const DoradeData::cell_spacing_fp_t &val, ostream &out)
  
{
  
  out << "=============== DoradeData cell spacing ===============" << endl;
  out << "  id: " << Radx::makeString(val.id, 4) << endl;
  out << "  nbytes: " << val.nbytes << endl;
  out << "  sizeof(cell_spacing_fp_t): " << sizeof(val) << endl;
  out << "  num_segments: " << val.num_segments << endl;
  out << "  dist_to_first: " << val.dist_to_first << endl;
  int nn = val.num_segments;
  if (nn > 8) {
    nn = 8;
  }
  for (int ii = 0; ii < nn; ii++) {
    out << "  spacing[" << ii << "]: " << val.spacing[ii] << endl;
    out << "  num_cells[" << ii << "]: " << val.num_cells[ii] << endl;
  }
  out << "=======================================================" << endl;

}

// print sweep info block

void DoradeData::print(const DoradeData::sweepinfo_t &val, ostream &out)
  
{
  
  out << "=============== DoradeData sweepinfo ===============" << endl;
  out << "  id: " << Radx::makeString(val.id, 4) << endl;
  out << "  nbytes: " << val.nbytes << endl;
  out << "  sizeof(sweepinfo_t): " << sizeof(val) << endl;
  out << "  radar_name: " << Radx::makeString(val.radar_name, 8) << endl;
  out << "  sweep_num: " << val.sweep_num << endl;
  out << "  num_rays: " << val.num_rays << endl;
  out << "  start_angle: " << val.start_angle << endl;
  out << "  stop_angle: " << val.stop_angle << endl;
  out << "  fixed_angle: " << val.fixed_angle << endl;
  out << "  filter_flag: " << val.filter_flag << endl;
  out << "====================================================" << endl;

}

// print platform block

void DoradeData::print(const DoradeData::platform_t &val, ostream &out)
  
{
  
  out << "=============== DoradeData platform ===============" << endl;
  out << "  id: " << Radx::makeString(val.id, 4) << endl;
  out << "  nbytes: " << val.nbytes << endl;
  out << "  sizeof(platform_t): " << sizeof(val) << endl;
  out << "  longitude: " << val.longitude << endl;
  out << "  latitude: " << val.latitude << endl;
  out << "  altitude_msl: " << val.altitude_msl << endl;
  out << "  altitude_agl: " << val.altitude_agl << endl;
  out << "  ew_velocity: " << val.ew_velocity << endl;
  out << "  ns_velocity: " << val.ns_velocity << endl;
  out << "  vert_velocity: " << val.vert_velocity << endl;
  out << "  heading: " << val.heading << endl;
  out << "  roll: " << val.roll << endl;
  out << "  pitch: " << val.pitch << endl;
  out << "  drift_angle: " << val.drift_angle << endl;
  out << "  rotation_angle: " << val.rotation_angle << endl;
  out << "  tilt: " << val.tilt << endl;
  out << "  ew_horiz_wind: " << val.ew_horiz_wind << endl;
  out << "  ns_horiz_wind: " << val.ns_horiz_wind << endl;
  out << "  vert_wind: " << val.vert_wind << endl;
  out << "  heading_change: " << val.heading_change << endl;
  out << "  pitch_change: " << val.pitch_change << endl;
  out << "===================================================" << endl;

}

// print ray block

void DoradeData::print(const DoradeData::ray_t &val, ostream &out)
  
{
  
  out << "=============== DoradeData ray ===============" << endl;
  out << "  id: " << Radx::makeString(val.id, 4) << endl;
  out << "  nbytes: " << val.nbytes << endl;
  out << "  sizeof(ray_t): " << sizeof(val) << endl;
  out << "  sweep_num: " << val.sweep_num << endl;
  out << "  julian_day: " << val.julian_day << endl;
  out << "  hour: " << val.hour << endl;
  out << "  minute: " << val.minute << endl;
  out << "  second: " << val.second << endl;
  out << "  millisecond: " << val.millisecond << endl;
  out << "  azimuth: " << val.azimuth << endl;
  out << "  elevation: " << val.elevation << endl;
  out << "  peak_power: " << val.peak_power << endl;
  out << "  true_scan_rate: " << val.true_scan_rate << endl;
  out << "  ray_status: " << val.ray_status << endl;
  out << "==============================================" << endl;

}

// print paramdata block

void DoradeData::print(const DoradeData::paramdata_t &val, ostream &out)
  
{
  
  out << "=============== DoradeData paramdata ===============" << endl;
  out << "  id: " << Radx::makeString(val.id, 4) << endl;
  out << "  nbytes: " << val.nbytes << endl;
  out << "  sizeof(paramdata_t): " << sizeof(val) << endl;
  out << "  pdata_name: " << Radx::makeString(val.pdata_name, 8) << endl;
  out << "====================================================" << endl;

}

// print qparamdata block

void DoradeData::print(const DoradeData::qparamdata_t &val, ostream &out)
  
{
  
  out << "=============== DoradeData qparamdata ===============" << endl;
  out << "  id: " << Radx::makeString(val.id, 4) << endl;
  out << "  nbytes: " << val.nbytes << endl;
  out << "  sizeof(qparamdata_t): " << sizeof(val) << endl;
  out << "  pdata_name: " << Radx::makeString(val.pdata_name, 8) << endl;
  out << "  extension_num: " << val.extension_num << endl;
  out << "  config_num: " << val.config_num << endl;
  for (int ii = 0; ii < 4; ii++) {
    out << "  first_cell[" << ii << "]: " << val.first_cell[ii] << endl;
    out << "  num_cells[" << ii << "]: " << val.num_cells[ii] << endl;
    out << "  criteria_value[" << ii << "]: " << val.criteria_value[ii] << endl;
  }
  out << "=====================================================" << endl;

}

// print extra stuff block

void DoradeData::print(const DoradeData::extra_stuff_t &val, ostream &out)
  
{
  
  out << "=============== DoradeData extra_stuff ===============" << endl;
  out << "  id: " << Radx::makeString(val.id, 4) << endl;
  out << "  nbytes: " << val.nbytes << endl;
  out << "  sizeof(extra_stuff_t): " << sizeof(val) << endl;
  out << "  one: " << val.one << endl;
  out << "  source_format: " << val.source_format << endl;
  out << "  offset_to_first_item: " << val.offset_to_first_item << endl;
  out << "  transition_flag: " << val.transition_flag << endl;
  out << "======================================================" << endl;

}

// print null block

void DoradeData::print(const DoradeData::null_block_t &val, ostream &out)
  
{
  
  out << "================ DoradeData null block ================" << endl;
  out << "  id: " << Radx::makeString(val.id, 4) << endl;
  out << "  nbytes: " << val.nbytes << endl;
  out << "=======================================================" << endl;

}

// print rotation angle table block

void DoradeData::print(const DoradeData::rot_angle_table_t &val, ostream &out)
  
{
  
  out << "=============== DoradeData rot angle table ===============" << endl;
  out << "  id: " << Radx::makeString(val.id, 4) << endl;
  out << "  nbytes: " << val.nbytes << endl;
  out << "  sizeof(rot_angle_table_t): " << sizeof(val) << endl;
  out << "  angle2ndx: " << val.angle2ndx << endl;
  out << "  ndx_que_size: " << val.ndx_que_size << endl;
  out << "  first_key_offset: " << val.first_key_offset << endl;
  out << "  angle_table_offset: " << val.angle_table_offset << endl;
  out << "  num_rays: " << val.num_rays << endl;
  out << "==========================================================" << endl;

}

void DoradeData::print(const DoradeData::rot_table_entry_t &val, ostream &out)
  
{
  
  out << "=============== DoradeData rot table entry ===============" << endl;
  out << "  rotation_angle: " << val.rotation_angle << endl;
  out << "  offset: " << val.offset << endl;
  out << "  size: " << val.size << endl;
  out << "==========================================================" << endl;

}

// print radar_test_status_t block

void DoradeData::print(const DoradeData::radar_test_status_t &val, ostream &out)

{

  out << "============ DoradeData radar_test_status_t =============" << endl;
  out << "  id: " << Radx::makeString(val.id, 4) << endl;
  out << "  nbytes: " << val.nbytes << endl;
  out << "  data_sys_status: " << val.data_sys_status << endl;
  out << "  radar_name: " << Radx::makeString(val.radar_name, 8) << endl;
  out << "  test_pulse_level: " << val.test_pulse_level << endl;
  out << "  test_pulse_dist: " << val.test_pulse_dist << endl;
  out << "  test_pulse_width: " << val.test_pulse_width << endl;
  out << "  test_pulse_freq: " << val.test_pulse_freq << endl;
  out << "  test_pulse_atten: " << val.test_pulse_atten << endl;
  out << "  test_pulse_fnum: " << val.test_pulse_fnum << endl;
  out << "  noise_power: " << val.noise_power << endl;
  out << "  ray_count: " << val.ray_count << endl;
  out << "  first_rec_gate: " << val.first_rec_gate << endl;
  out << "  last_rec_gate: " << val.last_rec_gate << endl;
  out << "=======================================================" << endl;

}

// print field_radar_t block

void DoradeData::print(const DoradeData::field_radar_t &val, ostream &out)

{

  out << "============ DoradeData field_radar_t =============" << endl;
  out << "  id: " << Radx::makeString(val.id, 4) << endl;
  out << "  nbytes: " << val.nbytes << endl;
  out << "  data_sys_id: " << val.data_sys_id << endl;
  out << "  loss_out: " << val.loss_out << endl;
  out << "  loss_in: " << val.loss_in << endl;
  out << "  loss_rjoint: " << val.loss_rjoint << endl;
  out << "  ant_v_dim: " << val.ant_v_dim << endl;
  out << "  ant_h_dim: " << val.ant_h_dim << endl;
  out << "  ant_noise_temp: " << val.ant_noise_temp << endl;
  out << "  r_noise_figure: " << val.r_noise_figure << endl;
  for (int ii = 0; ii < 5; ii++) {
    out << "  xmit_power[" << ii << "]: " << val.xmit_power[ii] << endl;
  }
  out << "  x_band_gain: " << val.x_band_gain << endl;
  for (int ii = 0; ii < 5; ii++) {
    out << "  receiver_gain[" << ii << "]: " << val.receiver_gain[ii] << endl;
  }
  for (int ii = 0; ii < 5; ii++) {
    out << "  if_gain[" << ii << "]: " << val.if_gain[ii] << endl;
  }
  out << "  conversion_gain: " << val.conversion_gain << endl;
  for (int ii = 0; ii < 5; ii++) {
    out << "  scale_factor[" << ii << "]: " << val.scale_factor[ii] << endl;
  }
  out << "  processor_const: " << val.processor_const << endl;
  out << "  dly_tube_antenna: " << val.dly_tube_antenna << endl;
  out << "  dly_rndtrip_chip_atod: " << val.dly_rndtrip_chip_atod << endl;
  out << "  dly_timmod_testpulse: " << val.dly_timmod_testpulse << endl;
  out << "  dly_modulator_on: " << val.dly_modulator_on << endl;
  out << "  dly_modulator_off: " << val.dly_modulator_off << endl;
  out << "  peak_power_offset: " << val.peak_power_offset << endl;
  out << "  test_pulse_offset: " << val.test_pulse_offset << endl;
  out << "  E_plane_angle: " << val.E_plane_angle << endl;
  out << "  H_plane_angle: " << val.H_plane_angle << endl;
  out << "  encoder_antenna_up: " << val.encoder_antenna_up << endl;
  out << "  pitch_antenna_up: " << val.pitch_antenna_up << endl;
  out << "  indepf_times_flg: " << val.indepf_times_flg << endl;
  out << "  indep_freq_gate: " << val.indep_freq_gate << endl;
  out << "  time_series_gate: " << val.time_series_gate << endl;
  out << "  num_base_params: " << val.num_base_params << endl;
  out << "  file_name: " << Radx::makeString(val. file_name, 80) << endl;
  out << "===================================================" << endl;

}

// print lidar_t block

void DoradeData::print(const DoradeData::lidar_t &val, ostream &out)

{

  out << "============ DoradeData lidar_t =============" << endl;
  out << "  id: " << Radx::makeString(val.id, 4) << endl;
  out << "  nbytes: " << val.nbytes << endl;
  out << "  lidar_name: " << Radx::makeString(val.lidar_name, 8) << endl;
  out << "  lidar_const: " << val.lidar_const << endl;
  out << "  pulse_energy: " << val.pulse_energy << endl;
  out << "  peak_power: " << val.peak_power << endl;
  out << "  pulse_width: " << val.pulse_width << endl;
  out << "  aperture_size: " << val.aperture_size << endl;
  out << "  field_of_view: " << val.field_of_view << endl;
  out << "  aperture_eff: " << val.aperture_eff << endl;
  out << "  beam_divergence: " << val.beam_divergence << endl;
  out << "  lidar_type: " << val.lidar_type << endl;
  out << "  scan_mode: " << val.scan_mode << endl;
  out << "  req_rotat_vel: " << val.req_rotat_vel << endl;
  out << "  scan_mode_pram0: " << val.scan_mode_pram0 << endl;
  out << "  scan_mode_pram1: " << val.scan_mode_pram1 << endl;
  out << "  num_parameter_des: " << val.num_parameter_des << endl;
  out << "  total_num_des: " << val.total_num_des << endl;
  out << "  data_compress: " << val.data_compress << endl;
  out << "  data_reduction: " << val.data_reduction << endl;
  out << "  data_red_parm0: " << val.data_red_parm0 << endl;
  out << "  data_red_parm1: " << val.data_red_parm1 << endl;
  out << "  lidar_longitude: " << val.lidar_longitude << endl;
  out << "  lidar_latitude: " << val.lidar_latitude << endl;
  out << "  lidar_altitude: " << val.lidar_altitude << endl;
  out << "  eff_unamb_vel: " << val.eff_unamb_vel << endl;
  out << "  eff_unamb_range: " << val.eff_unamb_range << endl;
  out << "  num_wvlen_trans: " << val.num_wvlen_trans << endl;
  out << "  prf: " << val.prf << endl;
  for (int ii = 0; ii < 10; ii++) {
    out << "  wavelength[" << ii << "]: " << val.wavelength[ii] << endl;
  }
  out << "=============================================" << endl;

}

// print field_lidar_t block

void DoradeData::print(const DoradeData::field_lidar_t &val, ostream &out)

{

  out << "============ DoradeData field_lidar_t =============" << endl;
  out << "  id: " << Radx::makeString(val.id, 4) << endl;
  out << "  nbytes: " << val.nbytes << endl;
  out << "  data_sys_id: " << val.data_sys_id << endl;
  for (int ii = 0; ii < 10; ii++) {
    out << "  transmit_beam_div[" << ii << "]: " << val.transmit_beam_div[ii] << endl;
  }
  for (int ii = 0; ii < 10; ii++) {
    out << "  xmit_power[" << ii << "]: " << val.xmit_power[ii] << endl;
  }
  for (int ii = 0; ii < 10; ii++) {
    out << "  receiver_fov[" << ii << "]: " << val.receiver_fov[ii] << endl;
  }
  for (int ii = 0; ii < 10; ii++) {
    out << "  receiver_type[" << ii << "]: " << val.receiver_type[ii] << endl;
  }
  for (int ii = 0; ii < 10; ii++) {
    out << "  r_noise_floor[" << ii << "]: " << val.r_noise_floor[ii] << endl;
  }
  for (int ii = 0; ii < 10; ii++) {
    out << "  receiver_spec_bw[" << ii << "]: " << val.receiver_spec_bw[ii] << endl;
  }
  for (int ii = 0; ii < 10; ii++) {
    out << "  receiver_elec_bw[" << ii << "]: " << val.receiver_elec_bw[ii] << endl;
  }
  for (int ii = 0; ii < 10; ii++) {
    out << "  calibration[" << ii << "]: " << val.calibration[ii] << endl;
  }
  out << "  range_delay: " << val.range_delay << endl;
  for (int ii = 0; ii < 10; ii++) {
    out << "  peak_power_multi[" << ii << "]: " << val.peak_power_multi[ii] << endl;
  }
  out << "  encoder_mirror_up: " << val.encoder_mirror_up << endl;
  out << "  pitch_mirror_up: " << val.pitch_mirror_up << endl;
  out << "  max_digitizer_count: " << val.max_digitizer_count << endl;
  out << "  max_digitizer_volt: " << val.max_digitizer_volt << endl;
  out << "  digitizer_rate: " << val.digitizer_rate << endl;
  out << "  total_num_samples: " << val.total_num_samples << endl;
  out << "  samples_per_cell: " << val.samples_per_cell << endl;
  out << "  cells_per_ray: " << val.cells_per_ray << endl;
  out << "  pmt_temp: " << val.pmt_temp << endl;
  out << "  pmt_gain: " << val.pmt_gain << endl;
  out << "  apd_temp: " << val.apd_temp << endl;
  out << "  apd_gain: " << val.apd_gain << endl;
  out << "  transect: " << val.transect << endl;
  for (int ii = 0; ii < 10; ii++) {
    out << "derived_names[" << ii << "]: "
         << Radx::makeString(val.derived_names[ii], 12) << endl;
    out << "derived_units[" << ii << "]: "
         << Radx::makeString(val.derived_units[ii], 8) << endl;
    out << "temp_names[" << ii << "]: "
         << Radx::makeString(val.temp_names[ii], 12) << endl;
  }
  out << "===================================================" << endl;

}

// print insitu_descript_t block

void DoradeData::print(const DoradeData::insitu_descript_t &val, ostream &out)

{

  out << "============ DoradeData insitu_descript_t =============" << endl;
  out << "  id: " << Radx::makeString(val.id, 4) << endl;
  out << "  nbytes: " << val.nbytes << endl;
  out << "  number_params: " << val.number_params << endl;
  for (int ii = 0; ii < val.number_params; ii++) {
    out << "  parameter[" << ii << "] name: "
        << Radx::makeString(val.params[ii].name, 8) << endl;
    out << "  parameter[" << ii << "] units: "
        << Radx::makeString(val.params[ii].units, 8) << endl;
  }
  out << "=======================================================" << endl;

}

// print insitu_data_t block

void DoradeData::print(const DoradeData::insitu_data_t &val, ostream &out)

{

  out << "============ DoradeData insitu_data_t =============" << endl;
  out << "  id: " << Radx::makeString(val.id, 4) << endl;
  out << "  nbytes: " << val.nbytes << endl;
  out << "  julian_day: " << val.julian_day << endl;
  out << "  hours: " << val.hours << endl;
  out << "  minutes: " << val.minutes << endl;
  out << "  seconds: " << val.seconds << endl;
  out << "===================================================" << endl;

}

// print indep_freq_t block

void DoradeData::print(const DoradeData::indep_freq_t &val, ostream &out)

{

  out << "============ DoradeData indep_freq_t =============" << endl;
  out << "  id: " << Radx::makeString(val.id, 4) << endl;
  out << "  nbytes: " << val.nbytes << endl;
  out << "==================================================" << endl;

}

// print minirims_data_t block

void DoradeData::print(const DoradeData::minirims_data_t &val, ostream &out)

{

  out << "============ DoradeData minirims_data_t =============" << endl;
  out << "  id: " << Radx::makeString(val.id, 4) << endl;
  out << "  nbytes: " << val.nbytes << endl;
  out << "  command: " << val.command << endl;
  out << "  status: " << val.status << endl;
  out << "  temperature: " << val.temperature << endl;
  for (int ii = 0; ii < 128; ii++) {
    out << "  x_axis_gyro[" << ii << "]: " << val.x_axis_gyro[ii] << endl;
    out << "  y_axis_gyro[" << ii << "]: " << val.y_axis_gyro[ii] << endl;
    out << "  z_axis_gyro[" << ii << "]: " << val.z_axis_gyro[ii] << endl;
    out << "  xr_axis_gyro[" << ii << "]: " << val.xr_axis_gyro[ii] << endl;
    out << "  x_axis_vel[" << ii << "]: " << val.x_axis_vel[ii] << endl;
    out << "  y_axis_vel[" << ii << "]: " << val.y_axis_vel[ii] << endl;
    out << "  z_axis_vel[" << ii << "]: " << val.z_axis_vel[ii] << endl;
    out << "  x_axis_pos[" << ii << "]: " << val.x_axis_pos[ii] << endl;
  }
  out << "=====================================================" << endl;

}

// print nav_descript_t block

void DoradeData::print(const DoradeData::nav_descript_t &val, ostream &out)

{

  out << "============ DoradeData nav_descript_t =============" << endl;
  out << "  id: " << Radx::makeString(val.id, 4) << endl;
  out << "  nbytes: " << val.nbytes << endl;
  out << "  ins_flag: " << val.ins_flag << endl;
  out << "  gps_flag: " << val.gps_flag << endl;
  out << "  minirims_flag: " << val.minirims_flag << endl;
  out << "  kalman_flag: " << val.kalman_flag << endl;
  out << "====================================================" << endl;

}

// print time_series_t block

void DoradeData::print(const DoradeData::time_series_t &val, ostream &out)

{

  out << "============ DoradeData time_series_t =============" << endl;
  out << "  id: " << Radx::makeString(val.id, 4) << endl;
  out << "  nbytes: " << val.nbytes << endl;
  out << "===================================================" << endl;

}

// print waveform_t block

void DoradeData::print(const DoradeData::waveform_t &val, ostream &out)

{

  out << "============ DoradeData waveform_t =============" << endl;
  out << "  id: " << Radx::makeString(val.id, 4) << endl;
  out << "  nbytes: " << val.nbytes << endl;
  out << "  ps_file_name: " << Radx::makeString(val.ps_file_name, 16) << endl;
  for (int ii = 0; ii < 6; ii++) {
    out << "  num_chips[" << ii << "]: " << val.num_chips[ii] << endl;
  }
  out << "  blank_chip: " << Radx::makeString(val.blank_chip, 256) << endl;
  out << "  repeat_seq: " << val.repeat_seq << endl;
  out << "  repeat_seq_dwel: " << val.repeat_seq_dwel << endl;
  out << "  total_pcp: " << val.total_pcp << endl;
  for (int ii = 0; ii < 6; ii++) {
    out << "  chip_offset[" << ii << "]: " << val.chip_offset[ii] << endl;
  }
  for (int ii = 0; ii < 6; ii++) {
    out << "  chip_width[" << ii << "]: " << val.chip_width[ii] << endl;
  }
  out << "  ur_pcp: " << val.ur_pcp << endl;
  out << "  uv_pcp: " << val.uv_pcp << endl;
  for (int ii = 0; ii < 6; ii++) {
    out << "  num_gates[" << ii << "]: " << val.num_gates[ii] << endl;
  }
  for (int ii = 0; ii < 2; ii++) {
    out << "  gate_dist1[" << ii << "]: " << val.gate_dist1[ii] << endl;
  }
  for (int ii = 0; ii < 2; ii++) {
    out << "  gate_dist2[" << ii << "]: " << val.gate_dist2[ii] << endl;
  }
  for (int ii = 0; ii < 2; ii++) {
    out << "  gate_dist3[" << ii << "]: " << val.gate_dist3[ii] << endl;
  }
  for (int ii = 0; ii < 2; ii++) {
    out << "  gate_dist4[" << ii << "]: " << val.gate_dist4[ii] << endl;
  }
  for (int ii = 0; ii < 2; ii++) {
    out << "  gate_dist5[" << ii << "]: " << val.gate_dist5[ii] << endl;
  }
  out << "================================================" << endl;

}

///////////////////////////////////////////////////////////////
// routines for printing format of structs

const char *DoradeData::_hform = "%9s %24s %7s %7s\n";
const char *DoradeData::_dform = "%9s %24s %7d %7d\n";

/// Print format of all DORADE structs

void DoradeData::printAllFormats(FILE *out)
{

  fprintf(out, "================= DORADE FORMAT ==================\n");

  DoradeData::comment_t comment;
  DoradeData::printFormat(comment, out);

  DoradeData::super_SWIB_t swib;
  DoradeData::printFormat(swib, out);

  DoradeData::volume_t volume;
  DoradeData::printFormat(volume, out);

  DoradeData::radar_t radar;
  DoradeData::printFormat(radar, out);

  DoradeData::correction_t correction;
  DoradeData::printFormat(correction, out);

  DoradeData::parameter_t parameter;
  DoradeData::printFormat(parameter, out);

  DoradeData::cell_vector_t cell_vector;
  DoradeData::printFormat(cell_vector, out);

  DoradeData::cell_spacing_fp_t cell_spacing;
  DoradeData::printFormat(cell_spacing, out);

  DoradeData::sweepinfo_t sweepinfo;
  DoradeData::printFormat(sweepinfo, out);

  DoradeData::platform_t platform;
  DoradeData::printFormat(platform, out);

  DoradeData::ray_t ray;
  DoradeData::printFormat(ray, out);

  DoradeData::paramdata_t paramdata;
  DoradeData::printFormat(paramdata, out);

  DoradeData::qparamdata_t qparamdata;
  DoradeData::printFormat(qparamdata, out);

  DoradeData::extra_stuff_t extra_stuff;
  DoradeData::printFormat(extra_stuff, out);

  DoradeData::null_block_t null_block;
  DoradeData::printFormat(null_block, out);

  DoradeData::rot_angle_table_t rot_angle_table;
  DoradeData::printFormat(rot_angle_table, out);

  DoradeData::rot_table_entry_t rot_table_entry;
  DoradeData::printFormat(rot_table_entry, out);

  DoradeData::radar_test_status_t radar_test_status;
  DoradeData::printFormat(radar_test_status, out);

  DoradeData::field_radar_t field_radar;
  DoradeData::printFormat(field_radar, out);

  DoradeData::lidar_t lidar;
  DoradeData::printFormat(lidar, out);

  DoradeData::field_lidar_t field_lidar;
  DoradeData::printFormat(field_lidar, out);

  DoradeData::insitu_descript_t insitu_descript;
  DoradeData::printFormat(insitu_descript, out);

  DoradeData::insitu_data_t insitu_data;
  DoradeData::printFormat(insitu_data, out);

  DoradeData::indep_freq_t indep_freq;
  DoradeData::printFormat(indep_freq, out);

  DoradeData::minirims_data_t minirims_data;
  DoradeData::printFormat(minirims_data, out);

  DoradeData::nav_descript_t nav_descript;
  DoradeData::printFormat(nav_descript, out);

  DoradeData::time_series_t time_series;
  DoradeData::printFormat(time_series, out);

  DoradeData::waveform_t waveform;
  DoradeData::printFormat(waveform, out);

  _printFormatDivider('=', out);

}

void DoradeData::_printFormatDivider(char val, FILE *out)
{
  for (int ii = 0; ii < _formDividerLen; ii++) {
    fprintf(out, "%c", val);
  }
  fprintf(out, "\n");
}
  
void DoradeData::_printFormatHeader(FILE *out)
{
  fprintf(out, _hform, "type", "name", "size", "offset");
  fprintf(out, _hform, "----", "----", "----", "------");
}
  
// print format of comment block

void DoradeData::printFormat(const DoradeData::comment_t &val, FILE *out)
  
{
  
  _printFormatDivider('-', out);
  fprintf(out, "  struct: 'comment_t'\n  size: %d\n  id: COMM\n\n", (int) sizeof(val));
  const char *id = val.id;
  _printFormatHeader(out);

  fprintf(out, _dform, "char", "id[4]", (int) sizeof(val.id), (char *) &val.id - id);
  fprintf(out, _dform, "si32", "nbytes", (int) sizeof(val.nbytes), (char *) &val.nbytes - id);
  fprintf(out, _dform, "char", "comment[500]", (int) sizeof(val.comment), (char *) val.comment - id);

  _printFormatDivider('-', out);

}

// print format of super sweep indentification block

void DoradeData::printFormat(const DoradeData::super_SWIB_t &val, FILE *out)
  
{
  
  _printFormatDivider('-', out);
  fprintf(out, "  struct: 'super_SWIB_t'\n  size: %d\n  id: SSWB\n\n", (int) sizeof(val));
  const char *id = val.id;
  _printFormatHeader(out);

  fprintf(out, _dform, "char", "id[4]", (int) sizeof(val.id), (char *) &val.id - id);
  fprintf(out, _dform, "si32", "nbytes", (int) sizeof(val.nbytes), (char *) &val.nbytes - id);
  fprintf(out, _dform, "si32", "last_used", (int) sizeof(val.last_used), (char *) &val.last_used - id);
  fprintf(out, _dform, "si32", "start_time", (int) sizeof(val.start_time), (char *) &val.start_time - id);
  fprintf(out, _dform, "si32", "stop_time", (int) sizeof(val.stop_time), (char *) &val.stop_time - id);
  fprintf(out, _dform, "si32", "sizeof_file", (int) sizeof(val.sizeof_file), (char *) &val.sizeof_file - id);
  fprintf(out, _dform, "si32", "compression_flag", (int) sizeof(val.compression_flag), (char *) &val.compression_flag - id);
  fprintf(out, _dform, "si32", "volume_time_stamp", (int) sizeof(val.volume_time_stamp), (char *) &val.volume_time_stamp - id);
  fprintf(out, _dform, "si32", "num_params", (int) sizeof(val.num_params), (char *) &val.num_params - id);
  fprintf(out, _dform, "char", "radar_name[8]", (int) sizeof(val.radar_name), (char *) val.radar_name - id);
  fprintf(out, _dform, "si32", "pad", (int) sizeof(val.pad), (char *) &val.pad - id);
  fprintf(out, _dform, "fl64", "d_start_time", (int) sizeof(val.d_start_time), (char *) &val.d_start_time - id);
  fprintf(out, _dform, "fl64", "d_stop_time", (int) sizeof(val.d_stop_time), (char *) &val.d_stop_time - id);
  fprintf(out, _dform, "si32", "version_num", (int) sizeof(val.version_num), (char *) &val.version_num - id);
  fprintf(out, _dform, "si32", "num_key_tables", (int) sizeof(val.num_key_tables), (char *) &val.num_key_tables - id);
  fprintf(out, _dform, "si32", "status", (int) sizeof(val.status), (char *) &val.status - id);
  fprintf(out, _dform, "si32", "place_holder[7]", (int) sizeof(val.place_holder), (char *) val.place_holder - id);

  fprintf(out, "  key_table:\n");
  int offset = (char *) val.key_table - id;
  for (int ii = 0; ii < 8; ii++) {
    if (ii < 2 || ii > 5) {
      char name[32];
      sprintf(name, "key_table[%d].offset", ii);
      fprintf(out, _dform, "si32", name, (int) sizeof(Radx::si32), offset);
      offset += sizeof(Radx::si32);
      sprintf(name, "key_table[%d].size  ", ii);
      fprintf(out, _dform, "si32", name, (int) sizeof(Radx::si32), offset);
      offset += sizeof(Radx::si32);
      sprintf(name, "key_table[%d].type  ", ii);
      fprintf(out, _dform, "si32", name, (int) sizeof(Radx::si32), offset);
      offset += sizeof(Radx::si32);
    } else {
      fprintf(out, "     ....\n");
      offset += 3 * sizeof(Radx::si32);
    }
  }

  _printFormatDivider('-', out);

}

// print format of volume block

void DoradeData::printFormat(const DoradeData::volume_t &val, FILE *out)
  
{
  
  _printFormatDivider('-', out);
  fprintf(out, "  struct: 'volume_t'\n  size: %d\n  id: VOLD\n\n", (int) sizeof(val));
  const char *id = val.id;
  _printFormatHeader(out);

  fprintf(out, _dform, "char", "id[4]", (int) sizeof(val.id), (char *) val.id - id);
  fprintf(out, _dform, "si32", "nbytes", (int) sizeof(val.nbytes), (char *) &val.nbytes - id);
  fprintf(out, _dform, "si16", "format_version", (int) sizeof(val.format_version), (char *) &val.format_version - id);
  fprintf(out, _dform, "si16", "volume_num", (int) sizeof(val.volume_num), (char *) &val.volume_num - id);
  fprintf(out, _dform, "si32", "maximum_bytes", (int) sizeof(val.maximum_bytes), (char *) &val.maximum_bytes - id);
  fprintf(out, _dform, "char", "proj_name[20]", (int) sizeof(val.proj_name), (char *) val.proj_name - id);
  fprintf(out, _dform, "si16", "year", (int) sizeof(val.year), (char *) &val.year - id);
  fprintf(out, _dform, "si16", "month", (int) sizeof(val.month), (char *) &val.month - id);
  fprintf(out, _dform, "si16", "day", (int) sizeof(val.day), (char *) &val.day - id);
  fprintf(out, _dform, "si16", "data_set_hour", (int) sizeof(val.data_set_hour), (char *) &val.data_set_hour - id);
  fprintf(out, _dform, "si16", "data_set_minute", (int) sizeof(val.data_set_minute), (char *) &val.data_set_minute - id);
  fprintf(out, _dform, "si16", "data_set_second", (int) sizeof(val.data_set_second), (char *) &val.data_set_second - id);
  fprintf(out, _dform, "char", "flight_num[8]", (int) sizeof(val.flight_num), (char *) val.flight_num - id);
  fprintf(out, _dform, "char", "gen_facility[8]", (int) sizeof(val.gen_facility), (char *) val.gen_facility - id);
  fprintf(out, _dform, "si16", "gen_year", (int) sizeof(val.gen_year), (char *) &val.gen_year - id);
  fprintf(out, _dform, "si16", "gen_month", (int) sizeof(val.gen_month), (char *) &val.gen_month - id);
  fprintf(out, _dform, "si16", "gen_day", (int) sizeof(val.gen_day), (char *) &val.gen_day - id);
  fprintf(out, _dform, "si16", "number_sensor_des", (int) sizeof(val.number_sensor_des), (char *) &val.number_sensor_des - id);
  
  _printFormatDivider('-', out);

}

// print format of radar block

void DoradeData::printFormat(const DoradeData::radar_t &val, FILE *out)
  
{
  
  _printFormatDivider('-', out);
  fprintf(out, "  struct: 'radar_t'\n  size: %d\n  id: RADD\n\n", (int) sizeof(val));
  const char *id = val.id;
  _printFormatHeader(out);

  fprintf(out, _dform, "char", "id[4]", (int) sizeof(val.id), (char *) val.id - id);
  fprintf(out, _dform, "si32", "nbytes", (int) sizeof(val.nbytes), (char *) &val.nbytes - id);
  fprintf(out, _dform, "char", "radar_name[8]", (int) sizeof(val.radar_name), (char *) val.radar_name - id);
  fprintf(out, _dform, "fl32", "radar_const", (int) sizeof(val.radar_const), (char *) &val.radar_const - id);
  fprintf(out, _dform, "fl32", "peak_power", (int) sizeof(val.peak_power), (char *) &val.peak_power - id);
  fprintf(out, _dform, "fl32", "noise_power", (int) sizeof(val.noise_power), (char *) &val.noise_power - id);
  fprintf(out, _dform, "fl32", "receiver_gain", (int) sizeof(val.receiver_gain), (char *) &val.receiver_gain - id);
  fprintf(out, _dform, "fl32", "antenna_gain", (int) sizeof(val.antenna_gain), (char *) &val.antenna_gain - id);
  fprintf(out, _dform, "fl32", "system_gain", (int) sizeof(val.system_gain), (char *) &val.system_gain - id);
  fprintf(out, _dform, "fl32", "horz_beam_width", (int) sizeof(val.horz_beam_width), (char *) &val.horz_beam_width - id);
  fprintf(out, _dform, "fl32", "vert_beam_width", (int) sizeof(val.vert_beam_width), (char *) &val.vert_beam_width - id);
  fprintf(out, _dform, "si16", "radar_type", (int) sizeof(val.radar_type), (char *) &val.radar_type - id);
  fprintf(out, _dform, "si16", "scan_mode", (int) sizeof(val.scan_mode), (char *) &val.scan_mode - id);
  fprintf(out, _dform, "fl32", "req_rotat_vel", (int) sizeof(val.req_rotat_vel), (char *) &val.req_rotat_vel - id);
  fprintf(out, _dform, "fl32", "scan_mode_pram0", (int) sizeof(val.scan_mode_pram0), (char *) &val.scan_mode_pram0 - id);
  fprintf(out, _dform, "fl32", "scan_mode_pram1", (int) sizeof(val.scan_mode_pram1), (char *) &val.scan_mode_pram1 - id);
  fprintf(out, _dform, "si16", "num_parameter_des", (int) sizeof(val.num_parameter_des), (char *) &val.num_parameter_des - id);
  fprintf(out, _dform, "si16", "total_num_des", (int) sizeof(val.total_num_des), (char *) &val.total_num_des - id);
  fprintf(out, _dform, "si16", "data_compress", (int) sizeof(val.data_compress), (char *) &val.data_compress - id);
  fprintf(out, _dform, "si16", "data_reduction", (int) sizeof(val.data_reduction), (char *) &val.data_reduction - id);
  fprintf(out, _dform, "fl32", "data_red_parm0", (int) sizeof(val.data_red_parm0), (char *) &val.data_red_parm0 - id);
  fprintf(out, _dform, "fl32", "data_red_parm1", (int) sizeof(val.data_red_parm1), (char *) &val.data_red_parm1 - id);
  fprintf(out, _dform, "fl32", "radar_longitude", (int) sizeof(val.radar_longitude), (char *) &val.radar_longitude - id);
  fprintf(out, _dform, "fl32", "radar_latitude", (int) sizeof(val.radar_latitude), (char *) &val.radar_latitude - id);
  fprintf(out, _dform, "fl32", "radar_altitude", (int) sizeof(val.radar_altitude), (char *) &val.radar_altitude - id);
  fprintf(out, _dform, "fl32", "eff_unamb_vel", (int) sizeof(val.eff_unamb_vel), (char *) &val.eff_unamb_vel - id);
  fprintf(out, _dform, "fl32", "eff_unamb_range", (int) sizeof(val.eff_unamb_range), (char *) &val.eff_unamb_range - id);
  fprintf(out, _dform, "si16", "num_freq_trans", (int) sizeof(val.num_freq_trans), (char *) &val.num_freq_trans - id);
  fprintf(out, _dform, "si16", "num_ipps_trans", (int) sizeof(val.num_ipps_trans), (char *) &val.num_ipps_trans - id);
  fprintf(out, _dform, "fl32", "freq1", (int) sizeof(val.freq1), (char *) &val.freq1 - id);
  fprintf(out, _dform, "fl32", "freq2", (int) sizeof(val.freq2), (char *) &val.freq2 - id);
  fprintf(out, _dform, "fl32", "freq3", (int) sizeof(val.freq3), (char *) &val.freq3 - id);
  fprintf(out, _dform, "fl32", "freq4", (int) sizeof(val.freq4), (char *) &val.freq4 - id);
  fprintf(out, _dform, "fl32", "freq5", (int) sizeof(val.freq5), (char *) &val.freq5 - id);
  fprintf(out, _dform, "fl32", "prt1", (int) sizeof(val.prt1), (char *) &val.prt1 - id);
  fprintf(out, _dform, "fl32", "prt2", (int) sizeof(val.prt2), (char *) &val.prt2 - id);
  fprintf(out, _dform, "fl32", "prt3", (int) sizeof(val.prt3), (char *) &val.prt3 - id);
  fprintf(out, _dform, "fl32", "prt4", (int) sizeof(val.prt4), (char *) &val.prt4 - id);
  fprintf(out, _dform, "fl32", "prt5", (int) sizeof(val.prt5), (char *) &val.prt5 - id);
  fprintf(out, _dform, "si32", "extension_num", (int) sizeof(val.extension_num), (char *) &val.extension_num - id);
  fprintf(out, _dform, "char", "config_name[8]", (int) sizeof(val.config_name), (char *) val.config_name - id);
  fprintf(out, _dform, "si32", "config_num", (int) sizeof(val.config_num), (char *) &val.config_num - id);
  fprintf(out, _dform, "fl32", "aperture_size", (int) sizeof(val.aperture_size), (char *) &val.aperture_size - id);
  fprintf(out, _dform, "fl32", "field_of_view", (int) sizeof(val.field_of_view), (char *) &val.field_of_view - id);
  fprintf(out, _dform, "fl32", "aperture_eff", (int) sizeof(val.aperture_eff), (char *) &val.aperture_eff - id);
  fprintf(out, _dform, "fl32", "aux_freq[11]", (int) sizeof(val.aux_freq), (char *) val.aux_freq - id);
  fprintf(out, _dform, "fl32", "aux_prt[11]", (int) sizeof(val.aux_prt), (char *) val.aux_prt - id);
  fprintf(out, _dform, "fl32", "pulse_width", (int) sizeof(val.pulse_width), (char *) &val.pulse_width - id);
  fprintf(out, _dform, "fl32", "primary_cop_baseln", (int) sizeof(val.primary_cop_baseln), (char *) &val.primary_cop_baseln - id);
  fprintf(out, _dform, "fl32", "secondary_cop_baseln", (int) sizeof(val.secondary_cop_baseln), (char *) &val.secondary_cop_baseln - id);
  fprintf(out, _dform, "fl32", "pc_xmtr_bandwidth", (int) sizeof(val.pc_xmtr_bandwidth), (char *) &val.pc_xmtr_bandwidth - id);
  fprintf(out, _dform, "si32", "pc_waveform_type", (int) sizeof(val.pc_waveform_type), (char *) &val.pc_waveform_type - id);
  fprintf(out, _dform, "char", "site_name[20]", (int) sizeof(val.site_name), (char *) val.site_name - id);

  _printFormatDivider('-', out);

}

// print format of correction factor block

void DoradeData::printFormat(const DoradeData::correction_t &val, FILE *out)
  
{
  
  _printFormatDivider('-', out);
  fprintf(out, "  struct: 'correction_t'\n  size: %d\n  id: CFAC\n\n", (int) sizeof(val));
  const char *id = val.id;
  _printFormatHeader(out);

  fprintf(out, _dform, "char", "id[4]", (int) sizeof(val.id), (char *) val.id - id);
  fprintf(out, _dform, "si32", "nbytes", (int) sizeof(val.nbytes), (char *) &val.nbytes - id);
  fprintf(out, _dform, "fl32", "azimuth_corr", (int) sizeof(val.azimuth_corr), (char *) &val.azimuth_corr - id);
  fprintf(out, _dform, "fl32", "elevation_corr", (int) sizeof(val.elevation_corr), (char *) &val.elevation_corr - id);
  fprintf(out, _dform, "fl32", "range_delay_corr", (int) sizeof(val.range_delay_corr), (char *) &val.range_delay_corr - id);
  fprintf(out, _dform, "fl32", "longitude_corr", (int) sizeof(val.longitude_corr), (char *) &val.longitude_corr - id);
  fprintf(out, _dform, "fl32", "latitude_corr", (int) sizeof(val.latitude_corr), (char *) &val.latitude_corr - id);
  fprintf(out, _dform, "fl32", "pressure_alt_corr", (int) sizeof(val.pressure_alt_corr), (char *) &val.pressure_alt_corr - id);
  fprintf(out, _dform, "fl32", "radar_alt_corr", (int) sizeof(val.radar_alt_corr), (char *) &val.radar_alt_corr - id);
  fprintf(out, _dform, "fl32", "ew_gndspd_corr", (int) sizeof(val.ew_gndspd_corr), (char *) &val.ew_gndspd_corr - id);
  fprintf(out, _dform, "fl32", "ns_gndspd_corr", (int) sizeof(val.ns_gndspd_corr), (char *) &val.ns_gndspd_corr - id);
  fprintf(out, _dform, "fl32", "vert_vel_corr", (int) sizeof(val.vert_vel_corr), (char *) &val.vert_vel_corr - id);
  fprintf(out, _dform, "fl32", "heading_corr", (int) sizeof(val.heading_corr), (char *) &val.heading_corr - id);
  fprintf(out, _dform, "fl32", "roll_corr", (int) sizeof(val.roll_corr), (char *) &val.roll_corr - id);
  fprintf(out, _dform, "fl32", "pitch_corr", (int) sizeof(val.pitch_corr), (char *) &val.pitch_corr - id);
  fprintf(out, _dform, "fl32", "drift_corr", (int) sizeof(val.drift_corr), (char *) &val.drift_corr - id);
  fprintf(out, _dform, "fl32", "rot_angle_corr", (int) sizeof(val.rot_angle_corr), (char *) &val.rot_angle_corr - id);
  fprintf(out, _dform, "fl32", "tilt_corr", (int) sizeof(val.tilt_corr), (char *) &val.tilt_corr - id);

  _printFormatDivider('-', out);

}

// print format of parameter block

void DoradeData::printFormat(const DoradeData::parameter_t &val, FILE *out)
  
{
  
  _printFormatDivider('-', out);
  fprintf(out, "  struct: 'parameter_t'\n  size: %d\n  id: PARM\n\n", (int) sizeof(val));
  const char *id = val.id;
  _printFormatHeader(out);

  fprintf(out, _dform, "char", "id[4]", (int) sizeof(val.id), (char *) val.id - id);
  fprintf(out, _dform, "si32", "nbytes", (int) sizeof(val.nbytes), (char *) &val.nbytes - id);
  fprintf(out, _dform, "char", "parameter_name[8]", (int) sizeof(val.parameter_name), (char *) val.parameter_name - id);
  fprintf(out, _dform, "char", "param_description[40]", (int) sizeof(val.param_description), (char *) val.param_description - id);
  fprintf(out, _dform, "char", "param_units[8]", (int) sizeof(val.param_units), (char *) val.param_units - id);
  fprintf(out, _dform, "si16", "interpulse_time", (int) sizeof(val.interpulse_time), (char *) &val.interpulse_time - id);
  fprintf(out, _dform, "si16", "xmitted_freq", (int) sizeof(val.xmitted_freq), (char *) &val.xmitted_freq - id);
  fprintf(out, _dform, "fl32", "recvr_bandwidth", (int) sizeof(val.recvr_bandwidth), (char *) &val.recvr_bandwidth - id);
  fprintf(out, _dform, "si16", "pulse_width", (int) sizeof(val.pulse_width), (char *) &val.pulse_width - id);
  fprintf(out, _dform, "si16", "polarization", (int) sizeof(val.polarization), (char *) &val.polarization - id);
  fprintf(out, _dform, "si16", "num_samples", (int) sizeof(val.num_samples), (char *) &val.num_samples - id);
  fprintf(out, _dform, "si16", "binary_format", (int) sizeof(val.binary_format), (char *) &val.binary_format - id);
  fprintf(out, _dform, "char", "threshold_field[8]", (int) sizeof(val.threshold_field), (char *) val.threshold_field - id);
  fprintf(out, _dform, "fl32", "threshold_value", (int) sizeof(val.threshold_value), (char *) &val.threshold_value - id);
  fprintf(out, _dform, "fl32", "parameter_scale", (int) sizeof(val.parameter_scale), (char *) &val.parameter_scale - id);
  fprintf(out, _dform, "fl32", "parameter_bias", (int) sizeof(val.parameter_bias), (char *) &val.parameter_bias - id);
  fprintf(out, _dform, "si32", "bad_data", (int) sizeof(val.bad_data), (char *) &val.bad_data - id);
  fprintf(out, _dform, "si32", "extension_num", (int) sizeof(val.extension_num), (char *) &val.extension_num - id);
  fprintf(out, _dform, "char", "config_name[8]", (int) sizeof(val.config_name), (char *) val.config_name - id);
  fprintf(out, _dform, "si32", "config_num", (int) sizeof(val.config_num), (char *) &val.config_num - id);
  fprintf(out, _dform, "si32", "offset_to_data", (int) sizeof(val.offset_to_data), (char *) &val.offset_to_data - id);
  fprintf(out, _dform, "fl32", "mks_conversion", (int) sizeof(val.mks_conversion), (char *) &val.mks_conversion - id);
  fprintf(out, _dform, "si32", "num_qnames", (int) sizeof(val.num_qnames), (char *) &val.num_qnames - id);
  fprintf(out, _dform, "char", "qdata_names[32]", (int) sizeof(val.qdata_names), (char *) val.qdata_names - id);
  fprintf(out, _dform, "si32", "num_criteria", (int) sizeof(val.num_criteria), (char *) &val.num_criteria - id);
  fprintf(out, _dform, "char", "criteria_names[32]", (int) sizeof(val.criteria_names), (char *) val.criteria_names - id);
  fprintf(out, _dform, "si32", "number_cells", (int) sizeof(val.number_cells), (char *) &val.number_cells - id);
  fprintf(out, _dform, "fl32", "meters_to_first_cell", (int) sizeof(val.meters_to_first_cell), (char *) &val.meters_to_first_cell - id);
  fprintf(out, _dform, "fl32", "meters_between_cells", (int) sizeof(val.meters_between_cells), (char *) &val.meters_between_cells - id);
  fprintf(out, _dform, "fl32", "eff_unamb_vel", (int) sizeof(val.eff_unamb_vel), (char *) &val.eff_unamb_vel - id);

  _printFormatDivider('-', out);

}

// print format of cell block

void DoradeData::printFormat(const DoradeData::cell_vector_t &val, FILE *out)
  
{
  
  _printFormatDivider('-', out);
  fprintf(out, "  struct: 'cell_vector_t'\n  size: %d\n  id: CELV\n\n", (int) sizeof(val));
  const char *id = val.id;
  _printFormatHeader(out);
  
  fprintf(out, _dform, "char", "id[4]", (int) sizeof(val.id), (char *) val.id - id);
  fprintf(out, _dform, "si32", "nbytes", (int) sizeof(val.nbytes), (char *) &val.nbytes - id);
  fprintf(out, _dform, "si32", "number_cells", (int) sizeof(val.number_cells), (char *) &val.number_cells - id);
  fprintf(out, _dform, "fl32", "dist_cells[1500]", (int) sizeof(val.dist_cells), (char *) val.dist_cells - id);

  _printFormatDivider('-', out);

}

// print format of cell spacing block

void DoradeData::printFormat(const DoradeData::cell_spacing_fp_t &val, FILE *out)
  
{
  
  _printFormatDivider('-', out);
  fprintf(out, "  struct: 'cell_spacing_fp_t'\n  size: %d\n  id: CSFD\n\n", (int) sizeof(val));
  const char *id = val.id;
  _printFormatHeader(out);
  
  fprintf(out, _dform, "char", "id[4]", (int) sizeof(val.id), (char *) val.id - id);
  fprintf(out, _dform, "si32", "nbytes", (int) sizeof(val.nbytes), (char *) &val.nbytes - id);
  fprintf(out, _dform, "si32", "num_segments", (int) sizeof(val.num_segments), (char *) &val.num_segments - id);
  fprintf(out, _dform, "fl32", "dist_to_first", (int) sizeof(val.dist_to_first), (char *) &val.dist_to_first - id);
  fprintf(out, _dform, "fl32", "spacing[8]", (int) sizeof(val.spacing), (char *) val.spacing - id);
  fprintf(out, _dform, "si16", "num_cells[8]", (int) sizeof(val.num_cells), (char *) val.num_cells  - id);

  _printFormatDivider('-', out);

}

// print format of sweep info block

void DoradeData::printFormat(const DoradeData::sweepinfo_t &val, FILE *out)
  
{
  
  _printFormatDivider('-', out);
  fprintf(out, "  struct: 'sweepinfo_t'\n  size: %d\n  id: SWIB\n\n", (int) sizeof(val));
  const char *id = val.id;
  _printFormatHeader(out);
  
  fprintf(out, _dform, "char", "id[4]", (int) sizeof(val.id), (char *) val.id - id);
  fprintf(out, _dform, "si32", "nbytes", (int) sizeof(val.nbytes), (char *) &val.nbytes - id);
  fprintf(out, _dform, "char", "radar_name[8]", (int) sizeof(val.radar_name), (char *) val.radar_name - id);
  fprintf(out, _dform, "si32", "sweep_num", (int) sizeof(val.sweep_num), (char *) &val.sweep_num - id);
  fprintf(out, _dform, "si32", "num_rays", (int) sizeof(val.num_rays), (char *) &val.num_rays - id);
  fprintf(out, _dform, "fl32", "start_angle", (int) sizeof(val.start_angle), (char *) &val.start_angle - id);
  fprintf(out, _dform, "fl32", "stop_angle", (int) sizeof(val.stop_angle), (char *) &val.stop_angle - id);
  fprintf(out, _dform, "fl32", "fixed_angle", (int) sizeof(val.fixed_angle), (char *) &val.fixed_angle - id);
  fprintf(out, _dform, "si32", "filter_flag", (int) sizeof(val.filter_flag), (char *) &val.filter_flag - id);

  _printFormatDivider('-', out);

}

// print format of platform block

void DoradeData::printFormat(const DoradeData::platform_t &val, FILE *out)
  
{
  
  _printFormatDivider('-', out);
  fprintf(out, "  struct: 'platform_t'\n  size: %d\n  id: ASIB\n\n", (int) sizeof(val));
  const char *id = val.id;
  _printFormatHeader(out);
  
  fprintf(out, _dform, "char", "id[4]", (int) sizeof(val.id), (char *) val.id - id);
  fprintf(out, _dform, "si32", "nbytes", (int) sizeof(val.nbytes), (char *) &val.nbytes - id);
  fprintf(out, _dform, "fl32", "longitude", (int) sizeof(val.longitude), (char *) &val.longitude - id);
  fprintf(out, _dform, "fl32", "latitude", (int) sizeof(val.latitude), (char *) &val.latitude - id);
  fprintf(out, _dform, "fl32", "altitude_msl", (int) sizeof(val.altitude_msl), (char *) &val.altitude_msl - id);
  fprintf(out, _dform, "fl32", "altitude_agl", (int) sizeof(val.altitude_agl), (char *) &val.altitude_agl - id);
  fprintf(out, _dform, "fl32", "ew_velocity", (int) sizeof(val.ew_velocity), (char *) &val.ew_velocity - id);
  fprintf(out, _dform, "fl32", "ns_velocity", (int) sizeof(val.ns_velocity), (char *) &val.ns_velocity - id);
  fprintf(out, _dform, "fl32", "vert_velocity", (int) sizeof(val.vert_velocity), (char *) &val.vert_velocity - id);
  fprintf(out, _dform, "fl32", "heading", (int) sizeof(val.heading), (char *) &val.heading - id);
  fprintf(out, _dform, "fl32", "roll", (int) sizeof(val.roll), (char *) &val.roll - id);
  fprintf(out, _dform, "fl32", "pitch", (int) sizeof(val.pitch), (char *) &val.pitch - id);
  fprintf(out, _dform, "fl32", "drift_angle", (int) sizeof(val.drift_angle), (char *) &val.drift_angle - id);
  fprintf(out, _dform, "fl32", "rotation_angle", (int) sizeof(val.rotation_angle), (char *) &val.rotation_angle - id);
  fprintf(out, _dform, "fl32", "tilt", (int) sizeof(val.tilt), (char *) &val.tilt - id);
  fprintf(out, _dform, "fl32", "ew_horiz_wind", (int) sizeof(val.ew_horiz_wind), (char *) &val.ew_horiz_wind - id);
  fprintf(out, _dform, "fl32", "ns_horiz_wind", (int) sizeof(val.ns_horiz_wind), (char *) &val.ns_horiz_wind - id);
  fprintf(out, _dform, "fl32", "vert_wind", (int) sizeof(val.vert_wind), (char *) &val.vert_wind - id);
  fprintf(out, _dform, "fl32", "heading_change", (int) sizeof(val.heading_change), (char *) &val.heading_change - id);
  fprintf(out, _dform, "fl32", "pitch_change", (int) sizeof(val.pitch_change), (char *) &val.pitch_change - id);

  _printFormatDivider('-', out);

}

// print format of ray block

void DoradeData::printFormat(const DoradeData::ray_t &val, FILE *out)
  
{
  
  _printFormatDivider('-', out);
  fprintf(out, "  struct: 'ray_t'\n  size: %d\n  id: RYIB\n\n", (int) sizeof(val));
  const char *id = val.id;
  _printFormatHeader(out);
  
  fprintf(out, _dform, "char", "id[4]", (int) sizeof(val.id), (char *) val.id - id);
  fprintf(out, _dform, "si32", "nbytes", (int) sizeof(val.nbytes), (char *) &val.nbytes - id);
  fprintf(out, _dform, "si32", "sweep_num", (int) sizeof(val.sweep_num), (char *) &val.sweep_num - id);
  fprintf(out, _dform, "si32", "julian_day", (int) sizeof(val.julian_day), (char *) &val.julian_day - id);
  fprintf(out, _dform, "si16", "hour", (int) sizeof(val.hour), (char *) &val.hour - id);
  fprintf(out, _dform, "si16", "minute", (int) sizeof(val.minute), (char *) &val.minute - id);
  fprintf(out, _dform, "si16", "second", (int) sizeof(val.second), (char *) &val.second - id);
  fprintf(out, _dform, "si16", "millisecond", (int) sizeof(val.millisecond), (char *) &val.millisecond - id);
  fprintf(out, _dform, "fl32", "azimuth", (int) sizeof(val.azimuth), (char *) &val.azimuth - id);
  fprintf(out, _dform, "fl32", "elevation", (int) sizeof(val.elevation), (char *) &val.elevation - id);
  fprintf(out, _dform, "fl32", "peak_power", (int) sizeof(val.peak_power), (char *) &val.peak_power - id);
  fprintf(out, _dform, "fl32", "true_scan_rate", (int) sizeof(val.true_scan_rate), (char *) &val.true_scan_rate - id);
  fprintf(out, _dform, "si32", "ray_status", (int) sizeof(val.ray_status), (char *) &val.ray_status - id);

  _printFormatDivider('-', out);

}

// print format of paramdata block

void DoradeData::printFormat(const DoradeData::paramdata_t &val, FILE *out)
  
{
  
  _printFormatDivider('-', out);
  fprintf(out, "  struct: 'paramdata_t'\n  size: %d\n  id: RDAT\n\n", (int) sizeof(val));
  const char *id = val.id;
  _printFormatHeader(out);
  
  fprintf(out, _dform, "char", "id[4]", (int) sizeof(val.id), (char *) val.id - id);
  fprintf(out, _dform, "si32", "nbytes", (int) sizeof(val.nbytes), (char *) &val.nbytes - id);
  fprintf(out, _dform, "char", "pdata_name[8]", (int) sizeof(val.pdata_name), (char *) val.pdata_name - id);

  _printFormatDivider('-', out);

}

// print format of qparamdata block

void DoradeData::printFormat(const DoradeData::qparamdata_t &val, FILE *out)
  
{
  
  _printFormatDivider('-', out);
  fprintf(out, "  struct: 'qparamdata_t'\n  size: %d\n  id: QDAT\n\n", (int) sizeof(val));
  const char *id = val.id;
  _printFormatHeader(out);
  
  fprintf(out, _dform, "char", "id[4]", (int) sizeof(val.id), (char *) val.id - id);
  fprintf(out, _dform, "si32", "nbytes", (int) sizeof(val.nbytes), (char *) &val.nbytes - id);
  fprintf(out, _dform, "char", "pdata_name[8]", (int) sizeof(val.pdata_name), (char *) val.pdata_name - id);
  fprintf(out, _dform, "si32", "extension_num", (int) sizeof(val.extension_num), (char *) &val.extension_num - id);
  fprintf(out, _dform, "si32", "config_num", (int) sizeof(val.config_num), (char *) &val.config_num - id);
  fprintf(out, _dform, "si16", "first_cell[4]", (int) sizeof(val.first_cell), (char *) val.first_cell - id);
  fprintf(out, _dform, "si16", "num_cells[4]", (int) sizeof(val.num_cells), (char *) val.num_cells - id);
  fprintf(out, _dform, "fl32", "criteria_value[4]", (int) sizeof(val.criteria_value), (char *) val.criteria_value - id);

  _printFormatDivider('-', out);

}

// print format of extra stuff block

void DoradeData::printFormat(const DoradeData::extra_stuff_t &val, FILE *out)
  
{
  
  _printFormatDivider('-', out);
  fprintf(out, "  struct: 'extra_stuff_t'\n  size: %d\n  id: XTSF\n\n", (int) sizeof(val));
  const char *id = val.id;
  _printFormatHeader(out);
  
  fprintf(out, _dform, "char", "id[4]", (int) sizeof(val.id), (char *) val.id - id);
  fprintf(out, _dform, "si32", "nbytes", (int) sizeof(val.nbytes), (char *) &val.nbytes - id);
  fprintf(out, _dform, "si32", "one", (int) sizeof(val.one), (char *) &val.one - id);
  fprintf(out, _dform, "si32", "source_format", (int) sizeof(val.source_format), (char *) &val.source_format - id);
  fprintf(out, _dform, "si32", "offset_to_first_item", (int) sizeof(val.offset_to_first_item), (char *) &val.offset_to_first_item - id);
  fprintf(out, _dform, "si32", "transition_flag", (int) sizeof(val.transition_flag), (char *) &val.transition_flag - id);

  _printFormatDivider('-', out);

}

// print format of null block

void DoradeData::printFormat(const DoradeData::null_block_t &val, FILE *out)
  
{
  
  _printFormatDivider('-', out);
  fprintf(out, "  struct: 'null_block_t'\n  size: %d\n  id: NULL\n\n", (int) sizeof(val));
  const char *id = val.id;
  _printFormatHeader(out);
  
  fprintf(out, _dform, "char", "id[4]", (int) sizeof(val.id), (char *) val.id - id);
  fprintf(out, _dform, "si32", "nbytes", (int) sizeof(val.nbytes), (char *) &val.nbytes - id);

  _printFormatDivider('-', out);

}

// print format of rotation angle table block

void DoradeData::printFormat(const DoradeData::rot_angle_table_t &val, FILE *out)
  
{
  
  _printFormatDivider('-', out);
  fprintf(out, "  struct: 'rot_angle_table_t'\n  size: %d\n  id: RKTB\n\n", (int) sizeof(val));
  const char *id = val.id;
  _printFormatHeader(out);
  
  fprintf(out, _dform, "char", "id[4]", (int) sizeof(val.id), (char *) val.id - id);
  fprintf(out, _dform, "si32", "nbytes", (int) sizeof(val.nbytes), (char *) &val.nbytes - id);

  fprintf(out, _dform, "fl32", "angle2ndx", (int) sizeof(val.angle2ndx), (char *) &val.angle2ndx - id);
  fprintf(out, _dform, "si32", "ndx_que_size", (int) sizeof(val.ndx_que_size), (char *) &val.ndx_que_size - id);
  fprintf(out, _dform, "si32", "first_key_offset", (int) sizeof(val.first_key_offset), (char *) &val.first_key_offset - id);
  fprintf(out, _dform, "si32", "angle_table_offset", (int) sizeof(val.angle_table_offset), (char *) &val.angle_table_offset - id);
  fprintf(out, _dform, "si32", "num_rays", (int) sizeof(val.num_rays), (char *) &val.num_rays - id);

  _printFormatDivider('-', out);

}

void DoradeData::printFormat(const DoradeData::rot_table_entry_t &val, FILE *out)
  
{
  
  _printFormatDivider('-', out);
  fprintf(out, "  struct: 'rot_table_entry_t'\n  size: %d\n\n", (int) sizeof(val));
  const char *id = (char *) &val.rotation_angle;
  _printFormatHeader(out);
  
  fprintf(out, _dform, "fl32", "rotation_angle", (int) sizeof(val.rotation_angle), (char *) &val.rotation_angle - id);
  fprintf(out, _dform, "si32", "offset", (int) sizeof(val.offset), (char *) &val.offset - id);
  fprintf(out, _dform, "si32", "size", (int) sizeof(val.size), (char *) &val.size - id);

  _printFormatDivider('-', out);

}

// print format of radar_test_status_t block

void DoradeData::printFormat(const DoradeData::radar_test_status_t &val, FILE *out)

{

  _printFormatDivider('-', out);
  fprintf(out, "  struct: 'radar_test_status_t'\n  size: %d\n  id: FRAD\n\n", (int) sizeof(val));
  const char *id = val.id;
  _printFormatHeader(out);
  
  fprintf(out, _dform, "char", "id[4]", (int) sizeof(val.id), (char *) val.id - id);
  fprintf(out, _dform, "si32", "nbytes", (int) sizeof(val.nbytes), (char *) &val.nbytes - id);
  fprintf(out, _dform, "si32", "data_sys_status", (int) sizeof(val.data_sys_status), (char *) &val.data_sys_status - id);
  fprintf(out, _dform, "char", "radar_name[8]", (int) sizeof(val.radar_name), (char *) val.radar_name - id);
  fprintf(out, _dform, "fl32", "test_pulse_level", (int) sizeof(val.test_pulse_level), (char *) &val.test_pulse_level - id);
  fprintf(out, _dform, "fl32", "test_pulse_dist", (int) sizeof(val.test_pulse_dist), (char *) &val.test_pulse_dist - id);
  fprintf(out, _dform, "fl32", "test_pulse_width", (int) sizeof(val.test_pulse_width), (char *) &val.test_pulse_width - id);
  fprintf(out, _dform, "fl32", "test_pulse_freq", (int) sizeof(val.test_pulse_freq), (char *) &val.test_pulse_freq - id);
  fprintf(out, _dform, "si16", "test_pulse_atten", (int) sizeof(val.test_pulse_atten), (char *) &val.test_pulse_atten - id);
  fprintf(out, _dform, "si16", "test_pulse_fnum", (int) sizeof(val.test_pulse_fnum), (char *) &val.test_pulse_fnum - id);
  fprintf(out, _dform, "fl32", "noise_power", (int) sizeof(val.noise_power), (char *) &val.noise_power - id);
  fprintf(out, _dform, "si32", "ray_count", (int) sizeof(val.ray_count), (char *) &val.ray_count - id);
  fprintf(out, _dform, "si16", "first_rec_gate", (int) sizeof(val.first_rec_gate), (char *) &val.first_rec_gate - id);
  fprintf(out, _dform, "si16", "last_rec_gate", (int) sizeof(val.last_rec_gate), (char *) &val.last_rec_gate - id);

  _printFormatDivider('-', out);

}

// print format of field_radar_t block

void DoradeData::printFormat(const DoradeData::field_radar_t &val, FILE *out)

{

  _printFormatDivider('-', out);
  fprintf(out, "  struct: 'field_radar_t'\n  size: %d\n  id: FRIB\n\n", (int) sizeof(val));
  const char *id = val.id;
  _printFormatHeader(out);
  
  fprintf(out, _dform, "char", "id[4]", (int) sizeof(val.id), (char *) val.id - id);
  fprintf(out, _dform, "si32", "nbytes", (int) sizeof(val.nbytes), (char *) &val.nbytes - id);
  fprintf(out, _dform, "si32", "data_sys_id", (int) sizeof(val.data_sys_id), (char *) &val.data_sys_id - id);
  fprintf(out, _dform, "fl32", "loss_out", (int) sizeof(val.loss_out), (char *) &val.loss_out - id);
  fprintf(out, _dform, "fl32", "loss_in", (int) sizeof(val.loss_in), (char *) &val.loss_in - id);
  fprintf(out, _dform, "fl32", "loss_rjoint", (int) sizeof(val.loss_rjoint), (char *) &val.loss_rjoint - id);
  fprintf(out, _dform, "fl32", "ant_v_dim", (int) sizeof(val.ant_v_dim), (char *) &val.ant_v_dim - id);
  fprintf(out, _dform, "fl32", "ant_h_dim", (int) sizeof(val.ant_h_dim), (char *) &val.ant_h_dim - id);
  fprintf(out, _dform, "fl32", "ant_noise_temp", (int) sizeof(val.ant_noise_temp), (char *) &val.ant_noise_temp - id);
  fprintf(out, _dform, "fl32", "r_noise_figure", (int) sizeof(val.r_noise_figure), (char *) &val.r_noise_figure - id);
  fprintf(out, _dform, "fl32", "xmit_power[5]", (int) sizeof(val.xmit_power), (char *) val.xmit_power - id);
  fprintf(out, _dform, "fl32", "x_band_gain", (int) sizeof(val.x_band_gain), (char *) &val.x_band_gain - id);
  fprintf(out, _dform, "fl32", "receiver_gain[5]", (int) sizeof(val.receiver_gain), (char *) val.receiver_gain - id);
  fprintf(out, _dform, "fl32", "if_gain[5]", (int) sizeof(val.if_gain), (char *) val.if_gain - id);
  fprintf(out, _dform, "fl32", "conversion_gain", (int) sizeof(val.conversion_gain), (char *) &val.conversion_gain - id);
  fprintf(out, _dform, "fl32", "scale_factor[5]", (int) sizeof(val.scale_factor), (char *) val.scale_factor - id);
  fprintf(out, _dform, "fl32", "processor_const", (int) sizeof(val.processor_const), (char *) &val.processor_const - id);
  fprintf(out, _dform, "si32", "dly_tube_antenna", (int) sizeof(val.dly_tube_antenna), (char *) &val.dly_tube_antenna - id);
  fprintf(out, _dform, "si32", "dly_rndtrip_chip_atod", (int) sizeof(val.dly_rndtrip_chip_atod), (char *) &val.dly_rndtrip_chip_atod - id);
  fprintf(out, _dform, "si32", "dly_timmod_testpulse", (int) sizeof(val.dly_timmod_testpulse), (char *) &val.dly_timmod_testpulse - id);
  fprintf(out, _dform, "si32", "dly_modulator_on", (int) sizeof(val.dly_modulator_on), (char *) &val.dly_modulator_on - id);
  fprintf(out, _dform, "si32", "dly_modulator_off", (int) sizeof(val.dly_modulator_off), (char *) &val.dly_modulator_off - id);
  fprintf(out, _dform, "fl32", "peak_power_offset", (int) sizeof(val.peak_power_offset), (char *) &val.peak_power_offset - id);
  fprintf(out, _dform, "fl32", "test_pulse_offset", (int) sizeof(val.test_pulse_offset), (char *) &val.test_pulse_offset - id);
  fprintf(out, _dform, "fl32", "E_plane_angle", (int) sizeof(val.E_plane_angle), (char *) &val.E_plane_angle - id);
  fprintf(out, _dform, "fl32", "H_plane_angle", (int) sizeof(val.H_plane_angle), (char *) &val.H_plane_angle - id);
  fprintf(out, _dform, "fl32", "encoder_antenna_up", (int) sizeof(val.encoder_antenna_up), (char *) &val.encoder_antenna_up - id);
  fprintf(out, _dform, "fl32", "pitch_antenna_up", (int) sizeof(val.pitch_antenna_up), (char *) &val.pitch_antenna_up - id);
  fprintf(out, _dform, "si16", "indepf_times_flg", (int) sizeof(val.indepf_times_flg), (char *) &val.indepf_times_flg - id);
  fprintf(out, _dform, "si16", "indep_freq_gate", (int) sizeof(val.indep_freq_gate), (char *) &val.indep_freq_gate - id);
  fprintf(out, _dform, "si16", "time_series_gate", (int) sizeof(val.time_series_gate), (char *) &val.time_series_gate - id);
  fprintf(out, _dform, "si16", "num_base_params", (int) sizeof(val.num_base_params), (char *) &val.num_base_params - id);
  fprintf(out, _dform, "char", "file_name[80]", (int) sizeof(val.file_name), (char *) val.file_name - id);

  _printFormatDivider('-', out);

}

// print format of lidar_t block

void DoradeData::printFormat(const DoradeData::lidar_t &val, FILE *out)

{

  _printFormatDivider('-', out);
  fprintf(out, "  struct: 'lidar_t'\n  size: %d\n  id: LIDR\n\n", (int) sizeof(val));
  const char *id = val.id;
  _printFormatHeader(out);
  
  fprintf(out, _dform, "char", "id[4]", (int) sizeof(val.id), (char *) val.id - id);
  fprintf(out, _dform, "si32", "nbytes", (int) sizeof(val.nbytes), (char *) &val.nbytes - id);
  fprintf(out, _dform, "char", "lidar_name[8]", (int) sizeof(val.lidar_name), (char *) val.lidar_name - id);
  fprintf(out, _dform, "fl32", "lidar_const", (int) sizeof(val.lidar_const), (char *) &val.lidar_const - id);
  fprintf(out, _dform, "fl32", "pulse_energy", (int) sizeof(val.pulse_energy), (char *) &val.pulse_energy - id);
  fprintf(out, _dform, "fl32", "peak_power", (int) sizeof(val.peak_power), (char *) &val.peak_power - id);
  fprintf(out, _dform, "fl32", "pulse_width", (int) sizeof(val.pulse_width), (char *) &val.pulse_width - id);
  fprintf(out, _dform, "fl32", "aperture_size", (int) sizeof(val.aperture_size), (char *) &val.aperture_size - id);
  fprintf(out, _dform, "fl32", "field_of_view", (int) sizeof(val.field_of_view), (char *) &val.field_of_view - id);
  fprintf(out, _dform, "fl32", "aperture_eff", (int) sizeof(val.aperture_eff), (char *) &val.aperture_eff - id);
  fprintf(out, _dform, "fl32", "beam_divergence", (int) sizeof(val.beam_divergence), (char *) &val.beam_divergence - id);
  fprintf(out, _dform, "si16", "lidar_type", (int) sizeof(val.lidar_type), (char *) &val.lidar_type - id);
  fprintf(out, _dform, "si16", "scan_mode", (int) sizeof(val.scan_mode), (char *) &val.scan_mode - id);
  fprintf(out, _dform, "fl32", "req_rotat_vel", (int) sizeof(val.req_rotat_vel), (char *) &val.req_rotat_vel - id);
  fprintf(out, _dform, "fl32", "scan_mode_pram0", (int) sizeof(val.scan_mode_pram0), (char *) &val.scan_mode_pram0 - id);
  fprintf(out, _dform, "fl32", "scan_mode_pram1", (int) sizeof(val.scan_mode_pram1), (char *) &val.scan_mode_pram1 - id);
  fprintf(out, _dform, "si16", "num_parameter_des", (int) sizeof(val.num_parameter_des), (char *) &val.num_parameter_des - id);
  fprintf(out, _dform, "si16", "total_num_des", (int) sizeof(val.total_num_des), (char *) &val.total_num_des - id);
  fprintf(out, _dform, "si16", "data_compress", (int) sizeof(val.data_compress), (char *) &val.data_compress - id);
  fprintf(out, _dform, "si16", "data_reduction", (int) sizeof(val.data_reduction), (char *) &val.data_reduction - id);
  fprintf(out, _dform, "fl32", "data_red_parm0", (int) sizeof(val.data_red_parm0), (char *) &val.data_red_parm0 - id);
  fprintf(out, _dform, "fl32", "data_red_parm1", (int) sizeof(val.data_red_parm1), (char *) &val.data_red_parm1 - id);
  fprintf(out, _dform, "fl32", "lidar_longitude", (int) sizeof(val.lidar_longitude), (char *) &val.lidar_longitude - id);
  fprintf(out, _dform, "fl32", "lidar_latitude", (int) sizeof(val.lidar_latitude), (char *) &val.lidar_latitude - id);
  fprintf(out, _dform, "fl32", "lidar_altitude", (int) sizeof(val.lidar_altitude), (char *) &val.lidar_altitude - id);
  fprintf(out, _dform, "fl32", "eff_unamb_vel", (int) sizeof(val.eff_unamb_vel), (char *) &val.eff_unamb_vel - id);
  fprintf(out, _dform, "fl32", "eff_unamb_range", (int) sizeof(val.eff_unamb_range), (char *) &val.eff_unamb_range - id);
  fprintf(out, _dform, "si32", "num_wvlen_trans", (int) sizeof(val.num_wvlen_trans), (char *) &val.num_wvlen_trans - id);
  fprintf(out, _dform, "fl32", "prf", (int) sizeof(val.prf), (char *) &val.prf - id);
  fprintf(out, _dform, "fl32", "wavelength[10]", (int) sizeof(val.wavelength), (char *) val.wavelength - id);

  _printFormatDivider('-', out);

}

// print format of field_lidar_t block

void DoradeData::printFormat(const DoradeData::field_lidar_t &val, FILE *out)

{

  _printFormatDivider('-', out);
  fprintf(out, "  struct: 'field_lidar_t'\n  size: %d\n  id: FLIB\n\n", (int) sizeof(val));
  const char *id = val.id;
  _printFormatHeader(out);
  
  fprintf(out, _dform, "char", "id[4]", (int) sizeof(val.id), (char *) val.id - id);
  fprintf(out, _dform, "si32", "nbytes", (int) sizeof(val.nbytes), (char *) &val.nbytes - id);

  fprintf(out, _dform, "si32", "data_sys_id", (int) sizeof(val.data_sys_id), (char *) &val.data_sys_id - id);
  fprintf(out, _dform, "fl32", "transmit_beam_div[10]", (int) sizeof(val.transmit_beam_div), (char *) val.transmit_beam_div - id);
  fprintf(out, _dform, "fl32", "xmit_power[10]", (int) sizeof(val.xmit_power), (char *) val.xmit_power - id);
  fprintf(out, _dform, "fl32", "receiver_fov[10]", (int) sizeof(val.receiver_fov), (char *) val.receiver_fov - id);
  fprintf(out, _dform, "si32", "receiver_type[10]", (int) sizeof(val.receiver_type), (char *) val.receiver_type - id);
  fprintf(out, _dform, "fl32", "r_noise_floor[10]", (int) sizeof(val.r_noise_floor), (char *) val.r_noise_floor - id);
  fprintf(out, _dform, "fl32", "receiver_spec_bw[10]", (int) sizeof(val.receiver_spec_bw), (char *) val.receiver_spec_bw - id);
  fprintf(out, _dform, "fl32", "receiver_elec_bw[10]", (int) sizeof(val.receiver_elec_bw), (char *) val.receiver_elec_bw - id);
  fprintf(out, _dform, "fl32", "calibration[10]", (int) sizeof(val.calibration), (char *) val.calibration - id);
  fprintf(out, _dform, "si32", "range_delay", (int) sizeof(val.range_delay), (char *) &val.range_delay - id);
  fprintf(out, _dform, "fl32", "peak_power_multi[10]", (int) sizeof(val.peak_power_multi), (char *) val.peak_power_multi - id);
  fprintf(out, _dform, "fl32", "encoder_mirror_up", (int) sizeof(val.encoder_mirror_up), (char *) &val.encoder_mirror_up - id);
  fprintf(out, _dform, "fl32", "pitch_mirror_up", (int) sizeof(val.pitch_mirror_up), (char *) &val.pitch_mirror_up - id);
  fprintf(out, _dform, "si32", "max_digitizer_count", (int) sizeof(val.max_digitizer_count), (char *) &val.max_digitizer_count - id);
  fprintf(out, _dform, "fl32", "max_digitizer_volt", (int) sizeof(val.max_digitizer_volt), (char *) &val.max_digitizer_volt - id);
  fprintf(out, _dform, "fl32", "digitizer_rate", (int) sizeof(val.digitizer_rate), (char *) &val.digitizer_rate - id);
  fprintf(out, _dform, "si32", "total_num_samples", (int) sizeof(val.total_num_samples), (char *) &val.total_num_samples - id);
  fprintf(out, _dform, "si32", "samples_per_cell", (int) sizeof(val.samples_per_cell), (char *) &val.samples_per_cell - id);
  fprintf(out, _dform, "si32", "cells_per_ray", (int) sizeof(val.cells_per_ray), (char *) &val.cells_per_ray - id);
  fprintf(out, _dform, "fl32", "pmt_temp", (int) sizeof(val.pmt_temp), (char *) &val.pmt_temp - id);
  fprintf(out, _dform, "fl32", "pmt_gain", (int) sizeof(val.pmt_gain), (char *) &val.pmt_gain - id);
  fprintf(out, _dform, "fl32", "apd_temp", (int) sizeof(val.apd_temp), (char *) &val.apd_temp - id);
  fprintf(out, _dform, "fl32", "apd_gain", (int) sizeof(val.apd_gain), (char *) &val.apd_gain - id);
  fprintf(out, _dform, "si32", "transect", (int) sizeof(val.transect), (char *) &val.transect - id);
  fprintf(out, _dform, "char", "derived_names[10][12]", (int) sizeof(val.derived_names), (char *) val.derived_names - id);
  fprintf(out, _dform, "char", "derived_units[10][8]", (int) sizeof(val.derived_units), (char *) val.derived_units - id);
  fprintf(out, _dform, "char", "temp_names[10][12]", (int) sizeof(val.temp_names), (char *) val.temp_names - id);

  _printFormatDivider('-', out);

}

// print format of insitu_descript_t block

void DoradeData::printFormat(const DoradeData::insitu_descript_t &val, FILE *out)

{

  _printFormatDivider('-', out);
  fprintf(out, "  struct: 'insitu_descript_t'\n  size: %d\n  id: SITU\n\n", (int) sizeof(val));
  const char *id = val.id;
  _printFormatHeader(out);
  
  fprintf(out, _dform, "char", "id[4]", (int) sizeof(val.id), (char *) val.id - id);
  fprintf(out, _dform, "si32", "nbytes", (int) sizeof(val.nbytes), (char *) &val.nbytes - id);
  fprintf(out, _dform, "si32", "number_params", (int) sizeof(val.number_params), (char *) &val.number_params - id);

  fprintf(out, "  params:\n");
  int offset = (char *) val.params - id;
  int size = sizeof(char[8]);
  for (int ii = 0; ii < 256; ii++) {
    if (ii < 3 || ii > 252) {
      char label[32];
      sprintf(label, "params[%d].name[8] ", ii);
      fprintf(out, _dform, "char", label, size, offset);
      offset += size;
      sprintf(label, "params[%d].units[8]", ii);
      fprintf(out, _dform, "char", label, size, offset);
      offset += size;
    } else if (ii < 6){
      fprintf(out, "     ....\n");
      offset += 2 * size;
    } else {
      offset += 2 * size;
    }
  }

  _printFormatDivider('-', out);

}


// print format of insitu_data_t block

void DoradeData::printFormat(const DoradeData::insitu_data_t &val, FILE *out)

{

  _printFormatDivider('-', out);
  fprintf(out, "  struct: 'insitu_data_t'\n  size: %d\n  id: ISIT\n\n", (int) sizeof(val));
  const char *id = val.id;
  _printFormatHeader(out);
  
  fprintf(out, _dform, "char", "id[4]", (int) sizeof(val.id), (char *) val.id - id);
  fprintf(out, _dform, "si32", "nbytes", (int) sizeof(val.nbytes), (char *) &val.nbytes - id);
  fprintf(out, _dform, "si16", "julian_day", (int) sizeof(val.julian_day), (char *) &val.julian_day - id);
  fprintf(out, _dform, "si16", "hours", (int) sizeof(val.hours), (char *) &val.hours - id);
  fprintf(out, _dform, "si16", "minutes", (int) sizeof(val.minutes), (char *) &val.minutes - id);
  fprintf(out, _dform, "si16", "seconds", (int) sizeof(val.seconds), (char *) &val.seconds - id);

  _printFormatDivider('-', out);

}

// print format of indep_freq_t block

void DoradeData::printFormat(const DoradeData::indep_freq_t &val, FILE *out)

{

  _printFormatDivider('-', out);
  fprintf(out, "  struct: 'indep_freq_t'\n  size: %d\n  id: INDF\n\n", (int) sizeof(val));
  const char *id = val.id;
  _printFormatHeader(out);
  
  fprintf(out, _dform, "char", "id[4]", (int) sizeof(val.id), (char *) val.id - id);
  fprintf(out, _dform, "si32", "nbytes", (int) sizeof(val.nbytes), (char *) &val.nbytes - id);

  _printFormatDivider('-', out);

}

// print format of minirims_data_t block

void DoradeData::printFormat(const DoradeData::minirims_data_t &val, FILE *out)

{

  _printFormatDivider('-', out);
  fprintf(out, "  struct: 'minirims_data_t'\n  size: %d\n  id: MINI\n\n", (int) sizeof(val));
  const char *id = val.id;
  _printFormatHeader(out);
  
  fprintf(out, _dform, "char", "id[4]", (int) sizeof(val.id), (char *) val.id - id);
  fprintf(out, _dform, "si32", "nbytes", (int) sizeof(val.nbytes), (char *) &val.nbytes - id);
  fprintf(out, _dform, "si16", "command", (int) sizeof(val.command), (char *) &val.command - id);
  fprintf(out, _dform, "si16", "status", (int) sizeof(val.status), (char *) &val.status - id);
  fprintf(out, _dform, "fl32", "temperature", (int) sizeof(val.temperature), (char *) &val.temperature - id);
  fprintf(out, _dform, "fl32", "x_axis_gyro[128]", (int) sizeof(val.x_axis_gyro), (char *) val.x_axis_gyro - id);
  fprintf(out, _dform, "fl32", "y_axis_gyro[128]", (int) sizeof(val.y_axis_gyro), (char *) val.y_axis_gyro - id);
  fprintf(out, _dform, "fl32", "z_axis_gyro[128]", (int) sizeof(val.z_axis_gyro), (char *) val.z_axis_gyro - id);
  fprintf(out, _dform, "fl32", "xr_axis_gyro[128]", (int) sizeof(val.xr_axis_gyro), (char *) val.xr_axis_gyro - id);
  fprintf(out, _dform, "fl32", "x_axis_vel[128]", (int) sizeof(val.x_axis_vel), (char *) val.x_axis_vel - id);
  fprintf(out, _dform, "fl32", "y_axis_vel[128]", (int) sizeof(val.y_axis_vel), (char *) val.y_axis_vel - id);
  fprintf(out, _dform, "fl32", "z_axis_vel[128]", (int) sizeof(val.z_axis_vel), (char *) val.z_axis_vel - id);
  fprintf(out, _dform, "fl32", "x_axis_pos[128]", (int) sizeof(val.x_axis_pos), (char *) val.x_axis_pos - id);

  _printFormatDivider('-', out);

}

// print format of nav_descript_t block

void DoradeData::printFormat(const DoradeData::nav_descript_t &val, FILE *out)

{

  _printFormatDivider('-', out);
  fprintf(out, "  struct: 'nav_descript_t'\n  size: %d\n  id: NDDS\n\n", (int) sizeof(val));
  const char *id = val.id;
  _printFormatHeader(out);
  
  fprintf(out, _dform, "char", "id[4]", (int) sizeof(val.id), (char *) val.id - id);
  fprintf(out, _dform, "si32", "nbytes", (int) sizeof(val.nbytes), (char *) &val.nbytes - id);
  fprintf(out, _dform, "si16", "ins_flag", (int) sizeof(val.ins_flag), (char *) &val.ins_flag - id);
  fprintf(out, _dform, "si16", "gps_flag", (int) sizeof(val.gps_flag), (char *) &val.gps_flag - id);
  fprintf(out, _dform, "si16", "minirims_flag", (int) sizeof(val.minirims_flag), (char *) &val.minirims_flag - id);
  fprintf(out, _dform, "si16", "kalman_flag", (int) sizeof(val.kalman_flag), (char *) &val.kalman_flag - id);

  _printFormatDivider('-', out);

}

// print format of time_series_t block

void DoradeData::printFormat(const DoradeData::time_series_t &val, FILE *out)

{

  _printFormatDivider('-', out);
  fprintf(out, "  struct: 'time_series_t'\n  size: %d\n  id: TIME\n\n", (int) sizeof(val));
  const char *id = val.id;
  _printFormatHeader(out);
  
  fprintf(out, _dform, "char", "id[4]", (int) sizeof(val.id), (char *) val.id - id);
  fprintf(out, _dform, "si32", "nbytes", (int) sizeof(val.nbytes), (char *) &val.nbytes - id);

  _printFormatDivider('-', out);

}

// print format of waveform_t block

void DoradeData::printFormat(const DoradeData::waveform_t &val, FILE *out)

{

  _printFormatDivider('-', out);
  fprintf(out, "  struct: 'waveform_t'\n  size: %d\n  id: WAVE\n\n", (int) sizeof(val));
  const char *id = val.id;
  _printFormatHeader(out);
  
  fprintf(out, _dform, "char", "id[4]", (int) sizeof(val.id), (char *) val.id - id);
  fprintf(out, _dform, "si32", "nbytes", (int) sizeof(val.nbytes), (char *) &val.nbytes - id);
  fprintf(out, _dform, "char", "ps_file_name[16]", (int) sizeof(val.ps_file_name), (char *) val.ps_file_name - id);
  fprintf(out, _dform, "si16", "num_chips[6]", (int) sizeof(val.num_chips), (char *) val.num_chips - id);
  fprintf(out, _dform, "char", "blank_chip[256]", (int) sizeof(val.blank_chip), (char *) val.blank_chip - id);
  fprintf(out, _dform, "fl32", "repeat_seq", (int) sizeof(val.repeat_seq), (char *) &val.repeat_seq - id);
  fprintf(out, _dform, "si16", "repeat_seq_dwel", (int) sizeof(val.repeat_seq_dwel), (char *) &val.repeat_seq_dwel - id);
  fprintf(out, _dform, "si16", "total_pcp", (int) sizeof(val.total_pcp), (char *) &val.total_pcp - id);
  fprintf(out, _dform, "si16", "chip_offset[6]", (int) sizeof(val.chip_offset), (char *) val.chip_offset - id);
  fprintf(out, _dform, "si16", "chip_width[6]", (int) sizeof(val.chip_width), (char *) val.chip_width - id);
  fprintf(out, _dform, "fl32", "ur_pcp", (int) sizeof(val.ur_pcp), (char *) &val.ur_pcp - id);
  fprintf(out, _dform, "fl32", "uv_pcp", (int) sizeof(val.uv_pcp), (char *) &val.uv_pcp - id);
  fprintf(out, _dform, "si16", "num_gates[6]", (int) sizeof(val.num_gates), (char *) val.num_gates - id);
  fprintf(out, _dform, "si16", "gate_dist1[2]", (int) sizeof(val.gate_dist1), (char *) val.gate_dist1 - id);
  fprintf(out, _dform, "si16", "gate_dist2[2]", (int) sizeof(val.gate_dist2), (char *) val.gate_dist2 - id);
  fprintf(out, _dform, "si16", "gate_dist3[2]", (int) sizeof(val.gate_dist3), (char *) val.gate_dist3 - id);
  fprintf(out, _dform, "si16", "gate_dist4[2]", (int) sizeof(val.gate_dist4), (char *) val.gate_dist4 - id);
  fprintf(out, _dform, "si16", "gate_dist5[2]", (int) sizeof(val.gate_dist5), (char *) val.gate_dist5 - id);

  _printFormatDivider('-', out);


}

///////////////////////////////////////////////////////////////
// byte swapping routines
// if force is false, only swaps on small-endian hosts
// if force is true, always swaps

void DoradeData::swap(DoradeData::comment_t &val,
                      bool force /* = false */)
{
  ByteOrder::swap32(&val.nbytes, 4 * 1, force);
}

void DoradeData::swap(DoradeData::super_SWIB_t &val,
                      bool force /* = false */)
{
  ByteOrder::swap32(&val.nbytes, 4 * 1, force);
  ByteOrder::swap32(&val.last_used, 4 * 7, force);
  ByteOrder::swap32(&val.pad, 4 * 1, force);
  ByteOrder::swap64(&val.d_start_time, 8 * 2, force);
  ByteOrder::swap32(&val.version_num, 4 * 10, force);
  ByteOrder::swap32(val.key_table, MAX_KEYS * sizeof(key_table_info_t), force);
}

void DoradeData::swap(DoradeData::super_SWIB_32bit_t &val,
                      bool force /* = false */)
{
  ByteOrder::swap32(&val.nbytes, 4 * 1, force);
  ByteOrder::swap32(&val.last_used, 4 * 7, force);
  ByteOrder::swap64(val.d_start_time, 8 * 2, force);
  ByteOrder::swap32(&val.version_num, 4 * 10, force);
  ByteOrder::swap32(val.key_table, MAX_KEYS * sizeof(key_table_info_t), force);
}

void DoradeData::swap(DoradeData::volume_t &val,
                      bool force /* = false */)
{
  ByteOrder::swap32(&val.nbytes, 4 * 1, force);
  ByteOrder::swap16(&val.format_version, 2 * 2, force);
  ByteOrder::swap32(&val.maximum_bytes, 4 * 1, force);
  ByteOrder::swap16(&val.year, 2 * 6, force);
  ByteOrder::swap16(&val.gen_year, 2 * 4, force);
}

void DoradeData::swapTimeToReasonable(DoradeData::volume_t &val)
{
  // Byte swap time based on reasonableness test.
  // May need to happen even if the other fields do not need swapping.

  RadxTime now(time(NULL));
  if ((val.year < 1980) || (val.year > now.getYear())) {
    ByteOrder::swap16(&val.year, 2, true);
  }
  if ((val.month < 1) || (val.month > 12)) {
    ByteOrder::swap16(&val.month, 2, true);
  }
  if ((val.day < 1) || (val.day > 31)) {
    ByteOrder::swap16(&val.day, 2, true);
  }
  if ((val.data_set_hour < 1) || (val.data_set_hour > 23)) {
    ByteOrder::swap16(&val.data_set_hour, 2, true);
  }
  if ((val.data_set_minute < 0) || (val.data_set_minute > 59)) {
    ByteOrder::swap16(&val.data_set_minute, 2, true);
  }
  if ((val.data_set_second < 0) || (val.data_set_second > 59)) {
    ByteOrder::swap16(&val.data_set_second, 2, true);
  }

}

void DoradeData::swap(DoradeData::radar_t &val,
                      bool force /* = false */)
{
  ByteOrder::swap32(&val.nbytes, 4 * 1, force);
  ByteOrder::swap32(&val.radar_const, 4 * 8, force);
  ByteOrder::swap16(&val.radar_type, 2 * 2, force);
  ByteOrder::swap32(&val.req_rotat_vel, 4 * 3, force);
  ByteOrder::swap16(&val.num_parameter_des, 2 * 4, force);
  ByteOrder::swap32(&val.data_red_parm0, 4 * 7, force);
  ByteOrder::swap16(&val.num_freq_trans, 2 * 2, force);
  ByteOrder::swap32(&val.freq1, 4 * 11, force);
  ByteOrder::swap32(&val.config_num, 4 * 31, force);
}

void DoradeData::swap(DoradeData::correction_t &val,
                      bool force /* = false */)
{
  ByteOrder::swap32(&val.nbytes, 4 * 1, force);
  ByteOrder::swap32(&val.azimuth_corr, 4 * 16, force);
}

void DoradeData::swap(DoradeData::parameter_t &val,
                      bool force /* = false */)
{
  ByteOrder::swap32(&val.nbytes, 4 * 1, force);
  ByteOrder::swap16(&val.interpulse_time, 2 * 2, force);
  ByteOrder::swap32(&val.recvr_bandwidth, 4 * 1, force);
  ByteOrder::swap16(&val.pulse_width, 2 * 4, force);
  ByteOrder::swap32(&val.threshold_value, 4 * 5, force);
  ByteOrder::swap32(&val.config_num, 4 * 4, force);
  ByteOrder::swap32(&val.num_criteria, 4 * 1, force);
  ByteOrder::swap32(&val.number_cells, 4 * 4, force);
}

void DoradeData::swap(DoradeData::cell_vector_t &val,
                      bool force /* = false */)
{
  ByteOrder::swap32(&val.nbytes, 4 * 1, force);
  ByteOrder::swap32(&val.number_cells, 4 * 1, force);
  ByteOrder::swap32(val.dist_cells, 4 * MAXCVGATES, force);
}

void DoradeData::swap(DoradeData::cell_spacing_fp_t &val,
                      bool force /* = false */)
{
  ByteOrder::swap32(&val.nbytes, 4 * 1, force);
  ByteOrder::swap32(&val.num_segments, 4 * 10, force);
  ByteOrder::swap16(&val.num_cells, 2 * 8, force);
}

void DoradeData::swap(DoradeData::sweepinfo_t &val,
                      bool force /* = false */)
{
  ByteOrder::swap32(&val.nbytes, 4 * 1, force);
  ByteOrder::swap32(&val.sweep_num, 4 * 6, force);
}

void DoradeData::swap(DoradeData::platform_t &val,
                      bool force /* = false */)
{
  ByteOrder::swap32(&val.nbytes, 4 * 1, force);
  ByteOrder::swap32(&val.longitude, 4 * 18, force);
}

void DoradeData::swap(DoradeData::ray_t &val,
                      bool force /* = false */)
{
  ByteOrder::swap32(&val.nbytes, 4 * 1, force);
  ByteOrder::swap32(&val.sweep_num, 4 * 2, force);
  ByteOrder::swap16(&val.hour, 2 * 4, force);
  ByteOrder::swap32(&val.azimuth, 4 * 5, force);
}

void DoradeData::swap(DoradeData::paramdata_t &val,
                      bool force /* = false */)
{
  ByteOrder::swap32(&val.nbytes, 4 * 1, force);
}

void DoradeData::swap(DoradeData::qparamdata_t &val,
                      bool force /* = false */)
{
  ByteOrder::swap32(&val.nbytes, 4 * 1, force);
  ByteOrder::swap32(&val.extension_num, 4 * 2, force);
  ByteOrder::swap16(val.first_cell, 2 * 8, force);
  ByteOrder::swap32(val.criteria_value, 4 * 4, force);
}

void DoradeData::swap(DoradeData::extra_stuff_t &val,
                      bool force /* = false */)
{
  ByteOrder::swap32(&val.nbytes, 4 * 1, force);
  ByteOrder::swap32(&val.one, 4 * 4, force);
}

void DoradeData::swap(DoradeData::null_block_t &val,
                      bool force /* = false */)
{
  ByteOrder::swap32(&val.nbytes, 4 * 1, force);
}

void DoradeData::swap(DoradeData::rot_angle_table_t &val,
                      bool force /* = false */)
{
  ByteOrder::swap32(&val.nbytes, 4 * 1, force);
  ByteOrder::swap32(&val.angle2ndx, 4 * 5, force);
}

void DoradeData::swap(DoradeData::rot_table_entry_t &val,
                      bool force /* = false */)
{
  ByteOrder::swap32(&val.rotation_angle, 4 * 3, force);
}

void DoradeData::swap(DoradeData::radar_test_status_t &val,
                      bool force /* = false */)
{
  ByteOrder::swap32(&val.nbytes, 4 * 1, force);
  ByteOrder::swap32(&val.data_sys_status, 4 * 1, force);
  ByteOrder::swap32(&val.test_pulse_level, 4 * 4, force);
  ByteOrder::swap16(&val.test_pulse_atten, 2 * 2, force);
  ByteOrder::swap32(&val.noise_power, 4 * 2, force);
  ByteOrder::swap16(&val.first_rec_gate, 2 * 2, force);
}

void DoradeData::swap(DoradeData::field_radar_t &val,
                      bool force /* = false */)
{
  ByteOrder::swap32(&val.nbytes, 4 * 1, force);
  ByteOrder::swap32(&val.data_sys_id, 4 * 42, force);
  ByteOrder::swap16(&val.indepf_times_flg, 2 * 4, force);
}

void DoradeData::swap(DoradeData::lidar_t &val,
                      bool force /* = false */)
{
  ByteOrder::swap32(&val.nbytes, 4 * 1, force);
  ByteOrder::swap32(&val.lidar_const, 4 * 8, force);
  ByteOrder::swap16(&val.lidar_type, 2 * 2, force);
  ByteOrder::swap32(&val.req_rotat_vel, 4 * 3, force);
  ByteOrder::swap16(&val.num_parameter_des, 2 * 4, force);
  ByteOrder::swap32(&val.data_red_parm0, 4 * 19, force);
}

void DoradeData::swap(DoradeData::field_lidar_t &val,
                      bool force /* = false */)
{
  ByteOrder::swap32(&val.nbytes, 4 * 1, force);
  ByteOrder::swap32(&val.data_sys_id, 4 * 105, force);
}

void DoradeData::swap(DoradeData::insitu_descript_t &val,
                      bool force /* = false */)
{
  ByteOrder::swap32(&val.nbytes, 4 * 1, force);
  ByteOrder::swap32(&val.number_params, 4 * 1, force);
}

void DoradeData::swap(DoradeData::insitu_data_t &val,
                      bool force /* = false */)
{
  ByteOrder::swap32(&val.nbytes, 4 * 1, force);
  ByteOrder::swap16(&val.julian_day, 2 * 4, force);
}

void DoradeData::swap(DoradeData::indep_freq_t &val,
                      bool force /* = false */)
{
  ByteOrder::swap32(&val.nbytes, 4 * 1, force);
}

void DoradeData::swap(DoradeData::minirims_data_t &val,
                      bool force /* = false */)
{
  ByteOrder::swap32(&val.nbytes, 4 * 1, force);
  ByteOrder::swap16(&val.command, 2 * 2, force);
  ByteOrder::swap32(&val.temperature, 4 * 1025, force);
}

void DoradeData::swap(DoradeData::nav_descript_t &val,
                      bool force /* = false */)
{
  ByteOrder::swap32(&val.nbytes, 4 * 1, force);
  ByteOrder::swap16(&val.ins_flag, 2 * 4, force);
}

void DoradeData::swap(DoradeData::time_series_t &val,
                      bool force /* = false */)
{
  ByteOrder::swap32(&val.nbytes, 4 * 1, force);
}

void DoradeData::swap(DoradeData::waveform_t &val,
                      bool force /* = false */)
{
  ByteOrder::swap32(&val.nbytes, 4 * 1, force);
  ByteOrder::swap16(val.num_chips, 2 * 16, force);
  ByteOrder::swap32(&val.repeat_seq, 4 * 1, force);
  ByteOrder::swap16(&val.repeat_seq_dwel, 2 * 14, force);
  ByteOrder::swap32(&val.ur_pcp, 4 * 2, force);
  ByteOrder::swap16(&val.num_gates, 2 * 16, force);
}

/////////////////////////////////////
// conversions to string from enums

string DoradeData::binaryFormatToStr(DoradeData::binary_format_t format)
{
  switch (format) {
    case DoradeData::BINARY_FORMAT_INT8: {
      return "BINARY_FORMAT_INT8";
    }
    case DoradeData::BINARY_FORMAT_INT16: {
      return "BINARY_FORMAT_INT16";
    }
    case DoradeData::BINARY_FORMAT_INT32: {
      return "BINARY_FORMAT_INT32";
    }
    case DoradeData::BINARY_FORMAT_FLOAT32: {
      return "BINARY_FORMAT_FLOAT32";
    }
    default:
      return "UNKNOWN";
  }
}

string DoradeData::radarTypeToStr(DoradeData::radar_type_t rtype)
{
  switch (rtype) {
    case DoradeData::RADAR_GROUND: {
      return "RADAR_GROUND";
    }
    case DoradeData::RADAR_AIR_FORE: {
      return "RADAR_AIR_FORE";
    }
    case DoradeData::RADAR_AIR_AFT: {
      return "RADAR_AIR_AFT";
    }
    case DoradeData::RADAR_AIR_TAIL: {
      return "RADAR_AIR_TAIL";
    }
    case DoradeData::RADAR_AIR_LF: {
      return "RADAR_AIR_LF";
    }
    case DoradeData::RADAR_SHIP: {
      return "RADAR_SHIP";
    }
    case DoradeData::RADAR_AIR_NOSE: {
      return "RADAR_AIR_NOSE";
    }
    case DoradeData::RADAR_SATELLITE: {
      return "RADAR_SATELLITE";
    }
    case DoradeData::LIDAR_MOVING: {
      return "LIDAR_MOVING";
    }
    case DoradeData::LIDAR_FIXED: {
      return "LIDAR_FIXED";
    }
    default:
      return "UNKNOWN";
  }
}

string DoradeData::lidarTypeToStr(DoradeData::lidar_type_t rtype)
{
  switch (rtype) {
    case DoradeData::LIDAR_GROUND: {
      return "LIDAR_GROUND";
    }
    case DoradeData::LIDAR_AIR_FORE: {
      return "LIDAR_AIR_FORE";
    }
    case DoradeData::LIDAR_AIR_AFT: {
      return "LIDAR_AIR_AFT";
    }
    case DoradeData::LIDAR_AIR_TAIL: {
      return "LIDAR_AIR_TAIL";
    }
    case DoradeData::LIDAR_AIR_LF: {
      return "LIDAR_AIR_LF";
    }
    case DoradeData::LIDAR_SHIP: {
      return "LIDAR_SHIP";
    }
    case DoradeData::LIDAR_AIR_FIXED: {
      return "LIDAR_AIR_FIXED";
    }
    case DoradeData::LIDAR_SATELLITE: {
      return "LIDAR_SATELLITE";
    }
    default:
      return "UNKNOWN";
  }
}

string DoradeData::fieldIdToStr(DoradeData::field_id_t id)
{
  switch (id) {
    case DoradeData::SW_ID_NUM: {
      return "SW_ID_NUM";
    }
    case DoradeData::VR_ID_NUM: {
      return "VR_ID_NUM";
    }
    case DoradeData::NCP_ID_NUM: {
      return "NCP_ID_NUM";
    }
    case DoradeData::DBZ_ID_NUM: {
      return "DBZ_ID_NUM";
    }
    case DoradeData::DZ_ID_NUM: {
      return "DZ_ID_NUM";
    }
    case DoradeData::VE_ID_NUM: {
      return "VE_ID_NUM";
    }
    case DoradeData::VG_ID_NUM: {
      return "VG_ID_NUM";
    }
    case DoradeData::VU_ID_NUM: {
      return "VU_ID_NUM";
    }
    case DoradeData::VD_ID_NUM: {
      return "VD_ID_NUM";
    }
    case DoradeData::DBM_ID_NUM: {
      return "DBM_ID_NUM";
    }
    default:
      return "UNKNOWN";
  }
}

string DoradeData::polarizationToStr(DoradeData::polarization_t ptype)
{
  switch (ptype) {
    case DoradeData::POLARIZATION_HORIZONTAL: {
      return "POLARIZATION_HORIZONTAL";
    }
    case DoradeData::POLARIZATION_VERTICAL: {
      return "POLARIZATION_VERTICAL";
    }
    case DoradeData::POLARIZATION_CIRCULAR_RIGHT: {
      return "POLARIZATION_CIRCULAR_RIGHT";
    }
    case DoradeData::POLARIZATION_ELLIPTICAL: {
      return "POLARIZATION_ELLIPTICAL";
    }
    case DoradeData::POLARIZATION_HV_ALT: {
      return "POLARIZATION_HV_ALT";
    }
    case DoradeData::POLARIZATION_HV_SIM: {
      return "POLARIZATION_HV_SIM";
    }
    default:
      return "UNKNOWN";
  }
}

string DoradeData::scanModeToStr(DoradeData::scan_mode_t mode)
{
  switch (mode) {
    case DoradeData::SCAN_MODE_CAL: {
      return "SCAN_MODE_CAL";
    }
    case DoradeData::SCAN_MODE_PPI: {
      return "SCAN_MODE_PPI";
    }
    case DoradeData::SCAN_MODE_COP: {
      return "SCAN_MODE_COP";
    }
    case DoradeData::SCAN_MODE_RHI: {
      return "SCAN_MODE_RHI";
    }
    case DoradeData::SCAN_MODE_VER: {
      return "SCAN_MODE_VER";
    }
    case DoradeData::SCAN_MODE_TAR: {
      return "SCAN_MODE_TAR";
    }
    case DoradeData::SCAN_MODE_MAN: {
      return "SCAN_MODE_MAN";
    }
    case DoradeData::SCAN_MODE_IDL: {
      return "SCAN_MODE_IDL";
    }
    case DoradeData::SCAN_MODE_SUR: {
      return "SCAN_MODE_SUR";
    }
    case DoradeData::SCAN_MODE_AIR: {
      return "SCAN_MODE_AIR";
    }
    case DoradeData::SCAN_MODE_HOR: {
      return "SCAN_MODE_HOR";
    }
    default:
      return "UNKNOWN";
  }
}

string DoradeData::scanModeToShortStr(DoradeData::scan_mode_t mode)
{
  switch (mode) {
    case DoradeData::SCAN_MODE_CAL: {
      return "CAL";
    }
    case DoradeData::SCAN_MODE_PPI: {
      return "PPI";
    }
    case DoradeData::SCAN_MODE_COP: {
      return "COP";
    }
    case DoradeData::SCAN_MODE_RHI: {
      return "RHI";
    }
    case DoradeData::SCAN_MODE_VER: {
      return "VER";
    }
    case DoradeData::SCAN_MODE_TAR: {
      return "TAR";
    }
    case DoradeData::SCAN_MODE_MAN: {
      return "MAN";
    }
    case DoradeData::SCAN_MODE_IDL: {
      return "IDL";
    }
    case DoradeData::SCAN_MODE_SUR: {
      return "SUR";
    }
    case DoradeData::SCAN_MODE_AIR: {
      return "AIR";
    }
    case DoradeData::SCAN_MODE_HOR: {
      return "HOR";
    }
    default:
      return "XXX";
  }
}

/////////////////////////////////////
// conversions to int from enum and vice versa
  
DoradeData::primary_axis_t DoradeData::primaryAxisFromInt(Radx::ui32 value)
{

  switch (value) {
  case 1:
    return DoradeData::Z;
    break;
  case 2:
    return DoradeData::Y;
    break;
  case 3:
    return DoradeData::X;
    break;
  case 4:
    return DoradeData::Z_PRIME;
    break;
  case 5:
    return DoradeData::Y_PRIME;
    break;
  case 6:
    return DoradeData::X_PRIME;
    break;
  default:
    throw "Unrecognized value for primary axis of rotation";
  }
}

Radx::ui32 DoradeData::primaryAxisToInt(DoradeData::primary_axis_t ptype)
{

  switch (ptype) {
  case DoradeData::Z:
    return 1;
    break;
  case DoradeData::Y:
    return 2;
    break;
  case DoradeData::X:
    return 3;
    break;
  case DoradeData::Z_PRIME:
    return 4;
    break;
  case DoradeData::Y_PRIME:
    return 5;
    break;
  case DoradeData::X_PRIME:
    return 6;
    break;
  default:
    throw "Unrecognized primary axis of rotation type";
  }
}

Radx::PrimaryAxis_t DoradeData::convertToRadxType(DoradeData::primary_axis_t doradeAxis)
{

  switch (doradeAxis) {
  case DoradeData::Z:
    return Radx::PRIMARY_AXIS_Z;
    break;
  case DoradeData::Y:
    return Radx::PRIMARY_AXIS_Y;
    break;
  case DoradeData::X:
    return Radx::PRIMARY_AXIS_X;
    break;
  case DoradeData::Z_PRIME:
    return Radx::PRIMARY_AXIS_Z_PRIME;
    break;
  case DoradeData::Y_PRIME:
    return Radx::PRIMARY_AXIS_Y_PRIME;
    break;
  case DoradeData::X_PRIME:
    return Radx::PRIMARY_AXIS_X_PRIME;
    break;
  default:
    throw "Unrecognized DORADE primary axis of rotation";
  }
}

DoradeData::primary_axis_t  DoradeData::convertToDoradeType(Radx::PrimaryAxis_t radxAxis)
{

  switch (radxAxis) {
  case Radx::PRIMARY_AXIS_Z:
    return DoradeData::Z;
    break;
  case Radx::PRIMARY_AXIS_Y:
    return DoradeData::Y;
    break;
  case Radx::PRIMARY_AXIS_X:
    return DoradeData::X;
    break;
  case Radx::PRIMARY_AXIS_Z_PRIME:
    return DoradeData::Z_PRIME;
    break;
  case Radx::PRIMARY_AXIS_Y_PRIME:
    return DoradeData::Y_PRIME;
    break;
  case Radx::PRIMARY_AXIS_X_PRIME:
    return DoradeData::X_PRIME;
    break;
  default:
    throw "Unrecognized Radx primary axis of rotation type";
  }
}


//////////////////////////////////////////////////////////////////
// decompress HRD 16-bit data
// routine to unpacks actual data assuming MIT/HRD compression
//
// The compression scheme simply encodes the bad values as a run,
// storing the run length. Good data is stored unchanged.
//
// Assumes data is in native byte order - i.e. it has already been
// swapped as appropriate.
//
// inputs:
//   comp: compressed data
//   n_comp: number of compressed words
//   bad_val: value to be used for bad data value
//
// output:
//   uncomp: uncompressed data
//           uncomp must be size at least max_uncomp
//   n_bads: number of bad values
//
// returns:
//   number of 16-bit words in uncompressed data

# define END_OF_COMPRESSION 1

# define SIGN16 0x8000
# define MASK15 0x7fff

# define MASK7 0x7f
# define SIGN8 0x80

int DoradeData::decompressHrd16(const unsigned short *comp, int n_comp,
                                unsigned short *uncomp, int max_uncomp,
                                int bad_val, int *n_bads)
{
  
  const unsigned short *us = comp;
  int nn = 0, nw =0;
  *n_bads = 0;
  
  while(true) {
    
    if(*us == END_OF_COMPRESSION || *us == 0) {
      return(nw);
    }
    
    nn = *us & MASK15;
    
    if( *us & SIGN16 ) { // data!
      us++;
      if(nw + nn > max_uncomp) {
        return(nw);
      }
      *n_bads = 0;
      for(; nn--;) {
        nw++;
        *uncomp++ = *us++;
      }
    } else { // fill with nulls
      if(*n_bads) {
        return(nw); // some other kind of termination flag
      }
      if(nw + nn > max_uncomp) {
        return(nw);
      }
      *n_bads = nn;
      us++;
      for(; nn--;) {
        nw++;
        *uncomp++ = bad_val;
      }
    }

  } // while
  
}

//////////////////////////////////////////////////////////////////
// implement hrd compression of 16-bit values
// and return the number of 16-bit words of compressed data
//
// The compression scheme simply encodes the bad values as a run,
// storing the run length. Good data is stored unchanged.
//
// Assumes data is in native host byte order
//
// inputs:
//   uncomp: uncompressed data
//   n_uncomp: number of uncompressed words
//   bad_val: value to be used for bad data value
//   max_comp: max number of compressed words,
//             should be at least n_uncomp + 16
// output:
//   comp: compressed data
//         uncomp must be size max_comp
//
// returns:
//   number of 16-bit words in compressed data

int DoradeData::compressHrd16(const unsigned short *uncomp, int n_uncomp,
                              unsigned short *comp, int max_comp,
                              unsigned short bad_val)
  
{

  int kount = 0, wcount = 0, data_run = 0;
  int YES = 1, NO = 0;
  const unsigned short *ss = uncomp;
  const unsigned short *end = uncomp + n_uncomp - 1;
  unsigned short *dd=comp;
  unsigned short *rlcode = NULL;

  if(n_uncomp < 2) {
    // Trying to compress less than 2 values
    memcpy(comp, uncomp, n_uncomp * sizeof(unsigned short));
    return n_uncomp;
  }

  for(;ss < end;) {
    
    // for each run examine the first two values
    
    kount = 2;
    rlcode = dd++;
    if(*(ss+1) != bad_val || *ss != bad_val) { // data run
      data_run = YES;
      *dd++ = *ss++;
      *dd++ = *ss++;
    } else { // bad_val run
      data_run = NO;
      ss += 2;
    }
    
    for(;ss < end;) { // for rest of the run
      if(data_run) {
        if(*(ss-1) == bad_val && *ss == bad_val && kount > 2) {
          // break data run
          *rlcode = SIGN16 | --kount;
          wcount += kount+1; // data plus code word
          ss--;
          dd--;
          break;
        }
        // continue the data run
        kount++;
        *dd++ = *ss++;
      }
      else { // bad_val run
        if(*ss != bad_val) { // break bad_val run
          *rlcode = kount;
          wcount++; // code word only
          break;
        }
        ss++;
        kount++; // continue bad_val run
      }
    } // secondary looop

  } // main loop

  // now look at the last value

  if(data_run) { // just stuff it no matter what it is
    if(ss == end) {
      *dd++ = *ss;
      kount++;
    }
    *rlcode = SIGN16 | kount;
    wcount += kount +1;
  } else if (*ss == bad_val) {
    *rlcode = ++kount;
    wcount++;
  } else { // there is one last datum at the end of a bad_val run
    if(kount == 2) {	// special case for just two bad_vals
      *rlcode = SIGN16 | 3;
      *dd++ = bad_val;
      *dd++ = bad_val;
      *dd++ = *ss;
      wcount += 4;
    }
    else {
      *rlcode = --kount;
      wcount++;
      *dd++ = SIGN16 | 2;
      *dd++ = bad_val;
      *dd++ = *ss;
      wcount += 3;
    }
  }
  *dd++ = END_OF_COMPRESSION;
  wcount++;
  return(wcount);

}

#ifdef JUNK

radar_test_status_t
field_radar_t
lidar_t
field_lidar_t
insitu_descript_t
insitu_data_t
indep_freq_t
minirims_data_t
nav_descript_t
time_series_t
waveform_t

#endif
