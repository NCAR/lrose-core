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
// Mdv/MdvxProj.hh
//
// This class is used to perform projective geometry on the data grid
// coordinates for an Mdvx field.
//
// If you use the default constructor, the projection will be set 
// to latlon. You must call one of the init() functions if you want
// alternative behavior.
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// September 1999
//
////////////////////////////////////////////////////////////////////

#ifndef MdvxProj_hh
#define MdvxProj_hh

#include <Mdv/Mdvx.hh>
#include <euclid/PjgMath.hh>
using namespace std;
class DsMdvxMsg;

class MdvxProj
{

public:

  friend class DsMdvxMsg;

  ///////////////////////
  // default constructor
  
  MdvxProj();
  
  /////////////////////////////////////////////////
  // construct from first field of Mdvx object
  
  MdvxProj(const Mdvx &mdvx);
  
  ///////////////////////////////////////////
  // Construct from master and field headers

  MdvxProj(const Mdvx::master_header_t &mhdr,
	   const Mdvx::field_header_t &fhdr);

  //////////////////////////////////////////////////////////
  // Construct from field header
  //
  // Sensor position will not be filled in, since this is
  // only available from the master header.

  MdvxProj(const Mdvx::field_header_t &fhdr);
  
  ////////////////////////////
  // Construct from coord struct

  MdvxProj(const Mdvx::coord_t &coord);

  ////////////////////////////
  // Copy constructor

  MdvxProj(const MdvxProj &rhs);

  ///////////////////////
  // destructor

  virtual ~MdvxProj();
  
  // assignment
  
  MdvxProj & operator=(const MdvxProj &rhs);
  
  // equality / inequality

  bool operator==(const MdvxProj &other) const;
  bool operator!=(const MdvxProj &other) const;
  
  /////////////////////////////////////
  // check that projection is supported
  //
  // Return true is proj type is supported, false otherwise
  //
  // Useful for checking if the constructor was given data which
  // can be used by this class.
  
  bool supported();
    
  /////////////////////////////////////////////////
  // initialize from first field of Mdvx object
  //
  
  void init(const Mdvx &mdvx);
  
  ////////////////////////////////////////////////
  // initialize from Mdvx master and field headers

  void init(const Mdvx::master_header_t &mhdr,
	    const Mdvx::field_header_t &fhdr);
  
  /////////////////////////////////////
  // initialize from Mdvx field header
  //
  // Sensor position will not be filled in, since this is only available
  // from the master header.

  void init(const Mdvx::field_header_t &fhdr);

  ///////////////////////////////
  // initialize from coord struct
  
  void init(const Mdvx::coord_t &coord);
  
  ///////////////////////////////
  // initialize projections
  
  // initialize latlon projection

  void initLatlon(double origin_lon = 0.0);

  // initialize Azimuthal Equidistant projection
  // This is the same as the flat projection
  
  void initAzimEquiDist(double origin_lat,
			double origin_lon,
			double rotation);

  // initialize radar flat projection
  // This is the same as the Azimuthal Equidistant projection
  
  void initFlat(double origin_lat,
		double origin_lon,
		double rotation) {
    initAzimEquiDist(origin_lat, origin_lon, rotation);
  }

  // initialize lambert conformal projection
  
  void initLambertConf(double origin_lat,
		       double origin_lon,
		       double lat1,
		       double lat2);

  void initLc2(double origin_lat,
	       double origin_lon,
	       double lat1,
	       double lat2) {
    initLambertConf(origin_lat, origin_lon, lat1, lat2);
  }

  // initialize polar radar projection
  
  void initPolarRadar(double origin_lat,
		      double origin_lon);

  // initialize polar stereographic projection.

  void initPolarStereo(double tangent_lon, 
		       Mdvx::pole_type_t poleType = Mdvx::POLE_NORTH,
                       double central_scale = 1.0);


  // deprecated. Instead, use initPolarStereo() above,
  // followed by setOffsetOrigin()

  void initPolarStereo(double origin_lat, 
		       double origin_lon,
		       double tangent_lon, 
		       Mdvx::pole_type_t poleType = Mdvx::POLE_NORTH,
                       double central_scale = 1.0);

  // initialize oblique stereographic projection.

  void initStereographic(double tangent_lat, 
                         double tangent_lon,
                         double central_scale = 1.0);
  
  // deprecated

  void initObliqueStereo(double origin_lat, 
			 double origin_lon,
			 double tangent_lat, 
			 double tangent_lon,
                         double central_scale = 1.0);

  // initialize Mercator projection
  
  void initMercator(double origin_lat = 0.0,
                    double origin_lon = 0.0);

  // initialize Transverse Mercator projection
  
  void initTransMercator(double origin_lat = 0.0,
                         double origin_lon = 0.0,
                         double central_scale = 1.0);

  // initialize albers equal area

  void initAlbers(double origin_lat,
                  double origin_lon,
                  double lat1,
                  double lat2);

  // initialize lambert azimuthal equal area
  
  void initLambertAzim(double origin_lat,
                       double origin_lon);

  // initialize Vertical Perspective (satellite view)
  // perspective point radius is in km from earth center
  
  void initVertPersp(double origin_lat,
                     double origin_lon,
                     double persp_radius);

  /////////////////////////////////////////////////////////////
  // set the grid.
  // This is not required if you are initializing from
  // MDV headers or the coord struct.

  void setGrid(int nx, int ny,
               double dx, double dy,
               double minx, double miny);
  
  /////////////////////////////////////////////////////////////
  // set sensor position
  // Ht is in km MSL

  void setSensorPosn(double sensor_lat,
		     double sensor_lon,
                     double sensor_ht = 0.0);
  
  ////////////////////////////////////////////////////////////
  // conditioning the longitude
  // Sometimes the computed longitude differs from the origin
  // or desired reference point by about 360 degrees.
  // These options allow you to force the computed longitude
  // to be in the desired range
  
  // Set longitude conditioning with respect to origin.
  // If set, xy2latlon() will return a longitude within 180 degrees
  // of the longitude origin of the projection.
  
  void setConditionLon2Origin(bool state = true);
  
  // Set longitude conditioning with respect to reference lon.
  // If set, xy2latlon() will return a longitude within 180 degrees
  // of the reference longitude.
  
  void setConditionLon2Ref(bool state = true,
			   double reference_lon = 0.0);
  
  // Condition longitude to be in same hemisphere as origin lon
  
  double conditionLon2Origin(double lon) const;

  // Condition longitude to be in same hemisphere as reference lon
  
  static double conditionLon2Ref(double lon, double ref);
  double getReferenceLon() const { return _reference_lon; }
  
  /////////////////////////////////////////////////////////////////////
  /// Set offset origin by specifying lat/lon.
  /// Normally the offset origin and projection origin are co-located
  /// This will compute false_northing and false_easting.
  ///   X = x_from_proj_origin + false_easting
  ///   Y = y_from_proj_origin + false_northing

  void setOffsetOrigin(double offset_lat,
                       double offset_lon);
  
  /////////////////////////////////////////////////////////////////////
  /// Set offset origin by specifying false_northing and false_easting.
  /// Normally the offset origin and projection origin are co-located
  /// This will compute offset lat and lon, which is the point:
  ///   (x = -false_northing, y = -false_easting)

  void setOffsetCoords(double false_northing,
                       double false_easting);
  
  ////////////////////////////////////////////////////////
  // Coordinate transformation functions.
  // Convert from XY to Lat/Lon and vice versa.
  // These rely on the projection having been initialized.

  // latlon 2 xy
  // most projections: x, y in km, z ignored
  // latlon projection: x, y in deg, z ignored
  // polar radar projection:
  //  x is range in km, y is az in deg, z is elevation in deg

  void latlon2xy(double lat, double lon,
		 double  &x, double &y,
		 double z = -9999.0) const;

  // xy 2 latlon
  // most projections: x, y in km, z ignored
  // latlon projection: x, y in deg, z ignored
  // polar radar projection:
  //  x is range in km, y is az in deg, z is elevation in deg
  
  void xy2latlon(double x, double y,
		 double &lat, double &lon,
		 double z = -9999.0) const;

  ///////////////////////////////////////////////////////////////////
  // Compute the the data x, y indices for the given lat/lon location.
  // If wrap_lon is true, the longitude will be wrapped across the 
  // domain for LATLON projections only.
  // These rely on both the projection and grid having been initialized.
  // returns 0 on success, -1 on failure (data outside grid)
  //
  // See xy2latlon and latlon2xy for details of transformation units
  
  int latlon2xyIndex(double lat, double lon,
		     int &x_index, int &y_index,
		     bool wrap_lon = false,
                     double z = -9999.0) const;
  
  int latlon2xyIndex(double lat, double lon,
		     double &x_index, double &y_index,
		     bool wrap_lon = false,
                     double z = -9999.0) const;

  ///////////////////////////////////////////////////////////////
  // Compute the index into the data array.
  // If wrap_lon is true, the longitude will be wrapped across the 
  // domain for LATLON projections only.
  // These rely on both the projection and grid having been initialized.
  // returns 0 on success, -1 on failure (data outside grid)

  int latlon2arrayIndex(double lat, double lon,
			int64_t &array_index,
			bool wrap_lon = false,
                        double z = -9999.0) const;

  // Compute the lat & lon given ix and iy rel to grid.
  
  void xyIndex2latlon(int ix, int iy,
		      double &lat, double &lon,
                      double z = -9999.0) const;
  
  void xyIndex2latlon(double x, double y,
		      double &lat, double &lon,
                      double z = -9999.0) const;
  
  // Compute the the data x, y indices for the given x/y location.
  // If wrap_lon is true, the longitude will be wrapped across the 
  // domain for LATLON projections only.
  // returns 0 on success, -1 on failure (data outside grid)
  
  int xy2xyIndex(double xx, double yy,
		 int &x_index, int &y_index,
		 bool wrap_lon = false) const;

  // Compute the the data x, y indices, in doubles, for the given
  // x/y location.
  // If wrap_lon is true, the longitude will be wrapped across the 
  // domain for LATLON projections only.
  // returns 0 on success, -1 on failure (data outside grid)

  int xy2xyIndex(double lat, double lon,
		 double &x_index, double &y_index,
		 bool wrap_lon = false) const;

  ///////////////////////////////////////////////////////////
  // Convert the given distance to kilometers.  The distance
  // is assumed to be in the units appropriate to the projection.

  double x2km(double x) const;
  
  // Convert the given distance in kilometers to the appropriate
  // number of grid spaces along the X axis.

  double km2xGrid(double x_km) const;
  
  // Convert the given distance in kilometers to the appropriate
  // number of grid spaces along the Y axis.

  double km2yGrid(double y_km) const;
  
  // Convert the given distance in number of grid spaces along
  // the X axis to kilometers.

  double xGrid2km(double x_grid) const;
  
  // Convert the given distance in number of grid spaces along
  // the Y axis to kilometers.
  
  double yGrid2km(double y_grid) const;

  // get lat/lon of grid origin (as opposed to projection origin)
  
  void getGridOrigin(double &lat, double &lon) const;

  // Get the maximum and minimum lat, lon by going
  // around the edge of the grid. Computationally expensive
  // in some cases.

  void getEdgeExtrema(double &minLat, double &minLon,
		      double &maxLat, double &maxLon) const;
  
  // Synchronize master and field header with info from
  // this object.
  //
  // If data_element_nbytes has not been set in the field header
  // before this call, volume_size will be incorrectly computed, so
  // you will need to compute volume_size independently.

  void syncToHdrs(Mdvx::master_header_t &mhdr,
		  Mdvx::field_header_t &fhdr) const;

  // syncToFieldHdr
  // Synchronize field header with info from this object.
  //
  // If data_element_nbytes has not been set in the field header
  // before this call, volume_size will be incorrectly computed, so
  // you will need to compute volume_size independently.
  
  void syncToFieldHdr(Mdvx::field_header_t &fhdr) const;
  
  // syncXyToFieldHdr
  // Synchronize field header with (x,y) info from this object.
  //
  // If data_element_nbytes has not been set in the field header
  // before this call, volume_size will be incorrectly computed, so
  // you will need to compute volume_size independently.

  void syncXyToFieldHdr(Mdvx::field_header_t &fhdr) const;

  ////////////////////
  // print the object

  void print(ostream &out, const bool print_z = true) const;

  // print the coord struct

  static void printCoord(const Mdvx::coord_t &coord, ostream &out);

  ////////////////////////////////
  // get reference to coord struct

  const PjgMath &getPjgMath() const { return (*_math); }
  const Mdvx::coord_t &getCoord() const { return (_coord); }
  Mdvx::projection_type_t getProjType() const { return (_proj_type); }

protected:

  // engine for computations

  PjgMath *_math;

  // grid struct

  Mdvx::coord_t _coord;

  // projection type and origin

  Mdvx::projection_type_t _proj_type;
  double _origin_lat;
  double _origin_lon;

  // option to condition longitude

  bool _condition_lon;
  double _reference_lon;

  // private methods

  MdvxProj & _copy(const MdvxProj &rhs);
  
  void _clear();

  void _initToDefaults();

  void _initFromCoords();
  void _checkPolarStereoCoords();
  
  static void _coord2ProjParams(Mdvx::projection_type_t proj_type,
				const Mdvx::coord_t &coord,
				fl64 *proj_params);

  static void _projParams2Coord(Mdvx::projection_type_t proj_type,
				const fl64 *proj_params,
				Mdvx::coord_t &coord);
  
  void _loadCoordFromFieldHdr(const Mdvx::field_header_t &fhdr);
  
  void _loadCoordFromMasterHdr(const Mdvx::master_header_t &mhdr);

  double _km2grid(double km_distance,
		  double grid_delta) const;
  
  double _ll_km2xGrid(double km_distance) const;

  double _ll_km2yGrid(double km_distance) const;
  
  double _grid2km(double grid_distance,
		  double grid_delta) const;
  
  double _ll_xGrid2km(double grid_distance) const;
  
  double _ll_yGrid2km(double grid_distance) const;

private:

};

#endif
