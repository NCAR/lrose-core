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
// AparTsSim.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2019
//
///////////////////////////////////////////////////////////////
//
// AparTsSim reads IWRF data from specified files, converts
// the data to APAR TS format, and writes the
// converted files to a specified location
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <cerrno>
#include <cstdio>
#include <unistd.h>

#include <toolsa/str.h>

#include "AparTsSim.hh"
#include "WriteToFile.hh"
#include "WriteToUdp.hh"
#include "ReadFromUdp.hh"

using namespace std;

// Constructor

AparTsSim::AparTsSim(int argc, char **argv)
  
{

  isOK = true;
  
  // set programe name
  
  _progName = "AparTsSim";
  
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

}

// destructor

AparTsSim::~AparTsSim()

{
  
}

//////////////////////////////////////////////////
// Run

int AparTsSim::Run ()
{
  
  PMU_auto_register("Run");
  
  if (_params.run_mode == Params::WRITE_TO_FILE) {
    return _runWriteToFile();
  } else if (_params.run_mode == Params::WRITE_TO_UDP) {
    return _runWriteToUdp();
  } else {
    return _runReadFromUdp();
  }

}

//////////////////////////////////////////////////
// Run in file mode

int AparTsSim::_runWriteToFile()
{
  
  PMU_auto_register("_runWriteToFile");
  
  WriteToFile convertToApar(_progName,
                            _params,
                            _args.inputFileList);
  
  return convertToApar.Run();

}

//////////////////////////////////////////////////
// Run in write to UDP mode

int AparTsSim::_runWriteToUdp()
{
  
  PMU_auto_register("_runWriteToUdp");
 
  WriteToUdp writeToUdp(_progName,
                        _params,
                        _args.inputFileList);

  return writeToUdp.Run();

}

//////////////////////////////////////////////////
// Run in read from UDP mode

int AparTsSim::_runReadFromUdp()
{
  
  PMU_auto_register("_runReadFromUdp");
  
  ReadFromUdp readFromUdp(_progName,
                          _params,
                          _args.inputFileList);
  
  return readFromUdp.Run();
  
}

////////////////////////////////////
// condition angle from 0 to 360

double AparTsSim::conditionAngle360(double angle)
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

double AparTsSim::conditionAngle180(double angle)
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
// Copy members from IWRF structs to APAR structs

void AparTsSim::copyIwrf2Apar(const iwrf_packet_info_t &iwrf,
                              apar_ts_packet_info_t &apar)
{

  apar.len_bytes = iwrf.len_bytes;
  apar.seq_num = iwrf.seq_num;
  apar.version_num = iwrf.version_num;
  apar.radar_id = iwrf.radar_id;
  apar.time_secs_utc = iwrf.time_secs_utc;
  apar.time_nano_secs = iwrf.time_nano_secs;
 
}

void AparTsSim::copyIwrf2Apar(const iwrf_radar_info_t &iwrf,
                              apar_ts_radar_info_t &apar)
{

  apar_ts_radar_info_init(apar);

  copyIwrf2Apar(iwrf.packet, apar.packet);

  apar.latitude_deg = iwrf.latitude_deg;
  apar.longitude_deg = iwrf.longitude_deg;
  apar.altitude_m = iwrf.altitude_m;
  apar.platform_type = iwrf.platform_type;
  apar.beamwidth_deg_h = iwrf.beamwidth_deg_h;
  apar.beamwidth_deg_v = iwrf.beamwidth_deg_v;
  apar.wavelength_cm = iwrf.wavelength_cm;
  apar.nominal_gain_ant_db_h = iwrf.nominal_gain_ant_db_h;
  apar.nominal_gain_ant_db_v = iwrf.nominal_gain_ant_db_v;

  STRncopy(apar.radar_name, "APAR-SIM", IWRF_MAX_RADAR_NAME);
  STRncopy(apar.site_name, "C130", IWRF_MAX_SITE_NAME);

}

void AparTsSim::copyIwrf2Apar(const iwrf_scan_segment_t &iwrf,
                              apar_ts_scan_segment_t &apar)
{

  apar_ts_scan_segment_init(apar);

  copyIwrf2Apar(iwrf.packet, apar.packet);

  apar.scan_mode = iwrf.scan_mode;
  apar.volume_num = iwrf.volume_num;
  apar.sweep_num = iwrf.sweep_num;

  apar.az_start = iwrf.az_start;
  apar.el_start = iwrf.el_start;
  apar.scan_rate = iwrf.scan_rate;
  apar.left_limit = iwrf.left_limit;
  apar.right_limit = iwrf.right_limit;
  apar.up_limit = iwrf.up_limit;
  apar.down_limit = iwrf.down_limit;
  apar.step = iwrf.step;
  
  apar.current_fixed_angle = iwrf.current_fixed_angle;

  apar.n_sweeps = iwrf.n_sweeps;

  memcpy(apar.fixed_angles,
         iwrf.fixed_angles,
         IWRF_MAX_FIXED_ANGLES * sizeof(fl32));
  
  apar.sun_scan_sector_width_az = iwrf.sun_scan_sector_width_az;
  apar.sun_scan_sector_width_el = iwrf.sun_scan_sector_width_el;

  STRncopy(apar.segment_name, iwrf.segment_name, IWRF_MAX_SEGMENT_NAME);
  STRncopy(apar.project_name, iwrf.project_name, IWRF_MAX_PROJECT_NAME);

}

void AparTsSim::copyIwrf2Apar(const iwrf_ts_processing_t &iwrf,
                              apar_ts_processing_t &apar)
{

  apar_ts_processing_init(apar);

  copyIwrf2Apar(iwrf.packet, apar.packet);

  apar.pol_mode = iwrf.pol_mode;
  apar.prf_mode = iwrf.prf_mode;
  apar.pulse_shape = iwrf.pulse_type;
  apar.pulse_width_us = iwrf.pulse_width_us;
  apar.start_range_m = iwrf.start_range_m;
  apar.gate_spacing_m = iwrf.gate_spacing_m;

  apar.test_pulse_range_km = iwrf.test_pulse_range_km;
  apar.test_pulse_length_us = iwrf.test_pulse_length_usec;
  
  apar.num_prts = iwrf.num_prts;
  apar.prt_us[0] = iwrf.prt_usec;
  apar.prt_us[1] = iwrf.prt2_usec;
  apar.prt_us[2] = iwrf.prt3_usec;
  apar.prt_us[3] = iwrf.prt4_usec;
  
}

void AparTsSim::copyIwrf2Apar(const iwrf_calibration_t &iwrf,
                              apar_ts_calibration_t &apar)
{

  apar_ts_calibration_init(apar);

  copyIwrf2Apar(iwrf.packet, apar.packet);

  apar.wavelength_cm = iwrf.wavelength_cm;
  apar.beamwidth_deg_h = iwrf.beamwidth_deg_h;
  apar.beamwidth_deg_v = iwrf.beamwidth_deg_v;
  apar.gain_ant_db_h = iwrf.gain_ant_db_h;
  apar.gain_ant_db_v = iwrf.gain_ant_db_v;
  apar.pulse_width_us = iwrf.pulse_width_us;
  apar.xmit_power_dbm_h = iwrf.xmit_power_dbm_h;
  apar.xmit_power_dbm_v = iwrf.xmit_power_dbm_v;
  apar.two_way_waveguide_loss_db_h = iwrf.two_way_waveguide_loss_db_h;
  apar.two_way_waveguide_loss_db_v = iwrf.two_way_waveguide_loss_db_v;
  apar.two_way_radome_loss_db_h = iwrf.two_way_radome_loss_db_h;
  apar.two_way_radome_loss_db_v = iwrf.two_way_radome_loss_db_v;
  apar.receiver_mismatch_loss_db = iwrf.receiver_mismatch_loss_db;
  apar.radar_constant_h = iwrf.radar_constant_h;
  apar.radar_constant_v = iwrf.radar_constant_v;
  apar.noise_dbm_hc = iwrf.noise_dbm_hc;
  apar.noise_dbm_hx = iwrf.noise_dbm_hx;
  apar.noise_dbm_vc = iwrf.noise_dbm_vc;
  apar.noise_dbm_vx = iwrf.noise_dbm_vx;
  apar.receiver_gain_db_hc = iwrf.receiver_gain_db_hc;
  apar.receiver_gain_db_hx = iwrf.receiver_gain_db_hx;
  apar.receiver_gain_db_vc = iwrf.receiver_gain_db_vc;
  apar.receiver_gain_db_vx = iwrf.receiver_gain_db_vx;
  apar.base_dbz_1km_hc = iwrf.base_dbz_1km_hc;
  apar.base_dbz_1km_hx = iwrf.base_dbz_1km_hx;
  apar.base_dbz_1km_vc = iwrf.base_dbz_1km_vc;
  apar.base_dbz_1km_vx = iwrf.base_dbz_1km_vx;
  apar.sun_power_dbm_hc = iwrf.sun_power_dbm_hc;
  apar.sun_power_dbm_hx = iwrf.sun_power_dbm_hx;
  apar.sun_power_dbm_vc = iwrf.sun_power_dbm_vc;
  apar.sun_power_dbm_vx = iwrf.sun_power_dbm_vx;
  apar.noise_source_power_dbm_h = iwrf.noise_source_power_dbm_h;
  apar.noise_source_power_dbm_v = iwrf.noise_source_power_dbm_v;
  apar.power_meas_loss_db_h = iwrf.power_meas_loss_db_h;
  apar.power_meas_loss_db_v = iwrf.power_meas_loss_db_v;
  apar.coupler_forward_loss_db_h = iwrf.coupler_forward_loss_db_h;
  apar.coupler_forward_loss_db_v = iwrf.coupler_forward_loss_db_v;
  apar.test_power_dbm_h = iwrf.test_power_dbm_h;
  apar.test_power_dbm_v = iwrf.test_power_dbm_v;
  apar.zdr_correction_db = iwrf.zdr_correction_db;
  apar.ldr_correction_db_h = iwrf.ldr_correction_db_h;
  apar.ldr_correction_db_v = iwrf.ldr_correction_db_v;
  apar.phidp_rot_deg = iwrf.phidp_rot_deg;
  apar.receiver_slope_hc = iwrf.receiver_slope_hc;
  apar.receiver_slope_hx = iwrf.receiver_slope_hx;
  apar.receiver_slope_vc = iwrf.receiver_slope_vc;
  apar.receiver_slope_vx = iwrf.receiver_slope_vx;
  apar.i0_dbm_hc = iwrf.i0_dbm_hc;
  apar.i0_dbm_hx = iwrf.i0_dbm_hx;
  apar.i0_dbm_vc = iwrf.i0_dbm_vc;
  apar.i0_dbm_vx = iwrf.i0_dbm_vx;
  apar.dynamic_range_db_hc = iwrf.dynamic_range_db_hc;
  apar.dynamic_range_db_hx = iwrf.dynamic_range_db_hx;
  apar.dynamic_range_db_vc = iwrf.dynamic_range_db_vc;
  apar.dynamic_range_db_vx = iwrf.dynamic_range_db_vx;
  apar.k_squared_water = iwrf.k_squared_water;
  apar.dbz_correction = iwrf.dbz_correction;

  STRncopy(apar.radar_name, "APAR-SIM", IWRF_MAX_RADAR_NAME);
  
}

void AparTsSim::copyIwrf2Apar(const iwrf_pulse_header_t &iwrf,
                              apar_ts_pulse_header_t &apar)
{

  apar_ts_pulse_header_init(apar);

  copyIwrf2Apar(iwrf.packet, apar.packet);

  apar.pulse_seq_num = iwrf.pulse_seq_num;

  apar.scan_mode = iwrf.scan_mode;
  apar.volume_num = iwrf.volume_num;
  apar.sweep_num = iwrf.sweep_num;

  apar.elevation = iwrf.elevation;
  apar.azimuth = iwrf.azimuth;
  
  if (iwrf.scan_mode == IWRF_SCAN_MODE_RHI ||
      iwrf.scan_mode == IWRF_SCAN_MODE_EL_SUR_360) {
    apar.fixed_angle = iwrf.fixed_az;
  } else {
    apar.fixed_angle = iwrf.fixed_el;
  }
  
  apar.prt = iwrf.prt;
  apar.prt_next = iwrf.prt_next;
  
  apar.pulse_width_us = iwrf.pulse_width_us;

  apar.n_gates = iwrf.n_gates;

  apar.n_channels = iwrf.n_channels;
  apar.iq_encoding = iwrf.iq_encoding;
  apar.hv_flag = iwrf.hv_flag;

  apar.phase_cohered = iwrf.phase_cohered;
  
  apar.status = iwrf.status;

  apar.n_data = iwrf.n_data;
  
  apar.scale = iwrf.scale;
  apar.offset = iwrf.offset;

  apar.start_range_m = iwrf.start_range_m;
  apar.gate_spacing_m = iwrf.gate_spacing_m;

  apar.event_flags = iwrf.event_flags;
  apar.chan_is_copol[0] = 1;
  apar.chan_is_copol[1] = 0;

}

void AparTsSim::copyIwrf2Apar(const iwrf_event_notice_t &iwrf,
                              apar_ts_event_notice_t &apar)
{

  apar_ts_event_notice_init(apar);

  copyIwrf2Apar(iwrf.packet, apar.packet);

  apar.start_of_sweep = iwrf.start_of_sweep;
  apar.end_of_sweep = iwrf.end_of_sweep;

  apar.start_of_volume = iwrf.start_of_volume;
  apar.end_of_volume = iwrf.end_of_volume;
  
  apar.scan_mode = iwrf.scan_mode;
  apar.volume_num = iwrf.volume_num;
  apar.sweep_num = iwrf.sweep_num;
  
  apar.current_fixed_angle = iwrf.current_fixed_angle;

}

