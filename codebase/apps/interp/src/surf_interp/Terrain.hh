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
// Terrain.hh
//
// Terrain class - handles the terrain.
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan  1999
//
///////////////////////////////////////////////////////////////

#ifndef Terrain_HH
#define Terrain_HH

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxFieldCode.hh>     
using namespace std;

class Terrain {
  
public:
  
  // constructor
  //
  // Reads a LATLON MDV file which has terrain in it.
  //
  Terrain(char *MDVFile);
  //

  // Allow the user to access the elevation data
  // by lat/lon.

  float Elevation(float lat, float lon);


  // destructor
  //
  // Frees the memory that was used to hold the Terrain.

  ~Terrain();


  // Public data.

  int  OutsideGrid; // true if a call to Terrain::Elevation has
  //                   a lat/lon outside the region covered by the
  //                   MDV file. In this case a value of 0.0 is returned.

  int Nx, Ny; // Public since I set these to -1 if the read fails.

protected:
  
private:
  
  float dx,dy; // In degrees.
  float LLlat, LLlon; // Lower left (SW) lat and lon.
  float URlat, URlon; // Upper right (NE).
  float *_Terrain; // Indexed as Terrain[i+nx*j] where
  // i is the lon index and j is the lat index.

  DsMdvx *mdv;

};

#endif



















