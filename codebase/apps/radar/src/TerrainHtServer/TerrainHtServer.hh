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
// TerrainHtServer.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2014
//
///////////////////////////////////////////////////////////////
//
// TerrainHtServer reads requests from a client, providing a
// lat/lon position. It returns the terrain height, and whether
// the location is water or not.
// 
////////////////////////////////////////////////////////////////

#ifndef TerrainHtServer_H
#define TerrainHtServer_H

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"

class ServerThread;
class SquareDegree;

using namespace std;

////////////////////////
// This class

class TerrainHtServer {
  
public:

  // constructor
  
  TerrainHtServer (int argc, char **argv);

  // destructor
  
  ~TerrainHtServer();

  // run 

  int Run();

  // data members

  bool isOK;

  // get terrain ht and water flag for a point
  // returns 0 on success, -1 on failure
  // sets terrainHtM and isWater args
  
  int getHt(double lat, double lon,
            double &terrainHtM, bool &isWater);

protected:
  
private:

  const static int nLat = 180;
  const static int nLon = 360;

  string _progName;
  Args _args;
  char *_paramsPath;
  Params _params;

  ServerThread *_serverThread;
  SquareDegree ***_tiles;

  double _latestLat;
  double _latestLon;
  
  int _getHt(double lat, double lon,
             double &terrainHtM, bool &isWater);

  void _updateCache();
  int _readForCache(double lat, double lon);
  void _freeTileMemory();

};

#endif
