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
/*
 *   Module: Sndg.hh
 *
 *   Author: Sue Dettling
 *
 *   Date:   6/19/03
 *
 *   Description: 
 */

#ifndef SNDG_HH
#define SNDG_HH

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <toolsa/MemBuf.hh>
#include <dataport/port_types.h>
using namespace std;

class Sndg {

private:
public:

  static const int SRC_NAME_LEN = 80;
  static const int SRC_FMT_LEN = 40;
  static const int SITE_NAME_LEN = 80;

  static const int HDR_SPARE_INTS = 2;
  static const int HDR_SPARE_FLOATS = 2;
  static const int PT_SPARE_FLOATS = 12;
  
  static const int HDR_NBYTES_32 = 48;
  static const int PT_NBYTES_32 = 128;


  static const fl32 VALUE_UNKNOWN;
  static const fl32 QC_GOOD;
  static const fl32 QC_MAYBE;
  static const fl32 QC_BAD;
  static const fl32 QC_INTERP;
  static const fl32 QC_MISSING;

  typedef struct 
  {
    si32 launchTime;
    si32 nPoints;
    si32 sourceId;
    si32 leadSecs;
    si32 spareInts[HDR_SPARE_INTS];
    
    fl32 lat;
    fl32 lon;
    fl32 alt;
    fl32 spareFloats[HDR_SPARE_FLOATS];

    si32 version;
    
    char sourceName[SRC_NAME_LEN];
    char sourceFmt[SRC_FMT_LEN];
    char siteName[SITE_NAME_LEN];
    
  } header_t;

  typedef struct 
  {
    fl32 time;               // seconds since launch time
    fl32 pressure;           // mb
    fl32 altitude;           // m
    fl32 u;                  // m/s
    fl32 v;                  // m/s
    fl32 w;                  // m/s
    fl32 rh;                 // % (0-100)
    fl32 temp;               // C
    fl32 dewpt;              // C
    fl32 windSpeed;          // m/s    
    fl32 windDir;            // direction wind is FROM in deg
    fl32 ascensionRate;      // m/s
    fl32 longitude;          // deg
    fl32 latitude;           // deg
    fl32 pressureQC;
    fl32 tempQC;
    fl32 humidityQC;
    fl32 uwindQC;
    fl32 vwindQC;
    fl32 ascensionRateQC;
    fl32 spareFloats[PT_SPARE_FLOATS];
    
  } point_t;

  // constructor

  Sndg();

  // destructor

  ~Sndg();
  
  ///////////////////////////////////////////
  // assemble()
  // Load up the buffer from the object.
  // Handles byte swapping.
  
  void assemble();

  // get the assembled buffer info

  void *getBufPtr() const { return _memBuf.getPtr(); }
  int getBufLen() const { return _memBuf.getLen(); }

  ///////////////////////////////////////////////////////////
  // disassemble()
  // Disassembles a buffer, sets the values in the object.
  // Handles byte swapping.
  // Returns 0 on success, -1 on failure

  int disassemble(const void *buf, int len);

  // set methods

  void setHeader(const header_t &hdr);
  void clearPoints();
  void addPoint(const point_t &pt, const bool update_pt_count = false);
  void setPoints(const vector< point_t > &new_points);
  
  // get methods

  int getVersion() { return _version; }
  const header_t &getHeader() const { return _hdr; }
  int getNumPoints() { return _points.size(); }
  const vector<point_t> &getPoints() const { return _points; }
  vector<point_t> &getPointsEditable() { return _points; }

  /////////////////////////
  // print

  void print(ostream &out, const string &spacer = "") const;
  void print_header( ostream &out, const string &spacer = "") const;

  void print(FILE *out, const string &spacer = "") const;
  void print_header(FILE *out, const string &spacer = "") const;
  
protected:

private:
  
  // data members

  int _version;
  header_t _hdr;
  vector<point_t> _points;
  MemBuf _memBuf;


  static void _print_point(const point_t &pt,
			   ostream &out, const string &spacer = "");
  static void _print_point(const point_t &pt,
			   FILE *out, const string &spacer = "");

  inline static void _print_value(ostream &out, const string &info1,
				  const float value, const string &info2)
  {
    if (value == VALUE_UNKNOWN)
      out << info1 << "VALUE_UNKNOWN" << endl;
    else
      out << info1 << value << info2 << endl;
  }
  
  inline static void _print_value(FILE *out, const string &info1,
				  const float value, const string &info2)
  {
    if (value == VALUE_UNKNOWN)
      fprintf(out, "%sVALUE_UNKNOWN\n", info1.c_str());
    else
      fprintf(out,"%s%f%s\n", info1.c_str(), value, info2.c_str());
  }
  
  // byte swapping

  static void _header_to_BE(header_t &hdr);
  static void _header_from_BE(header_t &hdr);
  static void _point_to_BE(point_t &pt);
  static void _point_from_BE(point_t &pt);

};

#endif /* SNDG_HH */
