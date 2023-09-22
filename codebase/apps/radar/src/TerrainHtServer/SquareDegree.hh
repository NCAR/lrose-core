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
// SquareDegree.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2014
//
///////////////////////////////////////////////////////////////
//
// SquareDegree handles the terrain information for a single
// 1 deg x 1 deg segment of the global grid.
// 
////////////////////////////////////////////////////////////////

#ifndef SquareDegree_HH
#define SquareDegree_HH

#include <string>
#include "Params.hh"
#include <toolsa/TaThread.hh>
#include <toolsa/os_config.h>
#include <dataport/port_types.h>
#include <Ncxx/Nc3File.hh>

using namespace std;

////////////////////////
// This class

class SquareDegree {
  
public:

  // constructor
  
  SquareDegree(const Params &params,
               double centerLat,
               double centerLon);
  
  // destructor
  
  ~SquareDegree();

  // get terrain ht and water flag for a point
  // returns 0 on success, -1 on failure
  // sets terrainHtM and isWater args
  
  int getHt(double lat, double lon,
            double &terrainHtM, bool &isWater);

  // read file to update cache
  // returns 0 on success, -1 on failure
  
  int readForCache();

  // free arrays
  
  void freeHtAndWaterArrays();

  // get latest access time

  time_t getLatestAccessTime() const { return _latestAccessTime; }

  // constants

  static const int PtsPerDeg = 120;
  static const double GridRes;

protected:
  
private:

  TaThread::SafeMutex _localMutex;
  static TaThread::SafeMutex *_globalMutex;
  
  Params _params;
  time_t _latestAccessTime;
  
  double _centerLat;
  double _centerLon;
  int _ulLatDeg, _ulLonDeg;
  
  char _demFilePath[MAX_PATH_LEN];
  char _waterFilePath[MAX_PATH_LEN];
  
  int _fileNx, _fileNy;
  double _dx, _dy;
  int _fileUlLatDeg, _fileUlLonDeg;

  fl32 **_htArray;
  ui08 **_waterArray;
  bool _waterAvail;

  Nc3File *_ncFile;
  Nc3Error *_ncErr;
  string _ncPathInUse;

  int _readFromFile();
  int _readWaterFile(size_t ulOffset);
  int _findFiles();

  int _openNc3File(const string &path);
  void _closeNc3File();
  int _readNc3Dim(const string &name, Nc3Dim* &dim);

};

#endif
