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
// EquidistantCylind - Equidistant Cylindrical grid
//     definition - also called Latitude/Longitude
//     or Plate Carree projection grid
//
//////////////////////////////////////////////////

#ifndef _EQUIDISTANT_
#define _EQUIDISTANT_

#include "GribSection.hh"
#include "GDS.hh"


#include <cstdio>
#include <dataport/port_types.h>
using namespace std;

class EquidistantCylind: public GDS {
public:

  EquidistantCylind();
  virtual ~EquidistantCylind();

  int unpack( ui08 *gdsPtr );

  int pack( ui08 *gdsPtr );

  inline double getLatPts() { return _ni; };
  inline double getLonPts() { return _nj; };

  inline double getDeltaLon() { return _di; };
  inline double getDeltaLat() { return _dj; };

  inline double getStartLat() { return _la1; };
  inline double getStartLon() { return _lo1; };
  inline double getEndLat() { return _la2; };

  inline void setStartLat(double newVal)
  { _la1 = newVal;
    _projection.setGridMins(_projection.getMinx(), newVal,
			    _projection.getMinz()); }
  inline void setEndLat(double newVal) { _la2 = newVal; };

  virtual void print(FILE *stream) const;
  virtual void print(ostream &stream) const;

  virtual void setRegular(int numRows, int numColumns);

private:

  int _ni;
  int _nj;
  double _la1;
  double _lo1;
  ui08 _resolutionFlag;
  double _la2;
  double _lo2;
  double _di;
  double _dj;
  ui08 _scanModeFlag;
  bool _earthSpherical;
  bool _directionIncsGiven;
  bool _uvRelToGrid;

};

#endif

