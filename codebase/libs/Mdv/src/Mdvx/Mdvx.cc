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
//////////////////////////////////////////////////////////
// Mdvx.cc
//
// Mdv access class
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 1999
//
//////////////////////////////////////////////////////////
//
// See <Mdv/Mdvx.hh> for details.
//
///////////////////////////////////////////////////////////

#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxChunk.hh>
#include <toolsa/mem.h>
#include <toolsa/TaStr.hh>
#include <toolsa/DateTime.hh>
#include <dataport/bigend.h>
#include <cmath>
using namespace std;

// initialize constants

const string Mdvx::FILE_LABEL = "NCAR RAP MDV FILE";
const int Mdvx::REVISION_NUMBER = 1;
const int Mdvx::MASTER_HEAD_MAGIC_COOKIE = 14142;
const int Mdvx::FIELD_HEAD_MAGIC_COOKIE = 14143;
const int Mdvx::VLEVEL_HEAD_MAGIC_COOKIE = 14144;
const int Mdvx::CHUNK_HEAD_MAGIC_COOKIE = 14145;
const int Mdvx::NUM_MASTER_HEADER_SI32 = 41;
const int Mdvx::NUM_MASTER_HEADER_FL32 = 21;
const int Mdvx::NUM_MASTER_HEADER_32 = 63;
const int Mdvx::NUM_FIELD_HEADER_SI32 = 39;
const int Mdvx::NUM_FIELD_HEADER_FL32 = 31;
const int Mdvx::NUM_FIELD_HEADER_32 = 71;
const int Mdvx::NUM_VLEVEL_HEADER_SI32 = 127;
const int Mdvx::NUM_VLEVEL_HEADER_FL32 = 127;
const int Mdvx::NUM_VLEVEL_HEADER_32 = 255;
const int Mdvx::NUM_CHUNK_HEADER_SI32 = 6;
const int Mdvx::NUM_CHUNK_HEADER_32 = 7;

char Mdvx::_printStr[_printStrLen];

//////////////////////
// Default Constructor
//

Mdvx::Mdvx()

{
  _debug = false;
  _heartbeatFunc = NULL;
  clear();
}

/////////////////////////////
// Copy constructor
//

Mdvx::Mdvx(const Mdvx &rhs)

{
  if (this != &rhs) {
    clear();
    _copy(rhs);
  }
}

/////////////////////////////
// Destructor

Mdvx::~Mdvx()

{
  clear();
}

/////////////////////////////
// Assignment
//

Mdvx &Mdvx::operator=(const Mdvx &rhs)

{
  return _copy(rhs);
}

////////////////////////////////////////////
// clear all memory, reset to starting state

void Mdvx::clear()

{
  _appName = "unknown";
  clearMasterHeader();
  clearRead();
  clearWrite();
  clearTimeListMode();
  clearNcf();
  clearFields();
  clearChunks();
  _currentFormat = FORMAT_MDV;
  _useExtendedPaths = false;
  _writeAddYearSubdir = false;
}

///////////////////////////////
// clear field and chunk memory

void Mdvx::clearFields()

{
  for (unsigned int i = 0; i < _fields.size(); i++) {
    delete _fields[i];
  }
  _fields.clear();
  _mhdr.n_fields = 0;
}

void Mdvx::clearChunks()
  
{
  for (unsigned int i = 0; i < _chunks.size(); i++) {
    delete _chunks[i];
  }
  _chunks.clear();
  _mhdr.n_chunks = 0;
}

/////////////////////////////
// copy
//

Mdvx &Mdvx::_copy(const Mdvx &rhs)

{

  if (&rhs == this) {
    return *this;
  }

  // copy members

  // headers

  _errStr = rhs._errStr;
  _appName = rhs._appName;
  _debug = rhs._debug;
  _mhdrFile = rhs._mhdrFile;
  _fhdrsFile = rhs._fhdrsFile;
  _vhdrsFile = rhs._vhdrsFile;
  _chdrsFile = rhs._chdrsFile;
  _mhdr = rhs._mhdr;
  _dataSetInfo = rhs._dataSetInfo;

  // fields
  
  for (size_t i = 0; i < _fields.size(); i++) {
    delete _fields[i];
  }
  _fields.clear();
  for (size_t i = 0; i < rhs._fields.size(); i++) {
    MdvxField *fldp = new MdvxField(*rhs._fields[i]);
    _fields.push_back(fldp);
  }

  // chunks

  for (size_t i = 0; i < _chunks.size(); i++) {
    delete _chunks[i];
  }
  _chunks.clear();
  for (size_t i = 0; i < rhs._chunks.size(); i++) {
    MdvxChunk *chunkp = new MdvxChunk(*rhs._chunks[i]);
    _chunks.push_back(chunkp);
  }

  // formats

  _currentFormat = rhs._currentFormat;
  _readFormat = rhs._readFormat;
  _writeFormat = rhs._writeFormat;

  // read request members

  _readTimeSet = rhs._readTimeSet;
  _readSearchMode = rhs._readSearchMode;
  _readSearchTime = rhs._readSearchTime;
  _readSearchMargin = rhs._readSearchMargin;
  _readForecastLeadTime = rhs._readForecastLeadTime;
  _readDir = rhs._readDir;

  _readPathSet = rhs._readPathSet;
  _readPath = rhs._readPath;

  _readQualifiersActive = rhs._readQualifiersActive;

  _readHorizLimitsSet = rhs._readHorizLimitsSet;
  _readMinLat = rhs._readMinLat;
  _readMinLon = rhs._readMinLon;
  _readMaxLat = rhs._readMaxLat;
  _readMaxLon = rhs._readMaxLon;

  _readVlevelLimitsSet = rhs._readVlevelLimitsSet;
  _readMinVlevel = rhs._readMinVlevel;
  _readMaxVlevel = rhs._readMaxVlevel;
  
  _readPlaneNumLimitsSet = rhs._readPlaneNumLimitsSet;
  _readMinPlaneNum = rhs._readMinPlaneNum;
  _readMaxPlaneNum = rhs._readMaxPlaneNum;
  
  _readEncodingType = rhs._readEncodingType;
  _readCompressionType = rhs._readCompressionType;
  _readScalingType = rhs._readScalingType;
  _readScale = rhs._readScale;
  _readBias = rhs._readBias;

  _readComposite = rhs._readComposite;
  _readFillMissing = rhs._readFillMissing;

  _readMaxVsectSamples = rhs._readMaxVsectSamples;
  _readNVsectSamples = rhs._readNVsectSamples;
  _readMaxVsectSamples = rhs._readMaxVsectSamples;
 
  _readVsectAsRhi = rhs._readVsectAsRhi;
  _readRhiAsPolar = rhs._readRhiAsPolar;
  _readRhiMaxAzError = rhs._readRhiMaxAzError;
  _readRhiRespectUserDist = rhs._readRhiRespectUserDist;

  _readFieldNums = rhs._readFieldNums;
  _readFieldNames = rhs._readFieldNames;
  _readChunkNums = rhs._readChunkNums;

  _readRemapSet = rhs._readRemapSet;
  _readRemapCoords = rhs._readRemapCoords;
  _readAutoRemap2LatLon = rhs._readAutoRemap2LatLon;
  
  _readDecimate = rhs._readDecimate;
  _readDecimateMaxNxy = rhs._readDecimateMaxNxy;

  _readSpecifyVlevelType = rhs._readSpecifyVlevelType;
  _readVlevelType = rhs._readVlevelType;
  
  _readFieldFileHeaders = rhs._readFieldFileHeaders;

  _readTimeListAlso = rhs._readTimeListAlso;
  _readAsSingleBuffer = rhs._readAsSingleBuffer;

  // write request members

  _writeLdataInfo = rhs._writeLdataInfo;
  _writeAsForecast = rhs._writeAsForecast;
  _useExtendedPaths = rhs._useExtendedPaths;
  _writeAddYearSubdir = rhs._writeAddYearSubdir;

  // path in use - the path being used for reading/writing

  _noFilesFoundOnRead = rhs._noFilesFoundOnRead;
  _pathInUse = rhs._pathInUse;

  // vertical section state

  _vsectDisableInterp = rhs._vsectDisableInterp;
  _vsectWayPts = rhs._vsectWayPts;
  _vsectSamplePts = rhs._vsectSamplePts;
  _vsectSegments = rhs._vsectSegments;
  _vsectDxKm = rhs._vsectDxKm;
  _vsectTotalLength = rhs._vsectTotalLength;

  // time list

  _timeList = rhs._timeList;

  // buffers

  _singleBuf = rhs._singleBuf;
  _xmlHdr = rhs._xmlHdr;
  _xmlBuf = rhs._xmlBuf;

  // netcdf
  
  _ncfValidTime = rhs._ncfValidTime;
  _ncfGenTime = rhs._ncfGenTime;
  _ncfForecastTime = rhs._ncfForecastTime;
  _ncfForecastDelta = rhs._ncfForecastDelta;
  _ncfIsForecast = rhs._ncfIsForecast;
  _ncfEpoch = rhs._ncfEpoch;
  _ncfBuf = rhs._ncfBuf;
  _ncfFileSuffix = rhs._ncfFileSuffix;
  _ncfConstrained = rhs._ncfConstrained;

  return *this;

}

///////////////////////////////////////////////////////////////////
// add a field
//
// The field must have been created using 'new'.
// The field object becomes 'owned' by the Mdvx object, and will be
// freed by it.

void Mdvx::addField(MdvxField *field)
{
  _fields.push_back(field);
  _mhdr.n_fields = _fields.size();
}

///////////////////////////////////////////////////////////////////
// delete a field
//
// The field to be deleted must already be managed by the
// Mdvx object. It will be freed.
//
// Returns -1 if field does not exist, 0 otherwise.

int Mdvx::deleteField(MdvxField *toBeDeleted)
{
  for (vector<MdvxField *>::iterator ii = _fields.begin();
       ii != _fields.end(); ii++) {
    if (*ii == toBeDeleted) {
      delete *ii;
      _fields.erase(ii);
      _mhdr.n_fields = _fields.size();
      return 0;
    }
  }
  return -1;
}

///////////////////////////////////////////////////////////////////
// add a chunk
//
// The chunk must have been created using 'new'.
// The chunk object becomes 'owned' by the Mdvx object, and will be
// freed by it.

void Mdvx::addChunk(MdvxChunk *chunk)
{
  _chunks.push_back(chunk);
  _mhdr.n_chunks = _chunks.size();
}

///////////////////////////////////////////////////////////////////
// delete a chunk
//
// The chunk to be deleted must already be managed by the
// Mdvx object. It will be freed.
//
// Returns -1 if chunk does not exist, 0 otherwise.

int Mdvx::deleteChunk(MdvxChunk *toBeDeleted)
{
  for (vector<MdvxChunk *>::iterator ii = _chunks.begin();
       ii != _chunks.end(); ii++) {
    if (*ii == toBeDeleted) {
      delete *ii;
      _chunks.erase(ii);
      _mhdr.n_chunks = _chunks.size();
      return 0;
    }
  }
  return -1;
}

////////////////////////
// get the primary times

// valid time is the centroid time

time_t Mdvx::getValidTime() const
{
  if (isNcf(_currentFormat)) {
    return _ncfValidTime;
  } else {
    return _mhdr.time_centroid;
  }
}

// generate time for forecast data

time_t Mdvx::getGenTime() const
{
  if (isNcf(_currentFormat)) {
    return _ncfGenTime;
  } else {
    return _mhdr.time_gen;
  }
}

// forecast time

time_t Mdvx::getForecastTime() const
{
  if (isNcf(_currentFormat)) {
    return _ncfForecastTime;
  } else {
    return _mhdr.forecast_time;
  }
}

// lead time of forecast

time_t Mdvx::getForecastLeadSecs() const
{
  if (isNcf(_currentFormat)) {
    return _ncfForecastDelta;
  } else {
    return _mhdr.forecast_delta;
  }
}
  
////////////////////////////////////////////////////////
// Get references for headers as they appear in the file.
// Use these after a call to readAllHeaders().
  
const Mdvx::master_header_t &Mdvx::getMasterHeaderFile() const {
  return (_mhdrFile);
}

// get vector of field headers exactly as in file

const vector<Mdvx::field_header_t>
  &Mdvx::getFieldHeadersFile() const {
  return _fhdrsFile;
}

// get vector of vlevel headers exactly as in file

const vector<Mdvx::vlevel_header_t>
  &Mdvx::getVlevelHeadersFile() const {
  return _vhdrsFile;
}

// get vector of chunk headers exactly as in file

const vector<Mdvx::chunk_header_t>
  &Mdvx::getChunkHeadersFile() const {
  return _chdrsFile;
}

// get field header exactly as in file
// caller must use getNFieldsFile() to ensure the
// requested field header and vlevel header is available

const Mdvx::field_header_t
&Mdvx::getFieldHeaderFile(const int field_num) const {
  return (_fhdrsFile[field_num]);
}

const Mdvx::vlevel_header_t
&Mdvx::getVlevelHeaderFile(const int field_num) const {
  return (_vhdrsFile[field_num]);
}

// get chunk header exactly as in file
// caller must use getNChunksFile() to ensure the
// requested chunk header is available

const Mdvx::chunk_header_t
&Mdvx::getChunkHeaderFile(const int chunk_num) const {
  return (_chdrsFile[chunk_num]);
}

////////////////////////
// set the primary times

// valid time is the centroid time

void Mdvx::setValidTime(time_t valid_time)
{
  if (isNcf(_currentFormat)) {
    _ncfValidTime = valid_time;
  } else {
    _mhdr.time_centroid = valid_time;
  }
}

// generate time for forecast data

void Mdvx::setGenTime(time_t gen_time)
{
  if (isNcf(_currentFormat)) {
    _ncfGenTime = gen_time;
  } else {
    _mhdr.time_gen = gen_time;
  }
}

// time of the forecast

void Mdvx::setForecastTime(time_t forecast_time)
{
  if (isNcf(_currentFormat)) {
    _ncfForecastTime = forecast_time;
  } else {
    _mhdr.forecast_time = forecast_time;
    for (int ii = 0; ii < (int) _fields.size(); ii++) {
      _fields[ii]->_fhdr.forecast_time = forecast_time;
    }
  }
}

// forecast lead time in seconds

void Mdvx::setForecastLeadSecs(int lead_secs)
{
  if (isNcf(_currentFormat)) {
    _ncfForecastDelta = lead_secs;
  } else {
    _mhdr.forecast_delta = lead_secs;
    for (int ii = 0; ii < (int) _fields.size(); ii++) {
      _fields[ii]->_fhdr.forecast_delta = lead_secs;
    }
  }
}
  
// begin and end times

void Mdvx::setBeginTime(time_t begin_time)
{
  if (!isNcf(_currentFormat)) {
    _mhdr.time_begin = begin_time;
  }
}

void Mdvx::setEndTime(time_t end_time)
{
  if (!isNcf(_currentFormat)) {
    _mhdr.time_end = end_time;
  }
}

// data collection type

void Mdvx::setDataCollectionType(data_collection_type_t dtype)
{
  _mhdr.data_collection_type = (si32) dtype;
}

//////////////////////////////
// set/clear the master header

void Mdvx::setMasterHeader(const master_header_t &mhdr) {
  _mhdr = mhdr;
  _dataSetInfo = mhdr.data_set_info;
}

void Mdvx::clearMasterHeader() {
  MEM_zero(_mhdr);
  _mhdr.num_data_times = 1;
  _mhdr.vlevel_included = true;
  _mhdr.grid_orientation = ORIENT_SN_WE;
  _mhdr.data_ordering = ORDER_XYZ;
  _dataSetInfo.clear();
}

///////////////////////////////////////
// set data set info in master header
//
// For info of lengths > 512 bytes, also store
// as a chunk.

void Mdvx::setDataSetInfo(const char *info)
{
  MEM_zero(_mhdr.data_set_info);
  STRncopy(_mhdr.data_set_info, info, MDV_INFO_LEN);
  _dataSetInfo = info;
  // for long strings, use a chunk
  if (_dataSetInfo.size() > MDV_INFO_LEN - 1) {
    MdvxChunk *chunk = getChunkById(CHUNK_DATA_SET_INFO);
    if (chunk == NULL) {
      // no data set info chunk yet
      chunk = new MdvxChunk;
      addChunk(chunk);
    }
    chunk->setId(CHUNK_DATA_SET_INFO);
    chunk->setInfo("data-set-info");
    chunk->setData(_dataSetInfo.c_str(),
                   _dataSetInfo.size() + 1);
  }
}

////////////////////////////////////////////////////////
// set the data set info from the chunks, if there is
// a DATA_SET_INFO chunk.

void Mdvx::_setDataSetInfoFromChunks()
{

  // do we have a chunk for the data set info?
  
  MdvxChunk *chunk = getChunkById(CHUNK_DATA_SET_INFO);
  if (chunk != NULL) {
    const char *data = (const char *) chunk->getData();
    // set string
    _dataSetInfo = data;
    // copy from string to master header
    MEM_zero(_mhdr.data_set_info);
    STRncopy(_mhdr.data_set_info, _dataSetInfo.c_str(), MDV_INFO_LEN);
  } else {
    // copy from master header to string
    _dataSetInfo = _mhdr.data_set_info;
  }

}

///////////////////////////////////////
// get the data set info

string Mdvx::getDataSetInfo() const
{

  return _dataSetInfo;

}

///////////////////////////////////
// set data set name

void Mdvx::setDataSetName(const char *name)
{
  MEM_zero(_mhdr.data_set_name);
  STRncopy(_mhdr.data_set_name, name, MDV_NAME_LEN);
}

///////////////////////////////////
// set data set source

void Mdvx::setDataSetSource(const char *source)
{
  MEM_zero(_mhdr.data_set_source);
  STRncopy(_mhdr.data_set_source, source, MDV_NAME_LEN);
}

/////////////////////////////////////////////////////////////////
// update the master header
//
// Make the master header consistent with the rest of the object

void Mdvx::updateMasterHeader() const

{

  _mhdr.n_fields = _fields.size();
  _mhdr.n_chunks = _chunks.size();

  _mhdr.max_nx = 0;
  _mhdr.max_ny = 0;
  _mhdr.max_nz = 0;
  _mhdr.data_dimension = 0;
  _mhdr.field_grids_differ = false;

  if (_fields.size() > 0) {

    const Mdvx::field_header_t &fhdr0 = _fields[0]->_fhdr;
    
    for (int i = 0; i < _mhdr.n_fields; i++) {
      const Mdvx::field_header_t &fhdr = _fields[i]->_fhdr;
      _mhdr.max_nx = MAX(_mhdr.max_nx, fhdr.nx);
      _mhdr.max_ny = MAX(_mhdr.max_ny, fhdr.ny);
      _mhdr.max_nz = MAX(_mhdr.max_nz, fhdr.nz);
      _mhdr.data_dimension = MAX(_mhdr.data_dimension, fhdr.data_dimension);
      if (fhdr.proj_type != fhdr0.proj_type ||
	  fhdr.nx != fhdr0.nx ||
	  fhdr.ny != fhdr0.ny ||
	  fhdr.nz != fhdr0.nz ||
	  fhdr.grid_minx != fhdr0.grid_minx ||
	  fhdr.grid_miny != fhdr0.grid_miny ||
	  fhdr.grid_minz != fhdr0.grid_minz ||
	  fhdr.grid_dx != fhdr0.grid_dx ||
	  fhdr.grid_dy != fhdr0.grid_dy ||
	  fhdr.grid_dz != fhdr0.grid_dz) {
	_mhdr.field_grids_differ = true;
      }
      if (fhdr.vlevel_type == fhdr0.vlevel_type) {
	_mhdr.vlevel_type = fhdr0.vlevel_type;
      } else {
	_mhdr.vlevel_type = VERT_TYPE_VARIABLE;
      }
      if (fhdr.native_vlevel_type == fhdr0.native_vlevel_type) {
	_mhdr.native_vlevel_type = fhdr0.native_vlevel_type;
      } else {
	_mhdr.native_vlevel_type = VERT_TYPE_VARIABLE;
      }
    } // i
    
    if (_writeAsForecast) {
      _mhdr.time_centroid = fhdr0.forecast_time;
    }

    if (_mhdr.forecast_time == 0 && fhdr0.forecast_time != 0) {
      _mhdr.forecast_time = fhdr0.forecast_time;
    }
    if (_mhdr.forecast_delta == 0 && fhdr0.forecast_delta != 0) {
      _mhdr.forecast_delta = fhdr0.forecast_delta;
    }


  } // if (_fields.size() > 0) 
  
}

////////////////////////////////
// get field name, given the num
//
// Returns name on success, NULL on failure

const char *Mdvx::getFieldName(int field_num) const

{
  
  if (field_num < 0 || field_num > (int) (_fields.size() - 1)) {
    return NULL;
  } else {
    return _fields[field_num]->getFieldName();
  }

}

const char *Mdvx::getFieldNameLong(int field_num) const

{
  if (field_num < 0 || field_num > (int) (_fields.size() - 1)) {
    return NULL;
  } else {
    return _fields[field_num]->getFieldNameLong();
  }
}

////////////////////////////////
// get field num, given the name
//
// Returns field number on success, -1 on failure

int Mdvx::getFieldNum(const char *field_name) const

{
  for (size_t i = 0; i < _fields.size(); i++) {
    if (!strcmp(field_name, _fields[i]->getFieldName())) {
      return i;
    }
    if (!strcmp(field_name, _fields[i]->getFieldNameLong())) {
      return i;
    }
  }
  return -1;
}

////////////////////////////////////////////////////
// get pointer to field object, by field num or name
// Returns NULL on failure.
  
MdvxField *Mdvx::getFieldByNum(int field_num) const
{
  if (field_num < 0 || field_num > (int) (_fields.size() - 1)) {
    return NULL;
  } else {
    return (_fields[field_num]);
  }
}

MdvxField *Mdvx::getFieldByName(const char *field_name) const
{
  int fieldNum = getFieldNum(field_name);
  return getFieldByNum(fieldNum);
}

////////////////////////////////////////////////////////
// get pointer to chunk object, by chunk num, id or info
// Returns NULL on failure.
  
MdvxChunk *Mdvx::getChunkByNum(int chunk_num) const
{
  if (chunk_num < 0 || chunk_num > (int) (_chunks.size() - 1)) {
    return NULL;
  } else {
    return (_chunks[chunk_num]);
  }
}

MdvxChunk *Mdvx::getChunkByInfo(const char *chunk_info) const
{
  for (size_t i = 0; i < _chunks.size(); i++) {
    if (!strcmp(chunk_info, _chunks[i]->getHeader().info)) {
      return (_chunks[i]);
    }
  }
  return NULL;
}

MdvxChunk *Mdvx::getChunkById(const int chunk_id) const
{
  for (size_t i = 0; i < _chunks.size(); i++) {
    if (chunk_id == _chunks[i]->getHeader().chunk_id) {
      return (_chunks[i]);
    }
  }
  return NULL;
}

////////////////////////////////////////////////////////////
// return data element size (bytes) for given encoding type

int Mdvx::dataElementSize(encoding_type_t encoding_type)

{

  switch(encoding_type) {
  case ENCODING_INT8:
    return 1;
    break;
  case ENCODING_INT16:
    return 2;
    break;
  case ENCODING_FLOAT32:
  case ENCODING_RGBA32:
    return 4;
    break;
  default:
    return 1;
  }
  
}

///////////////////////////////////////////////////
// check for constant dz in field headers

bool Mdvx::dzIsConstant(const field_header_t &fhdr,
			const vlevel_header_t &vhdr)
  
{

  if (fhdr.nz < 2) {
    return true;
  }

  double range = vhdr.level[fhdr.nz - 1] - vhdr.level[0];
  double maxDiff = fabs(range / 1000.0);
  double dz0 = vhdr.level[1] - vhdr.level[0];

  for (int i = 1; i < fhdr.nz - 1; i++) {
    double dz = vhdr.level[i+1] - vhdr.level[i];
    double diff = fabs(dz - dz0);
    if (diff > maxDiff) {
      return false;
    }
  } // i

  return true;

}

///////////////////////////////
// handle error string

void Mdvx::clearErrStr() const
{
  _errStr = "";
  TaStr::AddStr(_errStr, "Time for following error: ", DateTime::str());
}

void Mdvx::addToErrStr(const string &message1,
		       const string &message2,
		       const string &message3) const
{
  _errStr += message1;
  _errStr += message2;
  _errStr += message3;
}

////////////////////////////////////////
// compute the epoch from the valid time

int Mdvx::computeEpoch(time_t validTime)

{

  double dvalid = validTime;
  double epochLen = pow(2.0, 32.0);
  return (int) ((dvalid - epochLen / 2.0) / epochLen);

}

//////////////////////////////////////////////////////////
// get projection, using the metadata from the first field
// returns PROJ_UNKNOWN if no fields

Mdvx::projection_type_t Mdvx::getProjection() const

{
  if (_fields.size() < 1) {
    return PROJ_UNKNOWN;
  }
  const field_header_t &fhdr = _fields[0]->getFieldHeader();
  return (projection_type_t) fhdr.proj_type;
}

