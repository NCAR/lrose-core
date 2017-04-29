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
//////////////////////////////////////////////////////////////////////////
// RadxMsg.hh
//
// Class for serializing and deserializing Radx objects
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2017
//
//////////////////////////////////////////////////////////////////////////
//
// A message consists of, in order:
//
// (1) MsgHdr_t, which specifies nParts, the number of message parts 
// (2) nParts * PartHdr_t, the header for each part
// (3) nParts * contents - contents are padded to 8-byte boundaries 
//
// The message length is:
//   sizeof(MsgHdr_t) +
//   nParts * sizeof(PartHdr_t) +
//   sum of padded length for each part.
//
// The PartHdr_t headers contain the length and offset of each part.
//
// The type, subType and partType values are defined by the user.
//
// No byte swapping is done when the objects are created.
// When a message is decoded, swapping is performed as required.
//
//////////////////////////////////////////////////////////////////////////

#ifndef _RADX_MSG_HH_
#define _RADX_MSG_HH_

#include <string>
#include <vector>
#include <map>
#include <Radx/Radx.hh>
#include <Radx/RadxBuf.hh>
using namespace std;

//////////////////////////////
// class declarations

class RadxMsg

{

public:

  class Part;

  // message header struct - for assembled messages
  //
  // the cookie is set to the well-known value and is used to detect swapping.
  // msgType and subType are defined for the specific message type.
  // These values are set by the user.
  
  typedef struct {
    
    Radx::si64 cookie;
    Radx::si64 msgType;
    Radx::si64 subType;
    Radx::si64 nParts;
    Radx::si64 spare[2];
    
  } MsgHdr_t;

  //  part header struct - for assembled messages
  //
  //  partType is defined by the author of the message type.
  //  length refers to the length of the part data in bytes.
  //  padded length pads to an 8-byte boundary.
  //  offset refers to the offset of the part data from the start
  //  of the message.
  
  typedef struct {
    
    Radx::si64 partType;
    Radx::si64 offset;
    Radx::si64 length;
    Radx::si64 paddedLength;
    Radx::si64 spare[2];
    
  } PartHdr_t;
  
  // constructor
  
  RadxMsg(int msgType = 0, int subType = 0);

  // copy constructor

  RadxMsg(const RadxMsg &rhs);

  // destructor

  virtual ~RadxMsg();

  // assignment
  
  RadxMsg & operator=(const RadxMsg &rhs);

  //////////////////////////
  // decode a message header
  //
  // This is used if you just want to peek at the header before
  // deciding how to handle the message.
  // 
  // Returns 0 on success, -1 on error
  
  virtual int decodeHeader(const void *inMsg, size_t msgLen);

  ////////////////////////////////////////////////////
  // disassemble a message into parts, store in
  // RadxMsg object.
  //
  // If msgLen is provided, the parts are checked to make
  // sure they do not run over the end of the message.
  //
  // Returns 0 on success, -1 on error

  int disassemble(const void *inMsg, size_t msgLen);

  /////////////////////////////
  // get the message attributes

  int getMsgType() const { return (_msgType); }
  int getSubType() const { return (_subType); }

  //////////////////////////
  // get the number of parts

  inline ssize_t getNParts() const { return _nParts; }

  ////////////////////////////////////////////////
  // does a part exist?
  // returns the number of parts of the given type

  int partExists(int partType) const;

  ////////////////////////////////////////////
  // Get a part from the parts array, given
  // the index into the array.
  //
  // Returns pointer to part, NULL on failure.

  Part *getPart(size_t index) const;

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

  Part *getPartByType(int partType, int index = 0) const;
  
  //////////////////////////////////////////
  // set the message header types

  void setMsgType(int msgType) { _msgType = msgType; }
  void setSubType(int subType) {_subType = subType; }

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
  // The part is added at the end of the part list.
  
  void addPart(int partType, size_t len, const void *data);
  
  /////////////////////////////////////
  // assemble the parts into a message
  // Returns pointer to the assembled message.
  
  void *assemble();

  //////////////////////////////////
  // get pointer to assembled buffer

  inline void *assembledMsg() const { return _assembledMsg.getPtr(); }
  
  ///////////////////////////////
  // length of assembled message
  
  inline ssize_t lengthAssembled() const { return _assembledMsg.getLen(); }
  
  //////////////////////////////////////////
  // print out main header and parts headers

  virtual void print(ostream &out, const char *spacer) const;
  
  ////////////////////////////////
  // print out the message header
  
  virtual void printHeader(ostream &out, const char *spacer) const;
  
  /////////////////////
  // print part headers
  
  void printPartHeaders(ostream &out, const char *spacer) const;

  ///////////////////////////////////////////////////////////////
  // print part headers, using strings to label IDs as appropriate
  // Labels are passed in as a map.
  
  typedef pair<int, string> PartHeaderLabel;
  typedef map<int, string, less<int> > PartHeaderLabelMap;
  
  void printPartHeaders(ostream &out, const char *spacer,
			const PartHeaderLabelMap &labels) const;
  
  // debugging

  void setDebug(bool debugFlag = true) { _debug = debugFlag; }

  // is swapping required?

  bool getSwap() const { return _swap; }

protected:

  // well known value - used to check for swapping

  static const int _cookie = 1234567890;

  // debugging
  
  bool _debug;

  // flag to indicate swapping is requried

  bool _swap;

  // State info

  int _msgType;
  int _subType;
  int _nParts;

  // individual parts
  
  vector<Part *> _parts;

  // assembled message
  
  RadxBuf _assembledMsg;
  
  // copy this object
  
  virtual RadxMsg &_copy(const RadxMsg &rhs);
  
private:

  // allocate the parts vector
  
  void _allocParts(size_t nParts);

public:

  // inner class for message part
  
  class Part
  {
    
  public:
    
    ///////////////
    // constructor
    // You must choose the memory model in the constructor.
    
    Part();
    
    /////////////////////////////
    // Copy constructor
    //
    
    Part(const Part &rhs);

    // construct with data

    Part(int partType, size_t length, const void *data);
    
    //////////////
    // destructor
    
    ~Part();
    
    /////////////////////////////
    // Assignment
    //
    
    Part &operator=(const Part &rhs);
    
    ////////////////////////////////////////////////////////////
    // load part header from the part object
    
    void loadPartHdr(PartHdr_t &phdr);

    ////////////////////////////////////////////////////////////
    // load a part from an incoming message, which may
    // be swapped.
    //
    // Returns 0 on success, -1 on error
    // Error occurs if end of part is beyond end of message.
    
    int loadFromMsg(size_t partNum,
                    const void *inMsg,
                    size_t msgLen,
                    bool swap);
    
    ////////////////////////////////////////////////////
    // load a part from memory which is assumed to be in
    // host byte order
    
    void loadFromMem(int partType,
                     size_t len,
                     const void *inMem);
    
    //////////////////////////////////////////////////
    // get the type, length, offset and buffer pointer
    
    inline int getPartType() const { return _partType; }
    inline ssize_t getLength() const { return _length; }
    inline ssize_t getPaddedLength() const { return _paddedLength; }
    inline ssize_t getOffset() const { return _offset; }
    inline const void *getBuf() const { return _rbuf.getPtr(); }
    
    // set offset
    inline void setOffset(size_t offset) { _offset = offset; }
    
    // print header
    // If num is not specified, it is not printed
    
    void printHeader(ostream &out, const char *spacer, int num = -1) const;
    
    // print header with label
    // If num is not specified, it is not printed
    
    void printHeader(ostream &out, const char *spacer,
                     const string &label, int num = -1) const;
    
  protected:
    
  private:

    int _partType; // part type
    ssize_t _length; // length of part data
    ssize_t _paddedLength; // padded to even 8-bytes
    ssize_t _offset; // offset in assembled message
    
    RadxBuf _rbuf;
    
    Part &_copy(const Part &rhs);
    
  };

  /////////////////////////////////////
  // swap headers
  /////////////////////////////////////

  static void swapMsgHdr(MsgHdr_t &hdr);
  static void swapPartHdr(PartHdr_t &part);

};

#endif
