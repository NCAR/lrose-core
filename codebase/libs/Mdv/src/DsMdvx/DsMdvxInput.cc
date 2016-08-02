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
//////////////////////////////////////////////////////////
// DsMdvxInput.cc
//
// Class to provide input control for Mdv data.
//
// There are three modes of operation:
//
// Realtime mode:
//   The constructor passes in a URL to watch, as well as
//   as max valid age and a heartbeat function to be called while
//   waiting for new data to arrive.
//   When requested to read, the object will watch for new data.
//   When new data arrives it is read. If the heartbeat_func is not NULL,
//   it is called while polling continues.
//
// Archive mode:
//   The constructor passes in a URL and a start and end time.
//   A list of available times is compiled and stored.
//   When requested to read, the next data time is used.
//   The endOfData flag is set when the data is exhausted.
//
// Filelist mode:
//   The constructor passes in a vector of input paths.
//   When requested to read, the next path is used.
//   The endOfData flag is set when the data is exhausted.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2000
//
//////////////////////////////////////////////////////////
#include <didss/DsInputPath.hh>
#include <Mdv/DsMdvxInput.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/DateTime.hh>
using namespace std;

//////////////
// Constructor

DsMdvxInput::DsMdvxInput ()

{
  _mode = NO_MODE;
  _searchMarginSecs = 0;
}

/////////////
// Destructor

DsMdvxInput::~DsMdvxInput()
{
  
}

////////////////////////////////////////////////////////
// Set archive mode.
//
// Sets up DsMdvxTimes object in ARCHIVE mode.
//
// Load up archive time list.
// The list will contain all available data times
// between the start and end times for the given URL.
//
// Returns 0 on success, -1 on error.
// Use getErrStr() for error message.

int DsMdvxInput::setArchive(const string &url,
                            const time_t start_time,
                            const time_t end_time)
  
{
  _clearErrStr();
  int iret = _times.setArchive(url, start_time, end_time);
  if (iret) {
    _errStr = "ERROR - DsMdvxInput::setArchive\n";
    _errStr += _times.getErrStr();
    return -1;
  }
  _url = url;
  _mode = ARCHIVE_MODE;
  return 0;
}

////////////////////////////////////////////////////////
// Set archive mode.
//
// Sets up DsMdvxTimes object in ARCHIVE mode.
//
// Load up archive time list.
// The list will contain all available data times
// between the start and end times for the given URL.
//
// Returns 0 on success, -1 on error.
// Use getErrStr() for error message.

int DsMdvxInput::setArchiveFcst(const string &url,
                            const time_t start_time,
                            const time_t end_time)

{
  _clearErrStr();
  int iret = _times.setArchiveFcst(url, start_time, end_time);
  if (iret) {
    _errStr = "ERROR - DsMdvxInput::setArchive\n";
    _errStr += _times.getErrStr();
    return -1;
  }
  _url = url;
  _mode = ARCHIVE_FCST_MODE;
  return 0;
}

/////////////////////////////////////////////////////////
// Set realtime mode.
//
// Sets up DsMdvxTimes object in REALTIME mode.
//
//  url: URL for which the data times are required.
//
//  max_valid_age: the max valid age for data (secs)
//     The object will not return data which has not arrived
//     within this period. (NOTE: NOT IMPLEMENTED YET)
//
//  heartbeat_func: pointer to heartbeat_func.
//    If NULL this is ignored.
//    If non-NULL, this is called once per delay_msecs while
//    the routine is polling for new data.
//
//  delay_msecs
//     polling delay in millisecs.
//     The object will sleep for this time between polling attempts.
//
// Returns 0 on success, -1 on error.
// Use getErrStr() for error message.

int DsMdvxInput::setRealtime(const string &url,
                             const int max_valid_age,
                             const heartbeat_t heartbeat_func /* = NULL*/,
                             const int delay_msecs /* = 5000*/ )

{

  _clearErrStr();
  int iret = _times.setRealtime(url, max_valid_age,
                                heartbeat_func, delay_msecs);
  if (iret) {
    _errStr = "ERROR - DsMdvxInput::setRealtime\n";
    _errStr += _times.getErrStr();
    return -1;
  }
  _url = url;
  _mode = REALTIME_MODE;
  return 0;

}

//////////////////////////////////////////////////////////////////////
// Set filelist mode.
//
// input_file_list: list of files through which to iterate for reads.
//
// Returns 0 on success, -1 on error.
// Use getErrStr() for error message.
  
int DsMdvxInput::setFilelist(const vector<string> &input_file_list)

{
  _clearErrStr();
  _fileList = input_file_list;
  _fileListPtr = 0;
  _mode = FILELIST_MODE;
  return 0;
}

//////////////////////////////////////////////////////////////////////
// Get the data time from a filepath
//
// NOTE: this is a static utility method which can be invoked
//       without instantiating a DsMdvxInput class,
//       thus cannot be used with getErrStr()
//
//  path: file path to parse for determining data time
//  dataTime: returned utime parsed from the data set path
//
// Returns 0 on success, -1 on error.

int DsMdvxInput::getDataTime( const string& path, time_t& dataTime )
{
   //
   // For now, use DsInputPath
   //

  if (DsInputPath::getDataTime(path, dataTime)) {
    return -1;
  }
  
  return 0;
  
}

int DsMdvxInput::_setMdvxInArchiveFcstMode(DsMdvx &mdvx)
{
    int archivePtrIndex = _times.getArchivePtr();
    if (_times.getForecastTime(_forecastTime, archivePtrIndex)) {
      _errStr += "ERROR - DsMdvxInput::readAllHeadersNext\n";
      _errStr += _times.getErrStr();
      return -1;
    }

    if (_times.getNext(_dataTime)) {
      _errStr += "ERROR - DsMdvxInput::readAllHeadersNext\n";
      _errStr += _times.getErrStr();
      return -1;
    }

    mdvx.setReadTime(Mdvx::READ_SPECIFIED_FORECAST, _url,
		     _searchMarginSecs,
                     _dataTime,
                     _forecastTime-_dataTime);
    return 0;
}

///////////////////////////////////////////////////////////
// Read the next data set, using the DsMdvx object provided.
//
// In ARCHIVE mode, reads data for the next time in the list.
// In FILELIST mode, reads data from the next path in the list.
// In REALTIME mode, blocks until a new data time is available.
//
// Returns 0 on success, -1 on error.
// Use getErrStr() for error message.

int DsMdvxInput::readAllHeadersNext(DsMdvx &mdvx)
  
{

  _clearErrStr();
  if (_mode == NO_MODE) {
    _errStr += "ERROR - DsMdvxInput::readAllHeadersNext\n";
    _errStr += "  Mode not set.\n";
    return -1;
  }
  
  if (_mode == ARCHIVE_MODE || _mode == REALTIME_MODE) {

    if (_times.getNext(_dataTime)) {
      _errStr += "ERROR - DsMdvxInput::readAllHeadersNext\n";
      _errStr += _times.getErrStr();
      return -1;
    }

    // set the read time

    mdvx.setReadTime(Mdvx::READ_CLOSEST, _url,
		     _searchMarginSecs,
                     _dataTime);

    if (mdvx.readAllHeaders()) {
      _errStr += "ERROR - COMM - DsMdvxInput::readAllHeadersNext\n";
      _errStr += mdvx.getErrStr();
      return -1;
    }

  } else if (_mode == ARCHIVE_FCST_MODE) {
    if (_setMdvxInArchiveFcstMode(mdvx)) {
      _errStr += "ERROR - DsMdvxInput::readAllHeadersNext\n";
      _errStr += _times.getErrStr();
      return -1;
    }

    if (mdvx.readAllHeaders()) {
        _errStr += "ERROR - COMM - DsMdvxInput::readAllHeadersNext\n";
        _errStr += mdvx.getErrStr();
        return -1;
    }
  } else if (_mode == FILELIST_MODE) {
    
    if (_fileListPtr >= _fileList.size()) {
      _errStr += "ERROR - DsMdvxInput::readAllHeadersNext\n";
      _errStr += "  Filelist mode - list exhausted.\n";
      return -1;
    }

    string nextPath = _fileList[_fileListPtr];
    _fileListPtr++;

    if (getDataTime( nextPath, _dataTime )) {
      _errStr += "ERROR - DsMdvxInput::readAllHeadersNext\n";
      _errStr += "  Cannot parse path for dataTime: ";
      _errStr += nextPath;
      return -1;
    }
 
    mdvx.setReadPath(nextPath);
    
    if (mdvx.readAllHeaders()) {
      _errStr += "ERROR - COMM - DsMdvxInput::readAllHeadersNext\n";
      _errStr += mdvx.getErrStr();
      return -1;
    }
    
  } else {

    _errStr += "ERROR - DsMdvxInput::readAllHeadersNext\n";
    _addIntErr("  Unknown mode: ", _mode);
    return (-1);

  }
  
  return 0;

}


int DsMdvxInput::readVolumeNext(DsMdvx &mdvx)
  
{

  _clearErrStr();
  if (_mode == NO_MODE) {
    _errStr += "ERROR - DsMdvxInput::readVolumeNext\n";
    _errStr += "  Mode not set.\n";
    return -1;
  }
  
  if (_mode == ARCHIVE_MODE || _mode == REALTIME_MODE) {

    if (_times.getNext(_dataTime)) {
      _errStr += "ERROR - DsMdvxInput::readVolumeNext\n";
      _errStr += _times.getErrStr();
      return -1;
    }

    // set the read time

    mdvx.setReadTime(Mdvx::READ_CLOSEST, _url,
		     _searchMarginSecs,
		     _dataTime);

    if (mdvx.readVolume()) {
      _errStr += "ERROR - COMM - DsMdvxInput::readVolumeNext\n";
      _errStr += mdvx.getErrStr();
      return -1;
    }

  } else if (_mode == ARCHIVE_FCST_MODE) {
    if (_setMdvxInArchiveFcstMode(mdvx)) {
      _errStr += "ERROR - DsMdvxInput::readAllHeadersNext\n";
      _errStr += _times.getErrStr();
      return -1;
    }

    if (mdvx.readVolume()) {
      _errStr += "ERROR - COMM - DsMdvxInput::readVolumeNext\n";
      _errStr += mdvx.getErrStr();
      return -1;
    }
  } else if (_mode == FILELIST_MODE) {
    
    if (_fileListPtr >= _fileList.size()) {
      _errStr += "ERROR - DsMdvxInput::readVolumeNext\n";
      _errStr += "  Filelist mode - list exhausted.\n";
      return -1;
    }

    string nextPath = _fileList[_fileListPtr];
    _fileListPtr++;

    if (getDataTime( nextPath, _dataTime )) {
      _errStr += "ERROR - DsMdvxInput::readVolumeNext\n";
      _errStr += "  Cannot parse path for dataTime: ";
      _errStr += nextPath;
      return -1;
    }
 
    mdvx.setReadPath(nextPath);
    
    if (mdvx.readVolume()) {
      _errStr += "ERROR - COMM - DsMdvxInput::readVolumeNext\n";
      _errStr += mdvx.getErrStr();
      return -1;
    }

    
  } else {

    _errStr += "ERROR - DsMdvxInput::readVolumeNext\n";
    _addIntErr("  Unknown mode: ", _mode);
    return (-1);

  }
  
  return 0;

}

int DsMdvxInput::readVolumeNextWithMaxValidAge(DsMdvx &mdvx)
  
{

  _clearErrStr();

  if (_mode != REALTIME_MODE) {
    // this is a realtime only method
    return readVolumeNext(mdvx);
  }
  

  if (_times.getNext(_dataTime)) {
    _errStr += "ERROR - DsMdvxInput::readVolumeNext\n";
    _errStr += _times.getErrStr();
    return -1;
  }

  // set the read time, using the max realtime age as the margin

  mdvx.setReadTime(Mdvx::READ_CLOSEST, _url,
		   _times.getMaxRealtimeAge(),
		   _dataTime);

  if (mdvx.readVolume()) {
    _errStr += "ERROR - COMM - DsMdvxInput::readVolumeNext\n";
    _errStr += mdvx.getErrStr();
    return -1;
  }

  return 0;

}


int DsMdvxInput::readVsectionNext(DsMdvx &mdvx)
  
{

  _clearErrStr();
  if (_mode == NO_MODE) {
    _errStr += "ERROR - DsMdvxInput::readVsectionNext\n";
    _errStr += "  Mode not set.\n";
    return -1;
  }
  
  if (_mode == ARCHIVE_MODE || _mode == REALTIME_MODE) {

    if (_times.getNext(_dataTime)) {
      _errStr += "ERROR - DsMdvxInput::readVsectionNext\n";
      _errStr += _times.getErrStr();
      return -1;
    }

    // set the read time

    mdvx.setReadTime(Mdvx::READ_CLOSEST, _url,
		     _searchMarginSecs,
		     _dataTime);

    if (mdvx.readVsection()) {
      _errStr += "ERROR - COMM - DsMdvxInput::readVsectionNext\n";
      _errStr += mdvx.getErrStr();
      return -1;
    }

  } else if (_mode == ARCHIVE_FCST_MODE) {
    if (_setMdvxInArchiveFcstMode(mdvx)) {
      _errStr += "ERROR - DsMdvxInput::readAllHeadersNext\n";
      _errStr += _times.getErrStr();
      return -1;
    }
    if (mdvx.readVsection()) {
      _errStr += "ERROR - COMM - DsMdvxInput::readVsectionNext\n";
      _errStr += mdvx.getErrStr();
      return -1;
    }
  } else if (_mode == FILELIST_MODE) {
    
    if (_fileListPtr >= _fileList.size()) {
      _errStr += "ERROR - DsMdvxInput::readVsectionNext\n";
      _errStr += "  Filelist mode - list exhausted.\n";
      return -1;
    }

    string nextPath = _fileList[_fileListPtr];
    _fileListPtr++;

    if (getDataTime( nextPath, _dataTime )) {
      _errStr += "ERROR - DsMdvxInput::readVsectionNext\n";
      _errStr += "  Cannot parse path for dataTime: ";
      _errStr += nextPath;
      return -1;
    }

    mdvx.setReadPath(nextPath);
    
    if (mdvx.readVsection()) {
      _errStr += "ERROR - COMM - DsMdvxInput::readVsectionNext\n";
      _errStr += mdvx.getErrStr();
      return -1;
    }

    
  } else {

    _errStr += "ERROR - DsMdvxInput::readVsectionNext\n";
    _addIntErr("  Unknown mode: ", _mode);
    return (-1);

  }
  
  return 0;

}

///////////////////////////////////////////////////////////
// Get the next time.
//
// Passes the request down to the DsMdvxTimes object.
//
// Only available in ARCHIVE and REALTIME modes.
//
// In archive mode, gets the next time in the list,
// returns error when list is exhausted.
//
// In realtime mode, if delay_secs is non-negative
// blocks until a new data time is available.
// If delay_msecs is negative, does not block. Check
// getNextSuccess() to see if data is available.
//
// Returns 0 on success, -1 on failure.

int DsMdvxInput::getTimeNext(time_t &next_time)
  
{
  
  _clearErrStr();
  
  if (_mode != ARCHIVE_MODE && _mode != REALTIME_MODE) {
    _errStr += "ERROR - DsMdvxInput::getTimeNext\n";
    _errStr += "  Only available in REALTIME and ARCHIVE modes.\n";
    return -1;
  }
  
  return _times.getNext(next_time);

}

/////////////////////////
// reset to start of list
// 
// Archive mode only.

void DsMdvxInput::reset()
  
{
  if (_mode == ARCHIVE_MODE || _mode == REALTIME_MODE) {
    _times.reset();
  } else {
    _fileListPtr = 0;
  }
}

//////////////////////////////////////////////
// check whether we are at the end of the data
  
bool DsMdvxInput::endOfData() const

{
  if (_mode == ARCHIVE_MODE || _mode == REALTIME_MODE || _mode == ARCHIVE_FCST_MODE) {
    return (_times.endOfData());
  } else {
    if (_fileListPtr >= _fileList.size()) {
      return true;
    } else {
      return false;
    }
  }

}
  
/////////////////////////////////////
// add error string with int argument

void DsMdvxInput::_addIntErr(const char *err_str, const int iarg)
{
  _errStr += err_str;
  char str[32];
  sprintf(str, "%d\n", iarg);
  _errStr += str;
}

////////////////////////////////////////
// add error string with string argument

void DsMdvxInput::_addStrErr(const char *err_str, const string &sarg)
{
  _errStr += err_str;
  _errStr += sarg;
  _errStr += "\n";
}

////////////////////
// clear error string

void DsMdvxInput::_clearErrStr()
{
  _errStr = "";
  TaStr::AddStr(_errStr, "Time for following error: ", DateTime::str());
}
