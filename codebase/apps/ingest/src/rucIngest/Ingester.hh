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
/////////////////////////////////////////////////////////
// Ingester
//
// $Id: Ingester.hh,v 1.19 2016/03/07 01:23:11 dixon Exp $
//
////////////////////////////////////////////////////////
#ifndef _INGESTER_
#define _INGESTER_

#include <ctime>
#include <cstring>
#include <string>
#include <vector>
#include <list>
#include "GribMgr.hh"
#include "GribField.hh"
#include "Params.hh"

using namespace std;

class Ingester {
public:

  Ingester( Params &params );
  Ingester( Params &params, const vector<string>& fileList );
  Ingester( Params &params, const time_t& start,  const time_t& stop );
  ~Ingester();
   
  int processFile( string& filePath );
  inline void cleanup(){ _cleanup(); }

  int getForecastTime(){ return( _gribMgr.getForecastTime() ); }
  time_t getGenerateTime(){ return( _gribMgr.getGenerateTime() ); }

  fl32 getMissingVal(){ return( _missingVal ); }
  GribMgr* getGribMgr(){ return( &_gribMgr ); }

  const list<GribField*> &getGribFieldList(){ return( _gribFields ); }

  //
  // constants
  //
  static const int BUFFER_LEN;

private:

  //
  // Parameters
  //
  Params *_paramsPtr;

  bool _eofFound;

  GribMgr _gribMgr;

  //
  // list of GRIB field information structs
  //
  list<GribField*> _gribFields;

  //
  // requested output fields -- GRIB ID codes
  //
  list<Params::out_field_t> _outputFields;


  //
  // Missing value
  //
  fl32 _missingVal;

  //
  // Find the next instance of "GRIB" in the file
  //   
  size_t _findGrib(ui08 **gribPtr, size_t bufLen);

  //
  // check if we have a new GRIB field requested for output
  //
  bool _foundField(const int& param_id, const int& level_id, 
		   list<Params::out_field_t>& out, int& uc, 
		   float& u_rng, float& l_rng);


  //
  // 
  //
  bool _isFieldNew(const int& param_id, const int& level_id);

  //
  // 
  //
  GribField* _getField(const int& param_id, const int& level_id);

  //
  // clean up the GribField objects
  //
  void _cleanup();

};

#endif
