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
// PjgMath.cc
//
// Base class for PjgMath classes.
//
// The PjgMath classes are designed as a back-end math package for
// projective geometry.
//
// July 2008
//
//////////////////////////////////////////////////////////

#include <euclid/PjgMath.hh>
#include <cmath>
using namespace std;

#define TINY_ANGLE 1.e-4
#define TINY_DIST 1.e-2
#define TINY_DBL 1.e-10

////////////////////////
// Default constructor
//

PjgMath::PjgMath() :
        _proj_type(PjgTypes::PROJ_INVALID),
        _origin_lat(0.0),
        _origin_lon(0.0),
        _lat1(0.0),
        _lat2(0.0),
        _rotation(0.0),
        _tangent_lat(0.0),
        _tangent_lon(0.0),
        _pole_is_north (true),
        _central_scale(1.0),
        _persp_radius(6.0 * Pjg::EradKm),
        _offset_lat(0.0),
        _offset_lon(0.0),
        _false_northing(0.0),
        _false_easting(0.0)

{
  
}

///////////////
// Destructor

PjgMath::~PjgMath()

{
  return;
}

//////////////////////////////////////////////////
// create a new deep copy

PjgMath *PjgMath::newDeepCopy()
{

  // Create a new object, as a deep copy of the existing one
  // This copies at the derived class level

  const PjgPolarRadarMath *polarRadarMath =
    dynamic_cast< const PjgPolarRadarMath * >(this);
  if (polarRadarMath != 0)
    return new PjgPolarRadarMath(*polarRadarMath);
  
  const PjgAzimEquidistMath *azimEquidist =
    dynamic_cast< const PjgAzimEquidistMath* >(this);
  if (azimEquidist != 0)
    return new PjgAzimEquidistMath(*azimEquidist);
  
  const PjgAlbersMath *albers =
    dynamic_cast< const PjgAlbersMath* >(this);
  if (albers != 0)
    return new PjgAlbersMath(*albers);
    
  const PjgLambertConfMath *lambertConf =
    dynamic_cast< const PjgLambertConfMath* >(this);
  if (lambertConf != 0)
    return new PjgLambertConfMath(*lambertConf);
    
  const PjgLambertAzimMath *lambertAzim =
    dynamic_cast< const PjgLambertAzimMath* >(this);
  if (lambertAzim != 0)
    return new PjgLambertAzimMath(*lambertAzim);
    
  const PjgLatlonMath *latlon =
    dynamic_cast< const PjgLatlonMath* >(this);
  if (latlon != 0)
    return new PjgLatlonMath(*latlon);
  
  const PjgPolarStereoMath *polarStereo =
    dynamic_cast< const PjgPolarStereoMath* >(this);
  if (polarStereo != 0)
    return new PjgPolarStereoMath(*polarStereo);
    
  const PjgObliqueStereoMath *obliqueStereo =
    dynamic_cast< const PjgObliqueStereoMath* >(this);
  if (obliqueStereo != 0)
    return new PjgObliqueStereoMath(*obliqueStereo);
    
  const PjgMercatorMath *mercator =
    dynamic_cast< const PjgMercatorMath* >(this);
  if (mercator != 0)
    return new PjgMercatorMath(*mercator);
    
  const PjgTransMercatorMath *transMercator =
    dynamic_cast< const PjgTransMercatorMath* >(this);
  if (transMercator != 0)
    return new PjgTransMercatorMath(*transMercator);
    
  const PjgVertPerspMath *vertPersp =
    dynamic_cast< const PjgVertPerspMath* >(this);
  if (vertPersp != 0)
    return new PjgVertPerspMath(*vertPersp);
    
  return new PjgMath(*this);
    
}

//////////////////////////////////////////////////
// equality and inequality

bool PjgMath::operator==(const PjgMath &other) const
{
  
  if (_proj_type != other._proj_type) {
    return false;
  }
  if (_origin_lat != other._origin_lat) {
    return false;
  }
  if (_origin_lon != other._origin_lon) {
    return false;
  }
  if (_lat1 != other._lat1) {
    return false;
  }
  if (_lat2 != other._lat2) {
    return false;
  }
  if (_rotation != other._rotation) {
    return false;
  }
  if (_tangent_lat != other._tangent_lat) {
    return false;
  }
  if (_tangent_lon != other._tangent_lon) {
    return false;
  }
  if (_pole_is_north  != other._pole_is_north ) {
    return false;
  }
  if (_central_scale != other._central_scale) {
    return false;
  }
  if (_persp_radius != other._persp_radius) {
    return false;
  }
  if (_offset_lat != other._offset_lat) {
    return false;
  }
  if (_offset_lon != other._offset_lon) {
    return false;
  }
  if (_false_northing != other._false_northing) {
    return false;
  }
  if (_false_easting != other._false_easting) {
    return false;
  }
  
  return true;

}
  
bool PjgMath::operator!=(const PjgMath &other) const
{

  return !(*this == other);

}
  
/////////////////////////////////////////////
/// Set offset origin by specifying lat/lon.
/// Normally the offset origin and projection origin are co-located
/// This will compute false_northing and false_easting.
///   X = x_from_proj_origin + false_easting
///   Y = y_from_proj_origin + false_northing

void PjgMath::setOffsetOrigin(double offset_lat,
                              double offset_lon)
  
{
  
  _offset_lat = offset_lat;
  _offset_lon = offset_lon;

  if(_origin_lat == offset_lat && _origin_lon == offset_lon) {
    _false_easting = 0.0;
    _false_northing = 0.0;
    return;
  }
    
  double xx, yy;
  latlon2xy(_offset_lat, _offset_lon, xx, yy);
  _false_easting = -xx;
  _false_northing = -yy;

}

/////////////////////////////////////////////////////////////////////
/// Set offset origin by specifying false_northing and false_easting.
/// Normally the offset origin and projection origin are co-located
/// This will compute offset lat and lon, which is the point:
///   (x = -false_northing, y = -false_easting)

void PjgMath::setOffsetCoords(double false_northing,
                              double false_easting)

{

  if (false_easting == 0.0 && false_northing == 0.0) {
    _false_northing = 0;
    _false_easting = 0;
    _offset_lat = _origin_lat;
    _offset_lon = _origin_lon;
    return;
  }

  xy2latlon(-false_easting, -false_northing, _offset_lat, _offset_lon);

  _false_northing = false_northing;
  _false_easting = false_easting;
  
}
  
///////////////
// print object

void PjgMath::print(ostream &out) const

{

  out << "  Projection: "
      << PjgTypes::proj2string(_proj_type) << endl;

  printOffsetOrigin(out);

}

///////////////////////////////
// print object for debugging

void PjgMath::printDetails(ostream &out) const

{

  print(out);
  
}

///////////////////////
// print offset origin

void PjgMath::printOffsetOrigin(ostream &out) const

{
  
  if (_false_northing != 0.0 || _false_easting != 0.0) {
    out << "  Offset lon (deg): " << _offset_lon << endl;
    out << "  Offset lat (deg): " << _offset_lat << endl;
    out << "  False northing: " << _false_northing << endl;
    out << "  False easting: " << _false_easting << endl;
  }

}

///////////////////////
// LatLon conversions

/// convert lat/lon to x/y
/// z ignored except for PolarRadar projection
  
void PjgMath::latlon2xy(double lat, double lon,
                        double &x, double &y,
                        double z /* = -9999 */) const
  
{

  y = lat;
  x = lon;
  
}

/// convert x/y to lat/lon
/// z ignored except for PolarRadar projection

void PjgMath::xy2latlon(double x, double y,
                        double &lat, double &lon,
                        double z /* = -9999 */) const
  
{

  lat = y;
  lon = x;

}
     

///////////////////////////////////////////////////////////////////
// Condition longitude to be in same hemisphere as origin lon

void PjgMath::conditionLon2Origin(double &lon) const

{
  
  double diff = _origin_lon - lon;
  if (fabs(diff) > 180.0) {
    if (diff > 0) {
      lon += 360.0;
    } else {
      lon -= 360.0;
    }
  }
  
}

///////////////////////////////////////////////////////////////////
// Condition longitude to be in same hemisphere as reference lon

double PjgMath::conditionLon2Ref(double lon, double ref)

{
  
  double diff = ref - lon;
  if (fabs(diff) > 180.0) {
    if (diff > 0) {
      return lon + 360.0;
    } else {
      return lon - 360.0;
    }
  }
  return lon;
  
}

//////////////////////////////////////
// condition range

double PjgMath::conditionRange360(double a)
{
  while (a < 0.)
    a += 360.;
  while (a > 360.)
    a -= 360.;
  return a;
}

double PjgMath::conditionRange180(double a)
{
  while (a < -180.)
    a += 360.;
  while (a > 180.)
    a -= 360.;
  return a;
}

