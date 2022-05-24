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
  _is64Bit = false;
  clearMasterHeader();
  clearRead();
  clearWrite();
  clearTimeListMode();
  clearNcf();
  clearMdv2Ncf();
  clearFields();
  clearChunks();
  _internalFormat = FORMAT_MDV;
  _useExtendedPaths = true;
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

  _internalFormat = rhs._internalFormat;
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

  _read32BitHeaders = rhs._read32BitHeaders;

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

  _ncfInstitution = rhs._ncfInstitution;
  _ncfReferences = rhs._ncfReferences;
  _ncfComment = rhs._ncfComment;
  _mdv2NcfTransArray = rhs._mdv2NcfTransArray;

  _ncfCompress = rhs._ncfCompress;
  _ncfCompressionLevel = rhs._ncfCompressionLevel;

  _ncfFileFormat = rhs._ncfFileFormat;
  _ncfOutputLatlonArrays = rhs._ncfOutputLatlonArrays;
  _ncfOutputMdvAttr = rhs._ncfOutputMdvAttr;
  _ncfOutputMdvChunks = rhs._ncfOutputMdvChunks;
  _ncfOutputStartEndTimes =  rhs._ncfOutputStartEndTimes;
  _ncfRadialFileType = rhs._ncfRadialFileType;

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
  if (isNcf(_internalFormat)) {
    return _ncfValidTime;
  } else {
    return _mhdr.time_centroid;
  }
}

// generate time for forecast data

time_t Mdvx::getGenTime() const
{
  if (isNcf(_internalFormat)) {
    return _ncfGenTime;
  } else {
    return _mhdr.time_gen;
  }
}

// forecast time

time_t Mdvx::getForecastTime() const
{
  if (isNcf(_internalFormat)) {
    return _ncfForecastTime;
  } else {
    return _mhdr.forecast_time;
  }
}

// lead time of forecast

time_t Mdvx::getForecastLeadSecs() const
{
  if (isNcf(_internalFormat)) {
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
  if (isNcf(_internalFormat)) {
    _ncfValidTime = valid_time;
  } else {
    _mhdr.time_centroid = valid_time;
  }
}

// generate time for forecast data

void Mdvx::setGenTime(time_t gen_time)
{
  if (isNcf(_internalFormat)) {
    _ncfGenTime = gen_time;
  } else {
    _mhdr.time_gen = gen_time;
  }
}

// time of the forecast

void Mdvx::setForecastTime(time_t forecast_time)
{
  if (isNcf(_internalFormat)) {
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
  if (isNcf(_internalFormat)) {
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
  if (!isNcf(_internalFormat)) {
    _mhdr.time_begin = begin_time;
  }
}

void Mdvx::setEndTime(time_t end_time)
{
  if (!isNcf(_internalFormat)) {
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

/////////////////////////////////////////////////////////////////////
// Copy 32-bit structs to 64-bit structs, and vice versa

/////////////////////////////////////////////////////////////////////
// master header 32-bit to 64-bit

void Mdvx::_copyMasterHeader32to64(const master_header_32_t &mhdr32,
                                   master_header_64_t &mhdr64) 

{

  memset(&mhdr64, 0, sizeof(mhdr64));
  
  mhdr64.record_len1 = sizeof(mhdr64) - (2 * sizeof(si32));
  mhdr64.record_len2 = sizeof(mhdr64) - (2 * sizeof(si32));
  mhdr64.struct_id = MASTER_HEAD_MAGIC_COOKIE_64;
  mhdr64.revision_number = REVISION_NUMBER;

  mhdr64.time_gen = mhdr32.time_gen;
  mhdr64.user_time = mhdr32.user_time;
  mhdr64.time_begin = mhdr32.time_begin;
  mhdr64.time_end = mhdr32.time_end;
  mhdr64.time_centroid = mhdr32.time_centroid;
  mhdr64.time_expire = mhdr32.time_expire;
  mhdr64.num_data_times = mhdr32.num_data_times;
  mhdr64.index_number = mhdr32.index_number;
  mhdr64.data_dimension = mhdr32.data_dimension;
  mhdr64.data_collection_type = mhdr32.data_collection_type;
  mhdr64.user_data = mhdr32.user_data;
  mhdr64.native_vlevel_type = mhdr32.native_vlevel_type;
  mhdr64.vlevel_type = mhdr32.vlevel_type;
  mhdr64.vlevel_included = mhdr32.vlevel_included;
  mhdr64.grid_orientation = mhdr32.grid_orientation;
  mhdr64.data_ordering = mhdr32.data_ordering;
  mhdr64.n_fields = mhdr32.n_fields;
  mhdr64.max_nx = mhdr32.max_nx;
  mhdr64.max_ny = mhdr32.max_ny;
  mhdr64.max_nz = mhdr32.max_nz;
  mhdr64.n_chunks = mhdr32.n_chunks;
  mhdr64.field_hdr_offset = mhdr32.field_hdr_offset;
  mhdr64.vlevel_hdr_offset = mhdr32.vlevel_hdr_offset;
  mhdr64.chunk_hdr_offset = mhdr32.chunk_hdr_offset;
  mhdr64.field_grids_differ = mhdr32.field_grids_differ;
  memcpy(mhdr64.user_data_si32, mhdr32.user_data_si32,
         min(sizeof(mhdr32.user_data_si32), sizeof(mhdr64.user_data_si32)));
  mhdr64.time_written = mhdr32.time_written;
  mhdr64.epoch = mhdr32.epoch;
  mhdr64.forecast_time = mhdr32.forecast_time;
  mhdr64.forecast_delta = mhdr32.forecast_delta;
  memcpy(mhdr64.unused_si32,  mhdr32.unused_si32,
         min(sizeof(mhdr32.unused_si32), sizeof(mhdr64.unused_si32)));
  memcpy(mhdr64.user_data_fl32,  mhdr32.user_data_fl32,
         min(sizeof(mhdr32.user_data_fl32), sizeof(mhdr64.user_data_fl32)));
  mhdr64.sensor_lon = mhdr32.sensor_lon;
  mhdr64.sensor_lat = mhdr32.sensor_lat;
  mhdr64.sensor_alt = mhdr32.sensor_alt;
  memcpy(mhdr64.unused_fl32,  mhdr32.unused_fl32,
         min(sizeof(mhdr32.unused_fl32), sizeof(mhdr64.unused_fl32)));
  STRncopy(mhdr64.data_set_info, mhdr32.data_set_info,
           sizeof(mhdr32.data_set_info));
  STRncopy(mhdr64.data_set_name, mhdr32.data_set_name,
           sizeof(mhdr32.data_set_name));
  STRncopy(mhdr64.data_set_source, mhdr32.data_set_source,
           sizeof(mhdr32.data_set_source));

}

/////////////////////////////////////////////////////////////////////
// master header 64-bit to 32-bit

void Mdvx::_copyMasterHeader64to32(const master_header_64_t &mhdr64,
                                   master_header_32_t &mhdr32) 

{

  memset(&mhdr32, 0, sizeof(mhdr32));
  
  mhdr32.record_len1 = sizeof(mhdr32) - (2 * sizeof(si32));
  mhdr32.record_len2 = sizeof(mhdr32) - (2 * sizeof(si32));
  mhdr32.struct_id = MASTER_HEAD_MAGIC_COOKIE_32;
  mhdr32.revision_number = 1;

  mhdr32.time_gen = mhdr64.time_gen;
  mhdr32.user_time = mhdr64.user_time;
  mhdr32.time_begin = mhdr64.time_begin;
  mhdr32.time_end = mhdr64.time_end;
  mhdr32.time_centroid = mhdr64.time_centroid;
  mhdr32.time_expire = mhdr64.time_expire;
  mhdr32.num_data_times = mhdr64.num_data_times;
  mhdr32.index_number = mhdr64.index_number;
  mhdr32.data_dimension = mhdr64.data_dimension;
  mhdr32.data_collection_type = mhdr64.data_collection_type;
  mhdr32.user_data = mhdr64.user_data;
  mhdr32.native_vlevel_type = mhdr64.native_vlevel_type;
  mhdr32.vlevel_type = mhdr64.vlevel_type;
  mhdr32.vlevel_included = mhdr64.vlevel_included;
  mhdr32.grid_orientation = mhdr64.grid_orientation;
  mhdr32.data_ordering = mhdr64.data_ordering;
  mhdr32.n_fields = mhdr64.n_fields;
  mhdr32.max_nx = mhdr64.max_nx;
  mhdr32.max_ny = mhdr64.max_ny;
  mhdr32.max_nz = mhdr64.max_nz;
  mhdr32.n_chunks = mhdr64.n_chunks;
  mhdr32.field_hdr_offset = mhdr64.field_hdr_offset;
  mhdr32.vlevel_hdr_offset = mhdr64.vlevel_hdr_offset;
  mhdr32.chunk_hdr_offset = mhdr64.chunk_hdr_offset;
  mhdr32.field_grids_differ = mhdr64.field_grids_differ;
  memcpy(mhdr32.user_data_si32, mhdr64.user_data_si32,
         min(sizeof(mhdr32.user_data_si32), sizeof(mhdr64.user_data_si32)));
  mhdr32.time_written = mhdr64.time_written;
  mhdr32.epoch = mhdr64.epoch;
  mhdr32.forecast_time = mhdr64.forecast_time;
  mhdr32.forecast_delta = mhdr64.forecast_delta;
  memcpy(mhdr32.unused_si32,  mhdr64.unused_si32,
         min(sizeof(mhdr32.unused_si32), sizeof(mhdr64.unused_si32)));
  memcpy(mhdr32.user_data_fl32,  mhdr64.user_data_fl32,
         min(sizeof(mhdr32.user_data_fl32), sizeof(mhdr64.user_data_fl32)));
  mhdr32.sensor_lon = mhdr64.sensor_lon;
  mhdr32.sensor_lat = mhdr64.sensor_lat;
  mhdr32.sensor_alt = mhdr64.sensor_alt;
  memcpy(mhdr32.unused_fl32,  mhdr64.unused_fl32,
         min(sizeof(mhdr32.unused_fl32), sizeof(mhdr64.unused_fl32)));
  STRncopy(mhdr32.data_set_info, mhdr64.data_set_info,
           sizeof(mhdr32.data_set_info));
  STRncopy(mhdr32.data_set_name, mhdr64.data_set_name,
           sizeof(mhdr32.data_set_name));
  STRncopy(mhdr32.data_set_source, mhdr64.data_set_source,
           sizeof(mhdr32.data_set_source));

}

/////////////////////////////////////////////////////////////////////
// field header 32-bit to 64-bit

void Mdvx::_copyFieldHeader32to64(const field_header_32_t &fhdr32,
                                  field_header_64_t &fhdr64) 

{

  memset(&fhdr64, 0, sizeof(fhdr64));
  
  fhdr64.record_len1 = sizeof(fhdr64) - (2 * sizeof(si32));
  fhdr64.record_len2 = sizeof(fhdr64) - (2 * sizeof(si32));
  fhdr64.struct_id = FIELD_HEAD_MAGIC_COOKIE_64;

  fhdr64.field_code = fhdr32.field_code;
  fhdr64.user_time1 = fhdr32.user_time1;
  fhdr64.forecast_delta = fhdr32.forecast_delta;
  fhdr64.user_time2 = fhdr32.user_time2;
  fhdr64.user_time3 = fhdr32.user_time3;
  fhdr64.forecast_time = fhdr32.forecast_time;
  fhdr64.user_time4 = fhdr32.user_time4;
  fhdr64.nx = fhdr32.nx;
  fhdr64.ny = fhdr32.ny;
  fhdr64.nz = fhdr32.nz;
  fhdr64.proj_type = fhdr32.proj_type;
  fhdr64.encoding_type = fhdr32.encoding_type;
  fhdr64.data_element_nbytes = fhdr32.data_element_nbytes;
  fhdr64.field_data_offset = fhdr32.field_data_offset;
  fhdr64.volume_size = fhdr32.volume_size;
  memcpy(fhdr64.user_data_si32, fhdr32.user_data_si32,
         min(sizeof(fhdr32.user_data_si32), sizeof(fhdr64.user_data_si32)));
  fhdr64.compression_type = fhdr32.compression_type;
  fhdr64.transform_type = fhdr32.transform_type;
  fhdr64.scaling_type = fhdr32.scaling_type;
  fhdr64.native_vlevel_type = fhdr32.native_vlevel_type;
  fhdr64.vlevel_type = fhdr32.vlevel_type;
  fhdr64.dz_constant = fhdr32.dz_constant;
  fhdr64.data_dimension = fhdr32.data_dimension;
  fhdr64.zoom_clipped = fhdr32.zoom_clipped;
  fhdr64.zoom_no_overlap = fhdr32.zoom_no_overlap;
  fhdr64.requested_compression = fhdr32.requested_compression;
  memcpy(fhdr64.unused_si32,  fhdr32.unused_si32,
         min(sizeof(fhdr32.unused_si32), sizeof(fhdr64.unused_si32)));
  fhdr64.proj_origin_lat = fhdr32.proj_origin_lat;
  fhdr64.proj_origin_lon = fhdr32.proj_origin_lon;
  for(int i = 0; i < min(MDV32_MAX_PROJ_PARAMS, MDV64_MAX_PROJ_PARAMS); i++) {
    fhdr64.proj_param[i] = fhdr32.proj_param[i];
  }
  fhdr64.vert_reference = fhdr32.vert_reference;
  fhdr64.grid_dx = fhdr32.grid_dx;
  fhdr64.grid_dy = fhdr32.grid_dy;
  fhdr64.grid_dz = fhdr32.grid_dz;
  fhdr64.grid_minx = fhdr32.grid_minx;
  fhdr64.grid_miny = fhdr32.grid_miny;
  fhdr64.grid_minz = fhdr32.grid_minz;
  fhdr64.scale = fhdr32.scale;
  fhdr64.bias = fhdr32.bias;
  fhdr64.bad_data_value = fhdr32.bad_data_value;
  fhdr64.missing_data_value = fhdr32.missing_data_value;
  fhdr64.proj_rotation = fhdr32.proj_rotation;
  memcpy(fhdr64.user_data_fl32,  fhdr32.user_data_fl32,
         min(sizeof(fhdr32.user_data_fl32), sizeof(fhdr64.user_data_fl32)));
  fhdr64.min_value = fhdr32.min_value;
  fhdr64.max_value = fhdr32.max_value;
  fhdr64.min_value_orig_vol = fhdr32.min_value_orig_vol;
  fhdr64.max_value_orig_vol = fhdr32.max_value_orig_vol;
  fhdr64.unused_fl32[0] = fhdr32.unused_fl32;

  memcpy(fhdr64.field_name_long, fhdr32.field_name_long,
         sizeof(fhdr32.field_name_long));
  memcpy(fhdr64.field_name, fhdr32.field_name,
         sizeof(fhdr32.field_name));
  memcpy(fhdr64.units, fhdr32.units,
         sizeof(fhdr32.units));
  memcpy(fhdr64.transform, fhdr32.transform,
         sizeof(fhdr32.transform));
  memcpy(fhdr64.unused_char, fhdr32.unused_char,
         sizeof(fhdr32.unused_char));

}

/////////////////////////////////////////////////////////////////////
// field header 64-bit to 32-bit

void Mdvx::_copyFieldHeader64to32(const field_header_64_t &fhdr64,
                                  field_header_32_t &fhdr32) 

{

  memset(&fhdr32, 0, sizeof(fhdr32));
  
  fhdr32.record_len1 = sizeof(fhdr32) - (2 * sizeof(si32));
  fhdr32.record_len2 = sizeof(fhdr32) - (2 * sizeof(si32));
  fhdr32.struct_id = FIELD_HEAD_MAGIC_COOKIE_32;

  fhdr32.field_code = fhdr64.field_code;
  fhdr32.user_time1 = fhdr64.user_time1;
  fhdr32.forecast_delta = fhdr64.forecast_delta;
  fhdr32.user_time2 = fhdr64.user_time2;
  fhdr32.user_time3 = fhdr64.user_time3;
  fhdr32.forecast_time = fhdr64.forecast_time;
  fhdr32.user_time4 = fhdr64.user_time4;
  fhdr32.nx = fhdr64.nx;
  fhdr32.ny = fhdr64.ny;
  fhdr32.nz = fhdr64.nz;
  fhdr32.proj_type = fhdr64.proj_type;
  fhdr32.encoding_type = fhdr64.encoding_type;
  fhdr32.data_element_nbytes = fhdr64.data_element_nbytes;
  fhdr32.field_data_offset = fhdr64.field_data_offset;
  fhdr32.volume_size = fhdr64.volume_size;
  memcpy(fhdr32.user_data_si32, fhdr64.user_data_si32,
         min(sizeof(fhdr64.user_data_si32), sizeof(fhdr64.user_data_si32)));
  fhdr32.compression_type = fhdr64.compression_type;
  fhdr32.transform_type = fhdr64.transform_type;
  fhdr32.scaling_type = fhdr64.scaling_type;
  fhdr32.native_vlevel_type = fhdr64.native_vlevel_type;
  fhdr32.vlevel_type = fhdr64.vlevel_type;
  fhdr32.dz_constant = fhdr64.dz_constant;
  fhdr32.data_dimension = fhdr64.data_dimension;
  fhdr32.zoom_clipped = fhdr64.zoom_clipped;
  fhdr32.zoom_no_overlap = fhdr64.zoom_no_overlap;
  fhdr32.requested_compression = fhdr64.requested_compression;
  memcpy(fhdr32.unused_si32,  fhdr64.unused_si32, 
         min(sizeof(fhdr32.unused_si32), sizeof(fhdr64.unused_si32)));
  fhdr32.proj_origin_lat = fhdr64.proj_origin_lat;
  fhdr32.proj_origin_lon = fhdr64.proj_origin_lon;
  for(int i = 0; i < min(MDV32_MAX_PROJ_PARAMS, MDV64_MAX_PROJ_PARAMS); i++) {
    fhdr32.proj_param[i] = fhdr64.proj_param[i];
  }
  fhdr32.vert_reference = fhdr64.vert_reference;
  fhdr32.grid_dx = fhdr64.grid_dx;
  fhdr32.grid_dy = fhdr64.grid_dy;
  fhdr32.grid_dz = fhdr64.grid_dz;
  fhdr32.grid_minx = fhdr64.grid_minx;
  fhdr32.grid_miny = fhdr64.grid_miny;
  fhdr32.grid_minz = fhdr64.grid_minz;
  fhdr32.scale = fhdr64.scale;
  fhdr32.bias = fhdr64.bias;
  fhdr32.bad_data_value = fhdr64.bad_data_value;
  fhdr32.missing_data_value = fhdr64.missing_data_value;
  fhdr32.proj_rotation = fhdr64.proj_rotation;
  memcpy(fhdr32.user_data_fl32,  fhdr64.user_data_fl32,
         min(sizeof(fhdr32.user_data_fl32), sizeof(fhdr64.user_data_fl32)));
  fhdr32.min_value = fhdr64.min_value;
  fhdr32.max_value = fhdr64.max_value;
  fhdr32.min_value_orig_vol = fhdr64.min_value_orig_vol;
  fhdr32.max_value_orig_vol = fhdr64.max_value_orig_vol;
  fhdr32.unused_fl32 = fhdr64.unused_fl32[0];

  memcpy(fhdr32.field_name_long, fhdr64.field_name_long,
         sizeof(fhdr32.field_name_long));
  memcpy(fhdr32.field_name, fhdr64.field_name,
         sizeof(fhdr32.field_name));
  memcpy(fhdr32.units, fhdr64.units,
         sizeof(fhdr32.units));
  memcpy(fhdr32.transform, fhdr64.transform,
         sizeof(fhdr32.transform));
  memcpy(fhdr32.unused_char, fhdr64.unused_char,
         sizeof(fhdr32.unused_char));

}

/////////////////////////////////////////////////////////////////////
// vlevel header 32-bit to 64-bit

void Mdvx::_copyVlevelHeader32to64(const vlevel_header_32_t &vhdr32,
                                   vlevel_header_64_t &vhdr64) 
  
{

  memset(&vhdr64, 0, sizeof(vhdr64));
  
  vhdr64.record_len1 = sizeof(vhdr64) - (2 * sizeof(si32));
  vhdr64.record_len2 = sizeof(vhdr64) - (2 * sizeof(si32));
  vhdr64.struct_id = VLEVEL_HEAD_MAGIC_COOKIE_64;

  memcpy(vhdr64.type, vhdr32.type, sizeof(vhdr32.type));
  memcpy(vhdr64.level, vhdr32.level, sizeof(vhdr32.level));
  memcpy(vhdr64.unused_si32, vhdr32.unused_si32,
         min(sizeof(vhdr32.unused_si32), sizeof(vhdr64.unused_si32)));
  memcpy(vhdr64.unused_fl32, vhdr32.unused_fl32,
         min(sizeof(vhdr32.unused_fl32), sizeof(vhdr64.unused_fl32)));

}

/////////////////////////////////////////////////////////////////////
// vlevel header 64-bit to 32-bit

void Mdvx::_copyVlevelHeader64to32(const vlevel_header_64_t &vhdr64,
                                   vlevel_header_32_t &vhdr32) 

{

  memset(&vhdr32, 0, sizeof(vhdr32));
  
  vhdr32.record_len1 = sizeof(vhdr32) - (2 * sizeof(si32));
  vhdr32.record_len2 = sizeof(vhdr32) - (2 * sizeof(si32));
  vhdr32.struct_id = VLEVEL_HEAD_MAGIC_COOKIE_32;

  memcpy(vhdr32.type, vhdr64.type, sizeof(vhdr32.type));
  memcpy(vhdr32.level, vhdr64.level, sizeof(vhdr32.level));
  memcpy(vhdr32.unused_si32, vhdr64.unused_si32,
         min(sizeof(vhdr32.unused_si32), sizeof(vhdr64.unused_si32)));
  memcpy(vhdr32.unused_fl32, vhdr64.unused_fl32,
         min(sizeof(vhdr32.unused_fl32), sizeof(vhdr64.unused_fl32)));
  
}

/////////////////////////////////////////////////////////////////////
// chunk header 32-bit to 64-bit

void Mdvx::_copyChunkHeader32to64(const chunk_header_32_t &chdr32,
                                  chunk_header_64_t &chdr64) 

{

  memset(&chdr64, 0, sizeof(chdr64));
  
  chdr64.record_len1 = sizeof(chdr64) - (2 * sizeof(si32));
  chdr64.record_len2 = sizeof(chdr64) - (2 * sizeof(si32));
  chdr64.struct_id = CHUNK_HEAD_MAGIC_COOKIE_64;

  chdr64.chunk_id = chdr32.chunk_id;
  chdr64.chunk_data_offset = chdr32.chunk_data_offset;
  chdr64.size = chdr32.size;
  
  memcpy(chdr64.unused_si32, chdr32.unused_si32,
         min(sizeof(chdr32.unused_si32), sizeof(chdr64.unused_si32)));
  
  memcpy(chdr64.info, chdr32.info, 
         min(sizeof(chdr32.info), sizeof(chdr64.info)));

}

/////////////////////////////////////////////////////////////////////
// chunk header 64-bit to 32-bit

void Mdvx::_copyChunkHeader64to32(const chunk_header_64_t &chdr64,
                                  chunk_header_32_t &chdr32) 

{

  memset(&chdr32, 0, sizeof(chdr32));
  
  chdr32.record_len1 = sizeof(chdr32) - (2 * sizeof(si32));
  chdr32.record_len2 = sizeof(chdr32) - (2 * sizeof(si32));
  chdr32.struct_id = CHUNK_HEAD_MAGIC_COOKIE_32;

  chdr32.chunk_id = chdr64.chunk_id;
  chdr32.chunk_data_offset = chdr64.chunk_data_offset;
  chdr32.size = chdr64.size;

  memcpy(chdr32.unused_si32, chdr64.unused_si32,
         min(sizeof(chdr32.unused_si32), sizeof(chdr64.unused_si32)));

  memcpy(chdr32.info, chdr64.info,
         min(sizeof(chdr32.info), sizeof(chdr64.info)));

}

/////////////////////////////////////////////////////////////////////
// copy main headers to file headers
// useful for non-MDV reads, e.g. NetCDF

void Mdvx::_copyMainHeadersToFileHeaders()

{

  _mhdrFile = _mhdr;
  
  _fhdrsFile.clear();
  _vhdrsFile.clear();
  _chdrsFile.clear();

  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fhdrsFile.push_back(_fields[ii]->getFieldHeader());
    _vhdrsFile.push_back(_fields[ii]->getVlevelHeader());
  }

  for (size_t ii = 0; ii < _chunks.size(); ii++) {
    _chdrsFile.push_back(_chunks[ii]->getHeader());
  }

}

