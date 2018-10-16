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
// RadxFile.cc
//
// RadxFile object
//
// Base class for radar radial data files
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2010
//
///////////////////////////////////////////////////////////////

#include <Radx/Cf2RadxFile.hh>
#include <Radx/CfarrNcRadxFile.hh>
#include <Radx/D3rNcRadxFile.hh>
#include <Radx/DoeNcRadxFile.hh>
#include <Radx/DoradeRadxFile.hh>
#include <Radx/EdgeNcRadxFile.hh>
#include <Radx/ForayNcRadxFile.hh>
#include <Radx/GamicHdf5RadxFile.hh>
#include <Radx/GemRadxFile.hh>
#include <Radx/HrdRadxFile.hh>
#include <Radx/LeoRadxFile.hh>
#include <Radx/NcxxRadxFile.hh>
#include <Radx/NcfRadxFile.hh>
#include <Radx/NexradCmdRadxFile.hh>
#include <Radx/NexradRadxFile.hh>
#include <Radx/NidsRadxFile.hh>
#include <Radx/NoaaFslRadxFile.hh>
#include <Radx/NoxpNcRadxFile.hh>
#include <Radx/NsslMrdRadxFile.hh>
#include <Radx/OdimHdf5RadxFile.hh>
#include <Radx/RapicRadxFile.hh>
#include <Radx/SigmetRadxFile.hh>
#include <Radx/TdwrRadxFile.hh>
#include <Radx/TwolfRadxFile.hh>
#include <Radx/UfRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxPath.hh>
#include <Ncxx/Hdf5xx.hh>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <sys/stat.h>
#include <sys/time.h>
using namespace std;

const char *RadxFile::PATH_SEPARATOR = "/";

//////////////
// Constructor

RadxFile::RadxFile()
        
{
  clearErrStr();
  _debug = false;
  _verbose = false;
  _fileFormat = FILE_FORMAT_CFRADIAL;
  clearRead();
  clearWrite();
}

/////////////
// destructor

RadxFile::~RadxFile()

{
}

///////////////////////////////
// clear the object of all data

void RadxFile::clear()
{
  clearErrStr();
  clearRead();
  clearWrite();
}

/////////////////////////////////////////////////////////
// Check if specified file is supported by Radx
// Returns true if supported, false otherwise

bool RadxFile::isSupported(const string &path)

{

  if (isNetCDF(path)) {

    // netCDF files
  
    if (_isSupportedNetCDF(path)) {
      return true;
    }

  } else if (isHdf5(path)) {

    // netCDF files
    
    if (_isSupportedHdf5(path)) {
      return true;
    }

  }

  // all other types

  if (_isSupportedOther(path)) {
    return true;
  }

  return false;

}

/////////////////////////////////////////////////////////
// Check if specified netcdf file is supported by Radx
// Returns true if supported, false otherwise

bool RadxFile::_isSupportedNetCDF(const string &path)

{

  // try CF radial
  {
    NcfRadxFile file;
    if (file.isCfRadial(path)) {
      return true;
    }
  }
  
  // try Cfradial2
  {
    Cf2RadxFile file;
    if (file.isCfRadial2(path)) {
      return true;
    }
  }
  
  // try CFXX radial
  {
    NcxxRadxFile file;
    if (file.isCfRadialXx(path)) {
      return true;
    }
  }
  
  // try Foray NetCDF
  {
    ForayNcRadxFile file;
    if (file.isForayNc(path)) {
      return true;
    }
  }
  
  // try DOE netcdf
  {
    DoeNcRadxFile file;
    if (file.isDoeNc(path)) {
      return true;
    }
  }
  
  // try EEC Edge netcdf
  {
    EdgeNcRadxFile file;
    if (file.isEdgeNc(path)) {
      return true;
    }
  }
 
  // try NOXP netcdf
  {
    NoxpNcRadxFile file;
    if (file.isNoxpNc(path)) {
      return true;
    }
  }
  
  // try NEXRAD CMD NetCDF
  {
    NexradCmdRadxFile file;
    if (file.isNexradCmd(path)) {
      return true;
    }
  }
  
  // try D3R netcdf
  {
    D3rNcRadxFile file;
    if (file.isD3rNc(path)) {
      return true;
    }
  }
  
  // try NOAA FSL netcdf
  {
    NoaaFslRadxFile file;
    if (file.isNoaaFsl(path)) {
      return true;
    }
  }
  
  // try Cfarr netcdf
  {
    CfarrNcRadxFile file;
    if (file.isCfarrNc(path)) {
      return true;
    }
  }
    
  return false;

}

/////////////////////////////////////////////////////////
// Check if specified hdf5 file is supported by Radx
// Returns true if supported, false otherwise

bool RadxFile::_isSupportedHdf5(const string &path)

{

  // try ODIM HDF5
  {
    OdimHdf5RadxFile file;
    if (file.isOdimHdf5(path)) {
      return true;
    }
  }
  
  // try GAMIC HDF5
  {
    GamicHdf5RadxFile file;
    if (file.isGamicHdf5(path)) {
      return true;
    }
  }
  
  return false;

}

/////////////////////////////////////////////////////////
// Check if non netcdf or hdf5 file is supported by Radx
// Returns true if supported, false otherwise

bool RadxFile::_isSupportedOther(const string &path)
  
{

  // try Dorade next
  {
    DoradeRadxFile file;
    if (file.isDorade(path)) {
      return true;
    }
  }

  // try UF next
  {
    UfRadxFile file;
    if (file.isUf(path)) {
      return true;
    }
  }

  // try NEXRAD ARCHIVE 2
  {
    NexradRadxFile file;
    if (file.isNexrad(path)) {
      return true;
    }
  }

  // try SIGMET RAW 
  {
    SigmetRadxFile file;
    if (file.isSigmet(path)) {
      return true;
    }
  }

  // try Gematronik
  {
    GemRadxFile file;
    if (file.isGematronik(path)) {
      return true;
    }
  }

  // try Leosphere LIDAR
  {
    LeoRadxFile file;
    if (file.isLeosphere(path)) {
      return true;
    }
  }

  // try Rapic
  {
    RapicRadxFile file;
    if (file.isRapic(path)) {
      return true;
    }
  }

  // try Nids
  {
    NidsRadxFile file;
    if (file.isNids(path)) {
      return true;
    }
  }

  // try HRD - hurricane research division
  {
    HrdRadxFile file;
    if (file.isHrd(path)) {
      return true;
    }
  }

  // try TDWR - terminal Doppler weather radar
  {
    TdwrRadxFile file;
    if (file.isTdwr(path)) {
      return true;
    }
  }

  // try TWOLF LIDAR
  {
    TwolfRadxFile file;
    if (file.isTwolf(path)) {
      return true;
    }
  }

  // try NSSL MRD - NSSL data from NOAA tail radars
  {
    NsslMrdRadxFile file;
    if (file.isNsslMrd(path)) {
      return true;
    }
  }

  return false;

}

////////////////////////////////////////////////////////////
// Check if path is a NetCDF file
// Returns true if file is NetCDF, or false otherwise

bool RadxFile::isNetCDF(const string &path)
{

  RadxPath rpath(path);
  if (rpath.getExt() == "h5") {
    return false;
  }

  Nc3xFile ncf;
  if (ncf.openRead(path) == 0) {
    // open succeeded, so must be netcdf
    ncf.close();
    return true;
  }

  // could not open as netcdf

  return false;

}

////////////////////////////////////////////////////////////
// Check if path is an HDF5 file
// Returns true if file is HDF5, or false otherwise

bool RadxFile::isHdf5(const string &path)
{

  RadxPath rpath(path);
  if (rpath.getExt() == "nc") {
    return false;
  }

  if (H5File::isHdf5(path)) {
    return true;
  }

  return false;

}

/////////////////////////////////////////////////////////
// clear for writing

void RadxFile::clearWrite()
{
  _writeCompressed = true;
  _compressionLevel = 5;
  _writeLdataInfo = false;
  _writeFileNameMode = FILENAME_WITH_START_AND_END_TIMES;
  _writeFileNamePrefix.clear();
  _writeFileNameSuffix.clear();
  _writeIndividualSweeps = false;
  _writeInstrNameInFileName = true;
  _writeSiteNameInFileName = false;
  _writeSubsecsInFileName = true; 
  _writeScanTypeInFileName = true;
  _writeVolNumInFileName = false;
  _writeHyphenInDateTime = false; 
  _writeNativeByteOrder = false;
  _writeForceNgatesVary = false;
  _writeProposedStdNameInNcf = false;
}

/////////////////////////////////////////////////////////
// copy the write directives from another object

void RadxFile::copyWriteDirectives(const RadxFile &other)

{

  _writeNativeByteOrder = other._writeNativeByteOrder;
  _writeForceNgatesVary = other._writeForceNgatesVary;
  _writeFileNameMode = other._writeFileNameMode;
  _writeFileNamePrefix = other._writeFileNamePrefix;
  _writeFileNameSuffix = other._writeFileNameSuffix;
  _writeIndividualSweeps = other._writeIndividualSweeps;
  _writeInstrNameInFileName = other._writeInstrNameInFileName;
  _writeSiteNameInFileName = other._writeSiteNameInFileName;
  _writeSubsecsInFileName = other._writeSubsecsInFileName; 
  _writeScanTypeInFileName = other._writeScanTypeInFileName; 
  _writeVolNumInFileName = other._writeVolNumInFileName; 
  _writeHyphenInDateTime = other._writeHyphenInDateTime; 
  _writeCompressed = other._writeCompressed;
  _compressionLevel = other._compressionLevel;
  _writeLdataInfo = other._writeLdataInfo;
  _writeProposedStdNameInNcf = other._writeProposedStdNameInNcf;
  _ncFormat = other._ncFormat;
  _debug = other._debug;
  _verbose = other._verbose;

}

/////////////////////////////////////////////////////////
// copy the read directives from another object

void RadxFile::copyReadDirectives(const RadxFile &other)

{

  _readFieldNames = other._readFieldNames;
  _readFixedAngleLimitsSet = other._readFixedAngleLimitsSet;
  _readMinFixedAngle = other._readMinFixedAngle;
  _readMaxFixedAngle = other._readMaxFixedAngle;
  _readSweepNumLimitsSet = other._readSweepNumLimitsSet;
  _readMinSweepNum = other._readMinSweepNum;
  _readMaxSweepNum = other._readMaxSweepNum;
  _readStrictAngleLimits = other._readStrictAngleLimits;
  _readAggregateSweeps = other._readAggregateSweeps;
  _readIgnoreIdleMode = other._readIgnoreIdleMode;
  _readIgnoreTransitions = other._readIgnoreTransitions;
  _readTransitionNraysMargin = other._readTransitionNraysMargin;
  _readRemoveRaysAllMissing = other._readRemoveRaysAllMissing;
  _readSetMaxRange = other._readSetMaxRange;
  _readMaxRangeKm = other._readMaxRangeKm;
  _readRemoveLongRange = other._readRemoveLongRange;
  _readPreserveSweeps = other._readPreserveSweeps;
  _readComputeSweepAnglesFromVcpTables = other._readComputeSweepAnglesFromVcpTables;
  _readRemoveShortRange = other._readRemoveShortRange;
  _readMetadataOnly = other._readMetadataOnly;
  _readTimesOnly = other._readTimesOnly;
  _readSetRadarNum = other._readSetRadarNum;
  _readRadarNum = other._readRadarNum;
  _readChangeLatitudeSign = other._readChangeLatitudeSign;
  _readApplyGeorefs = other._readApplyGeorefs;
  _readRaysInInterval = other._readRaysInInterval;
  _readRaysStartTime = other._readRaysStartTime;
  _readRaysEndTime = other._readRaysEndTime;
  _readDwellSecs = other._readDwellSecs;
  _readDwellStatsMethod = other._readDwellStatsMethod;
  _debug = other._debug;
  _verbose = other._verbose;

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

int RadxFile::writeToDir(const RadxVol &vol,
                         const string &dir,
                         bool addDaySubDir,
                         bool addYearSubDir)
  
{

  if (vol.getNRays() < 1) {
    _addErrStr("ERROR - RadxFile::writeToDir");
    _addErrStr("  Output dir: ", dir);
    _addErrStr("  No rays in file, time: ", RadxTime::strm(vol.getStartTimeSecs()));
    return -1;
  }

  int iret = 0;

  if (_fileFormat == FILE_FORMAT_CFRADIAL ||
      _fileFormat == FILE_FORMAT_SIGMET_RAW ||
      _fileFormat == FILE_FORMAT_DOE_NC ||
      _fileFormat == FILE_FORMAT_NOXP_NC ||
      _fileFormat == FILE_FORMAT_D3R_NC ||
      _fileFormat == FILE_FORMAT_NOAA_FSL ||
      _fileFormat == FILE_FORMAT_NEXRAD_CMD ||
      _fileFormat == FILE_FORMAT_LEOSPHERE ||
      _fileFormat == FILE_FORMAT_TWOLF ||
      _fileFormat == FILE_FORMAT_HRD ||
      _fileFormat == FILE_FORMAT_EDGE_NC) {

    // CF radial
    
    if (_debug) {
      cerr << "INFO: RadxFile::writeToDir" << endl;
      cerr << "  Writing CfRadial file to dir: " << dir << endl;
    }

    NcfRadxFile file;
    file.copyWriteDirectives(*this);
    iret = file.writeToDir(vol, dir, addDaySubDir, addYearSubDir);
    _errStr = file.getErrStr();
    _dirInUse = file.getDirInUse();
    _pathInUse = file.getPathInUse();
    vol.setPathInUse(_pathInUse);
    _writePaths = file.getWritePaths();
    _writeDataTimes = file.getWriteDataTimes();

    if (_debug) {
      if (iret) {
        cerr << file.getErrStr() << endl;
      } else {
        cerr << "INFO: RadxFile::writeToDir" << endl;
        cerr << "  Wrote CfRadial file to path: " << _pathInUse << endl;
      }
    }

  } else if (_fileFormat == FILE_FORMAT_CFRADIAL2) {

    // CfRadial2
    
    if (_debug) {
      cerr << "INFO: RadxFile::writeToDir" << endl;
      cerr << "  Writing CFRadial2 file to dir: " << dir << endl;
    }

    Cf2RadxFile file;
    file.copyWriteDirectives(*this);
    iret = file.writeToDir(vol, dir, addDaySubDir, addYearSubDir);
    _errStr = file.getErrStr();
    _dirInUse = file.getDirInUse();
    _pathInUse = file.getPathInUse();
    vol.setPathInUse(_pathInUse);
    _writePaths = file.getWritePaths();
    _writeDataTimes = file.getWriteDataTimes();

    if (_debug) {
      if (iret) {
        cerr << file.getErrStr() << endl;
      } else {
        cerr << "INFO: RadxFile::writeToDir" << endl;
        cerr << "  Wrote CFRadial2 file to path: " << _pathInUse << endl;
      }
    }

  } else if (_fileFormat == FILE_FORMAT_NCXX) {

    // CfRadial using Ncxx interface
    
    if (_debug) {
      cerr << "INFO: RadxFile::writeToDir" << endl;
      cerr << "  Writing Ncxx file to dir: " << dir << endl;
    }

    NcxxRadxFile file;
    file.copyWriteDirectives(*this);
    iret = file.writeToDir(vol, dir, addDaySubDir, addYearSubDir);
    _errStr = file.getErrStr();
    _dirInUse = file.getDirInUse();
    _pathInUse = file.getPathInUse();
    vol.setPathInUse(_pathInUse);
    _writePaths = file.getWritePaths();
    _writeDataTimes = file.getWriteDataTimes();

    if (_debug) {
      if (iret) {
        cerr << file.getErrStr() << endl;
      } else {
        cerr << "INFO: RadxFile::writeToDir" << endl;
        cerr << "  Wrote Ncxx file to path: " << _pathInUse << endl;
      }
    }

  } else if (_fileFormat == FILE_FORMAT_FORAY_NC) {

    // FORAY 1
    
    if (_debug) {
      cerr << "INFO: RadxFile::writeToDir" << endl;
      cerr << "  Writing FORAY file to dir: " << dir << endl;
    }

    ForayNcRadxFile file;
    file.copyWriteDirectives(*this);
    iret = file.writeToDir(vol, dir, addDaySubDir, addYearSubDir);
    _errStr = file.getErrStr();
    _dirInUse = file.getDirInUse();
    _pathInUse = file.getPathInUse();
    vol.setPathInUse(_pathInUse);
    _writePaths = file.getWritePaths();
    _writeDataTimes = file.getWriteDataTimes();

    if (_debug) {
      if (iret) {
        cerr << file.getErrStr() << endl;
      } else {
        cerr << "INFO: RadxFile::writeToDir" << endl;
        cerr << "  Wrote FORAY file to path: " << _pathInUse << endl;
      }
    }

  } else if (_fileFormat == FILE_FORMAT_DORADE) {

    // Dorade
    
    if (_debug) {
      cerr << "INFO: RadxFile::writeToDir" << endl;
      cerr << "  Writing Dorade file to dir: " << dir << endl;
    }

    DoradeRadxFile file;
    file.copyWriteDirectives(*this);
    iret = file.writeToDir(vol, dir, addDaySubDir, addYearSubDir);
    _errStr = file.getErrStr();
    _dirInUse = file.getDirInUse();
    _pathInUse = file.getPathInUse();
    vol.setPathInUse(_pathInUse);
    _writePaths = file.getWritePaths();
    _writeDataTimes = file.getWriteDataTimes();

    if (_debug) {
      if (iret) {
        cerr << file.getErrStr() << endl;
      } else {
        cerr << "INFO: RadxFile::writeToDir" << endl;
        cerr << "  Wrote Dorade file to path: " << _pathInUse << endl;
      }
    }

  } else if (_fileFormat == FILE_FORMAT_UF) {

    // UF
    
    if (_debug) {
      cerr << "INFO: RadxFile::writeToDir" << endl;
      cerr << "  Writing UF file to dir: " << dir << endl;
    }

    UfRadxFile file;
    file.copyWriteDirectives(*this);
    iret = file.writeToDir(vol, dir, addDaySubDir, addYearSubDir);
    _errStr = file.getErrStr();
    _dirInUse = file.getDirInUse();
    _pathInUse = file.getPathInUse();
    vol.setPathInUse(_pathInUse);
    _writePaths = file.getWritePaths();
    _writeDataTimes = file.getWriteDataTimes();
    
    if (_debug) {
      if (iret) {
        cerr << file.getErrStr() << endl;
      } else {
        cerr << "INFO: RadxFile::writeToDir" << endl;
        cerr << "  Wrote UF file to path: " << _pathInUse << endl;
      }
    }

  } else if (_fileFormat == FILE_FORMAT_NEXRAD_AR2) {

    // NEXRAD level 2
    
    if (_debug) {
      cerr << "INFO: RadxFile::writeToDir" << endl;
      cerr << "  Writing NEXRAD file to dir: " << dir << endl;
    }
    
    NexradRadxFile file;
    file.copyWriteDirectives(*this);
    iret = file.writeToDir(vol, dir, addDaySubDir, addYearSubDir);
    _errStr = file.getErrStr();
    _dirInUse = file.getDirInUse();
    _pathInUse = file.getPathInUse();
    vol.setPathInUse(_pathInUse);
    _writePaths = file.getWritePaths();
    _writeDataTimes = file.getWriteDataTimes();
    
    if (_debug) {
      if (iret) {
        cerr << file.getErrStr() << endl;
      } else {
        cerr << "INFO: RadxFile::writeToDir" << endl;
        cerr << "  Wrote NEXRAD file to path: " << _pathInUse << endl;
      }
    }

  } else if (_fileFormat == FILE_FORMAT_NSSL_MRD) {

    // NSSL MRD airborne data
    
    if (_debug) {
      cerr << "INFO: RadxFile::writeToDir" << endl;
      cerr << "  Writing NSSL MRD file to dir: " << dir << endl;
    }
    
    NsslMrdRadxFile file;
    file.copyWriteDirectives(*this);
    iret = file.writeToDir(vol, dir, addDaySubDir, addYearSubDir);
    _errStr = file.getErrStr();
    _dirInUse = file.getDirInUse();
    _pathInUse = file.getPathInUse();
    vol.setPathInUse(_pathInUse);
    _writePaths = file.getWritePaths();
    _writeDataTimes = file.getWriteDataTimes();
    
    if (_debug) {
      if (iret) {
        cerr << file.getErrStr() << endl;
      } else {
        cerr << "INFO: RadxFile::writeToDir" << endl;
        cerr << "  Wrote NSSL MRD file to path: " << _pathInUse << endl;
      }
    }

  } else if (_fileFormat == FILE_FORMAT_ODIM_HDF5) {

    // NSSL MRD airborne data
    
    if (_debug) {
      cerr << "INFO: RadxFile::writeToDir" << endl;
      cerr << "  Writing ODIM HDF5 file to dir: " << dir << endl;
    }
    
    OdimHdf5RadxFile file;
    file.copyWriteDirectives(*this);
    iret = file.writeToDir(vol, dir, addDaySubDir, addYearSubDir);
    _errStr = file.getErrStr();
    _dirInUse = file.getDirInUse();
    _pathInUse = file.getPathInUse();
    vol.setPathInUse(_pathInUse);
    _writePaths = file.getWritePaths();
    _writeDataTimes = file.getWriteDataTimes();
    
    if (_debug) {
      if (iret) {
        cerr << file.getErrStr() << endl;
      } else {
        cerr << "INFO: RadxFile::writeToDir" << endl;
        cerr << "  Wrote ODIM HDF5 file to path: " << _pathInUse << endl;
      }
    }

  } else {

    _addErrStr("ERROR - RadxFile::writeToDir");
    _addErrInt("  File format not recognized: ", (int) _fileFormat);
    iret = -1;

  }

  return iret;
    
}
  
/////////////////////////////////////////////////////////
// Write data from volume to specified path
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs
// Use getPathInUse() for path written

int RadxFile::writeToPath(const RadxVol &vol,
                          const string &path)

{

  if (vol.getNRays() < 1) {
    _addErrStr("ERROR - RadxFile::writeToPath");
    _addErrStr("  Output path: ", path);
    _addErrStr("  No rays in file, time: ", RadxTime::strm(vol.getStartTimeSecs()));
    return -1;
  }

  int iret = 0;

  if (_fileFormat == FILE_FORMAT_CFRADIAL ||
      _fileFormat == FILE_FORMAT_SIGMET_RAW ||
      _fileFormat == FILE_FORMAT_DOE_NC ||
      _fileFormat == FILE_FORMAT_NOXP_NC ||
      _fileFormat == FILE_FORMAT_D3R_NC ||
      _fileFormat == FILE_FORMAT_NOAA_FSL ||
      _fileFormat == FILE_FORMAT_NEXRAD_CMD ||
      _fileFormat == FILE_FORMAT_LEOSPHERE ||
      _fileFormat == FILE_FORMAT_TWOLF ||
      _fileFormat == FILE_FORMAT_HRD ||
      _fileFormat == FILE_FORMAT_EDGE_NC) {

    // CF radial
    
    if (_debug) {
      cerr << "INFO: RadxFile::writeToPath" << endl;
      cerr << "  Writing CfRadial file to path: " << path << endl;
    }

    NcfRadxFile file;
    file.copyWriteDirectives(*this);
    iret = file.writeToPath(vol, path);
    _errStr = file.getErrStr();
    _dirInUse = file.getDirInUse();
    _pathInUse = file.getPathInUse();
    vol.setPathInUse(_pathInUse);
    _writePaths = file.getWritePaths();
    _writeDataTimes = file.getWriteDataTimes();
      
    if (_debug) {
      if (iret) {
        cerr << file.getErrStr() << endl;
      } else {
        cerr << "INFO: RadxFile::writeToPath" << endl;
        cerr << "  Wrote CfRadial file to path: " << _pathInUse << endl;
      }
    }

  } else if (_fileFormat == FILE_FORMAT_CFRADIAL2) {

    // CfRadial2
    
    if (_debug) {
      cerr << "INFO: RadxFile::writeToPath" << endl;
      cerr << "  Writing CFRadial2 file to path: " << path << endl;
    }

    Cf2RadxFile file;
    file.copyWriteDirectives(*this);
    iret = file.writeToPath(vol, path);
    _errStr = file.getErrStr();
    _dirInUse = file.getDirInUse();
    _pathInUse = file.getPathInUse();
    vol.setPathInUse(_pathInUse);
    _writePaths = file.getWritePaths();
    _writeDataTimes = file.getWriteDataTimes();

    if (_debug) {
      if (iret) {
        cerr << file.getErrStr() << endl;
      } else {
        cerr << "INFO: RadxFile::writeToPath" << endl;
        cerr << "  Wrote CFRadial2 file to path: " << _pathInUse << endl;
      }
    }

  } else if (_fileFormat == FILE_FORMAT_NCXX) {

    // CfRadial using Ncxx interface
    
    if (_debug) {
      cerr << "INFO: RadxFile::writeToPath" << endl;
      cerr << "  Writing Ncxx file to path: " << path << endl;
    }

    NcxxRadxFile file;
    file.copyWriteDirectives(*this);
    iret = file.writeToPath(vol, path);
    _errStr = file.getErrStr();
    _dirInUse = file.getDirInUse();
    _pathInUse = file.getPathInUse();
    vol.setPathInUse(_pathInUse);
    _writePaths = file.getWritePaths();
    _writeDataTimes = file.getWriteDataTimes();

    if (_debug) {
      if (iret) {
        cerr << file.getErrStr() << endl;
      } else {
        cerr << "INFO: RadxFile::writeToPath" << endl;
        cerr << "  Wrote Ncxx file to path: " << _pathInUse << endl;
      }
    }

  } else if (_fileFormat == FILE_FORMAT_FORAY_NC) {

    // FORAY 1
    
    if (_debug) {
      cerr << "INFO: RadxFile::writeToPath" << endl;
      cerr << "  Writing FORAY file to path: " << path << endl;
    }

    ForayNcRadxFile file;
    file.copyWriteDirectives(*this);
    iret = file.writeToPath(vol, path);
    _errStr = file.getErrStr();
    _dirInUse = file.getDirInUse();
    _pathInUse = file.getPathInUse();
    vol.setPathInUse(_pathInUse);
    _writePaths = file.getWritePaths();
    _writeDataTimes = file.getWriteDataTimes();

    if (_debug) {
      if (iret) {
        cerr << file.getErrStr() << endl;
      } else {
        cerr << "INFO: RadxFile::writeToPath" << endl;
        cerr << "  Wrote FORAY file to path: " << _pathInUse << endl;
      }
    }

  } else if (_fileFormat == FILE_FORMAT_DORADE) {

    // Dorade
    
    if (_debug) {
      cerr << "INFO: RadxFile::writeToPath" << endl;
      cerr << "  Writing Dorade file to path: " << path << endl;
    }

    DoradeRadxFile file;
    file.copyWriteDirectives(*this);
    iret = file.writeToPath(vol, path);
    _errStr = file.getErrStr();
    _dirInUse = file.getDirInUse();
    _pathInUse = file.getPathInUse();
    vol.setPathInUse(_pathInUse);
    _writePaths = file.getWritePaths();
    _writeDataTimes = file.getWriteDataTimes();

    if (_debug) {
      if (iret) {
        cerr << file.getErrStr() << endl;
      } else {
        cerr << "INFO: RadxFile::writeToPath" << endl;
        cerr << "  Wrote Dorade file to path: " << _pathInUse << endl;
      }
    }

  } else if (_fileFormat == FILE_FORMAT_UF) {

    // UF
    
    if (_debug) {
      cerr << "INFO: RadxFile::writeToPath" << endl;
      cerr << "  Writing UF file to path: " << path << endl;
    }

    UfRadxFile file;
    file.copyWriteDirectives(*this);
    iret = file.writeToPath(vol, path);
    _errStr = file.getErrStr();
    _dirInUse = file.getDirInUse();
    _pathInUse = file.getPathInUse();
    vol.setPathInUse(_pathInUse);
    _writePaths = file.getWritePaths();
    _writeDataTimes = file.getWriteDataTimes();

    if (_debug) {
      if (iret) {
        cerr << file.getErrStr() << endl;
      } else {
        cerr << "INFO: RadxFile::writeToPath" << endl;
        cerr << "  Wrote UF file to path: " << _pathInUse << endl;
      }
    }

  } else if (_fileFormat == FILE_FORMAT_NEXRAD_AR2) {

    // NEXRAD level 2
    
    if (_debug) {
      cerr << "INFO: RadxFile::writeToPath" << endl;
      cerr << "  Writing NEXRAD file to path: " << path << endl;
    }

    NexradRadxFile file;
    file.copyWriteDirectives(*this);
    iret = file.writeToPath(vol, path);
    _errStr = file.getErrStr();
    _dirInUse = file.getDirInUse();
    _pathInUse = file.getPathInUse();
    vol.setPathInUse(_pathInUse);
    _writePaths = file.getWritePaths();
    _writeDataTimes = file.getWriteDataTimes();

    if (_debug) {
      if (iret) {
        cerr << file.getErrStr() << endl;
      } else {
        cerr << "INFO: RadxFile::writeToPath" << endl;
        cerr << "  Wrote NEXRAD file to path: " << _pathInUse << endl;
      }
    }

  } else {

    _addErrStr("ERROR - RadxFile::writeToPath");
    _addErrInt("  File format not recognized: ", (int) _fileFormat);
    iret = -1;

  }

  return iret;
    
}

/////////////////////////////////////////////////////////
// Read in data file from specified path,
// load up volume object.
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int RadxFile::readFromPath(const string &path,
                           RadxVol &vol)

{

  clearErrStr();

  // first check for NetCDF

  if (isNetCDF(path)) {
    if (_readFromPathNetCDF(path, vol) == 0) {
      return 0;
    } else {
      return -1;
    }
  }

  // then check for HDF5

  if (isHdf5(path)) {
    if (_readFromPathHdf5(path, vol) == 0) {
      return 0;
    } else {
      return -1;
    }
  }

  // else fall through to other file types

  if (_readFromPathOther(path, vol) == 0) {
    return 0;
  }

  // do not recognize file type
  
  _addErrStr("ERROR - RadxFile::readFromPath");
  _addErrStr("  File format not recognized: ", path);
  return -1;

}

/////////////////////////////////////////////////////////
// Read in data file from specified netCDF path,
// load up volume object.
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int RadxFile::_readFromPathNetCDF(const string &path,
                                  RadxVol &vol)
  
{

  // try CF radial first

  {
    NcfRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isCfRadial(path)) {
      int iret = file.readFromPath(path, vol);
      if (_verbose) file.print(cerr);
      _errStr = file.getErrStr();
      _dirInUse = file.getDirInUse();
      _pathInUse = file.getPathInUse();
      vol.setPathInUse(_pathInUse);
      _readPaths = file.getReadPaths();
      if (iret == 0) {
        if (_debug) {
          cerr << "INFO: RadxFile::readFromPath" << endl;
          cerr << "  Read CfRadial file, path: " << _pathInUse << endl;
        }
      } else if (_verbose) {
        cerr << "===>> ERROR in CfRadial file <<===" << endl;
        cerr << file.getErrStr() << endl;
        cerr << "===>> ERROR in CfRadial file <<===" << endl;
      }
      return iret;
    } else {
      if (_verbose) {
        cerr << "Not CfRadial format" << endl;
        cerr << file.getErrStr() << endl;
      }
    }
  }

  // try CFRadial2 next

  {
    Cf2RadxFile file;
    file.copyReadDirectives(*this);
    if (file.isCfRadial2(path)) {
      int iret = file.readFromPath(path, vol);
      if (_verbose) file.print(cerr);
      _errStr = file.getErrStr();
      _dirInUse = file.getDirInUse();
      _pathInUse = file.getPathInUse();
      vol.setPathInUse(_pathInUse);
      _readPaths = file.getReadPaths();
      if (iret == 0) {
        if (_debug) {
          cerr << "INFO: RadxFile::readFromPath" << endl;
          cerr << "  Read CfRadial2 file, path: " << _pathInUse << endl;
        }
      }
      return iret;
    }
  }

  // try CF Ncxx next

  {
    NcxxRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isCfRadialXx(path)) {
      int iret = file.readFromPath(path, vol);
      if (_verbose) file.print(cerr);
      _errStr = file.getErrStr();
      _dirInUse = file.getDirInUse();
      _pathInUse = file.getPathInUse();
      vol.setPathInUse(_pathInUse);
      _readPaths = file.getReadPaths();
      if (iret == 0) {
        if (_debug) {
          cerr << "INFO: RadxFile::readFromPath" << endl;
          cerr << "  Read Ncxx file, path: " << _pathInUse << endl;
        }
      }
      return iret;
    }
  }

  // try Foray NetCDF next

  {
    ForayNcRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isForayNc(path)) {
      int iret = file.readFromPath(path, vol);
      if (_verbose) file.print(cerr);
      _errStr = file.getErrStr();
      _dirInUse = file.getDirInUse();
      _pathInUse = file.getPathInUse();
      vol.setPathInUse(_pathInUse);
      _readPaths = file.getReadPaths();
      if (iret == 0) {
        if (_debug) {
          cerr << "INFO: RadxFile::readFromPath" << endl;
          cerr << "  Read Foray NC file, path: " << _pathInUse << endl;
        }
      }
      return iret;
    }
  }

  // try DOE netcdf next

  {
    DoeNcRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isDoeNc(path)) {
      int iret = file.readFromPath(path, vol);
      if (_verbose) file.print(cerr);
      _errStr = file.getErrStr();
      _dirInUse = file.getDirInUse();
      _pathInUse = file.getPathInUse();
      vol.setPathInUse(_pathInUse);
      _readPaths = file.getReadPaths();
      if (iret == 0) {
        if (_debug) {
          cerr << "INFO: RadxFile::readFromPath" << endl;
          cerr << "  Read DOE NC file, path: " << _pathInUse << endl;
        }
      }
      return iret;
    }
  }

  // try NOXP netcdf next

  {
    NoxpNcRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isNoxpNc(path)) {
      int iret = file.readFromPath(path, vol);
      if (_verbose) file.print(cerr);
      _errStr = file.getErrStr();
      _dirInUse = file.getDirInUse();
      _pathInUse = file.getPathInUse();
      vol.setPathInUse(_pathInUse);
      _readPaths = file.getReadPaths();
      if (iret == 0) {
        if (_debug) {
          cerr << "INFO: RadxFile::readFromPath" << endl;
          cerr << "  Read NOXP NC file, path: " << _pathInUse << endl;
        }
      }
      return iret;
    }
  }

  // try D3R netcdf next

  {
    D3rNcRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isD3rNc(path)) {
      int iret = file.readFromPath(path, vol);
      if (_verbose) file.print(cerr);
      _errStr = file.getErrStr();
      _dirInUse = file.getDirInUse();
      _pathInUse = file.getPathInUse();
      vol.setPathInUse(_pathInUse);
      _readPaths = file.getReadPaths();
      if (iret == 0) {
        if (_debug) {
          cerr << "INFO: RadxFile::readFromPath" << endl;
          cerr << "  Read D3R NC file, path: " << _pathInUse << endl;
        }
      }
      return iret;
    }
  }

  // try NOAA FSL netcdf next

  {
    NoaaFslRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isNoaaFsl(path)) {
      int iret = file.readFromPath(path, vol);
      if (_verbose) file.print(cerr);
      _errStr = file.getErrStr();
      _dirInUse = file.getDirInUse();
      _pathInUse = file.getPathInUse();
      vol.setPathInUse(_pathInUse);
      _readPaths = file.getReadPaths();
      if (iret == 0) {
        if (_debug) {
          cerr << "INFO: RadxFile::readFromPath" << endl;
          cerr << "  Read NOAA FSL NC file, path: " << _pathInUse << endl;
        }
      }
      return iret;
    }
  }

  // try NEXRAD CMD next
  
  {
    NexradCmdRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isNexradCmd(path)) {
      int iret = file.readFromPath(path, vol);
      if (_verbose) file.print(cerr);
      _errStr = file.getErrStr();
      _dirInUse = file.getDirInUse();
      _pathInUse = file.getPathInUse();
      vol.setPathInUse(_pathInUse);
      _readPaths = file.getReadPaths();
      if (iret == 0) {
        if (_debug) {
          cerr << "INFO: RadxFile::readFromPath" << endl;
          cerr << "  Read NEXRAD CMD file, path: " << _pathInUse << endl;
        }
      }
      return iret;
    }
  }

  // try EEC Edge netcdf next

  {
    EdgeNcRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isEdgeNc(path)) {
      int iret = file.readFromPath(path, vol);
      if (_verbose) file.print(cerr);
      _errStr = file.getErrStr();
      _dirInUse = file.getDirInUse();
      _pathInUse = file.getPathInUse();
      vol.setPathInUse(_pathInUse);
      _readPaths = file.getReadPaths();
      if (iret == 0) {
        if (_debug) {
          cerr << "INFO: RadxFile::readFromPath" << endl;
          cerr << "  Read DOE NC file, path: " << _pathInUse << endl;
        }
      }
      return iret;
    }
  }

  // try Cfarr netcdf next

  {
    CfarrNcRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isCfarrNc(path)) {
      int iret = file.readFromPath(path, vol);
      if (_verbose) file.print(cerr);
      _errStr = file.getErrStr();
      _dirInUse = file.getDirInUse();
      _pathInUse = file.getPathInUse();
      vol.setPathInUse(_pathInUse);
      _readPaths = file.getReadPaths();
      if (iret == 0) {
        if (_debug) {
          cerr << "INFO: RadxFile::readFromPath" << endl;
          cerr << "  Read Cfarr NC file, path: " << _pathInUse << endl;
        }
      }
      return iret;
    }
  }

  // do not recognize file type
  
  return -1;

}

/////////////////////////////////////////////////////////
// Read in data file from specified HDF5 path,
// load up volume object.
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int RadxFile::_readFromPathHdf5(const string &path,
                                RadxVol &vol)
  
{

  // try ODIM HDF5 next

  {
    OdimHdf5RadxFile file;
    file.copyReadDirectives(*this);
    if (file.isOdimHdf5(path)) {
      int iret = file.readFromPath(path, vol);
      if (_verbose) file.print(cerr);
      _errStr = file.getErrStr();
      _dirInUse = file.getDirInUse();
      _pathInUse = file.getPathInUse();
      vol.setPathInUse(_pathInUse);
      _readPaths = file.getReadPaths();
      if (iret == 0) {
        if (_debug) {
          cerr << "INFO: RadxFile::readFromPath" << endl;
          cerr << "  Read ODIM HDF5 file, path: " << _pathInUse << endl;
        }
      }
      return iret;
    }
  }

  // try GAMIC HDF5 next

  {
    GamicHdf5RadxFile file;
    file.copyReadDirectives(*this);
    if (file.isGamicHdf5(path)) {
      int iret = file.readFromPath(path, vol);
      if (_verbose) file.print(cerr);
      _errStr = file.getErrStr();
      _dirInUse = file.getDirInUse();
      _pathInUse = file.getPathInUse();
      vol.setPathInUse(_pathInUse);
      _readPaths = file.getReadPaths();
      if (iret == 0) {
        if (_debug) {
          cerr << "INFO: RadxFile::readFromPath" << endl;
          cerr << "  Read GAMIC HDF5 file, path: " << _pathInUse << endl;
        }
      }
      return iret;
    }
  }

  // do not recognize file type
  
  return -1;

}

/////////////////////////////////////////////////////////
// Read in data file from non netcdf and hdf5 files
// load up volume object.
// Returns 0 on success, -1 on failure

int RadxFile::_readFromPathOther(const string &path,
                                 RadxVol &vol)

{

  // try Dorade next
  
  {
    DoradeRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isDorade(path)) {
      int iret = file.readFromPath(path, vol);
      if (_verbose) file.print(cerr);
      _errStr = file.getErrStr();
      _dirInUse = file.getDirInUse();
      _pathInUse = file.getPathInUse();
      vol.setPathInUse(_pathInUse);
      _readPaths = file.getReadPaths();
      if (iret == 0) {
        if (_debug) {
          cerr << "INFO: RadxFile::readFromPath" << endl;
          cerr << "  Read Dorade file, path: " << _pathInUse << endl;
        }
      }
     return iret;
    }
  }

  // try UF next

  {
    UfRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isUf(path)) {
      int iret = file.readFromPath(path, vol);
      if (_verbose) file.print(cerr);
      _errStr = file.getErrStr();
      _dirInUse = file.getDirInUse();
      _pathInUse = file.getPathInUse();
      vol.setPathInUse(_pathInUse);
      _readPaths = file.getReadPaths();
      if (iret == 0) {
        if (_debug) {
          cerr << "INFO: RadxFile::readFromPath" << endl;
          cerr << "  Read UF file, path: " << _pathInUse << endl;
        }
      }
      return iret;
    }
  }

  // try NEXRAD next

  {
    NexradRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isNexrad(path)) {
      int iret = file.readFromPath(path, vol);
      if (_verbose) file.print(cerr);
      _errStr = file.getErrStr();
      _dirInUse = file.getDirInUse();
      _pathInUse = file.getPathInUse();
      vol.setPathInUse(_pathInUse);
      _readPaths = file.getReadPaths();
      if (iret == 0) {
        if (_debug) {
          cerr << "INFO: RadxFile::readFromPath" << endl;
          cerr << "  Read NEXRAD file, path: " << _pathInUse << endl;
        }
      }
      return iret;
    }
  }

  // try SIGMET RAW next

  {
    SigmetRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isSigmet(path)) {
      int iret = file.readFromPath(path, vol);
      if (_verbose) file.print(cerr);
      _errStr = file.getErrStr();
      _dirInUse = file.getDirInUse();
      _pathInUse = file.getPathInUse();
      vol.setPathInUse(_pathInUse);
      _readPaths = file.getReadPaths();
      if (iret == 0) {
        if (_debug) {
          cerr << "INFO: RadxFile::readFromPath" << endl;
          cerr << "  Read SIGMET RAW file, path: " << _pathInUse << endl;
        }
      }
      return iret;
    }
  }

  // try Gematronik RAW next

  {
    GemRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isGematronik(path)) {
      int iret = file.readFromPath(path, vol);
      if (_verbose) file.print(cerr);
      _errStr = file.getErrStr();
      _dirInUse = file.getDirInUse();
      _pathInUse = file.getPathInUse();
      vol.setPathInUse(_pathInUse);
      _readPaths = file.getReadPaths();
      if (iret == 0) {
        if (_debug) {
          cerr << "INFO: RadxFile::readFromPath" << endl;
          cerr << "  Read GEMATRONIK VOL file, path: " << _pathInUse << endl;
        }
      }
      return iret;
    }
  }

  // try Leosphere next

  {
    LeoRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isLeosphere(path)) {
      int iret = file.readFromPath(path, vol);
      if (_verbose) file.print(cerr);
      _errStr = file.getErrStr();
      _dirInUse = file.getDirInUse();
      _pathInUse = file.getPathInUse();
      vol.setPathInUse(_pathInUse);
      _readPaths = file.getReadPaths();
      if (iret == 0) {
        if (_debug) {
          cerr << "INFO: RadxFile::readFromPath" << endl;
          cerr << "  Read LEOSPHERE file, path: " << _pathInUse << endl;
        }
      }
      return iret;
    }
  }

  // try RAPIC next

  {
    RapicRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isRapic(path)) {
      int iret = file.readFromPath(path, vol);
      if (_verbose) file.print(cerr);
      _errStr = file.getErrStr();
      _dirInUse = file.getDirInUse();
      _pathInUse = file.getPathInUse();
      vol.setPathInUse(_pathInUse);
      _readPaths = file.getReadPaths();
      if (iret == 0) {
        if (_debug) {
          cerr << "INFO: RadxFile::readFromPath" << endl;
          cerr << "  Read RAPIC file, path: " << _pathInUse << endl;
        }
      }
      return iret;
    }
  }

  // try NIDS next

  {
    NidsRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isNids(path)) {
      int iret = file.readFromPath(path, vol);
      if (_verbose) file.print(cerr);
      _errStr = file.getErrStr();
      _dirInUse = file.getDirInUse();
      _pathInUse = file.getPathInUse();
      vol.setPathInUse(_pathInUse);
      _readPaths = file.getReadPaths();
      if (iret == 0) {
        if (_debug) {
          cerr << "INFO: RadxFile::readFromPath" << endl;
          cerr << "  Read NIDS file, path: " << _pathInUse << endl;
        }
      }
      return iret;
    }
  }

  // try HRD next

  {
    HrdRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isHrd(path)) {
      int iret = file.readFromPath(path, vol);
      if (_verbose) file.print(cerr);
      _errStr = file.getErrStr();
      _dirInUse = file.getDirInUse();
      _pathInUse = file.getPathInUse();
      vol.setPathInUse(_pathInUse);
      _readPaths = file.getReadPaths();
      if (iret == 0) {
        if (_debug) {
          cerr << "INFO: RadxFile::readFromPath" << endl;
          cerr << "  Read HRD file, path: " << _pathInUse << endl;
        }
      }
      return iret;
    }
  }

  // try TDWR next

  {
    TdwrRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isTdwr(path)) {
      int iret = file.readFromPath(path, vol);
      if (_verbose) file.print(cerr);
      _errStr = file.getErrStr();
      _dirInUse = file.getDirInUse();
      _pathInUse = file.getPathInUse();
      vol.setPathInUse(_pathInUse);
      _readPaths = file.getReadPaths();
      if (iret == 0) {
        if (_debug) {
          cerr << "INFO: RadxFile::readFromPath" << endl;
          cerr << "  Read TDWR file, path: " << _pathInUse << endl;
        }
      }
      return iret;
    }
  }
  
  // try TWOLF next

  {
    TwolfRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isTwolf(path)) {
      int iret = file.readFromPath(path, vol);
      if (_verbose) file.print(cerr);
      _errStr = file.getErrStr();
      _dirInUse = file.getDirInUse();
      _pathInUse = file.getPathInUse();
      vol.setPathInUse(_pathInUse);
      _readPaths = file.getReadPaths();
      if (iret == 0) {
        if (_debug) {
          cerr << "INFO: RadxFile::readFromPath" << endl;
          cerr << "  Read TWOLF file, path: " << _pathInUse << endl;
        }
      }
      return iret;
    }
  }

  // try NSSL MRD - NSSL data from NOAA tail radars

  {
    NsslMrdRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isNsslMrd(path)) {
      int iret = file.readFromPath(path, vol);
      if (_verbose) file.print(cerr);
      _errStr = file.getErrStr();
      _dirInUse = file.getDirInUse();
      _pathInUse = file.getPathInUse();
      vol.setPathInUse(_pathInUse);
      _readPaths = file.getReadPaths();
      if (iret == 0) {
        if (_debug) {
          cerr << "INFO: RadxFile::readFromPath" << endl;
          cerr << "  Read NSSL MRD file, path: " << _pathInUse << endl;
        }
      }
      return iret;
    }
  }

  return -1;

}

//////////////////////////////////////////////////////////
// Read in data file from a series of specified paths and
// aggregate into a single volume object.
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int RadxFile::aggregateFromPaths(const vector<string> &paths,
                                 RadxVol &vol)
  
{

  // sanity check

  if (paths.size() < 1) {
    _addErrStr("ERROR - RadxFile::aggregateFromPaths");
    _addErrStr("  No files specified");
    return -1;
  }

  // read from first path
  
  if (readFromPath(paths[0], vol)) {
    _addErrStr("ERROR - RadxFile::aggregateFromPaths");
    return -1;
  }
  
  // set sweep numbers

  int sweepNum = 1;
  {
    const vector<RadxSweep *> &sweeps = vol.getSweeps();
    const vector<RadxRay *> &rays = vol.getRays();
    for (size_t isweep = 0; isweep < sweeps.size(); isweep++) {
      RadxSweep *sweep = sweeps[isweep];
      sweep->setSweepNumber(sweepNum);
      for (size_t iray = sweep->getStartRayIndex();
           iray <= sweep->getEndRayIndex(); iray++) {
        RadxRay *ray = rays[iray];
        ray->setSweepNumber(sweepNum);
      } // iray
      sweepNum++;
    } // isweep
  }
  
  // read remaining paths, aggregating as we go
  
  RadxVol latestVol;
  for (size_t ipath = 1; ipath < paths.size(); ipath++) {

    // read in latest

    if (readFromPath(paths[ipath], latestVol)) {
      _addErrStr("ERROR - RadxFile::aggregateFromPaths");
      return -1;
    }

    // aggregate
    
    const vector<RadxSweep *> &sweeps = latestVol.getSweeps();
    const vector<RadxRay *> &rays = latestVol.getRays();
    for (size_t isweep = 0; isweep < sweeps.size(); isweep++) {
      RadxSweep *sweep = sweeps[isweep];
      sweep->setSweepNumber(sweepNum);
      for (size_t iray = sweep->getStartRayIndex();
           iray <= sweep->getEndRayIndex(); iray++) {
        RadxRay *ray = rays[iray];
        ray->setSweepNumber(sweepNum);
        vol.addRay(ray);
      } // iray
      sweepNum++;
    } // isweep

    latestVol.clear();
    
  } // ipath

  // load up volme info from ray info
  
  vol.loadSweepInfoFromRays();
  vol.loadVolumeInfoFromRays();

  return 0;

}

/////////////////////////////////////////////////////////
// Read in data file from specified directory.
// NOTE: before calling this function, you must first call one
//       of the setReadMode....() functions to set up the search
// Load up volume object.
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int RadxFile::readFromDir(const string &dir,
                          RadxVol &vol)


{
  
  if (_debug) {
    _readTimeList.printRequest(cerr);
  }
  clearErrStr();

  // compile list of times for files in time interval

  _readTimeList.setDir(dir);
  if (_readTimeList.compile()) {
    _addErrStr("ERROR - RadxFile::readFromDir");
    _addErrStr("  Cannot compile read time list");
    _addErrStr("  dir: ", dir);
    _addErrStr(_readTimeList.getErrStr());
    _addErrStr(_readTimeList.getRequestString());
    return -1;
  }
  
  // special case for reading rays in interval

  if (_readRaysInInterval) {
    return _doReadRaysInInterval(dir, vol);
  }

  // get paths

  const vector<string> &paths = _readTimeList.getPathList();
  if (paths.size() < 1) {
    _addErrStr("ERROR - RadxFile::readFromDir");
    _addErrStr("  No suitable data files found");
    _addErrStr("  dir: ", dir);
    _addErrStr(_readTimeList.getRequestString());
    return -1;
  }

  // read first file in list

  if (readFromPath(paths[0], vol)) {
    _addErrStr("ERROR - RadxFile::readFromDir");
    _addErrStr("  dir: ", dir);
    _addErrStr("  path ", paths[0]);
    _addErrStr(_readTimeList.getRequestString());
    return -1;
  }

  return 0;

}

///////////////////////////////
// clear the error string

void RadxFile::clearErrStr()
{
  _errStr.clear();
}

/////////////////////////////////////////////////
/// Initialize for reading

void RadxFile::_initForRead(const string &path,
                            RadxVol &vol)
{
  clear();
  _readVol = &vol;
  _readVol->clear();
  _pathInUse = path;
  vol.setPathInUse(_pathInUse);
  _readPaths.clear();
}

///////////////////////////////////////////////
// add labelled integer value to error string,
// with optional following carriage return.

void RadxFile::_addErrInt(string label, int iarg, bool cr)
{
  Radx::addErrInt(_errStr, label, iarg, cr);
}

///////////////////////////////////////////////
// add labelled double value to error string,
// with optional following carriage return.
// Default format is %g.

void RadxFile::_addErrDbl(string label, double darg,
                          string format, bool cr)
  
{
  Radx::addErrDbl(_errStr, label, darg, format, cr);
}

////////////////////////////////////////
// add labelled string to error string
// with optional following carriage return.

void RadxFile::_addErrStr(string label, string strarg, bool cr)

{
  Radx::addErrStr(_errStr, label, strarg, cr);
}

/////////////////////////////////////////////////////////
// Utility routine to create a directory.  If the directory
// already exists, does nothing.
//
// Returns -1 on error, 0 otherwise.

int RadxFile::makeDir(const string &dir)
{
  
  // Status the directory to see if it already exists.
  
  struct stat stat_buf;
  if (stat(dir.c_str(), &stat_buf) == 0) {
    return 0;
  }
  
  // Directory doesn't exist, create it.
  
  if (mkdir(dir.c_str(),
	    S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
    // failed
    // check if dir has been made bu some other process
    // in the mean time, in which case return success
    if (stat(dir.c_str(), &stat_buf) == 0) {
      return 0;
    }
    return -1;
  }
  
  return 0;

}


/////////////////////////////////////////////////////////
// Utility routine to create a directory recursively.
// If the directory already exists, does nothing.
// Otherwise it recurses through the path, making all
// needed directories.
//
// Returns -1 on error, 0 otherwise.

int RadxFile::makeDirRecurse(const string &dir)
{

  // Status the directory to see if it already exists.
  // '/' dir will always exist, so this stops the recursion
  // automatically.
  
  struct stat dir_stat;
  if (stat(dir.c_str(), &dir_stat) == 0) {
    return 0;
  }
  
  // create up dir - one up the directory tree -
  // by searching for the previous delim and removing it
  // from the string.
  // If no delim, try to make the directory non-recursively.
  
  size_t delimPos = dir.find_last_of(PATH_SEPARATOR);
  if (delimPos == string::npos) {
    return makeDir(dir);
  } 

  string upDir(dir, 0, delimPos);
  
  // make the up dir
  
  if (makeDirRecurse(upDir)) {
    return -1;
  }

  // make this dir
  
  if (makeDir(dir)) {
    return -1;
  } else {
    return 0;
  }

}

/////////////////////////////////////////////////////////
// Given a dir, returns a string with a
// temporary file path.
//
// The intended use is to provide a tmp file path to which a file
// is written prior to renaming to the final name.
//
// The tmp path is in the same directory as the final path.
//
// If tmpFileName is non-empty, it is used for the file name.
// If it is empty, the name is 'tmp.pid.timesec.timeusec.tmp',
// where pid is determined using the getpid() function.

string RadxFile::tmpPathFromDir(const string &dir,
                                const string &tmpFileName)
  
{

  // get current time

  struct timeval now;
  gettimeofday(&now, NULL);

  // load up the tmp path
  
  string tmpPath(dir);
  tmpPath += PATH_SEPARATOR;
  
  if (tmpFileName.size() > 0) {
    tmpPath += tmpFileName;
  } else {
    char tmpName[1024];
    sprintf(tmpName, "tmp.%d.%ld.%ld.tmp", getpid(),
            (long) now.tv_sec, (long) now.tv_usec);
    tmpPath += tmpName;
  }

  return tmpPath;

}

/////////////////////////////////////////////////////////
// Given a file path, fills a string with a
// temporary file path.
//
// The intended use is to provide a tmp file path to which a file
// is written prior to renaming to the final name.
//
// The tmp path is in the same directory as the final file.
//
// If tmpFileName is non-empty, it is used for the file name.
// If it is empty, the name is 'tmp.pid.timesec.timeusec.tmp',
// where pid is determined using the getpid() function.

string RadxFile::tmpPathFromFilePath(const string &finalFilePath,
                                     const string &tmpFileName)

{

  // get the dir

  RadxPath path(finalFilePath);
  if (path.isDir()) {
    return tmpPathFromDir(finalFilePath, tmpFileName);
  } else {
    return tmpPathFromDir(path.getDirectory(), tmpFileName);
  }

}

/////////////////////////////////////////////////////////
// set up object for reading

// clear any previous settings

void RadxFile::clearRead()
{
  _readFieldNames.clear();
  _readFixedAngleLimitsSet = false;
  _readMinFixedAngle = Radx::missingMetaDouble;
  _readMaxFixedAngle = Radx::missingMetaDouble;
  _readSweepNumLimitsSet = false;
  _readMinSweepNum = Radx::missingMetaInt;
  _readMaxSweepNum = Radx::missingMetaInt;
  _readStrictAngleLimits = true;
  _readAggregateSweeps = false;
  _readIgnoreIdleMode = false;
  _readIgnoreTransitions = false;
  _readTransitionNraysMargin = 0;
  _readRemoveRaysAllMissing = false;
  _readSetMaxRange = false;
  _readMaxRangeKm = Radx::missingMetaDouble;
  _readRemoveLongRange = false;
  _readPreserveSweeps = false;
  _readComputeSweepAnglesFromVcpTables = false;
  _readRemoveShortRange = false;
  _readMetadataOnly = false;
  _readTimesOnly = false;
  _readSetRadarNum = -1;
  _readRadarNum = -1;
  _readChangeLatitudeSign = false;
  _readApplyGeorefs = false;
  _readRaysInInterval = false;
  _readRaysStartTime.clear();
  _readRaysEndTime.clear();
  _readDwellSecs = 0;
  _readDwellStatsMethod = RadxField::STATS_METHOD_MIDDLE;
}

// add a field to the read list
// if not fields added, all fields in the file will be read in

void RadxFile::addReadField(const string &field_name)
  
{

  // make sure field has not already been requested
  
  bool found = false;
  for (int kk = 0; kk < (int) _readFieldNames.size(); kk++) {
    if (_readFieldNames[kk] == field_name) {
      found = true;
    }
  }

  if (found) {
    return;
  }

  // add name to list

  _readFieldNames.push_back(field_name);

}

/// Is a field name needed on read?
/// Returns true or false.
/// If no fields have been specified on read, this method
/// will always return true because all fields are required.

bool RadxFile::isFieldRequiredOnRead(const string &field_name) const

{
  if (_readFieldNames.size() == 0) {
    return true;
  }
  for (int kk = 0; kk < (int) _readFieldNames.size(); kk++) {
    if (_readFieldNames[kk] == field_name) {
      return true;
    }
  }
  return false;
}

/////////////////////////////////////////////////////////////////
/// Limit the sweep data to be used
///
/// set elevation angle limits on read
///
/// If strict limits are not set (see below), 1 sweep is guaranted
/// to be returned. Even if no sweep lies within the limits the closest
/// sweep will be returned.
///
/// If strict limits are set (which is the default), then only data from
/// sweeps within the limits will be included.
  
void RadxFile::setReadFixedAngleLimits(double min_fixed_angle,
                                       double max_fixed_angle)

{
  _readFixedAngleLimitsSet = true;
  _readSweepNumLimitsSet = false;
  _readMinFixedAngle = min_fixed_angle;
  _readMaxFixedAngle = max_fixed_angle;
}

/// set sweep number limits on read
///
/// If strict limits are not set (see below), 1 sweep is guaranted
/// to be returned. Even if no sweep lies within the limits the closest
/// sweep will be returned.
///
/// If strict limits are set (which is the default), then only data from
/// sweeps within the limits will be included.
  
void RadxFile::setReadSweepNumLimits(int min_sweep_num,
                                     int max_sweep_num)
{
  _readSweepNumLimitsSet = true;
  _readFixedAngleLimitsSet = false;
  _readMinSweepNum = min_sweep_num;
  _readMaxSweepNum = max_sweep_num;
  
}

/// Set flag to indicate strict checking on the sweep angle/num limits.
/// Applies to both setReadFixedAngleLimits() and
/// setReadFixedAngleLimits().
///
/// The default is for strict limits on:
///   _readStrictAngleLimits = true.
///
/// If strict limits are off, if no sweep meets the limits set, the
/// closest sweep is chosen so that at least 1 sweep is returned in
/// the data read in.  If strict limits are set, then the read will
/// return an error if no sweep lies within the defined limits.

void RadxFile::setReadStrictAngleLimits(bool val)
{
  _readStrictAngleLimits = val;
}

/////////////////////////////////////////////////////////////////
// Set aggregation into volume on read.
// If true, individual sweep files will be aggregated into a
// volume when the files are read
  
void RadxFile::setReadAggregateSweeps(bool val)

{
  _readAggregateSweeps = val;
  _readTimeList.setReadAggregateSweeps(val);
}

/////////////////////////////////////////////////////////////////
// Set to ignore idle mode on read.
// If true, data from IDLE scan mode will not be read

void RadxFile::setReadIgnoreIdleMode(bool val)

{
  _readIgnoreIdleMode = val;
}

/////////////////////////////////////////////////////////////////
// Set to ignore transitions on read.
// If true, rays for which the antenna is in transtion
// not be read.
// Default is false.

void RadxFile::setReadIgnoreTransitions(bool val)

{
  _readIgnoreTransitions = val;
}

/////////////////////////////////////////////////////////////////
/// Set the number of transition rays to include as a margin.
/// Sometimes the transition flag is turned on too early in
/// a transition, on not turned off quickly enough after a transition.
/// If you set this to a number greater than 0, that number of rays
/// will be included at each end of the transition, i.e. the
/// transition will effectively be shorter at each end by this
/// number of rays

void RadxFile::setReadTransitionNraysMargin(int val)

{
  _readTransitionNraysMargin = val;
}

/////////////////////////////////////////////////////////////////
/// Set flag to indicate removal of rays, on read, for which
/// all fields have missing data at all gates.
/// Defaults to false.
/// If true, individual sweep files will be aggregated into a
/// volume when the files are read

void RadxFile::setReadRemoveRaysAllMissing(bool val)

{
  _readRemoveRaysAllMissing = val;
}

/////////////////////////////////////////////////////////////////
/// Set max range for data on read.
/// Gates beyond this range are removed.

void RadxFile::setReadMaxRangeKm(double maxRangeKm)

{
  _readSetMaxRange = true;
  _readMaxRangeKm = maxRangeKm;
}

/////////////////////////////////////////////////////////////////
/// Set flag to indicate that we want to preserve the
/// sweep details in the file we read in.
/// This generally applies to NEXRAD data - by default we
/// consolidate sweeps by combining split-cut sweeps
/// into a single sweep.
/// If this flag is true, we leave the sweeps unchanged.
/// Defaults to false.

void RadxFile::setReadPreserveSweeps(bool val)

{
  _readPreserveSweeps = val;
}

/////////////////////////////////////////////////////////////////
/// Set flag to indicate that we want to preserve the
/// sweep angles in the file we read in.
/// This generally applies to NEXRAD data - by default we
/// compute fixed angles when the file does not contain
/// a vcp header.
/// If this flag is true, we leave the sweeps unchanged.
/// Defaults to false.

void RadxFile::setReadComputeSweepAnglesFromVcpTables(bool val)

{
  _readComputeSweepAnglesFromVcpTables = val;
}

/////////////////////////////////////////////////////////////////
/// Set flag to indicate removal of rays for long range sweeps.
/// This generally only applies to NEXRAD data, in which
/// the long range sweeps only include DBZ data.
/// Defaults to false.

void RadxFile::setReadRemoveLongRange(bool val)

{
  _readRemoveLongRange = val;
}

/////////////////////////////////////////////////////////////////
/// Set flag to indicate removal of rays for short range sweeps.
/// This generally only applies to NEXRAD data, in which
/// the long range sweeps only include DBZ data.
/// Defaults to false.

void RadxFile::setReadRemoveShortRange(bool val)

{
  _readRemoveShortRange = val;
}

/////////////////////////////////////////////////////////////////
/// Set flag to indicate we should only read the main metadata,
/// including the sweep and field information, and NOT read the
/// rays and data fields.
/// Defaults to false.

void RadxFile::setReadMetadataOnly(bool val)

{
  _readMetadataOnly = val;
}

/////////////////////////////////////////////////////////////////
/// Set flag to indicate we should only read the times only,
/// and set the start and end times.
/// Defaults to false.

void RadxFile::setReadTimesOnly(bool val)

{
  _readTimesOnly = val;
  if (val) {
    _readMetadataOnly = true;
  }
}

/////////////////////////////////////////////////////////////////
/// Set radar number to be read in.
/// Only applies to file formats with data from more than 1 radar
/// in a file.
/// For example, HRD files have data from both the lower fuselage
/// (LF = 1) radar and the tail radar (TA = 2).
///
/// Defaults to -1. Mostly not used. If not set, then for HRD data
/// the tail radar data will be returned.

void RadxFile::setRadarNumOnRead(int radarNum)

{
  _readSetRadarNum = true;
  _readRadarNum = radarNum;
}

/////////////////////////////////////////////////////////////////
/// Set flag to indicate we should change the latitude sign
/// on read - applies to Rapic files
/// Defaults to false.

void RadxFile::setChangeLatitudeSignOnRead(bool val)
{
  _readChangeLatitudeSign = val;
}

/////////////////////////////////////////////////////////////////
/// Set flag to indicate we should apply the georeference
/// information on read, to compute earth-relative azimuth and
/// elevation.
/// Only applies to moving radars.
/// Defaults to false.

void RadxFile::setApplyGeorefsOnRead(bool val)
{
  _readApplyGeorefs = val;
}

/////////////////////////////////////////////////////////
// print

void RadxFile::print(ostream &out) const
  
{
  
  out << "=============== RadxFile ===============" << endl;

  out << "  fileFormat: " << getFileFormatAsString() << endl;
  out << "  dirInUse: " << _dirInUse << endl;
  out << "  pathInUse: " << _pathInUse << endl;

  out << "===========================================" << endl;

}

/////////////////////////////////////////////////////////
// print read request details

void RadxFile::printReadRequest(ostream &out) const
  
{
  
  out << "======= RadxFile read request =======" << endl;
  out << "  debug: " << (_debug?"Y":"N") << endl;
  out << "  verbose: " << (_verbose?"Y":"N") << endl;

  if (_readFieldNames.size() > 0) {
    out << "  readFieldNames: ";
    for (int ii = 0; ii < (int) _readFieldNames.size(); ii++) {
      out << _readFieldNames[ii];
      if (ii < (int) _readFieldNames.size() - 1) {
        out << ", ";
      }
    }
    out << endl;
  }

  if (_readFixedAngleLimitsSet) {
    out << "  readMinFixedAngle: " << _readMinFixedAngle << endl;
    out << "  readMaxFixedAngle: " << _readMaxFixedAngle << endl;
    out << "  readStrictAngleLimits: "
        << (_readStrictAngleLimits?"Y":"N") << endl;
  }
  if (_readSweepNumLimitsSet) {
    out << "  readMinSweepNum: " << _readMinSweepNum << endl;
    out << "  readMaxSweepNum: " << _readMaxSweepNum << endl;
    out << "  readStrictAngleLimits: "
        << (_readStrictAngleLimits?"Y":"N") << endl;
  }

  out << "  readIgnoreIdleMode: "
      << (_readIgnoreIdleMode?"Y":"N") << endl;
  out << "  readIgnoreTransitions: "
      << (_readIgnoreTransitions?"Y":"N") << endl;
  out << "  readTransitionNraysMargin: "
      << (_readTransitionNraysMargin) << endl;
  out << "  readAggregateSweeps: "
      << (_readAggregateSweeps?"Y":"N") << endl;
  out << "  readRemoveRaysAllMissing: "
      << (_readRemoveRaysAllMissing?"Y":"N") << endl;

  if (_readSetMaxRange) {
    cerr << "  readMaxRangeKm: " << _readMaxRangeKm << endl;
  }

  out << "  readRemoveLongRange: "
      << (_readRemoveLongRange?"Y":"N") << endl;
  out << "  readRemoveShortRange: "
      << (_readRemoveShortRange?"Y":"N") << endl;

  if (_readTimeList.getMode() != RadxTimeList::MODE_UNDEFINED) {
    out << "-------------------------------------" << endl;
    out << _readTimeList.getRequestString();
    out << "-------------------------------------" << endl;
    if (_readRaysInInterval) {
      out << "==>> ReadingRaysInInterval <<==" << endl;
      out << "  readRaysStartTime: " << _readRaysStartTime.asString(3) << endl;
      out << "  readRaysEndTime: " << _readRaysEndTime.asString(3) << endl;
      out << "  readDwellSecs: " << _readDwellSecs << endl;
      out << "  readDwellStatsMethod: "
          << RadxField::statsMethodToStr(_readDwellStatsMethod) << endl;
      out << "-------------------------------------" << endl;
    }
  }

  out << "=====================================" << endl;

}

/////////////////////////////////////////////////////////
// print write request details

void RadxFile::printWriteRequest(ostream &out) const
  
{
  
  out << "======= RadxFile write request =======" << endl;
  out << "  debug: " << (_debug?"Y":"N") << endl;
  out << "  verbose: " << (_verbose?"Y":"N") << endl;

  out << "  writeNativeByteOrder: "
      << (_writeNativeByteOrder?"Y":"N") << endl;
  out << "  writeForceNgatesVary: "
      << (_writeForceNgatesVary?"Y":"N") << endl;
  out << "  writeProposedStdNameInNcf: "
      << (_writeProposedStdNameInNcf?"Y":"N") << endl;
  out << "  writeFileNameMode: "
      << getFileNameModeAsString() << endl;
  out << "  writeFileNamePrefix: " << _writeFileNamePrefix << endl;
  out << "  writeFileNameSuffix: " << _writeFileNameSuffix << endl;
  out << "  writeIndividualSweeps: "
      << (_writeIndividualSweeps?"Y":"N") << endl;
  out << "  writeInstrNameInFileName: "
      << (_writeInstrNameInFileName?"Y":"N") << endl;
  out << "  writeSiteNameInFileName: "
      << (_writeSiteNameInFileName?"Y":"N") << endl;
  out << "  writeSubsecsInFileName: "
      << (_writeSubsecsInFileName?"Y":"N") << endl;
  out << "  writeScanTypeInFileName: "
      << (_writeScanTypeInFileName?"Y":"N") << endl;
  out << "  writeVolNumInFileName: "
      << (_writeVolNumInFileName?"Y":"N") << endl;
  out << "  writeHyphenInDateTime: "
      << (_writeHyphenInDateTime?"Y":"N") << endl;
  out << "  writeCompressed: "
      << (_writeCompressed?"Y":"N") << endl;
  out << "  compressionLevel: " << _compressionLevel << endl;
  out << "  writeLdataInfo: "
      << (_writeLdataInfo?"Y":"N") << endl;

  out << "=====================================" << endl;

}

/////////////////////////////////////////////////////////
// get file format as string

string RadxFile::getFileFormatAsString() const

{
  
  if (_fileFormat == FILE_FORMAT_CFRADIAL) {
    return "CFRADIAL";
  } else if (_fileFormat == FILE_FORMAT_NCXX) {
    return "NCXX";
  } else if (_fileFormat == FILE_FORMAT_CFRADIAL2) {
    return "CFRADIAL2";
  } else if (_fileFormat == FILE_FORMAT_DORADE) {
    return "DORADE";
  } else if (_fileFormat == FILE_FORMAT_UF) {
    return "UF";
  } else if (_fileFormat == FILE_FORMAT_FORAY_NC) {
    return "FORAY_NC";
  } else if (_fileFormat == FILE_FORMAT_NEXRAD_AR2) {
    return "NEXRAD_AR2";
  } else if (_fileFormat == FILE_FORMAT_SIGMET_RAW) {
    return "SIGMET_RAW";
  } else if (_fileFormat == FILE_FORMAT_GEM_XML) {
    return "GEM_XML";
  } else if (_fileFormat == FILE_FORMAT_LEOSPHERE) {
    return "LEOSPHERE";
  } else if (_fileFormat == FILE_FORMAT_DOE_NC) {
    return "DOE_NC";
  } else if (_fileFormat == FILE_FORMAT_MDV_RADIAL) {
    return "MDV_RADIAL";
  } else if (_fileFormat == FILE_FORMAT_NEXRAD_NIDS3) {
    return "NEXRAD_NIDS3";
  } else if (_fileFormat == FILE_FORMAT_HRD) {
    return "HRD";
  } else if (_fileFormat == FILE_FORMAT_NSSL_MRD) {
    return "NSSL_MRD";
  } else if (_fileFormat == FILE_FORMAT_TDWR) {
    return "TDWR";
  } else if (_fileFormat == FILE_FORMAT_ODIM_HDF5) {
    return "ODIM_HDF5";
  } else if (_fileFormat == FILE_FORMAT_GAMIC_HDF5) {
    return "GAMIC_HDF5";
  } else if (_fileFormat == FILE_FORMAT_ODIM_HDF5) {
    return "ODIM_HDF5";
  } else if (_fileFormat == FILE_FORMAT_NEXRAD_CMD) {
    return "NEXRAD_CMD";
  } else if (_fileFormat == FILE_FORMAT_TWOLF) {
    return "TWOLF";
  } else if (_fileFormat == FILE_FORMAT_D3R_NC) {
    return "D3R_NC";
  } else if (_fileFormat == FILE_FORMAT_NOAA_FSL) {
    return "NOAA_FSL";
  } else if (_fileFormat == FILE_FORMAT_NOXP_NC) {
    return "NOXP_NC";
  } else if (_fileFormat == FILE_FORMAT_CFARR) {
    return "CFARR";
  } else if (_fileFormat == FILE_FORMAT_NIMROD) {
    return "NIMROD";
  } else {
    return "UNKNOWN";
  }

}

/////////////////////////////////////////////////////////
// get file name_mode as string

string RadxFile::getFileNameModeAsString() const

{
  
  if (_writeFileNameMode == FILENAME_WITH_START_AND_END_TIMES) {
    return "FILENAME_WITH_START_AND_END_TIMES";
  } else if (_writeFileNameMode == FILENAME_WITH_START_TIME_ONLY) {
    return "FILENAME_WITH_START_TIME_ONLY";
  } else if (_writeFileNameMode == FILENAME_WITH_END_TIME_ONLY) {
    return "FILENAME_WITH_END_TIME_ONLY";
  } else {
    return "UNKNOWN";
  }

}

////////////////////////////////////////////////////////////
// Print native data in file
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int RadxFile::printNative(const string &path, ostream &out,
                          bool printRays, bool printData)
  
{

  // try netcdf first

  if (isNetCDF(path)) {
    return _printNativeNetCDF(path, out, printRays, printData);
  }
  
  // try hdf5 next
  
  if (isHdf5(path)) {
    return _printNativeHdf5(path, out, printRays, printData);
  }

  // fall through to other types
  
  return _printNativeOther(path, out, printRays, printData);
  
}

////////////////////////////////////////////////////////////
// Print native data in netcdf file
// Returns 0 on success, -1 on failure

int RadxFile::_printNativeNetCDF(const string &path, ostream &out,
                                 bool printRays, bool printData)
  
{

  // try CF radial
  {
    NcfRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isCfRadial(path)) {
      int iret = file.printNative(path, out, printRays, printData);
      if (iret) {
        _errStr = file.getErrStr();
      }
      return iret;
    }
  }
  
  // try Foray NetCDF next
  {
    ForayNcRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isForayNc(path)) {
      int iret = file.printNative(path, out, printRays, printData);
      if (iret) {
        _errStr = file.getErrStr();
      }
      return iret;
    }
  }
  
  // try DOE netcdf next
  {
    DoeNcRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isDoeNc(path)) {
      int iret = file.printNative(path, out, printRays, printData);
      if (iret) {
        _errStr = file.getErrStr();
      }
      return iret;
    }
  }
  
  // try NOXP netcdf next
  {
    NoxpNcRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isNoxpNc(path)) {
      int iret = file.printNative(path, out, printRays, printData);
      if (iret) {
        _errStr = file.getErrStr();
      }
      return iret;
    }
  }
  
  // try D3R netcdf next
  {
    D3rNcRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isD3rNc(path)) {
      int iret = file.printNative(path, out, printRays, printData);
      if (iret) {
        _errStr = file.getErrStr();
      }
      return iret;
    }
  }
  
  // try NOAA FSL netcdf next
  {
    NoaaFslRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isNoaaFsl(path)) {
      int iret = file.printNative(path, out, printRays, printData);
      if (iret) {
        _errStr = file.getErrStr();
      }
      return iret;
    }
  }
  
  // try NEXRAD CMD next
  {
    NexradCmdRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isNexradCmd(path)) {
      int iret = file.printNative(path, out, printRays, printData);
      if (iret) {
        _errStr = file.getErrStr();
      }
      return iret;
    }
  }
  
  // try EEC Edge netcdf next
  {
    EdgeNcRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isEdgeNc(path)) {
      int iret = file.printNative(path, out, printRays, printData);
      if (iret) {
        _errStr = file.getErrStr();
      }
      return iret;
    }
  }
  
  // try Cfarr netcdf next
  {
    CfarrNcRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isCfarrNc(path)) {
      int iret = file.printNative(path, out, printRays, printData);
      if (iret) {
        _errStr = file.getErrStr();
      }
      return iret;
    }
  }
  
  return -1;

}

////////////////////////////////////////////////////////////
// Print native data in hdf5 file
// Returns 0 on success, -1 on failure

int RadxFile::_printNativeHdf5(const string &path, ostream &out,
                               bool printRays, bool printData)
  
{

  // try ODIM HDF5 next
  {
    OdimHdf5RadxFile file;
    file.copyReadDirectives(*this);
    if (file.isOdimHdf5(path)) {
      int iret = file.printNative(path, out, printRays, printData);
      if (iret) {
        _errStr = file.getErrStr();
      }
      return iret;
    }
  }
  
  // try GAMIC HDF5 next
  {
    GamicHdf5RadxFile file;
    file.copyReadDirectives(*this);
    if (file.isGamicHdf5(path)) {
      int iret = file.printNative(path, out, printRays, printData);
      if (iret) {
        _errStr = file.getErrStr();
      }
      return iret;
    }
  }
  
  return -1;

}

////////////////////////////////////////////////////////////
// Print native data in other file types
// Returns 0 on success, -1 on failure

int RadxFile::_printNativeOther(const string &path, ostream &out,
                                bool printRays, bool printData)
  
{
  
  // try Dorade first
  {
    DoradeRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isDorade(path)) {
      int iret = file.printNative(path, out, printRays, printData);
      if (iret) {
        _errStr = file.getErrStr();
      }
      return iret;
    }
  }

  // try UF next
  {
    UfRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isUf(path)) {
      int iret = file.printNative(path, out, printRays, printData);
      if (iret) {
        _errStr = file.getErrStr();
      }
      return iret;
    }
  }

  // try NEXRAD next
  {
    NexradRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isNexrad(path)) {
      int iret = file.printNative(path, out, printRays, printData);
      if (iret) {
        _errStr = file.getErrStr();
      }
      return iret;
    }
  }

  // try SIGMET RAW next
  {
    SigmetRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isSigmet(path)) {
      int iret = file.printNative(path, out, printRays, printData);
      if (iret) {
        _errStr = file.getErrStr();
      }
      return iret;
    }
  }

  // try GEMATRONIK next
  {
    GemRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isGematronik(path)) {
      int iret = file.printNative(path, out, printRays, printData);
      if (iret) {
        _errStr = file.getErrStr();
      }
      return iret;
    }
  }

  // try LEOSPHERE next
  {
    LeoRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isLeosphere(path)) {
      int iret = file.printNative(path, out, printRays, printData);
      if (iret) {
        _errStr = file.getErrStr();
      }
      return iret;
    }
  }

  // try RAPIC next
  {
    RapicRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isRapic(path)) {
      int iret = file.printNative(path, out, printRays, printData);
      if (iret) {
        _errStr = file.getErrStr();
      }
      return iret;
    }
  }

  // try NIDS next
  {
    NidsRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isNids(path)) {
      int iret = file.printNative(path, out, printRays, printData);
      if (iret) {
        _errStr = file.getErrStr();
      }
      return iret;
    }
  }

  // try HRD next
  {
    HrdRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isSupported(path)) {
      int iret = file.printNative(path, out, printRays, printData);
      if (iret) {
        _errStr = file.getErrStr();
      }
      return iret;
    }
  }

  // try TDWR next
  {
    TdwrRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isSupported(path)) {
      int iret = file.printNative(path, out, printRays, printData);
      if (iret) {
        _errStr = file.getErrStr();
      }
      return iret;
    }
  }


  // try TWOLF next
  {
    TwolfRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isTwolf(path)) {
      int iret = file.printNative(path, out, printRays, printData);
      if (iret) {
        _errStr = file.getErrStr();
      }
      return iret;
    }
  }

  // try NSSL MRD - NSSL data from NOAA tail radars
  {
    NsslMrdRadxFile file;
    file.copyReadDirectives(*this);
    if (file.isSupported(path)) {
      int iret = file.printNative(path, out, printRays, printData);
      if (iret) {
        _errStr = file.getErrStr();
      }
      return iret;
    }
  }

  // do not recognize file type
  
  _addErrStr("ERROR - RadxFile::printNative");
  _addErrStr("  File format not recognized: ", path);
  return -1;

}

/////////////////////////////////////////////////////////////////////////
// Read in rays from multiple files in specified directory,
//   within the specified time interval
//
// If dwellSecs <= 0, all rays are returned unaltered.
// If dwellSecs > 0, ray data is combined using the specified stats mathod.
// 
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int RadxFile::_doReadRaysInInterval(const string &dir,
                                    RadxVol &vol)

{

  // get list of paths
  
  const vector<string> &paths = _readTimeList.getPathList();
  if (paths.size() < 1) {
    _addErrStr("ERROR - RadxFile::_doReadRaysInInterval");
    _addErrStr("  No suitable data files found");
    _addErrStr("  dir: ", dir);
    _addErrStr(_readTimeList.getRequestString());
    return -1;
  }

  // create a volume for stats

  RadxVol statsVol;
  RadxTime dwellStartTime, dwellEndTime;

  // loop through the paths
  
  for (size_t ipath = 0; ipath < paths.size(); ipath++) {

    string path(paths[ipath]);

    // read file into local volume

    RadxVol fileVol;
    if (readFromPath(path, fileVol)) {
      _addErrStr("ERROR - RadxFile::_doReadRaysInInterval");
      _addErrStr("  dir: ", dir);
      _addErrStr("  path ", path);
      _addErrStr(_readTimeList.getRequestString());
      return -1;
    }
    vol.copyMeta(fileVol);

    const vector<RadxRay *> &fileRays = fileVol.getRays();
    for (size_t iray = 0; iray < fileRays.size(); iray++) {

      // check ray is within requested time

      const RadxRay *fileRay = fileRays[iray];
      RadxTime rayTime = fileRay->getRadxTime();
      if (rayTime < _readRaysStartTime ||
          rayTime > _readRaysEndTime) {
        continue;
      }

      // add rays to stats vol
    
      RadxRay *ray = new RadxRay(*fileRay);
      if (statsVol.getNRays() == 0) {
        dwellStartTime = ray->getRadxTime();
      }
      statsVol.addRay(ray);
      int nRaysDwell = statsVol.getNRays();
      dwellEndTime = ray->getRadxTime();
      double dwellSecs = dwellEndTime - dwellStartTime;
      if (nRaysDwell > 1) {
        dwellSecs *= ((double) nRaysDwell / (nRaysDwell - 1.0));
      }

      // dwell time exceeded, so compute dwell ray and add to volume
      
      if (dwellSecs >= _readDwellSecs) {
        if (_verbose) {
          cerr << "==>> readRaysInInterval, using nrays: " << nRaysDwell << endl;
        }
        if (nRaysDwell <= 2) {
          // only 2 rays, use raw rays - don't do stats
          vector<RadxRay *> &statsRays = statsVol.getRays();
          for (size_t ii = 0; ii < statsRays.size(); ii++) {
            vol.addRay(statsRays[ii]);
          }
        } else {
          // compute stats
          RadxRay *dwellRay = statsVol.computeFieldStats(_readDwellStatsMethod);
          vol.addRay(dwellRay);
        }
        // reset
        statsVol.clearRays();
      }

    } // iray
    
  } // ipath

  // compute volume metadata

  vol.loadSweepInfoFromRays();
  vol.loadVolumeInfoFromRays();

  return 0;

}

