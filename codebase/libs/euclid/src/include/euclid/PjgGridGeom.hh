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

  void setNx(size_t val) { _nx = val; }
  void setNy(size_t val) { _ny = val; }
  void setNz(size_t val) { _nz = val; }

  void setDx(double val) { _dx = val; }
  void setDy(double val) { _dy = val; }
  void setDz(double val) { _dz = val; }

  void setMinx(double val) { _minx = val; }
  void setMiny(double val) { _miny = val; }
  void setMinz(double val) { _minz = val; }

  void setIsLatLon(bool val) { _isLatLon = val; }
  void setProjType(PjgTypes::proj_type_t val) { _projType = val; }

  void setXY(size_t nx, size_t ny,
             double dx, double dy,
             double minx, double miny) {
    _nx = nx;
    _ny = ny;
    _dx = dx;
    _dy = dy;
    _minx = minx;
    _miny = miny;
  }

  void setXYZ(size_t nx, size_t ny, size_t nz,
              double dx, double dy, double dz,
              double minx, double miny, double minz) {
    _nx = nx;
    _ny = ny;
    _nz = nz;
    _dx = dx;
    _dy = dy;
    _dz = dz;
    _minx = minx;
    _miny = miny;
    _minz = minz;
  }

  /// get methods
  /// units are km

  size_t nx() const { return _nx; }
  size_t ny() const { return _ny; }
  size_t nz() const { return _nz; }

  double dx() const { return _dx; }
  double dy() const { return _dy; }
  double dz() const { return _dz; }

  double minx() const { return _minx; }
  double miny() const { return _miny; }
  double minz() const { return _minz; }

  bool isLatLon() const { return _isLatLon; }
  PjgTypes::proj_type_t projType() const { return _projType; }
  
  // Print details of grid
  
  void print(ostream &out);

protected:

  bool _isLatLon;
  PjgTypes::proj_type_t _projType;
  
  size_t _nx, _ny, _nz;
  double _dx, _dy, _dz;
  double _minx, _miny, _minz;

private:

};

#endif











