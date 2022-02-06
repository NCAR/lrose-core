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
// RaxpolNcRadxFile.cc
//
// RaxpolNcRadxFile object
//
// NetCDF file data for radar radial data from OU RAXPOL radar
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2022
//
///////////////////////////////////////////////////////////////

#include <Radx/NcfRadxFile.hh>
#include <Radx/RaxpolNcRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxArray.hh>
#include <Radx/RadxXml.hh>
#include <Radx/RadxStr.hh>
#include <Radx/RadxComplex.hh>
#include <Radx/RadxReadDir.hh>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <algorithm>
using namespace std;

//////////////
// Constructor

RaxpolNcRadxFile::RaxpolNcRadxFile() : RadxFile()
  
{

  _ncFormat = NETCDF_CLASSIC;
  _readVol = NULL;
  clear();

}

/////////////
// destructor

RaxpolNcRadxFile::~RaxpolNcRadxFile()

{
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void RaxpolNcRadxFile::clear()
  
{

  clearErrStr();

  _file.close();

  _timeDim = NULL;
  _gateDim = NULL;

  _azimuthVar = NULL;
  _elevationVar = NULL;
  
  _DataType_attr.clear();
  _ScanType_attr.clear();
  _attributes_attr.clear();
  _Wavelength_unit_attr.clear();
  _Nyquist_Vel_unit_attr.clear();
  _radarName_unit_attr.clear();
  _radarName_value_attr.clear();
  _vcp_unit_attr.clear();
  _vcp_value_attr.clear();
  _ElevationUnits_attr.clear();
  _AzimuthUnits_attr.clear();
  _RangeToFirstGateUnits_attr.clear();
  _RadarParameters_attr.clear();
  _PRF_unit_attr.clear();
  _PulseWidth_unit_attr.clear();
  _MaximumRange_unit_attr.clear();
  _ProcessParameters_attr.clear();
  _RadarKit_VCP_Definition_attr.clear();
  _Waveform_attr.clear();
  _CreatedBy_attr.clear();
  _ContactInformation_attr.clear();
  _NCProperties_attr.clear();

  _dTimes.clear();
  _rayTimesIncrease = true;
  _nTimes = 0;
  _refTimeSecsFile = 0;

  _rangeKm.clear();
  _gateSpacingIsConstant = true;

  _latitudeDeg = 0.0;
  _longitudeDeg = 0.0;
  _altitudeKm = 0.0;

  _azimuths.clear();
  _elevations.clear();

  _title.clear();
  _institution.clear();
  _references.clear();
  _source.clear();
  _history.clear();
  _comment.clear();
  _statusXml.clear(); // global attributes

  _siteName.clear();
  _scanName.clear();
  _scanId = 0;
  _instrumentName.clear();

  _volumeNumber = 0;
  _instrumentType = Radx::INSTRUMENT_TYPE_RADAR;
  _platformType = Radx::PLATFORM_TYPE_FIXED;
  _primaryAxis = Radx::PRIMARY_AXIS_Z;
  
}

/////////////////////////////////////////////////////////
// Check if specified file is CfRadial format
// Returns true if supported, false otherwise

bool RaxpolNcRadxFile::isSupported(const string &path)

{
  
  if (isRaxpolNc(path)) {
    return true;
  }
  return false;

}

////////////////////////////////////////////////////////////
// Check if this is a RaxpolNc file
// Returns true on success, false on failure

bool RaxpolNcRadxFile::isRaxpolNc(const string &path)
  
{

  clear();
  
  // open file

  if (_file.openRead(path)) {
    if (_verbose) {
      cerr << "DEBUG - not RaxpolNc file" << endl;
      cerr << _file.getErrStr() << endl;
    }
    return false;
  }

  // read global attributes
  
  if (_readGlobalAttributes()) {
    return -1;
  }
  
  // read dimensions
  
  if (_readDimensions()) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not RaxpolNc file" << endl;
      cerr << _errStr << endl;
    }
    return false;
  }

  // check for radar name in global attributes

  string radarName;
  _file.readGlobAttr("radarName-value", radarName);
  transform(radarName.begin(), radarName.end(), radarName.begin(), ::tolower); 
  if (radarName.find("raxpol") == string::npos) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not RaxpolNc file" << endl;
      cerr << "  Global attr Radar is not RAXPOL" << endl;
    }
    return false;
  }

  Nc3Var *azVar = _file.getNc3File()->get_var("Azimuth");
  if (azVar == NULL) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not RaxpolNc file" << endl;
      cerr << "  Azimuth variable missing" << endl;
    }
    return false;
  }

  Nc3Var *elVar = _file.getNc3File()->get_var("Elevation");
  if (elVar == NULL) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not RaxpolNc file" << endl;
      cerr << "  Elevation variable missing" << endl;
    }
    return false;
  }

  Nc3Var *beamwidthVar = _file.getNc3File()->get_var("Beamwidth");
  if (beamwidthVar == NULL) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not RaxpolNc file" << endl;
      cerr << "  Beamwidth variable missing" << endl;
    }
    return false;
  }

  Nc3Var *gateWidthVar = _file.getNc3File()->get_var("GateWidth");
  if (gateWidthVar == NULL) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not RaxpolNc file" << endl;
      cerr << "  GateWidth variable missing" << endl;
    }
    return false;
  }

  // file is an RaxpolNc file

  _file.close();
  return true;

}

/////////////////////////////////////////////////////////
// Write data from volume to specified directory
//
// Writes not supported for this class
// Use NcfRadxFile methods instead.
//
// Returns 0 on success, -1 on failure

int RaxpolNcRadxFile::writeToDir(const RadxVol &vol,
                                 const string &dir,
                                 bool addDaySubDir,
                                 bool addYearSubDir)
  
{

  // Writing RaxpolNc files is not supported
  // therefore write in CF Radial format instead
  
  cerr << "WARNING - RaxpolNcRadxFile::writeToDir" << endl;
  cerr << "  Writing RaxpolNc format files not supported" << endl;
  cerr << "  Will write CfRadial file instead" << endl;
  
  // set up NcfRadxFile object

  NcfRadxFile ncfFile;
  ncfFile.copyWriteDirectives(*this);

  // perform write

  int iret = ncfFile.writeToDir(vol, dir, addDaySubDir, addYearSubDir);

  // set return values

  _errStr = ncfFile.getErrStr();
  _dirInUse = ncfFile.getDirInUse();
  _pathInUse = ncfFile.getPathInUse();
  vol.setPathInUse(_pathInUse);

  return iret;

}

/////////////////////////////////////////////////////////
// Write data from volume to specified path
//
// Writes not supported for this class
// Use NcfRadxFile methods instead.
//
// Returns 0 on success, -1 on failure

int RaxpolNcRadxFile::writeToPath(const RadxVol &vol,
                                  const string &path)
  
{

  // Writing RaxpolNc files is not supported
  // therefore write in CF Radial format instead

  cerr << "WARNING - RaxpolNcRadxFile::writeToPath" << endl;
  cerr << "  Writing RaxpolNc format files not supported" << endl;
  cerr << "  Will write CfRadial file instead" << endl;

  // set up NcfRadxFile object

  NcfRadxFile ncfFile;
  ncfFile.copyWriteDirectives(*this);

  // perform write

  int iret = ncfFile.writeToPath(vol, path);

  // set return values

  _errStr = ncfFile.getErrStr();
  _pathInUse = ncfFile.getPathInUse();
  vol.setPathInUse(_pathInUse);

  return iret;

}

////////////////////////////////////////////////
// get the date and time from a dorade file path
// returns 0 on success, -1 on failure

int RaxpolNcRadxFile::getTimeFromPath(const string &path, RadxTime &rtime)

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
    char cc;
    if (sscanf(start, "%4d%2d%2d%1c%2d%2d%2d",
               &year, &month, &day, &cc, &hour, &min, &sec) == 7) {
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

/////////////////////////////////////////////////////////
// print summary after read

void RaxpolNcRadxFile::print(ostream &out) const
  
{
  
  out << "=============== RaxpolNcRadxFile ===============" << endl;
  RadxFile::print(out);
  out << "  title: " << _title << endl;
  out << "  institution: " << _institution << endl;
  out << "  references: " << _references << endl;
  out << "  source: " << _source << endl;
  out << "  history: " << _history << endl;
  out << "  comment: " << _comment << endl;
  out << "  statusXml: " << _statusXml << endl;
  out << "  siteName: " << _siteName << endl;
  out << "  scanName: " << _scanName << endl;
  out << "  scanId: " << _scanId << endl;
  out << "  instrumentName: " << _instrumentName << endl;
  out << "  refTimeSecsFile: " << RadxTime::strm(_refTimeSecsFile) << endl;
  out << "  volumeNumber: " << _volumeNumber << endl;
  out << "  instrumentType: " 
      << Radx::instrumentTypeToStr(_instrumentType) << endl;
  out << "  platformType: " << Radx::platformTypeToStr(_platformType) << endl;
  out << "  primaryAxis: " << Radx::primaryAxisToStr(_primaryAxis) << endl;
  out << "  latitude: " << _latitudeDeg << endl;
  out << "  longitude: " << _longitudeDeg << endl;
  out << "  altitude: " << _altitudeKm << endl;
  out << "  frequencyHz: " << _frequencyHz << endl;
  
  out << "  startRangeKm: " << _remap.getStartRangeKm() << endl;
  out << "  gateSpacingKm: " << _remap.getGateSpacingKm() << endl;
  out << "  gateSpacingIsConstant: " << _gateSpacingIsConstant << endl;
  out << "===========================================" << endl;

}

////////////////////////////////////////////////////////////
// Print native data in uf file
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int RaxpolNcRadxFile::printNative(const string &path, ostream &out,
                                  bool printRays, bool printData)
  
{

  _addErrStr("ERROR - RaxpolNcRadxFile::printNative");
  _addErrStr("  Native print does not apply to NetCDF file: ", path);
  _addErrStr("  Use 'ncdump' instead");
  return -1;

}

////////////////////////////////////////////////////////////
// Read in data from specified path, load up volume object.
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs

int RaxpolNcRadxFile::readFromPath(const string &path,
                                   RadxVol &vol)
  
{

  _initForRead(path, vol);
  
  if (_debug) {
    cerr << "Reading path: " << path << endl;
  }

  _addErrStr("ERROR - RaxpolNcRadxFile::readFromPath");
  _addErrStr("  Path: ", path);
    
  // clear tmp rays
  
  _nTimes = 0;
  _rays.clear();

  // open file

  if (_file.openRead(path)) {
    _addErrStr(_file.getErrStr());
    return -1;
  }
  
  // read global attributes
  
  if (_readGlobalAttributes()) {
    return -1;
  }
  
  // read dimensions
  
  if (_readDimensions()) {
    return -1;
  }

  // set the times

  _setTimes();

  // set time variable now if that is all that is needed
  
  if (_readTimesOnly) {
    return 0;
  }
  
  // set position variables - lat/lon/alt
  
  _setPositionVariables();

  // set range geometry

  _setRangeGeometry();
  
  // read in ray variables

  if (_readRayVariables()) {
    return -1;
  }

  if (_readMetadataOnly) {

    // read field variables
    
    if (_readFieldVariables(true)) {
      return -1;
    }
    
  } else {

    // create the rays to be read in, filling out the metadata
    
    if (_createRays(path)) {
      return -1;
    }
    
    // add field variables to file rays
    
    if (_readFieldVariables(false)) {
      return -1;
    }

  }

  // close file

  _file.close();

  // append to read paths
  
  _readPaths.push_back(path);

  // check we have at least 1 ray

  if (_rays.size() < 1) {
    _addErrStr("  No rays found");
    return -1;
  }
  
  // load the data into the read volume

  if (_loadReadVolume()) {
    return -1;
  }
  
  // compute fixed angles as mean angle from sweeps
  
  _computeFixedAngles();
  
  // set format as read

  _fileFormat = FILE_FORMAT_RAXPOL_NC;

  // clean up

  _clearRayVariables();
  _dTimes.clear();

  return 0;

}

///////////////////////////////////
// read in the dimensions

int RaxpolNcRadxFile::_readDimensions()

{

  // read required dimensions

  int iret = 0;
  if (_sweepMode == Radx::SWEEP_MODE_RHI) {
    iret |= _file.readDim("Elevation", _timeDim);
  } else {
    iret |= _file.readDim("Azimuth", _timeDim);
  }
  if (iret == 0) {
    _nTimes = _timeDim->size();
  }

  _nGates = 0;
  iret |= _file.readDim("Gate", _gateDim);
  if (iret == 0) {
    _nGates = _gateDim->size();
  }
  
  if (iret) {
    _addErrStr("ERROR - RaxpolNcRadxFile::_file.readDimensions");
    return -1;
  }

  return 0;

}

///////////////////////////////////
// read the global attributes

int RaxpolNcRadxFile::_readGlobalAttributes()

{

  // string attributes

  _file.readGlobAttr("DataType", _DataType_attr);
  _file.readGlobAttr("ScanType", _ScanType_attr);
  _file.readGlobAttr("attributes", _attributes_attr);
  _file.readGlobAttr("Wavelength-unit", _Wavelength_unit_attr);
  _file.readGlobAttr("Nyquist_Vel-unit", _Nyquist_Vel_unit_attr);
  _file.readGlobAttr("radarName-unit", _radarName_unit_attr);
  _file.readGlobAttr("radarName-value", _radarName_value_attr);
  _file.readGlobAttr("vcp-unit", _vcp_unit_attr);
  _file.readGlobAttr("vcp-value", _vcp_value_attr);
  _file.readGlobAttr("ElevationUnits", _ElevationUnits_attr);
  _file.readGlobAttr("AzimuthUnits", _AzimuthUnits_attr);
  _file.readGlobAttr("RangeToFirstGateUnits", _RangeToFirstGateUnits_attr);
  _file.readGlobAttr("RadarParameters", _RadarParameters_attr);
  _file.readGlobAttr("PRF-unit", _PRF_unit_attr);
  _file.readGlobAttr("PulseWidth-unit", _PulseWidth_unit_attr);
  _file.readGlobAttr("MaximumRange-unit", _MaximumRange_unit_attr);
  _file.readGlobAttr("ProcessParameters", _ProcessParameters_attr);
  _file.readGlobAttr("RadarKit-VCP-Definition", _RadarKit_VCP_Definition_attr);
  _file.readGlobAttr("Waveform", _Waveform_attr);
  _file.readGlobAttr("CreatedBy", _CreatedBy_attr);
  _file.readGlobAttr("ContactInformation", _ContactInformation_attr);
  _file.readGlobAttr("_NCProperties", _NCProperties_attr);

  // integer attributes

  _file.readGlobAttr("Time", _Time_attr);
  _file.readGlobAttr("PRF-value", _PRF_value_attr);

  // float attributes

  _file.readGlobAttr("Latitude", _Latitude_attr);
  _file.readGlobAttr("LatitudeDouble", _LatitudeDouble_attr);
  _file.readGlobAttr("Longitude", _Longitude_attr);
  _file.readGlobAttr("LongitudeDouble", _LongitudeDouble_attr);
  _file.readGlobAttr("Heading", _Heading_attr);
  _file.readGlobAttr("Height", _Height_attr);
  _file.readGlobAttr("FractionalTime", _FractionalTime_attr);
  _file.readGlobAttr("Wavelength-value", _Wavelength_value_attr);
  _file.readGlobAttr("Nyquist_Vel-value", _Nyquist_Vel_value_attr);
  _file.readGlobAttr("Elevation", _Elevation_attr);
  _file.readGlobAttr("Azimuth", _Azimuth_attr);
  _file.readGlobAttr("GateSize", _GateSize_attr);
  _file.readGlobAttr("RangeToFirstGate", _RangeToFirstGate_attr);
  _file.readGlobAttr("MissingData", _MissingData_attr);
  _file.readGlobAttr("RangeFolded", _RangeFolded_attr);
  _file.readGlobAttr("PulseWidth-value", _PulseWidth_value_attr);
  _file.readGlobAttr("MaximumRange-value", _MaximumRange_value_attr);
  _file.readGlobAttr("NoiseH-ADU", _NoiseH_ADU_attr);
  _file.readGlobAttr("NoiseV-ADU", _NoiseV_ADU_attr);
  _file.readGlobAttr("SystemZCalH-dB", _SystemZCalH_dB_attr);
  _file.readGlobAttr("SystemZCalV-dB", _SystemZCalV_dB_attr);
  _file.readGlobAttr("SystemDCal-dB", _SystemDCal_dB_attr);
  _file.readGlobAttr("SystemPCal-Radians", _SystemPCal_Radians_attr);
  _file.readGlobAttr("ZCalH1-dB", _ZCalH1_dB_attr);
  _file.readGlobAttr("ZCalV1-dB", _ZCalV1_dB_attr);
  _file.readGlobAttr("ZCalH2-dB", _ZCalH2_dB_attr);
  _file.readGlobAttr("ZCalV2-dB", _ZCalV2_dB_attr);
  _file.readGlobAttr("DCal1-dB", _DCal1_dB_attr);
  _file.readGlobAttr("DCal2-dB", _DCal2_dB_attr);
  _file.readGlobAttr("PCal1-Radians", _PCal1_Radians_attr);
  _file.readGlobAttr("PCal2-Radians", _PCal2_Radians_attr);
  _file.readGlobAttr("SNRThreshold-dB", _SNRThreshold_dB_attr);
  _file.readGlobAttr("SQIThreshold-dB", _SQIThreshold_dB_attr);

  _title = "RAXPOL radar data";
  _institution = _ContactInformation_attr;
  _references = _attributes_attr;
  _source = _CreatedBy_attr;
  _history = _NCProperties_attr;
  _comment = "";
  
  if (_ScanType_attr.find("RHI") != string::npos) {
    _sweepMode = Radx::SWEEP_MODE_RHI;
  } else if (_ScanType_attr.find("PPI") != string::npos) {
    // _sweepMode = Radx::SWEEP_MODE_SECTOR;
    _sweepMode = Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE;
  } else {
    _sweepMode = Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE;
  }

  _instrumentName = _radarName_value_attr;
  _siteName = "unknown";
  _scanName = _ScanType_attr;

  double wavelengthM = _Wavelength_value_attr;
  _frequencyHz = Radx::LIGHT_SPEED / wavelengthM;
  
  _prfHz = _PRF_value_attr;
  _prtSec = 1.0 / _prfHz;
  _nyquistMps = _Nyquist_Vel_value_attr;

  _pulseWidthUsec = _PulseWidth_value_attr;
  _nSamples = Radx::missingMetaInt;
  
  // set the status XML from the attributes
  
  _statusXml.clear();
  _statusXml += RadxXml::writeStartTag("STATUS", 0);
  for (int ii = 0; ii < _file.getNc3File()->num_atts(); ii++) {
    Nc3Att *att = _file.getNc3File()->get_att(ii);
    if (att != NULL) {
      const char* strc = att->as_string(0);
      string val(strc);
      delete[] strc;
      string name(att->name());
      delete att;
      _statusXml += RadxXml::writeString(name, 1, val);
    }
  }
  _statusXml += RadxXml::writeEndTag("STATUS", 0);

  return 0;

}

///////////////////////////////////
// set the times

void RaxpolNcRadxFile::_setTimes()

{

  _dTimes.resize(_nTimes);
  _refTimeSecsFile = (time_t) _Time_attr;
  if (_nTimes > 0) {
    double startTime = _Time_attr + _FractionalTime_attr;
    for (size_t ii = 0; ii < _nTimes; ii++) {
      double deltaTime = ii / 180.0;
      _dTimes[ii] = startTime + deltaTime;
    }
  }
  
}

///////////////////////////////////
// set the range geometry

void RaxpolNcRadxFile::_setRangeGeometry()
  
{

  _startRangeKm = _RangeToFirstGate_attr / 1000.0;
  _gateSpacingKm = _GateSize_attr / 1000.0;
  _gateSpacingIsConstant = true;
  _geom.setRangeGeom(_startRangeKm, _gateSpacingKm);

  _rangeKm.resize(_nGates);
  for (size_t ii = 0; ii < _nGates; ii++) {
    _rangeKm[ii] = _startRangeKm + ii * _gateSpacingKm;
  }

}

///////////////////////////////////
// read the position variables

void RaxpolNcRadxFile::_setPositionVariables()

{

  _latitudeDeg = _LatitudeDouble_attr;
  _longitudeDeg = _LongitudeDouble_attr;
  _altitudeKm = _Height_attr / 1000.0;

}

///////////////////////////////////
// clear the ray variables

void RaxpolNcRadxFile::_clearRayVariables()

{

  _azimuths.clear();
  _elevations.clear();

}

///////////////////////////////////
// read in ray variables

int RaxpolNcRadxFile::_readRayVariables()

{

  _clearRayVariables();
  int iret = 0;
  
  _readRayVar(_azimuthVar, "Azimuth", _azimuths);
  if ((int) _azimuths.size() != _timeDim->size()) {
    _addErrStr("ERROR - Azimuth variable required");
    iret = -1;
  }
  
  _readRayVar(_elevationVar, "Elevation", _elevations);
  if ((int) _elevations.size() != _timeDim->size()) {
    _addErrStr("ERROR - Elevation variable required");
    iret = -1;
  }

  if (iret) {
    _addErrStr("ERROR - RaxpolNcRadxFile::_readRayVariables");
    return -1;
  }

  return 0;

}

///////////////////////////////////
// create the rays to be read in
// and set meta data

int RaxpolNcRadxFile::_createRays(const string &path)

{

  // create the rays
  
  _rays.clear();

  for (size_t iray = 0; iray < _nTimes; iray++) {
    
    // new ray
    
    RadxRay *ray = new RadxRay;
    
    // set time
    
    double dusecs = _dTimes[iray];
    time_t usecs = (time_t) dusecs;
    double subsecs = dusecs - usecs;
    int nanoSecs = (int) (subsecs * 1.0e9);
    ray->setTime(usecs, nanoSecs);

    // sweep info
    
    ray->setSweepNumber(0);
    ray->setAzimuthDeg(_azimuths[iray]);
    ray->setElevationDeg(_elevations[iray]);
    ray->setSweepMode(_sweepMode);
    
    // copy geom to rays
    
    ray->copyRangeGeom(_geom);

    // ray metadata

    ray->setPrtSec(_prtSec);
    ray->setNyquistMps(_nyquistMps);
    ray->setPulseWidthUsec(_pulseWidthUsec);
    ray->setUnambigRangeKm(_MaximumRange_value_attr);
    ray->setPolarizationMode(Radx::POL_MODE_HV_SIM);
    
    // add to ray vector

    _rays.push_back(ray);

  } // ii

  return 0;

}

////////////////////////////////////////////
// read the field variables

int RaxpolNcRadxFile::_readFieldVariables(bool metaOnly)

{

  // loop through the variables, adding data fields as appropriate
  
  for (int ivar = 0; ivar < _file.getNc3File()->num_vars(); ivar++) {
    
    Nc3Var* var = _file.getNc3File()->get_var(ivar);
    if (var == NULL) {
      continue;
    }

    int numDims = var->num_dims();
    // we need fields with 2 dimensions
    if (numDims != 2) {
      continue;
    }
    // check that we have the correct dimensions
    Nc3Dim* timeDim = var->get_dim(0);
    Nc3Dim* gateDim = var->get_dim(1);
    if (timeDim != _timeDim || gateDim != _gateDim) {
      continue;
    }
    
    // check the type
    string fieldName = var->name();
    Nc3Type ftype = var->type();
    if (ftype != nc3Double && ftype != nc3Float &&
        ftype != nc3Int && ftype != nc3Short) {
      // not a valid type
      if (_verbose) {
        cerr << "DEBUG - RaxpolNcRadxFile::_readFieldVariables" << endl;
        cerr << "  -->> rejecting field: " << fieldName << endl;
        cerr << "  -->> Should be short, int, float or double: " << fieldName << endl;
      }
      continue;
    }
    
    // check that we need this field
    
    if (!isFieldRequiredOnRead(fieldName)) {
      if (_verbose) {
        cerr << "DEBUG - RaxpolNcRadxFile::_readFieldVariables" << endl;
        cerr << "  -->> rejecting field: " << fieldName << endl;
      }
      continue;
    }

    if (_verbose) {
      cerr << "DEBUG - RaxpolNcRadxFile::_readFieldVariables" << endl;
      cerr << "  -->> adding field: " << fieldName << endl;
    }
    
    // set names, units, etc
    
    string name = var->name();
    
    string standardName;
    Nc3Att *standardNameAtt = var->get_att("standard_name");
    if (standardNameAtt != NULL) {
      standardName = Nc3xFile::asString(standardNameAtt);
      delete standardNameAtt;
    }
    
    string longName;
    Nc3Att *longNameAtt = var->get_att("long_name");
    if (longNameAtt != NULL) {
      longName = Nc3xFile::asString(longNameAtt);
      delete longNameAtt;
    }
    
    string units;
    Nc3Att *unitsAtt = var->get_att("Units");
    if (unitsAtt != NULL) {
      units = Nc3xFile::asString(unitsAtt);
      delete unitsAtt;
    }

    // folding

    bool fieldFolds = false;
    float foldLimitLower = Radx::missingMetaFloat;
    float foldLimitUpper = Radx::missingMetaFloat;
    if (name.find("VR") != string::npos ||
        name.find("VEL") != string::npos) {
      fieldFolds = true;
      foldLimitLower = _nyquistMps * -1.0;
      foldLimitUpper = _nyquistMps;
    }
    
    // if metadata only, don't read in fields

    if (metaOnly) {
      if (!_readVol->fieldExists(name)) {
        RadxField *field = new RadxField(name, units);
        field->setLongName(longName);
        field->setStandardName(standardName);
        if (fieldFolds &&
            foldLimitLower != Radx::missingMetaFloat &&
            foldLimitUpper != Radx::missingMetaFloat) {
          field->setFieldFolds(foldLimitLower, foldLimitUpper);
        }
        _readVol->addField(field);
      }
      continue;
    }

    int iret = 0;
    bool isDiscrete = false;

    switch (var->type()) {
      case nc3Double: {
        if (_addFl64FieldToRays(var, name, units, standardName, longName,
                                isDiscrete, fieldFolds,
                                foldLimitLower, foldLimitUpper)) {
          iret = -1;
        }
        break;
      }
      case nc3Float: {
        if (_addFl32FieldToRays(var, name, units, standardName, longName,
                                isDiscrete, fieldFolds,
                                foldLimitLower, foldLimitUpper)) {
          iret = -1;
        }
        break;
      }
      case nc3Int: {
        if (_addSi32FieldToRays(var, name, units, standardName, longName,
                                isDiscrete, fieldFolds,
                                foldLimitLower, foldLimitUpper)) {
          iret = -1;
        }
        break;
      }
      case nc3Short: {
        if (_addSi16FieldToRays(var, name, units, standardName, longName,
                                isDiscrete, fieldFolds,
                                foldLimitLower, foldLimitUpper)) {
          iret = -1;
        }
        break;
      }
      default: {
        iret = -1;
        // will not reach here because of earlier check on type
      }
        
    } // switch
    
    if (iret) {
      _addErrStr("ERROR - RaxpolNcRadxFile::_readFieldVariables");
      _addErrStr("  cannot read field name: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
      return -1;
    }
    
  } // ivar
  
  return 0;

}

///////////////////////////////////
// read a ray variable - double

int RaxpolNcRadxFile::_readRayVar(Nc3Var* &var, const string &name,
                                  vector<double> &vals, bool required)
  
{

  vals.clear();

  // get var

  var = _getRayVar(name, required);
  if (var == NULL) {
    if (!required) {
      for (size_t ii = 0; ii < _nTimes; ii++) {
        vals.push_back(Radx::missingMetaDouble);
      }
      clearErrStr();
      return 0;
    } else {
      _addErrStr("ERROR - RaxpolNcRadxFile::_readRayVar");
      return -1;
    }
  }

  // load up data

  double *data = new double[_nTimes];
  double *dd = data;
  int iret = 0;
  if (var->get(data, _nTimes)) {
    for (size_t ii = 0; ii < _nTimes; ii++, dd++) {
      vals.push_back(*dd);
    }
  } else {
    if (!required) {
      for (size_t ii = 0; ii < _nTimes; ii++) {
        vals.push_back(Radx::missingMetaDouble);
      }
      clearErrStr();
    } else {
      _addErrStr("ERROR - RaxpolNcRadxFile::_readRayVar");
      _addErrStr("  Cannot read variable: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
      iret = -1;
    }
  }
  delete[] data;
  return iret;

}

///////////////////////////////////
// read a ray variable - float

int RaxpolNcRadxFile::_readRayVar(Nc3Var* &var, const string &name,
                                  vector<float> &vals, bool required)
  
{

  vals.clear();

  // get var

  var = _getRayVar(name, required);
  if (var == NULL) {
    if (!required) {
      for (size_t ii = 0; ii < _nTimes; ii++) {
        vals.push_back(Radx::missingMetaFloat);
      }
      clearErrStr();
      return 0;
    } else {
      _addErrStr("ERROR - RaxpolNcRadxFile::_readRayVar");
      return -1;
    }
  }

  // load up data

  float *data = new float[_nTimes];
  float *dd = data;
  int iret = 0;
  if (var->get(data, _nTimes)) {
    for (size_t ii = 0; ii < _nTimes; ii++, dd++) {
      vals.push_back(*dd);
    }
  } else {
    if (!required) {
      for (size_t ii = 0; ii < _nTimes; ii++) {
        vals.push_back(Radx::missingMetaFloat);
      }
      clearErrStr();
    } else {
      _addErrStr("ERROR - RaxpolNcRadxFile::_readRayVar");
      _addErrStr("  Cannot read variable: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
      iret = -1;
    }
  }
  delete[] data;
  return iret;

}

///////////////////////////////////
// read a ray variable - integer

int RaxpolNcRadxFile::_readRayVar(Nc3Var* &var, const string &name,
                                  vector<int> &vals, bool required)

{

  vals.clear();

  // get var
  
  var = _getRayVar(name, required);
  if (var == NULL) {
    if (!required) {
      for (size_t ii = 0; ii < _nTimes; ii++) {
        vals.push_back(Radx::missingMetaInt);
      }
      clearErrStr();
      return 0;
    } else {
      _addErrStr("ERROR - RaxpolNcRadxFile::_readRayVar");
      return -1;
    }
  }

  // load up data

  int *data = new int[_nTimes];
  int *dd = data;
  int iret = 0;
  if (var->get(data, _nTimes)) {
    for (size_t ii = 0; ii < _nTimes; ii++, dd++) {
      vals.push_back(*dd);
    }
  } else {
    if (!required) {
      for (size_t ii = 0; ii < _nTimes; ii++) {
        vals.push_back(Radx::missingMetaInt);
      }
      clearErrStr();
    } else {
      _addErrStr("ERROR - RaxpolNcRadxFile::_readRayVar");
      _addErrStr("  Cannot read variable: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
      iret = -1;
    }
  }
  delete[] data;
  return iret;

}

///////////////////////////////////
// get a ray variable by name
// returns NULL on failure

Nc3Var* RaxpolNcRadxFile::_getRayVar(const string &name, bool required)

{

  // get var
  
  Nc3Var *var = _file.getNc3File()->get_var(name.c_str());
  if (var == NULL) {
    if (required) {
      _addErrStr("ERROR - RaxpolNcRadxFile::_getRayVar");
      _addErrStr("  Cannot read variable, name: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
    }
    return NULL;
  }

  // check time dimension
  
  if (var->num_dims() < 1) {
    if (required) {
      _addErrStr("ERROR - RaxpolNcRadxFile::_getRayVar");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has no dimensions");
    }
    return NULL;
  }
  Nc3Dim *timeDim = var->get_dim(0);
  if (timeDim != _timeDim) {
    if (required) {
      _addErrStr("ERROR - RaxpolNcRadxFile::_getRayVar");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has incorrect dimension, dim name: ", 
                 timeDim->name());
      _addErrStr("  should be: ", "time");
    }
    return NULL;
  }

  return var;

}

//////////////////////////////////////////////////////////////
// Add fl64 fields to _rays
// Returns 0 on success, -1 on failure

int RaxpolNcRadxFile::_addFl64FieldToRays(Nc3Var* var,
                                          const string &name,
                                          const string &units,
                                          const string &standardName,
                                          const string &longName,
                                          bool isDiscrete,
                                          bool fieldFolds,
                                          float foldLimitLower,
                                          float foldLimitUpper)
  
{

  // allocate arrays

  RadxArray<Radx::fl64> data_, rayData_;
  Radx::fl64 *data = data_.alloc(_nTimes * _nGates);
  Radx::fl64 *rayData = rayData_.alloc(_nGates);

  // get data from array

  int iret = !var->get(data, _nTimes, _nGates);
  if (iret) {
    return -1;
  }

  // set missing value
  
  Radx::fl64 missingVal = Radx::missingFl64;
  Nc3Att *missingValueAtt = var->get_att("_FillValue");
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_double(0);
    delete missingValueAtt;
  }
  
  // load field on rays
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    
    int index = iray * _nGates;
    for (size_t igate = 0; igate < _nGates; igate++, index++) {
      rayData[igate] = data[index];
    }
    
    RadxField *field =
      _rays[iray]->addField(name, units, _nGates,
                            missingVal,
                            rayData,
                            true);
    
    field->setStandardName(standardName);
    field->setLongName(longName);
    
    if (fieldFolds &&
        foldLimitLower != Radx::missingMetaFloat &&
        foldLimitUpper != Radx::missingMetaFloat) {
      field->setFieldFolds(foldLimitLower, foldLimitUpper);
    }
    if (isDiscrete) {
      field->setIsDiscrete(true);
    }

  }

  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add fl32 fields to _rays
// Returns 0 on success, -1 on failure

int RaxpolNcRadxFile::_addFl32FieldToRays(Nc3Var* var,
                                          const string &name,
                                          const string &units,
                                          const string &standardName,
                                          const string &longName,
                                          bool isDiscrete,
                                          bool fieldFolds,
                                          float foldLimitLower,
                                          float foldLimitUpper)
  
{

  // allocate arrays

  RadxArray<Radx::fl32> data_, rayData_;
  Radx::fl32 *data = data_.alloc(_nTimes * _nGates);
  Radx::fl32 *rayData = rayData_.alloc(_nGates);

  // get data from array
  
  int iret = !var->get(data, _nTimes, _nGates);
  if (iret) {
    return -1;
  }

  // set missing value
  
  Radx::fl32 missingVal = Radx::missingFl32;
  Nc3Att *missingValueAtt = var->get_att("_FillValue");
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_double(0);
    delete missingValueAtt;
  }

  // for phidp, convert to degrees
  
  string _units = units;
  if (name == "PHIDP") {
    _units = "deg";
    size_t nVals = _nTimes * _nGates;
    for (size_t ii = 0; ii < nVals; ii++) {
      if (data[ii] != missingVal) {
        data[ii] = Radx::toDegrees(data[ii]);
      }
    }
  }

  // load field on rays

  for (size_t iray = 0; iray < _rays.size(); iray++) {
    
    int index = iray * _nGates;
    for (size_t igate = 0; igate < _nGates; igate++, index++) {
      rayData[igate] = data[index];
    }
    
    RadxField *field =
      _rays[iray]->addField(name, _units, _nGates,
                            missingVal,
                            rayData,
                            true);
    
    field->setStandardName(standardName);
    field->setLongName(longName);
    
    if (fieldFolds &&
        foldLimitLower != Radx::missingMetaFloat &&
        foldLimitUpper != Radx::missingMetaFloat) {
      field->setFieldFolds(foldLimitLower, foldLimitUpper);
    }
    if (isDiscrete) {
      field->setIsDiscrete(true);
    }

  }
  
  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add si32 fields to _rays
// Returns 0 on success, -1 on failure

int RaxpolNcRadxFile::_addSi32FieldToRays(Nc3Var* var,
                                          const string &name,
                                          const string &units,
                                          const string &standardName,
                                          const string &longName,
                                          bool isDiscrete,
                                          bool fieldFolds,
                                          float foldLimitLower,
                                          float foldLimitUpper)
  
{

  // allocate arrays

  RadxArray<Radx::si32> data_, rayData_;
  Radx::si32 *data = data_.alloc(_nTimes * _nGates);
  Radx::si32 *rayData = rayData_.alloc(_nGates);

  // get data from array

  int iret = !var->get(data, _nTimes * _nGates);
  if (iret) {
    return -1;
  }

  // set missing value, scale factor and add offset

  Radx::si32 missingVal = Radx::missingSi32;
  Nc3Att *missingValueAtt = var->get_att("_FillValue");
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_double(0);
    delete missingValueAtt;
  }

  double scaleFactor = 1.0;
  Nc3Att *scaleFactorAtt = var->get_att("scale_factor");
  if (scaleFactorAtt != NULL) {
    scaleFactor = scaleFactorAtt->as_double(0);
    delete scaleFactorAtt;
  }
  
  double addOffset = 0.0;
  Nc3Att *addOffsetAtt = var->get_att("add_offset");
  if (addOffsetAtt != NULL) {
    addOffset = addOffsetAtt->as_double(0);
    delete addOffsetAtt;
  }
  
  // load field on rays

  for (size_t iray = 0; iray < _rays.size(); iray++) {
    
    int index = iray * _nGates;
    for (size_t igate = 0; igate < _nGates; igate++, index++) {
      rayData[igate] = data[index];
    }

    RadxField *field =
      _rays[iray]->addField(name, units, _nGates,
                            missingVal,
                            rayData,
                            scaleFactor,
                            addOffset,
                            true);
    
    field->setStandardName(standardName);
    field->setLongName(longName);
    
    if (fieldFolds &&
        foldLimitLower != Radx::missingMetaFloat &&
        foldLimitUpper != Radx::missingMetaFloat) {
      field->setFieldFolds(foldLimitLower, foldLimitUpper);
    }
    if (isDiscrete) {
      field->setIsDiscrete(true);
    }

  }
  
  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add si16 fields to _rays
// Returns 0 on success, -1 on failure

int RaxpolNcRadxFile::_addSi16FieldToRays(Nc3Var* var,
                                          const string &name,
                                          const string &units,
                                          const string &standardName,
                                          const string &longName,
                                          bool isDiscrete,
                                          bool fieldFolds,
                                          float foldLimitLower,
                                          float foldLimitUpper)
  
{

  // allocate arrays

  RadxArray<Radx::si16> data_, rayData_;
  Radx::si16 *data = data_.alloc(_nTimes * _nGates);
  Radx::si16 *rayData = rayData_.alloc(_nGates);

  // get data from array

  int iret = !var->get(data, _nTimes, _nGates);
  if (iret) {
    return -1;
  }

  // set missing value, scale factor and add offset

  Radx::si16 missingVal = Radx::missingSi16;
  Nc3Att *missingValueAtt = var->get_att("_FillValue");
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_double(0);
    delete missingValueAtt;
  }

  double scaleFactor = 1.0;
  Nc3Att *scaleFactorAtt = var->get_att("scale_factor");
  if (scaleFactorAtt != NULL) {
    scaleFactor = scaleFactorAtt->as_double(0);
    delete scaleFactorAtt;
  }
  
  double addOffset = 0.0;
  Nc3Att *addOffsetAtt = var->get_att("add_offset");
  if (addOffsetAtt != NULL) {
    addOffset = addOffsetAtt->as_double(0);
    delete addOffsetAtt;
  }
  
  // load field on rays

  for (size_t iray = 0; iray < _rays.size(); iray++) {
    
    int index = iray * _nGates;
    for (size_t igate = 0; igate < _nGates; igate++, index++) {
      rayData[igate] = data[index];
    }

    RadxField *field =
      _rays[iray]->addField(name, units, _nGates,
                            missingVal,
                            rayData,
                            scaleFactor,
                            addOffset,
                            true);
    
    field->setStandardName(standardName);
    field->setLongName(longName);
    
    if (fieldFolds &&
        foldLimitLower != Radx::missingMetaFloat &&
        foldLimitUpper != Radx::missingMetaFloat) {
      field->setFieldFolds(foldLimitLower, foldLimitUpper);
    }
    if (isDiscrete) {
      field->setIsDiscrete(true);
    }

  }
  
  return 0;
  
}

/////////////////////////////////////////////////////////
// load up the read volume with the data from this object

int RaxpolNcRadxFile::_loadReadVolume()
  
{

  // set metadata

  _readVol->setOrigFormat("DOE");
  _readVol->setVolumeNumber(_volumeNumber);
  _readVol->setInstrumentType(_instrumentType);
  _readVol->setPlatformType(_platformType);
  _readVol->setPrimaryAxis(_primaryAxis);

  _readVol->addFrequencyHz(_frequencyHz);

  _readVol->setTitle(_title);
  _readVol->setSource(_source);
  _readVol->setHistory(_history);
  _readVol->setInstitution(_institution);
  _readVol->setReferences(_references);
  _readVol->setComment(_comment);
  _readVol->setStatusXml(_statusXml);
  _readVol->setSiteName(_siteName);
  _readVol->setScanName(_scanName);
  _readVol->setScanId(_scanId);
  _readVol->setInstrumentName(_instrumentName);

  _readVol->setLatitudeDeg(_latitudeDeg);
  _readVol->setLongitudeDeg(_longitudeDeg);
  _readVol->setAltitudeKm(_altitudeKm);
  _readVol->setSensorHtAglM(3.0);

  _readVol->setRadarBeamWidthDegH(0.9);
  _readVol->setRadarBeamWidthDegV(0.9);

  _readVol->setRadarAntennaGainDbH(44.5);
  _readVol->setRadarAntennaGainDbV(44.5);

  _readVol->copyRangeGeom(_geom);

  for (int ii = 0; ii < (int) _rays.size(); ii++) {
    _rays[ii]->setVolumeNumber(_volumeNumber);
  }

  // add rays to vol - they will be freed by vol

  for (size_t ii = 0; ii < _rays.size(); ii++) {
    _readVol->addRay(_rays[ii]);
  }

  if (_readSetMaxRange) {
    _readVol->setMaxRangeKm(_readMaxRangeKm);
  }
  
  // memory responsibility has passed to the volume object, so clear
  // the vectors without deleting the objects to which they point

  _rays.clear();
  
  // load the sweep information from the rays

  _readVol->loadSweepInfoFromRays();

  // constrain the sweep data as appropriate
  
  if (_readFixedAngleLimitsSet) {
    if (_readVol->constrainByFixedAngle(_readMinFixedAngle, _readMaxFixedAngle,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - RaxpolNcRadxFile::_loadReadVolume");
      _addErrStr("  No data found within fixed angle limits");
      _addErrDbl("  min fixed angle: ", _readMinFixedAngle);
      _addErrDbl("  max fixed angle: ", _readMaxFixedAngle);
      return -1;
    }
  } else if (_readSweepNumLimitsSet) {
    if (_readVol->constrainBySweepNum(_readMinSweepNum, _readMaxSweepNum,
                                      _readStrictAngleLimits)) {
      _addErrStr("ERROR - RaxpolNcRadxFile::_loadReadVolume");
      _addErrStr("  No data found within sweep num limits");
      _addErrInt("  min sweep num: ", _readMinSweepNum);
      _addErrInt("  max sweep num: ", _readMaxSweepNum);
      return -1;
    }
  }

  // load the volume information from the rays

  _readVol->loadVolumeInfoFromRays();
  
  // check for indexed rays, set info on rays

  _readVol->checkForIndexedRays();

  return 0;

}

/////////////////////////////////////////////////////////////
// Compute the fixed angles by averaging the elevation angles
// on the sweeps

void RaxpolNcRadxFile::_computeFixedAngles()
  
{

  for (size_t isweep = 0; isweep < _readVol->getNSweeps(); isweep++) {

    RadxSweep &sweep = *_readVol->getSweeps()[isweep];

    double sumU = 0.0;
    double sumV = 0.0;
    
    for (size_t iray = sweep.getStartRayIndex();
         iray <= sweep.getEndRayIndex(); iray++) {
      const RadxRay &ray = *_readVol->getRays()[iray];
      double angle = ray.getElevationDeg();
      if (_sweepMode == Radx::SWEEP_MODE_RHI) {
        angle = ray.getAzimuthDeg();
      }
      double sinVal, cosVal;
      RadxComplex::sinCos(angle * Radx::DegToRad, sinVal, cosVal);
      sumU += cosVal;
      sumV += sinVal;
    }
    
    double meanAngle = atan2(sumV, sumU) * Radx::RadToDeg;
    double fixedAngle = ((int) (meanAngle * 100.0 + 0.5)) / 100.0;
    
    sweep.setFixedAngleDeg(fixedAngle);
      
    for (size_t iray = sweep.getStartRayIndex();
         iray <= sweep.getEndRayIndex(); iray++) {
      RadxRay &ray = *_readVol->getRays()[iray];
      ray.setFixedAngleDeg(fixedAngle);
    }

  } // isweep

  _readVol->loadFixedAnglesFromSweepsToRays();

}

/////////////////////////////////////////////////////////////////
// get list of field paths for the volume for the specified path

int RaxpolNcRadxFile::_getFieldPaths(const string &primaryPath,
                                     vector<string> &fileNames,
                                     vector<string> &filePaths,
                                     vector<string> &fieldNames)
  
{
  
  // init

  fileNames.clear();
  filePaths.clear();
  fieldNames.clear();

  // decompose the path to get the date/time prefix for the primary path
  // tokenize the primary file name
  // e.g. RAXPOL-20220129-171655-E2.0-Z.nc
  // or   RAXPOL-20220129-172114-A220.0-V.nc

  RadxPath rpath(primaryPath);
  const string &dir = rpath.getDirectory();
  const string &fileName = rpath.getFile();
  
  vector<string> primaryToks;
  RadxStr::tokenize(fileName, "-", primaryToks);
  if (primaryToks.size() < 5) {
    _addErrStr("ERROR - RaxpolNcRadxFile::_getFieldPaths");
    _addErrStr("  Bad file name: ", fileName);
    return -1;
  }

  // load up array of file names that match the prefix
  
  RadxReadDir rdir;
  if (rdir.open(dir.c_str()) == 0) {
    
    // Loop thru directory looking for the data file names
    // or forecast directories
    
    struct dirent *dp;
    for (dp = rdir.read(); dp != NULL; dp = rdir.read()) {
      
      string dName(dp->d_name);
      
      // exclude dir entries beginning with '.'
      
      if (dName[0] == '.') {
	continue;
      }

      // make sure we have RAXPOL files
      
      if (dName.find("RAXPOL") == string::npos) {
	continue;
      }

      // make sure we have .nc files
      
      if (dName.find(".nc") == string::npos) {
	continue;
      }

      // tokenize the file name

      vector<string> thisFileToks;
      RadxStr::tokenize(dName, "-", thisFileToks);
      if (thisFileToks.size() < 5) {
        continue;
      }
      
      if (primaryToks[1] == thisFileToks[1] &&
          primaryToks[2] == thisFileToks[2]) {

        // date matches
        
        fileNames.push_back(dName);

      } // if (dStr ...
      
    } // dp
    
    rdir.close();

  } // if (rdir ...

  // sort the file names

  sort(fileNames.begin(), fileNames.end());

  // load up the paths and field names

  for (size_t ii = 0; ii < fileNames.size(); ii++) {

    const string &fileName = fileNames[ii];

    size_t pos = fileName.find('.', 16);
    string fieldName = fileName.substr(16, pos - 16);
    fieldNames.push_back(fieldName);
    
    string dPath(dir);
    dPath += RadxPath::RADX_PATH_DELIM;
    dPath += fileName;
    filePaths.push_back(dPath);

  } // ii

  return 0;
  
}

