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
// NsslMrdRadxFile.cc
//
// NsslMrdRadxFile object
//
// Support for radial data in NSSL MRD format
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2015
//
///////////////////////////////////////////////////////////////

#include <Radx/NsslMrdRadxFile.hh>
#include <Radx/NcfRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/ByteOrder.hh>
#include <Radx/RadxArray.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxXml.hh>
#include <Radx/RadxStr.hh>
#include <iomanip>
#include <cstdio>
using namespace std;

int NsslMrdRadxFile::_volumeNumber = 0;
 
//////////////
// Constructor

NsslMrdRadxFile::NsslMrdRadxFile() : RadxFile()
                                   
{

  _readVol = NULL;
  _file = NULL;
  clear();

}

/////////////
// destructor

NsslMrdRadxFile::~NsslMrdRadxFile()

{
  clear();
}

/////////////////////////////////////////////////////////
// Check if specified file is NSSL MRD format
// Returns true if supported, false otherwise

bool NsslMrdRadxFile::isSupported(const string &path)

{

  if (isNsslMrd(path)) {
    return true;
  }

  return false;

}

////////////////////////////////////////////////////////////
// Check if this is an NSSL MRD file
// Returns true on success, false on failure

bool NsslMrdRadxFile::isNsslMrd(const string &path)
  
{

  clear();
  
  // open file
  
  if (_openRead(path)) {
    _addErrStr("ERROR - NsslMrdRadxFile::isNsslMrd");
    return false;
  }
  
  // read first record into input buffer

  if (_readRec()) {
    _close();
    return false;
  }
  _close();
  
  // load headers
  // if this fails then the headers are not valid
  
  if (_loadHeaders()) {
    return false;
  }

  return true;

}

/////////////////////////////////////////////////////////
// clear the data in the object

void NsslMrdRadxFile::clear()
  
{

  clearErrStr();

  _close();

  _nRaysRead = 0;

  _mrdIsSwapped = false;
  _dataLen = 0;

  _startTimeSecs = 0;
  _endTimeSecs = 0;
  _startNanoSecs = 0;
  _endNanoSecs = 0;

  _nGates = 0;
  _nSamples = 0;

  _frequencyHz = 9.31032e+09;
  _pulseWidthUs = 0.380263;
  _nyquist = 0.0;

  _gateSpacingKm = 0.0;
  _startRangeKm = 0.0;

  _latitude = Radx::missingMetaDouble;
  _longitude = Radx::missingMetaDouble;
  _altitudeM = Radx::missingMetaDouble;

}

/////////////////////////////////////////////////////////
// Write data from volume to specified directory
// Writes as CFRadial, NSSL MRD not supported on write
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs
// Use getPathInUse() for path written

int NsslMrdRadxFile::writeToDir(const RadxVol &vol,
                                const string &dir,
                                bool addDaySubDir,
                                bool addYearSubDir)
  
{

  clearErrStr();
  _dirInUse = dir;

  if (_debug) {
    cerr << "NsslMrdRadxFile::writeToDir" << endl;
    cerr << "  Writing to dir: " << dir << endl;
  }
  
  // written using the start time
  
  RadxTime ftime(vol.getStartTimeSecs());
  
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
    _addErrStr("ERROR - NsslMrdRadxFile::writeToDir");
    _addErrStr("  Cannot make output dir: ", outDir);
    return -1;
  }
  
  // compute path
  
  int volNum = vol.getVolumeNumber();
  string outName =
    computeFileName(volNum,
                    vol.getInstrumentName(),
                    ftime.getYear(), ftime.getMonth(), ftime.getDay(),
                    ftime.getHour(), ftime.getMin(), ftime.getSec());
  string outPath(outDir);
  outPath += PATH_SEPARATOR;
  outPath += outName;
  
  int iret = writeToPath(vol, outPath);
  
  if (iret) {
    _addErrStr("ERROR - NsslMrdRadxFile::writeToDir");
    return -1;
  }

  return 0;
  
}

/////////////////////////////////////////////////////////
// Write data from volume to specified path
// Writes as CFRadial, NSSL MRD not supported on write
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs
// Use getPathInUse() for path written

int NsslMrdRadxFile::writeToPath(const RadxVol &vol,
                                 const string &path)
  
{

  // initialize

  clearErrStr();
  _pathInUse = path;
  vol.setPathInUse(path);
  _writePaths.clear();
  _writeDataTimes.clear();

  // ensure we have computed the max number of gates
  
  vol.computeMaxNGates();

  // check comment for mrd-related items
  
  string comment = vol.getComment();
  string outerTag = "nssl_mrd";
  string flight_number, storm_name, aircraft_id;
  if (RadxXml::readStartTag(comment, outerTag) == 0) {
    RadxXml::readString(comment, "flight_number", flight_number);
    RadxXml::readString(comment, "storm_name", storm_name);
    RadxXml::readString(comment, "aircraft_id", aircraft_id);
  }

  // open the output file

  string tmpPath(tmpPathFromFilePath(path, ""));
  
  if (_debug) {
    cerr << "DEBUG - NsslMrdRadxFile::writeToPath" << endl;
    cerr << "  Writing to path: " << path << endl;
    cerr << "  Tmp path: " << tmpPath << endl;
  }

  if (_openWrite(tmpPath)) {
    _addErrStr("ERROR - NsslMrdRadxFile::writeToPath");
    _addErrStr("  Cannot open tmp dorade file: ", tmpPath);
    return -1;
  }
  
  // loop through the rays

  const vector<RadxRay *> &rays = vol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {

    RadxRay *ray = rays[iray];
    RadxTime rtime = ray->getRadxTime();

    // load up the headers

    mrd_header_t hdr;
    mrd_header2_t hdr2;
    
    memset(&hdr, 0, sizeof(hdr));
    memset(&hdr2, 0, sizeof(hdr2));

    hdr.nyq_vel_x10 = (int) floor(ray->getNyquistMps() * 10.0 + 0.5);
    hdr.julian_date = rtime.getDayOfYear();
    hdr.azimuth_samples = ray->getNSamples();
    hdr.gate_length = (int) (ray->getGateSpacingKm() * 1000.0 + 0.5);
    hdr.range_delay = (int) (ray->getStartRangeKm() * 1000.0 + 0.5);

    if (storm_name.size() > 0) {
      strncpy(hdr.storm_name, storm_name.c_str(), 12);
    }

    if (flight_number.size() > 0) {
      strncpy(hdr.flight_number, flight_number.c_str(), 8);
    } else {
      strncpy(hdr.flight_number, vol.getInstrumentName().c_str(), 8);
    }

    if (aircraft_id.size() > 0) {
      strncpy(hdr.aircraft_id, aircraft_id.c_str(), 2);
    } else {
      if (vol.getInstrumentName().find("42") != string::npos) {
        memcpy(hdr.aircraft_id, "42", 2);
      }
      if (vol.getInstrumentName().find("43") != string::npos) {
        memcpy(hdr.aircraft_id, "43", 2);
      }
    }

    hdr.num_good_gates = ray->getNGates();
    hdr.sweep_num = ray->getSweepNumber();
    hdr.max_gates = vol.getMaxNGates();
    
    RadxGeoref *georef = ray->getGeoreference();
    if (georef) {

      hdr.raw_rot_ang_x10 = (int) floor(georef->getRotation() * 10.0 + 0.5);

      double flat = georef->getLatitude();
      hdr.lat_deg = (int) flat;
      double flatmin = (flat - (double) hdr.lat_deg) * 60.0;
      hdr.lat_min = (int) flatmin;
      double flatsec = (flatmin - (double) hdr.lat_min) * 60.0;
      hdr.lat_sec_x10 = (int) floor(flatsec * 10.0 + 0.5);

      double flon = georef->getLongitude();
      hdr.lon_deg = (int) flon;
      double flonmin = (flon - (double) hdr.lon_deg) * 60.0;
      hdr.lon_min = (int) flonmin;
      double flonsec = (flonmin - (double) hdr.lon_min) * 60.0;
      hdr.lon_sec_x10 = (int) floor(flonsec * 10.0 + 0.5);

      hdr.altitude = (int) floor(georef->getAltitudeKmMsl() * 1000.0 + 0.5);
      hdr.roll_x10 = (int) floor(georef->getRoll() * 10.0 + 0.5);
      hdr.heading_x10 = (int) floor(georef->getHeading() * 10.0 + 0.5);
      hdr.drift_x10 = (int) floor(georef->getDrift() * 10.0 + 0.5);
      hdr.pitch_x10 = (int) floor(georef->getPitch() * 10.0 + 0.5);
      hdr.raw_tilt_x10 = (int) floor((georef->getTilt() + 180.0) * 10.0 + 0.5);
      
      double ewVel = georef->getEwVelocity();
      double nsVel = georef->getNsVelocity();
      double gndSpeed = sqrt(ewVel * ewVel + nsVel * nsVel);
      hdr.ground_speed_x64 = (int) floor(gndSpeed * 64.0 + 0.5);
      hdr.vert_airspeed_x64 = (int) floor(georef->getVertVelocity() * 64.0 + 0.5);

      double ewWind = georef->getEwWind();
      double nsWind = georef->getNsWind();
      double windSpeed = sqrt(ewWind * ewWind + nsWind * nsWind);
      double windDirn = 0.0;
      if (nsWind == 0.0 && ewWind == 0.0) {
        windDirn = 0.0;
      } else {
        windDirn = atan2(-ewWind, -nsWind) / Radx::DegToRad;
      }
      if (windDirn < 0) {
        windDirn += 360.0;
      }
      hdr.wind_dir_x10 = (int) floor(windDirn * 10.0 + 0.5);
      hdr.wind_speed_x10 = (int) floor(windSpeed * 10.0 + 0.5);
      
    } // if (georef() ...

    hdr2.year = rtime.getYear();
    hdr2.month = rtime.getMonth();
    hdr2.day = rtime.getDay();
    hdr2.hour = rtime.getHour();
    hdr2.minute = rtime.getMin();
    hdr2.second = rtime.getSec();

    // write leading fortran record

    Radx::ui32 frec = sizeof(hdr) + sizeof(hdr2);
    if (!ByteOrder::hostIsBigEndian()) {
      ByteOrder::swap32(&frec, sizeof(frec));
    }
    if (fwrite(&frec, sizeof(frec), 1, _file) != 1) {
      int errNum = errno;
      _addErrStr("ERROR - NsslMrdRadxFile::writeToPath");
      _addErrStr("  Cannot write leading header fortran record to file: ", tmpPath);
      _addErrStr(strerror(errNum));
      _close();
      return -1;
    }
    
    // swap the bytes and write the headers

    if (!ByteOrder::hostIsBigEndian()) {
      _swap(hdr);
      _swap(hdr2);
    }

    if (fwrite(&hdr, sizeof(hdr), 1, _file) != 1) {
      int errNum = errno;
      _addErrStr("ERROR - NsslMrdRadxFile::writeToPath");
      _addErrStr("  Cannot write header to file: ", tmpPath);
      _addErrStr(strerror(errNum));
      _close();
      return -1;
    }
    if (fwrite(&hdr2, sizeof(hdr2), 1, _file) != 1) {
      int errNum = errno;
      _addErrStr("ERROR - NsslMrdRadxFile::writeToPath");
      _addErrStr("  Cannot write header2 to file: ", tmpPath);
      _addErrStr(strerror(errNum));
      _close();
      return -1;
    }

    // write trailing fortran record

    if (fwrite(&frec, sizeof(frec), 1, _file) != 1) {
      int errNum = errno;
      _addErrStr("ERROR - NsslMrdRadxFile::writeToPath");
      _addErrStr("  Cannot write trailing header fortran record to file: ", tmpPath);
      _addErrStr(strerror(errNum));
      _close();
      return -1;
    }
    
    // create array for output data

    size_t nGates = ray->getNGates();
    RadxArray<Radx::ui16> packed_;
    Radx::ui16 *packed = packed_.alloc(nGates);

    // check number of fields

    vector<RadxField *> fields = ray->getFields();
    if (fields.size() < 2) {
      _addErrStr("ERROR - NsslMrdRadxFile::writeToPath");
      _addErrStr("  Too few fields for ray, file: ", tmpPath);
      _addErrInt("  nFields: ", fields.size());
      return -1;
    }
    
    // locate the dbz and vel fields

    RadxField *dbzField = NULL;
    for (size_t ifield = 0; ifield < fields.size(); ifield++) {
      RadxField *field = fields[ifield];
      if (field->getStandardName().find("reflectivity_factor") != string::npos) {
        dbzField = field;
        break;
      }
      string name = field->getName();
      for (size_t ii = 0; ii < name.size(); ii++) {
        name[ii] = tolower(name[ii]);
      }
      if (field->getName().find("dbz") != string::npos) {
        dbzField = field;
        break;
      }
      string units = field->getUnits();
      for (size_t ii = 0; ii < units.size(); ii++) {
        units[ii] = tolower(units[ii]);
      }
      if (field->getUnits().find("dbz") != string::npos) {
        dbzField = field;
        break;
      }
    } // ifield

    RadxField *velField = NULL;
    for (size_t ifield = 0; ifield < fields.size(); ifield++) {
      RadxField *field = fields[ifield];
      if (field->getStandardName().find("velocity") != string::npos) {
        velField = field;
        break;
      }
      string name = field->getName();
      for (size_t ii = 0; ii < name.size(); ii++) {
        name[ii] = tolower(name[ii]);
      }
      if (field->getName().find("vel") != string::npos) {
        velField = field;
        break;
      }
      string units = field->getUnits();
      for (size_t ii = 0; ii < units.size(); ii++) {
        units[ii] = tolower(units[ii]);
      }
      if (field->getUnits().find("m/s") != string::npos) {
        velField = field;
        break;
      }
    } // ifield

    if (dbzField == NULL) {
      dbzField = fields[0];
    }
    if (velField == NULL) {
      velField = fields[1];
    }

    // load up data array

    dbzField->convertToFl32();
    velField->convertToFl32();
    const Radx::fl32 *dbzArray = dbzField->getDataFl32();
    const Radx::fl32 *velArray = velField->getDataFl32();
    for (size_t igate = 0; igate < nGates; igate++) {

      double dbz = dbzArray[igate];
      int idbz = (int) floor(dbz + 0.5);
      if (idbz < 0) {
        idbz = 0;
      } else if (idbz > 63) {
        idbz = 63;
      }

      double vel = velArray[igate];
      int ivel = (int) floor(vel * 7.5 + 0.5) + 511;
      if (ivel < 0) {
        ivel = 0;
      } else if (ivel > 1023) {
        ivel = 1023;
      }

      Radx::ui32 packedVal = idbz << 10;
      packedVal |= ivel;

      packed[igate] = packedVal;

    } // igate
    
    // write leading fortran record

    frec = nGates * sizeof(Radx::ui16);
    if (!ByteOrder::hostIsBigEndian()) {
      ByteOrder::swap32(&frec, sizeof(frec));
    }
    if (fwrite(&frec, sizeof(frec), 1, _file) != 1) {
      int errNum = errno;
      _addErrStr("ERROR - NsslMrdRadxFile::writeToPath");
      _addErrStr("  Cannot write leading data fortran record to file: ", tmpPath);
      _addErrStr(strerror(errNum));
      _close();
      return -1;
    }

    // write data

    if (fwrite(packed, sizeof(Radx::ui16), nGates, _file) != nGates) {
      int errNum = errno;
      _addErrStr("ERROR - NsslMrdRadxFile::writeToPath");
      _addErrStr("  Cannot write packed data to file: ", tmpPath);
      _addErrStr(strerror(errNum));
      _close();
      return -1;
    }

    // write trailing fortran record

    if (fwrite(&frec, sizeof(frec), 1, _file) != 1) {
      int errNum = errno;
      _addErrStr("ERROR - NsslMrdRadxFile::writeToPath");
      _addErrStr("  Cannot write trailing data fortran record to file: ", tmpPath);
      _addErrStr(strerror(errNum));
      _close();
      return -1;

    }

  } // iray

  // close output file
  
  _close();
  
  // rename the tmp to final output file path
  
  if (rename(tmpPath.c_str(), _pathInUse.c_str())) {
    int errNum = errno;
    _addErrStr("ERROR - NsslMrdRadxFile::writeToPath");
    _addErrStr("  Cannot rename tmp file: ", tmpPath);
    _addErrStr("  to: ", _pathInUse);
    _addErrStr(strerror(errNum));
    return -1;
  }

  if (_debug) {
    cerr << "DEBUG - NsslMrdRadxFile::writeToPath" << endl;
    cerr << "  Renamed tmp path: " << tmpPath << endl;
    cerr << "     to final path: " << path << endl;
  }

  // add to array of write paths

  _writePaths.push_back(path);
  _writeDataTimes.push_back(vol.getStartTimeSecs());

  return 0;

}

////////////////////////////////////////////////////////////
// compute and return output file name

string NsslMrdRadxFile::computeFileName(int volNum,
                                        string instrumentName,
                                        int year, int month, int day,
                                        int hour, int min, int sec)
  
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

  char outName[BUFSIZ];
  sprintf(outName, "mrd.%d%02d%02d%02d%02d%02d.%s.%d.tape",
          year - 1900, month, day, hour, min, sec,
          instrumentName.c_str(), volNum);

  return outName;

}
  
////////////////////////////////////////////////////////////
// Read in data from specified path, load up volume object.
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs

int NsslMrdRadxFile::readFromPath(const string &path,
                                  RadxVol &vol)
  
{

  _initForRead(path, vol);

  if (!isNsslMrd(path)) {
    _addErrStr("ERROR - NsslMrdRadxFile::readFromPath");
    _addErrStr("  Not a recognized NSSL MRD file");
    return -1;
  }

  // get instrument name from path

  string instrumentName = _getInstrumentNameFromPath(path);
  
  // open file

  if (_openRead(path)) {
    _addErrStr("ERROR - NsslMrdRadxFile::readFromPath");
    return -1;
  }
  
  // set volume number - increments per file

  _volumeNumber++;
  
  // loop through records

  while (!feof(_file)) {

    // read a record - this should be a header record
    
    if (_readRec()) {
      if (feof(_file)) {
        continue;
      } else {
        _addErrStr("ERROR - NsslMrdRadxFile::readFromPath");
        _addErrStr("  Reading header record");
        _close();
        return -1;
      }
    }
    
    // load up headers
    // if this fails, read another record until we get headers

    while (_loadHeaders()) {
      if (_readRec()) {
        if (feof(_file)) {
          continue;
        } else {
          _addErrStr("ERROR - NsslMrdRadxFile::readFromPath");
          _addErrStr("  Reading header record");
          _close();
          return -1;
        }
      }
    } // while

    // read a data record
    
    if (_readRec()) {
      if (feof(_file)) {
        continue;
      } else {
        _addErrStr("ERROR - NsslMrdRadxFile::readFromPath");
        _addErrStr("  Reading data record");
        _close();
        return -1;
      }
    }

    // does the length match that expected of the data?

    int nGatesData = _inBufSize / sizeof(Radx::ui16);
    if (nGatesData != _hdr.num_good_gates) {
      // length does not match
      if (_debug) {
        cerr << "WARNING - got record of incorrect length" << endl;
        cerr << "  Expected num_good_gates: " << _hdr.num_good_gates << endl;
        cerr << "  Got nGates: " << nGatesData << endl;
      }
      // continue reading records
      continue;
    }
    
    // data matches expected number of gates

    _datBuf = _inBuf;
    _dataLen = _datBuf.getLen();
    _dataRec = (Radx::ui16 *) _datBuf.getPtr();
    _nRaysRead++;
    
    // handle the ray data
    
    _handleRay();

  } // while (!feof ...

  // close input file

  _close();

  // check we have some rays

  if (_readVol->getNRays() == 0) {
    _addErrStr("ERROR - NsslMrdRadxFile::readFromPath");
    _addErrStr("  No rays found, file: ", _pathInUse);
    return -1;
  }
  
  // set the meta data on the volume

  _setVolMetaData(instrumentName);
  
  // apply goeref info if applicable
  
  if (_readApplyGeorefs) {
    _readVol->applyGeorefs();
  }
  
  // load the sweep information from the rays
  
  _readVol->loadSweepInfoFromRays();
  
  // check for indexed rays, set info on rays
  
  _readVol->checkForIndexedRays();
  
  // because rays are not all the same length in gates
  // set the packing from the rays
  
  _readVol->setPackingFromRays();
  
  if (_debug) {
    _readVol->print(cerr);
  }
  
  // add to paths used on read
  
  _readPaths.push_back(path);

  // set internal format

  _fileFormat = FILE_FORMAT_NSSL_MRD;

  return 0;

}

//////////////////////////////////////
// open file for reading
// Returns 0 on success, -1 on failure

int NsslMrdRadxFile::_openRead(const string &path)
  
{

  _close();
  _file = fopen(path.c_str(), "r");
  
  // Check
  
  if (_file == NULL) {
    int errNum = errno;
    _addErrStr("ERROR - NsslMrdRadxFile::_openRead");
    _addErrStr("  Cannot open file for reading, path: ", path);
    _addErrStr("  ", strerror(errNum));
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////
// open file for writing
// Returns 0 on success, -1 on failure

int NsslMrdRadxFile::_openWrite(const string &path) 
{
  
  _close();
  _file = fopen(path.c_str(), "w");
  
  // Check
  
  if (_file == NULL) {
    int errNum = errno;
    _addErrStr("ERROR - NexradRadxFile::_openWrite");
    _addErrStr("  Cannot open file for writing, path: ", path);
    _addErrStr("  ", strerror(errNum));
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////
// close file if open

void NsslMrdRadxFile::_close()
  
{
  
  // close file if open
  
  if (_file) {
    fclose(_file);
    _file = NULL;
  }
  
}

/////////////////////////////////////////////////////////
// print summary after read

void NsslMrdRadxFile::print(ostream &out) const
  
{
  
  out << "=============== NsslMrdRadxFile ===============" << endl;
  RadxFile::print(out);
  out << "===========================================" << endl;

}

////////////////////////////////////////////////////////////
// Print native data in uf file
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int NsslMrdRadxFile::printNative(const string &path, ostream &out,
                                 bool printRays, bool printData)
  
{

  RadxVol vol;
  _initForRead(path, vol);

  if (!isNsslMrd(path)) {
    _addErrStr("ERROR - NsslMrdRadxFile::printNative");
    _addErrStr("  Not a recognized NSSL MRD file");
    return -1;
  }
  
  // open file

  if (_openRead(path)) {
    _addErrStr("ERROR - NsslMrdRadxFile::printNative");
    return -1;
  }

  // loop through records

  while (!feof(_file)) {

    // read a record - this should be a header record
    
    if (_readRec()) {
      if (feof(_file)) {
        continue;
      } else {
        _addErrStr("ERROR - NsslMrdRadxFile::printNative");
        _addErrStr("  Reading header record");
        _close();
        return -1;
      }
    }
    
    // load up headers
    // if this fails, read another record until we get headers

    while (_loadHeaders()) {
      if (_readRec()) {
        if (feof(_file)) {
          continue;
        } else {
          _addErrStr("ERROR - NsslMrdRadxFile::printNative");
          _addErrStr("  Reading header record");
          _close();
          return -1;
        }
      }
    } // while

    // read a data record
    
    if (_readRec()) {
      if (feof(_file)) {
        continue;
      } else {
        _addErrStr("ERROR - NsslMrdRadxFile::printNative");
        _addErrStr("  Reading data record");
        _close();
        return -1;
      }
    }

    // does the length match that expected of the data?

    int nGatesData = _inBufSize / sizeof(Radx::ui16);
    if (nGatesData != _hdr.num_good_gates) {
      // length does not match
      if (_debug) {
        cerr << "WARNING - got record of incorrect length" << endl;
        cerr << "  Expected num_good_gates: " << _hdr.num_good_gates << endl;
        cerr << "  Got nGates: " << nGatesData << endl;
      }
      // continue reading records
      continue;
    }
    
    // data matches expected number of gates

    _datBuf = _inBuf;
    _dataLen = _datBuf.getLen();
    _dataRec = (Radx::ui16 *) _datBuf.getPtr();
    _nRaysRead++;
    
    // print

    _printRay(printData, out);

  } // while

  // close and return

  _close();
  return 0;

}

/////////////////////////////////////
// read a record
// store data in buffer

int NsslMrdRadxFile::_readRec()
  
{
  
  // read 4 byte start block
  
  Radx::si32 startCount;
  if (fread(&startCount, sizeof(startCount), 1, _file) != 1) {
    int errNum = errno;
    _addErrStr("ERROR - NsslMrdRadxFile::_readRec");
    _addErrStr("  Reading startCount block");
    _addErrStr("  ", strerror(errNum));
    return -1;
  }
  
  if (startCount < 0 || startCount > 4096) {
    ByteOrder::swap32(&startCount, sizeof(startCount));
  }
  _inBufSize = startCount;
  
  if (_inBufSize == 0) {
    // end of file
    return -1;
  }

  if (_inBufSize < 1 || _inBufSize > 10000000) {
    _addErrStr("ERROR - NsslMrdRadxFile::_readRec");
    _addErrInt("  Bad decoded buf size: ", _inBufSize);
    return -1;
  }

  // read in record

  char *buf = (char *) _inBuf.reserve(_inBufSize);
  if (fread(buf, 1, _inBufSize, _file) != _inBufSize) {
    int errNum = errno;
    _addErrStr("ERROR - NsslMrdRadxFile::_readRec");
    _addErrStr("  Reading record");
    _addErrStr("  ", strerror(errNum));
    return -1;
  }
  
  // read 4 byte end block
  
  Radx::si32 endCount;
  if (fread(&endCount, sizeof(endCount), 1, _file) != 1) {
    int errNum = errno;
    _addErrStr("ERROR - NsslMrdRadxFile::_readRec");
    _addErrStr("  Reading endCount block");
    _addErrStr("  ", strerror(errNum));
    return -1;
  }

  if (endCount < 0 || endCount > 4096) {
    ByteOrder::swap32(&endCount, sizeof(endCount));
  }
  
  if (startCount != endCount) {
    _addErrStr("ERROR - NsslMrdRadxFile::_readRec");
    _addErrStr("  Start and endCount counts do not agree");
    _addErrInt("    startCount count: ", startCount);
    _addErrInt("    endCount count: ", endCount);
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////
// Load up headers from input buffer
// Returns 0 on success, -1 on failure

int NsslMrdRadxFile::_loadHeaders()
  
{

  mrd_header_t hdr;
  mrd_header2_t hdr2;

  // check size

  if (_inBufSize != sizeof(hdr) + sizeof(hdr2)) {
    return -1;
  }

  // copy to headers
  
  memcpy(&hdr, (char *) _inBuf.getPtr(), sizeof(hdr));
  memcpy(&hdr2, (char *) _inBuf.getPtr() + sizeof(hdr), sizeof(hdr2));

  // swap if needed

  _swap(hdr);
  _swap(hdr2);
  
  // check headers for consistency

  if (hdr2.year < 1970 || hdr2.year > 2100) {
    return -1;
  }
  if (hdr2.month < 1 || hdr2.month > 12) {
    return -1;
  }
  if (hdr2.day < 1 || hdr2.day > 31) {
    return -1;
  }
  if (hdr2.hour < 0 || hdr2.hour > 23) {
    return -1;
  }
  if (hdr2.minute < 0 || hdr2.minute > 59) {
    return -1;
  }
  if (hdr2.second < 0 || hdr2.second > 59) {
    return -1;
  }

  // save

  _hdr = hdr;
  _hdr2 = hdr2;

  return 0;
  
}

//////////////////////////////////////////////////////
// handle input rays

void NsslMrdRadxFile::_handleRay()
  
{
  
  int nGates = _hdr.num_good_gates;
  // int nFields = 2;

  // create a new Radx ray
  
  RadxRay *ray = new RadxRay();
  
  // set ray meta data
  
  _setRayMetadata(*ray);
  
  // load up field data and print out

  // get field data
  
  RadxArray<Radx::fl32> dbz_, vel_;
  Radx::fl32 *dbz = dbz_.alloc(nGates);
  Radx::fl32 *vel = vel_.alloc(nGates);

  for (int igate = 0; igate < _hdr.num_good_gates; igate++) {
    int ival = _dataRec[igate];
    dbz[igate] = _decodeDbz(ival);
    vel[igate] = _decodeVel(ival);
  }

  // add the DBZ field
  
  RadxField *dbzField = new RadxField("DBZ", "dBZ");
  dbzField->setStandardName("equivalent_reflectivity_factor");
  dbzField->setLongName("reflectivity");
  dbzField->copyRangeGeom(*ray);
  dbzField->setRangeGeom(_startRangeKm, _gateSpacingKm);
  dbzField->addDataFl32(nGates, dbz);
  ray->addField(dbzField);

  // add the VEL field
  
  RadxField *velField = new RadxField("VEL", "m/s");
  velField->setStandardName("radial_velocity_of_scatterers_away_from_instrument");
  velField->setLongName("radial_velocity");
  velField->copyRangeGeom(*ray);
  velField->setRangeGeom(_startRangeKm, _gateSpacingKm);
  velField->addDataFl32(nGates, vel);
  ray->addField(velField);
  
  // add to vector
  
  _readVol->addRay(ray);

}

///////////////////////////
// set volume meta data


void NsslMrdRadxFile::_setVolMetaData(const string &instrumentName)

{

  char text[128];

  _readVol->setOrigFormat("NSSL MRD");

  _readVol->setVolumeNumber(_volumeNumber);
  _readVol->setStartTime(_startTimeSecs, _startNanoSecs);
  _readVol->setEndTime(_endTimeSecs, _endNanoSecs);
  
  _readVol->setInstrumentName(instrumentName);
  if (_readSetRadarNum && _readRadarNum == 1) {
    _readVol->setScanName("ta_surveillance");
  } else {
    _readVol->setScanName("lf_surveillance");
  }
  _readVol->setScanId(0);
  _readVol->setSiteName(Radx::makeString(_hdr.storm_name, 12));

  _readVol->setInstrumentType(Radx::INSTRUMENT_TYPE_RADAR);
  _readVol->setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_TAIL);
  _readVol->setPrimaryAxis(Radx::PRIMARY_AXIS_Y_PRIME);
  _readVol->setTitle("NOAA TAIL RADAR");
  
  sprintf(text, "flight_number=%s",
          Radx::makeString(_hdr.flight_number, 8).c_str());
  _readVol->setSource(text);
  sprintf(text, "aircraft_id=%s",
          Radx::makeString(_hdr.aircraft_id, 2).c_str());
  _readVol->setReferences(text);
  
  _readVol->setHistory("Read in from raw NSSL MRD file");

  string comment;
  comment += RadxXml::writeStartTag("nssl_mrd", 0);
  comment += RadxXml::writeString("flight_number", 1, Radx::makeString(_hdr.flight_number, 8));
  comment += RadxXml::writeString("storm_name", 1, Radx::makeString(_hdr.storm_name, 12));
  comment += RadxXml::writeString("aircraft_id", 1, Radx::makeString(_hdr.aircraft_id, 2));
  comment += RadxXml::writeEndTag("nssl_mrd", 0);
  _readVol->setComment(comment);

  _readVol->setLatitudeDeg(_latitude);
  _readVol->setLongitudeDeg(_longitude);
  _readVol->setAltitudeKm(_altitudeM / 1000.0);
  
  _readVol->addFrequencyHz(_frequencyHz);

  _readVol->setRadarBeamWidthDegH(1.35);
  _readVol->setRadarBeamWidthDegV(1.90);
  _readVol->setRadarAntennaGainDbH(40.0);
  _readVol->setRadarAntennaGainDbV(40.0);

}

/////////////////////////
// set the ray metadata

void NsslMrdRadxFile::_setRayMetadata(RadxRay &ray)
  
{
  
  if (_debug) {
    _print(_hdr, cerr);
    _print(_hdr2, cerr);
  }
  
  double subSecs = 0.0;
  RadxTime rayTime(_hdr2.year, _hdr2.month, _hdr2.day,
                   _hdr2.hour, _hdr2.minute, _hdr2.second, subSecs);
  
  time_t raySecs = rayTime.utime();
  int rayNanoSecs = (int) (subSecs * 1.0e9 + 0.5);
  
  if (_startTimeSecs == 0 && _endTimeSecs == 0) {
    _startTimeSecs = raySecs;
    _startNanoSecs = rayNanoSecs;
  } 
  _endTimeSecs = raySecs;
  _endNanoSecs = rayNanoSecs;

  ray.setTime(raySecs, rayNanoSecs);
  ray.setVolumeNumber(_volumeNumber);

  ray.setPolarizationMode(Radx::POL_MODE_VERTICAL);
  
  ray.setPrtMode(Radx::PRT_MODE_FIXED);
  
  double rotation = _hdr.raw_rot_ang_x10 / 10.0;
  if (rotation < 0) {
    rotation += 360.0;
  }
  ray.setAzimuthDeg(rotation);

  double tilt = _hdr.raw_tilt_x10 / 10.0 - 180.0;
  double elevation = tilt;
  ray.setElevationDeg(elevation);

  int sweepNum = _hdr.sweep_num;
  ray.setSweepNumber(sweepNum);
        
  ray.setSweepMode(Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE);

  ray.setFixedAngleDeg(elevation);

  ray.setTrueScanRateDegPerSec(Radx::missingMetaDouble);
  ray.setTargetScanRateDegPerSec(Radx::missingMetaDouble);
  ray.setIsIndexed(false);
  ray.setAngleResDeg(Radx::missingMetaDouble);
  ray.setNSamples(_hdr.azimuth_samples);
  ray.setPulseWidthUsec(Radx::missingMetaDouble);
  ray.setPrtSec(Radx::missingMetaDouble);
  ray.setPrtRatio(1.0);
  _nyquist = _hdr.nyq_vel_x10 / 10.0;
  ray.setNyquistMps(_nyquist);

  _startRangeKm = _hdr.range_delay / 1000.0;
  _gateSpacingKm = _hdr.gate_length / 1000.0;
  
  double maxRangeKm = _startRangeKm + _hdr.max_gates * _gateSpacingKm;
  ray.setUnambigRangeKm(maxRangeKm);
  
  ray.setRangeGeom(_startRangeKm, _gateSpacingKm);

  ray.setMeasXmitPowerDbmH(Radx::missingMetaDouble);
  ray.setMeasXmitPowerDbmV(Radx::missingMetaDouble);

  // georeference

  RadxGeoref georef;

  georef.setTimeSecs(raySecs);
  georef.setNanoSecs(rayNanoSecs);

  _latitude = _hdr.lat_deg + _hdr.lat_min / 60.0 + _hdr.lat_sec_x10 / 36000.0;
  _longitude = _hdr.lon_deg + _hdr.lon_min / 60.0 + _hdr.lon_sec_x10 / 36000.0;
  _altitudeM = _hdr.altitude;

  georef.setLongitude(_longitude);
  georef.setLatitude(_latitude);
  georef.setAltitudeKmMsl(_altitudeM / 1000.0);

  georef.setRotation(rotation);
  georef.setTilt(tilt);

  double windDirn = _hdr.wind_dir_x10 / 10.0;
  double windSpeed = _hdr.wind_speed_x10 / 10.0;
  double uWind = -1.0 * (sin(windDirn * Radx::DegToRad) * windSpeed);
  double vWind = -1.0 * (cos(windDirn * Radx::DegToRad) * windSpeed);
  georef.setEwWind(uWind);
  georef.setNsWind(vWind);
  georef.setVertWind(Radx::missingMetaDouble);

  double heading = _hdr.heading_x10 / 10.0;
  double gndSpeed = _hdr.ground_speed_x64 / 64.0;
  double uu = sin(heading * Radx::DegToRad) * gndSpeed;
  double vv = cos(heading * Radx::DegToRad) * gndSpeed;
  georef.setEwVelocity(uu);
  georef.setNsVelocity(vv);
  georef.setVertVelocity(_hdr.vert_airspeed_x64 / 64.0);
  
  georef.setHeading(heading);
  georef.setRoll(_hdr.roll_x10 / 10.0);
  georef.setPitch(_hdr.pitch_x10 / 10.0);
  georef.setDrift(_hdr.drift_x10 / 10.0);

  ray.setGeoref(georef);
  
}

////////////////////////////////////////////////////////////////
// byte swap routines
//
// These only activate if _hrdIsSwapped has been set to true.
// Otherwise they simply return.

// swap arrays

void NsslMrdRadxFile::_swap(Radx::si16 *vals, int n)
{
  if (!_mrdIsSwapped) return;
  ByteOrder::swap16(vals, n * sizeof(Radx::si16), true);
}

void NsslMrdRadxFile::_swap(Radx::ui16 *vals, int n)

{
  if (!_mrdIsSwapped) return;
  ByteOrder::swap16(vals, n * sizeof(Radx::ui16), true);
}

void NsslMrdRadxFile::_swap(mrd_header_t &hdr)
  
{
  if (hdr.julian_date < 0 || hdr.julian_date > 367) {
    ByteOrder::swap16(&hdr.word_1, 23 * sizeof(Radx::si16));
    ByteOrder::swap16(&hdr.wind_dir_x10, 11 * sizeof(Radx::si16));
  }
}

void NsslMrdRadxFile::_swap(mrd_header2_t &hdr)
  
{
  if (hdr.year < 1900 || hdr.year > 2100) {
    ByteOrder::swap32(&hdr.year, 40 * sizeof(Radx::si32));
  }
}

////////////////////////////////
// get DBZ value from packed int

double NsslMrdRadxFile::_decodeDbz(int ival) 
{
  
  if (ival == 0) {
    return Radx::missingFl64;
  }
  int idbz = ival >> 10;
  return (double) idbz;

}

////////////////////////////////
// get VEL value from packed int

double NsslMrdRadxFile::_decodeVel(int ival) 
{

  if (ival == 0) {
    return Radx::missingFl64;
  }

  int ivel = ival & 1023;
  return (((double) ivel - 511.0) / 7.5);

}

//////////////////////////////////////////////////////
// main header

void NsslMrdRadxFile::_print(const mrd_header_t &hdr,
                             ostream &out)

{
  
  out << "=========== mrd_header_t ===========" << endl;
  out << "  word_1: " << hdr.word_1 << endl;
  out << "  word_2: " << hdr.word_2 << endl;
  out << "  word_3: " << hdr.word_3 << endl;
  out << "  raw_rot_ang_x10: " << hdr.raw_rot_ang_x10 << endl;
  out << "  rot_ang: " << hdr.raw_rot_ang_x10 / 10.0 << endl;
  out << "  lat_deg: " << hdr.lat_deg << endl;
  out << "  lat_min: " << hdr.lat_min << endl;
  out << "  lat_sec_x10: " << hdr.lat_sec_x10 << endl;
  out << "  latitude: " << hdr.lat_deg + hdr.lat_min / 60.0 + hdr.lat_sec_x10 / 36000.0 << endl;
  out << "  lon_deg: " << hdr.lon_deg << endl;
  out << "  lon_min: " << hdr.lon_min << endl;
  out << "  lon_sec_x10: " << hdr.lon_sec_x10 << endl;
  out << "  longitude: " << hdr.lon_deg + hdr.lon_min / 60.0 + hdr.lon_sec_x10 / 36000.0 << endl;
  out << "  altitude: " << hdr.altitude << endl;
  out << "  roll_x10: " << hdr.roll_x10 << endl;
  out << "  roll: " << hdr.roll_x10 / 10.0 << endl;
  out << "  heading_x10: " << hdr.heading_x10 << endl;
  out << "  heading: " << hdr.heading_x10 / 10.0 << endl;
  out << "  drift_x10: " << hdr.drift_x10 << endl;
  out << "  drift: " << hdr.drift_x10 / 10.0 << endl;
  out << "  pitch_x10: " << hdr.pitch_x10 << endl;
  out << "  pitch: " << hdr.pitch_x10 / 10.0 << endl;
  out << "  raw_tilt_x10: " << hdr.raw_tilt_x10 << endl;
  out << "  tilt: " << hdr.raw_tilt_x10 / 10.0 - 180.0 << endl;
  out << "  nyq_vel_x10: " << hdr.nyq_vel_x10 << endl;
  out << "  nyq_vel: " << hdr.nyq_vel_x10 / 10.0 << endl;
  out << "  julian_date: " << hdr.julian_date << endl;
  out << "  azimuth_samples: " << hdr.azimuth_samples << endl;
  out << "  gate_length: " << hdr.gate_length << endl;
  out << "  range_delay: " << hdr.range_delay << endl;
  out << "  ground_speed_x64: " << hdr.ground_speed_x64 << endl;
  out << "  ground_speed: " << hdr.ground_speed_x64 / 64.0 << endl;
  out << "  vert_airspeed_x64: " << hdr.vert_airspeed_x64 << endl;
  out << "  vert_airspeed: " << hdr.vert_airspeed_x64 / 64.0 << endl;
  out << "  flight_number[8]: " << Radx::makeString(hdr.flight_number, 8) << endl;
  out << "  storm_name[12]: " << Radx::makeString(hdr.storm_name, 12) << endl;
  out << "  wind_dir_x10: " << hdr.wind_dir_x10 << endl;
  out << "  wind_dir: " << hdr.wind_dir_x10 / 10.0 << endl;
  out << "  nav_flag: " << hdr.nav_flag << endl;
  out << "  wind_speed_x10: " << hdr.wind_speed_x10 << endl;
  out << "  wind_speed: " << hdr.wind_speed_x10 / 10.0 << endl;
  out << "  noise_threshold: " << hdr.noise_threshold << endl;
  out << "  corrected_tilt_x10: " << hdr.corrected_tilt_x10 << endl;
  out << "  corrected_tilt: " << hdr.corrected_tilt_x10 / 10.0 << endl;
  out << "  num_good_gates: " << hdr.num_good_gates << endl;
  out << "  gspd_vel_corr_x10: " << hdr.gspd_vel_corr_x10 << endl;
  out << "  gspd_vel_corr: " << hdr.gspd_vel_corr_x10 / 10.0 << endl;
  out << "  sweep_num: " << hdr.sweep_num << endl;
  out << "  max_gates: " << hdr.max_gates << endl;
  out << "  tilt_corr_flag: " << hdr.tilt_corr_flag << endl;
  out << "  altitude_flag: " << hdr.altitude_flag << endl;
  out << "  aircraft_id[2]: " << Radx::makeString(hdr.aircraft_id, 2) << endl;
  out << "====================================" << endl;
  
}

void NsslMrdRadxFile::_print(const mrd_header2_t &hdr,
                             ostream &out)
  
{
  
  out << "=========== mrd_header2_t ===========" << endl;
  out << "  year: " << hdr.year << endl;
  out << "  month: " << hdr.month << endl;
  out << "  day: " << hdr.day << endl;
  out << "  hour: " << hdr.hour << endl;
  out << "  minute: " << hdr.minute << endl;
  out << "  second: " << hdr.second << endl;
  // for (int ii = 0; ii < 34; ii++) {
  //   out << "  spares[" << ii << "]: " << hdr.spares[ii] << endl;
  // }
  out << "======================================" << endl;
  
}

//////////////////////////////////////////////////////
// print rays

void NsslMrdRadxFile::_printRay(bool printData,
                                ostream &out)
  
{

  // print headers
  
  _print(_hdr, out);
  _print(_hdr2, out);
  
  // print data if required
  
  if (printData) {
    
    vector<double> dbz, vel;
    for (int igate = 0; igate < _hdr.num_good_gates; igate++) {
      int ival = _dataRec[igate];
      dbz.push_back(_decodeDbz(ival));
      vel.push_back(_decodeVel(ival));
    }

    _printFieldData("DBZ", dbz, out);
    _printFieldData("VEL", vel, out);

  } // if (printData)

}

//////////////////////////////////////////////////////
// print field data

void NsslMrdRadxFile::_printFieldData(const string &fieldName,
                                      const vector<double> &data,
                                      ostream &out)
  
{
  
  out << "========================================================" << endl;
  out << "Ray data for field: " << fieldName << endl;
  out << "nGates: " << data.size() << endl;
  
  int printed = 0;
  int count = 1;
  double prevVal = data[0];
  for (size_t ii = 1; ii < data.size(); ii++) {
    double val = data[ii];
    if (val != prevVal) {
      _printPacked(out, count, prevVal, Radx::missingFl64);
      printed++;
      if (printed > 7) {
        out << endl;
        printed = 0;
      }
      prevVal = val;
      count = 1;
    } else {
      count++;
    }
  } // ii
  _printPacked(out, count, prevVal, Radx::missingFl64);
  out << endl;
  out << "========================================================" << endl;

}
  
/////////////////////////////////////////////////////////////////
// print in packed format, using count for identical data values

void NsslMrdRadxFile::_printPacked(ostream &out, int count,
                                   double val, double missing)
  
{
  
  char outstr[1024];
  if (count > 1) {
    out << count << "*";
  }
  if (val == missing) {
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

////////////////////////////////////////////////////////////////////////////////
// get instrument name from path

string NsslMrdRadxFile::_getInstrumentNameFromPath(const string path)

{

  RadxPath rpath(path);
  string fileName = rpath.getFile();
  vector<string> parts;
  RadxStr::tokenize(fileName, ".", parts);

  if (parts.size() >= 3) {
    return parts[2];
  } else {
    return "noaa-ta";
  }

}
  
