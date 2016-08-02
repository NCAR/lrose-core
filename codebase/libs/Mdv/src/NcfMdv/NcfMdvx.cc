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
///////////////////////////////////////////////////////////////
// NcfMdvx.cc
//
// NcfMdvx object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 1999
//
///////////////////////////////////////////////////////////////
//
// The NcfMdvx object adds server capability to the Mdvx class.
//
///////////////////////////////////////////////////////////////

#include <sys/types.h>
#include <unistd.h>
#include <cstdio>
#include <didss/RapDataDir.hh>
#include <Mdv/MdvxChunk.hh>
#include <Mdv/NcfMdvx.hh>
#include <Mdv/Mdv2NcfTrans.hh>
#include <Mdv/Ncf2MdvTrans.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxField.hh>
#include <dsserver/DmapAccess.hh>
#include <dsserver/DsLdataInfo.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/str.h>
using namespace std;

NcfMdvx::NcfMdvx() : DsMdvx()
{
}

NcfMdvx::~NcfMdvx()
{
}

/////////////////////////////
// Copy constructor
//

NcfMdvx::NcfMdvx(const NcfMdvx &rhs)

{
  if (this != &rhs) {
    _copy(rhs);
  }
}

//////////////////////////////
// Assignment
//

NcfMdvx &NcfMdvx::operator=(const NcfMdvx &rhs)
  
{
  return _copy(rhs);
}

//////////////////////////////
// Assignment
//

NcfMdvx &NcfMdvx::_copy(const NcfMdvx &rhs)

{
  
  if (&rhs == this) {
    return *this;
  }

  DsMdvx::_copy(rhs);

  return *this;
  
}

////////////////////////////////////////////////
// convert MDV to NETCDF CF
// returns 0 on success, -1 on failure

int NcfMdvx::convertMdv2Ncf(const string &url)
  
{
  
  if (_currentFormat != FORMAT_MDV) {
    _errStr += "ERROR - NcfMdvx::convertMdv2Ncf.\n";
    TaStr::AddStr(_errStr, "  Url: ", url);
    TaStr::AddStr(_errStr, "  Incorrect format: ", format2Str(_currentFormat));
    TaStr::AddStr(_errStr, "  Should be: ", format2Str(FORMAT_MDV));
    return -1;
  }

  // compute temporary file path

  time_t now = time(NULL);
  DateTime dnow(now);
  pid_t pid = getpid();
  char tmpFilePath[FILENAME_MAX];
  sprintf(tmpFilePath,
          "/tmp/NcfMdvx_convertMdv2Ncf_%.4d%.2d%.2d_%.2d%.2d%.2d_%.5d.nc",
          dnow.getYear(), dnow.getMonth(), dnow.getDay(),
          dnow.getHour(), dnow.getMin(), dnow.getSec(), pid);
          
  Mdv2NcfTrans trans;
  trans.clearData();
  trans.setDebug(_debug);
  
  if (trans.translate(*this, tmpFilePath)) {
    _errStr += "ERROR - NcfMdvx::convertMdv2Ncf.\n";
    TaStr::AddStr(_errStr, "  Url: ", url);
    _errStr += trans.getErrStr();
    return -1;
  }

  // open NCF file
  
  TaFile ncfFile;
  if (ncfFile.fopen(tmpFilePath, "rb") == NULL) {
    int errNum = errno;
    _errStr += "ERROR - NcfMdvx::convertMdv2Ncf\n";
    TaStr::AddStr(_errStr, "  Url: ", url);
    TaStr::AddStr(_errStr, "  Cannot open tmp file: ", tmpFilePath);
    _errStr += strerror(errNum);
    _errStr += "\n";
    unlink(tmpFilePath);
    return -1;
  }

  // stat the file to get size
  
  if (ncfFile.fstat()) {
    int errNum = errno;
    _errStr += "ERROR - NcfMdvx::convertMdv2Ncf\n";
    TaStr::AddStr(_errStr, "  Url: ", url);
    TaStr::AddStr(_errStr, "  Cannot stat tmp file: ", tmpFilePath);
    _errStr += strerror(errNum);
    _errStr += "\n";
    ncfFile.fclose();
    unlink(tmpFilePath);
    return -1;
  }
  stat_struct_t &fileStat = ncfFile.getStat();
  off_t fileLen = fileStat.st_size;
  
  // read in buffer
  
  _ncfBuf.reserve(fileLen);
  if (ncfFile.fread(_ncfBuf.getPtr(), 1, fileLen) != fileLen) {
    int errNum = errno;
    _errStr += "ERROR - NcfMdvx::convertMdv2Ncf\n";
    TaStr::AddStr(_errStr, "  Url: ", url);
    TaStr::AddStr(_errStr, "  Cannot read tmp file: ", tmpFilePath);
    _errStr += strerror(errNum);
    _errStr += "\n";
    ncfFile.fclose();
    unlink(tmpFilePath);
    return -1;
  }
  
  // close tmp file and remove
  
  ncfFile.fclose();
  unlink(tmpFilePath);

  // set current format
  
  _currentFormat = FORMAT_NCF;

  // set times etc

  _ncfValidTime = _mhdr.time_centroid;
  _ncfGenTime = _mhdr.time_gen;
  _ncfForecastTime = _mhdr.forecast_time;
  _ncfForecastDelta = _mhdr.forecast_delta;
  if (_mhdr.data_collection_type == Mdvx::DATA_FORECAST ||
      _mhdr.data_collection_type == Mdvx::DATA_EXTRAPOLATED) {
    _ncfIsForecast = true;
  } else {
    _ncfIsForecast = false;
  }
  _ncfEpoch = _mhdr.epoch;

  // clear MDV format data

  clearFields();
  clearChunks();

  return 0;

}

////////////////////////////////////////////////
// convert NETCDF CF to MDV
// given an object containing a netcdf file buffer
// returns 0 on success, -1 on failure

int NcfMdvx::convertNcf2Mdv(const string &url)
  
{

  if (_currentFormat != FORMAT_NCF) {
    _errStr += "ERROR - NcfMdvx::convertNcf2Mdv.\n";
    TaStr::AddStr(_errStr, "  Url: ", url);
    TaStr::AddStr(_errStr, "  Incorrect format: ", format2Str(_currentFormat));
    TaStr::AddStr(_errStr, "  Should be: ", format2Str(FORMAT_NCF));
    return -1;
  }

  // compute temporary file path
  
  time_t now = time(NULL);
  DateTime dnow(now);
  pid_t pid = getpid();
  char tmpFilePath[FILENAME_MAX];
  sprintf(tmpFilePath,
          "/tmp/NcfMdvx_convertNcf2Mdv_%.4d%.2d%.2d_%.2d%.2d%.2d_%.5d.nc",
          dnow.getYear(), dnow.getMonth(), dnow.getDay(),
          dnow.getHour(), dnow.getMin(), dnow.getSec(), pid);

  // write nc buffer to file

  if (_write_buffer_to_file(tmpFilePath, _ncfBuf.getLen(), _ncfBuf.getPtr())) {
    _errStr += "ERROR - NcfMdvx::convertNcf2Mdv\n";
    TaStr::AddStr(_errStr, "  Url: ", url);
    TaStr::AddStr(_errStr, "  Cannot write buffe to tmp file: ", tmpFilePath);
    return -1;
  }
          
  // create translator
  
  Ncf2MdvTrans trans;
  trans.setDebug(_debug);
  
  // perform translation
  // returns 0 on success, -1 on failure

  if (trans.translate(tmpFilePath, *this)) {
    _errStr += "ERROR - NcfMdvx::convertNcf2Mdv\n";
    TaStr::AddStr(_errStr, "  Url: ", url);
    TaStr::AddStr(_errStr, "  Cannot translate file: ", tmpFilePath);
    TaStr::AddStr(_errStr, trans.getErrStr());
    unlink(tmpFilePath);
    return -1;
  }

  // remove tmp file
  
  unlink(tmpFilePath);

  // remove netcdf data
  
  clearNcf();

  // set format to MDV
  
  _currentFormat = FORMAT_MDV;

  return 0;

}

//////////////////////////////////////////////////////
// Read the headers from a NETCDF CF file into MDV, given the file path
// Convert to NCF at the end if required
// Currently we read all of the data, which will include the headers.
// returns 0 on success, -1 on failure

int NcfMdvx::readAllHeadersNcf(const string &url)
  
{
  // First read in the whole file.  We do this because we don't have a way
  // to easily read the field header information without reading the field
  // data also.

  if (readNcf(url))
  {
    _errStr += "ERROR - NcfMdvx::readAllHeadersNcf\n";
    TaStr::AddStr(_errStr, "  Error reading NCF file and translating to MDV");
    return -1;
  }
  
  // Now fill in the file headers.  This isn't done in the translation because
  // the translation doesn't know that we are reading the entire file to get
  // the headers.

  _mhdrFile = _mhdr;
  
  for (size_t i = 0; i < _fields.size(); ++i)
  {
    MdvxField *field = _fields[i];
    
    _fhdrsFile.push_back(field->getFieldHeader());
    _vhdrsFile.push_back(field->getVlevelHeader());
  }
  
  for (size_t i = 0; i < _chunks.size(); ++i)
  {
    MdvxChunk *chunk = _chunks[i];
    
    _chdrsFile.push_back(chunk->getHeader());
  }
  
  return 0;
}

//////////////////////////////////////////////////////
// Read a NETCDF CF file into MDV, given the file path
// Convert to NCF at the end if required
// returns 0 on success, -1 on failure

int NcfMdvx::readNcf(const string &url)
  
{

  if (_currentFormat != FORMAT_NCF) {
    _errStr += "ERROR - NcfMdvx::readNcf\n";
    TaStr::AddStr(_errStr, "  Url: ", url);
    TaStr::AddStr(_errStr, "  Path ", _pathInUse);
    TaStr::AddStr(_errStr, "  Incorrect format: ", format2Str(_currentFormat));
    TaStr::AddStr(_errStr, "  Should be: ", format2Str(FORMAT_NCF));
    return -1;
  }

  // create translator
  
  Ncf2MdvTrans trans;
  trans.setDebug(_debug);
  
  // perform translation
  // returns 0 on success, -1 on failure
  
  if (trans.translate(_pathInUse, *this)) {
    _errStr += "ERROR - NcfMdvx::readNcf\n";
    TaStr::AddStr(_errStr, "  Url: ", url);
    TaStr::AddStr(_errStr, "  Path ", _pathInUse);
    TaStr::AddStr(_errStr, "  Cannot translate file to MDV");
    TaStr::AddStr(_errStr, trans.getErrStr());
    return -1;
  }

  // remove netcdf data
  
  clearNcf();

  // set nc-specific times from path

  _set_times_ncf();

  // set format to MDV
  
  _currentFormat = FORMAT_MDV;

  // convert back to NCF if needed
  // with read constraints having been applied
  
  if (_readFormat == FORMAT_NCF) {
    if (convertMdv2Ncf(_pathInUse)) {
      _errStr += "ERROR - NcfMdvx::readNcf\n";
      TaStr::AddStr(_errStr, "  Url: ", url);
      TaStr::AddStr(_errStr, "  Path ", _pathInUse);
      TaStr::AddStr(_errStr, "  Cannot translate file to NCF");
      TaStr::AddStr(_errStr, trans.getErrStr());
      return -1;
    }
  }

  return 0;

}

//////////////////////////////////////////////////////
// Read the metadata from a RADX file, given the file path
// Fill out the Mdv file headers
// returns 0 on success, -1 on failure

int NcfMdvx::readAllHeadersRadx(const string &url)
  
{

  // set up object for reading file
  
  RadxFile inFile;
  if (_debug) {
    inFile.setDebug(true);
  }

  inFile.setReadMetadataOnly(true);
  inFile.setReadRemoveLongRange(true);
  
  // read file in to RadxVol object
  
  RadxVol vol;
  if (inFile.readFromPath(_pathInUse, vol)) {
    _errStr += "ERROR - NcfMdvx::readAllHeadersRadx.\n";
    _errStr += "Cannot read in files.\n";
    TaStr::AddStr(_errStr, "  path: ", _pathInUse);
    _errStr += inFile.getErrStr();
    return -1;
  }

  // read in first field with data

  RadxVol vol0;
  if (vol.getNFields() > 0) {
    RadxField *rfld = vol.getField(0);
    string firstFieldName = rfld->getName();
    RadxFile inFile0;
    inFile0.addReadField(firstFieldName);
    if (inFile0.readFromPath(_pathInUse, vol0) == 0) {
      vol0.computeMaxNGates();
    }
  }

  // make sure sweeps are in ascending order, as required by MDV

  vol.reorderSweepsAsInFileAscendingAngle();
  vol.reorderSweepsAscendingAngle();

  // set format to MDV
  
  _currentFormat = FORMAT_MDV;

  // Now fill in the file headers.  This isn't done in the translation because
  // the translation doesn't know that we are reading the entire file to get
  // the headers.

  clear();

  // set master header

  setBeginTime(vol.getStartTimeSecs());
  setEndTime(vol.getEndTimeSecs());
  setValidTime(vol.getEndTimeSecs());
  setDataCollectionType(Mdvx::DATA_MEASURED);
  _mhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
  _mhdr.vlevel_type = Mdvx::VERT_TYPE_ELEV;
  _mhdr.n_fields = vol.getNFields();
  
  _mhdr.sensor_lat = vol.getLatitudeDeg();
  _mhdr.sensor_lon = vol.getLongitudeDeg();
  _mhdr.sensor_alt = vol.getAltitudeKm();

  _mhdr.max_nx = 0;
  _mhdr.max_ny = 0;
  _mhdr.max_nz = 0;

  const vector<RadxSweep *> &sweepsInFile = vol.getSweepsAsInFile();

  // add field headers

  for (size_t ii = 0; ii < vol.getNFields(); ii++) {

    RadxField *rfld = vol.getField(ii);
    Mdvx::field_header_t fhdr;
    Mdvx::vlevel_header_t vhdr;
    MEM_zero(fhdr);
    MEM_zero(vhdr);

    fhdr.nx = vol0.getMaxNGates();
    fhdr.ny = vol0.getNRays();
    fhdr.nz = sweepsInFile.size();
    
    if (fhdr.nx > _mhdr.max_nx) _mhdr.max_nx = fhdr.nx;
    if (fhdr.ny > _mhdr.max_ny) _mhdr.max_ny = fhdr.ny;
    if (fhdr.nz > _mhdr.max_nz) _mhdr.max_nz = fhdr.nz;

    fhdr.proj_type = Mdvx::PROJ_POLAR_RADAR;
    fhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
    fhdr.vlevel_type = Mdvx::VERT_TYPE_ELEV;
    fhdr.dz_constant = false;

    fhdr.proj_origin_lat = vol.getLatitudeDeg();
    fhdr.proj_origin_lon = vol.getLongitudeDeg();

    fhdr.grid_dx = 0.0;
    fhdr.grid_dy = 0.0;
    fhdr.grid_dz = 0.0;
    
    fhdr.grid_minx = 0.0;
    fhdr.grid_miny = 0.0;
    fhdr.grid_minz = 0.0;

    if (vol0.getNRays() > 0) {
      const RadxRay *ray0 = vol0.getRays()[0];
      fhdr.grid_dx = ray0->getGateSpacingKm();
      fhdr.grid_minx = ray0->getStartRangeKm();
      if (vol0.checkIsRhi()) {
        fhdr.grid_miny = ray0->getElevationDeg();
      } else {
        fhdr.grid_miny = ray0->getAzimuthDeg();
      }
    }
    
    if (vol0.getNSweeps() > 0) {
      const RadxSweep *sweep0 = vol0.getSweeps()[0];
      fhdr.grid_dy = sweep0->getAngleResDeg();
      fhdr.grid_minz = sweep0->getFixedAngleDeg();
    }
    
    STRncopy(fhdr.field_name_long,
             rfld->getLongName().c_str(), MDV_LONG_FIELD_LEN);
    
    STRncopy(fhdr.field_name,
             rfld->getName().c_str(), MDV_SHORT_FIELD_LEN);

    STRncopy(fhdr.units,
             rfld->getUnits().c_str(), MDV_UNITS_LEN);

    for (size_t jj = 0; jj < sweepsInFile.size(); jj++) {
      const RadxSweep *swp = sweepsInFile[jj];
      if (swp->getSweepMode() == Radx::SWEEP_MODE_RHI) {
        fhdr.native_vlevel_type = Mdvx::VERT_TYPE_AZ;
        fhdr.vlevel_type = Mdvx::VERT_TYPE_AZ;
        vhdr.type[jj] = Mdvx::VERT_TYPE_AZ;
      } else {
        vhdr.type[jj] = Mdvx::VERT_TYPE_ELEV;
      }
      vhdr.level[jj] = swp->getFixedAngleDeg();
    }
    
    _fhdrsFile.push_back(fhdr);
    _vhdrsFile.push_back(vhdr);

  } // ii
  
  _mhdrFile = _mhdr;

  return 0;

}

////////////////////////////////////////////////
// Read a RADX-type file, convert to MDV
// Convert to RADX at the end if required
// returns 0 on success, -1 on failure

int NcfMdvx::readRadx(const string &url)
  
{
  
  if (_currentFormat != FORMAT_RADX) {
    _errStr += "ERROR - NcfMdvx::readRadx.\n";
    TaStr::AddStr(_errStr, "  Url: ", url);
    TaStr::AddStr(_errStr, "  Path: ", _pathInUse);
    TaStr::AddStr(_errStr, "  Incorrect format: ", format2Str(_currentFormat));
    TaStr::AddStr(_errStr, "  Should be: ", format2Str(FORMAT_RADX));
    return -1;
  }

  // set up object for reading files
  
  RadxFile inFile;
  if (_debug) {
    inFile.setDebug(true);
  }
  // set vertical limits
  if (_readVlevelLimitsSet) {
    inFile.setReadFixedAngleLimits(_readMinVlevel, _readMaxVlevel);
    inFile.setReadStrictAngleLimits(false);
  } else if (_readPlaneNumLimitsSet) {
    inFile.setReadSweepNumLimits(_readMinPlaneNum, _readMaxPlaneNum);
    inFile.setReadStrictAngleLimits(false);
  }
  // set field names
  if (_readFieldNames.size() > 0) {
    for (size_t ii = 0; ii < _readFieldNames.size(); ii++) {
      inFile.addReadField(_readFieldNames[ii]);
    }
  }

  // ignore rays with antenna transitions
  
  inFile.setReadIgnoreTransitions(true);
  inFile.setReadRemoveLongRange(true);

  // read file in to RadxVol object
  
  RadxVol vol;
  if (inFile.readFromPath(_pathInUse, vol)) {
    _errStr += "ERROR - NcfMdvx::readRadx.\n";
    _errStr += "Cannot read in files.\n";
    TaStr::AddStr(_errStr, "  path: ", _pathInUse);
    _errStr += inFile.getErrStr();
    return -1;
  }
  
  // make sure sweeps are in ascending order, as required by MDV
  
  vol.reorderSweepsAsInFileAscendingAngle();
  vol.reorderSweepsAscendingAngle();

  // make sure gate geom is constant

  vol.remapToPredomGeom();
  
  // convert into Mdv
  
  Ncf2MdvTrans trans;
  trans.setDebug(_debug);
  if (trans.translateRadxVol(_pathInUse, vol, *this)) {
    _errStr += "ERROR - NcfMdvx::readRadx.\n";
    _errStr += "  Cannot translate RadxVol.\n";
    return -1;
  }

  // set format to MDV
  
  _currentFormat = FORMAT_MDV;

  // convert to NCF if needed
  
  if (_readFormat == FORMAT_NCF) {
    if (convertMdv2Ncf(_pathInUse)) {
      _errStr += "ERROR - NcfMdvx::readRadx\n";
      TaStr::AddStr(_errStr, "  Url: ", url);
      TaStr::AddStr(_errStr, "  Path ", _pathInUse);
      TaStr::AddStr(_errStr, "  Cannot translate file to NCF");
      TaStr::AddStr(_errStr, trans.getErrStr());
      return -1;
    }
  }

  return 0;

}

////////////////////////////////////////////////
// constrain NETCDF CF using read qualifiers
// returns 0 on success, -1 on failure

int NcfMdvx::constrainNcf(const string &url)
  
{

  // convert the NCF data to MDV format
  // This will also apply the read qualifiers
  
  if (convertNcf2Mdv(url)) {
    _errStr += "ERROR - NcfMdvx::constrainNcf.\n";
    TaStr::AddStr(_errStr, "  Url: ", url);
    return -1;
  }
    
  // convert back to NCF

  if (convertMdv2Ncf(url)) {
    _errStr += "ERROR - NcfMdvx::constrainNcf.\n";
    TaStr::AddStr(_errStr, "  Url: ", url);
    return -1;
  }
  
  return 0;

}

////////////////////////////////////////////////
// write to specified directory
// returns 0 on success, -1 on failure

int NcfMdvx::writeToDir(const string &url)
  
{
  
  if (_debug) {
    cerr << "WRITE TO DIR" << endl;
    printWriteOptions(cerr);
    cerr << "  current format: " << format2Str(_currentFormat) << endl;
    cerr << "  write format: " << format2Str(_writeFormat) << endl;
  }

  if (_currentFormat == FORMAT_NCF && _writeFormat == FORMAT_NCF) {
    // NCF to NCF - apply constraints
    if(_constrainNcfAndWrite(url)) {
      _errStr += "ERROR - NcfMdvx::writeToDir\n";
      return -1;
    } else {
      return 0;
    }
  }

  if (_currentFormat == FORMAT_NCF && _writeFormat == FORMAT_MDV) {
    // convert NCF to MDV and write
    if (_convertNcfToMdvAndWrite(url)) {
      _errStr += "ERROR - NcfMdvx::writeToDir\n";
      return -1;
    } else {
      return 0;
    }
  }

  if (_currentFormat == FORMAT_MDV && _writeFormat == FORMAT_NCF) {
    // convert MDV to NCF and write
    if (_convertMdvToNcfAndWrite(url)) {
      _errStr += "ERROR - NcfMdvx::writeToDir\n";
      return -1;
    } else {
      return 0;
    }
  }

  // ERROR - MDV to MDV not supported

  _errStr += "ERROR - NcfMdvx::writeToDir\n";
  TaStr::AddStr(_errStr, "  Url: ", url);
  TaStr::AddStr(_errStr, "  Both current and write formats are MDV");
  TaStr::AddStr(_errStr, "  NcfMdvx does not handle that case");
  return -1;

}

////////////////////////////////////////////////
// Convert NCF format to MDV format, and write
// returns 0 on success, -1 on failure

int NcfMdvx::_convertNcfToMdvAndWrite(const string &url)
  
{
  
  if (convertNcf2Mdv(url)) {
    _errStr += "ERROR - NcfMdvx::_convertNcfToMdvAndWrite()\n";
    return -1;
  }

  // local - direct write
  DsURL dsUrl(url);
  string writeDir(dsUrl.getFile());
  if (Mdvx::writeToDir(writeDir)) {
    _errStr += "ERROR - NcfMdvx::_convertNcfToMdvAndWrite()\n";
    return -1;
  }
    
  // reg with data mapper - the base class uses LdataInfo which does
  // not register

  DmapAccess dmap;
  string dataType = "mdv";
  if (_writeAsForecast) {
    int forecast_delta = _mhdr.time_centroid - _mhdr.time_gen;
    dmap.regLatestInfo(_mhdr.time_gen, writeDir, dataType, forecast_delta);
  } else {
    dmap.regLatestInfo(_mhdr.time_centroid, writeDir, dataType);
  }
  
  return 0;

}

////////////////////////////////////////////////
// Convert MDV format to NCF format, and write
// returns 0 on success, -1 on failure

int NcfMdvx::_convertMdvToNcfAndWrite(const string &url)
  
{

  // compute paths

  DsURL dsUrl(url);
  string outputDir;
  RapDataDir.fillPath(dsUrl.getFile(), outputDir);
  string outputPath;
  string dataType = "ncf";

  if (getProjection() == Mdvx::PROJ_POLAR_RADAR) {
    
    //radial data type
    
    Mdv2NcfTrans trans;
    trans.setDebug(_debug);
    trans.setRadialFileType(_ncfRadialFileType);
    if (trans.translateToCfRadial(*this, outputDir)) {
      TaStr::AddStr(_errStr, "ERROR - NcfMdvx::_convertMdvToNcfAndWrite()");
      TaStr::AddStr(_errStr, trans.getErrStr());
      return -1;
    }
    outputPath = trans.getNcFilePath();
    
    if (_ncfRadialFileType == DsMdvx::RADIAL_TYPE_CF_RADIAL) {
      dataType = "cfradial";
    } else if (_ncfRadialFileType == DsMdvx::RADIAL_TYPE_DORADE) {
      dataType = "dorade";
    } else if (_ncfRadialFileType == DsMdvx::RADIAL_TYPE_UF) {
      dataType = "uf";
    }
    
  } else {
    
    // basic CF - translate from Mdv
    
    outputPath = _computeNcfOutputPath(outputDir);
    Mdv2NcfTrans trans;
    trans.setDebug(_debug);
    if (trans.translate(*this, outputPath)) {
      cerr << "ERROR - NcfMdvx::_convertMdvToNcfAndWrite()" << endl;
      cerr << trans.getErrStr() << endl;
      return -1;
    }
    
  }
    
  // write latest data info
    
  _doWriteLdataInfo(outputDir, outputPath, dataType);
  _pathInUse = outputPath;

  return 0;

}

////////////////////////////////////////////////
// Constrain NCF and write
// First convert to MDV, which applies constraints.
// The convert back to NCF and write.
// returns 0 on success, -1 on failure

int NcfMdvx::_constrainNcfAndWrite(const string &url)
  
{
  
  if (convertNcf2Mdv(url)) {
    _errStr += "ERROR - NcfMdvx::_constrainNcfToMdvAndWrite()\n";
    return -1;
  }

  if (_convertMdvToNcfAndWrite(url)) {
    _errStr += "ERROR - NcfMdvx::_constrainNcfToMdvAndWrite()\n";
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////
// write as forecast?

bool NcfMdvx::_getWriteAsForecast()
{
  
  if (_ifForecastWriteAsForecast) {
    if (_currentFormat == FORMAT_NCF) {
      if (_ncfIsForecast) {
        return true;
      }
    } else {
      if (_mhdr.data_collection_type == Mdvx::DATA_FORECAST ||
          _mhdr.data_collection_type == Mdvx::DATA_EXTRAPOLATED) {
        return true;
      }
    }
  }

  return _writeAsForecast;

}
  
/////////////////////////////////////////////////////////
// compute output path for Ncf files

string NcfMdvx::_computeNcfOutputPath(const string &outputDir)
{
  
  // check environment for write control directives
  
  _checkEnvBeforeWrite();

  // compute path
  
  char yearSubdir[MAX_PATH_LEN];
  char outputBase[MAX_PATH_LEN];
  int forecastDelta = getForecastLeadSecs();
  bool writeAsForecast = _getWriteAsForecast();
  
  if (writeAsForecast) {
    
    if (_mhdr.data_collection_type != Mdvx::DATA_FORECAST && 
        _mhdr.data_collection_type != Mdvx::DATA_EXTRAPOLATED)
        _mhdr.data_collection_type = Mdvx::DATA_FORECAST;
 
    date_time_t genTime;
    genTime.unix_time = getGenTime();
    uconvert_from_utime(&genTime);
    
    sprintf(yearSubdir, "%.4d", genTime.year);

    if (!_useExtendedPaths) {
      sprintf(outputBase, "%.4d%.2d%.2d%sg_%.2d%.2d%.2d%sf_%.8d",
              genTime.year, genTime.month, genTime.day,
              PATH_DELIM, genTime.hour, genTime.min, genTime.sec,
              PATH_DELIM, forecastDelta);
    } else {
      sprintf(outputBase,
              "%.4d%.2d%.2d%s"
              "g_%.2d%.2d%.2d%s"
              "%.4d%.2d%.2d_g_%.2d%.2d%.2d_f_%.8d",
              genTime.year, genTime.month, genTime.day, PATH_DELIM,
              genTime.hour, genTime.min, genTime.sec, PATH_DELIM,
              genTime.year, genTime.month, genTime.day,
              genTime.hour, genTime.min, genTime.sec,
              forecastDelta);
    }

  } else {

    date_time_t validTime;
    validTime.unix_time = getValidTime();
    uconvert_from_utime(&validTime);
    sprintf(yearSubdir, "%.4d", validTime.year);
    if (!_useExtendedPaths) {
      sprintf(outputBase, "%.4d%.2d%.2d%s%.2d%.2d%.2d",
              validTime.year, validTime.month, validTime.day,
              PATH_DELIM, validTime.hour, validTime.min, validTime.sec);
    } else {
      sprintf(outputBase,
              "%.4d%.2d%.2d%s"
              "%.4d%.2d%.2d_%.2d%.2d%.2d",
              validTime.year, validTime.month, validTime.day, PATH_DELIM,
              validTime.year, validTime.month, validTime.day,
              validTime.hour, validTime.min, validTime.sec);
    }

  }

  string outputName;
  if (_writeAddYearSubdir) {
    outputName += yearSubdir;
    outputName += PATH_DELIM;
  }
  outputName += outputBase;
  outputName += ".mdv.nc";

  string outputPath(outputDir);
  outputPath += PATH_DELIM;
  outputPath += outputName;

  return outputPath;

}

//////////////////////////////////////

void NcfMdvx::_doWriteLdataInfo(const string &outputDir,
                                const string &outputPath,
                                const string &dataType)
{
  
  DsLdataInfo ldata(outputDir, _debug);
  ldata.setPathAndTime(outputDir, outputPath);
  time_t latestTime;
  if (_getWriteAsForecast()) {
    latestTime = getGenTime();
    int leadtime = _mhdr.forecast_delta;
    ldata.setLeadTime(leadtime);
    ldata.setIsFcast(true);
  } else {
    latestTime = getValidTime();
  }
  ldata.setDataType(dataType);
  ldata.write(latestTime);

}
