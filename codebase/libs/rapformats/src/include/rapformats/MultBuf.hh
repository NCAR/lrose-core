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
// MultBuf.hh
//
// A buffer with multiple parts - see also MultBufPart.hh
//
// A MultBuf represents a buffer which has multiple parts.
// The objects may be assembled (serialized) or
// disassembled (deserialized) to facilitate buffer storage.
//
// A MultBuf has 3 sections, a header struct which contains the
// number of parts, an array of part structs which indicate the
// part types, lengths and offsets, and the data parts themselves.
//
// MultBuf format:
//
//   header_t
//   nParts * part_t
//   nParts * data
//
// Much of this code copied from DsMessage.
//
// Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307-3000
//
// March 2000
//
////////////////////////////////////////////////////////////

#ifndef _MultBuf_hh
#define _MultBuf_hh


#include <iostream>
#include <vector>
#include <string>
#include <dataport/port_types.h>
#include <toolsa/MemBuf.hh>
using namespace std;

// forward class declarations
class MultBufPart;

class MultBuf

{

  friend class MultBufPart;

protected:

  // header struct:
  // id should be used to identify the type of data in the buffer.
  // version is used internally to keep future versions of the
  // format consistent.
  
  typedef struct {
    
    si32 id;
    si32 version;
    si32 n_parts;
    si32 spare[3];
    
  } header_t;
  
  // part struct:
  // part type is defined by the author of the code using the buffer.
  // length refers to the length of the part data in bytes.
  // offset refers to the offset of the part data from the start
  // of the buffer.
  
  typedef struct {
    
    si32 type;
    si32 offset;
    si32 len;
    si32 spare;
    
  } part_hdr_t;
  
public:

  //////////////
  // constructor

  MultBuf();

  ///////////////////
  // copy constructor
  
  MultBuf(const MultBuf &rhs);

  /////////////
  // destructor

  virtual ~MultBuf();

  // assignment
  
  MultBuf & operator=(const MultBuf &rhs);

  ////////////////////////////////////////////////////////////////
  // peek at header
  //
  // This is used if you just want to peek at the header id and
  // version before deciding how to handle the buffer.
  // 
  // Returns: 0 on success, -1 on error
  // Error string retrieved with getErrStr().
  
  int peekAtHeader(const void *in_buf, const int buf_len,
		   int *id = NULL, int *version = NULL);
  
  ////////////////////////////////////////////////////
  // disassemble a buffer into parts, store in
  // MultBuf object.
  //
  // Returns: 0 on success, -1 on error
  
  virtual int disassemble(const void *in_buf, const int buf_len);

  /////////////////////////////
  // get the header attributes

  int getId() const { return (_id); }
  int getVersion() const { return (_version); }

  //////////////////////////
  // get the number of parts

  inline int getNParts() const { return ((int) _parts.size()); }

  ////////////////////////////////////////////////
  // does a part exist?
  // returns the number of parts of the given type
  
  int partExists(const int part_type) const;

  ////////////////////////////////////////////
  // Get a part from the parts array, given
  // the index into the array.
  //
  // Returns pointer to part, NULL on failure.

  MultBufPart *getPart(const int index) const;

  ////////////////////////////////////////////////////////////
  // Get a part by type.
  //
  // The index refers only to parts of this type. For example,
  // an index of 2 will return the 3rd part of the given type.
  //
  // If more than 1 part of this type exists, use index to
  // select the required one.
  //
  // Returns pointer to the requested part, NULL on failure.

  MultBufPart *getPartByType(const int part_type, const int index = 0) const;
  
  //////////////////////////////////////////
  // set the buffer header attributes
  // These overwrite the existing attributes.

  void setId(const int id) { _id = id; }

  /////////////////////////////
  // clear before adding parts.
  //
  // This initializes the number of parts to 0.
  // It does NOT clear the header attributes set using the
  // set() routines.

  void clearParts();

  ///////////////////////////////////////////////////////////////
  // clear everything -- header and parts.

   void clearAll();

  ////////////////////////////
  // Add a part to the object.
  //
  // The part is added at the end of the part list.
  // The buffer must be in BE byte order.

  void addPart(const int part_type, const int len, const void *data);

  /////////////////////////////////////
  // assemble the parts into a buffer
  //
  // Returns pointer to the assembled buffer.
  
  void *assemble();

  //////////////////////////////////
  // get pointer to assembled buffer

  inline void *assembledBuf() const { return (_assembledBuf.getPtr()); }
  
  ///////////////////////////////
  // length of assembled buffer

  inline int lengthAssembled() const { return (_assembledBuf.getLen()); }
  
  //////////////////////////////////////////
  // print out main header and parts headers

  virtual void print(ostream &out, const string &spacer = "") const;

  ////////////////////////////////
  // print out the buffer header

  void printHeader(ostream &out, const string &spacer = "") const;

  /////////////////////
  // print part headers

  void printPartHeaders(ostream &out, const string &spacer = "") const;

  // debugging

  void setDebug(bool debug = true) { _debug = debug; }

  // get the error string

  const string &getErrStr() const { return (_errStr); }
  
protected:

  static const int currentVersion = 1;

  // State info

  int _id;
  int _version;

  // individual parts

  vector<MultBufPart*> _parts;

  // assembled buffer
  
  MemBuf _assembledBuf;

  // debugging

  bool _debug;

  // error string

  string _errStr;

  // byte order conversions

  static void _BE_to_header(header_t &hdr);
  static void _BE_from_header(header_t &hdr);
  static void _BE_to_part_hdr(part_hdr_t &part_hdr);
  static void _BE_from_part_hdr(part_hdr_t &part_hdr);

  // copy
  
  virtual MultBuf &_copy(const MultBuf &rhs);

private:

};

#endif
