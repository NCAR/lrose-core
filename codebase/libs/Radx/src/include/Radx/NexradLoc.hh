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
// Class for getting location of NEXRAD station
// 
// Mike Dixon, EOL, NCAR, Boulder, CO, 80307, USA
// August 2012
//
//////////////////////////////////////////////////////////////////////////

#ifndef _NEXRAD_LOC_HH_
#define _NEXRAD_LOC_HH_

#include <string>
using namespace std;

class NexradLoc
{

public:

  NexradLoc();
  ~NexradLoc();

  /// load the location based on the ID
  /// returns 0 on success, -1 in failure
  /// Use get() methods on success to get loc details

  int loadLocationFromId(int id);

  /// load the location based on the name
  /// returns 0 on success, -1 in failure
  /// Use get() methods on success to get loc details

  int loadLocationFromName(const string &name);

  /// load the location based on the file path
  /// returns 0 on success, -1 in failure
  /// Use get() methods on success to get loc details

  int loadLocationFromFilePath(const string &path);

  /// get methods after loading

  int getId() const { return _id; }
  const string &getName() const { return _name; }
  const string &getCity() const { return _city; }
  const string &getState() const { return _state; }
  double getLatDecimalDeg() const { return _latDecimalDeg; }
  double getLonDecimalDeg() const { return _lonDecimalDeg; }
  double getHtMeters() const { return _htMeters; }
  
private:

  int _id;
  string _name;
  string _city;
  string _state;
  double _latDecimalDeg;
  double _lonDecimalDeg;
  double _htMeters;

  // struct for location data
  
  typedef struct {
    int id;
    const char* name;
    const char* city;
    const char* state;
    int latDeg, latMin, latSec;
    int lonDeg, lonMin, lonSec;
    int htMeters;
  } locInfo_t;

  static const int _nStations = 156;
  static const locInfo_t _locInfo[_nStations];

  void _load(int index);
  
};

#endif
