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
// ReadDEM.hh
//
// Reads a DEM header file into a struct.
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb  1999
//
///////////////////////////////////////////////////////////////

#ifndef ReadDEM_HH
#define ReadDEM_HH

#include <cstdio>
using namespace std;

class ReadDEM {
  
public:
  
  // constructor
  // Reads the header into a struct.

  ReadDEM(char *BaseName, int debug);


  // destructor frees up.
  ~ReadDEM();


  // Allow the user to access the elevation data
  // by lat/lon. Return bad if point is inside grid but
  // bad. Return 0 if point is outside grid (and set
  // OutsideGrid).
  float Elevation(float lat, float lon, float bad);


  // Data.
  int Error;
  int SwapBytes;

  unsigned long NumRows, NumCols;

  int NoData;
  float URLon, URLat, LLLat, LLLon;

  float dx,dy;
  int OutsideGrid;

  FILE *ifp;

};

#endif


