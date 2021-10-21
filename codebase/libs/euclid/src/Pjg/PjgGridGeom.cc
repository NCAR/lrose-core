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
//////////////////////////////////////////////////////////
// PjgGridGeom.cc
//
// Class to represent grid geometry for PJG classes.
//
// Mike Dixon, EOL, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2021
//
//////////////////////////////////////////////////////////

#include <euclid/PjgGridGeom.hh>

////////////////////////
// Default constructor
//

PjgGridGeom::PjgGridGeom()

{
  _nx = _ny = 0;
  _dx = _dy = 0.0;
  _minx = _miny = 0.0;
  _isLatLon = true;
  _projType = PjgTypes::PROJ_LATLON;
}

///////////////
// Destructor

PjgGridGeom::~PjgGridGeom()

{
}

////////////////////////
// Print details of grid

void PjgGridGeom::print(ostream &out) const

{
  out << "============= PjgGridGeom =============" << endl;
  out << "  nx, ny, nz: "
      << _nx << ", " << _ny << ", " << _zKm.size() << endl;
  out << "  dx, dy, meanDz: "
      << _dx << ", " << _dy << ", " << meanDz() << endl;
  out << "  minx, miny, minz: "
      << _minx << ", " << _miny << ", " << minz() << endl;
  out << "  zKm: ";
  for (size_t ii = 0; ii < _zKm.size(); ii++) {
    out << _zKm[ii];
    if (ii != _zKm.size() - 1) {
      out << ", ";
    }
  }
  out << endl;
  out << "  isLatLon: " << (_isLatLon? "Y":"N") << endl;
  out << "  projType: " << PjgTypes::proj2string(_projType) << endl;
  out << "=======================================" << endl;
}

////////////////////////
// Compute delta z in km

double PjgGridGeom::dzKm(int iz) const

{
  
  if (iz < 0 || iz > (int) _zKm.size() - 1) {
    return 0;
  }

  if (iz == 0) {
    if (_zKm.size() < 2) {
      return 0;
    } else {
      return (_zKm[1] - _zKm[0]);
    }
  }

  if (iz == (int) _zKm.size() - 1) {
    if (_zKm.size() < 2) {
      return 0;
    } else {
      return (_zKm[_zKm.size() - 1] - _zKm[_zKm.size() - 2]);
    }
  }

  return (_zKm[iz + 1] - _zKm[iz - 1]) / 2.0;

}



