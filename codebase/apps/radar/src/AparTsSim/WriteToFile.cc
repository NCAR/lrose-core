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
// convert to APAR time series format,
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
#include <radar/apar_ts_functions.hh>
#include <radar/AparTsPulse.hh>

#include "WriteToFile.hh"
#include "AparTsSim.hh"

using namespace std;

// Constructor

WriteToFile::WriteToFile(const string &progName,
                         const Params &params,
                         vector<string> &inputFileList) :
        _progName(progName),
        _params(params),
        _inputFileList(inputFileList)
  
{

  _aparTsInfo = NULL;
  _pulseSeqNum = 0;
  _dwellSeqNum = 0;

  _aparTsDebug = AparTsDebug_t::OFF;
  if (_params.debug >= Params::DEBUG_EXTRA) {
    _aparTsDebug = AparTsDebug_t::VERBOSE;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    _aparTsDebug = AparTsDebug_t::NORM;
  }
  _aparTsInfo = new AparTsInfo(_aparTsDebug);

  // compute the scan strategy
  
  _simVolNum = 0;
  _simBeamNum = 0;
  _computeScanStrategy();

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Running WriteToFile - extra verbose debug mode" << endl;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Running WriteToFile - verbose debug mode" << endl;
  } else if (_params.debug) {
    cerr << "Running WriteToFile - debug mode" << endl;
  }

}

// destructor

WriteToFile::~WriteToFile()
  
{
  
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
// Convert 1 file to APAR format

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
  double azRange = AparTsSim::conditionAngle360(endAz - startAz);
  double deltaAzPerBeam = azRange / _params.n_beams_per_dwell;

  double startEl = dwellPulses.front()->getEl();
  double endEl = dwellPulses.back()->getEl();
  double elRange = AparTsSim::conditionAngle360(endEl - startEl);
  double deltaElPerBeam = elRange / _params.n_beams_per_dwell;

  vector<double> beamAz, beamEl;
  vector<int> sweepNum, volNum;
  vector<Radx::SweepMode_t> sweepMode;

  for (int ii = 0; ii < _params.n_beams_per_dwell; ii++) {
    if (_params.specify_scan_strategy) {
      if (_simBeamNum >= _simEl.size()) {
        _simBeamNum = 0;
        _simVolNum++;
      }
      beamAz.push_back(_simAz[_simBeamNum]);
      beamEl.push_back(_simEl[_simBeamNum]);
      sweepNum.push_back(_simSweepNum[_simBeamNum]);
      volNum.push_back(_simVolNum);
      sweepMode.push_back(_simSweepMode[_simBeamNum]);
    } else {
      beamAz.push_back(AparTsSim::conditionAngle360
                       (startAz + (ii + 0.5) * deltaAzPerBeam));
      beamEl.push_back(AparTsSim::conditionAngle180
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
        apar_ts_pulse_header_t aparHdr;
        AparTsSim::copyIwrf2Apar(iwrfPulse->getHdr(), aparHdr);
        aparHdr.azimuth = beamAz[ibeam];
        aparHdr.elevation = beamEl[ibeam];
        aparHdr.dwell_seq_num = _dwellSeqNum;
        aparHdr.beam_num_in_dwell = ibeam;
        aparHdr.visit_num_in_beam = ivisit;
        aparHdr.chan_is_copol[0] = 1;
        aparHdr.pulse_seq_num = _pulseSeqNum;

        if (_params.specify_scan_strategy) {
          aparHdr.volume_num = volNum[ibeam];
          aparHdr.sweep_num = sweepNum[ibeam];
          if (sweepMode[ibeam] == Radx::SWEEP_MODE_RHI) {
            aparHdr.scan_mode = (int) apar_ts_scan_mode_t::RHI;
          } else {
            aparHdr.scan_mode = (int) apar_ts_scan_mode_t::PPI;
          }
        }

        _pulseSeqNum++;

        // create co-polar pulse, set header

        AparTsPulse aparPulse(*_aparTsInfo, _aparTsDebug);
        aparPulse.setHeader(aparHdr);

        // add the co-pol IQ channel data as floats
        
        MemBuf iqBuf;
        fl32 **iqChans = iwrfPulse->getIqArray();
        iqBuf.add(iqChans[0], iwrfPulse->getNGates() * 2 * sizeof(fl32));
        aparPulse.setIqFloats(iwrfPulse->getNGates(), 1,
                              (const fl32 *) iqBuf.getPtr());

        
        // write ops info to file, if info has changed since last write
        
        if (_aparTsInfo->writeMetaQueueToFile(_out, true)) {
          cerr << "WriteToFile::_processDwellForFile" << endl;
          return -1;
        }

        // write pulse to file
        
        if (aparPulse.writeToFile(_out)) {
          cerr << "WriteToFile::_processDwellForFile" << endl;
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
  outputPath += ".apar_ts";

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
// Convert the IWRF metadata to APAR structs

void WriteToFile::_convertMeta2Apar(const IwrfTsInfo &info)
  
{

  // initialize the apar structs
  
  apar_ts_radar_info_init(_aparRadarInfo);
  apar_ts_scan_segment_init(_aparScanSegment);
  apar_ts_processing_init(_aparTsProcessing);
  apar_ts_calibration_init(_aparCalibration);

  // copy over the metadata members

  AparTsSim::copyIwrf2Apar(info.getRadarInfo(), _aparRadarInfo);
  AparTsSim::copyIwrf2Apar(info.getScanSegment(), _aparScanSegment);
  AparTsSim::copyIwrf2Apar(info.getTsProcessing(), _aparTsProcessing);
  if (info.isCalibrationActive()) {
    AparTsSim::copyIwrf2Apar(info.getCalibration(), _aparCalibration);
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    apar_ts_radar_info_print(stderr, _aparRadarInfo);
    apar_ts_scan_segment_print(stderr, _aparScanSegment);
    apar_ts_processing_print(stderr, _aparTsProcessing);
    apar_ts_calibration_print(stderr, _aparCalibration);
  }
}

////////////////////////////////////////////////////////////////////////
// compute simulated scan strategy

void WriteToFile::_computeScanStrategy()

{

  int beamsPerDwell = _params.n_beams_per_dwell;
  int sweepNum = 0;
  
  for (int iscan = 0; iscan < _params.sim_scans_n; iscan++) {
    
    const Params::sim_scan_t &scan = _params._sim_scans[iscan];
    
    if (scan.sim_type == Params::RHI_SIM) {
      
      Radx::SweepMode_t sweepMode = Radx::SWEEP_MODE_RHI;
      
      for (double az = scan.min_az;
           az <= scan.max_az;
           az += scan.delta_az) {
        for (int istride = 0; istride < beamsPerDwell; istride++) {
          for (double el = scan.min_el + istride * scan.delta_el;
               el <= scan.max_el; el += beamsPerDwell * scan.delta_el) {
            _simEl.push_back(el);
            _simAz.push_back(az);
            _simSweepNum.push_back(sweepNum);
            _simSweepMode.push_back(sweepMode);
          } // el
        } // istride
      } // az

    } else if (scan.sim_type == Params::PPI_SIM) {

      Radx::SweepMode_t sweepMode = Radx::SWEEP_MODE_SECTOR;
      
      for (double el = scan.min_el;
           el <= scan.max_el;
           el += scan.delta_el) {
        for (int istride = 0; istride < beamsPerDwell; istride++) {
          for (double az = scan.min_az + istride * scan.delta_az;
               az <= scan.max_az; az += beamsPerDwell * scan.delta_az) {
            _simEl.push_back(el);
            _simAz.push_back(az);
            _simSweepNum.push_back(sweepNum);
            _simSweepMode.push_back(sweepMode);
          } // az
        } // istride
      } // el
      
    } // if (scan.sim_type == Params::RHI_SIM)
    
    sweepNum++;
    
  } // iscan

}
