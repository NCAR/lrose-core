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
// ZoomBox.hh
//
// Zoom limits object
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2024
//
///////////////////////////////////////////////////////////////

#ifndef ZoomBox_HH
#define ZoomBox_HH

#include <string>
#include <vector>
#include <iostream>
#include <Radx/Radx.hh>
using namespace std;

//////////////////////////////////////////////////////////////////////
/// ZOOM BOX CLASS

class ZoomBox {
  
public:

  /// constructor
  
  ZoomBox();

  ZoomBox(double minLat, double maxLat,
          double minLon, double maxLon);

  /// copy constructor
  
  ZoomBox(const ZoomBox &rhs);

  /// destructor

  virtual ~ZoomBox();
  
  /// assignment
  
  ZoomBox& operator=(const ZoomBox &rhs);
  
  // Check for equality

  bool operator==(const ZoomBox &rhs);
  
  /// set limits
  
  void setLatLimits(double minLat, double maxLat);
  void setLonLimits(double minLon, double maxLon);
  
  /// clear imits
  
  void clearLimits();
  
  /// print
  
  virtual void print(ostream &out) const;

protected:
  
  // data

  double _minLat, _maxLat;
  double _minLon, _maxLon;
  
  // methods
  
  void _init();
  ZoomBox & _copy(const ZoomBox &rhs); 

private:
  
};

#endif

