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
// MdvxProj.cc
//
// Class for handling Mdvx projective geometry computations
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// September 1999
//
//////////////////////////////////////////////////////////
//
// See <Mdv/MdvxProj.hh> for details.
//
///////////////////////////////////////////////////////////

#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxRadar.hh>
#include <toolsa/mem.h>
#include <toolsa/pjg.h>
#include <toolsa/TaStr.hh>
#include <toolsa/toolsa_macros.h>
#include <math.h>
using namespace std;

#define TINY_ANGLE 1.e-4
#define TINY_DIST 1.e-2
#define TINY_FLOAT 1.e-10

////////////////////////////////////////////////////////////////////////
// Default constructor
//

MdvxProj::MdvxProj()
  
{

  _math = NULL;
  _initToDefaults();

}

/////////////////////////////////////////////////
// construct from first field of Mdvx object
//
  
MdvxProj::MdvxProj(const Mdvx &mdvx)
  
{

  _math = NULL;
  init(mdvx);

}

////////////////////////////////////////////////////////////////////////
// Construct from master and field headers
//

MdvxProj::MdvxProj(const Mdvx::master_header_t &mhdr,
		   const Mdvx::field_header_t &fhdr)
  
{

  _math = NULL;
  init(mhdr, fhdr);

}

////////////////////////////////////////////////////////////////////////
// Construct from field header
//
// Sensor position will not be filled in, since this is only available
// from the master header.

MdvxProj::MdvxProj(const Mdvx::field_header_t &fhdr)
  
{

  _math = NULL;
  init(fhdr);

}

////////////////////////////////////////////////////////////////////////
// Construct from coord struct
//

MdvxProj::MdvxProj(const Mdvx::coord_t &coord)
  
{

  _math = NULL;
  init(coord);

}

/////////////////////////////
// Copy constructor
//

MdvxProj::MdvxProj(const MdvxProj &rhs)
     
{

  _math = NULL;
  if (this != &rhs) {
    // Zero out the _coords structure before the _copy so that the
    // == operator will definitely work.
    MEM_zero(_coord);
    _copy(rhs);
  }

}

/////////////////////////////
// Destructor

MdvxProj::~MdvxProj()

{
  if (_math != NULL) {
    delete _math;
  }
}

/////////////////////////////
// Assignment
//

MdvxProj &MdvxProj::operator=(const MdvxProj &rhs)

{
  return _copy(rhs);
}

///////////////////////////////////////////////////
// equality operator

bool MdvxProj::operator==(const MdvxProj &other) const {

  if (_proj_type != other._proj_type) {
    return false;
  }
  if (_math != NULL && other._math != NULL &&
      *_math != *other._math) {
    return false;
  }
  if (_coord.nx != other._coord.nx) {
    return false;
  }
  if (_coord.ny != other._coord.ny) {
    return false;
  }
  if (_coord.dx != other._coord.dx) {
    return false;
  }
  if (_coord.dy != other._coord.dy) {
    return false;
  }
  if (_coord.minx != other._coord.minx) {
    return false;
  }
  if (_coord.miny != other._coord.miny) {
    return false;
  }
  return true;

}

// int equality operator

bool MdvxProj::operator!=(const MdvxProj &other) const {
  return !(*this == other);
}
  
//////////////////////////////////////////////////
// _copy - used by copy constructor and operator =
//

MdvxProj &MdvxProj::_copy(const MdvxProj &rhs)

{

  // check for self
  
  if (&rhs == this) {
    return *this;
  }

  // copy math object
  
  if (_math != NULL) {
    delete _math;
  }
  if (rhs._math != NULL) {
    _math = rhs._math->newDeepCopy();
  }

  // copy other members

  _coord = rhs._coord;
  _proj_type = rhs._proj_type;
  _origin_lat = rhs._origin_lat;
  _origin_lon = rhs._origin_lon;
  _condition_lon = rhs._condition_lon;
  _reference_lon = rhs._reference_lon;

  return *this;
  
}

/////////////////////////////////////
// check that projection is supported
//
// Return true is proj type is supported, false otherwise
//
// Useful for checking if the constructor was given data which
// can be used by this class.

bool MdvxProj::supported()

{
 
  if (_proj_type == Mdvx::PROJ_FLAT ||
      _proj_type == Mdvx::PROJ_LATLON ||
      _proj_type == Mdvx::PROJ_LAMBERT_CONF ||
      _proj_type == Mdvx::PROJ_LAMBERT_AZIM ||
      _proj_type == Mdvx::PROJ_POLAR_RADAR ||
      _proj_type == Mdvx::PROJ_POLAR_STEREO ||
      _proj_type == Mdvx::PROJ_OBLIQUE_STEREO ||
      _proj_type == Mdvx::PROJ_MERCATOR ||
      _proj_type == Mdvx::PROJ_TRANS_MERCATOR ||
      _proj_type == Mdvx::PROJ_ALBERS ||
      _proj_type == Mdvx::PROJ_VERT_PERSP) {
    return true;
  } else {
    return false;
  }
}

/////////////////////////////////////////////////
// initialize from first field of Mdvx object
//

void MdvxProj::init(const Mdvx &mdvx)
  
{

  _clear();

  // load from first field if exists

  MdvxField *fld0 = mdvx.getField(0);
  if (fld0) {
    init(fld0->getFieldHeader());
  } else {
    _initToDefaults();
  }

  // update from master header

  _loadCoordFromMasterHdr(mdvx.getMasterHeader());

  // set sensor location if radar available

  MdvxRadar mdvxRadar;
  if (mdvxRadar.loadFromMdvx(mdvx) == 0) {
    DsRadarParams radar = mdvxRadar.getRadarParams();
    setSensorPosn(radar.latitude, radar.longitude, radar.altitude);
  }

}

/////////////////////////////////////////////////
// initialize from Mdvx master and field headers
//

void MdvxProj::init(const Mdvx::master_header_t &mhdr,
		    const Mdvx::field_header_t &fhdr)
  
{

  _clear();
  init(fhdr);
  _loadCoordFromMasterHdr(mhdr);

}

////////////////////////////////////
// initialize from Mdvx field header
//
// Sensor position will not be filled in, since this is only available
// from the master header.


void MdvxProj::init(const Mdvx::field_header_t &fhdr)
  
{

  _clear();

  // first, load up coord struct from field header

  _loadCoordFromFieldHdr(fhdr);

  // now initialize from the _coord struct

  _initFromCoords();

}

/////////////////////////////////
// initialize from coord struct
//

void MdvxProj::init(const Mdvx::coord_t &coord)

{

  _clear();

  // first, copy coord

  _coord = coord;

  // now initialize from the _coord struct

  _initFromCoords();

}

/////////////////////////////////
// initialize from proj string
// i.e. proj 4 etc.
//

void MdvxProj::initFromProjStr(const string &projStr)

{
  
  _clear();

  // tokenize the string

  vector<string> toks;
  TaStr::tokenize(projStr, " ", toks);

  // determine the projection type

  Mdvx::projection_type_t ptype = Mdvx::PROJ_UNKNOWN;
  for (size_t itok = 0; itok < toks.size(); itok++) {
    string tok = toks[itok];
    if (tok.find("+proj") == string::npos) {
      continue;
    }
    if (tok.find("=aea") != string::npos) {
      ptype = Mdvx::PROJ_ALBERS;
    } else if (tok.find("=aeqd") != string::npos) {
      ptype = Mdvx::PROJ_FLAT;
    } else if (tok.find("=lcc") != string::npos) {
      ptype = Mdvx::PROJ_LAMBERT_CONF;
    } else if (tok.find("=laea") != string::npos) {
      ptype = Mdvx::PROJ_LAMBERT_AZIM;
    } else if (tok.find("=merc") != string::npos) {
      ptype = Mdvx::PROJ_MERCATOR;
    } else if (tok.find("=tmerc") != string::npos) {
      ptype = Mdvx::PROJ_TRANS_MERCATOR;
    } else if (tok.find("=ups") != string::npos) {
      ptype = Mdvx::PROJ_POLAR_STEREO;
    } else if (tok.find("=stere") != string::npos) {
      ptype = Mdvx::PROJ_OBLIQUE_STEREO;
    } else if (tok.find("=pconic") != string::npos) {
      ptype = Mdvx::PROJ_VERT_PERSP;
    } else if ((tok.find("=latlon") != string::npos) ||
               (tok.find("=lonlat") != string::npos) ||
               (tok.find("=latlong") != string::npos)) {
      ptype = Mdvx::PROJ_LATLON;
    } 
  } // itok

  if (ptype == Mdvx::PROJ_UNKNOWN) {
    cerr << "WARNING - MdvzProj::init from proj string" << endl;
    cerr << "  projStr: " << projStr << endl;
    cerr << "  proj type not supported" << endl;
    cerr << "  assuming LATLON projection" << endl;
    ptype = Mdvx::PROJ_LATLON;
  }

  // determine the projection parameters

  double lon_0 = 0.0;
  double lat_0 = 0.0;
  double lat_1 = 0.0;
  double lat_2 = 0.0;
  double x_0 = 0.0;
  double y_0 = 0.0;
  double k_0 = 1.0;
  bool isSouth = false;
  double perspRadius = 6.0 * Pjg::EradKm;
  bool unitsAreM = false;
  
  char cc;
  for (size_t itok = 0; itok < toks.size(); itok++) {
    string tok = toks[itok];
    if (tok.find("+lon_0") != string::npos) {
      sscanf(tok.c_str(), "+lon_0%1c%lg", &cc, &lon_0);
    }
    if (tok.find("+lat_0") != string::npos) {
      sscanf(tok.c_str(), "+lat_0%1c%lg", &cc, &lat_0);
    }
    if (tok.find("+lat_1") != string::npos) {
      sscanf(tok.c_str(), "+lat_1%1c%lg", &cc, &lat_1);
    }
    if (tok.find("+lat_2") != string::npos) {
      sscanf(tok.c_str(), "+lat_2%1c%lg", &cc, &lat_2);
    }
    if (tok.find("+x_0") != string::npos) {
      sscanf(tok.c_str(), "+x_0%1c%lg", &cc, &x_0);
    }
    if (tok.find("+y_0") != string::npos) {
      sscanf(tok.c_str(), "+y_0%1c%lg", &cc, &y_0);
    }
    if (tok.find("+k_0") != string::npos) {
      sscanf(tok.c_str(), "+k_0%1c%lg", &cc, &k_0);
    }
    if (tok.find("+south") != string::npos) {
      isSouth = true;
    }
    if ((tok.find("+units=m") != string::npos) ||
        (tok.find("+units#m") != string::npos)) {
      unitsAreM = true;
    }
  }

  if (unitsAreM) {
    x_0 /= 1000.0;
    y_0 /= 1000.0;
  }

  if (ptype == Mdvx::PROJ_LATLON) {
    initLatlon(lon_0);
  } else if (ptype == Mdvx::PROJ_ALBERS) {
    initAlbers(lat_0, lon_0, lat_1, lat_2);
  } else if (ptype == Mdvx::PROJ_FLAT) {
    initAzimEquiDist(lat_0, lon_0, 0.0);
  } else if (ptype == Mdvx::PROJ_LAMBERT_CONF) {
    initLambertConf(lat_0, lon_0, lat_1, lat_2);
  } else if (ptype == Mdvx::PROJ_LAMBERT_AZIM) {
    initLambertAzim(lat_0, lon_0);
  } else if (ptype == Mdvx::PROJ_MERCATOR) {
    initMercator(lat_0, lon_0);
  } else if (ptype == Mdvx::PROJ_TRANS_MERCATOR) {
    initTransMercator(lat_0, lon_0, k_0);
  } else if (ptype == Mdvx::PROJ_POLAR_STEREO) {
    if (isSouth) {
      initStereographic(lon_0, Mdvx::POLE_SOUTH, k_0);
    } else {
      initStereographic(lon_0, Mdvx::POLE_NORTH, k_0);
    }
  } else if (ptype == Mdvx::PROJ_OBLIQUE_STEREO) {
    initStereographic(lat_0, lon_0, k_0);
  } else if (ptype == Mdvx::PROJ_VERT_PERSP) {
    initVertPersp(lat_0, lon_0, perspRadius);
  }

  // if (x_0 != 0.0 || y_0 != 0.0) {
  //   setOffsetCoords(y_0, x_0);
  // }

}

////////////////////////////////
// initialize latlon projection
//

void MdvxProj::initLatlon(double origin_lon /* = 0.0 */)

{
  
  MEM_zero(_coord.proj_params);

  _coord.proj_type = Mdvx::PROJ_LATLON;
  _coord.proj_origin_lon = origin_lon;

  _initFromCoords();

}

///////////////////////////////////////
// initialize Azimuthal Equidistant projection
// This is the same as the flat projection


void MdvxProj::initAzimEquiDist(double origin_lat,
				double origin_lon,
				double rotation)
  
{

  MEM_zero(_coord.proj_params);

  _coord.proj_type = Mdvx::PROJ_FLAT;
  _coord.proj_origin_lat = origin_lat;
  _coord.proj_origin_lon = origin_lon;
  _coord.proj_params.flat.rotation = rotation;

  _initFromCoords();
  
}

//////////////////////////////////////////////////////////
// initialize lambert conformal projection with two lats.

void MdvxProj::initLambertConf(double origin_lat,
			       double origin_lon,
			       double lat1,
			       double lat2)
  
{

  MEM_zero(_coord.proj_params);

  _coord.proj_type = Mdvx::PROJ_LAMBERT_CONF;
  _coord.proj_origin_lat = origin_lat;
  _coord.proj_origin_lon = origin_lon;
  _coord.proj_params.lc2.lat1 = lat1;
  _coord.proj_params.lc2.lat2 = lat2;

  _initFromCoords();

}

///////////////////////////////////////
// initialize polar radar projection
// Uses Azimuthal Equidistant

void MdvxProj::initPolarRadar(double origin_lat,
			      double origin_lon)
  
{

  MEM_zero(_coord.proj_params);
  
  _coord.proj_type = Mdvx::PROJ_POLAR_RADAR;
  _coord.proj_origin_lat = origin_lat;
  _coord.proj_origin_lon = origin_lon;

  _initFromCoords();

}


/////////////////////////////////////////////
// initialize polar stereographic projection

void MdvxProj::initPolarStereo
  (double tangent_lon, 
   Mdvx::pole_type_t poleType, /* = Mdvx::POLE_NORTH */
   double central_scale /* = 1.0 */)
  
{

  MEM_zero(_coord.proj_params);
  
  _coord.proj_type = Mdvx::PROJ_POLAR_STEREO;
  _coord.proj_origin_lon = tangent_lon;
  _coord.proj_params.ps.tan_lon = tangent_lon;
  if (poleType == Mdvx::POLE_NORTH) {
    _coord.proj_origin_lat = 90.0;
    _coord.proj_params.ps.pole_type = 0;
  } else {
    _coord.proj_origin_lat = -90.0;
    _coord.proj_params.ps.pole_type = 1;
  }
  _coord.proj_params.ps.central_scale = central_scale;

  _initFromCoords();

}

// deprecated initializer
// Instead, use initPolarStereo() above,
// followed by setOffsetOrigin()

void MdvxProj::initPolarStereo
  (double origin_lat,
   double origin_lon,
   double tangent_lon, 
   Mdvx::pole_type_t poleType, /* = Mdvx::POLE_NORTH */
   double central_scale /* = 1.0 */)
  
{
  
  initPolarStereo(tangent_lon, poleType, central_scale);
  setOffsetOrigin(origin_lat, origin_lon);

}

/////////////////////////////////////////////
// initialize oblique stereographic projection

void MdvxProj::initStereographic(double tangent_lat, 
				 double tangent_lon,
                                 double central_scale /* = 1.0 */)
  
{

  MEM_zero(_coord.proj_params);

  _coord.proj_type = Mdvx::PROJ_OBLIQUE_STEREO;
  _coord.proj_origin_lat = tangent_lat;
  _coord.proj_origin_lon = tangent_lon;
  _coord.proj_params.os.tan_lat = tangent_lat;
  _coord.proj_params.os.tan_lon = tangent_lon;

  if (central_scale == 0) {
    _coord.proj_params.os.central_scale = 1.0;
  } else {
    _coord.proj_params.os.central_scale = central_scale;
  }

  _initFromCoords();

}

// deprecated initializer

void MdvxProj::initObliqueStereo(double origin_lat,
				 double origin_lon,
				 double tangent_lat, 
				 double tangent_lon,
                                 double central_scale /*= 1.0 */)
  
{

  MEM_zero(_coord.proj_params);
  
  _coord.proj_type = Mdvx::PROJ_OBLIQUE_STEREO;
  _coord.proj_origin_lat = origin_lat;
  _coord.proj_origin_lon = origin_lon;
  _coord.proj_params.os.tan_lat = tangent_lat;
  _coord.proj_params.os.tan_lon = tangent_lon;
  _coord.proj_params.os.central_scale = central_scale;

  _initFromCoords();

}

////////////////////////////////
// initialize Mercator projection
//

void MdvxProj::initMercator(double origin_lat,
                            double origin_lon)
{
  
  MEM_zero(_coord.proj_params);

  _coord.proj_type = Mdvx::PROJ_MERCATOR;
  _coord.proj_origin_lat = origin_lat;
  _coord.proj_origin_lon = origin_lon;

  _initFromCoords();

}

///////////////////////////////////////////
// initialize Transverse Mercator projection
//

void MdvxProj::initTransMercator(double origin_lat,
                                 double origin_lon,
                                 double central_scale)

{
  
  MEM_zero(_coord.proj_params);

  _coord.proj_type = Mdvx::PROJ_TRANS_MERCATOR;
  _coord.proj_origin_lat = origin_lat;
  _coord.proj_origin_lon = origin_lon;
  _coord.proj_params.tmerc.central_scale = central_scale;

  _initFromCoords();

}

//////////////////////////////////////////////////////////
// initialize Albers equal area conic projection

void MdvxProj::initAlbers(double origin_lat,
                          double origin_lon,
                          double lat1,
                          double lat2)
  
{
  
  MEM_zero(_coord.proj_params);

  _coord.proj_type = Mdvx::PROJ_ALBERS;
  _coord.proj_origin_lat = origin_lat;
  _coord.proj_origin_lon = origin_lon;
  _coord.proj_params.albers.lat1 = lat1;
  _coord.proj_params.albers.lat2 = lat2;

  _initFromCoords();

}

//////////////////////////////////////////////////////////
// initialize Lambert azimuthal equal area

void MdvxProj::initLambertAzim(double origin_lat,
                               double origin_lon)
  
{
  
  MEM_zero(_coord.proj_params);

  _coord.proj_type = Mdvx::PROJ_LAMBERT_AZIM;
  _coord.proj_origin_lat = origin_lat;
  _coord.proj_origin_lon = origin_lon;

  _initFromCoords();

}

//////////////////////////////////////////////////////////
// initialize Vertical Perspective (satellite view)
// perspective point radius is in km from earth center

void MdvxProj::initVertPersp(double origin_lat,
                             double origin_lon,
                             double persp_radius)
  
{
  
  MEM_zero(_coord.proj_params);

  _coord.proj_type = Mdvx::PROJ_VERT_PERSP;
  _coord.proj_origin_lat = origin_lat;
  _coord.proj_origin_lon = origin_lon;
  _coord.proj_params.vp.persp_radius = persp_radius;

  _initFromCoords();

}

/////////////////////////////////////////////////////////////
// set the grid.
// This is not required if you are initializing from
// MDV headers or the coord struct.

void MdvxProj::setGrid(int nx, int ny,
                       double dx, double dy,
                       double minx, double miny)

{

  _coord.nx = nx;
  _coord.ny = ny;
  _coord.nz = 1;

  _coord.dx = dx;
  _coord.dy = dy;
  _coord.dz = 1;

  _coord.minx = minx;
  _coord.miny = miny;
  _coord.minz = 0;

}

///////////////////////////////////////
// set sensor position
//
// Ht is in km MSL

void MdvxProj::setSensorPosn(double sensor_lat,
			     double sensor_lon,
			     double sensor_ht)

{

  _coord.sensor_lat = sensor_lat;
  _coord.sensor_lon = sensor_lon;
  _coord.sensor_z = sensor_ht;

  switch (_coord.proj_type) {
    
    case Mdvx::PROJ_LATLON: {
      _coord.sensor_x = _coord.sensor_lon;
      _coord.sensor_y = _coord.sensor_lat;
      break;
    }

    case Mdvx::PROJ_POLAR_RADAR: {
      _coord.sensor_x = _coord.proj_origin_lon;
      _coord.sensor_y = _coord.proj_origin_lat;
      _coord.sensor_lon = _coord.proj_origin_lon;
      _coord.sensor_lat = _coord.proj_origin_lat;
      break;
    }

    default: {
      double xx, yy;
      latlon2xy(_coord.sensor_lat, _coord.sensor_lon, xx, yy);
      _coord.sensor_x = xx;
      _coord.sensor_y = yy;
    }

  }

}
  
/////////////////////////////////////////////////////////////////
// Set longitude conditioning with respect to origin.
// If set, xy2latlon() will return a longitude within 180 degrees
// of the project origin.
  
void MdvxProj::setConditionLon2Origin(bool state /* = true */)
  
{
  _condition_lon = state;
  _reference_lon = _origin_lon;
}
  
/////////////////////////////////////////////////////////////////
// Set longitude conditioning with respect to reference lon.
// If set, xy2latlon() will return a longitude within 180 degrees
// of the reference longitude
  
void MdvxProj::setConditionLon2Ref(bool state /* = true */,
				   double reference_lon /* = 0.0 */)

{
  _condition_lon = state;
  _reference_lon = reference_lon;
}
  
///////////////////////////////////////////////////////////////////
// Condition longitude to be in same hemisphere as origin lon

double MdvxProj::conditionLon2Origin(double lon) const

{
  
  double diff = _origin_lon - lon;
  if (fabs(diff) > 180.0) {
    if (diff > 0) {
      return lon + 360.0;
    } else {
      return lon - 360.0;
    }
  }
  return lon;
  
}

///////////////////////////////////////////////////////////////////
// Condition longitude to be in same hemisphere as reference lon

double MdvxProj::conditionLon2Ref(double lon, double ref)

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

/////////////////////////////////////////////////////////////////////
/// Set offset origin by specifying lat/lon.
/// Normally the offset origin and projection origin are co-located
/// This will compute false_northing and false_easting.
///   X = x_from_proj_origin + false_easting
///   Y = y_from_proj_origin + false_northing

void MdvxProj::setOffsetOrigin(double offset_lat,
                               double offset_lon)

{
  if (_math != NULL) {
    _math->setOffsetOrigin(offset_lat, offset_lon);
    _coord.false_northing = _math->getFalseNorthing();
    _coord.false_easting = _math->getFalseEasting();
  }
}

/////////////////////////////////////////////////////////////////////
/// Set offset origin by specifying false_northing and false_easting.
/// Normally the offset origin and projection origin are co-located
/// This will compute offset lat and lon, which is the point:
///   (x = -false_northing, y = -false_easting)

void MdvxProj::setOffsetCoords(double false_northing,
                               double false_easting)

{
  if (_math != NULL) {
    _math->setOffsetCoords(false_northing, false_easting);
    _coord.false_northing = _math->getFalseNorthing();
    _coord.false_easting = _math->getFalseEasting();
  }
}

///////////////////////////////////////
// set origin lat/lon
// can be used to correct for errors

void MdvxProj::setOriginLat(double origin_lat)
{
  _coord.proj_origin_lat = origin_lat;
  _initFromCoords();
}

void MdvxProj::setOriginLon(double origin_lon)
{
  _coord.proj_origin_lon = origin_lon;
  _initFromCoords();
}

//////////////////////////////////////
// generic latlon-to-xy transformation
// most projections: x, y in km, z ignored
// latlon projection: x, y in deg, z ignored
// polar radar projection:
//  x is range in km, y is az in deg, z is elevation in deg

void MdvxProj::latlon2xy(double lat, double lon,
			 double  &xx, double &yy,
                         double zz /* = -9999.0*/) const

{
  
  if (_math == NULL) {
    xx = 0.0;
    yy = 0.0;
    return;
  }

  _math->latlon2xy(lat, lon, xx, yy, zz);

}

//////////////////////////////////////
// generic xy-to-latlon transformation
// most projections: x, y in km, z ignored
// latlon projection: x, y in deg, z ignored
// polar radar projection:
//  x is range in km, y is az in deg, z is elevation in deg

void MdvxProj::xy2latlon(double xx, double yy,
			 double &lat, double &lon,
			 double zz /* = -9999.0*/ ) const

{

  if (_math == NULL) {
    lat = 0.0;
    lon = 0.0;
    return;
  }

  _math->xy2latlon(xx, yy, lat, lon, zz);

  if (_condition_lon) {
    lon = conditionLon2Ref(lon, _reference_lon);
  }

}

///////////////////////////////////////////////////////////////
// latlon2xyIndex()
//
// Computes the the data x, y indices for the given lat/lon location.
//
// If wrap_lon is true, the longitude will be wrapped across the 
// domain for LATLON projections only.
//
// returns 0 on success, -1 on failure (data outside grid)

int MdvxProj::latlon2xyIndex(double lat, double lon,
			     int &x_index, int &y_index,
			     bool wrap_lon /* = false */,
                             double zz /* = -9999.0 */) const
{

  double xx, yy;
  latlon2xy(lat, lon, xx, yy, zz);

  return xy2xyIndex(xx, yy, x_index, y_index, wrap_lon);

}

///////////////////////////////////////////////////////////////
// latlon2xyIndex()
//
// Computes the the data x, y indices, in doubles, for the given
// lat/lon location.
//
// If wrap_lon is true, the longitude will be wrapped across the 
// domain for LATLON projections only.
//
// returns 0 on success, -1 on failure (data outside grid)

int MdvxProj::latlon2xyIndex(double lat, double lon,
			     double &x_index, double &y_index,
			     bool wrap_lon /* = false */,
                             double zz /* = -9999.0*/) const
{

  double xx, yy;
  latlon2xy(lat, lon, xx, yy, zz);
  return xy2xyIndex(xx, yy, x_index, y_index, wrap_lon);

}

///////////////////////////////////////////////////////////////
// latlon2arrayIndex()
//
// Computes the index into the data array.
//
// If wrap_lon is true, the longitude will be wrapped across the 
// domain for LATLON projections only.
//
// returns 0 on success, -1 on failure (data outside grid)

int MdvxProj::latlon2arrayIndex(double lat, double lon,
				int64_t &array_index,
				bool wrap_lon /* = false */,
                                double zz /* = -9999.0*/) const

{

  int x_index, y_index;

  if (latlon2xyIndex(lat, lon, x_index, y_index, wrap_lon, zz)) {
    array_index = 0;
    return -1;
  }

  array_index = (_coord.nx * (int64_t) y_index) + (int64_t) x_index;
  return 0;

}

///////////////////////////////////////////////////////////////
// xy2xyIndex()
//
// Computes the the data x, y indices for the given x/y location.
//
// If wrap_lon is true, the longitude will be wrapped across the 
// domain for LATLON projections only.
//
// returns 0 on success, -1 on failure (data outside grid)

int MdvxProj::xy2xyIndex(double xx, double yy,
			 int &x_index, int &y_index,
			 bool wrap_lon /* = false */) const
{
  
  int iret = 0;

  x_index = (int)((xx - _coord.minx) / _coord.dx + 0.5);
  y_index = (int)((yy - _coord.miny) / _coord.dy + 0.5);
  
  if (x_index < 0) {
    if (_proj_type == Mdvx::PROJ_LATLON && wrap_lon) {
      int x_index_1 = (int)((xx + 360.0 - _coord.minx) / _coord.dx + 0.5);
      if (x_index_1 >= _coord.nx) {
	x_index = 0;
	iret = -1;
      } else {
	x_index = x_index_1;
      }
    } else {
      x_index = 0;
      iret = -1;
    }
  }
  
  if (x_index >= _coord.nx) {
    if (_proj_type == Mdvx::PROJ_LATLON && wrap_lon) {
      int x_index_1 = (int)((xx - 360.0 - _coord.minx) / _coord.dx + 0.5);
      if (x_index_1 < 0) {
	x_index = _coord.nx - 1;
	iret = -1;
      } else {
	x_index = x_index_1;
      }
    } else {
      x_index = _coord.nx - 1;
      iret = -1;
    }
  }
  
  if (y_index < 0) {
    if (_proj_type == Mdvx::PROJ_POLAR_RADAR) {
      int y_index_1 = (int)((yy + 360.0 - _coord.miny) / _coord.dy + 0.5);
      if (y_index_1 >= _coord.ny) {
	y_index = 0;
	iret = -1;
      } else {
	y_index = y_index_1;
      }
    } else {
      y_index = 0;
      iret = -1;
    }
  }
  
  if (y_index >= _coord.ny) {
    if (_proj_type == Mdvx::PROJ_POLAR_RADAR) {
      int y_index_1 = (int)((yy - 360.0 - _coord.miny) / _coord.dy + 0.5);
      if (y_index_1 < 0) {
	y_index = _coord.ny - 1;
	iret = -1;
      } else {
	y_index = y_index_1;
      }
    } else {
      y_index = _coord.ny - 1;
      iret = -1;
    }
  }
  
  return iret;

}

///////////////////////////////////////////////////////////////
// xy2xyIndex()
//
// Computes the the data x, y indices, in doubles, for the given
// x/y location.
//
// If wrap_lon is true, the longitude will be wrapped across the 
// domain for LATLON projections only.
//
// returns 0 on success, -1 on failure (data outside grid)

int MdvxProj::xy2xyIndex(double xx, double yy,
			 double &x_index, double &y_index,
			 bool wrap_lon /* = false */) const
{

  int iret = 0;
  
  x_index = (xx - _coord.minx) / _coord.dx;
  y_index = (yy - _coord.miny) / _coord.dy;
  
  if (x_index < -0.5) {
    if (_proj_type == Mdvx::PROJ_LATLON && wrap_lon) {
      double x_index_1 = (xx + 360.0 - _coord.minx) / _coord.dx;
      if (x_index_1 > _coord.nx - 0.5) {
	x_index = -0.5;
	iret = -1;
      } else {
	x_index = x_index_1;
      }
    } else {
      x_index = -0.5;
      iret = -1;
    }
  }
  
  if (x_index > _coord.nx - 0.5) {
    if (_proj_type == Mdvx::PROJ_LATLON && wrap_lon) {
      double x_index_1 = (xx - 360.0 - _coord.minx) / _coord.dx;
      if (x_index_1 < - 0.5) {
	x_index = _coord.nx - 0.5;
	iret = -1;
      } else {
	x_index = x_index_1;
      }
    } else {
      x_index = _coord.nx - 0.5;
      iret = -1;
    }
  }
  
  if (y_index < 0) {
    if (_proj_type == Mdvx::PROJ_POLAR_RADAR) {
      double y_index_1 = (yy + 360.0 - _coord.miny) / _coord.dy;
      if (y_index_1 >= _coord.ny - 0.5) {
	y_index = -0.5;
	iret = -1;
      } else {
	y_index = y_index_1;
      }
    } else {
      y_index = -0.5;
      iret = -1;
    }
  }
  
  if (y_index >= _coord.ny) {
    if (_proj_type == Mdvx::PROJ_POLAR_RADAR) {
      double y_index_1 = (yy - 360.0 - _coord.miny) / _coord.dy;
      if (y_index_1 < -0.5) {
	y_index = _coord.ny - 0.5;
	iret = -1;
      } else {
	y_index = y_index_1;
      }
    } else {
      y_index = _coord.ny - 0.5;
      iret = -1;
    }
  }
  
  return iret;

}

///////////////////////////////////////////////////////////////
// xyIndex2latlon()
//
// Computes the lat & lon given ix and iy rel to grid.

void MdvxProj::xyIndex2latlon(int ix, int iy,
			      double &lat, double &lon,
                              double zz /* = -9999.0*/) const
  
{

  double xx = _coord.minx + ix * _coord.dx;
  double yy = _coord.miny + iy * _coord.dy;
  xy2latlon(xx, yy, lat, lon, zz);

}


///////////////////////////////////////////////////////////////
// xyIndex2latlon()
//
// Computes the lat & lon given ix and iy rel to grid.

void MdvxProj::xyIndex2latlon(double x, double y,
			      double &lat, double &lon,
                              double zz /* = -9999.0*/) const
  
{

  double xx = _coord.minx + x * _coord.dx;
  double yy = _coord.miny + y * _coord.dy;
  xy2latlon(xx, yy, lat, lon, zz);

}

///////////////////////////////////////////////////////////////
// x2km()
//
// Converts the given distance to kilometers.  The distance
// is assumed to be in the units appropriate to the projection.

double MdvxProj::x2km(double x) const
{
  switch (_proj_type) {
  case (Mdvx::PROJ_LATLON):
    return (x * KM_PER_DEG_AT_EQ);
  default:
    return (x);
  }

}


///////////////////////////////////////////////////////////////
// km2xGrid()
//
// Converts the given distance in kilometers to the appropriate
// number of grid spaces along the X axis.

double MdvxProj::km2xGrid(double x_km) const
{
  switch (_proj_type) {
  case (Mdvx::PROJ_LATLON):
    return _ll_km2xGrid(x_km);
  default:
    return _km2grid(x_km, _coord.dx);
  }

}


///////////////////////////////////////////////////////////////
// km2yGrid()
//
// Converts the given distance in kilometers to the appropriate
// number of grid spaces along the Y axis.

double MdvxProj::km2yGrid(double y_km) const
{
  switch (_proj_type) {
  case (Mdvx::PROJ_LATLON):
    return _ll_km2yGrid(y_km);
  default:
    return _km2grid(y_km, _coord.dy);
  }

}


///////////////////////////////////////////////////////////////
// xGrid2km()
//
// Converts the given distance in number of grid spaces along
// the X axis to kilometers.

double MdvxProj::xGrid2km(double x_grid) const
{
  switch (_proj_type) {
  case (Mdvx::PROJ_LATLON):
    return _ll_xGrid2km(x_grid);
  default:
    return _grid2km(x_grid, _coord.dx);
  }
}


///////////////////////////////////////////////////////////////
// yGrid2km()
//
// Converts the given distance in number of grid spaces along
// the Y axis to kilometers.

double MdvxProj::yGrid2km(double y_grid) const
{
  switch (_proj_type) {
  case (Mdvx::PROJ_LATLON):
    return _ll_yGrid2km(y_grid);
  default:
    return _grid2km(y_grid, _coord.dy);
  }
}

///////////////////////////////////////////////////////////////
// get lat/lon of grid origin (as opposed to projection origin)

void MdvxProj::getGridOrigin(double &lat, double &lon) const
{

  if (_coord.false_easting == 0 && _coord.false_northing == 0) {
    lat = _coord.proj_origin_lat;
    lon = _coord.proj_origin_lon;
    return;
  }

  xy2latlon(0.0, 0.0, lat, lon);

}

////////////////////////////////////////////////////////
// 
// getEdgeExtrema()
//
// Get the maximum and minimum lat, lon by going
// around the edge of the grid. Computationally expensive
// in some cases. Niles Oien November 2004.
//
void MdvxProj::getEdgeExtrema(double &minLat, double &minLon,
			      double &maxLat, double &maxLon) const
{

  // Initialize.

  double lat, lon;
  xyIndex2latlon(0, 0, lat, lon);
  minLat = maxLat = lat;         maxLon = minLon = lon;

  // Go around all four sides.

  // First, the sides defined by ix=0 and ix=Nx-1.

  for (int iy = 0; iy < _coord.ny; iy++){
    //
    xyIndex2latlon(0, iy, lat, lon);
    if (lon < minLon) minLon = lon;
    if (lon > maxLon) maxLon = lon;
    if (lat < minLat) minLat = lat;
    if (lat > maxLat) maxLat = lat;
    //
    xyIndex2latlon(_coord.nx-1, iy, lat, lon);
    if (lon < minLon) minLon = lon;
    if (lon > maxLon) maxLon = lon;
    if (lat < minLat) minLat = lat;
    if (lat > maxLat) maxLat = lat;
  }

  // Then, the sides defined by iy=0 and iy=Ny-1.

  for (int ix = 0; ix < _coord.nx; ix++){
    //
    xyIndex2latlon(ix, 0, lat, lon);
    if (lon < minLon) minLon = lon;
    if (lon > maxLon) maxLon = lon;
    if (lat < minLat) minLat = lat;
    if (lat > maxLat) maxLat = lat;
    //
    xyIndex2latlon(ix,  _coord.ny-1, lat, lon);
    if (lon < minLon) minLon = lon;
    if (lon > maxLon) maxLon = lon;
    if (lat < minLat) minLat = lat;
    if (lat > maxLat) maxLat = lat;
  }

}

////////////////////////////////////////////////////////
// syncToHdrs()
//
// Synchronize master and field header with info from
// this object.
//
// If data_element_nbytes has not been set in the field header
// before this call, volume_size will be incorrectly computed, so
// you will need to compute volume_size independently.

void MdvxProj::syncToHdrs(Mdvx::master_header_t &mhdr,
			  Mdvx::field_header_t &fhdr) const
  
{

  syncToFieldHdr(fhdr);

  mhdr.max_nx = MAX(mhdr.max_nx, fhdr.nx);
  mhdr.max_ny = MAX(mhdr.max_ny, fhdr.ny);
  mhdr.max_nz = MAX(mhdr.max_nz, fhdr.nz);

  mhdr.sensor_lon = _coord.sensor_lon;
  mhdr.sensor_lat = _coord.sensor_lat;
  mhdr.sensor_alt = _coord.sensor_z;

}

////////////////////////////////////////////////////////
// syncToFieldHdr
//
// Synchronize field header with info from this object.
//
// If data_element_nbytes has not been set in the field header
// before this call, volume_size will be incorrectly computed, so
// you will need to compute volume_size independently.

void MdvxProj::syncToFieldHdr(Mdvx::field_header_t &fhdr) const
  
{

  // first sync XY info

  syncXyToFieldHdr(fhdr);

  // then Z info

  fhdr.grid_minz = _coord.minz;
  fhdr.grid_dz = _coord.dz;
  fhdr.nz = _coord.nz;

  // compute vol size

  fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * fhdr.data_element_nbytes;

}

////////////////////////////////////////////////////////
// syncXyToFieldHdr
//
// Synchronize field header with (x,y) info from this object.
//
// If data_element_nbytes has not been set in the field header
// before this call, volume_size will be incorrectly computed, so
// you will need to compute volume_size independently.

void MdvxProj::syncXyToFieldHdr(Mdvx::field_header_t &fhdr) const
  
{

  // projection type

  fhdr.proj_type = _coord.proj_type;

  // centroid

  fhdr.proj_origin_lat = _coord.proj_origin_lat;
  fhdr.proj_origin_lon = _coord.proj_origin_lon;
  
  // rotation - special case for flat

  if (_coord.proj_type == Mdvx::PROJ_FLAT) {
    fhdr.proj_rotation = _coord.proj_params.flat.rotation;
  }

  // set proj params array

  _coord2ProjParams((Mdvx::projection_type_t) _coord.proj_type,
		    _coord, fhdr.proj_param);

  // grid

  fhdr.grid_minx = _coord.minx;
  fhdr.grid_miny = _coord.miny;

  fhdr.grid_dx = _coord.dx;
  fhdr.grid_dy = _coord.dy;

  fhdr.nx = _coord.nx;
  fhdr.ny = _coord.ny;

  fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * fhdr.data_element_nbytes;

}

#define BOOL_STR(a) (a == 0 ? "false" : "true")

///////////////
// print object

void MdvxProj::print(ostream &out, const bool print_z) const

{

  out << "Projection: " << Mdvx::projType2Str(_proj_type) << endl;
  out << "-----------" << endl;
  _math->print(out);
  out << endl;

  out << "Grid:" << endl;
  out << "----" << endl;
  if (print_z)
  {
    out << "  nx, ny, nz: " << _coord.nx << ", " << _coord.ny
	<< ", " << _coord.nz << endl;
  }
  else
  {
    out << "  nx, ny: " << _coord.nx << ", " << _coord.ny << endl;
  }
  
  if (print_z)
  {
    out << "  minx, miny, minz: " << _coord.minx << ", " << _coord.miny
	<< ", " << _coord.minz << endl;
  }
  else
  {
    out << "  minx, miny: " << _coord.minx << ", " << _coord.miny << endl;
  }
  
  double maxx = _coord.minx  + (_coord.nx - 1) *_coord.dx;
  double maxy = _coord.miny  + (_coord.ny - 1) *_coord.dy;
  out << "  maxx, maxy: " << maxx << ", " << maxy << endl;

  if (print_z)
  {
    out << "  dx, dy, dz: " << _coord.dx << ", " << _coord.dy
	<< ", " << _coord.dz << endl;
  }
  else
  {
    out << "  dx, dy: " << _coord.dx << ", " << _coord.dy << endl;
  }
  
  out << "  sensor_x, sensor_y, sensor_z: "
      << _coord.sensor_x << ", " << _coord.sensor_y
      << ", " << _coord.sensor_z << endl;

  out << "  sensor_lat, sensor_lon: "
      << _coord.sensor_lat << ", " << _coord.sensor_lon << endl;

  out << "  proj origin latitude: " << _coord.proj_origin_lat << endl;
  out << "  proj origin longitude: " << _coord.proj_origin_lon << endl;
  
  double gridOriginLat, gridOriginLon;
  getGridOrigin(gridOriginLat, gridOriginLon);

  out << "  grid origin latitude: " << gridOriginLat << endl;
  out << "  grid origin longitude: " << gridOriginLon << endl;

  double minLat, minLon, maxLat, maxLon;
  getEdgeExtrema(minLat, minLon, maxLat, maxLon);
  out << "  minLat, minLon: " << minLat << ", " << minLon << endl;
  out << "  maxLat, maxLon: " << maxLat << ", " << maxLon << endl;
  
  if (print_z)
  {
    out << "  dz_constant: " << BOOL_STR(_coord.dz_constant) << endl;
  }
  
  out << "  x units: " << _coord.unitsx << endl;
  out << "  y units: " << _coord.unitsy << endl;
  if (print_z)
  {
    out << "  z units: " << _coord.unitsz << endl;
  }
  
  out << endl;

}

/////////////////////
// print coord struct

void MdvxProj::printCoord(const Mdvx::coord_t &coord,
			  ostream &out)
  
{
  
  out << "  MdvxProj coord parameters" << endl;
  out << "  -------------------------" << endl;
  out << "    ProjType: " << Mdvx::projType2Str(coord.proj_type) << endl;
  if (coord.proj_type != Mdvx::PROJ_LATLON) {
    out << "    origin latitude: " << coord.proj_origin_lat << endl;
    out << "    origin longitude: " << coord.proj_origin_lon << endl;
  }
  switch (coord.proj_type) {
  case Mdvx::PROJ_FLAT:
    out << "    rotation: " << coord.proj_params.flat.rotation << endl;
    break;
  case Mdvx::PROJ_LAMBERT_CONF:
    out << "    lat1: " << coord.proj_params.lc2.lat1 << endl;
    out << "    lat2: " << coord.proj_params.lc2.lat2 << endl;
    break;
  case Mdvx::PROJ_POLAR_STEREO:
    out << "    tan_lon: " << coord.proj_params.ps.tan_lon << endl;
    if (coord.proj_params.ps.pole_type == 0) {
      out << "    pole: north" << endl;
    } else {
      out << "    pole: south" << endl;
    }
    out << "    central_scale: " << coord.proj_params.ps.central_scale << endl;
    break;
  case Mdvx::PROJ_OBLIQUE_STEREO:
    out << "    tan_lat: " << coord.proj_params.os.tan_lat << endl;
    out << "    tan_lon: " << coord.proj_params.os.tan_lon << endl;
    out << "    central_scale: " << coord.proj_params.os.central_scale << endl;
    break;
  case Mdvx::PROJ_TRANS_MERCATOR:
    out << "    central_scale: " << coord.proj_params.tmerc.central_scale << endl;
    break;
  case Mdvx::PROJ_ALBERS:
    out << "    lat1: " << coord.proj_params.albers.lat1 << endl;
    out << "    lat2: " << coord.proj_params.albers.lat2 << endl;
    break;
  case Mdvx::PROJ_VERT_PERSP:
    out << "    persp_radius: "
        << coord.proj_params.vp.persp_radius << endl;
    break;
  }
  
  out << "    false_easting: " << coord.false_easting << endl;
  out << "    false_northing: " << coord.false_northing << endl;

  out << "    nx, ny: " << coord.nx << ", " << coord.ny << endl;
  out << "    minx, miny: " << coord.minx << ", " << coord.miny << endl;
  out << "    dx, dy: " << coord.dx << ", " << coord.dy << endl;
  
  double maxx = coord.minx  + (coord.nx - 1) *coord.dx;
  double maxy = coord.miny  + (coord.ny - 1) *coord.dy;
  out << "    maxx, maxy: " << maxx << ", " << maxy << endl;

  if (coord.sensor_x != 0 || coord.sensor_y != 0 || coord.sensor_z != 0) {
    out << "    sensor_x, sensor_y, sensor_z: "
	<< coord.sensor_x << ", " << coord.sensor_y
	<< ", " << coord.sensor_z << endl;
  }

  if (coord.sensor_lat != 0 || coord.sensor_lon != 0) {
    out << "    sensor_lat, sensor_lon: "
	<< coord.sensor_lat << ", " << coord.sensor_lon << endl;
  }

  MdvxProj proj(coord);
  double minLat, minLon;
  proj.xy2latlon(coord.minx, coord.miny, minLat, minLon);
  out << "    minLat, minLon: " << minLat << ", " << minLon << endl;
  double maxLat, maxLon;
  proj.xy2latlon(maxx, maxy, maxLat, maxLon);
  out << "    maxLat, maxLon: " << maxLat << ", " << maxLon << endl;
  
  if (strlen(coord.unitsx) > 0) {
    out << "    x units: " << coord.unitsx << endl;
  }
  if (strlen(coord.unitsy) > 0) {
    out << "    y units: " << coord.unitsy << endl;
  }

}


/////////////////////
// protected routines
/////////////////////

/////////////////////////
// clear status

void MdvxProj::_clear()

{

  MEM_zero(_coord);
  _proj_type = Mdvx::PROJ_LATLON;
  _origin_lat = 0.0;
  _origin_lon = 0.0;
  _condition_lon = false;
  _reference_lon = 0.0;

}

///////////////////////////////
// initialize to default state

void MdvxProj::_initToDefaults()
  
{

  _clear();

  _coord.nx = 1;
  _coord.ny = 1;
  _coord.nz = 1;
  _coord.dx = 1.0;
  _coord.dy = 1.0;
  _coord.dz = 1.0;
  _coord.dz_constant = 1;
  _coord.nbytes_char = 3 * MDV_COORD_UNITS_LEN;

  initLatlon();

}

/////////////////////////////////
// initialize from _coord struct

void MdvxProj::_initFromCoords()

{

  _proj_type = (Mdvx::projection_type_t) _coord.proj_type;
  _origin_lat = _coord.proj_origin_lat;
  _origin_lon = _coord.proj_origin_lon;

  if (_math != NULL) {
    delete _math;
    _math = NULL;
  }

  // create a PjgMath object based on the proj_type

  switch (_proj_type) {
    
    case Mdvx::PROJ_FLAT: {
      _math = new PjgAzimEquidistMath(_coord.proj_origin_lat,
                                      _coord.proj_origin_lon,
                                      _coord.proj_params.flat.rotation);
      _math->setOffsetCoords(_coord.false_northing, _coord.false_easting);
      break;
    }
      
    case Mdvx::PROJ_LATLON: {
      _math = new PjgLatlonMath(_coord.proj_origin_lon);
      break;
    }
      
    case Mdvx::PROJ_LAMBERT_CONF: {
      _math = new PjgLambertConfMath(_coord.proj_origin_lat,
                                     _coord.proj_origin_lon,
                                     _coord.proj_params.lc2.lat1,
                                     _coord.proj_params.lc2.lat2);
      _math->setOffsetCoords(_coord.false_northing, _coord.false_easting);
      break;
    }
      
    case Mdvx::PROJ_POLAR_RADAR: {
      _math = new PjgPolarRadarMath(_coord.proj_origin_lat,
                                    _coord.proj_origin_lon);
                                    break;
    }
      
    case Mdvx::PROJ_POLAR_STEREO: {

      bool pole_is_north = true;
      if (_coord.proj_params.ps.pole_type != Mdvx::POLE_NORTH) {
        pole_is_north = false;
      }
      if (_coord.proj_params.ps.central_scale == 0.0) {
        _coord.proj_params.ps.central_scale = 1.0;
      }
      _math = new PjgPolarStereoMath(_coord.proj_params.ps.tan_lon,
                                     pole_is_north,
                                     _coord.proj_params.ps.central_scale);
      if (fabs(_coord.proj_origin_lat) != 90.0 ||
          _coord.proj_origin_lon != _coord.proj_params.ps.tan_lon) {
        _math->setOffsetOrigin(_coord.proj_origin_lat,
                               _coord.proj_origin_lon);
        _coord.false_northing = _math->getFalseNorthing();
        _coord.false_easting = _math->getFalseEasting();
      } else {
        _math->setOffsetCoords(_coord.false_northing, _coord.false_easting);
      }
      break;

    }
      
    case Mdvx::PROJ_OBLIQUE_STEREO: {
      if (_coord.proj_params.os.central_scale == 0.0) {
        _coord.proj_params.os.central_scale = 1.0;
      }
      _math = new PjgObliqueStereoMath(_coord.proj_params.os.tan_lat,
                                       _coord.proj_params.os.tan_lon,
                                       _coord.proj_params.os.central_scale);
      if (_coord.proj_origin_lat != _coord.proj_params.os.tan_lat ||
          _coord.proj_origin_lon != _coord.proj_params.os.tan_lon) {
        _math->setOffsetOrigin(_coord.proj_origin_lat,
                               _coord.proj_origin_lon);
        _coord.false_northing = _math->getFalseNorthing();
        _coord.false_easting = _math->getFalseEasting();
      } else {
        _math->setOffsetCoords(_coord.false_northing, _coord.false_easting);
      }
      break;
    }
      
    case Mdvx::PROJ_MERCATOR: {
      _math = new PjgMercatorMath(_coord.proj_origin_lat,
                                  _coord.proj_origin_lon);
      _math->setOffsetCoords(_coord.false_northing, _coord.false_easting);
      break;
    }
      
    case Mdvx::PROJ_TRANS_MERCATOR: {
      _math = new PjgTransMercatorMath(_coord.proj_origin_lat,
                                       _coord.proj_origin_lon,
                                       _coord.proj_params.tmerc.central_scale);
      _math->setOffsetCoords(_coord.false_northing, _coord.false_easting);
      break;
    }
      
    case Mdvx::PROJ_ALBERS: {
      _math = new PjgAlbersMath(_coord.proj_origin_lat,
                                _coord.proj_origin_lon,
                                _coord.proj_params.albers.lat1,
                                _coord.proj_params.albers.lat2);
      _math->setOffsetCoords(_coord.false_northing, _coord.false_easting);
      break;
    }
      
    case Mdvx::PROJ_LAMBERT_AZIM: {
      _math = new PjgLambertAzimMath(_coord.proj_origin_lat,
                                     _coord.proj_origin_lon);
      _math->setOffsetCoords(_coord.false_northing, _coord.false_easting);
      break;
    }
      
    case Mdvx::PROJ_VERT_PERSP: {
      _math = new PjgVertPerspMath(_coord.proj_origin_lat,
                                   _coord.proj_origin_lon,
                                   _coord.proj_params.vp.persp_radius);
      _math->setOffsetCoords(_coord.false_northing, _coord.false_easting);
      break;
    }
      
    case Mdvx::PROJ_VSECTION: {
      _math = new PjgLatlonMath(0.0);
      break;
    }
      
    default: {
      _initToDefaults();
      break;
    }
    
  } // switch

}

////////////////////////////////////////////////////////
// copy coord information to proj_param array
// Proj params array should be at least length MDV_MAX_PROJ_PARAMS

void MdvxProj::_coord2ProjParams(Mdvx::projection_type_t proj_type,
				 const Mdvx::coord_t &coord,
				 fl64 *proj_params)
  
{

  memset(proj_params, 0, MDV_MAX_PROJ_PARAMS * sizeof(fl32));

  switch (proj_type) {
    
  case Mdvx::PROJ_FLAT: {
    proj_params[0] = coord.proj_params.flat.rotation;
    break;
  }
    
  case Mdvx::PROJ_LAMBERT_CONF: {
    proj_params[0] = coord.proj_params.lc2.lat1;
    proj_params[1] = coord.proj_params.lc2.lat2;
    break;
  }
    
  case Mdvx::PROJ_POLAR_STEREO: {
    proj_params[0] = coord.proj_params.ps.tan_lon;
    proj_params[1] = coord.proj_params.ps.pole_type;
    proj_params[2] = coord.proj_params.ps.central_scale;
    proj_params[3] = coord.proj_params.ps.lad;
    break;
  }
    
  case Mdvx::PROJ_OBLIQUE_STEREO: {
    proj_params[0] = coord.proj_params.os.tan_lat;
    proj_params[1] = coord.proj_params.os.tan_lon;
    proj_params[2] = coord.proj_params.os.central_scale;
    break;
  }
    
  case Mdvx::PROJ_TRANS_MERCATOR: {
    proj_params[0] = coord.proj_params.tmerc.central_scale;
    break;
  }
    
  case Mdvx::PROJ_ALBERS: {
    proj_params[0] = coord.proj_params.albers.lat1;
    proj_params[1] = coord.proj_params.albers.lat2;
    break;
  }

  case Mdvx::PROJ_VERT_PERSP: {
    proj_params[0] = coord.proj_params.vp.persp_radius;
    break;
  }
    
  default: {
  }
    
  } // switch
  
  proj_params[6] = coord.false_northing;
  proj_params[7] = coord.false_easting;

}

////////////////////////////////////////////////////////
// copy proj_param info to coord struct
// Proj params array should be at least length 8 = MDV_MAX_PROJ_PARAMS

void MdvxProj::_projParams2Coord(Mdvx::projection_type_t proj_type,
				 const fl64 *proj_params,
				 Mdvx::coord_t &coord)
				  
{
  

  switch (proj_type) {
    
  case Mdvx::PROJ_FLAT:
    coord.proj_params.flat.rotation = proj_params[0];
    break;
    
  case Mdvx::PROJ_LAMBERT_CONF: 
    coord.proj_params.lc2.lat1 = proj_params[0];
    coord.proj_params.lc2.lat2 = proj_params[1];
    break;
  
  case Mdvx::PROJ_POLAR_STEREO:
    coord.proj_params.ps.tan_lon = proj_params[0];
    if (proj_params[1] == 0) {
      coord.proj_params.ps.pole_type = 0;
    } else {
      coord.proj_params.ps.pole_type = 1;
    }
    coord.proj_params.ps.central_scale = proj_params[2];
    coord.proj_params.ps.lad = proj_params[3];
    break;

  case Mdvx::PROJ_OBLIQUE_STEREO:
    coord.proj_params.os.tan_lat = proj_params[0];
    coord.proj_params.os.tan_lon = proj_params[1];
    coord.proj_params.os.central_scale = proj_params[2];
    break;

  case Mdvx::PROJ_TRANS_MERCATOR: 
    coord.proj_params.tmerc.central_scale = proj_params[0];
    break;
  
  case Mdvx::PROJ_ALBERS: 
    coord.proj_params.albers.lat1 = proj_params[0];
    coord.proj_params.albers.lat2 = proj_params[1];
    break;
  
  case Mdvx::PROJ_VERT_PERSP: 
    coord.proj_params.vp.persp_radius = proj_params[0];
    break;
  
  default:
    break;

  } // switch

  coord.false_northing = proj_params[6];
  coord.false_easting = proj_params[7];
  
}

/////////////////////////////////////////////
// Load up coord_t struct from field header

void MdvxProj::_loadCoordFromFieldHdr(const Mdvx::field_header_t &fhdr)

{

  // projection type

  _coord.proj_type = (Mdvx::projection_type_t) fhdr.proj_type;

  // origin

  _coord.proj_origin_lat = fhdr.proj_origin_lat;
  _coord.proj_origin_lon = fhdr.proj_origin_lon;

  // params array

  _projParams2Coord((Mdvx::projection_type_t) _coord.proj_type,
		    fhdr.proj_param, _coord);
  
  // sensor location

  switch (_coord.proj_type) {
    
  case Mdvx::PROJ_LATLON:
    _coord.sensor_x = _coord.sensor_lon;
    _coord.sensor_y = _coord.sensor_lat;
    break;

  case Mdvx::PROJ_POLAR_RADAR:
    _coord.sensor_x = _coord.proj_origin_lon;
    _coord.sensor_y = _coord.proj_origin_lat;
    break;

  default:
    _coord.sensor_x = 0.0;
    _coord.sensor_y = 0.0;
    break;
    
  }

  // grid

  _coord.minx = fhdr.grid_minx;
  _coord.miny = fhdr.grid_miny;
  _coord.minz = fhdr.grid_minz;

  _coord.dx = fhdr.grid_dx;
  _coord.dy = fhdr.grid_dy;
  _coord.dz = fhdr.grid_dz;

  _coord.nx = fhdr.nx;
  _coord.ny = fhdr.ny;
  _coord.nz = fhdr.nz;

  if (fhdr.vlevel_type == Mdvx::VERT_TYPE_Z) {
    _coord.dz_constant = 1;
  } else {
    _coord.dz_constant = 0;
  }

  // units

  _coord.nbytes_char = MDV_N_COORD_LABELS * MDV_COORD_UNITS_LEN;
  
  strcpy(_coord.unitsx, "");
  strcpy(_coord.unitsy, "");
  strcpy(_coord.unitsz, "");

  if (_coord.proj_type == Mdvx::PROJ_LATLON) {
    strcpy(_coord.unitsx, "deg");
    strcpy(_coord.unitsy, "deg");
  } else if (_coord.proj_type == Mdvx::PROJ_POLAR_RADAR ||
             _coord.proj_type == Mdvx::PROJ_RHI_RADAR) {
    strcpy(_coord.unitsx, "km");
    strcpy(_coord.unitsy, "deg");
    strcpy(_coord.unitsz, "deg");
  } else if (_coord.proj_type == Mdvx::PROJ_RADIAL) {
    strcpy(_coord.unitsx, "m");
    strcpy(_coord.unitsy, "deg");
  } else {
    strcpy(_coord.unitsx, "km");
    strcpy(_coord.unitsy, "km");
  }

  switch (fhdr.vlevel_type) {
  case Mdvx::VERT_TYPE_SURFACE:
    strcpy(_coord.unitsz, "");
    break;
  case Mdvx::VERT_TYPE_SIGMA_P:
    strcpy(_coord.unitsz, "sigma_p");
    break;
  case Mdvx::VERT_TYPE_PRESSURE:
    strcpy(_coord.unitsz, "mb");
    break;
  case Mdvx::VERT_TYPE_Z:
    strcpy(_coord.unitsz, "km");
    break;
  case Mdvx::VERT_TYPE_SIGMA_Z:
    strcpy(_coord.unitsz, "km");
    break;
  case Mdvx::VERT_TYPE_ETA:
    strcpy(_coord.unitsz, "eta");
    break;
  case Mdvx::VERT_TYPE_THETA:
    strcpy(_coord.unitsz, "K");
    break;
  case Mdvx::VERT_TYPE_ELEV:
    strcpy(_coord.unitsz, "deg");
    break;
  case Mdvx::VERT_VARIABLE_ELEV:
    strcpy(_coord.unitsz, "var_elev");
    break;
  case Mdvx::VERT_FIELDS_VAR_ELEV:
    strcpy(_coord.unitsz, "var_elev");
    break;
  case Mdvx::VERT_FLIGHT_LEVEL:
    strcpy(_coord.unitsz, "FL");
    break;
  default: {}
  }

}

////////////////////////////////////////////
// Load up coord_t struct from master header

void MdvxProj::_loadCoordFromMasterHdr(const Mdvx::master_header_t &mhdr)
  
{
  
  _coord.sensor_lon = mhdr.sensor_lon;
  _coord.sensor_lat = mhdr.sensor_lat;
  _coord.sensor_z = mhdr.sensor_alt;

}

/////////////////////////////////////////////////////////////////////////
// Convert the given distance in kilometers to the number of grid spaces.

double MdvxProj::_km2grid(double km_distance,
			  double grid_delta) const
{
  return km_distance / grid_delta;
}


/////////////////////////////////////////////////////////////////////////
// Convert the given distance in kilometers to the number of grid spaces
// in the X direction.

double MdvxProj::_ll_km2xGrid(double km_distance) const
{
  double mid_lat = _coord.miny + _coord.dy * _coord.ny / 2.0;
  double latitude_factor = cos(mid_lat * DEG_TO_RAD);
  return (km_distance * DEG_PER_KM_AT_EQ / latitude_factor) / _coord.dx;
}


/////////////////////////////////////////////////////////////////////////
// Convert the given distance in kilometers to the number of grid spaces
// in the Y direction.

double MdvxProj::_ll_km2yGrid(double km_distance) const
{
  return (km_distance * DEG_PER_KM_AT_EQ) / _coord.dy;
}


/////////////////////////////////////////////////////////////////////////
// Convert the given distance in number of grid spaces to kilometers.

double MdvxProj::_grid2km(double grid_distance,
			  double grid_delta) const
{
  return grid_distance * grid_delta;
}


/////////////////////////////////////////////////////////////////////////
// Convert the given distance in number of grid spaces in the X direction
// to kilometers.

double MdvxProj::_ll_xGrid2km(double grid_distance) const
{
  double mid_lat = _coord.miny + _coord.dy * _coord.ny / 2.0;
  double latitude_factor = cos(mid_lat * DEG_TO_RAD);
  
  return (grid_distance * KM_PER_DEG_AT_EQ * latitude_factor) * _coord.dx;
}


/////////////////////////////////////////////////////////////////////////
// Convert the given distance in number of grid spaces in the Y direction
// to kilometers.

double MdvxProj::_ll_yGrid2km(double grid_distance) const
{
  return (grid_distance * KM_PER_DEG_AT_EQ) * _coord.dy;
}


