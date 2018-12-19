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
// DoradeRadxFile.cc
//
// DoradeRadxFile object
//
// Dorade file data for radar radial data
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2010
//
///////////////////////////////////////////////////////////////

#include <Radx/DoradeRadxFile.hh>
#include <Radx/DoradeData.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/RadxCfactors.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxPath.hh>
#include <Radx/ByteOrder.hh>
#include <cstring>
#include <cmath>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>
using namespace std;

//////////////
// Constructor

DoradeRadxFile::DoradeRadxFile() : RadxFile()
  
{
  
  _file = NULL;
  _latestRay = NULL;
  _writeVol = NULL;
  _readVol = NULL;
  clear();

}

/////////////
// destructor

DoradeRadxFile::~DoradeRadxFile()

{
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void DoradeRadxFile::clear()
  
{

  clearErrStr();
  _close();
  
  _clearRays();
  _clearLatestRay();

  _nNullsFound = 0;
  _nGatesIn = 0;
  
  _parmIsOldVersion = false;
  _radarIsOldVersion = false;

  _sweepNumOnAg = -1;
  _volumeNum = 0;

  _ddIsSwapped = false;
  memset(&_ddComment, 0, sizeof(_ddComment));
  memset(&_ddSwib, 0, sizeof(_ddSwib));
  memset(&_ddVol, 0, sizeof(_ddVol));
  memset(&_ddRadar, 0, sizeof(_ddRadar));
  memset(&_ddLidar, 0, sizeof(_ddLidar));
  memset(&_ddCfac, 0, sizeof(_ddCfac));
  memset(&_ddCellv, 0, sizeof(_ddCellv));
  memset(&_ddCsfp, 0, sizeof(_ddCsfp));
  memset(&_ddSweep, 0, sizeof(_ddSweep));
  memset(&_ddPlat, 0, sizeof(_ddPlat));
  memset(&_ddRay, 0, sizeof(_ddRay));
  memset(&_ddPdat, 0, sizeof(_ddPdat));
  memset(&_ddQdat, 0, sizeof(_ddQdat));
  memset(&_ddExtra, 0, sizeof(_ddExtra));
  memset(&_ddRotTable, 0, sizeof(_ddRotTable));
  memset(&_ddRadarTestStatus, 0, sizeof(_ddRadarTestStatus));
  memset(&_ddFieldRadar, 0, sizeof(_ddFieldRadar));
  memset(&_ddFieldLidar, 0, sizeof(_ddFieldLidar));
  memset(&_ddInsituDescript, 0, sizeof(_ddInsituDescript));
  memset(&_ddInsituData, 0, sizeof(_ddInsituData));
  memset(&_ddIndepFreq, 0, sizeof(_ddIndepFreq));
  memset(&_ddMinirimsData, 0, sizeof(_ddMinirimsData));
  memset(&_ddNavDescript, 0, sizeof(_ddNavDescript));
  memset(&_ddTimeSeries, 0, sizeof(_ddTimeSeries));
  memset(&_ddWaveform, 0, sizeof(_ddWaveform));

  _writeNativeByteOrder = false;
  _rotEntries.clear();

  _rotationTableOffset = 0;
  _rotationTableSize = 0;
  _sedsBlockOffset = 0;
  _sedsBlockSize = 0;
  
  _writeFileNameMode = FILENAME_WITH_START_TIME_ONLY;

}

void DoradeRadxFile::_clearRays()
{
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    delete _rays[ii];
  }
  _rays.clear();
}

void DoradeRadxFile::_clearLatestRay()
{
  _latestRay = NULL;
  _rayInProgress = false;
}
  
/////////////////////////////////////////////////////////
// Write data from volume to specified directory
//
// A separate file will be written out for each sweep.
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

int DoradeRadxFile::writeToDir(const RadxVol &vol,
                               const string &dir,
                               bool addDaySubDir,
                               bool addYearSubDir)
  
{

  // clear array of write paths

  _writePaths.clear();
  _writeDataTimes.clear();

  // write one file for each sweep

  const vector<RadxSweep *> &sweeps = vol.getSweeps();

  for (size_t ii = 0; ii < sweeps.size(); ii++) {

    const RadxSweep &sweep = *sweeps[ii];
    
    // construct new volume just for this sweep
    
    RadxVol *sweepVol = new RadxVol(vol, sweep.getSweepNumber());
    
    // write the sweep file

    if (sweepVol->getNSweeps() == 1) {
      if (_writeSweepToDir(*sweepVol, dir, addDaySubDir, addYearSubDir)) {
        delete sweepVol;
        return -1;
      }
    }

    // clean up

    delete sweepVol;

  } // ii

  return 0;

}
  
// write single sweep to dir

int DoradeRadxFile::_writeSweepToDir(RadxVol &vol,
                                     const string &dir,
                                     bool addDaySubDir,
                                     bool addYearSubDir)
  
{
  
  clearErrStr();
  _dirInUse = dir;

  if (_debug) {
    cerr << "DEBUG - DoradeRadxFile::_writeSweepToDir" << endl;
    cerr << "  Writing to dir: " << dir << endl;
  }
  
  // Dorade always written using the start time

  RadxTime ftime(vol.getStartTimeSecs());
  int millisecs = (int) (vol.getStartNanoSecs() / 1.0e6 + 0.5);

  string outDir(dir);
  if (addYearSubDir) {
    char yearStr[BUFSIZ];
    sprintf(yearStr, "%s%.4d", PATH_SEPARATOR, ftime.getYear());
    outDir += yearStr;
  }
  if (addDaySubDir) {
    char dayStr[BUFSIZ];
    sprintf(dayStr, "%s%.4d%.2d%.2d", PATH_SEPARATOR,
            ftime.getYear(), ftime.getMonth(), ftime.getDay());
    outDir += dayStr;
  }

  // make sure output subdir exists
  
  if (makeDirRecurse(outDir)) {
    _addErrStr("ERROR - DoradeRadxFile::_writeSweepToDir");
    _addErrStr("  Cannot make output dir: ", outDir);
    return -1;
  }
  
  // compute path
  
  const RadxSweep &sweep = *vol.getSweeps()[0];
  string scanType = Radx::sweepModeToShortStr(sweep.getSweepMode());
  double fixedAngle = sweep.getFixedAngleDeg();
  int volNum = vol.getVolumeNumber();
  string outName =
    computeFileName(volNum, 1, fixedAngle,
                    vol.getInstrumentName(), scanType,
                    ftime.getYear(), ftime.getMonth(), ftime.getDay(),
                    ftime.getHour(), ftime.getMin(), ftime.getSec(),
                    millisecs);
  string outPath(outDir);
  outPath += PATH_SEPARATOR;
  outPath += outName;
  
  int iret = _writeSweepToPath(vol, outPath);

  if (iret) {
    _addErrStr("ERROR - DoradeRadxFile::_writeSweepToDir");
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

int DoradeRadxFile::writeToPath(const RadxVol &vol,
                                const string &path)
  
{
  
  // clear array of write paths

  _writePaths.clear();
  _writeDataTimes.clear();

  // If more than 1 sweep, just write the first one

  int nSweeps = vol.getSweeps().size();
  if (nSweeps < 1) {
    _addErrStr("ERROR - DoradeRadxFile::writeToPath");
    _addErrStr("  No sweeps found");
    _addErrStr("  Path: ", path);
    return -1;
  }

  // copy the sweep

  const vector<RadxSweep *> &sweeps = vol.getSweeps();
  const RadxSweep &sweep = *sweeps[0];
  RadxVol *sweepVol = new RadxVol(vol, sweep.getSweepNumber());

  // write to path

  if (_writeSweepToPath(*sweepVol, path)) {
    delete sweepVol;
    return -1;
  }
  
  delete sweepVol;

  return 0;

}

/////////////////////////////////////////////////////////
// Write data from volume to specified path
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs
// Use getPathInUse() for path written

int DoradeRadxFile::_writeSweepToPath(RadxVol &vol,
                                      const string &path)
  
{

  // initialize

  clearErrStr();
  _writeVol = &vol;
  _pathInUse = path;
  vol.setPathInUse(path);
  _rotationTableOffset = 0;
  _rotationTableSize = 0;
  _sedsBlockOffset = 0;
  _sedsBlockSize = 0;

  // ensure we have computed the max number of gates

  vol.computeMaxNGates();

  // set the fields on the volume

  vol.loadFieldsFromRays();

  // open the output file

  string tmpPath(tmpPathFromFilePath(path, ""));

  if (_debug) {
    cerr << "DEBUG - DoradeRadxFile::writeToPath" << endl;
    cerr << "  Writing to path: " << path << endl;
    cerr << "  Tmp path: " << tmpPath << endl;
  }

  if (_openWrite(tmpPath)) {
    _addErrStr("ERROR - DoradeRadxFile::writeToPath");
    _addErrStr("  Cannot open tmp dorade file: ", tmpPath);
    return -1;
  }

  // write comment

  if (_writeComment()) {
    _addErrStr("ERROR - DoradeRadxFile::writeToPath");
    _close();
    return -1;
  }

  // store location - need to rewrite sswb later after getting the
  // file size

  long superSwibPos = ftell(_file);

  // write super sweep info block

  if (_writeSuperSwib()) {
    _addErrStr("ERROR - DoradeRadxFile::writeToPath");
    _close();
    return -1;
  }
  
  // write volume info

  if (_writeVolume()) {
    _addErrStr("ERROR - DoradeRadxFile::writeToPath");
    _close();
    return -1;
  }
  
  // write radar and lidar info as applicable

  if (_writeRadar()) {
    _addErrStr("ERROR - DoradeRadxFile::writeToPath");
    _close();
    return -1;
  }

  if (_writeLidar()) {
    _addErrStr("ERROR - DoradeRadxFile::writeToPath");
    _close();
    return -1;
  }

  // write parameter info
  
  for (size_t ifield = 0; ifield < _writeVol->getFields().size(); ifield++) {
    if (_writeParameter(ifield)) {
      _addErrStr("ERROR - DoradeRadxFile::writeToPath");
      _close();
      return -1;
    }
  }

  // write cell spacing

  // if (_writeCellVector()) {
  if (_writeCellSpacingFp()) {
    _addErrStr("ERROR - DoradeRadxFile::writeToPath");
    _close();
    return -1;
  }
  
  // write correction factors

  if (_writeCorrectionFactors()) {
    _addErrStr("ERROR - DoradeRadxFile::writeToPath");
    _close();
    return -1;
  }

  // loop through the sweeps

  _rotEntries.clear();

  for (size_t isweep = 0; isweep < _writeVol->getSweeps().size(); isweep++) {
    const RadxSweep &sweep = *_writeVol->getSweeps()[isweep];

    if (_writeSweepInfo(isweep)) {
      _addErrStr("ERROR - DoradeRadxFile::writeToPath");
      _close();
      return -1;
    }

    for (size_t iray = sweep.getStartRayIndex();
         iray <= sweep.getEndRayIndex(); iray++) {
      
      // initialize rotation angle entry
      
      DoradeData::rot_table_entry_t rotEntry;
      DoradeData::init(rotEntry);
      rotEntry.offset = (Radx::si32) ftell(_file);
      
      // write ray

      DoradeData::ray_t ddRay;
      if (_writeRayInfo(iray, ddRay)) {
        _addErrStr("ERROR - DoradeRadxFile::writeToPath");
        _close();
        return -1;
      }
      
      if (_writeRayGeoref(iray, ddRay, rotEntry)) {
        _addErrStr("ERROR - DoradeRadxFile::writeToPath");
        _close();
        return -1;
      }
      
      const RadxRay &ray = *_writeVol->getRays()[iray];
      for (size_t ifield = 0; ifield < ray.getFields().size(); ifield++) {
        if (_writeRayData(iray, ifield)) {
          _addErrStr("ERROR - DoradeRadxFile::writeToPath");
          _close();
          return -1;
        }
      }

      // compute ray data size, store in rotation angle entry

      long pos = ftell(_file);
      rotEntry.size = pos - rotEntry.offset;

      // add entry to table vector

      _rotEntries.push_back(rotEntry);

    } // iray

  } // isweep

  // null block

  if (_writeNullBlock()) {
    _addErrStr("ERROR - DoradeRadxFile::writeToPath");
    _close();
    return -1;
  }

  // rotation angle block

  _rotationTableOffset = ftell(_file);
  if (_writeRotAngTable()) {
    _addErrStr("ERROR - DoradeRadxFile::writeToPath");
    _close();
    return -1;
  }
  _rotationTableSize = ftell(_file) - _rotationTableOffset; 
  _rotEntries.clear();

  // SEDS block

  _sedsBlockOffset = ftell(_file);
  if (_writeSedsBlock()) {
    _addErrStr("ERROR - DoradeRadxFile::writeToPath");
    _close();
    return -1;
  }
  _sedsBlockSize = ftell(_file) - _sedsBlockOffset; 

  // file write completed - get the file size in bytes

  long fileSize = ftell(_file);

  // rewind to start of superSwib

  if (fseek(_file, superSwibPos, SEEK_SET)) {
    int errNum = errno;
    _addErrStr("ERROR - DoradeRadxFile::writeToPath");
    _addErrStr("  Cannot seek back to start of superSwib: ", tmpPath);
    _addErrStr(strerror(errNum));
    _close();
    return -1;
  }

  // rewrite super Sweep info

  if (_writeSuperSwib(fileSize)) {
    _addErrStr("ERROR - DoradeRadxFile::writeToPath");
    _close();
    return -1;
  }
  
  // close output file
  
  _close();
  
  // rename the tmp to final output file path
  
  if (rename(tmpPath.c_str(), _pathInUse.c_str())) {
    int errNum = errno;
    _addErrStr("ERROR - DoradeRadxFile::writeToPath");
    _addErrStr("  Cannot rename tmp file: ", tmpPath);
    _addErrStr("  to: ", _pathInUse);
    _addErrStr(strerror(errNum));
    return -1;
  }

  if (_debug) {
    cerr << "DEBUG - DoradeRadxFile::writeToPath" << endl;
    cerr << "  Renamed tmp path: " << tmpPath << endl;
    cerr << "     to final path: " << path << endl;
  }

  // add to array of write paths

  _writePaths.push_back(path);
  _writeDataTimes.push_back(vol.getStartTimeSecs());

  return 0;

}

////////////////////////////////////////////////////////////
// compute and return dorade file name

string DoradeRadxFile::computeFileName(int volNum,
                                       int nSweeps,
                                       double fixedAngle,
                                       string instrumentName,
                                       string scanType,
                                       int year, int month, int day,
                                       int hour, int min, int sec,
                                       int millisecs)
  
{

  // make sure instrument name is reasonable length

  if (instrumentName.size() > 8) {
    instrumentName.resize(8);
  }

  // replaces spaces in strings with underscores

  for (size_t ii = 0; ii < instrumentName.size(); ii++) {
    if (isspace(instrumentName[ii])) {
      instrumentName[ii] = '_';
    }
  }
  for (size_t ii = 0; ii < scanType.size(); ii++) {
    if (isspace(scanType[ii] == ' ')) {
      scanType[ii] = '_';
    }
  }

  if (nSweeps != 1) {
    fixedAngle = 0.0;
  }

  char outName[BUFSIZ];
  sprintf(outName, "swp.%d%02d%02d%02d%02d%02d.%s.%d.%.1f_%s_v%03d",
          year - 1900, month, day, hour, min, sec,
          instrumentName.c_str(), millisecs, fixedAngle,
          scanType.c_str(), volNum);

  return outName;

}
  
/////////////////////////////////////////////////////////
// Check if specified file is Dorade format
// Returns true if supported, false otherwise

bool DoradeRadxFile::isSupported(const string &path)

{
  
  if (isDorade(path)) {
    return true;
  }
  return false;

}

////////////////////////////////////////////////////////////
// Check if this is a Dorade file
// Returns true on success, false on failure

bool DoradeRadxFile::isDorade(const string &path)
  
{

  _close();
  
  // open file
  
  if (_openRead(path)) {
    _addErrStr("ERROR - DoradeRadxFile::isDorade");
    return false;
  }
  
  // read ID of first block

  char id[4];
  if (fread(id, 1, 4, _file) != 4) {
    _close();
    return false;
  }
  _close();
  
  if (DoradeData::isValid(id)) {
    return true;
  }

  return false;

}

////////////////////////////////////////////////////////////
// Check if this file needs to be byte-swapped on this host
// Returns 0 on success, -1 on failure
// Use isSwapped() to get result after making this call.

int DoradeRadxFile::checkIsSwapped(const string &path)
  
{

  _ddIsSwapped = false;

  // open file
  
  if (_openRead(path)) {
    _addErrStr("ERROR - DoradeRadxFile::checkIsSwapped");
    return -1;
  }
  
  // read ID of first block
  
  char id[4];
  if (fread(id, 1, 4, _file) != 4) {
    _close();
    return -1;
  }

  // read nbytes
  
  Radx::si32 nBytes;
  if (fread(&nBytes, 4, 1, _file) != 1) {
    int errNum = errno;
    _addErrStr("ERROR - DoradeRadxFile::checkIsSwapped()");
    _addErrStr("  Cannot read data length");
    _addErrStr("  ID: ", id);
    _addErrStr(strerror(errNum));
    _close();
    return -1;
  }
  _close();
  
  // check for valid length

  string idStr = Radx::makeString(id, 4);
  if (idStr == "COMM") {
    if (nBytes == sizeof(DoradeData::comment_t)) {
      return 0;
    }
    ByteOrder::swap32(&nBytes, 4);
    if (nBytes == sizeof(DoradeData::comment_t)) {
      _ddIsSwapped = true;
      return 0;
    }
  } else if (idStr == "SSWB") {
    if (nBytes == sizeof(DoradeData::super_SWIB_t) ||
        nBytes == sizeof(DoradeData::super_SWIB_32bit_t)) {
      return 0;
    }
    ByteOrder::swap32(&nBytes, 4);
    if (nBytes == sizeof(DoradeData::super_SWIB_t) ||
        nBytes == sizeof(DoradeData::super_SWIB_32bit_t)) {
      _ddIsSwapped = true;
      return 0;
    }
  } else if (idStr == "VOLD") {
    if (nBytes == sizeof(DoradeData::volume_t)) {
      return 0;
    }
    ByteOrder::swap32(&nBytes, 4);
    if (nBytes == sizeof(DoradeData::volume_t)) {
      _ddIsSwapped = true;
      return 0;
    }
  } else if (idStr == "RADD") {
    if (nBytes == sizeof(DoradeData::radar_t) ||
        nBytes == DoradeData::radar_alt_len) {
      return 0;
    }
    ByteOrder::swap32(&nBytes, 4);
    if (nBytes == sizeof(DoradeData::radar_t) ||
        nBytes == DoradeData::radar_alt_len) {
      _ddIsSwapped = true;
      return 0;
    }
  } else if (idStr == "CFAC") {
    if (nBytes == sizeof(DoradeData::correction_t)) {
      return 0;
    }
    ByteOrder::swap32(&nBytes, 4);
    if (nBytes == sizeof(DoradeData::correction_t)) {
      _ddIsSwapped = true;
      return 0;
    }
  } else if (idStr == "PARM") {
    if (nBytes == sizeof(DoradeData::parameter_t) ||
        nBytes == DoradeData::parameter_alt_len) {
      return 0;
    }
    ByteOrder::swap32(&nBytes, 4);
    if (nBytes == sizeof(DoradeData::parameter_t) ||
        nBytes == DoradeData::parameter_alt_len) {
      _ddIsSwapped = true;
      return 0;
    }
  } else if (idStr == "CELV") {
    if (nBytes == sizeof(DoradeData::cell_vector_t)) {
      return 0;
    }
    ByteOrder::swap32(&nBytes, 4);
    if (nBytes == sizeof(DoradeData::cell_vector_t)) {
      _ddIsSwapped = true;
      return 0;
    }
  } else if (idStr == "CSFD") {
    if (nBytes == sizeof(DoradeData::cell_spacing_fp_t)) {
      return 0;
    }
    ByteOrder::swap32(&nBytes, 4);
    if (nBytes == sizeof(DoradeData::cell_spacing_fp_t)) {
      _ddIsSwapped = true;
      return 0;
    }
  } else if (idStr == "SWIB") {
    if (nBytes == sizeof(DoradeData::sweepinfo_t)) {
      return 0;
    }
    ByteOrder::swap32(&nBytes, 4);
    if (nBytes == sizeof(DoradeData::sweepinfo_t)) {
      _ddIsSwapped = true;
      return 0;
    }
  } else if (idStr == "RYIB") {
    if (nBytes == sizeof(DoradeData::ray_t)) {
      return 0;
    }
    ByteOrder::swap32(&nBytes, 4);
    if (nBytes == sizeof(DoradeData::ray_t)) {
      _ddIsSwapped = true;
      return 0;
    }
  } else if (idStr == "XSTF") {
    if (nBytes == sizeof(DoradeData::extra_stuff_t)) {
      return 0;
    }
    ByteOrder::swap32(&nBytes, 4);
    if (nBytes == sizeof(DoradeData::extra_stuff_t)) {
      _ddIsSwapped = true;
      return 0;
    }
    return 0;
  }

  _ddIsSwapped = false;
  return 0;

}

////////////////////////////////////////////////////////////
// Read in data from specified path, load up volume object.
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs

int DoradeRadxFile::readFromPath(const string &path,
                                 RadxVol &vol)
  
{

  _initForRead(path, vol);
  _ddParms.clear();

  // is this a Dorade file?

  if (!isDorade(path)) {
    _addErrStr("ERROR - DoradeRadxFile::readFromPath");
    _addErrStr("  Not a dorade file: ", path);
    return -1;
  }
  
  // check if data in this file is swapped - this sets _ddIsSwapped
  
  if (checkIsSwapped(path)) {
    _addErrStr("ERROR - DoradeRadxFile::readFromPath");
    _addErrStr("  Cannot check if swapped: ", path);
    return -1;
  }

  // are we ignoring IDLE mode data?
  
  if (_readIgnoreIdleMode) {
    if (path.find("_IDL_") != string::npos) {
      _addErrStr("ERROR - DoradeRadxFile::readFromPath");
      _addErrStr("  Ignoring IDLE scan mode file: ", path);
      return -1;
    }
  }

  // if the flag is set to aggregate sweeps into a volume on read,
  // call the method to handle that

  if (_readAggregateSweeps) {
    if (_readAggregatePaths(path)) {
      _addErrStr("ERROR - DoradeRadxFile::readFromPath");
      return -1;
    }
  } else {
    _sweepNumOnAg = -1;
    if (_readSweepFile(path)) {
      _addErrStr("ERROR - DoradeRadxFile::readFromPath");
      return -1;
    }
    _volumeNum = _ddVol.volume_num;
  }

  // load the data into the read volume

  if (_loadReadVolume()) {
    return -1;
  }

  // interpolate to make sure we have monotonically increasing times

  vol.interpRayTimes();

  // remove transitions if applicable

  if (_readIgnoreTransitions) {
    _readVol->removeTransitionRays(_readTransitionNraysMargin);
  }

  // set format as read

  _fileFormat = FILE_FORMAT_DORADE;

  return 0;

}

////////////////////////////////////////////////////////////
// Read in sweep data using the specified path as a starting
// point. Aggregate the data from the sweeps into a single
// volume.
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs

int DoradeRadxFile::_readAggregatePaths(const string &path)
  
{

  // get the list of paths which make up this volume

  vector<string> paths;
  _volumeNum = _getVolumePaths(path, paths);
  if (_debug) {
    cerr << "INFO - _readAggregatePaths" << endl;
    cerr << "  trigger path: " << path << endl;
    cerr << "  volNum: " << _volumeNum << endl;
    for (size_t ii = 0; ii < paths.size(); ii++) {
      cerr << "  path member: " << paths[ii] << endl;
    }
  }
  
  for (size_t ii = 0; ii < paths.size(); ii++) {
    _sweepNumOnAg = ii + 1;
    if (_readSweepFile(paths[ii])) {
      return -1;
    }
  }

  return 0;

}


////////////////////////////////////////////////////////////
// Load sweep data from file
// Returns 0 on success, -1 on failure

int DoradeRadxFile::_readSweepFile(const string &path)
  
{

  if (_debug) {
    cerr << "INFO - _readSweepFile()" << endl;
    cerr << "  path: " << path << endl;
  }

  // check if data in this file is swapped - this sets _ddIsSwapped
  
  if (checkIsSwapped(path)) {
    _addErrStr("ERROR - DoradeRadxFile::readSweepFile");
    _addErrStr("  Cannot check if swapped: ", path);
    return -1;
  }

  // open file

  if (_openRead(path)) {
    _addErrStr("ERROR - DoradeRadxFile::_readSweepFile");
    return -1;
  }

  // read through file, finding data blocks
  
  while (!feof(_file)) {

    // read ID
    
    char id[8];
    memset(id, 0, 8);
    
    if (fread(id, 1, 4, _file) != 4) {
      if (feof(_file)) {
        // done
        break;
      }
      int errNum = errno;
      _addErrStr("ERROR - DoradeRadxFile::_readSweepFile");
      _addErrStr("  Cannot read id string");
      _addErrStr("  File path: ", path);
      _addErrStr(strerror(errNum));
      _close();
      return -1;
    }
    string idStr(id);

    // read nbytes

    Radx::si32 nBytes;
    if (fread(&nBytes, 4, 1, _file) != 1) {
      int errNum = errno;
      _addErrStr("ERROR - DoradeRadxFile::_readSweepFile()");
      _addErrStr("  Cannot read data length");
      _addErrStr("  ID: ", id);
      _addErrStr("  File path: ", path);
      _addErrStr(strerror(errNum));
      _close();
      return -1;
    }

    // swap as needed

    if (_ddIsSwapped) {
      ByteOrder::swap32(&nBytes, 4, true);
    }

    // check for reasonableness
    
    if (nBytes < 8 || nBytes > 1000000) {
      _addErrStr("ERROR - DoradeRadxFile::_readSweepFile()");
      _addErrStr("  Bad number of bytes");
      _addErrStr("  ID: ", id);
      _addErrInt("  nBytes: ", nBytes);
      _addErrStr("  File path: ", path);
      _close();
      return -1;
    }

    // print out id and data length

    if (_verbose) {
      cerr << "ID, nBytes: " << id << ", " << nBytes << endl;
    }
    
    // allocate memory for block

    char *block = new char[nBytes];
    
    // back up 8 bytes
    
    long nSeek = -8;
    if (fseek(_file, nSeek, SEEK_CUR)) {
      int errNum = errno;
      _addErrStr("ERROR - DoradeRadxFile::_readSweepFile()");
      _addErrStr("  Cannot seek back to start of block");
      _addErrStr("  ID: ", id);
      _addErrInt("  len: ", nBytes);
      _addErrStr("  File path: ", path);
      _addErrStr(strerror(errNum));
      delete[] block;
      _close();
      return -1;
    }

    // read in block
    
    if (fread(block, 1, nBytes, _file) != (size_t) nBytes) {
      int errNum = errno;
      _addErrStr("ERROR - DoradeRadxFile::_readSweepFile()");
      _addErrStr("  Cannot read data block");
      _addErrStr("  ID: ", id);
      _addErrInt("  len: ", nBytes);
      _addErrStr("  File path: ", path);
      _addErrStr(strerror(errNum));
      delete[] block;
      _close();
      return -1;
    }

    // handle block

    if (_handleBlock(idStr, nBytes, block)) {
      _addErrStr("ERROR - DoradeRadxFile::_readSweepFile()");
      _addErrStr("  Cannot handle data block");
      _addErrStr("  ID: ", id);
      _addErrInt("  len: ", nBytes);
      _addErrStr("  File path: ", path);
      delete[] block;
      _close();
      return -1;
    }
    
    // free up block
    
    delete[] block;
    
  } // while (!feof(_file)
  
  // close file
  
  _close();

  // add to paths used on read
  
  _readPaths.push_back(path);

  return 0;

}

//////////////////////////////////////////////////////////
// get list of paths for the volume for the specified path
// returns the volume number

int DoradeRadxFile::_getVolumePaths(const string &path,
                                    vector<string> &paths)
  
{

  paths.clear();
  int volNum = -1;

  // find the volume number by searching for "_v"

  size_t vloc = path.find("_v");
  if (vloc == string::npos || vloc == 0 ||
      vloc == (path.size() - 1)) {
    // cannot find volume tag "_v"
    paths.push_back(path);
    return volNum;
  }

  // scan the volume number

  string volStr = path.substr(vloc);
  string numStr = path.substr(vloc + 2);
  if (sscanf(numStr.c_str(), "%d", &volNum) != 1) {
    volNum = -1;
    return volNum;
  }

  // find all in the same
  // directory as the specified path
  
  vector<string> pathList;
  RadxPath rpath(path);
  string dir = rpath.getDirectory();
  _addToPathList(dir, 0, 23, pathList);
  
  RadxPath dpath(dir);
  string parentDir = dpath.getDirectory();

  // if time is close to start of day, search previous directory

  RadxTime rtime = getTimeFromPath(path);
  int rhour = rtime.getHour();

  if (rhour == 0) {
    RadxTime prevDate(rtime.utime() - RadxTime::RADX_SECS_IN_DAY);
    char prevDir[RadxPath::RADX_MAX_PATH_LEN];
    sprintf(prevDir, "%s%s%.4d%.2d%.2d",
            parentDir.c_str(), RadxPath::RADX_PATH_DELIM,
            prevDate.getYear(), prevDate.getMonth(), prevDate.getDay());
    _addToPathList(prevDir, 23, 23, pathList);
  }

  // if time is close to end of day, search previous direectory

  if (rhour == 23) {
    RadxTime nextDate(rtime.utime() + RadxTime::RADX_SECS_IN_DAY);
    char nextDir[RadxPath::RADX_MAX_PATH_LEN];
    sprintf(nextDir, "%s%s%.4d%.2d%.2d",
            parentDir.c_str(), RadxPath::RADX_PATH_DELIM,
            nextDate.getYear(), nextDate.getMonth(), nextDate.getDay());
    _addToPathList(nextDir, 0, 0, pathList);
  }

  // sort the path list

  sort(pathList.begin(), pathList.end());

  // find the index of the requested file in the path list

  string fileName = rpath.getFile();
  int index = -1;
  for (int ii = 0; ii < (int) pathList.size(); ii++) {
    if (pathList[ii].find(fileName) != string::npos) {
      index = ii;
    }
  }
  
  if (index < 0) {
    paths.push_back(path);
    return volNum;
  }

  // find the start and end index of the paths with the
  // same vol number

  int startIndex = 0;
  for (int ii = index - 1; ii >= 0; ii--) {
    if (pathList[ii].find(volStr) == string::npos) {
      startIndex = ii + 1;
      break;
    }
  }

  int endIndex = (int) pathList.size() - 1;
  for (int ii = startIndex; ii < (int) pathList.size(); ii++) {
    if (pathList[ii].find(volStr) == string::npos) {
      endIndex = ii - 1;
      break;
    } else if (ii == (int) (pathList.size() - 1)) {
      endIndex = ii;
    }
  }

  // load up all paths from start to end index

  for (int ii = startIndex; ii <= endIndex; ii++) {
    paths.push_back(pathList[ii]);
  }

  return volNum;

}

///////////////////////////////////////////////////////////
// add to the path list, given time constraints

void DoradeRadxFile::_addToPathList(const string &dir,
                                    int minHour, int maxHour,
                                    vector<string> &paths)
  
{

  // find all paths with this volume number
  
  DIR *dirp;
  if((dirp = opendir(dir.c_str())) == NULL) {
    return;
  }
  
  struct dirent *dp;
  for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
    
    string fileName(dp->d_name);

    // exclude dir entries which cannot be valid
    if (fileName.find("swp.") != 0) {
      continue;
    }
    if (fileName.find("IDL") != string::npos) {
      continue;
    }
    if (fileName.size() < 20) {
      continue;
    }

    RadxTime rtime = getTimeFromPath(fileName);
    int hour = rtime.getHour();
    if (hour >= minHour && hour <= maxHour) {
      string filePath = dir;
      filePath += RadxPath::RADX_PATH_DELIM;
      filePath += fileName;
      paths.push_back(filePath);
    }

  } // dp

  closedir(dirp);

}

////////////////////////////////////////////////
// get the date and time from a dorade file path

RadxTime DoradeRadxFile::getTimeFromPath(const string &path)

{

  RadxPath rpath(path);
  const string &fileName = rpath.getFile();

  int year = 1970, month = 1, day = 1;
  int hour = 0, min = 0, sec = 0;
  
  if (fileName[4] == '1' || fileName[4] == '2') { // year 2000+
    if (sscanf(fileName.c_str(), "swp.%3d%2d%2d%2d%2d%2d",
               &year, &month, &day, &hour, &min, &sec) == 6) {
      year += 1900;
    }
  } else {
    if (sscanf(fileName.c_str(), "swp.%2d%2d%2d%2d%2d%2d",
               &year, &month, &day, &hour, &min, &sec) == 6) {
      year += 1900;
    }
  }

  RadxTime rtime(year, month, day, hour, min, sec);
  return rtime;
  
}

//////////////////////////////////////
// open netcdf file for reading
// Returns 0 on success, -1 on failure

int DoradeRadxFile::_openRead(const string &path)
  
{

  _close();
  _file = fopen(path.c_str(), "r");
  
  // Check that constructor succeeded
  
  if (_file == NULL) {
    int errNum = errno;
    _addErrStr("ERROR - DoradeRadxFile::_openRead");
    _addErrStr("  Cannot open file for reading, path: ", path);
    _addErrStr("  ", strerror(errNum));
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////
// open netcdf file for writing
// create error object so we can handle errors
// Returns 0 on success, -1 on failure

int DoradeRadxFile::_openWrite(const string &path) 
{
  
  _close();
  _file = fopen(path.c_str(), "w");
  
  // Check that constructor succeeded
  
  if (_file == NULL) {
    int errNum = errno;
    _addErrStr("ERROR - DoradeRadxFile::_openWrite");
    _addErrStr("  Cannot open file for writing, path: ", path);
    _addErrStr("  ", strerror(errNum));
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////
// close netcdf file if open
// remove error object if it exists

void DoradeRadxFile::_close()
  
{
  
  // close file if open
  
  if (_file) {
    fclose(_file);
    _file = NULL;
  }
  
}

///////////////////////////////////////////////////////
// handle the data block on read

int DoradeRadxFile::_handleBlock(const string &idStr, int nBytes,
                                 const char *block)
  
{

  // if a ray is in progress, we are looking for field data or platform data

  if (_rayInProgress) {
    
    if (idStr == "ASIB") {
      
      int nn = sizeof(DoradeData::platform_t);
      memset(&_ddPlat, 0, nn);
      if (nn > nBytes) nn = nBytes;
      memcpy(&_ddPlat, block, nn);
      if (_ddIsSwapped) DoradeData::swap(_ddPlat, true);
      if (_verbose) DoradeData::print(_ddPlat, cerr);
      _addGeorefToLatestRay();
      return 0;
      
    } else if (idStr == "RDAT") {
      
      int nn = sizeof(DoradeData::paramdata_t);
      memset(&_ddPdat, 0, nn);
      if (nn > nBytes) nn = nBytes;
      memcpy(&_ddPdat, block, nn);
      if (_ddIsSwapped) DoradeData::swap(_ddPdat, true);
      if (_verbose) DoradeData::print(_ddPdat, cerr);
      
      // handle field if parameters are available
      string fieldName(Radx::makeString(_ddPdat.pdata_name, 8));
      for (size_t ii = 0; ii < _ddParms.size(); ii++) {
        string name(Radx::makeString(_ddParms[ii].parameter_name, 8));
        if (name == fieldName) {
          _handleField(_ddParms[ii], _ddPdat.nbytes, sizeof(_ddPdat), block);
          break;
        }
      }

      return 0;
      
    } else if (idStr == "QDAT") {
      
      int nn = sizeof(DoradeData::qparamdata_t);
      memset(&_ddQdat, 0, nn);
      if (nn > nBytes) nn = nBytes;
      memcpy(&_ddQdat, block, nn);
      if (_ddIsSwapped) DoradeData::swap(_ddQdat, true);
      if (_verbose) DoradeData::print(_ddQdat, cerr);

      // handle field if parameters are available
      string fieldName(Radx::makeString(_ddQdat.pdata_name, 8));
      for (size_t ii = 0; ii < _ddParms.size(); ii++) {
        string name(Radx::makeString(_ddParms[ii].parameter_name, 8));
        if (name == fieldName) {
          _handleField(_ddParms[ii], _ddQdat.nbytes, sizeof(_ddQdat), block);
          break;
        }
      }

      return 0;
      
    } else if (idStr == "XSTF") {
      
      int nn = sizeof(DoradeData::extra_stuff_t);
      memset(&_ddExtra, 0, nn);
      if (nn > nBytes) nn = nBytes;
      memcpy(&_ddExtra, block, nn);
      if (_ddIsSwapped) DoradeData::swap(_ddExtra, true);
      if (_verbose) DoradeData::print(_ddExtra, cerr);
      
    } else {

      // we are done with the ray, continue

      _clearLatestRay();
      
    }

  } // if (_rayInProgress)

  if (idStr == "COMM") {

    int nn = sizeof(DoradeData::comment_t);
    memset(&_ddComment, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddComment, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddComment, true);
    if (_verbose) DoradeData::print(_ddComment, cerr);

  } else if (idStr == "SSWB") {

    DoradeData::init(_ddSwib);
    if (nBytes >= (int) sizeof(DoradeData::super_SWIB_t)) {
      // 8-byte aligned version
      int nn = sizeof(DoradeData::super_SWIB_t);
      if (nn > nBytes) nn = nBytes;
      memcpy(&_ddSwib, block, nn);
      if (_ddIsSwapped) DoradeData::swap(_ddSwib, true);
    } else {
      // 4-byte aligned version
      int nn = sizeof(DoradeData::super_SWIB_32bit_t);
      if (nn > nBytes) nn = nBytes;
      DoradeData::super_SWIB_32bit_t swib32;
      memcpy(&swib32, block, nn);
      if (_ddIsSwapped) DoradeData::swap(swib32, true);
      DoradeData::copy(swib32, _ddSwib);
    }
    if (_verbose) DoradeData::print(_ddSwib, cerr);

  } else if (idStr == "VOLD") {

    int nn = sizeof(DoradeData::volume_t);
    memset(&_ddVol, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddVol, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddVol, true);
    // Time fields may need swapping even if rest of vol does not.
    DoradeData::swapTimeToReasonable(_ddVol); 
    if (_verbose) DoradeData::print(_ddVol, cerr);

  } else if (idStr == "RADD") {

    int nn = sizeof(DoradeData::radar_t);
    memset(&_ddRadar, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddRadar, block, nn);
    if (nn >= (int) sizeof(DoradeData::radar_t)) {
      _radarIsOldVersion = false;
    } else {
      _radarIsOldVersion = true;
    }
    if (_ddIsSwapped) DoradeData::swap(_ddRadar, true);
    if (_verbose) DoradeData::print(_ddRadar, cerr);

    _radarName = Radx::makeString(_ddRadar.radar_name, 8);

    if (_readIgnoreIdleMode) {
      if (_ddRadar.scan_mode == DoradeData::SCAN_MODE_IDL) {
        _addErrStr("ERROR - DoradeRadxFile::_handleBlock");
        _addErrStr("  Ignoring IDLE scan mode file: ", _pathInUse);
        return -1;
      }
    }

  } else if (idStr == "LIDR") {

    int nn = sizeof(DoradeData::lidar_t);
    memset(&_ddLidar, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddLidar, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddLidar, true);
    if (_verbose) DoradeData::print(_ddLidar, cerr);

  } else if (idStr == "CFAC") {

    int nn = sizeof(DoradeData::correction_t);
    memset(&_ddCfac, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddCfac, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddCfac, true);
    if (_verbose) DoradeData::print(_ddCfac, cerr);
    
  } else if (idStr == "PARM") {

    int nn = sizeof(DoradeData::parameter_t);
    if (nn > nBytes) nn = nBytes;
    DoradeData::parameter_t parm;
    memset(&parm, 0, sizeof(parm));
    memcpy(&parm, block, nn);
    if (nn >= (int) sizeof(DoradeData::parameter_t)) {
      _parmIsOldVersion = false;
    } else {
      _parmIsOldVersion = true;
    }
    if (_ddIsSwapped) DoradeData::swap(parm, true);
    if (_verbose) DoradeData::print(parm, cerr);

    // add to parameter list if not already there
    bool alreadyStored = false;
    string newName(Radx::makeString(parm.parameter_name, 8));
    for (size_t ii = 0; ii < _ddParms.size(); ii++) {
      string name(Radx::makeString(_ddParms[ii].parameter_name, 8));
      if (name == newName) {
        alreadyStored = true;
        break;
      }
    }
    if (!alreadyStored) {
      _ddParms.push_back(parm);
    }

  } else if (idStr == "CELV") {

    int nn = sizeof(DoradeData::cell_vector_t);
    memset(&_ddCellv, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddCellv, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddCellv, true);
    if (_verbose) DoradeData::print(_ddCellv, cerr);

    _nGatesIn = _ddCellv.number_cells;

  } else if (idStr == "CSFD") {
    
    int nn = sizeof(DoradeData::cell_spacing_fp_t);
    memset(&_ddCsfp, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddCsfp, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddCsfp, true);
    if (_verbose) DoradeData::print(_ddCsfp, cerr);

    _nGatesIn = 0;
    int ns = _ddCsfp.num_segments;
    if (ns > 8) ns = 8;
    for (int ii = 0; ii < ns; ii++) {
      _nGatesIn += _ddCsfp.num_cells[ii];
    }

  } else if (idStr == "SWIB") {

    int nn = sizeof(DoradeData::sweepinfo_t);
    memset(&_ddSweep, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddSweep, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddSweep, true);
    if (_verbose) DoradeData::print(_ddSweep, cerr);
    
  } else if (idStr == "RYIB") {

    if (_handleRay(nBytes, block)) {
      _addErrStr("ERROR - DoradeRadxFile::_handleBlock");
      return -1;
    }
    
  } else if (idStr == "XSIF") {
    
    int nn = sizeof(DoradeData::extra_stuff_t);
    memset(&_ddExtra, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddExtra, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddExtra, true);
    if (_verbose) DoradeData::print(_ddExtra, cerr);

  } else if (idStr == "NULL") {
    
    _nNullsFound++;
    if (_verbose) {
      int nn = sizeof(DoradeData::null_block_t);
      DoradeData::null_block_t nullBlock;
      memset(&nullBlock, 0, nn);
      if (nn > nBytes) nn = nBytes;
      memcpy(&nullBlock, block, nn);
      if (_ddIsSwapped) DoradeData::swap(nullBlock, true);
      DoradeData::print(nullBlock, cerr);
    }

  } else if (idStr == "RKTB") {
    
    int nn = sizeof(DoradeData::rot_angle_table_t);
    memset(&_ddRotTable, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddRotTable, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddRotTable, true);
    if (_verbose) DoradeData::print(_ddRotTable, cerr);
    
    int offset = _ddRotTable.first_key_offset;
    for (int ii = 0; ii < _ddRotTable.num_rays;
         ii++, offset += sizeof(DoradeData::rot_table_entry_t)) {
      if (nBytes - offset < (int) sizeof(DoradeData::rot_table_entry_t)) {
        break;
      }
      DoradeData::rot_table_entry_t entry;
      memcpy(&entry, block + offset, sizeof(entry));
      if (_ddIsSwapped) DoradeData::swap(entry, true);
      if (_verbose) DoradeData::print(entry, cerr);
    }

  } else if (idStr == "SEDS") {

    _sedsStr = Radx::makeString(block + 8, nBytes - 8);
    if (_verbose) {
      cerr << "============= SEDS ==============" << endl;
      cerr << _sedsStr << endl;
      cerr << "=================================" << endl;
    }
    
  }

  return 0;

}

///////////////////////////////////////////////////////
// handle a ray on read

int DoradeRadxFile::_handleRay(int nBytes, const char *block)
  
{

  // check we have valid metadata

  if (!DoradeData::isValid(_ddVol)) {
    _addErrStr("ERROR - DoradeRadxFile::_handleRay");
    _addErrStr("  No valid volume block found");
    return -1;
  }
  if (!DoradeData::isValid(_ddSweep)) {
    _addErrStr("ERROR - DoradeRadxFile::_handleRay");
    _addErrStr("  No valid sweep block found");
    return -1;
  }
  if (!DoradeData::isValid(_ddRadar) && !DoradeData::isValid(_ddLidar)) {
    _addErrStr("ERROR - DoradeRadxFile::_handleRay");
    _addErrStr("  No valid radar or lidar block found");
    return -1;
  }
  if (!DoradeData::isValid(_ddCellv) && !DoradeData::isValid(_ddCsfp)) {
    _addErrStr("ERROR - DoradeRadxFile::_handleRay");
    _addErrStr("  No valid cell spacing block found");
    return -1;
  }
  if (_ddParms.size() == 0) {
    _addErrStr("ERROR - DoradeRadxFile::_handleRay");
    _addErrStr("  No valid parameter block found");
    return -1;
  }

  // create new ray
  
  int nn = sizeof(DoradeData::ray_t);
  memset(&_ddRay, 0, nn);
  if (nn > nBytes) nn = nBytes;
  memcpy(&_ddRay, block, nn);
  if (_ddIsSwapped) DoradeData::swap(_ddRay, true);
  if (_verbose) DoradeData::print(_ddRay, cerr);

  RadxRay *ray = new RadxRay;
  _latestRay = ray;
  _rayInProgress = true;

  ray->setVolumeNumber(_ddVol.volume_num);

  if (_sweepNumOnAg > 0) {
    // when aggregating, get the sweep number from the number
    // of files aggregated so far
    ray->setSweepNumber(_sweepNumOnAg);
  } else {
    ray->setSweepNumber(_ddSweep.sweep_num);
  }

  // range geometry
  // first try from cell table

  _rangeArray.clear();
  if (DoradeData::isValid(_ddCellv)) {
    if (_ddCellv.number_cells > DoradeData::MAXCVGATES) {
      _ddCellv.number_cells = DoradeData::MAXCVGATES;
    }
    for (int ii = 0; ii < _ddCellv.number_cells; ii++) {
      if (ii != 0 && _ddCellv.dist_cells[ii] == 0) {
        // error in range table
        // this should not happen, the file appears corrupt
        _rangeArray.clear();
        break;
      }
      _rangeArray.push_back((double) _ddCellv.dist_cells[ii] / 1000.0); // km
    }
  }

  if (_rangeArray.size() == 0) {
    int nseg = _ddCsfp.num_segments;
    if (nseg > 8) nseg = 8;
    double dist = _ddCsfp.dist_to_first / 1000.0; // km
    for (int ii = 0; ii < nseg; ii++) {
      double spacing = _ddCsfp.spacing[ii] / 1000.0; // km
      for (int jj = 0; jj < _ddCsfp.num_cells[ii]; jj++) {
        _rangeArray.push_back(dist);
        dist += spacing;
      }      
    }
  }

  if (_rangeArray.size() < 2) {
    _addErrStr("ERROR - DoradeRadxFile::_handleRay");
    _addErrStr("  No range array, or no cells found");
    return -1;
  }
  
  _remap.computeRangeLookup(_rangeArray);
  _constantGateSpacing = _remap.getGateSpacingIsConstant();
  ray->setRangeGeom(_remap.getStartRangeKm(), _remap.getGateSpacingKm());

  // sweep mode

  int scanMode = _ddRadar.scan_mode;
  if (!DoradeData::isValid(_ddRadar)) {
    scanMode = _ddLidar.scan_mode;
  }

  switch (scanMode) {
    case DoradeData::SCAN_MODE_CAL:
      ray->setSweepMode(Radx::SWEEP_MODE_CALIBRATION);
      break;
    case DoradeData::SCAN_MODE_PPI:
      ray->setSweepMode(Radx::SWEEP_MODE_SECTOR);
      break;
    case DoradeData::SCAN_MODE_COP:
      ray->setSweepMode(Radx::SWEEP_MODE_COPLANE);
      break;
    case DoradeData::SCAN_MODE_RHI:
      ray->setSweepMode(Radx::SWEEP_MODE_RHI);
      break;
    case DoradeData::SCAN_MODE_VER:
      ray->setSweepMode(Radx::SWEEP_MODE_VERTICAL_POINTING);
      break;
    case DoradeData::SCAN_MODE_MAN:
      ray->setSweepMode(Radx::SWEEP_MODE_MANUAL_PPI);
      break;
    case DoradeData::SCAN_MODE_IDL:
      ray->setSweepMode(Radx::SWEEP_MODE_IDLE);
      break;
    case DoradeData::SCAN_MODE_SUR:
      ray->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
      break;
    case DoradeData::SCAN_MODE_AIR:
      ray->setSweepMode(Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE);
      break;
    default:
      ray->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
  }

  // prf mode

  if (DoradeData::isValid(_ddRadar)) {
    
    ray->setPrtMode(Radx::PRT_MODE_FIXED);
    double prtLong = Radx::missingMetaDouble;
    double prtShort = Radx::missingMetaDouble;
    if (_ddRadar.prt1 > 0 && _ddRadar.prt2 < 0) {
      prtShort = _ddRadar.prt1 / 1000.0;
    } else if (_ddRadar.prt2 > 0 && _ddRadar.prt1 < 0) {
      prtShort = _ddRadar.prt2 / 1000.0;
    } else if (_ddRadar.prt1 > 0 && _ddRadar.prt2 > 0) {
      prtLong = _ddRadar.prt2 / 1000.0;
      prtShort = _ddRadar.prt1 / 1000.0;
      if (prtLong < prtShort) {
        // swap
        double temp = prtLong;
        prtLong = prtShort;
        prtShort = temp;
      }
    } else if (_ddRadar.prt1 > 0) {
      prtShort = _ddRadar.prt1 / 1000.0;
    }

    if ((prtShort != Radx::missingMetaDouble) && (prtLong != Radx::missingMetaDouble)) {
      if (_ddRadar.num_ipps_trans < 2) {
        ray->setPrtMode(Radx::PRT_MODE_FIXED);
        ray->setPrtSec(prtShort);
      } else {
        double prtRatio = prtShort / prtLong;
        ray->setPrtSec(prtShort);
        ray->setPrtRatio(prtRatio);
        ray->setPrtMode(Radx::PRT_MODE_STAGGERED);
      }
    } else if (prtShort != Radx::missingMetaDouble) {
      ray->setPrtMode(Radx::PRT_MODE_FIXED);
      ray->setPrtSec(prtShort);
      ray->setPrtRatio(1.0);
    }

  } else if (DoradeData::isValid(_ddLidar)) {
    
    ray->setPrtMode(Radx::PRT_MODE_FIXED);
    ray->setPrtSec(1.0 / _ddLidar.prf);
    
  }

  // follow mode

  switch (scanMode) {
    case DoradeData::SCAN_MODE_TAR:
      ray->setFollowMode(Radx::FOLLOW_MODE_TARGET);
      break;
    default:
      ray->setFollowMode(Radx::FOLLOW_MODE_NONE);
  }

  // set time

  RadxTime volTime(_ddVol.year, _ddVol.month, _ddVol.day,
                   _ddVol.data_set_hour, _ddVol.data_set_minute,
                   _ddVol.data_set_second);
  RadxTime rayTime(_ddVol.year, _ddVol.month, _ddVol.day,
                   _ddRay.hour, _ddRay.minute, _ddRay.second);
  time_t volSecs = volTime.utime();
  time_t raySecs = rayTime.utime();
  int diff = raySecs - volSecs;

  // adjust time for day change

  if (diff > 43200) {
    raySecs -= 86400;
  } else if (diff < -43200) {
    raySecs += 86400;
  }

  ray->setTime(raySecs, _ddRay.millisecond * 1.0e6);

  // position

  ray->setAzimuthDeg(_ddRay.azimuth);
  ray->setElevationDeg(_ddRay.elevation);
  ray->setFixedAngleDeg(_ddSweep.fixed_angle);
  ray->setTrueScanRateDegPerSec(_ddRay.true_scan_rate);
  ray->setAntennaTransition(_ddRay.ray_status == 1);
  if (DoradeData::isValid(_ddRadar)) {
    ray->setNyquistMps(_ddRadar.eff_unamb_vel);
    ray->setUnambigRangeKm(_ddRadar.eff_unamb_range);
  } else if (DoradeData::isValid(_ddLidar)) {
    ray->setNyquistMps(_ddLidar.eff_unamb_vel);
    ray->setUnambigRangeKm(_ddLidar.eff_unamb_range);
  } 

  double powerDbm = Radx::missingMetaDouble;
  if (_ddRay.peak_power > 0) {
    powerDbm = 10.0 * log10(_ddRay.peak_power * 1.0e6);
  }
  ray->setMeasXmitPowerDbmH(powerDbm);
  ray->setMeasXmitPowerDbmV(powerDbm);

  // add to array, data fields will be added later

  _rays.push_back(ray);

  return 0;

}
    
///////////////////////////////////////////////////////
// add georeference information to latest ray, on read

void DoradeRadxFile::_addGeorefToLatestRay()
  
{

  RadxGeoref ref;
  
  ref.setLongitude(_ddPlat.longitude);
  ref.setLatitude(_ddPlat.latitude);
  ref.setAltitudeKmMsl(_ddPlat.altitude_msl);
  ref.setAltitudeKmAgl(_ddPlat.altitude_agl);
  ref.setEwVelocity(_ddPlat.ew_velocity);
  ref.setNsVelocity(_ddPlat.ns_velocity);
  ref.setVertVelocity(_ddPlat.vert_velocity);
  ref.setHeading(_ddPlat.heading);
  ref.setRoll(_ddPlat.roll);
  ref.setPitch(_ddPlat.pitch);
  ref.setDrift(_ddPlat.drift_angle);
  double rotationAngle = _ddPlat.rotation_angle;
  //   if (_radarName.find("ELDR") != string::npos) {
  //     rotationAngle = fmod(450.0 - rotationAngle, 360.0);
  //   }
  ref.setRotation(rotationAngle);
  ref.setTilt(_ddPlat.tilt);
  ref.setEwWind(_ddPlat.ew_horiz_wind);
  ref.setNsWind(_ddPlat.ns_horiz_wind);
  ref.setVertWind(_ddPlat.vert_wind);
  ref.setHeadingRate(_ddPlat.heading_change);
  ref.setPitchRate(_ddPlat.pitch_change);

  ref.setRadxTime(_latestRay->getRadxTime());

  _latestRay->setGeoref(ref);

}

///////////////////////////////////////////////////////
// handle a data field on read

int DoradeRadxFile::_handleField(const DoradeData::parameter_t &parm,
                                 int nBytesBlock, int minOffset,
                                 const char *block)

{

  string fieldName = Radx::makeString(parm.parameter_name, 8);

  // sanity  check

  if (_nGatesIn <= 0) {
    _addErrStr("WARNING  - DoradeRadxFile::_handleField");
    _addErrInt("  Bad _nGatesIn: ", _nGatesIn);
    _addErrStr("  Field: ", fieldName);
    return 0;
  }

  // check field names if appropriate
  
  if (!isFieldRequiredOnRead(fieldName)) {
    return 0;
  }

  // special case for 16-bit data - may be compressed

  DoradeData::binary_format_t format =
    (DoradeData::binary_format_t) parm.binary_format;

  if (format == DoradeData::BINARY_FORMAT_INT16) {
    return _handleField16(parm, nBytesBlock, minOffset, block);
  }

  int byteWidth = 1;
  if (format == DoradeData::BINARY_FORMAT_FLOAT32) {
    byteWidth = 4;
  } else if (format == DoradeData::BINARY_FORMAT_INT32) {
    byteWidth = 4;
  }
  
  int offsetToData = parm.offset_to_data;
  if (offsetToData < minOffset) {
    offsetToData = minOffset;
  }

  const char *data = block + offsetToData;
  int nBytesData = nBytesBlock - offsetToData;
  int nGatesData = nBytesData / byteWidth;

  if (nGatesData > _nGatesIn) {
    _addErrStr("ERROR - DoradeRadxFile::_handleField");
    _addErrStr("  Too much data size for number of gates");
    _addErrStr("  Field: ", fieldName);
    _addErrInt("  nGates in headers: ", _nGatesIn);
    _addErrInt("  nGates in data: ", nGatesData);
    if (_debug) {
      cerr << _errStr << endl;
    }
    return -1;
  }

  RadxField *field = new RadxField(fieldName,
                                   Radx::makeString(parm.param_units, 8));
  field->setLongName(Radx::makeString(parm.param_description, 40));
  field->setStandardName(Radx::makeString(parm.config_name, 8));
  field->setThresholdFieldName(Radx::makeString(parm.threshold_field, 8));
  field->setThresholdValue(parm.threshold_value);
  field->copyRangeGeom(*_latestRay);

  // temporarily set the sampling ratio to the number of samples
  // this will be adjusted when all fields have been read in

  field->setSamplingRatio(parm.num_samples);

  // add the data to the latest ray

  if (format == DoradeData::BINARY_FORMAT_FLOAT32) {
    
    Radx::fl32 *dptr = new Radx::fl32[_nGatesIn];
    memcpy(dptr, data, nBytesData);
    if (_ddIsSwapped) ByteOrder::swap32(dptr, nBytesData);
    for (int ii = nGatesData; ii < _nGatesIn; ii++) {
      dptr[ii] = (Radx::fl32) parm.bad_data;
    }
    field->setTypeFl32(parm.bad_data);
    field->setDataFl32(_nGatesIn, dptr, true);
    delete[] dptr;
      
  } else if (format == DoradeData::BINARY_FORMAT_INT32) {
    
    Radx::si32 *dptr = new Radx::si32[_nGatesIn];
    memcpy(dptr, data, nBytesData);
    if (_ddIsSwapped) ByteOrder::swap32(dptr, nBytesData);
    for (int ii = nGatesData; ii < _nGatesIn; ii++) {
      dptr[ii] = (Radx::si32) parm.bad_data;
    }
    double scale = 1.0 / parm.parameter_scale;
    double bias = (-1.0 * parm.parameter_bias) / parm.parameter_scale;
    field->setTypeSi32(parm.bad_data, scale, bias);
    field->setDataSi32(_nGatesIn, dptr, true);
    delete[] dptr;
    
  } else if (format == DoradeData::BINARY_FORMAT_INT16) {
    
    Radx::si16 *dptr = new Radx::si16[_nGatesIn];
    memcpy(dptr, data, nBytesData);
    if (_ddIsSwapped) ByteOrder::swap16(dptr, nBytesData);
    for (int ii = nGatesData; ii < _nGatesIn; ii++) {
      dptr[ii] = (Radx::si16) parm.bad_data;
    }
    double scale = 1.0 / parm.parameter_scale;
    double bias = (-1.0 * parm.parameter_bias) / parm.parameter_scale;
    field->setTypeSi16(parm.bad_data, scale, bias);
    field->setDataSi16(_nGatesIn, dptr, true);
    delete[] dptr;
    
  } else {
    
    // int8 data
    
    Radx::si08 *dptr = new Radx::si08[_nGatesIn];
    memcpy(dptr, data, nBytesData);
    for (int ii = nGatesData; ii < _nGatesIn; ii++) {
      dptr[ii] = (Radx::si08) parm.bad_data;
    }
    double scale = 1.0 / parm.parameter_scale;
    double bias = (-1.0 * parm.parameter_bias) / parm.parameter_scale;
    field->setTypeSi08(parm.bad_data, scale, bias);
    field->setDataSi08(_nGatesIn, dptr, true);
    delete[] dptr;
    
  }
  
  // set parameter-specific values on ray

  double pulseWidthMeters = parm.pulse_width;
  double pulseWidthUs = (pulseWidthMeters / Radx::LIGHT_SPEED) * 1.0e6 * 2.0;
  _latestRay->setPulseWidthUsec(pulseWidthUs);
  // _latestRay->setNSamples(parm.num_samples);

  // polarization mode

  switch (parm.polarization) {
    case DoradeData::POLARIZATION_HORIZONTAL:
      _latestRay->setPolarizationMode(Radx::POL_MODE_HORIZONTAL);
      break;
    case DoradeData::POLARIZATION_VERTICAL:
      _latestRay->setPolarizationMode(Radx::POL_MODE_VERTICAL);
      break;
    case DoradeData::POLARIZATION_CIRCULAR_RIGHT:
      _latestRay->setPolarizationMode(Radx::POL_MODE_CIRCULAR);
      break;
    case DoradeData::POLARIZATION_ELLIPTICAL:
      _latestRay->setPolarizationMode(Radx::POL_MODE_CIRCULAR);
      break;
    default:
      _latestRay->setPolarizationMode(Radx::POL_MODE_HORIZONTAL);
  }

  // add field to latest ray

  if (!_constantGateSpacing) {
    field->remapRayGeom(_remap);
  }
  _latestRay->addField(field);
  
  return 0;

}

///////////////////////////////////////////////////////
// handle a 16-bit data field on read

int DoradeRadxFile::_handleField16(const DoradeData::parameter_t &parm,
                                   int nBytesBlock, int minOffset,
                                   const char *block)

{
  
  string fieldName = Radx::makeString(parm.parameter_name, 8);

  DoradeData::binary_format_t format =
    (DoradeData::binary_format_t) parm.binary_format;
  if (format != DoradeData::BINARY_FORMAT_INT16) {
    cerr << "ERROR - DoradeRadxFile::_handleField16" << endl;
    cerr << "  Not 16-bit data" << endl;
    return -1;
  }
  int byteWidth = 2;

  // get offset into data
  // old headers may have offset set to 0
  
  int offsetToData = parm.offset_to_data;
  if (offsetToData < minOffset) {
    offsetToData = minOffset;
  }
  
  // find data

  const char *data = block + offsetToData;
  int nBytesData = nBytesBlock - offsetToData;

  // load up raw buffer, and byte-swap as appropriate
  
  int nGatesRaw = nBytesData / byteWidth;
  unsigned short *raw = new unsigned short[nGatesRaw];
  memcpy(raw, data, nBytesData);
  if (_ddIsSwapped) ByteOrder::swap16(raw, nBytesData);

  // uncompress are required

  short *uncompressed = new short[_nGatesIn];
  int nGatesData = 0;
  if (_ddRadar.data_compress == DoradeData::COMPRESSION_HRD) {
    // compressed, so uncompress
    int nbads;
    nGatesData = DoradeData::decompressHrd16(raw, nGatesRaw,
                                             (unsigned short *) uncompressed,
                                             _nGatesIn, parm.bad_data, &nbads);
  } else {
    // not compressed, just copy
    if (nGatesRaw < _nGatesIn) {
      nGatesData = nGatesRaw;
    } else {
      nGatesData = _nGatesIn;
    }
    memcpy(uncompressed, raw, nGatesData * byteWidth);
  }
  delete[] raw;

  RadxField *field = new RadxField(fieldName,
                                   Radx::makeString(parm.param_units, 8));
  field->setLongName(Radx::makeString(parm.param_description, 40));
  field->setStandardName(Radx::makeString(parm.config_name, 8));
  field->setThresholdFieldName(Radx::makeString(parm.threshold_field, 8));
  field->setThresholdValue(parm.threshold_value);
  field->copyRangeGeom(*_latestRay);
  
  // temporarily set the sampling ratio to the number of samples
  // this will be adjusted when all fields have been read in

  field->setSamplingRatio(parm.num_samples);

  // add the data to the latest ray

  Radx::si16 *dptr = new Radx::si16[_nGatesIn];
  memcpy(dptr, uncompressed, nGatesData * byteWidth);
  for (int ii = nGatesData; ii < _nGatesIn; ii++) {
    dptr[ii] = (Radx::si16) parm.bad_data;
  }
  double scale = 1.0 / parm.parameter_scale;
  double bias = (-1.0 * parm.parameter_bias) / parm.parameter_scale;
  field->setTypeSi16(parm.bad_data, scale, bias);
  field->setDataSi16(_nGatesIn, dptr, true);

  delete[] dptr;
  delete[] uncompressed;
  
  // set parameter-specific values on ray

  double pulseWidthMeters = parm.pulse_width;
  double pulseWidthUs = (pulseWidthMeters / Radx::LIGHT_SPEED) * 1.0e6 * 2.0;
  _latestRay->setPulseWidthUsec(pulseWidthUs);
  // _latestRay->setNSamples(parm.num_samples);

  // polarization mode

  switch (parm.polarization) {
    case DoradeData::POLARIZATION_HORIZONTAL:
      _latestRay->setPolarizationMode(Radx::POL_MODE_HORIZONTAL);
      break;
    case DoradeData::POLARIZATION_VERTICAL:
      _latestRay->setPolarizationMode(Radx::POL_MODE_VERTICAL);
      break;
    case DoradeData::POLARIZATION_CIRCULAR_RIGHT:
      _latestRay->setPolarizationMode(Radx::POL_MODE_CIRCULAR);
      break;
    case DoradeData::POLARIZATION_ELLIPTICAL:
      _latestRay->setPolarizationMode(Radx::POL_MODE_CIRCULAR);
      break;
    default:
      _latestRay->setPolarizationMode(Radx::POL_MODE_HORIZONTAL);
  }

  // add field to latest ray

  if (!_constantGateSpacing) {
    field->remapRayGeom(_remap);
  }
  _latestRay->addField(field);
  
  return 0;

}

/////////////////////////////////////////////////////////
// load up the read volume with the data from this object
// returns 0 on success, -1 on failure

int DoradeRadxFile::_loadReadVolume()
  
{

  // set meta-data

  _readVol->setOrigFormat("DORADE");
  _readVol->setVolumeNumber(_volumeNum);

  if (DoradeData::isValid(_ddRadar)) {
    _readVol->setInstrumentType(Radx::INSTRUMENT_TYPE_RADAR);
  } else if (DoradeData::isValid(_ddLidar)) {
    _readVol->setInstrumentType(Radx::INSTRUMENT_TYPE_LIDAR);
  }

  switch (_ddRadar.radar_type) {
    case DoradeData::RADAR_GROUND:
      _readVol->setPlatformType(Radx::PLATFORM_TYPE_FIXED);
      break;
    case DoradeData::RADAR_AIR_FORE:
      _readVol->setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_FORE);
      break;
    case DoradeData::RADAR_AIR_AFT:
      _readVol->setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_AFT);
      break;
    case DoradeData::RADAR_AIR_TAIL:
      _readVol->setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_TAIL);
      break;
    case DoradeData::RADAR_AIR_LF:
      _readVol->setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_BELLY);
      break;
    case DoradeData::RADAR_SHIP:
      _readVol->setPlatformType(Radx::PLATFORM_TYPE_SHIP);
      break;
    case DoradeData::RADAR_AIR_NOSE:
      _readVol->setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_NOSE);
      break;
    case DoradeData::RADAR_SATELLITE:
      _readVol->setPlatformType(Radx::PLATFORM_TYPE_SATELLITE_ORBIT);
      break;
    case DoradeData::LIDAR_MOVING:
      _readVol->setPlatformType(Radx::PLATFORM_TYPE_VEHICLE);
      break;
    case DoradeData::LIDAR_FIXED:
      _readVol->setPlatformType(Radx::PLATFORM_TYPE_FIXED);
      break;
    default:
      _readVol->setPlatformType(Radx::PLATFORM_TYPE_FIXED);
  }

  int nFreq = _ddRadar.num_freq_trans;
  if (_ddRadar.freq1 > 0) {
    _readVol->addFrequencyHz(_ddRadar.freq1 * 1.0e9);
  }
  if (nFreq > 1) {
    if (_ddRadar.freq2 > 0) {
      _readVol->addFrequencyHz(_ddRadar.freq2 * 1.0e9);
    }
  }
  if (nFreq > 2) {
    if (_ddRadar.freq2 > 0) {
      _readVol->addFrequencyHz(_ddRadar.freq3 * 1.0e9);
    }
  }
  if (nFreq > 3) {
    if (_ddRadar.freq2 > 0) {
      _readVol->addFrequencyHz(_ddRadar.freq4 * 1.0e9);
    }
  }
  if (nFreq > 4) {
    if (_ddRadar.freq2 > 0) {
      _readVol->addFrequencyHz(_ddRadar.freq5 * 1.0e9);
    }
  }

  if (_ddRadar.antenna_gain > 0) {
    _readVol->setRadarAntennaGainDbH(_ddRadar.antenna_gain);
    _readVol->setRadarAntennaGainDbV(_ddRadar.antenna_gain);
  }
  if (_ddRadar.horz_beam_width > 0) {
    _readVol->setRadarBeamWidthDegH(_ddRadar.horz_beam_width);
  }
  if (_ddRadar.vert_beam_width > 0) {
    _readVol->setRadarBeamWidthDegV(_ddRadar.vert_beam_width);
  }

  if (_ddLidar.lidar_const > 0) {
    _readVol->setLidarConstant(_ddLidar.lidar_const);
  }
  if (_ddLidar.pulse_energy > 0) {
    _readVol->setLidarPulseEnergyJ(_ddLidar.pulse_energy);
  }
  if (_ddLidar.peak_power > 0) {
    _readVol->setLidarPeakPowerW(_ddLidar.peak_power);
  }
  if (_ddLidar.aperture_size > 0) {
    _readVol->setLidarApertureDiamCm(_ddLidar.aperture_size);
  }
  if (_ddLidar.aperture_eff > 0) {
    _readVol->setLidarApertureEfficiency(_ddLidar.aperture_eff);
  }
  if (_ddLidar.field_of_view > 0) {
    _readVol->setLidarFieldOfViewMrad(_ddLidar.field_of_view);
  }
  if (_ddLidar.beam_divergence > 0) {
    _readVol->setLidarBeamDivergenceMrad(_ddLidar.beam_divergence);
  }

  int nRays = _rays.size();
  if (nRays > 0) {
    _readVol->setStartTime(_rays[0]->getTimeSecs(),
                           _rays[0]->getNanoSecs());
    _readVol->setEndTime(_rays[nRays-1]->getTimeSecs(),
                         _rays[nRays-1]->getNanoSecs());
    _readVol->copyRangeGeom(*_rays[0]);
  }

  _readVol->setTitle(Radx::makeString(_ddVol.proj_name, 20));
  _readVol->setSource(Radx::makeString(_ddVol.gen_facility, 8));
  _readVol->setReferences(Radx::makeString(_ddVol.flight_num, 8));
  if (DoradeData::isValid(_ddComment)) {
    _readVol->setComment(Radx::makeString(_ddComment.comment, 500));
  }
  _readVol->setSiteName(Radx::makeString(_ddRadar.site_name, 20));
  _readVol->setInstrumentName(_radarName);
  _readVol->setHistory(_sedsStr);
  if (_radarName.find("ELDR") != string::npos) {
    _readVol->setPrimaryAxis(Radx::PRIMARY_AXIS_Y_PRIME);
  } 
  // if the extension_num contains a value
  // override the primary axis. 
  // the primary axis enum type starts at zero, but a zero
  // value for primary axis could also just be an unset parameter,
  // so we a base value of 1 for the enumeration. A zero indicates no
  // value and 1,2,3,4,5,6 indicate a valid primary axis value. 
  // Finally, for internal work, convert the Dorade axis type
  // to the Radx axis type.
  try {
    if (_ddRadar.extension_num > 0) {
      DoradeData::primary_axis_t axisType =
        DoradeData::primaryAxisFromInt(_ddRadar.extension_num);
      Radx::PrimaryAxis_t radxAxisType =
        DoradeData::convertToRadxType(axisType);
      _readVol->setPrimaryAxis(radxAxisType);
    }
  }  catch (const char*  ex) {
    // we cannot set the primary axis
    // report warning
    cerr << "WARNING - DoradeRadxFile::_loadReadVolume()" << endl;
    cerr << ex << endl;
  }

  _readVol->setLatitudeDeg(_ddRadar.radar_latitude);
  _readVol->setLongitudeDeg(_ddRadar.radar_longitude);
  _readVol->setAltitudeKm(_ddRadar.radar_altitude);

  // guess at pulse width and bandwidth from first field

  double pulseWidthMeters = Radx::missingMetaDouble;
  double pulseWidthUs = Radx::missingMetaDouble;
  double bandwidthMhz = Radx::missingMetaDouble;

  if (_ddParms.size() > 0) {
    pulseWidthMeters = _ddParms[0].pulse_width;
    pulseWidthUs = (pulseWidthMeters / Radx::LIGHT_SPEED) * 1.0e6 * 2.0;
    bandwidthMhz = _ddParms[0].recvr_bandwidth;
    _readVol->setRadarReceiverBandwidthMhz(bandwidthMhz);
  }
  
  // for each ray, find the maximum number of samples in any field
  // from the sampling ratio
  // also set calib index

  for (size_t ii = 0; ii < _rays.size(); ii++) {
    RadxRay *ray = _rays[ii];
    ray->setCalibIndex(0);
    int maxNSamples = 0;
    vector<RadxField *> fields = ray->getFields();
    for (size_t jj = 0; jj < fields.size(); jj++) {
      int nSamples = (int) (fields[jj]->getSamplingRatio() + 0.5);
      if (nSamples > maxNSamples) maxNSamples = nSamples;
    }
    ray->setNSamples(maxNSamples);
    for (size_t jj = 0; jj < fields.size(); jj++) {
      double samplingRatio = 
        fields[jj]->getSamplingRatio() / (double) maxNSamples;
      fields[jj]->setSamplingRatio(samplingRatio);
    }
  }
  
  // add rays to volume
  // responsibility for management of ray memory passes from 
  // this object to the volume

  for (size_t ii = 0; ii < _rays.size(); ii++) {
    _readVol->addRay(_rays[ii]);
    // get pulse width from rays
    pulseWidthUs = _rays[ii]->getPulseWidthUsec();
  }

  // memory allocation for rays has passed to _readVol,
  // so free up pointer array
  
  _rays.clear();

  // set max range

  if (_readSetMaxRange) {
    _readVol->setMaxRangeKm(_readMaxRangeKm);
  }
  
  // remove rays with all missing data, if requested
  
  if (_readRemoveRaysAllMissing) {
    _readVol->removeRaysWithDataAllMissing();
  }

  // add calibration information

  RadxRcalib *cal = new RadxRcalib;

  cal->setCalibTime(_readVol->getStartTimeSecs());
  if (_ddRadar.pulse_width <= 0) {
    cal->setPulseWidthUsec(pulseWidthUs);
  } else {
    cal->setPulseWidthUsec(_ddRadar.pulse_width);
  }
  double xmitPowerDbm = Radx::missingMetaDouble;
  if (_ddRadar.peak_power > 0) {
    xmitPowerDbm = 10.0 * log10(_ddRadar.peak_power * 1.0e6);
  }
  cal->setXmitPowerDbmH(xmitPowerDbm);
  cal->setXmitPowerDbmV(xmitPowerDbm);
  cal->setNoiseDbmHc(_ddRadar.noise_power);
  cal->setNoiseDbmVc(_ddRadar.noise_power);
  cal->setReceiverGainDbHc(_ddRadar.receiver_gain);
  cal->setReceiverGainDbVc(_ddRadar.receiver_gain);
  cal->setRadarConstantH(_ddRadar.radar_const);
  cal->setRadarConstantV(_ddRadar.radar_const);
  double waveguideLoss = _ddRadar.antenna_gain - _ddRadar.system_gain;
  cal->setTwoWayWaveguideLossDbH(waveguideLoss);
  cal->setTwoWayWaveguideLossDbV(waveguideLoss);

  _readVol->addCalib(cal);

  // add correction factors

  if (DoradeData::isValid(_ddCfac)) {
    RadxCfactors cfac;
    cfac.setAzimuthCorr(_ddCfac.azimuth_corr);
    cfac.setElevationCorr(_ddCfac.elevation_corr);
    cfac.setRangeCorr(_ddCfac.range_delay_corr);
    cfac.setLongitudeCorr(_ddCfac.longitude_corr);
    cfac.setLatitudeCorr(_ddCfac.latitude_corr);
    cfac.setPressureAltCorr(_ddCfac.pressure_alt_corr);
    cfac.setAltitudeCorr(_ddCfac.radar_alt_corr);
    cfac.setEwVelCorr(_ddCfac.ew_gndspd_corr);
    cfac.setNsVelCorr(_ddCfac.ns_gndspd_corr);
    cfac.setVertVelCorr(_ddCfac.vert_vel_corr);
    cfac.setHeadingCorr(_ddCfac.heading_corr);
    cfac.setRollCorr(_ddCfac.roll_corr);
    cfac.setPitchCorr(_ddCfac.pitch_corr);
    cfac.setDriftCorr(_ddCfac.drift_corr);
    cfac.setRotationCorr(_ddCfac.rot_angle_corr);
    cfac.setTiltCorr(_ddCfac.tilt_corr);
    _readVol->setCfactors(cfac);
  }

  // apply goeref info if applicable

  if (_readApplyGeorefs) {
    _readVol->applyGeorefs();
  }

  // load the sweep info from rays
  
  _readVol->loadSweepInfoFromRays();
  
  // constrain the sweep data as appropriate
  
  if (_readFixedAngleLimitsSet) {
    if (_readVol->constrainByFixedAngle(_readMinFixedAngle, _readMaxFixedAngle,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - DoradeRadxFile::_loadReadVolume");
      _addErrStr("  No data found within fixed angle limits");
      _addErrDbl("  min fixed angle: ", _readMinFixedAngle);
      _addErrDbl("  max fixed angle: ", _readMaxFixedAngle);
      return -1;
    }
  } else if (_readSweepNumLimitsSet) {
    if (_readVol->constrainBySweepNum(_readMinSweepNum, _readMaxSweepNum,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - DoradeRadxFile::_loadReadVolume");
      _addErrStr("  No data found within sweep num limits");
      _addErrInt("  min sweep num: ", _readMinSweepNum);
      _addErrInt("  max sweep num: ", _readMaxSweepNum);
      return -1;
    }
  }

  // set target scan rate

  _readVol->setTargetScanRateDegPerSec(_ddRadar.req_rotat_vel);

  // check for indexed rays, set info on rays
  
  _readVol->checkForIndexedRays();
  
  // load up volume info from rays

  _readVol->loadVolumeInfoFromRays();
  
  return 0;

}

/////////////////////////////////////////////////////////
// print object

void DoradeRadxFile::print(ostream &out) const
  
{
  
  out << "=========== DoradeRadxFile summary ===========" << endl;
  RadxFile::print(out);
  if (DoradeData::isValid(_ddComment)) DoradeData::print(_ddComment, out);
  if (DoradeData::isValid(_ddSwib)) DoradeData::print(_ddSwib, out);
  if (DoradeData::isValid(_ddVol)) DoradeData::print(_ddVol, out);
  if (DoradeData::isValid(_ddRadar)) DoradeData::print(_ddRadar, out);
  if (DoradeData::isValid(_ddCfac)) DoradeData::print(_ddCfac, out);
  for (int ii = 0; ii < (int) _ddParms.size(); ii++) {
    DoradeData::print(_ddParms[ii], out);
  }
  if (DoradeData::isValid(_ddCellv)) DoradeData::print(_ddCellv, out);
  if (DoradeData::isValid(_ddCsfp)) DoradeData::print(_ddCsfp, out);
  if (DoradeData::isValid(_ddSweep)) DoradeData::print(_ddSweep, out);
  if (DoradeData::isValid(_ddPlat)) DoradeData::print(_ddPlat, out);
  if (DoradeData::isValid(_ddExtra)) DoradeData::print(_ddExtra, out);
  if (DoradeData::isValid(_ddRotTable)) DoradeData::print(_ddRotTable, out);
  out << "===========================================" << endl;

}

////////////////////////////////////////////////////////////
// Print data in nantive dorade format
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int DoradeRadxFile::printNative(const string &path, ostream &out,
                                bool printRays, bool printData)
  
{

  clear();
  _ddParms.clear();
  
  // is this a Dorade file?

  if (!isDorade(path)) {
    _addErrStr("ERROR - DoradeRadxFile::printNative");
    _addErrStr("  Not a dorade file: ", path);
    return -1;
  }

  // check if data in this file is swapped - this sets _ddIsSwapped
  
  if (checkIsSwapped(path)) {
    _addErrStr("ERROR - DoradeRadxFile::printNative");
    _addErrStr("  Cannot check if swapped: ", path);
    return -1;
  }

  if (isSwapped()) {
    out << " file is byte-swapped" << endl;
  }

  // open file

  if (_openRead(path)) {
    _addErrStr("ERROR - DoradeRadxFile::printNative");
    return -1;
  }

  // read through file, finding data blocks

  while (!feof(_file)) {

    // read ID
    
    char id[8];
    memset(id, 0, 8);
    
    if (fread(id, 1, 4, _file) != 4) {
      if (feof(_file)) {
        // done
        break;
      }
      int errNum = errno;
      _addErrStr("ERROR - DoradeRadxFile::printNative");
      _addErrStr("  Cannot read id string");
      _addErrStr(strerror(errNum));
      _close();
      return -1;
    }
    string idStr(id);

    // read nbytes

    Radx::si32 nBytes;
    if (fread(&nBytes, 4, 1, _file) != 1) {
      int errNum = errno;
      _addErrStr("ERROR - DoradeRadxFile::printNative()");
      _addErrStr("  Cannot read data length");
      _addErrStr("  ID: ", id);
      _addErrStr("  File path: ", path);
      _addErrStr(strerror(errNum));
      _close();
      return -1;
    }

    // swap as needed

    if (_ddIsSwapped) {
      ByteOrder::swap32(&nBytes, 4, true);
    }

    // check for reasonableness

    if (nBytes < 8 || nBytes > 1000000) {
      _addErrStr("ERROR - DoradeRadxFile::printNative()");
      _addErrStr("  Bad number of bytes");
      _addErrStr("  ID: ", id);
      _addErrInt("  nBytes: ", nBytes);
      _addErrStr("  File path: ", path);
      _close();
      return -1;
    }

    // print out id and data length

    if (_verbose) {
      cerr << "ID, nBytes: " << id << ", " << nBytes << endl;
    }
      
    // allocate memory for block

    char *block = new char[nBytes];
    
    // back up 8 bytes
    
    long nSeek = -8;
    if (fseek(_file, nSeek, SEEK_CUR)) {
      int errNum = errno;
      _addErrStr("ERROR - DoradeRadxFile::printNative()");
      _addErrStr("  Cannot seek back to start of block");
      _addErrStr("  ID: ", id);
      _addErrInt("  len: ", nBytes);
      _addErrStr("  File path: ", path);
      _addErrStr(strerror(errNum));
      delete[] block;
      _close();
      return -1;
    }

    // read in block
    
    if (fread(block, 1, nBytes, _file) != (size_t) nBytes) {
      int errNum = errno;
      _addErrStr("ERROR - DoradeRadxFile::printNative()");
      _addErrStr("  Cannot read data block");
      _addErrStr("  ID: ", id);
      _addErrInt("  len: ", nBytes);
      _addErrStr("  File path: ", path);
      _addErrStr(strerror(errNum));
      delete[] block;
      _close();
      return -1;
    }

    // print block

    if (_printBlock(idStr, nBytes, block, out, printRays, printData)) {
      _addErrStr("ERROR - DoradeRadxFile::printNative()");
      _addErrStr("  Cannot print data block");
      _addErrStr("  ID: " , id);
      _addErrInt("  len: ", nBytes);
      _addErrStr("  File path: ", path);
      delete[] block;
      _close();
      return -1;
    }

    // free up block
    
    delete[] block;
    
  } // while (!feof(_file)

  // close file

  _close();

  return 0;

}

///////////////////////////////////////////////////////
// print the data block

int DoradeRadxFile::_printBlock(const string &idStr, int nBytes,
                                const char *block, ostream &out,
                                bool printRays, bool printData)
  
{

  if (idStr == "COMM") {

    int nn = sizeof(DoradeData::comment_t);
    memset(&_ddComment, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddComment, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddComment, true);
    DoradeData::print(_ddComment, out);

  } else if (idStr == "SSWB") {

    if (nBytes >= (int) sizeof(DoradeData::super_SWIB_t)) {
      // 8-byte aligned version
      DoradeData::init(_ddSwib);
      int nn = sizeof(DoradeData::super_SWIB_t);
      if (nn > nBytes) nn = nBytes;
      memcpy(&_ddSwib, block, nn);
      if (_ddIsSwapped) DoradeData::swap(_ddSwib, true);
      DoradeData::print(_ddSwib, out);
    } else {
      // 4-byte aligned version
      int nn = sizeof(DoradeData::super_SWIB_32bit_t);
      DoradeData::super_SWIB_32bit_t swib32;
      DoradeData::init(swib32);
      if (nn > nBytes) nn = nBytes;
      memcpy(&swib32, block, nn);
      if (_ddIsSwapped) DoradeData::swap(swib32, true);
      DoradeData::print(swib32, out);
    }

  } else if (idStr == "VOLD") {

    int nn = sizeof(DoradeData::volume_t);
    memset(&_ddVol, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddVol, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddVol, true);
    // Time fields may need swapping even if rest of vol does not.
    DoradeData::swapTimeToReasonable(_ddVol); 
    DoradeData::print(_ddVol, out);

  } else if (idStr == "RADD") {

    int nn = sizeof(DoradeData::radar_t);
    memset(&_ddRadar, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddRadar, block, nn);
    if (nn >= (int) sizeof(DoradeData::radar_t)) {
      _radarIsOldVersion = false;
    } else {
      _radarIsOldVersion = true;
    }
    if (_ddIsSwapped) DoradeData::swap(_ddRadar, true);
    DoradeData::print(_ddRadar, out);
    _radarName = Radx::makeString(_ddRadar.radar_name, 8);

  } else if (idStr == "FRAD") {

    int nn = sizeof(DoradeData::radar_test_status_t);
    memset(&_ddRadarTestStatus, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddRadarTestStatus, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddRadarTestStatus, true);
    DoradeData::print(_ddRadarTestStatus, out);

  } else if (idStr == "FRIB") {

    int nn = sizeof(DoradeData::field_radar_t);
    memset(&_ddFieldRadar, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddFieldRadar, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddFieldRadar, true);
    DoradeData::print(_ddFieldRadar, out);

  } else if (idStr == "LIDR") {

    int nn = sizeof(DoradeData::lidar_t);
    memset(&_ddLidar, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddLidar, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddLidar, true);
    DoradeData::print(_ddLidar, out);

  } else if (idStr == "FLIB") {

    int nn = sizeof(DoradeData::field_lidar_t);
    memset(&_ddFieldLidar, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddFieldLidar, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddFieldLidar, true);
    DoradeData::print(_ddFieldLidar, out);

  } else if (idStr == "CFAC") {
    
    int nn = sizeof(DoradeData::correction_t);
    memset(&_ddCfac, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddCfac, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddCfac, true);
    DoradeData::print(_ddCfac, out);
    
  } else if (idStr == "PARM") {

    int nn = sizeof(DoradeData::parameter_t);
    if (nn > nBytes) nn = nBytes;
    DoradeData::parameter_t parm;
    memset(&parm, 0, sizeof(parm));
    memcpy(&parm, block, nn);
    if (nn >= (int) sizeof(DoradeData::parameter_t)) {
      _parmIsOldVersion = false;
    } else {
      _parmIsOldVersion = true;
    }
    if (_ddIsSwapped) DoradeData::swap(parm, true);
    DoradeData::print(parm, out);

    // add to field list if not already there
    bool alreadyStored = false;
    string newName(Radx::makeString(parm.parameter_name, 8));
    for (int ii = 0; ii < (int) _ddParms.size(); ii++) {
      string name(Radx::makeString(_ddParms[ii].parameter_name, 8));
      if (name == newName) {
        alreadyStored = true;
        break;
      }
    }
    if (!alreadyStored) {
      _ddParms.push_back(parm);
    }
    
  } else if (idStr == "CELV") {
    
    int nn = sizeof(DoradeData::cell_vector_t);
    memset(&_ddCellv, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddCellv, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddCellv, true);
    DoradeData::print(_ddCellv, out);

    _nGatesIn = _ddCellv.number_cells;

  } else if (idStr == "CSFD") {
    
    int nn = sizeof(DoradeData::cell_spacing_fp_t);
    memset(&_ddCsfp, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddCsfp, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddCsfp, true);
    DoradeData::print(_ddCsfp, out);

    _nGatesIn = 0;
    int ns = _ddCsfp.num_segments;
    if (ns > 8) ns = 8;
    for (int ii = 0; ii < ns; ii++) {
      _nGatesIn += _ddCsfp.num_cells[ii];
    }

  } else if (idStr == "SWIB") {

    int nn = sizeof(DoradeData::sweepinfo_t);
    memset(&_ddSweep, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddSweep, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddSweep, true);
    DoradeData::print(_ddSweep, out);
    
  } else if (idStr == "ASIB") {

    int nn = sizeof(DoradeData::platform_t);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddPlat, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddPlat, true);
    DoradeData::print(_ddPlat, out);

  } else if (idStr == "RYIB") {
    
    int nn = sizeof(DoradeData::ray_t);
    memset(&_ddRay, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddRay, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddRay, true);
    DoradeData::print(_ddRay, out);

  } else if (idStr == "RDAT") {

    int nn = sizeof(DoradeData::paramdata_t);
    memset(&_ddPdat, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddPdat, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddPdat, true);
    if (printRays) DoradeData::print(_ddPdat, out);
    
    // print field if parameters are available
    if (printData) {
      string fieldName(Radx::makeString(_ddPdat.pdata_name, 8));
      for (int ii = 0; ii < (int) _ddParms.size(); ii++) {
        string name(Radx::makeString(_ddParms[ii].parameter_name, 8));
        if (name == fieldName) {
          if (_printField(_ddParms[ii], _ddPdat.nbytes,
                          sizeof(_ddPdat), block, out)) {
            return -1;
          }
          break;
        }
      }
    }

  } else if (idStr == "QDAT") {
    
    int nn = sizeof(DoradeData::qparamdata_t);
    memset(&_ddQdat, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddQdat, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddQdat, true);
    if (printRays) DoradeData::print(_ddQdat, out);

    // print field if parameters are available
    if (printData) {
      string fieldName(Radx::makeString(_ddQdat.pdata_name, 8));
      for (int ii = 0; ii < (int) _ddParms.size(); ii++) {
        string name(Radx::makeString(_ddParms[ii].parameter_name, 8));
        if (name == fieldName) {
          if (_printField(_ddParms[ii], _ddQdat.nbytes,
                          sizeof(_ddQdat), block, out)) {
            return -1;
          }
          break;
        }
      }
    }

  } else if (idStr == "XSIF") {
    
    int nn = sizeof(DoradeData::extra_stuff_t);
    memset(&_ddExtra, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddExtra, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddExtra, true);
    DoradeData::print(_ddExtra, out);
    
  } else if (idStr == "NULL") {
    
    int nn = sizeof(DoradeData::null_block_t);
    DoradeData::null_block_t nullBlock;
    memset(&nullBlock, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&nullBlock, block, nn);
    if (_ddIsSwapped) DoradeData::swap(nullBlock, true);
    DoradeData::print(nullBlock, out);
    
  } else if (idStr == "RKTB") {
    
    int nn = sizeof(DoradeData::rot_angle_table_t);
    memset(&_ddRotTable, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddRotTable, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddRotTable, true);
    DoradeData::print(_ddRotTable, out);
    
    int angleOffset = _ddRotTable.angle_table_offset;
    int nIndex = _ddRotTable.ndx_que_size;
    int nBytesIndex = nIndex * sizeof(Radx::si32);
    if (nBytesIndex + angleOffset > nBytes) {
      nBytesIndex = nBytes - angleOffset;
      nIndex = nBytesIndex / sizeof(Radx::si32);
    }
    Radx::si32 *angleIndex = new Radx::si32[nIndex];
    memcpy(angleIndex, block + angleOffset, nBytesIndex);
    if (_ddIsSwapped) ByteOrder::swap32(angleIndex, nBytesIndex, true);
    out << "============= Angle Index ==============" << endl;
    for (int ii = 0; ii < nIndex; ii++) {
      out << angleIndex[ii];
      if (ii != nIndex - 1) {
        out << " ";
      }
    }
    out << endl;
    out << "========================================" << endl;
    delete[] angleIndex;

    int keyOffset = _ddRotTable.first_key_offset;
    for (int ii = 0; ii < _ddRotTable.num_rays;
         ii++, keyOffset += sizeof(DoradeData::rot_table_entry_t)) {
      if (nBytes - keyOffset < (int) sizeof(DoradeData::rot_table_entry_t)) {
        break;
      }
      DoradeData::rot_table_entry_t entry;
      memcpy(&entry, block + keyOffset, sizeof(entry));
      if (_ddIsSwapped) DoradeData::swap(entry, true);
      DoradeData::print(entry, out);
    }

  } else if (idStr == "SEDS") {

    string sedsStr = Radx::makeString(block + 8, nBytes - 8);
    out << "============= SEDS ==============" << endl;
    out << sedsStr << endl;
    out << "==================================" << endl;
    
  } else if (idStr == "SITU") {

    int nn = sizeof(DoradeData::insitu_descript_t);
    memset(&_ddInsituDescript, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddInsituDescript, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddInsituDescript, true);
    DoradeData::print(_ddInsituDescript, out);

  } else if (idStr == "ISIT") {

    int nn = sizeof(DoradeData::insitu_data_t);
    memset(&_ddInsituData, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddInsituData, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddInsituData, true);
    DoradeData::print(_ddInsituData, out);

  } else if (idStr == "INDF") {

    int nn = sizeof(DoradeData::indep_freq_t);
    memset(&_ddIndepFreq, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddIndepFreq, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddIndepFreq, true);
    DoradeData::print(_ddIndepFreq, out);

  } else if (idStr == "MINI") {

    int nn = sizeof(DoradeData::minirims_data_t);
    memset(&_ddMinirimsData, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddMinirimsData, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddMinirimsData, true);
    DoradeData::print(_ddMinirimsData, out);

  } else if (idStr == "NDDS") {

    int nn = sizeof(DoradeData::nav_descript_t);
    memset(&_ddNavDescript, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddNavDescript, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddNavDescript, true);
    DoradeData::print(_ddNavDescript, out);

  } else if (idStr == "TIME") {

    int nn = sizeof(DoradeData::time_series_t);
    memset(&_ddTimeSeries, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddTimeSeries, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddTimeSeries, true);
    DoradeData::print(_ddTimeSeries, out);

  } else if (idStr == "WAVE") {

    int nn = sizeof(DoradeData::waveform_t);
    memset(&_ddWaveform, 0, nn);
    if (nn > nBytes) nn = nBytes;
    memcpy(&_ddWaveform, block, nn);
    if (_ddIsSwapped) DoradeData::swap(_ddWaveform, true);
    DoradeData::print(_ddWaveform, out);

  }

  return 0;

}

///////////////////////////////////////////////////////
// print a data field

int DoradeRadxFile::_printField(const DoradeData::parameter_t &parm,
                                int nBytesBlock,
                                int minOffset,
                                const char *block,
                                ostream &out)

{

  DoradeData::binary_format_t format =
    (DoradeData::binary_format_t) parm.binary_format;

  // special case for 16-bit data - may be compressed

  if (format == DoradeData::BINARY_FORMAT_INT16) {
    return _printField16(parm, nBytesBlock, minOffset, block, out);
  }

  int byteWidth = 1;
  if (format == DoradeData::BINARY_FORMAT_FLOAT32) {
    byteWidth = 4;
  } else if (format == DoradeData::BINARY_FORMAT_INT32) {
    byteWidth = 4;
  }

  int offsetToData = parm.offset_to_data;
  if (offsetToData < minOffset) {
    offsetToData = minOffset;
  }

  const char *data = block + offsetToData;
  int nBytesData = nBytesBlock - offsetToData;
  int nGatesData = nBytesData / byteWidth;
  
  if (nGatesData > _nGatesIn) {
    _addErrStr("ERROR - DoradeRadxFile::_printField");
    _addErrStr("  Too much data for number of gates");
    _addErrStr("  Field: ", Radx::makeString(parm.parameter_name, 8));
    _addErrInt("  nGates in headers: ", _nGatesIn);
    _addErrInt("  nGates in data: ", nGatesData);
    if (_debug) {
      cerr << _errStr << endl;
    }
    return -1;
  }

  if (nGatesData > _nGatesIn) {
    int nGatesTooMany = nGatesData - _nGatesIn;
    int nBytesTooMany = nGatesTooMany * byteWidth;
    offsetToData += nBytesTooMany;
    data = block + offsetToData;
    nBytesData = nBytesBlock - offsetToData;
    nGatesData = nBytesData / byteWidth;
  }

  if (nGatesData <= 0) {
    // nothing to print
    return 0;
  }
  
  // load data for printing

  double *ddata = new double[nGatesData];

  if (format == DoradeData::BINARY_FORMAT_FLOAT32) {
    
    Radx::fl32 *dptr = new Radx::fl32[nGatesData];
    memcpy(dptr, data, nBytesData);
    if (_ddIsSwapped) ByteOrder::swap32(dptr, nBytesData);
    for (int ii = 0; ii < nGatesData; ii++) {
      if (dptr[ii] == (Radx::fl32) parm.bad_data) {
        ddata[ii] = Radx::missingFl64;
      } else {
        ddata[ii] = dptr[ii];
      }
    }
    delete[] dptr;
          
  } else if (format == DoradeData::BINARY_FORMAT_INT32) {
    
    Radx::si32 *dptr = new Radx::si32[nGatesData];
    memcpy(dptr, data, nBytesData);
    if (_ddIsSwapped) ByteOrder::swap32(dptr, nBytesData);
    double scale = parm.parameter_scale;
    double bias = parm.parameter_bias;
    for (int ii = 0; ii < nGatesData; ii++) {
      double val = (dptr[ii] - bias) / scale;
      if (dptr[ii] == parm.bad_data) {
        ddata[ii] = Radx::missingFl64;
      } else {
        ddata[ii] = val;
      }
    }
    delete[] dptr;
    
  } else if (format == DoradeData::BINARY_FORMAT_INT16) {
    
    Radx::si16 *dptr = new Radx::si16[nGatesData];
    memcpy(dptr, data, nBytesData);
    if (_ddIsSwapped) ByteOrder::swap16(dptr, nBytesData);
    double scale = parm.parameter_scale;
    double bias = parm.parameter_bias;
    for (int ii = 0; ii < nGatesData; ii++) {
      double val = (dptr[ii] - bias) / scale;
      if (dptr[ii] == parm.bad_data) {
        ddata[ii] = Radx::missingFl64;
      } else {
        ddata[ii] = val;
      }
    }
    delete[] dptr;
    
  } else {
    
    // int8 data
    
    double scale = parm.parameter_scale;
    double bias = parm.parameter_bias;
    for (int ii = 0; ii < nGatesData; ii++) {
      double val = (data[ii] - bias) / scale;
      if (data[ii] == parm.bad_data) {
        ddata[ii] = Radx::missingFl64;
      } else {
        ddata[ii] = val;
      }
    }
    
  }

  // print data

  _printFieldData(out, nGatesData, ddata);

  // free up

  delete[] ddata;
  
  return 0;

}

///////////////////////////////////////////////////////
// print a 16-bit data field, handling uncompression

int DoradeRadxFile::_printField16(const DoradeData::parameter_t &parm,
                                  int nBytesBlock,
                                  int minOffset,
                                  const char *block,
                                  ostream &out)
  
{

  DoradeData::binary_format_t format =
    (DoradeData::binary_format_t) parm.binary_format;
  if (format != DoradeData::BINARY_FORMAT_INT16) {
    cerr << "ERROR - DoradeRadxFile::_printField16" << endl;
    cerr << "  Not 16-bit data" << endl;
    return -1;
  }
  int byteWidth = 2;

  // get offset into data
  // old headers may have offset set to 0
  
  int offsetToData = parm.offset_to_data;
  if (offsetToData < minOffset) {
    offsetToData = minOffset;
  }
  
  // find data

  const char *data = block + offsetToData;
  int nBytesData = nBytesBlock - offsetToData;

  // load up raw buffer, and byte-swap as appropriate
  
  int nGatesRaw = nBytesData / byteWidth;
  unsigned short *raw = new unsigned short[nGatesRaw];
  memcpy(raw, data, nBytesData);
  if (_ddIsSwapped) ByteOrder::swap16(raw, nBytesData);

  // uncompress are required

  short *uncompressed = new short[_nGatesIn];
  int nGatesData = 0;
  if (_ddRadar.data_compress == DoradeData::COMPRESSION_HRD) {
    // compressed, so uncompress
    int nbads;
    nGatesData = DoradeData::decompressHrd16(raw, nGatesRaw,
                                             (unsigned short *) uncompressed,
                                             _nGatesIn, parm.bad_data, &nbads);
  } else {
    // not compressed, just copy
    if (nGatesRaw < _nGatesIn) {
      nGatesData = nGatesRaw;
    } else {
      nGatesData = _nGatesIn;
    }
    memcpy(uncompressed, raw, nGatesData * byteWidth);
  }
  delete[] raw;
  
  if (nGatesData <= 0) {
    // nothing to print
    delete[] uncompressed;
    return 0;
  }
  
  // load data for printing

  double *ddata = new double[nGatesData];
  double scale = parm.parameter_scale;
  double bias = parm.parameter_bias;
  for (int ii = 0; ii < nGatesData; ii++) {
    if (uncompressed[ii] == parm.bad_data) {
      ddata[ii] = Radx::missingFl64;
    } else {
      ddata[ii] = (uncompressed[ii] - bias) / scale;
    }
  }

  // print data

  _printFieldData(out, nGatesData, ddata);

  // free up

  delete[] ddata;
  delete[] uncompressed;
  
  return 0;

}

///////////////////////////////
// print rotation angle table

int DoradeRadxFile::_printRotTable(const DoradeData::key_table_info_t &info,
                                   ostream &out)

{

  // seek to key table location
  
  if (fseek(_file, info.offset, SEEK_SET)) {
    int errNum = errno;
    _addErrStr("ERROR - DoradeRadxFile::printRotTable()");
    _addErrInt("  Cannot seek to offset: ", info.offset);
    _addErrStr(strerror(errNum));
    return -1;
  }

  // read in table

  memset(&_ddRotTable, 0, sizeof(_ddRotTable));
  if (fread(&_ddRotTable, sizeof(_ddRotTable), 1, _file) != 1) {
    int errNum = errno;
    _addErrStr("ERROR - DoradeRadxFile::printRotTable()");
    _addErrStr("  Cannot read table");
    _addErrInt("  offset: ", info.offset);
    _addErrInt("  size: ", sizeof(_ddRotTable));
    _addErrStr(strerror(errNum));
    return -1;
  }
  
  // swap as needed, then print
  
  if (_ddIsSwapped) DoradeData::swap(_ddRotTable, true);
  DoradeData::print(_ddRotTable, out);

  return 0;

}

/////////////////////////////////////////////////////////
// print field data

void DoradeRadxFile::_printFieldData(ostream &out, int nGates, const double *data) const
  
{

  out << "============== Field data =================" << endl;
  if (nGates == 0) {
    out << "========= currently no data =========" << endl;
  } else {
    int printed = 0;
    int count = 1;
    double prevVal = data[0];
    for (int ii = 1; ii < nGates; ii++) {
      double dval = data[ii];
      if (dval != prevVal) {
        _printPacked(out, count, prevVal);
        printed++;
        if (printed > 6) {
          out << endl;
          printed = 0;
        }
        prevVal = dval;
        count = 1;
      } else {
        count++;
      }
    } // ii
    _printPacked(out, count, prevVal);
    out << endl;
  }
  out << "===========================================" << endl;

}

/////////////////////////////////////////////////////////////////
// print in packed format, using count for identical data values

void DoradeRadxFile::_printPacked(ostream &out, int count, double val) const

{
  
  char outstr[1024];
  if (count > 1) {
    out << count << "*";
  }
  if (val == Radx::missingFl64) {
    out << "MISS ";
  } else {
    if (fabs(val) > 0.01) {
      sprintf(outstr, "%.3f ", val);
      out << outstr;
    } else if (val == 0.0) {
      out << "0.0 ";
    } else {
      sprintf(outstr, "%.3e ", val);
      out << outstr;
    }
  }
}

////////////////////////////////////////////////////////////
// Write comment block
// Returns 0 on success, -1 on failure

int DoradeRadxFile::_writeComment()
  
{

  // fill

  DoradeData::init(_ddComment);
  string commStr = _writeVol->getComment();
  commStr.resize(499);
  strncpy(_ddComment.comment, commStr.c_str(), 499);
  if (strlen(_ddComment.comment) == 0) {
    sprintf(_ddComment.comment, "%s",
            "Written by DoradeRadxFile object");
  }

  // byte swap as needed

  DoradeData::comment_t copy = _ddComment;
  if (!_writeNativeByteOrder) {
    DoradeData::swap(copy);
  }

  // write

  if (fwrite(&copy, sizeof(copy), 1, _file) != 1) {
    _addErrStr("ERROR - DoradeRadxFile::_writeComment()");
    _addErrStr("  Cannot write comment block");
    _addErrStr("  file path: ", _pathInUse);
    _addErrStr(strerror(errno));
    return -1;
  }
  return 0;

}

////////////////////////////////////////////////////////////
// Write super sweep info block
// Returns 0 on success, -1 on failure

int DoradeRadxFile::_writeSuperSwib(int fileSize)
  
{

  // check environment for extended paths
  bool write32Bit = false;
  char *dorade32BitWriteStr = getenv("DORADE_WRITE_32BIT_SWIB");
  if (dorade32BitWriteStr != NULL) {
    if (!strcasecmp(dorade32BitWriteStr, "TRUE")) {
      write32Bit = true;
    }
  }

  // fill

  DoradeData::init(_ddSwib);
  _ddSwib.last_used = time(NULL);
  _ddSwib.start_time = _writeVol->getStartTimeSecs();
  _ddSwib.stop_time = _writeVol->getEndTimeSecs();
  _ddSwib.sizeof_file = fileSize;
  _ddSwib.compression_flag = 0;
  _ddSwib.volume_time_stamp = _writeVol->getStartTimeSecs();
  _ddSwib.num_params = _writeVol->getFields().size();

  strncpy(_ddSwib.radar_name, _writeVol->getInstrumentName().c_str(), 8);
  double delta_start = _writeVol->getStartNanoSecs() / 1.0e9;
  double delta_stop = _writeVol->getEndNanoSecs() / 1.0e9;

  // if the start or stop time is less than the precision we can report, 
  // set the time to zero.  
  if ((delta_start > 1.0) || (delta_start < 0.0)) delta_start = 0.0;
  if ((delta_stop  > 1.0) || (delta_stop  < 0.0)) delta_stop  = 0.0;

  _ddSwib.d_start_time = _ddSwib.start_time + delta_start;
  _ddSwib.d_stop_time = _ddSwib.stop_time + delta_stop;;

  int numKeys = 0;
  if (_rotationTableSize > 0) {
    _ddSwib.key_table[numKeys].offset = _rotationTableOffset;
    _ddSwib.key_table[numKeys].size = _rotationTableSize;
    _ddSwib.key_table[numKeys].type = DoradeData::KEYED_BY_ROT_ANG;
    numKeys++;
  }
  if (_sedsBlockSize > 0) {
    _ddSwib.key_table[numKeys].offset = _sedsBlockOffset;
    _ddSwib.key_table[numKeys].size = _sedsBlockSize;
    _ddSwib.key_table[numKeys].type = DoradeData::SOLO_EDIT_SUMMARY;
    numKeys++;
  }
  _ddSwib.num_key_tables = numKeys;

  // byte swap as needed
  
  if (write32Bit) {
    DoradeData::super_SWIB_t copy64 = _ddSwib;
    DoradeData::super_SWIB_32bit_t copy;
    //  Ah, but we need to convert 64 bit to 32 bit structure
    // and change the number of bytes as well.
    int byteAdjustment = sizeof(_ddSwib.pad);
    copy64.sizeof_file -= byteAdjustment; // 4;
    copy64.nbytes -= byteAdjustment; // 4;
    DoradeData::copy(copy64, copy);
    if (!_writeNativeByteOrder) {
      DoradeData::swap(copy);
    }

    // write                                                                                                                   
    if (fwrite(&copy, sizeof(copy), 1, _file) != 1) {
      _addErrStr("ERROR - DoradeRadxFile::_writeSuperSwib()");
      _addErrStr("  Cannot write super sweep block 32");
      _addErrStr("  file path: ", _pathInUse);
      _addErrStr(strerror(errno));
      return -1;
    }
  } else {
    DoradeData::super_SWIB_t copy = _ddSwib;
 
    if (!_writeNativeByteOrder) {
      DoradeData::swap(copy);
    }

    // write

    if (fwrite(&copy, sizeof(copy), 1, _file) != 1) {
      _addErrStr("ERROR - DoradeRadxFile::_writeSuperSwib()");
      _addErrStr("  Cannot write super sweep block");
      _addErrStr("  file path: ", _pathInUse);
      _addErrStr(strerror(errno));
      return -1;
    }
  }
  return 0;

}

////////////////////////////////////////////////////////////
// Write volume block
// Returns 0 on success, -1 on failure

int DoradeRadxFile::_writeVolume()
  
{

  // fill

  DoradeData::init(_ddVol);

  _ddVol.volume_num = _writeVol->getVolumeNumber();
  _ddVol.maximum_bytes = (_writeVol->getNPoints() * sizeof(Radx::fl32) +
                          sizeof(DoradeData::paramdata_t));
  if (_ddVol.maximum_bytes < 32768) {
    _ddVol.maximum_bytes = 32768;
  }

  strncpy(_ddVol.proj_name, _writeVol->getTitle().c_str(), 20);
  strncpy(_ddVol.flight_num, _writeVol->getReferences().c_str(), 8);
  strncpy(_ddVol.gen_facility, _writeVol->getSource().c_str(), 8);

  RadxTime startTime(_writeVol->getStartTimeSecs());
  _ddVol.year = startTime.getYear();
  _ddVol.month = startTime.getMonth();
  _ddVol.day = startTime.getDay();
  _ddVol.data_set_hour = startTime.getHour();
  _ddVol.data_set_minute = startTime.getMin();
  _ddVol.data_set_second = startTime.getSec();

  RadxTime now(time(NULL));
  _ddVol.gen_year = now.getYear();
  _ddVol.gen_month = now.getMonth();
  _ddVol.gen_day = now.getDay();

  _ddVol.number_sensor_des = 1;

  // byte swap as needed

  DoradeData::volume_t copy = _ddVol;
  if (!_writeNativeByteOrder) {
    DoradeData::swap(copy);
  }

  // write

  if (fwrite(&copy, sizeof(copy), 1, _file) != 1) {
    _addErrStr("ERROR - DoradeRadxFile::_writeVolume()");
    _addErrStr("  Cannot write volume block");
    _addErrStr("  file path: ", _pathInUse);
    _addErrStr(strerror(errno));
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////
// Write radar info block
// Returns 0 on success, -1 on failure

int DoradeRadxFile::_writeRadar()
  
{

  if (_writeVol->getInstrumentType() != Radx::INSTRUMENT_TYPE_RADAR) {
    // method not applicable to radars
    return 0;
  }

  // fill

  DoradeData::init(_ddRadar);
  RadxRcalib cal;
  if (_writeVol->getRcalibs().size() > 0) {
    cal = *_writeVol->getRcalibs()[0];
  }

  strncpy(_ddRadar.radar_name, _writeVol->getInstrumentName().c_str(), 8);
  strncpy(_ddRadar.site_name, _writeVol->getSiteName().c_str(), 20);

  _ddRadar.radar_const = cal.getRadarConstantH();

  double xmitPowerDbm = cal.getXmitPowerDbmH();
  if (xmitPowerDbm < -999) {
    _ddRadar.peak_power = -999;
  } else {
    _ddRadar.peak_power = pow(10.0, xmitPowerDbm / 10.0) * 1.0e-6;
  }
  _ddRadar.noise_power = cal.getNoiseDbmHc();
  _ddRadar.receiver_gain = cal.getReceiverGainDbHc();
  _ddRadar.antenna_gain = _writeVol->getRadarAntennaGainDbH();
  _ddRadar.system_gain = _ddRadar.antenna_gain - cal.getTwoWayWaveguideLossDbH();
  _ddRadar.horz_beam_width = _writeVol->getRadarBeamWidthDegH();
  _ddRadar.vert_beam_width = _writeVol->getRadarBeamWidthDegV();
  _ddRadar.num_parameter_des = _writeVol->getFields().size();
  _ddRadar.total_num_des = _writeVol->getFields().size();
  if (_writeCompressed) {
    _ddRadar.data_compress = DoradeData::COMPRESSION_HRD;
  } else {
    _ddRadar.data_compress = DoradeData::COMPRESSION_NONE;
  }

  // set scan mode

  _ddRadar.scan_mode = DoradeData::SCAN_MODE_SUR;
  if (_writeVol->getSweeps().size() > 0) {
    const RadxSweep &sweep = *_writeVol->getSweeps()[0];
    switch (sweep.getSweepMode()) {
      case Radx::SWEEP_MODE_SECTOR:
        _ddRadar.scan_mode = DoradeData::SCAN_MODE_PPI;
        break;
      case Radx::SWEEP_MODE_COPLANE:
        _ddRadar.scan_mode = DoradeData::SCAN_MODE_COP;
        break;
      case Radx::SWEEP_MODE_RHI:
        _ddRadar.scan_mode = DoradeData::SCAN_MODE_RHI;
        break;
      case Radx::SWEEP_MODE_VERTICAL_POINTING:
        _ddRadar.scan_mode = DoradeData::SCAN_MODE_VER;
        break;
      case Radx::SWEEP_MODE_IDLE:
        _ddRadar.scan_mode = DoradeData::SCAN_MODE_IDL;
        break;
      case Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE:
        _ddRadar.scan_mode = DoradeData::SCAN_MODE_SUR;
        break;
      case Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE:
        _ddRadar.scan_mode = DoradeData::SCAN_MODE_AIR;
        break;
      case Radx::SWEEP_MODE_SUNSCAN:
        _ddRadar.scan_mode = DoradeData::SCAN_MODE_CAL;
        break;
      case Radx::SWEEP_MODE_POINTING:
        _ddRadar.scan_mode = DoradeData::SCAN_MODE_MAN;
        break;
      case Radx::SWEEP_MODE_CALIBRATION:
        _ddRadar.scan_mode = DoradeData::SCAN_MODE_CAL;
        break;
      case Radx::SWEEP_MODE_MANUAL_PPI:
      case Radx::SWEEP_MODE_MANUAL_RHI:
        _ddRadar.scan_mode = DoradeData::SCAN_MODE_MAN;
        break;
      default:
        _ddRadar.scan_mode = DoradeData::SCAN_MODE_SUR;
    }
    _ddRadar.req_rotat_vel = sweep.getTargetScanRateDegPerSec();
  }

  // set radar type
  // for aircraft radars override scan mode

  switch (_writeVol->getPlatformType()) {
    case Radx::PLATFORM_TYPE_FIXED:
    case Radx::PLATFORM_TYPE_VEHICLE:
      _ddRadar.radar_type = DoradeData::RADAR_GROUND;
      break;
    case Radx::PLATFORM_TYPE_SHIP:
      _ddRadar.radar_type = DoradeData::RADAR_SHIP;
      break;
    case Radx::PLATFORM_TYPE_AIRCRAFT_FORE:
      _ddRadar.radar_type = DoradeData::RADAR_AIR_FORE;
      _ddRadar.scan_mode = DoradeData::SCAN_MODE_AIR;
      break;
    case Radx::PLATFORM_TYPE_AIRCRAFT_AFT:
      _ddRadar.radar_type = DoradeData::RADAR_AIR_AFT;
      _ddRadar.scan_mode = DoradeData::SCAN_MODE_AIR;
      break;
    case Radx::PLATFORM_TYPE_AIRCRAFT_TAIL:
      _ddRadar.radar_type = DoradeData::RADAR_AIR_TAIL;
      _ddRadar.scan_mode = DoradeData::SCAN_MODE_AIR;
      break;
    case Radx::PLATFORM_TYPE_AIRCRAFT:
      _ddRadar.radar_type = DoradeData::RADAR_AIR_TAIL;
      _ddRadar.scan_mode = DoradeData::SCAN_MODE_AIR;
      break;
    case Radx::PLATFORM_TYPE_AIRCRAFT_BELLY:
    case Radx::PLATFORM_TYPE_AIRCRAFT_ROOF:
      _ddRadar.radar_type = DoradeData::RADAR_AIR_LF;
      break;
    case Radx::PLATFORM_TYPE_AIRCRAFT_NOSE:
      _ddRadar.radar_type = DoradeData::RADAR_AIR_NOSE;
      break;
    case Radx::PLATFORM_TYPE_SATELLITE_ORBIT:
    case Radx::PLATFORM_TYPE_SATELLITE_GEOSTAT:
      _ddRadar.radar_type = DoradeData::RADAR_SATELLITE;
      break;
    default:
      _ddRadar.radar_type = DoradeData::RADAR_GROUND;
  }
  
  _ddRadar.radar_longitude = _writeVol->getLongitudeDeg();
  _ddRadar.radar_latitude = _writeVol->getLatitudeDeg();
  _ddRadar.radar_altitude = _writeVol->getAltitudeKm();

  int nFreq = _writeVol->getFrequencyHz().size();
  _ddRadar.num_freq_trans = nFreq;
  if (nFreq > 0) {
    if (_writeVol->getFrequencyHz()[0] > 0) {
      _ddRadar.freq1 = _writeVol->getFrequencyHz()[0] / 1.0e9;
    }
    if (nFreq > 1) {
      if (_writeVol->getFrequencyHz()[1] > 0) {
        _ddRadar.freq2 = _writeVol->getFrequencyHz()[1] / 1.0e9;
      }
    }
    if (nFreq > 2) {
      if (_writeVol->getFrequencyHz()[2] > 0) {
        _ddRadar.freq3 = _writeVol->getFrequencyHz()[2] / 1.0e9;
      }
    }
    if (nFreq > 3) {
      if (_writeVol->getFrequencyHz()[3] > 0) {
        _ddRadar.freq4 = _writeVol->getFrequencyHz()[3] / 1.0e9;
      }
    }
    if (nFreq > 4) {
      if (_writeVol->getFrequencyHz()[4] > 0) {
        _ddRadar.freq5 = _writeVol->getFrequencyHz()[4] / 1.0e9;
      }
    }
  }

  _ddRadar.num_ipps_trans = 1;
  
  if (_writeVol->getRays().size() > 0) {
    const RadxRay &ray = *_writeVol->getRays()[0];
    _ddRadar.eff_unamb_vel = ray.getNyquistMps();
    _ddRadar.eff_unamb_range = ray.getUnambigRangeKm();
    _ddRadar.prt1 = 0;
    _ddRadar.prt2 = 0;
    _ddRadar.prt3 = 0;
    _ddRadar.prt4 = 0;
    _ddRadar.prt5 = 0;
    // check for missing data values
    double prt1 = ray.getPrtSec();
    if (prt1 != Radx::missingMetaDouble) {
      _ddRadar.prt1 = prt1 * 1000.0; // msecs
    }
    if (ray.getPrtMode() != Radx::PRT_MODE_FIXED) {
      _ddRadar.num_ipps_trans = 2;
      // check for missing data values
      double prt2 = ray.getPrtSec();
      if (prt2 != Radx::missingMetaDouble) {
        _ddRadar.prt2 = prt2 * 1000.0 / ray.getPrtRatio(); // msecs
      }
    }
    //    double pulseWidthUsec = ray.getPulseWidthUsec();
    //double pulseWidthMeters = (pulseWidthUsec / 1.0e6) * Radx::LIGHT_SPEED * 0.5;
    //_ddRadar.pulse_width = pulseWidthMeters;
    //double pulseWidthUsec = ray.getPulseWidthUsec();
    //double pulseWidthMeters = (pulseWidthUsec / 1.0e6) * Radx::LIGHT_SPEED * 0.5;
    _ddRadar.pulse_width = cal.getPulseWidthUsec(); // pulseWidthMeters;
  }

  try {
    Radx::PrimaryAxis_t axis_t = _writeVol->getPrimaryAxis();
    DoradeData::primary_axis_t doradeAxis = DoradeData::convertToDoradeType(axis_t);
    _ddRadar.extension_num = DoradeData::primaryAxisToInt(doradeAxis);
  } catch (const char*  ex) {
    // report warning
    cerr << "WARNING - DoradeRadxFile::_writeRadar()" << endl;
    cerr << ex << endl;
    cerr << "setting extension_num to zero indicating primary axis not set" << endl;
    _ddRadar.extension_num = 0;
  }
  _ddRadar.aperture_size = _writeVol->getLidarApertureDiamCm();
  _ddRadar.field_of_view = _writeVol->getLidarFieldOfViewMrad();
  _ddRadar.aperture_eff = _writeVol->getLidarApertureEfficiency();

  if (_ddLidar.lidar_const > 0) {
    _readVol->setLidarConstant(_ddLidar.lidar_const);
  }
  if (_ddLidar.pulse_energy > 0) {
    _readVol->setLidarPulseEnergyJ(_ddLidar.pulse_energy);
  }
  if (_ddLidar.peak_power > 0) {
    _readVol->setLidarPeakPowerW(_ddLidar.peak_power);
  }
  if (_ddLidar.aperture_size > 0) {
    _readVol->setLidarApertureDiamCm(_ddLidar.aperture_size);
  }
  if (_ddLidar.aperture_eff > 0) {
    _readVol->setLidarApertureEfficiency(_ddLidar.aperture_eff);
  }
  if (_ddLidar.field_of_view > 0) {
    _readVol->setLidarFieldOfViewMrad(_ddLidar.field_of_view);
  }
  if (_ddLidar.beam_divergence > 0) {
    _readVol->setLidarBeamDivergenceMrad(_ddLidar.beam_divergence);
  }

  // byte swap as needed

  DoradeData::radar_t copy = _ddRadar;
  if (!_writeNativeByteOrder) {
    DoradeData::swap(copy);
  }

  // write

  if (fwrite(&copy, sizeof(copy), 1, _file) != 1) {
    _addErrStr("ERROR - DoradeRadxFile::_writeRadar()");
    _addErrStr("  Cannot write radar block");
    _addErrStr("  file path: ", _pathInUse);
    _addErrStr(strerror(errno));
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////
// Write lidar info block
// Returns 0 on success, -1 on failure

int DoradeRadxFile::_writeLidar()
  
{

  if (_writeVol->getInstrumentType() != Radx::INSTRUMENT_TYPE_LIDAR) {
    // method not applicable to lidars
    return 0;
  }

  // fill
  
  DoradeData::init(_ddLidar);

  strncpy(_ddLidar.lidar_name, _writeVol->getInstrumentName().c_str(), 8);
  _ddLidar.lidar_const = _writeVol->getLidarConstant();
  _ddLidar.pulse_energy = _writeVol->getLidarPulseEnergyJ();
  _ddLidar.peak_power = _writeVol->getLidarPeakPowerW();
  _ddLidar.aperture_size = _writeVol->getLidarApertureDiamCm();
  _ddLidar.field_of_view = _writeVol->getLidarFieldOfViewMrad();
  _ddLidar.aperture_eff = _writeVol->getLidarApertureEfficiency();
  _ddLidar.beam_divergence = _writeVol->getLidarBeamDivergenceMrad();

  switch (_writeVol->getPlatformType()) {
    case Radx::PLATFORM_TYPE_FIXED:
    case Radx::PLATFORM_TYPE_VEHICLE:
      _ddLidar.lidar_type = DoradeData::LIDAR_GROUND;
      break;
    case Radx::PLATFORM_TYPE_SHIP:
      _ddLidar.lidar_type = DoradeData::LIDAR_SHIP;
      break;
    case Radx::PLATFORM_TYPE_AIRCRAFT_FORE:
      _ddLidar.lidar_type = DoradeData::LIDAR_AIR_FORE;
      break;
    case Radx::PLATFORM_TYPE_AIRCRAFT_AFT:
      _ddLidar.lidar_type = DoradeData::LIDAR_AIR_AFT;
      break;
    case Radx::PLATFORM_TYPE_AIRCRAFT_TAIL:
      _ddLidar.lidar_type = DoradeData::LIDAR_AIR_TAIL;
      break;
    case Radx::PLATFORM_TYPE_AIRCRAFT_BELLY:
    case Radx::PLATFORM_TYPE_AIRCRAFT_ROOF:
      _ddLidar.lidar_type = DoradeData::LIDAR_AIR_LF;
      break;
    case Radx::PLATFORM_TYPE_AIRCRAFT_NOSE:
      _ddLidar.lidar_type = DoradeData::LIDAR_AIR_FIXED;
      break;
    case Radx::PLATFORM_TYPE_SATELLITE_ORBIT:
    case Radx::PLATFORM_TYPE_SATELLITE_GEOSTAT:
      _ddLidar.lidar_type = DoradeData::LIDAR_SATELLITE;
      break;
    default:
      _ddLidar.lidar_type = DoradeData::LIDAR_GROUND;
  }
  
  _ddLidar.scan_mode = DoradeData::SCAN_MODE_SUR;
  if (_writeVol->getSweeps().size() > 0) {
    const RadxSweep &sweep = *_writeVol->getSweeps()[0];
    switch (sweep.getSweepMode()) {
      case Radx::SWEEP_MODE_SECTOR:
        _ddLidar.scan_mode = DoradeData::SCAN_MODE_PPI;
        break;
      case Radx::SWEEP_MODE_COPLANE:
        _ddLidar.scan_mode = DoradeData::SCAN_MODE_COP;
        break;
      case Radx::SWEEP_MODE_RHI:
        _ddLidar.scan_mode = DoradeData::SCAN_MODE_RHI;
        break;
      case Radx::SWEEP_MODE_VERTICAL_POINTING:
        _ddLidar.scan_mode = DoradeData::SCAN_MODE_VER;
        break;
      case Radx::SWEEP_MODE_IDLE:
        _ddLidar.scan_mode = DoradeData::SCAN_MODE_IDL;
        break;
      case Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE:
        _ddLidar.scan_mode = DoradeData::SCAN_MODE_SUR;
        break;
      case Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE:
        _ddLidar.scan_mode = DoradeData::SCAN_MODE_AIR;
        break;
      case Radx::SWEEP_MODE_SUNSCAN:
        _ddLidar.scan_mode = DoradeData::SCAN_MODE_CAL;
        break;
      case Radx::SWEEP_MODE_POINTING:
        _ddLidar.scan_mode = DoradeData::SCAN_MODE_MAN;
        break;
      case Radx::SWEEP_MODE_CALIBRATION:
        _ddLidar.scan_mode = DoradeData::SCAN_MODE_CAL;
        break;
      case Radx::SWEEP_MODE_MANUAL_PPI:
      case Radx::SWEEP_MODE_MANUAL_RHI:
        _ddLidar.scan_mode = DoradeData::SCAN_MODE_MAN;
        break;
      default:
        _ddLidar.scan_mode = DoradeData::SCAN_MODE_SUR;
    }
  }

  _ddLidar.num_parameter_des = _writeVol->getFields().size();
  _ddLidar.total_num_des = _writeVol->getFields().size();

  if (_writeCompressed) {
    _ddLidar.data_compress = DoradeData::COMPRESSION_HRD;
  } else {
    _ddLidar.data_compress = DoradeData::COMPRESSION_NONE;
  }
  
  _ddLidar.lidar_longitude = _writeVol->getLongitudeDeg();
  _ddLidar.lidar_latitude = _writeVol->getLatitudeDeg();
  _ddLidar.lidar_altitude = _writeVol->getAltitudeKm();

  if (_writeVol->getRays().size() > 0) {
    const RadxRay &ray = *_writeVol->getRays()[0];
    _ddLidar.eff_unamb_vel = ray.getNyquistMps();
    _ddLidar.eff_unamb_range = ray.getUnambigRangeKm();
    _ddLidar.prf = 1.0 / ray.getPrtSec();
    double pulseWidthUsec = ray.getPulseWidthUsec();
    _ddLidar.pulse_width = pulseWidthUsec;
  }
  
  
  int nFreq = _writeVol->getFrequencyHz().size();
  _ddLidar.num_wvlen_trans = nFreq;
  for (int ii = 0; ii < nFreq; ii++) {
    double freq = _writeVol->getFrequencyHz()[ii];
    double wavelengthM = Radx::LIGHT_SPEED / freq;
    _ddLidar.wavelength[ii] = wavelengthM;
  }
    
  // byte swap as needed

  DoradeData::lidar_t copy = _ddLidar;
  if (!_writeNativeByteOrder) {
    DoradeData::swap(copy);
  }

  // write

  if (fwrite(&copy, sizeof(copy), 1, _file) != 1) {
    _addErrStr("ERROR - DoradeRadxFile::_writeLidar()");
    _addErrStr("  Cannot write lidar block");
    _addErrStr("  file path: ", _pathInUse);
    _addErrStr(strerror(errno));
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////
// Write parameter block
// Returns 0 on success, -1 on failure

int DoradeRadxFile::_writeParameter(int fieldNum)
  
{

  // check indices

  if (fieldNum >= (int) _writeVol->getFields().size()) {
    _addErrStr("ERROR - DoradeRadxFile::_writeParameter()");
    _addErrStr("  Cannot write field parameter block");
    _addErrStr("  field number too high");
    _addErrInt("  this field number: ", fieldNum);
    _addErrInt("  max field number: ", (int) _writeVol->getFields().size() - 1);
    return -1;
  }
  const RadxField &field = *_writeVol->getFields()[fieldNum];

  // fill

  DoradeData::parameter_t parm;
  DoradeData::init(parm);
  
  strncpy(parm.parameter_name, field.getName().c_str(), 8);
  strncpy(parm.param_description, field.getLongName().c_str(), 40);
  strncpy(parm.param_units, field.getUnits().c_str(), 8);
  strncpy(parm.config_name, field.getStandardName().c_str(), 8);
  strncpy(parm.threshold_field, field.getThresholdFieldName().c_str(), 8);

  parm.interpulse_time = 1;
  parm.xmitted_freq = 1;

  parm.polarization = DoradeData::POLARIZATION_HORIZONTAL;
  parm.num_samples = 1;
  parm.threshold_value = field.getThresholdValue();

  if (_writeVol->getRays().size() > 0) {

    const RadxRay &ray = *_writeVol->getRays()[0];
    double pulseWidthUsec = ray.getPulseWidthUsec();
    if (pulseWidthUsec != Radx::missingMetaDouble) {
      double pulseWidthMeters = (pulseWidthUsec / 1.0e6) * Radx::LIGHT_SPEED * 0.5;
      parm.pulse_width = (short) (pulseWidthMeters + 0.5);
      parm.recvr_bandwidth = 1.0 / pulseWidthUsec;
    } else {
      parm.pulse_width = Radx::missingSi16; 
      parm.recvr_bandwidth = Radx::missingMetaDouble;
    }
    double rxBandwidthMhz = _writeVol->getRadarReceiverBandwidthMhz();
    if (rxBandwidthMhz > 0) {
      parm.recvr_bandwidth = rxBandwidthMhz;
    }

    parm.num_samples = (int) (ray.getNSamples() * field.getSamplingRatio() + 0.5);

    switch (ray.getPolarizationMode()) {
      case Radx::POL_MODE_HORIZONTAL:
        parm.polarization = DoradeData::POLARIZATION_HORIZONTAL;
        break;
      case Radx::POL_MODE_VERTICAL:
        parm.polarization = DoradeData::POLARIZATION_VERTICAL;
        break;
      case Radx::POL_MODE_HV_ALT:
        parm.polarization = DoradeData::POLARIZATION_HV_ALT;
        break;
      case Radx::POL_MODE_HV_SIM:
        parm.polarization = DoradeData::POLARIZATION_HV_SIM;
        break;
      case Radx::POL_MODE_CIRCULAR:
        parm.polarization = DoradeData::POLARIZATION_CIRCULAR_RIGHT;
        break;
      default:
        parm.polarization = DoradeData::POLARIZATION_HORIZONTAL;
    }

  }

  // force output type to si16, with bias 0 and suitable scale
  
  double scale = 1.0 / _getScale(field.getName());
  parm.binary_format = DoradeData::BINARY_FORMAT_INT16;
  parm.bad_data = field.getMissingSi16();
  parm.parameter_scale = scale; // Radx scale 0.01
  parm.parameter_bias = 0.0; // Radx offset 0.0

  parm.offset_to_data = sizeof(DoradeData::paramdata_t);
  parm.number_cells = field.getMaxNGates();
  parm.meters_to_first_cell = field.getStartRangeKm() * 1000.0;
  parm.meters_between_cells = field.getGateSpacingKm() * 1000.0;

  // byte swap as needed

  if (!_writeNativeByteOrder) {
    DoradeData::swap(parm);
  }

  // write

  if (fwrite(&parm, sizeof(parm), 1, _file) != 1) {
    _addErrStr("ERROR - DoradeRadxFile::_writeParameter()");
    _addErrStr("  Cannot write field parameter block");
    _addErrStr("  file path: ", _pathInUse);
    _addErrInt("  this field number: ", fieldNum);
    _addErrStr(strerror(errno));
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////
// Write cell vector block
// Returns 0 on success, -1 on failure

int DoradeRadxFile::_writeCellVector()
  
{

  // fill
  
  DoradeData::init(_ddCellv);

  _ddCellv.number_cells = _writeVol->getMaxNGates();
  if (_ddCellv.number_cells > DoradeData::MAXCVGATES) {
    _ddCellv.number_cells = DoradeData::MAXCVGATES;
  }

  double startRange = _writeVol->getStartRangeKm() * 1000.0;
  double gateSpacing = _writeVol->getGateSpacingKm() * 1000.0;

  double range = startRange;
  for (int ii = 0; ii < _ddCellv.number_cells; ii++, range += gateSpacing) {
    _ddCellv.dist_cells[ii] = range;
  }

  // byte swap as needed

  DoradeData::cell_vector_t copy = _ddCellv;
  if (!_writeNativeByteOrder) {
    DoradeData::swap(copy);
  }

  // write
  
  if (fwrite(&copy, sizeof(copy), 1, _file) != 1) {
    _addErrStr("ERROR - DoradeRadxFile::_writeCellVector()");
    _addErrStr("  Cannot write cell_vector block");
    _addErrStr("  file path: ", _pathInUse);
    _addErrStr(strerror(errno));
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////
// Write cell spacing block
// Returns 0 on success, -1 on failure

int DoradeRadxFile::_writeCellSpacingFp()
  
{

  // fill
  
  DoradeData::init(_ddCsfp);

  _ddCsfp.num_segments = 1;
  _ddCsfp.dist_to_first = _writeVol->getStartRangeKm() * 1000.0;
  _ddCsfp.spacing[0] = _writeVol->getGateSpacingKm() * 1000.0;
  _ddCsfp.num_cells[0] = _writeVol->getMaxNGates();

  // byte swap as needed

  DoradeData::cell_spacing_fp_t copy = _ddCsfp;
  if (!_writeNativeByteOrder) {
    DoradeData::swap(copy);
  }

  // write

  if (fwrite(&copy, sizeof(copy), 1, _file) != 1) {
    _addErrStr("ERROR - DoradeRadxFile::_writeCellSpacingFp()");
    _addErrStr("  Cannot write cell_spacing_fp block");
    _addErrStr("  file path: ", _pathInUse);
    _addErrStr(strerror(errno));
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////
// Write correction factors block
// Returns 0 on success, -1 on failure

int DoradeRadxFile::_writeCorrectionFactors()
  
{

  const RadxCfactors *cfactors = _writeVol->getCfactors();
  if (cfactors == NULL) {
    // no factors available
    return 0;
  }

  // fill
  
  DoradeData::init(_ddCfac);
  
  _ddCfac.azimuth_corr = cfactors->getAzimuthCorr();
  _ddCfac.elevation_corr = cfactors->getElevationCorr();
  _ddCfac.range_delay_corr = cfactors->getRangeCorr();
  _ddCfac.longitude_corr = cfactors->getLongitudeCorr();
  _ddCfac.latitude_corr = cfactors->getLatitudeCorr();
  _ddCfac.pressure_alt_corr = cfactors->getPressureAltCorr();
  _ddCfac.radar_alt_corr = cfactors->getAltitudeCorr();
  _ddCfac.ew_gndspd_corr = cfactors->getEwVelCorr();
  _ddCfac.ns_gndspd_corr = cfactors->getNsVelCorr();
  _ddCfac.vert_vel_corr = cfactors->getVertVelCorr();
  _ddCfac.heading_corr = cfactors->getHeadingCorr();
  _ddCfac.roll_corr = cfactors->getRollCorr();
  _ddCfac.pitch_corr = cfactors->getPitchCorr();
  _ddCfac.drift_corr = cfactors->getDriftCorr();
  _ddCfac.rot_angle_corr = cfactors->getRotationCorr();
  _ddCfac.tilt_corr = cfactors->getTiltCorr();

  // byte swap as needed
  
  DoradeData::correction_t copy = _ddCfac;
  if (!_writeNativeByteOrder) {
    DoradeData::swap(copy);
  }

  // write
  
  if (fwrite(&copy, sizeof(copy), 1, _file) != 1) {
    _addErrStr("ERROR - DoradeRadxFile::_writeCorrectionFactors()");
    _addErrStr("  Cannot write correction factor block");
    _addErrStr("  file path: ", _pathInUse);
    _addErrStr(strerror(errno));
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////
// Write sweep info block
// Returns 0 on success, -1 on failure

int DoradeRadxFile::_writeSweepInfo(int sweepNum)
  
{

  // check indices

  if (sweepNum >= (int) _writeVol->getSweeps().size()) {
    _addErrStr("ERROR - DoradeRadxFile::_writeSweepInfo()");
    _addErrStr("  Cannot write sweep info block");
    _addErrStr("  sweep number too high");
    _addErrInt("  this sweep number: ", sweepNum);
    _addErrInt("  max sweep number: ", (int) _writeVol->getSweeps().size() - 1);
    return -1;
  }
  const RadxSweep &sweep = *_writeVol->getSweeps()[sweepNum];

  if (sweep.getStartRayIndex() >= _writeVol->getRays().size()) {
    _addErrStr("ERROR - DoradeRadxFile::_writeSweepInfo()");
    _addErrStr("  Cannot write sweep info block");
    _addErrStr("  start ray number too high");
    _addErrInt("  sweep start ray number: ", sweep.getStartRayIndex());
    _addErrInt("  max ray number: ", (int) _writeVol->getRays().size() - 1);
    return -1;
  }
  if (sweep.getEndRayIndex() >= _writeVol->getRays().size()) {
    _addErrStr("ERROR - DoradeRadxFile::_writeSweepInfo()");
    _addErrStr("  Cannot write sweep info block");
    _addErrStr("  end ray number too high");
    _addErrInt("  sweep end ray number: ", sweep.getEndRayIndex());
    _addErrInt("  max ray number: ", (int) _writeVol->getRays().size() - 1);
    return -1;
  }

  const RadxRay &startRay = *_writeVol->getRays()[sweep.getStartRayIndex()];
  const RadxRay &endRay = *_writeVol->getRays()[sweep.getEndRayIndex()];

  if (_debug) {
    cerr << "  nRays in sweep to write: " << sweep.getNRays() << endl;
  }

  // fill
  
  DoradeData::init(_ddSweep);

  strncpy(_ddSweep.radar_name, _writeVol->getInstrumentName().c_str(), 8);

  _ddSweep.sweep_num = sweep.getSweepNumber();
  _ddSweep.num_rays = sweep.getEndRayIndex() - sweep.getStartRayIndex() + 1;

  if (sweep.getSweepMode() == Radx::SWEEP_MODE_RHI ||
      sweep.getSweepMode() == Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE ||
      sweep.getSweepMode() == Radx::SWEEP_MODE_MANUAL_RHI) {
    _ddSweep.start_angle = startRay.getElevationDeg();
    _ddSweep.stop_angle = endRay.getElevationDeg();
  } else {
    _ddSweep.start_angle = startRay.getAzimuthDeg();
    _ddSweep.stop_angle = endRay.getAzimuthDeg();
  }

  _ddSweep.fixed_angle = sweep.getFixedAngleDeg();

  // byte swap as needed

  DoradeData::sweepinfo_t copy = _ddSweep;
  if (!_writeNativeByteOrder) {
    DoradeData::swap(copy);
  }

  // write

  if (fwrite(&copy, sizeof(copy), 1, _file) != 1) {
    _addErrStr("ERROR - DoradeRadxFile::_writeSweepInfo()");
    _addErrStr("  Cannot write sweep info block");
    _addErrStr("  file path: ", _pathInUse);
    _addErrStr(strerror(errno));
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////
// Write ray info block
// Returns 0 on success, -1 on failure

int DoradeRadxFile::_writeRayInfo(int rayNum,
                                  DoradeData::ray_t &ddRay)
  
{

  // check indices

  if (rayNum >= (int) _writeVol->getRays().size()) {
    _addErrStr("ERROR - DoradeRadxFile::_writeRayInfo()");
    _addErrStr("  Cannot write ray info block");
    _addErrStr("  ray number too high");
    _addErrInt("  ray number: ", rayNum);
    _addErrInt("  max ray number: ", (int) _writeVol->getRays().size() - 1);
    return -1;
  }
  const RadxRay &xray = *_writeVol->getRays()[rayNum];
  
  // fill

  DoradeData::init(ddRay);

  ddRay.sweep_num = xray.getSweepNumber();

  time_t rayTimeSecs = xray.getTimeSecs();
  RadxTime rayTime(rayTimeSecs);
  RadxTime yearStart(rayTime.getYear(), 1, 1, 0, 0, 0);
  time_t yearStartSecs = yearStart.utime();
  int nsecsThisYear = rayTimeSecs - yearStartSecs;
  int julianDay = nsecsThisYear / 86400 + 1;
  ddRay.julian_day = julianDay;

  ddRay.hour = rayTime.getHour();
  ddRay.minute = rayTime.getMin();
  ddRay.second = rayTime.getSec();
  ddRay.millisecond = (int) (xray.getNanoSecs() * 1.0e-6 + 0.5);

  ddRay.azimuth = xray.getAzimuthDeg();
  ddRay.elevation = xray.getElevationDeg();
  if (xray.getMeasXmitPowerDbmH() < -999) {
    ddRay.peak_power = -999;
  } else {
    ddRay.peak_power = pow(10.0, xray.getMeasXmitPowerDbmH() / 10.0) * 1.0e-6;
  }
  ddRay.true_scan_rate = xray.getTrueScanRateDegPerSec();

  if (xray.getAntennaTransition()) {
    ddRay.ray_status = 1;
  }

  // byte swap as needed

  DoradeData::ray_t copy = ddRay;
  if (!_writeNativeByteOrder) {
    DoradeData::swap(copy);
  }

  // write
  
  if (fwrite(&copy, sizeof(copy), 1, _file) != 1) {
    _addErrStr("ERROR - DoradeRadxFile::_writeRayInfo()");
    _addErrStr("  Cannot write ray info block");
    _addErrStr("  file path: ", _pathInUse);
    _addErrInt("  ray number: ", rayNum);
    _addErrStr(strerror(errno));
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////
// Write platform georeference block
// Returns 0 on success, -1 on failure

int DoradeRadxFile::_writeRayGeoref(int rayNum,
                                    const DoradeData::ray_t &ddRay,
                                    DoradeData::rot_table_entry_t &rotEntry)
  
{
  
  // check indices
  
  if (rayNum >= (int) _writeVol->getRays().size()) {
    _addErrStr("ERROR - DoradeRadxFile::_writeRayGeoref()");
    _addErrStr("  Cannot write platform georef block");
    _addErrStr("  ray number too high");
    _addErrInt("  ray number: ", rayNum);
    _addErrInt("  max ray number: ", (int) _writeVol->getRays().size() - 1);
    return -1;
  }
  const RadxRay &xray = *_writeVol->getRays()[rayNum];
  const RadxGeoref *georef = xray.getGeoreference();

  if (georef == NULL) {
    // no platform georeference data available 
    // set table entry rotation from azimuth or elevation as appropriate
    if (_ddRadar.scan_mode == DoradeData::SCAN_MODE_RHI) {
      rotEntry.rotation_angle =
        FMOD360(450.0 - (ddRay.elevation + _ddCfac.elevation_corr));
    } else {
      rotEntry.rotation_angle = ddRay.azimuth + _ddCfac.azimuth_corr;
    }
    return 0;
  }
  
  // fill platform info

  DoradeData::platform_t ddPlat;
  DoradeData::init(ddPlat);

  ddPlat.longitude = georef->getLongitude();
  ddPlat.latitude = georef->getLatitude();
  ddPlat.altitude_msl = georef->getAltitudeKmMsl();
  ddPlat.altitude_agl = georef->getAltitudeKmAgl();
  ddPlat.ew_velocity = georef->getEwVelocity();
  ddPlat.ns_velocity = georef->getNsVelocity();
  ddPlat.vert_velocity = georef->getVertVelocity();
  ddPlat.heading = georef->getHeading();
  ddPlat.roll = georef->getRoll();
  ddPlat.pitch = georef->getPitch();
  ddPlat.drift_angle = georef->getDrift();
  double rotationAngle = georef->getRotation();
  //   if (_radarName.find("ELDR") != string::npos) {
  //     rotationAngle = fmod(450.0 - rotationAngle, 360.0);
  //   }
  ddPlat.rotation_angle = rotationAngle;
  ddPlat.tilt = georef->getTilt();
  ddPlat.ew_horiz_wind = georef->getEwWind();
  ddPlat.ns_horiz_wind = georef->getNsWind();
  ddPlat.vert_wind = georef->getVertWind();
  ddPlat.heading_change = georef->getHeadingRate();
  ddPlat.pitch_change = georef->getPitchRate();

  // set entry rotation
  
  rotEntry.rotation_angle = _ddRotation(ddRay, ddPlat);

  // byte swap as needed
  
  DoradeData::platform_t copy = ddPlat;
  if (!_writeNativeByteOrder) {
    DoradeData::swap(copy);
  }

  // write

  if (fwrite(&copy, sizeof(copy), 1, _file) != 1) {
    _addErrStr("ERROR - DoradeRadxFile::_writeRayGeoref()");
    _addErrStr("  Cannot write ray georeference block");
    _addErrStr("  file path: ", _pathInUse);
    _addErrInt("  ray number: ", rayNum);
    _addErrStr(strerror(errno));
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////
// Write ray data
// Returns 0 on success, -1 on failure

int DoradeRadxFile::_writeRayData(int rayNum, int fieldNum)
  
{

  // check indices

  if (rayNum >= (int) _writeVol->getRays().size()) {
    _addErrStr("ERROR - DoradeRadxFile::_writeRayData()");
    _addErrStr("  Cannot write ray data block");
    _addErrStr("  ray number too high");
    _addErrInt("  ray number: ", rayNum);
    _addErrInt("  max ray number: ", (int) _writeVol->getRays().size() - 1);
    return -1;
  }
  const RadxRay &xray = *_writeVol->getRays()[rayNum];

  if (fieldNum >= (int) xray.getFields().size()) {
    _addErrStr("ERROR - DoradeRadxFile::_writeRayData()");
    _addErrStr("  Cannot write ray data block");
    _addErrStr("  field number too high");
    _addErrInt("  this field number: ", fieldNum);
    _addErrInt("  max field number: ", (int) xray.getFields().size() - 1);
    return -1;
  }
  
  // make a copy of the field
  
  RadxField field(*xray.getFields()[fieldNum]);
  
  // convert to int16,

  double scale = _getScale(field.getName());
  field.convertToSi16(scale, 0.0);

  // pad to output number of gates

  size_t nGatesOut = _writeVol->getMaxNGates();
  Radx::si16 *fieldData = new Radx::si16[nGatesOut];
  memcpy(fieldData, field.getData(), field.getNPoints() * sizeof(Radx::si16));
  for (size_t ii = field.getNPoints(); ii < nGatesOut; ii++) {
    fieldData[ii] = field.getMissingSi16();
  }
  
  // compress the output data as appropriate
  
  int maxOut = nGatesOut + 16;
  unsigned short *dataOut = new unsigned short[maxOut];
  int nWordsOut = nGatesOut;
  
  if (_writeCompressed) {
    
    // bad val is 32768 - since this is in unsigned space
    nWordsOut = DoradeData::compressHrd16((unsigned short *) fieldData,
                                          nGatesOut,
                                          dataOut, maxOut, 
                                          32768);

  } else {

    memcpy(dataOut, fieldData, nWordsOut * sizeof(Radx::si16));
    
  }

  // free up tmp array

  delete[] fieldData;
  
  // compute number of data bytes to write
  
  int nBytesOut = nWordsOut * sizeof(Radx::si16);

  // write paramdata header
  
  DoradeData::paramdata_t pdat;
  DoradeData::init(pdat);
  strncpy(pdat.pdata_name, field.getName().c_str(), 8);
  pdat.nbytes = sizeof(pdat) + nBytesOut;

  // byte swap as needed
  
  if (!_writeNativeByteOrder) {
    DoradeData::swap(pdat);
  }

  // write header

  if (fwrite(&pdat, sizeof(pdat), 1, _file) != 1) {
    _addErrStr("ERROR - DoradeRadxFile::_writeRayData()");
    _addErrStr("  Cannot write paramdata block");
    _addErrStr("  file path: ", _pathInUse);
    _addErrInt("  field number: ", fieldNum);
    _addErrInt("  ray number: ", rayNum);
    _addErrStr(strerror(errno));
    return -1;
  }

  // byte swap as needed

  if (!_writeNativeByteOrder) {
    ByteOrder::swap16(dataOut, nBytesOut);
  }

  // write data

  if ((int) fwrite(dataOut,  nBytesOut, 1, _file) != 1) {
    _addErrStr("ERROR - DoradeRadxFile::_writeRayData()");
    _addErrInt("  Cannot write data, nBytes: ", nBytesOut);
    _addErrStr("  file path: ", _pathInUse);
    _addErrInt("  field number: ", fieldNum);
    _addErrInt("  ray number: ", rayNum);
    _addErrStr(strerror(errno));
    delete[] dataOut;
    return -1;
  }
  
  // free up

  delete[] dataOut;

  return 0;

}

////////////////////////////////////////////////////////////
// Write null block
// Returns 0 on success, -1 on failure

int DoradeRadxFile::_writeNullBlock()
  
{

  DoradeData::null_block_t nullBlock;
  DoradeData::init(nullBlock);

  // byte swap as needed

  if (!_writeNativeByteOrder) {
    DoradeData::swap(nullBlock);
  }

  // write

  if (fwrite(&nullBlock, sizeof(nullBlock), 1, _file) != 1) {
    _addErrStr("ERROR - DoradeRadxFile::_writeNullBlock()");
    _addErrStr("  Cannot write null block");
    _addErrStr("  file path: ", _pathInUse);
    _addErrStr(strerror(errno));
    return -1;
  }
  return 0;

}

////////////////////////////////////////////////////////////
// Write rotation angle table
// Returns 0 on success, -1 on failure

int DoradeRadxFile::_writeRotAngTable()
  
{

  int nIndex = 480;
  int nEntries = _rotEntries.size();

  // align index on 8 byte boundary

  int indexOffset = sizeof(DoradeData::rot_angle_table_t);
  indexOffset = ((indexOffset - 1) / 8 + 1) * 8;
  int keyOffset = indexOffset + nIndex * sizeof(Radx::si32);

  // compute size of block

  int blockSize = indexOffset + nIndex * sizeof(Radx::si32) +
    nEntries * sizeof(DoradeData::rot_angle_table_t);

  // allocate block

  char *block = new char[blockSize];
  memset(block, 0, blockSize);

  // load up table struct
  
  DoradeData::rot_ang_table table;
  DoradeData::init(table);
  table.nbytes = blockSize;
  table.angle2ndx = (double) nIndex / 360.0;
  table.ndx_que_size = nIndex;
  table.first_key_offset = keyOffset;
  table.angle_table_offset = indexOffset;
  table.num_rays = nEntries;

  // copy into block, swapping as needed
  
  DoradeData::rot_ang_table copy = table;
  if (!_writeNativeByteOrder) {
    DoradeData::swap(copy);
  }
  memcpy(block, &copy, sizeof(copy));

  // load up angle indices
  
  Radx::si32 *index = (Radx::si32 *) (block + indexOffset);
  for (int ii = 0; ii < nIndex; ii++) {
    index[ii] = -1;
  }
  for (int jj = 0; jj < nEntries; jj++) {
    double rotAngle = _rotEntries[jj].rotation_angle;
    int ii = (int) (rotAngle * table.angle2ndx);
    if (ii < 0 || ii > nIndex - 1) {
      ii = 0;
    }
    index[ii] = jj;
  }
  if (!_writeNativeByteOrder) {
    ByteOrder::swap32(index, nIndex * sizeof(Radx::si32));
  }
  
  // load up angle table entries
  
  for (int ii = 0; ii < nEntries; ii++) {
    DoradeData::rot_table_entry_t entry = _rotEntries[ii];
    if (!_writeNativeByteOrder) {
      DoradeData::swap(entry);
    }
    int offset = keyOffset + ii * sizeof(DoradeData::rot_table_entry_t);
    memcpy(block + offset, &entry, sizeof(entry));
  }

  // write

  if (fwrite(block, blockSize, 1, _file) != 1) {
    _addErrStr("ERROR - DoradeRadxFile::_writeRotAngleTable()");
    _addErrStr("  Cannot write table block");
    _addErrStr("  file path: ", _pathInUse);
    _addErrStr(strerror(errno));
    delete[] block;
    return -1;
  }

  delete[] block;
  return 0;

}

////////////////////////////////////////////////////////////
// Write SEDS block
// Returns 0 on success, -1 on failure

int DoradeRadxFile::_writeSedsBlock()
  
{
  
  if (_writeVol->getHistory().size() == 0) {
    // no history info
    return 0;
  }
  
  // compute size to be even 4 bytes
  
  int sedsSize = (((_writeVol->getHistory().size() + 12) / 4) + 1) * 4;
  char *sedsBlock = new char[sedsSize];
  memset(sedsBlock, 0, sedsSize);

  // load up block

  memcpy(sedsBlock, "SEDS", 4);
  Radx::si32 len = sedsSize;
  if (!_writeNativeByteOrder) {
    ByteOrder::swap32(&len, sizeof(len));
  }
  memcpy(sedsBlock + 4, &len, 4);
  memcpy(sedsBlock + 8, _writeVol->getHistory().c_str(),
         _writeVol->getHistory().size() + 1);
  
  // write

  if (fwrite(sedsBlock, sedsSize, 1, _file) != 1) {
    _addErrStr("ERROR - DoradeRadxFile::_writeSedsBlock()");
    _addErrStr("  Cannot write seds block");
    _addErrStr("  file path: ", _pathInUse);
    _addErrStr(strerror(errno));
    delete[] sedsBlock;
    return -1;
  }

  delete[] sedsBlock;
  return 0;

}

/////////////////////////////////////////////////////////////////
// angle computation routines
// apply corrections

double DoradeRadxFile::_ddLatitude(const DoradeData::platform_t &plat)
{
  return plat.latitude + _ddCfac.latitude_corr;
}

double DoradeRadxFile::_ddLongitude(const DoradeData::platform_t &plat)
{
  return plat.longitude + _ddCfac.longitude_corr;
}

double DoradeRadxFile::_ddAltitude(const DoradeData::platform_t &plat)
{
  return plat.altitude_msl + _ddCfac.pressure_alt_corr;
}

double DoradeRadxFile::_ddAltitudeAgl(const DoradeData::platform_t &plat)
{
  return plat.altitude_agl + _ddCfac.pressure_alt_corr;
}

double DoradeRadxFile::_ddRoll(const DoradeData::platform_t &plat)
{
  return plat.roll + _ddCfac.roll_corr;
}

double DoradeRadxFile::_ddPitch(const DoradeData::platform_t &plat)
{
  return plat.pitch + _ddCfac.pitch_corr;
}

double DoradeRadxFile::_ddHeading(const DoradeData::platform_t &plat)
{
  return plat.heading + _ddCfac.heading_corr;
}

double DoradeRadxFile::_ddDrift(const DoradeData::platform_t &plat)
{
  return plat.drift_angle + _ddCfac.drift_corr;
}

double DoradeRadxFile::_ddAzimuth(const DoradeData::ray_t &ray,
                                  const DoradeData::platform_t &plat)
{

  double az;

  if (_ddRadar.scan_mode == DoradeData::SCAN_MODE_AIR) {

    DoradeData::radar_angles_t rAngles;
    _ddRadarAngles(ray, plat, rAngles);
    az = FMOD360(DEGREES(rAngles.azimuth) + 360.0);
    
  } else if (_ddRadar.radar_type == DoradeData::RADAR_AIR_LF ||
             _ddRadar.radar_type == DoradeData::RADAR_AIR_NOSE) {

    // this is meant to apply only to the P3 lower fuselage data

    az = FMOD360(ray.azimuth + _ddCfac.azimuth_corr + _ddHeading(plat));

  } else {

    az = FMOD360(ray.azimuth + _ddCfac.azimuth_corr);
    
  }

  return az;

}

double DoradeRadxFile::_ddElevation(const DoradeData::ray_t &ray,
                                  const DoradeData::platform_t &plat)
{

  double el;

  if (_ddRadar.scan_mode == DoradeData::SCAN_MODE_AIR) {

    DoradeData::radar_angles_t rAngles;
    _ddRadarAngles(ray, plat, rAngles);
    el = FMOD360(DEGREES(rAngles.elevation) + 360.0);
    
  } else if (_ddRadar.radar_type == DoradeData::RADAR_AIR_LF ||
             _ddRadar.radar_type == DoradeData::RADAR_AIR_NOSE) {

    // this is meant to apply only to the P3 lower fuselage data

    double ElR = RADIANS(ray.elevation + _ddCfac.elevation_corr);
    double AzmR = RADIANS(ray.azimuth + _ddCfac.azimuth_corr);
    double PitchR = RADIANS(_ddPitch(plat));
    double RollR = RADIANS(_ddRoll(plat));

    double z = (cos(AzmR)*cos(ElR)*sin(PitchR)
                + sin(ElR)*cos(PitchR)*cos(RollR)
                - sin(AzmR)*cos(ElR)*cos(PitchR)*sin(RollR));
    
    if (z > 1.0) {
      z = 1.0;
    } else if (z < -1.0) {
      z = -1.0;
    }
    el = DEGREES(asin(z));
    
  } else {
    
    el = FMOD360(ray.elevation + _ddCfac.elevation_corr);

  }

  return el;

}

double DoradeRadxFile::_ddRotation(const DoradeData::ray_t &ray,
                                   const DoradeData::platform_t &plat)
{

  double rot;
  switch (_ddRadar.scan_mode) {

    case DoradeData::SCAN_MODE_AIR:
      rot = FMOD360(plat.rotation_angle + _ddCfac.rot_angle_corr + _ddRoll(plat));
      break;

    case DoradeData::SCAN_MODE_RHI:
      rot = FMOD360(450.0-(ray.elevation + _ddCfac.elevation_corr));
      break;

    case DoradeData::SCAN_MODE_TAR:
      if (_ddRadar.radar_type != DoradeData::RADAR_GROUND) {
        rot = FMOD360(plat.rotation_angle + _ddCfac.rot_angle_corr);
      } else {
        rot = _ddAzimuth(ray, plat);
      }
      break;

    default:
      rot = _ddAzimuth(ray, plat);

  }

  return rot;

}

double DoradeRadxFile::_ddNavRotation(const DoradeData::ray_t &ray,
                                      const DoradeData::platform_t &plat)
{

  double rot;
  switch (_ddRadar.scan_mode) {
    
    case DoradeData::SCAN_MODE_AIR:
      rot = FMOD360(plat.rotation_angle + _ddCfac.rot_angle_corr);
      break;

    case DoradeData::SCAN_MODE_RHI:
      rot = FMOD360(450.0-(ray.elevation + _ddCfac.elevation_corr));
      break;

    case DoradeData::SCAN_MODE_TAR:
      if (_ddRadar.radar_type != DoradeData::RADAR_GROUND) {
        rot = FMOD360(plat.rotation_angle + _ddCfac.rot_angle_corr);
      } else {
        rot = _ddAzimuth(ray, plat);
      }
      break;

    default:
      rot = _ddAzimuth(ray, plat);

  }

  return rot;

}

double DoradeRadxFile::_ddTilt(const DoradeData::ray_t &ray,
                               const DoradeData::platform_t &plat)
{
  
  double tilt;
  switch (_ddRadar.scan_mode) {
    
    case DoradeData::SCAN_MODE_AIR:
      DoradeData::radar_angles_t rAngles;
      _ddRadarAngles(ray, plat, rAngles);
      tilt = DEGREES(rAngles.tilt) + _ddCfac.tilt_corr;
      break;
      
    case DoradeData::SCAN_MODE_RHI:
      tilt = CART_ANGLE(ray.azimuth + _ddCfac.azimuth_corr);
      break;
      
    default:
      tilt = ray.elevation + _ddCfac.elevation_corr;

  }

  return tilt;

}

double DoradeRadxFile::_ddNavTilt(const DoradeData::ray_t &ray,
                                  const DoradeData::platform_t &plat)
{
  
  double tilt;
  switch (_ddRadar.scan_mode) {
    
    case DoradeData::SCAN_MODE_AIR:
      tilt = plat.tilt + _ddCfac.tilt_corr;
      break;
      
    case DoradeData::SCAN_MODE_RHI:
      tilt = CART_ANGLE(ray.azimuth + _ddCfac.azimuth_corr);
      break;
      
    default:
      tilt = _ddElevation(ray, plat);

  }

  return tilt;

}

///////////////////////////////////////////////////////////////////
// compute the true azimuth, elevation, etc. from platform
// parameters using Testud's equations with their different
// definitions of rotation angle, etc.
//
// see Wen-Chau Lee's paper
// "Mapping of the Airborne Doppler Radar Data"

void DoradeRadxFile::_ddRadarAngles(const DoradeData::ray_t &ray,
                                    const DoradeData::platform_t &plat,
                                    DoradeData::radar_angles_t &rAngles)
  
{

  double R = RADIANS(plat.roll + _ddCfac.roll_corr);
  double P = RADIANS(plat.pitch + _ddCfac.pitch_corr);
  double H = RADIANS(plat.heading + _ddCfac.heading_corr);
  double D = RADIANS(plat.drift_angle + _ddCfac.drift_corr);
  double T = H + D;
  
  double sinP = sin(P);
  double cosP = cos(P);
  double sinD = sin(D);
  double cosD = cos(D);
  
  if( _ddRadar.radar_type == DoradeData::RADAR_AIR_LF ||
      _ddRadar.radar_type == DoradeData::RADAR_AIR_NOSE ) {
    
    double lambda_a = RADIANS(CART_ANGLE(ray.azimuth + _ddCfac.azimuth_corr));
    double sin_lambda_a = sin(lambda_a);
    double cos_lambda_a = cos(lambda_a);
    
    double phi_a = RADIANS(ray.elevation + _ddCfac.elevation_corr);
    double sin_phi_a = sin(phi_a);
    double cos_phi_a = cos(phi_a);
    
    double sinR = sin(R);
    double cosR = cos(R);

    double xsubt = cos_lambda_a * cos_phi_a *
      (cosD * cosR - sinD * sinP * sinR) -
      sinD * cosP * sin_lambda_a * cos_phi_a +
      sin_phi_a * ( cosD * sinR + sinD * sinP * cosR );
    rAngles.x = xsubt;
    
    double ysubt = cos_lambda_a * cos_phi_a *
      ( sinD * cosR + cosD * sinP * sinR ) +
      cosD * cosP * sin_lambda_a * cos_phi_a +
      sin_phi_a * ( sinD * sinR - cosD * sinP * cosR );
    rAngles.y = ysubt;

    double zsubt= -cosP * sinR * cos_lambda_a * cos_phi_a +
      sinP * sin_lambda_a * cos_phi_a +
      cosP * cosR * sin_phi_a;
    rAngles.z = zsubt;
    
    rAngles.rotation_angle = atan2(xsubt, zsubt);
    rAngles.tilt = asin(ysubt);
    double lambda_t = atan2(xsubt, ysubt);
    rAngles.azimuth = fmod(lambda_t + T, M_PI * 2.0);
    rAngles.elevation = asin(zsubt);
    
  } else {
  
    double theta_a = RADIANS(plat.rotation_angle + _ddCfac.rot_angle_corr);
    double tau_a = RADIANS(plat.tilt + _ddCfac.tilt_corr);
    double sin_tau_a = sin(tau_a);
    double cos_tau_a = cos(tau_a);
    double sin_theta_rc = sin(theta_a + R); /* roll corrected rotation angle */
    double cos_theta_rc = cos(theta_a + R); /* roll corrected rotation angle */
    
    double xsubt = (cos_theta_rc * sinD * cos_tau_a * sinP
                    + cosD * sin_theta_rc * cos_tau_a
                    -sinD * cosP * sin_tau_a);
    rAngles.x = xsubt;
    
    double ysubt = (-cos_theta_rc * cosD * cos_tau_a * sinP
                    + sinD * sin_theta_rc * cos_tau_a
                    + cosP * cosD * sin_tau_a);
    rAngles.y = ysubt;
    
    double zsubt = (cosP * cos_tau_a * cos_theta_rc
                    + sinP * sin_tau_a);
    rAngles.z = zsubt;
    
    rAngles.rotation_angle = atan2(xsubt, zsubt);
    rAngles.tilt = asin(ysubt);
    double lambda_t = atan2(xsubt, ysubt);
    rAngles.azimuth = fmod(lambda_t + T, M_PI * 2.0);
    rAngles.elevation = asin(zsubt);

  }

}

///////////////////////////////////////////////////////////////////
// get the scale for a field name

double DoradeRadxFile::_getScale(const string &name)

{
  
  string lowerCaseName;
  for (size_t ii = 0; ii < name.size(); ii++) {
    const char cc = (char) tolower(name[ii]);
    lowerCaseName.append(1, cc);
  }

  double scale = 0.01;

  if (lowerCaseName.find("rho") != string::npos) {
    scale = 0.0001;
  } else if (lowerCaseName.find("ncp") != string::npos) {
    scale = 0.0001;
  } else if (lowerCaseName.find("sqi") != string::npos) {
    scale = 0.0001;
  } else if (lowerCaseName.find("kdp") != string::npos) {
    scale = 0.001;
  } else if (lowerCaseName.find("phi") != string::npos) {
    scale = 0.02;
  }

  return scale;

}


