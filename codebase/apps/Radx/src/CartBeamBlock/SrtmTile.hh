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
// SrtmTile.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2014
//
///////////////////////////////////////////////////////////////
//
// SrtmTile handles the terrain information for a single
// 1 deg x 1 deg segment of the global grid.
// 
////////////////////////////////////////////////////////////////

#ifndef SrtmTile_HH
#define SrtmTile_HH

#include <string>
#include "Params.hh"
#include <toolsa/os_config.h>
#include <toolsa/TaThread.hh>
#include <toolsa/TaArray2D.hh>

using namespace std;

////////////////////////
// This class

class SrtmTile {
  
public:

  // constructor
  
  SrtmTile(const string &demDir,
           double centerLat,
           double centerLon,
           bool debug = false);
  
  // destructor
  
  ~SrtmTile();

  // get terrain ht for a point
  // returns 0 on success, -1 on failure
  // sets terrainHtM
  
  int getHt(double lat, double lon, int16_t &terrainHtM);
  
  // read file to update cache
  // returns 0 on success, -1 on failure
  
  int readForCache();

  // free arrays
  
  void freeHtArray();

  // get latest access time

  time_t getLatestAccessTime() const { return _latestAccessTime; }

protected:
  
private:

  TaThread::SafeMutex _localMutex;
  static TaThread::SafeMutex *_globalMutex;
  
  bool _debug;

  string _demDir;
  char _demFilePath[MAX_PATH_LEN];

  double _centerLat;
  double _centerLon;
  int _llLatDeg, _llLonDeg;
  int _nx, _ny;
  double _dx, _dy;
  
  time_t _latestAccessTime;
  
  int _ptsPerDeg;
  double _gridRes;

  int _fileUlLatDeg, _fileUlLonDeg;
  
  TaArray2D<int16_t> _htArray;
  
  int _readFromFile();
  int _findFile();

};

#endif
