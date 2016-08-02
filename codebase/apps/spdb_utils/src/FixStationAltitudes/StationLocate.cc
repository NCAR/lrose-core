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
///////////////////////////////////////////////////////////////////
// StationLocate.cc : Relates a metar message to a lat,lon based
// on a file
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 1999
//
/////////////////////////////////////////////////////////////////////

#include "StationLocate.hh"

#include <cstdio>
#include <cerrno>
#include <cstring>
#include <iostream>

// constructor
  
StationLocate::StationLocate()
  
{

}

// Destructor

StationLocate::~StationLocate()
{

}

/////////////////////////////////////////////////////
// load station info from file
//
// Returns 0 on success, -1 on failure

int StationLocate::load(const string &StationList)
  
{

  FILE *fp = fopen(StationList.c_str(), "rt");
  if (fp == NULL) {
    int errNum = errno;
    cerr << "ERROR - StationLocate::StationLocate" << endl;
    cerr << "  Could not open station list file: " << StationList << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  StationLocationType loc;
  char StationName[5];

  char Line[1024];
  int num = 0;
  while (NULL != fgets(Line, 1024, fp)){
    
    if (Line[0] == '#') {
      continue;
    }

    if (4 == sscanf(Line,"%4s, %lf, %lf, %lf",
		    StationName, &loc.Lat, &loc.Lon, &loc.Elev)){
      string IDCode(StationName);
      _map[IDCode] = loc;
      num++;
    }
    
  } // while
  
  fclose(fp);
  if (num == 0){
    cerr << "WARNING - StationLocate::StationLocate" << endl;
    cerr << "  No stations found in file: " << StationList << endl;
    return -1;
  }

  return 0;

}


// public methods

int StationLocate::getPos(const string &name,
			  double &lat, double &lon, double &elev) 
  
{
  
  string IDCode(name, 0, 4);
  
  StationMap::iterator ii = _map.find(IDCode);
  if (ii == _map.end()) {
    return -1;
  }
  
  lat =  (*ii).second.Lat;
  lon =  (*ii).second.Lon;
  elev = (*ii).second.Elev;
  
  return 0;

}
