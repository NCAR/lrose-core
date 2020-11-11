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
#include <Mdv/MdvxChunk.hh>
#include <Mdv/DsMdvxMsg.hh>
#include <Mdv/Mdv2NcfTrans.hh>
#include <Mdv/Ncf2MdvTrans.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxField.hh>
#include <didss/RapDataDir.hh>
#include <didss/DsURL.hh>
#include <dsserver/DsLocator.hh>
#include <dsserver/DmapAccess.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/mem.h>
#include <cerrno>
#include <cstdlib>
#include <unistd.h>
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

  if (_debug) {
    cerr << "DEBUG - Mdvx::setNcfHeader" << endl;
  }

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

  _internalFormat = FORMAT_NCF;

}


/////////////////////////////////////////////////////////////////
// set NCF buffer in MDVX object

void Mdvx::setNcfBuffer(const void *ncBuf,
			int nbytes)

{
  
  if (_debug) {
    cerr << "DEBUG - Mdvx::setNcfBuffer" << endl;
  }

  _ncfBuf.free();
  _ncfBuf.add(ncBuf, nbytes);
  _internalFormat = FORMAT_NCF;

}


//////////////////
// clear NCF data

void Mdvx::clearNcf()
  
{
  _ncfBuf.free();
  _internalFormat = FORMAT_MDV;
  _ncfValidTime = 0;
  _ncfGenTime = 0;
  _ncfForecastTime = 0;
  _ncfForecastDelta = 0;
  _ncfIsForecast = false;
  _ncfEpoch = 0;
  _ncfFileSuffix.clear();
}

// set the suffix for the netCDF file
// file name will end .mdv.suffix.nc

void Mdvx::setNcfFileSuffix(const string &suffix)
  
{
  _ncfFileSuffix = suffix;
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
// read NetCDF volume into buffer _ncfBuf
// assumes _pathInUse has been set by caller
// returns 0 on success, -1 on failure

int Mdvx::_readVolumeIntoNcfBuf()
  
{

  // open NCF file
  
  TaFile ncfFile;
  if (ncfFile.fopenUncompress(_pathInUse.c_str(), "rb") == NULL) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_readVolumeIntoNcfBuf\n";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }

  // stat the file to get size

  if (ncfFile.fstat()) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_readVolumeIntoNcfBuf\n";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }
  stat_struct_t &fileStat = ncfFile.getStat();
  size_t fileLen = fileStat.st_size;
  
  // read in buffer

  _ncfBuf.reserve(fileLen);
  if (ncfFile.fread(_ncfBuf.getPtr(), 1, fileLen) != fileLen) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_readVolumeIntoNcfBuf\n";
    _errStr += strerror(errNum);
    _errStr += "\n";
    ncfFile.fclose();
    return -1;
  }
  
  // close file
  
  ncfFile.fclose();

  // set current format

  _internalFormat = FORMAT_NCF;
  
  // set times etc

  if (_setTimesNcf()) {
    _errStr += "ERROR - Mdvx::_readVolumeIntoNcfBuf\n";
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////
// Set times related to NCF files, from path
// assumes _pathInUse has been set by caller
// Returns 0 on success, -1 on failure

int Mdvx::_setTimesNcf()
  
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
    TaStr::AddStr(_errStr, "ERROR - Mdvx::_setTimesNcf");
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
      TaStr::AddStr(_errStr, "ERROR - Mdvx::_setTimesNcf");
      TaStr::AddStr(_errStr, "  pathInUse: ", _pathInUse);
      TaStr::AddStr(_errStr, "  Cannot parse path for gen time");
      return -1;
    }
    
    int year, month, day;
    if (sscanf(parts[nParts-3].c_str(),
               "%4d%2d%2d", &year, &month, &day) != 3) {
      TaStr::AddStr(_errStr, "ERROR - Mdvx::_setTimesNcf");
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
      TaStr::AddStr(_errStr, "ERROR - Mdvx::_setTimesNcf");
      TaStr::AddStr(_errStr, "  pathInUse: ", _pathInUse);
      TaStr::AddStr(_errStr, "  Cannot parse path for valid time");
      return -1;
    }
    
    int year, month, day;
    if (sscanf(parts[nParts-2].c_str(),
               "%4d%2d%2d", &year, &month, &day) != 3) {
      TaStr::AddStr(_errStr, "ERROR - Mdvx::_setTimesNcf");
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

///////////////////////////////////////////////////////
// print ncf info

void Mdvx::printNcfInfo(ostream &out) const

{

  if (_internalFormat == FORMAT_NCF) {
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
    out << "data length (bytes):    " << _ncfBuf.getLen() << endl;
    out << endl;
    return;
  }

}

////////////////////////////////
// MDV to netCDF CF conversion

// set mdv to ncf conversion attributes
// For conversion of MDV into netCDF.

void Mdvx::setMdv2NcfAttr(const string &institution,
                          const string &references,
                          const string &comment)

{

  _ncfInstitution = institution;
  _ncfReferences = references;
  _ncfComment = comment;

}

// replace comment attribute with input string.

void Mdvx::setMdv2NcfCommentAttr(const string &comment)
{
  _ncfComment = comment;
}

// set compression - uses HDF5

void Mdvx::setMdv2NcfCompression(bool compress,
                                 int compressionLevel)
  
{
  
  _ncfCompress = compress;
  _ncfCompressionLevel = compressionLevel;

}

// set the output format of the netCDF file

void Mdvx::setMdv2NcfFormat(nc_file_format_t fileFormat)

{
  _ncfFileFormat = fileFormat;
}

// set the file type for polar radar files

void Mdvx::setRadialFileType(radial_file_type_t fileType)

{
  _ncfRadialFileType = fileType;
}

// set output parameters - what should be included

void Mdvx::setMdv2NcfOutput(bool outputLatlonArrays,
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

void Mdvx::addMdv2NcfTrans(string mdvFieldName,
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

void Mdvx::clearMdv2Ncf()
  
{
  _ncfInstitution.clear();
  _ncfReferences.clear();
  _ncfComment.clear();
  _mdv2NcfTransArray.clear();
  _ncfCompress = true;
  _ncfCompressionLevel = 4;
  _ncfFileFormat = NCF_FORMAT_NETCDF4;
  _ncfOutputLatlonArrays = true;
  _ncfOutputMdvAttr = true;
  _ncfOutputMdvChunks = true;
  _ncfOutputStartEndTimes = true;
  _ncfRadialFileType = RADIAL_TYPE_CF;

}

////////////////////////////////////////////////
// return string representation of packing type

string Mdvx::ncfPack2Str(const ncf_pack_t packing)

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

Mdvx::ncf_pack_t Mdvx::ncfPack2Enum(const string &packing)

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

string Mdvx::ncFormat2Str(const nc_file_format_t format)

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

Mdvx::nc_file_format_t Mdvx::ncFormat2Enum(const string &format)

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

string Mdvx::radialFileType2Str(const radial_file_type_t ftype)

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

Mdvx::radial_file_type_t Mdvx::radialFileType2Enum(const string &ftype)

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


/////////////////////////////////////////////////////////
// Write NetCDF data buffer, in _ncfBuf, to path
// File name is based on the specified path.
// Returns 0 on success, -1 on error.

int Mdvx::_writeNcfBufToFile(const string &outputPath) const

{

  // remove compressed file if it exists
  
  ta_remove_compressed(outputPath.c_str());

  // write the buffer to the file

  if (_writeBufferToFile(outputPath, _ncfBuf.getLen(), _ncfBuf.getPtr())) {
    TaStr::AddStr(_errStr, "ERROR - Mdvx::_writeNcfBufToFile");
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////
// Write out in CF-NetCDF format, to path.
// File name is based on the specified path.
// Conversion is carried out during the write.
// Returns 0 on success, -1 on error.

int Mdvx::_writeAsNcf(const string &outputPath) const

{

  if (_debug) {
    cerr << "DEBUG - Mdvx::_writeAsNcf" << endl;
  }

  // remove compressed file if it exists
  
  ta_remove_compressed(outputPath.c_str());

  // write as CF
  
  Mdv2NcfTrans trans;
  trans.clearData();
  trans.setDebug(_debug);
  if(_heartbeatFunc != NULL) {
    trans.setHeartbeatFunction(_heartbeatFunc);
  }
  
  if (trans.writeCf(*this, outputPath)) {
    _errStr += "ERROR - Mdvx::_writeAsNcf.\n";
    TaStr::AddStr(_errStr, "  Path: ", outputPath);
    _errStr += trans.getErrStr();
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////
// convert MDV format to NETCDF CF format
// returns 0 on success, -1 on failure

int Mdvx::_writeToNcfBuf(const string &path)
  
{
  
  if (_debug) {
    cerr << "DEBUG - Mdvx::_writeToNcfBuf" << endl;
  }

  if (_internalFormat != FORMAT_MDV) {
    _errStr += "ERROR - Mdvx::_writeToNcfBuf.\n";
    TaStr::AddStr(_errStr, "  Path: ", path);
    TaStr::AddStr(_errStr, "  Incorrect format: ", format2Str(_internalFormat));
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
          "/tmp/Mdvx_writeToNcfBuf_%.4d%.2d%.2d_%.2d%.2d%.2d_%.5d.nc",
          dnow.getYear(), dnow.getMonth(), dnow.getDay(),
          dnow.getHour(), dnow.getMin(), dnow.getSec(), pid);
          
  Mdv2NcfTrans trans;
  trans.clearData();
  trans.setDebug(_debug);
  if(_heartbeatFunc != NULL) {
    trans.setHeartbeatFunction(_heartbeatFunc);
  }
  
  if (trans.writeCf(*this, tmpFilePath)) {
    _errStr += "ERROR - Mdvx::_writeToNcfBuf.\n";
    TaStr::AddStr(_errStr, "  Path: ", path);
    _errStr += trans.getErrStr();
    return -1;
  }

  // open NCF file
  
  TaFile ncfFile;
  if (ncfFile.fopen(tmpFilePath, "rb") == NULL) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_writeToNcfBuf\n";
    TaStr::AddStr(_errStr, "  Path: ", path);
    TaStr::AddStr(_errStr, "  Cannot open tmp file: ", tmpFilePath);
    _errStr += strerror(errNum);
    _errStr += "\n";
    unlink(tmpFilePath);
    return -1;
  }

  // stat the file to get size
  
  if (ncfFile.fstat()) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_writeToNcfBuf\n";
    TaStr::AddStr(_errStr, "  Path: ", path);
    TaStr::AddStr(_errStr, "  Cannot stat tmp file: ", tmpFilePath);
    _errStr += strerror(errNum);
    _errStr += "\n";
    ncfFile.fclose();
    unlink(tmpFilePath);
    return -1;
  }
  stat_struct_t &fileStat = ncfFile.getStat();
  size_t fileLen = fileStat.st_size;
  
  // read in buffer
  
  _ncfBuf.reserve(fileLen);
  if (ncfFile.fread(_ncfBuf.getPtr(), 1, fileLen) != fileLen) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_writeToNcfBuf\n";
    TaStr::AddStr(_errStr, "  Path: ", path);
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
  
  if (_debug) {
    cerr << "DEBUG - Mdvx::_writeToNcfBuf - setting to FORMAT_NCF" << endl;
  }
  _internalFormat = FORMAT_NCF;

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
// convert NETCDF CF to MDV
// given an object containing a netcdf file buffer
// returns 0 on success, -1 on failure

int Mdvx::_readFromNcfBuf(const string &path)
  
{

  if (_internalFormat != FORMAT_NCF) {
    _errStr += "ERROR - Mdvx::_readFromNcfBuf.\n";
    TaStr::AddStr(_errStr, "  Path: ", path);
    TaStr::AddStr(_errStr, "  Incorrect format: ", format2Str(_internalFormat));
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
          "/tmp/Mdvx_readFromNcfBuf_%.4d%.2d%.2d_%.2d%.2d%.2d_%.5d.nc",
          dnow.getYear(), dnow.getMonth(), dnow.getDay(),
          dnow.getHour(), dnow.getMin(), dnow.getSec(), pid);

  // write nc buffer to file

  if (_writeBufferToFile(tmpFilePath, _ncfBuf.getLen(), _ncfBuf.getPtr())) {
    _errStr += "ERROR - Mdvx::_readFromNcfBuf\n";
    TaStr::AddStr(_errStr, "  Path: ", path);
    TaStr::AddStr(_errStr, "  Cannot write buffe to tmp file: ", tmpFilePath);
    return -1;
  }
          
  // create translator
  
  Ncf2MdvTrans trans;
  trans.setDebug(_debug);
  
  // perform translation
  // returns 0 on success, -1 on failure

  if (trans.readCf(tmpFilePath, *this)) {
    _errStr += "ERROR - Mdvx::_readFromNcfBuf\n";
    TaStr::AddStr(_errStr, "  Path: ", path);
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
  
  _internalFormat = FORMAT_MDV;

  // convert the output fields appropriately

  if (_readFormat == FORMAT_MDV) {
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
// Read the headers from a NETCDF CF file into MDV, given the file path
// Convert to NCF at the end if required
// Currently we read all of the data, which will include the headers.
// returns 0 on success, -1 on failure

int Mdvx::_readAllHeadersNcf(const string &path)
  
{

  // First read in the whole file.  We do this because we don't have a way
  // to easily read the field header information without reading the field
  // data also.
  
  // perform read into temporary mdvx object
  // returns 0 on success, -1 on failure

  Ncf2MdvTrans trans;
  trans.setDebug(_debug);
  trans.setReadData(false);
  Mdvx mdvx;
  if (trans.readCf(path, mdvx)) {
    _errStr += "ERROR - Mdvx::_readAllHeadersNcf\n";
    TaStr::AddStr(_errStr, "  Path ", path);
    TaStr::AddStr(_errStr, "  Cannot translate file to MDV");
    TaStr::AddStr(_errStr, trans.getErrStr());
    return -1;
  }

  // clear this object

  clearFields();
  clearChunks();
  _fhdrsFile.clear();
  _vhdrsFile.clear();
  _chdrsFile.clear();
  
  // Now set the normal and file headers.

  _mhdr = mdvx.getMasterHeader();
  _mhdrFile = mdvx.getMasterHeader();
  
  for (size_t ii = 0; ii < mdvx.getNFields(); ii++) {
    MdvxField *field = mdvx.getField(ii);
    Mdvx::field_header_t fhdr = field->getFieldHeader();
    Mdvx::vlevel_header_t vhdr = field->getVlevelHeader();
    addField(new MdvxField(fhdr, vhdr, NULL, false, false, false));
    _fhdrsFile.push_back(fhdr);
    _vhdrsFile.push_back(vhdr);
  }
  
  for (size_t ii = 0; ii < mdvx.getNChunks(); ii++) {
    MdvxChunk *chunk = mdvx.getChunkByNum(ii);
    Mdvx::chunk_header_t chdr = chunk->getHeader();
    addChunk(new MdvxChunk(chdr));
    _chdrsFile.push_back(chunk->getHeader());
  }
  
  return 0;
}

//////////////////////////////////////////////////////
// Read a NETCDF CF file into MDV
// Convert to NCF at the end if required
// returns 0 on success, -1 on failure

int Mdvx::_readNcf(const string &path)
  
{

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
  
  if (trans.readCf(path, *this)) {
    _errStr += "ERROR - Mdvx::_readNcf\n";
    TaStr::AddStr(_errStr, "  Path ", path);
    TaStr::AddStr(_errStr, "  Cannot translate file to MDV");
    TaStr::AddStr(_errStr, trans.getErrStr());
    return -1;
  }

  // remove netcdf data
  
  clearNcf();

  // set nc-specific times from path

  _setTimesNcf();
  
  // set format to MDV
  
  _internalFormat = FORMAT_MDV;

  // convert the output fields appropriately

  for (int ii = 0; ii < (int) _fields.size(); ii++) {
    _fields[ii]->convertType(readEncodingType,
                             readCompressionType,
                             readScalingType,
                             readScale, readBias);
  }

  // copy the main headers to file headers

  _copyMainHeadersToFileHeaders();

  return 0;

}

//////////////////////////////////////////////////////
// Read the metadata from a RADX file
// Fill out the Mdv file headers
// returns 0 on success, -1 on failure

int Mdvx::_readAllHeadersRadx(const string &path)
  
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
  if (inFile.readFromPath(path, vol)) {
    _errStr += "ERROR - Mdvx::readAllHeadersRadx.\n";
    _errStr += "Cannot read in files.\n";
    TaStr::AddStr(_errStr, "  path: ", path);
    _errStr += inFile.getErrStr();
    return -1;
  }
  
  // read in first field with data

  RadxVol vol0;
  const vector<RadxField *> fields = vol.getFields();
  if (fields.size() > 0) {
    const RadxField *rfld = fields[0];
    string firstFieldName = rfld->getName();
    RadxFile inFile0;
    inFile0.addReadField(firstFieldName);
    if (inFile0.readFromPath(path, vol0) == 0) {
      vol0.computeMaxNGates();
    }
  }
  
  // make sure sweeps are in ascending order, as required by MDV

  vol.reorderSweepsAsInFileAscendingAngle();
  vol.reorderSweepsAscendingAngle();

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
  _mhdr.n_fields = vol.getFields().size();
  
  _mhdr.sensor_lat = vol.getLatitudeDeg();
  _mhdr.sensor_lon = vol.getLongitudeDeg();
  _mhdr.sensor_alt = vol.getAltitudeKm();

  _mhdr.max_nx = 0;
  _mhdr.max_ny = 0;
  _mhdr.max_nz = 0;

  const vector<RadxSweep *> &sweepsInFile = vol.getSweepsAsInFile();

  // add field headers

  vector<RadxField *> flds = vol.getFields();
  for (size_t ifield = 0; ifield < flds.size(); ifield++) {

    RadxField *rfld = flds[ifield];
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
    addField(new MdvxField(fhdr, vhdr, NULL, false, false, false));
    
  } // ifield
  
  _mhdrFile = _mhdr;

  // set internal format to MDV
  
  _internalFormat = FORMAT_MDV;

  return 0;

}

////////////////////////////////////////////////
// Read a RADX-type file, convert to MDV
// Convert to RADX at the end if required
// returns 0 on success, -1 on failure

int Mdvx::_readRadx(const string &path)
  
{

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
  if (inFile.readFromPath(path, vol)) {
    _errStr += "ERROR - Mdvx::_readRadx.\n";
    _errStr += "Cannot read in files.\n";
    TaStr::AddStr(_errStr, "  path: ", path);
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
  if (trans.translateRadxVol2Mdv(path, vol, *this)) {
    _errStr += "ERROR - Mdvx::_readRadx.\n";
    _errStr += "  Cannot translate RadxVol.\n";
    return -1;
  }

  // copy the main headers to file headers

  _copyMainHeadersToFileHeaders();
  
  // set format to MDV
  
  _internalFormat = FORMAT_MDV;

  // for MDV format, convert the output fields appropriately

  if (_readFormat == FORMAT_MDV) {
    for (int ii = 0; ii < (int) _fields.size(); ii++) {
      _fields[ii]->convertType(readEncodingType,
                               readCompressionType,
                               readScalingType,
                               readScale, readBias);
    }
  }

  return 0;

}

///////////////////////////////////////
// print mdv to ncf convert request

void Mdvx::printConvertMdv2NcfRequest(ostream &out) const

{

  out << "Mdvx convert MDV to NCF request" << endl;
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

