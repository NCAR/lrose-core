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
/////////////////////////////////////////////////////////////
// NcxxRadxFile.cc
//
// NcxxRadxFile object
//
// NetCDF file data for radar radial data in CF-compliant file
//
// See also NcxxRadxFile_read.cc and NcxxRadxFile_write.cc
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2016
//
///////////////////////////////////////////////////////////////

#include <Radx/NcxxRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/RadxPath.hh>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>
using namespace std;

//////////////
// Constructor

NcxxRadxFile::NcxxRadxFile() : RadxFile(), RadxNcfStr()
  
{

  _conventions = BaseConvention;
  _version = CurrentVersion;

  _ncFormat = NETCDF_CLASSIC;

  _writeVol = NULL;
  _readVol = NULL;

  clear();

}

/////////////
// destructor

NcxxRadxFile::~NcxxRadxFile()

{
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void NcxxRadxFile::clear()
  
{

  clearErrStr();

  _file.close();

  _estNoiseAvailHc = false;
  _estNoiseAvailVc = false;
  _estNoiseAvailHx = false;
  _estNoiseAvailVx = false;

  _georefsActive = false;

  _correctionsActive = false;

  _conventions.clear();
  _version.clear();
  _title.clear();
  _institution.clear();
  _references.clear();
  _source.clear();
  _history.clear();
  _comment.clear();
  _author.clear();
  _origFormat.clear();
  _driver.clear();
  _created.clear();
  _statusXml.clear();
  _siteName.clear();
  _scanName.clear();
  _scanId = 0;
  _instrumentName.clear();

  _rayTimesIncrease = true;
  _refTimeSecsFile = 0;

  _volumeNumber = 0;
  _instrumentType = Radx::missingInstrumentType;
  _platformType = Radx::missingPlatformType;
  _primaryAxis = Radx::PRIMARY_AXIS_Z;

  _radarBeamWidthDegH = Radx::missingMetaDouble;
  _radarBeamWidthDegV = Radx::missingMetaDouble;
  _radarAntennaGainDbH = Radx::missingMetaDouble;
  _radarAntennaGainDbV = Radx::missingMetaDouble;

  _lidarConstant = Radx::missingMetaDouble;
  _lidarPulseEnergyJ = Radx::missingMetaDouble;
  _lidarPeakPowerW = Radx::missingMetaDouble;
  _lidarApertureDiamCm = Radx::missingMetaDouble;
  _lidarApertureEfficiency = Radx::missingMetaDouble;
  _lidarFieldOfViewMrad = Radx::missingMetaDouble;
  _lidarBeamDivergenceMrad = Radx::missingMetaDouble;

  _latitude.clear();
  _longitude.clear();
  _altitude.clear();

  _rangeKm.clear();
  _gateSpacingIsConstant = true;

  _clearRays();
  _clearSweeps();
  _clearCals();
  _clearFields();

  _sweepsOrig.clear();
  _sweepsInFile.clear();
  _sweepsToRead.clear();

  _nGatesVary = false;
  _nPoints = 0;

}

void NcxxRadxFile::_clearRays()
{
  for (int ii = 0; ii < (int) _raysVol.size(); ii++) {
    delete _raysVol[ii];
  }
  _raysVol.clear();
}

void NcxxRadxFile::_clearSweeps()
{
  for (int ii = 0; ii < (int) _sweeps.size(); ii++) {
    delete _sweeps[ii];
  }
  _sweeps.clear();
}

void NcxxRadxFile::_clearCals()
{
  for (int ii = 0; ii < (int) _rCals.size(); ii++) {
    delete _rCals[ii];
  }
  _rCals.clear();
}

void NcxxRadxFile::_clearFields()
{
  for (int ii = 0; ii < (int) _fields.size(); ii++) {
    delete _fields[ii];
  }
  _fields.clear();
}

/////////////////////////////////////////////////////////
// Check if specified file is CfRadial format
// Returns true if supported, false otherwise

bool NcxxRadxFile::isSupported(const string &path)

{
  
  if (isCfRadialXx(path)) {
    return true;
  }
  return false;

}

////////////////////////////////////////////////////////////
// Check if this is a CfRadial file
// Returns true on success, false on failure

bool NcxxRadxFile::isCfRadialXx(const string &path)
  
{

  clear();
  
  // open file

  try {
    _file.open(path, NcxxFile::read);
  } catch (NcxxException& e) {
    if (_verbose) {
      cerr << "DEBUG - not CfRadial file: " << path << endl;
    }
    return false;
  }

  // read dimensions
  
  if (_readDimensions()) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not CfRadialXx file" << endl;
      cerr << _errStr << endl;
    }
    return false;
  }

  // read global attributes
  
  if (_readGlobalAttributes()) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not CfRadialXx file" << endl;
      cerr << _errStr << endl;
    }
    return false;
  }

  // check history

  if (_history.find("NcxxRadxFile") == string::npos) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not CfRadialXx file" << endl;
      cerr << "  No NcxxRadxFile string in history" << endl;
    }
    return false;
  }

  // file has the correct dimensions, so it is a CfRadial file

  _file.close();
  return true;

}

////////////////////////////////////////////////
// get the date and time from a dorade file path
// returns 0 on success, -1 on failure

int NcxxRadxFile::getTimeFromPath(const string &path, RadxTime &rtime)

{

  RadxPath rpath(path);
  const string &fileName = rpath.getFile();
  
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
    int year, month, day, hour, min, sec;
    if (sscanf(start, "%4d%2d%2d_%2d%2d%2d",
               &year, &month, &day, &hour, &min, &sec) == 6) {
      if (year < 1900 || month < 1 || month > 12 || day < 1 || day > 31) {
        return -1;
      }
      if (hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59) {
        return -1;
      }
      rtime.set(year, month, day, hour, min, sec);
      return 0;
    }
    start++;
  }
  
  return -1;
  
}

////////////////////////////////////////
// convert Radx::DataType_t to Nc3Type

NcxxType NcxxRadxFile::_getNcxxType(Radx::DataType_t dtype)

{
  switch (dtype) {
    case Radx::FL64:
      return ncxxDouble;
    case Radx::FL32:
      return ncxxFloat;
    case Radx::SI32:
      return ncxxInt;
    case Radx::SI16:
      return ncxxShort;
    case Radx::SI08:
    default:
      return ncxxByte;
  }
}

//////////////////////////////////////////////////////////
// convert RadxFile::netcdf_format_t to Nc3File::FileFormat

NcxxFile::FileFormat 
  NcxxRadxFile::_getFileFormat(RadxFile::netcdf_format_t format)

{
  switch (format) {
    case NETCDF4_CLASSIC:
      return NcxxFile::nc4classic;
      break;
    case NETCDF_OFFSET_64BIT:
      return NcxxFile::classic64;
      break;
    case NETCDF4:
      return NcxxFile::nc4;
      break;
    case NETCDF_CLASSIC:
    default:
      return NcxxFile::classic;
  }
}

/////////////////////////////////////////////////////////
// print summary after read

void NcxxRadxFile::print(ostream &out) const
  
{
  
  out << "=============== NcxxRadxFile ===============" << endl;
  RadxFile::print(out);
  out << "  title: " << _title << endl;
  out << "  institution: " << _institution << endl;
  out << "  references: " << _references << endl;
  out << "  source: " << _source << endl;
  out << "  history: " << _history << endl;
  out << "  comment: " << _comment << endl;
  out << "  author: " << _author << endl;
  out << "  origFormat: " << _origFormat << endl;
  out << "  driver: " << _driver << endl;
  out << "  created: " << _created << endl;
  out << "  statusXml: " << _statusXml << endl;
  out << "  siteName: " << _siteName << endl;
  out << "  scanName: " << _scanName << endl;
  out << "  scanId: " << _scanId << endl;
  out << "  instrumentName: " << _instrumentName << endl;
  out << "  refTimeSecsFile: " << RadxTime::strm(_refTimeSecsFile) << endl;
  out << "  volumeNumber: " << _volumeNumber << endl;
  out << "  instrumentType: " << Radx::instrumentTypeToStr(_instrumentType) << endl;
  out << "  platformType: " << Radx::platformTypeToStr(_platformType) << endl;
  out << "  primaryAxis: " << Radx::primaryAxisToStr(_primaryAxis) << endl;
  if (_latitude.size() > 0) {
    out << "  latitude: " << _latitude[0] << endl;
  }
  if (_longitude.size() > 0) {
    out << "  longitude: " << _longitude[0] << endl;
  }
  if (_altitude.size() > 0) {
    out << "  altitude: " << _altitude[0] << endl;
  }
  for (size_t ii = 0; ii < _frequency.size(); ii++) {
    out << "  frequencyHz[" << ii << "]: " << _frequency[ii] << endl;
  }

  if (_instrumentType == Radx::INSTRUMENT_TYPE_RADAR) {
    out << "  radarBeamWidthDegH: " << _radarBeamWidthDegH << endl;
    out << "  radarBeamWidthDegV: " << _radarBeamWidthDegV << endl;
    out << "  radarAntennaGainDbH: " << _radarAntennaGainDbH << endl;
    out << "  radarAntennaGainDbV: " << _radarAntennaGainDbV << endl;
  } else {
    out << "  lidarConstant: " << _lidarConstant << endl;
    out << "  lidarPulseEnergyJ: " << _lidarPulseEnergyJ << endl;
    out << "  lidarPeakPowerW: " << _lidarPeakPowerW << endl;
    out << "  lidarApertureDiamCm: " << _lidarApertureDiamCm << endl;
    out << "  lidarApertureEfficiency: " << _lidarApertureEfficiency << endl;
    out << "  lidarFieldOfViewMrad: " << _lidarFieldOfViewMrad << endl;
    out << "  lidarBeamDivergenceMrad: " << _lidarBeamDivergenceMrad << endl;
  }
  _geom.print(out);
  out << "  gateSpacingIsConstant: " << _gateSpacingIsConstant << endl;
  out << "===========================================" << endl;

}

////////////////////////////////////////////////////////////
// Print native data in uf file
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int NcxxRadxFile::printNative(const string &path, ostream &out,
                              bool printRays, bool printData)
  
{

  _addErrStr("ERROR - NcxxRadxFile::printNative");
  _addErrStr("  Native print does not apply to NetCDF file: ", path);
  _addErrStr("  Use 'ncdump' instead");
  return -1;

}

//////////////////////////////////////////////////////////////////
// interpret float and double vals, with respect to missing vals

Radx::fl64 NcxxRadxFile::_checkMissingDouble(double val)

{
  if (fabs(val - Radx::missingMetaDouble) < 0.0001) {
    return Radx::missingMetaDouble;
  }
  if (val < -1.0e6) {
    return Radx::missingMetaDouble;
  }
  return val;
}

Radx::fl32 NcxxRadxFile::_checkMissingFloat(float val)

{

  if (fabs(val - Radx::missingMetaFloat) < 0.0001) {
    return Radx::missingMetaFloat;
  }
  if (val < -1.0e6) {
    return Radx::missingMetaFloat;
  }
  return val;

}

