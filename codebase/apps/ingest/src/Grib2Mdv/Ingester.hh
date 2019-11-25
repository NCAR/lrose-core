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
////////////////////////////////////////////////////////
#ifndef _INGESTER_
#define _INGESTER_

#include <ctime>
#include <string>
#include <vector>
#include <list>
#include <toolsa/membuf.h>
#include "GribMgr.hh"
#include "GribField.hh"
#include "Params.hh"

using namespace std;


class Ingester {
public:


  Ingester( Params &params );
  Ingester( Params &params, const vector<string>& fileList );
  Ingester( Params &params, const time_t& start,  const time_t& stop );
  virtual ~Ingester();
 
  /// Set up the Ingester for a multi-file conversion session.
  /// \param inputGribType is the kind of GRIB file to process
  void setup(Params::grib_type_t inputGribType);

  /// convert a single file
  int processFile( string& filePath );

  /// Load the next GRIB record from the file into _gribBuf.
  /// Generally you call this routine repeatedly.
  /// _gribBuf is loaded with one entire grib record.
  /// This routine may set _eofFound to true, so stop reading then.
  /// \param file the open file being processed
  /// \return true if a record was successfully loaded
  bool loadNextGrib(FILE *file);

  /// clean up after processing a single file
  inline void cleanup(){ _cleanup(); }

  /// Tear down the Ingester after a multi-file conversion session.
  void tearDown();

  int getForecastTime(){ return( _gribMgr->getForecastTime() ); }
  time_t getGenerateTime(){ return( _gribMgr->getGenerateTime() ); }

  fl32 getMissingVal(){ return( _missingVal ); }
  GribMgr* getGribMgr(){ return( _gribMgr ); }

  const list<GribField*> &getGribFieldList(){ return( _gribFields ); }

  /// access to private members
  bool getEofFound()		{ return _eofFound; }

  //
  // constants
  //

private:

  //
  // Parameters
  //
  Params *_paramsPtr;

  bool _eofFound;

  MEMbuf *_gribBuf;	/// memory buffer that holds the grib record
  GribMgr *_gribMgr;

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

  /// Check if we have a new GRIB field requested for output.
  /// \param param_id from the grib record
  /// \param level_id from the grib record
  /// \param out list of fields requested by user in params file
  /// \param uc receives the user-requested output units
  bool _foundField(const int& param_id, const int& level_id, 
		   list<Params::out_field_t>& out, int& uc);


  //
  // 
  //
  bool _isFieldNew(const int& param_id, const int& level_id,
                   const time_t& generate_time, const int& forecast_time);

  //
  // 
  //
  GribField* _getField(const int& param_id, const int& level_id,
                       const time_t& generate_time, const int& forecast_time);

  //
  // clean up the GribField objects
  //
  void _cleanup();

};

#endif
