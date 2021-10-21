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
////////////////////////////////////////////////////////////////////
// PjgGridGeom.hh
//
// Class to represent grid geometry for PJG classes.
//
// Mike Dixon, EOL, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2021
//
////////////////////////////////////////////////////////////////////

#ifndef PjgGridGeom_hh
#define PjgGridGeom_hh

#include <euclid/PjgTypes.hh>
#include <ostream>
#include <vector>

class PjgGridGeom
{

public:
  
  ///////////////////////
  ///constructor
  
  PjgGridGeom();

  ///////////////////////
  /// destructor

  virtual ~PjgGridGeom();
  
  /// set methods
  /// units are km

  inline void setNx(size_t val) { _nx = val; }
  inline void setNy(size_t val) { _ny = val; }

  inline void setDx(double val) { _dx = val; }
  inline void setDy(double val) { _dy = val; }
  
  inline void setMinx(double val) { _minx = val; }
  inline void setMiny(double val) { _miny = val; }
  
  inline void setZKm(const vector<double> &val) { _zKm = val; }
  inline void addZKm(double val) { _zKm.push_back(val); }
  inline void clearZKm() { _zKm.clear(); }

  inline void setIsLatLon(bool val) { _isLatLon = val; }
  inline void setProjType(PjgTypes::proj_type_t val) { _projType = val; }
  
  /// get methods
  /// units are km

  inline size_t nx() const { return _nx; }
  inline size_t ny() const { return _ny; }
  inline size_t nz() const { return _zKm.size(); }
  
  inline double dx() const { return _dx; }
  inline double dy() const { return _dy; }
  inline double meanDz() const {
    if (_zKm.size() > 1) {
      return (_zKm[_zKm.size()-1] - _zKm[0]) / (_zKm.size() - 1);
    } else {
      return 0;
    }
  }
  
  inline double minx() const { return _minx; }
  inline double miny() const { return _miny; }
  inline double minz() const {
    if (_zKm.size() > 0) {
      return _zKm[0];
    } else {
      return 0;
    }
  }
  inline const vector<double> &zKm() const { return _zKm; }
  inline double zKm(int iz) const { return _zKm[iz]; }
  double dzKm(int iz) const;

  inline bool isLatLon() const { return _isLatLon; }
  inline PjgTypes::proj_type_t projType() const { return _projType; }
  
  // Print details of grid
  
  void print(ostream &out) const;

protected:

  bool _isLatLon;
  PjgTypes::proj_type_t _projType;
  
  size_t _nx, _ny;
  double _dx, _dy;
  double _minx, _miny;

  vector<double> _zKm;

private:

};

#endif











