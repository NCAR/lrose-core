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

////////////////////////////////////////////////////////////////////
// mdv/MdvRead.hh
//
// Class for reading Mdv files
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 1999
//
////////////////////////////////////////////////////////////////////
//
// MdvRead allows you to read part or all of an MDV file.
// The intention is to present the headers is an unaltered form, so that
// they represent the file accurately. For example, even if you only
// read one field, the master header will still show the number of fields
// actually in the file.
//
// You use the class as follows:
//
//   1. Call openFile() to open the file.
//
//   2. Call the read routines to read specified parts of the file,
//      which are then represented by the various members of the class.
//      These members provide a 
//
//   3. If you wish, call the load routines to load up members which are
//      not necessarily a direct mirror of the file. For example,
//      loadFieldNames() loads up a map of <field_name, field_num>,
//      which is a summary of information from the field headers.
//
//   4. When done reading, call closeFile. This will be automatically
//      be called when opening a new file, and when the destructor is
//      called.
//
// You may call any of the read routines at any time. If these functions
// require header information which has not yet been read, these headers
// will be read. Once a header has been read once, it will not be read
// again until a new file is opened.
//
// Use only the read routines in the MdvRead class, not those in the
// objects contained by MdvRead.
//
// Once you have read the required parts, access to the data is available
// through the contained objects. Check the header files for details.
//
//   MdvReadField: field headers, vlevel headers, planes, volumes.
//   MdvReadChunk: chunk headers, chunk data, if any.
//   MdvReadRadar: radar struct representation of radar chunks, is any.
//
//////////////////////////////////////////////////////////////////////

#ifndef MdvRead_HH
#define MdvRead_HH

#include <Mdv/mdv/mdv_file.h>
#include <Mdv/mdv/MdvReadField.hh>
#include <Mdv/mdv/MdvReadChunk.hh>
#include <Mdv/mdv/MdvReadRadar.hh>
#include <vector>
#include <map>
#include <string>
#include <cstdio>
using namespace std;

class MdvRead
{

  friend class MdvReadField;
  friend class MdvReadChunk;
  friend class MdvReadRadar;

public:
  
  typedef map<string, int, less<string> > NameMap;

  ///////////////////////
  // constructor
  
  MdvRead();

  ///////////////////////
  // destructor

  virtual ~MdvRead();

  ///////////////////////
  // open MDV file
  // Returns 0 on success, -1 on failure
  
  int openFile(const string &file_path);

  ///////////////////////
  // close file if open
  
  void closeFile();

  ///////////////////////////////////////
  // free up memory associated with file
  
  void clear();

  ///////////////////////
  // read the master header
  // Returns 0 on success, -1 on failure
  
  int readMasterHeader();

  ///////////////////////////////////
  // read headers for a single field
  // Both field and vlevel headers are read and _grid is loaded.
  // Returns 0 on success, -1 on failure
  
  int readFieldHeaders(const int field_num);
  
  //////////////////////////////////////
  // read headers for all fields
  // Both field and vlevel headers are read and _grid is loaded.
  // Returns 0 on success, -1 on failure
  
  int readFieldHeaders();

  ///////////////////////////////////////
  // load field names into map for lookup
  // Returns 0 on success,  -1 on failure
  
  int loadFieldNames();

  /////////////////////////////////
  // read header for a single chunk
  // Returns 0 on success,  -1 on failure
  
  int readChunkHeader(const int chunk_num);
  
  /////////////////////////////
  // read header for all chunks
  // Returns 0 on success,  -1 on failure
    
  int readChunkHeaders();

  ////////////////////////////////////////
  // read a single chunk, header and data
  // Returns 0 on success,  -1 on failure
  
  int readChunk(int chunk_num);

  /////////////////////////////////////
  // read a all chunks, header and data
  // Returns 0 on success,  -1 on failure
  
  int readChunks();
  
  ///////////////////
  // loadRadar
  // Load up radar object if chunk radar params available
  // Returns 0 on success,  -1 on failure
  
  int loadRadar();

  ///////////////////////////////////////////
  // read a single plane, given field number
  // If fhdr is non-NULL, it is filled out with the values applicable
  // after the read and any relevant conversions.
  // Returns 0 on success, -1 on failure
  
  int readPlane(const int field_num,
		const int plane_num,
		const int return_data_type,
		MDV_field_header_t *fhdr = NULL);

  ////////////////////////////////////////
  // read a single plane, given field name
  // If fhdr is non-NULL, it is filled out with the values applicable
  // after the read and any relevant conversions.
  // Returns 0 on success, -1 on failure
  
  int readPlane(const char *field_name,
		const int plane_num,
		const int return_data_type,
		MDV_field_header_t *fhdr = NULL);
  
  //////////////////////////////////////////
  // read a single plane, given field number
  // and vlevel.
  // If fhdr is non-NULL, it is filled out with the values applicable
  // after the read and any relevant conversions.
  // Returns 0 on success, -1 on failure
  
  int readPlane(const int field_num,
		const double vlevel,
		const int return_data_type,
		MDV_field_header_t *fhdr = NULL);
  
  ////////////////////////////////////////
  // read a single plane, given field name
  // and vlevel
  // If fhdr is non-NULL, it is filled out with the values applicable
  // after the read and any relevant conversions.
  // Returns 0 on success, -1 on failure
  
  int readPlane(const char *field_name,
		const double vlevel,
		const int return_data_type,
		MDV_field_header_t *fhdr = NULL);
  
  /////////////////////////////////////////
  // get the plane num and vlevel of the
  // latest plane read.

  int getPlaneNum(int field_num) {
    return (_fields[field_num].getPlaneNum());
  }

  double getPlaneVlevel(int field_num) {
    return (_fields[field_num].getPlaneVlevel());
  }

  //////////////////////////////////////////////
  // read composite for given field number
  // return_data_type - only MDV_INT8 supported
  // If fhdr is non-NULL, it is filled out with the values applicable
  // after the read and any relevant conversions.
  // Returns 0 on success, -1 on failure
  
  int readComposite(const int field_num,
		    const int return_data_type = MDV_INT8,
		    MDV_field_header_t *fhdr = NULL);
  
  //////////////////////////////////////////////
  // read composite for given field name
  // return_data_type - only MDV_INT8 supported
  // If fhdr is non-NULL, it is filled out with the values applicable
  // after the read and any relevant conversions.
  // Returns 0 on success, -1 on failure
  
  int readComposite(const char *field_name,
		    const int return_data_type = MDV_INT8,
		    MDV_field_header_t *fhdr = NULL);
  
  //////////////////////////////////////
  // read a data volume, given field_num
  // If fhdr is non-NULL, it is filled out with the values applicable
  // after the read and any relevant conversions.
  // Returns 0 on success, -1 on failure
  
  int readVol(const int field_num, const int return_data_type,
	      MDV_field_header_t *fhdr = NULL);

  ////////////////////////////////////////
  // read a data volume, given field name
  // If fhdr is non-NULL, it is filled out with the values applicable
  // after the read and any relevant conversions.
  // Returns 0 on success, -1 on failure
  
  int readVol(const char *field_name, const int return_data_type,
	      MDV_field_header_t *fhdr = NULL);

  ////////////////////////////////////////
  // data member access

  // get master header

  MDV_master_header_t &getMasterHeader() { return (_masterHeader); }

  // get reference to field objects

  MdvReadField &getField(int i) { return (_fields[i]); }
  vector<MdvReadField> &getFields() { return (_fields); }

  // get reference to field header, without having to read the whole
  //  field volume.
  // 
  int getFieldHeader(int fieldNum, MDV_field_header_t & returnHeader);

  // get name of specified field

  char *getFieldName(const int field_num);

  // get a field number given the field name
  // Returns field number on success,  -1 on failure
  
  int getFieldNum(const char *field_name);

  // get reference to chunk objects

  MdvReadChunk &getChunk(const int chunk_num) {
    return (_chunks[chunk_num]);
  }
  vector<MdvReadChunk> &getChunks() { return (_chunks); }

  // get reference to radar object

  MdvReadRadar &getRadar() { return (_radar); }

  // get file path used

  string &getFilePath() { return (_filePath); }

protected:

  MDV_master_header_t _masterHeader;
  bool _masterHeaderRead;

  vector<MdvReadField> _fields;
  NameMap _fieldNames;
  bool _fieldNamesLoaded;

  vector<MdvReadChunk> _chunks;
  MdvReadRadar _radar;

  FILE *_fp;

  string _filePath;

private:

};

#endif


