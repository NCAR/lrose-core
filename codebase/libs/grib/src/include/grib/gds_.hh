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
///////////////////////////////////////////////////
// GDS - Grid Description Section
//
// Used wgrib by Wesley Ebisuzaki at NOAA as
// reference (http://wesley.wwb.noaa.gov/wgrib.html)
// 
// gds differs from GDS in that it does not use the Pjg
// class as the struct for setting and getting the
// projection. The Pjg class was not flexible enough
// to handle all the possible projections of a grib 
// file (in particular it couldn't handle lat/lon).
//
// Also this class must be extended for each grib projection,
// to handle that projections pack and unpack.
// The gribrecord class handles which gds extended class to call.
//    -JCraig-
//
//////////////////////////////////////////////////
#ifndef _GDS_
#define _GDS_

#include "GribSection.hh"
#include <vector>

using namespace std;

class gds: public GribSection {
public:

  typedef enum {
    DO_XY = 0,
    DO_YX = 1
  } data_ordering_t;

  typedef enum {
    GO_OTHER = 0,
    GO_SN_WE = 1,
    GO_NS_WE = 2,
    GO_SN_EW = 3,
    GO_NS_EW = 4
  } grid_orientation_t;


  // projection ID's -- octet 6 in GDS
  typedef enum {
    EQUIDISTANT_CYL_PROJ_ID = 0,
    MERCATOR_PROJ_ID = 1,
    GNOMONIC_PROJ_ID = 2,
    LAMBERT_CONFORMAL_PROJ_ID = 3,
    GAUSSIAN_LAT_LON_PROJ_ID = 4,
    POLAR_STEREOGRAPHIC_PROJ_ID = 5,
    UNIVERSAL_TRANSVERSE_MERCATOR_PROJ_ID = 6,
    SIMPLE_POLYCONIC_PROJ_ID = 7,
    ALBERS_EQUAL_AREA_PROJ_ID = 8,
    MILLER_CYLINDRICAL_PROJ_ID = 9,
    ROTATED_LAT_LON_PROJ_ID = 10,
    OBLIQUE_LAMBERT_PROJ_ID = 13
  } grid_type_t;


  struct gds_t {
    gds_t() {
      gridType = EQUIDISTANT_CYL_PROJ_ID;
      numVertical = 0;
      verticalOrPoints = 0;
      nx = 0;
      ny = 0;
      resMode = 0;
      lov = 0.0;
      dx = 0.0;
      dy = 0.0;
      scanMode = 0;
      originLat = 0.0;
      originLon = 0.0;
      latin1 = 0.0;
      latin2 = 0.0;
      lastLat = 0.0;
      lastLon = 0.0;
      resolutionFlag = 0;
      dataOrder = DO_XY;
      gridOrientation = GO_SN_WE;
    }

    // This Constructor takes all the necessary variables for
    // a basic Equdistant (lat/lon) projection.
    gds_t(const int numx, const int numy,
	  const double deltax, const double deltay, 
	  const double minx, const double miny,
	  const double maxx, const double maxy) : 
      nx (numx), ny (numy), dx(deltax), dy(deltay), originLat(minx), originLon(miny),
      lastLat(maxx), lastLon(maxy)  {
      gridType = EQUIDISTANT_CYL_PROJ_ID;
      numVertical = 0;
      verticalOrPoints = 0;
      resMode = 0;
      lov = 0.0;
      scanMode = 0;
      latin1 = 0.0;
      latin2 = 0.0;
      resolutionFlag = 0;
      resolutionFlag = 128;  // Dx and Dy given, Earth assumed spherical
      dataOrder = DO_XY;
      gridOrientation = GO_SN_WE;
    }

    // This Constructor takes all the necessary variables for
    // a basic lambert projection.
    gds_t(const int numx, const int numy,
	  const double deltax, const double deltay, 
	  const double minx, const double miny,
	  const double lat1, const double lat2, const double lo) : 
      nx (numx), ny (numy), lov(lo), dx(deltax), dy(deltay),
      originLat(minx), originLon(miny),
      latin1(lat1), latin2(lat2)  {
      gridType = LAMBERT_CONFORMAL_PROJ_ID;
      numVertical = 0;
      verticalOrPoints = 0;
      lastLat = 0;
      lastLon = 0;
      resMode = 0;
      scanMode = 0;
      resolutionFlag = 0;
      resMode = 136; // Dx and Dy given, Earth assumed Spherical, 
      //                u and v components of vector quantities resolved relative to the
      //                defined grid in the direction of increasing x and y
      dataOrder = DO_XY;
      gridOrientation = GO_SN_WE;
    }

    grid_type_t gridType;
    int numVertical;  /// NV, the number of vertical coordinate parameters

    /// PV, the location (octet number) of the list of vertical coordinate parameters, if present or
    /// PL, the location (octet number) of the list of numbers of points in each row
    ///     (when no vertical parameters are present), if present, or
    /// 255 (all bits set to 1) if neither are present
    int verticalOrPoints;
    int nx, ny;
    int resMode;
    double lov;
    double dx, dy;
    int scanMode;
    double originLat, originLon;
    double latin1, latin2;
    int resolutionFlag;     // resolution and component flag
    double lastLat, lastLon;
    data_ordering_t dataOrder;
    grid_orientation_t gridOrientation;
  };

  gds();
  virtual ~gds();

  /// Pure virtual function
  /// Sub classes must implement this function
  virtual int unpack( ui08 *gdsPtr )=0;

  /// Retrieve the number of points per row for quasi-regular grids.
  /// Quasi-rectangular grids have fewer points per row near the poles.
  int unpackPtsPerRow(ui08 *gdsPtr);

  /// Pure virtual function
  /// Sub classes must implement this function
  virtual int pack( ui08 *gdsPtr )=0;

  /// Some grids are irregular, with a variable row length.
  /// \return Do we have a regular (rectangular) grid?
  virtual bool isRegular() const;

  /// Display object contents on the specified output stream.
  /// \param stream where output goes (possibly stdout)
  virtual void print(FILE *stream) const;

  /// Display list of numbers of points in each row (for quasi-rectangular grids).
  virtual void printQuasiList(FILE *stream) const;

  /// Return the total number of grid points in this GDS.
  /// This calculation is easy except for quasi-rectangular grids,
  /// where we have to add up the number of points per row.
  int getNumGridPoints() const;

  /// Return a reference to the vector of row lengths (irregular grids only).
  vector<int> &getNumPtsPerRow() { return _numPtsPerRow; }
  const vector<int> &getNumPtsPerRowConst() const { return _numPtsPerRow; }

  // Return a reference to the internal data struct.
  inline const gds_t &getProjection() const { return _data_struct; }  

  inline int getProjType() const    { return _data_struct.gridType; }
  inline double getFirstLat() const { return _data_struct.originLat; }
  inline double getFirstLon() const { return _data_struct.originLon; }
  inline double getEndLat() const   { return _data_struct.lastLat; }
  inline double getEndLon() const   { return _data_struct.lastLon; }
  inline double getLov() const      { return _data_struct.lov; }
  inline double getLatin1() const   { return _data_struct.latin1; }
  inline double getLatin2() const   { return _data_struct.latin2; }
  inline int getNx() const          { return _data_struct.nx; }
  inline int getNy() const          { return _data_struct.ny; }
  inline int getGridDim() const     { return (_data_struct.nx * _data_struct.ny); }
  inline double getDx() const       { return _data_struct.dx; }
  inline double getDy() const       { return _data_struct.dy; }
  inline grid_orientation_t getGridOrientation() const { return _data_struct.gridOrientation; }
  
  void setProjection(const gds_t &new_projection);
  inline void setNx(int newVal)          { _data_struct.nx = newVal; }
  inline void setNy(int newVal)          { _data_struct.ny = newVal; }
  inline void setDx(double newVal)       { _data_struct.dx = newVal; }
  inline void setDy(double newVal)       { _data_struct.dy = newVal; }
  inline void setFirstLat(double newVal) { _data_struct.originLat = newVal; }
  inline void setFirstLon(double newVal) { _data_struct.originLon = newVal; }
  inline void setEndLat(double newVal)   { _data_struct.lastLat = newVal; }
  inline void setEndLon(double newVal)   { _data_struct.lastLon = newVal; }
  inline void setLov(double newVal)      { _data_struct.lov = newVal; }
  inline void setLatin1(double newVal)   { _data_struct.latin1 = newVal; }
  inline void setLatin2(double newVal)   { _data_struct.latin2 = newVal; }
  inline void setGridOrientation(const grid_orientation_t& orientation) 
  { 
    _data_struct.gridOrientation = orientation; 
  }

  inline void setDataOrdering(const data_ordering_t& ordering) 
  { 
    _data_struct.dataOrder = ordering; 
  }

  inline void setExpectedSize(const int& size)
  { 
    _expectedSize = size; 
  }

protected:

  static const int NUM_SECTION_BYTES_LAMBERT = 42;
  static const int NUM_SECTION_BYTES = 32;
  static const int NO_VERTICAL_POINTS = 255;	/// code for neither vert coords nor points
  static const int EXPECTED_SIZE = 45;

  int _expectedSize;

  gds_t _data_struct;

  /// the number of points per row (only for quasi-regular grids)
  vector<int> _numPtsPerRow;

  static const double DEGREES_SCALE_FACTOR;
  static const double GRID_SCALE_FACTOR;
   
};

#endif

