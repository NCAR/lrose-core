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
// GemVolXml2Dsr.cc
//
// GemVolXml2Dsr object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2006
//
///////////////////////////////////////////////////////////////
//
// GemVolXml2Dsr reads Gematronik SCAN-format radar volume file
// and reformats the contents into a DsRadar FMQ.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <toolsa/os_config.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include "GemVolXml2Dsr.hh"
using namespace std;

// Constructor

GemVolXml2Dsr::GemVolXml2Dsr(int argc, char **argv)

{

  _input = NULL;
  isOK = true;
  _currentTime = 0;
  _currentNn = -1;
  _volNum = 0;

  // set programe name

  _progName = "GemVolXml2Dsr";
  ucopyright((char *) _progName.c_str());

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
  }

  // check that file list set in archive and simulate mode
  
  if (_params.mode == Params::ARCHIVE && _args.inputFileList.size() == 0) {
    cerr << "ERROR: GemVolXml2Dsr::GemVolXml2Dsr." << endl;
    cerr << "  Mode is ARCHIVE."; 
    cerr << "  You must use -f to specify files on the command line."
	 << endl;
    _args.usage(_progName, cerr);
    isOK = false;
  }
    
  if (_params.mode == Params::SIMULATE && _args.inputFileList.size() == 0) {
    cerr << "ERROR: GemVolXml2Dsr::GemVolXml2Dsr." << endl;
    cerr << "  Mode is SIMULATE."; 
    cerr << "  You must use -f to specify files on the command line."
	 << endl;
    _args.usage(_progName, cerr);
    isOK = false;
  }
    
  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  // initialize the data input object
  
  if (_params.mode == Params::REALTIME) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _params.max_realtime_valid_age,
			     PMU_auto_register,
			     _params.use_ldata_info_file,
                             false);
    _input->setDirScanSleep(_params.dir_scan_interval_secs);
    _input->setFileQuiescence(_params.input_file_quiescence_secs);
  } else {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _args.inputFileList);
  }

  // set up input file objects

  for (int ii = 0; ii < _params.fields_n; ii++) {
    InputFile *input = new InputFile(_params,
                                     _params._fields[ii].input_field_name,
                                     _params._fields[ii].output_field_name,
                                     _params._fields[ii].output_units,
                                     _params._fields[ii].required);
    _inputFiles.push_back(input);
  }
  if (_inputFiles.size() < 1) {
    cerr << "ERROR - GemVolXml2Dsr" << endl;
    cerr << "  Must have at least 1 field" << endl;
    isOK = false;
    return;
  }

  // initialize the output queue
  
  if (_rQueue.init(_params.output_fmq_url,
		   _progName.c_str(),
		   _params.debug >= Params::DEBUG_VERBOSE,
		   DsFmq::READ_WRITE, DsFmq::END,
		   _params.output_fmq_compress,
		   _params.output_fmq_nslots,
		   _params.output_fmq_size)) {
    cerr << "ERROR - GemVolXml2Dsr" << endl;
    cerr << "  Cannot open fmq, URL: " << _params.output_fmq_url << endl;
    isOK = false;
    return;
  }

  if (_params.output_fmq_compress) {
    _rQueue.setCompressionMethod(TA_COMPRESSION_ZLIB);
  }

  if (_params.write_blocking) {
    _rQueue.setBlockingWrite();
  }

  return;

}

// destructor

GemVolXml2Dsr::~GemVolXml2Dsr()

{

  if (_input) {
    delete _input;
  }

  for (int ii = 0; ii < (int) _inputFiles.size(); ii++) {
    delete _inputFiles[ii];
  }
  _inputFiles.clear();

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int GemVolXml2Dsr::Run ()
{

  int iret = 0;
  PMU_auto_register("Run");
  
  if (_params.mode == Params::SIMULATE) {
    
    // simulate mode - go through the file list repeatedly
    
    while (true) {
      
      char *inputPath;
      _input->reset();
      while ((inputPath = _input->next()) != NULL) {
	PMU_auto_register("Simulate mode");
	if (_processFile(inputPath)) {
	  iret = -1;
	}
      } // while
      
    }

  } else {
    
    // loop until end of data
    
    char *inputPath;
    while ((inputPath = _input->next()) != NULL) {
      PMU_auto_register("Processing file");
      if (_processFile(inputPath)) {
	iret = -1;
      }
    }
      
  } // if (_params.mode == Params::SIMULATE)

  return iret;

}

//////////////////////////////////////////////////
// process file

int GemVolXml2Dsr::_processFile(const char *inputPath)
  
{

  if (_params.debug) {
    cerr << "Processing file: " << inputPath << endl;
  }

  // decode the file time from the path

  Path path(inputPath);
  string fileName = path.getFile();
  time_t fileTime;
  string inputFieldName;
  int nn;

  if (_parseFileName(fileName, fileTime, nn, inputFieldName)) {
    cerr << "ERROR - GemVolXml2Dsr::_processFile" << endl;
    cerr << "  Cannot decode time from file name: " << fileName << endl;
    return -1;
  }

  // if (_params.debug >= Params::DEBUG_VERBOSE) {
  if (_params.debug) {
    cerr << "---->> FileName: " << fileName << endl;
    cerr << "  File time: " << DateTime::strm(fileTime) << endl;
    cerr << "  nn: " << nn << endl;
    cerr << "  inputFieldName: " << inputFieldName << endl;
  }

  if (_currentNn < 0) {

    _currentTime = fileTime;
    _currentNn = nn;
    
  } else {

    if (_currentTime != fileTime || _currentNn != nn) {
      _processCurrent();
      _currentTime = fileTime;
      _currentNn = nn;
    }

  }

  // read file if wanted

  for (int ii = 0; ii < (int) _inputFiles.size(); ii++) {

    if (inputFieldName == _inputFiles[ii]->getInputFieldName()) {

      if (_inputFiles[ii]->read(path.getPath(),
                                fileName,
                                fileTime,
                                nn)) {
        cerr << "WARNING - GemVolXml2Dsr::_processFile" << endl;
        cerr << "  Cannot read file: " << path.getPath() << endl;
      }
      
    }

  }

  // check if we are ready to write out

  if (_allFieldsPresent()) {
    _processCurrent();
    _currentTime = 0;
    _currentNn = -1;
  }

  return 0;

}

/////////////////////////////////////////////////////
/// set the file time from the file name
/// returns 0 on success, -1 on failure

int GemVolXml2Dsr::_parseFileName(const string &fileName,
                                  time_t &fileTime,
                                  int &num,
                                  string &fieldName)
  
{

  // find first digit in entry name - if no digits, return now
  
  const char *start = NULL;
  for (size_t ii = 0; ii < fileName.size(); ii++) {
    if (isdigit(fileName[ii])) {
      start = fileName.c_str() + ii;
      break;
    }
  }
  if (!start) return -1;
  const char *end = start + strlen(start);
  
  // iteratively try getting the date and time from the string
  // moving along by one character at a time
  
  while (start < end - 6) {
    int year, month, day, hour, min, sec, nn;
    if (sscanf(start, "%4d%2d%2d%2d%2d%2d%2d",
               &year, &month, &day, &hour, &min, &sec, &nn) == 7) {
      if (year < 1900 || month < 1 || month > 12 || day < 1 || day > 31) {
        return -1;
      }
      if (hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59) {
        return -1;
      }
      DateTime ftime(year, month, day, hour, min, sec);
      fileTime = ftime.utime();
      num = nn;
      string ending(start + 16);
      size_t dotPos = ending.find('.');
      if (dotPos == string::npos) {
        cerr << "ERROR - GemVolXml2Dsr::_parseFileName" << endl;
        cerr << "  Cannot decode field name from file name: " << fileName << endl;
        return -1;
      }
      fieldName = ending.substr(0, dotPos);
      return 0;
    }
    start++;
  }
  
  return -1;



  
}

//////////////////////////////////////////////////
// process the data currently available

int GemVolXml2Dsr::_processCurrent()
  
{

  // fo we have all of the data
  
  int iret = 0;

  if (_allFieldsPresent()) {
    if (_writeOut()) {
      iret = -1;
    }
  } else {
    if (_params.debug) {
      cerr << "Not all fields present, cannot write output data" << endl;
    }
    iret = -1;
  }
  
  // clear info on files

  for (int ii = 0; ii < (int) _inputFiles.size(); ii++) {
    _inputFiles[ii]->clear();
  }

  // in simulate mode, sleep a while

  if (_params.mode == Params::SIMULATE) {
    for (int ii = 0; ii < _params.simulate_sleep_secs; ii++) {
      PMU_auto_register("Zzzz ...");
      umsleep(1000);
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// write out

int GemVolXml2Dsr::_writeOut()
  
{
  
  if (_params.debug) {
    cerr << "======= Writing out ==========" << endl;
  }

  // compute number of tilts

  if (_computeNTilts()) {
    cerr << "ERROR - GemVolXml2Dsr::_writeOut" << endl;
    cerr << "  Number of tilts varies from file to file" << endl;
    cerr << "  Cannot write output data" << endl;
    return -1;
  }
  
  // start of volume flag
  
  time_t volStartTime = _inputFiles[0]->getScanTime() + _params.input_file_time_offset_secs;
  time_t volEndTime = _inputFiles[0]->getVolTime() + _params.input_file_time_offset_secs;
  time_t prevTiltStartTime = volStartTime;

  if (_params.mode == Params::SIMULATE) {
    time_t now = time(NULL);
    volStartTime = now;
    volEndTime = now;
    prevTiltStartTime = now;
  }

  _rQueue.putStartOfVolume(_volNum, volStartTime);

  // compute the max byte width - this will be used for all fields

  _outputByteWidth = 1;

  for (int ifile = 0; ifile < (int) _inputFiles.size(); ifile++) {
    if ((int) _inputFiles[ifile]->getTilts().size() < _nTilts) {
      cerr << "ERROR - not enough tilts in file, field: "
           << _inputFiles[ifile]->getInputFieldName() << endl;
      continue;
    }
    for (int itilt = 0; itilt < _nTilts; itilt++) {
      const Tilt &field = *(_inputFiles[ifile]->getTilts()[itilt]);
      int byteWidth = field.getDataByteWidth();
      if (byteWidth > _outputByteWidth) {
        _outputByteWidth = byteWidth;
      }
    } // itilt
  } // ifile

  // loop through tilts
  
  for (int itilt = 0; itilt < _nTilts; itilt++) {
    
    const Tilt &tilt = *(_inputFiles[0]->getTilts()[itilt]);

    // times

    time_t tiltStartTime = tilt.getStartTime();
    time_t tiltEndTime = volEndTime;
    if (_params.mode == Params::SIMULATE) {
      tiltEndTime = volEndTime;
      tiltStartTime = volStartTime;
    }
    if (itilt != _nTilts - 1) {
      tiltEndTime = prevTiltStartTime;
    }
    prevTiltStartTime = tiltStartTime;

    // compute tilt parameters
    
    if (_computeTiltParams(itilt) == 0) {

      // start of tilt flag
      
      _rQueue.putStartOfTilt(itilt, tiltStartTime);
      
      // radar and field params
      
      if (_writeParams(itilt)) {
        cerr << "ERROR - File2Fmq::write" << endl;
        cerr << "  Cannot write tilt params to the output queue" << endl;
        cerr << "  Tilt num: " << itilt << endl;
        cerr << "  URL: " << _params.output_fmq_url << endl;
        return -1;
      }
      
      // beams
      if (_writeBeams(itilt, tiltStartTime, tiltEndTime)) {
        cerr << "ERROR - File2Fmq::write" << endl;
        cerr << "  Cannot write the beams to queue" << endl;
        cerr << "  Tilt num: " << itilt << endl;
        cerr << "  URL: " << _params.output_fmq_url << endl;
        return -1;
      }
    
      // end of tilt flag
    
      _rQueue.putEndOfTilt(itilt, tiltEndTime);

    } // if (_computeTiltParams ...

  } // itilt
  
  // end of volume flag

  _rQueue.putEndOfVolume(_volNum, volEndTime);

  _volNum++;

  return 0;

}

////////////////////////////////////////
// Write radar and field params to queue
//
// Returns 0 on success, -1 on failure

int GemVolXml2Dsr::_writeParams(int tiltNum)

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "writing params" << endl;
  }

  const Tilt &field0 = *(_inputFiles[0]->getTilts()[tiltNum]);

  // Set radar parameters
  
  DsRadarMsg msg;
  DsRadarParams &rp = msg.getRadarParams();

  // first get params from the parameter file

  rp.radarId = 0;
  rp.radarType = DS_RADAR_GROUND_TYPE;
  rp.numGates = _nGates;
  rp.samplesPerBeam = field0.getNSamples();
  rp.scanType = _params.scan_type_id;
  rp.scanMode = DS_RADAR_SURVEILLANCE_MODE;
  const string &polarStr = _inputFiles[0]->getPolarization();
  if (polarStr.find("Dual") != string::npos) {
    rp.polarization = DS_POLARIZATION_DUAL_TYPE;
  } else if (polarStr.find("Vert") != string::npos) {
    rp.polarization = DS_POLARIZATION_VERT_TYPE;
  } else {
    rp.polarization = DS_POLARIZATION_HORIZ_TYPE;
  }
  
  rp.altitude = _inputFiles[0]->getRadarAlt() / 1000.0;
  rp.latitude = _inputFiles[0]->getRadarLat();
  rp.longitude = _inputFiles[0]->getRadarLon();
  if (_params.override_radar_location) {
    rp.altitude = _params.radar_altitude;
    rp.latitude = _params.radar_latitude;
    rp.longitude = _params.radar_longitude;
  }
  
  rp.gateSpacing = field0.getGateSpacing();
  rp.startRange = field0.getStartRange();
  rp.horizBeamWidth = _inputFiles[0]->getRadarBeamwidth();
  rp.vertBeamWidth = _inputFiles[0]->getRadarBeamwidth();
  
  const string &pulseWidthStr = _inputFiles[0]->getPulseWidth();
  if (pulseWidthStr.find("Short") != string::npos) {
    rp.pulseWidth = _params.short_pulse_width_us;
  } else {
    rp.pulseWidth = _params.long_pulse_width_us;
  }
  rp.pulseRepFreq = field0.getPrf();
  rp.wavelength = _inputFiles[0]->getRadarWavelength() * 100.0;
  
  rp.xmitPeakPower = field0.getXmitPeakPowerKw();
  rp.radarConstant = field0.getRadarConst();
  rp.receiverMds = field0.getNoisePowerDbz() - rp.radarConstant;

  rp.receiverGain = _params.receiver_gain;
  rp.antennaGain = _params.antenna_gain;
  rp.systemGain = _params.system_gain;
  
  rp.unambigVelocity = ((rp.wavelength / 100.0) * rp.pulseRepFreq) / 4.0;
  rp.unambigRange = 150000.0 / rp.pulseRepFreq;
  
  rp.radarName = _inputFiles[0]->getRadarName();
  rp.scanTypeName = _params.scan_type_name;
  
  // Add field parameters to the message

  vector<DsFieldParams*> &fp = msg.getFieldParams();

  _nFields = 0;
  for (int ii = 0; ii < (int) _inputFiles.size(); ii++) {

    if (!_inputFiles[ii]->getFound()) {
      continue;
    }
    _nFields++;

    const Tilt &field = *(_inputFiles[ii]->getTilts()[tiltNum]);

    double minVal = field.getMinValue();
    double maxVal = field.getMaxValue();
    double dataRange = maxVal - minVal;
    int byteWidth = field.getDataByteWidth();
    double scale = dataRange / 254;
    if (byteWidth > 1) {
      scale = dataRange / 65534;
    }
    double bias = minVal - scale;
    
    DsFieldParams* thisParams =
      new DsFieldParams(_inputFiles[ii]->getOutputFieldName().c_str(),
                        _inputFiles[ii]->getOutputUnits().c_str(),
                        scale, bias, _outputByteWidth);
    fp.push_back(thisParams);

  }

  rp.numFields = _nFields;
  
  // write the message
  
  if (_rQueue.putDsMsg
      (msg,
       DsRadarMsg::RADAR_PARAMS | DsRadarMsg::FIELD_PARAMS)) {
    cerr << "ERROR - GemVolXml2Dsr::writeParams" << endl;
    cerr << "  Cannot put params to queue" << endl;
    return -1;
  }
  
  return 0;

}

////////////////////////////////////////
// Write beam data to queue
//
// Returns 0 on success, -1 on failure

int GemVolXml2Dsr::_writeBeams(int tiltNum,
                               time_t startTime,
                               time_t endTime)

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "writing beams" << endl;
  }

  int iret = 0;

  DsRadarMsg msg;
  DsRadarBeam &beam = msg.getRadarBeam();

  const Tilt &field0 = *(_inputFiles[0]->getTilts()[tiltNum]);
  const vector<double> &azAngles = field0.getAzAngles();

  double deltaTime = ((double) endTime - (double) startTime) / _nAz;

  // loop through the beams

  for (int iaz = 0; iaz < _nAz; iaz++) {

    // params
    
    beam.dataTime = startTime + (int) (iaz * deltaTime + 0.5);
    beam.volumeNum = _volNum;
    beam.tiltNum = tiltNum;
    beam.azimuth = azAngles[iaz];
    beam.elevation = field0.getElev();
    beam.targetElev = beam.elevation;
    
    // compose data

    if (_outputByteWidth == 1) {

      // all fields are single byte
    
      int nBytes = _nFields * _nGates * _outputByteWidth;
      ui08 *beamData = new ui08[nBytes];
      
      for (int ifld = 0; ifld < (int) _inputFiles.size(); ifld++) {
        if (!_inputFiles[ifld]->getFound()) {
          continue;
        }
        const Tilt &field = *(_inputFiles[ifld]->getTilts()[tiltNum]);
        const ui08 *fldData = field.getFieldData();
        const ui08 *fd = fldData + (iaz * _nGates);
        ui08 *bd = beamData + ifld;
        for (int igate = 0; igate < _nGates; igate++, bd += _nFields, fd++) {
          *bd = *fd;
        }
      } // ifld

      // load into output beam

      beam.loadData(beamData, nBytes, _outputByteWidth);
      delete[] beamData;

    } else {

      // some fields are 2 bytes, map all fields into 2 bytes

      int nBytes = _nFields * _nGates * _outputByteWidth;
      ui16 *beamData = new ui16[nBytes];
      
      for (int ifld = 0; ifld < (int) _inputFiles.size(); ifld++) {

        if (!_inputFiles[ifld]->getFound()) {
          continue;
        }
        const Tilt &field = *(_inputFiles[ifld]->getTilts()[tiltNum]);
        int inputByteWidth = field.getDataByteWidth();
        
        if (inputByteWidth == 1) {
          const ui08 *fldData = field.getFieldData();
          const ui08 *fd = fldData + (iaz * _nGates);
          ui16 *bd = beamData + ifld;
          for (int igate = 0; igate < _nGates; igate++, bd += _nFields, fd++) {
            *bd = *fd;
          }
        } else {
          // byte width 2
          const ui16 *fldData = (ui16 *) field.getFieldData();
          const ui16 *fd = fldData + (iaz * _nGates);
          ui16 *bd = beamData + ifld;
          for (int igate = 0; igate < _nGates; igate++, bd += _nFields, fd++) {
            *bd = *fd;
          }
        } // iniputByteWidth
        
      } // ifld

      // load into output beam

      beam.loadData(beamData, nBytes, _outputByteWidth);
      delete[] beamData;

    } // if (_outputByteWidth == 1) {

    // write the message
    
    if (_rQueue.putDsMsg(msg, DsRadarMsg::RADAR_BEAM)) {
      iret = -1;
    }
    
    if (_params.mode != Params::REALTIME && _params.beam_wait_msecs > 0) {
      umsleep(_params.beam_wait_msecs);
    }
    
  } // iaz

  if (iret) {
    cerr << "ERROR - File2Fmq::writeBeams" << endl;
    cerr << "  Cannot put radar beams to queue" << endl;
  }
  
  return iret;

}


//////////////////////////////////////////////////
// do we have all input fields

bool GemVolXml2Dsr::_allFieldsPresent()
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "========= Found files ==========" << endl;
    for (int ii = 0; ii < (int) _inputFiles.size(); ii++) {
      cerr << "File num: " << ii << endl;
      _inputFiles[ii]->print(cerr);
      cerr << "-----------------" << endl;
    }
  }
  
  bool success = true;
  for (int ii = 0; ii < (int) _inputFiles.size(); ii++) {
    if (!_inputFiles[ii]->getFound() && _inputFiles[ii]->getRequired()) {
      if (_params.debug) {
 	cerr << "  NOTE - waiting on file for field: "
 	     << _inputFiles[ii]->getInputFieldName() << endl;
      }
      success = false;
    }
  }

  return success;

}

//////////////////////////////////////////////////
// compute number of tilts
// returns 0 on success, -1 on failure

int GemVolXml2Dsr::_computeNTilts()
  
{
  _nTilts = _inputFiles[0]->getNTilts();
  for (int ii = 1; ii < (int) _inputFiles.size(); ii++) {
    if (_inputFiles[ii]->getFound()) {
      if (_inputFiles[ii]->getNTilts() != _nTilts) {
        return -1;
      }
    }
  }
  return 0;
}

//////////////////////////////////////////////////
// compute tilt parameters
// returns 0 on success, -1 on failure

int GemVolXml2Dsr::_computeTiltParams(int tiltNum)
  
{

  const Tilt &field0 = *(_inputFiles[0]->getTilts()[tiltNum]);
  
  _nAz = field0.getNAz();
  _nGates = field0.getNGates();
  _elev = field0.getElev();

  for (int ii = 1; ii < (int) _inputFiles.size(); ii++) {
    if (_inputFiles[ii]->getFound()) {
      const Tilt &field = *(_inputFiles[ii]->getTilts()[tiltNum]);
      if (_nAz != field.getNAz() ||
          _nGates != field.getNGates()) {
        return -1;
      }
    }
  }

  return 0;

}



