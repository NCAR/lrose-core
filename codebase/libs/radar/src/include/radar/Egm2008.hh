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
// Egm2008.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2019
//
///////////////////////////////////////////////////////////////
//
// Geoid height correction for EGM 2008
//
// See:
// https://earth-info.nga.mil/GandG/wgs84/gravitymod/egm2008/egm08_wgs84.html
//
////////////////////////////////////////////////////////////////

#ifndef Egm2008_H
#define Egm2008_H

#include <string>
#include <vector>
#include <dataport/port_types.h>

using namespace std;

class Egm2008 {
  
public:

  // constructor
  
  Egm2008();
  
  // destructor
  
  ~Egm2008();
  
  // read in the Geoid corrections file
  // for 2008 version of EGM
  
  int readGeoid(const string &path);

  // get the closest geoid correction in M for a given lat/lon
  
  double getClosestGeoidM(double lat, double lon) const;

  // get the interpolated geoid correction in M for a given lat/lon
  
  double getInterpGeoidM(double lat, double lon) const;

  // debugging

  void setDebug() { _debug = true; }
  void setVerbose() { _verbose = true; }

  // grid methods

  inline int getLatIndexClosest(double lat) const {
    int ilat = (int) (((90.0 - lat) / _gridRes) + 0.5);
    if (ilat < 0) {
      ilat = 0;
    } else if (ilat > _nLat - 1) {
      ilat = _nLat - 1;
    }
    return ilat;
  }

  inline int getLonIndexClosest(double lon) const {
    if (lon < 0) {
      lon += 360.0;
    }
    int ilon = (int) ((lon / _gridRes) + 0.5);
    if (ilon > _nLon - 1) {
      ilon = 0;
    }
    return ilon;
  }

  inline int getLatIndexAbove(double lat) const {
    int ilat = (int) ((90.0 - lat) / _gridRes);
    if (ilat < 0) {
      ilat = 0;
    } else if (ilat > _nLat - 1) {
      ilat = _nLat - 1;
    }
    return ilat;
  }

  inline int getLatIndexBelow(double lat) const {
    int ilatAbove = getLatIndexAbove(lat);
    int ilatBelow = ilatAbove + 1;
    if (ilatBelow > _nLat - 1) {
      ilatBelow = _nLat - 1;
    }
    return ilatBelow;
  }
  
  inline int getLonIndexBelow(double lon) const {
    if (lon < 0) {
      lon += 360.0;
    }
    int ilon = (int) (lon / _gridRes);
    if (ilon > _nLon - 1) {
      ilon = 0;
    }
    return ilon;
  }

  inline int getLonIndexAbove(double lon) const {
    int ilonBelow = getLonIndexBelow(lon);
    int ilonAbove = ilonBelow + 1;
    if (ilonAbove > _nLon - 1) {
      ilonAbove = 0;
    }
    return ilonAbove;
  }

  inline double getLat(int latIndex) const {
    return 90.0 - ((double) latIndex * _gridRes);
  }

  inline double getLon(int lonIndex) const {
    return ((double) lonIndex * _gridRes);
  }

protected:
private:
  
  bool _debug;
  bool _verbose;

  fl32 *_geoidM;

  int _nPtsPerDeg;
  double _gridRes;
  int _nLat;
  int _nLon;
  int _nPoints;

};


#endif
