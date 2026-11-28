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


#ifndef BUFR_STORM2SPDB_HH
#define BUFR_STORM2SPDB_HH

#include <string>
#include <cstdlib>
#include <sys/time.h>
#include <toolsa/DateTime.hh>
#include <mel_bufr/mel_bufr.h>
#include "BufrVariableDecoder.hh"

using namespace std;

class BufrStorm2Spdb {
  
public:

  /**
   * Constructor
   */
  BufrStorm2Spdb(BUFR_Val_t &bv, DateTime &genTime, 
		 DateTime &fcstTime,int &altLowerBound, 
		 int &altUpperBound, string outUrl, bool debug);

  /**
   * Destructor
   */
  ~BufrStorm2Spdb();

  /** 
   * Expected storm fxy's
   *      0  8  5  Meteorological attribute significance (storm centre)
   *      0  8  7  Dimensional significance (value for point)
   *      0  5  2  Latitude (coarse)
   *      0  6  2  Longitude (coarse)
   *      0  1  26  WMO storm name (use "unknown" for a sandstorm)
   *      0  19  1  Synoptic features (value for type of storm)
   *      0  8  7  Dimensional significance (cancel)
   *      0  8  5  Meteorological attribute significance (cancel/end of object)
   */
 BufrVariableDecoder::Status_t   process();

protected:
  
private:
  
  const static int VAR_NOT_SET;
  const static int BUFRSTORM2SPDB_MISSING;

  BUFR_Val_t _bv;
  DateTime _genTime;
  DateTime _fcstTime;
  DateTime _altLowerBound;
  DateTime _altUpperBound;
  BufrVariableDecoder _bufrDecoder;
  string _outUrl;
  DsSpdb _spdb;
  int _code;
  bool _debug;
  int _featureGeom;
  char _stormName[32];
  float _stormCenterLat;
  float _stormCenterLon;
  int _synopticFeature;
  bool _haveData;

  void _clearStormVars();
  bool _varsSet();
  int _createGenPt();
  int _writeSpdb();
  void _printStormVars();
 
};


#endif


