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
// MM5Grid.h
//
// This class is responsible for computations concerning the 
// model grid localtions.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
/////////////////////////////////////////////////////////////

#ifndef MM5Grid_H
#define MM5Grid_H

#include <dataport/port_types.h>
#include <string>
#include <mm5/MM5Data.hh>
#include <Mdv/MdvxProj.hh>
using namespace std;


class MM5Grid {
  
public:

  // constructor
  
  MM5Grid(const string &prog_name, bool debug,
	  int model_nlat, int model_nlon,
	  fl32 **model_lat, fl32 **model_lon);
  
  MM5Grid(const string &prog_name, bool debug, MM5Data &inData, double central_scale_factor = 1.0);
  
  // destructor
  
  ~MM5Grid();

  // find model location for a lat/lon
  
  int findModelLoc(double lat, double lon);

  int getGridIndices(double lat, double lon);

  // Set for non-interpolation at the given point

  int setNonInterp(int ilat, int ilon);

  // model grid locations

  int latIndex;
  int lonIndex;

  // interpolation factors for each corner

  double wtSW;
  double wtNW;
  double wtNE;
  double wtSE;

  MdvxProj mm5proj;
  
protected:
  
private:

  const string &_progName;
  bool _debug;

  int _nLat;
  int _nLon; 

  int _nx;  
  int _ny; 

  fl32 **_lat;
  fl32 **_lon;

  double _minLat;
  double _minLon;
  double _maxLat;
  double _maxLon;
  double _meanDLat;
  double _meanDLon;
  double _midLon;

  double _minx;
  double _miny;
  double _dx;
  double _dy;

  double _conditionLon(double lon);

};

#endif

