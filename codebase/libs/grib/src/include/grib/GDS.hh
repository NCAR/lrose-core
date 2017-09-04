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
//////////////////////////////////////////////////
#ifndef _GDS_
#define _GDS_

#include <euclid/Pjg.hh>

#include "GribSection.hh"
#include <vector>

using namespace std;

class GDS: public GribSection {
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
  static const int EQUIDISTANT_CYL_PROJ_ID = 0;
  static const int MERCATOR_PROJ_ID = 1;
  static const int GNOMONIC_PROJ_ID = 2;
  static const int LAMBERT_CONFORMAL_PROJ_ID = 3;
  static const int GAUSSIAN_LAT_LON_PROJ_ID = 4;
  static const int POLAR_STEREOGRAPHIC_PROJ_ID = 5;
  static const int UNIVERSAL_TRANSVERSE_MERCATOR_PROJ_ID = 6;
  static const int SIMPLE_POLYCONIC_PROJ_ID = 7;
  static const int ALBERS_EQUAL_AREA_PROJ_ID = 8;
  static const int MILLER_CYLINDRICAL_PROJ_ID = 9;
  static const int ROTATED_LAT_LON_PROJ_ID = 10;
  static const int OBLIQUE_LAMBERT_PROJ_ID = 13;

  GDS();
  virtual ~GDS();

  int unpack( ui08 *gdsPtr );

  /// Retrieve the number of points per row for quasi-regular grids.
  /// Quasi-rectangular grids have fewer points per row near the poles.
  int unpackPtsPerRow(ui08 *gdsPtr);

  int pack( ui08 *gdsPtr );

  /// Some grids are irregular, with a variable row length.
  /// \return Do we have a regular (rectangular) grid?
  virtual bool isRegular() const;

  /// Display object contents on the specified output stream.
  /// \param stream where output goes (possibly stdout)
  virtual void print(FILE *stream) const;
  virtual void print(ostream &stream) const;

  /// Display list of numbers of points in each row (for quasi-rectangular grids).
  virtual void printQuasiList(FILE *stream) const;
  virtual void printQuasiList(ostream &stream) const;

  /// Return the total number of grid points in this GDS.
  /// This calculation is easy except for quasi-rectangular grids,
  /// where we have to add up the number of points per row.
  int getNumGridPoints() const;

  /// Return a reference to the vector of row lengths (irregular grids only).
  vector<int> &getNumPtsPerRow() { return _numPtsPerRow; }
  const vector<int> &getNumPtsPerRowConst() const { return _numPtsPerRow; }

  //
  // Returns for all supported projection types -
  // should return NULL if gds did not indicate
  // that type of projection.
  //
  inline int getGridDim() const
    { return (_projection.getNx() * _projection.getNy()); }
  
  inline int getProjType() const {return (_projType); }
  inline const Pjg &getProjection() const { return _projection; }
  
  inline double getFirstLat() const 
  { double first_lat, first_lon;
    _projection.xy2latlon(_projection.getMinx(), _projection.getMiny(),
			  first_lat, first_lon);
    return first_lat;
  }

  inline double getFirstLon() const 
  { double first_lat, first_lon;
    _projection.xy2latlon(_projection.getMinx(), _projection.getMiny(),
			  first_lat, first_lon);
    return first_lon;
  }

  inline double getLastLat() const 
  { double last_lat, last_lon;
    _projection.xy2latlon(_projection.getMinx() + _projection.getNx()*_projection.getDx(), 
			  _projection.getMiny() + _projection.getNy()*_projection.getDy(),
			  last_lat, last_lon);
    return last_lat;
  }

  inline double getLastLon() const 
  { double last_lat, last_lon;
    _projection.xy2latlon(_projection.getMinx() + _projection.getNx()*_projection.getDx(), 
			  _projection.getMiny() + _projection.getNy()*_projection.getDy(),
			  last_lat, last_lon);
    return last_lon;
  }

  inline double getOriginLat() const {return (_originLat); }
  inline double getOriginLon() const {return (_originLon); }
  inline double getLov() const {return (_lov); }
  inline double getLatin1() const {return (_latin1); }
  inline double getLatin2() const {return (_latin2); }
  inline int getNx() const { return _projection.getNx(); }
  inline int getNy() const { return _projection.getNy(); }
  inline double getMinx() const { return _projection.getMinx(); }
  inline double getMiny() const { return _projection.getMiny(); }
  inline double getDx() const { return _projection.getDx(); }
  inline double getDy() const { return _projection.getDy(); }
  inline grid_orientation_t getGridOrientation() const { return _gridOrientation; }
  
  virtual void setProjection(const Pjg &new_projection);

  inline void setGridOrientation(const grid_orientation_t& orientation) 
  { 
    _gridOrientation = orientation; 
  }

  inline void setDataOrdering(const data_ordering_t& ordering) 
  { 
    _dataOrder = ordering; 
  }

  inline void setExpectedSize(const int& size)
  { 
    _expectedSize = size; 
  }

  /// Inform this GDS that we have converted the quasi-grid to a regular grid.
  /// You should make this call after calling setData() on the corresponding BDS.
  /// \param numRows the new number of rows in the rectangular grid
  /// \param numColumns the new number of columns in the rectangular grid
  virtual void setRegular(int numRows, int numColumns);

protected:

  static const int NUM_SECTION_BYTES = 42;
  static const int NO_VERTICAL_POINTS = 255;	/// code for neither vert coords nor points
  static const int EXPECTED_SIZE = 32;  // Minimum expected number of octets for record
 
  int _expectedSize;
  int _numVertical;      /// NV, the number of vertical coordinate parameters

  /// PV, the location (octet number) of the list of vertical coordinate parameters, if present or
  /// PL, the location (octet number) of the list of numbers of points in each row
  ///     (when no vertical parameters are present), if present, or
  /// 255 (all bits set to 1) if neither are present
  int _verticalOrPoints;
 
  int _projType;
  int _resMode;
  int _scanMode;
  double _originLat;
  double _originLon;
  double _lov;
  double _latin1;
  double _latin2;
  data_ordering_t _dataOrder;
  grid_orientation_t _gridOrientation;

  Pjg _projection;

  /// the number of points per row (only for quasi-regular grids)
  vector<int> _numPtsPerRow;

  static const double DEGREES_SCALE_FACTOR;
  static const double GRID_SCALE_FACTOR;


  ui08 _setScanMode();

};

#endif

