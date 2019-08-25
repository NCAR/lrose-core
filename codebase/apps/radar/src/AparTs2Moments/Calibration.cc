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
///////////////////////////////////////////////////////////
// Calibration.cc
// 
// read and store calibration data
//
// EOL, NCAR, Boulder CO
//
// August 2010
//
// Mike Dixon
//
///////////////////////////////////////////////////////////

#include <cerrno>
#include <cmath>
#include <cassert>
#include <toolsa/file_io.h>
#include <toolsa/DateTime.hh>
#include <toolsa/ReadDir.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/TaArray.hh>
#include <radar/AparTsInfo.hh>
#include "Calibration.hh"

//////////////////
// Constructor

Calibration::Calibration(const Params &params) :
        _params(params)
{

  // initialize
  
  _calAvailable = false;
  _prevTimeRequested = 0;
  _prevPulseWidth = -1.0;
  _radarName = "unknown";
  _calTime = 0;

}

//////////////////
// Destructor


Calibration::~Calibration()
{
}

//////////////////////////////////////////////////////////
// Read calibration for a given file path.
// Returns 0 on success, -1 on failure

int Calibration::readCal(const string &filePath)

{

  // read from file

  if (_readCalFromFile(filePath)) {
    return -1;
  }

  // debug print

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _calib.print(cerr);
  }

  return 0;

}

//////////////////////////////////////////////////////////
// Load calibration appropriate to a given beam
// Returns 0 on success, -1 on failure

int Calibration::loadCal(const Beam *beam)
  
{
  
  if (_params.use_cal_from_time_series) {
    
    _setCalFromTimeSeries(beam);
    
  } else {
    
    // read calibration for this pulse width, if appropriate
    
    if (_params.set_cal_by_pulse_width) {
      
      if (_params.pulse_width_cals_n < 1) {
        cerr << "WARNING - Calibration::loadCal" << endl;
        cerr << "  No calibration directories specified for pulse width." << endl;
        cerr << "  Set parameter 'set_cal_by_pulse_width = false'" << endl;
        return -1;
      }
      
      if (_checkPulseWidthAndRead(beam)) {
        return -1;
      }
      
    }
    
  }
  
  return 0;

}

/////////////////////////////////////////////////
// Set the calibration from the ops info
// that the baseDbz1km values are set

void Calibration::_setCalFromTimeSeries(const Beam *beam)
  
{
  
  const AparTsInfo &tsInfo = beam->getOpsInfo();

  tsInfo.setAparTsCalib(_calib);
    
  // apply corrections as appropriate

  _applyCalCorrections();

}


//////////////////////////////////////////////////////////
// Read calibration for a specific pulse width,
// appropriate to a given beam.
// Returns 0 on success, -1 on failure

int Calibration::_checkPulseWidthAndRead(const Beam *beam)

{

  // check for change in pulse width
  
  double beamPulseWidthUs = beam->getPulseWidth() * 1.0e6;
  
  if (fabs(beamPulseWidthUs - _prevPulseWidth) < 0.000001) {
    // don't need to read
    return 0;
  }

  // pulse width has changed - reset
  
  _calAvailable = false;
  _prevTimeRequested = 0;
  _calTime = 0;
  
  // set directory for pulse width
  
  double minDiff = 1.0e99;
  int index = 0;
  _calDirForPulseWidth = _params._pulse_width_cals[0].cal_dir;
  for (int ii = 0; ii < _params.pulse_width_cals_n; ii++) {
    
    Params::pulse_width_cal_t entry = _params._pulse_width_cals[ii];

    double pwidth = entry.pulse_width_us;
    double diff = fabs(pwidth - beamPulseWidthUs);
    if (diff < minDiff) {
      _calDirForPulseWidth = entry.cal_dir;
      minDiff = diff;
      index = ii;
    }
    
  } // ii
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Reading new cal, pulse width (us): " << beamPulseWidthUs << endl;
    cerr << "  Cal dir: " << _calDirForPulseWidth << endl;
  }
  
  _prevPulseWidth = beamPulseWidthUs;

  // read the calibration

  Params::pulse_width_cal_t entry = _params._pulse_width_cals[index]; 
  if (_readCal(beam->getTimeSecs(), _calDirForPulseWidth)) {
    return -1;
  }
  
  // override from entry as required
  
  if (_params.override_cal_dbz_correction) {
    _calib.setDbzCorrection(_params.dbz_correction);
  }

  if (_params.override_cal_zdr_correction) {
    if (entry.zdr_correction_db > -9990) {
      _calib.setZdrCorrectionDb(entry.zdr_correction_db);
    }
  }
  
  if (_params.override_cal_system_phidp) {
    if (entry.system_phidp_deg > -9990) {
      _calib.setSystemPhidpDeg(entry.system_phidp_deg);
    }
  }
    
  return 0;

}


////////////////////////////////////////////////////////////////////////
// search for and read a cal file, given the time

int Calibration::_readCal(time_t utime, const string &calDir)

{

  if (utime == 0) {
    if (_params.debug) {
      cerr << "WARNING - Calibration::_readCal" << endl;
      cerr << "  Cannot read cal files, data time still 0" << endl;
    }
    return -1;
  }

  // compile file list

  FileMap fileMap;
  int iret = _compileFileList(calDir, fileMap);
  if (iret && _params.debug) {
    cerr << "WARNING - Calibration::_readCal" << endl;
    cerr << "  Cannot read cal files" << endl;
  }
  if (!_calAvailable && fileMap.size() == 0) {
    cerr << "ERROR - Calibration::_readCal" << endl;
    cerr << "  No calibration files available" << endl;
    return -1;
  }

  // find best file

  double minDiff = 1.0e99;
  string bestPath = "";
  time_t bestTime = 0;
  for (FileMap::iterator it = fileMap.begin(); it != fileMap.end(); it++) {
    const FilePair &filePair = *it;
    double fileTime = (double) filePair.first;
    double diff = fabs((double) utime - fileTime);
    if (diff < minDiff) {
      bestTime = filePair.first;
      const string &path = filePair.second;
      bestPath = path;
      minDiff = diff;
    }
  } // it

  if (bestPath.size() == 0 && _params.debug) {
    cerr << "WARNING - Calibration::_readCal" << endl;
    cerr << "  No cal files available" << endl;
    return -1;
  }
  
  // print out cal file list

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Data time: " << DateTime::strm(utime) << endl;
    cerr << "Reading in calibration - available files: " << endl;
    for (FileMap::iterator it = fileMap.begin(); it != fileMap.end(); it++) {
      const FilePair &filePair = *it;
      cerr << "  cal path: " << filePair.second << endl;
      cerr << "      time: " << DateTime::strm(filePair.first) << endl;
    } // it
    cerr << "Best path: " << bestPath << ", time diff (days): " << minDiff / 86400.0 << endl;
  }

  bool success = false;

  if (bestTime != _calTime) {

    if (_readCalFromFile(bestPath) == 0) {

      _calTime = bestTime;
      success = true;
      
    } else {
      
      cerr << "WARNING - Calibration::_readCal" << endl;
      cerr << "  Cannot read best file: " << bestPath << endl;
      
      // try the files in reverse order
      
      for (FileMap::reverse_iterator it = fileMap.rbegin();
	   it != fileMap.rend(); it++) {
	const FilePair &filePair = *it;
	const string &path = filePair.second;
	if (_params.debug) {
	  cerr << "  Trying cal file: " << path << endl;
	}
	if (_readCalFromFile(path) == 0) {
	  if (_params.debug) {
	    cerr << "SUCCESS - Calibration::_readCal" << endl;
	    cerr << "  Read this file instead: " << path << endl;
	  }
	  success = true;
	  continue;
	}
      } // it
    }
      
  } else {

    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "Skipping cal file - " << bestPath 
	   << " - File utime same as previous cal" << endl;
    }

  } // if (bestTime != _calTime)

  if (success) {

    if (_params.debug) {
      cerr << "Read calibration, time: " << DateTime::strm(_calTime) << endl;
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        _calib.print(cerr);
      }
    }

    return 0;

  } else {

    if (bestTime != _calTime) {
      cerr << "ERROR - Calibration::_readCal" << endl;
      cerr << "  Could not find new valid param file" << endl;
    }
    return -1;

  }

}

////////////////////////////////////////////////////////////////////////
// read a given cal file

int Calibration::_readCalFromFile(const string &calPath)

{

  string errStr;
  if (_calib.readFromXmlFile(calPath, errStr)) {
    cerr << "ERROR - Calibration::_readCalFromFile" << endl;
    cerr << "  Cannot decode cal file: " << calPath << endl;
    cerr << errStr;
    _calAvailable = false;
    return -1;
  }
  
  _calFilePath = calPath;
  _calAvailable = true;

  // apply corrections as appropriate

  _applyCalCorrections();

  if (_params.debug) {
    cerr << "Done reading calibration file: " << calPath << endl;
  }

  return 0;

}

////////////////////////
// _checkDirForParams()
//
// Recurse down the directory tree, checking for existence
// of _DsFileDist files
//
// The findLdataInfo argument indicates whether we are looking
// for (a) _DsFileDist params files (false) or
//     (b) _latest_data_info files (true)
//
// Returns 0 on success, -1 on failure.

int Calibration::_compileFileList(const string &dirPath,
                                  FileMap &fileMap)
  
{
  
  // Try to open the directory
  
  ReadDir rdir;
  if (rdir.open(dirPath.c_str())) {
    cerr << "ERROR - " << "Calibration::_compileFileList." << endl;
    cerr << "  Cannot open dirPath: \'" << dirPath << "\'" << endl;
    cerr << "  " << strerror(errno) << endl;
    return (-1);
  }

  // Loop thru directory looking for sub-directories
  
  struct dirent *dp;
  for (dp = rdir.read(); dp != NULL; dp = rdir.read()) {
    
    // exclude the '.' and '..' entries
    
    if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) {
      continue;
    }

    // stat the file
    
    string path = dirPath;
    path += PATH_DELIM;
    path += dp->d_name;
    struct stat fileStat;

    if (stat(path.c_str(), &fileStat)) {
      // file has disappeared, so ignore
      continue;
    }

    // is this an XML file? If not, skip

    if (strstr(dp->d_name, ".xml") == NULL) {
      continue;
    }
    
    // check for directory
    
    if (S_ISDIR(fileStat.st_mode)) {

      // this is a directory, so recurse into it
      
      _compileFileList(path, fileMap);
      
    } else {
      
      // read the file, get the time from it

      AparTsCalib cal;
      string errStr;
      if (cal.readFromXmlFile(path, errStr) == 0) {
        // add to map
        FilePair filePair(cal.getCalibTime(), path);
        fileMap.insert(filePair);
      }

    } // if (S_ISDIR ...
    
  } // dp
    
  rdir.close();
  
  return 0;

}

////////////////////////////////////////////////////////////////
// apply calibration corrections

void Calibration::_applyCalCorrections()
  
{

  
  if (_params.override_cal_dbz_correction) {
    _calib.setDbzCorrection(_params.dbz_correction);
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "Calibration::_applyCalCorrections()" << endl;
      cerr << "  setting dbz_correction: " << _params.dbz_correction << endl;
    }
  }
  
  if (_params.override_cal_zdr_correction) {
    _calib.setZdrCorrectionDb(_params.zdr_correction_db);
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "Calibration::_applyCalCorrections()" << endl;
      cerr << "  setting zdr_correction_db: " << _params.zdr_correction_db << endl;
    }
  }
  
  if (_params.override_cal_ldr_corrections) {
    _calib.setLdrCorrectionDbH(_params.ldr_correction_db_h);
    _calib.setLdrCorrectionDbV(_params.ldr_correction_db_v);
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "Calibration::_applyCalCorrections()" << endl;
      cerr << "  setting ldr_correction_db_h: " << _params.ldr_correction_db_h << endl;
      cerr << "  setting ldr_correction_db_v: " << _params.ldr_correction_db_v << endl;
    }
  }
  
  if (_params.override_cal_system_phidp) {
    _calib.setSystemPhidpDeg(_params.system_phidp_deg);
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "Calibration::_applyCalCorrections()" << endl;
      cerr << "  setting system_phidp_deg: " << _params.system_phidp_deg << endl;
    }
  }

}

