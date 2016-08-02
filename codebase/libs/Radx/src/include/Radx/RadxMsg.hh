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
// RadxFile.hh
//
// RadxFile object
//
// Base class for radar radial data files
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2014
//
///////////////////////////////////////////////////////////////

#ifndef _RADX_MSG_HH_
#define _RADX_MSG_HH_

#include <string>
#include <vector>
#include <map>
#include <Radx/Radx.hh>
#include <Radx/RadxBuf.hh>
using namespace std;

//////////////////////////////
// forward class declarations
//

class RadxMsg

{

public:

  class Part;

  /*
   * header struct
   *
   * type, subType are defined and used for the
   * specific message type. These values may be used in whatever manner
   * the author chooses.
   *
   */
  
  typedef struct {
    
    Radx::si32 type;
    Radx::si32 subType;
    Radx::si32 nParts;
    Radx::si32 spare[6];
    
  } Hdr_t;

  /*
   * part struct
   *
   * dataType is defined by the author of the message type.
   * length refers to the length of the part data in bytes.
   * offset refers to the offset of the part data from the start
   * of the message.
   */
  
  typedef struct {
    
    Radx::si32 dataType;
    Radx::si32 offset;
    Radx::si32 len;
    Radx::si32 spare[3];
    
  } Part_t;
  
  // constructor
  
  RadxMsg(const string &name);

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
  // If msg_len is set, checks that the msg is big enough
  //   to hold at least a DsMsgHdr_t. Otherwise, assumes
  //   that the message is big enough and blindly copies
  //   memory.
  //
  // Returns: -1 If msg_len is set and is smaller than a DsMsgHdr_t.
  //           0 Otherwise.

  virtual int decodeHeader(const void *in_msg, const ssize_t msg_len = -1);

  ////////////////////////////////////////////////////
  // disassemble a message into parts, store in
  // RadxMsg object.
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

  inline const string &getName() const { return _name; }
  int getType() const { return (_type); }
  int getSubType() const { return (_subType); }

  //////////////////////////
  // get the number of parts

  inline ssize_t getNParts() const { return _nParts; }

  ////////////////////////////////////////////////
  // does a part exist?
  // returns the number of parts of the given type

  int partExists(const int data_type) const;

  ////////////////////////////////////////////
  // Get a part from the parts array, given
  // the index into the array.
  //
  // Returns pointer to part, NULL on failure.

  Part *getPart(const ssize_t index) const;

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

  Part *getPartByType(const int data_type, const ssize_t index = 0) const;
  
  // get the xml header for message
  
  string getXmlHdr() const;

  //////////////////////////////////////////
  // set the message header attributes
  //
  // These overwrite the existing attributes.

  void setHdrAttr(const int type,
		  const int sub_type = -1);

  void setType(const int type) { _type = type; }

  void setSubType(const int sub_type) {_subType = sub_type; }

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
  void addPart(const string &name, ssize_t len, const void *data);

  /////////////////////////////////////
  // assemble the parts into a message
  //
  // Returns pointer to the assembled message.
  
  Radx::ui08 *assemble();

  //////////////////////////////////
  // get pointer to assembled buffer

  inline Radx::ui08 *assembledMsg() const { return (_assembledMsg); }
  
  ///////////////////////////////
  // length of assembled message

  inline ssize_t lengthAssembled() const { return (_lengthAssembled); }
  
  //////////////////////////////////////////
  // print out main header and parts headers

  virtual void print(ostream &out, const char *spacer) const;

  ////////////////////////////////
  // print out the message header

  virtual void printHeader(ostream &out, const char *spacer) const;

  /////////////////////
  // print part headers
  // using IDs printed out simply as integers
  
  void printPartHeaders(ostream &out, const char *spacer) const;

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
  
  virtual void printHeader(ostream *out, const char *spacer) const;

  // debugging

  void setDebug(bool debugFlag = true) { _debug = debugFlag; }

protected:

  //
  // State info
  //

  string _name;
  int _type;
  int _subType;
  ssize_t _nParts;

  // individual parts

  vector<Part *> _parts;

  // assembled message
  
  Radx::ui08 *_assembledMsg;
  ssize_t _lengthAssembled;

  // debugging

  bool _debug;

  // copy this object
  
  virtual RadxMsg &_copy(const RadxMsg &rhs);
  
private:

  ssize_t _nAssembledAlloc;

  // allocate the parts vector
  
  void _allocParts(const ssize_t n_parts);

  // allocate the buffer for the assembled message

  void _allocAssembledMsg();

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

    // construct with message

    Part(const string &name, size_t length, const void *data);
    
    //////////////
    // destructor
    
    ~Part();
    
    /////////////////////////////
    // Assignment
    //
    
    Part &operator=(const Part &rhs);
    
    ////////////////////////////////////////////////////////////
    // load a part from an incoming message which is assumed to
    // be in BE byte order
    //
    // If msg_len is provided, the part is checked to make
    // sure it does not run over the end of the message.
    //
    // Returns 0 on success, -1 on error
    // Error occurs if end of part is beyond end of message.
    
    int loadFromMsg(const ssize_t part_num,
                    const void *in_msg,
                    const ssize_t msg_len = -1);
    
    ////////////////////////////////////////////////////
    // load a part from memory which is assumed to be in
    // host byte order
    
    void loadFromMem(const int type,
                     const ssize_t len,
                     const void *in_mem);
    
    //////////////////////////////////////////////////
    // get the type, length, offset and buffer pointer

    inline const string &getName() const { return _name; }
    inline int getType() const { return _type; }
    inline ssize_t getLength() const { return _length; }
    inline ssize_t getPaddedLength() const { return _paddedLength; }
    inline ssize_t getOffset() const { return _offset; }
    inline const Radx::ui08 *getBuf() const { return _buf; }
    
    // get the xml header for the part

    string getXmlHdr() const;

    // set offset
    inline void setOffset(const ssize_t offset) { _offset = offset; }
    
    // print header
    // If num is not specified, it is not printed
    
    void printHeader(ostream &out, const char *spacer, ssize_t num = -1) const;
    
    // print header with label
    // If num is not specified, it is not printed
    
    void printHeader(ostream &out, const char *spacer,
                     const string &label, ssize_t num = -1) const;
    
  protected:
    
  private:

    string _name;
    int _type; // part type
    ssize_t _length; // length of part data
    ssize_t _paddedLength; // padded to even 8-bytes
    ssize_t _offset; // offset in assembled message
    
    Radx::ui08 *_buf;
    ssize_t _nBufAlloc;
    
    RadxBuf _rbuf;

    void _allocBuf();
    Part &_copy(const Part &rhs);
    
  };
  
  /*******************
   * BE_to_Hdr()
   *
   * Convert BE to Hdr_t
   */
  
  static void BE_to_Hdr(Hdr_t *hdr);
  
  /*******************
   * BE_from_Hdr()
   *
   * Convert Hdr_t to BE
   */
  
  static void BE_from_Hdr(Hdr_t *hdr);
  
  /*******************
   * BE_to_Part()
   *
   * Convert BE to Part_t
   */
  
  static void BE_to_Part(Part_t *part);
  
  /***************************
   * BE_from_Part()
   *
   * Convert Part_t to BE
   */
  
  static void BE_from_Part(Part_t *part);

};

#endif
