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
// TsGamic2Iwrf.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2016
//
///////////////////////////////////////////////////////////////
//
// TsGamic2Iwrf reads pulse IQ data in GAMIC format and
// and writes it to files in IWRF time series format
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <ctime>
#include <toolsa/DateTime.hh>
#include <toolsa/uusleep.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/TaFile.hh>
#include <dsserver/DsLdataInfo.hh>
#include <radar/RadarComplex.hh>
#include "TsGamic2Iwrf.hh"

using namespace std;

// Constructor

TsGamic2Iwrf::TsGamic2Iwrf(int argc, char **argv)
  
{

  // initialize

  isOK = true;
  _reader = NULL;
  _opsType = OPS_UNKNOWN;
  _nGates = 0;
  _nChannels = 0;
  _nIQ = 0;
  _nIQPerChannel = 0;
  _out = NULL;

  _packetSeqNum = 0;
  _pulseSeqNum = 0;
  _pulseTimeSecs = 0;
  _pulseTimeNanoSecs = 0;
  _prevTimeSecs = 0;
  _prevTimeNanoSecs = 0;
  _prevBurstPhase = 0.0;

  // set programe name
  
  _progName = "TsGamic2Iwrf";

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
  
  if (_params.debug) {
    cerr << "Running TsGamic2Iwrf" << endl;
    cerr << "Saving files to dir: " << _params.output_dir << endl;
  }

  // initialize file reader

  if (_params.mode == Params::REALTIME) {
    
    // realtime mode - scan the directory
    
    _reader = new DsInputPath(_progName,
                              _params.debug >= Params::DEBUG_VERBOSE,
                              _params.input_dir,
                              864000,
                              PMU_auto_register,
                              false,
                              true);
    
  } else {
    
    // ARCHIVE mode

    if (_args.inputFileList.size() < 1) {
      cerr << "ERROR - " << _progName << endl;
      cerr << "  ARCHIVE mode" << endl;
      cerr << "  No files specified" << endl;
      cerr << "  You must specify a file list on the command line" << endl;
      isOK = false;
      return;
    }
    
    _reader = new DsInputPath(_progName,
                              _params.debug >= Params::DEBUG_VERBOSE,
                              _args.inputFileList);
    
  }
  
  return;
  
}

// destructor

TsGamic2Iwrf::~TsGamic2Iwrf()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int TsGamic2Iwrf::Run ()
{

  PMU_auto_register("Run");
  int iret = 0;
  
  // loop until end of data
  
  char *path;
  while ((path = _reader->next()) != NULL) {
    if (_processFile(path)) {
      cerr << "WARNING - TsGamic2Iwrf::Run" << endl;
      cerr << "  Errors in processing file: " << path << endl;
      iret = -1;
    }
  } // while ((filePath ...
  
  return iret;
  
}

//////////////////////////////////////////////////
// process a file
//
// Returns 0 on success, -1 on failure

int TsGamic2Iwrf::_processFile(const char* filePath)

{
  
  if (_params.debug) {
    cerr << "Processing file: " << filePath << endl;
  }
  
  // open input gamic file
  
  TaFile _in;
  FILE *in;
  if ((in = _in.fopenUncompress(filePath, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - TsGamic2Iwrf::_processFile";
    cerr << "  Cannot open file for reading: " << filePath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  _filePulseCount = 0;

  // loop through pulse records

  while (!feof(in)) {

    // read in header
    
    if (fread(&_gamicHdr, sizeof(_gamicHdr), 1, in) != 1) {
      if (feof(in)) {
        break;
      }
      int errNum = errno;
      cerr << "ERROR - TsGamic2Iwrf::_processFile" << endl;
      cerr << "  Reading file: " << filePath << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Read in header:" << endl;
      _print(_gamicHdr, cerr);
    }

    if (_gamicHdr.numDualIQ > 0) {
      _nGates = _gamicHdr.numDualIQ;
      _opsType = DUAL_POL;
      _nChannels = 2;
    } else if (_gamicHdr.numHorIQ > 0) {
      _nGates = _gamicHdr.numHorIQ;
      _opsType = HORIZONTAL_ONLY;
      _nChannels = 1;
    } else if (_gamicHdr.numVerIQ > 0) {
      _nGates = _gamicHdr.numVerIQ;
      _opsType = VERTICAL_ONLY;
      _nChannels = 1;
    }

    _nIQPerChannel = 2 * _nGates;
    _nIQ = _nIQPerChannel * _nChannels;

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      switch (_opsType) {
        case DUAL_POL:
          cerr << "  Ops type: DUAL_POL" << endl;
          break;
        case HORIZONTAL_ONLY:
          cerr << "  Ops type: HORIZONTAL_ONLY" << endl;
          break;
        case VERTICAL_ONLY:
          cerr << "  Ops type: VERTICAL_ONLY" << endl;
          break;
        default:
          cerr << "  Ops type: UNKNOWN" << endl;
      }
      cerr << "  nGates: " << _nGates << endl;
      cerr << "  nChannels: " << _nChannels << endl;
      cerr << "  nIQ: " << _nIQ << endl;
    }
    
    // read in IQ data

    _iq = _iq_.alloc(_nIQ);
    if ((int) fread(_iq, sizeof(fl32), _nIQ, in) != _nIQ) {
      int errNum = errno;
      cerr << "ERROR - TsGamic2Iwrf::_profeddFile";
      cerr << "  Reading file: " << filePath << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }

    // align to next 4096-byte boundary, for next read

    long pos = ftell(in);
    int bytesToRead = 0;
    if (pos % 4096 != 0) {
      bytesToRead = 4096 - (pos % 4096);
    }
    fseek(in, bytesToRead, SEEK_CUR);

    // for multiple channels, reorganize the IQ data
    // in IWRF, the complete channels follow each other
    // in GAMIC, each gate has all channels

    if (_nChannels > 1) {

      TaArray<fl32> tmp_;
      fl32 *tmp = tmp_.alloc(_nIQ);
      memcpy(tmp, _iq, _nIQ * sizeof(fl32));

      for (int igate = 0; igate < _nGates; igate++) {
        for (int ichan = 0; ichan < _nChannels; ichan++) {
          fl32 *source = tmp + (igate * _nChannels * 2) + ichan * 2;
          fl32 *target = _iq + (ichan * _nGates * 2) + igate * 2;
          memcpy(target, source, 2 * sizeof(fl32));
        } // ichan
      } // igate
      
    } // if (_nChannels > 1) 
    
    // scale the IQ data for nominal digital receiver gain

    double scaleFactorH = pow(10.0, _params.rx_gain_nominal_db_h / 20.0);
    double scaleFactorV = pow(10.0, _params.rx_gain_nominal_db_v / 20.0);
    
    for (int ichan = 0; ichan < _nChannels; ichan++) {
      double scaleFactor = scaleFactorH;
      if (ichan == 1) {
        scaleFactor = scaleFactorV;
      }
      fl32 *val =  _iq + ichan * _nIQPerChannel;
      for (int ii = 0; ii < _nIQPerChannel; ii++, val++) {
        *val /= scaleFactor;
      }
    } // ichan
    
    // handle this pulse

    _handleInputPulse();

  } // while

  // close the files

  _in.fclose();
  _closeOutputFile();

  return 0;

}

/////////////////////////////
// handle a pulse

int TsGamic2Iwrf::_handleInputPulse()

{

  // set time

  _prevTimeSecs = _pulseTimeSecs;
  _prevTimeNanoSecs = _pulseTimeNanoSecs;

  _pulseTimeSecs = _gamicHdr.timestampS;
  _pulseTimeNanoSecs = _gamicHdr.timestampUS * 1000;

  // load up info

  _loadIwrfInfo();

  // create iwrf pulse
  
  IwrfTsPulse pulse(_info);

  // load iwrf pulse from GAMIC data

  _loadIwrfPulse(pulse);

  // open file if needed
  
  _filePulseCount++;
  if (_filePulseCount == 1) {
    _openOutputFile(pulse);
  }

  // write ops info to file every 1000 pulses

  if (_filePulseCount % 1000 == 1) {
    _info.writeMetaToFile(_out, 0);
  }
  
  // reformat pulse as needed

  if (_params.output_format == Params::FORMAT_TSARCHIVE) {
    pulse.convertToPacked(IWRF_IQ_ENCODING_FL32);
  } else if (_params.output_packing == Params::PACKING_FL32) {
    pulse.convertToPacked(IWRF_IQ_ENCODING_FL32);
  } else if (_params.output_packing == Params::PACKING_SCALED_SI16) {
    pulse.convertToPacked(IWRF_IQ_ENCODING_SCALED_SI16);
  } else if (_params.output_packing == Params::PACKING_DBM_PHASE_SI16) {
    pulse.convertToPacked(IWRF_IQ_ENCODING_DBM_PHASE_SI16);
  } else if (_params.output_packing == Params::PACKING_SIGMET_FL16) {
    pulse.convertToPacked(IWRF_IQ_ENCODING_SIGMET_FL16);
  }

  // write pulse to file
  
  if (_params.output_format == Params::FORMAT_TSARCHIVE) {
    pulse.writeToTsarchiveFile(_out);
  } else {
    pulse.writeToFile(_out);
  }

  return 0;

}

////////////////////////////////////////
// load IWRF info from Gamic pulse data

void TsGamic2Iwrf::_loadIwrfInfo()

{

  // time

  _info.setTime(_pulseTimeSecs, _pulseTimeNanoSecs);

  // set radar info

  iwrf_radar_info_t radarInfo;
  iwrf_radar_info_init(radarInfo);
  iwrf_set_packet_time(radarInfo.packet, _pulseTimeSecs, _pulseTimeNanoSecs);
  _packetSeqNum++;
  iwrf_set_packet_seq_num(radarInfo.packet, _packetSeqNum);

  radarInfo.latitude_deg = _params.radar_latitude;
  radarInfo.longitude_deg = _params.radar_longitude;
  radarInfo.altitude_m = _params.radar_altitude_m;
  radarInfo.platform_type = IWRF_RADAR_PLATFORM_FIXED;

  radarInfo.beamwidth_deg_h = _gamicHdr.horizontalBeamwidth;
  radarInfo.beamwidth_deg_v = _gamicHdr.verticalBeamwidth;
  radarInfo.wavelength_cm = _gamicHdr.radarWavelength * 100.0;
  radarInfo.nominal_gain_ant_db_h = _params.antenna_gain_db;
  radarInfo.nominal_gain_ant_db_v = _params.antenna_gain_db;

  STRncopy(radarInfo.radar_name, _params.radar_name, IWRF_MAX_RADAR_NAME);
  STRncopy(radarInfo.site_name, _params.site_name, IWRF_MAX_SITE_NAME);
  
  _info.setRadarInfo(radarInfo);
  _info.setRadarInfoActive(true);

  // set processing

  iwrf_ts_processing_t tsProc;
  iwrf_ts_processing_init(tsProc);
  iwrf_set_packet_time(tsProc.packet, _pulseTimeSecs, _pulseTimeNanoSecs);
  _packetSeqNum++;
  iwrf_set_packet_seq_num(tsProc.packet, _packetSeqNum);

  switch (_params.xmit_rcv_mode) {

    case Params::DP_ALT_HV_CO_ONLY:
      tsProc.xmit_rcv_mode = IWRF_ALT_HV_CO_ONLY;
      tsProc.pol_mode = IWRF_POL_MODE_HV_ALT;
      break;
    case Params::DP_ALT_HV_CO_CROSS:
      tsProc.xmit_rcv_mode = IWRF_ALT_HV_CO_CROSS;
      tsProc.pol_mode = IWRF_POL_MODE_HV_ALT;
      break;
    case Params::DP_ALT_HV_FIXED_HV:
      tsProc.xmit_rcv_mode = IWRF_ALT_HV_FIXED_HV;
      tsProc.pol_mode = IWRF_POL_MODE_HV_ALT;
      break;
    case Params::DP_SIM_HV_FIXED_HV:
      tsProc.xmit_rcv_mode = IWRF_SIM_HV_FIXED_HV;
      tsProc.pol_mode = IWRF_POL_MODE_HV_SIM;
      break;
    case Params::DP_SIM_HV_SWITCHED_HV:
      tsProc.xmit_rcv_mode = IWRF_SIM_HV_SWITCHED_HV;
      tsProc.pol_mode = IWRF_POL_MODE_HV_SIM;
      break;
    case Params::DP_H_ONLY_FIXED_HV:
      tsProc.xmit_rcv_mode = IWRF_H_ONLY_FIXED_HV;
      tsProc.pol_mode = IWRF_POL_MODE_H;
      break;
    case Params::DP_V_ONLY_FIXED_HV:
      tsProc.xmit_rcv_mode = IWRF_V_ONLY_FIXED_HV;
      tsProc.pol_mode = IWRF_POL_MODE_V;
      break;
    case Params::DP_ALT_HHVV_FIXED_HV:
      tsProc.xmit_rcv_mode = IWRF_ALT_HHVV_FIXED_HV;
      tsProc.pol_mode = IWRF_POL_MODE_HV_ALT;
      break;
    case Params::SINGLE_POL:
      tsProc.xmit_rcv_mode = IWRF_SINGLE_POL;
      if (_opsType == HORIZONTAL_ONLY) {
        tsProc.pol_mode = IWRF_POL_MODE_H;
      } else if (_opsType == VERTICAL_ONLY) {
        tsProc.pol_mode = IWRF_POL_MODE_V;
      }
      break;
    case Params::XMIT_RCV_UNKNOWN:
    default:
      tsProc.xmit_rcv_mode = IWRF_XMIT_RCV_MODE_NOT_SET;
      tsProc.pol_mode = IWRF_POL_MODE_NOT_SET;

  }


  if (_gamicHdr.highPrf == _gamicHdr.lowPrf ||
      _gamicHdr.lowPrf == 0) {
    tsProc.xmit_phase_mode = IWRF_XMIT_PHASE_MODE_FIXED;
    tsProc.num_prts = 1;
  } else {
    double prt1 = 1.0 / _gamicHdr.highPrf;
    double prt2 = 1.0 / _gamicHdr.lowPrf;
    double prtRatio = prt1 / prt2;
    if (prtRatio < 1) {
      prtRatio = prt2 / prt1;
    }
    if (prtRatio < 1.3) {
      tsProc.xmit_phase_mode = IWRF_PRF_MODE_STAGGERED_4_5;
    } else if (prtRatio < 1.4) {
      tsProc.xmit_phase_mode = IWRF_PRF_MODE_STAGGERED_3_4;
    } else {
      tsProc.xmit_phase_mode = IWRF_PRF_MODE_STAGGERED_2_3;
    }
    tsProc.num_prts = 2;
  }

  tsProc.pulse_type = IWRF_PULSE_TYPE_RECT;

  tsProc.prt_usec = (1.0 / _gamicHdr.highPrf) * 1.0e6;
  tsProc.prt2_usec = (1.0 / _gamicHdr.lowPrf) * 1.0e6;

  tsProc.cal_type = IWRF_CAL_TYPE_CW_CAL;
  
  tsProc.burst_range_offset_m = 0.0;
  tsProc.pulse_width_us = _getPulseWidth(_gamicHdr.pwIndex);
  
  tsProc.start_range_m = _gamicHdr.rangeResolutionIQ / 2.0;
  tsProc.gate_spacing_m = _gamicHdr.rangeResolutionIQ;
  
  tsProc.integration_cycle_pulses = 0;
  tsProc.clutter_filter_number = 0;
  tsProc.range_gate_averaging = 0;
  tsProc.max_gate = _nGates;

  tsProc.test_power_dbm = 0.0;
  tsProc.test_pulse_range_km = 0.0;
  tsProc.test_pulse_length_usec = 0.0;

  tsProc.xmit_flag[0] = 0;
  tsProc.xmit_flag[1] = 0;

  tsProc.beams_are_indexed = 0;
  tsProc.specify_dwell_width = 0;
  tsProc.indexed_beam_width_deg = 0;
  tsProc.indexed_beam_spacing_deg = 0;

  tsProc.prt3_usec = 0;
  tsProc.prt4_usec = 0;
  
  tsProc.block_mode_prt2_pulses = 0;
  tsProc.block_mode_prt3_pulses = 0;
  tsProc.block_mode_prt4_pulses = 0;
  
  _info.setTsProcessing(tsProc);
  _info.setTsProcessingActive(true);

  // set cal info

  iwrf_calibration_t cal;
  iwrf_calibration_init(cal);
  iwrf_set_packet_time(cal.packet, _pulseTimeSecs, _pulseTimeNanoSecs);
  _packetSeqNum++;
  iwrf_set_packet_seq_num(cal.packet, _packetSeqNum);

  cal.wavelength_cm = _gamicHdr.radarWavelength * 100.0;
  cal.beamwidth_deg_h = _gamicHdr.horizontalBeamwidth;
  cal.beamwidth_deg_v = _gamicHdr.verticalBeamwidth;
  cal.wavelength_cm = _gamicHdr.radarWavelength * 100.0;
  cal.gain_ant_db_h = _params.antenna_gain_db;
  cal.gain_ant_db_v = _params.antenna_gain_db;

  cal.pulse_width_us = _getPulseWidth(_gamicHdr.pwIndex);

  cal.xmit_power_dbm_h = _params.xmit_power_dbm_h;
  cal.xmit_power_dbm_v = _params.xmit_power_dbm_v;
  
  // cal.noise_source_power_dbm_h = _gamicHdr.noisePowerH;
  // cal.noise_source_power_dbm_v = _gamicHdr.noisePowerV;

  cal.base_dbz_1km_hc = _gamicHdr.dbz0H;
  cal.base_dbz_1km_vc = _gamicHdr.dbz0V;

  cal.zdr_correction_db = _gamicHdr.zdrOffset;
  cal.ldr_correction_db_h = _gamicHdr.ldrOffset;
  cal.ldr_correction_db_v = _gamicHdr.ldrOffset;
  cal.phidp_rot_deg = _gamicHdr.phidpOffset;

  STRncopy(cal.radar_name, _params.radar_name, IWRF_MAX_RADAR_NAME);

  _info.setCalibration(cal);
  _info.setCalibrationActive(true);

}


////////////////////////////////////////
// load IWRF pulse from Gamic pulse data

void TsGamic2Iwrf::_loadIwrfPulse(IwrfTsPulse &pulse)

{

  // set header
  
  _pulseSeqNum = _gamicHdr.pulseCounter;
  _packetSeqNum++;
  pulse.setPktSeqNum(_packetSeqNum);
  pulse.set_pulse_seq_num(_pulseSeqNum);

  pulse.setTime(_pulseTimeSecs, _pulseTimeNanoSecs);

  pulse.set_scan_mode(_params.scan_mode);
  pulse.set_follow_mode(IWRF_FOLLOW_MODE_NONE);

  pulse.set_sweep_num(-1);
  pulse.set_volume_num(-1);

  double azimuthDeg = double(_gamicHdr.aziTag) * 359.9945068359375 / 65535.0;
  double elevationDeg = double(_gamicHdr.eleTag) * 179.9945068359375 / 32767.0;
  
  pulse.set_fixed_el(elevationDeg);
  pulse.set_fixed_az(azimuthDeg);
  pulse.set_elevation(elevationDeg);
  pulse.set_azimuth(azimuthDeg);

  double prt =
    (double) (_pulseTimeSecs - _prevTimeSecs) +
    (double) (_pulseTimeNanoSecs - _prevTimeNanoSecs) / 1.0e9;

  pulse.set_prt(prt);
  pulse.set_prt_next(IWRF_MISSING_FLOAT);
  pulse.set_pulse_width_us(_getPulseWidth(_gamicHdr.pwIndex));
  pulse.set_n_gates(_nGates);
  pulse.set_n_channels(_nChannels);
  pulse.set_iq_encoding(IWRF_IQ_ENCODING_FL32);
  pulse.set_hv_flag(0);
  pulse.set_antenna_transition(0);
  pulse.set_phase_cohered(1);
  pulse.set_status(0);
  pulse.set_n_data(_nGates * _nChannels * 2);

  double phaseDiff = RadarComplex::diffDeg(_gamicHdr.burstPhase, _prevBurstPhase);
  for (int ichan = 0; ichan < _nChannels; ichan++) {
    pulse.set_iq_offset(ichan, 0);
    pulse.set_burst_mag(ichan, _gamicHdr.burstPower);
    pulse.set_burst_arg(ichan, _gamicHdr.burstPhase);
    pulse.set_burst_arg_diff(ichan, phaseDiff);
  }
  _prevBurstPhase = _gamicHdr.burstPhase;
  
  pulse.set_scale(1.0);
  pulse.set_offset(0.0);
  pulse.set_n_gates_burst(0);
  pulse.set_start_range_m(_gamicHdr.rangeResolutionIQ / 2.0);
  pulse.set_gate_spacing_m(_gamicHdr.rangeResolutionIQ);

  // set IQ data

  pulse.setIqFloats(_nGates, _nChannels, _iq);

}


///////////////////////////////////////////////////
// get the pulse width from the params lookup table

double TsGamic2Iwrf::_getPulseWidth(int index)
  
{

  if (index > _params.pulse_width_lookup_n - 1) {
    cerr << "WARNING - pulse witdth index not supported: " << index << endl;
    cerr << "  Check parameter pulse_width_lookup" << endl;
    cerr << "  Using pulse width for index = 0" << endl;
    return _params._pulse_width_lookup[0].pulse_width_us;
  }

  return _params._pulse_width_lookup[index].pulse_width_us;

}

/////////////////////////////////
// open a new output file

int TsGamic2Iwrf::_openOutputFile(const IwrfTsPulse &pulse)

{

  // close out old file

  _closeOutputFile();

  // get time
  
  time_t ptime = pulse.getTime();
  int nanoSecs = pulse.getNanoSecs();
  int milliSecs = nanoSecs / 1000000;
  if (milliSecs > 999) {
    milliSecs = 999;
  }

  DateTime ttime(ptime);
  _outputTime = ttime.utime();

  // compute antenna pos strings
  
  iwrf_scan_mode_t scanMode = pulse.getScanMode();

  char fixedAngleStr[64];
  char movingAngleStr[64];
  
  if (scanMode == IWRF_SCAN_MODE_RHI ||
      scanMode == IWRF_SCAN_MODE_MANRHI) {
    double el = pulse.getEl();
    if (el < 0) {
      el += 360;
    }
    double az = pulse.getAz();
    sprintf(fixedAngleStr, "_%.3d", (int) (az * 10.0 + 0.5));
    sprintf(movingAngleStr, "_%.3d", (int) (el + 0.5));
  } else {
    double az = pulse.getAz();
    if (az < 0) {
      az += 360;
    }
    double el = pulse.getEl();
    sprintf(fixedAngleStr, "_%.3d", (int) (el * 10.0 + 0.5));
    sprintf(movingAngleStr, "_%.3d", (int) (az + 0.5));
  }

  // compute scan mode string
  
  string scanModeStr;
  // if (_params.add_scan_mode_to_file_name) {
    switch (scanMode) {
    case IWRF_SCAN_MODE_SECTOR:
      scanModeStr = ".sec";
      break;
    case IWRF_SCAN_MODE_COPLANE:
      scanModeStr = ".coplane";
      break;
    case IWRF_SCAN_MODE_RHI:
      scanModeStr = ".rhi";
      break;
    case IWRF_SCAN_MODE_VERTICAL_POINTING:
      scanModeStr = ".vert";
      break;
    case IWRF_SCAN_MODE_IDLE:
      scanModeStr = ".idle";
      break;
    case IWRF_SCAN_MODE_AZ_SUR_360:
      scanModeStr = ".sur";
      break;
    case IWRF_SCAN_MODE_EL_SUR_360:
      scanModeStr = ".elsur";
      break;
    case IWRF_SCAN_MODE_SUNSCAN:
      scanModeStr = ".sun";
      break;
    case IWRF_SCAN_MODE_POINTING:
      scanModeStr = ".point";
      break;
    case IWRF_SCAN_MODE_MANPPI:
      scanModeStr = ".manppi";
      break;
    case IWRF_SCAN_MODE_MANRHI:
      scanModeStr = ".manrhi";
      break;
    default: {}
    }
    // }
  
  // make the output dir

  char subdir[1024];
  _outputDir = _params.output_dir;
  
  if (_params.debug) {
    cerr << "Using outputDir: " << _outputDir << endl;
  }
  
  sprintf(subdir, "%s/%.4d%.2d%.2d", _outputDir.c_str(),
          ttime.getYear(), ttime.getMonth(), ttime.getDay());
  
  if (ta_makedir_recurse(subdir)) {
    int errNum = errno;
    cerr << "ERROR - TsGamic2Iwrf" << endl;
    cerr << "  Cannot make output directory: " << subdir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // compute output path

  string format;
  if (_params.output_format == Params::FORMAT_TSARCHIVE) {
    format = ".tsarchive";
  } else {
    format = ".iwrf_ts";
  }

  string packing;
  if (_params.output_format == Params::FORMAT_TSARCHIVE) {
    packing = "";
  } else if (_params.output_packing == Params::PACKING_FL32) {
    packing = ".fl32";
  } else if (_params.output_packing == Params::PACKING_SCALED_SI16) {
    packing = ".scaled";
  } else if (_params.output_packing == Params::PACKING_DBM_PHASE_SI16) {
    packing = ".dbm_phase";
  } else if (_params.output_packing == Params::PACKING_SIGMET_FL16) {
    packing = ".sigmet";
  }

  char name[1024];
  sprintf(name, "%.4d%.2d%.2d_%.2d%.2d%.2d.%.3d%s%s%s%s%s",
          ttime.getYear(), ttime.getMonth(), ttime.getDay(),
          ttime.getHour(), ttime.getMin(), ttime.getSec(), milliSecs,
	  fixedAngleStr, movingAngleStr, scanModeStr.c_str(),
          packing.c_str(), format.c_str());
  _outputName = name;

  char relPath[1024];
  sprintf(relPath, "%.4d%.2d%.2d/%s",
          ttime.getYear(), ttime.getMonth(), ttime.getDay(), name);
  _relPath = relPath;

  char path[1024];
  sprintf(path, "%s/%s", _outputDir.c_str(), relPath);
  
  // open file

  if ((_out = fopen(path, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - TsGamic2Iwrf" << endl;
    cerr << "  Cannot open output file: " << path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  _outputPath = path;

  if (_params.debug) {
    cerr << "Opened new file: " << _outputPath << endl;
  }

  return 0;

}

///////////////////////////////////////////
// close output file
//
// Returns 0 if file already open, -1 if not

int TsGamic2Iwrf::_closeOutputFile()

{

  // close out old file
  
  if (_out != NULL) {

    fclose(_out);
    _out = NULL;

    if (_params.debug) {
      cerr << "Done with file: " << _outputPath << endl;
    }

    // write latest data info file
    
    DsLdataInfo ldata(_outputDir,
		      _params.debug >= Params::DEBUG_VERBOSE);
    ldata.setLatestTime(_outputTime);
    ldata.setRelDataPath(_relPath);
    ldata.setWriter("TsGamic2Iwrf");
    ldata.setDataType("iwrf_ts");
    if (ldata.write(_outputTime)) {
      cerr << "ERROR - cannot write LdataInfo" << endl;
      cerr << " outputDir: " << _outputDir << endl;
    }
   
    return 0;

  }

  return -1;

}


///////////////////////////////////////////
// print gamic header

void TsGamic2Iwrf::_print(const gamic_header_t &hdr,
                          ostream &out)

{

  out << "GAMIC HEADER BLOCK" << endl;

  out << "  command: " << hdr.command << endl;
  out << "  length: " << hdr.length << endl;
  out << "  acqModeA: " << hdr.acqModeA << endl;
  out << "  acqModeB: " << hdr.acqModeB << endl;

  out << "  numBurst: " << hdr.numBurst << endl;
  out << "  offsetBurst: " << hdr.offsetBurst << endl;
  out << "  numHorIQ: " << hdr.numHorIQ << endl;
  out << "  offsetHorIQ: " << hdr.offsetHorIQ << endl;
  out << "  numVerIQ: " << hdr.numVerIQ << endl;
  out << "  offsetVerIQ: " << hdr.offsetVerIQ << endl;
  out << "  numDualIQ: " << hdr.numDualIQ << endl;
  out << "  offsetDualIQ: " << hdr.offsetDualIQ << endl;

  out << "  aziTag: " << hdr.aziTag << endl;
  out << "  eleTag: " << hdr.eleTag << endl;

  double azimuthDeg = double(hdr.aziTag) * 359.9945068359375 / 65535.0;
  double elevationDeg = double(hdr.eleTag) * 179.9945068359375 / 32767.0;

  out << "  azimuthDeg: " << azimuthDeg << endl;
  out << "  elevationDeg: " << elevationDeg << endl;

  out << "  aziSpeed: " << hdr.aziSpeed << endl;
  out << "  eleSpeed: " << hdr.eleSpeed << endl;

  out << "  angleSyncFlag: " << hdr.angleSyncFlag << endl;
  out << "  angleSyncPulseCount: " << hdr.angleSyncPulseCount << endl;
  out << "  pulseCounter: " << hdr.pulseCounter << endl;

  out << "  highPrf: " << hdr.highPrf << endl;
  out << "  lowPrf: " << hdr.lowPrf << endl;
  out << "  prf3: " << hdr.prf3 << endl;
  out << "  prf4: " << hdr.prf4 << endl;
  out << "  prfIndicator: " << hdr.prfIndicator << endl;

  out << "  rangeResolutionIQ: " << hdr.rangeResolutionIQ << endl;
  out << "  numberOfValidRangeBins: " << hdr.numberOfValidRangeBins << endl;

  out << "  timestampS: " << hdr.timestampS << endl;
  out << "  timestampUS: " << hdr.timestampUS << endl;

  out << "  meanBurstPower: " << hdr.meanBurstPower << endl;
  out << "  burstPower: " << hdr.burstPower << endl;
  out << "  meanBurstFreq: " << hdr.meanBurstFreq << endl;
  out << "  burstPhase: " << hdr.burstPhase << endl;
  out << "  lastBurstPhase: " << hdr.lastBurstPhase << endl;

  out << "  IFSamplingFreq: " << hdr.IFSamplingFreq << endl;

  out << "  converterPhaseDiffHorizontal: " << hdr.converterPhaseDiffHorizontal << endl;
  out << "  converterPhaseDiffVertical: " << hdr.converterPhaseDiffVertical << endl;
  out << "  converterPowerDiffHorizontal: " << hdr.converterPowerDiffHorizontal << endl;
  out << "  converterPowerDiffVertical: " << hdr.converterPowerDiffVertical << endl;

  out << "  afcOffset: " << hdr.afcOffset << endl;
  out << "  afcMode: " << hdr.afcMode << endl;
  out << "  errorFlags: " << hdr.errorFlags << endl;

  for (int ii = 0; ii < 8; ii++) {
    out << "  adcTemperature[" << ii << "]: " << (int) hdr.adcTemperature[ii] << endl;
  }

  out << "  ifdPowerFlags: " << hdr.ifdPowerFlags << endl;

  out << "  headerID: " << hdr.headerID << endl;

  out << "  adcHLMaxAmplitude: " << hdr.adcHLMaxAmplitude << endl;
  out << "  adcHHMaxAmplitude: " << hdr.adcHHMaxAmplitude << endl;
  out << "  adcVLMaxAmplitude: " << hdr.adcVLMaxAmplitude << endl;
  out << "  adcVHMaxAmplitude: " << hdr.adcVHMaxAmplitude << endl;
  out << "  adcOverflowFlags: " << hdr.adcOverflowFlags << endl;

  out << "  pciDMATransferRate: " << hdr.pciDMATransferRate << endl;
  out << "  pciDMAFIFOFillDegree: " << hdr.pciDMAFIFOFillDegree << endl;

  out << "  errorConditionCounter: " << hdr.errorConditionCounter << endl;

  out << "  dspFirmwareVersion: " << hdr.dspFirmwareVersion << endl;
  out << "  fpgaPciFirmwareVersion: " << hdr.fpgaPciFirmwareVersion << endl;
  out << "  fpgaIfdFirmwareVersion: " << hdr.fpgaIfdFirmwareVersion << endl;

  out << "  checksum: " << hdr.checksum << endl;

  out << "  radarWavelength: " << hdr.radarWavelength << endl;
  out << "  horizontalBeamwidth: " << hdr.horizontalBeamwidth << endl;
  out << "  verticalBeamwidth: " << hdr.verticalBeamwidth << endl;

  out << "  pwIndex: " << hdr.pwIndex << endl;
  out << "  pulseWidthUs: " << _getPulseWidth(hdr.pwIndex) << endl;

  out << "  noisePowerH: " << hdr.noisePowerH << endl;
  out << "  noisePowerV: " << hdr.noisePowerV << endl;
  out << "  dbz0H: " << hdr.dbz0H << endl;
  out << "  dbz0V: " << hdr.dbz0V << endl;
  out << "  dbz0C: " << hdr.dbz0C << endl;
  out << "  zdrOffset: " << hdr.zdrOffset << endl;
  out << "  ldrOffset: " << hdr.ldrOffset << endl;
  out << "  phidpOffset: " << hdr.phidpOffset << endl;

  out << "  sweepUID: " << hdr.sweepUID << endl;

  out << "==================" << endl;

}

