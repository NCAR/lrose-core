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
// /PjgMath.hh
//
// Base class for PjgMath classes.
//
// The PjgMath classes are designed as a back-end math package for
// projective geometry.
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2008
//
////////////////////////////////////////////////////////////////////

#ifndef PjgMath_hh
#define PjgMath_hh

#include <iostream>
#include <euclid/Pjg.hh>
#include <euclid/sincos.h>
using namespace std;

class PjgMath
{

public:

  ///////////////////////
  ///constructor
  
  PjgMath();
  
  ///////////////////////
  /// destructor

  virtual ~PjgMath();
  
  /// create a new deep copy, at derived class level
  
  PjgMath *newDeepCopy();

  /// equality

  bool operator==(const PjgMath &other) const;

  /// inequality

  bool operator!=(const PjgMath &other) const;

  /////////////////////////////////////////////////////////////////////
  /// Set offset origin by specifying lat/lon.
  /// Normally the offset origin and projection origin are co-located
  /// This will compute false_northing and false_easting.
  ///   X = x_from_proj_origin + false_easting
  ///   Y = y_from_proj_origin + false_northing

  virtual void setOffsetOrigin(double offset_lat,
                               double offset_lon);
  
  /////////////////////////////////////////////////////////////////////
  /// Set offset origin by specifying false_northing and false_easting.
  /// Normally the offset origin and projection origin are co-located
  /// This will compute offset lat and lon, which is the point:
  ///   (x = -false_northing, y = -false_easting)

  virtual void setOffsetCoords(double false_northing,
                               double false_easting);
  
  ////////////////////////////////////////////////
  // functions for XY to LatLon / LatLon to XY

  /// convert lat/lon to x/y
  /// z ignored except for PolarRadar projection
  
  virtual void latlon2xy(double lat, double lon,
                         double &x, double &y,
                         double z = -9999) const;
  
  /// convert x/y to lat/lon
  /// z ignored except for PolarRadar projection
  
  virtual void xy2latlon(double x, double y,
                         double &lat, double &lon,
                         double z = -9999) const;
  
  ////////////////////
  // Condition angles
  
  /// Condition longitude to be in same hemisphere as origin lon
  
  void conditionLon2Origin(double &lon) const;

  /// Condition longitude to be in same hemisphere as reference lon
  
  static double conditionLon2Ref(double lon, double ref_lon);
  
  /// condition angle so it is between 0 and 360
  
  static double conditionRange360(double a);
  
  /// condition angle so it is between -180 and 180
  
  static double conditionRange180(double a);
  
  /////////////////////
  /// print the object
  
  virtual void print(ostream &out) const; // basic params only

  /// print details

  virtual void printDetails(ostream &out) const; // includes derived params

  /// print offset origin
  
  void printOffsetOrigin(ostream &out) const;

  //////////////
  // get methods
  
  /// get latitude of projection origin
  
  inline double getOriginLat() const { return _origin_lat; }

  /// get longitude of projection origin
  
  inline double getOriginLon() const { return _origin_lon; }

  /// get central scale
  /// Polar Stereorgraphic, Transverse Mercator

  inline double getCentralScale() const { return _central_scale; }

  /// get standard latitude 1
  /// Lambert Conformal Conic, Albers Equal Area Conic

  inline double getLat1() const { return _lat1; }

  /// get standard latitude 2
  /// Lambert Conformal Conic, Albers Equal Area Conic

  inline double getLat2() const { return _lat2; }

  /// get rotation relative to TN (deg)
  /// optional on Azimuthal Equidistant (FLAT)

  inline double getRotation() const { return _rotation; }

  /// get pole - polar stereographic

  inline PjgTypes::pole_type_t getPole() const {
    return (_pole_is_north? PjgTypes::POLE_NORTH : PjgTypes::POLE_SOUTH);
  }
  
  /// get tangent latitude - stereographic

  inline double getTangentLat() const { return _tangent_lat; }

  /// get tangent longitude - polar stereographic

  inline double getTangentLon() const { return _tangent_lon; }

  /// get false northing and easting

  inline double getFalseNorthing() const { return _false_northing; }
  inline double getFalseEasting() const { return _false_easting; }

  /// get radius of perspective point - Vertical Perspective
  
  inline double getPerspRadius() const { return _persp_radius; }

protected:

  // data on the base class

  PjgTypes::proj_type_t _proj_type;

  // origin of the projection

  double _origin_lat; // lat of origin (deg)
  double _origin_lon; // lon of origin (deg)

  // conic projections

  double _lat1;  // standard lat1 for conic projections
  double _lat2;  // standard lat2 for conic projections
  
  // rotation - flat
  
  double _rotation; // rotation relative to TN - deg

  // stereographic

  double _tangent_lat; // lat of tangent point
  double _tangent_lon; // lon of tangent point

  // polar stereographic

  bool _pole_is_north; // which pole? (polar stereographic)

  // for polar stereographic and transverse mercator
  
  double _central_scale;
  
  // radius of perspective point - vertical perspective projection
  // km above center of earth

  double _persp_radius;

  // offset origin, if specified
  // normally the offset origin and projection origin are co-located
  // X and Y are 0.0 at this point
  // X = x_from_origin + false_easting
  // Y = y_from_origin + false_northing

  double _offset_lat; // lat of offset origin (deg)
  double _offset_lon; // lon of offset origin (deg)

  double _false_northing; // added to y coord computed from proj origin
  double _false_easting;  // added to x coord computed from proj origin

private:

};

#include <euclid/PjgAzimEquidistMath.hh>
#include <euclid/PjgPolarRadarMath.hh>
#include <euclid/PjgAlbersMath.hh>
#include <euclid/PjgLambertAzimMath.hh>
#include <euclid/PjgLambertConfMath.hh>
#include <euclid/PjgLatlonMath.hh>
#include <euclid/PjgPolarStereoMath.hh>
#include <euclid/PjgObliqueStereoMath.hh>
#include <euclid/PjgMercatorMath.hh>
#include <euclid/PjgTransMercatorMath.hh>
#include <euclid/PjgVertPerspMath.hh>

#endif











