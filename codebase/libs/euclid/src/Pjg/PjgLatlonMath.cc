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
// PjgLatlonMath.cc
//
// Low-level math for latlon projection.
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2008
//
//////////////////////////////////////////////////////////

#include <euclid/PjgMath.hh>
using namespace std;

////////////////////////
// Constructor

PjgLatlonMath::PjgLatlonMath(double origin_lon /* = 0.0 */) :
        PjgMath()
  
{
  _proj_type = PjgTypes::PROJ_LATLON;
  _origin_lon = origin_lon;
}

/////////////////////////////////////////////
/// Set offset origin by specifying lat/lon.
/// Not applicable to this projection.

void PjgLatlonMath::setOffsetOrigin(double offset_lat,
                                    double offset_lon)
  
{

}

/////////////////////////////////////////////////////////////////////
/// Set offset origin by specifying false_northing and false_easting.
/// Not applicable to this projection.

void PjgLatlonMath::setOffsetCoords(double false_northing,
                                    double false_easting)
  
{

}
  
///////////////
// print object

void PjgLatlonMath::print(ostream &out) const

{

  out << "  Projection: "
      << PjgTypes::proj2string(_proj_type) << endl;

  if (_origin_lon != 0.0) {
    out << "  origin_lon (deg): " << _origin_lon << endl;
  }

}

///////////////////////////////
// print object for debugging

void PjgLatlonMath::printDetails(ostream &out) const

{
  
  print(out);

}

///////////////////////
// LatLon conversions

// x = lon and y = lat, z is ignored

void PjgLatlonMath::latlon2xy(double lat, double lon,
                              double &x, double &y,
                              double z /* = -9999 */) const
  
{

  y = lat;
  x = lon;
  
}

// lon = x and lat = y, z is ignored

void PjgLatlonMath::xy2latlon(double x, double y,
                              double &lat, double &lon,
                              double z /* = -9999 */) const
  
{

  lat = y;
  lon = x;

}
     
