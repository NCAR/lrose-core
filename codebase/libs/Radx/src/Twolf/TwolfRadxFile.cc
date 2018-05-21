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
// TwolfRadxFile.cc
//
// TWOLF format for lidar radial data
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2015
//
///////////////////////////////////////////////////////////////

#include <Radx/NcfRadxFile.hh>
#include <Radx/TwolfRadxFile.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxBuf.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxXml.hh>
#include <Radx/RadxStr.hh>
#include <Radx/RadxAngleHist.hh>
#include <cstring>
#include <cmath>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <cstdlib>
using namespace std;

int TwolfRadxFile::_volNum = 0;

//////////////
// Constructor

TwolfRadxFile::TwolfRadxFile() : RadxFile()
  
{

  _readVol = NULL;
  _file = NULL;
  clear();

}

/////////////
// destructor

TwolfRadxFile::~TwolfRadxFile()

{
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void TwolfRadxFile::clear()
  
{

  clearErrStr();
  _close();
  _clearRays();

}

void TwolfRadxFile::_clearRays()
{
  for (int ii = 0; ii < (int) _rays.size(); ii++) {
    delete _rays[ii];
  }
  _rays.clear();
}

/////////////////////////////////////////////////////////
// Check if specified file is Twolf format
// Returns true if supported, false otherwise

bool TwolfRadxFile::isSupported(const string &path)

{
  
  if (isTwolf(path)) {
    return true;
  }
  return false;

}

////////////////////////////////////////////////////////////
// Check if this is a Twolf file
// Returns true on success, false on failure

bool TwolfRadxFile::isTwolf(const string &path)
  
{

  _close();
  
  // open file
  
  if (_openRead(path)) {
    _addErrStr("ERROR - TwolfRadxFile::isTwolf");
    return false;
  }

  // read in 10 lines, make sure this seems like a TWOLF file
  // we expect "shot hour min sec msec gateNum
  // and the first 5 fields should repeat a number of time

  char line[10000];
  int shot, hour, min, sec, msec, gateNum;
  int shot0 = 0, hour0 = 0, min0 = 0, sec0 = 0, msec0 = 0;
  
  for (int ii = 0; ii < 10; ii++) {

    if (fgets(line, 10000, _file) == NULL) {
      _close();
      return false;
    }

    if (sscanf(line, "%d %d %d %d %d %d",
               &shot, &hour, &min, &sec, &msec, &gateNum) != 6) {
      _close();
      return false;
    }

    if (gateNum != ii + 1) {
      _close();
      return false;
    }

    if (ii == 0) {
      shot0 = shot;
      hour0 = hour;
      min0 = min;
      sec0 = sec;
      msec0 = msec;
    } else {
      if (shot != shot0 ||
          hour != hour0 ||
          min != min0 ||
          sec != sec0 ||
          msec != msec0) {
        _close();
        return false;
      }
    }

  }

  _close();
  
  return true;

}

////////////////////////////////////////////////
// set the start date and time from a file path
//
// Returns 0 on success, -1 on failure

int TwolfRadxFile::_getStartTimeFromPath(const string &path)

{

  _startTime.set(RadxTime::NEVER);

  RadxPath rpath(path);
  const string &fileName = rpath.getFile();

  // find first digit

  const char *firstDigit = fileName.c_str();
  while (!isdigit(*firstDigit)) {
    firstDigit++;
    if (*firstDigit == '\0') {
      // got null
      _addErrStr("ERROR - TwolfRadxFile::getStartTimeFromPath");
      _addErrStr("  Cannot get time from path: ", path);
      _addErrStr("  Should be: *_mmddyyyy_hhmmss*");
      return -1;
    }
  }
  if (strlen(firstDigit) < 15) {
    _addErrStr("ERROR - TwolfRadxFile::getStartTimeFromPath");
    _addErrStr("  Cannot get time from path: ", path);
    _addErrStr("  Should be: *_mmddyyyy_hhmmss*");
    return -1;
  }

  int year, month, day, hour, min, sec;
  if (sscanf(firstDigit, "%2d%2d%4d_%2d%2d%2d",
             &month, &day, &year, &hour, &min, &sec) != 6) {
    _addErrStr("ERROR - TwolfRadxFile::getStartTimeFromPath");
    _addErrStr("  Cannot get time from path: ", path);
    _addErrStr("  Should be: *_mmddyyyy_hhmmss*");
    return -1;
  }

  _startTime.set(year, month, day, hour, min, sec);

  return 0;

}

////////////////////////////////////////////////////////////
// Read in data from specified path, load up volume object.
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs

int TwolfRadxFile::readFromPath(const string &path,
                                RadxVol &vol)
  
{

  _initForRead(path, vol);

  // is this a TWOLF file?
  
  if (!isTwolf(_pathInUse)) {
    _addErrStr("ERROR - TwolfRadxFile::readFromPath");
    _addErrStr("  Not a twolf file: ", _pathInUse);
    return -1;
  }

  // get start date and time from path

  if (_getStartTimeFromPath(path)) {
    _addErrStr("ERROR - TwolfRadxFile::readFromPath");
    return -1;
  }
  
  // open data file

  if (_openRead(_pathInUse)) {
    _addErrStr("ERROR - TwolfRadxFile::readFromPath");
    return -1;
  }

  // read single file

  if (_readFile(path)) {
    _addErrStr("ERROR - TwolfRadxFile::readFromPath");
    return -1;
  }

  _readPaths.push_back(path);

  // is this an RHI

  _rhiMode = false;
  if (RadxAngleHist::checkIsRhi(_rays)) {

    _rhiMode = true;
    double startAz = _rays[0]->getAzimuthDeg();
    for (size_t ii = 0; ii < _rays.size(); ii++) {
      RadxRay *ray = _rays[ii];
      ray->setSweepMode(Radx::SWEEP_MODE_RHI);
      ray->setFixedAngleDeg(ray->getAzimuthDeg());
      // check for over-the-top condition
      double azimuth = ray->getAzimuthDeg();
      double elevation = ray->getElevationDeg();
      double deltaAz = fabs(azimuth - startAz);
      if (deltaAz > 180) {
        deltaAz = fabs(deltaAz - 360.0);
      }
      if (fabs(deltaAz - 180) < 1.0) {
        // over the top RHI
        ray->setAzimuthDeg(startAz);
        ray->setFixedAngleDeg(startAz);
        ray->setElevationDeg(180.0 - elevation);
      }
    }

  } else {

    for (size_t ii = 0; ii < _rays.size(); ii++) {
      RadxRay *ray = _rays[ii];
      ray->setSweepMode(Radx::SWEEP_MODE_SECTOR);
      ray->setFixedAngleDeg(ray->getElevationDeg());
    }

  }

  // load the data into the read volume

  if (_loadReadVolume()) {
    return -1;
  }

  // set the packing from the rays

  _readVol->setPackingFromRays();

  // set format as read
  
  _fileFormat = FILE_FORMAT_TWOLF;

  return 0;

}

////////////////////////////////////////////////////////////
// Load rays from file
// Returns 0 on success, -1 on failure

int TwolfRadxFile::_readFile(const string &path)
  
{

  if (_debug) {
    cerr << "INFO - _readFile()" << endl;
    cerr << "  path: " << path << endl;
  }

  // is this a Twolf file?
  
  if (!isTwolf(path)) {
    _addErrStr("ERROR - TwolfRadxFile::readFromPath");
    _addErrStr("  Not a twolf file: ", path);
    return -1;
  }

  // open data file
  
  if (_openRead(path)) {
    _addErrStr("ERROR - TwolfRadxFile::readFromPath");
    return -1;
  }

  // read in ray data

  if (_readRayData()) {
    _addErrStr("ERROR - TwolfRadxFile::readFromPath");
    _close();
    return -1;
  }

  // close file

  _close();

  if (_debug) {
    cerr << "End of file" << endl;
  }
  
  if (_rays.size() < 1) {
    _addErrStr("ERROR - TwolfRadxFile::readFromPath");
    _addErrStr("  No rays found");
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////
// Read in ray ata
// Returns 0 on success, -1 on failure

int TwolfRadxFile::_readRayData()
  
{
  
  // read through the data records
  
  char line[65536];

  int shot, hour, minute, second, msec, gate;
  double azimuth, elevation;
  double afe, losdist, x, y, height;
  double vlos, snr, qual, xdist;

  bool firstLine = true;
  int prevShot = -9999;
  int prevGate = 999999;
  RadxRay *ray = NULL;

  _startRangeKm = 397.5;
  _gateSpacingKm = 46.5;

  while (!feof(_file)) {
    
    // get next line
    
    if (fgets(line, 65536, _file) == NULL) {
      break;
    }

    if (sscanf(line,
               "%d %d %d %d %d %d "
               "%lg %lg %lg %lg %lg %lg %lg %lg %lg %lg %lg",
               &shot, &hour, &minute, &second, &msec, &gate,
               &azimuth, &elevation,
               &afe, &losdist, &x, &y, &height,
               &vlos, &snr, &qual, &xdist) != 17) {
      continue;
    }

    if (firstLine) {
      // first line read
      prevShot = shot;
      firstLine = false;
    }

    // end of ray?

    if (shot != prevShot || feof(_file)) {
      
      // end of current ray
      
      if (_vel.size() > 0) {
        
        RadxField *velField = new RadxField("vel", "m/s");
        velField->setLongName("radial_velocity");
        velField->setStandardName
          ("radial_velocity_of_scatterers_away_from_instrument");
        velField->setMissingFl32(Radx::missingFl32);
        velField->setDataFl32(_vel.size(), &_vel[0], true);
        ray->addField(velField);
        
        RadxField *snrField = new RadxField("snr", "dB");
        snrField->setLongName("Signal_to_noise_ratio");
        snrField->setStandardName("Signal_to_noise_ratio");
        snrField->setMissingFl32(Radx::missingFl32);
        snrField->setDataFl32(_snr.size(), &_snr[0], true);
        ray->addField(snrField);
        
        RadxField *qualField = new RadxField("qual", "");
        qualField->setLongName("Quality_flag");
        qualField->setMissingFl32(Radx::missingFl32);
        qualField->setDataFl32(_qual.size(), &_qual[0], true);
        ray->addField(qualField);
        
        // add ray to vector
        
        _rays.push_back(ray);
        
      }
        
      ray = NULL;
      
    } // if (shot != prevShot || feof(_file)) 

    if (azimuth < 0) {
      azimuth += 360.0;
    } else if (azimuth > 360) {
      azimuth -= 360.0;
    }

    // start of ray?

    if (ray == NULL) {

      ray = new RadxRay;
      prevGate = gate - 1;
      RadxTime rayTime(_startTime.getYear(),
                       _startTime.getMonth(),
                       _startTime.getDay(),
                       hour, minute, second, msec / 1000.0);
      if (rayTime - _startTime < 0) {
        // change of day
        rayTime += 86400;
      }
      ray->setTime(rayTime);
      ray->setVolumeNumber(_volNum);
      ray->setSweepNumber(-1);
      ray->setAzimuthDeg(azimuth);
      ray->setElevationDeg(elevation);
      _vel.clear();
      _snr.clear();
      _qual.clear();
    }
    
    // fill in any missing gates

    if (gate - prevGate > 1) {
      for (int igate = prevGate + 1; igate < gate; igate++) {
        _vel.push_back(Radx::missingFl32);
        _snr.push_back(Radx::missingFl32);
        _qual.push_back(Radx::missingFl32);
        _range.push_back(_startRangeKm + _gateSpacingKm * igate);
      }
    }

    // compute gate spacing

    if (gate == 2) {
      _startRangeKm = _range[0] / 1000.0;
      _gateSpacingKm = (_range[1] - _range[0]) / 1000.0;
      ray->setRangeGeom(_startRangeKm, _gateSpacingKm);
    }

    // save data fields

    _vel.push_back(vlos);
    _snr.push_back(snr);
    _qual.push_back(qual);
    _range.push_back(losdist);

    _afe = afe;
    
    prevShot = shot;
    prevGate = gate;
               
  } // while

  return 0;

}

//////////////////////////////////////
// open netcdf file for reading
// Returns 0 on success, -1 on failure

int TwolfRadxFile::_openRead(const string &path)
  
{

  _close();
  _file = fopen(path.c_str(), "r");
  
  // Check that constructor succeeded
  
  if (_file == NULL) {
    int errNum = errno;
    _addErrStr("ERROR - TwolfRadxFile::_openRead");
    _addErrStr("  Cannot open file for reading, path: ", path);
    _addErrStr("  ", strerror(errNum));
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////
// close netcdf file if open
// remove error object if it exists

void TwolfRadxFile::_close()
  
{
  
  // close file if open
  
  if (_file) {
    fclose(_file);
    _file = NULL;
  }
  
}

/////////////////////////////////////////////////////////
// load up the read volume with the data from this object

int TwolfRadxFile::_loadReadVolume()
  
{

  int nRays = _rays.size();
  if (nRays < 1) {
    if (_debug) {
      cerr << "WARNING - TwolfRadxFile::_loadReadVolume" << endl;
      cerr << "  No rays" << endl;
    }
    return -1;
  }
  
  _readVol->clear();

  // set meta-data
 
  _readVol->setOrigFormat("TWOLF");
  _readVol->setVolumeNumber(0);
  _readVol->setInstrumentType(Radx::INSTRUMENT_TYPE_RADAR);
  _readVol->setPlatformType(Radx::PLATFORM_TYPE_FIXED);
  
  _readVol->setStartTime(_rays[0]->getTimeSecs(),
                         _rays[0]->getNanoSecs());
  _readVol->setEndTime(_rays[nRays-1]->getTimeSecs(),
                       _rays[nRays-1]->getNanoSecs());

  _readVol->setTitle("TWOLF LIDAR");
  _readVol->setSource("TWOLF");
  _readVol->setInstrumentName("TWOLF");

  _readVol->setLatitudeDeg(0.0);
  _readVol->setLongitudeDeg(0.0);
  _readVol->setAltitudeKm(0.0);
  _readVol->setSensorHtAglM(0.0);

  // add rays

  for (int ii = 0; ii < (int) _rays.size(); ii++) {
    _readVol->addRay(_rays[ii]);
  }

  // memory allocation for rays has passed to _readVol,
  // so free up pointer array

  _rays.clear();
  
  // load the sweep information from the rays

  _readVol->loadSweepInfoFromRays();
  
  // constrain the sweep data as appropriate
  
  if (_readFixedAngleLimitsSet) {
    if (_readVol->constrainByFixedAngle(_readMinFixedAngle, _readMaxFixedAngle,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - TwolfRadxFile::_loadReadVolume");
      _addErrStr("  No data found within fixed angle limits");
      _addErrDbl("  min fixed angle: ", _readMinFixedAngle);
      _addErrDbl("  max fixed angle: ", _readMaxFixedAngle);
      return -1;
    }
  } else if (_readSweepNumLimitsSet) {
    if (_readVol->constrainBySweepNum(_readMinSweepNum, _readMaxSweepNum,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - TwolfRadxFile::_loadReadVolume");
      _addErrStr("  No data found within sweep num limits");
      _addErrInt("  min sweep num: ", _readMinSweepNum);
      _addErrInt("  max sweep num: ", _readMaxSweepNum);
      return -1;
    }
  }

  // check for indexed rays, set info on rays
  
  _readVol->checkForIndexedRays();

  // load up volume info from rays

  _readVol->loadVolumeInfoFromRays();

  return 0;
  
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

int TwolfRadxFile::writeToDir(const RadxVol &vol,
                            const string &dir,
                            bool addDaySubDir,
                            bool addYearSubDir)
  
{

  // Writing Twolf files is not supported
  // therefore write in CF Radial format instead

  cerr << "WARNING - TwolfRadxFile::writeToDir" << endl;
  cerr << "  Writing TWOLF format files not supported" << endl;
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

  return iret;

}

/////////////////////////////////////////////////////////
// Write data from volume to specified path
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs
// Use getPathInUse() for path written

int TwolfRadxFile::writeToPath(const RadxVol &vol,
                             const string &path)
  
{

  // Writing TWOLF files is not supported
  // therefore write in CF Radial format instead

  cerr << "WARNING - TwolfRadxFile::writeToPath" << endl;
  cerr << "  Writing Twolf format files not supported" << endl;
  cerr << "  Will write CfRadial file instead" << endl;

  // set up NcfRadxFile object

  NcfRadxFile ncfFile;
  ncfFile.copyWriteDirectives(*this);

  // perform write

  int iret = ncfFile.writeToPath(vol, path);

  // set return values

  _errStr = ncfFile.getErrStr();
  _pathInUse = ncfFile.getPathInUse();

  return iret;

}

/////////////////////////////////////////////////////////
// print data after read

void TwolfRadxFile::print(ostream &out) const
  
{
  
  out << "=============== TwolfRadxFile ===============" << endl;
  RadxFile::print(out);
  out << "===========================================" << endl;

}

////////////////////////////////////////////////////////////
// Print native data in uf file
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int TwolfRadxFile::printNative(const string &path, ostream &out,
                               bool printRays, bool printData)
  
{

  clear();

  // is this a Twolf file?
  
  if (!isTwolf(path)) {
    _addErrStr("ERROR - TwolfRadxFile::printNative");
    _addErrStr("  Not a TWOLF file: ", path);
    return -1;
  }

  out << "NOTE - native print not supported for TWOLF files" << endl;
  cerr << "  path: " << path << endl;
  cerr << "  Use 'cat' instead" << endl;

  return 0;

}

