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
// MdvRead.cc
//
// Class for reading Mdv files
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 1999
//
//////////////////////////////////////////////////////////
//
// See <Mdv/mdv/MdvRead.hh> for details.
//
///////////////////////////////////////////////////////////

#include <Mdv/mdv/MdvRead.hh>
#include <toolsa/mem.h>
#include <toolsa/file_io.h>
#include <Mdv/mdv/mdv_read.h>
#include <cerrno>
using namespace std;

/////////////////////////////
// Constructor
//

MdvRead::MdvRead()

{

  MEM_zero(_masterHeader);
  _masterHeaderRead = false;
  _fieldNamesLoaded = false;
  _fp = NULL;

}

/////////////////////////////
// Destructor

MdvRead::~MdvRead()

{
  closeFile();
}

///////////////////////////////////////
// open MDV file for reading
//
// Returns 0 on success,  -1 on failure

int MdvRead::openFile(const string &file_path)

{

  closeFile();
  clear();
  _filePath = file_path;

  // check that file is an MDV file

  if (!MDV_verify((char *) _filePath.c_str())) {
    cerr << "ERROR - MdvRead::openFile" << endl;
    cerr << "  File path '" << _filePath << "'" << endl;
    cerr << "  File not in MDV format" << endl;
    return (-1);
  }
  
  if ((_fp = ta_fopen_uncompress((char *) _filePath.c_str(),
				 "rb")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - MdvRead::openFile" << endl;
    cerr << "  Cannot open file path '" << _filePath
	 << "' for reading" << endl;
    cerr << "  " << strerror(errNum) << endl;
    return (-1);
  }

  return (0);
  
}

/////////////////////
// close file if open

void MdvRead::closeFile()

{
  
  if (_fp) {
    fclose(_fp);
  }
  _fp = NULL;

}

////////////////////////////////////
// clear memory associated with file

void MdvRead::clear()

{
  
  MEM_zero(_masterHeader);
  _masterHeaderRead = false;
  _fieldNamesLoaded = false;
  _fieldNames.erase(_fieldNames.begin(), _fieldNames.end());
  _fields.erase(_fields.begin(), _fields.end());
  _chunks.erase(_chunks.begin(), _chunks.end());
  _radar.clear();
  
}

/////////////////////////
// read the master header
//
// Returns 0 on success,  -1 on failure

int MdvRead::readMasterHeader()

{

  // don't read again if already done once

  if (_masterHeaderRead) {
    return (0);
  }

  if (!_fp) {
    cerr << "ERROR - MdvRead::readMasterHeader" << endl;
    cerr << "  File not open" << endl;
    return (-1);
  }

  // Read the master header.
  
  if (MDV_load_master_header(_fp, &_masterHeader) != MDV_SUCCESS) {
    cerr << "ERROR - MdvRead::readMasterHeader" << endl;
    cerr << "  Cannot load master header" << endl;
    cerr << "  File path '" << _filePath << "'" << endl;
    return (-1);
  }

  _masterHeaderRead = true;

  // create field and chunk arrays

  for (int i = 0; i < _masterHeader.n_fields; i++) {
    MdvReadField field(this, i);
    _fields.push_back(field);
  }

  for (int i = 0; i < _masterHeader.n_chunks; i++) {
    MdvReadChunk chunk(this, i);
    _chunks.push_back(chunk);
  }

  return (0);

}

///////////////////////////////////
// read headers for a single field
//
// Returns 0 on success,  -1 on failure
  
int MdvRead::readFieldHeaders(const int field_num)

{

  if (readMasterHeader()) {
    return (-1);
  }

  //
  // Small check added by Niles.
  //
  if (field_num >= _masterHeader.n_fields){
    cerr << "ERROR - MdvRead::readFieldHeaders" << endl;
    cerr << "Cannot access field " << field_num << endl;
    return (-1);
  }


  return (_fields[field_num]._readHeaders());

}
  
//////////////////////////////
// read headers for all fields
//
// Returns 0 on success,  -1 on failure
  
int MdvRead::readFieldHeaders()

{

  if (readMasterHeader()) {
    return (-1);
  }

  for (int i = 0;  i < _masterHeader.n_fields; i++) {
    if (_fields[i]._readHeaders()) {
      return(-1);
    }
  }

  return (0);

}

////////////////////////////////////////
// get field header for a given field_num
//   without any previous reading.
//
// returns -1 on failure
  
int MdvRead::getFieldHeader(int fieldNum, MDV_field_header_t & returnHeader)
{
  if (readFieldHeaders(fieldNum)) {
    return -1;
  }

  returnHeader = _fields[fieldNum].getFieldHeader();
  return 0;
}

////////////////////////////////////////
// get field name for a given field_num
//
// returns NULL on failure
  
char *MdvRead::getFieldName(const int field_num)

{
  
  if (readMasterHeader()) {
    return (NULL);
  }

  if (field_num > _masterHeader.n_fields - 1) {
    return (NULL);
  }

  if (readFieldHeaders(field_num)) {
    return (NULL);
  }

  return (_fields[field_num].getFieldHeader().field_name);

}

///////////////////////////////////////
// load field names into map for lookup
//
// Returns 0 on success,  -1 on failure
  
int MdvRead::loadFieldNames()

{

  if (_fieldNamesLoaded) {
    return (0);
  }

  if (readFieldHeaders()) {
    return (-1);
  }

  for (int i = 0;  i < _masterHeader.n_fields; i++) {
    _fieldNames[_fields[i].getFieldHeader().field_name] = i;
  }

  _fieldNamesLoaded = true;

  return (0);

}
  
//////////////////////////////////////////////////
// get a field number given the field name
//
// Returns field number on success,  -1 on failure
  
int MdvRead::getFieldNum(const char *field_name)

{

  if (loadFieldNames()) {
    return (-1);
  }

  NameMap::iterator ii = _fieldNames.find(field_name);
  if (ii == _fieldNames.end()) {
    cerr << "ERROR - MdvRead::getFieldName" << endl;
    cerr << "  No field '" << field_name << "' not in file" << endl;
    cerr << "  File path '" << _filePath << "'" << endl;
    return (-1);
  }

  int field_num = (*ii).second;

  return (field_num);

}
  
//////////////////////////////////////////
// read a single plane, given field number
// and plane number
//
// If fhdr is non-NULL, it is filled out with the values applicable after the
// read and any conversions
//
// Returns 0 on success, -1 on failure
  
int MdvRead::readPlane(const int field_num,
		       const int plane_num,
		       const int return_data_type,
		       MDV_field_header_t *fhdr /* = NULL*/ )
  
{
  
  if (readFieldHeaders(field_num)) {
    return (-1);
  }

  return (_fields[field_num]._readPlane(plane_num, return_data_type, fhdr));

}
  
////////////////////////////////////////
// read a single plane, given field name
// and plane number
//
// If fhdr is non-NULL, it is filled out with the values applicable after the
// read and any conversions
//
// Returns 0 on success, -1 on failure
  
int MdvRead::readPlane(const char *field_name,
		       const int plane_num,
		       const int return_data_type,
		       MDV_field_header_t *fhdr /* = NULL*/ )
  
{
  
  if (loadFieldNames()) {
    cerr << "ERROR - MdvRead::readPlane" << endl;
    cerr << "  loadFieldNames() failed." << endl;
    return (-1);
  }

  NameMap::iterator ii = _fieldNames.find(field_name);
  if (ii == _fieldNames.end()) {
    cerr << "ERROR - MdvRead::readPlane" << endl;
    cerr << "  No field '" << field_name << "' not in file" << endl;
    cerr << "  File path '" << _filePath << "'" << endl;
    return (-1);
  }

  int field_num = (*ii).second;

  return (readPlane(field_num, plane_num, return_data_type, fhdr));

}
  
//////////////////////////////////////////
// read a single plane, given field number
// and vlevel.
//
// If fhdr is non-NULL, it is filled out with the values applicable after the
// read and any conversions
//
// Returns 0 on success, -1 on failure
  
int MdvRead::readPlane(const int field_num,
		       const double vlevel,
		       const int return_data_type,
		       MDV_field_header_t *fhdr /* = NULL*/ )
  
{
  
  if (readFieldHeaders(field_num)) {
    return (-1);
  }

  return (_fields[field_num]._readPlane(vlevel, return_data_type, fhdr));

}
  
////////////////////////////////////////
// read a single plane, given field name
// and vlevel
//
// If fhdr is non-NULL, it is filled out with the values applicable after the
// read and any conversions
//
// Returns 0 on success, -1 on failure
  
int MdvRead::readPlane(const char *field_name,
		       const double vlevel,
		       const int return_data_type,
		       MDV_field_header_t *fhdr /* = NULL*/ )
  
{
  
  if (loadFieldNames()) {
    cerr << "ERROR - MdvRead::readPlane" << endl;
    cerr << "  loadFieldNames() failed." << endl;
    return (-1);
  }

  NameMap::iterator ii = _fieldNames.find(field_name);
  if (ii == _fieldNames.end()) {
    cerr << "ERROR - MdvRead::readPlane" << endl;
    cerr << "  No field '" << field_name << "' not in file" << endl;
    cerr << "  File path '" << _filePath << "'" << endl;
    return (-1);
  }

  int field_num = (*ii).second;

  return (readPlane(field_num, vlevel, return_data_type, fhdr));

}
  
//////////////////////////////////////////////
// Read composite for given field number
// Encoded return types are not supported.
//
// If fhdr is non-NULL, it is filled out with the values applicable after the
// read and any conversions
//
// Returns 0 on success, -1 on failure
  
int MdvRead::readComposite(const int field_num,
			   const int return_data_type /* = MDV_INT8*/,
			   MDV_field_header_t *fhdr /* = NULL*/ )
  
{
  
  if (readFieldHeaders(field_num)) {
    return (-1);
  }

  return (_fields[field_num]._readComposite(return_data_type, fhdr));

}
  
//////////////////////////////////////////////
// read composite for given field name
// Encoded return types are not supported.
//
// If fhdr is non-NULL, it is filled out with the values applicable after the
// read and any conversions
//
// Returns 0 on success, -1 on failure
  
int MdvRead::readComposite(const char *field_name,
			   const int return_data_type /* = MDV_INT8*/,
			   MDV_field_header_t *fhdr /* = NULL*/ )
  
{
  
  if (loadFieldNames()) {
    cerr << "ERROR - MdvRead::readComposite" << endl;
    cerr << "  loadFieldNames() failed." << endl;
    return (-1);
  }

  NameMap::iterator ii = _fieldNames.find(field_name);
  if (ii == _fieldNames.end()) {
    cerr << "ERROR - MdvRead::readComposite" << endl;
    cerr << "  No field '" << field_name << "' not in file" << endl;
    cerr << "  File path '" << _filePath << "'" << endl;
    return (-1);
  }

  int field_num = (*ii).second;

  return (readComposite(field_num, return_data_type, fhdr));

}
  
/////////////////////////////////////////
// read a data volume, given field number
//
// If fhdr is non-NULL, it is filled out with the values applicable after the
// read and any conversions
//
// Returns 0 on success, -1 on failure
  
int MdvRead::readVol(const int field_num, const int return_data_type,
		     MDV_field_header_t *fhdr /* = NULL*/ )
  
{
  
  if (readFieldHeaders(field_num)) {
    return (-1);
  }

  return (_fields[field_num]._readVol(return_data_type, fhdr));

}

////////////////////////////////////////
// read a data volume, given field name
//
// If fhdr is non-NULL, it is filled out with the values applicable after the
// read and any conversions
//
// Returns 0 on success, -1 on failure
  
int MdvRead::readVol(const char *field_name, const int return_data_type,
		     MDV_field_header_t *fhdr /* = NULL*/ )
  
{
  
  if (loadFieldNames()) {
    cerr << "ERROR - MdvRead::readVol" << endl;
    cerr << "  loadFieldNames() failed." << endl;
    return (-1);
  }

  NameMap::iterator ii = _fieldNames.find(field_name);
  if (ii == _fieldNames.end()) {
    cerr << "ERROR - MdvRead::readPlane" << endl;
    cerr << "  No field '" << field_name << "' not in file" << endl;
    cerr << "  File path '" << _filePath << "'" << endl;
    return (-1);
  }

  int field_num = (*ii).second;

  return (readVol(field_num, return_data_type, fhdr));

}
  
/////////////////////////////////
// read header for a single chunk
//
// Returns 0 on success,  -1 on failure
  
int MdvRead::readChunkHeader(const int chunk_num)
  
{

  if (readMasterHeader()) {
    return (-1);
  }

  return (_chunks[chunk_num]._readHeader());
  
}
  
/////////////////////////////////
// read headers for all chunks
//
// Returns 0 on success,  -1 on failure
  
int MdvRead::readChunkHeaders()
  
{

  if (readMasterHeader()) {
    return (-1);
  }

  for (int i = 0;  i < _masterHeader.n_chunks; i++) {
    if (_chunks[i]._readHeader()) {
      return(-1);
    }
  }

  return (0);
  
}
  
//////////////////////
// read a single chunk
//
// Returns 0 on success,  -1 on failure
  
int MdvRead::readChunk(const int chunk_num)

{

  if (readChunkHeader(chunk_num)) {
    return (-1);
  }

  return (_chunks[chunk_num]._read());
  
}
  
///////////////////
// read all chunks
//
// Returns 0 on success,  -1 on failure
  
int MdvRead::readChunks()

{

  if (readChunkHeaders()) {
    return (-1);
  }

  for (int i = 0;  i < _masterHeader.n_chunks; i++) {
    if (_chunks[i]._read()) {
      return(-1);
    }
  }

  return (0);

}
  
///////////////////
// loadRadar
//
// Load up radar object if chunk radar params available
//

int MdvRead::loadRadar()

{

  if (readChunks()) {
    return (-1);
  }

  for (int i = 0;  i < _masterHeader.n_chunks; i++) {
    if (_chunks[i]._data != NULL) {
      _radar._loadFromChunk(_chunks[i], _masterHeader.n_fields);
    }
  }

  return (0);

}
  
  

