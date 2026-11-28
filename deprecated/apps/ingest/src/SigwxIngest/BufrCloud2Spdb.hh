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


#ifndef BUFR_CLOUD2SPDB_HH
#define BUFR_CLOUD2SPDB_HH

#include <string>
#include <cstdlib>
#include <sys/time.h>
#include <toolsa/DateTime.hh>
#include <mel_bufr/mel_bufr.h>
#include <Spdb/DsSpdb.hh>
#include "BufrVariableDecoder.hh"
using namespace std;

class BufrCloud2Spdb {
  
public:

  // constructor

  BufrCloud2Spdb(BUFR_Val_t &bv, DateTime &genTime, 
		 DateTime &fcstTime,int &altLowerBound, 
		 int &altUpperBound, string outUrl, bool debug);

  // destructor
  
  ~BufrCloud2Spdb();
  /**
   * Expected fxy variables in a cloud report
   *  0-08-011 = table; Meteorological feature (==12)
   *  0-08-007 = table; Dimensional significance (0 = point, 1 = Line, 
   *             2 = area, 3= volume)
   *  0-07-002 = Height or altitude (cloud base in meters)
   *  0-07-002 = Height or altitude (cloud top in meters )
   * A series of latitude and longitude points determining cloud boundary
   *  0-05-002 = Latitude (coarse accuracy)
   *  0-06-002 = Longitude (coarse accuracy)
   *
   *  0-20-008 = table; Cloud distribution for aviation
   *  0-20-012 = table; Cloud type
   * 
   *  0-08-007 = table; Dimensional significance (end/cancel)
   *  0-08-011 = table; Meteorological feature (end/cancel)
   */
  BufrVariableDecoder::Status_t process();

protected:
  
private:

  const static int VAR_NOT_SET;
  const static int BUFRCLOUD2SPDB_MISSING;

  BUFR_Val_t _bv;
  DateTime _genTime;
  DateTime _fcstTime;
  DateTime _altLowerBound;
  DateTime _altUpperBound;
  BufrVariableDecoder _bufrDecoder;
  DsSpdb _spdb;
  string _outUrl;
  int _metCode;
  int _cloudBase;
  int _cloudTop;
  int _featureGeom;
  vector <pair <float, float> > _cloudBoundary;
  int _distCode;
  int _cloudType;
  bool _haveData;
  bool _debug;

  void _clearCloudVars();
  bool _varsSet();
  int _createGenPoly();
  int _writeSpdb();
  void _printCloudVars();

};

#endif
