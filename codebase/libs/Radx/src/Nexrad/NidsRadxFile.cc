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
// NidsRadxFile.cc
//
// NidsRadxFile object
//
// NetCDF file data for radar radial data in FORAY format
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2010
//
///////////////////////////////////////////////////////////////

#include <Radx/NidsRadxFile.hh>
#include <Radx/NexradLoc.hh>
#include <Radx/NcfRadxFile.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxArray.hh>
#include <Radx/ByteOrder.hh>
#include <Radx/RadxRemap.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/RadxStr.hh>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <cerrno>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <bzlib.h>
using namespace std;

//////////////
// Constructor

NidsRadxFile::NidsRadxFile() : RadxFile()
  
{

  _readVol = NULL;
  _file = NULL;
  _isZipped = false;
  _clear();

}

/////////////
// destructor

NidsRadxFile::~NidsRadxFile()

{
  _clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void NidsRadxFile::_clear()
  
{
  
  clearErrStr();
  _close();

  _writeFileNameMode = FILENAME_WITH_START_TIME_ONLY;

  _volNum = Radx::missingMetaInt;
  _vcpNum = Radx::missingMetaInt;

  _radarName = "unknown";
  
  _latitude = Radx::missingMetaDouble;
  _longitude = Radx::missingMetaDouble;
  _altitudeM = Radx::missingMetaDouble;

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

int NidsRadxFile::writeToDir(const RadxVol &vol,
                             const string &dir,
                             bool addDaySubDir,
                             bool addYearSubDir)
  
{
  
  // Writing NIDS files is not supported
  // therefore write in CF Radial format instead
  
  cerr << "WARNING - NidsRadxFile::writeToDir" << endl;
  cerr << "  Writing NIDS format files not supported" << endl;
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
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs
// Use getPathInUse() for path written

int NidsRadxFile::writeToPath(const RadxVol &vol,
                              const string &path)
  
{

  // Writing NIDS files is not supported
  // therefore write in CF Radial format instead

  cerr << "WARNING - NidsRadxFile::writeToPath" << endl;
  cerr << "  Writing NIDS format files not supported" << endl;
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

/////////////////////////////////////////////////////////
// Check if specified file is a NIDS level3 file
// Returns true if supported, false otherwise

bool NidsRadxFile::isSupported(const string &path)

{
  
  if (isNids(path)) {
    return true;
  }
  return false;

}

////////////////////////////////////////////////////////////
// Check if this is a NIDS level3 file
// Returns true on success, false on failure
// Side effect:
//   sets _isZipped if file is internally compressed

bool NidsRadxFile::isNids(const string &path)
  
{

  _clear();
  _isZipped = false;
  
  // open file

  if (_openRead(path)) {
    _addErrStr("ERROR - NidsRadxFile::isNexrad");
    return false;
  }

  // read buffer at start of file
  
  char buf[sizeof(_NIDS_header_t)];
  if (fread(buf, 1, sizeof(_NIDS_header_t), _file) != sizeof(_NIDS_header_t)) {
    _close();
    return false;
  }
  _close();
  
  // check for "001 \r \r \n ? ? ? \r \r \n"
  // that indicates the file is compressed

  if (buf[0] == 0x01 &&
      buf[1] == '\r' &&
      buf[2] == '\r' &&
      buf[3] == '\n' &&
      buf[8] == '\r' &&
      buf[9] == '\r' &&
      buf[10] == '\n') {
    // NIDS compressed
    _isZipped = true;
    return true;
  }

  // swap the header

  _NIDS_header_t hdr;
  memcpy(&hdr, buf, sizeof(hdr));
  _NIDS_BE_to_mess_header(&hdr);

  // check for file size

  struct stat fstat;
  if (stat(path.c_str(), &fstat)) {
    _addErrStr("ERROR - NidsRadxFile::isNexrad");
    return false;
  }

  if (hdr.mlength != fstat.st_size) {
    // incorrect length for a NIDS file
    return false;
  }

  return true;

}

////////////////////////////////////////////////////////////
// Read in data from specified path, load up volume object.
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs

int NidsRadxFile::readFromPath(const string &path,
                               RadxVol &vol)
  
{

  _clear();
  _readVol = &vol;
  _readVol->clear();
  _pathInUse = path;
  vol.setPathInUse(_pathInUse);
  _readPaths.clear();

  // perform the read

  if (_doRead(path, cerr, false)) {
    _addErrStr("ERROR - NidsRadxFile::readFromPath");
    _addErrStr("  Path: ", _pathInUse);
    return -1;
  }

  // add rays to the vol
  
  if (_addRays()) {
    _addErrStr("ERROR - NidsRadxFile::readFromPath");
    _addErrStr("  Path: ", _pathInUse);
    return -1;
  }

  // check we got some data
  
  if (_readVol->getRays().size() < 1) {
    _addErrStr("ERROR - NidsRadxFile::readFromPath");
    _addErrStr("  No valid rays found");
    return -1;
  }
  
  // remove unwanted fields
  
  if (_readFieldNames.size() > 0) {
    _removeUnwantedFields();
  }
  
  // finalize the read volume
  
  if (_finalizeReadVolume()) {
    return -1;
  }

  // add to paths used on read

  _readPaths.push_back(path);

  // set the packing from the rays

  _readVol->setPackingFromRays();
  
  // set format as read

  _fileFormat = FILE_FORMAT_NEXRAD_NIDS3;

  if (_debug) {
    _readVol->print(cerr);
  }
  
  return 0;

}

//////////////////////////////////////
// open netcdf file for reading
// Returns 0 on success, -1 on failure

int NidsRadxFile::_openRead(const string &path)
  
{

  _close();
  _file = fopen(path.c_str(), "r");
  if (!_file) {
    int errNum = errno;
    _addErrStr("ERROR - NidsRadxFile::_openRead");
    _addErrStr("  Cannot open file for reading, path: ", path);
    _addErrStr("  ", strerror(errNum));
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////
// close netcdf file if open
// remove error object if it exists

void NidsRadxFile::_close()
  
{
  
  // close file if open, delete ncFile
  
  if (_file) {
    fclose(_file);
    _file = NULL;
  }

}

////////////////////////////////////////////////////////////
// Perform read
// Returns 0 on success, -1 on failure

int NidsRadxFile::_doRead(const string &path,
                          ostream &out,
                          bool printNative)
  
{
  
  if (!isNids(path)) {
    _addErrStr("ERROR - NidsRadxFile::_doRead");
    _addErrStr("  Not a NIDS 3 file");
    return -1;
  }

  // get file size
  
  struct stat fstat;
  if (stat(path.c_str(), &fstat)) {
    int errNum = errno;
    _addErrStr("ERROR - NidsRadxFile::_doRead");
    _addErrStr(strerror(errNum));
    return false;
  }
  
  // open file
  
  if (_openRead(_pathInUse)) {
    _addErrStr("ERROR - NidsRadxFile::_doRead");
    return -1;
  }

  // read into buffer

  RadxBuf readBuf;
  readBuf.reserve(fstat.st_size);
  if ((int) fread(readBuf.getPtr(), 1, fstat.st_size, _file) != fstat.st_size) {
    _addErrStr("ERROR - NidsRadxFile::_doRead");
    _addErrStr("  Cannot read file into buffer");
    _close();
    return -1;
  }
  _close();
  
  _dataBuf.clear();
  if (_isZipped) {
    // unzip the data
    if (_NIDS_uncompress(readBuf, _dataBuf, _radarName)) {
      _addErrStr("ERROR - NidsRadxFile::_doRead");
      _addErrStr("  Cannot uncompress buffer");
      _close();
      return -1;
    }
  } else {
    // copy from read buf
    _dataBuf = readBuf;
  }
  
#ifdef DEBUGGING  
  // write tmp file
  
  FILE *tmp;
  if ((tmp = fopen("/tmp/nids.uncompressed", "w")) == NULL) {
    cerr << "Cannot open tmp file" << endl;
    return -1;
  }
  if (fwrite(_dataBuf.getPtr(), 1, _dataBuf.getLen(), tmp) != _dataBuf.getLen()) {
    cerr << "Cannot write tmp file" << endl;
    fclose(tmp);
    return -1;
  }
  cerr << "-->> Wrote file /tmp/nids.uncompressed" << endl;
  fclose(tmp);
#endif
  
  // load NIDS header
  
  memcpy(&_nhdr, _dataBuf.getPtr(), sizeof(_nhdr));
  _NIDS_BE_to_mess_header(&_nhdr);

  if (_verbose) {
    _NIDS_print_mess_hdr(stderr, "", &_nhdr);
  } else if (printNative) {
    if (&out == &cout) {
      _NIDS_print_mess_hdr(stdout, "", &_nhdr);
    } else {
      _NIDS_print_mess_hdr(stderr, "", &_nhdr);
    }
  }
  
  _volNum = _nhdr.seqnum;
  _sweepNum = _nhdr.elevnum;
  _elevAngle = _nhdr.pd[0] / 10.0;
  _scanId = _nhdr.vscan;
  _vcpNum = _nhdr.vcp;
  
  // check size
  
  if (_dataBuf.getLen() < sizeof(_nhdr) + _nhdr.lendat) {
    _addErrStr("ERROR - NidsRadxFile::_doRead");
    _addErrInt("  Data buffer len too short: ", _dataBuf.getLen());
    _addErrInt("  Min required: ", sizeof(_nhdr) + _nhdr.lendat);
    return -1;
  }
  
  // set data ptr
  
  _dataPtr= ((Radx::ui08 *) _dataBuf.getPtr()) + sizeof(_nhdr);
  
  // check product format

  if (_dataPtr[0] != 0xaf || _dataPtr[1] != 0x1f ) {
    _addErrStr("ERROR - NidsRadxFile::_doRead");
    _addErrStr("  File not in radial format");
    return -1;
  }

  // set output val array for decoding to floats
  
  _setOutputVals();

  // read radial header
  
  memcpy(&_rhdr, _dataPtr, sizeof(_NIDS_radial_header_t));
  _dataPtr += sizeof(_NIDS_radial_header_t);
  _NIDS_BE_to_radial_header(&_rhdr);

  if (_verbose) {
    _NIDS_print_radial_hdr(stderr, "", &_rhdr);
  } else if (printNative) {
    if (&out == &cout) {
      _NIDS_print_radial_hdr(stdout, "", &_rhdr);
    } else {
      _NIDS_print_radial_hdr(stderr, "", &_rhdr);
    }
  }
  
  _nGates = _rhdr.num_r_bin;
  _gateSpacing = 1.0;
  _startRange = (_rhdr.first_r_bin + 0.5) * _gateSpacing;

  if (_verbose) {
    cerr << "  elevAngle: " << _elevAngle << endl;
    cerr << "  ngates: " << _nGates << endl;
    cerr << "  gateSpacing: " << _gateSpacing << endl;
    cerr << "  startRange: " << _startRange << endl;
  } else if (printNative) {
    out << "  elevAngle: " << _elevAngle << endl;
    out << "  ngates: " << _nGates << endl;
    out << "  gateSpacing: " << _gateSpacing << endl;
    out << "  startRange: " << _startRange << endl;
  }
  
  // set volume metadata
  
  _scanTime = _nhdr.vsdate * 86400 + _nhdr.vstime * 65536 + _nhdr.vstim2;
  _latitude = _nhdr.lat * 0.001;
  _longitude = _nhdr.lon * 0.001;
  _altitudeM = _nhdr.height * 0.3048;
  
  return 0;

}
  
/////////////////////////////////////////////////////////////
// remove unwanted fields from rays

void NidsRadxFile::_removeUnwantedFields()
  
{

  // load up vector of rays with trimmed field list

  vector<RadxRay *> goodRays, badRays;
  vector<RadxRay *> volRays = _readVol->getRays();

  for (size_t iray = 0; iray < volRays.size(); iray++) {
    
    RadxRay *ray = volRays[iray];
    badRays.push_back(ray);

    RadxRay *good = new RadxRay;
    goodRays.push_back(good);

    // copy over the metadata only
    
    good->copyMetaData(*ray);

    // add fields are requested

    for (size_t jj = 0; jj < _readFieldNames.size(); jj++) {
      string readField = _readFieldNames[jj];
      for (size_t kk = 0; kk < ray->getNFields(); kk++) {
        string rayField = ray->getFields()[kk]->getName();
        if (rayField == readField) {
          RadxField *field = new RadxField(*ray->getFields()[kk]);
          good->addField(field);
        }
      } // kk
    } // jj

  } // iray

  // replace original rays with new ones

  _readVol->removeBadRays(goodRays, badRays);

}

/////////////////////////////////////////////////////////////
// finalize the read volume

int NidsRadxFile::_finalizeReadVolume()
  
{

  _readVol->setStartTime(_scanTime, 0);
  _readVol->setEndTime(_scanTime, 0);
  
  _readVol->setLatitudeDeg(_latitude);
  _readVol->setLongitudeDeg(_longitude);
  _readVol->setAltitudeKm(_altitudeM / 1000.0);
  
  _readVol->setScanId(_scanId);
  
  char vcpStr[128];
  sprintf(vcpStr, "vcp-%d", _vcpNum);
  _readVol->setScanName(vcpStr);

  _readVol->setVolumeNumber(_volNum);
  _readVol->setInstrumentType(Radx::INSTRUMENT_TYPE_RADAR);
  _readVol->setPlatformType(Radx::PLATFORM_TYPE_FIXED);
  
  _readVol->addFrequencyHz(2.85 * 1.0e9);

  _readVol->setRadarAntennaGainDbH(45.8);
  _readVol->setRadarAntennaGainDbV(45.8);
  _readVol->setRadarBeamWidthDegH(0.92);
  _readVol->setRadarBeamWidthDegV(0.92);
  
  _readVol->setTitle("");
  _readVol->setSource("NIDS 3 data");
  _readVol->setScanName("Surveillance");
  _readVol->setInstrumentName(_radarName);
  _readVol->setSiteName(_radarName);
  
  // set max range

  if (_readSetMaxRange) {
    _readVol->setMaxRangeKm(_readMaxRangeKm);
  }
  
  // remove rays with all missing data, if requested

  if (_readRemoveRaysAllMissing) {
    _readVol->removeRaysWithDataAllMissing();
  }

  // load the sweep information from the rays
  
  _readVol->loadSweepInfoFromRays();
  
  // constrain the sweep data as appropriate
  
  if (_readFixedAngleLimitsSet) {
    if (_readVol->constrainByFixedAngle(_readMinFixedAngle, _readMaxFixedAngle,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - NidsRadxFile::_finalizeReadVolume");
      _addErrStr("  No data found within fixed angle limits");
      _addErrDbl("  min fixed angle: ", _readMinFixedAngle);
      _addErrDbl("  max fixed angle: ", _readMaxFixedAngle);
      return -1;
    }
  } else if (_readSweepNumLimitsSet) {
    if (_readVol->constrainBySweepNum(_readMinSweepNum, _readMaxSweepNum,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - NidsRadxFile::_finalizeReadVolume");
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
// print summary after read

void NidsRadxFile::print(ostream &out) const
  
{
  
  out << "=============== NidsRadxFile ===============" << endl;
  RadxFile::print(out);
  out << "  scanTime: " << RadxTime::strm(_scanTime) << endl;
  out << "  volumeNumber: " << _volNum << endl;
  out << "  radarName: " << _radarName << endl;
  out << "  latitude: " << _latitude << endl;
  out << "  longitude: " << _longitude << endl;
  out << "  altitudeM: " << _altitudeM << endl;
  out << "===========================================" << endl;

}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// Print native data in uf file
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int NidsRadxFile::printNative(const string &path, ostream &out,
                              bool printRays, bool printData)
  
{
  
  _clear();
  _pathInUse = path;

  if (_doRead(path, out, true)) {
    _addErrStr("ERROR - NidsRadxFile::printNative");
    _addErrStr("  Path: ", _pathInUse);
    return -1;
  }
  
  // print rays
  
  if (printRays) {
    if (_printRays(out, printData)) {
      _addErrStr("ERROR - NidsRadxFile::printNative");
      _addErrStr("  Path: ", _pathInUse);
      return -1;
    }
  }

  _fileFormat = FILE_FORMAT_NEXRAD_NIDS3;

  return 0;

}

//////////////////////////////////////////////////////
// set the output values, to decode to floats

void NidsRadxFile::_setOutputVals()

{

  for (int ii = 1; ii < 16; ii++ ) {
    Radx::ui08 msbyte, lsbyte;
    msbyte = (_nhdr.pd[ii+1] & 0xff00) >> 8;
    lsbyte = (_nhdr.pd[ii+1] & 0x00ff);
    _outputVals[ii] = lsbyte;
    if (msbyte & 0x01) {
      _outputVals[ii] *= -1.0;
    }
    if (msbyte & 0x10) {
      _outputVals[ii] /= 10.0;
    }
    if (_verbose) {
      cerr << "  bin ii, data val: " << ii << ", " << _outputVals[ii] << endl;
    }
  }

}

///////////////////////////////////////////////////////
// Decode rays, add to volume

int NidsRadxFile::_addRays()
  
{

  for (int irad = 0; irad < _rhdr.num_radials; irad++) {
    
    // read in a radial
    
    _NIDS_beam_header_t bhdr;
    memcpy(&bhdr, _dataPtr, sizeof(_NIDS_beam_header_t));
    _dataPtr += sizeof(_NIDS_beam_header_t);
    _NIDS_BE_to_beam_header(&bhdr);
    if (_verbose) {
      _NIDS_print_beam_hdr(stderr, "", &bhdr);
    }
    
    double az =
      bhdr.radial_start_angle / 10.0 + bhdr.radial_delta_angle / 20.0;

    RadxRay *ray = new RadxRay();
    ray->setVolumeNumber(_volNum);
    ray->setSweepNumber(_sweepNum);
    ray->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
    ray->setTime(_scanTime, 0);
    ray->setAzimuthDeg(az);
    ray->setElevationDeg(_elevAngle);
    ray->setFixedAngleDeg(_elevAngle);
    ray->setIsIndexed(true);
    ray->setAngleResDeg(bhdr.radial_delta_angle / 10.0);
    ray->setRangeGeom(_startRange, _gateSpacing);
    
    // create field

    int fcode = _nhdr.mcode;
    string fieldName, fieldUnits;
    string standardName, longName;
    if (fcode >= 16 && fcode <= 21) {
      // base reflectivity
      fieldName = "REF";
      fieldUnits = "dBZ";
      standardName = "equivalent_reflectivity_factor";
      longName = "reflectivity";
    } else if (fcode >= 2 && fcode <= 27) {
      // velocity
      fieldName = "VEL";
      fieldUnits = "m/s";
      standardName = "radial_velocity_of_scatterers_away_from_instrument";
      longName = "radial velocity";
    } else if (fcode >= 28 && fcode <= 30) {
      // spectrum width
      fieldName = "SW";
      fieldUnits = "m/s";
      standardName = "doppler_spectrum_width";
      longName = "spectrum width";
    } else if (fcode >= 55 && fcode <= 56) {
      // storm relative velocity
      fieldName = "VEL_REL";
      fieldUnits = "m/s";
      standardName = "radial_velocity_of_scatterers_away_from_instrument";
      longName = "storm-relative radial velocity";
    }

    RadxField *field = new RadxField(fieldName, fieldUnits);
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->setRangeGeom(_startRange, _gateSpacing);

    RadxArray<Radx::fl32> data_;
    Radx::fl32 *data = data_.alloc(_nGates);
    Radx::fl32 missing = Radx::missingFl32;

    // Run Length decode this radial

    int nbytes_run = bhdr.num_halfwords * 2;
    int nbins = 0;
    int gateNum = 0;
    for (int run = 0; run < nbytes_run; run++ ) {
      int drun = *_dataPtr >> 4;
      int dcode = *_dataPtr & 0xf;
      nbins += drun;
      if (nbins > _rhdr.num_r_bin) {
	_addErrStr("ERROR - NidsRadxFile::_addRays");
	_addErrStr("Bad gate count");
	return -1;
      }
      Radx::fl32 val = missing;
      if (dcode != 0) {
        val = _outputVals[dcode];
      }
      for (int ii = 0; ii < drun; ii++) {
        data[gateNum] = val;
        gateNum++;
      }
      _dataPtr++;
    } // run
    
    // add data to field

    field->addDataFl32(gateNum, data);

    // convert to shorts

    field->convertToSi16();

    // add field to ray

    ray->addField(field);

    // add ray to volume

    _readVol->addRay(ray);

  } // irad

  return 0;

}

///////////////////////////////////////////////////////
// Print rays

int NidsRadxFile::_printRays(ostream &out, bool printData)
  
{

  for (int irad = 0; irad < _rhdr.num_radials; irad++) {
    
    // read in a radial
    
    _NIDS_beam_header_t bhdr;
    memcpy(&bhdr, _dataPtr, sizeof(_NIDS_beam_header_t));
    _dataPtr += sizeof(_NIDS_beam_header_t);
    _NIDS_BE_to_beam_header(&bhdr);
    if (&out == &cout) {
      _NIDS_print_beam_hdr(stdout, "", &bhdr);
    } else {
      _NIDS_print_beam_hdr(stderr, "", &bhdr);
    }
    
    double az =
      bhdr.radial_start_angle / 10.0 + bhdr.radial_delta_angle / 20.0;

    out << "az: " << az << endl;
    if (printData) {
      out << "data: ";
    }

    // Run Length decode this radial
    
    RadxArray<Radx::fl32> data_;
    Radx::fl32 *data = data_.alloc(_nGates);
    Radx::fl32 missing = Radx::missingFl32;

    int nbytes_run = bhdr.num_halfwords * 2;
    int nbins = 0;
    int gateNum = 0;
    for (int run = 0; run < nbytes_run; run++ ) {
      int drun = *_dataPtr >> 4;
      int dcode = *_dataPtr & 0xf;
      nbins += drun;
      if (nbins > _rhdr.num_r_bin) {
	_addErrStr("ERROR - NidsRadxFile::_printRays");
	_addErrStr("Bad gate count");
	return -1;
      }
      Radx::fl32 val = missing;
      if (dcode != 0) {
        val = _outputVals[dcode];
      }
      for (int ii = 0; ii < drun; ii++) {
        data[gateNum] = val;
        gateNum++;
        if (printData) {
          out << " " << dcode << ":" << val;
        }
      }
      _dataPtr++;
    } // run

    if (printData) {
      out << endl;
    }
    
  } // irad

  return 0;

}

