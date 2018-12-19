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
// NcfRadxFile.cc
//
// NcfRadxFile object
//
// NetCDF file data for radar radial data in CF-compliant file
//
// See also NcfRadxFile_read.cc and NcfRadxFile_write.cc
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2009
//
///////////////////////////////////////////////////////////////

#include <Radx/NcfRadxFile.hh>
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

NcfRadxFile::NcfRadxFile() : RadxFile(), RadxNcfStr()
  
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

NcfRadxFile::~NcfRadxFile()

{
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void NcfRadxFile::clear()
  
{

  clearErrStr();

  _file.close();

  _timeDim = NULL;
  _rangeDim = NULL;
  _nPointsDim = NULL;
  _sweepDim = NULL;
  _calDim = NULL;
  _stringLen8Dim = NULL;
  _stringLen32Dim = NULL;
#ifdef NOTYET
  _stringLen64Dim = NULL;
  _stringLen256Dim = NULL;
#endif
  _frequencyDim = NULL;
  
  _volumeNumberVar = NULL;
  _instrumentTypeVar = NULL;
  _platformTypeVar = NULL;
  _primaryAxisVar = NULL;
  _startTimeVar = NULL;
  _endTimeVar = NULL;

  _frequencyVar = NULL;

  _radarAntennaGainHVar = NULL;
  _radarAntennaGainVVar = NULL;
  _radarBeamWidthHVar = NULL;
  _radarBeamWidthVVar = NULL;
  _radarRxBandwidthVar = NULL;

  _lidarConstantVar = NULL;
  _lidarPulseEnergyJVar = NULL;
  _lidarPeakPowerWVar = NULL;
  _lidarApertureDiamCmVar = NULL;
  _lidarApertureEfficiencyVar = NULL;
  _lidarFieldOfViewMradVar = NULL;
  _lidarBeamDivergenceMradVar = NULL;

  _timeVar = NULL;
  _rangeVar = NULL;
  _rayNGatesVar = NULL;
  _rayStartIndexVar = NULL;
  _rayStartRangeVar = NULL;
  _rayGateSpacingVar = NULL;

  _projVar = NULL;
  _latitudeVar = NULL;
  _longitudeVar = NULL;
  _altitudeVar = NULL;

  _sweepNumberVar = NULL;
  _sweepModeVar = NULL;
  _polModeVar = NULL;
  _prtModeVar = NULL;
  _sweepFollowModeVar = NULL;
  _sweepFixedAngleVar = NULL;
  _fixedAnglesFound = false;
  _targetScanRateVar = NULL;
  _sweepStartRayIndexVar = NULL;
  _sweepEndRayIndexVar = NULL;
  _raysAreIndexedVar = NULL;
  _rayAngleResVar = NULL;
  _intermedFreqHzVar = NULL;

  _rCalTimeVar = NULL;
  _rCalPulseWidthVar = NULL;
  _rCalXmitPowerHVar = NULL;
  _rCalXmitPowerVVar = NULL;
  _rCalTwoWayWaveguideLossHVar = NULL;
  _rCalTwoWayWaveguideLossVVar = NULL;
  _rCalTwoWayRadomeLossHVar = NULL;
  _rCalTwoWayRadomeLossVVar = NULL;
  _rCalReceiverMismatchLossVar = NULL;
  _rCalRadarConstHVar = NULL;
  _rCalRadarConstVVar = NULL;
  _rCalAntennaGainHVar = NULL;
  _rCalAntennaGainVVar = NULL;
  _rCalNoiseHcVar = NULL;
  _rCalNoiseHxVar = NULL;
  _rCalNoiseVcVar = NULL;
  _rCalNoiseVxVar = NULL;
  _rCalReceiverGainHcVar = NULL;
  _rCalReceiverGainHxVar = NULL;
  _rCalReceiverGainVcVar = NULL;
  _rCalReceiverGainVxVar = NULL;
  _rCalBaseDbz1kmHcVar = NULL;
  _rCalBaseDbz1kmHxVar = NULL;
  _rCalBaseDbz1kmVcVar = NULL;
  _rCalBaseDbz1kmVxVar = NULL;
  _rCalSunPowerHcVar = NULL;
  _rCalSunPowerHxVar = NULL;
  _rCalSunPowerVcVar = NULL;
  _rCalSunPowerVxVar = NULL;
  _rCalNoiseSourcePowerHVar = NULL;
  _rCalNoiseSourcePowerVVar = NULL;
  _rCalPowerMeasLossHVar = NULL;
  _rCalPowerMeasLossVVar = NULL;
  _rCalCouplerForwardLossHVar = NULL;
  _rCalCouplerForwardLossVVar = NULL;
  _rCalDbzCorrectionVar = NULL;
  _rCalZdrCorrectionVar = NULL;
  _rCalLdrCorrectionHVar = NULL;
  _rCalLdrCorrectionVVar = NULL;
  _rCalSystemPhidpVar = NULL;
  _rCalTestPowerHVar = NULL;
  _rCalTestPowerVVar = NULL;
  _rCalReceiverSlopeHcVar = NULL;
  _rCalReceiverSlopeHxVar = NULL;
  _rCalReceiverSlopeVcVar = NULL;
  _rCalReceiverSlopeVxVar = NULL;

  _azimuthVar = NULL;
  _elevationVar = NULL;
  _pulseWidthVar = NULL;
  _prtVar = NULL;
  _nyquistVar = NULL;
  _unambigRangeVar = NULL;
  _antennaTransitionVar = NULL;
  _georefsAppliedVar = NULL;
  _nSamplesVar = NULL;
  _calIndexVar = NULL;
  _xmitPowerHVar = NULL;
  _xmitPowerVVar = NULL;
  _scanRateVar = NULL;

  _estNoiseDbmHcVar = NULL;
  _estNoiseDbmVcVar = NULL;
  _estNoiseDbmHxVar = NULL;
  _estNoiseDbmVxVar = NULL;

  _estNoiseAvailHc = false;
  _estNoiseAvailVc = false;
  _estNoiseAvailHx = false;
  _estNoiseAvailVx = false;

  _georefsActive = false;
  _altitudeAglVar = NULL;

  _correctionsActive = false;
  _azimuthCorrVar = NULL;
  _elevationCorrVar = NULL;
  _rangeCorrVar = NULL;
  _longitudeCorrVar = NULL;
  _latitudeCorrVar = NULL;
  _pressureAltCorrVar = NULL;
  _altitudeCorrVar = NULL;
  _ewVelCorrVar = NULL;
  _nsVelCorrVar = NULL;
  _vertVelCorrVar = NULL;
  _headingCorrVar = NULL;
  _rollCorrVar = NULL;
  _pitchCorrVar = NULL;
  _driftCorrVar = NULL;
  _rotationCorrVar = NULL;
  _tiltCorrVar = NULL;

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

void NcfRadxFile::_clearRays()
{
  for (int ii = 0; ii < (int) _raysVol.size(); ii++) {
    delete _raysVol[ii];
  }
  _raysVol.clear();
}

void NcfRadxFile::_clearSweeps()
{
  for (int ii = 0; ii < (int) _sweeps.size(); ii++) {
    delete _sweeps[ii];
  }
  _sweeps.clear();
}

void NcfRadxFile::_clearCals()
{
  for (int ii = 0; ii < (int) _rCals.size(); ii++) {
    delete _rCals[ii];
  }
  _rCals.clear();
}

void NcfRadxFile::_clearFields()
{
  for (int ii = 0; ii < (int) _fields.size(); ii++) {
    delete _fields[ii];
  }
  _fields.clear();
}

/////////////////////////////////////////////////////////
// Check if specified file is CfRadial format
// Returns true if supported, false otherwise

bool NcfRadxFile::isSupported(const string &path)

{
  
  if (isCfRadial(path)) {
    return true;
  }
  return false;

}

////////////////////////////////////////////////////////////
// Check if this is a CfRadial file
// Returns true on success, false on failure

bool NcfRadxFile::isCfRadial(const string &path)
  
{

  clear();
  
  // open file

  if (_file.openRead(path)) {
    if (_verbose) {
      cerr << "DEBUG - not CfRadial file" << endl;
      cerr << _file.getErrStr() << endl;
    }
    return false;
  }

  // read dimensions
  if (_readDimensions()) {
    _file.close();
    if (_verbose) {
      cerr << "DEBUG - not CfRadial file" << endl;
      cerr << _errStr << endl;
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

int NcfRadxFile::getTimeFromPath(const string &path, RadxTime &rtime)

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

Nc3Type NcfRadxFile::_getNc3Type(Radx::DataType_t dtype)

{
  switch (dtype) {
    case Radx::FL64:
      return nc3Double;
    case Radx::FL32:
      return nc3Float;
    case Radx::SI32:
      return nc3Int;
    case Radx::SI16:
      return nc3Short;
    case Radx::SI08:
    default:
      return nc3Byte;
  }
}

//////////////////////////////////////////////////////////
// convert RadxFile::netcdf_format_t to Nc3File::FileFormat

Nc3File::FileFormat 
  NcfRadxFile::_getFileFormat(RadxFile::netcdf_format_t format)

{
  switch (format) {
    case NETCDF4_CLASSIC:
      return Nc3File::Netcdf4Classic;
      break;
    case NETCDF_OFFSET_64BIT:
      return Nc3File::Offset64Bits;
      break;
    case NETCDF4:
      return Nc3File::Netcdf4;
      break;
    case NETCDF_CLASSIC:
    default:
      return Nc3File::Classic;
  }
}

/////////////////////////////////////////////////////////
// print summary after read

void NcfRadxFile::print(ostream &out) const
  
{
  
  out << "=============== NcfRadxFile ===============" << endl;
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

int NcfRadxFile::printNative(const string &path, ostream &out,
                             bool printRays, bool printData)
  
{

  _addErrStr("ERROR - NcfRadxFile::printNative");
  _addErrStr("  Native print does not apply to NetCDF file: ", path);
  _addErrStr("  Use 'ncdump' instead");
  return -1;

}

//////////////////////////////////////////////////////////////////
// interpret float and double vals, with respect to missing vals

Radx::fl64 NcfRadxFile::_checkMissingDouble(double val)

{
  if (fabs(val - Radx::missingMetaDouble) < 0.0001) {
    return Radx::missingMetaDouble;
  }
  if (val < -1.0e6) {
    return Radx::missingMetaDouble;
  }
  return val;
}

Radx::fl32 NcfRadxFile::_checkMissingFloat(float val)

{

  if (fabs(val - Radx::missingMetaFloat) < 0.0001) {
    return Radx::missingMetaFloat;
  }
  if (val < -1.0e6) {
    return Radx::missingMetaFloat;
  }
  return val;

}
