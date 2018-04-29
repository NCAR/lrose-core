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
// TdwrRadxFile.cc
//
// TdwrRadxFile object
//
// Support for radial data in TDWR format
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2013
//
///////////////////////////////////////////////////////////////

#include <Radx/TdwrRadxFile.hh>
#include <Radx/TdwrLoc.hh>
#include <Radx/NcfRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/ByteOrder.hh>
#include <iomanip>
#include <cstdio>
using namespace std;

//////////////
// Constructor

TdwrRadxFile::TdwrRadxFile() :
        RadxFile()

{

  _readVol = NULL;
  _file = NULL;
  clear();

  _doPrint = false;
  _printRays = false;
  _printData = false;

}

/////////////
// destructor

TdwrRadxFile::~TdwrRadxFile()

{
  clear();
}

/////////////////////////////////////////////////////////
// Check if specified file is TDWR format
// Returns true if supported, false otherwise

bool TdwrRadxFile::isSupported(const string &path)

{

  if (isTdwr(path)) {
    return true;
  }

  return false;

}

////////////////////////////////////////////////////////////
// Check if this is an TDWR file
// Returns true on success, false on failure

bool TdwrRadxFile::isTdwr(const string &path)
  
{

  clear();
  
  // open file
  
  if (_openRead(path)) {
    _addErrStr("ERROR - TdwrRadxFile::isTdwr");
    return false;
  }
  
  // read first 4 bytes as message header

  message_hdr_t mhdr;
  if (fread(&mhdr, sizeof(mhdr), 1, _file) != 1) {
    _close();
    return false;
  }
  _close();

  // try unswapped
  
  _tdwrIsSwapped = false;
  if (_isTdwr(mhdr)) {
    return true;
  }

  // swap

  ByteOrder::swap16(&mhdr, 4, true);
  _tdwrIsSwapped = true;
  if (_isTdwr(mhdr)) {
    return true;
  }

  // no good

  return false;

}

//////////////////////////////////////////////////////////////
// Check if this message header is appropriate for a TDWR file
// Returns true on success, false on failure

  bool TdwrRadxFile::_isTdwr(const message_hdr_t &mhdr)
  
{

  if (mhdr.message_id == 0x2b00 && 
      mhdr.message_length == 6144) {
    return true;
  }
  
  if (mhdr.message_id == 0x2b01 && 
      mhdr.message_length == 6144) {
    return true;
  }
  
  if (mhdr.message_id == 0x2b02 && 
      mhdr.message_length == 1024) {
    return true;
  }
  
  if (mhdr.message_id == 0x2c00 && 
      mhdr.message_length <= 124) {
    return true;
  }
  
  if (mhdr.message_id == 0x2c01 && 
      mhdr.message_length <= 1172) {
    return true;
  }
  
  if (mhdr.message_id == 0x2c02 && 
      mhdr.message_length == 64) {
    return true;
  }

  return false;

}

/////////////////////////////////////////////////////////
// clear the data in the object

void TdwrRadxFile::clear()
  
{

  clearErrStr();

  _close();

  _tdwrIsSwapped = false;

  _startTimeSecs = 0;
  _endTimeSecs = 0;
  _startNanoSecs = 0;
  _endNanoSecs = 0;

  _nGatesData = 0;
  _nGatesDwell = 0;
  _nSamples = 0;
  _nyquist = 0.0;

  _pulseWidthUs = 0.0;
  _wavelengthM = 0.0;
  _prf = 0.0;
  _prtSecs = 0.0;
  _nyquist = 0.0;

  _gateSpacingKm = 0.0;
  _startRangeKm = 0.0;

  _latitude = Radx::missingMetaDouble;
  _longitude = Radx::missingMetaDouble;
  _altitudeM = Radx::missingMetaDouble;
  _frequency = 5.6e9;

}

/////////////////////////////////////////////////////////
// Write data from volume to specified directory
// Writes as CFRadial, TDWR not supported on write
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs
// Use getPathInUse() for path written

int TdwrRadxFile::writeToDir(const RadxVol &vol,
                             const string &dir,
                             bool addDaySubDir,
                             bool addYearSubDir)
  
{

  // Writing TDWR files is not supported
  // therefore write in CF Radial format instead

  cerr << "WARNING - TdwrRadxFile::writeToDir" << endl;
  cerr << "  Writing TDWR format files not supported" << endl;
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
// Writes as CFRadial, TDWR not supported on write
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs
// Use getPathInUse() for path written

int TdwrRadxFile::writeToPath(const RadxVol &vol,
                              const string &path)
  
{

  // Writing TDWR files is not supported
  // therefore write in CF Radial format instead

  cerr << "WARNING - TdwrRadxFile::writeToPath" << endl;
  cerr << "  Writing TDWR format files not supported" << endl;
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

int TdwrRadxFile::readFromPath(const string &path,
                               RadxVol &vol)
  
{

  _doPrint = false;
  _printRays = false;
  _printData = false;

  _initForRead(path, vol);
  
  if (!isTdwr(path)) {
    _addErrStr("ERROR - TdwrRadxFile::readFromPath");
    _addErrStr("  Not a recognized TDWR file");
    return -1;
  }
  
  // open file

  if (_openRead(path)) {
    _addErrStr("ERROR - TdwrRadxFile::readFromPath");
    return -1;
  }
  
  // read in file contents
  
  if (_performRead(cerr)) {
    _close();
    return -1;
  }

  // close input file
  
  _close();

  // check

  if (_readVol->getNRays() == 0) {
    _addErrStr("ERROR - TdwrRadxFile::readFromPath");
    _addErrStr("  No rays found, file: ", _pathInUse);
    return -1;
  }
  
  // set the meta data on the volume

  if (_setVolMetaData(path)) {
    return -1;
  }
  
  // remove rays with all missing data, if requested
  
  if (_readRemoveRaysAllMissing) {
    _readVol->removeRaysWithDataAllMissing();
  }

  // load the sweep information from the rays

  _readVol->loadSweepInfoFromRays();
  
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

  _fileFormat = FILE_FORMAT_TDWR;

  return 0;

}

//////////////////////////////////////
// open netcdf file for reading
// Returns 0 on success, -1 on failure

int TdwrRadxFile::_openRead(const string &path)
  
{

  _close();
  _file = fopen(path.c_str(), "r");
  
  // Check that constructor succeeded
  
  if (_file == NULL) {
    int errNum = errno;
    _addErrStr("ERROR - TdwrRadxFile::_openRead");
    _addErrStr("  Cannot open file for reading, path: ", path);
    _addErrStr("  ", strerror(errNum));
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////
// close netcdf file if open
// remove error object if it exists

void TdwrRadxFile::_close()
  
{
  
  // close file if open
  
  if (_file) {
    fclose(_file);
    _file = NULL;
  }
  
}

/////////////////////////////////////////////////////////
// print summary after read

void TdwrRadxFile::print(ostream &out) const
  
{
  
  out << "=============== TdwrRadxFile ===============" << endl;
  RadxFile::print(out);
  out << "===========================================" << endl;

}

////////////////////////////////////////////////////////////
// Print native data in uf file
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int TdwrRadxFile::printNative(const string &path, ostream &out,
                              bool printRays, bool printData)
  
{

  _doPrint = true;
  _printRays = printRays;
  _printData = printData;

  clear();
  RadxVol vol;
  _readVol = &vol;
  _readVol->clear();
  _pathInUse = path;
  vol.setPathInUse(_pathInUse);
  _readPaths.clear();

  if (!isTdwr(path)) {
    _addErrStr("ERROR - TdwrRadxFile::printNative");
    _addErrStr("  Not a recognized TDWR file");
    return -1;
  }
  
  // open file

  if (_openRead(path)) {
    _addErrStr("ERROR - TdwrRadxFile::printNative");
    return -1;
  }

  // perform the read, printing as required
  
  if (_performRead(out)) {
    _close();
    return -1;
  }

  // close and return

  _close();
  return 0;

}

////////////////////////////////////////////////////////////
// Read in data from specified path, load up volume object.
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs

int TdwrRadxFile::_performRead(ostream &out)
  
{

  // read in file contents
  
  while (!feof(_file)) {

    if (_verbose) {
      cerr << "  posn in file: " << ftell(_file) << endl;
    }

    // read message header
    
    if (fread(&_mhdr, sizeof(_mhdr), 1, _file) != 1) {
      int errNum = errno;
      if (feof(_file)) {
        return 0;
      }
      cerr << "ERROR - cannot read message header, file: " 
           << _pathInUse << endl;
      cerr << strerror(errNum) << endl;
      return -1;
    }
    
    // swap message header

    _swap(_mhdr);

    // print message header
    
    if (_doPrint || _debug) {
      _print(_mhdr, out);
    }

    // read in rest of buffer
    
    int bytesToRead = REC_LEN - sizeof(message_hdr_t);
    
    if ((int) fread(_dataBuf, 1, bytesToRead, _file) != bytesToRead) {
      int errNum = errno;
      cerr << "ERROR - cannot read data, file: " << _pathInUse << endl;
      cerr << strerror(errNum) << endl;
      return -1;
    }
    
    if ((_mhdr.message_id == NORMAL_PRF_BASE_DATA ||
         _mhdr.message_id == LOW_PRF_BASE_DATA) &&
        (_mhdr.message_length >= sizeof(_dhdr))) {

      _handleRay(out);

    } else {
      
      // LLWAS message

      if (_doPrint || _debug) {
        if (_mhdr.message_id == LLWAS_SENSOR) {
          cerr << "===>> LLWAS_SENSOR message" << endl;
        } else if (_mhdr.message_id == LLWASIII_DATA) {
          cerr << "===>> LLWASIII_DATA message" << endl;
        } else if (_mhdr.message_id == LLWASII_DATA) {
          cerr << "===>> LLWASII_DATA message" << endl;
        } else if (_mhdr.message_id == LLWASII_MAPPING) {
          cerr << "===>> LLWASII_MAPPING message" << endl;
        }
      }

    }
    
  }
  
  return 0;
  
}

///////////////////////////
// handle incoming ray
//
// Returns 0 on success, -1 on failure

int TdwrRadxFile::_handleRay(ostream &out)

{

  // set data header
  
  memcpy(&_dhdr, _dataBuf, sizeof(_dhdr));
  
  // swap header
  
  _swap(_dhdr);
  
  // print header
  
  if (_doPrint || _debug) {
    _print(_dhdr, out);
  }

  // create a new Radx ray
  
  RadxRay *ray = new RadxRay();
  
  // set ray data
  
  if (_setRayData(*ray)) {
    delete ray;
    return -1;
  }

  // add ray to vol

  _readVol->addRay(ray);
  
  return 0;

}
      
///////////////////////////
// set volume meta data

int TdwrRadxFile::_setVolMetaData(const string &path)

{

  _readVol->setOrigFormat("TDWR");

  _readVol->setVolumeNumber(_volumeNumber);
  _readVol->setStartTime(_startTimeSecs, _startNanoSecs);
  _readVol->setEndTime(_endTimeSecs, _endNanoSecs);
  
  _readVol->setInstrumentName("TDWR");
  _readVol->setScanName("surveillance");
  _readVol->setScanId(0);
  _readVol->setSiteName("UNKNOWN");

  _readVol->setInstrumentType(Radx::INSTRUMENT_TYPE_RADAR);
  _readVol->setPlatformType(Radx::PLATFORM_TYPE_FIXED);
  _readVol->setPrimaryAxis(Radx::PRIMARY_AXIS_Z);
  _readVol->setTitle("TERMINAL DOPPLER WEATHER RADAR");
  
  _readVol->setSource(_pathInUse);
  _readVol->setReferences("Read in by class TdwrRadxFile");
  _readVol->setHistory("Read in from TDWR volume file");

  TdwrLoc loc;
  if (loc.loadLocationFromFilePath(path) == 0) {
    _readVol->setSiteName(loc.getName());
    _latitude = loc.getLatitudeDeg();
    _longitude = loc.getLongitudeDeg();
    _altitudeM = loc.getRadarHtM();
    _frequency = loc.getFreqGhz() * 1.0e9;
  }
  
  _readVol->setLatitudeDeg(_latitude);
  _readVol->setLongitudeDeg(_longitude);
  _readVol->setAltitudeKm(_altitudeM / 1000.0);

  _readVol->addWavelengthM(_wavelengthM);

  _readVol->setRadarBeamWidthDegH(0.55);
  _readVol->setRadarBeamWidthDegV(0.55);
  _readVol->setRadarAntennaGainDbH(50.0);
  _readVol->setRadarAntennaGainDbV(50.0);

  _readVol->setFrequencyHz(_frequency);

  _readVol->loadSweepInfoFromRays();
  _readVol->computeFixedAnglesFromRays();

  // constrain the sweep data as appropriate
  
  if (_readFixedAngleLimitsSet) {
    if (_readVol->constrainByFixedAngle(_readMinFixedAngle, _readMaxFixedAngle,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - TdwrRadxFile::_setVolMetaData");
      _addErrStr("  No data found within fixed angle limits");
      _addErrDbl("  min fixed angle: ", _readMinFixedAngle);
      _addErrDbl("  max fixed angle: ", _readMaxFixedAngle);
      return -1;
    }
  } else if (_readSweepNumLimitsSet) {
    if (_readVol->constrainBySweepNum(_readMinSweepNum, _readMaxSweepNum,
                                      _readStrictAngleLimits)) {
      _addErrStr("ERROR - TdwrRadxFile::_setVolMetaData");
      _addErrStr("  No data found within sweep num limits");
      _addErrInt("  min sweep num: ", _readMinSweepNum);
      _addErrInt("  max sweep num: ", _readMaxSweepNum);
      return -1;
    }
  }

  _readVol->loadVolumeInfoFromRays();
  _readVol->reorderSweepsAscendingAngle();

  return 0;

}

/////////////////////////
// set the ray data
//
// Returns 0 on success, -1 on failure

int TdwrRadxFile::_setRayData(RadxRay &ray)
  
{

  // time

  time_t raySecs = _dhdr.timestamp;
  int rayNanoSecs = 0;
  if (_startTimeSecs == 0 && _endTimeSecs == 0) {
    _startTimeSecs = raySecs;
    _startNanoSecs = rayNanoSecs;
  } 
  _endTimeSecs = raySecs;
  _endNanoSecs = rayNanoSecs;
  ray.setTime(raySecs, rayNanoSecs);

  // scan info

  ray.setVolumeNumber(_dhdr.volume_count);
  int sweep_number = (_dhdr.scan_info_flag & 0xff000000) >> 24;
  ray.setSweepNumber(sweep_number);

  if (_dhdr.volume_flag & 0x8000) {
  }
  if (_dhdr.volume_flag & 0x4000) {
    ray.setStartOfVolumeFlag(true);
  }

  if (_dhdr.scan_info_flag & 0x800000) {
    ray.setEndOfSweepFlag(true);
  }
  if (_dhdr.scan_info_flag & 0x400000) {
    ray.setStartOfSweepFlag(true);
  }

  bool isRhi = false;
  int scan_strategy = (_dhdr.volume_flag & 0x00ff);
  if (scan_strategy == SCAN_RHI) {
    isRhi = true;
  }
  if (isRhi) {
    ray.setSweepMode(Radx::SWEEP_MODE_RHI);
  } else {
    ray.setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
  }

  ray.setPolarizationMode(Radx::POL_MODE_HORIZONTAL);
  ray.setPrtMode(Radx::PRT_MODE_FIXED);
  
  _iAz = _dhdr.integer_azimuth;
  ray.setAzimuthDeg(_dhdr.azimuth);
  ray.setElevationDeg(_dhdr.current_elevation);

  ray.setAntennaTransition(false);

  _peakPowerW = _dhdr.power_trans * 1000.0;
  _prtSecs = _dhdr.pri * 1.0e-6;
  _prf = 1.0 / _prtSecs;

  _nSamples = (_dhdr.dwell_flag & 0x0fff);
  _nGatesData = _dhdr.final_range_sample + 1;
  _nGatesDwell = _dhdr.rng_samples_per_dwell;
  _lastGate = _dhdr.load_shed_final_sample;
  _scanRate = _dhdr.angular_scan_rate;

  ray.setTrueScanRateDegPerSec(_scanRate);
  ray.setIsIndexed(true);
  ray.setAngleResDeg(1.0);
  ray.setNSamples(_nSamples);
  ray.setPulseWidthUsec(1.1);
  ray.setPrtSec(_prtSecs);
  ray.setPrtRatio(1.0);
  // ray.setNyquistMps(_nyquist);

  double maxRangeKm = (_prtSecs * Radx::LIGHT_SPEED) / 2000.0;
  ray.setUnambigRangeKm(maxRangeKm);

  bool isLongRange = false;
  if (_mhdr.message_id == LOW_PRF_BASE_DATA) {
    isLongRange = true;
    ray.setIsLongRange(true);
  } else if (_mhdr.message_id == NORMAL_PRF_BASE_DATA) {
    ray.setIsLongRange(false);
  }

  // range geom

  double startRangeKm = RANGE_TO_FIRST_GATE / 1000.0;
  double gateSpacingKm = NORMAL_PRF_RESOLUTION / 1000.0;
  if (isLongRange) {
    gateSpacingKm = LOW_PRF_RESOLUTION / 1000.0;
  }
  ray.setRangeGeom(startRangeKm, gateSpacingKm);

  // check we have good data

  bool dataIsGood = true;
  if (_dhdr.vol_elev_status_flag & 0x0000000f) {
    dataIsGood = false;
  }

  // set data fields
  
  if (isLongRange) {
    
    // allocate data arrays
    
    Radx::fl32 *dbz = new Radx::fl32[_nGatesData];
    Radx::fl32 *snr = new Radx::fl32[_nGatesData];

    // set data

    if (dataIsGood) {
      
      const Radx::ui08 *data =
        _dataBuf + sizeof(data_hdr_t);
      
      for (int ii = 0; ii < _nGatesData; ii++, data++) {
        dbz[ii] = *data * 0.5 - 30.0;
      }
      for (int ii = 0; ii < _nGatesData; ii++, data++) {
        snr[ii] = *data * 0.5;
      }

    } else {
      
      for (int ii = 0; ii < _nGatesData; ii++) {
        dbz[ii] = Radx::missingFl32;
        snr[ii] = Radx::missingFl32;
      }

    }

    // add data to rays
    
    ray.addField("DBZ", "dBZ", 
                 _nGatesData, Radx::missingFl32,
                 dbz, true);
    
    ray.addField("SNR", "dB", 
                 _nGatesData, Radx::missingFl32,
                 snr, true);
    
    // free up

    delete[] dbz;    
    delete[] snr;
    
  } else {

    // short range

    // allocate data arrays
    
    Radx::fl32 *dbz = new Radx::fl32[_nGatesData];
    Radx::fl32 *snr = new Radx::fl32[_nGatesData];
    Radx::fl32 *width = new Radx::fl32[_nGatesData];
    Radx::fl32 *uvel = new Radx::fl32[_nGatesData];
    Radx::fl32 *dvel = new Radx::fl32[_nGatesData];

    // set data

    if (dataIsGood) {
      
      const normal_data_t *data = (normal_data_t *)
        (_dataBuf + sizeof(data_hdr_t));
      
      for (int ii = 0; ii < _nGatesData; ii++, data++) {
        
        if (data->dbz == 0) {
          dbz[ii] = Radx::missingFl32;
        } else {
          dbz[ii] = data->dbz * 0.5 - 30.0;
        }

        if (data->snr == 0) {
          snr[ii] = Radx::missingFl32;
        } else {
          snr[ii] = data->snr * 0.5;
        }

        width[ii] = data->width * 0.25;

        Radx::ui16 uuvel = data->uvel;
        if (uuvel == 0) {
          uvel[ii] = Radx::missingFl32;
        } else {
          if (_tdwrIsSwapped) {
            ByteOrder::swap16(&uuvel,2);
          }
          uvel[ii] = (double) uuvel * 0.25 - 80.0;
        }

        Radx::ui16 ddvel = data->dvel;
        if (ddvel == 0) {
          dvel[ii] = Radx::missingFl32;
        } else {
          if (_tdwrIsSwapped) {
            ByteOrder::swap16(&ddvel,2);
          }
          dvel[ii] = (double) ddvel * 0.25 - 80.0;
        }
        
      }
      
    } else {
      
      for (int ii = 0; ii < _nGatesData; ii++) {
        dbz[ii] = Radx::missingFl32;
        snr[ii] = Radx::missingFl32;
        width[ii] = Radx::missingFl32;
        uvel[ii] = Radx::missingFl32;
        dvel[ii] = Radx::missingFl32;
      }

    }
    
    // add data to rays
    
    ray.addField("DBZ", "dBZ", 
                 _nGatesData, Radx::missingFl32,
                 dbz, true);
    
    ray.addField("SNR", "dB", 
                 _nGatesData, Radx::missingFl32,
                 snr, true);
    
    ray.addField("WIDTH", "m/s", 
                 _nGatesData, Radx::missingFl32,
                 width, true);
    
    ray.addField("VEL_RAW", "m/s", 
                 _nGatesData, Radx::missingFl32,
                 uvel, true);
    
    ray.addField("VEL_DEAL", "m/s", 
                 _nGatesData, Radx::missingFl32,
                 dvel, true);
    
    // free up

    delete[] dbz;    
    delete[] snr;
    delete[] width;
    delete[] uvel;
    delete[] dvel;
    
  }

  // convert to si16 with dynamic scaling

  ray.convertToSi16();
  
  return 0;

}

//////////////////////////////////////////////////////
// byte swapping

void TdwrRadxFile::_swap(message_hdr_t &mhdr)
{
  
  if (!_tdwrIsSwapped) return;

  ByteOrder::swap16(&mhdr.message_id,2);
  ByteOrder::swap16(&mhdr.message_length,2);
  
}

void TdwrRadxFile::_swap(data_hdr_t &dhdr)
{
  
  if (!_tdwrIsSwapped) return;

  ByteOrder::swap16(&dhdr.volume_count,2);
  ByteOrder::swap16(&dhdr.volume_flag,2);
  
  ByteOrder::swap16(&dhdr.power_trans,2);
  ByteOrder::swap16(&dhdr.playback_flag,2);
  
  ByteOrder::swap32(&dhdr.scan_info_flag,4);
  
  ByteOrder::swap32(&dhdr.current_elevation,4);
  ByteOrder::swap32(&dhdr.angular_scan_rate,4);
  
  ByteOrder::swap16(&dhdr.pri,2);
  ByteOrder::swap16(&dhdr.dwell_flag,2);
  ByteOrder::swap16(&dhdr.final_range_sample,2);
  ByteOrder::swap16(&dhdr.rng_samples_per_dwell,2);	
  
  ByteOrder::swap32(&dhdr.azimuth,4);
  ByteOrder::swap32(&dhdr.total_noise_power,4); 
  ByteOrder::swap32(&dhdr.timestamp,4);
  ByteOrder::swap16(&dhdr.base_data_type,2);
  ByteOrder::swap16(&dhdr.vol_elev_status_flag,2);
  
  ByteOrder::swap16(&dhdr.integer_azimuth,2);
  ByteOrder::swap16(&dhdr.load_shed_final_sample,2);
  
}

//////////////////////////////////////////////////////
// printing

void TdwrRadxFile::_print(message_hdr_t &mhdr,
                          ostream &out)
  
{
  
  out << "================ TDWR message header ================" << endl;
  out << "  message_id: 0x" << hex << mhdr.message_id << dec << endl;
  out << "  message_length: " << mhdr.message_length << endl;
  out << "=====================================================" << endl;
  
}


void TdwrRadxFile::_print(data_hdr_t &dhdr,
                          ostream &out)

{

  out << "================ TDWR data header ================" << endl;
  out << "  volume_number: " << dhdr.volume_count << endl;
  out << "  volume_flag: " << dhdr.volume_flag << endl;
  if (dhdr.volume_flag & 0x8000) {
    out << "  END-OF-VOL" << endl;
  }
  if (dhdr.volume_flag & 0x4000) {
    out << "  START-OF-VOL" << endl;
  }
  int scan_strategy = (dhdr.volume_flag & 0x00ff);
  out << "  scan_strategy: " << scan_strategy << endl;
  out << "  power_trans: " << dhdr.power_trans << endl;
  out << "  playback_flag: " << dhdr.playback_flag << endl;
  out << "  scan_info_flag: " << dhdr.scan_info_flag << endl;
  int sweep_number = (dhdr.scan_info_flag & 0xff000000) >> 24;
  out << "  sweep_number: " << sweep_number << endl;
  if (dhdr.scan_info_flag & 0x800000) {
    out << "  END-OF-TILT" << endl;
  }
  if (dhdr.scan_info_flag & 0x400000) {
    out << "  START-OF-TILT" << endl;
  }
  out << "  current_elevation: " << dhdr.current_elevation << endl;
  out << "  angular_scan_rate: " << dhdr.angular_scan_rate << endl;
  out << "  pri: " << dhdr.pri << endl;
  out << "  dwell_flag: " << dhdr.dwell_flag << endl;
  int n_samples = (dhdr.dwell_flag & 0x0fff);
  out << "  n_samples: " << n_samples << endl;
  out << "  final_range_sample: " << dhdr.final_range_sample << endl;
  int n_gates = dhdr.final_range_sample + 1;
  out << "  n_gates: " << n_gates << endl;
  out << "  rng_samples_per_dwell: " << dhdr.rng_samples_per_dwell << endl;
  out << "  azimuth: " << dhdr.azimuth << endl;
  out << "  total_noise_power (dBm): " << 10.0 * log10(dhdr.total_noise_power) << endl;
  out << "  timestamp: " << RadxTime::strm(dhdr.timestamp) << endl;
  out << "  base_data_type: " << dhdr.base_data_type << endl;
  out << "  vol_elev_status_flag: " << dhdr.vol_elev_status_flag << endl;
  out << "  integer_azimuth: " << dhdr.integer_azimuth << endl;
  out << "  load_shed_final_sample: " << dhdr.load_shed_final_sample << endl;
  out << "==================================================" << endl;
  
}
