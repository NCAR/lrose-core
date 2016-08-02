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
// GenPoly.hh
//
// C++ wrapper for generic polyline/polygon data.
//
// Generic polyline data is stored in a buffer of length buf_len,
// in the following order:
//
//   GenPoly::header_t: just the one of these
//   GenPoly::vertex_t: n_vertices of these
//   Data values: n_fields * n_levels * fl32
//   Name string: name_len chars, including trailing null
//   Field info string: field_info_len chars, including trailing null
//   Text string: text_len chars, including trailing null
//   ID value: int
//
// Header:
//   See typedef
//
// Vertices:
//   See typedef.  There will be n_vertices of these.
//
// Data values:
//   The data is stored as fl32s, in a 1-D array, starting with level 0.
//   For 1-D data, n_levels is set to 1.
//
// Name string - optional:
//   The name of the polyline. If name_len is 0, the name is blank. 
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
//   Identifier of object with which this polyline is associated.
//
// Nancy Rehak, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// December 2003
//////////////////////////////////////////////////////////////

#ifndef _GenPoly_hh
#define _GenPoly_hh


#include <string>
#include <cstdio>
#include <iostream>
#include <vector>

#include <dataport/port_types.h>
#include <euclid/WorldPoint2D.hh>
#include <toolsa/MemBuf.hh>

using namespace std;

class GenPoly
{

public:

  //////////////////
  // Public types //
  //////////////////

  // struct for header data

  typedef struct
  {
    ti32 time;
    si32 n_vertices;// number of vertices in polyline
    si32 n_fields;  // number of data fields - must be at least 1.
    si32 n_levels;  // number of data levels - for 1D data this will be 1
    si32 name_len;  // length of name string, including trailing null
    si32 field_info_len; // length of field info string,
                         // including trailing null
    si32 text_len;  // length of optional text string
    si32 buf_len;   // total length of buffer
    si32 id;        // identifier of object with which this point is associated
    si32 closed_flag;  // 0 if object is polyline, 1 if object is polygon
                       // For a polygon, the first point does not have to
                       // be repeated at the end.
    si32 spare[2];
  } header_t;

  // Structure for vertex data

  typedef struct
  {
    fl32 lat;
    fl32 lon;
  } vertex_t;
  
  // class for field info - name and units

  class FieldInfo
  {
  public:
    string name;
    string units;
  };


  ////////////////////
  // Public methods //
  ////////////////////

  // constructor

  GenPoly();

  // destructor

  virtual ~GenPoly();
  
  //////////////////////////////////////////////
  // Check we have a valid object.
  // Use this after the set methods to check the object.
  // On failure, use getErrStr() to see error
  
  virtual bool check() const;

  ///////////////////////////////////////////////////////////////
  // set methods
  
  void clear();

  void setName(const string &name) { _name = name; }
  void setId(const int id) { _id = id; }
  void setTime(const time_t time) { _time = time; }
  void setExpireTime(const time_t time) { _expireTime = time; }
  void setNLevels(const int n_levels) { _nLevels = n_levels; }
  void setClosedFlag(const bool closed_flag) { _closed = closed_flag; }
  
  void clearVertices() { _vertices.erase(_vertices.begin(), _vertices.end()); }
  void addVertex(const vertex_t &vertex) { _vertices.push_back(vertex); }
  void addVertex(const WorldPoint2D &vertex)
    {
      vertex_t new_vertex;
      new_vertex.lat = vertex.lat;
      new_vertex.lon = vertex.lon;
      _vertices.push_back(new_vertex);
    }
  
  void clearVals() { _vals.erase(_vals.begin(), _vals.end()); }
  void addVal(double val) { _vals.push_back(val); }
  
  void clearFieldInfo();
  void addFieldInfo(const string &name, const string &units);
  
  // set the field info from a composite string
  // returns true on success, false on failure
  bool setFieldInfo(const string &infoStr);

  void setText(const string &text) { _text = text; }

  ///////////////////////////////////////////////////////////
  // disassemble()
  // Disassembles a buffer, sets the object values.
  // Handles byte swapping.
  // Returns true on success, false on failure

  bool disassemble(const void *buf, int len);

  //////////////////////////////////////////////////////////////////
  // get methods

  const string &getName() const { return (_name); }
  int getId() const { return _id; }
  string getFieldInfoStr() const;
  const string &getText() const { return (_text); }

  time_t getTime() const { return (_time); }
  time_t getExpireTime() const { return (_expireTime); }

  int getNFields() const { return (_fieldInfo.size()); }
  int getNLevels() const { return (_nLevels); }
  
  // Get a field number given the name
  // Returns -1 on error.
  
  int getFieldNum(const string &name) const;

  // Get a list of field numbers whose field names begin
  // with the given string.

  vector< int > getFieldListPrefix(const string &prefix) const;
  
  // get the field name and units, given the field number

  string getFieldName(const int field_num) const;
  string getFieldUnits(const int field_num) const;

  bool isClosed() const { return _closed; }
  
  ////////////////////////////////////////////////
  // get/reset data values
  
  // get a value, given the 1D index

  double get1DVal(const int index) const;
  void set1DVal(const int index, const double value);

  // get a value, given the 2D indices

  double get2DVal(const int level_num, const int field_num) const;
  void set2DVal(const int level_num, const int field_num, const double value);

  // missing data value

  static const double missingVal;

  ////////////////////////////////////////////////
  // get vertices

  inline int getNumVertices() const { return _vertices.size(); }
  
  inline vertex_t getVertex(const int vertex_num) const
    { return _vertices[vertex_num]; }
  
  void calcCentroid(double &centroid_lat, double &centroid_lon) const;
  void calcCentroid(float &centroid_lat, float &centroid_lon) const;
  
  ///////////////////////////////////////////
  // assemble()
  // Load up the buffer from the object.
  // Handles byte swapping.
  //
  // returns true on success, false on failure
  // Use getErrStr() on failure.
  
  bool assemble();

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

protected:

  time_t _time;
  time_t _expireTime;
  int _nLevels;
  int _id;
  bool _closed;
  
  string _name;
  string _text;
  mutable string _errStr;

  vector< vertex_t > _vertices;
  vector< double > _vals;
  vector< FieldInfo > _fieldInfo;

  MemBuf _memBuf;

private:

  void _combineFieldInfo(string &fieldInfoStr) const;

  static void _BE_from_header(header_t &hdr);
  static void _BE_to_header(header_t &hdr);
  static void _BE_from_vertex(vertex_t &vertex);
  static void _BE_to_vertex(vertex_t &vertex);

  static void _printHeader(ostream &out, const header_t hdr);

};


#endif
