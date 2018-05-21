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
// HrdRadxFile.cc
//
// HrdRadxFile object
//
// Support for radial data in HRD format
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2011
//
///////////////////////////////////////////////////////////////

#include <Radx/HrdRadxFile.hh>
#include <Radx/NcfRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/ByteOrder.hh>
#include <iomanip>
#include <cstdio>
using namespace std;

const double HrdRadxFile::_angleConversion = 360.0 / 65536.0;
int HrdRadxFile::_volumeNumber = 0;
 
//////////////
// Constructor

HrdRadxFile::HrdRadxFile() : RadxFile()
                                   
{

  _readVol = NULL;
  _file = NULL;
  clear();

}

/////////////
// destructor

HrdRadxFile::~HrdRadxFile()

{
  clear();
}

/////////////////////////////////////////////////////////
// Check if specified file is HRD format
// Returns true if supported, false otherwise

bool HrdRadxFile::isSupported(const string &path)

{

  if (isHrd(path)) {
    return true;
  }

  return false;

}

////////////////////////////////////////////////////////////
// Check if this is an HRD file
// Returns true on success, false on failure

bool HrdRadxFile::isHrd(const string &path)
  
{

  clear();
  
  // open file
  
  if (_openRead(path)) {
    _addErrStr("ERROR - HrdRadxFile::isHrd");
    return false;
  }
  
  // read first 64 bytes

  Radx::si16 buf[64];
  if (fread(buf, sizeof(Radx::si16), 64, _file) != 64) {
    _close();
    return false;
  }
  _close();
  
  _hrdIsSwapped = false;
  if (buf[0] == 0 &&  // tape header flag
      buf[1] == 8 && 
      buf[3] == 8) {
    _hrdIsSwapped = true;
    ByteOrder::swap16(buf, 64 * sizeof(Radx::si16), true);
  } 
  if (buf[0] == 2048 &&  // tape header flag
      buf[1] == 0 && 
      buf[3] == 8) {
    _hrdIsSwapped = true;
    ByteOrder::swap16(buf, 64 * sizeof(Radx::si16), true);
  }
  
  // check selected bytes
  
  if (buf[0] == 0 &&  // tape header flag
      buf[1] == 2048 &&
      buf[3] == 2048) {
    return true;
  }
  if (buf[0] == 8 &&  // ??
      buf[1] == 0 &&
      buf[3] == 2048) {
    return true;
  }

  return false;

}

/////////////////////////////////////////////////////////
// clear the data in the object

void HrdRadxFile::clear()
  
{

  clearErrStr();

  _close();

  _hrdIsSwapped = false;
  _isTailRadar = true;
  _dataLen = 0;

  _startTimeSecs = 0;
  _endTimeSecs = 0;
  _startNanoSecs = 0;
  _endNanoSecs = 0;

  _nGates = 0;
  _nSamples = 0;
  _nyquist = 0.0;

  _pulseWidthUs = 0.0;
  _wavelengthM = 0.0;
  _prf = 0.0;
  _prt0 = 0.0;
  _prt1 = 0.0;
  _nyquist = 0.0;

  _staggered = false;
  _staggerRatio = 0.0;
  
  _gateSpacingKm = 0.0;
  _startRangeKm = 0.0;

  _latitude = Radx::missingMetaDouble;
  _longitude = Radx::missingMetaDouble;
  _altitudeM = Radx::missingMetaDouble;

  _prevSweepNum = -1;
  _prevElev = -9999;

}

/////////////////////////////////////////////////////////
// Write data from volume to specified directory
// Writes as CFRadial, HRD not supported on write
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs
// Use getPathInUse() for path written

int HrdRadxFile::writeToDir(const RadxVol &vol,
                            const string &dir,
                            bool addDaySubDir,
                            bool addYearSubDir)
  
{

  // Writing HRD files is not supported
  // therefore write in CF Radial format instead

  cerr << "WARNING - HrdRadxFile::writeToDir" << endl;
  cerr << "  Writing HRD format files not supported" << endl;
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
// Writes as CFRadial, HRD not supported on write
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs
// Use getPathInUse() for path written

int HrdRadxFile::writeToPath(const RadxVol &vol,
                             const string &path)
  
{

  // Writing HRD files is not supported
  // therefore write in CF Radial format instead

  cerr << "WARNING - HrdRadxFile::writeToPath" << endl;
  cerr << "  Writing HRD format files not supported" << endl;
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

////////////////////////////////////////////////////////////
// Read in data from specified path, load up volume object.
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs

int HrdRadxFile::readFromPath(const string &path,
                              RadxVol &vol)
  
{

  _initForRead(path, vol);

  if (!isHrd(path)) {
    _addErrStr("ERROR - HrdRadxFile::readFromPath");
    _addErrStr("  Not a recognized HRD file");
    return -1;
  }
  
  // open file

  if (_openRead(path)) {
    _addErrStr("ERROR - HrdRadxFile::readFromPath");
    return -1;
  }
  
  // set volume number - increments per file

  _volumeNumber++;
  
  // loop through records

  while (!feof(_file)) {
    
    if (_readRec()) {
      if (feof(_file) || _inBufSize == 0) {
        clearErrStr();
        _close();
        break;
      } else {
        _addErrStr("ERROR - HrdRadxFile::readFromPath");
        _close();
        return -1;
      }
    }

    // get record type

    int recType = _getRecType();

    if (recType == 0) {

      // load in header record
      
      _loadHeaderRec();

      if (_verbose) {
        _print(_hdr, cerr);
      }

    } else if (recType == 1) {

      // read in data record
      
      _loadDataRec();

      if (_verbose) {
        _print(_dataRecHdr, cerr);
      }

      // handle the ray data

      _handleRays();

    } else {

      cerr << "WARNING - HrdRadxFile::readFromPath" << endl;
      cerr << "  Bad rec type: " << recType << endl;
      cerr << "  File offset: " << ftell(_file) << endl;
      
    }

  }

  // close input file

  _close();

  if (_readVol->getNRays() == 0) {
    _addErrStr("ERROR - HrdRadxFile::readFromPath");
    _addErrStr("  No rays found, file: ", _pathInUse);
    return -1;
  }
  
  // set the meta data on the volume

  _setVolMetaData();
  
  // remove rays with all missing data, if requested
  
  if (_readRemoveRaysAllMissing) {
    _readVol->removeRaysWithDataAllMissing();
  }

  // apply goeref info if applicable

  if (_readApplyGeorefs) {
    _readVol->applyGeorefs();
  }

  // load the sweep information from the rays

  _readVol->loadSweepInfoFromRays();
  
  // constrain the sweep data as appropriate
  
  if (_readFixedAngleLimitsSet) {
    if (_readVol->constrainByFixedAngle(_readMinFixedAngle, _readMaxFixedAngle,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - HrdRadxFile::readFromPath");
      _addErrStr("  File: ", _pathInUse);
      _addErrStr("  No data found within fixed angle limits");
      _addErrDbl("  min fixed angle: ", _readMinFixedAngle);
      _addErrDbl("  max fixed angle: ", _readMaxFixedAngle);
      return -1;
    }
  } else if (_readSweepNumLimitsSet) {
    if (_readVol->constrainBySweepNum(_readMinSweepNum, _readMaxSweepNum,
                                      _readStrictAngleLimits)) {
      _addErrStr("ERROR - HrdRadxFile::readFromPath");
      _addErrStr("  File: ", _pathInUse);
      _addErrStr("  No data found within sweep num limits");
      _addErrInt("  min sweep num: ", _readMinSweepNum);
      _addErrInt("  max sweep num: ", _readMaxSweepNum);
      return -1;
    }
  }

  // check for indexed rays, set info on rays
  
  _readVol->checkForIndexedRays();
  
  // set the packing from the rays
  
  _readVol->setPackingFromRays();

  if (_debug) {
    _readVol->print(cerr);
  }
  
  // add to paths used on read
  
  _readPaths.push_back(path);

  // set internal format

  _fileFormat = FILE_FORMAT_HRD;

  return 0;

}

//////////////////////////////////////
// open netcdf file for reading
// Returns 0 on success, -1 on failure

int HrdRadxFile::_openRead(const string &path)
  
{

  _close();
  _file = fopen(path.c_str(), "r");
  
  // Check that constructor succeeded
  
  if (_file == NULL) {
    int errNum = errno;
    _addErrStr("ERROR - HrdRadxFile::_openRead");
    _addErrStr("  Cannot open file for reading, path: ", path);
    _addErrStr("  ", strerror(errNum));
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////
// close netcdf file if open
// remove error object if it exists

void HrdRadxFile::_close()
  
{
  
  // close file if open
  
  if (_file) {
    fclose(_file);
    _file = NULL;
  }
  
}

/////////////////////////////////////////////////////////
// print summary after read

void HrdRadxFile::print(ostream &out) const
  
{
  
  out << "=============== HrdRadxFile ===============" << endl;
  RadxFile::print(out);
  out << "===========================================" << endl;

}

////////////////////////////////////////////////////////////
// Print native data in uf file
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int HrdRadxFile::printNative(const string &path, ostream &out,
                             bool printRays, bool printData)
  
{

  clear();
  RadxVol vol;
  _readVol = &vol;
  _readVol->clear();
  _pathInUse = path;
  vol.setPathInUse(_pathInUse);
  _readPaths.clear();

  if (!isHrd(path)) {
    _addErrStr("ERROR - HrdRadxFile::printNative");
    _addErrStr("  Not a recognized HRD file");
    return -1;
  }
  
  // open file

  if (_openRead(path)) {
    _addErrStr("ERROR - HrdRadxFile::printNative");
    return -1;
  }

  // loop through records

  while (!feof(_file)) {

    if (_readRec()) {
      if (feof(_file) || _inBufSize == 0) {
        clearErrStr();
        _close();
        return 0;
      } else {
        _addErrStr("ERROR - HrdRadxFile::printNative");
        _close();
        return -1;
      }
    }

    // get record type

    int recType = _getRecType();

    if (recType == 0) {

      // load in header record
      
      _loadHeaderRec();

      // print main header
      
      _print(_hdr, out);

    } else if (recType == 1) {

      // read in data record
      
      _loadDataRec();

      // print data record header
      
      _print(_dataRecHdr, out);

      // print the ray data

      if (printRays) {
        _printRays(printData, out);
      }

    } else {

      cerr << "WARNING - HrdRadxFile::printNative" << endl;
      cerr << "  Bad rec type: " << recType << endl;
      cerr << "  File offset: " << ftell(_file) << endl;
      
    }

  }

  // close and return

  _close();
  return 0;

}

///////////////////////////
// set volume meta data

void HrdRadxFile::_setVolMetaData()

{

  char text[128];

  _readVol->setOrigFormat("HRD");

  _readVol->setVolumeNumber(_volumeNumber);
  _readVol->setStartTime(_startTimeSecs, _startNanoSecs);
  _readVol->setEndTime(_endTimeSecs, _endNanoSecs);
  
  if (_readSetRadarNum && _readRadarNum == 1) {
    _readVol->setInstrumentName("HRDLF");
    _readVol->setScanName("surveillance");
  } else {
    if (_hdr.aircraft_id > 0) {
      sprintf(text, "HRDT%dP3", _hdr.aircraft_id);
    } else {
      sprintf(text, "HRDTP3");
    }
    _readVol->setInstrumentName(text);
    _readVol->setScanName("tail_surveillance");
  }
  _readVol->setScanId(0);
  _readVol->setSiteName(Radx::makeString(_hdr.project_id, 8));

  if (_readSetRadarNum && _readRadarNum == 1) {
    _readVol->setInstrumentType(Radx::INSTRUMENT_TYPE_RADAR);
    _readVol->setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_BELLY);
    _readVol->setPrimaryAxis(Radx::PRIMARY_AXIS_Z);
    _readVol->setTitle("NOAA LOWER FUSELAGE_RADAR");
  } else {
    _readVol->setInstrumentType(Radx::INSTRUMENT_TYPE_RADAR);
    _readVol->setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_TAIL);
    _readVol->setPrimaryAxis(Radx::PRIMARY_AXIS_Y_PRIME);
    _readVol->setTitle("NOAA TAIL RADAR");
  }
  
  sprintf(text, "flight_id=%s",
          Radx::makeString(_hdr.flight_id, 8).c_str());
  _readVol->setSource(text);
  sprintf(text, "aircraft_id=%d", _hdr.aircraft_id);
  _readVol->setReferences(text);
  
  _readVol->setHistory("Read in from raw HRD file");

  _readVol->setLatitudeDeg(_latitude);
  _readVol->setLongitudeDeg(_longitude);
  _readVol->setAltitudeKm(_altitudeM / 1000.0);

  _readVol->addWavelengthM(_wavelengthM);

  if (_readSetRadarNum && _readRadarNum == 1) {
    _readVol->setRadarBeamWidthDegH(1.1);
    _readVol->setRadarBeamWidthDegV(4.1);
    _readVol->setRadarAntennaGainDbH(35.0);
    _readVol->setRadarAntennaGainDbV(35.0);
  } else {
    _readVol->setRadarBeamWidthDegH(1.35);
    _readVol->setRadarBeamWidthDegV(1.90);
    _readVol->setRadarAntennaGainDbH(40.0);
    _readVol->setRadarAntennaGainDbV(40.0);
  }

}

////////////////////////////////////////////////////////////////
// byte swap routines
//
// These only activate if _hrdIsSwapped has been set to true.
// Otherwise they simply return.

// swap main header

void HrdRadxFile::_swap(hrd_header_t &hdr)
  
{
  
  if (!_hrdIsSwapped) return;
  ByteOrder::swap16(&hdr.header_flag, 11 * sizeof(Radx::si16));
  ByteOrder::swap16(&hdr.word_36, 4 * sizeof(Radx::si16));
  ByteOrder::swap16(&hdr.data_header_len, 37 * sizeof(Radx::si16));
  ByteOrder::swap16(hdr.words_85_to_100, 16 * sizeof(Radx::si16));
  _swap(hdr.radar_lf);
  _swap(hdr.radar_ta);
  
}

// swap radar info header

void HrdRadxFile::_swap(hrd_radar_info_t &radar)

{
  if (!_hrdIsSwapped) return;
  ByteOrder::swap16(&radar, sizeof(radar));
}

// swap data rec header

void HrdRadxFile::_swap(hrd_data_rec_header_t &rec)

{
  if (!_hrdIsSwapped) return;
  ByteOrder::swap16(&rec, sizeof(rec) - 2);
}

// swap ray header

void HrdRadxFile::_swap(hrd_ray_header_t &ray)
  
{
  if (!_hrdIsSwapped) return;
  ByteOrder::swap16(&ray.sizeof_ray, 1 * sizeof(Radx::ui16));
  ByteOrder::swap16(&ray.minute, 18 * sizeof(Radx::si16));
}

// swap arrays

void HrdRadxFile::_swap(Radx::si16 *vals, int n)
{
  if (!_hrdIsSwapped) return;
  ByteOrder::swap16(vals, n * sizeof(Radx::si16), true);
}

void HrdRadxFile::_swap(Radx::ui16 *vals, int n)

{
  if (!_hrdIsSwapped) return;
  ByteOrder::swap16(vals, n * sizeof(Radx::ui16), true);
}

/////////////////////////////////////
// read a record
// store data in buffer

int HrdRadxFile::_readRec()
  
{
  
  // read 4 byte start block
  
  Radx::si32 startCount;
  if (fread(&startCount, sizeof(startCount), 1, _file) != 1) {
    int errNum = errno;
    _addErrStr("ERROR - HrdRadxFile::_readRec");
    _addErrStr("  Reading startCount block");
    _addErrStr("  ", strerror(errNum));
    return -1;
  }
  
  if (_hrdIsSwapped && (startCount < 0 || startCount > 65536)) {
    ByteOrder::swap32(&startCount, sizeof(startCount));
  }
  _inBufSize = startCount;
  
  if (_inBufSize == 0) {
    // end of file
    return -1;
  }

  if (_inBufSize < 1 || _inBufSize > 10000000) {
    _addErrStr("ERROR - HrdRadxFile::_readRec");
    _addErrInt("  Bad decoded buf size: ", _inBufSize);
    return -1;
  }

  // read in record

  char *buf = (char *) _inBuf.reserve(_inBufSize);
  if (fread(buf, 1, _inBufSize, _file) != _inBufSize) {
    int errNum = errno;
    _addErrStr("ERROR - HrdRadxFile::_readRec");
    _addErrStr("  Reading record");
    _addErrStr("  ", strerror(errNum));
    return -1;
  }
  
  // read 4 byte end block
  
  Radx::si32 endCount;
  if (fread(&endCount, sizeof(endCount), 1, _file) != 1) {
    int errNum = errno;
    _addErrStr("ERROR - HrdRadxFile::_readRec");
    _addErrStr("  Reading endCount block");
    _addErrStr("  ", strerror(errNum));
    return -1;
  }

  if (_hrdIsSwapped && (endCount < 0 || endCount > 65536)) {
    ByteOrder::swap32(&endCount, sizeof(endCount));
  }
  
  if (startCount != endCount) {
    _addErrStr("ERROR - HrdRadxFile::_readRec");
    _addErrStr("  Start and endCount counts do not agree");
    _addErrInt("    startCount count: ", startCount);
    _addErrInt("    endCount count: ", endCount);
    return -1;
  }

  return 0;

}

/////////////////////////////////////
// get the record type

int HrdRadxFile::_getRecType()
  
{
  
  Radx::si16 recType;
  memcpy(&recType, _inBuf.getPtr(), sizeof(recType));
  if (_hrdIsSwapped) {
    ByteOrder::swap16(&recType, sizeof(recType), true);
  }
  return recType;

}

/////////////////////////////////////
// load a header record

void HrdRadxFile::_loadHeaderRec()
  
{

  memcpy(&_hdr, _inBuf.getPtr(), sizeof(_hdr));
  
  // swap as needed
  
  if (_hrdIsSwapped) {
    _swap(_hdr);
  }

}

/////////////////////////////////////
// load a data record

void HrdRadxFile::_loadDataRec()
  
{

  memcpy(&_dataRecHdr, _inBuf.getPtr(), sizeof(_dataRecHdr));
  
  // swap as needed
  
  if (_hrdIsSwapped) {
    _swap(_dataRecHdr);
  }

  // read in data
  
  _dataLen = _dataRecHdr.sizeof_rec - sizeof(_dataRecHdr);
  _dataBuf = (unsigned char *) _inBuf.getPtr() + sizeof(_dataRecHdr);

  // load up radar-specific quantities

  if (_dataRecHdr.radar_num == 1) {
    _radar = &_hdr.radar_lf;
    _isTailRadar = false;
  } else {
    _radar = &_hdr.radar_ta;
    _isTailRadar = true;
  }
  
  // set radar params
  
  _nGates = _radar->num_output_bins;
  _nSamples = _radar->sample_size;
  _wavelengthM = _radar->waveln_xe4 * 0.0001;

  _prf = _radar->PRF;
  _prt0 = 1.0 / _prf;
  _prt1 = _prt0;
  
  if(_radar->DSP_flag & 0x300) {
    /* 2/3 or 3/4 PRF */
    _staggered = true;
    _staggerRatio = 3.0 / 4.0;
    if (_radar->DSP_flag & 0x100) {
      _staggerRatio = 2.0 / 3.0;
    }
    _prt1 = _prt0 / _staggerRatio;
    _nyquist = (_wavelengthM * 0.25) / (_prt1 - _prt0);
  } else {
    _staggered = false;
    _staggerRatio = 1.0;
    _nyquist = (_wavelengthM / _prt0) * 0.25;
  }

  double pulseWidthS = _radar->pulse_width_xe8 * 1.e-8;
  if (pulseWidthS == 0) {
    if (_isTailRadar) {
      pulseWidthS = 0.5e-6;
    } else {
      pulseWidthS = 6.0e-6;
    }
  }

  _pulseWidthUs = pulseWidthS * 1.e-6;
  _gateSpacingKm = _radar->bin_spacing_xe3 / 1000.0;
  _startRangeKm = _radar->range_b1;

}

//////////////////////////////////////////////////////
// handle input rays

void HrdRadxFile::_handleRays()
  
{
  
  unsigned char *ptr = _dataBuf;
  unsigned char *endPtr = ptr + _dataLen;
  
  while (ptr < endPtr) {

    // print header
    
    hrd_ray_header_t rayHdr;
    memcpy(&rayHdr, ptr, sizeof(rayHdr));
    _swap(rayHdr);
    
    if (rayHdr.sizeof_ray < sizeof(rayHdr)) {
      // done
      return;
    }

    unsigned char *dataBuf = ptr + sizeof(rayHdr);
    int dataLen = rayHdr.sizeof_ray - sizeof(rayHdr);
    
    if (_isTailRadar) {
      
      // TA data
      
      if (!_readSetRadarNum || _readRadarNum != 1) {
        _handleTaRay(rayHdr, dataBuf, dataLen);
      }
        
    } else {
      
      // LF data
      
      if (_readRadarNum == 1) {
        _handleLfRay(rayHdr, dataBuf, dataLen);
      }
      
    }
    
    // move to next ray
    
    ptr += rayHdr.sizeof_ray;
    
  }

}

//////////////////////////////////////////////////////
// handle LF ray data

void HrdRadxFile::_handleLfRay(const hrd_ray_header_t &rayHdr,
                               const unsigned char *dataBuf,
                               int dataLen)
  
{

  // uncompress the data
  
  RadxBuf buf;
  _uncompress(dataBuf, dataLen, buf);
  unsigned char *bytes = (unsigned char *) buf.getPtr();

  // check number of gates
  
  int nGates = buf.getLen();
  if (nGates != _nGates) {
    cerr << "WARNING - HrdRadxFile::_handleLfData" << endl;
    cerr << "  Bad nGates: " << nGates << endl;
    cerr << "  Should be: " << _nGates << endl;
  }
  
  // load up DBZ field data

  Radx::si08 *data = new Radx::si08[nGates];
  for (int ii = 0; ii < nGates; ii++) {
    data[ii] = (int) bytes[ii] - 128;
  }
  
  // create a new Radx ray
  
  RadxRay *ray = new RadxRay();
  
  // set ray meta data
  
  _setRayMetadata(*ray, rayHdr);
  
  // check that we need DBZ field
  
  if (isFieldRequiredOnRead("DBZ")) {
    
    // add the field
    
    RadxField *field = new RadxField("DBZ", "dBZ");
    field->setStandardName("equivalent_reflectivity_factor");
    field->setLongName("reflectivity");
    field->copyRangeGeom(*ray);
    field->setRangeGeom(_startRangeKm, _gateSpacingKm);

    double scale = 0.5;
    double offset = 32.0;
    field->setTypeSi08(-128, scale, offset);
    field->addDataSi08(nGates, data);
    
    if (_debug) {
      ray->print(cerr);
    }

    ray->addField(field);
    
  }

  // add to vector
  
  _readVol->addRay(ray);

  delete[] data;

}

//////////////////////////////////////////////////////
// handle TA ray data

void HrdRadxFile::_handleTaRay(const hrd_ray_header_t &rayHdr,
                               const unsigned char *dataBuf,
                               int dataLen)

{

  // uncompress the data

  RadxBuf buf;
  _uncompress(dataBuf, dataLen, buf);
  unsigned char *bytes = (unsigned char *) buf.getPtr();
  
  // compute the number of fields for the data in the raw array
  
  bool haveDbz = (rayHdr.field_code & 0x80);
  bool haveVel = (rayHdr.field_code & 0x40);
  bool haveWidth = (rayHdr.field_code & 0x20);
  
  int nFields = 0;
  if (haveDbz) nFields++;
  if (haveVel) nFields++;
  if (haveWidth) nFields++;

  int nGates = _nGates;

  if (nGates * nFields > (int) buf.getLen()) {
    cerr << "WARNING - HrdRadxFile::_handleTaData" << endl;
    cerr << "  nFields: " << nFields << endl;
    cerr << "  nGates: " << nGates << endl;
    cerr << "  Not enough data, found nbytes: " << buf.getLen() << endl;
    cerr << "  Expecting: " << nGates * nFields << endl;
    nGates = buf.getLen() / nFields;
  }

  // create a new Radx ray
  
  RadxRay *ray = new RadxRay();
  
  // set ray meta data
  
  _setRayMetadata(*ray, rayHdr);
  
  // load up field data and print out

  int fieldCount = 0;
  Radx::si08 *data = new Radx::si08[nGates];

  if (haveDbz) {

    int jj = fieldCount;
    for (int ii = 0; ii < nGates; ii++, jj += nFields) {
      data[ii] = (int) bytes[jj] - 128;
    }

    if (isFieldRequiredOnRead("DBZ")) {
      
      // add the field
      
      RadxField *field = new RadxField("DBZ", "dBZ");
      field->setStandardName("equivalent_reflectivity_factor");
      field->setLongName("reflectivity");
      field->copyRangeGeom(*ray);
      field->setRangeGeom(_startRangeKm, _gateSpacingKm);
      
      double scale = 0.5;
      double offset = 32.0;
      field->setTypeSi08(-128, scale, offset);
      field->addDataSi08(nGates, data);
      
      if (_debug) {
        ray->print(cerr);
      }
      
      ray->addField(field);

    }

    fieldCount++;

  }
  
  if (haveVel) {

    int jj = fieldCount;
    for (int ii = 0; ii < nGates; ii++, jj += nFields) {
      data[ii] = (int) bytes[jj] -128;
    }
    if (isFieldRequiredOnRead("VEL")) {
      
      // add the field
      
      RadxField *field = new RadxField("VEL", "m/s");
      field->setStandardName("radial_velocity_of_scatterers_away_from_instrument");
      field->setLongName("radial_velocity");
      field->copyRangeGeom(*ray);
      
      field->setFieldFolds(-_nyquist, _nyquist);
      field->setRangeGeom(_startRangeKm, _gateSpacingKm);

      double scale = _nyquist / 127.0;
      double offset = 0.0;
      field->setTypeSi08(-128, scale, offset);
      field->addDataSi08(nGates, data);
      
      if (_debug) {
        ray->print(cerr);
      }
      
      ray->addField(field);

    }

    fieldCount++;

  }
  
  if (haveWidth) {

    int jj = fieldCount;
    for (int ii = 0; ii < nGates; ii++, jj += nFields) {
      data[ii] = (int) bytes[jj] - 128;
    }

    if (isFieldRequiredOnRead("WIDTH")) {
      
      // add the field
      
      RadxField *field = new RadxField("WIDTH", "m/s");
      field->setStandardName("doppler_spectrum_width");
      field->setLongName("spectrum_width");
      field->copyRangeGeom(*ray);
      field->setRangeGeom(_startRangeKm, _gateSpacingKm);
      
      double scale = _nyquist / 256.0;
      double offset = _nyquist / 2.0;
      field->setTypeSi08(-128, scale, offset);
      field->addDataSi08(nGates, data);
      
      if (_debug) {
        ray->print(cerr);
      }
      
      ray->addField(field);

    }

    fieldCount++;

  }
  
  // add to vector
  
  if (/* _readIgnoreTransitions && */ray->getAntennaTransition()) {
    delete ray;
  } else {
    _readVol->addRay(ray);
  }

  delete[] data;

}

/////////////////////////
// set the ray metadata

void HrdRadxFile::_setRayMetadata(RadxRay &ray,
                                  const hrd_ray_header_t &rayHdr)
  
{
  
  if (_debug) {
    _print(rayHdr, cerr);
  }

  int secs = rayHdr.seconds_x100 / 100;
  double subSecs = (rayHdr.seconds_x100 - (secs * 100)) / 100.0;

  int year = rayHdr.year;
  if (year < 50) {
    year += 2000;
  } else if (year > 50 && year < 100) {
    year += 1900;
  }
  RadxTime rayTime(year, rayHdr.month, rayHdr.day,
                   rayHdr.hour, rayHdr.minute, secs, subSecs);
  
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

  if (_isTailRadar) {
    ray.setPolarizationMode(Radx::POL_MODE_VERTICAL);
  } else {
    ray.setPolarizationMode(Radx::POL_MODE_HORIZONTAL);
  }
  
  if (_staggered) {
    ray.setPrtMode(Radx::PRT_MODE_STAGGERED);
  } else {
    ray.setPrtMode(Radx::PRT_MODE_FIXED);
  }

  double azimuth = _getAngle(rayHdr.azimuth);
  if (azimuth < 0) {
    azimuth += 360.0;
  }
  ray.setAzimuthDeg(azimuth);

  double elevation = _getAngle(rayHdr.elevation);
  if (_isTailRadar) {
    ray.setElevationDeg(elevation);
  } else {
    ray.setElevationDeg(elevation * -1.0);
  }

  if (_isTailRadar) {
    if (rayHdr.elevation == 0) {
      ray.setAntennaTransition(true);
    }
  }

  int sweepNum = _dataRecHdr.sweep_num;
  if (_isTailRadar) {
    if (_prevSweepNum != sweepNum) {
      // check for change in elevation num
      // if none, then ignore the sweep number change
      if (fabs(elevation - _prevElev) < 0.01) {
        sweepNum = _prevSweepNum;
      }
    }
  }
  ray.setSweepNumber(sweepNum);
  _prevElev = elevation;
  _prevSweepNum = sweepNum;
        
  if (_isTailRadar) {
    ray.setSweepMode(Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE);
  } else {
    ray.setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
  }

  if (_isTailRadar) {
    ray.setFixedAngleDeg(elevation);
  } else {
    ray.setFixedAngleDeg(_getAngle(_radar->tilt_angle));
  }
  ray.setTrueScanRateDegPerSec(Radx::missingMetaDouble);
  ray.setTargetScanRateDegPerSec(_radar->sweep_speed_x10 * 0.6);
  ray.setIsIndexed(false);
  ray.setAngleResDeg(Radx::missingMetaDouble);
  ray.setNSamples(_nSamples);
  ray.setPulseWidthUsec(_pulseWidthUs);
  ray.setPrtSec(_prt0);
  ray.setPrtRatio(_staggerRatio);
  ray.setNyquistMps(_nyquist);
  double maxRangeKm = (_prt0 * Radx::LIGHT_SPEED) / 2000.0;
  ray.setUnambigRangeKm(maxRangeKm);

  double xmitPeakPowerWatts = 70000.0;
  if (_isTailRadar) {
    xmitPeakPowerWatts = 60000.0;
  }
  double xmitPeakPowerDbm = 10.0 * log10(xmitPeakPowerWatts * 1.0e3);
  ray.setMeasXmitPowerDbmH(xmitPeakPowerDbm);
  ray.setMeasXmitPowerDbmV(xmitPeakPowerDbm);
  ray.setRangeGeom(_startRangeKm, _gateSpacingKm);

  // georeference

  RadxGeoref georef;
  georef.setTimeSecs(raySecs);
  georef.setNanoSecs(rayNanoSecs);

  _latitude = _getAngle(rayHdr.latitude);
  _longitude = _getAngle(rayHdr.longitude);
  _altitudeM = rayHdr.altitude_xe3;

  georef.setLongitude(_longitude);
  georef.setLatitude(_latitude);
  georef.setAltitudeKmMsl(_altitudeM / 1000.0);

  georef.setEwVelocity(rayHdr.ac_vew_x10 / 10.0);
  georef.setNsVelocity(rayHdr.ac_vns_x10 / 10.0);
  georef.setVertVelocity(rayHdr.ac_vud_x10 / 10.0);
  georef.setHeading(_getAngle(rayHdr.ac_heading));
  georef.setRoll(_getAngle(rayHdr.ac_roll));
  georef.setPitch(_getAngle(rayHdr.ac_pitch));
  georef.setDrift(_getAngle(rayHdr.ac_drift));
  georef.setRotation(azimuth);
  georef.setTilt(ray.getElevationDeg());
  georef.setEwWind(rayHdr.ac_ui_x10 / 10.0);
  georef.setNsWind(rayHdr.ac_vi_x10 / 10.0);
  georef.setVertWind(rayHdr.ac_wi_x10 / 10.0);

  ray.setGeoref(georef);
  
}

//////////////////////////////////////////////////////
// print main header

void HrdRadxFile::_print(const hrd_header_t &hdr,
                         ostream &out)

{
  
  out << "=========== hrd_header_t ===========" << endl;
  out << "  header_flag: " << hdr.header_flag << endl;
  out << "  sizeof_header: " << hdr.sizeof_header << endl;
  out << "  tape_num: " << hdr.tape_num << endl;
  out << "  hd_fmt_ver: " << hdr.hd_fmt_ver << endl;
  out << "  year: " << hdr.year << endl;
  out << "  month: " << hdr.month << endl;
  out << "  day: " << hdr.day << endl;
  out << "  hour: " << hdr.hour << endl;
  out << "  minute: " << hdr.minute << endl;
  out << "  second: " << hdr.second << endl;
  out << "  LF_menu: " << Radx::makeString(hdr.LF_menu, 16) << endl;
  out << "  TA_menu: " << Radx::makeString(hdr.TA_menu, 16) << endl;
  out << "  Data_menu[16]: " << Radx::makeString(hdr.Data_menu, 16) << endl;
  out << "  nav_system: " << hdr.nav_system << endl;
  out << "  LU_tape_drive: " << hdr.LU_tape_drive << endl;
  out << "  aircraft_id: " << hdr.aircraft_id << endl;
  out << "  flight_id[8]: " << Radx::makeString(hdr.flight_id, 8) << endl;
  out << "  data_header_len: " << hdr.data_header_len << endl;
  out << "  ray_header_len: " << hdr.ray_header_len << endl;
  out << "  time_zone_offset: " << hdr.time_zone_offset << endl;
  out << "=====>> LOWER FUSELAGE RADAR <<=====" << endl;
  _print(hdr.radar_lf, out);
  out << "==========>> TAIL RADAR <<==========" << endl;
  _print(hdr.radar_ta, out);
  if (strlen(hdr.comments) > 0) {
    out << "=======>> COMMENTS" << endl;
    cerr << hdr.comments << endl;
  }
  out << "====================================" << endl;
  
}

//////////////////////////////////////////////////////
// print radar info header

void HrdRadxFile::_print(const hrd_radar_info_t &radar,
                         ostream &out)

{
  
  out << "========= hrd_radar_info_t =========" << endl;

  out << "  sample_size: " << radar.sample_size << endl;
  out << "  DSP_flag: " << radar.DSP_flag << endl;
  out << "  refl_slope_x4096: " << radar.refl_slope_x4096 << endl;
  out << "  refl_noise_thr_x16: " << radar.refl_noise_thr_x16 << endl;
  out << "  clutter_cor_thr_x16: " << radar.clutter_cor_thr_x16 << endl;
  out << "  SQI_thr: " << radar.SQI_thr << endl;
  out << "  width_power_thr_x16: " << radar.width_power_thr_x16 << endl;
  out << "  calib_refl_x16: " << radar.calib_refl_x16 << endl;
  out << "  AGC_decay_code: " << radar.AGC_decay_code << endl;
  out << "  dual_PRF_stabil_delay: " << radar.dual_PRF_stabil_delay << endl;
  out << "  thr_flags_uncorr_refl: " << radar.thr_flags_uncorr_refl << endl;
  out << "  thr_flags_vel: " << radar.thr_flags_vel << endl;
  out << "  thr_flags_width: " << radar.thr_flags_width << endl;
  out << "  data_mode: " << radar.data_mode << endl;
  out << "  range_b1: " << radar.range_b1 << endl;
  out << "  variable_spacing_flag: " << radar.variable_spacing_flag << endl;
  out << "  bin_spacing_xe3: " << radar.bin_spacing_xe3 << endl;
  out << "  num_input_bins: " << radar.num_input_bins << endl;
  out << "  range_avg_state: " << radar.range_avg_state << endl;
  out << "  b1_adjust_xe4: " << radar.b1_adjust_xe4 << endl;
  out << "  num_output_bins: " << radar.num_output_bins << endl;
  out << "  PRT_noise_sample: " << radar.PRT_noise_sample << endl;
  out << "  range_noise_sample: " << radar.range_noise_sample << endl;
  out << "  log_rec_noise_x64: " << radar.log_rec_noise_x64 << endl;
  out << "  I_A2D_offset_x256: " << radar.I_A2D_offset_x256 << endl;
  out << "  Q_A2D_offset_x256: " << radar.Q_A2D_offset_x256 << endl;
  out << "  waveln_xe4: " << radar.waveln_xe4 << endl;
  out << "  pulse_width_xe8: " << radar.pulse_width_xe8 << endl;
  out << "  PRF: " << radar.PRF << endl;
  out << "  DSS_flag: " << radar.DSS_flag << endl;
  out << "  trans_recv_number: " << radar.trans_recv_number << endl;
  out << "  transmit_power: " << radar.transmit_power << endl;
  out << "  gain_control_flag: " << radar.gain_control_flag << endl;
  out << "  scan_mode: " << radar.scan_mode << endl;
  out << "  sweep_speed_x10: " << radar.sweep_speed_x10 << endl;
  out << "  tilt_angle: " << radar.tilt_angle << endl;
  out << "  sector_center: " << radar.sector_center << endl;
  out << "  sector_width: " << radar.sector_width << endl;

  out << "====================================" << endl;
  
}

//////////////////////////////////////////////////////
// print data record header

void HrdRadxFile::_print(const hrd_data_rec_header_t &rec,
                         ostream &out)

{
  
  out << "====== hrd_data_rec_header_t =======" << endl;

  out << "  data_record_flag: " << rec.data_record_flag << endl;
  out << "  sizeof_rec: " << rec.sizeof_rec << endl;
  out << "  sweep_num: " << rec.sweep_num << endl;
  out << "  rec_num: " << rec.rec_num << endl;
  out << "  radar_num: " << (int) rec.radar_num << endl;
  out << "  rec_num_flag: " << (int) rec.rec_num_flag << endl;

  out << "====================================" << endl;
  
}

//////////////////////////////////////////////////////
// print ray header

void HrdRadxFile::_print(const hrd_ray_header_t &ray,
                         ostream &out)

{
  
  out << "======== hrd_ray_header_t ==========" << endl;

  out << "  sizeof_ray: " << ray.sizeof_ray << endl;
  out << "  field_code: 0x" << hex << (int) ray.field_code << dec << endl;
  if (ray.field_code & 0x80) {
    out << "    reflectivity" << endl;
  }
  if (ray.field_code & 0x40) {
    out << "    velocity" << endl;
  }
  if (ray.field_code & 0x20) {
    out << "    width" << endl;
  }
  if (ray.field_code & 0x10) {
    out << "    from TA DSP" << endl;
  }
  if (ray.field_code & 0x08) {
    out << "    from LF DSP" << endl;
  }
  if (ray.field_code & 0x04) {
    out << "    time-series" << endl;
  }
  out << "  year: " << (int) ray.year << endl;
  out << "  month: " << (int) ray.month << endl;
  out << "  day: " << (int) ray.day << endl;
  out << "  ray_code: " << (int) ray.ray_code << endl;
  out << "  hour: " << (int) ray.hour << endl;
  out << "  minute: " << ray.minute << endl;
  out << "  seconds: " << (double) ray.seconds_x100 / 100.0 << endl;
  out << "  latitude: " << _getAngle(ray.latitude) << endl;
  out << "  longitude: " << _getAngle(ray.longitude) << endl;
  out << "  altitude_xe3: " << ray.altitude_xe3 << endl;
  out << "  ac_vew: " << ray.ac_vew_x10 / 10.0 << endl;
  out << "  ac_vns: " << ray.ac_vns_x10 / 10.0 << endl;
  out << "  ac_vud: " << ray.ac_vud_x10 / 10.0 << endl;
  out << "  ac_ui: " << ray.ac_ui_x10 / 10.0 << endl;
  out << "  ac_vi: " << ray.ac_vi_x10 / 10.0 << endl;
  out << "  ac_wi: " << ray.ac_wi_x10 / 10.0 << endl;
  out << "  RCU_status: " << ray.RCU_status << endl;
  out << "  elevation: " << _getAngle(ray.elevation) << endl;
  out << "  azimuth: " << _getAngle(ray.azimuth) << endl;
  out << "  ac_pitch: " << _getAngle(ray.ac_pitch) << endl;
  out << "  ac_roll: " << _getAngle(ray.ac_roll) << endl;
  out << "  ac_drift: " << _getAngle(ray.ac_drift) << endl;
  out << "  ac_heading: " << _getAngle(ray.ac_heading) << endl;

  out << "====================================" << endl;
  
}

//////////////////////////////////////////////////////
// print rays

void HrdRadxFile::_printRays(bool printData, ostream &out)
  
{
  
  unsigned char *ptr = _dataBuf;
  unsigned char *endPtr = ptr + _dataLen;
  
  while (ptr < endPtr) {

    // print header
    
    hrd_ray_header_t rayHdr;
    memcpy(&rayHdr, ptr, sizeof(rayHdr));
    _swap(rayHdr);
    _print(rayHdr, out);
    
    if (rayHdr.sizeof_ray < sizeof(rayHdr)) {
      // done
      return;
    }

    // print data if required

    if (printData) {

      unsigned char *dataBuf = ptr + sizeof(rayHdr);
      int dataLen = rayHdr.sizeof_ray - sizeof(rayHdr);
      
      if (rayHdr.field_code & 0x10) {
      
        // TA data

        bool haveDbz = (rayHdr.field_code & 0x80);
        bool haveVel = (rayHdr.field_code & 0x40);
        bool haveWidth = (rayHdr.field_code & 0x20);
        
        _printTaData(dataBuf, dataLen,
                     haveDbz, haveVel, haveWidth,
                     out);
        
      } else if (rayHdr.field_code & 0x08) {

        // LF data

        _printLfData(dataBuf, dataLen, out);
        
      }

    }
    
    // move to next ray
    
    ptr += rayHdr.sizeof_ray;
    
  }

}

//////////////////////////////////////////////////////
// print LF data

void HrdRadxFile::_printLfData(const unsigned char *dataBuf,
                               int dataLen,
                               ostream &out)

{

  // uncompress the data
  
  RadxBuf buf;
  _uncompress(dataBuf, dataLen, buf);
  unsigned char *bytes = (unsigned char *) buf.getPtr();

  // check number of gates

  int nGates = buf.getLen();
  if (nGates != _nGates) {
    cerr << "WARNING - HrdRadxFile::_printLfData" << endl;
    cerr << "  Bad nGates: " << nGates << endl;
    cerr << "  Should be: " << _nGates << endl;
  }
  
  Radx::fl32 *data = new Radx::fl32[nGates];
  for (int ii = 0; ii < nGates; ii++) {
    data[ii] = _dbzVal(bytes[ii]);
  }

  // print
  
  _printFieldData("LF DBZ", data, nGates, out);

  delete[] data;

}
  
//////////////////////////////////////////////////////
// print TA data

void HrdRadxFile::_printTaData(const unsigned char *dataBuf,
                               int dataLen,
                               bool haveDbz,
                               bool haveVel,
                               bool haveWidth,
                               ostream &out)

{

  // uncompress the data

  RadxBuf buf;
  _uncompress(dataBuf, dataLen, buf);
  unsigned char *bytes = (unsigned char *) buf.getPtr();
  
  // compute the number of fields for the data in the raw array

  int nFields = 0;
  if (haveDbz) nFields++;
  if (haveVel) nFields++;
  if (haveWidth) nFields++;

  int nGates = _nGates;
  if (nGates * nFields > (int) buf.getLen()) {
    cerr << "ERROR - HrdRadxFile::_printTaData" << endl;
    cerr << "  nFields: " << nFields << endl;
    cerr << "  nGates: " << nGates << endl;
    cerr << "  Not enough data, found nbytes: " << buf.getLen() << endl;
    cerr << "  Expecting: " << nGates * nFields << endl;
    nGates = buf.getLen() / nFields;
  }

  // load up field data and print out

  int fieldCount = 0;
  Radx::fl32 *data = new Radx::fl32[nGates];

  if (haveDbz) {
    int jj = fieldCount;
    for (int ii = 0; ii < nGates; ii++, jj += nFields) {
      data[ii] = _dbzVal(bytes[jj]);
    }
    _printFieldData("TA DBZ", data, nGates, out);
    fieldCount++;
  }
  
  if (haveVel) {
    int jj = fieldCount;
    for (int ii = 0; ii < nGates; ii++, jj += nFields) {
      data[ii] = _velVal(bytes[jj]);
    }
    _printFieldData("TA VEL", data, nGates, out);
    fieldCount++;
  }
  
  if (haveWidth) {
    int jj = fieldCount;
    for (int ii = 0; ii < nGates; ii++, jj += nFields) {
      data[ii] = _widthVal(bytes[jj]);
    }
    _printFieldData("TA WIDTH", data, nGates, out);
    fieldCount++;
  }
  
  delete[] data;

}

//////////////////////////////////////////////////////
// print field data

void HrdRadxFile::_printFieldData(const string &label,
                                  const Radx::fl32 *data,
                                  int nGates,
                                  ostream &out)
  
{

  out << "========================================================" << endl;
  out << "Ray data for: " << label << endl;
  out << "nGates: " << nGates << endl;

  int printed = 0;
  int count = 1;
  Radx::fl32 prevVal = data[0];
  for (int ii = 1; ii < nGates; ii++) {
    Radx::fl32 val = data[ii];
    if (val != prevVal) {
      _printPacked(out, count, prevVal, Radx::missingFl32);
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
  _printPacked(out, count, prevVal, Radx::missingFl32);
  out << endl;
  out << "========================================================" << endl;

}
  
//////////////////////////////////////////////////////
// uncompress packed data

void HrdRadxFile::_uncompress(const unsigned char *dataBuf,
                              int dataLen,
                              RadxBuf &buf)
  
{

  buf.clear();
  Radx::ui16 *comp = (Radx::ui16 *) dataBuf;
  int nwords = dataLen / 2;

  for (int ii = 0; ii < nwords; ii++) {

    Radx::ui16 word = comp[ii];
    ByteOrder::swap16(&word, sizeof(word), true);
    int msbit = word & 0x8000;
    int lbits = word & 0x7fff;

    if (msbit == 0) {

      if (lbits == 1) {
        // end of ray
        return;
      }
      
      // zeros

      Radx::ui16 zero = 0;
      for (int jj = 0; jj < lbits; jj++) {
        buf.add(&zero, sizeof(zero));
      } // jj

    } else {

      for (int jj = 0; jj < lbits; jj++) {
        ii++;
        word = comp[ii];
        buf.add(&word, sizeof(word));
      } // jj

    } // if (msbit)

  } // ii

}

////////////////////////////////
// get DBZ value from scaled int

Radx::fl32 HrdRadxFile::_dbzVal(int ival) 
{
  
  if (ival == 0) {
    return Radx::missingFl32;
  }
  return (ival - 64.0) / 2.0;

}

////////////////////////////////
// get VEL value from scaled int

Radx::fl32 HrdRadxFile::_velVal(int ival) 
{

  if (ival == 0) {
    return Radx::missingFl32;
  }
  return ((ival - 128.0) / 127.0) * _nyquist;

}

////////////////////////////////
// get WIDTH value from scaled int

Radx::fl32 HrdRadxFile::_widthVal(int ival) 
{

  if (ival == 0) {
    return Radx::missingFl32;
  }
  return (ival / 256.0) * _nyquist;

}

/////////////////////////////////////////////////////////////////
// print in packed format, using count for identical data values

void HrdRadxFile::_printPacked(ostream &out, int count,
                               Radx::fl32 val, Radx::fl32 missing)

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

//////////////////////////////////////////////////////
// compute angle as double

double HrdRadxFile::_getAngle(int binaryAngle)
{
  return ((double) binaryAngle * _angleConversion);
}

