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
// GenPt.hh
//
// C++ wrapper for generic point data.
//
// Generic point data is stored in a buffer of length buf_len,
// in the following order:
//
//   GenPt::header_t
//   Data values: n_fields * n_levels * fl32
//   Name string: name_len chars, including trailing null
//   Field info string: field_info_len chars, including trailing null
//   Text string: text_len chars, including trailing null
//
// Header:
//   See typedef
//
// Data values:
//   The data is stored as fl32s, in a 1-D array, starting with level 0.
//   For 1-D data, n_levels is set to 1.
//
// Name string - optional:
//   The name of the point. If name_len is 0, the name is blank. 
//
// Field info string - required:
//   The field information - field names and units. Commas delimit the
//   fields. Colons separate the field name from the units.
//   e.g. precip:mm,Temp:C,RH:%
//
// Text string - optional:
//   Text string. If text_len is zero, text is blank.   
//
// ID value - optional:
//   Identifier of object with which this point is associated.
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// March 2000
//////////////////////////////////////////////////////////////

#ifndef _GenPt_hh
#define _GenPt_hh


#include <string>
#include <cstdio>
#include <iostream>
#include <vector>
#include <toolsa/MemBuf.hh>
#include <dataport/port_types.h>
using namespace std;

class GenPt {

public:

  // struct for header data

  typedef struct {
    ti32 time;
    fl32 lat;
    fl32 lon;
    si32 n_fields;  // number of data fields - must be at least 1.
    si32 n_levels;  // number of data levels - for 1D data this will be 1
    si32 name_len;  // length of name string, including trailing null
    si32 field_info_len; // length of field info string,
                         // including trailing null
    si32 text_len;  // length of optional text string
    si32 buf_len;   // total length of buffer
    si32 id;        // identifier of object with which this point is associated
    si32 type;      // one of data_type_t
    si32 spare;
  } header_t;

  typedef enum {
    DATA_SURFACE =  0,  // surface data
    DATA_SOUNDING = 1,  // sounding data
    DATA_MOBILE  =  2   // mobile data
  } data_type_t;

  // class for field info - name and units

  class FieldInfo {
  public:
    string name;
    string units;
  };

  // constructor

  GenPt();

  // destructor

  ~GenPt();
  
  //////////////////////////////////////////////
  // Check we have a valid object.
  // Use this after the set methods to check the object.
  // On failure, use getErrStr() to see error
  
  bool check() const;

  ///////////////////////////////////////////////////////////////
  // set methods
  
  void clear();

  void setName(const string &name) { _name = name; }
  void setId(const int id) { _id = id; }
  void setTime(time_t time) { _time = time; }
  void setLat(double lat) { _lat = lat; }
  void setLon(double lon) { _lon = lon; }
  void setNLevels(int n_levels) { _nLevels = n_levels; }
  void setType(data_type_t type) { _type = type;}

  void clearVals() { _vals.erase(_vals.begin(), _vals.end()); }
  void addVal(double val) { _vals.push_back(val); }
  
  void clearFieldInfo();
  void addFieldInfo(const string &name, const string &units);
  
  // set the field info from a composite string
  // returns 0 on success, -1 on failure
  int setFieldInfo(const string &infoStr);

  void setText(const string &text) { _text = text; }

  ///////////////////////////////////////////////////////////
  // disassemble()
  // Disassembles a buffer, sets the object values.
  // Handles byte swapping.
  // Returns 0 on success, -1 on failure

  int disassemble(const void *buf, int len);

  //////////////////////////////////////////////////////////////////
  // get methods

  const string &getName() const { return (_name); }
  int getId() const { return _id; }
  string getFieldInfoStr() const;
  const string &getText() const { return (_text); }
  data_type_t getType() const { return _type; }

  time_t getTime() const { return (_time); }
  double getLat() const { return (_lat); }
  double getLon() const { return (_lon); }

  int getNFields() const { return (_fieldInfo.size()); }
  int getNLevels() const { return (_nLevels); }
  
  // Get a field number given the name
  // Returns -1 on error.
  
  int getFieldNum(const string &name) const;

  // get the field name and units, given the field number

  string getFieldName(int field_num);
  string getFieldUnits(int field_num);

  ////////////////////////////////////////////////
  // get data values
  
  // get a value, given the 1D index

  double get1DVal(int index) const;

  // get a value, given the 2D indices

  double get2DVal(int level_num, int field_num) const;

  // missing data value

  static const double missingVal;

  ///////////////////////////////////////////
  // assemble()
  // Load up the buffer from the object.
  // Handles byte swapping.
  //
  // returns 0 on success, -1 on failure
  // Use getErrStr() on failure.
  
  int assemble();

  // get the assembled buffer pointer

  void *getBufPtr() const { return _memBuf.getPtr(); }
  int getBufLen() const { return _memBuf.getLen(); }

  ////////////////
  // error string
  
  const string &getErrStr() const { return (_errStr); }

  /////////////////////////
  // print

  void print(FILE *out) const;
  void print(ostream &out) const;

  char* dataType2Str(data_type_t type) const;

protected:

  time_t _time;
  double _lat;
  double _lon;
  int _nLevels;
  int _id;
  data_type_t _type;
  
  string _name;
  string _text;
  mutable string _errStr;

  vector<double> _vals;
  vector<FieldInfo> _fieldInfo;

  MemBuf _memBuf;

private:

  void _combineFieldInfo(string &fieldInfoStr) const;
  void _BE_from_header(header_t &hdr);
  void _BE_to_header(header_t &hdr);

};


#endif
