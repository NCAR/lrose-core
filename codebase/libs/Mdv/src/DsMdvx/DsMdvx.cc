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
// DsMdvx.cc
//
// DsMdvx object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 1999
//
///////////////////////////////////////////////////////////////
//
// The DsMdvx object adds server capability to the Mdvx class.
//
///////////////////////////////////////////////////////////////

#include <toolsa/TaStr.hh>
#include <toolsa/ugetenv.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/DsMdvxMsg.hh>
#include <Mdv/Mdv2NcfTrans.hh>
#include <Mdv/Ncf2MdvTrans.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxField.hh>
#include <dsserver/DsLocator.hh>
#include <dsserver/DmapAccess.hh>
#include <dsserver/DsClient.hh>
#include <dsserver/DsLdataInfo.hh>
#include <didss/RapDataDir.hh>
using namespace std;

DsMdvx::DsMdvx() : Mdvx()
{
  clearRead();
  clearWrite();
  clearTimeListMode();
}

DsMdvx::~DsMdvx()
{
}

/////////////////////////////
// Copy constructor
//

DsMdvx::DsMdvx(const DsMdvx &rhs)

{
  if (this != &rhs) {
    _copy(rhs);
  }
}

// clear all memory, initialize to starting state

void DsMdvx::clear()
{
  Mdvx::clear();
}

//////////////////////////////
// Assignment
//

DsMdvx &DsMdvx::operator=(const DsMdvx &rhs)
  
{
  return _copy(rhs);
}

//////////////////////////////
// Assignment
//

DsMdvx &DsMdvx::_copy(const DsMdvx &rhs)

{
  
  if (&rhs == this) {
    return *this;
  }

  Mdvx::_copy(rhs);

  // copy members

  _readDirUrl = rhs._readDirUrl;
  _readPathUrl = rhs._readPathUrl;
  _timeListUrl = rhs._timeListUrl;

  _calcClimo = rhs._calcClimo;
  _climoTypeList = rhs._climoTypeList;
  _climoDataStart = rhs._climoDataStart;
  _climoDataEnd = rhs._climoDataEnd;
  _climoStartTime = rhs._climoStartTime;
  _climoEndTime = rhs._climoEndTime;
  
  return *this;
  
}

////////////////////
// overload clearRead

void DsMdvx::clearRead()
  
{
  Mdvx::clearRead();
  clearReadTime();
  clearReadPath();
  clearClimoRead();
}

////////////////////
// Clear climatology request information

void DsMdvx::clearClimoRead()
{
  clearCalcClimo();
  clearClimoTypeList();
  setClimoDataRange(DateTime::NEVER, DateTime::NEVER);
  setClimoTimeRange(-1, -1, -1, -1, -1, -1);
}

////////////////////////////////////////////////////////////////////////
// read time
//
// You must either set the readTime or readPath.
//
// If you specify read time, the path is determined from the dir
// the search_time and the search mode.
//
// Search modes are as follows:
//
//   READ_LAST: read last available data set
//   READ_CLOSEST: read closest data within search_margin of search_time
//   READ_FIRST_BEFORE: read first data at or before search_time,
//                      within search_margin
//   READ_FIRST_AFTER:  read first data at or after search_time,
//                      within search_margin
//   READ_BEST_FORECAST: read best forecast within 
//                       search_margin of search_time.
//                       Takes data from latest available gen time.
//   READ_SPECIFIED_FORECAST: read forecast generated at search time,
//                            closest to forecast_lead_time,
//                            within search_margin

void DsMdvx::setReadTime(const read_search_mode_t mode,
			 const string &read_dir_url,
			 const int search_margin /* = 0*/,
			 const time_t search_time /* = 0*/,
			 const int forecast_lead_time /* = 0*/ )
  
{
  _readSearchMode = mode;
  _readDirUrl = read_dir_url;
  DsURL readUrl(read_dir_url);
  _readDir = readUrl.getFile();
  _readSearchTime = search_time;
  _readSearchMargin = search_margin;
  _readForecastLeadTime = forecast_lead_time;
  _readTimeSet = true;
  clearReadPath();
}

void DsMdvx::clearReadTime()

{
  _readSearchMode = READ_LAST;
  _readDirUrl = ".";
  _readSearchTime = 0;
  _readSearchMargin = 0;
  _readForecastLeadTime = 0;
  _readTimeSet = false;
}

////////////////////////////////////////////////////////////////////////
// read path
//
// You must either set the readTime or readPath.
// If you specify the path, that exact path will be used.

void DsMdvx::setReadPath(const string &read_path_url)
  
{
  _readPathUrl = read_path_url;
  DsURL readUrl(read_path_url);
  _readPath = readUrl.getFile();
  _readPathSet = true;
  clearReadTime();
}

void DsMdvx::clearReadPath()

{
  _readPath = ".";
  _readPathSet = false;
}

/////////////////////////////////////////////////////////////////
// setTimeListModeValid
//
// Set the time list so that it finds all of the valid data
// times between the start and end times.
// For forecast data where multiple forecasts exist for the same
// valid time, a single valid time will be returned.

void DsMdvx::setTimeListModeValid(const string &url,
				  time_t start_time,
				  time_t end_time)
  
{

  _timeListUrl = url;
  DsURL tlistUrl(url);
  const string &dir = tlistUrl.getFile();
  Mdvx::setTimeListModeValid(dir, start_time, end_time);

}

/////////////////////////////////////////////////////////////////
// setTimeListModeGen
//
// set the time list mode so that it finds all of the
// generate times between the start and end times

void DsMdvx::setTimeListModeGen(const string &url,
				time_t start_gen_time,
				time_t end_gen_time)
  
{

  _timeListUrl = url;
  DsURL tlistUrl(url);
  const string &dir = tlistUrl.getFile();
  Mdvx::setTimeListModeGen(dir, start_gen_time, end_gen_time);

}

/////////////////////////////////////////////////////////////////
// setTimeListModeForecast
//
// set the time list mode so that it finds all of the forecast
// times for the given generate time

void DsMdvx::setTimeListModeForecast(const string &url,
				     time_t gen_time)

{

  _timeListUrl = url;
  DsURL tlistUrl(url);
  const string &dir = tlistUrl.getFile();
  Mdvx::setTimeListModeForecast(dir, gen_time);

}

// setTimeListModeLead is equivalent, and is deprecated

void DsMdvx::setTimeListModeLead(const string &url,
				 time_t gen_time)
{
  setTimeListModeForecast(url, gen_time);
}

/////////////////////////////////////////////////////////////////
// setTimeListModeGenPlusForecasts
//
// Set the time list so that it finds all of the
// generate times between the start and end gen times.
// Then, for each generate time, all of the forecast times are
// found. These are made available in the
// _forecastTimesArray, which is represented by vector<vector<time_t> >

void DsMdvx::setTimeListModeGenPlusForecasts(const string &url,
					     time_t start_gen_time,
					     time_t end_gen_time)
  
{
  
  _timeListUrl = url;
  DsURL tlistUrl(url);
  const string &dir = tlistUrl.getFile();
  Mdvx::setTimeListModeGenPlusForecasts(dir, start_gen_time, end_gen_time);
  
}

/////////////////////////////////////////////////////////////////
// setModeValidMultGen
//
// Set the time list so that it finds all of the forecasts
// within the time interval specified. For each forecast found
// the associated generate time is also determined.
// The forecast times will be available in the _timeList array.
// The generate times will be available in the _genTimes array.

void DsMdvx::setTimeListModeValidMultGen(const string &url,
					 time_t start_time,
					 time_t end_time)
  
{
  
  _timeListUrl = url;
  DsURL tlistUrl(url);
  const string &dir = tlistUrl.getFile();
  Mdvx::setTimeListModeValidMultGen(dir, start_time, end_time);

}

/////////////////////////////////////////////////////////////////
// setTimeListModeFirst
//
// set the time list mode so that it finds the first data time

void DsMdvx::setTimeListModeFirst(const string &url)

{

  _timeListUrl = url;
  DsURL tlistUrl(url);
  const string &dir = tlistUrl.getFile();
  Mdvx::setTimeListModeFirst(dir);

}

/////////////////////////////////////////////////////////////////
// setTimeListModeLast
//
// set the time list mode so that it finds the last data time

void DsMdvx::setTimeListModeLast(const string &url)

{

  _timeListUrl = url;
  DsURL tlistUrl(url);
  const string &dir = tlistUrl.getFile();
  Mdvx::setTimeListModeLast(dir);

}

/////////////////////////////////////////////////////////////////
// setTimeListModeClosest
//
// set the time list mode so that it finds the closest available data time
// to the search time within the search margin

void DsMdvx::setTimeListModeClosest(const string &url,
				    time_t search_time,
				    int time_margin)
  
{
  _timeListUrl = url;
  DsURL tlistUrl(url);
  const string &dir = tlistUrl.getFile();
  Mdvx::setTimeListModeClosest(dir, search_time, time_margin);
}

/////////////////////////////////////////////////////////////////
// setTimeListModeFirstBefore
//
// set the time list mode so that it finds the first available data time
// before the search time within the search margin

void DsMdvx::setTimeListModeFirstBefore(const string &url,
					time_t search_time,
					int time_margin)
  
{
  _timeListUrl = url;
  DsURL tlistUrl(url);
  const string &dir = tlistUrl.getFile();
  Mdvx::setTimeListModeFirstBefore(dir, search_time, time_margin);
}

/////////////////////////////////////////////////////////////////
// setTimeListModeFirstAfter
//
// set the time list mode so that it finds the first available data time
// after the search time within the search margin

void DsMdvx::setTimeListModeFirstAfter(const string &url,
				       time_t search_time,
				       int time_margin)
  
{
  _timeListUrl = url;
  DsURL tlistUrl(url);
  const string &dir = tlistUrl.getFile();
  Mdvx::setTimeListModeFirstAfter(dir, search_time, time_margin);
}

/////////////////////////////////////////////////////////////////
// setTimeListModeBestForecast
//
// Set the time list so that it returns the best forecast
// for the search time, within the time margin

void DsMdvx::setTimeListModeBestForecast(const string &url,
					 time_t search_time,
					 int time_margin)
  
{
  _timeListUrl = url;
  DsURL tlistUrl(url);
  const string &dir = tlistUrl.getFile();
  Mdvx::setTimeListModeBestForecast(dir, search_time, time_margin);
}

/////////////////////////////////////////////////////////////////
// setTimeListModeSpecifiedForecast
//
// Set the time list so that it returns the forecast for the given
// generate time, closest to the search time, within the time margin

void DsMdvx::setTimeListModeSpecifiedForecast(const string &url,
					      time_t gen_time,
					      time_t search_time,
					      int time_margin)

{
  _timeListUrl = url;
  DsURL tlistUrl(url);
  const string &dir = tlistUrl.getFile();
  Mdvx::setTimeListModeSpecifiedForecast(dir, gen_time,
					 search_time, time_margin);
}

/////////////////////////////////////////////////////////////////
// clearTimeListMode
//
// clear out time list mode info

void DsMdvx::clearTimeListMode()

{
  Mdvx::clearTimeListMode();
  _timeListUrl = "";

}

////////////////////
// calcClimo

void DsMdvx::setCalcClimo()
{
  _calcClimo = true;
}

void DsMdvx::clearCalcClimo()
{
  _calcClimo = false;
}

////////////////////
// climo statistic type

void DsMdvx::clearClimoTypeList()
{
  _climoTypeList.erase(_climoTypeList.begin(), _climoTypeList.end());
}


void DsMdvx::addClimoStatType(const climo_type_t climo_type,
			      const bool divide_by_num_obs,
			      const double param1, const double param2)
{
  climo_stat_t climo_stat;
  
  climo_stat.type = climo_type;
  climo_stat.divide_by_num_obs = divide_by_num_obs;
  climo_stat.params[0] = param1;
  climo_stat.params[1] = param2;
  
  _climoTypeList.push_back(climo_stat);
}

////////////////////
// climo date range

void DsMdvx::setClimoDataRange(const DateTime start_time,
			       const DateTime end_time)
{
  _climoDataStart = start_time;
  _climoDataEnd = end_time;
}

void DsMdvx::setClimoDataRange(const time_t start_time,
			       const time_t end_time)
{
  _climoDataStart.set(start_time);
  _climoDataEnd.set(end_time);
}

////////////////////
// climo time range

void DsMdvx::setClimoTimeRange(const int start_hour,
			       const int start_minute,
			       const int start_second,
			       const int end_hour,
			       const int end_minute,
			       const int end_second)
{
  _climoStartTime.hour = start_hour;
  _climoStartTime.minute = start_minute;
  _climoStartTime.second = start_second;
  
  _climoEndTime.hour = end_hour;
  _climoEndTime.minute = end_minute;
  _climoEndTime.second = end_second;
}

//////////////////////////////////
// override readAllHeaders method

int DsMdvx::readAllHeaders()
{

  clearErrStr();

  // resolve server details

  DsURL url;
  bool contactServer;
  if (_resolveReadUrl(url, &contactServer)) {
    _errStr += "ERROR - COMM DsMdvx::readAllHeaders.\n";
    return -1;
  }

  if (contactServer) {
    return _readAllHeadersRemote(url);
  } else {
    return Mdvx::readAllHeaders();
  }
  
  
}

//////////////////////////////////
// read the headers from a remote server

int DsMdvx::_readAllHeadersRemote(const DsURL &url)
{

  // assemble message packet

  DsMdvxMsg msg;
  if (_debug) {
    msg.setDebug();
  }
  if (_read32BitHeaders) {
    msg._setUse32BitHeaders(true);
  } else {
    msg._setUse32BitHeaders(false);
  }

  void *msgBuf = msg.assembleReadAllHdrs(*this);
  if (msgBuf == NULL) {
    _errStr += "ERROR - DsMdvx::readAllHeaders.\n";
    _errStr += "  Assembling outgoing message, URL:\n";
    _errStr += url.getURLStr();
    _errStr += "\n";
    return -1;
  }

  // communicate with server

  if (_communicate(url, msg, msgBuf, msg.lengthAssembled())) {
    _errStr += "ERROR - COMM DsMdvx::readAllHeaders.\n";
    _errStr += "  Communicating with server\n";
    return -1;
  }

  if (msg.getError()) {
    return -1;
  }

  if (msg.getSubType() != DsMdvxMsg::MDVP_READ_ALL_HDRS) {
    _errStr += "ERROR - DsMdvx::readAllHeaders.\n";
    TaStr::AddInt(_errStr, "  Incorrect return subType: ", msg.getSubType());
    return -1;
  }

  return 0;

}

//////////////////////////////////
// override readVolume method

int DsMdvx::readVolume()
{

  clearErrStr();

  // resolve server details - should we contact server?

  DsURL url;
  bool contactServer;
  if (_resolveReadUrl(url, &contactServer)) {
    _errStr += "ERROR - COMM DsMdvx::readVolume.\n";
    return -1;
  }

  if (contactServer) {
    return _readVolumeRemote(url);
  } else {
    return Mdvx::readVolume();
  }

}

//////////////////////////////////
// read volume via server

int DsMdvx::_readVolumeRemote(const DsURL &url)
{

  // assemble message packet
  
  DsMdvxMsg msg;
  if (_debug) {
    msg.setDebug();
  }
  if (_read32BitHeaders) {
    msg._setUse32BitHeaders(true);
  } else {
    msg._setUse32BitHeaders(false);
  }

  void *msgBuf = msg.assembleReadVolume(*this);
  if (msgBuf == NULL) {
    _errStr += "ERROR - DsMdvx::_readVolumeRemote.\n";
    _errStr += "  Assembling outgoing message, URL:\n";
    _errStr += url.getURLStr();
    _errStr += "\n";
    return -1;
  }

  // communicate with server
  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("DsMdvx::_readVolumeRemote");
  }

  if (_communicate(url, msg, msgBuf, msg.lengthAssembled())) {
    _errStr += "ERROR - COMM - DsMdvx::_readVolumeRemote.\n";
    _errStr += "  Communicating with server\n";
    return -1;
  }

  if (msg.getError()) {
    return -1;
  }

  if (msg.getSubType() != DsMdvxMsg::MDVP_READ_VOLUME) {
    _errStr += "ERROR - DsMdvx::_readVolumeRemote.\n";
    TaStr::AddInt(_errStr, "  Incorrect return subType: ", msg.getSubType());
    return -1;
  }

  return 0;

}

//////////////////////////////////
// override readVsection method

int DsMdvx::readVsection()
{

  clearErrStr();

  // resolve server details

  DsURL url;
  bool contactServer;
  if (_resolveReadUrl(url, &contactServer)) {
    _errStr += "ERROR - COMM - DsMdvx::readVsection.\n";
    return -1;
  }

  if (contactServer) {
    return _readVsectionRemote(url);
  } else {
    return Mdvx::readVsection();
  }

}

//////////////////////////////////
// read vsection from server

int DsMdvx::_readVsectionRemote(const DsURL &url)
{

  DsMdvxMsg msg;
  if (_debug) {
    msg.setDebug();
  }
  if (_read32BitHeaders) {
    msg._setUse32BitHeaders(true);
  } else {
    msg._setUse32BitHeaders(false);
  }

  void *msgBuf = msg.assembleReadVsection(*this);
  if (msgBuf == NULL) {
    _errStr += "ERROR - DsMdvx::_readVsectionRemote.\n";
    _errStr += "  Assembling outgoing message, URL:\n";
    _errStr += url.getURLStr();
    _errStr += "\n";
    return -1;
  }
  
  // communicate with server
  
  if (_communicate(url, msg, msgBuf, msg.lengthAssembled())) {
    _errStr += "ERROR - COMM - DsMdvx::_readVsectionRemote.\n";
    _errStr += "  Communicating with server\n";
    return -1;
  }
  
  if (msg.getError()) {
    return -1;
  }
  
  if (msg.getSubType() != DsMdvxMsg::MDVP_READ_VSECTION) {
    _errStr += "ERROR - DsMdvx::_readVsectionRemote.\n";
    TaStr::AddInt(_errStr, "  Incorrect return subType: ", msg.getSubType());
    return -1;
  }
  
  return 0;

}

//////////////////////////////////
// override compileTimeList method

int DsMdvx::compileTimeList()
{

  clearErrStr();

  // resolve server details

  DsURL url;
  bool contactServer;
  if (_resolveTimeListUrl(url, &contactServer)) {
    _errStr += "ERROR - COMM - DsMdvx::compileTimeList.\n";
    return -1;
  }

  if (!contactServer) {
    // Set the local directory based on the URL
    _timeList.setDir(url.getFile());
    
    // local - direct read
    return (Mdvx::compileTimeList());
  }

  // assemble message packet
  
  DsMdvxMsg msg;
  if (_debug) {
    msg.setDebug();
  }
  if (_read32BitHeaders) {
    msg._setUse32BitHeaders(true);
  } else {
    msg._setUse32BitHeaders(false);
  }

  void *msgBuf = msg.assembleCompileTimeList(*this);
  if (msgBuf == NULL) {
    _errStr += "ERROR - DsMdvx::compileTimeList.\n";
    _errStr += "  Assembling outgoing message, URL:\n";
    _errStr += url.getURLStr();
    _errStr += "\n";
    return -1;
  }

  // communicate with server

  if (_communicate(url, msg, msgBuf, msg.lengthAssembled())) {
    _errStr += "ERROR - COMM - DsMdvx::compileTimeList.\n";
    _errStr += "  Communicating with server\n";
    return -1;
  }

  if (msg.getError()) {
    return -1;
  }

  if (msg.getSubType() != DsMdvxMsg::MDVP_COMPILE_TIME_LIST) {
    _errStr += "ERROR - DsMdvx::compileTimeList.\n";
    TaStr::AddInt(_errStr, "  Incorrect return subType: ", msg.getSubType());
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////////////
// Compile time-height (time-series) profile, according to the
// read settings and time list specifications.
//
// Before using this call:
//
// (a) Set up read parameters, such as fields, vlevels, encoding
//     type etc.
// (b) Set up the lat/lon of the point to be sampled, using:
//     addReadWayPt(lat, lon)
// (c) Set up the time list, using setTimeListMode????()
//
// Data fields returned will be time-height profiles, with time
// in the x dimension, ny = 1, height in the z dimension.
//
// Actual times for the data will be returned in the time lists,
// namely validTimes and genTimes.

// Returns 0 on success, -1 on failure
// getErrStr() retrieves the error string.

int DsMdvx::compileTimeHeight()
  
{

  clearErrStr();
  
  // resolve server details

  DsURL url;
  bool contactServer;
  if (_resolveTimeListUrl(url, &contactServer)) {
    _errStr += "ERROR - COMM - DsMdvx::compileTimeHeight.\n";
    return -1;
  }

  if (!contactServer) {
    // local - direct read
    return (Mdvx::compileTimeHeight());
  }
  
  // assemble message packet
  
  DsMdvxMsg msg;
  if (_debug) {
    msg.setDebug();
  }
  if (_read32BitHeaders) {
    msg._setUse32BitHeaders(true);
  } else {
    msg._setUse32BitHeaders(false);
  }

  void *msgBuf = msg.assembleCompileTimeHeight(*this);
  if (msgBuf == NULL) {
    _errStr += "ERROR - DsMdvx::compileTimeHeight.\n";
    _errStr += "  Assembling outgoing message, URL:\n";
    _errStr += url.getURLStr();
    _errStr += "\n";
    return -1;
  }

  // communicate with server

  if (_communicate(url, msg, msgBuf, msg.lengthAssembled())) {
    _errStr += "ERROR - COMM - DsMdvx::readTimeHeight.\n";
    _errStr += "  Communicating with server\n";
    return -1;
  }

  if (msg.getError()) {
    return -1;
  }

  if (msg.getSubType() != DsMdvxMsg::MDVP_COMPILE_TIME_HEIGHT) {
    _errStr += "ERROR - DsMdvx::compileTimeHeight.\n";
    TaStr::AddInt(_errStr, "  Incorrect return subType: ", msg.getSubType());
    return -1;
  }
  
  return 0;

}

////////////////////////////////////////////////////////
// Write to directory
//
// File path is computed - see setWriteAsForecast().
// _latest_data_info file is written as appropriate - 
//    see setWriteLdataInfo().
//
// Returns 0 on success, -1 on error.
// getErrStr() retrieves the error string.

int DsMdvx::writeToDir(const string &output_url)
{
  
  clearErrStr();
  
  // check environment for write control
  
  _checkEnvBeforeWrite();

  // check environment for extended paths
  
  char *extendedPathsStr = getenv("MDV_WRITE_USING_EXTENDED_PATHS");
  if (extendedPathsStr != NULL) {
    if (!strcasecmp(extendedPathsStr, "TRUE")) {
      _useExtendedPaths = true;
    }
  }

  // resolve server details
  
  DsURL url;
  bool contactServer;
  if (_resolveOutputUrl(url, output_url, &contactServer)) {
    _errStr += "ERROR - COMM - DsMdvx::writeToDir.\n";
    return -1;
  }

  // if we don't need to use the server, this is a local write
  
  if (!contactServer) {
    // local write
    string dir = url.getFile();
    if (Mdvx::writeToDir(dir)) {
      _errStr += "ERROR - DsMdvx::writeToDir\n";
      TaStr::AddStr(_errStr, "  Writing locally to dir: ", dir);
      return -1;
    }
    return 0;
  }

  // need to contact server
  // assemble message packet
  
  DsMdvxMsg msg;
  if (_debug) {
    msg.setDebug();
  }
  if (_read32BitHeaders) {
    msg._setUse32BitHeaders(true);
  } else {
    msg._setUse32BitHeaders(false);
  }

  void *msgBuf =
    msg.assembleWrite(DsMdvxMsg::MDVP_WRITE_TO_DIR,
                      *this, url.getURLStr());
  if (msgBuf == NULL) {
    _errStr += "ERROR - DsMdvx::writeToDir.\n";
    _errStr += "  Assembling outgoing message, URL:\n";
    _errStr += url.getURLStr();
    _errStr += "\n";
    return -1;
  }

  // communicate with server

  if (_communicate(url, msg, msgBuf, msg.lengthAssembled())) {
    _errStr += "ERROR - COMM - DsMdvx::writeToDir.\n";
    _errStr += "  Communicating with server\n";
    return -1;
  }

  if (msg.getError()) {
    return -1;
  }
  
  if (msg.getSubType() != DsMdvxMsg::MDVP_WRITE_TO_DIR) {
    _errStr += "ERROR - DsMdvx::writeToDir.\n";
    TaStr::AddInt(_errStr, "  Incorrect return subType: ", msg.getSubType());
    return -1;
  }

  return 0;

}

//////////////////////////////////
// override writeToPath method

int DsMdvx::writeToPath(const string &output_url)
{

  clearErrStr();
  
  // resolve server details

  DsURL url;
  bool contactServer;
  if (_resolveOutputUrl(url, output_url, &contactServer)) {
    _errStr += "ERROR - COMM - DsMdvx::writeToPath.\n";
    return -1;
  }

  if (!contactServer) {
    // local - direct write
    return Mdvx::writeToPath(url.getFile());
  }
  
  // assemble message packet

  DsMdvxMsg msg;
  if (_debug) {
    msg.setDebug();
  }
  if (_read32BitHeaders) {
    msg._setUse32BitHeaders(true);
  } else {
    msg._setUse32BitHeaders(false);
  }

  void *msgBuf =
    msg.assembleWrite(DsMdvxMsg::MDVP_WRITE_TO_PATH, *this, url.getURLStr());
  if (msgBuf == NULL) {
    _errStr += "ERROR - DsMdvx::writeToPath.\n";
    _errStr += "  Assembling outgoing message, URL:\n";
    _errStr += url.getURLStr();
    _errStr += "\n";
    return -1;
  }

  // communicate with server

  if (_communicate(url, msg, msgBuf, msg.lengthAssembled())) {
    _errStr += "ERROR - COMM - DsMdvx::writeToPath.\n";
    _errStr += "  Communicating with server\n";
    return -1;
  }

  if (msg.getError()) {
    return -1;
  }

  if (msg.getSubType() != DsMdvxMsg::MDVP_WRITE_TO_PATH) {
    _errStr += "ERROR - DsMdvx::writeToPath.\n";
    TaStr::AddInt(_errStr, "  Incorrect return subType: ", msg.getSubType());
    return -1;
  }

  return 0;

}

///////////////////////////////////////////////////////////////////
// write files for an object which contains data for multiple
// forecast times
//
// Some applications (such as converters from NetCDF) produce an
// Mdvx object which contains fields from different forecast times.
//
// This routine separates out the fields for each time, and writes
// one file per time
//
// Returns 0 on success, -1 on failure

int DsMdvx::writeMultForecasts(const string &output_url)
  
{

  // create a set containing unique times

  set<time_t> ftimes;
  for (int ifield = 0; ifield < (int) _fields.size(); ifield++) {
    ftimes.insert(_fields[ifield]->getFieldHeader().forecast_time);
  }
  
  // loop through times in the set

  set<time_t>::iterator itime;
  for (itime = ftimes.begin(); itime != ftimes.end(); itime++) {
    
    time_t forecastTime = *itime;

    // create object for just this forecast time
    
    DsMdvx outMdvx;
    outMdvx._mhdr = _mhdr;
    outMdvx._mhdr.data_dimension = 0;
    outMdvx._mhdr.data_collection_type = Mdvx::DATA_FORECAST;
    outMdvx._mhdr.num_data_times = 1; 

    outMdvx._mhdr.time_centroid = forecastTime;
    outMdvx._mhdr.forecast_time = forecastTime;
    int leadTime = forecastTime - _mhdr.time_gen;
    outMdvx._mhdr.forecast_delta = leadTime;
      
    // Add fields which have this forecast time
      
    for (int ifield = 0; ifield < (int) _fields.size(); ifield++) {
      Mdvx::field_header_t fhdr = _fields[ifield]->getFieldHeader();
      time_t fieldTime = fhdr.forecast_time;
      if (fieldTime == forecastTime) {
        MdvxField *newField = new MdvxField(*_fields[ifield]);
        outMdvx.addField(newField);
      }
    } // ifield

    // add chunks

    for (int ichunk = 0; ichunk < (int) _chunks.size(); ichunk++) {
      MdvxChunk *newChunk = new MdvxChunk(*_chunks[ichunk]);
      outMdvx.addChunk(newChunk);
    }
    
    // prepare write
    
    outMdvx.setWriteAsForecast(); // to prevent overwrites
    outMdvx.setWriteLdataInfo();
    outMdvx._useExtendedPaths = _useExtendedPaths;
    outMdvx._internalFormat = FORMAT_MDV;
    outMdvx._writeFormat = _writeFormat;
 
    // write to file
      
    if (outMdvx.writeToDir(output_url)) {
      _errStr += "ERROR - writeToDirMultForecasts\n";
      return -1;
    }
      
  } // itime

  return 0;

}

//////////////////////////////
// overload print read request

void DsMdvx::printReadRequest(ostream &out)

{

  Mdvx::printReadRequest(out);
  if (_readTimeSet) {
    out << "  Read dir url: " << _readDirUrl << endl;
  } else if (_readPathSet) {
    out << "  Read path url: " << _readPathUrl << endl;
  }
  
  if (_calcClimo) 
  {
    out << "  Calc climo?: " << _calcClimo << endl;

    vector< climo_stat_t >::iterator stat_iter;
    
    for (stat_iter = _climoTypeList.begin(); stat_iter != _climoTypeList.end();
	 ++stat_iter)
    {
      out << "  Climo stat: " << climoType2Str(stat_iter->type);
      
      if (stat_iter->type == CLIMO_TYPE_NUM_OBS_GT ||
	  stat_iter->type == CLIMO_TYPE_NUM_OBS_GE ||
	  stat_iter->type == CLIMO_TYPE_NUM_OBS_LT ||
	  stat_iter->type == CLIMO_TYPE_NUM_OBS_LE)
	out << "   " << stat_iter->params[0];
      
      if (stat_iter->divide_by_num_obs)
	out << "   divided by num obs";
      
      out << endl;
    }
    
    out << "  Climo data start time: " << _climoDataStart << endl;
    out << "  Climo data end time: " << _climoDataEnd << endl;
    out << "  Climo start hour: " << _climoStartTime.hour << endl;
    out << "  Climo start minute: " << _climoStartTime.minute << endl;
    out << "  Climo start second: " << _climoStartTime.second << endl;
    out << "  Climo end hour: " << _climoEndTime.hour << endl;
    out << "  Climo end minute: " << _climoEndTime.minute << endl;
    out << "  Climo end second: " << _climoEndTime.second << endl;
    
  }
  
}


///////////////////////////////////
// overload print time list request

void DsMdvx::printTimeListRequest(ostream &out)

{

  Mdvx::printTimeListRequest(out);
  out << "  url: " << _timeListUrl << endl;

}
  
///////////////////////////////////
// overload print time height request

void DsMdvx::printTimeHeightRequest(ostream &out)

{

  Mdvx::printTimeHeightRequest(out);
  out << "  url: " << _timeListUrl << endl;

}
  
////////////////////////////////////////////
// Communicate with server 
//  Uses http tunnel and proxy if required.
//
// Open the server's  socket, write the
// request message, receive and disassemble the reply.
// Add and strip Http headers if required.
//
// Returns 0 on success, -1 on error

int DsMdvx::_communicate(const DsURL &url,
			 DsMdvxMsg &msg,
			 const void *msgBuf,
			 const int msgLen)
  
{

  DsClient client;
  client.setDebug(_debug);
  client.setErrStr("ERROR - DsMdvx::_communicate\n");
  
  if (client.communicateAutoFwd(url, DsMdvxMsg::MDVP_REQUEST_MESSAGE,
				msgBuf, msgLen)) {
    _errStr += client.getErrStr();
    return -1;
  }
  
  // disassemble the reply
  
  if (_debug) {
    cerr << "----> DsMdvx::_communicate() dissasembling reply" << endl;
  }
  
  if (msg.disassemble(client.getReplyBuf(), client.getReplyLen(), *this)) {
    _errStr += "ERROR - COMM - DsMdvx::_communicate msg.disassemble\n";
    _errStr += "Invalid reply - cannot disassemble\n";
    _errStr += msg.getErrStr();
    _errStr += "\n";
    return -1;
  }

  return 0;

}

//////////////////////////
// resolve URL for reading
//
// Sets _readPath or _readDir as appropriate for local reads.
//
// Returns 0 on success, -1 on failure.

int DsMdvx::_resolveReadUrl(DsURL &url, bool *contact_server)

{
  if (_readPathSet) {
    url.setURLStr(_readPathUrl);
    RapDataDir.fillPath(url.getFile(), _readPath);
  } else {
    url.setURLStr(_readDirUrl);
    RapDataDir.fillPath(url.getFile(), _readDir);
  }
  DsLOCATOR locator;

  if (locator.resolve(url, contact_server, false)) {
    _errStr += "ERROR - COMM - DsMdvx::_resolveReadUrl.\n";
    _errStr += "  Cannot resolve URL: ";
    _errStr += url.getURLStr();
    _errStr += "\n";
    return -1;
  }
  
  return 0;

}

//////////////////////////
// resolve URL for writing
//
// Returns 0 on success, -1 on failure.

int DsMdvx::_resolveOutputUrl(DsURL &url,
                              const string &output_url,
                              bool *contact_server)
  
{

  // resolve server details

  url.setURLStr(output_url);
  DsLOCATOR locator;
  if (locator.resolve(url, contact_server, false)) {
    _errStr += "ERROR - COMM - DsMdvx::_resolveWriteUrl.\n";
    _errStr += "  Cannot resolve URL: ";
    _errStr += url.getURLStr();
    _errStr += "\n";
    return -1;
  }

  return 0;

}

////////////////////////////
// resolve URL for time list
//
// Sets _timeListDir as appropriate for local reads.
//
// Returns 0 on success, -1 on failure.

int DsMdvx::_resolveTimeListUrl(DsURL &url, bool *contact_server)

{

  url.setURLStr(_timeListUrl);
  // RapDataDir.fillPath(url.getFile(), _timeListDir);
  DsLOCATOR locator;
  if (locator.resolve(url, contact_server, false)) {
    _errStr += "ERROR - COMM - DsMdvx::_resolveTimeListUrl.\n";
    _errStr += "  Cannot resolve URL: ";
    _errStr += url.getURLStr();
    _errStr += "\n";
    return -1;
  }
  
  return 0;

}

