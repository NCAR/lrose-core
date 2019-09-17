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

#include <dataport/bigend.h>
#include <dataport/swap.h>
#include <toolsa/DateTime.hh>
#include <toolsa/Path.hh>
#include <toolsa/file_io.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/str.h>
#include <toolsa/MemBuf.hh>
#include <radar/apar_ts_functions.hh>
#include <radar/AparTsPulse.hh>
#include "AparTsSim.hh"

#include <sys/socket.h>
#include <netinet/in.h>
#ifdef __linux
#include <arpa/inet.h>
#endif

using namespace std;

// Constructor

AparTsSim::AparTsSim(int argc, char **argv)
  
{

  isOK = true;
  _aparTsInfo = NULL;
  _sampleSeqNum = 0;
  _pulseSeqNum = 0;
  _dwellSeqNum = 0;
  _udpFd = -1;
  _errCount = 0;

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
  
  _aparTsDebug = APAR_TS_DEBUG_OFF;
  if (_params.debug >= Params::DEBUG_EXTRA) {
    _aparTsDebug = APAR_TS_DEBUG_VERBOSE;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    _aparTsDebug = APAR_TS_DEBUG_NORM;
  }
  _aparTsInfo = new AparTsInfo(_aparTsDebug);

}

// destructor

AparTsSim::~AparTsSim()

{
  
  // delete pulses to free memory
  
  for (size_t ii = 0; ii < _dwellPulses.size(); ii++) {
    delete _dwellPulses[ii];
  }
  _dwellPulses.clear();

}

//////////////////////////////////////////////////
// Run

int AparTsSim::Run ()
{
  
  PMU_auto_register("Run");
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Running AparTsSim - extra verbose debug mode" << endl;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Running AparTsSim - verbose debug mode" << endl;
  } else if (_params.debug) {
    cerr << "Running AparTsSim - debug mode" << endl;
  }

  if (_params.output_mode == Params::OUTPUT_FILES) {
    return _runFileMode();
  } else {
    return _runUdpMode();
  }

}

//////////////////////////////////////////////////
// Run in file mode

int AparTsSim::_runFileMode()
{
  
  PMU_auto_register("_runFileMode");
  
  // loop through the input files

  int iret = 0;
  for (size_t ii = 0; ii < _args.inputFileList.size(); ii++) {
    if (_convertFile(_args.inputFileList[ii])) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in UDP mode

int AparTsSim::_runUdpMode()
{
  
  PMU_auto_register("_runUdpMode");
 
  // this is a simulation mode
  // loop through the input files, and repeat

  int iret = 0;
  while (true) {
    for (size_t ii = 0; ii < _args.inputFileList.size(); ii++) {
      if (_convert2Udp(_args.inputFileList[ii])) {
        iret = -1;
      }
    }
  }
  
  return iret;

}

////////////////////////////////////////////////////
// Convert 1 file to APAR format

int AparTsSim::_convertFile(const string &inputPath)
  
{

  if (_params.debug) {
    cerr << "Reading input file: " << inputPath << endl;
  }

  // set up a vector with a single file entry

  vector<string> fileList;
  fileList.push_back(inputPath);

  // create reader for just that one file

  IwrfDebug_t iwrfDebug = IWRF_DEBUG_OFF;
  if (_params.debug >= Params::DEBUG_EXTRA) {
    iwrfDebug = IWRF_DEBUG_VERBOSE;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrfDebug = IWRF_DEBUG_NORM;
  } 
  IwrfTsReaderFile reader(fileList, iwrfDebug);
  const IwrfTsInfo &tsInfo = reader.getOpsInfo();

  // read through pulses until we have current metadata

  {
    IwrfTsPulse *iwrfPulse = reader.getNextPulse();
    bool haveMetadata = false;
    while (iwrfPulse != NULL) {
      if (tsInfo.isRadarInfoActive() &&
          tsInfo.isScanSegmentActive() &&
          tsInfo.isTsProcessingActive()) {
        // we have the necessary metadata
        haveMetadata = true;
        delete iwrfPulse;
        break;
      }
      delete iwrfPulse;
    }
    if (!haveMetadata) {
      cerr << "ERROR - AparTsSim::_convertFile()" << endl;
      cerr << "Metadata missing for file: " << inputPath << endl;
      return -1;
    }
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_radar_info_print(stderr, tsInfo.getRadarInfo());
    iwrf_scan_segment_print(stderr, tsInfo.getScanSegment());
    iwrf_ts_processing_print(stderr, tsInfo.getTsProcessing());
    if (tsInfo.isCalibrationActive()) {
      iwrf_calibration_print(stderr, tsInfo.getCalibration());
    }
  }

  // convert the metadata to APAR types
  // set the metadata in the info metadata queue

  _convertMeta2Apar(tsInfo);
  _aparTsInfo->setRadarInfo(_aparRadarInfo);
  _aparTsInfo->setScanSegment(_aparScanSegment);
  _aparTsInfo->setTsProcessing(_aparTsProcessing);
  if (tsInfo.isCalibrationActive()) {
    _aparTsInfo->setCalibration(_aparCalibration);
  }
  
  // reset reader queue to start

  reader.reset();

  // compute number of pulses per dwell

  size_t nPulsesPerDwell = 
    _params.n_samples_per_visit *
    _params.n_visits_per_beam *
    _params.n_beams_per_dwell;

  if (_params.debug) {
    cerr << "  ==>> nPulsesPerDwell: " << nPulsesPerDwell << endl;
  }

  // read in all pulses

  IwrfTsPulse *iwrfPulse = reader.getNextPulse();
  while (iwrfPulse != NULL) {

    // convert to floats

    iwrfPulse->convertToFL32();

    // open output file as needed
    
    if (_openOutputFile(inputPath, *iwrfPulse)) {
      cerr << "ERROR - AparTsSim::_convertFile" << endl;
      cerr << "  Processing file: " << inputPath << endl;
      return -1;
    }
    
    // add pulse to dwell
    
    _dwellPulses.push_back(iwrfPulse);

    // if we have a full dwell, process the pulses in it

    if (_dwellPulses.size() == nPulsesPerDwell) {

      // process dwell

      _processDwellForFile(_dwellPulses);
      _dwellSeqNum++;

      // delete pulses to free memory
      
      for (size_t ii = 0; ii < _dwellPulses.size(); ii++) {
        delete _dwellPulses[ii];
      }
      _dwellPulses.clear();

    }
      
    // read next one

    iwrfPulse = reader.getNextPulse();

  } // while

  // close output file

  _closeOutputFile();
  
  return 0;
  
}

/////////////////////////////
// process pulses in a dwell

int AparTsSim::_processDwellForFile(vector<IwrfTsPulse *> &dwellPulses)
  
{

  // compute the angles for the beams in the dwell

  double startAz = dwellPulses.front()->getAz();
  double endAz = dwellPulses.back()->getAz();
  double azRange = _conditionAngle360(endAz - startAz);
  double deltaAzPerBeam = azRange / _params.n_beams_per_dwell;

  double startEl = dwellPulses.front()->getEl();
  double endEl = dwellPulses.back()->getEl();
  double elRange = _conditionAngle360(endEl - startEl);
  double deltaElPerBeam = elRange / _params.n_beams_per_dwell;

  vector<double> beamAz, beamEl;
  for (int ii = 0; ii < _params.n_beams_per_dwell; ii++) {
    beamAz.push_back(_conditionAngle360(startAz + (ii + 0.5) * deltaAzPerBeam));
    beamEl.push_back(_conditionAngle180(startEl + (ii + 0.5) * deltaElPerBeam));
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-----------------------------------------------" << endl;
    cerr << "startAz, endAz, deltaAzPerBeam: "
         << startAz << ", " << endAz << ", " << deltaAzPerBeam << endl;
    cerr << "startEl, endEl, deltaElPerBeam: "
         << startEl << ", " << endEl << ", " << deltaElPerBeam << endl;
    for (int ii = 0; ii < _params.n_beams_per_dwell; ii++) {
      cerr << "  ii, az, el: "
           << ii << ", " << beamAz[ii] << ", " << beamEl[ii] << endl;
    }
    cerr << "-----------------------------------------------" << endl;
  }

  // loop through all of the pulses

  int pulseNumInDwell = 0;
  for (int ivisit = 0; ivisit < _params.n_visits_per_beam; ivisit++) {
    for (int ibeam = 0; ibeam < _params.n_beams_per_dwell; ibeam++) {
      for (int ipulse = 0; ipulse < _params.n_samples_per_visit; 
           ipulse++, pulseNumInDwell++) {
        
        // copy pulse header

        IwrfTsPulse *iwrfPulse = dwellPulses[pulseNumInDwell];
        apar_ts_pulse_header_t aparHdr;
        _copyIwrf2Apar(iwrfPulse->getHdr(), aparHdr);
        aparHdr.azimuth = beamAz[ibeam];
        aparHdr.elevation = beamEl[ibeam];
        aparHdr.dwell_seq_num = _dwellSeqNum;
        aparHdr.beam_num_in_dwell = ibeam;
        aparHdr.visit_num_in_beam = ivisit;
        aparHdr.chan_is_copol[0] = 1;
        aparHdr.pulse_seq_num = _pulseSeqNum;
        _pulseSeqNum++;

        // create co-polar pulse, set header

        AparTsPulse aparPulse(*_aparTsInfo, _aparTsDebug);
        aparPulse.setHeader(aparHdr);

        // add the co-pol IQ channel data as floats
        
        MemBuf iqBuf;
        fl32 **iqChans = iwrfPulse->getIqArray();
        // for (int ichan = 0; ichan < iwrfPulse->getNChannels(); ichan++) {
        //   iqBuf.add(iqChans[ichan], iwrfPulse->getNGates() * 2 * sizeof(fl32));
        // }
        // aparPulse.setIqFloats(iwrfPulse->getNGates(),
        //                       iwrfPulse->getNChannels(),
        //                       (const fl32 *) iqBuf.getPtr());
        iqBuf.add(iqChans[0], iwrfPulse->getNGates() * 2 * sizeof(fl32));
        aparPulse.setIqFloats(iwrfPulse->getNGates(), 1,
                              (const fl32 *) iqBuf.getPtr());

        
        // write ops info to file, if info has changed since last write
        
        if (_aparTsInfo->writeMetaQueueToFile(_out, true)) {
          cerr << "AparTsSim::_processDwellForFile" << endl;
          return -1;
        }

        // write pulse to file
        
        if (aparPulse.writeToFile(_out)) {
          cerr << "AparTsSim::_processDwellForFile" << endl;
          return -1;
        }

        // optionally add a cross-pol pulse

        if (_params.add_cross_pol_sample_at_end_of_visit &&
            ipulse == _params.n_samples_per_visit - 1) {

          apar_ts_pulse_header_t xpolHdr = aparHdr;
          xpolHdr.chan_is_copol[0] = 0;
          xpolHdr.pulse_seq_num = _pulseSeqNum;
          _pulseSeqNum++;
          
          AparTsPulse xpolPulse(*_aparTsInfo, _aparTsDebug);
          xpolPulse.setHeader(xpolHdr);
          
          // add the cross-pol IQ channel data as floats
          
          MemBuf xpolBuf;
          xpolBuf.add(iqChans[1], iwrfPulse->getNGates() * 2 * sizeof(fl32));
          xpolPulse.setIqFloats(iwrfPulse->getNGates(), 1,
                                (const fl32 *) xpolBuf.getPtr());
          
          // write out
          
          if (xpolPulse.writeToFile(_out)) {
            cerr << "AparTsSim::_processDwellForFile" << endl;
            return -1;
          }
          
        } // if (_params.add_cross_pol_sample_at_end_of_visit) {

      } // ipulse
    } // ibeam
  } // ivisit

  return 0;

}

/////////////////////////////////
// open output file, if needed

int AparTsSim::_openOutputFile(const string &inputPath,
                               const IwrfTsPulse &pulse)

{

  if (_out != NULL) {
    return 0;
  }

  // get time from pulse

  DateTime ptime(pulse.getTime());

  // make the output subdir

  char subdir[4000];
  sprintf(subdir, "%s%s%.4d%.2d%.2d",
          _params.output_dir,
          PATH_DELIM,
          ptime.getYear(), ptime.getMonth(), ptime.getDay());
  
  if (ta_makedir_recurse(subdir)) {
    int errNum = errno;
    cerr << "ERROR - AparTsSim" << endl;
    cerr << "  Cannot make output directory: " << subdir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // compute name from input path

  Path inPath(inputPath);
  string outputPath(subdir);
  outputPath += PATH_DELIM;
  outputPath += inPath.getBase();
  outputPath += ".apar_ts";

  // open file

  if ((_out = fopen(outputPath.c_str(), "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - AparTsSim" << endl;
    cerr << "  Cannot open output file: " << outputPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Writing to output file: " << outputPath << endl;
  }

  return 0;

}

///////////////////////////////////////////
// close output file
//
// Returns 0 if file already open, -1 if not

void AparTsSim::_closeOutputFile()

{
  
  // close out old file
  
  if (_out != NULL) {
    fclose(_out);
    _out = NULL;
  }

}
  
/////////////////////////////
// reformat

void AparTsSim::_reformat2Apar(const IwrfTsPulse &pulse)
  
{
  

}

///////////////////////////////////////////////
// Convert the IWRF metadata to APAR structs

void AparTsSim::_convertMeta2Apar(const IwrfTsInfo &info)
  
{

  // initialize the apar structs
  
  apar_ts_radar_info_init(_aparRadarInfo);
  apar_ts_scan_segment_init(_aparScanSegment);
  apar_ts_processing_init(_aparTsProcessing);
  apar_ts_calibration_init(_aparCalibration);

  // copy over the metadata members

  _copyIwrf2Apar(info.getRadarInfo(), _aparRadarInfo);
  _copyIwrf2Apar(info.getScanSegment(), _aparScanSegment);
  _copyIwrf2Apar(info.getTsProcessing(), _aparTsProcessing);
  if (info.isCalibrationActive()) {
    _copyIwrf2Apar(info.getCalibration(), _aparCalibration);
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    apar_ts_radar_info_print(stderr, _aparRadarInfo);
    apar_ts_scan_segment_print(stderr, _aparScanSegment);
    apar_ts_processing_print(stderr, _aparTsProcessing);
    apar_ts_calibration_print(stderr, _aparCalibration);
  }
}

//////////////////////////////////////////////////
// Copy members from IWRF structs to APAR structs

void AparTsSim::_copyIwrf2Apar(const iwrf_packet_info_t &iwrf,
                               apar_ts_packet_info_t &apar)
{

  apar_ts_packet_info_init(apar);
  apar.id = iwrf.id;
  apar.len_bytes = iwrf.len_bytes;
  apar.seq_num = iwrf.seq_num;
  apar.version_num = iwrf.version_num;
  apar.radar_id = iwrf.radar_id;
  apar.time_secs_utc = iwrf.time_secs_utc;
  apar.time_nano_secs = iwrf.time_nano_secs;
 
}

void AparTsSim::_copyIwrf2Apar(const iwrf_radar_info_t &iwrf,
                               apar_ts_radar_info_t &apar)
{

  apar_ts_radar_info_init(apar);

  _copyIwrf2Apar(iwrf.packet, apar.packet);

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

void AparTsSim::_copyIwrf2Apar(const iwrf_scan_segment_t &iwrf,
                               apar_ts_scan_segment_t &apar)
{

  apar_ts_scan_segment_init(apar);

  _copyIwrf2Apar(iwrf.packet, apar.packet);

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

  memcpy(apar.fixed_angles, iwrf.fixed_angles, IWRF_MAX_FIXED_ANGLES * sizeof(fl32));
  
  apar.sun_scan_sector_width_az = iwrf.sun_scan_sector_width_az;
  apar.sun_scan_sector_width_el = iwrf.sun_scan_sector_width_el;

  STRncopy(apar.segment_name, iwrf.segment_name, IWRF_MAX_SEGMENT_NAME);
  STRncopy(apar.project_name, iwrf.project_name, IWRF_MAX_PROJECT_NAME);

}

void AparTsSim::_copyIwrf2Apar(const iwrf_ts_processing_t &iwrf,
                               apar_ts_processing_t &apar)
{

  apar_ts_processing_init(apar);

  _copyIwrf2Apar(iwrf.packet, apar.packet);

  // apar.xmit_rcv_mode = iwrf.xmit_rcv_mode;
  apar.pol_mode = iwrf.pol_mode;
  apar.prf_mode = iwrf.prf_mode;
  apar.pulse_shape = iwrf.pulse_type;
  apar.pulse_width_us = iwrf.pulse_width_us;
  apar.start_range_m = iwrf.start_range_m;
  apar.gate_spacing_m = iwrf.gate_spacing_m;

  // apar.n_samples_per_visit = _params.n_samples_per_visit;
  // apar.n_visits_per_beam = _params.n_visits_per_beam;
  // apar.n_beams_per_dwell = _params.n_beams_per_dwell;

  apar.test_pulse_range_km = iwrf.test_pulse_range_km;
  apar.test_pulse_length_us = iwrf.test_pulse_length_usec;
  
  apar.num_prts = iwrf.num_prts;
  apar.prt_us[0] = iwrf.prt_usec;
  apar.prt_us[1] = iwrf.prt2_usec;
  apar.prt_us[2] = iwrf.prt3_usec;
  apar.prt_us[3] = iwrf.prt4_usec;
  
}

void AparTsSim::_copyIwrf2Apar(const iwrf_calibration_t &iwrf,
                               apar_ts_calibration_t &apar)
{

  apar_ts_calibration_init(apar);

  _copyIwrf2Apar(iwrf.packet, apar.packet);

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

void AparTsSim::_copyIwrf2Apar(const iwrf_pulse_header_t &iwrf,
                               apar_ts_pulse_header_t &apar)
{

  apar_ts_pulse_header_init(apar);

  _copyIwrf2Apar(iwrf.packet, apar.packet);

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

void AparTsSim::_copyIwrf2Apar(const iwrf_event_notice_t &iwrf,
                               apar_ts_event_notice_t &apar)
{

  apar_ts_event_notice_init(apar);

  _copyIwrf2Apar(iwrf.packet, apar.packet);

  apar.start_of_sweep = iwrf.start_of_sweep;
  apar.end_of_sweep = iwrf.end_of_sweep;

  apar.start_of_volume = iwrf.start_of_volume;
  apar.end_of_volume = iwrf.end_of_volume;
  
  apar.scan_mode = iwrf.scan_mode;
  apar.volume_num = iwrf.volume_num;
  apar.sweep_num = iwrf.sweep_num;
  
  apar.current_fixed_angle = iwrf.current_fixed_angle;

}

////////////////////////////////////
// condition angle from 0 to 360

double AparTsSim::_conditionAngle360(double angle)
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

double AparTsSim::_conditionAngle180(double angle)
{
  if (angle < -180) {
    return angle + 360.0;
  } else if (angle >= 180) {
    return angle - 360.0;
  } else {
    return angle;
  }
}

////////////////////////////////////////////////////
// Convert 1 file to UDP

int AparTsSim::_convert2Udp(const string &inputPath)
  
{
  
  if (_params.debug) {
    cerr << "Reading input file: " << inputPath << endl;
  }

  // set up a vector with a single file entry
  
  vector<string> fileList;
  fileList.push_back(inputPath);

  // create reader for just that one file

  IwrfDebug_t iwrfDebug = IWRF_DEBUG_OFF;
  if (_params.debug >= Params::DEBUG_EXTRA) {
    iwrfDebug = IWRF_DEBUG_VERBOSE;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrfDebug = IWRF_DEBUG_NORM;
  } 
  IwrfTsReaderFile reader(fileList, iwrfDebug);
  const IwrfTsInfo &tsInfo = reader.getOpsInfo();

  // read through pulses until we have current metadata
  
  {
    IwrfTsPulse *iwrfPulse = reader.getNextPulse();
    bool haveMetadata = false;
    while (iwrfPulse != NULL) {
      if (tsInfo.isRadarInfoActive() &&
          tsInfo.isScanSegmentActive() &&
          tsInfo.isTsProcessingActive()) {
        // we have the necessary metadata
        haveMetadata = true;
        delete iwrfPulse;
        break;
      }
      delete iwrfPulse;
    }
    if (!haveMetadata) {
      cerr << "ERROR - AparTsSim::_convert2Udp()" << endl;
      cerr << "Metadata missing for file: " << inputPath << endl;
      return -1;
    }
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_radar_info_print(stderr, tsInfo.getRadarInfo());
    iwrf_scan_segment_print(stderr, tsInfo.getScanSegment());
    iwrf_ts_processing_print(stderr, tsInfo.getTsProcessing());
    if (tsInfo.isCalibrationActive()) {
      iwrf_calibration_print(stderr, tsInfo.getCalibration());
    }
  }

  // reset reader queue to start

  reader.reset();

  // compute number of pulses per dwell

  size_t nPulsesPerDwell = 
    _params.n_samples_per_visit *
    _params.n_visits_per_beam *
    _params.n_beams_per_dwell;

  if (_params.debug) {
    cerr << "  ==>> nPulsesPerDwell: " << nPulsesPerDwell << endl;
  }

  // read in all pulses

  IwrfTsPulse *iwrfPulse = reader.getNextPulse();
  while (iwrfPulse != NULL) {
    
    // open output UDP as needed
    
    if (_openOutputUdp()) {
      cerr << "ERROR - AparTsSim::_convert2Udp" << endl;
      cerr << "  Processing file: " << inputPath << endl;
      cerr << "  Cannot open UDP output device" << endl;
      return -1;
    }
    
    // add pulse to dwell
    
    _dwellPulses.push_back(iwrfPulse);
    
    // if we have a full dwell, process the pulses in it

    if (_dwellPulses.size() == nPulsesPerDwell) {

      // process dwell

      _processDwellForUdp(_dwellPulses);
      _dwellSeqNum++;

      // delete pulses to free memory
      
      for (size_t ii = 0; ii < _dwellPulses.size(); ii++) {
        delete _dwellPulses[ii];
      }
      _dwellPulses.clear();

    }
      
    // read next one

    iwrfPulse = reader.getNextPulse();

  } // while

  // close output file

  _closeOutputFile();
  
  return 0;
  
}

////////////////////////////////////////////
// process pulses in a dwell for UDP output

int AparTsSim::_processDwellForUdp(vector<IwrfTsPulse *> &dwellPulses)
  
{

  // compute the angles for the beams in the dwell

  double startAz = dwellPulses.front()->getAz();
  double endAz = dwellPulses.back()->getAz();
  double azRange = _conditionAngle360(endAz - startAz);
  double deltaAzPerBeam = azRange / _params.n_beams_per_dwell;

  double startEl = dwellPulses.front()->getEl();
  double endEl = dwellPulses.back()->getEl();
  double elRange = _conditionAngle360(endEl - startEl);
  double deltaElPerBeam = elRange / _params.n_beams_per_dwell;

  vector<double> beamAz, beamEl;
  for (int ii = 0; ii < _params.n_beams_per_dwell; ii++) {
    beamAz.push_back(_conditionAngle360(startAz + (ii + 0.5) * deltaAzPerBeam));
    beamEl.push_back(_conditionAngle180(startEl + (ii + 0.5) * deltaElPerBeam));
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-----------------------------------------------" << endl;
    cerr << "startAz, endAz, deltaAzPerBeam: "
         << startAz << ", " << endAz << ", " << deltaAzPerBeam << endl;
    cerr << "startEl, endEl, deltaElPerBeam: "
         << startEl << ", " << endEl << ", " << deltaElPerBeam << endl;
    for (int ii = 0; ii < _params.n_beams_per_dwell; ii++) {
      cerr << "  ii, az, el: "
           << ii << ", " << beamAz[ii] << ", " << beamEl[ii] << endl;
    }
    cerr << "-----------------------------------------------" << endl;
  }

  // loop through all of the pulses

  int pulseNumInDwell = 0;
  for (int ivisit = 0; ivisit < _params.n_visits_per_beam; ivisit++) {
    for (int ibeam = 0; ibeam < _params.n_beams_per_dwell; ibeam++) {
      for (int ipulse = 0; ipulse < _params.n_samples_per_visit; 
           ipulse++, pulseNumInDwell++) {
        
        // convert to si16 as needed
        
        IwrfTsPulse *iwrfPulse = dwellPulses[pulseNumInDwell];
        if (iwrfPulse->getPackedEncoding() != IWRF_IQ_ENCODING_SCALED_SI16) {
          iwrfPulse->convertToPacked(IWRF_IQ_ENCODING_SCALED_SI16);
        }

        // set the metadata

        ui64 sampleNumber = _sampleSeqNum;
        ui64 pulseNumber = _pulseSeqNum;
        
        si64 secondsTime = iwrfPulse->getTime();
        ui32 nanoSecs = iwrfPulse->getNanoSecs();
        
        ui32 pulseStartIndex = 0;
        ui64 dwellNum = _dwellSeqNum;
        ui32 beamNumInDwell = ibeam;
        ui32 visitNumInBeam = ivisit;
        
        double azRad = beamAz[ibeam] * DEG_TO_RAD;
        double elRad = beamEl[ibeam] * DEG_TO_RAD;
        double uu = cos(elRad) * sin(azRad);
        double vv = sin(elRad);
        
        bool isXmitH = iwrfPulse->isHoriz();
        bool isCoPolRx = true;
        
        int nGates = iwrfPulse->getNGates();
        const si16 *iqData = (const si16 *) iwrfPulse->getPackedData();

        // create the buffer for co-polar packet
        
        MemBuf pktBuf;
        _createPacketBuf(sampleNumber,
                         pulseNumber,
                         secondsTime, nanoSecs,
                         pulseStartIndex,
                         dwellNum,
                         beamNumInDwell,
                         visitNumInBeam,
                         uu, vv, 
                         isXmitH, isCoPolRx,
                         nGates, iqData,
                         pktBuf);

        // write the packet to UDP

        if (_writeBufToUdp(pktBuf)) {
          cerr << "ERROR - _processDwellForUdp" << endl;
          return -1;
        }
        _pulseSeqNum++;
        _sampleSeqNum += nGates;

        // optionally add a cross-pol pulse
        // at the end of the dwell
        
        if (_params.add_cross_pol_sample_at_end_of_visit &&
            ipulse == _params.n_samples_per_visit - 1) {
          
          isCoPolRx = false;
          sampleNumber = _sampleSeqNum;
          pulseNumber = _pulseSeqNum;
          iqData += nGates * 2; // channel 1
          
          pktBuf.clear();
          _createPacketBuf(sampleNumber,
                           pulseNumber,
                           secondsTime, nanoSecs,
                           pulseStartIndex,
                           dwellNum,
                           beamNumInDwell,
                           visitNumInBeam,
                           uu, vv, 
                           isXmitH, isCoPolRx,
                           nGates, iqData,
                           pktBuf);

          // write the packet to UDP
          
          if (_writeBufToUdp(pktBuf)) {
            cerr << "ERROR - _processDwellForUdp" << endl;
            return -1;
          }

          _pulseSeqNum++;
          _sampleSeqNum += nGates;
          
        } // if (_params.add_cross_pol_sample_at_end_of_visit ...

      } // ipulse
    } // ibeam
  } // ivisit

  return 0;

}

////////////////////////////////////////////////////////////////////////
// create a packet buffer

void AparTsSim::_createPacketBuf(ui64 sampleNumber,
                                 ui64 pulseNumber,
                                 si64 secondsTime,
                                 ui32 nanoSecs,
                                 ui32 pulseStartIndex,
                                 ui64 dwellNum,
                                 ui32 beamNumInDwell,
                                 ui32 visitNumInBeam,
                                 double uu,
                                 double vv,
                                 bool isXmitH,
                                 bool isCoPolRx,
                                 int nGates,
                                 const si16 *iqData,
                                 MemBuf &buf)

{
  
  // init

  buf.clear();

  // message type

  ui16 messageType = 0x0001;
  BE_from_array_16(&messageType, sizeof(messageType));
  buf.add(&messageType, sizeof(messageType));

  // AESA ID

  ui16 aesaId = 0;
  BE_from_array_16(&aesaId, sizeof(aesaId));
  buf.add(&aesaId, sizeof(aesaId));

  // channel number

  ui16 chanNum = 0;
  BE_from_array_16(&chanNum, sizeof(chanNum));
  buf.add(&chanNum, sizeof(chanNum));

  // flags
  
  ui32 flags = 0;
  if (isXmitH) {
    // bit 0 indicates H transmit
    flags |= 1;
  }
  if ((isXmitH && isCoPolRx) || (!isXmitH && !isCoPolRx)) {
    // git 1 indicates H receive
    flags |= 2;
  }
  // bit 3 is pulse start flag
  // always on since we have 1 pulse per packet
  flags |= 4;

  BE_from_array_32(&flags, sizeof(flags));
  buf.add(&flags, sizeof(flags));

  // beam index - always 200 for now

  ui32 beamIndex = 200;
  BE_from_array_32(&beamIndex, sizeof(beamIndex));
  buf.add(&beamIndex, sizeof(beamIndex));
  
  // sample number - first sample in this packet

  ui64 sampleNum = sampleNumber;
  BE_from_array_64(&sampleNum, sizeof(sampleNum));
  buf.add(&sampleNum, sizeof(sampleNum));
  
  // pulse number

  ui64 pulseNum = pulseNumber;
  BE_from_array_64(&pulseNum, sizeof(pulseNum));
  buf.add(&pulseNum, sizeof(pulseNum));

  // time

  si64 secs = secondsTime;
  BE_from_array_64(&secs, sizeof(secs));
  buf.add(&secs, sizeof(secs));

  ui32 nsecs = nanoSecs;
  BE_from_array_32(&nsecs, sizeof(nsecs));
  buf.add(&nsecs, sizeof(nsecs));

  // start index

  ui32 startIndex = pulseStartIndex;
  BE_from_array_32(&startIndex, sizeof(startIndex));
  buf.add(&startIndex, sizeof(startIndex));

  // angles

  fl32 u = uu;
  BE_from_array_32(&u, sizeof(u));
  buf.add(&u, sizeof(u));

  fl32 v = vv;
  BE_from_array_32(&v, sizeof(v));
  buf.add(&v, sizeof(v));

  // dwell, beam, visit

  ui64 dNum = dwellNum;
  BE_from_array_64(&dNum, sizeof(dNum));
  buf.add(&dNum, sizeof(dNum));

  ui32 bNum = beamNumInDwell;
  BE_from_array_32(&bNum, sizeof(bNum));
  buf.add(&bNum, sizeof(bNum));

  ui32 vNum = visitNumInBeam;
  BE_from_array_32(&vNum, sizeof(vNum));
  buf.add(&vNum, sizeof(vNum));

  // number of samples = number of gates

  ui32 nSamples = nGates;
  BE_from_array_32(&nSamples, sizeof(nSamples));
  buf.add(&nSamples, sizeof(nSamples));

  // 3 spares

  ui32 spare = 0;
  buf.add(&spare, sizeof(spare));
  buf.add(&spare, sizeof(spare));
  buf.add(&spare, sizeof(spare));

  // IQ data

  si16 *iq = new si16[nGates * 2];
  int nBytesIq = nGates * 2 * sizeof(si16);
  memcpy(iq, iqData, nBytesIq);
  BE_from_array_16(iq, nBytesIq);
  buf.add(iq, nBytesIq);
  delete[] iq;

}

  
////////////////////////////////////////////////////
// Open output UDP device
// Returns 0 on success, -1 on error

int AparTsSim::_openOutputUdp()
  
{

  if (_udpFd > 0) {
    // already open
    return 0;
  }

  // open socket
  
  if  ((_udpFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    perror ("Could not open UDP socket: ");
    _udpFd = -1;
    return -1;
  }
  
  // bind local address to the socket
  
  struct sockaddr_in localAddr;
  MEM_zero(localAddr);
  uint16_t sourcePort = _params.udp_source_port;
  localAddr.sin_port = htons(sourcePort);
  localAddr.sin_family = AF_INET;
  localAddr.sin_addr.s_addr = htonl (INADDR_ANY);
  
  if (bind (_udpFd, (struct sockaddr *) &localAddr, 
	    sizeof (localAddr)) < 0) {
    perror ("bind error:");
    fprintf(stderr, "Could bind UDP socket, port %d\n", sourcePort);
    close (_udpFd);
    _udpFd = -1;
    return -1;
  }
  
  // set socket for broadcast
  
  int option = 1;
  if (setsockopt(_udpFd, SOL_SOCKET, SO_BROADCAST | SO_REUSEADDR,
		 (char *) &option, sizeof(option)) < 0) {
    perror ("Could not set broadcast on - setsockopt error");
    close (_udpFd);
    _udpFd = -1;
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////
// Write buffer to UDP
// Returns 0 on success, -1 on error

int AparTsSim::_writeBufToUdp(const MemBuf &buf)
  
{

  // set up destination address structure
  
  struct sockaddr_in destAddr;
  MEM_zero(destAddr);
  if (inet_aton(_params.udp_dest_address, &destAddr.sin_addr) == 0) {
    fprintf(stderr, "Cannot translate address: %s - may be invalid\n",
            _params.udp_dest_address);
    close (_udpFd);
    _udpFd = -1;
    return -1;
  }
  destAddr.sin_family = AF_INET;
  uint16_t destPort = _params.udp_dest_port;
  destAddr.sin_port = htons(destPort);
  destAddr.sin_addr.s_addr = inet_addr(_params.udp_dest_address);
  
  if (sendto(_udpFd, buf.getPtr(), buf.getLen(), 0,
             (struct sockaddr *) &destAddr,
             sizeof(destAddr)) != (ssize_t) buf.getLen()) {
    if (_errCount % 1000 == 0) {
      perror("_writeBufToUdp: ");
      cerr << "Cannot write UDP packet to port: " << _params.udp_dest_port << endl;
      cerr << "  Pkt len: " << buf.getLen() << endl;
    }
    _errCount++;
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Sent UDP packet, len: " << buf.getLen() << endl;
  }
  
  return 0;

}


