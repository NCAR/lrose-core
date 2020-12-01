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
///////////////////////////////////////////////////////////////
// IpsTsSim.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2019
//
///////////////////////////////////////////////////////////////
//
// Support for Independent Pulse Sampling.
//
// IpsTsSim reads IWRF data from specified files, converts
// the data to IPS TS format, and writes the
// converted files to a specified location
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <cerrno>
#include <cstdio>
#include <unistd.h>

#include <toolsa/str.h>

#include "IpsTsSim.hh"
#include "WriteToFile.hh"
#include "WriteToUdp.hh"
#include "WriteToFmq.hh"
#include "ReadFromUdp.hh"

using namespace std;

// Constructor

IpsTsSim::IpsTsSim(int argc, char **argv)
  
{

  isOK = true;
  
  // set programe name
  
  _progName = "IpsTsSim";
  
  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
    return;
  }

  // check params

  if (_args.inputFileList.size() < 1) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  No input time series file specified" << endl;
    cerr << "  You need to specify at least 1 file" << endl;
    cerr << "  Use -f option" << endl;
    _args.usage(_progName, cerr);
    isOK = false;
    return;
  }

  // init process mapper registration

  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

}

// destructor

IpsTsSim::~IpsTsSim()

{
  
  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int IpsTsSim::Run ()
{
  
  PMU_auto_register("Run");
  
  if (_params.run_mode == Params::WRITE_TO_FILE) {
    return _runWriteToFile();
  } else if (_params.run_mode == Params::WRITE_TO_UDP) {
    return _runWriteToUdp();
  } else if (_params.run_mode == Params::WRITE_TO_FMQ) {
    return _runWriteToFmq();
  } else {
    return _runReadFromUdp();
  }

}

//////////////////////////////////////////////////
// Run in file mode

int IpsTsSim::_runWriteToFile()
{
  
  PMU_auto_register("_runWriteToFile");
  
  WriteToFile convertToIps(_progName,
                            _params,
                            _args.inputFileList);
  
  return convertToIps.Run();

}

//////////////////////////////////////////////////
// Run in write to UDP mode

int IpsTsSim::_runWriteToUdp()
{
  
  PMU_auto_register("_runWriteToUdp");
 
  WriteToUdp writeToUdp(_progName,
                        _params,
                        _args.inputFileList);

  return writeToUdp.Run();

}

//////////////////////////////////////////////////
// Run in write to FMQ mode

int IpsTsSim::_runWriteToFmq()
{
  
  PMU_auto_register("_runWriteToFmq");
 
  WriteToFmq writeToFmq(_progName,
                        _params,
                        _args.inputFileList);

  return writeToFmq.Run();

}

//////////////////////////////////////////////////
// Run in read from UDP mode

int IpsTsSim::_runReadFromUdp()
{
  
  PMU_auto_register("_runReadFromUdp");
  
  ReadFromUdp readFromUdp(_progName,
                          _params,
                          _args.inputFileList);
  
  return readFromUdp.Run();
  
}

////////////////////////////////////
// condition angle from 0 to 360

double IpsTsSim::conditionAngle360(double angle)
{
  if (angle < 0) {
    return angle + 360.0;
  } else if (angle >= 360.0) {
    return angle - 360.0;
  } else {
    return angle;
  }
}

////////////////////////////////////
// condition angle from -180 to 180

double IpsTsSim::conditionAngle180(double angle)
{
  if (angle < -180) {
    return angle + 360.0;
  } else if (angle >= 180) {
    return angle - 360.0;
  } else {
    return angle;
  }
}

//////////////////////////////////////////////////
// Copy members from IWRF structs to IPS structs

void IpsTsSim::copyIwrf2Ips(const iwrf_packet_info_t &iwrf,
                              ips_ts_packet_info_t &ips)
{

  ips.len_bytes = iwrf.len_bytes;
  ips.seq_num = iwrf.seq_num;
  ips.version_num = iwrf.version_num;
  ips.radar_id = iwrf.radar_id;
  ips.time_secs_utc = iwrf.time_secs_utc;
  ips.time_nano_secs = iwrf.time_nano_secs;
 
}

void IpsTsSim::copyIwrf2Ips(const iwrf_radar_info_t &iwrf,
                              ips_ts_radar_info_t &ips)
{

  ips_ts_radar_info_init(ips);

  copyIwrf2Ips(iwrf.packet, ips.packet);

  ips.latitude_deg = iwrf.latitude_deg;
  ips.longitude_deg = iwrf.longitude_deg;
  ips.altitude_m = iwrf.altitude_m;
  ips.platform_type = iwrf.platform_type;
  ips.beamwidth_deg_h = iwrf.beamwidth_deg_h;
  ips.beamwidth_deg_v = iwrf.beamwidth_deg_v;
  ips.wavelength_cm = iwrf.wavelength_cm;
  ips.nominal_gain_ant_db_h = iwrf.nominal_gain_ant_db_h;
  ips.nominal_gain_ant_db_v = iwrf.nominal_gain_ant_db_v;

  STRncopy(ips.radar_name, "IPS-SIM", IWRF_MAX_RADAR_NAME);
  STRncopy(ips.site_name, "C130", IWRF_MAX_SITE_NAME);

}

void IpsTsSim::copyIwrf2Ips(const iwrf_scan_segment_t &iwrf,
                              ips_ts_scan_segment_t &ips)
{

  ips_ts_scan_segment_init(ips);

  copyIwrf2Ips(iwrf.packet, ips.packet);

  ips.scan_mode = iwrf.scan_mode;
  ips.volume_num = iwrf.volume_num;
  ips.sweep_num = iwrf.sweep_num;

  ips.az_start = iwrf.az_start;
  ips.el_start = iwrf.el_start;
  ips.scan_rate = iwrf.scan_rate;
  ips.left_limit = iwrf.left_limit;
  ips.right_limit = iwrf.right_limit;
  ips.up_limit = iwrf.up_limit;
  ips.down_limit = iwrf.down_limit;
  ips.step = iwrf.step;
  
  ips.current_fixed_angle = iwrf.current_fixed_angle;

  ips.n_sweeps = iwrf.n_sweeps;

  memcpy(ips.fixed_angles,
         iwrf.fixed_angles,
         IWRF_MAX_FIXED_ANGLES * sizeof(fl32));
  
  ips.sun_scan_sector_width_az = iwrf.sun_scan_sector_width_az;
  ips.sun_scan_sector_width_el = iwrf.sun_scan_sector_width_el;

  STRncopy(ips.segment_name, iwrf.segment_name, IWRF_MAX_SEGMENT_NAME);
  STRncopy(ips.project_name, iwrf.project_name, IWRF_MAX_PROJECT_NAME);

}

void IpsTsSim::copyIwrf2Ips(const iwrf_ts_processing_t &iwrf,
                              ips_ts_processing_t &ips)
{

  ips_ts_processing_init(ips);

  copyIwrf2Ips(iwrf.packet, ips.packet);

  ips.pol_mode = iwrf.pol_mode;
  ips.prf_mode = iwrf.prf_mode;
  ips.pulse_shape = iwrf.pulse_type;
  ips.pulse_width_us = iwrf.pulse_width_us;
  ips.start_range_m = iwrf.start_range_m;
  ips.gate_spacing_m = iwrf.gate_spacing_m;

  ips.test_pulse_range_km = iwrf.test_pulse_range_km;
  ips.test_pulse_length_us = iwrf.test_pulse_length_usec;
  
  ips.num_prts = iwrf.num_prts;
  ips.prt_us[0] = iwrf.prt_usec;
  ips.prt_us[1] = iwrf.prt2_usec;
  ips.prt_us[2] = iwrf.prt3_usec;
  ips.prt_us[3] = iwrf.prt4_usec;
  
}

void IpsTsSim::copyIwrf2Ips(const iwrf_calibration_t &iwrf,
                              ips_ts_calibration_t &ips)
{

  ips_ts_calibration_init(ips);

  copyIwrf2Ips(iwrf.packet, ips.packet);

  ips.wavelength_cm = iwrf.wavelength_cm;
  ips.beamwidth_deg_h = iwrf.beamwidth_deg_h;
  ips.beamwidth_deg_v = iwrf.beamwidth_deg_v;
  ips.gain_ant_db_h = iwrf.gain_ant_db_h;
  ips.gain_ant_db_v = iwrf.gain_ant_db_v;
  ips.pulse_width_us = iwrf.pulse_width_us;
  ips.xmit_power_dbm_h = iwrf.xmit_power_dbm_h;
  ips.xmit_power_dbm_v = iwrf.xmit_power_dbm_v;
  ips.two_way_waveguide_loss_db_h = iwrf.two_way_waveguide_loss_db_h;
  ips.two_way_waveguide_loss_db_v = iwrf.two_way_waveguide_loss_db_v;
  ips.two_way_radome_loss_db_h = iwrf.two_way_radome_loss_db_h;
  ips.two_way_radome_loss_db_v = iwrf.two_way_radome_loss_db_v;
  ips.receiver_mismatch_loss_db = iwrf.receiver_mismatch_loss_db;
  ips.radar_constant_h = iwrf.radar_constant_h;
  ips.radar_constant_v = iwrf.radar_constant_v;
  ips.noise_dbm_hc = iwrf.noise_dbm_hc;
  ips.noise_dbm_hx = iwrf.noise_dbm_hx;
  ips.noise_dbm_vc = iwrf.noise_dbm_vc;
  ips.noise_dbm_vx = iwrf.noise_dbm_vx;
  ips.receiver_gain_db_hc = iwrf.receiver_gain_db_hc;
  ips.receiver_gain_db_hx = iwrf.receiver_gain_db_hx;
  ips.receiver_gain_db_vc = iwrf.receiver_gain_db_vc;
  ips.receiver_gain_db_vx = iwrf.receiver_gain_db_vx;
  ips.base_dbz_1km_hc = iwrf.base_dbz_1km_hc;
  ips.base_dbz_1km_hx = iwrf.base_dbz_1km_hx;
  ips.base_dbz_1km_vc = iwrf.base_dbz_1km_vc;
  ips.base_dbz_1km_vx = iwrf.base_dbz_1km_vx;
  ips.sun_power_dbm_hc = iwrf.sun_power_dbm_hc;
  ips.sun_power_dbm_hx = iwrf.sun_power_dbm_hx;
  ips.sun_power_dbm_vc = iwrf.sun_power_dbm_vc;
  ips.sun_power_dbm_vx = iwrf.sun_power_dbm_vx;
  ips.noise_source_power_dbm_h = iwrf.noise_source_power_dbm_h;
  ips.noise_source_power_dbm_v = iwrf.noise_source_power_dbm_v;
  ips.power_meas_loss_db_h = iwrf.power_meas_loss_db_h;
  ips.power_meas_loss_db_v = iwrf.power_meas_loss_db_v;
  ips.coupler_forward_loss_db_h = iwrf.coupler_forward_loss_db_h;
  ips.coupler_forward_loss_db_v = iwrf.coupler_forward_loss_db_v;
  ips.test_power_dbm_h = iwrf.test_power_dbm_h;
  ips.test_power_dbm_v = iwrf.test_power_dbm_v;
  ips.zdr_correction_db = iwrf.zdr_correction_db;
  ips.ldr_correction_db_h = iwrf.ldr_correction_db_h;
  ips.ldr_correction_db_v = iwrf.ldr_correction_db_v;
  ips.phidp_rot_deg = iwrf.phidp_rot_deg;
  ips.receiver_slope_hc = iwrf.receiver_slope_hc;
  ips.receiver_slope_hx = iwrf.receiver_slope_hx;
  ips.receiver_slope_vc = iwrf.receiver_slope_vc;
  ips.receiver_slope_vx = iwrf.receiver_slope_vx;
  ips.i0_dbm_hc = iwrf.i0_dbm_hc;
  ips.i0_dbm_hx = iwrf.i0_dbm_hx;
  ips.i0_dbm_vc = iwrf.i0_dbm_vc;
  ips.i0_dbm_vx = iwrf.i0_dbm_vx;
  ips.dynamic_range_db_hc = iwrf.dynamic_range_db_hc;
  ips.dynamic_range_db_hx = iwrf.dynamic_range_db_hx;
  ips.dynamic_range_db_vc = iwrf.dynamic_range_db_vc;
  ips.dynamic_range_db_vx = iwrf.dynamic_range_db_vx;
  ips.k_squared_water = iwrf.k_squared_water;
  ips.dbz_correction = iwrf.dbz_correction;

  STRncopy(ips.radar_name, "IPS-SIM", IWRF_MAX_RADAR_NAME);
  
}

void IpsTsSim::copyIwrf2Ips(const iwrf_pulse_header_t &iwrf,
                              ips_ts_pulse_header_t &ips)
{

  ips_ts_pulse_header_init(ips);

  copyIwrf2Ips(iwrf.packet, ips.packet);

  ips.pulse_seq_num = iwrf.pulse_seq_num;

  ips.scan_mode = iwrf.scan_mode;
  ips.volume_num = iwrf.volume_num;
  ips.sweep_num = iwrf.sweep_num;

  ips.elevation = iwrf.elevation;
  ips.azimuth = iwrf.azimuth;
  
  if (iwrf.scan_mode == IWRF_SCAN_MODE_RHI ||
      iwrf.scan_mode == IWRF_SCAN_MODE_EL_SUR_360) {
    ips.fixed_angle = iwrf.fixed_az;
  } else {
    ips.fixed_angle = iwrf.fixed_el;
  }
  
  ips.prt = iwrf.prt;
  ips.prt_next = iwrf.prt_next;
  
  ips.pulse_width_us = iwrf.pulse_width_us;

  ips.n_gates = iwrf.n_gates;

  ips.n_channels = iwrf.n_channels;
  ips.iq_encoding = iwrf.iq_encoding;
  ips.hv_flag = iwrf.hv_flag;

  ips.phase_cohered = iwrf.phase_cohered;
  
  ips.status = iwrf.status;

  ips.n_data = iwrf.n_data;
  
  ips.scale = iwrf.scale;
  ips.offset = iwrf.offset;

  ips.start_range_m = iwrf.start_range_m;
  ips.gate_spacing_m = iwrf.gate_spacing_m;

  ips.event_flags = iwrf.event_flags;
  ips.chan_is_copol[0] = 1;
  ips.chan_is_copol[1] = 0;

}

void IpsTsSim::copyIwrf2Ips(const iwrf_event_notice_t &iwrf,
                              ips_ts_event_notice_t &ips)
{

  ips_ts_event_notice_init(ips);

  copyIwrf2Ips(iwrf.packet, ips.packet);

  ips.start_of_sweep = iwrf.start_of_sweep;
  ips.end_of_sweep = iwrf.end_of_sweep;

  ips.start_of_volume = iwrf.start_of_volume;
  ips.end_of_volume = iwrf.end_of_volume;
  
  ips.scan_mode = iwrf.scan_mode;
  ips.volume_num = iwrf.volume_num;
  ips.sweep_num = iwrf.sweep_num;
  
  ips.current_fixed_angle = iwrf.current_fixed_angle;

}

