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
// CalibMgr.cc
// 
// Manages calibration data for different pulse widths
//
// EOL, NCAR, Boulder CO
//
// July 2026
//
// Mike Dixon
//
/////////////////////////////////////////////////////////////

#include <cerrno>
#include <cmath>
#include <cassert>
#include <toolsa/file_io.h>
#include <toolsa/DateTime.hh>
#include <toolsa/ReadDir.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/TaArray.hh>
#include <radar/IwrfTsInfo.hh>
#include <Spdb/DsSpdb.hh>
#include "CalibMgr.hh"

//////////////////
// Constructor

CalibMgr::CalibMgr(const Params &params) :
        _params(params)
{
  
}

//////////////////
// Destructor


CalibMgr::~CalibMgr()
{

}

///////////////////////////////////////////////////////////////
// Read calibrations for pulse widths specified in params file
// Returns 0 on success, -1 on failure

int CalibMgr::readCals(time_t dataStartTime)

{

  for (int ii = 0; ii < _params.pulse_width_cals_n; ii++) {

    // get the params
    
    const Params::pulse_width_cal_t &calParams = _params._pulse_width_cals[ii];
    double pulseWidthUs = calParams.pulse_width_us;

    // create cal object
    
    Calibration *cal = new Calibration(_params, pulseWidthUs);
    _cals.push_back(cal);

    // determine cal file name
    
    string calPath;
    if (calParams.specify_file_name) {
      calPath = calParams.cal_dir;
      calPath += PATH_DELIM;
      calPath += calParams.cal_file_name;
    } else {
      if (_getBestFilePath(calParams.cal_dir, dataStartTime, calPath)) {
        cerr << "Cannot find calibration for pulse width (us): "
             << pulseWidthUs << endl;
        return -1;
      }
    }

    // read the file
    
    if (cal->readCal(calPath)) {
      cerr << "Cannot read calibration for pulse width (us): " << pulseWidthUs << endl;
      cerr << "  Cal file path: " << calPath << endl;
      return -1;
    }
    
    // debug print
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "======================================================" << endl;
      cerr << "Read in cal for pulse width: " << pulseWidthUs << endl;
      cal->getIwrfCalib().print(cerr);
      cerr << "======================================================" << endl;
    }

  } // ii
  
  return 0;

}

//////////////////////////////////////////////////////////
// get cal for pulse width

const IwrfCalib &CalibMgr::getIwrfCalib(double pulseWidthUs) const
{

  double minDiff = 1.0e6;
  double bestIndex = 0;
  
  for (size_t ii = 0; ii < _cals.size(); ii++) {

    // get cal closest to the desired pulse width
    
    const Calibration *cal = _cals[ii];
    double calPulseWidthUs = cal->getPulseWidthUs();
    double diff = fabs(pulseWidthUs - calPulseWidthUs);
    if (diff < minDiff) {
      bestIndex = ii;
      minDiff = diff;
    }
    
  } // ii

  return _cals[bestIndex]->getIwrfCalib();

}

////////////////////////////////////////////////////////////////////////
// get the path for the best cal file

int CalibMgr::_getBestFilePath(const string &calDir,
                               time_t dataTime,
                               string &calPath)

{

  // compile file list
  
  FileMap fileMap;
  if (_compileFileList(calDir, fileMap)) {
    cerr << "ERROR - CalibMgr::_getBestFilePath" << endl;
    cerr << "  Cannot find files in dir: " << calDir << endl;
    return -1;
  }
  if (fileMap.size() == 0) {
    cerr << "ERROR - CalibMgr::_getBestFilePath" << endl;
    cerr << "  Cannot find files in dir: " << calDir << endl;
    return -1;
  }

  // find best file

  double minDiff = 1.0e99;
  string bestPath = "";
  time_t bestTime = 0;
  for (FileMap::iterator it = fileMap.begin(); it != fileMap.end(); it++) {
    const FilePair &filePair = *it;
    double fileTime = (double) filePair.first;
    double diff = fabs((double) dataTime - fileTime);
    if (diff < minDiff) {
      bestTime = filePair.first;
      bestPath = filePair.second;
      minDiff = diff;
    }
  } // it
  
  if (bestPath.size() == 0) {
    cerr << "ERROR - CalibMgr::_getBestFilePath" << endl;
    cerr << "  Cal dir: " << calDir << endl;
    cerr << "  No cal files available" << endl;
    cerr << "  dataTime: " << utimstr(dataTime) << endl;
    return -1;
  }
  
  calPath = bestPath;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "======================================================" << endl;
    cerr << "Found cal file: " << bestPath << endl;
    cerr << "      cal time: " << DateTime::strm(bestTime) << endl;
    cerr << "======================================================" << endl;
  }

  return 0;

}

////////////////////////////////////////////////////////////////////
// Compile list of files available in dir
//
// Returns 0 on success, -1 on failure.

int CalibMgr::_compileFileList(const string &dirPath,
                               FileMap &fileMap)
  
{
  
  // Try to open the directory
  
  ReadDir rdir;
  if (rdir.open(dirPath.c_str())) {
    cerr << "ERROR - " << "CalibMgr::_compileFileList" << endl;
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
    
    // read the file, get the time from it
    
    IwrfCalib cal;
    string errStr;
    if (cal.readFromXmlFile(path, errStr) == 0) {
      // add to map
      FilePair filePair(cal.getCalibTime(), path);
      fileMap.insert(filePair);
    }
    
  } // dp
    
  rdir.close();
  
  return 0;

}

