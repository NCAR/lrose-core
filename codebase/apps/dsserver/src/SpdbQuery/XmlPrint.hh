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
// XmlPrint.hh
//
// Xml Printing object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2009
//
/////////////////////////////////////////////////////////////

#ifndef XML_PRINT_HH
#define XML_PRINT_HH

#include <iostream>
#include <cstdio>
#include <Spdb/Spdb.hh>
#include "Args.hh"
using namespace std;

class XmlPrint {
  
public:

  // constructor
  
  XmlPrint(FILE *out, ostream &ostr,
           const Args &args);

  // destructor

  virtual ~XmlPrint();
  
  // print methods

  void chunkHdr(const Spdb::chunk_t &chunk, int startIndentLevel);
  void ltg(const Spdb::chunk_t &chunk, int startIndentLevel);
  void acGeoref(const Spdb::chunk_t &chunk, int startIndentLevel);
  void sigmet(const Spdb::chunk_t &chunk, int startIndentLevel);
  void taf(const Spdb::chunk_t &chunk, int startIndentLevel);
  void amdar(const Spdb::chunk_t &chunk, int startIndentLevel);
  void wxObs(const Spdb::chunk_t &chunk, int startIndentLevel);
  void xml(const Spdb::chunk_t &chunk, int startIndentLevel);

protected:
  
private:

  FILE *_out;
  ostream &_ostr;
  const Args &_args;

  double _minLat;
  double _maxLat;
  double _minLon;
  double _maxLon;
  
  bool _checkLatLon(double lat, double lon);

};

#endif

