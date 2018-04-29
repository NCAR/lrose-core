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
// EdgeNcRadxFile.cc
//
// EdgeNcRadxFile object
//
// NetCDF data for radar radial data converted from EDGE format
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 2016
//
///////////////////////////////////////////////////////////////

#include <Radx/NcfRadxFile.hh>
#include <Radx/EdgeNcRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxArray.hh>
#include <Radx/RadxXml.hh>
#include <Radx/RadxReadDir.hh>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <algorithm>
using namespace std;

//////////////
// Constructor

EdgeNcRadxFile::EdgeNcRadxFile() : RadxFile()
  
{

  _ncFormat = NETCDF_CLASSIC;
  _readVol = NULL;
  _volumeNumber = 0;
  clear();

}

/////////////
// destructor

EdgeNcRadxFile::~EdgeNcRadxFile()

{ 
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void EdgeNcRadxFile::clear()
  
{

  clearErrStr();

  _file.close();

  _azimuthDim = NULL;
  _gateDim = NULL;

  _azimuthVar = NULL;
  _beamWidthVar = NULL;
  _gateWidthVar = NULL;

  _TypeName_attr.clear();
  _DataType_attr.clear();
  _Latitude_attr = 0.0;
  _Longitude_attr = 0.0;
  _Height_attr = 0;
  _Time_attr = -1;
  _FractionalTime_attr = 0.0;
  _attributes_attr.clear();
  _NyquistVelocity_unit_attr.clear();
  _NyquistVelocity_value_attr = 0.0;
  _vcp_unit_attr.clear();
  _vcp_value_attr = 0.0;
  _radarName_unit_attr.clear();
  _radarName_value_attr.clear();
  _ColorMap_unit_attr.clear();
  _ColorMap_value_attr.clear();
  _Elevation_attr = 0.0;
  _ElevationUnits_attr.clear();
  _MissingData_attr = -99900.0;
  _RangeFolded_attr = -99901.0;
  _RadarParameters_attr.clear();
  _PRF_unit_attr.clear();
  _PRF_value_attr = 0.0;
  _PulseWidth_unit_attr.clear();
  _PulseWidth_value_attr = 0.0;
  _MaximumRange_unit_attr.clear();
  _MaximumRange_value_attr = 0.0;
  _ConversionPlugin_attr.clear();

  _title.clear();
  _institution.clear();
  _references.clear();
  _source.clear();
  _history.clear();
  _comment.clear();

  _statusXml.clear(); // global attributes
  _siteName.clear();
  _scanName.clear();
  _instrumentName.clear();

  _scanId = 0;

  _rayTimesIncrease = true;
  _refTimeSecsFile = 0;

  _instrumentType = Radx::INSTRUMENT_TYPE_RADAR;
  _platformType = Radx::PLATFORM_TYPE_FIXED;
  _primaryAxis = Radx::PRIMARY_AXIS_Z;

  _latitude = 0.0;
  _longitude = 0.0;;
  _altitudeKm = 0.0;

  _rangeKm.clear();
  _gateSpacingIsConstant = true;

}

/////////////////////////////////////////////////////////
// Check if specified file is CfRadial format
// Returns true if supported, false otherwise

bool EdgeNcRadxFile::isSupported(const string &path)

{
  
  if (isEdgeNc(path)) {
    return true;
  }
  return false;

}

////////////////////////////////////////////////////////////
// Check if this is a EdgeNc file
// Returns true on success, false on failure

bool EdgeNcRadxFile::isEdgeNc(const string &path)
  
{

  clear();
  
  // open file

  if (_file.openRead(path)) {
    if (_verbose) {
      cerr << "DEBUG - not EdgeNc file" << endl;
      cerr << _file.getErrStr() << endl;
    }
    return false;
  }

  // read dimensions
  
  _firstFieldInSweep = true;
  if (_readDimensions()) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not EdgeNc file" << endl;
      cerr << _errStr << endl;
    }
    return false;
  }

  // check existence of some global attributes

  int iret = 0;
  if (_file.readGlobAttr("TypeName", _TypeName_attr) ||
      _file.readGlobAttr("DataType", _DataType_attr) ||
      _file.readGlobAttr("Time", _FractionalTime_attr) ||
      _file.readGlobAttr("radarName-value", _radarName_value_attr) ||
      _file.readGlobAttr("ConversionPlugin", _ConversionPlugin_attr)) {
    iret = -1;
  }
  _file.close();

  if (iret) {
    if (_verbose) {
      cerr << "DEBUG - not EdgeNc file" << endl;
    }
    return false;
  }
  
  // file has the correct dimensions and attributes, so it is an EdgeNc file
  
  return true;

}

/////////////////////////////////////////////////////////
// Write data from volume to specified directory
//
// Writes not supported for this class
// Use NcfRadxFile methods instead.
//
// Returns 0 on success, -1 on failure

int EdgeNcRadxFile::writeToDir(const RadxVol &vol,
                               const string &dir,
                               bool addDaySubDir,
                               bool addYearSubDir)
  
{

  // Writing EdgeNc files is not supported
  // therefore write in CF Radial format instead
  
  cerr << "WARNING - EdgeNcRadxFile::writeToDir" << endl;
  cerr << "  Writing EdgeNc format files not supported" << endl;
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

int EdgeNcRadxFile::writeToPath(const RadxVol &vol,
                                const string &path)
  
{

  // Writing EdgeNc files is not supported
  // therefore write in CF Radial format instead

  cerr << "WARNING - EdgeNcRadxFile::writeToPath" << endl;
  cerr << "  Writing EdgeNc format files not supported" << endl;
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

int EdgeNcRadxFile::getTimeFromPath(const string &path, RadxTime &rtime)

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

void EdgeNcRadxFile::print(ostream &out) const
  
{
  
  out << "=============== EdgeNcRadxFile ===============" << endl;
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
  out << "  latitude: " << _latitude << endl;
  out << "  longitude: " << _longitude << endl;
  out << "  altitude: " << _altitudeKm << endl;
  
  out << "  startRangeKm: " << _remap.getStartRangeKm() << endl;
  out << "  gateSpacingKm: " << _remap.getGateSpacingKm() << endl;
  out << "  gateSpacingIsConstant: " << _gateSpacingIsConstant << endl;
  out << "===========================================" << endl;

}

////////////////////////////////////////////////////////////
// Print native data in uf file
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int EdgeNcRadxFile::printNative(const string &path, ostream &out,
                                bool printRays, bool printData)
  
{

  _addErrStr("ERROR - EdgeNcRadxFile::printNative");
  _addErrStr("  Native print edges not apply to NetCDF file: ", path);
  _addErrStr("  Use 'ncdump' instead");
  return -1;

}

////////////////////////////////////////////////////////////
// Read in data from specified path, load up volume object.
// Also reads in paths for other fields in sweep, and other
// files in same volume if aggregation is on.
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs

int EdgeNcRadxFile::readFromPath(const string &primaryPath,
                                 RadxVol &vol)
  
{

  _initForRead(primaryPath, vol);

  // read in primary path

  if (_debug) {
    cerr << "Reading primary path: " << primaryPath << endl;
  }

  _firstFieldInSweep = true;
  if (_readSweepField(primaryPath)) {
    return -1;
  }

  // get secondary files for other fields from same sweep
  
  vector<string> secondaryPaths;
  _getSecondaryFieldPaths(primaryPath, secondaryPaths);

  // read in secondary paths

  _firstFieldInSweep = false;
  if (!_readTimesOnly) {
    for (size_t ii = 0; ii < secondaryPaths.size(); ii++) {
      if (_debug) {
        cerr << "Reading secondary path: " << secondaryPaths[ii] << endl;
      }
      if (_readSweepField(secondaryPaths[ii])) {
        return -1;
      }
    } // ii
  } // if (!_readTimesOnly)

  // load the rays into the volume
  
  _addRaysToVolume();
  
  // finalize the read volume
  
  if (_finalizeReadVolume()) {
    return -1;
  }
  
  // set format as read

  _fileFormat = FILE_FORMAT_EDGE_NC;

  // clean up

  _clearRayVariables();
  _dTimes.clear();

  // increment volume number

  _volumeNumber++;
  return 0;

} 

////////////////////////////////////////////////////////////
// Read in data for a sweep, from a file.
// Returns 0 on success, -1 on failure

int EdgeNcRadxFile::_readSweepField(const string &sweepPath)
  
{

  string errStr("ERROR - EdgeNcRadxFile::_readSweep");

  if (_debug) {
    cerr << "  reading sweep path: " << sweepPath << endl;
    cerr << "  firstFileInSweep: " << (_firstFieldInSweep?"Y":"N") << endl;
  }
  
  // intialize

  if (_firstFieldInSweep) {
    _rays.clear();
    _nAzimuthsInFile = 0;
    _nGatesInFile = 0;
  }

  // open file

  if (_file.openRead(sweepPath)) {
    _addErrStr(_file.getErrStr());
    return -1;
  }
  
  // read dimensions
  
  if (_readDimensions()) {
    _addErrStr(errStr);
    return -1;
  }

  // read time variable now if that is all that is needed
  
  if (_readTimesOnly) {
    if (_readTimes()) {
      _addErrStr(errStr);
      return -1;
    }
    return 0;
  }
  
  // read global attributes
  
  if (_readGlobalAttributes()) {
    _addErrStr(errStr);
    return -1;
  }
  
  // read time variable
  
  if (_readTimes()) {
    _addErrStr(errStr);
    return -1;
  }

  // read in ray variables

  if (_firstFieldInSweep) {

    if (_readRayVariables()) {
      _addErrStr(errStr);
      return -1;
    }

    // set up the range array
    
    _setRangeArray();

  }

  if (_readMetadataOnly) {

    // read field variables
    
    if (_readFieldVariable(true)) {
      _addErrStr(errStr);
      return -1;
    }
    
  } else {

    if (_firstFieldInSweep) {
      // create the rays to be read in, filling out the metadata
      if (_createRays()) {
        _addErrStr(errStr);
        return -1;
      }
    }
    
    // add field variables to file rays
    
    if (_readFieldVariable(false)) {
      _addErrStr(errStr);
      return -1;
    }

  }

  // close file

  _file.close();
  
  // append to read paths
  
  _readPaths.push_back(sweepPath);
  
  return 0;

}

///////////////////////////////////
// read in the dimensions

int EdgeNcRadxFile::_readDimensions()

{

  // read required dimensions

  int iret = 0;
  iret |= _file.readDim("Azimuth", _azimuthDim);
  if (iret == 0) {
    if (_firstFieldInSweep) {
      _nAzimuthsInFile = _azimuthDim->size();
    } else {
      if ((int) _nAzimuthsInFile != _azimuthDim->size()) {
        _addErrStr("ERROR - EdgeNcRadxFile::_file.readDimensions");
        _addErrInt("  nAzimuths changed from: ", (int) _nAzimuthsInFile);
        _addErrInt("                      to: ", _azimuthDim->size());
        return -1;
      }
    }
  }

  iret |= _file.readDim("Gate", _gateDim);
  if (iret == 0) {
    if (_firstFieldInSweep) {
      _nGatesInFile = _gateDim->size();
    } else {
      if ((int) _nGatesInFile != _gateDim->size()) {
        _addErrStr("ERROR - EdgeNcRadxFile::_file.readDimensions");
        _addErrInt("  nGates changed from: ", _nGatesInFile);
        _addErrInt("                      to: ", _gateDim->size());
        return -1;
      }
    }
  }
  
  _nGatesVary = false;

  if (iret) {
    _addErrStr("ERROR - EdgeNcRadxFile::_file.readDimensions");
    return -1;
  }

  return 0;

}

///////////////////////////////////
// read the global attributes

int EdgeNcRadxFile::_readGlobalAttributes()

{

  _file.readGlobAttr("TypeName", _TypeName_attr);
  _file.readGlobAttr("DataType", _DataType_attr);
  _file.readGlobAttr("Latitude", _Latitude_attr);
  _file.readGlobAttr("Longitude", _Longitude_attr);
  _file.readGlobAttr("Height", _Height_attr);
  _file.readGlobAttr("Time", _Time_attr);
  _file.readGlobAttr("FractionalTime", _FractionalTime_attr);
  _file.readGlobAttr("ibutesattr", _attributes_attr);
  _file.readGlobAttr("NyquistVelocity-unit", _NyquistVelocity_unit_attr);
  _file.readGlobAttr("NyquistVelocity-value", _NyquistVelocity_value_attr);
  _file.readGlobAttr("vcp-unit", _vcp_unit_attr);
  _file.readGlobAttr("vcp-value", _vcp_value_attr);
  _file.readGlobAttr("radarName-unit", _radarName_unit_attr);
  _file.readGlobAttr("radarName-value", _radarName_value_attr);
  _file.readGlobAttr("ColorMap-unit", _ColorMap_unit_attr);
  _file.readGlobAttr("ColorMap-value", _ColorMap_value_attr);
  _file.readGlobAttr("Elevation", _Elevation_attr);
  _file.readGlobAttr("ElevationUnits", _ElevationUnits_attr);
  _file.readGlobAttr("MissingData", _MissingData_attr);
  _file.readGlobAttr("RangeFolded", _RangeFolded_attr);
  _file.readGlobAttr("RadarParameters", _RadarParameters_attr);
  _file.readGlobAttr("PRF-unit", _PRF_unit_attr);
  _file.readGlobAttr("PRF-value", _PRF_value_attr);
  _file.readGlobAttr("PulseWidth-unit", _PulseWidth_unit_attr);
  _file.readGlobAttr("PulseWidth-value", _PulseWidth_value_attr);
  _file.readGlobAttr("MaximumRange-unit", _MaximumRange_unit_attr);
  _file.readGlobAttr("MaximumRange-value", _MaximumRange_value_attr);
  _file.readGlobAttr("ConversionPlugin", _ConversionPlugin_attr);

  _startTime.set(_Time_attr);
  _startTime.setSubSec(_FractionalTime_attr);
  _refTimeSecsFile = _startTime.utime();
  
  _title = _TypeName_attr;
  _institution = "";
  _references = "";
  _source = _radarName_value_attr;
  _history = _ConversionPlugin_attr;
  _comment = _attributes_attr;

  _siteName = _radarName_value_attr;
  _scanName = _vcp_value_attr;
  _instrumentName = _radarName_value_attr;
  _fieldName = _ColorMap_value_attr;

  _nyquistMps = _NyquistVelocity_value_attr;

  _latitude = _Latitude_attr;
  _longitude = _Longitude_attr;
  _altitudeKm = (double) _Height_attr / 1000.0;
  _maxRangeKm =  _MaximumRange_value_attr;

  _elevationFixedAngle = _Elevation_attr;
  _prfHz = (double) _PRF_value_attr;
  _pulseWidthUs = _PulseWidth_value_attr;

  _missingDataValue = _MissingData_attr;
  _rangeFoldedValue = _RangeFolded_attr;
  
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
// read the times

int EdgeNcRadxFile::_readTimes()

{

  // read the time attributes

  _file.readGlobAttr("Time", _Time_attr);
  _file.readGlobAttr("FractionalTime", _FractionalTime_attr);

  _startTime.set(_Time_attr);
  _startTime.setSubSec(_FractionalTime_attr);
  
  // set the time array
  
  _dTimes.clear();
  for (size_t ii = 0; ii < _nAzimuthsInFile; ii++) {
    _dTimes.push_back(0.0);
  }

  return 0;

}

///////////////////////////////////
// set the range array variable

void EdgeNcRadxFile::_setRangeArray()

{

  // get range units

  double kmPerUnit = 1.0; // default - units in km
  Nc3Att* unitsAtt = _gateWidthVar->get_att("Units");
  if (unitsAtt != NULL) {
    string units = Nc3xFile::asString(unitsAtt);
    if (units == "m" || units == "Meters") {
      kmPerUnit = 0.001;
    }
    delete unitsAtt;
  }

  // set range vector
  
  double gateSpacingKm = _gateWidths[0] * kmPerUnit;
  double startRangeKm = gateSpacingKm / 2.0;
  _rangeKm.clear();
  double range = startRangeKm;
  for (size_t ii = 0; ii < _nGatesInFile; ii++, range += gateSpacingKm) {
    _rangeKm.push_back(range);
  }

  // set the geometry from the range vector
  
  _remap.computeRangeLookup(_rangeKm);
  _gateSpacingIsConstant = _remap.getGateSpacingIsConstant();
  _geom.setRangeGeom(_remap.getStartRangeKm(), _remap.getGateSpacingKm());

}

///////////////////////////////////
// clear the ray variables

void EdgeNcRadxFile::_clearRayVariables()

{

  _azimuths.clear();
  _beamWidths.clear();
  _gateWidths.clear();

}

///////////////////////////////////
// read in ray variables

int EdgeNcRadxFile::_readRayVariables()

{

  _clearRayVariables();
  int iret = 0;

  _readRayVar(_azimuthVar, "Azimuth", _azimuths);
  if ((int) _azimuths.size() != _azimuthDim->size()) {
    _addErrStr("ERROR - Azimuth variable required");
    iret = -1;
  }
  
  _readRayVar(_beamWidthVar, "Beamwidth", _beamWidths);
  if ((int) _beamWidths.size() != _azimuthDim->size()) {
    _addErrStr("ERROR - Beamwidth variable required");
    iret = -1;
  }
  
  _readRayVar(_gateWidthVar, "GateWidth", _gateWidths);
  if ((int) _gateWidths.size() != _azimuthDim->size()) {
    _addErrStr("ERROR - Gatewidth variable required");
    iret = -1;
  }
  
  if (iret) {
    _addErrStr("ERROR - EdgeNcRadxFile::_readRayVariables");
    return -1;
  }

  return 0;

}

///////////////////////////////////
// create the rays to be read in
// and set meta data

int EdgeNcRadxFile::_createRays()

{
  
  // create the rays

  _rays.clear();
  
  for (size_t iray = 0; iray < _nAzimuthsInFile; iray++) {
    
    // new ray
    
    RadxRay *ray = new RadxRay;
    ray->copyRangeGeom(_geom);
    
    // set time
    
    double rayTimeDouble = _dTimes[iray];
    time_t rayUtimeSecs = _refTimeSecsFile + (time_t) rayTimeDouble;
    double rayIntSecs;
    double rayFracSecs = modf(rayTimeDouble, &rayIntSecs);
    int rayNanoSecs = (int) (rayFracSecs * 1.0e9);
    ray->setTime(rayUtimeSecs, rayNanoSecs);
    
    // sweep info
    
    ray->setSweepNumber(0);
    ray->setAzimuthDeg(_azimuths[iray]);
    ray->setElevationDeg(_elevationFixedAngle);
    ray->setFixedAngleDeg(_elevationFixedAngle);
    ray->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
    ray->setPolarizationMode(Radx::POL_MODE_HORIZONTAL);
    
    // add to ray vector
    
    _rays.push_back(ray);

  } // iray

  return 0;

}

////////////////////////////////////////////
// read the field variable

int EdgeNcRadxFile::_readFieldVariable(bool metaOnly)

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
    Nc3Dim* azDim = var->get_dim(0);
    Nc3Dim* gateDim = var->get_dim(1);
    if (azDim != _azimuthDim || gateDim != _gateDim) {
      continue;
    }
    
    // check the type
    string fieldName = var->name();
    Nc3Type ftype = var->type();
    if (ftype != nc3Double && ftype != nc3Float) {
      // not a valid type
      if (_verbose) {
        cerr << "DEBUG - EdgeNcRadxFile::_readFieldVariable" << endl;
        cerr << "  -->> rejecting field: " << fieldName << endl;
        cerr << "  -->> Should be float or double: " << fieldName << endl;
      }
      continue;
    }

    // check that we need this field

    if (!isFieldRequiredOnRead(fieldName)) {
      if (_verbose) {
        cerr << "DEBUG - EdgeNcRadxFile::_readFieldVariables" << endl;
        cerr << "  -->> rejecting field: " << fieldName << endl;
      }
      continue;
    }

    if (_verbose) {
      cerr << "DEBUG - EdgeNcRadxFile::_readFieldVariables" << endl;
      cerr << "  -->> adding field: " << fieldName << endl;
    }
    
    // set names, units, etc
    
    string name = _fieldName;
    string standardName = _fieldName;
    string longName = var->name();
    
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
    if (_fieldName.find("Velocity") != string::npos) {
      fieldFolds = true;
      foldLimitLower = _nyquistMps * -1.0;
      foldLimitUpper = _nyquistMps;
    }
    
    // if metadata only, don't read in fields
    
    if (metaOnly) {
      bool fieldAlreadyAdded = false;
      for (size_t ii = 0; ii < _readVol->getNFields(); ii++) {
        if (_readVol->getField(ii)->getName() == name) {
          fieldAlreadyAdded = true;
          break;
        }
      }
      if (!fieldAlreadyAdded) {
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
      default: {
        iret = -1;
        // will not reach here because of earlier check on type
      }

    } // switch
    
    if (iret) {
      _addErrStr("ERROR - EdgeNcRadxFile::_readFieldVariables");
      _addErrStr("  cannot read field name: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
      return -1;
    }

  } // ivar

  return 0;

}

///////////////////////////////////
// read a ray variable - float

int EdgeNcRadxFile::_readRayVar(Nc3Var* &var, const string &name,
                                vector<float> &vals, bool required)

{

  vals.clear();

  // get var

  var = _getRayVar(name, required);
  if (var == NULL) {
    if (!required) {
      for (size_t ii = 0; ii < _nAzimuthsInFile; ii++) {
        vals.push_back(Radx::missingMetaFloat);
      }
      clearErrStr();
      return 0;
    } else {
      _addErrStr("ERROR - EdgeNcRadxFile::_readRayVar");
      return -1;
    }
  }

  // load up data

  float *data = new float[_nAzimuthsInFile];
  float *dd = data;
  int iret = 0;
  if (var->get(data, _nAzimuthsInFile)) {
    for (size_t ii = 0; ii < _nAzimuthsInFile; ii++, dd++) {
      vals.push_back(*dd);
    }
  } else {
    if (!required) {
      for (size_t ii = 0; ii < _nAzimuthsInFile; ii++) {
        vals.push_back(Radx::missingMetaFloat);
      }
      clearErrStr();
    } else {
      _addErrStr("ERROR - EdgeNcRadxFile::_readRayVar");
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

Nc3Var* EdgeNcRadxFile::_getRayVar(const string &name, bool required)

{

  // get var
  
  Nc3Var *var = _file.getNc3File()->get_var(name.c_str());
  if (var == NULL) {
    if (required) {
      _addErrStr("ERROR - EdgeNcRadxFile::_getRayVar");
      _addErrStr("  Cannot read variable, name: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
    }
    return NULL;
  }

  // check time dimension
  
  if (var->num_dims() < 1) {
    if (required) {
      _addErrStr("ERROR - EdgeNcRadxFile::_getRayVar");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has no dimensions");
    }
    return NULL;
  }
  Nc3Dim *timeDim = var->get_dim(0);
  if (timeDim != _azimuthDim) {
    if (required) {
      _addErrStr("ERROR - EdgeNcRadxFile::_getRayVar");
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
// Add fl64 fields to _raysFromFile
// The _raysFromFile array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int EdgeNcRadxFile::_addFl64FieldToRays(Nc3Var* var,
                                        const string &name,
                                        const string &units,
                                        const string &standardName,
                                        const string &longName,
                                        bool isDiscrete,
                                        bool fieldFolds,
                                        float foldLimitLower,
                                        float foldLimitUpper)
  
{

  // get data from array
  
  Radx::fl64 *data = new Radx::fl64[_nAzimuthsInFile * _nGatesInFile];
  int iret = !var->get(data, _nAzimuthsInFile, _nGatesInFile);
  if (iret) {
    delete[] data;
    return -1;
  }

  // set missing value

  Radx::fl64 missingVal = _missingDataValue;

  // load field on rays

  for (size_t iray = 0; iray < _nAzimuthsInFile; iray++) {
    
    RadxRay *ray = _rays[iray];

    int startIndex = iray * _nGatesInFile;
    
    RadxField *field = ray->addField(name, units, _nGatesInFile,
                                     missingVal,
                                     data + startIndex,
                                     true);
    
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->copyRangeGeom(_geom);
    
    if (fieldFolds &&
        foldLimitLower != Radx::missingMetaFloat &&
        foldLimitUpper != Radx::missingMetaFloat) {
      field->setFieldFolds(foldLimitLower, foldLimitUpper);
    }
    if (isDiscrete) {
      field->setIsDiscrete(true);
    }

  }
  
  delete[] data;
  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add fl32 fields to _raysFromFile
// The _raysFromFile array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int EdgeNcRadxFile::_addFl32FieldToRays(Nc3Var* var,
                                        const string &name,
                                        const string &units,
                                        const string &standardName,
                                        const string &longName,
                                        bool isDiscrete,
                                        bool fieldFolds,
                                        float foldLimitLower,
                                        float foldLimitUpper)
  
{

  // get data from array
  
  Radx::fl32 *data = new Radx::fl32[_nAzimuthsInFile * _nGatesInFile];
  int iret = !var->get(data, _nAzimuthsInFile, _nGatesInFile);
  if (iret) {
    delete[] data;
    return -1;
  }

  // set missing value

  Radx::fl32 missingVal = _missingDataValue;

  // load field on rays

  for (size_t iray = 0; iray < _nAzimuthsInFile; iray++) {
    
    RadxRay *ray = _rays[iray];

    int startIndex = iray * _nGatesInFile;
    
    RadxField *field = ray->addField(name, units, _nGatesInFile,
                                     missingVal,
                                     data + startIndex,
                                     true);
    
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->copyRangeGeom(_geom);
    
    if (fieldFolds &&
        foldLimitLower != Radx::missingMetaFloat &&
        foldLimitUpper != Radx::missingMetaFloat) {
      field->setFieldFolds(foldLimitLower, foldLimitUpper);
    }
    if (isDiscrete) {
      field->setIsDiscrete(true);
    }

  }
  
  delete[] data;
  return 0;
  
}

/////////////////////////////////////////////////////////
// initialize the read volume from the primary data

void EdgeNcRadxFile::_initializeReadVolume()
  
{

  // set metadata

  _readVol->setOrigFormat("EDGE");
  _readVol->setVolumeNumber(_volumeNumber);
  _readVol->setInstrumentType(_instrumentType);
  _readVol->setPlatformType(_platformType);
  _readVol->setPrimaryAxis(_primaryAxis);

  // _readVol->addFrequencyHz(_frequencyGhz * 1.0e9);
  
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

  _readVol->setLatitudeDeg(_latitude);
  _readVol->setLongitudeDeg(_longitude);
  _readVol->setAltitudeKm(_altitudeKm);

  _readVol->copyRangeGeom(_geom);

}

/////////////////////////////////////////////////////////
// add rays to volume

void EdgeNcRadxFile::_addRaysToVolume()
  
{

  // set volume number

  for (size_t iray = 0; iray < _rays.size(); iray++) {
    _rays[iray]->setVolumeNumber(_volumeNumber);
  }

  // add rays to vol - they will be freed by vol
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    _readVol->addRay(_rays[iray]);
  }

}

/////////////////////////////////////////////////////////
// finalize the read volume

int EdgeNcRadxFile::_finalizeReadVolume()
  
{

  if (_readSetMaxRange) {
    _readVol->setMaxRangeKm(_readMaxRangeKm);
  }
  
  // load the sweep information from the rays

  _readVol->loadSweepInfoFromRays();

  // constrain the sweep data as appropriate
  
  if (_readFixedAngleLimitsSet) {
    if (_readVol->constrainByFixedAngle(_readMinFixedAngle, _readMaxFixedAngle,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - EdgeNcRadxFile::_finalizeReadVolume");
      _addErrStr("  No data found within fixed angle limits");
      _addErrDbl("  min fixed angle: ", _readMinFixedAngle);
      _addErrDbl("  max fixed angle: ", _readMaxFixedAngle);
      return -1;
    }
  } else if (_readSweepNumLimitsSet) {
    if (_readVol->constrainBySweepNum(_readMinSweepNum, _readMaxSweepNum,
                                      _readStrictAngleLimits)) {
      _addErrStr("ERROR - EdgeNcRadxFile::_finalizeReadVolume");
      _addErrStr("  No data found within sweep num limits");
      _addErrInt("  min sweep num: ", _readMinSweepNum);
      _addErrInt("  max sweep num: ", _readMaxSweepNum);
      return -1;
    }
  }
  
  // optionally remove all rays with missing data

  if (_readRemoveRaysAllMissing) {
    _readVol->removeRaysWithDataAllMissing();
  }
  
  // load the volume information from the rays

  _readVol->loadVolumeInfoFromRays();
  
  // check for indexed rays, set info on rays

  _readVol->checkForIndexedRays();

  return 0;

}

/////////////////////////////////////////////////////////////////
// get list of field paths for the volume for the specified path

void EdgeNcRadxFile::_getSecondaryFieldPaths(const string &primaryPath,
                                             vector<string> &secondaryPaths)
  
{
  
  // init

  vector<string> fileNames;
  vector<string> fieldNames;
  secondaryPaths.clear();
  
  // decompose the path to get the date/time prefix for the primary path
  
  RadxPath ppath(primaryPath);
  const string &dir = ppath.getDirectory();
  const string &fileName = ppath.getFile();
  const string &ext = ppath.getExt();

  // find the last '-' in the file name

  size_t prefixLen = fileName.find_last_of('-') + 1;
  string prefix(fileName.substr(0, prefixLen));
  
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

      // make sure we have files with correct extension
      
      if (dName.find(ext) == string::npos) {
	continue;
      }
      
      string dStr(dName.substr(0, prefixLen));
      
      if (dStr == prefix) {
        // get field name from file name
        size_t pos = dName.find('.', prefixLen);
        if (pos != string::npos) {
          fileNames.push_back(dName);
        } // if (pos ...
      } // if (dStr ...
      
    } // dp
    
    rdir.close();

  } // if (rdir ...

  // sort the file names

  sort(fileNames.begin(), fileNames.end());

  // load up the secondary paths and field names

  for (size_t ii = 0; ii < fileNames.size(); ii++) {
    
    const string &fileName = fileNames[ii];

    size_t pos = fileName.find('.', prefixLen);
    string fieldName = fileName.substr(prefixLen, pos - prefixLen);
    fieldNames.push_back(fieldName);
    
    string dPath(dir);
    dPath += RadxPath::RADX_PATH_DELIM;
    dPath += fileName;

    if (dPath != primaryPath) {
      secondaryPaths.push_back(dPath);
    }

  } // ii
  
}

