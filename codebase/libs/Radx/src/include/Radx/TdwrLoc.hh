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
//////////////////////////////////////////////////////////////////////////
// 
// Class for getting location of TDWR station
// 
// Mike Dixon, EOL, NCAR, Boulder, CO, 80307, USA
// Nov 2013
//
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Lat/Lon from:
//
//  http://www.wispa.org/tdwr-locations-and-frequencies
//
// Ground heights from google:
//
//  http://www.daftlogic.com/sandbox-google-maps-find-altitude.htm
//
// Radar height = ground height plus a nominal 30 meters.
//////////////////////////////////////////////////////////////////////////

#ifndef _TDWR_LOC_HH_
#define _TDWR_LOC_HH_

#include <string>
using namespace std;

class TdwrLoc
{

public:

  TdwrLoc();
  ~TdwrLoc();

  /// load the location based on the name
  /// returns 0 on success, -1 in failure
  /// Use get() methods on success to get loc details

  int loadLocationFromName(const string &name);

  /// load the location based on the file path
  /// returns 0 on success, -1 in failure
  /// Use get() methods on success to get loc details

  int loadLocationFromFilePath(const string &path);

  /// get methods after loading

  const string &getName() const { return _name; }
  double getLatitudeDeg() const { return _latitudeDeg; }
  double getLongitudeDeg() const { return _longitudeDeg; }
  double getGroundHtM() const { return _groundHtM; }
  double getRadarHtM() const { return _radarHtM; }
  double getFreqGhz() const { return _freqGhz; }
  
private:

  string _name;
  double _latitudeDeg;
  double _longitudeDeg;
  double _groundHtM;
  double _radarHtM;
  double _freqGhz;

  // struct for location data
  
  typedef struct {
    const char* name;
    double latitudeDeg;
    double longitudeDeg;
    double groundHtM;
    double radarHtM;
    double freqGhz;
  } locInfo_t;

  static const int _nStations = 55;
  static const locInfo_t _locInfo[_nStations];

  void _load(int index);
  
};

#endif
