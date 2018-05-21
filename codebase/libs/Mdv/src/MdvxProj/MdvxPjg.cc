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
/*********************************************************************
 * MdvxPjg.cc: class implementing projective geometry calculations
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2001
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxPjg.hh>
#include <Mdv/MdvxRadar.hh>
#include <toolsa/mem.h>
#include <toolsa/pjg.h>
#include <toolsa/toolsa_macros.h>
#include <math.h>
using namespace std;


//////////////////
// Constructors //
//////////////////

/**********************************************************************
 * Default constructor
 */

MdvxPjg::MdvxPjg() :
  Pjg(),
  _supported(false),
  _dataOrdering(Mdvx::ORDER_XYZ)
{
  MEM_zero(_coord);
  _init();
}


/**********************************************************************
 * Construct from first field of Mdvx object.
 */

MdvxPjg::MdvxPjg(const Mdvx &mdvx) :
  Pjg(),
  _supported(false),
  _dataOrdering(Mdvx::ORDER_XYZ)
{
  MEM_zero(_coord);
  init(mdvx);
}


/**********************************************************************
 * Construct from master and field headers.
 */

MdvxPjg::MdvxPjg(const Mdvx::master_header_t &mhdr,
		 const Mdvx::field_header_t &fhdr) :
  Pjg(),
  _supported(false),
  _dataOrdering((Mdvx::grid_order_indices_t)mhdr.data_ordering)
{
  MEM_zero(_coord);
  init(mhdr, fhdr);
}


/**********************************************************************
 * Construct from field header.
 *
 * Sensor position will not be filled in, since this is only available
 * from the master header.
 */

MdvxPjg::MdvxPjg(const Mdvx::field_header_t &fhdr) :
  Pjg(),
  _supported(false),
  _dataOrdering(Mdvx::ORDER_XYZ)
{
  MEM_zero(_coord);
  init(fhdr);
}


/**********************************************************************
 * Construct from coord struct.
 */

MdvxPjg::MdvxPjg(const Mdvx::coord_t &coord) :
  Pjg(),
  _supported(false),
  _dataOrdering(Mdvx::ORDER_XYZ)
{
  MEM_zero(_coord);
  init(coord);
}


/**********************************************************************
 * Construct from Pjg object
 */

MdvxPjg::MdvxPjg(const Pjg &pjg) :
  Pjg(pjg),
  _supported(false),
  _dataOrdering(Mdvx::ORDER_XYZ)
{
  MEM_zero(_coord);
  init(pjg);
}


/**********************************************************************
 * Copy constructor.
 */

MdvxPjg::MdvxPjg(const MdvxPjg &rhs) :
  Pjg(rhs)
{
  this->_coord = rhs._coord;
  this->_supported = rhs._supported;
  this->_dataOrdering = rhs._dataOrdering;
}

/**********************************************************************
 * Destructor
 */

MdvxPjg::~MdvxPjg()
{
  // Do nothing
}


/////////////////////////////
// Assignment
//

MdvxPjg &MdvxPjg::operator=(const MdvxPjg &rhs)

{
  if (&rhs == this)
    return *this;

  Pjg::operator=(rhs);
  this->_coord = rhs._coord;
  this->_supported = rhs._supported;

  return *this;
}

/**********************************************************************
 * supported() - Check that the underlying projection is supported.
 *
 * Return true is proj type is supported, false otherwise.
 *
 * Useful for checking if the constructor was given data which
 * can be used by this class.
 */

bool MdvxPjg::supported()
{
  return _supported;
}


/**********************************************************************
 * init() - Initialize the object from the given information.
 */

void MdvxPjg::init(const Mdvx &mdvx)
{
  // load from first field if exists

  MdvxField *fld0 = mdvx.getField(0);
  if (fld0)
    init(fld0->getFieldHeader());
  else
    _init();

  // update from master header

  _loadCoordFromMasterHdr(mdvx.getMasterHeader());

  // set sensor location if radar available

  MdvxRadar mdvxRadar;
  if (mdvxRadar.loadFromMdvx(mdvx) == 0)
  {
    DsRadarParams radar = mdvxRadar.getRadarParams();
    setSensorPosn(radar.latitude, radar.longitude, radar.altitude);
  }
}


void MdvxPjg::init(const Mdvx::master_header_t &mhdr,
		   const Mdvx::field_header_t &fhdr)
{
  init(fhdr);
  _loadCoordFromMasterHdr(mhdr);
}


void MdvxPjg::init(const Mdvx::field_header_t &fhdr)
{
  _supported = false;
  
  switch (fhdr.proj_type)
  {
  case Mdvx::PROJ_FLAT :
  {
    _supported = true;
    
    double rotation = fhdr.proj_param[0];
    initFlat(fhdr.proj_origin_lat, fhdr.proj_origin_lon,
	     rotation,
	     fhdr.nx, fhdr.ny, fhdr.nz,
	     fhdr.grid_dx, fhdr.grid_dy, fhdr.grid_dz,
	     fhdr.grid_minx, fhdr.grid_miny, fhdr.grid_minz);

    break;
  }

  case Mdvx::PROJ_LATLON :
    _supported = true;
    
    initLatlon(fhdr.nx, fhdr.ny, fhdr.nz,
	       fhdr.grid_dx, fhdr.grid_dy, fhdr.grid_dz,
	       fhdr.grid_minx, fhdr.grid_miny, fhdr.grid_minz);
    break;

  case Mdvx::PROJ_LAMBERT_CONF :
  {
    _supported = true;
    
    double lat1 = fhdr.proj_param[0];
    double lat2 = fhdr.proj_param[1];

    initLc2(fhdr.proj_origin_lat, fhdr.proj_origin_lon, lat1, lat2,
	    fhdr.nx, fhdr.ny, fhdr.nz,
	    fhdr.grid_dx, fhdr.grid_dy, fhdr.grid_dz,
	    fhdr.grid_minx, fhdr.grid_miny, fhdr.grid_minz);

    break;
  }

  case Mdvx::PROJ_POLAR_RADAR :
    _supported = true;
    
    initPolarRadar(fhdr.proj_origin_lat, fhdr.proj_origin_lon,
		   fhdr.nx, fhdr.ny, fhdr.nz,
		   fhdr.grid_dx, fhdr.grid_dy, fhdr.grid_dz,
		   fhdr.grid_minx, fhdr.grid_miny, fhdr.grid_minz);
    break;

  case Mdvx::PROJ_POLAR_STEREO :
    _supported = true;
    
    PjgTypes::pole_type_t pole_type;
    if (fhdr.proj_param[1] == 0) {
      pole_type = PjgTypes::POLE_NORTH;
    } else {
      pole_type = PjgTypes::POLE_SOUTH;
    }
    
    initPolarStereo(static_cast<double>(fhdr.proj_param[0]), 
		    pole_type,
		    static_cast<double>(fhdr.proj_param[2]),
		    fhdr.nx, fhdr.ny, fhdr.nz,
		    fhdr.grid_dx, fhdr.grid_dy, fhdr.grid_dz,
		    fhdr.grid_minx, fhdr.grid_miny, fhdr.grid_minz);


    break;

  case Mdvx::PROJ_OBLIQUE_STEREO :
    _supported = true;
    
    initObliqueStereo(fhdr.proj_origin_lat, fhdr.proj_origin_lon,
		      static_cast<double>(fhdr.proj_param[1]), 
		      static_cast<double>(fhdr.proj_param[2]),
		      fhdr.nx, fhdr.ny, fhdr.nz,
		      fhdr.grid_dx, fhdr.grid_dy, fhdr.grid_dz,
		      fhdr.grid_minx, fhdr.grid_miny, fhdr.grid_minz);
    break;

  case Mdvx::PROJ_MERCATOR :
    _supported = true;
    
    initMercator(fhdr.proj_origin_lat, fhdr.proj_origin_lon,
	       fhdr.nx, fhdr.ny, fhdr.nz,
	       fhdr.grid_dx, fhdr.grid_dy, fhdr.grid_dz,
	       fhdr.grid_minx, fhdr.grid_miny, fhdr.grid_minz);
    break;

  default:
  {
    // for unsupported projection, use latlon since this is
    // a null operation

    cerr << "WARNING - MdvxPjg::initFromHdrs." << endl;
    cerr << "MDV proj type " << fhdr.proj_type << " not supported." << endl;

    initLatlon(fhdr.nx, fhdr.ny, fhdr.nz,
	       fhdr.grid_dx, fhdr.grid_dy, fhdr.grid_dz,
	       fhdr.grid_minx, fhdr.grid_miny, fhdr.grid_minz);
  }
  } /* endswitch - fhdr.proj_type */
  
  _loadCoordFromFieldHdr(fhdr);
}


void MdvxPjg::init(const Mdvx::coord_t &coord)
{
  _supported = false;
  
  switch (coord.proj_type)
  {
  case Mdvx::PROJ_FLAT :
    _supported = true;
    
    initFlat(coord.proj_origin_lat, coord.proj_origin_lon,
	     coord.proj_params.flat.rotation,
	     coord.nx, coord.ny, coord.nz,
	     coord.dx, coord.dy, coord.dz,
	     coord.minx, coord.miny, coord.minz);

    break;
    
  case Mdvx::PROJ_LATLON :
    _supported = true;
    
    initLatlon(coord.nx, coord.ny, coord.nz,
	       coord.dx, coord.dy, coord.dz,
	       coord.minx, coord.miny, coord.minz);

    break;
    
  case Mdvx::PROJ_LAMBERT_CONF :
    _supported = true;
    
    initLc2(coord.proj_origin_lat, coord.proj_origin_lon,
	    coord.proj_params.lc2.lat1, coord.proj_params.lc2.lat2,
	    coord.nx, coord.ny, coord.nz,
	    coord.dx, coord.dy, coord.dz,
	    coord.minx, coord.miny, coord.minz);

    break;
    
  case Mdvx::PROJ_POLAR_RADAR :
    _supported = true;
    
    initPolarRadar(coord.proj_origin_lat, coord.proj_origin_lon,
		   coord.nx, coord.ny, coord.nz,
		   coord.dx, coord.dy, coord.dz,
		   coord.minx, coord.miny, coord.minz);

    break;
    
  case Mdvx::PROJ_MERCATOR :
    _supported = true;
    
    initMercator(coord.proj_origin_lat, coord.proj_origin_lon,
	       coord.nx, coord.ny, coord.nz,
	       coord.dx, coord.dy, coord.dz,
	       coord.minx, coord.miny, coord.minz);

    break;
    
  default :
    cerr << "WARNING - MdvxPjg::init from coord" << endl;
    cerr << "MDV proj type " << coord.proj_type << " not supported." << endl;

    break;
  } /* endswitch - coord.proj_type */
  
}


void MdvxPjg::init(const Pjg &pjg)
{
  MEM_zero(_coord);
  _supported = false;
  
  switch (pjg.getProjType())
  {
  case PjgTypes::PROJ_FLAT :
    initFlat(pjg.getOriginLat(), pjg.getOriginLon(),
	     pjg.getRotation(),
	     pjg.getNx(), pjg.getNy(), pjg.getNz(),
	     pjg.getDx(), pjg.getDy(), pjg.getDz(),
	     pjg.getMinx(), pjg.getMiny(), pjg.getMinz());
    _supported = true;
    break;
    
  case PjgTypes::PROJ_LATLON :
    initLatlon(pjg.getNx(), pjg.getNy(), pjg.getNz(),
	       pjg.getDx(), pjg.getDy(), pjg.getDz(),
	       pjg.getMinx(), pjg.getMiny(), pjg.getMinz());
    _supported = true;
    break;
    
  case PjgTypes::PROJ_LC1 :
    initLc2(pjg.getOriginLat(), pjg.getOriginLon(),
	    pjg.getLat1(), pjg.getLat1(),
	    pjg.getNx(), pjg.getNy(), pjg.getNz(),
	    pjg.getDx(), pjg.getDy(), pjg.getDz(),
	    pjg.getMinx(), pjg.getMiny(), pjg.getMinz());
    _supported = true;
    break;
    
  case PjgTypes::PROJ_LC2 :
    initLc2(pjg.getOriginLat(), pjg.getOriginLon(),
	    pjg.getLat1(), pjg.getLat2(),
	    pjg.getNx(), pjg.getNy(), pjg.getNz(),
	    pjg.getDx(), pjg.getDy(), pjg.getDz(),
	    pjg.getMinx(), pjg.getMiny(), pjg.getMinz());
    _supported = true;
    break;
    
  case PjgTypes::PROJ_POLAR_RADAR :
    initPolarRadar(pjg.getOriginLat(), pjg.getOriginLon(),
		   pjg.getNx(), pjg.getNy(), pjg.getNz(),
		   pjg.getDx(), pjg.getDy(), pjg.getDz(),
		   pjg.getMinx(), pjg.getMiny(), pjg.getMinz());
    _supported = true;
    break;
    
  case PjgTypes::PROJ_MERCATOR :
    initMercator(pjg.getOriginLat(), pjg.getOriginLon(),
	       pjg.getNx(), pjg.getNy(), pjg.getNz(),
	       pjg.getDx(), pjg.getDy(), pjg.getDz(),
	       pjg.getMinx(), pjg.getMiny(), pjg.getMinz());
    _supported = true;
    break;
    
  default:
    cerr << "WARNING - MdvxPjg::init from Pjg" << endl;
    cerr << "Pjg proj type " << pjg.getProjType() << " not supported." << endl;
  }
}


/**********************************************************************
 * initFlat() - Initialize flat earth projection.
 */

void MdvxPjg::initFlat(const double origin_lat, const double origin_lon,
		       const double rotation,
		       const int nx, const int ny, const int nz,
		       const double dx, const double dy, const double dz,
		       const double minx, const double miny, const double minz)
{
  Pjg::initFlat(origin_lat, origin_lon, rotation,
		nx, ny, nz, dx, dy, dz, minx, miny, minz);
  
  MEM_zero(_coord);
  _coord.proj_type = Mdvx::PROJ_FLAT;
  _coord.proj_origin_lat = origin_lat;
  _coord.proj_origin_lon = origin_lon;
  _coord.proj_params.flat.rotation = rotation;
  _coord.nx = nx;
  _coord.ny = ny;
  _coord.nz = nz;
  _coord.dx = dx;
  _coord.dy = dy;
  _coord.dz = dz;
  _coord.minx = minx;
  _coord.miny = miny;
  _coord.minz = minz;
}


/**********************************************************************
 * initLc2() - Initialize lambert conformal projection with two lats.
 */

void MdvxPjg::initLc2(const double origin_lat, const double origin_lon,
		      const double lat1, const double lat2,
		      const int nx, const int ny, const int nz,
		      const double dx, const double dy, const double dz,
		      const double minx, const double miny, const double minz)
{
  Pjg::initLc2(origin_lat, origin_lon, lat1, lat2,
	       nx, ny, nz, dx, dy, dz, minx, miny, minz);
  
  MEM_zero(_coord);
  _coord.proj_type = Mdvx::PROJ_LAMBERT_CONF;
  _coord.proj_origin_lat = origin_lat;
  _coord.proj_origin_lon = origin_lon;
  _coord.proj_params.lc2.lat1 = lat1;
  _coord.proj_params.lc2.lat2 = lat2;
  _coord.nx = nx;
  _coord.ny = ny;
  _coord.nz = nz;
  _coord.dx = dx;
  _coord.dy = dy;
  _coord.dz = dz;
  _coord.minx = minx;
  _coord.miny = miny;
  _coord.minz = minz;
}


/**********************************************************************
 * initLatlon() - Initialize latlon projection.
 */

void MdvxPjg::initLatlon(const int nx, const int ny, const int nz,
			 const double dx, const double dy, const double dz,
			 const double minx, const double miny,
			 const double minz)
{
  Pjg::initLatlon(nx, ny, nz, dx, dy, dz, minx, miny, minz);
  
  MEM_zero(_coord);
  _coord.proj_type = Mdvx::PROJ_LATLON;
  _coord.nx = nx;
  _coord.ny = ny;
  _coord.nz = nz;
  _coord.dx = dx;
  _coord.dy = dy;
  _coord.dz = dz;
  _coord.minx = minx;
  _coord.miny = miny;
  _coord.minz = minz;
}


/**********************************************************************
 * initPolarRadar() - Initialize polar radar projection.
 */

void MdvxPjg::initPolarRadar(const double origin_lat, const double origin_lon,
			     const int nx, const int ny, const int nz,
			     const double dx, const double dy, const double dz,
			     const double minx, const double miny,
			     const double minz)
{
  Pjg::initPolarRadar(origin_lat, origin_lon,
		      nx, ny, nz, dx, dy, dz, minx, miny, minz);
  
  MEM_zero(_coord);
  _coord.proj_type = Mdvx::PROJ_POLAR_RADAR;
  _coord.proj_origin_lat = origin_lat;
  _coord.proj_origin_lon = origin_lon;
  _coord.nx = nx;
  _coord.ny = ny;
  _coord.nz = nz;
  _coord.dx = dx;
  _coord.dy = dy;
  _coord.dz = dz;
  _coord.minx = minx;
  _coord.miny = miny;
  _coord.minz = minz;
}


/**********************************************************************
 * initPolarStereo() - Initialize polar stereographic projection.
 */

void MdvxPjg::initPolarStereo
(const double tangent_lon,
 const PjgTypes::pole_type_t pole_type /* = PjgTypes::POLE_NORTH*/,
 const double central_scale /* = 1.0 */,
 const int nx /* = 1*/,
 const int ny /* = 1*/,
 const int nz /* = 1*/,
 const double dx /* = 1.0*/,
 const double dy /* = 1.0*/,
 const double dz /* = 1.0*/,
 const double minx /* = 0.0*/,
 const double miny /* = 0.0*/,
 const double minz /* = 0.0*/ )
  
{

  Pjg::initPolarStereo(tangent_lon, pole_type, central_scale,
		       nx, ny, nz, dx, dy, dz, minx, miny, minz);
  
  MEM_zero(_coord);
  _coord.proj_type = Mdvx::PROJ_POLAR_STEREO;
  if (pole_type == PjgTypes::POLE_NORTH)
    _coord.proj_origin_lat = 90.0;
  else
    _coord.proj_origin_lat = -90.0;
  _coord.proj_origin_lon = tangent_lon;
  _coord.nx = nx;
  _coord.ny = ny;
  _coord.nz = nz;
  _coord.dx = dx;
  _coord.dy = dy;
  _coord.dz = dz;
  _coord.minx = minx;
  _coord.miny = miny;
  _coord.minz = minz;
}

/**********************************************************************
 * initObliqueStereo() - Initialize oblique stereographic projection.
 */

void MdvxPjg::initObliqueStereo(const double origin_lat,
				const double origin_lon,
				const double tangent_lat,
				const double tangent_lon,
				const int nx /* = 1*/,
				const int ny /* = 1*/,
				const int nz /* = 1*/,
				const double dx /* = 1.0*/,
				const double dy /* = 1.0*/,
				const double dz /* = 1.0*/,
				const double minx /* = 0.0*/,
				const double miny /* = 0.0*/,
				const double minz /* = 0.0*/ )
{
  Pjg::initObliqueStereo(origin_lat, origin_lon, tangent_lat, tangent_lon,
			 nx, ny, nz, dx, dy, dz, minx, miny, minz);
  
  MEM_zero(_coord);
  _coord.proj_type = Mdvx::PROJ_OBLIQUE_STEREO;
  _coord.proj_origin_lat = origin_lat;
  _coord.proj_origin_lon = origin_lon;
  _coord.nx = nx;
  _coord.ny = ny;
  _coord.nz = nz;
  _coord.dx = dx;
  _coord.dy = dy;
  _coord.dz = dz;
  _coord.minx = minx;
  _coord.miny = miny;
  _coord.minz = minz;
}
  

/**********************************************************************
 * initMercator() - Initialize Mercator projection.
 */

void MdvxPjg::initMercator(const double origin_lat, const double origin_lon,
			 const int nx, const int ny, const int nz,
			 const double dx, const double dy, const double dz,
			 const double minx, const double miny,
			 const double minz)
{
  Pjg::initMercator(origin_lat, origin_lon, nx, ny, nz, dx, dy, dz, minx, miny, minz);
  
  MEM_zero(_coord);
  _coord.proj_type = Mdvx::PROJ_MERCATOR;
  _coord.proj_origin_lat = origin_lat;
  _coord.proj_origin_lon = origin_lon;
  _coord.nx = nx;
  _coord.ny = ny;
  _coord.nz = nz;
  _coord.dx = dx;
  _coord.dy = dy;
  _coord.dz = dz;
  _coord.minx = minx;
  _coord.miny = miny;
  _coord.minz = minz;
}


/**********************************************************************
 * setSensorPosn() - Set the sensor position.  Ht is in km MSL.
 */

void MdvxPjg::setSensorPosn(double sensor_lat,
			    double sensor_lon,
			    double sensor_ht)
{
  _coord.sensor_lat = sensor_lat;
  _coord.sensor_lon = sensor_lon;
  _coord.sensor_z = sensor_ht;
  
  switch (_coord.proj_type)
  {
  case Mdvx::PROJ_FLAT :
  {
    MdvxPjg proj;
    double xx, yy;
    proj.initFlat(_coord.proj_origin_lat, _coord.proj_origin_lon,
		  _coord.proj_params.flat.rotation,
		  _coord.nx, _coord.ny, _coord.nz,
		  _coord.dx, _coord.dy, _coord.dz,
		  _coord.minx, _coord.miny, _coord.minz);
    proj.latlon2xy(_coord.sensor_lat, _coord.sensor_lon, xx, yy);
    _coord.sensor_x = xx;
    _coord.sensor_y = yy;

    break;
  }
  
  case Mdvx::PROJ_LATLON :
    _coord.sensor_x = _coord.sensor_lon;
    _coord.sensor_y = _coord.sensor_lat;
    break;
    
  case Mdvx::PROJ_POLAR_RADAR :
    _coord.sensor_x = _coord.proj_origin_lon;
    _coord.sensor_y = _coord.proj_origin_lat;
    break;
    
  case Mdvx::PROJ_MERCATOR :
  {
    MdvxPjg proj;
    double xx, yy;
    proj.initMercator(_coord.proj_origin_lat, _coord.proj_origin_lon,
		  _coord.nx, _coord.ny, _coord.nz,
		  _coord.dx, _coord.dy, _coord.dz,
		  _coord.minx, _coord.miny, _coord.minz);
    proj.latlon2xy(_coord.sensor_lat, _coord.sensor_lon, xx, yy);
    _coord.sensor_x = xx;
    _coord.sensor_y = yy;

    break;
  }
  
  default:
    _coord.sensor_x = 0.0;
    _coord.sensor_y = 0.0;
    break;
    
  } /* endswitch - _coord.proj_type */

}
  

/**********************************************************************
 * getProjType() - Retrieve the Mdvx projection type.
 */

int MdvxPjg::getProjType(void) const
{
  switch (Pjg::getProjType())
  {
  case PjgTypes::PROJ_LATLON :
    return Mdvx::PROJ_LATLON;
  case PjgTypes::PROJ_LC1 :
  case PjgTypes::PROJ_LC2 :
    return Mdvx::PROJ_LAMBERT_CONF;
  case PjgTypes::PROJ_MERCATOR :
    return Mdvx::PROJ_MERCATOR;
  case PjgTypes::PROJ_POLAR_STEREO :
    return Mdvx::PROJ_POLAR_STEREO;
  case PjgTypes::PROJ_POLAR_ST_ELLIP :
    return Mdvx::PROJ_POLAR_ST_ELLIP;
  case PjgTypes::PROJ_CYL_EQUIDIST :
    return Mdvx::PROJ_CYL_EQUIDIST;
  case PjgTypes::PROJ_FLAT :
    return Mdvx::PROJ_FLAT;
  case PjgTypes::PROJ_POLAR_RADAR :
    return Mdvx::PROJ_POLAR_RADAR;
  case PjgTypes::PROJ_RADIAL :
    return Mdvx::PROJ_RADIAL;
    
  case PjgTypes::PROJ_INVALID :
  case PjgTypes::PROJ_UNKNOWN :
  default:
    return Mdvx::PROJ_UNKNOWN;
  } /* end switch - Pjg::getProjType() */
  
}
  

/**********************************************************************
 * syncToFieldHdr() - Synchronize field header with info from this
 *                    object.
 */

void MdvxPjg::syncToFieldHdr(Mdvx::field_header_t &fhdr) const
{
  fhdr.proj_origin_lat = _coord.proj_origin_lat;
  fhdr.proj_origin_lon = _coord.proj_origin_lon;
  
  fhdr.proj_type = _coord.proj_type;
  if (_coord.proj_type == Mdvx::PROJ_FLAT)
  {
    fhdr.proj_rotation = _coord.proj_params.flat.rotation;
    fhdr.proj_param[0] = _coord.proj_params.flat.rotation;
  }
  else if (_coord.proj_type == Mdvx::PROJ_LAMBERT_CONF)
  {
    fhdr.proj_param[0] = _coord.proj_params.lc2.lat1;
    fhdr.proj_param[1] = _coord.proj_params.lc2.lat2;
  }
  
  fhdr.grid_minx = _coord.minx;
  fhdr.grid_miny = _coord.miny;
  fhdr.grid_minz = _coord.minz;

  fhdr.grid_dx = _coord.dx;
  fhdr.grid_dy = _coord.dy;
  fhdr.grid_dz = _coord.dz;

  fhdr.nx = _coord.nx;
  fhdr.ny = _coord.ny;
  fhdr.nz = _coord.nz;

  fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * fhdr.data_element_nbytes;
}


/**********************************************************************
 * syncXyToFieldHdr() - Synchronize field header with (x,y) info from
 *                      this object.
 */

void MdvxPjg::syncXyToFieldHdr(Mdvx::field_header_t &fhdr) const
{
  fhdr.proj_origin_lat = _coord.proj_origin_lat;
  fhdr.proj_origin_lon = _coord.proj_origin_lon;
  
  fhdr.proj_type = _coord.proj_type;
  if (_coord.proj_type == Mdvx::PROJ_FLAT)
  {
    fhdr.proj_rotation = _coord.proj_params.flat.rotation;
    fhdr.proj_param[0] = _coord.proj_params.flat.rotation;
  }
  else if (_coord.proj_type == Mdvx::PROJ_LAMBERT_CONF)
  {
    fhdr.proj_param[0] = _coord.proj_params.lc2.lat1;
    fhdr.proj_param[1] = _coord.proj_params.lc2.lat2;
  }
  
  fhdr.grid_minx = _coord.minx;
  fhdr.grid_miny = _coord.miny;

  fhdr.grid_dx = _coord.dx;
  fhdr.grid_dy = _coord.dy;

  fhdr.nx = _coord.nx;
  fhdr.ny = _coord.ny;

  fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * fhdr.data_element_nbytes;
}


/**********************************************************************
 * syncToHdrs() - Synchronize master and field header with info from
 *                this object.
 */

void MdvxPjg::syncToHdrs(Mdvx::master_header_t &mhdr,
			 Mdvx::field_header_t &fhdr) const
{
  syncToFieldHdr(fhdr);

  mhdr.max_nx = MAX(mhdr.max_nx, fhdr.nx);
  mhdr.max_ny = MAX(mhdr.max_nx, fhdr.ny);
  mhdr.max_nz = MAX(mhdr.max_nx, fhdr.nz);

  mhdr.sensor_lon = _coord.sensor_lon;
  mhdr.sensor_lat = _coord.sensor_lat;
  mhdr.sensor_alt = _coord.sensor_z;
}


/**********************************************************************
 * print() - Print the object information to the given print stream.
 */

void MdvxPjg::print(ostream &out) const
{
  out << "MdvxPjg object" << endl;
  out << "---------------" << endl;

  printCoord(_coord, out);
}

/**********************************************************************
 * printCoord() - Print the given coord structure to the given print
 *                stream.
 */

#define BOOL_STR(a) (a == 0 ? "false" : "true")

void MdvxPjg::printCoord(const Mdvx::coord_t &coord,
			 ostream &out)
{
  out << "MdvxPjg coord parameters" << endl;
  out << "-------------------------" << endl;
  
  switch (coord.proj_type)
  {
  case Mdvx::PROJ_FLAT :
    out << "  proj_type: flat" << endl;
    break;
    
  case Mdvx::PROJ_LATLON :
    out << "  proj_type: latlon" << endl;
    break;
    
  case Mdvx::PROJ_LAMBERT_CONF :
    out << "  proj_type: Lambert conformal" << endl;
    break;
    
  case Mdvx::PROJ_POLAR_RADAR :
    out << "  proj_type: polar radar" << endl;
    break;
    
  case Mdvx::PROJ_RADIAL :
    out << "  proj_type: radial" << endl;
    break;
    
  case Mdvx::PROJ_MERCATOR :
    out << "  proj_type: Mercator" << endl;
    break;

  case Mdvx::PROJ_OBLIQUE_STEREO :
    out << "  proj_type: oblique stereo" << endl;
    break;

  default:
    out << "  proj_type: UNKNOWN: " << coord.proj_type << endl;
    break;
  } /* endswitch - coord.proj_type */
  
  out << "  origin latitude: " << coord.proj_origin_lat << endl;
  out << "  origin longitude: " << coord.proj_origin_lon << endl;
  out << "  grid rotation: " << coord.proj_params.flat.rotation << endl;

  out << "  nx, ny, nz: " << coord.nx << ", " << coord.ny
      << ", " << coord.nz << endl;

  out << "  minx, miny, minz: " << coord.minx << ", " << coord.miny
      << ", " << coord.minz << endl;

  out << "  maxx, maxy: "
      << coord.minx  + (coord.nx - 1) *coord.dx  << ", "
      << coord.miny  + (coord.ny - 1) *coord.dy  << endl;

  out << "  dx, dy, dz: " << coord.dx << ", " << coord.dy
      << ", " << coord.dz << endl;

  out << "  sensor_x, sensor_y, sensor_z: "
      << coord.sensor_x << ", " << coord.sensor_y
      << ", " << coord.sensor_z << endl;

  out << "  sensor_lat, sensor_lon: "
      << coord.sensor_lat << ", " << coord.sensor_lon << endl;

  out << "  dz_constant: " << BOOL_STR(coord.dz_constant) << endl;

  out << "  x units: " << coord.unitsx << endl;
  out << "  y units: " << coord.unitsy << endl;
  out << "  z units: " << coord.unitsz << endl;
  
}


/**********************************************************************
 * xyIndex2arrayIndex() - Computes the index into the data array.
 *
 * Returns the calculated array index on success, -1 on failure
 * (data outside grid).
 */

int MdvxPjg::xyIndex2arrayIndex(const int ix, const int iy, const int iz) const
{
  if (ix < 0 || ix >= _coord.nx ||
      iy < 0 || iy >= _coord.ny ||
      iz < 0 || iz >= _coord.nz)
    return -1;
  
  switch(_dataOrdering)
  {
  case Mdvx::ORDER_XYZ :
    return ix + (iy * _coord.nx) + (iz * _coord.nx * _coord.ny);
    
  case Mdvx::ORDER_YXZ :
    return iy + (ix * _coord.ny) + (iz * _coord.ny * _coord.nx); 
    
  case Mdvx::ORDER_XZY :
    return ix + (iz * _coord.nx) + (iy * _coord.nx * _coord.nz); 
    
  case Mdvx::ORDER_YZX :
    return iy + (iz * _coord.ny) + (ix * _coord.ny * _coord.nz); 
    
  case Mdvx::ORDER_ZXY :
    return iz + (ix * _coord.nz) + (iy * _coord.nz * _coord.nx); 
    
  case Mdvx::ORDER_ZYX :
    return iz + (iy * _coord.nz) + (ix * _coord.nz * _coord.ny); 
  }
  
  return -1;
}

  
///////////////////////
// protected methods //
///////////////////////

/**********************************************************************
 * _init() - Initialize from scratch.
 */

void MdvxPjg::_init()
{
  _coord.nx = 1;
  _coord.ny = 1;
  _coord.nz = 1;
  _coord.dx = 1.0;
  _coord.dy = 1.0;
  _coord.dz = 1.0;
  _coord.dz_constant = 1;
  _coord.nbytes_char = 3 * MDV_COORD_UNITS_LEN;

  initLatlon(_coord.nx, _coord.ny, _coord.nz,
	     _coord.dx, _coord.dy, _coord.dz,
	     _coord.minx, _coord.miny, _coord.minz);
}


/**********************************************************************
 * _loadCoordFromFieldHdr() - Load up coord_t struct from field header
 */

void MdvxPjg::_loadCoordFromFieldHdr(const Mdvx::field_header_t &fhdr)
{
  _coord.proj_origin_lat = fhdr.proj_origin_lat;
  _coord.proj_origin_lon = fhdr.proj_origin_lon;

  _coord.proj_type = (Mdvx::projection_type_t) fhdr.proj_type;
  if (_coord.proj_type == Mdvx::PROJ_FLAT)
  {
    _coord.proj_params.flat.rotation = fhdr.proj_param[0];
  }
  else if (_coord.proj_type == Mdvx::PROJ_LAMBERT_CONF)
  {
    _coord.proj_params.lc2.lat1 = fhdr.proj_param[0];
    _coord.proj_params.lc2.lat2 = fhdr.proj_param[1];
  }
  
  _coord.minx = fhdr.grid_minx;
  _coord.miny = fhdr.grid_miny;
  _coord.minz = fhdr.grid_minz;

  _coord.dx = fhdr.grid_dx;
  _coord.dy = fhdr.grid_dy;
  _coord.dz = fhdr.grid_dz;

  _coord.nx = fhdr.nx;
  _coord.ny = fhdr.ny;
  _coord.nz = fhdr.nz;

  switch (_coord.proj_type)
  {
  case Mdvx::PROJ_FLAT:
    _coord.sensor_x = 0.0;
    _coord.sensor_y = 0.0;
    break;
    
  case Mdvx::PROJ_LATLON :
    _coord.sensor_x = _coord.sensor_lon;
    _coord.sensor_y = _coord.sensor_lat;
    break;
    
  case Mdvx::PROJ_POLAR_RADAR :
    _coord.sensor_x = _coord.proj_origin_lon;
    _coord.sensor_y = _coord.proj_origin_lat;
    break;
    
  default:
    _coord.sensor_x = 0.0;
    _coord.sensor_y = 0.0;
  } /* endswitch - _coord.proj_type */

  if (fhdr.vlevel_type == Mdvx::VERT_TYPE_Z)
    _coord.dz_constant = 1;
  else
    _coord.dz_constant = 0;

  _coord.nbytes_char = MDV_N_COORD_LABELS * MDV_COORD_UNITS_LEN;
  
  strcpy(_coord.unitsx, "");
  strcpy(_coord.unitsy, "");
  strcpy(_coord.unitsz, "");
  
  switch (_coord.proj_type)
  {
  case Mdvx::PROJ_FLAT:
  case Mdvx::PROJ_LAMBERT_CONF :
  case Mdvx::PROJ_MERCATOR:
    strcpy(_coord.unitsx, "km");
    strcpy(_coord.unitsy, "km");
    break;

  case Mdvx::PROJ_LATLON :
    strcpy(_coord.unitsx, "deg");
    strcpy(_coord.unitsy, "deg");
    break;
    
  case Mdvx::PROJ_POLAR_RADAR :
  case Mdvx::PROJ_RHI_RADAR :
    strcpy(_coord.unitsx, "km");
    strcpy(_coord.unitsy, "deg");
    strcpy(_coord.unitsz, "deg");
    break;
    
  case Mdvx::PROJ_RADIAL :
    strcpy(_coord.unitsx, "m");
    strcpy(_coord.unitsy, "deg");
    break;
    
  default:
    strcpy(_coord.unitsx, "unknown");
    strcpy(_coord.unitsy, "unknown");
    break;
  } /* endswitch - _coord.proj_type */

  switch (fhdr.vlevel_type)
  {
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
  default:
    break;
  } /* endswitch - fhdr.vlevel_type */
}


/**********************************************************************
 * _loadCoordFromMasterHdr() - Load up coord_t struct from master header.
 */

void MdvxPjg::_loadCoordFromMasterHdr(const Mdvx::master_header_t &mhdr)
{
  _coord.sensor_lon = mhdr.sensor_lon;
  _coord.sensor_lat = mhdr.sensor_lat;
  _coord.sensor_z = mhdr.sensor_alt;
}
