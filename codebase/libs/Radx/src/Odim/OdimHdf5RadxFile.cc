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
// OdimHdf5RadxFile.cc
//
// ODIM HDF5 file data for radar radial data
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2013
//
///////////////////////////////////////////////////////////////

#include <Radx/NcfRadxFile.hh>
#include <Radx/OdimHdf5RadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxXml.hh>
#include <Radx/ByteOrder.hh>
#include <Radx/RadxComplex.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/RadxReadDir.hh>
#include <Radx/RadxArray.hh>
#include <Radx/RadxStr.hh>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <algorithm>

//////////////
// Constructor

OdimHdf5RadxFile::OdimHdf5RadxFile() : RadxFile()
  
{

  _volumeNumber = 0;
  _ncFormat = NETCDF_CLASSIC;
  _readVol = NULL;
  clear();
  
}

/////////////
// destructor

OdimHdf5RadxFile::~OdimHdf5RadxFile()

{
  clear();
  if (_file) {
    _file->close();
    delete _file;
  }
}

/////////////////////////////////////////////////////////
// clear the data in the object

void OdimHdf5RadxFile::clear()
  
{

  clearErrStr();

  _file = NULL;

  _instrumentType = Radx::INSTRUMENT_TYPE_RADAR;
  _platformType = Radx::PLATFORM_TYPE_FIXED;
  _primaryAxis = Radx::PRIMARY_AXIS_Z;
  _nSweeps = 0;

  _conventions.clear();

  _objectStr.clear();
  _version.clear();
  _dateStr.clear();
  _timeStr.clear();
  _source.clear();
  
  _latitudeDeg = Radx::missingMetaDouble;
  _longitudeDeg = Radx::missingMetaDouble;
  _altitudeKm = Radx::missingMetaDouble;

  _task.clear();
  _system.clear();
  _simulated.clear();
  _software.clear();
  _swVersion.clear();

  _polTypeStr.clear();
  _polModeStr.clear();
  _polMode = Radx::POL_MODE_NOT_SET;

  _wavelengthM = Radx::missingMetaDouble;
  _frequencyHz = Radx::missingMetaDouble;
  _scanRateRpm = Radx::missingMetaDouble;
  _scanRateDegPerSec = Radx::missingMetaDouble;
  
  _pulseWidthUs = Radx::missingMetaDouble;
  _rxBandwidthMhz = Radx::missingMetaDouble;
  
  _lowPrfHz = Radx::missingMetaDouble;
  _highPrfHz = Radx::missingMetaDouble;
  _prtMode = Radx::PRT_MODE_FIXED;
  _prtRatio = Radx::missingMetaDouble;
  
  _txLossH = Radx::missingMetaDouble;
  _txLossV = Radx::missingMetaDouble;
  _injectLossH = Radx::missingMetaDouble;
  _injectLossV = Radx::missingMetaDouble;
  _rxLossH = Radx::missingMetaDouble;
  _rxLossV = Radx::missingMetaDouble;

  _radomeLossOneWayH = Radx::missingMetaDouble;
  _radomeLossOneWayV = Radx::missingMetaDouble;
  _antennaGainH = Radx::missingMetaDouble;
  _antennaGainV = Radx::missingMetaDouble;
  _beamWidthH = Radx::missingMetaDouble;
  _beamWidthV = Radx::missingMetaDouble;
  _radarConstantH = Radx::missingMetaDouble;
  _radarConstantV = Radx::missingMetaDouble;
  _dbz0H = Radx::missingMetaDouble;
  _dbz0V = Radx::missingMetaDouble;
  _noiseH = Radx::missingMetaDouble;
  _noiseV = Radx::missingMetaDouble;

  _gasAttenDbPerKm = Radx::missingMetaDouble;
  _nomTxPowerKw = Radx::missingMetaDouble;
  _unambigVelMps = Radx::missingMetaDouble;

  _powerDiff = Radx::missingMetaDouble;
  _phaseDiff = Radx::missingMetaDouble;

  _nSamples = 0;

  _azMethod.clear();
  _binMethod.clear();

  _pointAccEl = Radx::missingMetaDouble;
  _pointAccAz = Radx::missingMetaDouble;

  _malfuncFlag = false;
  _malfuncMsg.clear();
  _maxRangeKm = Radx::missingMetaDouble;

  _comment.clear();

  _sqiThreshold = Radx::missingMetaDouble;
  _csrThreshold = Radx::missingMetaDouble;
  _logThreshold = Radx::missingMetaDouble;
  _snrThreshold = Radx::missingMetaDouble;

  _peakPowerKw = Radx::missingMetaDouble;
  _avPowerKw = Radx::missingMetaDouble;
  _dynRangeDb = Radx::missingMetaDouble;
  
  _polarization.clear();

  _nDataFields = 0;
  _nQualityFields = 0;
  _nFields = 0;
  _sweepMode = Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE;

  _a1Gate = 0;
  _aStart = 0.0;
  _fixedAngleDeg = Radx::missingMetaDouble;
  _nGates = 0;
  _nRaysSweep = 0;
  _startRangeKm = Radx::missingMetaDouble;
  _gateSpacingKm = Radx::missingMetaDouble;

  _product.clear();
  _fieldName.clear();
  _startDateStr.clear();
  _startTimeStr.clear();
  _endDateStr.clear();
  _endTimeStr.clear();
  _sweepStartSecs = 0;
  _sweepEndSecs = 0;
  _scale = Radx::missingMetaDouble;
  _offset = Radx::missingMetaDouble;
  _missingDataVal = Radx::missingMetaDouble;
  _lowDataVal = Radx::missingMetaDouble;

  _sweepStatusXml.clear();
  _statusXml.clear();

  _gateGeomVaries = false;
  _extendedMetaDataOnWrite = false;

}

/////////////////////////////////////////////////////////
// Check if specified file is CfRadial format
// Returns true if supported, false otherwise

bool OdimHdf5RadxFile::isSupported(const string &path)

{
  
  if (isOdimHdf5(path)) {
    return true;
  }
  return false;

}

////////////////////////////////////////////////////////////
// Check if this is a OdimHdf5 file
// Returns true on success, false on failure

bool OdimHdf5RadxFile::isOdimHdf5(const string &path)
  
{

  clear();
  
  if (!H5File::isHdf5(path)) {
    if (_verbose) {
      cerr << "DEBUG - not OdimHdf5 file" << endl;
    }
    return false;
  }

  // suppress automatic exception printing so we can handle
  // errors appropriately

  H5::Exception::dontPrint();

  // open file

  H5File file(path, H5F_ACC_RDONLY);

  // check for how group

  Group *how = NULL;
  try {
    how = new Group(file.openGroup("how"));
  }
  catch (H5::Exception e) {
    if (_verbose) {
      cerr << "No 'how' group, not ODIM file" << endl;
    }
    if (how) delete how;
    return false;
  }
  delete how;

  // check for what group

  Group *what = NULL;
  try {
    what = new Group(file.openGroup("what"));
  }
  catch (H5::Exception e) {
    if (_verbose) {
      cerr << "No 'what' group, not ODIM file" << endl;
    }
    if (what) delete what;
    return false;
  }
  delete what;

  // check for where group

  Group *where = NULL;
  try {
    where = new Group(file.openGroup("where"));
  }
  catch (H5::Exception e) {
    if (_verbose) {
      cerr << "No 'where' group, not ODIM file" << endl;
    }
    if (where) delete where;
    return false;
  }
  delete where;

  // check for dataset1 group

  Group *dataset1 = NULL;
  try {
    dataset1 = new Group(file.openGroup("dataset1"));
  }
  catch (H5::Exception e) {
    if (_verbose) {
      cerr << "No 'dataset1' group, not ODIM file" << endl;
    }
    if (dataset1) delete dataset1;
    return false;
  }
  delete dataset1;

  // good

  return true;

}

/////////////////////////////////////////////////////////
// Write data from volume to specified directory
//
// If addDaySubDir is true, a  subdir will be
// created with the name dir/yyyymmdd/
//
// If addYearSubDir is true, a subdir will be
// created with the name dir/yyyy/
//
// If both addDaySubDir and addYearSubDir are true,
// the subdir will be dir/yyyy/yyyymmdd/
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs
// Use getDirInUse() for dir written
// Use getPathInUse() for path written

int OdimHdf5RadxFile::writeToDir(const RadxVol &vol,
                                 const string &dir,
                                 bool addDaySubDir,
                                 bool addYearSubDir)
  
{
  
  if (_debug) {
    cerr << "DEBUG - OdimHdf5RadxFile::writeToDir" << endl;
    cerr << "  Writing to dir: " << dir << endl;
  }

  _writePaths.clear();
  _writeDataTimes.clear();
  clearErrStr();

  _dirInUse = dir;

  RadxTime startTime(vol.getStartTimeSecs());
  int startMillisecs = (int) (vol.getStartNanoSecs() / 1.0e6 + 0.5);
  if (startMillisecs > 999) {
    startTime.set(vol.getStartTimeSecs() + 1);
    startMillisecs -= 1000;
  }
  RadxTime endTime(vol.getEndTimeSecs());
  int endMillisecs = (int) (vol.getEndNanoSecs() / 1.0e6 + 0.5);
  if (endMillisecs > 999) {
    endTime.set(vol.getEndTimeSecs() + 1);
    endMillisecs -= 1000;
  }

  RadxTime fileTime = startTime;
  int fileMillisecs = startMillisecs;
  if (_writeFileNameMode == FILENAME_WITH_END_TIME_ONLY) {
    fileTime = endTime;
    fileMillisecs = endMillisecs;
  }

  string outDir(dir);
  if (addYearSubDir) {
    char yearStr[BUFSIZ];
    sprintf(yearStr, "%s%.4d", PATH_SEPARATOR, fileTime.getYear());
    outDir += yearStr;
  }
  if (addDaySubDir) {
    char dayStr[BUFSIZ];
    sprintf(dayStr, "%s%.4d%.2d%.2d", PATH_SEPARATOR,
            fileTime.getYear(), fileTime.getMonth(), fileTime.getDay());
    outDir += dayStr;
  }

  // make sure output subdir exists
  
  if (makeDirRecurse(outDir)) {
    _addErrStr("ERROR - OdimHdf5RadxFile::writeToDir");
    _addErrStr("  Cannot make output dir: ", outDir);
    return -1;
  }
  
  // compute write path

  string writePath = _computeWritePath(vol,
                                       startTime, startMillisecs,
                                       endTime, endMillisecs,
                                       fileTime, fileMillisecs,
                                       outDir);
  
  // do the write

  int iret = writeToPath(vol, writePath);

  if (iret) {
    _addErrStr("ERROR - OdimHdf5RadxFile::writeToDir");
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////
// Write data from volume to specified path
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs
// Use getPathInUse() for path written

int OdimHdf5RadxFile::writeToPath(const RadxVol &vol,
                                  const string &path)
  
{

  clearErrStr();
  _pathInUse = path;
  vol.setPathInUse(_pathInUse);
  _writePaths.clear();
  _writeDataTimes.clear();
  _gateGeomVaries = vol.gateGeomVariesByRay();
  _sweepMode = vol.getPredomSweepModeFromAngles();

  // open the tmp output file
  
  string tmpPath = tmpPathFromFilePath(path, "");

  if (_debug) {
    cerr << "DEBUG - OdimHdf5RadxFile::writeToPath" << endl;
    cerr << "  Writing to path: " << path << endl;
    cerr << "  Tmp path is: " << tmpPath << endl;
  }

  try {
    if (_doWrite(vol, tmpPath)) {
      _addErrStr("ERROR - OdimHdf5RadxFile::writeToPath");
      _addErrStr("  Cannot write to tmp path: ", tmpPath);
      _closeFile();
      return -1;
    }
  }
  catch (H5::Exception error) {
    _addErrStr("ERROR - OdimHdf5RadxFile::writeToPath");
    _addErrStr("  Error in _doWrite()");
    _addErrStr(error.getDetailMsg());
    _closeFile();
    return -1;
  }

  // rename the tmp to final output file path
  
  if (rename(tmpPath.c_str(), _pathInUse.c_str())) {
    int errNum = errno;
    _addErrStr("ERROR - NcfRadxFile::writeToPath");
    _addErrStr("  Cannot rename tmp file: ", tmpPath);
    _addErrStr("  to: ", _pathInUse);
    _addErrStr(strerror(errNum));
    return -1;
  }
  
  if (_debug) {
    cerr << "DEBUG - NcfRadxFile::writeToPath" << endl;
    cerr << "  Renamed tmp path: " << tmpPath << endl;
    cerr << "     to final path: " << path << endl;
  }

  _writePaths.push_back(path);
  _writeDataTimes.push_back(vol.getStartTimeSecs());
  
  return 0;

}

/////////////////////////////////////////////////////////
// print summary after read

void OdimHdf5RadxFile::print(ostream &out) const
  
{
  
  out << "=============== OdimHdf5RadxFile ===============" << endl;
  RadxFile::print(out);

  out << "  volumeNumber: " << _volumeNumber << endl;
  out << "  instrumentType: " 
      << Radx::instrumentTypeToStr(_instrumentType) << endl;
  out << "  platformType: " << Radx::platformTypeToStr(_platformType) << endl;
  out << "  primaryAxis: " << Radx::primaryAxisToStr(_primaryAxis) << endl;
  out << "  nSweeps: " << _nSweeps << endl;
  out << "  conventions: " << _conventions << endl;
  out << "  objectStr: " << _objectStr << endl;
  out << "  version: " << _version << endl;
  out << "  dateStr: " << _dateStr << endl;
  out << "  timeStr: " << _timeStr << endl;
  out << "  source: " << _source << endl;
  out << "  latitudeDeg: " << _latitudeDeg << endl;
  out << "  longitudeDeg: " << _longitudeDeg << endl;
  out << "  altitudeKm: " << _altitudeKm << endl;
  out << "  task: " << _task << endl;
  out << "  system: " << _system << endl;
  out << "  simulated: " << _simulated << endl;
  out << "  software: " << _software << endl;
  out << "  swVersion: " << _swVersion << endl;
  out << "  wavelengthM: " << _wavelengthM << endl;
  out << "  frequencyHz: " << _frequencyHz << endl;
  out << "  scanRateRpm: " << _scanRateRpm << endl;
  out << "  scanRateDegPerSec: " << _scanRateDegPerSec << endl;
  out << "  pulseWidthUs: " << _pulseWidthUs << endl;
  out << "  rxBandwidthMhz: " << _rxBandwidthMhz << endl;
  out << "  lowPrfHz: " << _lowPrfHz << endl;
  out << "  highPrfHz: " << _highPrfHz << endl;
  out << "  prtMode: " << Radx::prtModeToStr(_prtMode) << endl;
  out << "  prtRatio: " << _prtRatio << endl;
  out << "  txLossH: " << _txLossH << endl;
  out << "  txLossV: " << _txLossV << endl;
  out << "  injectLossH: " << _injectLossH << endl;
  out << "  injectLossV: " << _injectLossV << endl;
  out << "  rxLossH: " << _rxLossH << endl;
  out << "  rxLossV: " << _rxLossV << endl;
  out << "  radomeLossOneWayH: " << _radomeLossOneWayH << endl;
  out << "  radomeLossOneWayV: " << _radomeLossOneWayV << endl;
  out << "  antennaGainH: " << _antennaGainH << endl;
  out << "  antennaGainV: " << _antennaGainV << endl;
  out << "  beamWidthH: " << _beamWidthH << endl;
  out << "  beamWidthV: " << _beamWidthV << endl;
  out << "  radarConstantH: " << _radarConstantH << endl;
  out << "  radarConstantV: " << _radarConstantV << endl;
  out << "  dbz0H: " << _dbz0H << endl;
  out << "  dbz0V: " << _dbz0V << endl;
  out << "  noiseH: " << _noiseH << endl;
  out << "  noiseV: " << _noiseV << endl;
  out << "  gasAttenDbPerKm: " << _gasAttenDbPerKm << endl;
  out << "  nomTxPowerKw: " << _nomTxPowerKw << endl;
  out << "  unambigVelMps: " << _unambigVelMps << endl;
  out << "  powerDiff: " << _powerDiff << endl;
  out << "  phaseDiff: " << _phaseDiff << endl;
  out << "  nSamples: " << _nSamples << endl;
  out << "  azMethod: " << _azMethod << endl;
  out << "  binMethod: " << _binMethod << endl;
  out << "  pointAccEl: " << _pointAccEl << endl;
  out << "  pointAccAz: " << _pointAccAz << endl;
  out << "  malfuncFlag: " << (_malfuncFlag?"Y":"N") << endl;
  out << "  malfuncMsg: " << _malfuncMsg << endl;
  out << "  maxRangeKm: " << _maxRangeKm << endl;
  out << "  comment: " << _comment << endl;
  out << "  sqiThreshold: " << _sqiThreshold << endl;
  out << "  csrThreshold: " << _csrThreshold << endl;
  out << "  logThreshold: " << _logThreshold << endl;
  out << "  snrThreshold: " << _snrThreshold << endl;
  out << "  peakPowerKw: " << _peakPowerKw << endl;
  out << "  avPowerKw: " << _avPowerKw << endl;
  out << "  dynRangeDb: " << _dynRangeDb << endl;
  out << "  polarization: " << _polarization << endl;
  out << "  nDataFields: " << _nDataFields << endl;
  out << "  nQualityFields: " << _nQualityFields << endl;
  out << "  nFields: " << _nFields << endl;
  out << "  sweepMode: " << Radx::sweepModeToStr(_sweepMode) << endl;
  out << "  a1Gate: " << _a1Gate << endl;
  out << "  aStart: " << _aStart << endl;
  out << "  fixedAngleDeg: " << _fixedAngleDeg << endl;
  out << "  nGates: " << _nGates << endl;
  out << "  nRaysSweep: " << _nRaysSweep << endl;
  out << "  startRangeKm: " << _startRangeKm << endl;
  out << "  gateSpacingKm: " << _gateSpacingKm << endl;
  out << "  product: " << _product << endl;
  out << "  quantity: " << _fieldName << endl;
  out << "  startDateStr: " << _startDateStr << endl;
  out << "  startTimeStr: " << _startTimeStr << endl;
  out << "  endDateStr: " << _endDateStr << endl;
  out << "  endTimeStr: " << _endTimeStr << endl;
  out << "  sweepStartSecs: " << _sweepStartSecs << endl;
  out << "  sweepEndSecs: " << _sweepEndSecs << endl;
  out << "  scale: " << _scale << endl;
  out << "  offset: " << _offset << endl;
  out << "  missingDataVal: " << _missingDataVal << endl;
  out << "  lowDataVal: " << _lowDataVal << endl;
  out << "  sweepStatusXml: " << _sweepStatusXml << endl;
  out << "  statusXml: " << _statusXml << endl;
  
  out << "===========================================" << endl;

}

////////////////////////////////////////////////////////////
// Print native data in uf file
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int OdimHdf5RadxFile::printNative(const string &path, ostream &out,
                                  bool printRays, bool printData)
  
{

  if (!H5File::isHdf5(path)) {
    return false;
  }
  
  // open file
  
  H5File file(path, H5F_ACC_RDONLY);
  
  out << "Printing ODIM HDF5 contents" << endl;
  out << "  file path: " << file.getFileName() << endl;
  out << "  file size: " << file.getFileSize() << endl;
  
  try {
    Group root(file.openGroup("/"));
    _utils.printGroup(root, "/", out, printRays, printData);
  }
  catch (H5::Exception e) {
    _addErrStr("ERROR - trying to read ODIM HDF5 file");
    _addErrStr(e.getDetailMsg());
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

int OdimHdf5RadxFile::readFromPath(const string &path,
                                   RadxVol &vol)
  
{

  if (_debug) {
    cerr << "Reading ODIM HDF5 file, master path: " << path << endl;
  }

  _initForRead(path, vol);
  _volumeNumber++;

  // is this a Gematronik file? - each field is stored in a separate file

  string dateStr, fieldName, volName;
  if (_isGematronikFieldFile(path, dateStr, fieldName, volName)) {
    return _readGemFieldFiles(path, dateStr, fieldName, vol);
  }

  // not gematronik - read single file

  return _readFromPath(path, vol);

}

////////////////////////////////////////////////////////////
// Read in data from specified path, load up volume object.
//
// Returns 0 on success, -1 on failure

int OdimHdf5RadxFile::_readFromPath(const string &path,
                                    RadxVol &vol)
  
{

  if (_debug) {
    cerr << "_readFromPath, reading ODIM HDF5 file, path: " << path << endl;
  }

  // initialize status XML

  _statusXml.clear();
  _statusXml += RadxXml::writeStartTag("Status", 0);
  
  string errStr("ERROR - OdimHdf5RadxFile::readFromPath");
  if (!H5File::isHdf5(path)) {
    _addErrStr("ERROR - not a ODIM HDF5 file");
    return -1;
  }
  
  // use try block to catch any exceptions
  
  try {

    // open file
    
    H5File file(path, H5F_ACC_RDONLY);
    if (_debug) {
      cerr << "  file size: " << file.getFileSize() << endl;
    }
    
    // get the root group
    
    Group root(file.openGroup("/"));

    // root attributes

    Hdf5xx::DecodedAttr decodedAttr;
    _utils.loadAttribute(root, "Conventions", "root-attr", decodedAttr);
    _conventions = decodedAttr.getAsString();

    // set the number of sweeps

    if (_getNSweeps(root)) {
      _addErrStr("ERROR - OdimHdf5RadxFile::readFromPath");
      _addErrStr("  path: ", path);
      return -1;
    }

    // read the root how, what and where groups
    
    if (_readRootSubGroups(root)) {
      _addErrStr("ERROR - OdimHdf5RadxFile::readFromPath");
      _addErrStr("  path: ", path);
      return -1;
    }

    // read the sweeps
    
    for (int isweep = 0; isweep < _nSweeps; isweep++) {
      if (_readSweep(root, isweep)) {
        return -1;
      }
      _statusXml += _sweepStatusXml;
    }

  }

  catch (H5::Exception e) {
    _addErrStr("ERROR - reading ODIM HDF5 file");
    _addErrStr(e.getDetailMsg());
    return -1;
  }

  // finalize status xml

  _setStatusXml();
  _statusXml += RadxXml::writeEndTag("Status", 0);

  // append to read paths
  
  _readPaths.push_back(path);

  // load the data into the read volume
  
  if (_finalizeReadVolume()) {
    return -1;
  }
  
  // set format as read

  _fileFormat = FILE_FORMAT_ODIM_HDF5;

  return 0;

}

//////////////////////////////////////////////
// get the number of sweeps in the vol

int OdimHdf5RadxFile::_getNSweeps(Group &root)

{
  
  // init

  _nSweeps = 0;

  // look through all objects, counting up the data sets
  
  for (size_t ii = 0; ii <= root.getNumObjs(); ii++) {
    
    char datasetName[128];
    sprintf(datasetName, "dataset%d", (int) ii);
    
    Group *ds = NULL;
    try {
      ds = new Group(root.openGroup(datasetName));
    }
    catch (H5::Exception e) {
      // data set does not exist
      if (ds) delete ds;
      continue;
    }
    _nSweeps = ii;
    delete ds;
    
  }

  if (_nSweeps == 0) {
    _addErrStr("ERROR - no sweeps found");
    return -1;
  }

  if (_debug) {
    cerr << "  _nSweeps: " << _nSweeps << endl;
  }

  return 0;

}

//////////////////////////////////////////////
// get the number of fields in a sweep

int OdimHdf5RadxFile::_getNFields(Group &sweep)
  
{
  
  // init

  _nDataFields = 0;
  _nQualityFields = 0;
  _nFields = 0;

  // look through all objects, counting up the data sets
  
  for (size_t ii = 0; ii <= sweep.getNumObjs(); ii++) {
    char dataName[128];
    sprintf(dataName, "data%d", (int) ii);
    Group *data = NULL;
    try {
      data = new Group(sweep.openGroup(dataName));
    }
    catch (H5::Exception e) {
      // data set does not exist
      if (data) delete data;
      continue;
    }
    _nDataFields = ii;
    delete data;
  }

  for (size_t ii = 0; ii <= sweep.getNumObjs(); ii++) {
    char qualityName[128];
    sprintf(qualityName, "quality%d", (int) ii);
    Group *quality = NULL;
    try {
      quality = new Group(sweep.openGroup(qualityName));
    }
    catch (H5::Exception e) {
      // quality set does not exist
      if (quality) delete quality;
      continue;
    }
    _nQualityFields = ii;
    delete quality;
  }

  _nFields = _nDataFields + _nQualityFields;

  if (_nFields == 0) {
    _addErrStr("ERROR - no fields found");
    return -1;
  }
  
  if (_debug) {
    cerr << "  _nFields: " << _nFields << endl;
  }

  return 0;
  
}

//////////////////////////////////////////////
// clear the sweep variables

void OdimHdf5RadxFile::_clearSweepVars()

{

  _sweepStatusXml.clear();
  _fixedAngleDeg = Radx::missingMetaDouble;
  _malfuncFlag = false;
  _malfuncMsg.clear();
  _nGates = 0;
  _nRaysSweep = 0;
  _nFields = 0;

}

//////////////////////////////////////////////
// read sweep

int OdimHdf5RadxFile::_readSweep(Group &root, int sweepNumber)

{
  
  // clear
  
  _clearSweepVars();
  
  // compute dataset name: dataset1, dataset2 etc ...

  char sweepName[128];
  sprintf(sweepName, "dataset%d", sweepNumber + 1);

  if (_debug) {
    cerr << "===== reading sweep " << sweepNumber << " "
         << "group: " << sweepName
         << " =====" << endl;
  }
  
  // open scan group
  
  Group sweep(root.openGroup(sweepName));
  
  // read sweep what group
  
  Group *what = NULL;
  try {
    char label[128];
    sprintf(label, "%s what", sweepName);
    what = new Group(sweep.openGroup("what"));
    if (_readSweepWhat(*what, label)) {
      delete what;
      return -1;
    }
  }
  catch (H5::Exception e) {
    if (_debug) {
      cerr << "NOTE - no 'what' group for sweep: " << sweepName << endl;
    }
  }
  if (what) delete what;
  
  // read sweep where group
  
  Group *where = NULL;
  try {
    char label[128];
    sprintf(label, "%s where", sweepName);
    where = new Group(sweep.openGroup("where"));
    if (_readSweepWhere(*where, label)) {
      delete where;
      return -1;
    }
  }
  catch (H5::Exception e) {
    if (_debug) {
      cerr << "NOTE - no 'where' group for sweep: " << sweepName << endl;
    }
  }
  if (where) delete where;

  // read sweep how group
  
  Group *how = NULL;
  try {
    how = new Group(sweep.openGroup("how"));
    char label[128];
    sprintf(label, "%s how", sweepName);
    if (_readSweepHow(*how, label)) {
      return -1;
    }
  }
  catch (H5::Exception e) {
    if (_verbose) {
      cerr << "NOTE - no 'how' group for sweep: " << sweepName << endl;
    }
  }
  if (how) delete how;
  
  // check if this sweep is required
  
  if (_readFixedAngleLimitsSet && _readStrictAngleLimits) {
    if (_fixedAngleDeg < _readMinFixedAngle ||
        _fixedAngleDeg > _readMaxFixedAngle) {
      _clearSweepVars();
      return 0;
    }
  } else if (_readSweepNumLimitsSet && _readStrictAngleLimits) {
    if (sweepNumber < _readMinSweepNum ||
        sweepNumber > _readMaxSweepNum) {
      _clearSweepVars();
      return 0;
    }
  }

  // create rays for this sweep

  _createRaysForSweep(sweepNumber);
  
  // determine number of fields
  
  if (_getNFields(sweep)) {
    _addErrInt("ERROR - no fields in sweep: ", sweepNumber);
    return -1;
  }

  // add data fields to rays
  
  for (int ifield = 0; ifield < _nDataFields; ifield++) {
    if (_addFieldToRays("data", sweep, _sweepRays, ifield)) {
      // free up rays from array
      for (size_t ii = 0; ii < _sweepRays.size(); ii++) {
        delete _sweepRays[ii];
      }
      _sweepRays.clear();
      return -1;
    }
  } // ifield
  
  // add quality fields to rays
  
  for (int ifield = 0; ifield < _nQualityFields; ifield++) {
    if (_addFieldToRays("quality", sweep, _sweepRays, ifield)) {
      // free up rays from array
      for (size_t ii = 0; ii < _sweepRays.size(); ii++) {
        delete _sweepRays[ii];
      }
      _sweepRays.clear();
      return -1;
    }
  } // ifield
  
  // add rays to vol in time order
  
  for (size_t ii = 0; ii < _sweepRays.size(); ii++) {
    // first ray index is a1gate
    int jj = (ii + _a1Gate) % _nRaysSweep;
    _readVol->addRay(_sweepRays[jj]);
  }
  _sweepRays.clear();

  return 0;

}

//////////////////////////////////////////////
// set status xml

void OdimHdf5RadxFile::_setStatusXml()

{
  
  _statusXml += RadxXml::writeDouble("pointAccEl", 1, _pointAccEl);
  _statusXml += RadxXml::writeDouble("pointAccAz", 1, _pointAccAz);
  _statusXml += RadxXml::writeString("azMethod", 1, _azMethod);
  _statusXml += RadxXml::writeString("binMethod", 1, _binMethod);
  _statusXml += RadxXml::writeString("polarization", 1, _polarization);
  _statusXml += RadxXml::writeDouble("sqiThreshold", 1, _sqiThreshold);
  _statusXml += RadxXml::writeDouble("csrThreshold", 1, _csrThreshold);
  _statusXml += RadxXml::writeDouble("logThreshold", 1, _logThreshold);
  _statusXml += RadxXml::writeDouble("snrThreshold", 1, _snrThreshold);
  _statusXml += RadxXml::writeDouble("peakPowerKw", 1, _peakPowerKw);
  _statusXml += RadxXml::writeDouble("avPowerKw", 1, _avPowerKw);
  _statusXml += RadxXml::writeDouble("dynRangeDb", 1, _dynRangeDb);
  
}
  
//////////////////////////////////////////////
// set sweep status xml

void OdimHdf5RadxFile::_setSweepStatusXml(int sweepNum)

{
  
  // initialize

  char tag[128];
  sprintf(tag, "SweepStatus_%d", sweepNum);
  _sweepStatusXml += RadxXml::writeStartTag(tag, 1);

  _sweepStatusXml += RadxXml::writeInt("a1Gate", 2, _a1Gate);
  _sweepStatusXml += RadxXml::writeDouble("maxRangeKm", 2, _maxRangeKm);
  if (_simulated.size() > 0) {
    _sweepStatusXml += RadxXml::writeString("simulated", 2, _simulated);
  }
  _sweepStatusXml += RadxXml::writeString("startTime", 2, _startDateStr + _startTimeStr);
  _sweepStatusXml += RadxXml::writeString("endTime", 2, _endDateStr + _endTimeStr);
  
  // finalize XML
  
  _sweepStatusXml += RadxXml::writeEndTag(tag, 1);

  if (_debug) {
    cerr << "========= sweep status XML ===============" << endl;
    cerr << _sweepStatusXml;
    cerr << "==========================================" << endl;
  }

}
  
//////////////////////////////////////////////
// read the root how, where and what groups

int OdimHdf5RadxFile::_readRootSubGroups(Group &root)

{

  Group what(root.openGroup("what"));
  if (_readRootWhat(what)) {
    return -1;
  }

  Group where(root.openGroup("where"));
  if (_readRootWhere(where)) {
    return -1;
  }

  Group how(root.openGroup("how"));
  _readHow(how, "root how");

  return 0;

}

//////////////////////////////////////////////
// read the root what group

int OdimHdf5RadxFile::_readRootWhat(Group &what)

{

  Hdf5xx::DecodedAttr decodedAttr;

  if (_utils.loadAttribute(what, "object", "root-what", decodedAttr)) {
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  _objectStr = decodedAttr.getAsString();

  if (_objectStr != "PVOL" && _objectStr != "SCAN" &&
      _objectStr != "AZIM" && _objectStr != "ELEV") {
    _addErrStr("Bad object type: ", _objectStr);
    _addErrStr("  Must be 'PVOL','SCAN','AZIM'or'ELEV'");
    return -1;
  }

  _utils.loadAttribute(what, "version", "root-what", decodedAttr);
  _version = decodedAttr.getAsString();
  
  _utils.loadAttribute(what, "date", "root-what", decodedAttr);
  _dateStr = decodedAttr.getAsString();
  
  _utils.loadAttribute(what, "time", "root-what", decodedAttr);
  _timeStr = decodedAttr.getAsString();
  
  _utils.loadAttribute(what, "source", "root-what", decodedAttr);
  _source = decodedAttr.getAsString();
  
  if (_debug) {
    cerr  << "  root what _objectStr: " << _objectStr << endl;
    cerr  << "  root what _version: " << _version << endl;
    cerr  << "  root what _dateStr: " << _dateStr << endl;
    cerr  << "  root what _timeStr: " << _timeStr << endl;
    cerr  << "  root what _source: " << _source << endl;
  }

  return 0;

}

//////////////////////////////////////////////
// read the root where group

int OdimHdf5RadxFile::_readRootWhere(Group &where)

{

  Hdf5xx::DecodedAttr decodedAttr;
  
  if (_utils.loadAttribute(where, "height", "root-where", decodedAttr)) {
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  _altitudeKm = decodedAttr.getAsDouble() / 1000.0;
  
  if (_utils.loadAttribute(where, "lat", "root-where", decodedAttr)) {
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  _latitudeDeg = decodedAttr.getAsDouble();
  
  if (_utils.loadAttribute(where, "lon", "root-where", decodedAttr)) {
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  _longitudeDeg = decodedAttr.getAsDouble();
  
  if (_debug) {
    cerr  << "  root where _altitudeKm: " << _altitudeKm << endl;
    cerr  << "  root where _latitudeDeg: " << _latitudeDeg << endl;
    cerr  << "  root where _longitudeDeg: " << _longitudeDeg << endl;
  }

  return 0;

}

//////////////////////////////////////////////
// read the how group

void OdimHdf5RadxFile::_readHow(Group &how, const string &label)

{

  Hdf5xx::DecodedAttr decodedAttr;

  if (_utils.loadAttribute(how, "task", label, decodedAttr) == 0) {
    _task = decodedAttr.getAsString();
    if (_debug) {
      cerr << "  " << label << " _task: "
           << _task << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "system", label, decodedAttr) == 0) {
    _system = decodedAttr.getAsString();
    if (_debug) {
      cerr << "  " << label << " _system: "
           << _system << endl;
    }
  }

  if (_utils.loadAttribute(how, "simulated", label, decodedAttr) == 0) {
    _simulated = decodedAttr.getAsString();
    if (_debug) {
      cerr << "  " << label << " _simulated: "
           << _simulated << endl;
    }
  }

  if(_utils.loadAttribute(how, "software", label, decodedAttr) == 0) {
    _software = decodedAttr.getAsString();
    if (_debug) {
      cerr << "  " << label << " _software: "
           << _software << endl;
    }
  }

  if (_utils.loadAttribute(how, "sw_version", label, decodedAttr) == 0) {
    _swVersion = decodedAttr.getAsString();
    if (_debug) {
      cerr << "  " << label << " _swVersion: "
           << _swVersion << endl;
    }
  }

  // polarization mode

  _polTypeStr.clear();
  _polModeStr.clear();
  _polMode = Radx::POL_MODE_NOT_SET;
  if (_utils.loadAttribute(how, "poltype", label, decodedAttr) == 0) {
    _polTypeStr = decodedAttr.getAsString();
    if (_utils.loadAttribute(how, "polmode", label, decodedAttr) == 0) {
      _polModeStr = decodedAttr.getAsString();
    }
  }
  if (_polTypeStr == "single") {
    if (_polModeStr == "single-H") {
      _polMode = Radx::POL_MODE_HORIZONTAL;
    } else if (_polModeStr == "single-V") {
      _polMode = Radx::POL_MODE_VERTICAL;
    } else if (_polModeStr == "single-C") {
      _polMode = Radx::POL_MODE_CIRCULAR;
    } else if (_polModeStr == "LDR-H") {
      _polMode = Radx::POL_MODE_HV_H_XMIT;
    } else if (_polModeStr == "LDR-V") {
      _polMode = Radx::POL_MODE_HV_V_XMIT;
    }
  } else if (_polTypeStr == "switched-dual") {
    _polMode = Radx::POL_MODE_HV_ALT;
  } else if (_polTypeStr == "simultaneous-dual") {
    _polMode = Radx::POL_MODE_HV_SIM;
  }

  // beam width

  if (_utils.loadAttribute(how, "beamwidth", label, decodedAttr) == 0) {
    _beamWidthH = decodedAttr.getAsDouble();
    _beamWidthV = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _beamWidthH: " << _beamWidthH << endl;
      cerr << "  " << label << " _beamWidthV: " << _beamWidthV << endl;
    }
  }
  if (_utils.loadAttribute(how, "beamwH", label, decodedAttr) == 0) {
    _beamWidthH = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _beamWidthH: " << _beamWidthH << endl;
    }
  }
  if (_utils.loadAttribute(how, "beamwV", label, decodedAttr) == 0) {
    _beamWidthV = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _beamWidthV: " << _beamWidthV << endl;
    }
  }

  // antenna speed

  if (_utils.loadAttribute(how, "rpm", label, decodedAttr) == 0 ||
      _utils.loadAttribute(how, "elevspeed", label, decodedAttr) == 0) {
    _scanRateRpm = decodedAttr.getAsDouble();
    _scanRateDegPerSec = _scanRateRpm * 6.0;
    if (_debug) {
      cerr << "  " << label << " _scanRateRpm: " << _scanRateRpm << endl;
      cerr << "  " << label << " _scanRateDegPerSec: " << _scanRateDegPerSec << endl;
    }
  }

  // wavelength

  if (_utils.loadAttribute(how, "wavelength", label, decodedAttr) == 0) {
    _wavelengthM = decodedAttr.getAsDouble() / 100.0;
    _frequencyHz = Radx::LIGHT_SPEED / _wavelengthM;
    if (_debug) {
      cerr << "  " << label << " _wavelengthM: " << _wavelengthM << endl;
      cerr << "  " << label << " _frequencyHz: " << _frequencyHz << endl;
    }
  }

  // pulsewidth

  if (_utils.loadAttribute(how, "pulsewidth", label, decodedAttr) == 0) {
    _pulseWidthUs = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _pulseWidthUs: " << _pulseWidthUs << endl;
    }
  }

  // receiver bandwidth

  if (_utils.loadAttribute(how, "RXbandwidth", label, decodedAttr) == 0) {
    _rxBandwidthMhz = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _rxBandwidthMhz: " << _rxBandwidthMhz << endl;
    }
  }

  // prt

  if (_utils.loadAttribute(how, "lowprf", label, decodedAttr) == 0) {
    _lowPrfHz = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _lowPrfHz: " << _lowPrfHz << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "highprf", label, decodedAttr) == 0) {
    _highPrfHz = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _highPrfHz: " << _highPrfHz << endl;
    }
  }

  if (_lowPrfHz != _highPrfHz) {
    _prtMode = Radx::PRT_MODE_STAGGERED;
  } else {
    _prtMode = Radx::PRT_MODE_FIXED;
  }
  if (_debug) {
    cerr << "  " << label << " _prtMode: " << Radx::prtModeToStr(_prtMode) << endl;
  }

  // calibration

  if (_utils.loadAttribute(how, "TXlossH", label, decodedAttr) == 0) {
    _txLossH = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _txLossH: " << _txLossH << endl;
    }
  }
  if (_utils.loadAttribute(how, "TXlossV", label, decodedAttr) == 0) {
    _txLossV = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _txLossV: " << _txLossV << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "injectlossH", label, decodedAttr) == 0) {
    _injectLossH = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _injectLossH: " << _injectLossH << endl;
    }
  }
  if (_utils.loadAttribute(how, "injectlossV", label, decodedAttr) == 0) {
    _injectLossV = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _injectLossV: " << _injectLossV << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "RXlossH", label, decodedAttr) == 0) {
    _rxLossH = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _rxLossH: " << _rxLossH << endl;
    }
  }
  if (_utils.loadAttribute(how, "RXlossV", label, decodedAttr) == 0) {
    _rxLossV = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _rxLossV: " << _rxLossV << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "radomelossH", label, decodedAttr) == 0) {
    _radomeLossOneWayH = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _radomeLossOneWayH: " << _radomeLossOneWayH << endl;
    }
  }
  if (_utils.loadAttribute(how, "radomelossV", label, decodedAttr) == 0) {
    _radomeLossOneWayV = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _radomeLossOneWayV: " << _radomeLossOneWayV << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "nomAntgainH", label, decodedAttr) == 0) {
    _antennaGainH = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _antennaGainH: " << _antennaGainH << endl;
    }
  }
  if (_utils.loadAttribute(how, "nomAntgainV", label, decodedAttr) == 0) {
    _antennaGainV = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _antennaGainV: " << _antennaGainV << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "gasattn", label, decodedAttr) == 0) {
    _gasAttenDbPerKm = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _gasAttenDbPerKm: " << _gasAttenDbPerKm << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "radConstH", label, decodedAttr) == 0) {
    _radarConstantH = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _radarConstantH: " << _radarConstantH << endl;
    }
  }
  if (_utils.loadAttribute(how, "radConstV", label, decodedAttr) == 0) {
    _radarConstantV = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _radarConstantV: " << _radarConstantV << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "nomTXpower", label, decodedAttr) == 0) {
    _nomTxPowerKw = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _nomTxPowerKw: " << _nomTxPowerKw << endl;
    }
  }

  if (_utils.loadAttribute(how, "powerdiff", label, decodedAttr) == 0) {
    _powerDiff = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _powerDiff: " << _powerDiff << endl;
    }
  }
  if (_utils.loadAttribute(how, "phasediff", label, decodedAttr) == 0) {
    _phaseDiff = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _phaseDiff: " << _phaseDiff << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "NI", label, decodedAttr) == 0) {
    _unambigVelMps = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _unambigVelMps: " << _unambigVelMps << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "Vsamples", label, decodedAttr) == 0) {
    _nSamples = decodedAttr.getAsInt();
    if (_debug) {
      cerr << "  " << label << " _nSamples: " << _nSamples << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "azmethod", label, decodedAttr) == 0) {
    _azMethod = decodedAttr.getAsString();
    if (_debug) {
      cerr << "  " << label << " _azMethod: " << _azMethod << endl;
    }
  }

  if (_utils.loadAttribute(how, "binmethod", label, decodedAttr) == 0) {
    _binMethod = decodedAttr.getAsString();
    if (_debug) {
      cerr << "  " << label << " _binMethod: " << _binMethod << endl;
    }
  }

  if (_utils.loadAttribute(how, "pointaccEl", label, decodedAttr) == 0) {
    _pointAccEl = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _pointAccEl: " << _pointAccEl << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "pointaccAz", label, decodedAttr) == 0) {
    _pointAccAz = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _pointAccAz: " << _pointAccAz << endl;
    }
  }
  
  if(_utils.loadAttribute(how, "malfunc", label, decodedAttr)== 0) {
    _malfuncFlag = false;
    string sval = decodedAttr.getAsString();
    if (sval == "True") {
      _malfuncFlag = true;
    }
    if (_debug) {
      cerr << "  " << label << " _malfuncFlag: " << _malfuncFlag << endl;
    }
  }

  if(_utils.loadAttribute(how, "radar_msg", label, decodedAttr)== 0) {
    _malfuncMsg = decodedAttr.getAsString();
    if (_debug) {
      cerr << "  " << label << " _malfuncMsg: " << _malfuncMsg << endl;
    }
  }
  
  if(_utils.loadAttribute(how, "radhoriz", label, decodedAttr)== 0) {
    _maxRangeKm = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _maxRangeKm: " << _maxRangeKm << endl;
    }
  }

  if(_utils.loadAttribute(how, "NEZH", label, decodedAttr)== 0) {
    _dbz0H = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _dbz0H: " << _dbz0H << endl;
    }
  }
  if(_utils.loadAttribute(how, "NEZV", label, decodedAttr)== 0) {
    _dbz0V = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _dbz0V: " << _dbz0V << endl;
    }
  }

  if(_utils.loadAttribute(how, "nsampleH", label, decodedAttr)== 0) {
    _noiseH = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _noiseH: " << _noiseH << endl;
    }
  }
  if(_utils.loadAttribute(how, "nsampleV", label, decodedAttr)== 0) {
    _noiseV = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _noiseV: " << _noiseV << endl;
    }
  }

  if(_utils.loadAttribute(how, "comment", label, decodedAttr)== 0) {
    _comment = decodedAttr.getAsString();
    if (_debug) {
      cerr << "  " << label << " _comment: "
           << _comment << endl;
    }
  }
  
  if(_utils.loadAttribute(how, "SQI", label, decodedAttr)== 0) {
    _sqiThreshold = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _sqiThreshold: "
           << _sqiThreshold << endl;
    }
  }

  if(_utils.loadAttribute(how, "CSR", label, decodedAttr)== 0) {
    _csrThreshold = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _csrThreshold: "
           << _csrThreshold << endl;
    }
  }

  if(_utils.loadAttribute(how, "LOG", label, decodedAttr)== 0) {
    _logThreshold = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _logThreshold: "
           << _logThreshold << endl;
    }
  }

  if(_utils.loadAttribute(how, "S2N", label, decodedAttr)== 0) {
    _snrThreshold = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _snrThreshold: "
           << _snrThreshold << endl;
    }
  }

  if(_utils.loadAttribute(how, "peakpwr", label, decodedAttr)== 0) {
    _peakPowerKw = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _peakPowerKw: "
           << _peakPowerKw << endl;
    }
  }

  if(_utils.loadAttribute(how, "avgpwr", label, decodedAttr)== 0) {
    _avPowerKw = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _avPowerKm: "
           << _avPowerKw << endl;
    }
  }

  if(_utils.loadAttribute(how, "dynrange", label, decodedAttr)== 0) {
    _dynRangeDb = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _dynRangeDb: "
           << _dynRangeDb << endl;
    }
  }

  if(_utils.loadAttribute(how, "polarization", label, decodedAttr)== 0) {
    _polarization = decodedAttr.getAsString();
    if (_debug) {
      cerr << "  " << label << " _polarization: "
           << _polarization << endl;
    }
  }

}

//////////////////////////////////////////////
// read the required how group for a sweep

int OdimHdf5RadxFile::_readSweepHow(Group &how, const string &label)

{

  // read the scalar how attributes

  _readHow(how, label);

  double sweepStartSecs = _sweepStartSecs;
  double sweepEndSecs = _sweepEndSecs;
  double sweepDeltaSecs = sweepEndSecs - sweepStartSecs;
  double rayDeltaSecs = sweepDeltaSecs / (double) (_nRaysSweep - 1);

  _aStart = 0;
  Hdf5xx::DecodedAttr decodedAttr;
  if (_utils.loadAttribute(how, "astart", label, decodedAttr) == 0) {
    _aStart = decodedAttr.getAsDouble();
  }

  for (int ii = 0; ii < _nRaysSweep; ii++) {

    // default angles

    if (_sweepMode == Radx::SWEEP_MODE_RHI) {
      double deltaAngle = 1.0;
      _rayEl[ii] = fmod(ii * deltaAngle, 360.0);
      _rayAz[ii] = _fixedAngleDeg;
    } else {
      double deltaAngle = 360.0 / _nRaysSweep;
      _rayAz[ii] = _aStart + (ii + 0.5) * deltaAngle;
      _rayAz[ii] = Radx::conditionAz(_rayAz[ii]);
      _rayEl[ii] = _fixedAngleDeg;
    }

    // the first ray in the sweep is at index _a1Gate

    int jj = (ii + _a1Gate) % _nRaysSweep;
    _rayTime[jj] = sweepStartSecs + ii * rayDeltaSecs;

  } // ii

  // read in array attributes

  if (_sweepMode == Radx::SWEEP_MODE_RHI) {

    // ray azimuth angles

    Hdf5xx::ArrayAttr rayAz;
    if (_utils.loadArrayAttribute(how, "azangles", label, rayAz) == 0) {
      if ((int) rayAz.getLen() != _nRaysSweep) {
        _addErrStr("ERROR - In reading PPI sweep, incorrect number of azangles");
        _addErrInt("  Found array len: ", rayAz.getLen());
        _addErrInt("  Expected nRays: ", _nRaysSweep);
        return -1;
      }
      const double *azangles = rayAz.getAsDoubles();
      for (int ii = 0; ii < _nRaysSweep; ii++) {
        _rayAz[ii] = azangles[ii];
      }
    }

    // ray elevation angles

    Hdf5xx::ArrayAttr startEl, stopEl;
    if ((_utils.loadArrayAttribute(how, "startelA", label, startEl) == 0) &&
        (_utils.loadArrayAttribute(how, "stopelA", label, stopEl)) == 0) {
      
      if ((int) startEl.getLen() != _nRaysSweep) {
        _addErrStr("ERROR - In reading PPI sweep, incorrect number of startelA");
        _addErrInt("  Found array len: ", startEl.getLen());
        _addErrInt("  Expected nRays: ", _nRaysSweep);
        return -1;
      }
      if ((int) stopEl.getLen() != _nRaysSweep) {
        _addErrStr("ERROR - In reading PPI sweep, incorrect number of stopelA");
        _addErrInt("  Found array len: ", stopEl.getLen());
        _addErrInt("  Expected nRays: ", _nRaysSweep);
        return -1;
      }
      const double *startEls = startEl.getAsDoubles();
      const double *stopEls = stopEl.getAsDoubles();
      for (int ii = 0; ii < _nRaysSweep; ii++) {
        _rayEl[ii] = RadxComplex::computeMeanDeg(startEls[ii], stopEls[ii]);
      }
    }

    // ray times
    
    Hdf5xx::ArrayAttr startTime, stopTime;
    if ((_utils.loadArrayAttribute(how, "startelT", label, startTime) == 0) &&
        (_utils.loadArrayAttribute(how, "stopelT", label, stopTime)) == 0) {
      
      if ((int) startTime.getLen() != _nRaysSweep) {
        _addErrStr("ERROR - In reading PPI sweep, incorrect number of startelT");
        _addErrInt("  Found array len: ", startTime.getLen());
        _addErrInt("  Expected nRays: ", _nRaysSweep);
        return -1;
      }
      if ((int) stopTime.getLen() != _nRaysSweep) {
        _addErrStr("ERROR - In reading PPI sweep, incorrect number of stopelT");
        _addErrInt("  Found array len: ", stopTime.getLen());
        _addErrInt("  Expected nRays: ", _nRaysSweep);
        return -1;
      }
      const double *startTimes = startTime.getAsDoubles();
      const double *stopTimes = stopTime.getAsDoubles();
      for (int ii = 0; ii < _nRaysSweep; ii++) {
        _rayTime[ii] = (startTimes[ii] + stopTimes[ii]) / 2.0;
      }
    }

  } else {

    // ray elevation angles

    Hdf5xx::ArrayAttr rayEl;
    if (_utils.loadArrayAttribute(how, "elangles", label, rayEl) == 0) {
      if ((int) rayEl.getLen() != _nRaysSweep) {
        _addErrStr("ERROR - In reading PPI sweep, incorrect number of elangles");
        _addErrInt("  Found array len: ", rayEl.getLen());
        _addErrInt("  Expected nRays: ", _nRaysSweep);
        return -1;
      }
      const double *elangles = rayEl.getAsDoubles();
      for (int ii = 0; ii < _nRaysSweep; ii++) {
        _rayEl[ii] = elangles[ii];
      }
    }

    // ray azimuth angles

    Hdf5xx::ArrayAttr startAz, stopAz;
    if ((_utils.loadArrayAttribute(how, "startazA", label, startAz) == 0) &&
        (_utils.loadArrayAttribute(how, "stopazA", label, stopAz)) == 0) {
      
      if ((int) startAz.getLen() != _nRaysSweep) {
        _addErrStr("ERROR - In reading PPI sweep, incorrect number of startazA");
        _addErrInt("  Found array len: ", startAz.getLen());
        _addErrInt("  Expected nRays: ", _nRaysSweep);
        return -1;
      }
      if ((int) stopAz.getLen() != _nRaysSweep) {
        _addErrStr("ERROR - In reading PPI sweep, incorrect number of stopazA");
        _addErrInt("  Found array len: ", stopAz.getLen());
        _addErrInt("  Expected nRays: ", _nRaysSweep);
        return -1;
      }
      const double *startAzs = startAz.getAsDoubles();
      const double *stopAzs = stopAz.getAsDoubles();
      for (int ii = 0; ii < _nRaysSweep; ii++) {
        _rayAz[ii] = RadxComplex::computeMeanDeg(startAzs[ii], stopAzs[ii]);
        if (_rayAz[ii] < 0) {
          _rayAz[ii] += 360.0;
        }
      }
    }

    // ray times
    
    Hdf5xx::ArrayAttr startTime, stopTime;
    if ((_utils.loadArrayAttribute(how, "startazT", label, startTime) == 0) &&
        (_utils.loadArrayAttribute(how, "stopazT", label, stopTime)) == 0) {
      
      if ((int) startTime.getLen() != _nRaysSweep) {
        _addErrStr("ERROR - In reading PPI sweep, incorrect number of startazT");
        _addErrInt("  Found array len: ", startTime.getLen());
        _addErrInt("  Expected nRays: ", _nRaysSweep);
        return -1;
      }
      if ((int) stopTime.getLen() != _nRaysSweep) {
        _addErrStr("ERROR - In reading PPI sweep, incorrect number of stopazT");
        _addErrInt("  Found array len: ", stopTime.getLen());
        _addErrInt("  Expected nRays: ", _nRaysSweep);
        return -1;
      }
      const double *startTimes = startTime.getAsDoubles();
      const double *stopTimes = stopTime.getAsDoubles();
      for (int ii = 0; ii < _nRaysSweep; ii++) {
        _rayTime[ii] = (startTimes[ii] + stopTimes[ii]) / 2.0;
      }
    }
    
  }

  return 0;

}

//////////////////////////////////////////////
// read what group for sweep

int OdimHdf5RadxFile::_readSweepWhat(Group &what, const string &label)

{
  
  Hdf5xx::DecodedAttr decodedAttr;

  // product scan type
  
  _product = "SCAN";
  _sweepMode = Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE;
  if (_utils.loadAttribute(what, "product", label, decodedAttr) == 0) {
    _product = decodedAttr.getAsString();
    if (_debug) {
      cerr << "  " << label << " _product: " << _product << endl;
    }
    if (_product == "RHI") {
      _sweepMode = Radx::SWEEP_MODE_RHI;
    } else if (_product == "AZIM") {
      _sweepMode = Radx::SWEEP_MODE_SECTOR;
    }
  }

  // date / time

  if (_utils.loadAttribute(what, "startdate", label, decodedAttr)) {
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  _startDateStr = decodedAttr.getAsString();
  if (_debug) {
    cerr << "  " << label << " _startDateStr: " << _startDateStr << endl;
  }

  if (_utils.loadAttribute(what, "enddate", label, decodedAttr)) {
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  _endDateStr = decodedAttr.getAsString();
  if (_debug) {
    cerr << "  " << label << " _endDateStr: " << _endDateStr << endl;
  }

  if (_utils.loadAttribute(what, "starttime", label, decodedAttr)) {
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  _startTimeStr = decodedAttr.getAsString();
  if (_debug) {
    cerr << "  " << label << " _startTimeStr: " << _startTimeStr << endl;
  }

  if (_utils.loadAttribute(what, "endtime", label, decodedAttr)) {
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  _endTimeStr = decodedAttr.getAsString();
  if (_debug) {
    cerr << "  " << label << " _endTimeStr: " << _endTimeStr << endl;
  }

  RadxTime sweepStartTime(_startDateStr + _startTimeStr);
  RadxTime sweepEndTime(_endDateStr + _endTimeStr);

  _sweepStartSecs = sweepStartTime.utime();
  _sweepEndSecs = sweepEndTime.utime();

  return 0;

}

//////////////////////////////////////////////
// read what group for data field

int OdimHdf5RadxFile::_readDataWhat(Group &what, const string &label)

{

  Hdf5xx::DecodedAttr decodedAttr;

  // field quantity

  if (_utils.loadAttribute(what, "quantity", label, decodedAttr) == 0) {
    _fieldName = decodedAttr.getAsString();
    if (_debug) {
      cerr << "  " << label << " _fieldName: " << _fieldName << endl;
    }
  }

  if (_utils.loadAttribute(what, "gain", label, decodedAttr) == 0) {
    _scale = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _scale: " << _scale << endl;
    }
  }
  
  if (_utils.loadAttribute(what, "offset", label, decodedAttr) == 0) {
    _offset = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _offset: " << _offset << endl;
    }
  }
  
  if (_utils.loadAttribute(what, "nodata", label, decodedAttr) == 0) {
    _missingDataVal = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _missingDataVal: " << _missingDataVal << endl;
    }
  }
  
  if (_utils.loadAttribute(what, "undetect", label, decodedAttr) == 0) {
    _lowDataVal = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _lowDataVal: " << _lowDataVal << endl;
    }
  }
  
  return 0;

}

//////////////////////////////////////////////
// read a sweep where group

int OdimHdf5RadxFile::_readSweepWhere(Group &where, const string& label)

{

  Hdf5xx::DecodedAttr decodedAttr;

  if (_utils.loadAttribute(where, "a1gate", label, decodedAttr)) {
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  _a1Gate = decodedAttr.getAsInt();
  
  if (_utils.loadAttribute(where, "elangle", label, decodedAttr)) {
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  _fixedAngleDeg = decodedAttr.getAsDouble();
  
  if (_utils.loadAttribute(where, "nbins", label, decodedAttr)) {
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  _nGates = decodedAttr.getAsInt();
  
  if (_utils.loadAttribute(where, "nrays", label, decodedAttr)) {
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  _nRaysSweep = decodedAttr.getAsInt();

  // initialize ray vectors
  
  _rayAz.resize(_nRaysSweep);
  _rayEl.resize(_nRaysSweep);
  _rayTime.resize(_nRaysSweep);
  
  if (_utils.loadAttribute(where, "rscale", label, decodedAttr)) {
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  _gateSpacingKm = decodedAttr.getAsDouble() / 1000.0;
  
  if (_utils.loadAttribute(where, "rstart", label, decodedAttr)) {
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  _startRangeKm = decodedAttr.getAsDouble();
  
  if (_debug) {
    cerr << "  " << label << " _a1Gate: " << _a1Gate << endl;
    cerr << "  " << label << " _fixedAngleDeg: " << _fixedAngleDeg << endl;
    cerr << "  " << label << " _nGates: " << _nGates << endl;
    cerr << "  " << label << " _nRaysSweep: " << _nRaysSweep << endl;
    cerr << "  " << label << " _gateSpacingKm: " << _gateSpacingKm << endl;
    cerr << "  " << label << " _startRangeKm: " << _startRangeKm << endl;
  }
  return 0;

}

////////////////////////////////////////
// create the rays for this sweep

void OdimHdf5RadxFile::_createRaysForSweep(int sweepNumber)
{
  
  // create rays for sweep

  for (int  iray = 0; iray < _nRaysSweep; iray++) {

    // time

    time_t raySecs = (time_t) _rayTime[iray];
    int msecs = (int) ((_rayTime[iray] - raySecs) * 1000.0 + 0.5);

    if (_verbose) {
      char rayTimeStr[128];
      sprintf(rayTimeStr, "%s.%.3d", RadxTime::strm(raySecs).c_str(), msecs);
      cerr << "ray iray, time, el, az: " << iray << ", "
           << rayTimeStr << ", "
           << _rayEl[iray] << ", "
           << _rayAz[iray] << endl;
    }

    RadxRay *ray = new RadxRay;
    ray->setTime(_rayTime[iray]);
    ray->setAzimuthDeg(_rayAz[iray]);
    ray->setElevationDeg(_rayEl[iray]);
    ray->setVolumeNumber(_volumeNumber);
    ray->setSweepNumber(sweepNumber);
    ray->setNSamples(_nSamples);
    ray->setPulseWidthUsec(_pulseWidthUs);
    ray->setPrtSec(1.0 / _highPrfHz);
    ray->setPrtRatio(_lowPrfHz / _highPrfHz);
    ray->setNyquistMps(_unambigVelMps);
    ray->setUnambigRangeKm(_maxRangeKm);
    ray->setTargetScanRateDegPerSec(_scanRateDegPerSec);
    ray->setFixedAngleDeg(_fixedAngleDeg);

    ray->setSweepMode(_sweepMode);
    ray->setPrtMode(_prtMode);
    ray->setRangeGeom(_startRangeKm, _gateSpacingKm);

    _sweepRays.push_back(ray);

  } // iray
  
}

//////////////////////////////////////////////////
// add field to rays

int OdimHdf5RadxFile::_addFieldToRays(const char *label,
                                      Group &sweep,
                                      vector<RadxRay *> &rays, 
                                      int fieldNum)

{
  // compute field name
  
  char dataGroupName[1024];
  sprintf(dataGroupName, "%s%d", label, fieldNum + 1);
  
  // get data group
  
  Group *dg = NULL;
  try {
    dg = new Group(sweep.openGroup(dataGroupName));
  }
  catch (H5::Exception e) {
    _addErrStr("ERROR - OdimHdf5RadxFile::_addFieldToRays");
    _addErrStr("  Cannot open data grop");
    _addErrStr("  Data group name: ", dataGroupName);
    _addErrStr(e.getDetailMsg());
    if (dg) delete dg;
    return -1;
  }

  // read what group
  
  Group *what = NULL;
  try {
    char label[128];
    sprintf(label, "%s what", dataGroupName);
    what = new Group(dg->openGroup("what"));
    if (_readDataWhat(*what, label)) {
      delete what;
      delete dg;
      return -1;
    }
  }
  catch (H5::Exception e) {
    if (_debug) {
      cerr << "NOTE - no 'what' group for data field: " << dataGroupName << endl;
    }
  }
  if (what) delete what;
  
  // read sweep how group
  
  Group *how = NULL;
  try {
    how = new Group(dg->openGroup("how"));
    char label[128];
    sprintf(label, "%s how", dataGroupName);
    _readHow(*how, label);
  }
  catch (H5::Exception e) {
    if (_verbose) {
      cerr << "NOTE - no 'how' group for data field: " << dataGroupName << endl;
    }
  }
  if (how) delete how;
  
  // check that we need this field
  
  if (!isFieldRequiredOnRead(_fieldName)) {
    if (_verbose) {
      cerr << "DEBUG - OdimHdf5RadxFile::_addFieldToRays" << endl;
      cerr << "  -->> rejecting field: " << _fieldName << endl;
    }
    delete dg;
    return 0;
  }

  // get units, standard name and long name

  string units, standardName, longName;
  _lookupUnitsAndNames(_fieldName, units, standardName, longName);

  // get data set
  
  DataSet *ds = NULL;
  try {
    ds = new DataSet(dg->openDataSet("data"));
  }
  catch (H5::Exception e) {
    _addErrStr("ERROR - OdimHdf5RadxFile::_addFieldToRays");
    _addErrStr("  Cannot open data set for field: ", _fieldName);
    _addErrStr(e.getDetailMsg());
    if (ds) delete ds;
    delete dg;
    return -1;
  }

  // get data size
  
  DataSpace dataspace = ds->getSpace();
  int nPoints = dataspace.getSimpleExtentNpoints();

  // get dimensions

  int ndims = dataspace.getSimpleExtentNdims();
  if (ndims != 2) {
    _addErrStr("ERROR - OdimHdf5RadxFile::_addFieldToRays");
    _addErrStr("  Field name: ", _fieldName);
    _addErrStr("  Data is not 2-D array");
    _addErrStr("  Should be [nrays][ngates]");
    delete ds;
    delete dg;
    return -1;

  }

  hsize_t dims[2];
  dataspace.getSimpleExtentDims(dims);
  int nRays = dims[0];
  int nGates = dims[1];
  
  if (nRays != (int) rays.size()) {
    _addErrStr("ERROR - OdimHdf5RadxFile::_addFieldToRays");
    _addErrStr("  Data name: ", dataGroupName);
    _addErrStr("  Field name: ", _fieldName);
    _addErrStr("  nRays incorrect, should match nRaysSweep");
    _addErrInt("  Found: ", nRays);
    _addErrInt("  Expected: ", rays.size());
    delete ds;
    delete dg;
    return -1;
  }

  if (nGates != _nGates) {
    _addErrStr("ERROR - OdimHdf5RadxFile::_addFieldToRays");
    _addErrStr("  Data name: ", dataGroupName);
    _addErrStr("  Field name: ", _fieldName);
    _addErrStr("  nGates incorrect, should match bin_count");
    _addErrInt("  nGates: ", nGates);
    _addErrInt("  bin_count: ", _nGates);
    delete ds;
    delete dg;
    return -1;
  }
  
  // get data type and size

  DataType dtype = ds->getDataType();
  H5T_class_t aclass = dtype.getClass();
  size_t tsize = dtype.getSize();
  
  if (aclass == H5T_INTEGER) {

    if (tsize == 1) {

      _loadSi08Field(*ds, _fieldName, units, standardName, longName,
                     nRays, nGates, nPoints,
                     _scale, _offset, rays);
  
    } else if (tsize == 2) {

      _loadSi16Field(*ds, _fieldName, units, standardName, longName,
                     nRays, nGates, nPoints,
                     _scale, _offset, rays);
  
    } else if (tsize == 4) {

      _loadSi32Field(*ds, _fieldName, units, standardName, longName,
                     nRays, nGates, nPoints,
                     _scale, _offset, rays);
  
    } else {
      
      _addErrStr("ERROR - OdimHdf5RadxFile::_addFieldToRays");
      _addErrStr("  Data name: ", dataGroupName);
      _addErrStr("  Field name: ", _fieldName);
      _addErrInt("  integer data size not supported: ", tsize);
      delete ds;
      delete dg;
      return -1;
      
    }

  } else if (aclass == H5T_FLOAT) {

    if (tsize == 4) {

      _loadFl32Field(*ds, _fieldName, units, standardName, longName,
                     nRays, nGates, nPoints, rays);
  
    } else if (tsize == 8) {
      
      _loadFl64Field(*ds, _fieldName, units, standardName, longName,
                     nRays, nGates, nPoints, rays);
  
    } else {

      _addErrStr("ERROR - OdimHdf5RadxFile::_addFieldToRays");
      _addErrStr("  Data name: ", dataGroupName);
      _addErrStr("  Field name: ", _fieldName);
      _addErrInt("  float data size not supported: ", tsize);
      delete ds;
      delete dg;
      return -1;
      
    }

  } else {
    
    _addErrStr("ERROR - OdimHdf5RadxFile::_addFieldToRays");
    _addErrStr("  Data name: ", dataGroupName);
    _addErrStr("  Field name: ", _fieldName);
    _addErrStr("  data type not supported: ", dtype.fromClass());
    delete ds;
    delete dg;
    return -1;
      
  }

  // clean up

  delete ds;
  delete dg;
  return 0;

}

///////////////////////////////////////////////////////////////////
// Load float array for given data set, using the type passed in

int OdimHdf5RadxFile::_loadFloatArray(DataSet &ds,
                                      const string dsname,
                                      int nPoints,
                                      double scale,
                                      double offset,
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

        Radx::ui08 *ivals = new Radx::ui08[nPoints];
        ds.read(ivals, dtype);
        for (int ii = 0; ii < nPoints; ii++) {
          if (ivals[ii] == 0) {
            floatVals[ii] = Radx::missingFl32;
          } else {
            floatVals[ii] = offset + ivals[ii] * scale;
          }
        }
        delete[] ivals;

      } else if (tsize == 2) {

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
            floatVals[ii] = offset + ivals[ii] * scale;
          }
        }
        delete[] ivals;

      } else if (tsize == 4) {

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
            floatVals[ii] = offset + ivals[ii] * scale;
          }
        }
        delete[] ivals;

      } else if (tsize == 8) {

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
            floatVals[ii] = offset + ivals[ii] * scale;
          }
        }
        delete[] ivals;
      }

    } else {

      // signed

      if (tsize == 1) {

        Radx::si08 *ivals = new Radx::si08[nPoints];
        ds.read(ivals, dtype);
        for (int ii = 0; ii < nPoints; ii++) {
          if (ivals[ii] == 0) {
            floatVals[ii] = Radx::missingFl32;
          } else {
            floatVals[ii] = offset + ivals[ii] * scale;
          }
        }
        delete[] ivals;

      } else if (tsize == 2) {

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
            floatVals[ii] = offset + ivals[ii] * scale;
          }
        }
        delete[] ivals;

      } else if (tsize == 4) {

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
            floatVals[ii] = offset + ivals[ii] * scale;
          }
        }
        delete[] ivals;

      } else if (tsize == 8) {

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
            floatVals[ii] = offset + ivals[ii] * scale;
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

void OdimHdf5RadxFile::_loadSi08Field(DataSet &ds,
                                      const string &fieldName,
                                      const string &units,
                                      const string &standardName,
                                      const string &longName,
                                      int nRays,
                                      int nGates,
                                      int nPoints,
                                      double scale,
                                      double offset,
                                      vector<RadxRay *> &rays)
  
{
  
  DataType dtype = ds.getDataType();
  IntType intType = ds.getIntType();
  H5T_sign_t sign = intType.getSign();

  int imin = -128;

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

void OdimHdf5RadxFile::_loadSi16Field(DataSet &ds,
                                      const string &fieldName,
                                      const string &units,
                                      const string &standardName,
                                      const string &longName,
                                      int nRays,
                                      int nGates,
                                      int nPoints,
                                      double scale,
                                      double offset,
                                      vector<RadxRay *> &rays)
  
{
  
  DataType dtype = ds.getDataType();
  IntType intType = ds.getIntType();
  H5T_order_t order = intType.getOrder();
  H5T_sign_t sign = intType.getSign();

  int imin = -32768;

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

void OdimHdf5RadxFile::_loadSi32Field(DataSet &ds,
                                      const string &fieldName,
                                      const string &units,
                                      const string &standardName,
                                      const string &longName,
                                      int nRays,
                                      int nGates,
                                      int nPoints,
                                      double scale,
                                      double offset,
                                      vector<RadxRay *> &rays)
  
{

  DataType dtype = ds.getDataType();
  IntType intType = ds.getIntType();
  H5T_order_t order = intType.getOrder();
  H5T_sign_t sign = intType.getSign();
  
  Radx::si64 imin = -2147483648;

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

void OdimHdf5RadxFile::_loadFl32Field(DataSet &ds,
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

void OdimHdf5RadxFile::_loadFl64Field(DataSet &ds,
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
// lookup units and names appropriate to field name

void OdimHdf5RadxFile::_lookupUnitsAndNames(const string &fieldName, 
                                            string &units,
                                            string &standardName,
                                            string &longName)
  
{

  if (fieldName == "TH") {
    units = "dBZ";
    longName = "Logged horizontally-polarized total (uncorrected) reflectivity factor";
    standardName = "equivalent_reflectivity_factor";
  }

  if (fieldName == "TV") {
    units = "dBZ";
    longName = "Logged vertically-polarized total (uncorrected) reflectivity factor";
    standardName = "equivalent_reflectivity_factor";
  }

  if (fieldName == "DBZH") {
    units = "dBZ";
    longName = "Logged horizontally-polarized (corrected) reflectivity factor";
    standardName = "equivalent_reflectivity_factor";
  }

  if (fieldName == "DBZV") {
    units = "dBZ";
    longName = "Logged vertically-polarized (corrected) reflectivity factor";
    standardName = "equivalent_reflectivity_factor";
  }

  if (fieldName == "ZDR") {
    units = "dB";
    longName = "Logged differential reflectivity";
    standardName = "corrected_log_differential_reflectivity_hv";
  }

  if (fieldName == "RHOHV") {
    units = "0-1 ";
    longName = "Correlation between Zh and Zv";
    standardName = "cross_correlation_ratio_hv";
  }

  if (fieldName == "LDR") {
    units = "dB";
    longName = "Linear depolarization ratio";
    standardName = "log_linear_depolarization_ratio_hv";
  }

  if (fieldName == "PHIDP") {
    units = "deg";
    longName = "Differential phase";
    standardName = "differential_phase_hv";
  }

  if (fieldName == "KDP") {
    units = "deg/km";
    longName = "Specific differential phase";
    standardName = "specific_differential_phase_hv";
  }

  if (fieldName == "SQI") {
    units = "0-1";
    longName = "Signal quality index";
    longName = "normalized_coherent_power";
  }

  if (fieldName == "SNR") {
    units = "0-1";
    longName = "Normalized signal-to-noise ratio";
    standardName = "signal_to_noise_ratio";
  }

  if (fieldName == "VRAD") {
    units = "m/s";
    longName = "Radial velocity";
    standardName = "radial_velocity_of_scatterers_away_from_instrument";
  }

  if (fieldName == "WRAD") {
    units = "m/s";
    longName = "Spectral width of radial velocity";
    standardName = "doppler_spectrum_width";
  }
  
  if (fieldName == "QIND") {
    units = "0-1";
    longName = "Spatially analayzed quality indicator";
  }

}


//////////////////////////////////////////////////////////
// load up the read volume with the data from this object

int OdimHdf5RadxFile::_finalizeReadVolume()
  
{

  // set metadata

  _readVol->setOrigFormat("ODIMHDF5");
  _readVol->setVolumeNumber(_volumeNumber);
  _readVol->setInstrumentType(_instrumentType);
  _readVol->setPlatformType(_platformType);
  _readVol->setPrimaryAxis(_primaryAxis);

  _readVol->addFrequencyHz(_frequencyHz);
  
  _readVol->setTitle("ODIM radar data, version: " + _version);
  _readVol->setSource(_source);
  
  string hist;
  if (_system.size() > 0) {
    hist += "system:" + _system;
  }
  if (_software.size() > 0) {
    if (hist.size() > 0) hist += ", ";
    hist += " software:" + _software;
  }
  if (_swVersion.size() > 0) {
    if (hist.size() > 0) hist += ", ";
    hist += " version:" + _swVersion;
  }
  if (_conventions.size() > 0) {
    if (hist.size() > 0) hist += ", ";
    hist += " hdf5-conventions:" + _conventions;
  }
  _readVol->setHistory(hist);

  _readVol->setInstitution("");

  string ref;
  if (_polarization.size() > 0) {
    ref += "polarization:" + _polarization;
  }
  if (_azMethod.size() > 0) {
    if (ref.size() > 0) ref += ", ";
    ref += "azMethod:" + _azMethod;
  }
  if (_binMethod.size() > 0) {
    if (ref.size() > 0) ref += ", ";
    ref += "binMethod:" + _binMethod;
  }
  if (_malfuncFlag && _malfuncMsg.size() > 0) {
    if (ref.size() > 0) ref += ", ";
    ref += "malfunc:" + _malfuncMsg;
  }
  _readVol->setReferences(ref);

  _readVol->setComment(_comment);
  _readVol->setStatusXml(_statusXml);

  vector<string> sourceToks;
  RadxStr::tokenize(_source, ":,", sourceToks);
  for (size_t ii = 0; ii < sourceToks.size(); ii++) {
    if (sourceToks[ii].find("RAD") != string::npos) {
      if (ii < sourceToks.size() - 1) {
        _readVol->setInstrumentName(sourceToks[ii+1]);
      }
    }
    if (sourceToks[ii].find("PLC") != string::npos) {
      if (ii < sourceToks.size() - 1) {
        _readVol->setSiteName(sourceToks[ii+1]);
      }
    }
    if (sourceToks[ii].find("WMO") != string::npos) {
      if (ii < sourceToks.size() - 1) {
        _readVol->setSiteName(sourceToks[ii+1]);
      }
    }
  }
  _readVol->setSiteName(_source);

  _readVol->setLatitudeDeg(_latitudeDeg);
  _readVol->setLongitudeDeg(_longitudeDeg);
  _readVol->setAltitudeKm(_altitudeKm);

  _readVol->setRadarBeamWidthDegH(_beamWidthH);
  _readVol->setRadarBeamWidthDegV(_beamWidthV);
  _readVol->setRadarAntennaGainDbH(_antennaGainH);
  _readVol->setRadarAntennaGainDbV(_antennaGainV);
  _readVol->setRadarReceiverBandwidthMhz(_rxBandwidthMhz);

  if (_readSetMaxRange) {
    _readVol->setMaxRangeKm(_readMaxRangeKm);
  }

  // calibration

  RadxRcalib *cal = new RadxRcalib;
  cal->setPulseWidthUsec(_pulseWidthUs);
  cal->setXmitPowerDbmH(10.0 * log10(_peakPowerKw * 1.0e6));
  cal->setXmitPowerDbmV(10.0 * log10(_peakPowerKw * 1.0e6));
  cal->setTwoWayWaveguideLossDbH(_txLossH * 2.0);
  cal->setTwoWayWaveguideLossDbV(_txLossH * 2.0);
  cal->setCouplerForwardLossDbH(_injectLossH);
  cal->setCouplerForwardLossDbV(_injectLossV);
  cal->setReceiverGainDbHc(-_rxLossH);
  cal->setReceiverGainDbVc(-_rxLossV);
  cal->setTwoWayRadomeLossDbH(_radomeLossOneWayH * 2.0);
  cal->setTwoWayRadomeLossDbV(_radomeLossOneWayV * 2.0);
  cal->setRadarConstantH(_radarConstantH);
  cal->setRadarConstantV(_radarConstantV);
  cal->setAntennaGainDbH(_antennaGainH);
  cal->setAntennaGainDbV(_antennaGainV);
  cal->setBaseDbz1kmHc(_dbz0H);
  cal->setBaseDbz1kmVc(_dbz0V);
  cal->setNoiseDbmHc(_noiseH);
  cal->setNoiseDbmVc(_noiseV);
  cal->setZdrCorrectionDb(_powerDiff);
  cal->setSystemPhidpDeg(_phaseDiff);
  _readVol->addCalib(cal);

  // load the sweep information from the rays

  // _readVol->loadSweepInfoFromRays();
  
  // constrain the sweep data as appropriate
  
  if (_readFixedAngleLimitsSet) {
    if (_readVol->constrainByFixedAngle(_readMinFixedAngle, _readMaxFixedAngle,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - OdimHdf5RadxFile::_finalizeReadVolume");
      _addErrStr("  No data found within fixed angle limits");
      _addErrDbl("  min fixed angle: ", _readMinFixedAngle);
      _addErrDbl("  max fixed angle: ", _readMaxFixedAngle);
      return -1;
    }
  } else if (_readSweepNumLimitsSet) {
    if (_readVol->constrainBySweepNum(_readMinSweepNum, _readMaxSweepNum,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - OdimHdf5RadxFile::_finalizeReadVolume");
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

  // check for sweep mode

  Radx::SweepMode_t sweepMode = _readVol->getPredomSweepModeFromAngles();
  if (sweepMode == Radx::SWEEP_MODE_RHI) {
    _readVol->setScanName("RHI");
  } else if (sweepMode == Radx::SWEEP_MODE_SECTOR) {
    _readVol->setScanName("SEC");
  } else {
    _readVol->setScanName("SUR");
  }
  vector<RadxRay *> &rays = _readVol->getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {
    rays[iray]->setSweepMode(sweepMode);
  }

  // load the sweep information from the rays

  _readVol->loadSweepInfoFromRays();
  
  return 0;

}

////////////////////////////////////////////////////////////
// Is this a Gematronik field file?
// Each field is stored in a separate file

bool OdimHdf5RadxFile::_isGematronikFieldFile(const string &path,
                                              string &dateStr,
                                              string &fieldName,
                                              string &volName)
  
{

  RadxPath rpath(path);
  string fileName = rpath.getFile();

  // find location of first digit
  
  size_t firstDigitLoc = string::npos;
  for (size_t ii = 0; ii < fileName.size(); ii++) {
    if (isdigit(fileName[ii])) {
      firstDigitLoc = ii;
      break;
    }
  }

  // read in date / time

  int year, month, day, hour, min, sec, id;
  if (sscanf(fileName.c_str() + firstDigitLoc,
             "%4d%2d%2d%2d%2d%2d%2d",
             &year, &month, &day, &hour, &min, &sec, &id) != 7) {
    return false;
  }

  RadxTime ptime(year, month, day, hour, min, sec);
  dateStr = fileName.substr(firstDigitLoc, 16);

  // get field name

  size_t startFieldNamePos = firstDigitLoc + 16;
  size_t endFieldNamePos = fileName.find(".", startFieldNamePos);
  if (endFieldNamePos == string::npos) {
    return false;
  }
  size_t endVolPos = fileName.find(".", endFieldNamePos + 1);
  if (endVolPos == string::npos) {
    return false;
  }

  fieldName = fileName.substr(startFieldNamePos,
                              endFieldNamePos - startFieldNamePos);
  
  volName = fileName.substr(endFieldNamePos + 1,
                            endVolPos - endFieldNamePos - 1);
  
  return true;

}

////////////////////////////////////////////////////////////
// Read in data from Gematronik field files
// Each field is stored in a separate file
//
// Returns 0 on success, -1 on failure

int OdimHdf5RadxFile::_readGemFieldFiles(const string &path,
                                         const string &dateStr,
                                         const string &fieldName,
                                         RadxVol &vol)
  
{

  // get dir

  RadxPath rpath(path);
  string dirPath = rpath.getDirectory();

  // load up array of file names that match the dateStr
  
  vector<string> fileNames;
  RadxReadDir rdir;
  if (rdir.open(dirPath.c_str()) == 0) {
    
    // Loop thru directory looking for the data file names
    // or forecast directories
    
    struct dirent *dp;
    for (dp = rdir.read(); dp != NULL; dp = rdir.read()) {
      
      // exclude dir entries beginning with '.'
      
      if (dp->d_name[0] == '.') {
	continue;
      }

      // make sure we have .vol files

      if (strstr(dp->d_name, ".vol") == NULL) {
	continue;
      }

      string dName(dp->d_name);
      if (dName.find(dateStr) != string::npos) {
        fileNames.push_back(dName);
      }
      
    } // dp
    
    rdir.close();

  } // if (rdir ...

  // sort the file names
  
  sort(fileNames.begin(), fileNames.end());
  
  // read in files
  
  for (size_t ifile = 0; ifile < fileNames.size(); ifile++) {
    
    const string &fileName = fileNames[ifile];
    string filePath = dirPath + RadxPath::RADX_PATH_DELIM + fileName;

    if (ifile == 0) {

      // primary file
      // read into final vol

      if (_readFromPath(filePath, vol)) {
        return -1;
      }
      
    } else {

      // secondary file
      // read into temporary volume
      
      RadxVol tmpVol;
      if (_readFromPath(filePath, tmpVol)) {
        return -1;
      }

      // copy fields across to final volume if the geometry is the same

      vector<RadxRay *> &volRays = vol.getRays();
      vector<RadxRay *> &tmpRays = tmpVol.getRays();

      if (volRays.size() == tmpRays.size()) {

        for (size_t iray = 0; iray < volRays.size(); iray++) {
          
          RadxRay *volRay = volRays[iray];
          RadxRay *tmpRay = tmpRays[iray];
          
          vector<RadxField *> tmpFields = tmpRay->getFields();
          for (size_t ifield = 0; ifield < tmpFields.size(); ifield++) {
            RadxField *newField = new RadxField(*tmpFields[ifield]);
            volRay->addField(newField);
          } // ifield
          
        } // iray

      } // if (volRays.size() == tmpRays.size()
      
    } // if (ifile == 0)
    
  } // ifile
  
  return 0;

}

////////////////////
// open file

int OdimHdf5RadxFile::_openFileForWriting(const string &writePath)

{
  _closeFile();

  //  Turn off the auto-printing when failure occurs so that we can
  // handle the errors appropriately
  H5::Exception::dontPrint();

  try {
    _file = new H5File(writePath.c_str(), H5F_ACC_TRUNC);
  }
  
  // catch failure caused by the H5File operations
  catch(H5::Exception error)
  {
    _addErrStr("ERROR - OdimHdf5RadxFile::_openFileForWriting");
    _addErrStr("  Cannot open file for writing: ", writePath);
    _addErrStr(error.getDetailMsg());
    return -1;
  }

  return 0;

}

////////////////////
// close file

void OdimHdf5RadxFile::_closeFile()

{
  if (_file) {
    _file->close();
    delete _file;
    _file = NULL;
  }
}

///////////////////////////////////////////////////////////////////////////
// Compute the output path

string OdimHdf5RadxFile::_computeWritePath(const RadxVol &writeVol,
                                           const RadxTime &startTime,
                                           int startMillisecs,
                                           const RadxTime &endTime,
                                           int endMillisecs,
                                           const RadxTime &fileTime,
                                           int fileMillisecs,
                                           const string &dir)

{

  // compute path
  
  string instName;
  if (_writeInstrNameInFileName) {
    if (writeVol.getInstrumentName().size() > 0) {
      instName = "_";
      instName += writeVol.getInstrumentName();
    }
  }

  string siteName;
  if (_writeSiteNameInFileName) {
    if (writeVol.getSiteName().size() > 0) {
      siteName = "_";
      siteName += writeVol.getSiteName();
    }
  }

  string scanName;
  Radx::SweepMode_t sweepMode = writeVol.getPredomSweepModeFromAngles();
  if (sweepMode == Radx::SWEEP_MODE_RHI) {
    scanName = "_RHI";
  } else if (sweepMode == Radx::SWEEP_MODE_SECTOR) {
    scanName = "_SEC";
  } else {
    scanName = "_SUR";
  }

  int volNum = writeVol.getVolumeNumber();
  char volNumStr[1024];
  if (_writeVolNumInFileName && volNum >= 0) {
    sprintf(volNumStr, "_v%d", volNum);
  } else {
    volNumStr[0] = '\0'; // NULL str
  }

  string prefix = "odim.";
  if (_writeFileNamePrefix.size() > 0) {
    prefix = _writeFileNamePrefix;
  }

  char dateTimeConnector = '_';
  if (_writeHyphenInDateTime) {
    dateTimeConnector = '-';
  }

  char fileName[BUFSIZ];
  if (_writeFileNameMode == FILENAME_WITH_START_AND_END_TIMES) {

    char startSubsecsStr[64];
    char endSubsecsStr[64];
    if (_writeSubsecsInFileName) {
      sprintf(startSubsecsStr, ".%.3d", startMillisecs);
      sprintf(endSubsecsStr, ".%.3d", endMillisecs);
    } else {
      startSubsecsStr[0] = '\0';
      endSubsecsStr[0] = '\0';
    }

    sprintf(fileName,
            "%s%.4d%.2d%.2d%c%.2d%.2d%.2d%s"
            "_to_%.4d%.2d%.2d%c%.2d%.2d%.2d%s"
            "%s%s%s"
            "%s.h5",
            prefix.c_str(),
            startTime.getYear(), startTime.getMonth(), startTime.getDay(),
            dateTimeConnector,
            startTime.getHour(), startTime.getMin(), startTime.getSec(),
            startSubsecsStr,
            endTime.getYear(), endTime.getMonth(), endTime.getDay(),
            dateTimeConnector,
            endTime.getHour(), endTime.getMin(), endTime.getSec(),
            endSubsecsStr,
            instName.c_str(), siteName.c_str(), volNumStr,
            scanName.c_str());

  } else {
    
    char fileSubsecsStr[64];
    if (_writeSubsecsInFileName) {
      sprintf(fileSubsecsStr, ".%.3d", fileMillisecs);
    } else {
      fileSubsecsStr[0] = '\0';
    }

    sprintf(fileName,
            "%s%.4d%.2d%.2d%c%.2d%.2d%.2d%s"
            "%s%s%s"
            "%s.h5",
            prefix.c_str(),
            fileTime.getYear(), fileTime.getMonth(), fileTime.getDay(),
            dateTimeConnector,
            fileTime.getHour(), fileTime.getMin(), fileTime.getSec(),
            fileSubsecsStr,
            instName.c_str(), siteName.c_str(), volNumStr,
            scanName.c_str());

  }

  // make sure the file name is valid - i.e. no / or whitespace

  for (size_t ii = 0; ii < strlen(fileName); ii++) {
    if (isspace(fileName[ii]) || fileName[ii] == '/') {
      fileName[ii] = '_';
    }
  }

  // construct path

  string outPath(dir);
  outPath += PATH_SEPARATOR;
  outPath += fileName;
  return outPath;

}

/////////////////////////////////////////////////////////
// Perform the write to the specified temp path
//
// Returns 0 on success, -1 on failure

int OdimHdf5RadxFile::_doWrite(const RadxVol &vol,
                               const string &tmpPath)
  
{

  int iret = 0;

  if (_openFileForWriting(tmpPath)) {
    _addErrStr("ERROR - OdimHdf5RadxFile::_doWrite");
    return -1;
  }
  
  // add top-level groups

  Group what(_file->createGroup("what"));
  Group where(_file->createGroup("where"));
  Group how(_file->createGroup("how"));
  
  // add file (root) attributes

  Group root(_file->openGroup("/"));
  Hdf5xx::addAttr(root, "Conventions", "ODIM_H5/V2_2");

  // add mandatory top-level what attributes

  if (vol.getNSweeps() > 1) {
    Hdf5xx::addAttr(what, "object", "PVOL");
  } else {
    if (_sweepMode == Radx::SWEEP_MODE_SECTOR) {
      Hdf5xx::addAttr(what, "object", "AZIM");
    } else if (_sweepMode == Radx::SWEEP_MODE_RHI) {
      Hdf5xx::addAttr(what, "object", "ELEV");
    } else {
      Hdf5xx::addAttr(what, "object", "SCAN");
    }
  }

  _version = vol.getVersion();
  if (_version.size() > 0 && _version.find("H5rad") != string::npos) {
    Hdf5xx::addAttr(what, "version", vol.getVersion());
  } else {
    Hdf5xx::addAttr(what, "version", "H5rad 2.2");
  }
  _extendedMetaDataOnWrite = false;
  if (_version.find("H5rad 3") != string::npos) {
    _extendedMetaDataOnWrite = true;
  }

  RadxTime startTime(vol.getStartTimeSecs(), vol.getStartNanoSecs() / 1.0e9);
  char dateStr[32];
  sprintf(dateStr, "%.4d%.2d%.2d",
          startTime.getYear(), startTime.getMonth(), startTime.getDay());
  Hdf5xx::addAttr(what, "date", dateStr);

  char timeStr[32];
  sprintf(timeStr, "%.2d%.2d%.2d",
          startTime.getHour(), startTime.getMin(), startTime.getSec());
  Hdf5xx::addAttr(what, "time", timeStr);

  if (vol.getSource().size() > 0) {
    Hdf5xx::addAttr(what, "source", vol.getSource());
  } else {
    string radarName("RAD:");
    radarName += vol.getInstrumentName();
    Hdf5xx::addAttr(what, "source", radarName);
  }

  // add top-level where attributes
  
  Hdf5xx::addAttr(where, "height", vol.getAltitudeKm() * 1000.0);
  Hdf5xx::addAttr(where, "lat", vol.getLatitudeDeg());
  Hdf5xx::addAttr(where, "lon", vol.getLongitudeDeg());

  // add top-level how attributes

  Radx::fl64 startepochs = (Radx::fl64) vol.getStartTimeSecs();
  Hdf5xx::addAttr(how, "startepochs", startepochs);
  
  Radx::fl64 endepochs = (Radx::fl64) vol.getEndTimeSecs();
  Hdf5xx::addAttr(how, "endepochs", endepochs);

  Radx::si64 nSweeps = vol.getNSweeps();
  Hdf5xx::addAttr(how, "scan_count", nSweeps);
  
  Hdf5xx::addAttr(what, "software", "LROSE-Radx");

  Hdf5xx::addAttr(how, "wavelength", vol.getWavelengthCm());

  double beamWidthH = vol.getRadarBeamWidthDegH();
  Hdf5xx::addAttr(how, "beamwidth", beamWidthH);

  // nominal antenna gain

  double antennaGainH = vol.getRadarAntennaGainDbH();
  Hdf5xx::addAttr(how, "nomAntgain", antennaGainH);
  Hdf5xx::addAttr(how, "nomAntgainH", antennaGainH);
  double antennaGainV = vol.getRadarAntennaGainDbV();
  Hdf5xx::addAttr(how, "nomAntgainV", antennaGainV);
  
  Hdf5xx::addAttr(how, "RXbandwidth", vol.getPlatform().getRadarReceiverBandwidthMhz());
  
  if (vol.getNRcalibs() > 0) {

    const RadxRcalib &cal = *(vol.getRcalibs()[0]);
    
    if (cal.getTwoWayWaveguideLossDbH() != Radx::missingMetaDouble) {
      double txLossH = cal.getTwoWayWaveguideLossDbH() / 2.0;
      Hdf5xx::addAttr(how, "TXlossH", txLossH);
    }
    if (cal.getTwoWayWaveguideLossDbV() != Radx::missingMetaDouble) {
      double txLossV = cal.getTwoWayWaveguideLossDbV() / 2.0;
      Hdf5xx::addAttr(how, "TXlossV", txLossV);
    }
    
    if (cal.getPowerMeasLossDbH() != Radx::missingMetaDouble &&
        cal.getCouplerForwardLossDbH() != Radx::missingMetaDouble) {
      double injectLossH =
        cal.getPowerMeasLossDbH() + cal.getCouplerForwardLossDbH();
      Hdf5xx::addAttr(how, "injectLossH", injectLossH);
    }
    if (cal.getPowerMeasLossDbV() != Radx::missingMetaDouble &&
        cal.getCouplerForwardLossDbV() != Radx::missingMetaDouble) {
      double injectLossV =
        cal.getPowerMeasLossDbV() + cal.getCouplerForwardLossDbV();
      Hdf5xx::addAttr(how, "injectLossV", injectLossV);
    }

    if (cal.getTwoWayWaveguideLossDbH() != Radx::missingMetaDouble &&
        cal.getReceiverGainDbHc() != Radx::missingMetaDouble) {
      double rxLossH =
        cal.getTwoWayWaveguideLossDbH() / 2.0 - cal.getReceiverGainDbHc();
      Hdf5xx::addAttr(how, "RXlossH", rxLossH);
    }
    if (cal.getTwoWayWaveguideLossDbV() != Radx::missingMetaDouble &&
        cal.getReceiverGainDbVc() != Radx::missingMetaDouble) {
      double rxLossV =
        cal.getTwoWayWaveguideLossDbV() / 2.0 - cal.getReceiverGainDbVc();
      Hdf5xx::addAttr(how, "RXlossV", rxLossV);
    }

    if (cal.getTwoWayRadomeLossDbH() != Radx::missingMetaDouble) {
      double radomeLossH = cal.getTwoWayRadomeLossDbH() / 2.0;
      Hdf5xx::addAttr(how, "radomelossH", radomeLossH);
    }
    if (cal.getTwoWayRadomeLossDbV() != Radx::missingMetaDouble) {
      double radomeLossV = cal.getTwoWayRadomeLossDbV() / 2.0;
      Hdf5xx::addAttr(how, "radomelossV", radomeLossV);
    }

    if (cal.getAntennaGainDbH() != Radx::missingMetaDouble) {
      double antGainH = cal.getAntennaGainDbH();
      Hdf5xx::addAttr(how, "antgainH", antGainH);
    }
    if (cal.getAntennaGainDbV() != Radx::missingMetaDouble) {
      double antGainV = cal.getAntennaGainDbV();
      Hdf5xx::addAttr(how, "antgainV", antGainV);
    }

    if (cal.getBeamWidthDegH() != Radx::missingMetaDouble) {
      double beamWidthH = cal.getBeamWidthDegH();
      Hdf5xx::addAttr(how, "beamwH", beamWidthH);
    }
    if (cal.getBeamWidthDegV() != Radx::missingMetaDouble) {
      double beamWidthV = cal.getBeamWidthDegV();
      Hdf5xx::addAttr(how, "beamwV", beamWidthV);
    }

    if (cal.getRadarConstantH() != Radx::missingMetaDouble) {
      double radarConstH = cal.getRadarConstantH();
      Hdf5xx::addAttr(how, "radconstH", radarConstH);
    }
    if (cal.getRadarConstantV() != Radx::missingMetaDouble) {
      double radarConstV = cal.getRadarConstantV();
      Hdf5xx::addAttr(how, "radconstV", radarConstV);
    }

    if (cal.getXmitPowerDbmH() != Radx::missingMetaDouble) {
      double txPwrHKw = pow(10.0, cal.getXmitPowerDbmH() / 10.0) * 1.0e-6;
      Hdf5xx::addAttr(how, "nomTXpower", txPwrHKw);
    }

    if (cal.getZdrCorrectionDb() != Radx::missingMetaDouble) {
      double zdrCorrDb = cal.getZdrCorrectionDb();
      Hdf5xx::addAttr(how, "powerdiff", zdrCorrDb);
    }

    if (cal.getSystemPhidpDeg() != Radx::missingMetaDouble) {
      double systemPhidp = cal.getSystemPhidpDeg();
      Hdf5xx::addAttr(how, "phasediff", systemPhidp);
    }

    if (cal.getBaseDbz1kmHc() != Radx::missingMetaDouble) {
      double dbz1kmH = cal.getBaseDbz1kmHc();
      Hdf5xx::addAttr(how, "NEZH", dbz1kmH);
    }
    if (cal.getBaseDbz1kmVc() != Radx::missingMetaDouble) {
      double dbz1kmV = cal.getBaseDbz1kmVc();
      Hdf5xx::addAttr(how, "NEZV", dbz1kmV);
    }

    if (cal.getNoiseDbmHc() != Radx::missingMetaDouble) {
      double noiseH = cal.getNoiseDbmHc();
      Hdf5xx::addAttr(how, "nsampleH", noiseH);
    }
    if (cal.getNoiseDbmVc() != Radx::missingMetaDouble) {
      double noiseV = cal.getNoiseDbmVc();
      Hdf5xx::addAttr(how, "nsampleV", noiseV);
    }

  } // if (vol.getNRcalibs() > 0) {

  // loop through the sweeps

  for (size_t isweep = 0; isweep < vol.getNSweeps(); isweep++) {

    // make a volume containing just this sweep
    
    RadxVol sweepVol(vol, isweep);

    // write the sweep
    if (_writeSweep(sweepVol, isweep)) {
      iret = -1;
    }

  } // isweep

  // clean up
  
  _closeFile();
  
  if (iret) {
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////
// write a sweep
//
// Returns 0 on success, -1 on failure

int OdimHdf5RadxFile::_writeSweep(RadxVol &sweepVol,
                                  size_t isweep)
  
{

  // initialize return code

  int iret = 0;

  // create sweep pointer

  RadxSweep *sweep = sweepVol.getSweeps()[0];
  
  // sanity check

  if (sweepVol.getNRays() < 2) {
    cerr << "WARNING - OdimHdf5RadxFile::_writeSweep" << endl;
    cerr << "  No rays present, sweep index: " << isweep << endl;
    return iret;
  }

  // surveillance sweeps should only be 360 degrees

  if (sweepVol.checkIsSurveillance()) {
    sweepVol.trimSurveillanceSweepsTo360Deg();
  }

  // set start and end times

  RadxRay *startRay = sweepVol.getRays()[0];
  RadxRay *endRay = sweepVol.getRays()[sweepVol.getNRays() - 1];
  RadxTime startTime(startRay->getTimeSecs(), startRay->getNanoSecs() / 1.0e9);
  RadxTime endTime(endRay->getTimeSecs(), endRay->getNanoSecs() / 1.0e9);

  // ensure the fixed angles have been set - i.e. are not missing
  // do not force this - i.e. only compute the fixed angle
  // if it is not already set.

  bool force = false;
  sweepVol.computeFixedAnglesFromRays(force);

  // ensure all rays on sweep have a constant number of rays
  
  sweepVol.setNGatesConstant();

  // ensure the range geom is constant

  sweepVol.remapToPredomGeom();

  // compute the scan rate

  sweepVol.computeSweepScanRatesFromRays();

  // save the starting azimuth for the sweep

  double startAz = sweepVol.getRays()[0]->getAzimuthDeg();
  Radx::si64 ray1Offset = 0;

  // for surveillance or sector, sort the rays into ascending azimuth
  
  if (sweepVol.checkIsSurveillance() ||
      sweepVol.checkIsSector()) {
    sweepVol.sortSweepRaysByAzimuth();
    // find the ray offset for the starting azimuth
    vector<RadxRay *> &rays = sweepVol.getRays();
    for (size_t iray = 0; iray < rays.size(); iray++) {
      if (rays[iray]->getAzimuthDeg() == startAz) {
        ray1Offset = iray;
        break;
      }
    } // iray
  }

  // create the sweep dataset
  
  char datasetName[128];
  sprintf(datasetName, "dataset%d", (int) isweep + 1);
  Group dataset(_file->createGroup(datasetName));
  
  // add the what group attributes
  
  Group what(dataset.createGroup("what"));
  Group where(dataset.createGroup("where"));
  Group how(dataset.createGroup("how"));

  char startDateStr[32];
  sprintf(startDateStr, "%.4d%.2d%.2d",
          startTime.getYear(), startTime.getMonth(), startTime.getDay());
  Hdf5xx::addAttr(what, "startdate", startDateStr);

  char startTimeStr[32];
  sprintf(startTimeStr, "%.2d%.2d%.2d",
          startTime.getHour(), startTime.getMin(), startTime.getSec());
  Hdf5xx::addAttr(what, "starttime", startTimeStr);

  char endDateStr[32];
  sprintf(endDateStr, "%.4d%.2d%.2d",
          endTime.getYear(), endTime.getMonth(), endTime.getDay());
  Hdf5xx::addAttr(what, "enddate", endDateStr);

  char endTimeStr[32];
  sprintf(endTimeStr, "%.2d%.2d%.2d",
          endTime.getHour(), endTime.getMin(), endTime.getSec());
  Hdf5xx::addAttr(what, "endtime", endTimeStr);

  switch(sweepVol.getPredomSweepModeFromAngles()) {
    case Radx::SWEEP_MODE_RHI:
      Hdf5xx::addAttr(what, "product", "RHI");
      break;
    case Radx::SWEEP_MODE_SECTOR:
      Hdf5xx::addAttr(what, "product", "AZIM");
      break;
    default:
      Hdf5xx::addAttr(what, "product", "SCAN");
  }
  
  // add the where group attributes
  
  RadxRay *ray0 = sweepVol.getRays()[0];
  RadxRay *ray1 = sweepVol.getRays()[1];

  Radx::fl64 elevDeg = sweep->getFixedAngleDeg();
  Hdf5xx::addAttr(where, "elangle", elevDeg);

  Radx::si64 nbins = ray0->getNGates();
  Hdf5xx::addAttr(where, "nbins", nbins);

  Radx::fl64 rstartKm = ray0->getStartRangeKm();
  Hdf5xx::addAttr(where, "rstart", rstartKm);

  Radx::fl64 rscaleM = ray0->getGateSpacingKm() * 1000.0;
  Hdf5xx::addAttr(where, "rscale", rscaleM);

  Radx::si64 nrays = sweepVol.getNRays();
  Hdf5xx::addAttr(where, "nrays", nrays);

  Hdf5xx::addAttr(where, "a1gate", ray1Offset);

  if (sweepVol.checkIsSector()) {
    Radx::fl64 startAz = ray0->getAzimuthDeg();
    RadxRay *rayLast = sweepVol.getRays()[sweepVol.getNRays()-1];
    Radx::fl64 stopAz = rayLast->getAzimuthDeg();
    Hdf5xx::addAttr(where, "startaz", startAz);
    Hdf5xx::addAttr(where, "stopaz", stopAz);
  }

  // add the how group attributes
  
  Radx::si64 scanIndex = isweep + 1;
  Hdf5xx::addAttr(how, "scan_index", scanIndex);

  double az0 = ray0->getAzimuthDeg();
  double az1 = ray1->getAzimuthDeg();
  double deltaAz = Radx::conditionAngleDelta(az1 - az0);
  _aStart = Radx::conditionAz(az0 - deltaAz / 2.0);
  Hdf5xx::addAttr(how, "astart", _aStart);

  Radx::fl64 pulseWidthUs = ray0->getPulseWidthUsec();
  Hdf5xx::addAttr(how, "pulsewidth", pulseWidthUs);
  
  if (_sweepMode == Radx::SWEEP_MODE_RHI) {
    Radx::fl64 elevRate = sweep->getMeasuredScanRateDegPerSec();
    Hdf5xx::addAttr(how, "elevspeed", elevRate);
  } else {
    Radx::fl64 rpm = sweep->getMeasuredScanRateDegPerSec() / 6.0;
    Hdf5xx::addAttr(how, "rpm", rpm);
  }

  if (ray0->getPrtSec() != Radx::missingMetaDouble) {

    double shortPrt = ray0->getPrtSec();
    Radx::fl64 highprf = 1.0 / shortPrt;
    Radx::fl64 lowprf = 1.0 / shortPrt;
    if (ray0->getPrtMode() == Radx::PRT_MODE_STAGGERED ||
        ray0->getPrtMode() == Radx::PRT_MODE_DUAL) {
      double ratio = ray0->getPrtRatio();
      if (ratio != Radx::missingMetaDouble) {
        double longPrt = shortPrt * ratio;
        lowprf = 1.0 / longPrt;
      }
    }
    Hdf5xx::addAttr(how, "lowprf", lowprf);
    Hdf5xx::addAttr(how, "highprf", highprf);

  }

  Hdf5xx::addAttr(how, "TXtype", "unknown");

  _polMode = ray0->getPolarizationMode();
  if (_polMode == Radx::POL_MODE_HORIZONTAL) {
    Hdf5xx::addAttr(how, "poltype", "single");
    Hdf5xx::addAttr(how, "polmode", "single-H");
  } else if (_polMode == Radx::POL_MODE_VERTICAL) {
    Hdf5xx::addAttr(how, "poltype", "single");
    Hdf5xx::addAttr(how, "polmode", "single-V");
  } else if (_polMode == Radx::POL_MODE_CIRCULAR) {
    Hdf5xx::addAttr(how, "poltype", "single");
    Hdf5xx::addAttr(how, "polmode", "single-C");
  } else if (_polMode == Radx::POL_MODE_HV_ALT) {
    Hdf5xx::addAttr(how, "poltype", "switched-dual");
    Hdf5xx::addAttr(how, "polmode", "switched-dual");
  } else if (_polMode == Radx::POL_MODE_HV_SIM) {
    Hdf5xx::addAttr(how, "poltype", "simultaneous-dual");
    Hdf5xx::addAttr(how, "polmode", "simultaneous-dual");
  } else if (_polMode == Radx::POL_MODE_HV_H_XMIT) {
    Hdf5xx::addAttr(how, "poltype", "single");
    Hdf5xx::addAttr(how, "polmode", "LDR-H");
  } else if (_polMode == Radx::POL_MODE_HV_V_XMIT) {
    Hdf5xx::addAttr(how, "poltype", "single");
    Hdf5xx::addAttr(how, "polmode", "LDR-V");
  } else {
    Hdf5xx::addAttr(how, "poltype", "single");
    Hdf5xx::addAttr(how, "polmode", "single-H");
  }

  Radx::fl64 nyquist = ray0->getNyquistMps();
  Hdf5xx::addAttr(how, "NI", nyquist);

  Radx::si64 nsamples = ray0->getNSamples();
  Hdf5xx::addAttr(how, "Vsamples", nsamples);

  if (_extendedMetaDataOnWrite) {
    switch(sweepVol.getPredomSweepModeFromAngles()) {
      case Radx::SWEEP_MODE_RHI:
        Hdf5xx::addAttr(how, "description", "rhi_sweep");
        break;
      case Radx::SWEEP_MODE_SECTOR:
        Hdf5xx::addAttr(how, "description", "sector_sweep");
        break;
      default:
        Hdf5xx::addAttr(how, "description", "surveillance_sweep");
    }
  }
  
  // ray times

  const vector<RadxRay *> &rays = sweepVol.getRays();
  vector<Radx::fl64> rayTimes, timeStart, timeEnd;
  rayTimes.resize(rays.size());
  timeStart.resize(rays.size());
  timeEnd.resize(rays.size());
  for (size_t ii = 0; ii < rays.size(); ii++) {
    const RadxRay *ray = rays[ii];
    RadxTime rayTime(ray->getRadxTime());
    rayTimes[ii] = rayTime.asDouble();
  } // ii
  for (size_t ii = 1; ii < rays.size(); ii++) {
    double deltaTime = rayTimes[ii] - rayTimes[ii-1];
    double deltaHalf = deltaTime / 2.0;
    timeStart[ii] = rayTimes[ii-1] + deltaHalf;
    timeEnd[ii-1] = rayTimes[ii] - deltaHalf;
    if (ii == 1) {
      timeStart[ii-1] = rayTimes[ii-1] - deltaHalf;
    } else if (ii == rays.size() - 1) {
      timeEnd[ii] = rayTimes[ii] + deltaHalf;
    }
  } // ii

  if (_sweepMode == Radx::SWEEP_MODE_RHI) {

    // ray azimuth angles
    
    vector<Radx::fl64> azAngles;
    for (size_t ii = 0; ii < rays.size(); ii++) {
      const RadxRay *ray = rays[ii];
      azAngles.push_back(ray->getAzimuthDeg());
    } // ii
    Hdf5xx::addAttr(how, "azangles", azAngles);
    
    // ray elevation angles
    
    vector<Radx::fl64> elAngles, elStart, elEnd;
    elAngles.resize(rays.size());
    elStart.resize(rays.size());
    elEnd.resize(rays.size());
    for (size_t ii = 0; ii < rays.size(); ii++) {
      const RadxRay *ray = rays[ii];
      elAngles[ii] = ray->getElevationDeg();
    } // ii
    for (size_t ii = 1; ii < rays.size(); ii++) {
      double deltaEl = Radx::conditionAngleDelta(elAngles[ii] - elAngles[ii-1]);
      double deltaHalf = deltaEl / 2.0;
      elStart[ii] = Radx::conditionEl(elAngles[ii-1] + deltaHalf);
      elEnd[ii-1] = Radx::conditionEl(elAngles[ii] - deltaHalf);
      if (ii == 1) {
        elStart[ii-1] = Radx::conditionEl(elAngles[ii-1] - deltaHalf);
      } else if (ii == rays.size() - 1) {
        elEnd[ii] = Radx::conditionEl(elAngles[ii] + deltaHalf);
      }
    } // ii
    Hdf5xx::addAttr(how, "startelA", elStart);
    Hdf5xx::addAttr(how, "stopelA", elEnd);
    Hdf5xx::addAttr(how, "startelT", timeStart);
    Hdf5xx::addAttr(how, "stopelT", timeEnd);

  } else {

    // ray elevation angles
    
    vector<Radx::fl64> elevAngles;
    for (size_t ii = 0; ii < rays.size(); ii++) {
      const RadxRay *ray = rays[ii];
      elevAngles.push_back(ray->getElevationDeg());
    } // ii
    Hdf5xx::addAttr(how, "elangles", elevAngles);
    
    // ray azimuth angles
    
    vector<Radx::fl64> azAngles, azStart, azEnd;
    azAngles.resize(rays.size());
    azStart.resize(rays.size());
    azEnd.resize(rays.size());
    for (size_t ii = 0; ii < rays.size(); ii++) {
      const RadxRay *ray = rays[ii];
      azAngles[ii] = ray->getAzimuthDeg();
    } // ii
    for (size_t ii = 1; ii < rays.size(); ii++) {
      double deltaAz = Radx::conditionAngleDelta(azAngles[ii] - azAngles[ii-1]);
      double deltaHalf = deltaAz / 2.0;
      azStart[ii] = Radx::conditionAz(azAngles[ii-1] + deltaHalf);
      azEnd[ii-1] = Radx::conditionAz(azAngles[ii] - deltaHalf);
      if (ii == 1) {
        azStart[ii-1] = Radx::conditionAz(azAngles[ii-1] - deltaHalf);
      } else if (ii == rays.size() - 1) {
        azEnd[ii] = Radx::conditionAz(azAngles[ii] + deltaHalf);
      }
    } // ii
    Hdf5xx::addAttr(how, "startazA", azStart);
    Hdf5xx::addAttr(how, "stopazA", azEnd);
    Hdf5xx::addAttr(how, "startazT", timeStart);
    Hdf5xx::addAttr(how, "stopazT", timeEnd);

  }

  // convert field representation to contiguous array on the volume

  sweepVol.loadFieldsFromRays();

  // add fields to data set

  vector<RadxField *> fields = sweepVol.getFields();
  for (size_t ifield = 0; ifield < fields.size(); ifield++) {
    if (_writeField(sweepVol, isweep, *fields[ifield], ifield, dataset)) {
      iret = -1;
    }
  }

  if (iret) {
    return -1;
  } else {
    return 0;
  }

}

/////////////////////////////////////////////////////////
// add a field to a data set for writing
// Returns 0 on success, -1 on failure

int OdimHdf5RadxFile::_writeField(RadxVol &sweepVol,
                                  size_t isweep,
                                  RadxField &field,
                                  size_t ifield,
                                  Group &dataset)
  
{

  // create the field group
  
  char dataName[128];
  sprintf(dataName, "data%d", (int) ifield + 1);
  Group data(dataset.createGroup(dataName));
  
  // add the what and how group attributes
  
  Group what(data.createGroup("what"));

  Hdf5xx::addAttr(what, "quantity", field.getName());
  if (_verbose) {
    cerr << "... sweep index: " << isweep 
         << ", adding field: " << field.getName() << endl;
  }

  Radx::fl64 scale = 1.0;
  Radx::fl64 offset = 0.0;
  if (field.getDataType() == Radx::SI08 ||
      field.getDataType() == Radx::SI16 ||
      field.getDataType() == Radx::SI32) {
    scale = field.getScale();
    offset = field.getOffset();
  }
  
  Hdf5xx::addAttr(what, "gain", scale);
  Hdf5xx::addAttr(what, "offset", offset);

  Radx::fl64 miss = field.getMissing();
  Hdf5xx::addAttr(what, "undetect", miss);
  Hdf5xx::addAttr(what, "nodata", miss);

  Group how(data.createGroup("how"));
  field.computeMinAndMax();
  Hdf5xx::addAttr(how, "min", field.getMinValue());
  Hdf5xx::addAttr(how, "max", field.getMaxValue());
  if (_extendedMetaDataOnWrite) {
    Hdf5xx::addAttr(how, "standard_name", field.getStandardName());
    Hdf5xx::addAttr(how, "long_name", field.getLongName());
    Hdf5xx::addAttr(how, "units", field.getUnits());
  }

  // create the data space

  size_t nRays = field.getNRays();
  size_t nGates = field.getMaxNGates();

  hsize_t fdim[2] = {nRays, nGates};
  DataSpace fspace(2, fdim);
  // chunk dimensions for compression
  hsize_t chunkDims[2] = {nRays/10, nGates/4};
  DSetCreatPropList plist;
  plist.setChunk(2, chunkDims);
  plist.setDeflate(_compressionLevel);

  switch (field.getDataType()) {
    case Radx::FL64: {
      Radx::fl32 fillValue = field.getMissingFl64();
      H5::PredType predType = H5::PredType::IEEE_F64LE;
      if (ByteOrder::hostIsBigEndian()) {
        predType = H5::PredType::IEEE_F64BE;
      }
      plist.setFillValue(predType, &fillValue);
      DataSet dset(data.createDataSet("data", predType,
                                      fspace, plist));
      dset.write(field.getDataFl64(), predType, fspace, fspace);
      Hdf5xx::addAttr(dset, "CLASS", "IMAGE");
      Hdf5xx::addAttr(dset, "IMAGE_VERSION", "1.2");
      break;
    }
    case Radx::FL32: {
      Radx::fl32 fillValue = field.getMissingFl32();
      H5::PredType predType = H5::PredType::IEEE_F32LE;
      if (ByteOrder::hostIsBigEndian()) {
        predType = H5::PredType::IEEE_F32BE;
      }
      plist.setFillValue(predType, &fillValue);
      DataSet dset(data.createDataSet("data", predType,
                                      fspace, plist));
      dset.write(field.getDataFl32(), predType, fspace, fspace);
      Hdf5xx::addAttr(dset, "CLASS", "IMAGE");
      Hdf5xx::addAttr(dset, "IMAGE_VERSION", "1.2");
      break;
    }
    case Radx::SI32: {
      Radx::si32 fillValue = field.getMissingSi32();
      H5::PredType predType = H5::PredType::STD_I32LE;
      if (ByteOrder::hostIsBigEndian()) {
        predType = H5::PredType::STD_I32BE;
      }
      plist.setFillValue(predType, &fillValue);
      DataSet dset(data.createDataSet("data", predType,
                                      fspace, plist));
      dset.write(field.getDataSi32(), predType, fspace, fspace);
      Hdf5xx::addAttr(dset, "CLASS", "IMAGE");
      Hdf5xx::addAttr(dset, "IMAGE_VERSION", "1.2");
      break;
    }
    case Radx::SI16: {
      Radx::si16 fillValue = field.getMissingSi16();
      H5::PredType predType = H5::PredType::STD_I16LE;
      if (ByteOrder::hostIsBigEndian()) {
        predType = H5::PredType::STD_I16BE;
      }
      plist.setFillValue(predType, &fillValue);
      DataSet dset(data.createDataSet("data", predType,
                                      fspace, plist));
      dset.write(field.getDataSi16(), predType, fspace, fspace);
      Hdf5xx::addAttr(dset, "CLASS", "IMAGE");
      Hdf5xx::addAttr(dset, "IMAGE_VERSION", "1.2");
      break;
    }
    case Radx::SI08:
    default: {
      Radx::si08 fillValue = field.getMissingSi08();
      H5::PredType predType = H5::PredType::STD_I8LE;
      if (ByteOrder::hostIsBigEndian()) {
        predType = H5::PredType::STD_I8BE;
      }
      plist.setFillValue(predType, &fillValue);
      DataSet dset(data.createDataSet("data", predType,
                                      fspace, plist));
      dset.write(field.getDataSi08(), predType, fspace, fspace);
      Hdf5xx::addAttr(dset, "CLASS", "IMAGE");
      Hdf5xx::addAttr(dset, "IMAGE_VERSION", "1.2");
      break;
    }
  }

  return 0;

}

