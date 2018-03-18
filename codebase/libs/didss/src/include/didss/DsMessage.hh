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

////////////////////////////////////////////////////////////////////////////////
//
// Terri Betancourt
// RAP, NCAR, POBox 3000, Boulder, CO, 80307-3000
// 
// May 1998
// 
////////////////////////////////////////////////////////////////////////////////

#ifndef _DS_MESSAGE_INC_
#define _DS_MESSAGE_INC_

#include <cstdio>
#include <string>
#include <vector>
#include <map>
#include <dataport/port_types.h>
#include <didss/ds_message.h>
using namespace std;

//////////////////////////////
// forward class declarations
//

class DsMsgPart;

class DsMessage

{

public:

  /////////////////////////////////////////////////////////////////
  // memory model - the user can choose either to copy memory areas
  // into memory local to the object, or to point to memory held
  // by other objects. If the PointToMem model is used, the user
  // must make sure that the memory pointed to remains valid for
  // as long as this class needs it. If you are assembling a message,
  // the memory must be valid until after the assemble. If you are
  // disassembling a message, the memory must remain valid until 
  // after you are done with the components of the message.
  //
  // The memory is actually managed by the DsMsgPart class, which
  // holds the pointer to the data buffer for each class.
  
  typedef enum { CopyMem, PointToMem } memModel_t;

  // constructor
  //
  // Memory model defaults to local copy
  
  DsMessage(memModel_t mem_model = CopyMem);

  // copy constructor

  DsMessage(const DsMessage &rhs);

  // destructor

  virtual ~DsMessage();

  // assignment
  
  DsMessage & operator=(const DsMessage &rhs);

  //////////////////////////
  // decode a message header
  //
  // This is used if you just want to peek at the header before
  // deciding how to handle the message.
  // 
  // If msg_len is set, checks that the msg is big enough
  //   to hold at least a DsMsgHdr_t. Otherwise, assumes
  //   that the message is big enough and blindly copies
  //   memory.
  //
  // Returns: -1 If msg_len is set and is smaller than a DsMsgHdr_t.
  //           0 Otherwise.

  virtual int decodeHeader(const void *in_msg, ssize_t msg_len = -1);

  ////////////////////////////////////////////////////
  // disassemble a message into parts, store in
  // DsMessage object.
  //
  // If msg_len is provided, the parts are checked to make
  // sure they do not run over the end of the message.
  //
  // If msg_len is provided, the parts are checked to make
  // sure they do not run over the end of the message.
  // 
  // If msg_len is set, checks that the msg is big enough
  //   to hold at least a DsMsgHdr_t. Otherwise, assumes
  //   that the message is big enough and blindly copies
  //   memory.
  //
  // Returns: -1 If msg_len is set and is smaller than a DsMsgHdr_t.
  //               or if one of the message parts cannot be decoded.
  //           0 Otherwise.

  int disassemble(const void *in_msg, const ssize_t msg_len = -1);

  /////////////////////////////
  // get the message attributes

  int getType() const { return (_type); }
  int getSubType() const { return (_subType); }
  int getMode() const { return (_mode); }
  int getFlags() const { return (_flags); }
  bool getError() const { return (_error); }

  int getMajorVersion() const { return (_majorVersion); }
  int getMinorVersion() const { return (_minorVersion); }
  ssize_t getSerialNum() const { return (_serialNum); }

  //////////////////////////
  // get the number of parts

  inline ssize_t getNParts() const { return (_nParts); }

  ////////////////////////////////////////////////
  // does a part exist?
  // returns the number of parts of the given type

  int partExists(const int data_type) const;

  ////////////////////////////////////////////
  // Get a part from the parts array, given
  // the index into the array.
  //
  // Returns pointer to part, NULL on failure.

  DsMsgPart *getPart(const ssize_t index) const;

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

  DsMsgPart *getPartByType(const int data_type, const ssize_t index = 0) const;
  
  //////////////////////////////////////////
  // set the message header attributes
  //
  // These overwrite the existing attributes.

  void setHdrAttr(const int type,
		  const int sub_type = -1,
		  const int mode = -1,
		  const int flags = 0,
		  const int major_version = 1,
		  const int minor_version = 0,
		  const ssize_t serial_num = -1);

  void setType(const int type) { _type = type; }

  void setSubType(const int sub_type) {_subType = sub_type; }

  void setMode(const int mode) { _mode = mode; }

  void setFlags(const int flags) { _flags = flags; }

  void setError(const int error) { _error = error; }

  void setMajorVersion(const int major_version)
  { _majorVersion = major_version; }

  void setMinorVersion(const int minor_version)
  { _minorVersion = minor_version; }

  void setSerialNum(const ssize_t serial_num)
  { _serialNum = serial_num; }

  /////////////////////////////
  // clear before adding parts.
  //
  // This initializes the number of parts to 0.
  //
  // It does NOT clear the header attributes set using the
  // set() routines.

  void clearParts();

  ///////////////////////////////////////////////////////////////
  // clear everything -- header and parts.
  //
  // A convenience routine for clients who want to call setType()
  // instead of setHdrAttr() before assembling a message.

   void clearAll();

  ////////////////////////////
  // Add a part to the object.
  //
  // The part is added at the end of the part list.
  //
  // The buffer must be in BE byte order.

  void addPart(const int type, const ssize_t len, const void *data);

  /////////////////////////////////////
  // assemble the parts into a message
  //
  // Returns pointer to the assembled message.
  
  ui08 *assemble();

  //////////////////////////////////
  // get pointer to assembled buffer

  inline ui08 *assembledMsg() const { return (_assembledMsg); }
  
  ///////////////////////////////
  // length of assembled message

  inline ssize_t lengthAssembled() const { return (_lengthAssembled); }
  
  //////////////////////////////////////////
  // print out main header and parts headers

  virtual void print(ostream &out, const char *spacer = "") const;

  ////////////////////////////////
  // print out the message header

  virtual void printHeader(ostream &out, const char *spacer = "") const;

  /////////////////////
  // print part headers
  // using IDs printed out simply as integers
  
  void printPartHeaders(ostream &out, const char *spacer = "") const;

  ///////////////////////////////////////////////////////////////
  // print part headers, using strings to label IDs as appropriate
  // Labels are passed in as a map.
  
  typedef pair<int, string> PartHeaderLabel;
  typedef map<int, string, less<int> > PartHeaderLabelMap;

  void printPartHeaders(ostream &out, const char *spacer,
			const PartHeaderLabelMap &labels) const;

  ////////////////////////////////
  // print out the message header
  // Backward-compatibility
  
  virtual void printHeader(ostream *out, const char *spacer = "") const;

  // debugging

  void setDebug(bool debugFlag = true) { _debug = debugFlag; }

protected:

  //
  // State info
  //

  memModel_t _memModel;
  int _type;
  int _subType;
  int _mode;
  int _flags;
  int _majorVersion;
  int _minorVersion;
  ssize_t _serialNum;
  int _category;
  int _error;
  ssize_t _nParts;

  // individual parts

  vector< DsMsgPart* > _parts;

  // assembled message

  ui08 *_assembledMsg;
  ssize_t _lengthAssembled;

  // debugging

  bool _debug;

  // copy this object
  
  DsMessage &_copy(const DsMessage &rhs);
  
private:

  ssize_t _nAssembledAlloc;

  // allocate the parts vector
  
  void _allocParts(const ssize_t n_parts);

  // allocate the buffer for the assembled message

  void _allocAssembledMsg();

};

#endif
