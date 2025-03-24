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
// WayPts.hh
//
// Way points for vert section
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2024
//
///////////////////////////////////////////////////////////////

#ifndef WayPts_HH
#define WayPts_HH

#include <string>
#include <vector>
#include <iostream>
using namespace std;

//////////////////////////////////////////////////////////////////////
/// Way points for vert section

class WayPts {
  
public:

  /// constructor
  
  WayPts();

  /// copy constructor
  
  WayPts(const WayPts &rhs);
  
  /// destructor

  virtual ~WayPts();
  
  /// assignment
  
  WayPts& operator=(const WayPts &rhs);
  
  // Check for equality
  
  bool operator==(const WayPts &rhs);
  
  /// add a point
  
  void addPoint(double lat, double lon);
  
  /// clear way points
  
  void clear();
  
  /// print
  
  virtual void print(ostream &out) const;

  // get methods

  size_t getNPoints() const { return _lat.size(); }
  const vector<double> &getLatList() const { return _lat; }
  const vector<double> &getLonList() const { return _lon; }
  void getPoint(size_t index, double &lat, double &lon) const;

protected:
  
  // data
  
  vector<double> _lat;
  vector<double> _lon;
  
  // methods
  
  WayPts & _copy(const WayPts &rhs); 

private:
  
};

#endif

