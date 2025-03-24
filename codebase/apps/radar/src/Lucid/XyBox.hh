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
// XyBox.hh
//
// XY box limits object
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2025
//
///////////////////////////////////////////////////////////////

#ifndef XyBox_HH
#define XyBox_HH

#include <iostream>
using namespace std;

//////////////////////////////////////////////////////////////////////
/// ZOOM BOX CLASS

class XyBox {
  
public:

  /// constructor
  
  XyBox();

  XyBox(double minY, double maxY,
        double minX, double maxX);
  
  /// copy constructor
  
  XyBox(const XyBox &rhs);

  /// destructor

  virtual ~XyBox();
  
  /// assignment
  
  XyBox& operator=(const XyBox &rhs);
  
  // Check for equality

  bool operator== (const XyBox &rhs);
  bool operator!= (const XyBox &rhs);
  
  /// set limits
  
  void setLimits(double minY, double maxY,
                 double minX, double maxX);
  
  /// clear limits
  
  void clearLimits();
  
  /// print
  
  virtual void print(ostream &out) const;

  // get limits

  double getMinY() const { return _minY; }
  double getMaxY() const { return _maxY; }
  double getMinX() const { return _minX; }
  double getMaxX() const { return _maxX; }
  
  static const double defaultLimit;

protected:

  // data

  double _minY, _maxY;
  double _minX, _maxX;
  
  // methods
  
  void _init();
  XyBox & _copy(const XyBox &rhs); 

private:
  
};

#endif

