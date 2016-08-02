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
// Mdvx_ncf.cc
//
// NetCDF CF routines for Mdvx class
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2008
//
//////////////////////////////////////////////////////////

#include <Mdv/Mdvx.hh>
#include <didss/RapDataDir.hh>
#include <toolsa/TaStr.hh>
#include <cerrno>
using namespace std;

/////////////////////////////////////////////////////////////////
// set NCF data in MDVX object
//
// For forecast data, the valid time represents the time at which
// the forecast is valid, and forecastLeadSecs is the time from the
// forecast reference time (gen time) to the valid time.

void Mdvx::setNcf(const void *ncBuf,
                  int nbytes,
                  time_t validTime,
                  bool isForecast /* = false */,
                  int forecastLeadSecs /* = 0 */,
                  int epoch /* = 0 */)

{
  
  // Set the header information

  setNcfHeader(validTime, isForecast, forecastLeadSecs, epoch);
  
  // Set the buffer information

  setNcfBuffer(ncBuf, nbytes);
  
}


/////////////////////////////////////////////////////////////////
// set NCF header information in MDVX object
//
// For forecast data, the valid time represents the time at which
// the forecast is valid, and forecastLeadSecs is the time from the
// forecast reference time (gen time) to the valid time.

void Mdvx::setNcfHeader(time_t validTime,
			bool isForecast /* = false */,
			int forecastLeadSecs /* = 0 */,
			int epoch /* = 0 */)

{
  
  _currentFormat = FORMAT_NCF;

  _ncfValidTime = validTime;
  _ncfEpoch = epoch;
  
  if (isForecast) {
    _ncfForecastTime = validTime;
    _ncfGenTime = validTime - forecastLeadSecs;
    _ncfForecastDelta = forecastLeadSecs;
    _ncfIsForecast = true;
  } else {
    _ncfForecastTime = 0;
    _ncfGenTime = 0;
    _ncfForecastDelta = 0;
    _ncfIsForecast = false;
  }

}


/////////////////////////////////////////////////////////////////
// set NCF buffer in MDVX object

void Mdvx::setNcfBuffer(const void *ncBuf,
			int nbytes)

{
  
  _currentFormat = FORMAT_NCF;

  _ncfBuf.free();
  _ncfBuf.add(ncBuf, nbytes);
  
}


//////////////////
// clear NCF data

void Mdvx::clearNcf()
  
{
  _ncfBuf.free();
  _currentFormat = FORMAT_MDV;
  _ncfValidTime = 0;
  _ncfGenTime = 0;
  _ncfForecastTime = 0;
  _ncfForecastDelta = 0;
  _ncfIsForecast = false;
  _ncfEpoch = 0;
  _ncfFileSuffix.clear();
  _ncfConstrained = true;
}

// set the suffix for the netCDF file
// file name will end .mdv.suffix.nc

void Mdvx::setNcfFileSuffix(const string &suffix)
  
{
  _ncfFileSuffix = suffix;
}

// set whether NCF data is constrained
// i.e. have read constraints been applied?

void Mdvx::setConstrained(bool state)

{
  _ncfConstrained = state;
}

//////////////////////////////////////////
// is specified format netCDF CF

bool Mdvx::isNcf(mdv_format_t format) const
{
  if (format == FORMAT_NCF) {
    return true;
  } else {
    return false;
  }
}

////////////////////////////////////////////////////
// read volume method for NCF files
// returns 0 on success, -1 on failure

int Mdvx::_read_volume_ncf()
  
{

  // open NCF file
  
  TaFile ncfFile;
  if (ncfFile.fopenUncompress(_pathInUse.c_str(), "rb") == NULL) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_read_volume_ncf\n";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }

  // stat the file to get size

  if (ncfFile.fstat()) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_read_volume_ncf\n";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }
  stat_struct_t &fileStat = ncfFile.getStat();
  off_t fileLen = fileStat.st_size;
  
  // read in buffer

  _ncfBuf.reserve(fileLen);
  if (ncfFile.fread(_ncfBuf.getPtr(), 1, fileLen) != fileLen) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_read_volume_ncf\n";
    _errStr += strerror(errNum);
    _errStr += "\n";
    ncfFile.fclose();
    return -1;
  }
  
  // close file
  
  ncfFile.fclose();

  // set current format

  _currentFormat = FORMAT_NCF;

  // flag that read constraints have not been applied

  _ncfConstrained = false;

  // set times etc

  if (_set_times_ncf()) {
    _errStr += "ERROR - Mdvx::_read_volume_ncf\n";
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////
// Set times related to NCF files, from path
// Returns 0 on success, -1 on failure

int Mdvx::_set_times_ncf()
  
{

  // tokenize the path, to break it into directories
  
  vector<string> parts;
  TaStr::tokenize(_pathInUse, PATH_DELIM, parts);
  int nParts = (int) parts.size();

  // try for long format first

  if (nParts > 0) {

    string fileName = parts[parts.size() - 1];

    // find first digit in file name - if no digits, return now
    
    const char *start = NULL;
    for (size_t ii = 0; ii < fileName.size(); ii++) {
      if (isdigit(fileName[ii])) {
        start = fileName.c_str() + ii;
        break;
      }
    }

    if (start) {

      int year, month, day, hour, min, sec;
      long leadSecs;
      
      if (sscanf(start,
                 "%4d%2d%2d_g_%2d%2d%2d_f_%8ld",
                 &year, &month, &day, &hour, &min, &sec, &leadSecs) == 7) {
        DateTime genTime(year, month, day, hour, min, sec);
        _ncfForecastDelta = leadSecs;
        _ncfGenTime = genTime.utime();
        _ncfValidTime = genTime.utime() + leadSecs;
        _ncfForecastTime = _ncfValidTime;
        _ncfIsForecast = true;
        _ncfEpoch = computeEpoch(_ncfValidTime);
        return 0;
      }
      
      const char *end = start + strlen(start);
      // check all possible starting locations for date/time string
      while (start < end - 6) {
        if (sscanf(start,
                   "%4d%2d%2d_%2d%2d%2d",
                   &year, &month, &day, &hour, &min, &sec) == 6) {
          DateTime validTime(year, month, day, hour, min, sec);
          _ncfValidTime = validTime.utime();
          _ncfGenTime = 0;
          _ncfForecastTime = 0;
          _ncfForecastDelta = 0;
          _ncfIsForecast = false;
          _ncfEpoch = computeEpoch(_ncfValidTime);
          return 0;
        }
        start++;
      }
        
    } // if (start)

  } // if (nParts > 0)

  // if got here, must be short format
  
  if (nParts < 3) {
    TaStr::AddStr(_errStr, "ERROR - Mdvx::_set_times_ncf");
    TaStr::AddStr(_errStr, "  pathInUse: ", _pathInUse);
    TaStr::AddStr(_errStr, "  Cannot parse path to get times");
    TaStr::AddInt(_errStr, "  Too few directories in path: ", nParts);
    return -1;
  }
  
  // check the last part, to see if it is a forecast style

  bool isForecast = false;
  int leadSecs = -1;
  long isecs;
  if (sscanf(parts[nParts-1].c_str(), "f_%8ld", &isecs) == 1) {
    leadSecs = isecs;
    isForecast = true;
  }

  if (isForecast) {
    
    int hour, min, sec;
    if (sscanf(parts[nParts-2].c_str(),
               "g_%2d%2d%2d", &hour, &min, &sec) != 3) {
      TaStr::AddStr(_errStr, "ERROR - Mdvx::_set_times_ncf");
      TaStr::AddStr(_errStr, "  pathInUse: ", _pathInUse);
      TaStr::AddStr(_errStr, "  Cannot parse path for gen time");
      return -1;
    }
    
    int year, month, day;
    if (sscanf(parts[nParts-3].c_str(),
               "%4d%2d%2d", &year, &month, &day) != 3) {
      TaStr::AddStr(_errStr, "ERROR - Mdvx::_set_times_ncf");
      TaStr::AddStr(_errStr, "  pathInUse: ", _pathInUse);
      TaStr::AddStr(_errStr, "  Cannot parse path for gen date");
      return -1;
    }

    DateTime genTime(year, month, day, hour, min, sec);
    
    _ncfForecastDelta = leadSecs;
    _ncfGenTime = genTime.utime();
    _ncfValidTime = genTime.utime() + leadSecs;
    _ncfForecastTime = _ncfValidTime;
    _ncfIsForecast = true;
    _ncfEpoch = computeEpoch(_ncfValidTime);
    
  } else {

    // not forecast

    int hour, min, sec;
    if (sscanf(parts[nParts-1].c_str(),
               "%2d%2d%2d", &hour, &min, &sec) != 3) {
      TaStr::AddStr(_errStr, "ERROR - Mdvx::_set_times_ncf");
      TaStr::AddStr(_errStr, "  pathInUse: ", _pathInUse);
      TaStr::AddStr(_errStr, "  Cannot parse path for valid time");
      return -1;
    }
    
    int year, month, day;
    if (sscanf(parts[nParts-2].c_str(),
               "%4d%2d%2d", &year, &month, &day) != 3) {
      TaStr::AddStr(_errStr, "ERROR - Mdvx::_set_times_ncf");
      TaStr::AddStr(_errStr, "  pathInUse: ", _pathInUse);
      TaStr::AddStr(_errStr, "  Cannot parse path for valid date");
      return -1;
    }

    DateTime validTime(year, month, day, hour, min, sec);
    
    _ncfValidTime = validTime.utime();
    _ncfGenTime = 0;
    _ncfForecastTime = 0;
    _ncfForecastDelta = 0;
    _ncfIsForecast = false;
    _ncfEpoch = computeEpoch(_ncfValidTime);
    
  }

  return 0;

}

/////////////////////////////////////////////////////////
// Write to path as NCF
// File is written to files based on the specified path.
// Returns 0 on success, -1 on error.

int Mdvx::_write_as_ncf(const string &output_path) const

{

  // compute path
  
  string outPathStr;
  RapDataDir.fillPath(output_path, outPathStr);

  string ncfPathStr = outPathStr + getNcfExt();
  _pathInUse = ncfPathStr;

  if (_debug) {
    cerr << "Mdvx - writing to NCF path: " << ncfPathStr << endl;
  }

  // remove compressed file if it exists
  
  ta_remove_compressed(ncfPathStr.c_str());

  if (_write_buffer_to_file(ncfPathStr, _ncfBuf.getLen(), _ncfBuf.getPtr())) {
    cerr << "ERROR - Mdvx::_write_as_ncf" << endl;
    return -1;
  }

  return 0;

}

///////////////////////////////////////////////////////
// print ncf info

void Mdvx::printNcfInfo(ostream &out) const

{

  if (_currentFormat == FORMAT_NCF) {
    out << endl;
    out << "NetCDF CF FORMAT file" << endl;
    out << "---------------------" << endl;
    out << endl;
    out << "valid_time:             " << _timeStr(_ncfValidTime) << endl;
    out << "gen_time:               " << _timeStr(_ncfGenTime) << endl;
    if (_ncfIsForecast) {
      out << "forecast_time:          " << _timeStr(_ncfForecastTime) << endl;
      out << "forecast_delta(secs):   " << _ncfForecastDelta << endl;
    }
    out << "epoch:                  " << _ncfEpoch << endl;
    out << "constraints applied?    " << _ncfConstrained << endl;
    out << "data length (bytes):    " << _ncfBuf.getLen() << endl;
    out << endl;
    return;
  }

}

////////////////////////////////////
// get ncf file extension
// Adds in suffix if appropriate

string Mdvx::getNcfExt() const

{

  string ext;
  if (_ncfFileSuffix.size() > 0) {
    ext += ".";
    ext += _ncfFileSuffix;
  }
  ext += ".nc";

  return ext;

}


