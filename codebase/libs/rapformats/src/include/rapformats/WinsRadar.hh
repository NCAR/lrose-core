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
// WinsRadar.hh
//
// WinsRadar object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2000
//
///////////////////////////////////////////////////////////////

#ifndef WinsRadar_H
#define WinsRadar_H


#include <dataport/port_types.h>
#include <iostream>
#include <string>
using namespace std;

class WinsRadar {
  
public:

  typedef struct {
    char volume_info[20];
    si32 lat;
    si32 lon;
    si16 ht;
    si16 year;
    si16 month;
    si16 day;
    si16 hour;
    si16 min_start;
    si16 min_end;
    char field_name[4];
    char field_units[8];
    si16 nx;
    si16 ny;
    si16 dx;
    si16 dy;
    si16 scale;
    si16 data_for_byte_0;
    si16 data_for_byte_254;
    si16 log_flag;
    char color_table_name[8];
    si32 min_lat;
    si32 min_lon;
    si32 max_lat;
    si32 max_lon;
    si32 spare[104];
  } header_t;

  // constructor

  WinsRadar ();

  // destructor
  
  ~WinsRadar();

  // read in a file

  int readFile(const string &file_path);

  // write out to a file

  int writeFile (const string &file_path);

  // print the header and data array

  void printHeader(ostream &out) const;
  void printData(ostream &out) const;

  // set debugging

  void setDebug(bool debug = true) { _debug = debug; }
  
  // access to data members
  
  const header_t &getHeader() const { return _header; }
  const char *getVolumeInfo() const { return _volumeInfo; }
  const char *getFieldName() const { return _fieldName; }
  const char *getFieldUnits() const { return _fieldUnits; }
  const char *getColorTableName() const { return _colorTableName; }
  const ui08 *getData() const { return _data; }
  double getVmin() const { return _vmin; }
  double getVmax() const { return _vmax; }
  double getBias() const { return _bias; }
  double getScale() const { return _scale; }

  // set the header

  void setHeader(const header_t &hdr) { _header = hdr; }

protected:
  
  // data members

  header_t _header;
  char _volumeInfo[24];
  char _fieldName[8];
  char _fieldUnits[8];
  char _colorTableName[12];
  ui08 *_data;
  bool _debug;

  bool _headerSwapped;

  double _vmin;
  double _vmax;
  double _bias;
  double _scale;

  int _interpretHeader();

  void _swapHeader(header_t &hdr);

  void _terminateString(const char *input_text,
			char *stored_text,
			int max_len);

private:

};

#endif
