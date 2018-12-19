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
// GamicHdf5RadxFile.cc
//
// GAMIC HDF5 file data for radar radial data
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2013
//
///////////////////////////////////////////////////////////////

#include <Radx/NcfRadxFile.hh>
#include <Radx/GamicHdf5RadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxXml.hh>
#include <Radx/ByteOrder.hh>
#include <Radx/RadxComplex.hh>
#include <cstring>
#include <cstdio>
#include <cmath>

//////////////
// Constructor

GamicHdf5RadxFile::GamicHdf5RadxFile() : RadxFile()
  
{

  _volumeNumber = 0;
  _ncFormat = NETCDF_CLASSIC;
  _readVol = NULL;
  clear();
  
}

/////////////
// destructor

GamicHdf5RadxFile::~GamicHdf5RadxFile()

{
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void GamicHdf5RadxFile::clear()
  
{

  clearErrStr();

  _instrumentType = Radx::missingInstrumentType;
  _platformType = Radx::missingPlatformType;
  _primaryAxis = Radx::PRIMARY_AXIS_Z;

  _azBeamWidthDeg = Radx::missingFl64;
  _elBeamWidthDeg = Radx::missingFl64;
  
  _hostName.clear();
  _sdpName.clear();
  _sdpVersion.clear();
  _simulated.clear();
  _siteName.clear();
  _software.clear();
  _swVersion.clear();
  _templateName.clear();

  _dateStr.clear();
  _objectStr.clear();
  _nSweeps = 0;
  _setsScheduled.clear();
  _version.clear();

  _latitudeDeg = Radx::missingFl64;
  _longitudeDeg = Radx::missingFl64;
  _altitudeKm = Radx::missingFl64;

  _prfHz = Radx::missingFl64;
  _angleStepDeg = Radx::missingFl64;
  _angleSync = false;
  _nGates = Radx::missingSi32;
  _clutterFilterNumber = Radx::missingSi32;
  _angleStart = Radx::missingFl64;
  _angleStop = Radx::missingFl64;
  _fixedAngleDeg = Radx::missingFl64;
  _malfunc = Radx::missingSi32;
  _pulseWidthUs = Radx::missingFl64;
  _wavelengthM = Radx::missingFl64;
  _frequencyHz = Radx::missingFl64;
  _maxRangeKm = Radx::missingFl64;
  _nSamplesRange = Radx::missingSi32;
  _nSamplesTime = Radx::missingSi32;
  _startRangeKm = Radx::missingFl64;
  _gateSpacingKm = Radx::missingFl64;
  _nRaysSweep = Radx::missingSi32;
  _scanRateDegPerSec = Radx::missingFl64;
  _sweepTimeStamp.clear();
  _prtMode = Radx::PRT_MODE_FIXED;
  _prtRatio = Radx::missingFl64;

  // sweep 'what' attributes

  _product.clear();
  _scanTypeStr.clear();
  _isRhi = false;
  _nFields = 0;

  // sweep extended attributes

  _sweepStatusXml.clear();
  _unambigVel = Radx::missingFl64;
  _statusXml.clear();
  
  _statusXml.clear(); // global attributes
  _siteName.clear();
  _scanTypeStr.clear();

}

/////////////////////////////////////////////////////////
// Check if specified file is CfRadial format
// Returns true if supported, false otherwise

bool GamicHdf5RadxFile::isSupported(const string &path)

{
  
  if (isGamicHdf5(path)) {
    return true;
  }
  return false;

}

////////////////////////////////////////////////////////////
// Check if this is a GamicHdf5 file
// Returns true on success, false on failure

bool GamicHdf5RadxFile::isGamicHdf5(const string &path)
  
{

  clear();
  
  if (!H5File::isHdf5(path)) {
    if (_verbose) {
      cerr << "DEBUG - not GamicHdf5 file" << endl;
    }
    return false;
  }

  // suppress automatic exception printing so we can handle
  // errors appropriately

  Exception::dontPrint();

  // open file

  H5File file(path, H5F_ACC_RDONLY);

  // check for how group

  Group *how = NULL;
  try {
    how = new Group(file.openGroup("how"));
  }
  catch (H5::Exception e) {
    if (_verbose) {
      cerr << "No 'how' group, not GAMIC file" << endl;
    }
    if (how) delete how;
    return false;
  }

  // check for beam width variables

  Attribute *elevBeamWidth = NULL;
  try {
    elevBeamWidth = new Attribute(how->openAttribute("elevation_beam"));
  }
  catch (H5::Exception e) {
    if (_verbose) {
      cerr << "No how.elevation_beam width attribute, not GAMIC file" << endl;
    }
    if (elevBeamWidth) delete elevBeamWidth;
    if (how) delete how;
    return false;
  }
  delete elevBeamWidth;

  Attribute *azBeamWidth = NULL;
  try {
    azBeamWidth = new Attribute(how->openAttribute("azimuth_beam"));
  }
  catch (H5::Exception e) {
    if (_verbose) {
      cerr << "No how.azimuth_beam width attribute, not GAMIC file" << endl;
    }
    if (azBeamWidth) delete azBeamWidth;
    if (how) delete how;
    return false;
  }
  delete azBeamWidth;

  // clean up

  delete how;

  // good

  return true;

}

/////////////////////////////////////////////////////////
// Write data from volume to specified directory
//
// Writes not supported for this class
// Use NcfRadxFile methods instead.
//
// Returns 0 on success, -1 on failure

int GamicHdf5RadxFile::writeToDir(const RadxVol &vol,
                                  const string &dir,
                                  bool addDaySubDir,
                                  bool addYearSubDir)
  
{

  // Writing GamicHdf5 files is not supported
  // therefore write in CF Radial format instead
  
  cerr << "WARNING - GamicHdf5RadxFile::writeToDir" << endl;
  cerr << "  Writing GamicHdf5 format files not supported" << endl;
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

int GamicHdf5RadxFile::writeToPath(const RadxVol &vol,
                                   const string &path)
  
{

  // Writing GamicHdf5 files is not supported
  // therefore write in CF Radial format instead

  cerr << "WARNING - GamicHdf5RadxFile::writeToPath" << endl;
  cerr << "  Writing GamicHdf5 format files not supported" << endl;
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
// get the date and time from a gamic file path
// returns 0 on success, -1 on failure

int GamicHdf5RadxFile::getTimeFromPath(const string &path, RadxTime &rtime)

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
    char cc1, cc2, cc3, cc4, cc5, cc6;
    if (sscanf(start, "%4d%1c%2d%1c%2d%1c%1c%2d%1c%2d%1c%2d",
               &year, &cc1, &month, &cc2, &day, &cc3,
               &cc4, &hour, &cc5, &min, &cc6, &sec) == 12) {
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

void GamicHdf5RadxFile::print(ostream &out) const
  
{
  
  out << "=============== GamicHdf5RadxFile ===============" << endl;
  RadxFile::print(out);

  out << "  volumeNumber: " << _volumeNumber << endl;
  out << "  instrumentType: " 
      << Radx::instrumentTypeToStr(_instrumentType) << endl;
  out << "  platformType: " << Radx::platformTypeToStr(_platformType) << endl;
  out << "  primaryAxis: " << Radx::primaryAxisToStr(_primaryAxis) << endl;

  out << "  _azBeamWidthDeg: " << _azBeamWidthDeg << endl;
  out << "  _elBeamWidthDeg: " << _elBeamWidthDeg << endl;
  out << "  _hostName: " << _hostName << endl;
  out << "  _sdpName: " << _sdpName << endl;
  out << "  _sdpVersion: " << _sdpVersion << endl;
  out << "  _simulated: " << _simulated << endl;
  out << "  _siteName: " << _siteName << endl;
  out << "  _software: " << _software << endl;
  out << "  _swVersion: " << _swVersion << endl;
  out << "  _templateName: " << _templateName << endl;

  out << "  _dateStr: " << _dateStr << endl;
  out << "  _objectStr: " << _objectStr << endl;
  out << "  _nSweeps: " << _nSweeps << endl;
  out << "  _setsScheduled: " << _setsScheduled << endl;
  out << "  _version: " << _version << endl;

  out << "  latitude: " << _latitudeDeg << endl;
  out << "  longitude: " << _longitudeDeg << endl;
  out << "  altitude: " << _altitudeKm << endl;

  out << "  _prfHz: " << _prfHz << endl;
  out << "  _angleStepDeg: " << _angleStepDeg << endl;
  out << "  _angleSync: " << _angleSync << endl;
  out << "  _nGates: " << _nGates << endl;
  out << "  _clutterFilterNumber: " << _clutterFilterNumber << endl;
  out << "  _angleStart: " << _angleStart << endl;
  out << "  _angleStop: " << _angleStop << endl;
  out << "  _fixedAngleDeg: " << _fixedAngleDeg << endl;
  out << "  _malfunc: " << _malfunc << endl;
  out << "  _pulseWidthUs: " << _pulseWidthUs << endl;
  out << "  _wavelengthM: " << _wavelengthM << endl;
  out << "  _frequencyHz: " << _frequencyHz << endl;
  out << "  _maxRangeKm: " << _maxRangeKm << endl;
  out << "  _nSamplesRange: " << _nSamplesRange << endl;
  out << "  _nSamplesTime: " << _nSamplesTime << endl;
  out << "  _startRangeKm: " << _startRangeKm << endl;
  out << "  _gateSpacingKm: " << _gateSpacingKm << endl;
  out << "  _nRaysSweep: " << _nRaysSweep << endl;
  out << "  _scanRateDegPerSec: " << _scanRateDegPerSec << endl;
  out << "  _sweepTimeStamp: " << _sweepTimeStamp << endl;
  out << "  _prtMode: " << _prtMode << endl;
  out << "  _prtRatio: " << _prtRatio << endl;

  out << "  _product: " << _product << endl;
  out << "  _scanTypeStr: " << _scanTypeStr << endl;
  out << "  _isRhi: " << _isRhi << endl;
  out << "  _nFields: " << _nFields << endl;
  
  out << "  _unambigVel: " << _unambigVel << endl;
  out << "  _statusXml: " << _statusXml << endl;
  
  out << "===========================================" << endl;

}

////////////////////////////////////////////////////////////
// Print native data in uf file
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int GamicHdf5RadxFile::printNative(const string &path, ostream &out,
                                   bool printRays, bool printData)
  
{

  if (!H5File::isHdf5(path)) {
    return false;
  }
  
  // open file
  
  H5File file(path, H5F_ACC_RDONLY);
  
  out << "Printing GAMIC HDF5 contents" << endl;
  out << "  file path: " << file.getFileName() << endl;
  out << "  file size: " << file.getFileSize() << endl;
  
  try {
    Group root(file.openGroup("/"));
    _utils.printGroup(root, "/", out, printRays, printData);
  }
  catch (H5::Exception e) {
    _addErrStr("ERROR - trying to read GAMIC HDF5 file");
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////
// Read in data from specified path, load up volume object.
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs

int GamicHdf5RadxFile::readFromPath(const string &path,
                                    RadxVol &vol)
  
{

  if (_debug) {
    cerr << "Reading GAMIC HDF5 file, path: " << path << endl;
  }

  _initForRead(path, vol);
  _volumeNumber++;
  _sweepNumber = 0;

  // initialize status XML

  _statusXml.clear();
  _statusXml += RadxXml::writeStartTag("Status", 0);
  
  string errStr("ERROR - GamicHdf5RadxFile::readFromPath");
  if (!H5File::isHdf5(path)) {
    _addErrStr("ERROR - not a GAMIC HDF5 file");
    return -1;
  }
  
  try {
    
    // open file
    
    H5File file(path, H5F_ACC_RDONLY);
    if (_debug) {
      cerr << "  file size: " << file.getFileSize() << endl;
    }
    
    // get the root group
    
    Group root(file.openGroup("/"));
    
    // process the root how, what and where groups
    
    if (_readRootSubGroups(root)) {
      return -1;
    }
    
    // process the sweeps
    
    for (int isweep = 0; isweep < _nSweeps; isweep++) {
      _sweepNumber++;
      if (_readSweep(root, isweep)) {
        return -1;
      }
      _statusXml += _sweepStatusXml;
    }
    
  }
  
  catch (H5::Exception e) {
    _addErrStr("ERROR - reading GAMIC HDF5 file");
    return -1;
  }

  // append to read paths
  
  _readPaths.push_back(path);
  
  // finalize status xml
  
  _statusXml += RadxXml::writeEndTag("Status", 0);

  // load the data into the read volume
  
  if (_finalizeReadVolume()) {
    return -1;
  }
  
  // set format as read

  _fileFormat = FILE_FORMAT_GAMIC_HDF5;

  return 0;

}

//////////////////////////////////////////////
// process the root how, where and what groups

int GamicHdf5RadxFile::_readRootSubGroups(Group &root)

{

  Group how(root.openGroup("how"));
  if (_readRootHow(how)) {
    return -1;
  }

  Group what(root.openGroup("what"));
  if (_readRootWhat(what)) {
    return -1;
  }

  Group where(root.openGroup("where"));
  if (_readRootWhere(where)) {
    return -1;
  }


  return 0;

}

//////////////////////////////////////////////
// process the root how group

int GamicHdf5RadxFile::_readRootHow(Group &how)

{

  Hdf5xx::DecodedAttr decodedAttr;

  if (_utils.loadAttribute(how, "azimuth_beam", "root-how-group", decodedAttr)) {
    return -1;
  }
  _azBeamWidthDeg = decodedAttr.getAsDouble();
  
  if (_utils.loadAttribute(how, "elevation_beam", "root-how-group", decodedAttr)) {
    return -1;
  }
  _elBeamWidthDeg = decodedAttr.getAsDouble();
  
  _utils.loadAttribute(how, "host_name", "root-how-group", decodedAttr);
  _hostName = decodedAttr.getAsString();

  _utils.loadAttribute(how, "sdp_name", "root-how-group", decodedAttr);
  _sdpName = decodedAttr.getAsString();
  
  _utils.loadAttribute(how, "sdp_version", "root-how-group", decodedAttr);
  _sdpVersion = decodedAttr.getAsString();

  _utils.loadAttribute(how, "simulated", "root-how-group", decodedAttr);
  _simulated = decodedAttr.getAsString();

  if (_utils.loadAttribute(how, "site_name", "root-how-group", decodedAttr)) {
    return -1;
  }
  _siteName = decodedAttr.getAsString();

  _utils.loadAttribute(how, "software", "root-how-group", decodedAttr);
  _software = decodedAttr.getAsString();

  _utils.loadAttribute(how, "sw_version", "root-how-group", decodedAttr);
  _swVersion = decodedAttr.getAsString();

  _utils.loadAttribute(how, "template_name", "root-how-group", decodedAttr);
  _templateName = decodedAttr.getAsString();

  if (_debug) {
    cerr  << "====>> root how attr _azBeamWidthDeg: " << _azBeamWidthDeg << endl;
    cerr  << "====>> root how attr _elBeamWidthDeg: " << _elBeamWidthDeg << endl;
    cerr  << "====>> root how attr _hostName: " << _hostName << endl;
    cerr  << "====>> root how attr _sdpName: " << _sdpName << endl;
    cerr  << "====>> root how attr _sdpVersion: " << _sdpVersion << endl;
    cerr  << "====>> root how attr _simulated: " << _simulated << endl;
    cerr  << "====>> root how attr _siteName: " << _siteName << endl;
    cerr  << "====>> root how attr _software: " << _software << endl;
    cerr  << "====>> root how attr _swVersion: " << _swVersion << endl;
    cerr  << "====>> root how attr _templateName: " << _templateName << endl;
  }

  return 0;

}

//////////////////////////////////////////////
// process the root what group

int GamicHdf5RadxFile::_readRootWhat(Group &what)

{

  Hdf5xx::DecodedAttr decodedAttr;

  _utils.loadAttribute(what, "date", "root-what-group", decodedAttr);
  _dateStr = decodedAttr.getAsString();
  
  if (_utils.loadAttribute(what, "object", "root-what-group", decodedAttr)) {
    return -1;
  }
  _objectStr = decodedAttr.getAsString();
  
  if (_utils.loadAttribute(what, "sets", "root-what-group", decodedAttr)) {
    return -1;
  }
  _nSweeps = decodedAttr.getAsInt();
  
  if (_utils.loadAttribute(what, "sets_scheduled", "root-what-group", decodedAttr) == 0) {
    _setsScheduled = decodedAttr.getAsString();
  } else {
    char buffer[128];
    snprintf(buffer, 128, "%d", _nSweeps);
    _setsScheduled = buffer; 
  }
  
   _utils.loadAttribute(what, "version", "root-what-group", decodedAttr);
   _version = decodedAttr.getAsString();
  
  if (_debug) {
    cerr  << "====>> root what attr _dateStr: " << _dateStr << endl;
    cerr  << "====>> root what attr _objectStr: " << _objectStr << endl;
    cerr  << "====>> root what attr _nSweeps: " << _nSweeps << endl;
    cerr  << "====>> root what attr _setsScheduled: " << _setsScheduled << endl;
    cerr  << "====>> root what attr _version: " << _version << endl;
  }

  return 0;

}

//////////////////////////////////////////////
// process the root where group

int GamicHdf5RadxFile::_readRootWhere(Group &where)

{

  Hdf5xx::DecodedAttr decodedAttr;
  
  if (_utils.loadAttribute(where, "height", "root-where-group", decodedAttr)) {
    return -1;
  }
  _altitudeKm = decodedAttr.getAsDouble() / 1000.0;
  
  if (_utils.loadAttribute(where, "lat", "root-where-group", decodedAttr)) {
    return -1;
  }
  _latitudeDeg = decodedAttr.getAsDouble();
  
  if (_utils.loadAttribute(where, "lon", "root-where-group", decodedAttr)) {
    return -1;
  }
  _longitudeDeg = decodedAttr.getAsDouble();
  
  if (_debug) {
    cerr  << "====>> root where attr _altitudeKm: " << _altitudeKm << endl;
    cerr  << "====>> root where attr _latitudeDeg: " << _latitudeDeg << endl;
    cerr  << "====>> root where attr _longitudeDeg: " << _longitudeDeg << endl;
  }

  return 0;

}

//////////////////////////////////////////////
// process sweeps

int GamicHdf5RadxFile::_readSweep(Group &root, int sweepNumber)

{
  
  // clear

  _clearSweepVars();

  // compute scan name: scan0, scan1 etc ...

  char scanName[128];
  sprintf(scanName, "scan%d", sweepNumber);
  
  // open scan group
  
  Group sweep(root.openGroup(scanName));
  
  // process scan how group
  
  Group how(sweep.openGroup("how"));
  if (_readSweepHow(how, sweepNumber)) {
    return -1;
  }

  // check if this sweep is required

  if (_readStrictAngleLimits) {
    if (_readFixedAngleLimitsSet) {
      if (_fixedAngleDeg < _readMinFixedAngle ||
          _fixedAngleDeg > _readMaxFixedAngle) {
        _clearSweepVars();
        return 0;
      }
    } else if (_readSweepNumLimitsSet) {
      if (sweepNumber < _readMinSweepNum ||
          sweepNumber > _readMaxSweepNum) {
        _clearSweepVars();
        return 0;
      }
    }
  }

  // process scan what group
  
  Group what(sweep.openGroup("what"));
  if (_readSweepWhat(what, sweepNumber)) {
    return -1;
  }

  // process scan how extended group if it exists
  
  Group *extended = NULL;
  try {
    extended = new Group(how.openGroup("extended"));
  }
  catch (H5::Exception e) {
    if (_verbose) {
      cerr << "No scan 'extended' how group" << endl;
    }
    if (extended) delete extended;
  }
  if (extended) {
    if (_readSweepExtended(*extended)) {
      return -1;
    }
    delete extended;
  }

  // read in ray headers, create rays

  DataSet rayHdrs(sweep.openDataSet("ray_header"));
  if (_readRays(sweep, sweepNumber)) {
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////
// clear the sweep variables

void GamicHdf5RadxFile::_clearSweepVars()

{

  _sweepStatusXml.clear();
  _prfHz = Radx::missingFl64;
  _angleSync = 0;
  _angleStepDeg = 1.0;
  _angleStart = Radx::missingFl64;
  _angleStop = Radx::missingFl64;
  _fixedAngleDeg = Radx::missingFl64;
  _nGates = 0;
  _clutterFilterNumber = 0;
  _malfunc = 0;
  _pulseWidthUs = Radx::missingFl64;
  _wavelengthM = Radx::missingFl64;
  _frequencyHz = Radx::missingFl64;
  _maxRangeKm = 0;
  _nSamplesRange = 0;
  _nSamplesTime = 0;
  _startRangeKm = 0;
  _gateSpacingKm = 0;
  _nRaysSweep = 0;
  _scanRateDegPerSec = 0.0;
  _sweepTimeStamp.clear();
  _prtMode = Radx::PRT_MODE_FIXED;
  _prtRatio = 1.0;
  _product = "SCAN";
  _scanTypeStr = "PPI";
  _isRhi = false;
  _nFields = 0;

}

//////////////////////////////////////////////
// process a sweep how group

int GamicHdf5RadxFile::_readSweepHow(Group &how, int sweepNum)

{

  Hdf5xx::DecodedAttr decodedAttr;

  _utils.loadAttribute(how, "PRF", "sweep-how-group", decodedAttr);
  _prfHz = decodedAttr.getAsDouble();

  _angleSync = 0;
  if (_utils.loadAttribute(how, "angle_sync", "sweep-how-group", decodedAttr) == 0) {
    _angleSync = ((decodedAttr.getAsInt() == 0)? false : true);
  }
  _utils.loadAttribute(how, "angle_step", "sweep-how-group", decodedAttr);
  _angleStepDeg = decodedAttr.getAsDouble();

  _angleStart = Radx::missingFl64;
  if (_utils.loadAttribute(how, "azi_start", "sweep-how-group", decodedAttr) == 0) {
    _angleStart = decodedAttr.getAsDouble();
  }
  if (_utils.loadAttribute(how, "ele_start", "sweep-how-group", decodedAttr) == 0) {
    _angleStart = decodedAttr.getAsDouble();
  }

  _angleStop = Radx::missingFl64;
  if (_utils.loadAttribute(how, "azi_stop", "sweep-how-group", decodedAttr) == 0) {
    _angleStop = decodedAttr.getAsDouble();
  }
  if (_utils.loadAttribute(how, "ele_stop", "sweep-how-group", decodedAttr) == 0) {
    _angleStop = decodedAttr.getAsDouble();
  }
  
  _fixedAngleDeg = Radx::missingFl64;
  if (_utils.loadAttribute(how, "elevation", "sweep-how-group", decodedAttr) == 0) {
    _fixedAngleDeg = decodedAttr.getAsDouble();
  }
  if (_utils.loadAttribute(how, "azimuth", "sweep-how-group", decodedAttr) == 0) {
    _fixedAngleDeg = decodedAttr.getAsDouble();
  }

  if (_utils.loadAttribute(how, "bin_count", "sweep-how-group", decodedAttr)) {
    return -1;
  }
  _nGates = decodedAttr.getAsInt();

  _utils.loadAttribute(how, "clutter_filter_number", "sweep-how-group", decodedAttr);
  _clutterFilterNumber = decodedAttr.getAsInt();

  _utils.loadAttribute(how, "malfunc", "sweep-how-group", decodedAttr);
  _malfunc = decodedAttr.getAsInt();

  _pulseWidthUs = 0;
  if (_utils.loadAttribute(how, "pulse_width_us", "sweep-how-group", decodedAttr) == 0) {
    _pulseWidthUs = decodedAttr.getAsDouble();
  }
  
  _wavelengthM = Radx::missingFl64;
  _frequencyHz = Radx::missingFl64;
  if (_utils.loadAttribute(how, "radar_wave_length", "sweep-how-group", decodedAttr) == 0) {
    _wavelengthM = decodedAttr.getAsDouble();
    _frequencyHz = Radx::LIGHT_SPEED / _wavelengthM;
  }

  _maxRangeKm = 0;
  if (_utils.loadAttribute(how, "range", "sweep-how-group", decodedAttr) == 0) {
    _maxRangeKm = decodedAttr.getAsDouble() / 1000.0;
  }

  _nSamplesRange = 1;
  if (_utils.loadAttribute(how, "range_samples", "sweep-how-group", decodedAttr) == 0) {
    _nSamplesRange = decodedAttr.getAsInt();
  }

  _nSamplesTime = 1;
  if (_utils.loadAttribute(how, "time_samples", "sweep-how-group", decodedAttr) == 0) {
    _nSamplesTime = decodedAttr.getAsInt();
  }

  if (_utils.loadAttribute(how, "range_start", "sweep-how-group", decodedAttr)) {
    return -1;
  }
  _startRangeKm = decodedAttr.getAsDouble() / 1000.0;
  
  if (_utils.loadAttribute(how, "range_step", "sweep-how-group", decodedAttr)) {
    return -1;
  }
  _gateSpacingKm = decodedAttr.getAsDouble() / 1000.0;

  if (_utils.loadAttribute(how, "ray_count", "sweep-how-group", decodedAttr)) {
    return -1;
  }
  _nRaysSweep = decodedAttr.getAsInt();

  _scanRateDegPerSec = Radx::missingFl64;
  if (_utils.loadAttribute(how, "scan_speed", "sweep-how-group", decodedAttr) == 0) {
    _scanRateDegPerSec = decodedAttr.getAsDouble();
  }

  _sweepTimeStamp.clear();
  if (_utils.loadAttribute(how, "timestamp", "sweep-how-group", decodedAttr) == 0) {
    _sweepTimeStamp = decodedAttr.getAsString();
  }

  _prtMode = Radx::PRT_MODE_FIXED;
  _prtRatio = 1.0;
  if (_utils.loadAttribute(how, "unfolding", "sweep-how-group", decodedAttr) == 0) {
    int unfolding = decodedAttr.getAsInt();
    if (unfolding == 1) {
      _prtMode = Radx::PRT_MODE_STAGGERED;
      _prtRatio = 2.0 / 3.0;
    } else if (unfolding == 2) {
      _prtMode = Radx::PRT_MODE_STAGGERED;
      _prtRatio = 3.0 / 4.0;
    } else if (unfolding == 3) {
      _prtMode = Radx::PRT_MODE_STAGGERED;
      _prtRatio = 4.0 / 5.0;
    }
  }

  if (_debug) {
    cerr << "============== attributes for sweep number " 
         << sweepNum << " ===============" << endl;
    cerr  << "====>> sweep how attr _prfHz: " << _prfHz << endl;
    cerr  << "====>> sweep how attr _angleStepDeg: " << _angleStepDeg << endl;
    cerr  << "====>> sweep how attr _angleSync: " << _angleSync << endl;
    cerr  << "====>> sweep how attr _angleStart: " << _angleStart << endl;
    cerr  << "====>> sweep how attr _angleStop: " << _angleStop << endl;
    cerr  << "====>> sweep how attr _nGates: " << _nGates << endl;
    cerr  << "====>> sweep how attr _clutterFilterNumber: " << _clutterFilterNumber << endl;
    cerr  << "====>> sweep how attr _fixedAngleDeg: " << _fixedAngleDeg << endl;
    cerr  << "====>> sweep how attr _malfunc: " << _malfunc << endl;
    cerr  << "====>> sweep how attr _pulseWidthUs: " << _pulseWidthUs << endl;
    cerr  << "====>> sweep how attr _wavelengthM: " << _wavelengthM << endl;
    cerr  << "====>> sweep how attr _frequencyHz: " << _frequencyHz << endl;
    cerr  << "====>> sweep how attr _maxRangeKm: " << _maxRangeKm << endl;
    cerr  << "====>> sweep how attr _nSamplesRange: " << _nSamplesRange << endl;
    cerr  << "====>> sweep how attr _nSamplesTime: " << _nSamplesTime << endl;
    cerr  << "====>> sweep how attr _startRangeKm: " << _startRangeKm << endl;
    cerr  << "====>> sweep how attr _gateSpacingKm: " << _gateSpacingKm << endl;
    cerr  << "====>> sweep how attr _nRaysSweep: " << _nRaysSweep << endl;
    cerr  << "====>> sweep how attr _scanRateDegPerSec: " << _scanRateDegPerSec << endl;
    cerr  << "====>> sweep how attr _sweepTimeStamp: " << _sweepTimeStamp << endl;
    cerr  << "====>> sweep how attr _prtMode: " << Radx::prtModeToStr(_prtMode) << endl;
    cerr  << "====>> sweep how attr _prtRatio: " << _prtRatio << endl;
  }

  return 0;

}

//////////////////////////////////////////////
// process a sweep what group

int GamicHdf5RadxFile::_readSweepWhat(Group &what, int sweepNum)

{

  Hdf5xx::DecodedAttr decodedAttr;

  _product = "SCAN";
  if (_utils.loadAttribute(what, "product", "sweep-what-group", decodedAttr) == 0) {
    _product = decodedAttr.getAsString();
  }
  
  _scanTypeStr = "PPI";
  if (_utils.loadAttribute(what, "scan_type", "sweep-what-group", decodedAttr) == 0) {
    _scanTypeStr = decodedAttr.getAsString();
  }
  if (_scanTypeStr == "PPI") {
    _isRhi = false;
  } else {
    _isRhi = true;
  }
  
  if (_utils.loadAttribute(what, "descriptor_count", "sweep-what-group", decodedAttr)) {
    return -1;
  }
  _nFields = decodedAttr.getAsInt();
  
  if (_debug) {
    cerr  << "====>> sweep what attr _product: " << _product << endl;
    cerr  << "====>> sweep what attr _scanTypeStr: " << _scanTypeStr << endl;
    cerr  << "====>> sweep what attr _isRhi: " << (_isRhi? "Y":"N") << endl;
    cerr  << "====>> sweep what attr _nFields: " << _nFields << endl;
  }

  return 0;

}

//////////////////////////////////////////////
// process a sweep extended group

int GamicHdf5RadxFile::_readSweepExtended(Group &extended)

{

  // initialize

  char tag[128];
  sprintf(tag, "SweepExtendedAttr_%d", _sweepNumber);
  _sweepStatusXml += RadxXml::writeStartTag(tag, 1);

  // loop through attributes, decoding each
  
  Hdf5xx::DecodedAttr decodedAttr;
  for (int ii = 0; ii < extended.getNumAttrs(); ii++) {

    hid_t attrId = H5Aopen_idx(extended.getId(), ii);
    Attribute attr(attrId);
    string name(attr.getName());
    
    if (_utils.loadAttribute(extended, name, 
                             "sweep-extended-group", decodedAttr) == 0) {
      if (decodedAttr.isString()) {
        _sweepStatusXml += 
          RadxXml::writeString(name, 2, decodedAttr.getAsString());
      } else if (decodedAttr.isInt()) {
        char valStr[128];
        sprintf(valStr, "%lld", (long long int) decodedAttr.getAsInt());
        _sweepStatusXml += RadxXml::writeString(name, 2, valStr);
      } else if (decodedAttr.isDouble()) {
        char valStr[128];
        sprintf(valStr, "%lg", decodedAttr.getAsDouble());
        _sweepStatusXml += RadxXml::writeString(name, 2, valStr);
      }
    }

  } // ii
  
  // finalize XML
  
  _sweepStatusXml += RadxXml::writeEndTag(tag, 1);

  // nyquist

  _unambigVel = Radx::missingFl64;
  if (_utils.loadAttribute(extended, "unambiguous_velocity", 
                     "sweep-extended-group", decodedAttr) == 0) {
    _unambigVel = decodedAttr.getAsDouble();
  } else if (_utils.loadAttribute(extended, "nyquist_velocity", 
                            "sweep-extended-group", decodedAttr) == 0) {
    _unambigVel = decodedAttr.getAsDouble();
  }
  
  if (_debug) {
    cerr << "========= sweep extended attr XML ===============" << endl;
    cerr << _sweepStatusXml;
    cerr << "=================================================" << endl;
    cerr  << "====>> sweep extended attr _unambigVel: " << _unambigVel << endl;
  }

  return 0;

}
  
////////////////////////////////////////
// read in the ray headers, create rays


int GamicHdf5RadxFile::_readRays(Group &sweep, int sweepNumber)
{
  
  clearErrStr();  
  DataSet *rayHdrs = NULL;
  try {
    rayHdrs = new DataSet(sweep.openDataSet("ray_header"));
  }
  catch (H5::Exception e) {
    _addErrInt("ERROR - no 'ray_header' data set, sweep number: ", sweepNumber);
    if (rayHdrs) delete rayHdrs;
    return -1;
  }
  DataType dtype = rayHdrs->getDataType();
  H5T_class_t aclass = dtype.getClass();
  DataSpace dataspace = rayHdrs->getSpace();

  if (aclass != H5T_COMPOUND) {
    _addErrStr("ERROR - GamicHdf5RadxFile::_readRays");
    _addErrStr("  ray_headers is not COMPOUND type");
    delete rayHdrs;
    return -1;
  }

  int ndims = dataspace.getSimpleExtentNdims();
  if (ndims != 1) {
    _addErrStr("ERROR - GamicHdf5RadxFile::_readRays");
    _addErrStr("  Dimension incorrect");
    _addErrInt("    Found: ", ndims);
    _addErrStr("    Expected: 1");
    delete rayHdrs;
    return -1;
  }

  int nRays = dataspace.getSimpleExtentNpoints();
  if (nRays != _nRaysSweep) {
    _addErrStr("ERROR - GamicHdf5RadxFile::_readRays");
    _addErrStr("  nRays incorrect, should match nRaysSweep");
    _addErrInt("    Found: ", nRays);
    _addErrInt("    Expected: ", _nRaysSweep);
    delete rayHdrs;
    return -1;
  }

  // read in headers

  CompType compType = rayHdrs->getCompType();
  int msize = compType.getSize();
  char *buf = new char[nRays * msize]; 
  rayHdrs->read(buf, dtype);
  delete rayHdrs;
  
  // decode headers, create rays

  vector<RadxRay *> rays;
  int iret = 0;
  for (int iray = 0; iray < nRays; iray++) {
    
    int offset = iray * msize;
    RadxRay *ray = new RadxRay;
    if (_loadRayMetadata(compType, iray, buf + offset, ray)) {
      _addErrStr("ERROR - GamicHdf5RadxFile::_readRays");
      _addErrStr("        cannot load ray metadata");
      delete ray;
      iret = -1;
      break;
    }

    // set ray props

    ray->setVolumeNumber(_volumeNumber);
    ray->setSweepNumber(_sweepNumber);
    if (_angleSync) {
      ray->setIsIndexed(true);
      ray->setAngleResDeg(_angleStepDeg);
    }
    ray->setNSamples(_nSamplesTime * _nSamplesRange);
    ray->setPulseWidthUsec(_pulseWidthUs);
    ray->setPrtSec(1.0 / _prfHz);
    ray->setPrtRatio(_prtRatio);
    ray->setNyquistMps(_unambigVel);
    ray->setUnambigRangeKm(_maxRangeKm);
    ray->setTargetScanRateDegPerSec(_scanRateDegPerSec);
    ray->setFixedAngleDeg(_fixedAngleDeg);

    if (_isRhi) {
      ray->setSweepMode(Radx::SWEEP_MODE_RHI);
    } else {
      if (_angleStart == _angleStop) {
        ray->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
      } else {
        ray->setSweepMode(Radx::SWEEP_MODE_SECTOR);
      }
    }
    ray->setPrtMode(_prtMode);
    ray->setRangeGeom(_startRangeKm, _gateSpacingKm);

    rays.push_back(ray);

  } // iray

  // clean up
  
  delete[] buf;

  // error?

  if (iret) {
    // free up rays from array
    for (size_t ii = 0; ii < rays.size(); ii++) {
      delete rays[ii];
    }
    return -1;
  }

  // add fields to rays

  for (int ifield = 0; ifield < _nFields; ifield++) {
    if (_addFieldToRays(sweep, rays, ifield)) {
      // free up rays from array
      for (size_t ii = 0; ii < rays.size(); ii++) {
        delete rays[ii];
      }
      return -1;
    }
  } // ifield
  
  // add rays to vol
  
  for (size_t ii = 0; ii < rays.size(); ii++) {
    _readVol->addRay(rays[ii]);
  }


  return 0;
}

////////////////////////////////////////
// read in the ray headers, create rays

int GamicHdf5RadxFile::_loadRayMetadata(CompType compType,
                                        int iray, 
                                        char *buf,
                                        RadxRay *ray)
{

  // time
  
  Radx::si64 timeStamp = 0;
  if (_utils.loadIntVar(compType, buf, "timestamp", timeStamp)) {
    return -1;
  }
  time_t timeSecs = timeStamp / 1000000;
  double  nanoSecs = (timeStamp - timeSecs * 1000000) * 1000.0;
  ray->setTime(timeSecs, nanoSecs);

  // azimuth

  Radx::fl64 azStart = 0.0;
  if (_utils.loadFloatVar(compType, buf, "azimuth_start", azStart)) {
    return -1;
  }
  Radx::fl64 azStop = 0.0;
  if (_utils.loadFloatVar(compType, buf, "azimuth_stop", azStop)) {
    return -1;
  }

  double az = RadxComplex::computeMeanDeg(azStart, azStop);
  if (az < 0.0) {
    az += 360.0;
  }
  ray->setAzimuthDeg(az);

  // elevation
  
  Radx::fl64 elStart = 0.0;
  if (_utils.loadFloatVar(compType, buf, "elevation_start", elStart)) {
    return -1;
  }
  Radx::fl64 elStop = 0.0;
  if (_utils.loadFloatVar(compType, buf, "elevation_stop", elStop)) {
    return -1;
  }
  double el = RadxComplex::computeMeanDeg(elStart, elStop);
  ray->setElevationDeg(el);

  // angular rate
  
  if (_isRhi) {
    Radx::fl64 angleRate = 0.0;
    if (_utils.loadFloatVar(compType, buf, "el_speed", angleRate) == 0) {
      ray->setTrueScanRateDegPerSec(angleRate);
    }
  } else {
    Radx::fl64 angleRate = 0.0;
    if (_utils.loadFloatVar(compType, buf, "az_speed", angleRate) == 0) {
      ray->setTrueScanRateDegPerSec(angleRate);
    }
  }
  
  return 0;

}

//////////////////////////////////////////////////
// add field to rays

int GamicHdf5RadxFile::_addFieldToRays(Group &sweep,
                                       vector<RadxRay *> &rays, 
                                       int fieldNum)

{
  // compute field name

  char momentName[1024];
  sprintf(momentName, "moment_%d", fieldNum);

  // get data set
  
  DataSet *ds = NULL;
  try {
    ds = new DataSet(sweep.openDataSet(momentName));
  }
  catch (H5::Exception e) {
    _addErrStr("ERROR - GamicHdf5RadxFile::_addFieldToRays");
    _addErrStr("  Cannot open data set for moment");
    _addErrStr("  Moment name: ", momentName);
    if (ds) delete ds;
    return -1;
  }
  
  // get field name

  Hdf5xx::DecodedAttr decodedAttr;

  if (_utils.loadAttribute(*ds, "moment", 
                           momentName, decodedAttr)) {
    _addErrStr("ERROR - GamicHdf5RadxFile::_addFieldToRays");
    _addErrStr("  Moment name: ", momentName);
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  string fieldName = decodedAttr.getAsString();

  // check that we need this field
  
  if (!isFieldRequiredOnRead(fieldName)) {
    if (_verbose) {
      cerr << "DEBUG - GamicHdf5RadxFile::_addFieldToRays" << endl;
      cerr << "  -->> rejecting field: " << fieldName << endl;
    }
    return 0;
  }

  // units

  string units;
  if (_utils.loadAttribute(*ds, "unit", 
                           momentName, decodedAttr) == 0) {
    units = decodedAttr.getAsString();
  }

  // dynamic range of packed data

  if (_utils.loadAttribute(*ds, "dyn_range_max", 
                           momentName, decodedAttr)) {
    _addErrStr("ERROR - GamicHdf5RadxFile::_addFieldToRays");
    _addErrStr("  Moment name: ", momentName);
    _addErrStr("  Field name: ", fieldName);
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  double dynRangeMax = decodedAttr.getAsDouble();

  if (_utils.loadAttribute(*ds, "dyn_range_min", 
                           momentName, decodedAttr)) {
    _addErrStr("ERROR - GamicHdf5RadxFile::_addFieldToRays");
    _addErrStr("  Moment name: ", momentName);
    _addErrStr("  Field name: ", fieldName);
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  double dynRangeMin = decodedAttr.getAsDouble();

  // get standard name and long name if applicable

  string standardName, longName;
  _lookupStandardName(fieldName, units, standardName, longName);

  // get data size

  DataSpace dataspace = ds->getSpace();
  int nPoints = dataspace.getSimpleExtentNpoints();

  // get dimensions

  int ndims = dataspace.getSimpleExtentNdims();
  if (ndims != 2) {
    _addErrStr("ERROR - GamicHdf5RadxFile::_addFieldToRays");
    _addErrStr("  Moment name: ", momentName);
    _addErrStr("  Field name: ", fieldName);
    _addErrStr("  Data is not 2-D array");
    _addErrStr("  Should be [nrays][ngates]");
    delete ds;
    return -1;

  }

  hsize_t dims[2];
  dataspace.getSimpleExtentDims(dims);
  int nRays = dims[0];
  int nGates = dims[1];
  
  if (nRays != (int) rays.size()) {
    _addErrStr("ERROR - GamicHdf5RadxFile::_addFieldToRays");
    _addErrStr("  Moment name: ", momentName);
    _addErrStr("  Field name: ", fieldName);
    _addErrStr("  nRays incorrect, should match nRaysSweep");
    _addErrInt("  Found: ", nRays);
    _addErrInt("  Expected: ", rays.size());
    delete ds;
    return -1;
  }

  if (nGates != _nGates) {
    _addErrStr("ERROR - GamicHdf5RadxFile::_addFieldToRays");
    _addErrStr("  Moment name: ", momentName);
    _addErrStr("  Field name: ", fieldName);
    _addErrStr("  nGates incorrect, should match bin_count");
    _addErrInt("  nGates: ", nGates);
    _addErrInt("  bin_count: ", _nGates);
    delete ds;
    return -1;
  }
  
  // get data type and size

  DataType dtype = ds->getDataType();
  H5T_class_t aclass = dtype.getClass();
  size_t tsize = dtype.getSize();
  
  if (aclass == H5T_INTEGER) {

    if (tsize == 1) {

      _loadSi08Field(*ds, fieldName, units, standardName, longName,
                     nRays, nGates, nPoints,
                     dynRangeMin, dynRangeMax, rays);
  
    } else if (tsize == 2) {

      _loadSi16Field(*ds, fieldName, units, standardName, longName,
                     nRays, nGates, nPoints,
                     dynRangeMin, dynRangeMax, rays);
  
    } else if (tsize == 4) {

      _loadSi32Field(*ds, fieldName, units, standardName, longName,
                     nRays, nGates, nPoints,
                     dynRangeMin, dynRangeMax, rays);
  
    } else {
      
      _addErrStr("ERROR - GamicHdf5RadxFile::_addFieldToRays");
      _addErrStr("  Moment name: ", momentName);
      _addErrStr("  Field name: ", fieldName);
      _addErrInt("  integer data size not supported: ", tsize);
      delete ds;
      return -1;
      
    }

  } else if (aclass == H5T_FLOAT) {

    if (tsize == 4) {

      _loadFl32Field(*ds, fieldName, units, standardName, longName,
                     nRays, nGates, nPoints, rays);
  
    } else if (tsize == 8) {
      
      _loadFl64Field(*ds, fieldName, units, standardName, longName,
                     nRays, nGates, nPoints, rays);
  
    } else {

      _addErrStr("ERROR - GamicHdf5RadxFile::_addFieldToRays");
      _addErrStr("  Moment name: ", momentName);
      _addErrStr("  Field name: ", fieldName);
      _addErrInt("  float data size not supported: ", tsize);
      delete ds;
      return -1;
      
    }

  } else {
    
    _addErrStr("ERROR - GamicHdf5RadxFile::_addFieldToRays");
    _addErrStr("  Moment name: ", momentName);
    _addErrStr("  Field name: ", fieldName);
    _addErrStr("  data type not supported: ", dtype.fromClass());
    delete ds;
    return -1;
      
  }

#ifdef TESTING
    
  // create float array for holding the data

  Radx::fl32 *floatVals = new Radx::fl32[nPoints];

  // load up float array

  if (_loadFloatArray(*ds, fieldName, nPoints, 
                      dynRangeMin, dynRangeMax, floatVals)) {
    _addErrStr("ERROR - GamicHdf5RadxFile::_addFieldToRays");
    _addErrStr("  Data is not integer or float type");
    _addErrStr("  Moment name: ", momentName);
    _addErrStr("  Field name: ", fieldName);
    delete ds;
    delete[] floatVals;
    return -1;
  }

  // add data to rays

  for (size_t iray = 0; iray < rays.size(); iray++) {

    // load field data

    int dataOffset = iray * nGates;
    RadxField *field = new RadxField(fieldName, units);
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->setMissingFl32(Radx::missingFl32);
    field->addDataFl32(nGates, floatVals + dataOffset);
    field->setRangeGeom(_startRangeKm, _gateSpacingKm);
    
    // convert field to 16-bit

    field->convertToSi16();

    // add to ray

    rays[iray]->addField(field);

  } // iray
  delete[] floatVals;

#endif

  // clean up

  delete ds;
  return 0;

}

///////////////////////////////////////////////////////////////////
// Load float array for given data set, using the type passed in

int GamicHdf5RadxFile::_loadFloatArray(DataSet &ds,
                                       const string dsname,
                                       int nPoints,
                                       double dynRangeMin,
                                       double dynRangeMax,
                                       Radx::fl32 *floatVals)
  
{

  DataType dtype = ds.getDataType();
  H5T_class_t aclass = dtype.getClass();
  
  if (aclass == H5T_INTEGER) {

    IntType intType = ds.getIntType();
    H5T_order_t order = intType.getOrder();
    H5T_sign_t sign = intType.getSign();
    size_t tsize = intType.getSize();
    
    if (sign == H5T_SGN_NONE) {
      
      // unsigned
      
      if (tsize == 1) {

        double range = dynRangeMax - dynRangeMin;
        double scale = range / 255.0;
        Radx::ui08 *ivals = new Radx::ui08[nPoints];
        ds.read(ivals, dtype);
        for (int ii = 0; ii < nPoints; ii++) {
          if (ivals[ii] == 0) {
            floatVals[ii] = Radx::missingFl32;
          } else {
            floatVals[ii] = dynRangeMin + ivals[ii] * scale;
          }
        }
        delete[] ivals;

      } else if (tsize == 2) {

        double range = dynRangeMax - dynRangeMin;
        double scale = range / 65535.0;
        Radx::ui16 *ivals = new Radx::ui16[nPoints];
        ds.read(ivals, dtype);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap16(ivals, nPoints * sizeof(Radx::ui16), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap16(ivals, nPoints * sizeof(Radx::ui16), true);
          }
        }
        for (int ii = 0; ii < nPoints; ii++) {
          if (ivals[ii] == 0) {
            floatVals[ii] = Radx::missingFl32;
          } else {
            floatVals[ii] = dynRangeMin + ivals[ii] * scale;
          }
        }
        delete[] ivals;

      } else if (tsize == 4) {

        double range = dynRangeMax - dynRangeMin;
        double scale = range / (pow(2.0, 32.0) - 1.0);
        Radx::ui32 *ivals = new Radx::ui32[nPoints];
        ds.read(ivals, dtype);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap32(ivals, nPoints * sizeof(Radx::ui32), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap32(ivals, nPoints * sizeof(Radx::ui32), true);
          }
        }
        for (int ii = 0; ii < nPoints; ii++) {
          if (ivals[ii] == 0) {
            floatVals[ii] = Radx::missingFl32;
          } else {
            floatVals[ii] = dynRangeMin + ivals[ii] * scale;
          }
        }
        delete[] ivals;

      } else if (tsize == 8) {

        double range = dynRangeMax - dynRangeMin;
        double scale = range / (pow(2.0, 64.0) - 1.0);
        Radx::ui64 *ivals = new Radx::ui64[nPoints];
        ds.read(ivals, dtype);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap64(ivals, nPoints * sizeof(Radx::ui64), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap64(ivals, nPoints * sizeof(Radx::ui64), true);
          }
        }
        for (int ii = 0; ii < nPoints; ii++) {
          if (ivals[ii] == 0) {
            floatVals[ii] = Radx::missingFl32;
          } else {
            floatVals[ii] = dynRangeMin + ivals[ii] * scale;
          }
        }
        delete[] ivals;
      }

    } else {

      // signed

      if (tsize == 1) {

        double range = dynRangeMax - dynRangeMin;
        double scale = range / 255.0;
        Radx::si08 *ivals = new Radx::si08[nPoints];
        ds.read(ivals, dtype);
        for (int ii = 0; ii < nPoints; ii++) {
          if (ivals[ii] == 0) {
            floatVals[ii] = Radx::missingFl32;
          } else {
            floatVals[ii] = dynRangeMin + ivals[ii] * scale;
          }
        }
        delete[] ivals;

      } else if (tsize == 2) {

        double range = dynRangeMax - dynRangeMin;
        double scale = range / 65535.0;
        Radx::si16 *ivals = new Radx::si16[nPoints];
        ds.read(ivals, dtype);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap16(ivals, nPoints * sizeof(Radx::si16), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap16(ivals, nPoints * sizeof(Radx::si16), true);
          }
        }
        for (int ii = 0; ii < nPoints; ii++) {
          if (ivals[ii] == 0) {
            floatVals[ii] = Radx::missingFl32;
          } else {
            floatVals[ii] = dynRangeMin + ivals[ii] * scale;
          }
        }
        delete[] ivals;

      } else if (tsize == 4) {

        double range = dynRangeMax - dynRangeMin;
        double scale = range / (pow(2.0, 32.0) - 1.0);
        Radx::si32 *ivals = new Radx::si32[nPoints];
        ds.read(ivals, dtype);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap32(ivals, nPoints * sizeof(Radx::si32), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap32(ivals, nPoints * sizeof(Radx::si32), true);
          }
        }
        for (int ii = 0; ii < nPoints; ii++) {
          if (ivals[ii] == 0) {
            floatVals[ii] = Radx::missingFl32;
          } else {
            floatVals[ii] = dynRangeMin + ivals[ii] * scale;
          }
        }
        delete[] ivals;

      } else if (tsize == 8) {

        double range = dynRangeMax - dynRangeMin;
        double scale = range / (pow(2.0, 64.0) - 1.0);
        Radx::si64 *ivals = new Radx::si64[nPoints];
        ds.read(ivals, dtype);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap64(ivals, nPoints * sizeof(Radx::si64), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap64(ivals, nPoints * sizeof(Radx::si64), true);
          }
        }
        for (int ii = 0; ii < nPoints; ii++) {
          if (ivals[ii] == 0) {
            floatVals[ii] = Radx::missingFl32;
          } else {
            floatVals[ii] = dynRangeMin + ivals[ii] * scale;
          }
        }
        delete[] ivals;
      }

    }

  } else if (aclass == H5T_FLOAT) {

    FloatType flType = ds.getFloatType();
    H5T_order_t order = flType.getOrder();
    size_t tsize = flType.getSize();
    
    if (tsize == 4) {

      Radx::fl32 *fvals = new Radx::fl32[nPoints];
      ds.read(fvals, dtype);
      if (ByteOrder::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          ByteOrder::swap32(fvals, nPoints * sizeof(Radx::fl32), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          ByteOrder::swap32(fvals, nPoints * sizeof(Radx::fl32), true);
        }
      }
      for (int ii = 0; ii < nPoints; ii++) {
        floatVals[ii] = fvals[ii];
      }
      delete[] fvals;

    } else if (tsize == 8) {

      Radx::fl64 *fvals = new Radx::fl64[nPoints];
      ds.read(fvals, dtype);
      if (ByteOrder::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          ByteOrder::swap64(fvals, nPoints * sizeof(Radx::fl64), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          ByteOrder::swap64(fvals, nPoints * sizeof(Radx::fl64), true);
        }
      }
      for (int ii = 0; ii < nPoints; ii++) {
        floatVals[ii] = fvals[ii];
      }
      delete[] fvals;

    }

  } else {

    return -1;

  }

  return 0;

}

///////////////////////////////////////////////////////////////////
// Load si08 array for given data set, using the type passed in

void GamicHdf5RadxFile::_loadSi08Field(DataSet &ds,
                                       const string &fieldName,
                                       const string &units,
                                       const string &standardName,
                                       const string &longName,
                                       int nRays,
                                       int nGates,
                                       int nPoints,
                                       double dynRangeMin,
                                       double dynRangeMax,
                                       vector<RadxRay *> &rays)
  
{
  
  DataType dtype = ds.getDataType();
  IntType intType = ds.getIntType();
  H5T_sign_t sign = intType.getSign();

  int irange = 255;
  int imin = -128;

  double range = dynRangeMax - dynRangeMin;
  double scale = range / (double) irange;
  double offset = dynRangeMin;
  
  Radx::si08 *ivals = new Radx::si08[nPoints];
  
  if (sign == H5T_SGN_NONE) {
    
    // unsigned
    
    Radx::ui08 *uvals = new Radx::ui08[nPoints];
    ds.read(uvals, dtype);
    for (int ii = 0; ii < nPoints; ii++) {
      int ival = (int) uvals[ii] + imin;
      ivals[ii] = (Radx::si08) ival;
    }
    delete[] uvals;
    offset -= (double) imin * scale;
    
  } else {
    
    // signed
    
    ds.read(ivals, dtype);

  }
  
  // add data to rays
  
  for (size_t iray = 0; iray < rays.size(); iray++) {
    
    // load field data
    
    int dataOffset = iray * nGates;
    RadxField *field = new RadxField(fieldName, units);
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->setTypeSi08(Radx::missingSi08, scale, offset);
    field->addDataSi08(nGates, ivals + dataOffset);
    field->setRangeGeom(_startRangeKm, _gateSpacingKm);
    
    // add to ray

    rays[iray]->addField(field);

  } // iray

  // clean up

  delete[] ivals;

}

///////////////////////////////////////////////////////////////////
// Load si16 array for given data set, using the type passed in

void GamicHdf5RadxFile::_loadSi16Field(DataSet &ds,
                                       const string &fieldName,
                                       const string &units,
                                       const string &standardName,
                                       const string &longName,
                                       int nRays,
                                       int nGates,
                                       int nPoints,
                                       double dynRangeMin,
                                       double dynRangeMax,
                                       vector<RadxRay *> &rays)
  
{
  
  DataType dtype = ds.getDataType();
  IntType intType = ds.getIntType();
  H5T_order_t order = intType.getOrder();
  H5T_sign_t sign = intType.getSign();

  int irange = 65535;
  int imin = -32768;

  double range = dynRangeMax - dynRangeMin;
  double scale = range / (double) irange;
  double offset = dynRangeMin;
  
  Radx::si16 *vals = new Radx::si16[nPoints];
  
  if (sign == H5T_SGN_NONE) {
    
    // unsigned
    
    Radx::ui16 *uvals = new Radx::ui16[nPoints];
    ds.read(uvals, dtype);
    
    if (ByteOrder::hostIsBigEndian()) {
      if (order == H5T_ORDER_LE) {
        ByteOrder::swap16(uvals, nPoints * sizeof(Radx::ui16), true);
      }
    } else {
      if (order == H5T_ORDER_BE) {
        ByteOrder::swap16(uvals, nPoints * sizeof(Radx::ui16), true);
      }
    }
    
    for (int ii = 0; ii < nPoints; ii++) {
      int ival = (int) uvals[ii] + imin;
      vals[ii] = (Radx::si16) ival;
    }
    delete[] uvals;
    offset -= (double) imin * scale;
  
  } else {
    
    // signed
    
    ds.read(vals, dtype);

    if (ByteOrder::hostIsBigEndian()) {
      if (order == H5T_ORDER_LE) {
        ByteOrder::swap16(vals, nPoints * sizeof(Radx::ui16), true);
      }
    } else {
      if (order == H5T_ORDER_BE) {
        ByteOrder::swap16(vals, nPoints * sizeof(Radx::ui16), true);
      }
    }
    
  }
  
  // add data to rays
  
  for (size_t iray = 0; iray < rays.size(); iray++) {
    
    // load field data
    
    int dataOffset = iray * nGates;
    RadxField *field = new RadxField(fieldName, units);
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->setTypeSi16(Radx::missingSi16, scale, offset);
    field->addDataSi16(nGates, vals + dataOffset);
    field->setRangeGeom(_startRangeKm, _gateSpacingKm);
    
    // add to ray

    rays[iray]->addField(field);

  } // iray

  // clean up

  delete[] vals;

}

///////////////////////////////////////////////////////////////////
// Load si32 array for given data set, using the type passed in

void GamicHdf5RadxFile::_loadSi32Field(DataSet &ds,
                                       const string &fieldName,
                                       const string &units,
                                       const string &standardName,
                                       const string &longName,
                                       int nRays,
                                       int nGates,
                                       int nPoints,
                                       double dynRangeMin,
                                       double dynRangeMax,
                                       vector<RadxRay *> &rays)
  
{

  DataType dtype = ds.getDataType();
  IntType intType = ds.getIntType();
  H5T_order_t order = intType.getOrder();
  H5T_sign_t sign = intType.getSign();
  
  Radx::si64 irange = 4294967295;
  Radx::si64 imin = -2147483648;

  double range = dynRangeMax - dynRangeMin;
  double scale = range / (double) irange;
  double offset = dynRangeMin;
  
  Radx::si32 *vals = new Radx::si32[nPoints];
  
  if (sign == H5T_SGN_NONE) {
    
    // unsigned
    
    Radx::ui32 *uvals = new Radx::ui32[nPoints];
    ds.read(uvals, dtype);
    
    if (ByteOrder::hostIsBigEndian()) {
      if (order == H5T_ORDER_LE) {
        ByteOrder::swap32(uvals, nPoints * sizeof(Radx::ui32), true);
      }
    } else {
      if (order == H5T_ORDER_BE) {
        ByteOrder::swap32(uvals, nPoints * sizeof(Radx::ui32), true);
      }
    }
    
    for (int ii = 0; ii < nPoints; ii++) {
      Radx::si64 ival = (Radx::si64) uvals[ii] + imin;
      vals[ii] = (Radx::si32) ival;
    }
    delete[] uvals;
    offset -= (double) imin * scale;
  
  } else {
    
    // signed
    
    ds.read(vals, dtype);

    if (ByteOrder::hostIsBigEndian()) {
      if (order == H5T_ORDER_LE) {
        ByteOrder::swap32(vals, nPoints * sizeof(Radx::ui32), true);
      }
    } else {
      if (order == H5T_ORDER_BE) {
        ByteOrder::swap32(vals, nPoints * sizeof(Radx::ui32), true);
      }
    }
    
  }
  
  // add data to rays
  
  for (size_t iray = 0; iray < rays.size(); iray++) {
    
    // load field data
    
    int dataOffset = iray * nGates;
    RadxField *field = new RadxField(fieldName, units);
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->setTypeSi32(Radx::missingSi32, scale, offset);
    field->addDataSi32(nGates, vals + dataOffset);
    field->setRangeGeom(_startRangeKm, _gateSpacingKm);
    
    // add to ray

    rays[iray]->addField(field);

  } // iray

  // clean up

  delete[] vals;

}

///////////////////////////////////////////////////////////////////
// Load fl32 array for given data set, using the type passed in

void GamicHdf5RadxFile::_loadFl32Field(DataSet &ds,
                                       const string &fieldName,
                                       const string &units,
                                       const string &standardName,
                                       const string &longName,
                                       int nRays,
                                       int nGates,
                                       int nPoints,
                                       vector<RadxRay *> &rays)
  
{
  
  DataType dtype = ds.getDataType();
  FloatType floatType = ds.getFloatType();
  H5T_order_t order = floatType.getOrder();
  
  Radx::fl32 *vals = new Radx::fl32[nPoints];
  ds.read(vals, dtype);
  
  if (ByteOrder::hostIsBigEndian()) {
    if (order == H5T_ORDER_LE) {
      ByteOrder::swap32(vals, nPoints * sizeof(Radx::fl32), true);
    }
  } else {
    if (order == H5T_ORDER_BE) {
      ByteOrder::swap32(vals, nPoints * sizeof(Radx::fl32), true);
    }
  }
    
  // add data to rays
  
  for (size_t iray = 0; iray < rays.size(); iray++) {
    
    // load field data
    
    int dataOffset = iray * nGates;
    RadxField *field = new RadxField(fieldName, units);
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->setTypeFl32(Radx::missingFl32);
    field->addDataFl32(nGates, vals + dataOffset);
    field->setRangeGeom(_startRangeKm, _gateSpacingKm);
    
    // add to ray

    rays[iray]->addField(field);

  } // iray

  // clean up

  delete[] vals;

}

///////////////////////////////////////////////////////////////////
// Load fl64 array for given data set, using the type passed in

void GamicHdf5RadxFile::_loadFl64Field(DataSet &ds,
                                       const string &fieldName,
                                       const string &units,
                                       const string &standardName,
                                       const string &longName,
                                       int nRays,
                                       int nGates,
                                       int nPoints,
                                       vector<RadxRay *> &rays)
  
{
  
  DataType dtype = ds.getDataType();
  FloatType floatType = ds.getFloatType();
  H5T_order_t order = floatType.getOrder();
  
  Radx::fl64 *vals = new Radx::fl64[nPoints];
  ds.read(vals, dtype);
  
  if (ByteOrder::hostIsBigEndian()) {
    if (order == H5T_ORDER_LE) {
      ByteOrder::swap64(vals, nPoints * sizeof(Radx::fl64), true);
    }
  } else {
    if (order == H5T_ORDER_BE) {
      ByteOrder::swap64(vals, nPoints * sizeof(Radx::fl64), true);
    }
  }
    
  // add data to rays
  
  for (size_t iray = 0; iray < rays.size(); iray++) {
    
    // load field data
    
    int dataOffset = iray * nGates;
    RadxField *field = new RadxField(fieldName, units);
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->setTypeFl64(Radx::missingFl64);
    field->addDataFl64(nGates, vals + dataOffset);
    field->setRangeGeom(_startRangeKm, _gateSpacingKm);
    
    // add to ray

    rays[iray]->addField(field);

  } // iray

  // clean up

  delete[] vals;

}

//////////////////////////////////////////////////////////
// lookup long and standard names appropriate to field name

void GamicHdf5RadxFile::_lookupStandardName(const string &fieldName, 
                                            const string &units,
                                            string &standardName,
                                            string &longName)
  
{

  if (fieldName.find("Z") != string::npos &&
      units.find("dBZ") != string::npos) {
    standardName = "equivalent_reflectivity_factor";
  }

  if (fieldName.find("V") != string::npos &&
      units.find("m/s") != string::npos) {
    standardName = "radial_velocity_of_scatterers_away_from_instrument";
  }

  if (fieldName.find("W") != string::npos &&
      units.find("m/s") != string::npos) {
    standardName = "doppler_spectrum_width";
  }

  if (fieldName.find("SQI") != string::npos) {
    standardName = "normalized_coherent_power";
  }

  if (fieldName.find("CCOR") != string::npos &&
      units.find("dB") != string::npos) {
    standardName = "clutter_power";
  }

  if (fieldName.find("SIGPOW") != string::npos &&
      units.find("dB") != string::npos) {
    standardName = "signal_power";
  }

  if (fieldName.find("SNR") != string::npos &&
      units.find("dB") != string::npos) {
    standardName = "signal_to_noise_ratio";
  }

  if (fieldName.find("ZDR") != string::npos &&
      units.find("dB") != string::npos) {
    standardName = "corrected_log_differential_reflectivity_hv";
  }

  if (fieldName.find("PHI") != string::npos &&
      units.find("deg") != string::npos) {
    standardName = "differential_phase_hv";
  }

  if (fieldName.find("RHO") != string::npos) {
    standardName = "cross_correlation_ratio_hv";
  }

  if (fieldName.find("KDP") != string::npos) {
    standardName = "specific_differential_phase_hv";
  }

  if (fieldName.find("LDR") != string::npos &&
      units.find("dB") != string::npos) {
    standardName = "log_linear_depolarization_ratio_hv";
  }

  if (fieldName == "I") {
    standardName = "in_phase_signal";
  }

  if (fieldName == "Q") {
    standardName = "quadrature_signal";
  }

  if (fieldName == "Z") {
    longName = "corrected_reflectivity_for_both_clutter_and_second_trip";
  }

  if (fieldName == "UZ") {
    longName = "uncorrected_reflectivity";
  }

  if (fieldName == "Zh") {
    longName = "corrected_reflectivity_for_both_clutter_and_second_trip_horizontal_channel";
  }

  if (fieldName == "Zv") {
    longName = "corrected_reflectivity_for_both_clutter_and_second_trip_vertical_channel";
  }

  if (fieldName == "UZh") {
    longName = "uncorrected_reflectivity_horizontal_channel";
  }

  if (fieldName == "UZv") {
    longName = "uncorrected_reflectivity_vertical_channel";
  }

  if (fieldName == "AZh") {
    longName = "rainfall_attenuation_clutter_corrected_reflectivity_horizontal_channel";
  }

  if (fieldName == "V") {
    longName = "unfolded_radial_velocity_from_corrected_timeseries";
  }

  if (fieldName == "VF") {
    longName = "folded_radial_velocity_from_corrected_timeseries";
  }

  if (fieldName == "UV") {
    longName = "unfolded_radial_velocity_from_uncorrected_raw_timeseries";
  }

  if (fieldName == "UVF") {
    longName = "folded_radial_velocity_from_uncorrected_raw_timeseries";
  }

  if (fieldName == "Vh") {
    longName = "radial_velocity_from_corrected_timeseries_horizontal_channel";
  }

  if (fieldName == "Vv") {
    longName = "radial_velocity_from_corrected_timeseries_vertical_channel";
  }

  if (fieldName == "UVh") {
    longName = "radial_velocity_from_uncorrected_timeseries_horizontal_channel";
  }

  if (fieldName == "UVv") {
    longName = "radial_velocity_from_uncorrected_timeseries_vertical_channel";
  }

  if (fieldName == "VFh") {
    longName = "folded_radial_velocity_from_corrected_timeseries_horizontal_channel";
  }

  if (fieldName == "VFv") {
    longName = "folded_radial_velocity_from_corrected_timeseries_vertical_channel";
  }

  if (fieldName == "UnV") {
    longName = "folded_radial_velocity_from_uncorrected_timeseries";
  }

  if (fieldName == "UnVFh") {
    longName = "folded_radial_velocity_from_uncorrected_timeseries_horizontal_channel";
  }

  if (fieldName == "UnVFv") {
    longName = "folded_radial_velocity_from_ucorrected_timeseries_vertical_channel";
  }

  if (fieldName == "W") {
    longName = "spectral_width_from_corrected_timeseries";
  }

  if (fieldName == "UW") {
    longName = "spectral_width_from_uncorrected_timeseries";
  }

  if (fieldName == "CW") {
    longName = "spectral_width_corrected_for_decorrelation_caused_by_antenna_rotation_from_clutter_corrected_timeseries";
  }

  if (fieldName == "UCW") {
    longName = "spectral_width_corrected_for_decorrelation_caused_by_antenna_rotation_from_uncorrected_timeseries";
  }

  if (fieldName == "Wh") {
    longName = "spectral_width_from_corrected_timeseries_horizontal_channel";
  }

  if (fieldName == "Wv") {
    longName = "spectral_width_from_corrected_timeseries_vertical_channel";
  }

  if (fieldName == "UWh") {
    longName = "spectral_width_from_uncorrected_timeseries_horizontal_channel";
  }

  if (fieldName == "UWv") {
    longName = "spectral_width_from_uncorrected_timeseries_vertical_channel";
  }

  if (fieldName == "CWh") {
    longName = "spectral_width_corrected_for_decorrelation_caused_by_antenna_rotation_horizontal_channel";
  }

  if (fieldName == "CWv") {
    longName = "spectral_width_corrected_for_decorrelation_caused_by_antenna_rotation_vertical_channel";
  }

  if (fieldName == "SQI") {
    longName = "signal_quality_index";
  }

  if (fieldName == "SQIh") {
    longName = "signal_quality_index_horizontal_channel";
  }

  if (fieldName == "SQIv") {
    longName = "signal_quality_index_vertical_channel";
  }

  if (fieldName == "CCOR") {
    longName = "clutter_power_correction";
  }

  if (fieldName == "CCORh") {
    longName = "clutter_power_correction_horizontal_channel";
  }

  if (fieldName == "CCORv") {
    longName = "clutter_power_correction_horizontal_channel";
  }

  if (fieldName == "SIGPOW") {
    longName = "signal_power";
  }

  if (fieldName == "SNR") {
    longName = "raw_signal_noise_ratio";
  }

  if (fieldName == "SNRh") {
    longName = "raw_signal_noise_ratio_horizontal";
  }

  if (fieldName == "SNRv") {
    longName = "raw_signal_noise_ratio_vertical";
  }

  if (fieldName == "DFT") {
    longName = "signal_spectrum_amplitude";
  }

  if (fieldName == "DFTh") {
    longName = "signal_spectrum_amplitude_horizontal_channel";
  }

  if (fieldName == "DFTv") {
    longName = "signal_spectrum_amplitude_vertical_channel";
  }

  if (fieldName == "LOG") {
    longName = "logarithmic_amplitude_10_log_Isq_+_Qsq__last_pulse_from_batch";
  }

  if (fieldName == "LOGh") {
    longName = "logarithmic_amplitude_10log_Isq_+_Qsq__horizontal_channel";
  }

  if (fieldName == "LOGv") {
    longName = "logarithmic_amplitude_10log_Isq_+_Qsq__vertical_channel";
  }
  
  if (fieldName == "CMAP") {
    longName = "censor_map";
  }

  if (fieldName == "ZDR") {
    longName = "differential_reflectivity_from_clutter_corrected_data";
  }

  if (fieldName == "UZDR") {
    longName = "differential_reflectivity_from_uncorrected_data";
  }

  if (fieldName == "AZDR") {
    longName = "rainfall_attenuation_corrected_clutter_corrected_differential_reflectivity";
  }

  if (fieldName == "ZDR1") {
    longName = "differential_reflectivity_from_clutter_corrected_data_1st_LAG_algorithm";
  }

  if (fieldName == "UZDR1") {
    longName = "differential_reflectivity_from_uncorrected_data_1st_LAG_algorithm";
  }

  if (fieldName == "AZDR1") {
    longName = "rainfall_attenuation_corrected_clutter_corrected_differential_reflectivity_1st_LAG_algorithm";
  }

  if (fieldName == "PHIDP") {
    longName = "differential_phase_from_corrected_timeseries";
  }

  if (fieldName == "UPHIDP") {
    longName = "differential_phase_from_uncorrected_timeseries";
  }

  if (fieldName == "PHIH") {
    longName = "differential_phase_for_horizontal_transmit_only_from_corrected_timeseries";
  }

  if (fieldName == "UPHIH") {
    longName = "differential_phase_for_horizontal_transmit_only_from_uncorrected_timeseries";
  }

  if (fieldName == "KDP") {
    longName = "specific_differential_phase";
  }

  if (fieldName == "RHOHV") {
    longName = "cross_correlation_coefficient_from_corrected_timeseries";
  }

  if (fieldName == "URHOHV") {
    longName = "cross_correlation_coefficient_from_uncorrected_timeseries";
  }

  if (fieldName == "RHOH") {
    longName = "cross_correlation_coefficient_for_horizontal_transmit_only_corrected_timeseries";
  }

  if (fieldName == "URHOH") {
    longName = "cross_correlation_coefficient_for_horizontal_transmit_only_uncorrected_timeseries";
  }

  if (fieldName == "LDR") {
    longName = "linear_depolarization_ratio_from_corrected_data";
  }

  if (fieldName == "ULDR") {
    longName = "linear_depolarization_ratio_from_corrected_data";
  }

}


//////////////////////////////////////////////////////////
// load up the read volume with the data from this object

int GamicHdf5RadxFile::_finalizeReadVolume()
  
{

  // set metadata

  _readVol->setOrigFormat("GAMIC");
  _readVol->setVolumeNumber(_volumeNumber);
  _readVol->setInstrumentType(_instrumentType);
  _readVol->setPlatformType(_platformType);
  _readVol->setPrimaryAxis(_primaryAxis);

  _readVol->addFrequencyHz(_frequencyHz);
  
  _readVol->setTitle("GAMIC radar data, version: " + _version);
  _readVol->setSource(_hostName);
  
  string hist = "software: " + _software + " version: " + _swVersion;
  _readVol->setHistory(hist);

  _readVol->setInstitution("");

  string ref = "SDP: " + _sdpName + " version: " + _sdpVersion;
  _readVol->setReferences(ref);

  char comment[10000];
  sprintf(comment,
          "template_name: %s, "
          "clutter_filter_number: %d",
          _templateName.c_str(), _clutterFilterNumber);
          
  _readVol->setComment(comment);

  _readVol->setStatusXml(_statusXml);
  _readVol->setSiteName(_siteName);
  _readVol->setScanName(_scanTypeStr);

  _readVol->setLatitudeDeg(_latitudeDeg);
  _readVol->setLongitudeDeg(_longitudeDeg);
  _readVol->setAltitudeKm(_altitudeKm);

  _readVol->setRadarBeamWidthDegH(_azBeamWidthDeg);
  _readVol->setRadarBeamWidthDegV(_elBeamWidthDeg);

  if (_readSetMaxRange) {
    _readVol->setMaxRangeKm(_readMaxRangeKm);
  }
  
  // load the sweep information from the rays

  _readVol->loadSweepInfoFromRays();
  
  // constrain the sweep data as appropriate
  
  if (_readFixedAngleLimitsSet) {
    if (_readVol->constrainByFixedAngle(_readMinFixedAngle, _readMaxFixedAngle,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - GamicHdf5RadxFile::_finalizeReadVolume");
      _addErrStr("  No data found within fixed angle limits");
      _addErrDbl("  min fixed angle: ", _readMinFixedAngle);
      _addErrDbl("  max fixed angle: ", _readMaxFixedAngle);
      return -1;
    }
  } else if (_readSweepNumLimitsSet) {
    if (_readVol->constrainBySweepNum(_readMinSweepNum, _readMaxSweepNum,
                                      _readStrictAngleLimits)) {
      _addErrStr("ERROR - GamicHdf5RadxFile::_finalizeReadVolume");
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

