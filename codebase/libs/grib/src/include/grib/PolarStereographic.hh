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
///////////////////////////////////////////////////
// PolarStereographic - polar stereographic grid
// Author:  Carl Drews
// Created:  October 2005
//
//////////////////////////////////////////////////

#ifndef _POLAR_STEREO_
#define _POLAR_STEREO_

#include "GribSection.hh"
#include "GDS.hh"


#include <cstdio>
#include <dataport/port_types.h>
using namespace std;

class PolarStereographic: public GDS {
public:

  PolarStereographic();
  virtual ~PolarStereographic();

  int unpack( ui08 *gdsPtr );

  int pack( ui08 *gdsPtr );

  inline double getXPts() { return _nx; };
  inline double getYPts() { return _ny; };

  inline double getDeltaX() { return _dx; };
  inline double getDeltaY() { return _dy; };

  inline double getStartLat() { return _la1; };
  inline double getStartLon() { return _lo1; };

  void setProjection(const Pjg &new_projection);

  inline void setStartLat(double newVal)
  { _la1 = newVal;
    _projection.setGridMins(_projection.getMinx(), newVal,
			    _projection.getMinz()); }

  virtual void print(FILE *stream) const;
  virtual void print(ostream &stream) const;

  virtual void setRegular(int numRows, int numColumns);

private:

  int _nx;
  int _ny;
  double _la1;
  double _lo1;
  ui08 _resolutionFlag;
  double _dx;
  double _dy;
  ui08 _projectionCenterFlag;
  ui08 _scanModeFlag;

  bool _earthSpherical;
  bool _directionIncsGiven;
  bool _uvRelToGrid;
};

#endif

