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
// LatLonBox.hh
//
// Zoom limits object
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2024
//
///////////////////////////////////////////////////////////////

#ifndef LatLonBox_HH
#define LatLonBox_HH

#include <iostream>
using namespace std;

//////////////////////////////////////////////////////////////////////
/// ZOOM BOX CLASS

class LatLonBox {
  
public:

  /// constructor
  
  LatLonBox();

  LatLonBox(double minLat, double maxLat,
            double minLon, double maxLon);

  /// copy constructor
  
  LatLonBox(const LatLonBox &rhs);

  /// destructor

  virtual ~LatLonBox();
  
  /// assignment
  
  LatLonBox& operator=(const LatLonBox &rhs);
  
  // Check for equality

  bool operator==(const LatLonBox &rhs);
  
  /// set limits
  
  void setLimits(double minLat, double maxLat,
                 double minLon, double maxLon);
  
  /// clear limits
  
  void clearLimits();
  
  /// print
  
  virtual void print(ostream &out) const;

  // get limits

  double getMinLat() const { return _minLat; }
  double getMaxLat() const { return _maxLat; }
  double getMinLon() const { return _minLon; }
  double getMaxLon() const { return _maxLon; }

protected:
  
  // data

  double _minLat, _maxLat;
  double _minLon, _maxLon;
  
  // methods
  
  void _init();
  LatLonBox & _copy(const LatLonBox &rhs); 

private:
  
};

#endif

