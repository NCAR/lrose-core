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
// WriteToFile.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2019
//
///////////////////////////////////////////////////////////////
//
// Resample IWRF time series data,
// convert to IPS time series format,
// and write out to files
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <cerrno>
#include <cstdio>
#include <unistd.h>

#include <dataport/bigend.h>
#include <toolsa/DateTime.hh>
#include <toolsa/Path.hh>
#include <toolsa/file_io.h>
#include <toolsa/str.h>
#include <toolsa/MemBuf.hh>
#include <radar/ips_ts_functions.hh>
#include <radar/IpsTsPulse.hh>

#include "WriteToFile.hh"
#include "IpsTsSim.hh"
#include "SimScanStrategy.hh"

using namespace std;

// Constructor

WriteToFile::WriteToFile(const string &progName,
                         const Params &params,
                         vector<string> &inputFileList) :
        _progName(progName),
        _params(params),
        _inputFileList(inputFileList)
  
{

  _ipsTsInfo = NULL;
  _pulseSeqNum = 0;
  _dwellSeqNum = 0;

  _ipsTsDebug = IpsTsDebug_t::OFF;
  if (_params.debug >= Params::DEBUG_EXTRA) {
    _ipsTsDebug = IpsTsDebug_t::VERBOSE;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    _ipsTsDebug = IpsTsDebug_t::NORM;
  }
  _ipsTsInfo = new IpsTsInfo(_ipsTsDebug);

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Running WriteToFile - extra verbose debug mode" << endl;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Running WriteToFile - verbose debug mode" << endl;
  } else if (_params.debug) {
    cerr << "Running WriteToFile - debug mode" << endl;
  }

  // compute the scan strategy

  _strategy = new SimScanStrategy(_params);
  
}

// destructor

WriteToFile::~WriteToFile()
  
{

  delete _strategy;
  
  // delete pulses to free memory
  
  for (size_t ii = 0; ii < _dwellPulses.size(); ii++) {
    delete _dwellPulses[ii];
  }
  _dwellPulses.clear();

}

//////////////////////////////////////////////////
// Run

int WriteToFile::Run ()
{
  
  PMU_auto_register("WriteToFile::Run");
  
  // loop through the input files
  
  int iret = 0;
  for (size_t ii = 0; ii < _inputFileList.size(); ii++) {
    if (_convertFile(_inputFileList[ii])) {
      iret = -1;
    }
  }

  return iret;

}

////////////////////////////////////////////////////
// Convert 1 file to IPS format

int WriteToFile::_convertFile(const string &inputPath)
  
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
      cerr << "ERROR - WriteToFile::_convertFile()" << endl;
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

  // convert the metadata to IPS types
  // set the metadata in the info metadata queue

  _convertMeta2Ips(tsInfo);
  _ipsTsInfo->setRadarInfo(_ipsRadarInfo);
  _ipsTsInfo->setScanSegment(_ipsScanSegment);
  _ipsTsInfo->setTsProcessing(_ipsTsProcessing);
  if (tsInfo.isCalibrationActive()) {
    _ipsTsInfo->setCalibration(_ipsCalibration);
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
      cerr << "ERROR - WriteToFile::_convertFile" << endl;
      cerr << "  Processing file: " << inputPath << endl;
      return -1;
    }
    
    // add pulse to dwell
    
    _dwellPulses.push_back(iwrfPulse);

    // if we have a full dwell, process the pulses in it

    if (_dwellPulses.size() == nPulsesPerDwell) {

      // process dwell

      _processDwell(_dwellPulses);
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

int WriteToFile::_processDwell(vector<IwrfTsPulse *> &dwellPulses)
  
{

  // compute the angles for the beams in the dwell

  double startAz = dwellPulses.front()->getAz();
  double endAz = dwellPulses.back()->getAz();
  double azRange = IpsTsSim::conditionAngle360(endAz - startAz);
  double deltaAzPerBeam = azRange / _params.n_beams_per_dwell;

  double startEl = dwellPulses.front()->getEl();
  double endEl = dwellPulses.back()->getEl();
  double elRange = IpsTsSim::conditionAngle360(endEl - startEl);
  double deltaElPerBeam = elRange / _params.n_beams_per_dwell;

  vector<double> beamAz, beamEl;
  vector<int> sweepNum, volNum;
  vector<Radx::SweepMode_t> sweepMode;

  for (int ii = 0; ii < _params.n_beams_per_dwell; ii++) {
    if (_params.specify_scan_strategy) {
      SimScanStrategy::angle_t angle = _strategy->getNextAngle();
      beamAz.push_back(angle.az);
      beamEl.push_back(angle.el);
      sweepNum.push_back(angle.sweepNum);
      volNum.push_back(angle.volNum);
      sweepMode.push_back(angle.sweepMode);
    } else {
      beamAz.push_back(IpsTsSim::conditionAngle360
                       (startAz + (ii + 0.5) * deltaAzPerBeam));
      beamEl.push_back(IpsTsSim::conditionAngle180
                       (startEl + (ii + 0.5) * deltaElPerBeam));
    } // if (_params.specify_scan_strategy)
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
        ips_ts_pulse_header_t ipsHdr;
        IpsTsSim::copyIwrf2Ips(iwrfPulse->getHdr(), ipsHdr);
        ipsHdr.azimuth = beamAz[ibeam];
        ipsHdr.elevation = beamEl[ibeam];
        ipsHdr.dwell_seq_num = _dwellSeqNum;
        ipsHdr.beam_num_in_dwell = ibeam;
        ipsHdr.visit_num_in_beam = ivisit;
        ipsHdr.chan_is_copol[0] = 1;
        ipsHdr.pulse_seq_num = _pulseSeqNum;

        if (_params.specify_scan_strategy) {
          ipsHdr.volume_num = volNum[ibeam];
          ipsHdr.sweep_num = sweepNum[ibeam];
          if (sweepMode[ibeam] == Radx::SWEEP_MODE_RHI) {
            ipsHdr.scan_mode = (int) ips_ts_scan_mode_t::RHI;
          } else {
            ipsHdr.scan_mode = (int) ips_ts_scan_mode_t::PPI;
          }
        }

        _pulseSeqNum++;

        // create co-polar pulse, set header

        IpsTsPulse ipsPulse(*_ipsTsInfo, _ipsTsDebug);
        ipsPulse.setHeader(ipsHdr);

        // add the co-pol IQ channel data as floats
        
        MemBuf iqBuf;
        fl32 **iqChans = iwrfPulse->getIqArray();
        iqBuf.add(iqChans[0], iwrfPulse->getNGates() * 2 * sizeof(fl32));
        ipsPulse.setIqFloats(iwrfPulse->getNGates(), 1,
                              (const fl32 *) iqBuf.getPtr());

        
        // write ops info to file, if info has changed since last write
        
        if (_ipsTsInfo->writeMetaQueueToFile(_out, true)) {
          cerr << "WriteToFile::_processDwellForFile" << endl;
          return -1;
        }

        // write pulse to file
        
        if (ipsPulse.writeToFile(_out)) {
          cerr << "WriteToFile::_processDwellForFile" << endl;
          return -1;
        }

        // optionally add a cross-pol pulse

        if (_params.add_cross_pol_sample_at_end_of_visit &&
            ipulse == _params.n_samples_per_visit - 1) {

          ips_ts_pulse_header_t xpolHdr = ipsHdr;
          xpolHdr.chan_is_copol[0] = 0;
          xpolHdr.pulse_seq_num = _pulseSeqNum;
          _pulseSeqNum++;
          
          IpsTsPulse xpolPulse(*_ipsTsInfo, _ipsTsDebug);
          xpolPulse.setHeader(xpolHdr);
          
          // add the cross-pol IQ channel data as floats
          
          MemBuf xpolBuf;
          xpolBuf.add(iqChans[1], iwrfPulse->getNGates() * 2 * sizeof(fl32));
          xpolPulse.setIqFloats(iwrfPulse->getNGates(), 1,
                                (const fl32 *) xpolBuf.getPtr());
          
          // write out
          
          if (xpolPulse.writeToFile(_out)) {
            cerr << "WriteToFile::_processDwellForFile" << endl;
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

int WriteToFile::_openOutputFile(const string &inputPath,
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
    cerr << "ERROR - WriteToFile" << endl;
    cerr << "  Cannot make output directory: " << subdir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // compute name from input path

  Path inPath(inputPath);
  string outputPath(subdir);
  outputPath += PATH_DELIM;
  outputPath += inPath.getBase();
  outputPath += ".ips_ts";

  // open file

  if ((_out = fopen(outputPath.c_str(), "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - WriteToFile" << endl;
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

void WriteToFile::_closeOutputFile()

{
  
  // close out old file
  
  if (_out != NULL) {
    fclose(_out);
    _out = NULL;
  }

}
  
///////////////////////////////////////////////
// Convert the IWRF metadata to IPS structs

void WriteToFile::_convertMeta2Ips(const IwrfTsInfo &info)
  
{

  // initialize the ips structs
  
  ips_ts_radar_info_init(_ipsRadarInfo);
  ips_ts_scan_segment_init(_ipsScanSegment);
  ips_ts_processing_init(_ipsTsProcessing);
  ips_ts_calibration_init(_ipsCalibration);

  // copy over the metadata members

  IpsTsSim::copyIwrf2Ips(info.getRadarInfo(), _ipsRadarInfo);
  IpsTsSim::copyIwrf2Ips(info.getScanSegment(), _ipsScanSegment);
  IpsTsSim::copyIwrf2Ips(info.getTsProcessing(), _ipsTsProcessing);
  if (info.isCalibrationActive()) {
    IpsTsSim::copyIwrf2Ips(info.getCalibration(), _ipsCalibration);
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    ips_ts_radar_info_print(stderr, _ipsRadarInfo);
    ips_ts_scan_segment_print(stderr, _ipsScanSegment);
    ips_ts_processing_print(stderr, _ipsTsProcessing);
    ips_ts_calibration_print(stderr, _ipsCalibration);
  }
}

