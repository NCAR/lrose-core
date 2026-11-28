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
// ModelGrid.h
//
// This class provides the pressure at any given flight level
// between -10 and 500.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
/////////////////////////////////////////////////////////////

#ifndef ModelGrid_H
#define ModelGrid_H

#include "Params.hh"
#include <dataport/port_types.h>
using namespace std;

class ModelGrid {
  
public:

  // constructor
  
  ModelGrid(char *prog_name, Params *params,
	    int model_nlat, int model_nlon,
	    fl32 **model_lat, fl32 **model_lon);
  
  // destructor
  
  ~ModelGrid();

  // find model location for a lat/lon
  
  int findModelLoc(double lat, double lon);

  // model grid locations

  int latIndex;
  int lonIndex;

  // interpolation factors for each corner

  double wtSW;
  double wtNW;
  double wtNE;
  double wtSE;
  
protected:
  
private:

  char *_progName;
  Params *_params;

  int _nLat;
  int _nLon;

  fl32 **_lat;
  fl32 **_lon;

  double _minLat;
  double _minLon;
  double _maxLat;
  double _maxLon;
  double _meanDLat;
  double _meanDLon;

};

#endif
