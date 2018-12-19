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
// SigmetRadxFile.cc
//
// SigmetRadxFile object
//
// Support for radial data in SIGMET format
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2011
//
///////////////////////////////////////////////////////////////

#include <Radx/SigmetRadxFile.hh>
#include <Radx/SigmetData.hh>
#include <Radx/NcfRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxArray.hh>
#include <Radx/ByteOrder.hh>
#include <Radx/RadxComplex.hh>
#include <Radx/RadxRcalib.hh>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>
using namespace std;

const double SigmetRadxFile::_angleConversion = 360.0 / 65536.0;
int SigmetRadxFile::_volumeNumber = 0;
 
//////////////
// Constructor

SigmetRadxFile::SigmetRadxFile() : RadxFile()
                                   
{

  _readVol = NULL;
  _file = NULL;
  clear();

}

/////////////
// destructor

SigmetRadxFile::~SigmetRadxFile()

{
  clear();
}

/////////////////////////////////////////////////////////
// Check if specified file is SIGMET format
// Returns true if supported, false otherwise

bool SigmetRadxFile::isSupported(const string &path)

{
  
  if (isSigmet(path)) {
    return true;
  }

  return false;

}

////////////////////////////////////////////////////////////
// Check if this is a SIGMET RAW file
// Returns true on success, false on failure

bool SigmetRadxFile::isSigmet(const string &path)
  
{

  clear();
  
  // open file
  
  if (_openRead(path)) {
    _addErrStr("ERROR - SigmetRadxFile::Sigmet");
    return false;
  }
  
  // read first 32 bytes

  char hdr[32];
  if (fread(hdr, 32, 1, _file) != 1) {
    _close();
    return false;
  }
  _close();

  // check selected bytes

  if (hdr[0] == 27 &&  // format ID
      hdr[1] == 0 &&
      hdr[24] == 15 && // product ID = raw data set
      hdr[25] == 0) {
    _sigmetIsSwapped = false;
    return true;
  }

  if (hdr[0] == 0 &&  // format ID
      hdr[1] == 27 &&
      hdr[24] == 0 && // product ID = raw data set
      hdr[25] == 15) {
    _sigmetIsSwapped = true;
    return true;
  }

  return false;

}

/////////////////////////////////////////////////////////
// clear the data in the object

void SigmetRadxFile::clear()
  
{

  clearErrStr();

  _close();

  _nGates = 0;
  _sigmetIsSwapped = false;
  _nyquist = 0.0;
  _dualPrt = false;
  _prtRatio = 1.0;

  _sweepStartTime = 0;
  _startTimeSecs = 0;
  _endTimeSecs = 0;
  _startNanoSecs = 0;
  _endNanoSecs = 0;

  _isDualPol = false;
  _isHrdTailRadar = false;
  _isHrdLfRadar = false;
  _hasSubsecTime = false;

  _hrdSourcesSet = false;

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

int SigmetRadxFile::writeToDir(const RadxVol &vol,
                               const string &dir,
                               bool addDaySubDir,
                               bool addYearSubDir)
  
{

  // Writing SIGMET files is not supported
  // therefore write in CF Radial format instead

  cerr << "WARNING - SigmetRadxFile::writeToDir" << endl;
  cerr << "  Writing SIGMET format files not supported" << endl;
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

int SigmetRadxFile::writeToPath(const RadxVol &vol,
                                const string &path)
  
{

  // Writing SIGMET files is not supported
  // therefore write in CF Radial format instead

  cerr << "WARNING - SigmetRadxFile::writeToPath" << endl;
  cerr << "  Writing SIGMET format files not supported" << endl;
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

int SigmetRadxFile::readFromPath(const string &path,
                                 RadxVol &vol)
  
{

  _initForRead(path, vol);

  // initialize with default setup

  _readVol->setInstrumentType(Radx::INSTRUMENT_TYPE_RADAR);
  _readVol->setPlatformType(Radx::PLATFORM_TYPE_FIXED);
  _readVol->setPrimaryAxis(Radx::PRIMARY_AXIS_Z);

  // open file

  if (_openRead(path)) {
    _addErrStr("ERROR - SigmetRadxFile::readFromPath");
    return -1;
  }

  // set volume number - increments per file

  _volumeNumber++;
  
  // read in product and ingest headers
  
  if (_readHeaders(false, cerr)) {
    _addErrStr("ERROR - SigmetRadxFile::readFromPath");
    _addErrStr("  Reading header, file: ", _pathInUse);
    return -1;
  }

  // read the data, a sweep at a time
  
  int iret = 0;
  while (!feof(_file)) {
    
    if (_readSweepData(false, cerr) == 0) {
      
      if (_nFields > 0) {
        if (_processSweep(false, false, cerr)) {
          _addErrStr("ERROR - SigmetRadxFile::readFromPath");
          _addErrStr("  Processing sweep, file: ", _pathInUse);
          iret = -1;
        }
      }

    } else {
      
      iret = -1;
      
    }

  } // while

  if (_readVol->getNRays() == 0) {
    _addErrStr("ERROR - SigmetRadxFile::readFromPath");
    _addErrStr("  No rays found, file: ", _pathInUse);
    return -1;
  }

  // sort rays by time order if we have accurate time

  if (_hasSubsecTime) {
    _readVol->sortRaysByTime();
  }
  
  // finalize the read volume
  
  if (_finalizeReadVolume()) {
    return -1;
  }

  if (_verbose) {
    _readVol->print(cerr);
  }
  
  // set the packing from the rays

  _readVol->setPackingFromRays();

  // add to paths used on read
  
  _readPaths.push_back(path);

  // set internal format

  _fileFormat = FILE_FORMAT_FORAY_NC;

  return iret;

}

//////////////////////////////////////
// open netcdf file for reading
// Returns 0 on success, -1 on failure

int SigmetRadxFile::_openRead(const string &path)
  
{

  _close();
  _file = fopen(path.c_str(), "r");
  
  // Check that constructor succeeded
  
  if (_file == NULL) {
    int errNum = errno;
    _addErrStr("ERROR - SigmetRadxFile::_openRead");
    _addErrStr("  Cannot open file for reading, path: ", path);
    _addErrStr("  ", strerror(errNum));
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////
// close netcdf file if open
// remove error object if it exists

void SigmetRadxFile::_close()
  
{
  
  // close file if open
  
  if (_file) {
    fclose(_file);
    _file = NULL;
  }
  
}

/////////////////////////////////////////////////////////////
// finalize the read volume

int SigmetRadxFile::_finalizeReadVolume()
  
{

  // set the meta data on the volume

  _setVolMetaData();
  
  // remove rays with all missing data, if requested

  if (_readRemoveRaysAllMissing) {
    _readVol->removeRaysWithDataAllMissing();
  }

  // apply goeref info if applicable
  // but do not force this - if previously applied do nothing

  if (_readApplyGeorefs) {
    _readVol->applyGeorefs(false);
  }

  // load the sweep information from the rays

  _readVol->loadSweepInfoFromRays();
  
  // constrain the sweep data as appropriate
  
  if (_readFixedAngleLimitsSet) {
    if (_readVol->constrainByFixedAngle(_readMinFixedAngle, _readMaxFixedAngle,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - SigmetRadxFile::_finalizeReadVolume");
      _addErrStr("  No data found within fixed angle limits");
      _addErrDbl("  min fixed angle: ", _readMinFixedAngle);
      _addErrDbl("  max fixed angle: ", _readMaxFixedAngle);
      return -1;
    }
  } else if (_readSweepNumLimitsSet) {
    if (_readVol->constrainBySweepNum(_readMinSweepNum, _readMaxSweepNum,
                                      _readStrictAngleLimits)) {
      _addErrStr("ERROR - SigmetRadxFile::_finalizeReadVolume");
      _addErrStr("  No data found within sweep num limits");
      _addErrInt("  min sweep num: ", _readMinSweepNum);
      _addErrInt("  max sweep num: ", _readMaxSweepNum);
      return -1;
    }
  }

  // check for indexed rays, set info on rays
  
  _readVol->checkForIndexedRays();

  // load volume metadata from rays

  _readVol->loadVolumeInfoFromRays();

  return 0;

}

/////////////////////////////////////////////////////////
// print summary after read

void SigmetRadxFile::print(ostream &out) const
  
{
  
  out << "=============== SigmetRadxFile ===============" << endl;
  RadxFile::print(out);
  out << "===========================================" << endl;

}

////////////////////////////////////////////////////////////
// Print native data in uf file
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int SigmetRadxFile::printNative(const string &path, ostream &out,
                                bool printRays, bool printData)
  
{

  clear();
  RadxVol vol;
  _readVol = &vol;
  _readVol->clear();
  _pathInUse = path;
  vol.setPathInUse(_pathInUse);
  _readPaths.clear();

  // open file

  if (_openRead(path)) {
    _addErrStr("ERROR - SigmetRadxFile::printNative");
    return -1;
  }

  // read in product and ingest headers
  
  if (_readHeaders(true, out)) {
    _addErrStr("ERROR - SigmetRadxFile::printNative");
    _addErrStr("  Reading header, file: ", _pathInUse);
    return -1;
  }

  if (!printRays) {
    return 0;
  }
  
  // read the data, a sweep at a time
  
  int iret = 0;
  while (!feof(_file)) {

    if (_readSweepData(true, out) == 0) {
      if (_processSweep(true, printData, out)) {
        _addErrStr("ERROR - SigmetRadxFile::printNative");
        _addErrStr("  Processing sweep, file: ", _pathInUse);
        iret = -1;
      }
    } else {
      iret = -1;
    }

  } // while
  
  return iret;

}

//////////////////////////////
// read in a record

int SigmetRadxFile::_readRecord()

{

  if (fread(_record, 1, RAW_RECORD_LEN, _file) != RAW_RECORD_LEN) {
    if (!feof(_file)) {
      int errNum = errno;
      _addErrStr("ERROR reading raw record");
      _addErrStr("  ", strerror(errNum));
    }
    return -1;
  }
  
  return 0;
  
}

/////////////////////////////////////
// read in product and ingest headers

int SigmetRadxFile::_readHeaders(bool doPrint, ostream &out)

{

  // product configuration

  if (_readRecord()) {
    _addErrStr("ERROR - SigmetRadxFile::_readHeaders");
    _addErrStr("  Cannot read in product header");
    return -1;
  }

  memcpy(&_prodHdr, _record, sizeof(_prodHdr));
  
  // check is swapping is needed

  if (_prodHdr.id_hdr.id == 27) {
    // no swapping needed
    _sigmetIsSwapped = false;
    if (doPrint || _debug) {
      out << "Note: byte swapping is not needed for this file" << endl;
    }
  } else {
    _sigmetIsSwapped = true;
    _swap(_prodHdr);
    if (_prodHdr.id_hdr.id == 27) {
      if (doPrint || _debug) {
        out << "Note: byte swapping is on for this file" << endl;
      }
    } else {
      // bad data
      _addErrStr("ERROR - SigmetRadxFile::_readHeaders");
      _addErrStr("  Cannot recognize product header, even after swapping");
      _sigmetIsSwapped = false;
      return -1;
    }
  }

  if (doPrint || _verbose) {
    _print(_prodHdr, out);
  }

  // ingest headers

  if (_readRecord()) {
    _addErrStr("ERROR - SigmetRadxFile::_readHeaders");
    _addErrStr("  Cannot read in product header");
    return -1;
  }

  memcpy(&_inHdr, _record, sizeof(_inHdr));
  _swap(_inHdr);
  if (doPrint || _verbose) {
    _print(_inHdr, out);
  }
  _nbytesExtendedHdr =
    _inHdr.ingest_conf.nbytes_in_ext_hdr - sizeof(ray_header_t);

  // compute derived quantities

  _pulseWidthUs = _prodHdr.end.pulse_width_us_100 / 100.0;
  _wavelengthCm = _prodHdr.end.wavelength_cm_100 / 100.0;
  _wavelengthM = _wavelengthCm / 100.0;
  _prf = _prodHdr.end.prf_hz;
  _prtSec = 1.0 / _prf;
  _unambigRangeKm = (_prtSec * Radx::LIGHT_SPEED) / 2000.0;

  // nyquist depends on pulsing scheme

  _nyquist = _wavelengthM / (4.0 * _prtSec);
  switch (_prodHdr.end.trig_rate_scheme) {
    case PRF_DUAL_4_5:
      _prtRatio = 5.0 / 4.0;
      _nyquist *= 4.0;
      _dualPrt = true;
      _prtRatio = 5.0 / 4.0;
      break;
    case PRF_DUAL_3_4:
      _nyquist *= 3.0;
      _dualPrt = true;
      _prtRatio = 4.0 / 3.0;
      break;
    case PRF_DUAL_2_3:
      _nyquist *= 2.0;
      _dualPrt = true;
      _prtRatio = 3.0 / 2.0;
      break;
    default:
      _dualPrt = false;
  }

  // NOAA HRD radar?
  
  _isHrdTailRadar = false;
  _isHrdLfRadar = false;
  string hardwareName = Radx::makeString(_prodHdr.end.hardware_name, 16);
  if (_readRadarNum == 1) {
    _isHrdLfRadar = true;
  } else if (_readRadarNum == 2) {
    _isHrdTailRadar = true;
  } else if (hardwareName.find("-Tail") != string::npos) {
    _isHrdTailRadar = true;
  } else if (hardwareName.find("-TM") != string::npos) {
    _isHrdTailRadar = true;
  } else if (hardwareName.find("-TS") != string::npos) {
    _isHrdTailRadar = true;
  } else if (hardwareName.find("noaa1-master") != string::npos) {
    _isHrdTailRadar = true;
  } else if (hardwareName.find("-LF") != string::npos) {
    _isHrdLfRadar = true;
  }

  if (doPrint || _debug) {
    out << "============== summary so far ===============" << endl;
    out << "  pulseWidthUs: " << _pulseWidthUs << endl;
    out << "  wavelengthCm: " << _wavelengthCm << endl;
    out << "  wavelengthM: " << _wavelengthM << endl;
    out << "  prf: " << _prf << endl;
    out << "  prtSec: " << _prtSec << endl;
    out << "  nyquist: " << _nyquist << endl;
    out << "  unambigRangeKm: " << _unambigRangeKm << endl;
    out << "  nbytesExtendedHdr: " << _nbytesExtendedHdr << endl;
    if (_isHrdTailRadar) {
      out << "  HRD Tail Radar: true" << endl;
    } else if (_isHrdLfRadar) {
      out << "  HRD LF Radar: true" << endl;
    }
    out << "  sigmetIsSwapped: " << (_sigmetIsSwapped?"Y":"N") << endl;
    out << "=============================================" << endl;
  }

  return 0;

}

/////////////////////////////////////
// read in data from a sweep

int SigmetRadxFile::_readSweepData(bool doPrint, ostream &out)

{

  // read in first record in sweep

  if (_readRecord()) {
    if (feof(_file)) {
      return 0;
    }
    _addErrStr("ERROR - SigmetRadxFile::_readSweepData");
    return -1;
  }
  Radx::ui08 *ptr = _record;

  // check raw header
  
  memcpy(&_rawHdr, ptr, sizeof(_rawHdr));
  _swap(_rawHdr);
  if (doPrint || _verbose) {
    _print(_rawHdr, out);
  }
  ptr += sizeof(_rawHdr);
  _sweepNumber = _rawHdr.sweep_num;
  _sweepIndex = _sweepNumber - 1;
  
  // set ingest data headers for each field
  
  _nRaysSweep = 0;
  _inDatHdrs.clear();
  _nBytesData = 0;
  while (ptr < (Radx::ui08 *) _record + RAW_RECORD_LEN) {
    ingest_data_header_t inDatHdr;
    memcpy(&inDatHdr, ptr, sizeof(inDatHdr));
    _swap(inDatHdr);
    if (inDatHdr.id_hdr.id != 24 ||
	inDatHdr.sweep_num != _sweepNumber) {
      break;
    }
    if (doPrint || _verbose) {
      out << "====>>> field number: " << _inDatHdrs.size() << endl;
      _print(inDatHdr, out);
    }

    sigmet_time_t time = inDatHdr.time;
    RadxTime stime(time.year, time.month, time.day, 0, 0, time.sec);
    int msecs = time.msecs & 1023; // lower 10 bits
    _sweepStartTime.set(stime.utime(), msecs / 1000.0);

    _fixedAngle = _binAngleToDouble(inDatHdr.fixed_angle);
    _nRaysSweep = inDatHdr.n_rays_written;
    int nBytesThisField = inDatHdr.id_hdr.nbytes - sizeof(ingest_data_header_t);
    _nBytesData += nBytesThisField;
    ptr += sizeof(inDatHdr);
    _inDatHdrs.push_back(inDatHdr);
    if (_inDatHdrs.size() == 1) {
      _inDatHdr0 = inDatHdr;
    }
  } // while

  // compute the number of fields,
  // i.e. those headers with non-zero data_codes

  _nFields = 0;
  for (size_t ii = 0; ii < _inDatHdrs.size(); ii++) {
    if (_inDatHdrs[ii].data_code != FIELD_EXT_HDR) {
      _nFields++;
    }
  }
  
  if (doPrint || _verbose) {
    out << "==>> nFields: " << _nFields << endl;
    out << "==>> nBytesData: " << _nBytesData << endl;
  }

  if (_nFields < 1) {
    if (_debug) {
      cerr << "WARNING - SigmetRadxFile::_readSweepData()" << endl;
      cerr << "  No fields found" << endl;
    }
    return 0;
  }
  
  for (size_t ii = 0; ii < _inDatHdrs.size(); ii++) {
    const ingest_data_header_t &inDatHdr = _inDatHdrs[ii];
    int fieldId = inDatHdr.data_code;
    if (fieldId != FIELD_EXT_HDR) {
      string name = _fieldId2Name(fieldId);
      string units = _fieldId2Units(fieldId);
      double scale = 1.0, bias = 0.0;
      _fieldId2ScaleBias(fieldId, scale, bias);
      if (doPrint || (_debug && _sweepIndex == 0) || _verbose) {
        out << "Found the following fields: " << endl;
        out << "  fieldId, name, units, byteWidth, scale, bias: "
             << fieldId << ", " << name << ", " << units << ", "
             << inDatHdr.bits_per_bin / 8 << ", "
             << scale << ", " << bias << endl;
      }
    }
  }

  // add remaining data to buffer
  
  _inBuf.reset();
  int nBytesLeft = RAW_RECORD_LEN - (ptr - _record);
  if (doPrint || _verbose) {
    out << "first record, nBytesLeft: " << nBytesLeft << endl;
  }
  _inBuf.add(ptr, nBytesLeft);

  // read remaining records for this sweep
  
  while (!feof(_file)) {

    // save current position, for stepping back

    long pos = ftell(_file);

    // read in next record

    if (_readRecord()) {
      if (feof(_file)) {
        break;
      }
      _addErrStr("ERROR - SigmetRadxFile::_readSweepData");
      return -1;
    }
    ptr = _record;

    // check header for sweep number change

    memcpy(&_rawHdr, ptr, sizeof(_rawHdr));
    if (doPrint || _verbose) {
      _print(_rawHdr, out);
    }
    ptr += sizeof(_rawHdr);
    
    if (_rawHdr.sweep_num != _sweepNumber) {
      // gone one record too far, step back, return
      fseek(_file, pos, SEEK_SET);
      break;
    }
    
    // add to in buffer
    
    int nBytesLeft = RAW_RECORD_LEN - (ptr - _record);
    if (doPrint || _verbose) {
      out << "later record, nBytesLeft: " << nBytesLeft << endl;
    }
    _inBuf.add(ptr, nBytesLeft);

  } // while

  // reset the data buffer

  _dataBuf.reset();

  // create an array filled with zeros, for copying as required

  Radx::si16 zeros[65536];
  memset(zeros, 0, 65536 * sizeof(Radx::si16));

  // read input buffer, uncompressing

  int nComp16 = _inBuf.getLen() / sizeof(Radx::si16);
  Radx::si16 *comp = (Radx::si16 *) _inBuf.getPtr();
  
  if ((doPrint && _debug) || _verbose) {
    out << "nComp16: " << nComp16 << endl;
  }

  int count = 0;
  int prevCount = 0;
  int nRayFieldsFound = 0;
  _nBytesRayField.clear();

  for (int ii = 0; ii < nComp16; ii++) {
    Radx::si16 code = comp[ii];
    if ((doPrint && _debug) || _verbose) {
      out << "ii, code, nRayFields: "
	   << ii << ", " << code <<  ", "
	   << nRayFieldsFound << " - ";
    }
    if (code == 1) {
      int nBytesField = (count - prevCount) * 2;
      prevCount = count;
      if ((doPrint && _debug) || _verbose) {
	out << "  ==>> end of field, nBytesField: " << nBytesField << " <<==";
      }
      if (nBytesField > 0) {
        nRayFieldsFound++;
        _nBytesRayField.push_back(nBytesField);
        if (nRayFieldsFound == _nRaysSweep * (int) _inDatHdrs.size()) {
          break;
        }
      }
    } else if (code >= 0x0003) {
      Radx::si16 nZeros = code;
      if ((doPrint && _debug) || _verbose) {
	out << " " << nZeros << " zeros";
      }
      count += nZeros;
      // add zeros to data buffer
      _dataBuf.add(zeros, nZeros * sizeof(Radx::si16));
    } else {
      Radx::si16 nDataWords = (code & 0x7FFF);
      if(nDataWords >= 1) {
	if ((doPrint && _debug) || _verbose) {
	  out << " " << nDataWords << "  data words follow";
	}

        // add data to data buffer

        _dataBuf.add(comp + ii + 1, nDataWords * sizeof(Radx::si16));
	ii += nDataWords;
	count += nDataWords;
      }
    }
    if ((doPrint && _debug) || _verbose) {
      out << ", count: " << count << endl;
    }
  } // ii

  if (_dataBuf.getLen() < sizeof(ray_header_t)) {
    _addErrStr("ERROR - SigmetRadxFile::_readSweepData");
    _addErrStr("  No data in sweep");
    return -1;
  }

  // get the number of gates from the ray header at
  // the start of the data
  
  ray_header_t rayHdr;
  memcpy(&rayHdr, _dataBuf.getPtr(), sizeof(rayHdr));
  _swap(rayHdr);
  if (doPrint || _debug) {
    out << "====== rayHdr at start of sweep ======" << endl;
    _print(rayHdr, out);
    out << "======================================" << endl;
  }

  return 0;

}

/////////////////////////////////////
// process a sweep of data
//
// Returns 0 on success, -1 on failure

int SigmetRadxFile::_processSweep(bool doPrint, bool printData, ostream &out)

{

  // set the ray info, so that we know where the rays start etc
  // also determining _nGates

  _setRayInfo();
  
  if (doPrint || _debug) {
    out << "Processing sweep: " << endl;
    out << "  nGates, nFields: "
        << _nGates << ", " << _nFields << endl;
  }
  
  // find start index with earliest time
  // the rays are stored in azimuth order, but the sweep may have started
  // somewhere other than north, so we need to find the starting point

  int startIndex = 0;
  for (size_t iray = 1; iray < _rayInfo.size(); iray++) {
    if (_rayInfo[iray].hdr.seconds < _rayInfo[iray-1].hdr.seconds) {
      startIndex = iray;
      break;
    }
  }
  
  // initialize pointer to start of data buf

  for (size_t iray = 0; iray < _rayInfo.size(); iray++) {

    // get reference to stored ray, taking the start index
    // into account

    int rayIndex = (iray + startIndex) % _rayInfo.size();
    const RayInfo &rayInfo = _rayInfo[rayIndex];
    Radx::ui08 *rayPtr = rayInfo.offset;

    // create a new Radx ray

    RadxRay *ray = new RadxRay();

    // load up field data

    int fieldNum = 0;

    for (size_t ifield = 0; ifield < _inDatHdrs.size(); ifield++) {
      
      // get ray header

      ray_header_t rayHdr;
      memcpy(&rayHdr, rayPtr, sizeof(rayHdr));
      _swap(rayHdr);

      if (doPrint || _verbose) {
        out << "====== rayHdr ======" << endl;
        _print(rayHdr, out);
        out << "====================" << endl;
      }

      // if first field, set ray meta data
      
      if (ifield == 0) {
        _setRayMetadata(*ray, rayHdr);
      }

      // handle extended headers - data_code == 0

      const ingest_data_header_t &inDatHdr = _inDatHdrs[ifield];
      if (inDatHdr.data_code == FIELD_EXT_HDR) {
        // extended header instead of field
        int fieldLen = rayInfo.nBytesField[ifield];
        _handleExtendedHeader(ray, rayIndex, rayInfo, 
                              rayPtr, fieldLen, 
                              doPrint, out);
        rayPtr += fieldLen;
        continue;
      }

      int nGates = rayHdr.n_gates;
      int byteWidth = inDatHdr.bits_per_bin / 8;
      Radx::ui08 *dataPtr = rayPtr + sizeof(ray_header_t);
      
      // check that we need this field
      
      int fieldId = inDatHdr.data_code;
      string name = _fieldId2Name(fieldId);
      string units = _fieldId2Units(fieldId);

      if (doPrint || isFieldRequiredOnRead(name)) {
    
        // add the field
        
        double scale = 1.0, bias = 0.0;
        _fieldId2ScaleBias(fieldId, scale, bias); // get unsigned scale/bias
        _convertBias2Signed(fieldId, scale, bias); // convert bias to signed
        _checkDualPol(fieldId); // check if this is a dual pol field
        
        RadxField *field = new RadxField(name, units);
        field->copyRangeGeom(*ray);

        if (fieldId == FIELD_KDP) {

          // special case for 1-byte KDP
          // turn into 2-byte KDP

          scale = 0.01;
          bias = -327.68;
          Radx::si16 *sdata = new Radx::si16[nGates];
          Radx::si16 *sptr = sdata;
          Radx::ui08 *uptr = (Radx::ui08 *) dataPtr;
          for (int ii = 0; ii < nGates; ii++, sptr++, uptr++) {
            double ival = *uptr;
            double kdpVal = 0.0;
            int oval = 0;
            if (ival > 0) {
              if (ival > 128) {
                kdpVal = (0.25 * pow(600.0, ((ival - 129.0) / 126.0))) / _wavelengthCm;
              } else if (ival < 128) {
                kdpVal = (-0.25 * pow(600.0, ((127.0 - ival) / 126.0))) / _wavelengthCm;
              }
              oval = (int) floor((kdpVal - bias) / scale + 0.5);
              if (oval < 0) {
                oval = 0;
              } else if (oval > 65535) {
                oval = 65535;
              }
            }
            *sptr = (Radx::si16) oval;
          }

          field->setTypeSi16(-32768, scale, bias);
          field->addDataSi16(nGates, sdata);
          delete[] sdata;
          
        } else if (fieldId == FIELD_SQI ||
                   fieldId == FIELD_RHOHV ||
                   fieldId == FIELD_RHOH ||
                   fieldId == FIELD_RHOH) {
          
          // special case for 1-byte fields that range from 0 to 1
          //   val = sqrt((n-1)/253)
          // turn into 2-byte fields from 0 to 1

          scale = 0.001;
          bias = 0.0;
          Radx::si16 *sdata = new Radx::si16[nGates];
          Radx::si16 *sptr = sdata;
          Radx::ui08 *uptr = (Radx::ui08 *) dataPtr;
          for (int ii = 0; ii < nGates; ii++, sptr++, uptr++) {
            double ival = *uptr;
            int oval = 0;
            if (ival >= 1.0) {
              double dval = sqrt((ival - 1.0) / 253.0);
              oval = (int) floor((dval - bias) / scale + 0.5);
              if (oval < 0) {
                oval = 0;
              } else if (oval > 65535) {
                oval = 65535;
              }
            }
            *sptr = (Radx::si16) oval;
          }

          field->setTypeSi16(-32768, scale, bias);
          field->addDataSi16(nGates, sdata);
          delete[] sdata;

        } else if (byteWidth == 1) {

          // byte width is 1

          field->setTypeSi08(-128, scale, bias);
          Radx::si08 *sdata = new Radx::si08[nGates];
          Radx::si08 *sptr = sdata;
          Radx::ui08 *uptr = (Radx::ui08 *) dataPtr;
          for (int ii = 0; ii < nGates; ii++, sptr++, uptr++) {
            int ival = *uptr - 128;
            *sptr = (Radx::si08) ival;
          }
          field->addDataSi08(nGates, sdata);
          delete[] sdata;

        } else {

          // byte width is 2
          if (_sigmetIsSwapped) {
            // swap in place
            _swap((Radx::ui16 *) dataPtr, nGates);
          }
          field->setTypeSi16(-32768, scale, bias);
          Radx::si16 *sdata = new Radx::si16[nGates];
          Radx::si16 *sptr = sdata;
          Radx::ui16 *uptr = (Radx::ui16 *) dataPtr;
          for (int ii = 0; ii < nGates; ii++, sptr++, uptr++) {
            int ival = *uptr - 32768;
            *sptr = (Radx::si16) ival;
          }
          field->addDataSi16(nGates, sdata);
          delete[] sdata;
        }
        
        ray->addField(field);
        
        if (doPrint && printData) {
          field->printWithData(out);
        }
        
      } // if (needField)
      
      rayPtr += rayInfo.nBytesField[ifield];
      fieldNum++;
      
    } // ifield
    
    if (doPrint || _verbose) {
      ray->print(out);
    }

    // add to vector

    _readVol->addRay(ray);

  } // iray
  
  return 0;

}

//////////////////////////////////////
// Handle extended header for ray

int SigmetRadxFile::_handleExtendedHeader(RadxRay *ray,
                                          int rayIndex,
                                          const RayInfo &rayInfo,
                                          const Radx::ui08 *rayPtr,
                                          int fieldLen,
                                          bool doPrint,
                                          ostream &out)
  
{
  
  // this 'field' is actually an extended header

  // get ray header
  
  ray_header_t rayHdr;
  memcpy(&rayHdr, rayPtr, sizeof(rayHdr));
  _swap(rayHdr);
  
  if (doPrint || _verbose) {
    out << "====== Extended header ======" << endl;
    out << "====== rayHdr ======" << endl;
    _print(rayHdr, out);
    out << "=============================" << endl;
  }

  int rayHdrLen = sizeof(ray_header_t);
  int extBufLen = fieldLen - rayHdrLen;
  if (doPrint || _verbose) {
    out << "  fieldLen: " << fieldLen << endl;
    out << "  rayHdrLen: " << rayHdrLen << endl;
    out << "  extBufLen: " << extBufLen << endl;
  }
  
  // get extended header type

  dsp_data_mask_t data_mask = _inHdr.task_conf.dsp_info.data_mask;
  _swap(data_mask);
  int extendedHeaderType = data_mask.xhdr_type;
  if (doPrint || _verbose) {
    _print("  data_mask: ", data_mask, out);
    out << "  extendedHeaderType: " << extendedHeaderType << endl;
  }

  if (extendedHeaderType == 0) {

    if (extBufLen <= (int) sizeof(ext_header_ver0)) {

      ext_header_ver0 xh0;
      memset(&xh0, 0, sizeof(xh0));
      memcpy(&xh0, rayPtr + rayHdrLen, extBufLen);
      _swap(xh0);
      if (doPrint || _verbose) {
        _print(xh0, out);
      }
      
    }

  } else if (extendedHeaderType == 1) {

    if (extBufLen >= (int) sizeof(ext_header_ver1)) {
      
      ext_header_ver1 xh1;
      memset(&xh1, 0, sizeof(xh1));
      memcpy(&xh1, rayPtr + rayHdrLen, sizeof(xh1));
      _swap(xh1);
      if (doPrint || _verbose) {
        _print(xh1, out);
      }
      RadxGeoref georef;
      _setGeoref(rayHdr, xh1, ray, georef);
      ray->setGeoref(georef);

    }

  } else if (extendedHeaderType == 2) {

    if (extBufLen == sizeof(hrd_tdr_ext_header_t)) {

      // probably HRD extended header

      hrd_tdr_ext_header_t hrd;
      memcpy(&hrd, rayPtr + rayHdrLen, sizeof(hrd));
      _swap(hrd);
      if (doPrint || _verbose) {
        _print(hrd, out);
      }
      RadxGeoref georef;
      _setGeoref(rayHdr, hrd, ray, georef);
      ray->setGeoref(georef);
      
    } else {

      // client specified external header type 2

      if (extBufLen >= (int) sizeof(ext_header_ver2)) {
        
        ext_header_ver2 xh2;
        memset(&xh2, 0, sizeof(xh2));
        memcpy(&xh2, rayPtr + rayHdrLen, sizeof(xh2));
        _swap(xh2);
        if (doPrint || _verbose) {
          _print(xh2, out);
        }
        
      }

    }

  }
  
  // for (int iii = 0; iii < (fieldLen / 2) - 1; iii++) {
  //   Radx::si16 s16;
  //   Radx::si32 s32;
  //   memcpy(&s16, rayPtr + iii * 2, sizeof(s16));
  //   memcpy(&s32, rayPtr + iii * 2, sizeof(s32));
  //   fprintf(stderr,
  //           "WWWWWW iii, s16, s32, ang16, ang32: " 
  //           "%3d %6d %13d %10.4f %10.4f\n",
  //           iii, s16, s32, 
  //           _binAngleToDouble(s16),
  //           _binAngleToDouble(s32));
  // }

  return 0;

}

/////////////////////////////////////
// Set georef from extended header

void SigmetRadxFile::_setGeoref(const ray_header_t &rayHdr,
                                const ext_header_ver1 &xh1,
                                RadxRay *ray,
                                RadxGeoref &georef)

{

  RadxTime rayTime = _sweepStartTime + xh1.msecs_since_sweep_start / 1000.0;
  time_t raySecs = rayTime.utime();
  int rayNanoSecs = (int) (rayTime.getSubSec() * 1.0e9 + 0.5);
  ray->setTime(rayTime);
  _hasSubsecTime = true;

  georef.setTimeSecs(raySecs);
  georef.setNanoSecs(rayNanoSecs);
  georef.setLongitude(_binAngleToDouble(xh1.lon));
  georef.setLatitude(_binAngleToDouble(xh1.lat));
  georef.setAltitudeKmMsl(xh1.alt_m_msl / 1000.0);
  georef.setEwVelocity(xh1.vel_e_cm_per_sec / 100.0);
  georef.setNsVelocity(xh1.vel_n_cm_per_sec / 100.0);
  georef.setVertVelocity(xh1.vel_up_cm_per_sec / 100.0);
  georef.setHeading(_binAngleToDouble(xh1.heading));
  georef.setRoll(_binAngleToDouble(xh1.roll));
  georef.setPitch(_binAngleToDouble(xh1.pitch));
  // georef.setDrift(0.0);
  // georef.setRotation(0.0);
  // georef.setTilt(0.0);
  // georef.setEwWind(0.0);
  // georef.setNsWind(0.0);
  // georef.setVertWind(0.0);
  georef.setHeadingRate(_binAngleToDouble(xh1.heading_rate));
  georef.setPitchRate(_binAngleToDouble(xh1.pitch_rate));

  // update metadata

  _readVol->setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT);

  if (_isHrdTailRadar) {
    // tail radar for NOAA aircraft
    _readVol->setPrimaryAxis(Radx::PRIMARY_AXIS_Y_PRIME);
    _readVol->setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_TAIL);
  } else if (_isHrdLfRadar) {
    // lower fuselage radar for NOAA aircraft
    _readVol->setPrimaryAxis(Radx::PRIMARY_AXIS_Z_PRIME);
    _readVol->setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_BELLY);
  }
  
}

/////////////////////////////////////////
// Set georef from HRD extended header

void SigmetRadxFile::_setGeoref(const ray_header_t &rayHdr,
                                const hrd_tdr_ext_header_t &hrd,
                                RadxRay *ray,
                                RadxGeoref &georef)

{

  // set sources on reading in HRD extended headers

  if (!_hrdSourcesSet) {
    _setHrdSources();
  }

  // which platform are we on?

  string hardware = 
    Radx::makeString(_prodHdr.end.hardware_name, 16);
  bool isP3 = true;
  if (hardware.find("N49") != string::npos) {
    isP3 = false;
  }

  // time

  RadxTime rayTime = _sweepStartTime + hrd.msecs_since_sweep_start / 1000.0;
  time_t raySecs = rayTime.utime();
  int rayNanoSecs = (int) (rayTime.getSubSec() * 1.0e9);
  ray->setTime(rayTime);
  _hasSubsecTime = true;
  
  // posn

  double longitude = _binAngleToDouble(hrd.irs_long);
  double latitude = _binAngleToDouble(hrd.irs_lat);
  double altitudeKmMsl = hrd.gps_alt_cm / 1.0e5;
  double altitudeKmAgl = hrd.aamps_radar_alt_cm / 1.0e5;

  if (!isP3) {
    longitude = _binAngleToDouble(hrd.gps_long);
    latitude = _binAngleToDouble(hrd.gps_lat);
  }

  if (_hrdPosnSource == USE_GPS_POSN) {
    longitude = _binAngleToDouble(hrd.gps_long);
    latitude = _binAngleToDouble(hrd.gps_lat);
  } else if (_hrdPosnSource == USE_AAMPS_POSN) {
    longitude = _binAngleToDouble(hrd.aamps_long);
    latitude = _binAngleToDouble(hrd.aamps_lat);
    altitudeKmMsl = hrd.aamps_alt_cm / 1.0e5;
  }

  // platform velocity

  double ewVelocity = hrd.irs_vel_e_cm_per_sec / 100.0;
  double nsVelocity = hrd.irs_vel_n_cm_per_sec / 100.0;
  double vertVelocity = hrd.irs_vel_v_cm_per_sec / 100.0;

  if (_hrdPosnSource == USE_GPS_POSN) {
    ewVelocity = hrd.gps_vel_e_cm_per_sec / 100.0;
    nsVelocity = hrd.gps_vel_n_cm_per_sec / 100.0;
    vertVelocity = hrd.gps_vel_v_cm_per_sec / 100.0;
  }

  // attitude and rates

  double heading = _binAngleToDouble(hrd.irs_heading);
  double roll = _binAngleToDouble(hrd.irs_roll);
  double pitch = _binAngleToDouble(hrd.irs_pitch);
  double drift = _binAngleToDouble(hrd.irs_drift);
  double track = _binAngleToDouble(hrd.irs_tru_track);
  double pitchRate = _binAngleToDouble(hrd.irs_pitch_r);
  double rollRate = _binAngleToDouble(hrd.irs_roll_r);
  double yawRate = _binAngleToDouble(hrd.irs_yaw_r);

  if (_hrdPosnSource == USE_AAMPS_POSN) {
    roll = _binAngleToDouble(hrd.aamps_roll);
    pitch = _binAngleToDouble(hrd.aamps_pitch);
    drift = _binAngleToDouble(hrd.aamps_drift);
    track = _binAngleToDouble(hrd.aamps_track);
  }

  // set rotation and tilt from az/el
  // store corrected rotation and tilt in az/el
  
  double tilt = ray->getAzimuthDeg();
  double rotation = ray->getElevationDeg();

  if (_isHrdTailRadar) {
    rotation = 90.0 - rotation;
    // rotation -= roll;
    if (rotation < 0) {
      rotation += 360.0;
    } else if (rotation > 360.0) {
      rotation -= 360.0;
    }
    if (tilt > 180) {
      tilt -= 360.0;
    } else if (tilt < -180) {
      tilt += 360.0;
    }
    ray->setAzimuthDeg(rotation);
    ray->setElevationDeg(tilt);
  }

  // wind speed and direction

  double windSpeed = hrd.aamps_wind_vel_cm_per_sec / 100.0;
  double windDirn = _binAngleToDouble(hrd.aamps_wind_dir);

  if (_hrdWindSource == USE_IRS_WIND) {
    windSpeed = hrd.irs_wind_vel_cm_per_sec / 100.0;
    windDirn = _binAngleToDouble(hrd.irs_wind_dir);
  }
  
  double windDirnRad = windDirn * Radx::DegToRad;
  double sinDirn, cosDirn;
  Radx::sincos(windDirnRad, sinDirn, cosDirn);

  // set georef

  georef.setTimeSecs(raySecs);
  georef.setNanoSecs(rayNanoSecs);
  
  georef.setLongitude(longitude);
  georef.setLatitude(latitude);
  georef.setAltitudeKmMsl(altitudeKmMsl);
  georef.setAltitudeKmAgl(altitudeKmAgl);

  georef.setRotation(rotation);
  georef.setTilt(tilt);

  georef.setEwVelocity(ewVelocity);
  georef.setNsVelocity(nsVelocity);
  georef.setVertVelocity(vertVelocity);
  
  georef.setHeading(heading);
  georef.setRoll(roll);
  georef.setPitch(pitch);
  georef.setDrift(drift);
  georef.setTrack(track);

  georef.setEwWind(windSpeed * sinDirn * -1.0);
  georef.setNsWind(windSpeed * cosDirn * -1.0);
  georef.setVertWind(hrd.aamps_wind_vel_v_cm_per_sec / 100.0);
  
  georef.setHeadingRate(yawRate);
  georef.setPitchRate(pitchRate);
  georef.setRollRate(rollRate);

  // set sweep mode

  if (_isHrdTailRadar) {
    ray->setSweepMode(Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE);
  } else {
    ray->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
  }

  // update metadata

  _readVol->setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT);
  
  if (_isHrdTailRadar) {
    // tail radar for NOAA aircraft
    _readVol->setPrimaryAxis(Radx::PRIMARY_AXIS_Y_PRIME);
    _readVol->setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_TAIL);
    // compute AZ and EL from georefs, set on ray
    if (_readApplyGeorefs) {
      _computeAzElTail(ray, georef);
    }
  } else if (_isHrdLfRadar) {
    // lower fuselage radar for NOAA aircraft
    _readVol->setPrimaryAxis(Radx::PRIMARY_AXIS_Z_PRIME);
    _readVol->setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_BELLY);
    // compute AZ and EL from georefs, set on ray
    if (_readApplyGeorefs) {
      _computeAzElLf(ray, georef);
    }
  }
  
}

///////////////////////////////////////////////////////////////////
// compute the true azimuth, elevation, etc. from platform
// parameters using Testud's equations with their different
// definitions of rotation angle, etc.
//
// see Wen-Chau Lee's paper
// "Mapping of the Airborne Doppler Radar Data"

void SigmetRadxFile::_computeAzElTail(RadxRay *ray,
                                      const RadxGeoref &georef)
  
{
  
  double R = georef.getRoll() * Radx::DegToRad;
  double P = georef.getPitch() * Radx::DegToRad;
  double H = georef.getHeading() * Radx::DegToRad;
  double D = georef.getDrift() * Radx::DegToRad;
  double T = H + D;
  
  double sinP = sin(P);
  double cosP = cos(P);
  double sinD = sin(D);
  double cosD = cos(D);
  
  double theta_a = georef.getRotation() * Radx::DegToRad;
  double tau_a = georef.getTilt() * Radx::DegToRad;
  double sin_tau_a = sin(tau_a);
  double cos_tau_a = cos(tau_a);
  double sin_theta_rc = sin(theta_a + R); /* roll corrected rotation angle */
  double cos_theta_rc = cos(theta_a + R); /* roll corrected rotation angle */
  
  double xsubt = (cos_theta_rc * sinD * cos_tau_a * sinP
                  + cosD * sin_theta_rc * cos_tau_a
                  -sinD * cosP * sin_tau_a);
  
  double ysubt = (-cos_theta_rc * cosD * cos_tau_a * sinP
                  + sinD * sin_theta_rc * cos_tau_a
                  + cosP * cosD * sin_tau_a);
  
  double zsubt = (cosP * cos_tau_a * cos_theta_rc
                  + sinP * sin_tau_a);
  
  double lambda_t = atan2(xsubt, ysubt);
  ray->setAzimuthDeg(fmod(lambda_t + T, M_PI * 2.0));
  ray->setElevationDeg(asin(zsubt));
  ray->setGeorefApplied(true);
  
}

///////////////////////////////////////////////////////////////////
// compute the true azimuth, elevation, etc. from platform
// parameters using Testud's equations with their different
// definitions of rotation angle, etc.

void SigmetRadxFile::_computeAzElLf(RadxRay *ray,
                                    const RadxGeoref &georef)
  
{

  // it seems that the LF radar data has the az/el pre-computed
  // from the georef system by the time we get the data
  // so no action is needed

  return;

#ifdef NOTNOW  

  ray->setAzimuthDeg(georef.getRotation() + georef.getHeading());
  ray->setElevationDeg(georef.getTilt());

#endif

}

/////////////////////////////////////
// Set the ray info

void SigmetRadxFile::_setRayInfo()

{

  _rayInfo.clear();
  _nGates = 0;

  // get the time for each ray, so we can output them in the correct order

  Radx::ui08 *rayPtr = (Radx::ui08 *) _dataBuf.getPtr();
  Radx::ui08 *end = rayPtr + _dataBuf.getLen();

  int rayFieldNum = 0;
  double az = Radx::missingMetaDouble;
  double el = Radx::missingMetaDouble;
  for (int iray = 0; iray < _nRaysSweep; iray++) {

    RayInfo rayInfo;
    rayInfo.offset = rayPtr; // offset saved in first field

    // set number of bytes per field
    // this includes the ray_hdr for the field
    // if the number of bytes is 0, the ray is missing and we can skip it
    
    int sweepNum = -1;
    for (size_t ifield = 0; ifield < _inDatHdrs.size(); ifield++) {
      const ingest_data_header_t &inDatHdr = _inDatHdrs[ifield];
      sweepNum = inDatHdr.sweep_num;
      int nBytesField = _nBytesRayField[rayFieldNum];
      rayInfo.nBytesField.push_back(nBytesField);
      rayInfo.nBytesTotal += nBytesField;
      rayFieldNum++;
    } // ifield

    if (rayInfo.nBytesTotal == 0) {
      // skip this ray - zero length
      continue;
    }
    
    // check buffer is large enough
    
    if (rayPtr + rayInfo.nBytesTotal > end) {
      cerr << "WARNING - SigmetRadxFile::_setRayInfo" << endl;
      cerr << "  Data buffer too small - overflow occurred" << endl;
      cerr << "  nRaysSweep: " << _nRaysSweep << endl;
      cerr << "  sweepNum: " << sweepNum << endl;
      cerr << "  rayNum: " << iray << endl;
      cerr << "  el: " << el << endl;
      cerr << "  az: " << az << endl;
      cerr << "  Ignoring rays beyond this point" << endl;
      return;
    }
    
    // set info from ray header for first real field

    bool infoSaved = false;

    for (size_t ifield = 0; ifield < _inDatHdrs.size(); ifield++) {
      
      const ingest_data_header_t &inDatHdr = _inDatHdrs[ifield];
      int fieldId = inDatHdr.data_code;
      
      if (fieldId == FIELD_EXT_HDR) {
        // extended header instead of field
        rayPtr += rayInfo.nBytesField[ifield];
        continue;
      }

      // real data

      if (!infoSaved) {

        ray_header_t rayHdr;
        memcpy(&rayHdr, rayPtr, sizeof(rayHdr));
        _swap(rayHdr);
        
        // header saved using first real field
        
        rayInfo.hdr = rayHdr;
        if (rayHdr.n_gates > _nGates) {
          _nGates = rayHdr.n_gates;
        }
      
        _rayInfo.push_back(rayInfo);
        infoSaved = true;

      }
    
      rayPtr += rayInfo.nBytesField[ifield];

    } // ifield
    
  } // iray

}

///////////////////////////
// set volume meta data

void SigmetRadxFile::_setVolMetaData()

{

  _readVol->setOrigFormat("SIGMETRAW");

  _readVol->setVolumeNumber(_volumeNumber);
  _readVol->setStartTime(_startTimeSecs, _startNanoSecs);
  _readVol->setEndTime(_endTimeSecs, _endNanoSecs);

  string instName = 
    Radx::makeString(_prodHdr.end.hardware_name, 16);
  if (instName.size() < 1) {
    instName = "unknown";
  }
  _readVol->setInstrumentName(instName);

  _readVol->setSiteName
    (Radx::makeString(_prodHdr.end.ingest_site_name, 16));
  
  _readVol->setScanName(Radx::makeString(_prodHdr.conf.task_name, 12));

  _readVol->setScanId(_prodHdr.end.site_mask);
  
  _readVol->setTitle(Radx::makeString(_prodHdr.conf.prod_name, 12));

  _readVol->setSource("Sigmet IRIS software");
  _readVol->setReferences("Conversion software: Radx::SigmetRadxFile");
  
  string history = "Sigmet RAW file: ";
  history += Radx::makeString(_inHdr.ingest_conf.file_name, 80);
  _readVol->setHistory(history);

  _readVol->setLatitudeDeg(_binAngleToDouble(_inHdr.ingest_conf.latitude));
  _readVol->setLongitudeDeg(_binAngleToDouble(_inHdr.ingest_conf.longitude));

  double altGroundKm = _inHdr.ingest_conf.ground_ht_msl_meters / 1000.0;
  double htAglKm = _inHdr.ingest_conf.radar_ht_agl_meters / 1000.0;

  _readVol->setAltitudeKm(altGroundKm + htAglKm);
  _readVol->setSensorHtAglM(_inHdr.ingest_conf.radar_ht_agl_meters);

  _readVol->addWavelengthCm(_wavelengthCm);

  double horizBeamWidth =
    _binAngleToDouble(_inHdr.task_conf.misc_info.beam_width_h);
  if (horizBeamWidth < 0.1) {
    horizBeamWidth = 1.0;
  }
  double vertBeamWidth =
    _binAngleToDouble(_inHdr.task_conf.misc_info.beam_width_v);
  if (vertBeamWidth < 0.1) {
    vertBeamWidth = 1.0;
  }

  _readVol->setRadarBeamWidthDegH(horizBeamWidth);
  _readVol->setRadarBeamWidthDegV(vertBeamWidth);

  // calibration

  RadxRcalib *calib = new RadxRcalib;
  double radarConstant = _inHdr.task_conf.calib_info.radar_const_h_100 / 100.0;
  calib->setRadarConstantH(radarConstant);
  if (_isDualPol) {
    calib->setRadarConstantV(radarConstant);
  }
  calib->setPulseWidthUsec(_pulseWidthUs);
  double xmitPeakPowerWatts = _inHdr.task_conf.misc_info.xmit_power_watts;
  double xmitPeakPowerDbm = 10.0 * log10(xmitPeakPowerWatts * 1.0e3);
  calib->setXmitPowerDbmH(xmitPeakPowerDbm);
  if (_isDualPol) {
    calib->setXmitPowerDbmV(xmitPeakPowerDbm);
  }

  double noiseDb = _prodHdr.end.cal_noise_db_100 / 100.0;
  calib->setNoiseDbmHc(noiseDb);
  if (_isDualPol) {
    calib->setNoiseDbmVc(noiseDb);
  }

  _readVol->addCalib(calib);

}

/////////////////////////
// set the beam metadata
// returns the ray time

void SigmetRadxFile::_setRayMetadata(RadxRay &ray,
                                     const ray_header_t &rayHdr)

{
  
  if (_verbose) {
    _print(rayHdr, cerr);
  }
    
  RadxTime rayTime = _sweepStartTime + (double) rayHdr.seconds;

  time_t raySecs = rayTime.utime();
  int rayNanoSecs = (int) (rayTime.getSubSec() * 1.0e9 + 0.5);

  if (_startTimeSecs == 0 && _endTimeSecs == 0) {
    _startTimeSecs = raySecs;
    _startNanoSecs = rayNanoSecs;
  } 
  _endTimeSecs = raySecs;
  _endNanoSecs = rayNanoSecs;

  ray.setTime(raySecs, rayNanoSecs);
  ray.setVolumeNumber(_volumeNumber);
  ray.setSweepNumber(_sweepIndex);
  ray.setCalibIndex(0);

  if (_inHdr.task_conf.scan_info.scan_mode == SCAN_MODE_RHI) {
    ray.setSweepMode(Radx::SWEEP_MODE_RHI);
  } else if (_inHdr.task_conf.scan_info.scan_mode == SCAN_MODE_PPI) {
    ray.setSweepMode(Radx::SWEEP_MODE_SECTOR);
  } else {
    ray.setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
  }

  if (_prodHdr.end.polarization == 1) {
    ray.setPolarizationMode(Radx::POL_MODE_HORIZONTAL);
  } else {
    ray.setPolarizationMode(Radx::POL_MODE_VERTICAL);
  }

  if (_dualPrt) {
    ray.setPrtMode(Radx::PRT_MODE_DUAL);
  } else {
    ray.setPrtMode(Radx::PRT_MODE_FIXED);
  }

  double startEl = _binAngleToDouble(rayHdr.start_el);
  double endEl = _binAngleToDouble(rayHdr.end_el);
  double startAz = _binAngleToDouble(rayHdr.start_az);
  double endAz = _binAngleToDouble(rayHdr.end_az);

  double meanEl = RadxComplex::computeMeanDeg(startEl, endEl);
  double meanAz = RadxComplex::computeMeanDeg(startAz, endAz);

  if (meanAz < 0) {
    meanAz += 360.0;
  }
  
  ray.setAzimuthDeg(meanAz);
  ray.setElevationDeg(meanEl);

  double fixedAngle = _getFixedAngle(_sweepIndex, meanEl, meanAz);
  ray.setFixedAngleDeg(fixedAngle);

  ray.setTrueScanRateDegPerSec(Radx::missingMetaDouble);
  ray.setTargetScanRateDegPerSec(Radx::missingMetaDouble);
  ray.setIsIndexed(false);
  ray.setAngleResDeg(360.0 / (double) _inDatHdr0.nrays_per_revolution);
  ray.setAntennaTransition(false);
  ray.setNSamples(_prodHdr.end.nsamples);
  ray.setPulseWidthUsec(_pulseWidthUs);
  ray.setPrtSec(1.0 / _prf);
  ray.setPrtRatio(_prtRatio);
  ray.setNyquistMps(_nyquist);
  ray.setUnambigRangeKm(_unambigRangeKm);
  ray.setMeasXmitPowerDbmH(Radx::missingMetaDouble);
  ray.setMeasXmitPowerDbmV(Radx::missingMetaDouble);

  double startRangeKm = _inHdr.task_conf.range_info.range_first_bin_cm / 1.0e5;
  double gateSpacingKm = _inHdr.task_conf.range_info.output_gate_spacing_cm / 1.0e5;
  ray.setRangeGeom(startRangeKm, gateSpacingKm);

}

////////////////////////////////////////////
// get the fixed angle for the sweep number

double SigmetRadxFile::_getFixedAngle(int sweepIndex, double el, double az)

{

  const task_scan_info_t &info = _inHdr.task_conf.scan_info;
  int nSweeps = info.n_sweeps;
  if (nSweeps > MAX_SWEEPS) {
    nSweeps = MAX_SWEEPS;
  }

  if (info.scan_mode == SCAN_MODE_RHI) {
    // vertical scanning - rhi
    if (sweepIndex < 0 || sweepIndex >= nSweeps) {
      return az;
    }
    double fixedAngle = _binAngleToDouble(info.u.rhi.az_list[sweepIndex]);
    return fixedAngle;
  }

  // horizontal scanning - ppi or surveillance
  if (sweepIndex < 0 || sweepIndex >= nSweeps) {
    return el;
  }
  double fixedAngle = _binAngleToDouble(info.u.ppi.el_list[sweepIndex]);
  return fixedAngle;

}

////////////////////////////////////////////////////////////////    
// set sources on reading HRD extended headers

void SigmetRadxFile::_setHrdSources()

{

  // determine POSN source for HRD data

  _hrdPosnSource = USE_IRS_POSN;
  const char *posnStr = getenv("HRD_USE_GPS_POSN");
  if (posnStr && !strcasecmp(posnStr, "true")) {
    _hrdPosnSource = USE_GPS_POSN;
  }
  if (_hrdPosnSource == USE_IRS_POSN) {
    posnStr = getenv("HRD_USE_AAMPS_POSN");
    if (posnStr && !strcasecmp(posnStr, "true")) {
      _hrdPosnSource = USE_AAMPS_POSN;
    }
  }
  
  // determine WIND source for HRD data

  _hrdWindSource = USE_AAMPS_WIND;
  const char *windStr = getenv("HRD_USE_IRS_WIND");
  if (windStr && !strcasecmp(windStr, "true")) {
    _hrdWindSource = USE_IRS_WIND;
  }

  if (_debug) {
    switch (_hrdPosnSource) {
      case USE_IRS_POSN:
        cerr << "Using IRS for posn data" << endl;
        break;
      case USE_GPS_POSN:
        cerr << "Using GPS for posn data" << endl;
        break;
      case USE_AAMPS_POSN:
        cerr << "Using AAMPS for posn data" << endl;
        break;
    }
    switch (_hrdWindSource) {
      case USE_IRS_WIND:
        cerr << "Using IRS for wind data" << endl;
        break;
      case USE_AAMPS_WIND:
        cerr << "Using AAMPS for wind data" << endl;
        break;
    }
  }

  _hrdSourcesSet = true;

}

////////////////////////////////////////////////////////////////    
// convert a field ID to name

string SigmetRadxFile::_fieldId2Name(int fieldId)

{

  switch (fieldId) {
    case FIELD_DBZ_TOT:
      return "DBZ_TOT";
    case FIELD_DBZ:
      return "DBZ";
    case FIELD_VEL:
      return "VEL";
    case FIELD_WIDTH:
      return "WIDTH";
    case FIELD_ZDR:
      return "ZDR";
    case FIELD_ORAIN:
      return "ORAIN";
    case FIELD_DBZ_CORR:
      return "DBZ_CORR";
    case FIELD_DBZ_TOT_2:
      return "DBZ_TOT";
    case FIELD_DBZ_2:
      return "DBZ";
    case FIELD_VEL_2:
      return "VEL";
    case FIELD_WIDTH_2:
      return "WIDTH";
    case FIELD_ZDR_2:
      return "ZDR";
    case FIELD_RAINRATE_2:
      return "RAINRATE_2";
    case FIELD_KDP:
      return "KDP";
    case FIELD_KDP_2:
      return "KDP";
    case FIELD_PHIDP:
      return "PHIDP";
    case FIELD_VEL_CORR:
      return "VEL_CORR";
    case FIELD_SQI:
      return "SQI";
    case FIELD_RHOHV:
    case FIELD_RHOHV_2:
      return "RHOHV";
    case FIELD_DBZ_CORR_2:
      return "DBZ_CORR";
    case FIELD_VEL_CORR_2:
      return "VEL_CORR";
    case FIELD_SQI_2:
      return "SQI";
    case FIELD_PHIDP_2:
      return "PHIDP";
    case FIELD_LDRH:
    case FIELD_LDRH_2:
      return "LDRH";
    case FIELD_LDRV:
    case FIELD_LDRV_2:
      return "LDRV";
    case FIELD_FLAGS:
    case FIELD_FLAGS_2:
      return "FLAGS";
    case FIELD_ECHO_TOPS:
      return "ECHO_TOPS";
    case FIELD_VIL_2:
      return "VIL";
    case FIELD_RAW:
      return "RAW";
    case FIELD_SHEAR:
      return "SHEAR";
    case FIELD_DIVERGE_2:
      return "DIVERGE";
    case FIELD_FLIQUID_2:
      return "FLIQUID";
    case FIELD_USER:
      return "USER";
    case FIELD_OTHER:
      return "OTHER";
    case FIELD_DEFORM_2:
      return "DEFORM";
    case FIELD_VVEL_2:
      return "VVEL";
    case FIELD_HVEL_2:
      return "HVEL";
    case FIELD_HDIR_2:
      return "HDIR";
    case FIELD_AXDIL_2:
      return "AXDIL";
    case FIELD_TIME_2:
      return "TIME";
    case FIELD_RHOH:
    case FIELD_RHOH_2:
      return "RHOH";
    case FIELD_RHOV:
    case FIELD_RHOV_2:
      return "RHOV";
    case FIELD_PHIH:
    case FIELD_PHIH_2:
      return "PHIH";
    case FIELD_PHIV:
    case FIELD_PHIV_2:
      return "PHIV";
    case FIELD_USER_2:
      return "USER";
    case FIELD_HCLASS:
    case FIELD_HCLASS_2:
      return "HCLASS";
    case FIELD_ZDRC:
    case FIELD_ZDRC_2:
      return "ZDRC";
    case FIELD_TEMP_2:
      return "TEMP";
    case FIELD_VIR_2:
      return "VIR";
    case FIELD_DBTV:
    case FIELD_DBTV_2:
      return "DBZV";
    case FIELD_DBZV:
    case FIELD_DBZV_2:
      return "DBZV";
    case FIELD_SNR:
    case FIELD_SNR_2:
      return "SNR";
    case FIELD_ALBEDO:
    case FIELD_ALBEDO_2:
      return "ALBEDO";
    case FIELD_VILD_2:
      return "VILD";
    case FIELD_TURB_2:
      return "TURB";
    case FIELD_DBTE:
    case FIELD_DBTE_2:
      return "DBTE";
    case FIELD_DBZE:
    case FIELD_DBZE_2:
      return "DBZE";
    default: {
      char name[128];
      sprintf(name, "UNKNOWN-ID-%d", fieldId);
      return name;
    }
  }
}

////////////////////////////////////////////////////////////////    
// convert a field ID to units

string SigmetRadxFile::_fieldId2Units(int fieldId)

{

  switch (fieldId) {
    case FIELD_DBZ_TOT:
    case FIELD_DBZ:
    case FIELD_ORAIN:
    case FIELD_DBZ_CORR:
    case FIELD_DBZ_TOT_2:
    case FIELD_DBZ_2:
    case FIELD_DBZ_CORR_2:
    case FIELD_DBZV_2:
    case FIELD_DBZV:
    case FIELD_DBZE:
    case FIELD_DBZE_2:
      return "dBZ";
    case FIELD_VEL:
    case FIELD_WIDTH:
    case FIELD_VEL_2:
    case FIELD_WIDTH_2:
    case FIELD_VEL_CORR:
    case FIELD_VEL_CORR_2:
    case FIELD_VVEL_2:
    case FIELD_HVEL_2:
      return "m/s";
    case FIELD_ZDR:
    case FIELD_ZDR_2:
    case FIELD_LDRH:
    case FIELD_LDRH_2:
    case FIELD_LDRV:
    case FIELD_LDRV_2:
    case FIELD_ZDRC:
    case FIELD_ZDRC_2:
    case FIELD_SNR:
    case FIELD_SNR_2:
      return "dB";
    case FIELD_DBTV:
    case FIELD_DBTV_2:
    case FIELD_DBTE:
    case FIELD_DBTE_2:
      return "dBm";
    case FIELD_KDP:
    case FIELD_KDP_2:
      return "deg/km";
    case FIELD_PHIDP:
    case FIELD_PHIDP_2:
    case FIELD_HDIR_2:
    case FIELD_AXDIL_2:
    case FIELD_PHIH:
    case FIELD_PHIH_2:
    case FIELD_PHIV:
    case FIELD_PHIV_2:
      return "deg";
    case FIELD_ECHO_TOPS:
      return "km";
    case FIELD_VIL_2:
    case FIELD_FLIQUID_2:
      return "mm";
    case FIELD_RAINRATE_2:
      return "mm/hr";
    case FIELD_SHEAR:
      return "m/s/m";
    case FIELD_DIVERGE_2:
    case FIELD_DEFORM_2:
      return "10**-4";
    case FIELD_TIME_2:
      return "sec";
    case FIELD_TEMP_2:
      return "C";
    case FIELD_VIR_2:
      return "dBZ.m";
    case FIELD_ALBEDO:
    case FIELD_ALBEDO_2:
      return "%";
    case FIELD_SQI:
    case FIELD_RHOHV:
    case FIELD_RHOHV_2:
    case FIELD_SQI_2:
    case FIELD_FLAGS:
    case FIELD_FLAGS_2:
    case FIELD_RAW:
    case FIELD_USER:
    case FIELD_OTHER:
    case FIELD_RHOH:
    case FIELD_RHOH_2:
    case FIELD_RHOV:
    case FIELD_RHOV_2:
    case FIELD_USER_2:
    case FIELD_HCLASS:
    case FIELD_HCLASS_2:
    case FIELD_VILD_2:
    case FIELD_TURB_2:
    default:
      return "";
  }

}


//////////////////////////////////////
// Get scale, bias from field ID
// for unsigned data

void SigmetRadxFile::_fieldId2ScaleBias(int fieldId,
                                        double &scale,
                                        double &bias)
  
{
  
  scale = 1.0;
  bias = 0.0;
  
  switch (fieldId) {

    case FIELD_DBZ_TOT:
    case FIELD_DBZ:
    case FIELD_DBZ_CORR:
    case FIELD_DBZE:
    case FIELD_DBZV:
    case FIELD_SNR:
    case FIELD_DBTV:
    case FIELD_DBTE:
      scale = 0.5;
      bias = -32.0;
      break;
    case FIELD_VEL:
      scale = _nyquist / 127.0;
      bias = (-1.0 * _nyquist) * (128.0 / 127.0);
      break;
    case FIELD_WIDTH:
      scale = _nyquist / 256.0;
      bias = 0.0;
      break;
    case FIELD_ZDR:
    case FIELD_ZDRC:
      scale = 1.0 / 16.0;
      bias = -8.0;
      break;
    case FIELD_WIDTH_2:
      scale = 0.01;
      bias = 0.0;
      break;
    case FIELD_PHIDP:
    case FIELD_PHIH:
    case FIELD_PHIV:
      scale = 180.0 / 254.0;
      bias = scale * -1.0;
      break;
    case FIELD_VEL_CORR:
      scale = 75.0 / 127.0;
      bias = (-1.0 * 75.0) * (128.0 / 127.0);
      break;
    case FIELD_RHOHV_2:
    case FIELD_RHOH_2:
    case FIELD_RHOV_2:
    case FIELD_SQI_2:
      scale = 1.0 / 65533.0;
      bias = scale * -1.0;
      break;
    case FIELD_PHIDP_2:
    case FIELD_PHIH_2:
    case FIELD_PHIV_2:
      scale = 360.0 / 65534.0;
      bias = scale * -1.0;
      break;
    case FIELD_LDRH:
    case FIELD_LDRV:
      scale = 0.2;
      bias = -45.2;
      break;
    case FIELD_ECHO_TOPS:
    case FIELD_HDIR_2:
      scale = 0.1;
      bias = 0.0;
      break;
      break;
    case FIELD_AXDIL_2:
      scale = 0.10;
      bias = 0.0;
      break;
    case FIELD_ALBEDO:
      scale = 100.0 / 253.0;
      bias = 0.0;
      break;
    case FIELD_VIL_2:
      scale = 0.001;
      bias = 0.0;
      break;
    case FIELD_DBZ_TOT_2:
    case FIELD_DBTV_2:
    case FIELD_DBZ_2:
    case FIELD_DBZV_2:
    case FIELD_SNR_2:
    case FIELD_DBZ_CORR_2:
    case FIELD_DBZE_2:
    case FIELD_VEL_2:
    case FIELD_KDP_2:
    case FIELD_VEL_CORR_2:
    case FIELD_ZDR_2:
    case FIELD_ZDRC_2:
    case FIELD_LDRH_2:
    case FIELD_LDRV_2:
    case FIELD_USER_2:
    case FIELD_VVEL_2:
    case FIELD_HVEL_2:
    case FIELD_ALBEDO_2:
    case FIELD_VILD_2:
    case FIELD_DBTE_2:
    case FIELD_TURB_2:
    case FIELD_TEMP_2:
    case FIELD_VIR_2:
    case FIELD_DEFORM_2:
      scale = 0.01;
      bias = -327.68;
      break;
    case FIELD_DIVERGE_2:
      scale = 0.001;
      bias = -32.768;
      break;
    case FIELD_TIME_2:
      scale = 1.0;
      bias = -32767;
      break;
    case FIELD_FLAGS:
    case FIELD_FLAGS_2:
    case FIELD_ORAIN:
    case FIELD_RAW:
    case FIELD_USER:
    case FIELD_OTHER:
    case FIELD_HCLASS:
    case FIELD_HCLASS_2:
    case FIELD_SHEAR:
      // special cases - dealt with later
    case FIELD_FLIQUID_2:
    case FIELD_RAINRATE_2:
    case FIELD_KDP:
    case FIELD_SQI:
    case FIELD_RHOHV:
    case FIELD_RHOH:
    case FIELD_RHOV:
    default:
      scale = 1.0;
      bias = 0.0;
  }

}

//////////////////////////////////////
// Convert scale, bias to signed

void SigmetRadxFile::_convertBias2Signed(int fieldId,
                                         double scale,
                                         double &bias)
  
{
  
  switch (fieldId) {

    // 16-bit

    case FIELD_DBZ_TOT_2:
    case FIELD_DBZ_2:
    case FIELD_VEL_2:
    case FIELD_WIDTH_2:
    case FIELD_ZDR_2:
    case FIELD_RAINRATE_2:
    case FIELD_KDP_2:
    case FIELD_RHOHV_2:
    case FIELD_DBZ_CORR_2:
    case FIELD_VEL_CORR_2:
    case FIELD_SQI_2:
    case FIELD_PHIDP_2:
    case FIELD_LDRH_2:
    case FIELD_LDRV_2:
    case FIELD_FLAGS_2:
    case FIELD_VIL_2:
    case FIELD_DIVERGE_2:
    case FIELD_FLIQUID_2:
    case FIELD_DEFORM_2:
    case FIELD_VVEL_2:
    case FIELD_HVEL_2:
    case FIELD_HDIR_2:
    case FIELD_AXDIL_2:
    case FIELD_TIME_2:
    case FIELD_RHOH_2:
    case FIELD_RHOV_2:
    case FIELD_PHIH_2:
    case FIELD_PHIV_2:
    case FIELD_USER_2:
    case FIELD_HCLASS_2:
    case FIELD_ZDRC_2:
    case FIELD_TEMP_2:
    case FIELD_VIR_2:
    case FIELD_DBTV_2:
    case FIELD_DBZV_2:
    case FIELD_SNR_2:
    case FIELD_ALBEDO_2:
    case FIELD_VILD_2:
    case FIELD_TURB_2:
    case FIELD_DBTE_2:
    case FIELD_DBZE_2:
      bias += 32768.0 * scale;
      break;

    // 8 bit

    default:
      bias += 128.0 * scale;
      break;

  }

}

//////////////////////////////////////
// Check is this is a dual pol field
// If so, set flag

void SigmetRadxFile::_checkDualPol(int fieldId)
  
{
  
  switch (fieldId) {

    case FIELD_DBZ_TOT_2:
    case FIELD_DBZV_2:
    case FIELD_KDP_2:
    case FIELD_ZDR_2:
    case FIELD_LDRH_2:
    case FIELD_LDRV_2:
    case FIELD_PHIDP_2:
    case FIELD_RHOHV_2:
    case FIELD_ZDR:
    case FIELD_PHIDP:
    case FIELD_KDP:
    case FIELD_RHOHV:
    case FIELD_LDRH:
    case FIELD_LDRV:
    case FIELD_RHOH:
    case FIELD_RHOH_2:
    case FIELD_RHOV:
    case FIELD_RHOV_2:
    case FIELD_PHIH:
    case FIELD_PHIH_2:
    case FIELD_PHIV:
    case FIELD_PHIV_2:
    case FIELD_ZDRC:
    case FIELD_ZDRC_2:
    case FIELD_DBTV:
    case FIELD_DBTV_2:
    case FIELD_DBZV:
      _isDualPol = true;
    default: {}
  }

}

//////////////////////////////////
// convert binary angle to double

double SigmetRadxFile::_binAngleToDouble(Radx::ui16 binAngle)
{
  return (double) binAngle * 360.0 / ((double) 0xffff + 1.0);
}

double SigmetRadxFile::_binAngleToDouble(Radx::si16 binAngle)
{
  return (double) binAngle * 360.0 / ((double) 0xffff + 1.0);
}

double SigmetRadxFile::_binAngleToDouble(Radx::ui32 binAngle)
{
  return (double) binAngle * 360.0 / ((double) 0xffffffff + 1.0);
}

double SigmetRadxFile::_binAngleToDouble(Radx::si32 binAngle)
{
  return (double) binAngle * 360.0 / ((double) 0xffffffff + 1.0);
}

///////////////////////////////
// print a char array

void SigmetRadxFile::_printCharArray(ostream &out, const char *buf, int len)

{
  for (int ii = 0; ii < len; ii++) {
    if (buf[ii] == '\0') {
      return;
    }
    if (isprint(buf[ii])) {
      out << buf[ii];
    } else {
      out << " ";
    }
  }
}

////////////////////////////////////////////////////////////////    
// convert time to string

string SigmetRadxFile::_time2Str(const sigmet_time_t &time)

{
  
  RadxTime stime(time.year, time.month, time.day, 0, 0, time.sec);
  char msecsStr[32];
  int msecs = time.msecs & 1023; // lower 10 bits
  sprintf(msecsStr, "%.3d", msecs);
  string tstr = RadxTime::strm(stime.utime());
  tstr += ".";
  tstr += msecsStr;
  return tstr;
  
}

////////////////////////////////////////////////////////////////    
// convert label to string, ensuring null termination

string SigmetRadxFile::_label2Str(const char *label, int maxLen)

{
  
  RadxArray<char> _cstr;
  char *cstr = _cstr.alloc(maxLen + 1);
  memset(cstr, 0, maxLen + 1);
  memcpy(cstr, label, maxLen);
  return cstr;

}

///////////////////////////////////////////////////
// printing routines

void SigmetRadxFile::_print(const sigmet_id_hdr_t &hdr, ostream &out)

{
  out << "  id: " << hdr.id << endl;
  out << "  version: " << hdr.version << endl;
  out << "  nbytes: " << hdr.nbytes << endl;
  out << "  flags: " << hdr.flags << endl;
}

void SigmetRadxFile::_print(const prod_header_t &hdr, ostream &out)
  
{
  out << "===== PRODUCT HEADER =====" << endl;
  out << "  Size: " << sizeof(hdr) << endl;
  _print(hdr.id_hdr, out);
  _print(hdr.conf, out);
  _print(hdr.end, out);
  out << "=================================" << endl;

}

void SigmetRadxFile::_print(const prod_conf_t &prod, ostream &out)
  
{
  out << "----- PRODUCT CONFIGURATION -----" << endl;
  out << "  Size: " << sizeof(prod) << endl;
  _print(prod.id_hdr, out);
  out << "  prodType: " << prod.ptype << endl;
  out << "  scheduling " << prod.scheduling << endl;
  out << "  secs_between_runs: " << prod.secs_between_runs << endl;
  out << "  product_time: " << _time2Str(prod.product_time) << endl;
  out << "  ingest_time: " << _time2Str(prod.ingest_time) << endl;
  out << "  start_sched_time: " << _time2Str(prod.start_sched_time) << endl;
  out << "  product name: " << _label2Str(prod.prod_name, 12) << endl;
  out << "  task name: " << _label2Str(prod.task_name, 12) << endl;
  out << "  flags: " << prod.flags << endl;
  out << "  xsize: " << prod.xsize << endl;
  out << "  ysize: " << prod.ysize << endl;
  out << "  zsize: " << prod.zsize << endl;
  out << "  range_last_bin_cm: " << prod.range_last_bin_cm << endl;
  out << "  data_type_out: " << prod.data_type_out << endl;
  out << "  projection name: "
      << _label2Str(prod.projection_name, 12) << endl;
  out << "  data_type_in: " << prod.data_type_in << endl;
  out << "  projection type: " << (int) prod.projection_type << endl;
  out << "  radial_smoothing_range_km_100: "
      << prod.radial_smoothing_range_km_100 << endl;
  out << "  n_runs: " << prod.n_runs << endl;
  out << "  zr_coeff: " << prod.zr_coeff << endl;
  out << "  zr_expon: " << prod.zr_expon << endl;
  out << "  2d_smooth_x: " << prod.twod_smooth_x << endl;
  out << "  2d_smooth_y: " << prod.twod_smooth_y << endl;

  out << "---------------------------------" << endl;

}

void SigmetRadxFile::_print(const prod_end_t &end, ostream &out)
  
{
  out << "----- PRODUCT END -----" << endl;
  out << "  Size: " << sizeof(end) << endl;
  out << "  prod_sitename: "
      << _label2Str(end.prod_sitename, 16) << endl;
  out << "  prod_version: "
      << _label2Str(end.prod_version, 8) << endl;
  out << "  iris_version: "
      << _label2Str(end.iris_version, 8) << endl;
  out << "  oldest_data_time: "
      << _time2Str(end.oldest_data_time) << endl;
  out << "  minutes_lst_west_of_gmt: " << end.minutes_lst_west_of_gmt << endl;
  out << "  hardware_name: "
      << _label2Str(end.hardware_name, 16) << endl;
  out << "  ingest_site_name: "
      << _label2Str(end.ingest_site_name, 16) << endl;
  out << "  minutes_rec_west_of_gmt: " << end.minutes_rec_west_of_gmt << endl;
  out << "  latitude_center: " << end.latitude_center << endl;
  out << "  longitude_center: " << end.longitude_center << endl;
  out << "  ground_ht_msl_meters: " << end.ground_ht_msl_meters << endl;
  out << "  radar_ht_agl_meters: " << end.radar_ht_agl_meters << endl;
  out << "  prf_hz: " << end.prf_hz << endl;
  out << "  pulse_width_us_100: " << end.pulse_width_us_100 << endl;
  out << "  dsp_type: " << end.dsp_type << endl;
  out << "  trig_rate_scheme: " << end.trig_rate_scheme << endl;
  out << "  nsamples: " << end.nsamples << endl;
  out << "  clut_filter_file_name: "
      << _label2Str(end.clut_filter_file_name, 12) << endl;
  out << "  dop_filter_first_bin: " << (int) end.dop_filter_first_bin << endl;
  out << "  wavelength_cm_100: " << end.wavelength_cm_100 << endl;
  out << "  trunc_ht_above_radar_cm: " << end.trunc_ht_above_radar_cm << endl;
  out << "  range_first_bin_cm: " << end.range_first_bin_cm << endl;
  out << "  range_last_bin_cm: " << end.range_last_bin_cm << endl;
  out << "  n_gates: " << end.n_gates << endl;
  out << "  input_file_count: " << end.input_file_count << endl;
  out << "  polarization: " << end.polarization << endl;
  out << "  i0_cal_db_100: " << end.i0_cal_db_100 << endl;
  out << "  cal_noise_db_100: " << end.cal_noise_db_100 << endl;
  out << "  radar_const_h_100: " << end.radar_const_h_100 << endl;
  out << "  receiver_bandwidth: " << end.receiver_bandwidth << endl;
  out << "  noise_level_db_100: " << end.noise_level_db_100 << endl;
  out << "  ldr_offset_db_100: " << end.ldr_offset_db_100 << endl;
  out << "  zdr_offset_db_100: " << end.zdr_offset_db_100 << endl;
  out << "  lambert_lat1: " << end.lambert_lat1 << endl;
  out << "  lambert_lat2: " << end.lambert_lat2 << endl;
  out << "  earth_radius_cm: " << end.earth_radius_cm << endl;
  out << "  earth_flattening_1000000: "
      << end.earth_flattening_1000000 << endl;
  out << "  faults_bits: " << end.faults_bits << endl;
  out << "  site_mask: " << end.site_mask << endl;
  out << "  log_filter_first: " << end.log_filter_first << endl;
  out << "  dsp_clutmap: " << end.dsp_clutmap << endl;
  out << "  proj_ref_lat: " << end.proj_ref_lat << endl;
  out << "  proj_ref_lon: " << end.proj_ref_lon << endl;
  out << "  sequence_num: " << end.sequence_num << endl;
  out << "  melting_ht_m_msl: " << end.melting_ht_m_msl << endl;
  out << "  ht_radar_above_ref_m: " << end.ht_radar_above_ref_m << endl;
  out << "  n_results_elements: " << end.n_results_elements << endl;
  out << "  wind_speed: " << (int) end.wind_speed << endl;
  out << "  wind_dirn: " << (double) end.wind_dirn  * (360.0 / 256.0) << endl;
  out << "  local_tz: " << _label2Str(end.local_tz, 8) << endl;

  out << "---------------------------------" << endl;

}

void SigmetRadxFile::_print(const ingest_header_t &hdr, ostream &out)
  
{
  out << "===== INGEST HEADER =====" << endl;
  out << "  Size: " << sizeof(hdr) << endl;
  _print(hdr.id_hdr, out);
  _print(hdr.ingest_conf, out);
  _print(hdr.task_conf, out);
  out << "=========================" << endl;
}


void SigmetRadxFile::_print(const ingest_conf_t &conf, ostream &out)
  
{
  out << "----- INGEST CONFIGURATION -----" << endl;
  out << "  Size: " << sizeof(conf) << endl;
  out << "  file_name: " << _label2Str(conf.file_name, 80) << endl;
  out << "  num_data_files: " << conf.num_data_files << endl;
  out << "  nsweeps_completed: " << conf.nsweeps_completed << endl;
  out << "  total_size_files: " << conf.total_size_files << endl;
  out << "  volume_time: " << _time2Str(conf.volume_time) << endl;
  out << "  nbytes_in_ray_hdrs: " << conf.nbytes_in_ray_hdrs << endl;
  out << "  nbytes_in_ext_hdr: " << conf.nbytes_in_ext_hdr << endl;
  out << "  nbytes_in_task_config: " << conf.nbytes_in_task_config << endl;
  out << "  playback_version: " << conf.playback_version << endl;
  out << "  iris_version: " << _label2Str(conf.iris_version, 8) << endl;
  out << "  hardware_name: " << _label2Str(conf.hardware_name, 16) << endl;
  out << "  time_zone_local_mins_west_of_gmt: "
      << conf.time_zone_local_mins_west_of_gmt << endl;
  out << "  site_name: " << _label2Str(conf.site_name, 16) << endl;
  out << "  time_zone_rec_mins_west_of_gmt: "
      << conf.time_zone_rec_mins_west_of_gmt << endl;
  out << "  latitude: " << _binAngleToDouble(conf.latitude) << endl;
  out << "  longitude: " << _binAngleToDouble(conf.longitude) << endl;
  out << "  ground_height_m_msl: " << conf.ground_ht_msl_meters << endl;
  out << "  radar_height_m_agl: " << conf.radar_ht_agl_meters << endl;
  out << "  radar_height_m_msl: " << conf.radar_ht_msl_cm / 100.0 << endl;
  out << "  nrays_per_revolution: " << conf.nrays_per_revolution << endl;
  out << "  index_of_first_ray: " << conf.index_of_first_ray << endl;
  out << "  n_rays_sweep: " << conf.n_rays_sweep << endl;
  out << "  nbytes_gparm: " << conf.nbytes_gparm << endl;
  out << "  radar_ht_msl_cm: " << conf.radar_ht_msl_cm << endl;
  out << "  platform_vel[0]: " << conf.platform_vel[0] << endl;
  out << "  platform_vel[1]: " << conf.platform_vel[1] << endl;
  out << "  platform_vel[2]: " << conf.platform_vel[2] << endl;
  out << "  ant_offset[0]: " << conf.ant_offset[0] << endl;
  out << "  ant_offset[1]: " << conf.ant_offset[1] << endl;
  out << "  ant_offset[2]: " << conf.ant_offset[2] << endl;
  out << "  fault_bits: " << conf.fault_bits << endl;
  out << "  ht_melting_m_msl: " << conf.ht_melting_m_msl << endl;
  out << "  timezone_name: " << _label2Str(conf.timezone_name, 8) << endl;
  out << "  flags: " << conf.flags << endl;
  out << "  conf_name: " << _label2Str(conf.conf_name, 16) << endl;
  out << "-------------------------" << endl;
}

void SigmetRadxFile::_print(const task_conf_t &conf, ostream &out)
  
{
  out << "----- TASK CONFIGURATION -----" << endl;
  out << "  Size: " << sizeof(conf) << endl;
  _print(conf.id_hdr, out);
  _print(conf.sched_info, out);
  _print(conf.dsp_info, out);
  _print(conf.calib_info, out);
  _print(conf.range_info, out);
  _print(conf.scan_info, out);
  _print(conf.misc_info, out);
  _print(conf.end_info, out);
  out << "-------------------------" << endl;
}

void SigmetRadxFile::_print(const task_sched_info_t &info, ostream &out)
  
{
  out << "~~~~~ SCHED INFO ~~~~~" << endl;
  out << "  Size: " << sizeof(info) << endl;
  out << "  start_secs: " << info.start_secs << endl;
  out << "  stop_secs: " << info.stop_secs << endl;
  out << "  skip_secs: " << info.skip_secs << endl;
  out << "  last_run_secs: " << info.last_run_secs << endl;
  out << "  time_used_secs: " << info.time_used_secs << endl;
  out << "  day_last_run: " << info.day_last_run << endl;
  out << "  flag: " << info.flag << endl;
  out << "~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
}

void SigmetRadxFile::_print(const string &label,
                            const dsp_data_mask_t &mask,
                            ostream &out)
  
{
  out << "  DSP DATA MASK: " << label << endl;
  out << "    word_0: " << mask.word_0 << endl;
  out << "    xhdr_type: " << mask.xhdr_type << endl;
  out << "    word_1: " << mask.word_1 << endl;
  out << "    word_2: " << mask.word_2 << endl;
  out << "    word_3: " << mask.word_3 << endl;
  out << "    word_4: " << mask.word_4 << endl;
}


void SigmetRadxFile::_print(const task_dsp_info_t &info, ostream &out)
  
{
  out << "~~~~~ DSP INFO ~~~~~" << endl;
  out << "  Size: " << sizeof(info) << endl;
  out << "  major_mode: " << info.major_mode << endl;
  out << "  dsp_type: " << info.dsp_type << endl;
  _print("data_mask", info.data_mask, out);
  _print("data_mask_orig", info.data_mask_orig, out);
  out << "  low_prf_hz: " << info.low_prf_hz << endl;
  out << "  low_prf_frac_hz: " << info.low_prf_frac_hz << endl;
  out << "  low_prf_sample_size: " << info.low_prf_sample_size << endl;
  out << "  low_prf_averaging: " << info.low_prf_averaging << endl;
  out << "  refl_db_thresh_100: " << info.refl_db_thresh_100 << endl;
  out << "  velocity_db_thresh_100: " << info.velocity_db_thresh_100 << endl;
  out << "  width_db_thresh_100: " << info.width_db_thresh_100 << endl;
  out << "  prf_hz: " << info.prf_hz << endl;
  out << "  pulse_width_usec_100: " << info.pulse_width_usec_100 << endl;
  out << "  trig_rate_flag: " << info.trig_rate_flag << endl;
  out << "  n_pulses_stabilization: " << info.n_pulses_stabilization << endl;
  out << "  agc_coeff: " << info.agc_coeff << endl;
  out << "  nsamples: " << info.nsamples << endl;
  out << "  gain_code: " << info.gain_code << endl;
  out << "  filter_name: " << _label2Str(info.filter_name, 12) << endl;
  out << "  dop_filter_first_bin: " << (int) info.dop_filter_first_bin << endl;
  out << "  log_filter_unused: " << (int) info.log_filter_unused << endl;
  out << "  fixed_gain_1000: " << info.fixed_gain_1000 << endl;
  out << "  gas_atten_10000: " << info.gas_atten_10000 << endl;
  out << "  clutmap_num: " << info.clutmap_num << endl;
  out << "  xmit_phase_sequence: " << info.xmit_phase_sequence << endl;
  out << "  config_header_command_mask: "
      << info.config_header_command_mask << endl;
  out << "  ts_playback_flags: " << info.ts_playback_flags << endl;
  out << "~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
}

void SigmetRadxFile::_print(const task_calib_info_t &info, ostream &out)
  
{
  out << "~~~~~ CALIB INFO ~~~~~" << endl;
  out << "  Size: " << sizeof(info) << endl;
  out << "  log_slope: " << info.log_slope << endl;
  out << "  log_noise_thresh: " << info.log_noise_thresh << endl;
  out << "  clut_corr_thresh: " << info.clut_corr_thresh << endl;
  out << "  sqi_thresh_256: " << info.sqi_thresh_256 << endl;
  out << "  sig_power_thresh_16: " << info.sig_power_thresh_16 << endl;
  out << "  z_calib_16: " << info.z_calib_16 << endl;
  out << "  flags_z_uncorrected: " << info.flags_z_uncorrected << endl;
  out << "  flags_z_corrected: " << info.flags_z_corrected << endl;
  out << "  flags_vel: " << info.flags_vel << endl;
  out << "  flags_width: " << info.flags_width << endl;
  out << "  flags_zdr: " << info.flags_zdr << endl;
  out << "  flags: " << info.flags << endl;
  out << "  ldr_bias_100: " << info.ldr_bias_100 << endl;
  out << "  zdr_bias_16: " << info.zdr_bias_16 << endl;
  out << "  clut_thresh_100: " << info.clut_thresh_100 << endl;
  out << "  clut_skip: " << info.clut_skip << endl;
  out << "  io_horiz_100: " << info.io_horiz_100 << endl;
  out << "  io_vert_100: " << info.io_vert_100 << endl;
  out << "  cal_time_noise_100: " << info.cal_time_noise_100 << endl;
  out << "  cal_noise_vert: " << info.cal_noise_vert << endl;
  out << "  radar_const_h_100: " << info.radar_const_h_100 << endl;
  out << "  radar_const_v_100: " << info.radar_const_v_100 << endl;
  out << "  receiver_bandwidth_khz: " << info.receiver_bandwidth_khz << endl;
  out << "~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
}

void SigmetRadxFile::_print(const task_range_info_t &info, ostream &out)
  
{
  out << "~~~~~ RANGE INFO ~~~~~" << endl;
  out << "  Size: " << sizeof(info) << endl;
  out << "  range_first_bin_cm: " << info.range_first_bin_cm << endl;
  out << "  range_last_bin_cm: " << info.range_last_bin_cm << endl;
  out << "  n_input_gates: " << info.n_input_gates << endl;
  out << "  n_output_gates: " << info.n_output_gates << endl;
  out << "  input_gate_spacing_cm: " << info.input_gate_spacing_cm << endl;
  out << "  output_gate_spacing_cm: " << info.output_gate_spacing_cm << endl;
  out << "  bin_length_variable: " << info.bin_length_variable << endl;
  out << "  gate_averaging: " << info.gate_averaging << endl;
  out << "  gate_smoothing: " << info.gate_smoothing << endl;
  out << "~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
}

void SigmetRadxFile::_print(const task_scan_info_t &info, ostream &out)
  
{
  out << "~~~~~ SCAN INFO ~~~~~" << endl;
  out << "  Size: " << sizeof(info) << endl;
  out << "  scan_mode: " << info.scan_mode << endl;
  out << "  angular_res_deg_1000: " << info.angular_res_deg_1000 << endl;
  out << "  scan_speed_bin_per_sec: " << info.scan_speed_bin_per_sec << endl;
  out << "  n_sweeps: " << info.n_sweeps << endl;
  int nSweeps = info.n_sweeps;
  if (nSweeps > MAX_SWEEPS) {
    nSweeps = MAX_SWEEPS;
  }
  if (info.scan_mode == SCAN_MODE_RHI) {
    out << "  RHI scan" << endl;
    out << "  start_el: "
	<< _binAngleToDouble(info.u.rhi.start_el) << endl;
    out << "  end_el: "
	<< _binAngleToDouble(info.u.rhi.end_el) << endl;
    out << "  start_end: " << (int) info.u.rhi.start_end << endl;
    for (int ii = 0; ii < nSweeps; ii++) {
      out << "    sweep num, az: " << ii << ", "
          << _binAngleToDouble(info.u.rhi.az_list[ii]) << endl;
    }
  } else if (info.scan_mode == SCAN_MODE_PPI) {
    out << "  PPI scan" << endl;
    out << "  start_az: "
	<< _binAngleToDouble(info.u.ppi.start_az) << endl;
    out << "  end_az: "
	<<  _binAngleToDouble(info.u.ppi.end_az) << endl;
    out << "  start_end: " << (int) info.u.ppi.start_end << endl;
    for (int ii = 0; ii < nSweeps; ii++) {
      out << "    sweep num, el: "
          << ii << ", "
          <<  _binAngleToDouble(info.u.ppi.el_list[ii]) << endl;
    }
  } else {
    out << "  SURVEILLANCE scan" << endl;
    out << "  start_az: "
	<< _binAngleToDouble(info.u.ppi.start_az) << endl;
    out << "  end_az: "
	<<  _binAngleToDouble(info.u.ppi.end_az) << endl;
    for (int ii = 0; ii < nSweeps; ii++) {
      out << "    sweep num, el: "
          << ii << ", "
          <<  _binAngleToDouble(info.u.ppi.el_list[ii]) << endl;
    }
  }
  out << "~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
}

void SigmetRadxFile::_print(const task_misc_info_t &info, ostream &out)
  
{
  out << "~~~~~ MISC INFO ~~~~~" << endl;
  out << "  Size: " << sizeof(info) << endl;
  out << "  wavelength_cm_100: " << info.wavelength_cm_100 << endl;
  out << "  user_id: " << _label2Str(info.user_id, 16) << endl;
  out << "  xmit_power_watts: " << info.xmit_power_watts << endl;
  out << "  flags: " << info.flags << endl;
  out << "  polarization: " << info.polarization << endl;
  out << "  trunc_ht_above_radar_cm: "
      << info.trunc_ht_above_radar_cm << endl;
  out << "  beam_width_h: " << _binAngleToDouble(info.beam_width_h) << endl;
  out << "  beam_width_v: " << _binAngleToDouble(info.beam_width_v) << endl;
  out << "~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
}

void SigmetRadxFile::_print(const task_end_info_t &info, ostream &out)
  
{
  out << "~~~~~ END INFO ~~~~~" << endl;
  out << "  Size: " << sizeof(info) << endl;
  out << "  id_major: " << info.id_major << endl;
  out << "  id_minor: " << info.id_minor << endl;
  out << "  task_name: " << _label2Str(info.task_name, 12) << endl;
  out << "  script: " << _label2Str(info.script, 80) << endl;
  out << "  ntasks_hybrid: " << info.ntasks_hybrid << endl;
  out << "  task_state: " << info.task_state << endl;
  out << "  start_time: " << _time2Str(info.start_time) << endl;
  out << "~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
}

void SigmetRadxFile::_print(const raw_product_header_t &hdr, ostream &out)

{
  out << "===== RAW HEADER =====" << endl;
  out << "  Size: " << sizeof(hdr) << endl;
  out << "  record_num: " << hdr.record_num << endl;
  out << "  sweep_num: " << hdr.sweep_num << endl;
  out << "  byte_offset: " << hdr.byte_offset << endl;
  out << "  ray_num: " << hdr.ray_num << endl;
  out << "  flags: " << hdr.flags << endl;
}

void SigmetRadxFile::_print(const ingest_data_header_t &hdr, ostream &out)
  
{
  out << "===== INGEST DATA HEADER =====" << endl;
  out << "  Size: " << sizeof(hdr) << endl;
  _print(hdr.id_hdr, out);
  out << "  time: " << _time2Str(hdr.time) << endl;
  out << "  sweep_num: " << hdr.sweep_num << endl;
  out << "  nrays_per_revolution: " << hdr.nrays_per_revolution << endl;
  out << "  angle_of_first_pointer: " << hdr.angle_of_first_pointer << endl;
  out << "  n_rays_total: " << hdr.n_rays_total << endl;
  out << "  n_rays_written: " << hdr.n_rays_written << endl;
  out << "  fixed_angle: " << _binAngleToDouble(hdr.fixed_angle) << endl;
  out << "  bits_per_bin: " << hdr.bits_per_bin << endl;
  out << "  data_code: " << hdr.data_code << endl;
}

void SigmetRadxFile::_print(const ray_header_t &hdr, ostream &out)
  
{
  out << "~~~~~ RAY HEADER ~~~~~" << endl;
  out << "  Size: " << sizeof(hdr) << endl;
  out << "  start_az: " << _binAngleToDouble(hdr.start_az) << endl;
  out << "  start_el: " << _binAngleToDouble(hdr.start_el) << endl;
  out << "  end_az: " << _binAngleToDouble(hdr.end_az) << endl;
  out << "  end_el: " << _binAngleToDouble(hdr.end_el) << endl;
  out << "  n_gates: " << hdr.n_gates << endl;
  out << "  seconds: " << hdr.seconds << endl;
  out << "~~~~~~~~~~~~~~~~~~~~~~" << endl;
}

void SigmetRadxFile::_print(const ext_header_ver0 &hdr, ostream &out)
{
  out << "===== EXTENDED HDR V0 =====" << endl;
  out << "  Size: " << sizeof(hdr) << endl;
  out << "  msecs_since_sweep_start: " << hdr.msecs_since_sweep_start << endl;
  out << "  calib_signal_level: " << hdr.calib_signal_level << endl;
  out << "===========================" << endl;
}

void SigmetRadxFile::_print(const ext_header_ver1 &hdr, ostream &out)
{
  out << "===== EXTENDED HDR V1 =====" << endl;
  out << "  Size: " << sizeof(hdr) << endl;
  out << "  msecs_since_sweep_start: " << hdr.msecs_since_sweep_start << endl;
  out << "  calib_signal_level: " << hdr.calib_signal_level << endl;
  out << "  az: " << _binAngleToDouble(hdr.az) << endl;
  out << "  elev: " << _binAngleToDouble(hdr.elev) << endl;
  out << "  train_order: " << _binAngleToDouble(hdr.train_order) << endl;
  out << "  elev_order: " << _binAngleToDouble(hdr.elev_order) << endl;
  out << "  pitch: " << _binAngleToDouble(hdr.pitch) << endl;
  out << "  roll: " << _binAngleToDouble(hdr.roll) << endl;
  out << "  heading: " << _binAngleToDouble(hdr.heading) << endl;
  out << "  az_rate: " << _binAngleToDouble(hdr.az_rate) << endl;
  out << "  elev_rate: " << _binAngleToDouble(hdr.elev_rate) << endl;
  out << "  pitch_rate: " << _binAngleToDouble(hdr.pitch_rate) << endl;
  out << "  roll_rate: " << _binAngleToDouble(hdr.roll_rate) << endl;
  out << "  lat: " << _binAngleToDouble(hdr.lat) << endl;
  out << "  lon: " << _binAngleToDouble(hdr.lon) << endl;
  out << "  heading_rate: " << _binAngleToDouble(hdr.heading_rate) << endl;
  out << "  alt_m_msl: " << hdr.alt_m_msl << endl;
  out << "  vel_e(m/s): " << hdr.vel_e_cm_per_sec / 100.0 << endl;
  out << "  vel_n(m/s): " << hdr.vel_n_cm_per_sec / 100.0 << endl;
  out << "  msecs_since_update: " << hdr.msecs_since_update << endl;
  out << "  vel_up(m/s): " << hdr.vel_up_cm_per_sec / 100.0 << endl;
  out << "  nav_sys_km_flag: " << hdr.nav_sys_ok_flag << endl;
  out << "  radial_vel_corr: " << hdr.radial_vel_corr / 100.0 << endl;
  out << "===========================" << endl;
}

void SigmetRadxFile::_print(const ext_header_ver2 &hdr, ostream &out)
{
  out << "===== EXTENDED HDR V2 =====" << endl;
  out << "  Size: " << sizeof(hdr) << endl;
  out << "  msecs_since_sweep_start: " << hdr.msecs_since_sweep_start << endl;
  out << "  calib_signal_level: " << hdr.calib_signal_level << endl;
  out << "  nbytes_in_header: " << hdr.nbytes_in_header << endl;
  out << "===========================" << endl;
}

void SigmetRadxFile::_print(const hrd_tdr_ext_header_t &hdr, ostream &out)
{
  out << "===== HRD TAIL RADAR EXTENDED HDR =====" << endl;
  out << "  Size: " << sizeof(hdr) << endl;
  out << "  msecs_since_sweep_start: " << hdr.msecs_since_sweep_start << endl;
  out << "  calib_signal_level: " << hdr.calib_signal_level << endl;
  out << "  nbytes_in_header: " << hdr.nbytes_in_header << endl;
  out << "  __pad_1: " << hdr.__pad_1 << endl;
  out << "  gps_age_msecs: " << hdr.gps_age_msecs << endl;
  out << "  irs_age_msecs: " << hdr.irs_age_msecs << endl;
  out << "  aamps_age_msecs: " << hdr.aamps_age_msecs << endl;
  out << "  gps_lat: " << _binAngleToDouble(hdr.gps_lat) << endl;
  out << "  gps_long: " << _binAngleToDouble(hdr.gps_long) << endl;
  out << "  gps_alt (m): " << hdr.gps_alt_cm / 100.0 << endl;
  out << "  gps_vel_e (m/s): " << hdr.gps_vel_e_cm_per_sec / 100.0 << endl;
  out << "  gps_vel_n (m/s): " << hdr.gps_vel_n_cm_per_sec / 100.0<< endl;
  out << "  gps_vel_v (m/s): " << hdr.gps_vel_v_cm_per_sec / 100.0 << endl;
  out << "  irs_lat: " << _binAngleToDouble(hdr.irs_lat) << endl;
  out << "  irs_long: " << _binAngleToDouble(hdr.irs_long) << endl;
  out << "  irs_vel_e (m/s): " << hdr.irs_vel_e_cm_per_sec / 100.0 << endl;
  out << "  irs_vel_n (m/s): " << hdr.irs_vel_n_cm_per_sec / 100.0 << endl;
  out << "  irs_vel_v (m/s): " << hdr.irs_vel_v_cm_per_sec / 100.0 << endl;
  out << "  irs_pitch: " << _binAngleToDouble(hdr.irs_pitch) << endl;
  out << "  irs_roll: " << _binAngleToDouble(hdr.irs_roll) << endl;
  out << "  irs_heading: " << _binAngleToDouble(hdr.irs_heading) << endl;
  out << "  irs_drift: " << _binAngleToDouble(hdr.irs_drift) << endl;
  out << "  irs_tru_track: " << _binAngleToDouble(hdr.irs_tru_track) << endl;
  out << "  irs_pitch_r: " << _binAngleToDouble(hdr.irs_pitch_r) << endl;
  out << "  irs_roll_r: " << _binAngleToDouble(hdr.irs_roll_r) << endl;
  out << "  irs_yaw_r: " << _binAngleToDouble(hdr.irs_yaw_r) << endl;
  out << "  irs_wind_vel (m/s): "
      << hdr.irs_wind_vel_cm_per_sec / 100.0 << endl;
  out << "  irs_wind_dir: " << _binAngleToDouble(hdr.irs_wind_dir) << endl;
  out << "  __pad_2: " << hdr.__pad_2 << endl;
  out << "  aamps_lat: " << _binAngleToDouble(hdr.aamps_lat) << endl;
  out << "  aamps_long: " << _binAngleToDouble(hdr.aamps_long) << endl;
  out << "  aamps_alt (m): " << hdr.aamps_alt_cm / 100.0 << endl;
  out << "  aamps_ground_vel (m/s): "
      << hdr.aamps_ground_vel_cm_per_sec / 100.0  << endl;
  out << "  time_stamp (UTC): " << 
    RadxTime::strm(hdr.time_stamp_secs_utc) << endl;
  out << "  aamps_vel_v (m/s): " 
      << hdr.aamps_vel_v_cm_per_sec / 100.0  << endl;
  out << "  aamps_pitch: " << _binAngleToDouble(hdr.aamps_pitch) << endl;
  out << "  aamps_roll: " << _binAngleToDouble(hdr.aamps_roll) << endl;
  out << "  aamps_heading: " << _binAngleToDouble(hdr.aamps_heading) << endl;
  out << "  aamps_drift: " << _binAngleToDouble(hdr.aamps_drift) << endl;
  out << "  aamps_track: " << _binAngleToDouble(hdr.aamps_track) << endl;
  out << "  __pad_4: " << hdr.__pad_4 << endl;
  out << "  aamps_radar_alt (m): " << hdr.aamps_radar_alt_cm / 100.0 << endl;
  out << "  aamps_wind_vel (m/s): "
      << hdr.aamps_wind_vel_cm_per_sec / 100.0 << endl;
  out << "  aamps_wind_dir: " << _binAngleToDouble(hdr.aamps_wind_dir) << endl;
  out << "  __pad_5: " << hdr.__pad_5 << endl;
  out << "  aamps_wind_vel_v (m/s): "
      << hdr.aamps_wind_vel_v_cm_per_sec / 100.0 << endl;


  out << "=======================================" << endl;
}

////////////////////////////////////////////////////////////////
// byte swap routines
//
// These only activate if _sigmetIsSwapped has been set to true.
// Otherwise they simply return.

void SigmetRadxFile::_swap(Radx::si16 *vals, int n)
{
  if (!_sigmetIsSwapped) return;
  ByteOrder::swap16(vals, n * sizeof(Radx::si16), true);
}

void SigmetRadxFile::_swap(Radx::ui16 *vals, int n)

{
  if (!_sigmetIsSwapped) return;
  ByteOrder::swap16(vals, n * sizeof(Radx::ui16), true);
}

void SigmetRadxFile::_swap(Radx::si32 *vals, int n)
{
  if (!_sigmetIsSwapped) return;
  ByteOrder::swap32(vals, n * sizeof(Radx::si32), true);
}

void SigmetRadxFile::_swap(Radx::ui32 *vals, int n)

{
  if (!_sigmetIsSwapped) return;
  ByteOrder::swap32(vals, n * sizeof(Radx::ui32), true);
}

void SigmetRadxFile::_swap(Radx::fl32 *vals, int n)

{
  if (!_sigmetIsSwapped) return;
  ByteOrder::swap32(vals, n * sizeof(Radx::fl32), true);
}

void SigmetRadxFile::_swap(sigmet_id_hdr_t &val)

{
  if (!_sigmetIsSwapped) return;
  _swap(&val.id, 2);
  _swap(&val.nbytes, 1);
  _swap(&val.flags, 1);
}

void SigmetRadxFile::_swap(sigmet_time_t &val)

{
  if (!_sigmetIsSwapped) return;
  _swap(&val.sec, 1);
  _swap(&val.msecs, 4);
}

void SigmetRadxFile::_swap(prod_header_t &val)
{
  if (!_sigmetIsSwapped) {
    return;
  }
  _swap(val.id_hdr);
  _swap(val.conf);
  _swap(val.end);
}

void SigmetRadxFile::_swap(prod_conf_t &val)
{
  if (!_sigmetIsSwapped) {
    return;
  }
  _swap(val.id_hdr);
  _swap(&val.ptype, 2);
  _swap(&val.secs_between_runs, 1);
  _swap(val.product_time);
  _swap(val.ingest_time);
  _swap(val.start_sched_time);
  _swap(&val.flags, 1);
  _swap(&val.xscale, 10);
  _swap(&val.data_type_out, 1);
  _swap(&val.data_type_in, 1);
  _swap(&val.radial_smoothing_range_km_100, 1);
  _swap(&val.n_runs, 1);
  _swap(&val.zr_coeff, 2);
  _swap(&val.twod_smooth_x, 2);
}

void SigmetRadxFile::_swap(prod_end_t &val)
{
  if (!_sigmetIsSwapped) return;
  _swap(val.oldest_data_time);
  _swap(&val.minutes_lst_west_of_gmt, 1);
  _swap(&val.minutes_rec_west_of_gmt, 1);
  _swap(&val.latitude_center, 2);
  _swap(&val.ground_ht_msl_meters, 2);
  _swap(&val.prf_hz, 2);
  _swap(&val.dsp_type, 3);
  _swap(&val.dop_filter_first_bin, 1);
  _swap(&val.wavelength_cm_100, 5);
  _swap(&val.input_file_count, 7);
  _swap(&val.lambert_lat1, 6);
  _swap(&val.log_filter_first, 2);
  _swap(&val.proj_ref_lat, 2);
  _swap(&val.sequence_num, 1);
  _swap(&val.melting_ht_m_msl, 3);
}

void SigmetRadxFile::_swap(ingest_header_t &val)
{
  if (!_sigmetIsSwapped) return;
  _swap(val.id_hdr);
  _swap(val.ingest_conf);
  _swap(val.task_conf);
}


void SigmetRadxFile::_swap(ingest_conf_t &val)
{
  if (!_sigmetIsSwapped) return;
  _swap(&val.num_data_files, 2);
  _swap(&val.total_size_files, 1);
  _swap(val.volume_time);
  _swap(&val.nbytes_in_ray_hdrs, 4);
  _swap(&val.time_zone_local_mins_west_of_gmt, 1);
  _swap(&val.time_zone_rec_mins_west_of_gmt, 1);
  _swap(&val.latitude, 2);
  _swap(&val.ground_ht_msl_meters, 6);
  _swap(&val.radar_ht_msl_cm, 8);
  _swap(&val.ht_melting_m_msl, 1);
  _swap(&val.flags, 1);
}

void SigmetRadxFile::_swap(task_conf_t &val)
{
  if (!_sigmetIsSwapped) return;
  _swap(val.id_hdr);
  _swap(val.sched_info);
  _swap(val.dsp_info);
  _swap(val.calib_info);
  _swap(val.range_info);
  _swap(val.scan_info);
  _swap(val.misc_info);
  _swap(val.end_info);
}

void SigmetRadxFile::_swap(task_sched_info_t &val)
{
  if (!_sigmetIsSwapped) return;
  _swap(&val.start_secs, 6);
  _swap(&val.flag, 1);
}

void SigmetRadxFile::_swap(dsp_data_mask_t &val)
{
  if (!_sigmetIsSwapped) return;
  _swap(&val.word_0, 6);
}

void SigmetRadxFile::_swap(task_dsp_info_t &val)
{
  if (!_sigmetIsSwapped) return;
  _swap(&val.major_mode, 2);
  _swap(val.data_mask);
  _swap(val.data_mask_orig);
  _swap(&val.low_prf_hz, 7);
  _swap(&val.prf_hz, 2);
  _swap(&val.trig_rate_flag, 5);
  _swap(&val.fixed_gain_1000, 4);
  _swap(&val.config_header_command_mask, 1);
  _swap(&val.ts_playback_flags, 1);
}

void SigmetRadxFile::_swap(task_calib_info_t &val)
{
  if (!_sigmetIsSwapped) return;
  _swap(&val.log_slope, 5);
  _swap(&val.z_calib_16, 6);
  _swap(&val.flags, 1);
  _swap(&val.ldr_bias_100, 11);
}

void SigmetRadxFile::_swap(task_range_info_t &val)
{
  if (!_sigmetIsSwapped) return;
  _swap(&val.range_first_bin_cm, 2);
  _swap(&val.n_input_gates, 2);
  _swap(&val.input_gate_spacing_cm, 2);
  _swap(&val.bin_length_variable, 3);
}

void SigmetRadxFile::_swap(scan_info_union_t &val)
{
  if (!_sigmetIsSwapped) return;
  _swap(&val.ppi.start_az, 2 + MAX_SWEEPS);
}

void SigmetRadxFile::_swap(task_scan_info_t &val)
{
  if (!_sigmetIsSwapped) return;
  _swap(&val.scan_mode, 4);
  _swap(val.u);
}

void SigmetRadxFile::_swap(task_misc_info_t &val)
{
  if (!_sigmetIsSwapped) return;
  _swap(&val.wavelength_cm_100, 1);
  _swap(&val.xmit_power_watts, 1);
  _swap(&val.flags, 2);
  _swap(&val.trunc_ht_above_radar_cm, 1);
  _swap(&val.nbytes_comment, 1);
  _swap(&val.beam_width_h, 12);
}

void SigmetRadxFile::_swap(task_end_info_t &val)
{
  if (!_sigmetIsSwapped) return;
  _swap(&val.id_major, 2);
  _swap(&val.ntasks_hybrid, 1);
  _swap(&val.task_state, 1);
  _swap(val.start_time);
}

void SigmetRadxFile::_swap(raw_product_header_t &val)
{
  if (!_sigmetIsSwapped) return;
  _swap(&val.record_num, 5);
}

void SigmetRadxFile::_swap(ingest_data_header_t &val)
{
  if (!_sigmetIsSwapped) return;
  _swap(val.id_hdr);
  _swap(val.time);
  _swap(&val.sweep_num, 8);
}

void SigmetRadxFile::_swap(ray_header_t &val)
{
  if (!_sigmetIsSwapped) return;
  _swap(&val.start_az, 6);
}

void SigmetRadxFile::_swap(ext_header_ver0 &val)
{
  if (!_sigmetIsSwapped) return;
  _swap(&val.msecs_since_sweep_start, 4);
  _swap(&val.calib_signal_level, 16);
}

void SigmetRadxFile::_swap(ext_header_ver1 &val)
{
  if (!_sigmetIsSwapped) return;
  _swap(&val.msecs_since_sweep_start, 4);
  _swap(&val.calib_signal_level, 24);
  _swap(&val.lat, 4);
  _swap(&val.lon, 4);
  _swap(&val.heading_rate, 8);
  _swap(&val.msecs_since_update, 4);
  _swap(&val.vel_up_cm_per_sec, 8);
}

void SigmetRadxFile::_swap(ext_header_ver2 &val)
{
  if (!_sigmetIsSwapped) return;
  _swap(&val.msecs_since_sweep_start, 4);
  _swap(&val.calib_signal_level, 4);
  _swap(&val.nbytes_in_header, 4);
}

void SigmetRadxFile::_swap(hrd_tdr_ext_header_t &val)
{
  if (!_sigmetIsSwapped) return;

  _swap(&val.msecs_since_sweep_start, 4);
  _swap(&val.calib_signal_level, 12);
  _swap(&val.gps_lat, 44);
  _swap(&val.irs_pitch, 16);
  _swap(&val.irs_wind_vel_cm_per_sec, 4);
  _swap(&val.irs_wind_dir, 4);
  _swap(&val.aamps_lat, 24);
  _swap(&val.aamps_pitch, 12);
  _swap(&val.aamps_radar_alt_cm, 8);
  _swap(&val.aamps_wind_dir, 4);
  _swap(&val.aamps_wind_vel_v_cm_per_sec, 4);

}
