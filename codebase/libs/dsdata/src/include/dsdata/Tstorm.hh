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
/////////////////////////////////////////////////////////////////
// Tstorm class
// 
// $Id: Tstorm.hh,v 1.24 2016/03/03 18:06:34 dixon Exp $
////////////////////////////////////////////////////////////////
#ifndef _TSTORM_HH
#define _TSTORM_HH

#include <cstdio>
#include <vector>

#include <euclid/point.h>
#include <euclid/Polyline.hh>
#include <euclid/WorldPolygon2D.hh>
#include <dsdata/TstormGrid.hh>
#include <rapformats/tstorm_spdb.h>
#include <toolsa/toolsa_macros.h>

using namespace std;

//
// Forward class declarations
//
class Polyline;
class Pjg;

class Tstorm {

 public:

   ///////////////
   // Constants //
   ///////////////

   static const double MISSING_VAL;

  
   //////////////////
   // Public types //
   //////////////////

   enum trendType { UNKNOWN = -2, DECREASING = -1, 
                    STEADY = 0, INCREASING = 1 };


   //////////////////
   // Constructors //
   //////////////////

   Tstorm( tstorm_spdb_entry_t* spdbEntry, int numSides,
           TstormGrid& grid, double startAz, double deltaAz,
           time_t validTime );

   Tstorm(const Tstorm &orig);
  

   ////////////////
   // Destructor //
   ////////////////

   ~Tstorm();


   ////////////////////
   // Access methods //
   ////////////////////

   bool            isForecastValid() const { return _forecastValid; }
   double          getArea() const { return _area; }
   double          getDareaDt() const { return _dAreaDt; }
   double          getSpeed() const { return _speed; }
   double          getDirection() const { return _direction; }
  double getU() const   /* in m/s */
    { return _speed * sin(_direction * DEG_TO_RAD) / MPERSEC_TO_KMPERHOUR; }
  double getV() const   /* in m/s */
    { return _speed * cos(_direction * DEG_TO_RAD) / MPERSEC_TO_KMPERHOUR; }
  
   double          getAspectRatio() const { return (_ellipseMajorRadius/_ellipseMinorRadius); }
   int             getSimpleTrack() const { return _simpleTrack; }
   int             getComplexTrack() const { return _complexTrack; }
   TstormGrid&     getGrid() const { return _grid; }
   void            getCentroid( double& lat, double& lon ) const;
   void            getCentroid( float& lat, float& lon ) const;
   void            getCentroid( double* lat, double* lon ) const;
   void            getCentroid( float* lat, float* lon ) const;
   double          getCentroidLat() const { return _centroidLat; }
   double          getCentroidLon() const { return _centroidLon; }
   void            getLeadingEdge( float* x, float* y ) const;
   void            getLeadingEdge( double* x, double* y ) const;
   double          getMaxRadial() const { return _maxRadial; }
   double          getStartAz() const { return _startAz; }
   double          getDeltaAz() const { return _deltaAz; }
   time_t          getValidTime() const { return _dataTime; }
   Polyline*       getDetectionPoly() const
                   {
		     if (_detectionPoly == 0)
		       _detectionPoly = forecastPoly(0);
      
		     return _detectionPoly;
		   }
   vector<double>& getRadials() { return _radials; }
   double          getAlgorithmValue() { return _algorithmValue; }
  

   // add methods to set Tstorm members
   void        setSpeed(const double& that_speed) { _speed = that_speed; }
   void        setDirection(const double& that_dir) { _direction = that_dir; }
   void        setForecastValid(const bool &fcst_state) { _forecastValid = fcst_state; }
   void        setTop(const double &top) { _top = top; }
  
   void setAlgorithmValue(const double algorithm_value)
          { _algorithmValue = algorithm_value; }
  
   void        setGrid(TstormGrid& that_grid) { _grid = that_grid; }

   // For these versions, the X/Y values in the Polyline object are expected
   // to be in the same units as indicated by the TstormGrid within this
   // object.

   void        setDetectionPoly(const Polyline &polygon);
  
   void        setDetectionPoly(const Point_d *polygon,
				const int num_polygon_pts);
  
   // For this version, the polyline contains lat/lon values for the
   // vertices.

   void        setDetectionPoly(const WorldPolygon2D &polygon);
  
   void        setEntryValues(tstorm_spdb_entry_t &entry) const;
  

   //////////////////////////
   // Input/Output methods //
   //////////////////////////

   void print(ostream &out, const bool print_radials = false,
	      const string &leader = "") const;
  
   void print(FILE *out, const bool print_radials = false,
	      const string &leader = "") const;
  
   void simplePrint(ostream &out, const string &leader = "") const;
  
   void simplePrint(FILE *out, const string &leader = "") const;
  
   static const string trend2String(const trendType trend);
  

   /////////////////////
   // Utility methods //
   /////////////////////

   Polyline*   forecastPoly(const int forecastSecs,
			    const bool grow = true) const;
  
   WorldPolygon2D* forecastWorldPoly(int forecastSecs, bool grow=true) const;

   static int trendType2Titan(const trendType trend_type);
   static trendType titanTrend2TrendType(const int titan_trend);
  
   double distanceCentroidKm(const Tstorm &storm2) const;
   double distanceEdgeKm(const Tstorm &storm2) const;
   double intersection(Tstorm &storm2);
  
   void combineTstorms(const Tstorm &storm2);

   void findMaxRadial();
   void findLeadingEdge();

  /**********************************************************************
   * getPolygonGrid() - Given a projection and a grid matching that
   *                    projection, calculate the gridded filled polygon for
   *                    the given TITAN storm entry.  The grid square
   *                    representing the polygon in the given grid will
   *                    be set to non-zero on return.
   *
   * Returns true on success, false on failure.  Sets the polygon grid
   * squares in the given grid to non-zero and returns the min and max
   * indices in the calling arguments.
   */

  bool getPolygonGrid(const Pjg &projection,
		      unsigned char *grid,
		      int &min_x, int &min_y,
		      int &max_x, int &max_y,
		      const int forecast_secs = 0) const;
  

  /**********************************************************************
   * getPolygonGridGrowKm() - Given a projection and a grid matching that
   *                          projection, calculate the gridded filled
   *                          polygon for the given TITAN storm entry after
   *                          increasing the size of the polygon by the given
   *                          number of km.  The grid square representing the
   *                          polygon in the given grid will be set to non-zero
   *                          on return.
   *
   * Returns true on success, false on failure.  Sets the polygon grid
   * squares in the given grid to non-zero (leaving the other squares in the
   * given grid at their original value) and returns the min and max indices
   * in the calling arguments.
   */

  bool getPolygonGridGrowKm(const Pjg &projection,
			    unsigned char *grid,
			    int &min_x, int &min_y,
			    int &max_x, int &max_y,
			    const double growth_km,
			    const int forecast_secs = 0) const;
  

  /**********************************************************************
   * getPolygonGridGrowPercent() - Given a projection and a grid matching
   *                               that projection, calculate the gridded
   *                               filled polygon for the given TITAN storm
   *                               entry after increasing the size of the
   *                               polygon by the given percent.  The grid
   *                               square representing the polygon in the
   *                               given grid will be set to non-zero on
   *                               return.
   *
   * Returns true on success, false on failure.  Sets the polygon grid
   * squares in the given grid to non-zero (leaving the other squares in the
   * given grid at their original value) and returns the min and max indices
   * in the calling arguments.
   */

  bool getPolygonGridGrowPercent(const Pjg &projection,
				 unsigned char *grid,
				 int &min_x, int &min_y,
				 int &max_x, int &max_y,
				 const double growth_percent,
				 const int forecast_secs = 0) const;
  

 private:

   //
   // Valid time of the storm
   //
   time_t _dataTime;

   //
   // Number of sides in the polygon that represents the storm
   //
   int  _nSides;

   //
   // Origin of center of storm (deg)
   //
   double _centroidLat;
   double _centroidLon;
   
   //
   // Speed (km/h) and direction (deg T) of the storm
   //
   double _direction;
   double _speed;
   
   //
   // Track numbers
   //
   int _simpleTrack;
   int _complexTrack;
   
   //
   // Area (km squared) and rate of change of area (km squared per hour)
   //
   double _area;
   double _dAreaDt;

   //
   // Top of storm (km MSL)
   //
   double _top;

   //
   // Ellipse information
   //   (deg T, km or deg, km or deg)
   //
   double _ellipseOrientation;
   double _ellipseMinorRadius;
   double _ellipseMajorRadius;
   
   //
   // Is the forecast valid?
   //
   bool _forecastValid;

   //
   // Maximum dbz 
   //
   int  _dbzMax;

   //
   // Trends
   //
   trendType _intensityTrend;
   trendType _sizeTrend;

   //
   // Algorithm value stored in database
   //

   double _algorithmValue;
  
   //
   // Radials that describe the polygon of the storm
   //   In grid units
   //
   double           _startAz;
   double           _deltaAz;
   double           _maxRadial;
   TstormGrid&     _grid;
   vector< double > _radials;
   void            _clearRadials();
  
   //
   // Points on polygon
   //  Units depend on the projection 
   //
   mutable Polyline              *_detectionPoly;
   pair< double, double >         _leadingEdge;
   void                           _clearDetection();

   //
   // Gridded polygon projection members
   //
   double        _minLat;
   double        _minLon;
   double        _maxLat;
   double        _maxLon;
   unsigned char *_polyGrid;
   int           _polyGridAlloc;
   Point_d       *_vertices;
   int           _verticesAlloc;
 
   void _combinePolygons(const Tstorm &storm2);
  
   static void _normalizeTheta(double &theta,
			       const double base_angle);
  
   void _normalizeRadialNum(int &radial_num) const;

#ifdef NOT_NOW  
   unsigned char *_getPolygonGrid(const Pjg* proj);
#endif

   bool _polygonsIntersect(Polyline *that_poly);
   bool _intersectTest(const double& a_min_lat, const double& a_min_lon, 
		       const double& a_max_lat, const double& a_max_lon, 
		       const double& b_min_lat, const double& b_min_lon, 
		       const double& b_max_lat, const double& b_max_lon);

 };

#endif
