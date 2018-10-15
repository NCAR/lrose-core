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
  clearMdv2Ncf();
}

DsMdvx::~DsMdvx()
{
  clearMdv2Ncf();
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
  clearMdv2Ncf();
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
  
  _ncfInstitution = rhs._ncfInstitution;
  _ncfReferences = rhs._ncfReferences;
  _ncfComment = rhs._ncfComment;
  _mdv2NcfTransArray = rhs._mdv2NcfTransArray;

  _ncfCompress = rhs._ncfCompress;
  _ncfCompressionLevel = rhs._ncfCompressionLevel;

  _ncfFileFormat = rhs._ncfFileFormat;
  _ncfOutputLatlonArrays = rhs._ncfOutputLatlonArrays;
  _ncfOutputMdvAttr = rhs._ncfOutputMdvAttr;
  _ncfOutputMdvChunks = rhs._ncfOutputMdvChunks;
  _ncfOutputStartEndTimes =  rhs._ncfOutputStartEndTimes;
  _ncfRadialFileType = rhs._ncfRadialFileType;

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
    return _readAllHeadersLocal(url);
  }
  
  
}

/////////////////////////////////////////
// read the headers from a local file

int DsMdvx::_readAllHeadersLocal(const DsURL &url)
{
  // Compute the read path of the file and process specially if it is
  // a netCDF file.

  if (_computeReadPath()) {
    _errStr += "ERROR - DsMdvx::_readAllHeadersLocal\n";
    return -1;
  }

  if (isNcfFile(_pathInUse)) {
    
    // read NCF headers
    
    _currentFormat = FORMAT_NCF;
    if (readAllHeadersNcf(url.getURLStr())) {
      _errStr += "ERROR - DsMdvx::_readAllHeadersLocal.\n";
      TaStr::AddStr(_errStr, "  Reading headers from NCF file");
      TaStr::AddStr(_errStr, "  Url: ", url.getURLStr());
      return -1;
    }
    
  } else if (isRadxFile(_pathInUse)) {
    
    // read RADX radial radar format file
    
    _currentFormat = FORMAT_RADX;
    if (readAllHeadersRadx(url.getURLStr())) {
      _errStr += "ERROR - DsMdvx::_readAllHeadersLocal.\n";
      TaStr::AddStr(_errStr, "  Reading headers from RADX file");
      TaStr::AddStr(_errStr, "  Url: ", url.getURLStr());
      return -1;
    }
    
  } else {
    
    // local direct read

    return Mdvx::readAllHeaders();
  }
  
  return 0;
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
    return _readVolumeLocal(url);
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

  if (convertFormatOnRead(url.getURLStr())) {
    _errStr += "ERROR - DsMdvx::_readVolumeRemote.\n";
    TaStr::AddStr(_errStr, "  Converting format after read");
    return -1;
  }

  return 0;

}

//////////////////////////////////
// read volume from local file

int DsMdvx::_readVolumeLocal(const DsURL &url)
{

  // read time list as appropriate
  // get the path of the file
  
  if (_readTimeListAlso) {
    if (compileTimeList()) {
      _errStr += "ERROR - DsMdvx::_readVolumeLocal\n";
      _errStr += "  Time list requested in addition to volume data.\n";
      return -1;
    }
  }
  
  if (_computeReadPath()) {
    _errStr += "ERROR - DsMdvx::_readVolumeLocal\n";
    return -1;
  }
  
  if (isNcfFile(_pathInUse)) {
    
    // read NCF file

    _currentFormat = FORMAT_NCF;
    if (readNcf(url.getURLStr())) {
      _errStr += "ERROR - DsMdvx::_readVolumeLocal.\n";
      TaStr::AddStr(_errStr, "  Reading NCF file");
      TaStr::AddStr(_errStr, "  Url: ", url.getURLStr());
      return -1;
    }
    
  } else if (isRadxFile(_pathInUse)) {
    
    // read RADX radial radar format file
    
    _currentFormat = FORMAT_RADX;
    if (readRadx(url.getURLStr())) {
      _errStr += "ERROR - DsMdvx::_readVolumeLocal.\n";
      TaStr::AddStr(_errStr, "  Reading RADX file");
      TaStr::AddStr(_errStr, "  Url: ", url.getURLStr());
      return -1;
    }
    
  } else {
    
    // local - direct read
    
    if (_read_volume(_readFillMissing, _readDecimate, true)) {
      _errStr += "ERROR - DsMdvx::_readVolumeLocal.\n";
      TaStr::AddStr(_errStr, "  Reading local file");
      TaStr::AddStr(_errStr, "  Url: ", url.getURLStr());
      return -1;
    }
    
    if (convertFormatOnRead(url.getURLStr())) {
      _errStr += "ERROR - DsMdvx::_readVolumeLocal.\n";
      TaStr::AddStr(_errStr, "  Converting format after read");
      TaStr::AddStr(_errStr, "  Url: ", url.getURLStr());
      return -1;
    }
  }
  
  // success
  
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
    return _readVsectionLocal(url);
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
  
  if (convertFormatOnRead(url.getURLStr())) {
    _errStr += "ERROR - DsMdvx::_readVsectionRemote.\n";
    TaStr::AddStr(_errStr, "  Converting format after read");
    return -1;
  }
  
  return 0;

}

//////////////////////////////////
// read Vsection from local files

int DsMdvx::_readVsectionLocal(const DsURL &url)
{

  // read time list as appropriate
  
  if (_readTimeListAlso) {
    if (compileTimeList()) {
      _errStr += "ERROR - DsMdvx::_readVsectionLocal\n";
      _errStr += "  Time list requested in addition to vsection data.\n";
      return -1;
    }
  }
  
  // get the path of the file
  
  if (_computeReadPath()) {
    _errStr += "ERROR - DsMdvx::_readVsectionLocal\n";
    return -1;
  }
  
  if (isNcfFile(_pathInUse)) {
    
    // read NCF file
    
    _currentFormat = FORMAT_NCF;
    if (readNcf(url.getURLStr())) {
      _errStr += "ERROR - DsMdvx::_readVsectionLocal.\n";
      TaStr::AddStr(_errStr, "  Reading NCF file");
      TaStr::AddStr(_errStr, "  Url: ", url.getURLStr());
      return -1;
    }
    
  } else if (isRadxFile(_pathInUse)) {
    
    // read RADX radial radar format file
    
    _currentFormat = FORMAT_RADX;
    if (readRadx(url.getURLStr())) {
      _errStr += "ERROR - DsMdvx::_readVsectionLocal.\n";
      TaStr::AddStr(_errStr, "  Reading RADX file");
      TaStr::AddStr(_errStr, "  Url: ", url.getURLStr());
      return -1;
    }
    
  } else {
    
    // local Mdv file - direct read
    
    if (Mdvx::readVsection()) {
      _errStr += "ERROR - DsMdvx::_readVsectionLocal.\n";
      TaStr::AddStr(_errStr, "  Reading local file");
      TaStr::AddStr(_errStr, "  Url: ", url.getURLStr());
      return -1;
    }
    if (convertFormatOnRead(url.getURLStr())) {
      _errStr += "ERROR - DsMdvx::_readVsectionLocal.\n";
      TaStr::AddStr(_errStr, "  Converting format after read");
      TaStr::AddStr(_errStr, "  Url: ", url.getURLStr());
      return -1;
    }
    
  } // if (isNcfFile(_pathInUse)) 
  
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
    _errStr += "ERROR - COMM - DsMdvx::compileTimeList.\n";
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

  if (!contactServer) {
    // local write
    if (_writeToDirLocal(output_url)) {
      _errStr += "ERROR - DsMdvx::writeToDir\n";
      return -1;
    }
    return 0;
  }

  // if either current or write format is not MDV,
  // contact NcMdvServer to write file
  // if (_writeFormat == FORMAT_NCF || _currentFormat == FORMAT_NCF) {
  //   if (_writeNcfToDir(output_url)) {
  //     _errStr += "ERROR - DsMdvx::writeToDir\n";
  //     _errStr += "  Writing NCF file via NcMdvServer\n";
  //     TaStr::AddStr(_errStr, "  Url: ", output_url);
  //     return -1;
  //   }
  //   return 0;
  // }

  // need to contact server
  // assemble message packet

  DsMdvxMsg msg;
  if (_debug) {
    msg.setDebug();
  }
  void *msgBuf =
    msg.assembleWrite(DsMdvxMsg::MDVP_WRITE_TO_DIR, *this, url.getURLStr());
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
  
  if (convertFormatOnWrite(output_url)) {
    _errStr += "ERROR - COMM - DsMdvx::writeToPath.\n";
    return -1;
  }
  
  // resolve server details

  DsURL url;
  bool contactServer;
  if (_resolveOutputUrl(url, output_url, &contactServer)) {
    _errStr += "ERROR - COMM - DsMdvx::writeToPath.\n";
    return -1;
  }

  if (!contactServer) {
    // local - direct write
    return (Mdvx::writeToPath(url.getFile().c_str()));
  }

  // assemble message packet

  DsMdvxMsg msg;
  if (_debug) {
    msg.setDebug();
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
    outMdvx._currentFormat = FORMAT_MDV;
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
  
  // url.setProtocol("mdvp");
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

////////////////////////////////
// MDV to netCDF CF conversion

// set mdv to ncf conversion attributes
// For conversion of MDV into netCDF.

void DsMdvx::setMdv2NcfAttr(const string &institution,
                            const string &references,
                            const string &comment)

{

  _ncfInstitution = institution;
  _ncfReferences = references;
  _ncfComment = comment;

}

// replace comment attribute with input string.

void DsMdvx::setMdv2NcfCommentAttr(const string &comment)
{
  _ncfComment = comment;
}

// set compression - uses HDF5

void DsMdvx::setMdv2NcfCompression(bool compress,
                                   int compressionLevel)
  
{
  
  _ncfCompress = compress;
  _ncfCompressionLevel = compressionLevel;

}

// set the output format of the netCDF file

void DsMdvx::setMdv2NcfFormat(nc_file_format_t fileFormat)

{
  _ncfFileFormat = fileFormat;
}

// set the file type for polar radar files

void DsMdvx::setRadialFileType(radial_file_type_t fileType)

{
  _ncfRadialFileType = fileType;
}

// set output parameters - what should be included

void DsMdvx::setMdv2NcfOutput(bool outputLatlonArrays,
                              bool outputMdvAttr,
                              bool outputMdvChunks,
                              bool outputStartEndTimes)

{

  _ncfOutputLatlonArrays = outputLatlonArrays;
  _ncfOutputMdvAttr = outputMdvAttr;
  _ncfOutputMdvChunks = outputMdvChunks;
  _ncfOutputStartEndTimes = outputStartEndTimes;
}

//////////////////////////////////////////
// add mdv to ncf field translation info
// For conversion of MDV into netCDF.

void DsMdvx::addMdv2NcfTrans(string mdvFieldName,
                             string ncfFieldName,
                             string ncfStandardName,
                             string ncfLongName,
                             string ncfUnits,
                             bool doLinearTransform,
                             double linearMult,
                             double linearOffset,
                             ncf_pack_t packing)
  
{
  
  Mdv2NcfFieldTrans trans;

  trans.mdvFieldName = mdvFieldName;
  trans.ncfFieldName = ncfFieldName;
  trans.ncfStandardName = ncfStandardName;
  trans.ncfLongName = ncfLongName;
  trans.ncfUnits = ncfUnits;
  trans.doLinearTransform = doLinearTransform;
  trans.linearMult = linearMult;
  trans.linearOffset = linearOffset;
  trans.packing = packing;


  _mdv2NcfTransArray.push_back(trans);
  
}
  
// clear MDV to NetCDF parameters

void DsMdvx::clearMdv2Ncf()
  
{
  _ncfInstitution.clear();
  _ncfReferences.clear();
  _ncfComment.clear();
  _mdv2NcfTransArray.clear();
  _ncfCompress = true;
  _ncfCompressionLevel = 9;
  _ncfFileFormat = NCF_FORMAT_NETCDF4;
  _ncfOutputLatlonArrays = true;
  _ncfOutputMdvAttr = true;
  _ncfOutputMdvChunks = true;
  _ncfOutputStartEndTimes = true;
  _ncfRadialFileType = RADIAL_TYPE_CF;

}

////////////////////////////////////////////////
// return string representation of packing type

string DsMdvx::ncfPack2Str(const ncf_pack_t packing)

{

  switch(packing)  {
    case NCF_PACK_FLOAT:
      return "NCF_PACK_FLOAT"; 
    case NCF_PACK_SHORT:
      return "NCF_PACK_SHORT"; 
    case NCF_PACK_BYTE:
      return "NCF_PACK_BYTE"; 
    case NCF_PACK_ASIS:
      return "NCF_PACK_ASIS"; 
    default:
      return "NCF_PACK_FLOAT";
  }

}

////////////////////////////////////////////////
// return enum representation of packing type

DsMdvx::ncf_pack_t DsMdvx::ncfPack2Enum(const string &packing)

{

  if (!packing.compare("NCF_PACK_FLOAT")) {
    return NCF_PACK_FLOAT;
  } else if (!packing.compare("NCF_PACK_SHORT")) {
    return NCF_PACK_SHORT;
  } else if (!packing.compare("NCF_PACK_BYTE")) {
    return NCF_PACK_BYTE;
  } else if (!packing.compare("NCF_PACK_ASIS")) {
    return NCF_PACK_ASIS;
  } else {
    return NCF_PACK_FLOAT;
  }

}

 ////////////////////////////////////////////////
// return string representation of nc format

string DsMdvx::ncFormat2Str(const nc_file_format_t format)

{
  
  switch(format)  {
    case NCF_FORMAT_NETCDF4:
      return "NCF_FORMAT_NETCDF4";
    case NCF_FORMAT_CLASSIC:
      return "NCF_FORMAT_CLASSIC"; 
    case NCF_FORMAT_OFFSET64BITS:
      return "NCF_FORMAT_OFFSET64BITS"; 
    case NCF_FORMAT_NETCFD4_CLASSIC:
      return "NCF_FORMAT_NETCFD4_CLASSIC"; 
    default:
      return "NCF_FORMAT_NETCDF4";
  }
  
}

////////////////////////////////////////////////
// return enum representation of nc format

DsMdvx::nc_file_format_t DsMdvx::ncFormat2Enum(const string &format)

{

  if (!format.compare("NCF_FORMAT_NETCDF4")) {
    return NCF_FORMAT_NETCDF4;
  } else if (!format.compare("NCF_FORMAT_CLASSIC")) {
    return NCF_FORMAT_CLASSIC;
  } else if (!format.compare("NCF_FORMAT_OFFSET64BITS")) {
    return NCF_FORMAT_OFFSET64BITS;
  } else if (!format.compare("NCF_FORMAT_NETCFD4_CLASSIC")) {
    return NCF_FORMAT_NETCFD4_CLASSIC;
  } else {
    return NCF_FORMAT_NETCDF4;
  }

}


////////////////////////////////////////////////
// return string representation of file type

string DsMdvx::radialFileType2Str(const radial_file_type_t ftype)

{
  
  switch(ftype)  {
    case RADIAL_TYPE_CF_RADIAL:
      return "RADIAL_TYPE_CF_RADIAL"; 
    case RADIAL_TYPE_DORADE:
      return "RADIAL_TYPE_DORADE"; 
    case RADIAL_TYPE_UF:
      return "RADIAL_TYPE_UF"; 
    case RADIAL_TYPE_CF:
    default:
      return "RADIAL_TYPE_CF";
  }
  
}

////////////////////////////////////////////////
// return enum representation of file type

DsMdvx::radial_file_type_t DsMdvx::radialFileType2Enum(const string &ftype)

{

  if (!ftype.compare("RADIAL_TYPE_CF_RADIAL")) {
    return RADIAL_TYPE_CF_RADIAL;
  } else if (!ftype.compare("RADIAL_TYPE_DORADE")) {
    return RADIAL_TYPE_DORADE;
  } else if (!ftype.compare("RADIAL_TYPE_UF")) {
    return RADIAL_TYPE_UF;
  } else {
    return RADIAL_TYPE_CF;
  }

}


////////////////////////////////////////////////
// converts format to that requested on read
// returns 0 on success, -1 on failure

int DsMdvx::convertFormatOnRead(const string &read_url)
  
{

  if (_readFormat == FORMAT_NCF && _currentFormat == FORMAT_MDV) {
    if (convertMdv2Ncf(read_url)) {
      _errStr += "ERROR - COMM - DsMdvx::convertFormatOnRead.\n";
      _errStr += "  Converting MDV to NCF\n";
      return -1;
    }
//   } else if (_readFormat == FORMAT_MDV && _currentFormat == FORMAT_NCF) {
//     if (convertNcf2Mdv(read_url)) {
//       _errStr += "ERROR - COMM - DsMdvx::convertFormatOnRead.\n";
//       _errStr += "  Converting NCF to MDV\n";
//       return -1;
//     }
//   } else if (!_ncfConstrained &&
//              _readFormat == FORMAT_NCF && _currentFormat == FORMAT_NCF) {
//     if (constrainNcf(read_url)) {
//       _errStr += "ERROR - COMM - DsMdvx::convertFormatOnRead.\n";
//       _errStr += "  Constraining NCF using read specifications\n";
//       return -1;
//     }
  }
  
  return 0;

}

////////////////////////////////////////////////
// before writing, convert format to that specified
// for writing
// returns 0 on success, -1 on failure

int DsMdvx::convertFormatOnWrite(const string &url)
  
{

  if (_writeFormat == FORMAT_NCF && _currentFormat == FORMAT_MDV) {
    if (convertMdv2Ncf(url)) {
      _errStr += "ERROR - COMM - DsMdvx::convertFormatOnWrite.\n";
      _errStr += "  Converting MDV to NCF\n";
      return -1;
    }
  } else if (_writeFormat == FORMAT_MDV && _currentFormat == FORMAT_NCF) {
    if (convertNcf2Mdv(url)) {
      _errStr += "ERROR - COMM - DsMdvx::convertFormatOnWrite.\n";
      _errStr += "  Converting NCF to MDV\n";
      return -1;
    }
  }
  
  return 0;

}

////////////////////////////////////////////////
// convert MDV format to NETCDF CF format
// returns 0 on success, -1 on failure

int DsMdvx::convertMdv2Ncf(const string &url)
  
{
  
  if (_currentFormat != FORMAT_MDV) {
    _errStr += "ERROR - DxMdvx::convertMdv2Ncf.\n";
    TaStr::AddStr(_errStr, "  Url: ", url);
    TaStr::AddStr(_errStr, "  Incorrect format: ", format2Str(_currentFormat));
    TaStr::AddStr(_errStr, "  Should be: ", format2Str(FORMAT_MDV));
    return -1;
  }

  // ensure master header is consistent with file body

  updateMasterHeader();

  // compute temporary file path

  time_t now = time(NULL);
  DateTime dnow(now);
  pid_t pid = getpid();
  char tmpFilePath[FILENAME_MAX];
  sprintf(tmpFilePath,
          "/tmp/DxMdvx_convertMdv2Ncf_%.4d%.2d%.2d_%.2d%.2d%.2d_%.5d.nc",
          dnow.getYear(), dnow.getMonth(), dnow.getDay(),
          dnow.getHour(), dnow.getMin(), dnow.getSec(), pid);
          
  Mdv2NcfTrans trans;
  trans.clearData();
  trans.setDebug(_debug);
  if(_heartbeatFunc != NULL) {
    trans.setHeartbeatFunction(_heartbeatFunc);
  }

  
  if (trans.translate(*this, tmpFilePath)) {
    _errStr += "ERROR - DxMdvx::convertMdv2Ncf.\n";
    TaStr::AddStr(_errStr, "  Url: ", url);
    _errStr += trans.getErrStr();
    return -1;
  }

  // open NCF file
  
  TaFile ncfFile;
  if (ncfFile.fopen(tmpFilePath, "rb") == NULL) {
    int errNum = errno;
    _errStr += "ERROR - DxMdvx::convertMdv2Ncf\n";
    TaStr::AddStr(_errStr, "  Url: ", url);
    TaStr::AddStr(_errStr, "  Cannot open tmp file: ", tmpFilePath);
    _errStr += strerror(errNum);
    _errStr += "\n";
    unlink(tmpFilePath);
    return -1;
  }

  // stat the file to get size
  
  if (ncfFile.fstat()) {
    int errNum = errno;
    _errStr += "ERROR - DxMdvx::convertMdv2Ncf\n";
    TaStr::AddStr(_errStr, "  Url: ", url);
    TaStr::AddStr(_errStr, "  Cannot stat tmp file: ", tmpFilePath);
    _errStr += strerror(errNum);
    _errStr += "\n";
    ncfFile.fclose();
    unlink(tmpFilePath);
    return -1;
  }
  stat_struct_t &fileStat = ncfFile.getStat();
  off_t fileLen = fileStat.st_size;
  
  // read in buffer
  
  _ncfBuf.reserve(fileLen);
  if (ncfFile.fread(_ncfBuf.getPtr(), 1, fileLen) != fileLen) {
    int errNum = errno;
    _errStr += "ERROR - DxMdvx::convertMdv2Ncf\n";
    TaStr::AddStr(_errStr, "  Url: ", url);
    TaStr::AddStr(_errStr, "  Cannot read tmp file: ", tmpFilePath);
    _errStr += strerror(errNum);
    _errStr += "\n";
    ncfFile.fclose();
    unlink(tmpFilePath);
    return -1;
  }
  
  // close tmp file and remove
  
  ncfFile.fclose();
  unlink(tmpFilePath);

  // set current format
  
  _currentFormat = FORMAT_NCF;

  // set times etc

  _ncfValidTime = _mhdr.time_centroid;
  _ncfGenTime = _mhdr.time_gen;
  _ncfForecastTime = _mhdr.forecast_time;
  _ncfForecastDelta = _mhdr.forecast_delta;
  if (_mhdr.data_collection_type == Mdvx::DATA_FORECAST ||
      _mhdr.data_collection_type == Mdvx::DATA_EXTRAPOLATED) {
    _ncfIsForecast = true;
  } else {
    _ncfIsForecast = false;
  }
  _ncfEpoch = _mhdr.epoch;

  // clear MDV format data

  clearFields();
  clearChunks();

  return 0;

}

////////////////////////////////////////////////
// convert MDV format to NETCDF CF format
// using the NcMdvServer (deprecated)
// returns 0 on success, -1 on failure

int DsMdvx::_convertMdv2NcfViaServer(const string &url)
  
{
  
  if (_currentFormat != FORMAT_MDV) {
    _errStr += "ERROR - DsMdvx::convertMdv2Ncf.\n";
    TaStr::AddStr(_errStr, "  Incorrect format: ", format2Str(_currentFormat));
    TaStr::AddStr(_errStr, "  Should be: ", format2Str(FORMAT_MDV));
    return -1;
  }

  // set up URL for doing conversion via NcMdvServer

  DsURL trans_url(url);
  trans_url.setProtocol("mdvp");
  trans_url.setTranslator("NcMdvServer");
  
  DsLOCATOR locator;
  bool contact_server;
  if (locator.resolve(trans_url, &contact_server, false)) {
    _errStr += "ERROR - COMM - DsMdvx::convertMdv2Ncf.\n";
    _errStr += "  Cannot resolve URL: ";
    _errStr += trans_url.getURLStr();
    _errStr += "\n";
    return -1;
  }
  
  if (_debug) {
    cerr << "convertMdv2Ncf(): MDV to NCF" << endl;
    cerr << "  URL: " << trans_url.getURLStr() << endl;
  }

  // assemble message packet

  DsMdvxMsg msg;
  if (_debug) {
    msg.setDebug();
  }
  void *msgBuf =
    msg.assembleConvertMdv2Ncf(*this, trans_url.getURLStr());
  if (msgBuf == NULL) {
    _errStr += "ERROR - DsMdvx::convertMdv2Ncf.\n";
    _errStr += "  Assembling outgoing message, URL:\n";
    _errStr += trans_url.getURLStr();
    _errStr += "\n";
    return -1;
  }

  // communicate with server

  if (_communicate(trans_url, msg, msgBuf, msg.lengthAssembled())) {
    _errStr += "ERROR - COMM - DsMdvx::convertMdv2Ncf.\n";
    _errStr += "  Communicating with server\n";
    return -1;
  }

  if (msg.getError()) {
    return -1;
  }

  if (msg.getSubType() != DsMdvxMsg::MDVP_CONVERT_MDV_TO_NCF) {
    _errStr += "ERROR - DsMdvx::convertMdv2Ncf\n";
    TaStr::AddInt(_errStr, "  Incorrect return subType: ", msg.getSubType());
    return -1;
  }

  _ncfConstrained = true;

  return 0;

}

////////////////////////////////////////////////
// convert NETCDF CF to MDV
// given an object containing a netcdf file buffer
// returns 0 on success, -1 on failure

int DsMdvx::convertNcf2Mdv(const string &url)
  
{

  if (_currentFormat != FORMAT_NCF) {
    _errStr += "ERROR - DsMdvx::convertNcf2Mdv.\n";
    TaStr::AddStr(_errStr, "  Url: ", url);
    TaStr::AddStr(_errStr, "  Incorrect format: ", format2Str(_currentFormat));
    TaStr::AddStr(_errStr, "  Should be: ", format2Str(FORMAT_NCF));
    return -1;
  }

  // save read details

  Mdvx::encoding_type_t readEncodingType = _readEncodingType;
  Mdvx::compression_type_t readCompressionType = _readCompressionType;
  Mdvx::scaling_type_t readScalingType = _readScalingType;
  double readScale = _readScale;
  double readBias = _readBias;

  // compute temporary file path
  
  time_t now = time(NULL);
  DateTime dnow(now);
  pid_t pid = getpid();
  char tmpFilePath[FILENAME_MAX];
  sprintf(tmpFilePath,
          "/tmp/DsMdvx_convertNcf2Mdv_%.4d%.2d%.2d_%.2d%.2d%.2d_%.5d.nc",
          dnow.getYear(), dnow.getMonth(), dnow.getDay(),
          dnow.getHour(), dnow.getMin(), dnow.getSec(), pid);

  // write nc buffer to file

  if (_write_buffer_to_file(tmpFilePath, _ncfBuf.getLen(), _ncfBuf.getPtr())) {
    _errStr += "ERROR - DsMdvx::convertNcf2Mdv\n";
    TaStr::AddStr(_errStr, "  Url: ", url);
    TaStr::AddStr(_errStr, "  Cannot write buffe to tmp file: ", tmpFilePath);
    return -1;
  }
          
  // create translator
  
  Ncf2MdvTrans trans;
  trans.setDebug(_debug);
  
  // perform translation
  // returns 0 on success, -1 on failure

  if (trans.translate(tmpFilePath, *this)) {
    _errStr += "ERROR - DsMdvx::convertNcf2Mdv\n";
    TaStr::AddStr(_errStr, "  Url: ", url);
    TaStr::AddStr(_errStr, "  Cannot translate file: ", tmpFilePath);
    TaStr::AddStr(_errStr, trans.getErrStr());
    unlink(tmpFilePath);
    return -1;
  }

  // remove tmp file
  
  unlink(tmpFilePath);

  // remove netcdf data
  
  clearNcf();

  // set format to MDV
  
  _currentFormat = FORMAT_MDV;

  // convert the output fields appropriately

  for (int ii = 0; ii < (int) _fields.size(); ii++) {
    _fields[ii]->convertType(readEncodingType,
                             readCompressionType,
                             readScalingType,
                             readScale, readBias);
  }

  return 0;

}

////////////////////////////////////////////////
// convert NETCDF CF format to MDV format
// using the NcMdvServer (deprecated)
// returns 0 on success, -1 on failure

int DsMdvx::_convertNcf2MdvViaServer(const string &url)
  
{

  if (_currentFormat != FORMAT_NCF) {
    _errStr += "ERROR - DsMdvx::convertNcf2Mdv.\n";
    TaStr::AddStr(_errStr, "  Incorrect format: ", format2Str(_currentFormat));
    TaStr::AddStr(_errStr, "  Should be: ", format2Str(FORMAT_NCF));
    return -1;
  }
  
  // save read details

  Mdvx::encoding_type_t readEncodingType = _readEncodingType;
  Mdvx::compression_type_t readCompressionType = _readCompressionType;
  Mdvx::scaling_type_t readScalingType = _readScalingType;
  double readScale = _readScale;
  double readBias = _readBias;

  // set up URL for doing conversion via NcMdvServer
  
  DsURL trans_url(url);
  trans_url.setProtocol("mdvp");
  trans_url.setTranslator("NcMdvServer");
  
  if (_debug) {
    cerr << "convertNcf2Mdv(): NCF to MDV" << endl;
    cerr << "  URL: " << trans_url.getURLStr() << endl;
  }

  DsLOCATOR locator;
  bool contact_server;
  if (locator.resolve(trans_url, &contact_server, false)) {
    _errStr += "ERROR - COMM - DsMdvx::convertNcf2Mdv.\n";
    _errStr += "  Cannot resolve URL: ";
    _errStr += trans_url.getURLStr();
    _errStr += "\n";
    return -1;
  }
  
  // assemble message packet

  DsMdvxMsg msg;
  if (_debug) {
    msg.setDebug();
  }
  void *msgBuf =
    msg.assembleConvertNcf2Mdv(*this, trans_url.getURLStr());
  if (msgBuf == NULL) {
    _errStr += "ERROR - DsMdvx::convertNcf2Mdv.\n";
    _errStr += "  Assembling outgoing message, URL:\n";
    _errStr += trans_url.getURLStr();
    _errStr += "\n";
    return -1;
  }

  // communicate with server

  if (_communicate(trans_url, msg, msgBuf, msg.lengthAssembled())) {
    _errStr += "ERROR - COMM - DsMdvx::convertNcf2Mdv.\n";
    _errStr += "  Communicating with server\n";
    return -1;
  }

  if (msg.getError()) {
    return -1;
  }

  if (msg.getSubType() != DsMdvxMsg::MDVP_CONVERT_NCF_TO_MDV) {
    _errStr += "ERROR - DsMdvx::convertNcf2Mdv.\n";
    TaStr::AddInt(_errStr, "  Incorrect return subType: ", msg.getSubType());
    return -1;
  }

  // convert the output fields appropriately

  for (int ii = 0; ii < (int) _fields.size(); ii++) {
    _fields[ii]->convertType(readEncodingType,
                             readCompressionType,
                             readScalingType,
                             readScale, readBias);
  }

  return 0;

}

//////////////////////////////////////////////////////
// Read the headers from a NETCDF CF file into MDV, given the file path
// Convert to NCF at the end if required
// Currently we read all of the data, which will include the headers.
// returns 0 on success, -1 on failure

int DsMdvx::readAllHeadersNcf(const string &url)
  
{

  // First read in the whole file.  We do this because we don't have a way
  // to easily read the field header information without reading the field
  // data also.

  if (readNcf(url))
  {
    _errStr += "ERROR - DsMdvx::readAllHeadersNcf\n";
    TaStr::AddStr(_errStr, "  Error reading NCF file and translating to MDV");
    return -1;
  }
  
  // Now fill in the file headers.  This isn't done in the translation because
  // the translation doesn't know that we are reading the entire file to get
  // the headers.

  _mhdrFile = _mhdr;
  
  for (size_t i = 0; i < _fields.size(); ++i)
  {
    MdvxField *field = _fields[i];
    
    _fhdrsFile.push_back(field->getFieldHeader());
    _vhdrsFile.push_back(field->getVlevelHeader());
  }
  
  for (size_t i = 0; i < _chunks.size(); ++i)
  {
    MdvxChunk *chunk = _chunks[i];
    
    _chdrsFile.push_back(chunk->getHeader());
  }
  
  return 0;
}

////////////////////////////////////////////////
// read the headers from a NCF file, using the NcMdvServer
// deprecated
// returns 0 on success, -1 on failure

int DsMdvx::_readAllHeadersNcfViaServer(const string &url)
  
{

  if (_currentFormat != FORMAT_NCF) {
    _errStr += "ERROR - DsMdvx::readAllHeadersNcf.\n";
    TaStr::AddStr(_errStr, "  Incorrect format: ", format2Str(_currentFormat));
    TaStr::AddStr(_errStr, "  Should be: ", format2Str(FORMAT_NCF));
    return -1;
  }
  
  // set up URL for doing conversion via NcMdvServer
  
  DsURL trans_url(url);
  trans_url.setProtocol("mdvp");
  trans_url.setTranslator("NcMdvServer");
  
  if (_debug) {
    cerr << "readAllHeadersNcf(): Reading headers from NCF-type file" << endl;
    cerr << "  URL: " << trans_url.getURLStr() << endl;
  }

  DsLOCATOR locator;
  bool contact_server;
  if (locator.resolve(trans_url, &contact_server, false)) {
    _errStr += "ERROR - COMM - DsMdvx::readAllHeadersNcf\n";
    _errStr += "  Cannot resolve URL: ";
    _errStr += trans_url.getURLStr();
    _errStr += "\n";
    return -1;
  }
  
  // assemble message packet
  
  DsMdvxMsg msg;
  if (_debug) {
    msg.setDebug();
  }
  void *msgBuf =
    msg.assembleReadAllHdrsNcf(*this, trans_url.getURLStr());
  if (msgBuf == NULL) {
    _errStr += "ERROR - DsMdvx::readAllHeadersNcf\n";
    _errStr += "  Assembling outgoing message, URL:\n";
    _errStr += trans_url.getURLStr();
    _errStr += "\n";
    return -1;
  }

  // communicate with server
  
  if (_communicate(trans_url, msg, msgBuf, msg.lengthAssembled())) {
    _errStr += "ERROR - COMM - DsMdvx::readAllHeadersNcf\n";
    _errStr += "  Communicating with server\n";
    return -1;
  }

  if (msg.getError()) {
    return -1;
  }

  if (msg.getSubType() != DsMdvxMsg::MDVP_READ_ALL_HDRS_NCF) {
    _errStr += "ERROR - DsMdvx::readAllHeadersNcf\n";
    TaStr::AddInt(_errStr, "  Incorrect return subType: ", msg.getSubType());
    return -1;
  }
  
  return 0;
  
}

//////////////////////////////////////////////////////
// Read a NETCDF CF file into MDV, given the file path
// Convert to NCF at the end if required
// returns 0 on success, -1 on failure

int DsMdvx::readNcf(const string &url)
  
{

  if (_currentFormat != FORMAT_NCF) {
    _errStr += "ERROR - DsMdvx::readNcf\n";
    TaStr::AddStr(_errStr, "  Url: ", url);
    TaStr::AddStr(_errStr, "  Path ", _pathInUse);
    TaStr::AddStr(_errStr, "  Incorrect format: ", format2Str(_currentFormat));
    TaStr::AddStr(_errStr, "  Should be: ", format2Str(FORMAT_NCF));
    return -1;
  }

  // save read details
  
  Mdvx::encoding_type_t readEncodingType = _readEncodingType;
  Mdvx::compression_type_t readCompressionType = _readCompressionType;
  Mdvx::scaling_type_t readScalingType = _readScalingType;
  double readScale = _readScale;
  double readBias = _readBias;

  // create translator
  
  Ncf2MdvTrans trans;
  trans.setDebug(_debug);
  
  // perform translation
  // returns 0 on success, -1 on failure
  
  if (trans.translate(_pathInUse, *this)) {
    _errStr += "ERROR - DsMdvx::readNcf\n";
    TaStr::AddStr(_errStr, "  Url: ", url);
    TaStr::AddStr(_errStr, "  Path ", _pathInUse);
    TaStr::AddStr(_errStr, "  Cannot translate file to MDV");
    TaStr::AddStr(_errStr, trans.getErrStr());
    return -1;
  }

  // remove netcdf data
  
  clearNcf();

  // set nc-specific times from path

  _set_times_ncf();

  // set format to MDV
  
  _currentFormat = FORMAT_MDV;

  // convert back to NCF if needed
  // with read constraints having been applied
  
  if (_readFormat == FORMAT_NCF) {
    if (convertMdv2Ncf(_pathInUse)) {
      _errStr += "ERROR - DsMdvx::readNcf\n";
      TaStr::AddStr(_errStr, "  Url: ", url);
      TaStr::AddStr(_errStr, "  Path ", _pathInUse);
      TaStr::AddStr(_errStr, "  Cannot translate file to NCF");
      TaStr::AddStr(_errStr, trans.getErrStr());
      return -1;
    }
  }

  // for MDV format, convert the output fields appropriately
  
  if (_currentFormat == FORMAT_MDV) {
    for (int ii = 0; ii < (int) _fields.size(); ii++) {
      _fields[ii]->convertType(readEncodingType,
                               readCompressionType,
                               readScalingType,
                               readScale, readBias);
    }
  }

  return 0;

}

////////////////////////////////////////////////
// read NCF file, using the NcMdvServer
// deprecated
// returns 0 on success, -1 on failure

int DsMdvx::_readNcfViaServer(const string &url)
  
{
  
  if (_currentFormat != FORMAT_NCF) {
    _errStr += "ERROR - DsMdvx::readNcf.\n";
    TaStr::AddStr(_errStr, "  Incorrect format: ", format2Str(_currentFormat));
    TaStr::AddStr(_errStr, "  Should be: ", format2Str(FORMAT_NCF));
    return -1;
  }
  
  // save read details
  
  Mdvx::encoding_type_t readEncodingType = _readEncodingType;
  Mdvx::compression_type_t readCompressionType = _readCompressionType;
  Mdvx::scaling_type_t readScalingType = _readScalingType;
  double readScale = _readScale;
  double readBias = _readBias;

  // set up URL for doing conversion via NcMdvServer
  
  DsURL trans_url(url);
  trans_url.setProtocol("mdvp");
  trans_url.setTranslator("NcMdvServer");
  
  if (_debug) {
    cerr << "readNcf(): Reading NCF-type file" << endl;
    cerr << "  URL: " << trans_url.getURLStr() << endl;
  }

  DsLOCATOR locator;
  bool contact_server;
  if (locator.resolve(trans_url, &contact_server, false)) {
    _errStr += "ERROR - COMM - DsMdvx::readNcf\n";
    _errStr += "  Cannot resolve URL: ";
    _errStr += trans_url.getURLStr();
    _errStr += "\n";
    return -1;
  }
  
  // assemble message packet
  
  DsMdvxMsg msg;
  if (_debug) {
    msg.setDebug();
  }
  void *msgBuf =
    msg.assembleReadNcf(*this, trans_url.getURLStr());
  if (msgBuf == NULL) {
    _errStr += "ERROR - DsMdvx::readNcf\n";
    _errStr += "  Assembling outgoing message, URL:\n";
    _errStr += trans_url.getURLStr();
    _errStr += "\n";
    return -1;
  }

  // communicate with server
  
  if (_communicate(trans_url, msg, msgBuf, msg.lengthAssembled())) {
    _errStr += "ERROR - COMM - DsMdvx::readNcf\n";
    _errStr += "  Communicating with server\n";
    return -1;
  }

  if (msg.getError()) {
    return -1;
  }

  if (msg.getSubType() != DsMdvxMsg::MDVP_READ_NCF) {
    _errStr += "ERROR - DsMdvx::readNcf\n";
    TaStr::AddInt(_errStr, "  Incorrect return subType: ", msg.getSubType());
    return -1;
  }
  
  // for MDV format, convert the output fields appropriately
  
  if (_currentFormat == FORMAT_MDV) {
    for (int ii = 0; ii < (int) _fields.size(); ii++) {
      _fields[ii]->convertType(readEncodingType,
                               readCompressionType,
                               readScalingType,
                               readScale, readBias);
    }
  }

  return 0;
  
}

//////////////////////////////////////////////////////
// Read the metadata from a RADX file, given the file path
// Fill out the Mdv file headers
// returns 0 on success, -1 on failure

int DsMdvx::readAllHeadersRadx(const string &url)
  
{

  // set up object for reading file
  
  RadxFile inFile;
  if (_debug) {
    inFile.setDebug(true);
  }

  inFile.setReadMetadataOnly(true);
  inFile.setReadRemoveLongRange(true);
  
  // read file in to RadxVol object
  
  RadxVol vol;
  if (inFile.readFromPath(_pathInUse, vol)) {
    _errStr += "ERROR - DsMdvx::readAllHeadersRadx.\n";
    _errStr += "Cannot read in files.\n";
    TaStr::AddStr(_errStr, "  path: ", _pathInUse);
    _errStr += inFile.getErrStr();
    return -1;
  }

  // read in first field with data

  RadxVol vol0;
  if (vol.getNFields() > 0) {
    RadxField *rfld = vol.getField(0);
    string firstFieldName = rfld->getName();
    RadxFile inFile0;
    inFile0.addReadField(firstFieldName);
    if (inFile0.readFromPath(_pathInUse, vol0) == 0) {
      vol0.computeMaxNGates();
    }
  }

  // make sure sweeps are in ascending order, as required by MDV

  vol.reorderSweepsAsInFileAscendingAngle();
  vol.reorderSweepsAscendingAngle();

  // set format to MDV
  
  _currentFormat = FORMAT_MDV;

  // Now fill in the file headers.  This isn't done in the translation because
  // the translation doesn't know that we are reading the entire file to get
  // the headers.

  clear();

  // set master header

  setBeginTime(vol.getStartTimeSecs());
  setEndTime(vol.getEndTimeSecs());
  setValidTime(vol.getEndTimeSecs());
  setDataCollectionType(Mdvx::DATA_MEASURED);
  _mhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
  _mhdr.vlevel_type = Mdvx::VERT_TYPE_ELEV;
  _mhdr.n_fields = vol.getNFields();
  
  _mhdr.sensor_lat = vol.getLatitudeDeg();
  _mhdr.sensor_lon = vol.getLongitudeDeg();
  _mhdr.sensor_alt = vol.getAltitudeKm();

  _mhdr.max_nx = 0;
  _mhdr.max_ny = 0;
  _mhdr.max_nz = 0;

  const vector<RadxSweep *> &sweepsInFile = vol.getSweepsAsInFile();

  // add field headers

  for (size_t ii = 0; ii < vol.getNFields(); ii++) {

    RadxField *rfld = vol.getField(ii);
    Mdvx::field_header_t fhdr;
    Mdvx::vlevel_header_t vhdr;
    MEM_zero(fhdr);
    MEM_zero(vhdr);

    fhdr.nx = vol0.getMaxNGates();
    fhdr.ny = vol0.getNRays();
    fhdr.nz = sweepsInFile.size();
    
    if (fhdr.nx > _mhdr.max_nx) _mhdr.max_nx = fhdr.nx;
    if (fhdr.ny > _mhdr.max_ny) _mhdr.max_ny = fhdr.ny;
    if (fhdr.nz > _mhdr.max_nz) _mhdr.max_nz = fhdr.nz;

    fhdr.proj_type = Mdvx::PROJ_POLAR_RADAR;
    fhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
    fhdr.vlevel_type = Mdvx::VERT_TYPE_ELEV;
    fhdr.dz_constant = false;

    fhdr.proj_origin_lat = vol.getLatitudeDeg();
    fhdr.proj_origin_lon = vol.getLongitudeDeg();

    fhdr.grid_dx = 0.0;
    fhdr.grid_dy = 0.0;
    fhdr.grid_dz = 0.0;
    
    fhdr.grid_minx = 0.0;
    fhdr.grid_miny = 0.0;
    fhdr.grid_minz = 0.0;

    if (vol0.getNRays() > 0) {
      const RadxRay *ray0 = vol0.getRays()[0];
      fhdr.grid_dx = ray0->getGateSpacingKm();
      fhdr.grid_minx = ray0->getStartRangeKm();
      if (vol0.checkIsRhi()) {
        fhdr.grid_miny = ray0->getElevationDeg();
      } else {
        fhdr.grid_miny = ray0->getAzimuthDeg();
      }
    }
    
    if (vol0.getNSweeps() > 0) {
      const RadxSweep *sweep0 = vol0.getSweeps()[0];
      fhdr.grid_dy = sweep0->getAngleResDeg();
      fhdr.grid_minz = sweep0->getFixedAngleDeg();
    }
    
    STRncopy(fhdr.field_name_long,
             rfld->getLongName().c_str(), MDV_LONG_FIELD_LEN);
    
    STRncopy(fhdr.field_name,
             rfld->getName().c_str(), MDV_SHORT_FIELD_LEN);

    STRncopy(fhdr.units,
             rfld->getUnits().c_str(), MDV_UNITS_LEN);

    for (size_t jj = 0; jj < sweepsInFile.size(); jj++) {
      const RadxSweep *swp = sweepsInFile[jj];
      if (swp->getSweepMode() == Radx::SWEEP_MODE_RHI) {
        fhdr.native_vlevel_type = Mdvx::VERT_TYPE_AZ;
        fhdr.vlevel_type = Mdvx::VERT_TYPE_AZ;
        vhdr.type[jj] = Mdvx::VERT_TYPE_AZ;
      } else {
        vhdr.type[jj] = Mdvx::VERT_TYPE_ELEV;
      }
      vhdr.level[jj] = swp->getFixedAngleDeg();
    }
    
    _fhdrsFile.push_back(fhdr);
    _vhdrsFile.push_back(vhdr);

  } // ii
  
  _mhdrFile = _mhdr;

  return 0;

}

////////////////////////////////////////////////
// read the headers from a RADX file, using the NcMdvServer
// deprecated
// returns 0 on success, -1 on failure

int DsMdvx::_readAllHeadersRadxViaServer(const string &url)
  
{

  if (_currentFormat != FORMAT_RADX) {
    _errStr += "ERROR - DsMdvx::readAllHeadersRadx.\n";
    TaStr::AddStr(_errStr, "  Incorrect format: ", format2Str(_currentFormat));
    TaStr::AddStr(_errStr, "  Should be: ", format2Str(FORMAT_RADX));
    return -1;
  }
  
  // set up URL for doing conversion via NcMdvServer
  
  DsURL trans_url(url);
  trans_url.setProtocol("mdvp");
  trans_url.setTranslator("NcMdvServer");
  
  if (_debug) {
    cerr << "readAllHeadersRadx(): Reading headers from RADX-type file" << endl;
    cerr << "  URL: " << trans_url.getURLStr() << endl;
  }

  DsLOCATOR locator;
  bool contact_server;
  if (locator.resolve(trans_url, &contact_server, false)) {
    _errStr += "ERROR - COMM - DsMdvx::readAllHeadersRadx\n";
    _errStr += "  Cannot resolve URL: ";
    _errStr += trans_url.getURLStr();
    _errStr += "\n";
    return -1;
  }
  
  // assemble message packet
  
  DsMdvxMsg msg;
  if (_debug) {
    msg.setDebug();
  }
  void *msgBuf =
    msg.assembleReadAllHdrsRadx(*this, trans_url.getURLStr());
  if (msgBuf == NULL) {
    _errStr += "ERROR - DsMdvx::readAllHeadersRadx\n";
    _errStr += "  Assembling outgoing message, URL:\n";
    _errStr += trans_url.getURLStr();
    _errStr += "\n";
    return -1;
  }

  // communicate with server
  
  if (_communicate(trans_url, msg, msgBuf, msg.lengthAssembled())) {
    _errStr += "ERROR - COMM - DsMdvx::readAllHeadersRadx\n";
    _errStr += "  Communicating with server\n";
    return -1;
  }

  if (msg.getError()) {
    return -1;
  }

  if (msg.getSubType() != DsMdvxMsg::MDVP_READ_ALL_HDRS_RADX) {
    _errStr += "ERROR - DsMdvx::readAllHeadersRadx\n";
    TaStr::AddInt(_errStr, "  Incorrect return subType: ", msg.getSubType());
    return -1;
  }
  
  return 0;
  
}

////////////////////////////////////////////////
// Read a RADX-type file, convert to MDV
// Convert to RADX at the end if required
// returns 0 on success, -1 on failure

int DsMdvx::readRadx(const string &url)
  
{

  if (_currentFormat != FORMAT_RADX) {
    _errStr += "ERROR - DsMdvx::readRadx.\n";
    TaStr::AddStr(_errStr, "  Url: ", url);
    TaStr::AddStr(_errStr, "  Path: ", _pathInUse);
    TaStr::AddStr(_errStr, "  Incorrect format: ", format2Str(_currentFormat));
    TaStr::AddStr(_errStr, "  Should be: ", format2Str(FORMAT_RADX));
    return -1;
  }

  // save read details
  
  Mdvx::encoding_type_t readEncodingType = _readEncodingType;
  Mdvx::compression_type_t readCompressionType = _readCompressionType;
  Mdvx::scaling_type_t readScalingType = _readScalingType;
  double readScale = _readScale;
  double readBias = _readBias;

  // set up object for reading files
  
  RadxFile inFile;
  if (_debug) {
    inFile.setDebug(true);
  }
  // set vertical limits
  if (_readVlevelLimitsSet) {
    inFile.setReadFixedAngleLimits(_readMinVlevel, _readMaxVlevel);
    inFile.setReadStrictAngleLimits(false);
  } else if (_readPlaneNumLimitsSet) {
    inFile.setReadSweepNumLimits(_readMinPlaneNum, _readMaxPlaneNum);
    inFile.setReadStrictAngleLimits(false);
  }
  // set field names
  if (_readFieldNames.size() > 0) {
    for (size_t ii = 0; ii < _readFieldNames.size(); ii++) {
      inFile.addReadField(_readFieldNames[ii]);
    }
  }

  // ignore rays with antenna transitions
  // inFile.setReadIgnoreTransitions(true);

  // remove long-range rays from NEXRAD
  // inFile.setReadRemoveLongRange(true);

  // read file in to RadxVol object
  
  RadxVol vol;
  if (inFile.readFromPath(_pathInUse, vol)) {
    _errStr += "ERROR - DsMdvx::readRadx.\n";
    _errStr += "Cannot read in files.\n";
    TaStr::AddStr(_errStr, "  path: ", _pathInUse);
    _errStr += inFile.getErrStr();
    return -1;
  }
  
  // make sure sweeps are in ascending order, as required by MDV
  
  vol.reorderSweepsAsInFileAscendingAngle();
  vol.reorderSweepsAscendingAngle();

  // make sure gate geom is constant

  vol.remapToPredomGeom();
  
  // convert into Mdv
  
  Ncf2MdvTrans trans;
  trans.setDebug(_debug);
  if (trans.translateRadxVol(_pathInUse, vol, *this)) {
    _errStr += "ERROR - DsMdvx::readRadx.\n";
    _errStr += "  Cannot translate RadxVol.\n";
    return -1;
  }

  // set format to MDV
  
  _currentFormat = FORMAT_MDV;

  // convert to NCF if needed
  
  if (_readFormat == FORMAT_NCF) {
    if (convertMdv2Ncf(_pathInUse)) {
      _errStr += "ERROR - DsMdvx::readRadx\n";
      TaStr::AddStr(_errStr, "  Url: ", url);
      TaStr::AddStr(_errStr, "  Path ", _pathInUse);
      TaStr::AddStr(_errStr, "  Cannot translate file to NCF");
      TaStr::AddStr(_errStr, trans.getErrStr());
      return -1;
    }
  }

  // for MDV format, convert the output fields appropriately

  if (_currentFormat == FORMAT_MDV) {
    for (int ii = 0; ii < (int) _fields.size(); ii++) {
      _fields[ii]->convertType(readEncodingType,
                               readCompressionType,
                               readScalingType,
                               readScale, readBias);
    }
  }

  return 0;

}

////////////////////////////////////////////////
// read RADX file, using the NcMdvServer
// deprecated
// returns 0 on success, -1 on failure

int DsMdvx::_readRadxViaServer(const string &url)
  
{
  
  if (_currentFormat != FORMAT_RADX) {
    _errStr += "ERROR - DsMdvx::readRadx.\n";
    TaStr::AddStr(_errStr, "  Incorrect format: ", format2Str(_currentFormat));
    TaStr::AddStr(_errStr, "  Should be: ", format2Str(FORMAT_RADX));
    return -1;
  }
  
  // save read details
  
  Mdvx::encoding_type_t readEncodingType = _readEncodingType;
  Mdvx::compression_type_t readCompressionType = _readCompressionType;
  Mdvx::scaling_type_t readScalingType = _readScalingType;
  double readScale = _readScale;
  double readBias = _readBias;

  // set up URL for doing conversion via NcMdvServer
  
  DsURL trans_url(url);
  trans_url.setProtocol("mdvp");
  trans_url.setTranslator("NcMdvServer");
  
  if (_debug) {
    cerr << "readRadx(): Reading RADX-type file" << endl;
    cerr << "  URL: " << trans_url.getURLStr() << endl;
  }

  DsLOCATOR locator;
  bool contact_server;
  if (locator.resolve(trans_url, &contact_server, false)) {
    _errStr += "ERROR - COMM - DsMdvx::readRadx\n";
    _errStr += "  Cannot resolve URL: ";
    _errStr += trans_url.getURLStr();
    _errStr += "\n";
    return -1;
  }
  
  // assemble message packet
  
  DsMdvxMsg msg;
  if (_debug) {
    msg.setDebug();
  }
  void *msgBuf =
    msg.assembleReadRadx(*this, trans_url.getURLStr());
  if (msgBuf == NULL) {
    _errStr += "ERROR - DsMdvx::readRadx\n";
    _errStr += "  Assembling outgoing message, URL:\n";
    _errStr += trans_url.getURLStr();
    _errStr += "\n";
    return -1;
  }

  // communicate with server
  
  if (_communicate(trans_url, msg, msgBuf, msg.lengthAssembled())) {
    _errStr += "ERROR - COMM - DsMdvx::readRadx\n";
    _errStr += "  Communicating with server\n";
    return -1;
  }

  if (msg.getError()) {
    return -1;
  }

  if (msg.getSubType() != DsMdvxMsg::MDVP_READ_RADX) {
    _errStr += "ERROR - DsMdvx::readRadx\n";
    TaStr::AddInt(_errStr, "  Incorrect return subType: ", msg.getSubType());
    return -1;
  }
  
  // for MDV format, convert the output fields appropriately

  if (_currentFormat == FORMAT_MDV) {
    for (int ii = 0; ii < (int) _fields.size(); ii++) {
      _fields[ii]->convertType(readEncodingType,
                               readCompressionType,
                               readScalingType,
                               readScale, readBias);
    }
  }

  return 0;

}

////////////////////////////////////////////////
// write NCF file, using the NcMdvServer
// returns 0 on success, -1 on failure

int DsMdvx::_writeNcfToDir(const string &url)
                      
{
  
  // set up URL for doing conversion via NcMdvServer
  
  DsURL trans_url(url);
  trans_url.setProtocol("mdvp");
  trans_url.setTranslator("NcMdvServer");
  
  if (_debug) {
    cerr << "_writeNcfToDir(): Reading NCF-type file" << endl;
    cerr << "  URL: " << trans_url.getURLStr() << endl;
  }

  DsLOCATOR locator;
  bool contact_server;
  if (locator.resolve(trans_url, &contact_server, false)) {
    _errStr += "ERROR - COMM - DsMdvx::_writeNcfToDir\n";
    _errStr += "  Cannot resolve URL: ";
    _errStr += trans_url.getURLStr();
    _errStr += "\n";
    return -1;
  }
  
  // assemble message packet
  
  DsMdvxMsg msg;
  if (_debug) {
    msg.setDebug();
  }
  void *msgBuf =
    msg.assembleWrite(DsMdvxMsg::MDVP_WRITE_TO_DIR, *this, trans_url.getURLStr());

  if (msgBuf == NULL) {
    _errStr += "ERROR - DsMdvx::_writeNcfToDir\n";
    _errStr += "  Assembling outgoing message, URL:\n";
    _errStr += trans_url.getURLStr();
    _errStr += "\n";
    return -1;
  }

  // communicate with server
  
  if (_communicate(trans_url, msg, msgBuf, msg.lengthAssembled())) {
    _errStr += "ERROR - COMM - DsMdvx::_writeNcfToDir\n";
    _errStr += "  Communicating with server\n";
    return -1;
  }
  
  if (msg.getError()) {
    return -1;
  }

  if (msg.getSubType() != DsMdvxMsg::MDVP_WRITE_TO_DIR) {
    _errStr += "ERROR - DsMdvx::_writeNcfToDir\n";
    TaStr::AddInt(_errStr, "  Incorrect return subType: ", msg.getSubType());
    _errStr += "  Should be: MDVP_WRITE_TO_DIR\n";
    return -1;
  }
  
  return 0;
  
}

////////////////////////////////////////////////
// constrain NETCDF CF using read qualifiers
// returns 0 on success, -1 on failure

int DsMdvx::constrainNcf(const string &url)
  
{

  // convert the NCF data to MDV format
  // This will also apply the read qualifiers
  
  if (convertNcf2Mdv(url)) {
    _errStr += "ERROR - DsMdvx::constrainNcf.\n";
    TaStr::AddStr(_errStr, "  Url: ", url);
    return -1;
  }
    
  // convert back to NCF

  if (convertMdv2Ncf(url)) {
    _errStr += "ERROR - DsMdvx::constrainNcf.\n";
    TaStr::AddStr(_errStr, "  Url: ", url);
    return -1;
  }
  
  return 0;

}

/////////////////////////////////////////////////////////////
// constrain NETCDF CF using read qualifiers, via NcMdvServer
// deprecated
// Used when and NC file is read, and NC data is requested
// returns 0 on success, -1 on failure

int DsMdvx::_constrainNcfViaServer(const string &url)
  
{

  if (_currentFormat != FORMAT_NCF) {
    _errStr += "ERROR - DsMdvx::constrainNcf.\n";
    TaStr::AddStr(_errStr, "  Incorrect format: ", format2Str(_currentFormat));
    TaStr::AddStr(_errStr, "  Should be: ", format2Str(FORMAT_NCF));
    return -1;
  }
  
  // set up URL for doing conversion via NcMdvServer
  
  DsURL trans_url(url);
  trans_url.setTranslator("NcMdvServer");
  
  if (_debug) {
    cerr << "constrainNcf(): NCF to MDV to NCF" << endl;
    cerr << "  URL: " << trans_url.getURLStr() << endl;
  }

  DsLOCATOR locator;
  bool contact_server;
  if (locator.resolve(trans_url, &contact_server, false)) {
    _errStr += "ERROR - COMM - DsMdvx::constrainNcf.\n";
    _errStr += "  Cannot resolve URL: ";
    _errStr += trans_url.getURLStr();
    _errStr += "\n";
    return -1;
  }
  
  // assemble message packet

  DsMdvxMsg msg;
  if (_debug) {
    msg.setDebug();
  }
  void *msgBuf =
    msg.assembleConstrainNcf(*this, trans_url.getURLStr());
  if (msgBuf == NULL) {
    _errStr += "ERROR - DsMdvx::constrainNcf.\n";
    _errStr += "  Assembling outgoing message, URL:\n";
    _errStr += trans_url.getURLStr();
    _errStr += "\n";
    return -1;
  }

  // communicate with server
  
  if (_communicate(trans_url, msg, msgBuf, msg.lengthAssembled())) {
    _errStr += "ERROR - COMM - DsMdvx::constrainNcf.\n";
    _errStr += "  Communicating with server\n";
    return -1;
  }
  
  if (msg.getError()) {
    return -1;
  }

  if (msg.getSubType() != DsMdvxMsg::MDVP_CONSTRAIN_NCF) {
    _errStr += "ERROR - DsMdvx::constrainNcf.\n";
    TaStr::AddInt(_errStr, "  Incorrect return subType: ", msg.getSubType());
    return -1;
  }

  _ncfConstrained = true;

  return 0;

}

///////////////////////////////////////
// print mdv to ncf convert request

void DsMdvx::printConvertMdv2NcfRequest(ostream &out)

{

  out << "DsMdvx convert MDV to NCF request" << endl;
  out << "---------------------------------" << endl;

  cerr << "  Institution: " << _ncfInstitution << endl;
  cerr << "  References: " << _ncfReferences << endl;
  cerr << "  Comment: " << _ncfComment << endl;
  cerr << "  Compress? " << (_ncfCompress? "Y" : "N") << endl;
  cerr << "  Compression level: " << _ncfCompressionLevel << endl;
  cerr << "  NC format: " << ncFormat2Str(_ncfFileFormat) << endl;
  cerr << "  Polar radar file type: "
       << radialFileType2Str(_ncfRadialFileType) << endl;
  cerr << "  OutputLatlonArrays? "
       << (_ncfOutputLatlonArrays? "Y" : "N") << endl;
  cerr << "  OutputMdvAttr? "
       << (_ncfOutputMdvAttr? "Y" : "N") << endl;
  cerr << "  OutputMdvChunks? "
       << (_ncfOutputMdvChunks? "Y" : "N") << endl;
  cerr << "  OutputStartEndTimes? "
       << (_ncfOutputStartEndTimes? "Y" : "N") << endl;

  cerr << endl;

  for (int ii = 0; ii < (int) _mdv2NcfTransArray.size(); ii++) {
    const Mdv2NcfFieldTrans &trans = _mdv2NcfTransArray[ii];
    cerr << "  ------------------" << endl;
    cerr << "  Field translation:" << endl;
    cerr << "    mdvFieldName: " << trans.mdvFieldName << endl;
    cerr << "    ncfFieldName: " << trans.ncfFieldName << endl;
    cerr << "    ncfStandardName: " << trans.ncfStandardName << endl;
    cerr << "    ncfLongName: " << trans.ncfLongName << endl;
    cerr << "    ncfUnits: " << trans.ncfUnits << endl;
    cerr << "    doLinearTransform? "
         << (trans.doLinearTransform? "Y" : "N") << endl;
    cerr << "    linearMult: " << trans.linearMult << endl;
    cerr << "    linearOffset: " << trans.linearOffset << endl;
    cerr << "    packing: " << ncfPack2Str(trans.packing) << endl;
  }
  cerr << endl;

}

/////////////////////////////////////////////////////////
// write as forecast?

bool DsMdvx::_getWriteAsForecast()
{
  
  if (_ifForecastWriteAsForecast) {
    if (_currentFormat == FORMAT_NCF) {
      if (_ncfIsForecast) {
        return true;
      }
    } else {
      if (_mhdr.data_collection_type == Mdvx::DATA_FORECAST ||
          _mhdr.data_collection_type == Mdvx::DATA_EXTRAPOLATED) {
        return true;
      }
    }
  }

  return _writeAsForecast;

}
  
/////////////////////////////////////////////////////////
// compute output path for Ncf files

string DsMdvx::_computeNcfOutputPath(const string &outputDir)
{
  
  // check environment for write control directives
  
  _checkEnvBeforeWrite();

  char *writeCompStr = getenv("MDV_NCF_COMPRESSION_LEVEL");
  if (writeCompStr != NULL) {
    int level = atoi(writeCompStr);
    _ncfCompress = true;
    _ncfCompressionLevel = level;
  }
  
  // compute path
  
  char yearSubdir[MAX_PATH_LEN];
  char outputBase[MAX_PATH_LEN];
  int forecastDelta = getForecastLeadSecs();
  bool writeAsForecast = _getWriteAsForecast();
  
  if (writeAsForecast) {
    
    if (_mhdr.data_collection_type != Mdvx::DATA_FORECAST && 
        _mhdr.data_collection_type != Mdvx::DATA_EXTRAPOLATED)
        _mhdr.data_collection_type = Mdvx::DATA_FORECAST;
 
    date_time_t genTime;
    genTime.unix_time = getGenTime();
    uconvert_from_utime(&genTime);
    
    sprintf(yearSubdir, "%.4d", genTime.year);

    if (!_useExtendedPaths) {
      sprintf(outputBase, "%.4d%.2d%.2d%sg_%.2d%.2d%.2d%sf_%.8d",
              genTime.year, genTime.month, genTime.day,
              PATH_DELIM, genTime.hour, genTime.min, genTime.sec,
              PATH_DELIM, forecastDelta);
    } else {
      sprintf(outputBase,
              "%.4d%.2d%.2d%s"
              "g_%.2d%.2d%.2d%s"
              "%.4d%.2d%.2d_g_%.2d%.2d%.2d_f_%.8d",
              genTime.year, genTime.month, genTime.day, PATH_DELIM,
              genTime.hour, genTime.min, genTime.sec, PATH_DELIM,
              genTime.year, genTime.month, genTime.day,
              genTime.hour, genTime.min, genTime.sec,
              forecastDelta);
    }

  } else {

    date_time_t validTime;
    validTime.unix_time = getValidTime();
    uconvert_from_utime(&validTime);
    sprintf(yearSubdir, "%.4d", validTime.year);
    if (!_useExtendedPaths) {
      sprintf(outputBase, "%.4d%.2d%.2d%s%.2d%.2d%.2d",
              validTime.year, validTime.month, validTime.day,
              PATH_DELIM, validTime.hour, validTime.min, validTime.sec);
    } else {
      sprintf(outputBase,
              "%.4d%.2d%.2d%s"
              "%.4d%.2d%.2d_%.2d%.2d%.2d",
              validTime.year, validTime.month, validTime.day, PATH_DELIM,
              validTime.year, validTime.month, validTime.day,
              validTime.hour, validTime.min, validTime.sec);
    }

  }

  string outputName;
  if (_writeAddYearSubdir) {
    outputName += yearSubdir;
    outputName += PATH_DELIM;
  }
  outputName += outputBase;
  outputName += ".mdv.nc";

  string outputPath(outputDir);
  outputPath += PATH_DELIM;
  outputPath += outputName;

  return outputPath;

}

////////////////////////////////////////////////
// write to specified directory, locally
// returns 0 on success, -1 on failure

int DsMdvx::_writeToDirLocal(const string &url)
  
{
  
  if (_debug) {
    cerr << "WRITE TO DIR" << endl;
    printWriteOptions(cerr);
    cerr << "  current format: " << format2Str(_currentFormat) << endl;
    cerr << "  write format: " << format2Str(_writeFormat) << endl;
  }

  if (_currentFormat == FORMAT_NCF && _writeFormat == FORMAT_NCF) {
    // NCF to NCF - apply constraints
    if(_constrainNcfAndWrite(url)) {
      _errStr += "ERROR - DsMdvx::_writeToDirLocal\n";
      return -1;
    } else {
      return 0;
    }
  }

  if (_currentFormat == FORMAT_NCF && _writeFormat == FORMAT_MDV) {
    // convert NCF to MDV and write
    if (_convertNcfToMdvAndWrite(url)) {
      _errStr += "ERROR - DsMdvx::_writeToDirLocal\n";
      return -1;
    } else {
      return 0;
    }
  }

  if (_currentFormat == FORMAT_MDV && _writeFormat == FORMAT_NCF) {
    // convert MDV to NCF and write
    if (_convertMdvToNcfAndWrite(url)) {
      _errStr += "ERROR - DsMdvx::_writeToDirLocal\n";
      return -1;
    } else {
      return 0;
    }
  }

  // MDV to MDV

  if (_writeAsMdv(url)) {
    _errStr += "ERROR - DsMdvx::_writeToDirLocal\n";
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////
// Convert NCF format to MDV format, and write
// returns 0 on success, -1 on failure

int DsMdvx::_convertNcfToMdvAndWrite(const string &url)
  
{
  
  if (convertNcf2Mdv(url)) {
    _errStr += "ERROR - DsMdvx::_convertNcfToMdvAndWrite()\n";
    return -1;
  }

  // local - direct write
  DsURL dsUrl(url);
  string writeDir(dsUrl.getFile());
  if (Mdvx::writeToDir(writeDir)) {
    _errStr += "ERROR - DsMdvx::_convertNcfToMdvAndWrite()\n";
    return -1;
  }
    
  // reg with data mapper - the base class uses LdataInfo which does
  // not register

  DmapAccess dmap;
  string dataType = "mdv";
  if (_writeAsForecast) {
    int forecast_delta = _mhdr.time_centroid - _mhdr.time_gen;
    dmap.regLatestInfo(_mhdr.time_gen, writeDir, dataType, forecast_delta);
  } else {
    dmap.regLatestInfo(_mhdr.time_centroid, writeDir, dataType);
  }
  
  return 0;

}

////////////////////////////////////////////////
// Convert MDV format to NCF format, and write
// returns 0 on success, -1 on failure

int DsMdvx::_convertMdvToNcfAndWrite(const string &url)
  
{

  // compute paths

  DsURL dsUrl(url);
  string outputDir;
  RapDataDir.fillPath(dsUrl.getFile(), outputDir);
  string outputPath;
  string dataType = "ncf";

  if (getProjection() == Mdvx::PROJ_POLAR_RADAR) {
    
    //radial data type
    
    Mdv2NcfTrans trans;
    trans.setDebug(_debug);
    if(_heartbeatFunc != NULL) {
      trans.setHeartbeatFunction(_heartbeatFunc);
    }
    trans.setRadialFileType(_ncfRadialFileType);
    if (trans.translateToCfRadial(*this, outputDir)) {
      TaStr::AddStr(_errStr, "ERROR - DsMdvx::_convertMdvToNcfAndWrite()");
      TaStr::AddStr(_errStr, trans.getErrStr());
      return -1;
    }
    outputPath = trans.getNcFilePath();
    
    if (_ncfRadialFileType == DsMdvx::RADIAL_TYPE_CF_RADIAL) {
      dataType = "cfradial";
    } else if (_ncfRadialFileType == DsMdvx::RADIAL_TYPE_DORADE) {
      dataType = "dorade";
    } else if (_ncfRadialFileType == DsMdvx::RADIAL_TYPE_UF) {
      dataType = "uf";
    }
    
  } else {
    
    // basic CF - translate from Mdv
    
    outputPath = _computeNcfOutputPath(outputDir);
    Mdv2NcfTrans trans;
    trans.setDebug(_debug);
    if(_heartbeatFunc != NULL) {
      trans.setHeartbeatFunction(_heartbeatFunc);
    }

    if (trans.translate(*this, outputPath)) {
      cerr << "ERROR - DsMdvx::_convertMdvToNcfAndWrite()" << endl;
      cerr << trans.getErrStr() << endl;
      return -1;
    }
    
  }
    
  // write latest data info
    
  _doWriteLdataInfo(outputDir, outputPath, dataType);
  _pathInUse = outputPath;

  return 0;

}

////////////////////////////////////////////////
// Constrain NCF and write
// First convert to MDV, which applies constraints.
// The convert back to NCF and write.
// returns 0 on success, -1 on failure

int DsMdvx::_constrainNcfAndWrite(const string &url)
  
{
  
  if (convertNcf2Mdv(url)) {
    _errStr += "ERROR - DsMdvx::_constrainNcfToMdvAndWrite()\n";
    return -1;
  }

  if (_convertMdvToNcfAndWrite(url)) {
    _errStr += "ERROR - DsMdvx::_constrainNcfToMdvAndWrite()\n";
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////
// write MDV to specified directory, locally
// returns 0 on success, -1 on failure

int DsMdvx::_writeAsMdv(const string &url)
  
{
  
  DsURL dsUrl(url);
  string writeDir(dsUrl.getFile());

  // local - direct write
  if (Mdvx::writeToDir(writeDir)) {
    _errStr += "ERROR - DsMdvx::_writeAsMdv\n";
    return -1;
  }
  
  // reg with data mapper - the base class uses LdataInfo which does
  // not register
  
  time_t latestTime;
  if (_writeAsForecast) {
    latestTime = _mhdr.time_gen;
  } else {
    latestTime = _mhdr.time_centroid;
  }
  DmapAccess dmap;
  string dataType = "mdv";
  if (_writeFormat == FORMAT_XML) {
    dataType = "xml";
  } else if (_writeFormat == FORMAT_NCF) {
    dataType = "nc";
  }
  if (_writeAsForecast) {
    int forecast_delta = _mhdr.time_centroid - _mhdr.time_gen;
    dmap.regLatestInfo(latestTime, writeDir, dataType, forecast_delta);
  } else {
    dmap.regLatestInfo(latestTime, writeDir, dataType);
  }
  
  return 0;

}

//////////////////////////////////////

void DsMdvx::_doWriteLdataInfo(const string &outputDir,
                               const string &outputPath,
                               const string &dataType)
{
  
  DsLdataInfo ldata(outputDir, _debug);
  ldata.setPathAndTime(outputDir, outputPath);
  time_t latestTime;
  if (_getWriteAsForecast()) {
    latestTime = getGenTime();
    int leadtime = _mhdr.forecast_delta;
    ldata.setLeadTime(leadtime);
    ldata.setIsFcast(true);
  } else {
    latestTime = getValidTime();
  }
  ldata.setDataType(dataType);
  ldata.write(latestTime);

}

