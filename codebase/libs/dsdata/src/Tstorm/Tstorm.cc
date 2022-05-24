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
////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cfloat>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <cassert>

#include <dsdata/Tstorm.hh>
#include <euclid/boundary.h>
#include <euclid/geometry.h>
#include <euclid/PjgCalc.hh>
#include <euclid/PjgTypes.hh>
#include <euclid/Polyline.hh>
#include <rapformats/titan_grid.h>
#include <rapmath/math_macros.h>
#include <toolsa/DateTime.hh>
#include <toolsa/globals.h>
#include <toolsa/pjg.h>
#include <toolsa/mem.h>
using namespace std;

///////////////
// Constants //
///////////////

const double Tstorm::MISSING_VAL = FLT_MAX;


//////////////////
// Constructors //
//////////////////

//
// Constructor based on data from an SPDB database
//

Tstorm::Tstorm( tstorm_spdb_entry_t* spdbEntry, int numSides,
                TstormGrid& tGrid, double stAz, double dAz,
                time_t validTime ) :
  _dataTime(validTime),
  _nSides(numSides),
  _centroidLat(spdbEntry->latitude),
  _centroidLon(spdbEntry->longitude),
  _direction(spdbEntry->direction),
  _speed(spdbEntry->speed),
  _simpleTrack(spdbEntry->simple_track_num),
  _complexTrack(spdbEntry->complex_track_num),
  _area(spdbEntry->area),
  _dAreaDt(spdbEntry->darea_dt),
  _top(spdbEntry->top),
  _ellipseOrientation(spdbEntry->ellipse_orientation),
  _ellipseMinorRadius(spdbEntry->ellipse_minor_radius),
  _ellipseMajorRadius(spdbEntry->ellipse_major_radius),
  _forecastValid(spdbEntry->forecast_valid == 1 ? true : false),
  _dbzMax(spdbEntry->dbz_max),
  _intensityTrend(titanTrend2TrendType(spdbEntry->intensity_trend)),
  _sizeTrend(titanTrend2TrendType(spdbEntry->size_trend)),
  _algorithmValue(spdbEntry->algorithm_value),
  _startAz(stAz),
  _deltaAz(dAz),
  _grid(tGrid)
{
   for( int irad = 0; irad < _nSides; irad++ ) {

      double radial = spdbEntry->polygon_radials[irad] *
	 spdbEntry->polygon_scale;
      
      _radials.push_back( radial );
   }
   
   _detectionPoly = forecastPoly(0);
   
   findMaxRadial();
   findLeadingEdge();

   _minLat = 0.0;
   _minLon = 0.0;
   _maxLat = 0.0;
   _maxLon = 0.0;
   _polyGrid = 0;
   _polyGridAlloc = 0;
   _vertices = 0;
   _verticesAlloc = 0;
}


//
// Copy constructor
//

Tstorm::Tstorm(const Tstorm &orig) :
  _dataTime(orig._dataTime),
  _nSides(orig._nSides),
  _centroidLat(orig._centroidLat),
  _centroidLon(orig._centroidLon),
  _direction(orig._direction),
  _speed(orig._speed),
  _simpleTrack(orig._simpleTrack),
  _complexTrack(orig._complexTrack),
  _area(orig._area),
  _dAreaDt(orig._dAreaDt),
  _top(orig._top),
  _ellipseOrientation(orig._ellipseOrientation),
  _ellipseMinorRadius(orig._ellipseMinorRadius),
  _ellipseMajorRadius(orig._ellipseMajorRadius),
  _forecastValid(orig._forecastValid),
  _dbzMax(orig._dbzMax),
  _intensityTrend(orig._intensityTrend),
  _sizeTrend(orig._sizeTrend),
  _algorithmValue(orig._algorithmValue),
  _startAz(orig._startAz),
  _deltaAz(orig._deltaAz),
  _maxRadial(orig._maxRadial),
  _grid(orig._grid),
  _radials(orig._radials),
  _detectionPoly(new Polyline(*orig._detectionPoly)),
  _leadingEdge(orig._leadingEdge),
  _minLat(orig._minLat),
  _minLon(orig._minLon),
  _maxLat(orig._maxLat),
  _maxLon(orig._maxLon),
  _polyGrid(orig._polyGrid),
  _polyGridAlloc(orig._polyGridAlloc),
  _vertices(orig._vertices),
  _verticesAlloc(orig._verticesAlloc)
{
  // Do nothing -- it's all done in the initialization.
}


////////////////
// Destructor //
////////////////

Tstorm::~Tstorm() 
{
  _clearRadials();
  delete _detectionPoly;
  ufree(_polyGrid);
  ufree(_vertices);

}

void
Tstorm::findMaxRadial() 
{
   _maxRadial = 0;
   
   vector< double >::iterator it;

   for( it = _radials.begin(); it != _radials.end(); it++ ) {
      if( (*it) > _maxRadial ) {
	 _maxRadial  = (*it);
      }
   }
}


////////////////////
// Access methods //
////////////////////

void
Tstorm::getCentroid( double& lat, double& lon ) const
{
   lat = _centroidLat;
   lon = _centroidLon;
}

void
Tstorm::getCentroid( float& lat, float& lon ) const
{
   lat = _centroidLat;
   lon = _centroidLon;
}

void
Tstorm::getCentroid( double* lat, double* lon ) const
{
   *lat = _centroidLat;
   *lon = _centroidLon;
}

void
Tstorm::getCentroid( float* lat, float* lon ) const
{
   *lat = _centroidLat;
   *lon = _centroidLon;
}

void
Tstorm::getLeadingEdge( float* x, float* y ) const
{
   *x = _leadingEdge.first;
   *y = _leadingEdge.second;
}
      
void
Tstorm::getLeadingEdge( double* x, double* y ) const
{
   *x = _leadingEdge.first;
   *y = _leadingEdge.second;
}
      
void
Tstorm::findLeadingEdge() 
{
   int   leadingIdex = (int) (( _direction - _startAz ) / _deltaAz );
   if(leadingIdex < 0)
   {
     leadingIdex += _radials.size();
   }

   double theta       = leadingIdex * _deltaAz + _startAz;
   
   titan_grid_comps_t comps;
   
   double centroidX, centroidY;
   double deltaX, deltaY;
   double deltaLat, deltaLon;
   
   switch( _grid.getProjType() ) {
       case PjgTypes::PROJ_FLAT:
	  
          TITAN_init_flat( (double) _grid.getOriginLat(), 
                           (double) _grid.getOriginLon(), 0, &comps );
          TITAN_latlon2xy( &comps, _centroidLat, _centroidLon, 
                           &centroidX, &centroidY );

          deltaX = _radials[leadingIdex] * sin(theta) * _grid.getDx();
	  deltaY = _radials[leadingIdex] * cos(theta) * _grid.getDy();
	  
          _leadingEdge.first  = centroidX + deltaX;
	  _leadingEdge.second = centroidY + deltaY;

	  break;
	  
       case PjgTypes::PROJ_LATLON:

          deltaLat = _radials[leadingIdex] * cos(theta) * _grid.getDy();
	  deltaLon = _radials[leadingIdex] * sin(theta) * _grid.getDx();
	  
          _leadingEdge.first  = _centroidLat + deltaLat;
	  _leadingEdge.second = _centroidLon + deltaLon;

	  break;
	  
       default:
          _leadingEdge.first  = MISSING_VAL;
	  _leadingEdge.second = MISSING_VAL;
	  break;
   }
}
   

void Tstorm::setDetectionPoly(const Polyline &polygon)
{
  Polyline local_polygon(polygon);
  
  // Make sure the new polygon is closed

  local_polygon.closePolyline();
  
  // Convert the original polygon to the format needed by the
  // geometry routines

  int num_orig_points = local_polygon.getNumPts();
  Point_d *orig_polygon = new Point_d[num_orig_points];
  
  for (int i = 0; i < num_orig_points; ++i)
  {
    orig_polygon[i].x = local_polygon.getX(i);
    orig_polygon[i].y = local_polygon.getY(i);
  }
  
  setDetectionPoly(orig_polygon, num_orig_points);
  
  // Free temporary memory

  delete[] orig_polygon;
}


void Tstorm::setDetectionPoly(const WorldPolygon2D &polygon)
{
  // Convert the original polygon to the format needed by the
  // geometry routines

  int num_orig_points = polygon.getNumPoints() + 1;
  Point_d *orig_polygon = new Point_d[num_orig_points];
  
  WorldPoint2D *point;
  
  point = polygon.getFirstPoint();
  
  orig_polygon[0].x = orig_polygon[num_orig_points - 1].x = point->lon;
  orig_polygon[0].y = orig_polygon[num_orig_points - 1].y = point->lat;
  
  int i;
  
  for (i = 1, point = polygon.getNextPoint(); point != 0;
       point = polygon.getNextPoint(), ++i)
  {
    orig_polygon[i].x = point->lon;
    orig_polygon[i].y = point->lat;
  }
  
  setDetectionPoly(orig_polygon, num_orig_points);
  
  // Free temporary memory

  delete[] orig_polygon;
}


void Tstorm::setDetectionPoly(const Point_d *polygon,
			      const int num_polygon_pts)
{
  // Calculate the new centroid and set the object members

  Point_d centroid_pt;
    
  EG_polygon_centroid_d(polygon, num_polygon_pts, &centroid_pt);
  
  _grid.xy2latlon(centroid_pt.x, centroid_pt.y,
		 _centroidLat, _centroidLon);
  
  // Calculate the sine and cosine of each angle for the polygon radials

  Point_d *rays = new Point_d[_nSides];
    
  _startAz = EG_init_ray_TN(rays, _nSides, &centroid_pt);

  // Now calculate the new radials

  Star_point *star_pts = new Star_point[_nSides + 1];
  
  int num_star_pts = EG_make_star_TN((Point_d *)polygon, num_polygon_pts,
				     rays, _nSides, _startAz,
				     &centroid_pt, star_pts);
  
  assert (num_star_pts == _nSides);
  

  // Update the area of the polygon
  Point_d *xy_polygon = new Point_d[num_polygon_pts];
  for ( int i = 0; i < num_polygon_pts; i++ ) 
  {
    double r, theta;
    _grid.latlon2RTheta(_grid.getOriginLat(), _grid.getOriginLon(), polygon[i].y, polygon[i].x, 
		       r, theta);
    
   xy_polygon[i].x = r*cos(theta*DEG_TO_RAD);
   xy_polygon[i].y = r*sin(theta*DEG_TO_RAD);
  }
  _area = EG_polygon_area_d((Point_d *)xy_polygon, num_polygon_pts);
  delete[] xy_polygon;

  _clearRadials();
  
  for (int i = 0; i < _nSides; ++i)
  {
   _radials.push_back(star_pts[i].r / _grid.getDx());
  }

  findMaxRadial();
  
  delete _detectionPoly;
  _detectionPoly = forecastPoly(0);
  
  // Free temporary memory

  delete[] rays;
  delete[] star_pts;
}


void Tstorm::setEntryValues(tstorm_spdb_entry_t &entry) const
{
  memset(&entry, 0, sizeof(tstorm_spdb_entry_t));
  
  entry.longitude = _centroidLon;
  entry.latitude = _centroidLat;
  entry.direction = _direction;
  entry.speed = _speed;
  entry.simple_track_num = _simpleTrack;
  entry.complex_track_num = _complexTrack;
  entry.area = _area;
  entry.darea_dt = _dAreaDt;
  entry.top = _top;
  entry.ellipse_orientation = _ellipseOrientation;
  entry.ellipse_minor_radius = _ellipseMinorRadius;
  entry.ellipse_major_radius = _ellipseMajorRadius;
  entry.polygon_scale = _maxRadial / 255.0;
  entry.algorithm_value = _algorithmValue;
  
  int num_poly_sides = MIN(N_POLY_SIDES, _radials.size());
  
  for (int i = 0; i < num_poly_sides; ++i)
    entry.polygon_radials[i] = (ui08)(_radials[i] / entry.polygon_scale + 0.5);
  
  if (_forecastValid)
    entry.forecast_valid = 1;
  
  entry.dbz_max = _dbzMax;
  
  entry.intensity_trend = trendType2Titan(_intensityTrend);
  entry.size_trend = trendType2Titan(_sizeTrend);
}


//////////////////////////
// Input/Output methods //
//////////////////////////

void
Tstorm::print(ostream &out, const bool print_radials,
	      const string &leader) const
{
  out << leader << "Tstorm object" << endl;
  out << leader << "=============" << endl;
  _grid.print(out, leader + "   ");
  out << leader << "data time: " << DateTime(_dataTime) << endl;
  out << leader << "num sides: " << _nSides << endl;
  out << leader << "centroid lat: " << _centroidLat << endl;
  out << leader << "centroid lon: " << _centroidLon << endl;
  out << leader << "direction: " << _direction << " deg T" << endl;
  out << leader << "speed: " << _speed << "km/h" << endl;
  out << leader << "simple track num: " << _simpleTrack << endl;
  out << leader << "complex track num: " << _complexTrack << endl;
  out << leader << "area: " << _area << " km2" << endl;
  out << leader << "area rate of change: " << _dAreaDt << " km2/hr" << endl;
  out << leader << "top: " << _top << " km MSL" << endl;
  out << leader << "ellipse info:" << endl;
  out << leader << "   orientation: " << _ellipseOrientation << " deg T" << endl;
  out << leader << "   minor radius: " << _ellipseMinorRadius << " " <<
    _grid.getXUnits() << endl;
  out << leader << "   major radius: " << _ellipseMajorRadius << " " <<
    _grid.getXUnits() << endl;
  out << leader << "forecast valid?: " << _forecastValid  << endl;
  out << leader << "dbz max: " << _dbzMax << endl;
  out << leader << "intensity trend: " << trend2String(_intensityTrend) << endl;
  out << leader << "size trend: " << trend2String(_sizeTrend) << endl;
  out << leader << "algorithm value: " << _algorithmValue << endl;
  out << leader << "start azimuth: " << _startAz << endl;
  out << leader << "delta azimuth: " << _deltaAz << endl;
  out << leader << "max radial: " << _maxRadial << endl;
  if (print_radials)
  {
    out << leader << "radials:" << endl;
    
    vector< double >::const_iterator radial;
    
    for (radial = _radials.begin(); radial != _radials.end(); ++radial)
      out << leader << "   " << *radial << endl;

    if (_detectionPoly != 0)
    {
      out << leader << "detection polyline:" << endl;
    
      int num_vertices = _detectionPoly->getNumPts();
      
      out << leader << "   num vertices: " << num_vertices << endl;
      
      for (int i = 0; i < num_vertices; ++i)
	out << leader << "   " << i << ": " <<
	  _detectionPoly->getX(i) << " " << _detectionPoly->getY(i) << endl;
	
    } /* endif - _detectionPoly != 0 */
    
  } /* endif - print_radials */
}


void
Tstorm::simplePrint(ostream &out, const string &leader) const
{
  out << leader << "Tstorm object" << endl;
  out << leader << "=============" << endl;
  out << leader << "data time: " << DateTime(_dataTime) << endl;
  out << leader << "centroid lat: " << _centroidLat << endl;
  out << leader << "centroid lon: " << _centroidLon << endl;
  out << leader << "direction: " << _direction << " deg T" << endl;
  out << leader << "speed: " << _speed << "km/h" << endl;
  out << leader << "area: " << _area << " km^2" << endl;
  out << leader << "major axis: " << _ellipseMajorRadius << " km" << endl;
  out << leader << "minor axis: " << _ellipseMinorRadius << " km" << endl;
  out << leader << "aspect ratio: " << 
    (_ellipseMajorRadius/_ellipseMinorRadius) << endl;
  out << leader << "forecast valid?: " << _forecastValid << endl;
}


void
Tstorm::print(FILE *out, const bool print_radials,
	      const string &leader) const
{
  fprintf(out, "%sTstorm object\n", leader.c_str());
  fprintf(out, "%s==============n", leader.c_str());
  _grid.print(out, leader + "   ");
  fprintf(out, "%sdata time: %s\n",
	  leader.c_str(), DateTime::str(_dataTime).c_str());
  fprintf(out, "%snum sides: %d\n", leader.c_str(), _nSides);
  fprintf(out, "%scentroid lat: %f\n", leader.c_str(), _centroidLat);
  fprintf(out, "%scentroid lon: %f\n", leader.c_str(), _centroidLon);
  fprintf(out, "%sdirection: %f deg T\n", leader.c_str(), _direction);
  fprintf(out, "%sspeed: %f km/h\n", leader.c_str(), _speed);
  fprintf(out, "%ssimple track num: %d\n", leader.c_str(), _simpleTrack);
  fprintf(out, "%scomplex track num: %d\n", leader.c_str(), _complexTrack);
  fprintf(out, "%sarea: %f km2\n", leader.c_str(), _area);
  fprintf(out, "%sarea rate of change: %f km2/hr\n", leader.c_str(), _dAreaDt);
  fprintf(out, "%stop: %f km MSL\n", leader.c_str(), _top);
  fprintf(out, "%sellipse info:\n", leader.c_str());
  fprintf(out, "%s   orientation: %f deg T\n",
	  leader.c_str(), _ellipseOrientation);
  fprintf(out, "%s   minor radius: %f %s\n",
	  leader.c_str(), _ellipseMinorRadius, _grid.getXUnits().c_str());
  fprintf(out, "%s   major radius: %f %s\n",
	  leader.c_str(), _ellipseMajorRadius, _grid.getXUnits().c_str());
  fprintf(out, "%sforecast valid?: %d\n", leader.c_str(),_forecastValid);
  fprintf(out, "%sdbz max: %d\n", leader.c_str(), _dbzMax);
  fprintf(out, "%sintensity trend: %s\n",
	  leader.c_str(), trend2String(_intensityTrend).c_str());
  fprintf(out, "%ssize trend: %s\n",
	  leader.c_str(), trend2String(_sizeTrend).c_str());
  fprintf(out, "%salgorithm value: %f\n",
	  leader.c_str(), _algorithmValue);
  fprintf(out, "%sstart azimuth: %f\n", leader.c_str(), _startAz);
  fprintf(out, "%sdelta azimuth: %f\n", leader.c_str(), _deltaAz);
  fprintf(out, "%smax radial: %f\n", leader.c_str(), _maxRadial);
  if (print_radials)
  {
    fprintf(out, "%sradials:\n", leader.c_str());
    
    vector< double >::const_iterator radial;
    
    for (radial = _radials.begin(); radial != _radials.end(); ++radial)
      fprintf(out, "%s   %f\n", leader.c_str(), *radial);

    if (_detectionPoly != 0)
    {
      fprintf(out, "%sdetection polyline:\n", leader.c_str());
    
      int num_vertices = _detectionPoly->getNumPts();
      
      for (int i = 0; i < num_vertices; ++i)
	fprintf(out, "%s   %d: %7.3f %7.3f\n",
		leader.c_str(), i,
		_detectionPoly->getX(i), _detectionPoly->getY(i));
	
    } /* endif - _detectionPoly != 0 */
    
  } /* endif - print_radials */
}

void
Tstorm::simplePrint(FILE *out, const string &leader) const
{
  fprintf(out, "%sTstorm object\n", leader.c_str());
  fprintf(out, "%s==============n", leader.c_str());
  fprintf(out, "%sdata time: %s\n",
	  leader.c_str(), DateTime::str(_dataTime).c_str());
  fprintf(out, "%scentroid lat: %f\n", leader.c_str(), _centroidLat);
  fprintf(out, "%scentroid lon: %f\n", leader.c_str(), _centroidLon);
  fprintf(out, "%sdirection: %f deg T\n", leader.c_str(), _direction);
  fprintf(out, "%sspeed: %f km/h\n", leader.c_str(), _speed);
  fprintf(out, "%sarea: %f km^2\n", leader.c_str(), _area);
  fprintf(out, "%smajor axis: %f km\n", leader.c_str(), _ellipseMajorRadius);
  fprintf(out, "%sminor axis: %f km\n", leader.c_str(), _ellipseMinorRadius);
  fprintf(out, "%saspect ratio: %f\n", leader.c_str(), 
	  (_ellipseMajorRadius/_ellipseMinorRadius));
  fprintf(out, "%sforecast valid?: %d\n", leader.c_str(),_forecastValid);
}

/////////////////////
// Utility methods //
/////////////////////

Polyline* 
Tstorm::forecastPoly(const int forecastSecs, const bool grow) const
{
   Polyline           *forecastPoly = NULL;
   
   double              range;
   double              centroidX, centroidY;
   double              forecastX, forecastY;
   double              linealGrowth;

   float              *xData   = NULL;
   float              *yData   = NULL;
   
   
   int    nPoints      = _radials.size();
   double distanceKm   = _speed * forecastSecs / 3600.0;
   double dirRad       = _direction * DEG_TO_RAD;
   double theta        = _startAz * DEG_TO_RAD;
   double dTheta       = _deltaAz * DEG_TO_RAD;
   double forecastArea =
     MAX(_area + (_dAreaDt * (double)forecastSecs / 3600.0), 1.0);

   if (grow)
     linealGrowth = sqrt( forecastArea / _area );
   else
     linealGrowth = 1.0;
   
   xData = new float[nPoints];
   yData = new float[nPoints];
	  
   _grid.latlon2xy(_centroidLat, _centroidLon, 
		  centroidX, centroidY );

   forecastX = centroidX + _grid.km2x(distanceKm * sin( dirRad ));
   forecastY = centroidY + _grid.km2x(distanceKm * cos( dirRad ));

   for( int i = 0; i < nPoints; i++ )
   {
     range = (_radials[i]) * linealGrowth;

     xData[i] = forecastX + range * sin(theta) * _grid.getDx();
     yData[i] = forecastY + range * cos(theta) * _grid.getDy();

     theta += dTheta;
   }

   forecastPoly = new Polyline( _grid.getOriginLat(), 
				_grid.getOriginLon(), 0.0,
				nPoints, xData, yData, forecastX,
				forecastY, _dataTime+forecastSecs,
				Polyline::CLOSED );
	  
   delete[] xData;
   delete[] yData;
	  
   return( forecastPoly );
   
} 

WorldPolygon2D* Tstorm::forecastWorldPoly(int forecastSecs, bool grow) const
{
  WorldPolygon2D *forecast_poly = new WorldPolygon2D();
   
  // Calculate the lineal growth

  double lineal_growth;
  double forecast_area =
    MAX(_area + (_dAreaDt * (double)forecastSecs / 3600.0), 1.0);

  if (grow)
    lineal_growth = sqrt(forecast_area / _area);
  else
    lineal_growth = 1.0;
   
  // Get the forecast polygon centroid in X/Y units.  This is the
  // detection centroid moved based on the speed and direction.

  double centroid_x, centroid_y;

  _grid.latlon2xy(_centroidLat, _centroidLon, 
		 centroid_x, centroid_y);

  double distance_km = _speed * forecastSecs / 3600.0;
  double dir_rad = _direction * DEG_TO_RAD;

  double forecast_x = centroid_x + _grid.km2x(distance_km * sin(dir_rad));
  double forecast_y = centroid_y + _grid.km2x(distance_km * cos(dir_rad));

  // Now use the polygon radials to get the forecast polygon points.

  double theta = _startAz * DEG_TO_RAD;
  double d_theta = _deltaAz * DEG_TO_RAD;

  for (size_t i = 0; i < _radials.size(); ++i)
  {
    double range = (_radials[i]) * lineal_growth;

    double x = forecast_x + range * sin(theta) * _grid.getDx();
    double y = forecast_y + range * cos(theta) * _grid.getDy();

    double lat, lon;
     
    _grid.xy2latlon(x, y, lat, lon);
     
    WorldPoint2D *point = new WorldPoint2D(lat, lon);
     
    forecast_poly->addPoint(point);
     
    theta += d_theta;
  } /* endfor -- i */

  return forecast_poly;
} 

const string
Tstorm::trend2String(const trendType trend)
{
  switch (trend)
  {
  case UNKNOWN :
    return "UNKNOWN";
    
  case DECREASING :
    return "DECREASING";
    
  case STEADY :
    return "STEADY";
    
  case INCREASING :
    return "INCREASING";
  }
  
  return "INVALID TREND TYPE";
}


int Tstorm::trendType2Titan(const trendType trend_type)
{
  switch (trend_type)
  {
  case DECREASING :
    return -1;
    
  case STEADY :
    return 0;
    
  case INCREASING :
    return 1;
    
  case UNKNOWN :
  default:
    return -2;
  } /* endswitch - trend_type */
}


Tstorm::trendType Tstorm::titanTrend2TrendType(const int titan_trend)
{
  switch (titan_trend)
  {
  case -1 :
    return DECREASING;
    
  case 0 :
    return STEADY;
    
  case 1 :
    return INCREASING;
    
  default:
    return UNKNOWN;
  } /* endswitch - trend_type */
}


double Tstorm::distanceCentroidKm(const Tstorm &storm2) const
{
  double lat_diff = storm2._centroidLat - this->_centroidLat;
  double lon_diff = storm2._centroidLon - this->_centroidLon;
  
  double distance_deg = sqrt((lat_diff * lat_diff) + (lon_diff * lon_diff));
  
  return distance_deg * KM_PER_DEG_AT_EQ;
}


double Tstorm::distanceEdgeKm(const Tstorm &storm2) const
{
  // Calculate the distance of each vertex of storm2 from the centroid
  // of this storm.  The shortest distance here will determine which vertex
  // from storm2 to use as the closest vertex.

  double min_distance = 0;
  // int storm2_vertex_num;
  double storm2_lat = 0, storm2_lon = 0;
  
  for (int i = 0; i < storm2._detectionPoly->getNumPts(); ++i)
  {
    double detect_lat, detect_lon;
    
    _grid.xy2latlon(storm2._detectionPoly->getX(i),
		   storm2._detectionPoly->getY(i),
		   detect_lat, detect_lon);
    
    double lat_diff = _centroidLat - detect_lat;
    double lon_diff = _centroidLon - detect_lon;
    
    double curr_distance = (lat_diff * lat_diff) + (lon_diff * lon_diff);
    
    if (i == 0 ||
	curr_distance < min_distance)
    {
      min_distance = curr_distance;
      // storm2_vertex_num = i;
      storm2_lat = detect_lat;
      storm2_lon = detect_lon;
    }
  } /* endfor - i */
  
  // We now know which vertex to use from storm2.  Now we need to find the
  // vertex to use from the storm itself.  This will be the vertex
  // closest to the line between the storm2 vertex and this storm's
  // centroid.

  double vertex_distance;
  double vertex_theta;
  
  _grid.latlon2RTheta(storm2_lat, storm2_lon,
		     _centroidLat, _centroidLon,
		     vertex_distance, vertex_theta);
  
  while (vertex_theta < _startAz)
    vertex_theta += 360.0;


  // Now, calculate the detection poly vertex theta. Note it is not the
  // same as storm2's vertex theta. The detection poly vertex theta is 
  // 180 degrees out of phase with the storm2 vertex angle. 
  //
  // The detection poly start azumith must also be accounted for in the 
  // determination of its vertex theta. There is no guarntee that the
  // start azimuth is 0 degrees.

  double detect_start_az; // need to shift into a -180 to 180 degree range
  if (_startAz > 180.0)
    detect_start_az = _startAz - 360.0;
  else
    detect_start_az = _startAz;

  double detect_theta;
  if (vertex_theta > 180.0)
    detect_theta = vertex_theta - 180.0 - detect_start_az;
  else
    detect_theta = vertex_theta + 180.0 - detect_start_az;

  while (detect_theta < 0.0)
    detect_theta += 360.0;

  int this_vertex_num =
    (int)(((detect_theta - _startAz) / _deltaAz) + 0.5) % _nSides;


  // Now calculate the distance from the storm2 vertex to the vertex of
  // this storm.  This is the distance we return.

  double this_lat, this_lon;
  
  _grid.xy2latlon(_detectionPoly->getX(this_vertex_num),
		 _detectionPoly->getY(this_vertex_num),
		 this_lat, this_lon);
  
  double distance, theta;
  
  _grid.latlon2RTheta(this_lat, this_lon,
		     storm2_lat, storm2_lon,
		     distance, theta);
  
  // Finally, make sure the storms aren't overlapping at the given
  // radials.  If they are overlapping, the distance should be 0.

  double storm2_vertex_distance, storm2_vertex_theta;
  double this_vertex_distance, this_vertex_theta;
  
  _grid.latlon2RTheta(_centroidLat, _centroidLon,
		     this_lat, this_lon,
		     this_vertex_distance, this_vertex_theta);
  
  _grid.latlon2RTheta(_centroidLat, _centroidLon,
		     storm2_lat, storm2_lon,
		     storm2_vertex_distance, storm2_vertex_theta);
  
  if (storm2_vertex_distance < this_vertex_distance)
    distance = 0.0;
  
  return distance;
}


double Tstorm::intersection(Tstorm &storm2)
{

  Polyline *storm2_polygon = storm2.getDetectionPoly();

#ifdef NOTNOW
  cerr << "  THIS POLYGON INFO:" << endl;
  for (int i = 0; i < _detectionPoly->getNumPts(); ++i)
  {
    cerr << "X,Y : " << _detectionPoly->getX(i) << ", " << _detectionPoly->getY(i) << endl;
  }

  cerr << "  STORM2 POLYGON INFO:" << endl;
  for (int i = 0; i < storm2_polygon->getNumPts(); ++i)
  {
    cerr << "X,Y : " << storm2_polygon->getX(i) << ", " << storm2_polygon->getY(i) << endl;
  }
#endif

  // check that storm2 is on the same projection

  TstormGrid& storm2_grid = storm2.getGrid();
  if (storm2_grid.getProjType() != _grid.getProjType())
  {
    cerr << "ERROR:: Storms do not have same underlying projection." << endl;
    return 0.0;
  }


  // check that there is some intersection based on max. and min. latitudes
  // and longitudes

  if (!_polygonsIntersect(storm2_polygon)) 
  {
    cerr << "WARNING:: Storms do not intersect." << endl;
      return 0.0;
  }

  int min_x = 0, min_y = 0, max_x = 0, max_y = 0;

  int grid_size = _grid.getNx() * _grid.getNy();

  if (_polyGridAlloc < grid_size)
  {
    _polyGridAlloc = grid_size;
    
    if (_polyGrid == 0)
      _polyGrid = (unsigned char *)umalloc(_polyGridAlloc);
    else
      _polyGrid = (unsigned char *)urealloc(_polyGrid, _polyGridAlloc);
  }
  
  memset(_polyGrid, 0, _polyGridAlloc);
  


  // Convert the SPDB polygon to lat/lon

  if (_verticesAlloc < _detectionPoly->getNumPts())
  {
    _verticesAlloc = _detectionPoly->getNumPts();
      
    if (_vertices == 0)
      _vertices = (Point_d *)umalloc(_verticesAlloc * sizeof(Point_d));
    else
      _vertices = (Point_d *)urealloc(_vertices,
				     _verticesAlloc * sizeof(Point_d));
  }
    
  // Convert the lat/lon polygon to the format needed for the
  // fill_polygon routine.  Get the bounding box while we're
  // here

  for (int i = 0; i < _detectionPoly->getNumPts(); i++)
  {
    int x, y;
      
    _grid.latlon2xyIndex(_detectionPoly->getY(i), 
			_detectionPoly->getX(i), x, y);
      
    if (x < 0)
      _vertices[i].x = 0.0;
    else if (x >= _grid.getNx())
      _vertices[i].x = (double)(_grid.getNx() - 1);
    else
      _vertices[i].x = (double)x;

    if (y < 0)
      _vertices[i].y = 0.0;
    else if (y >= _grid.getNy())
      _vertices[i].y = (double)(_grid.getNy() - 1);
    else
      _vertices[i].y = (double)y;
      
    if (i == 0)
    {
      min_x = (int)_vertices[i].x;
      max_x = (int)_vertices[i].x;
      min_y = (int)_vertices[i].y;
      max_y = (int)_vertices[i].y;
    }
    else
    {
      if (min_x > (int)_vertices[i].x)
	min_x = (int)_vertices[i].x;
      if (max_x < (int)_vertices[i].x)
	max_x = (int)_vertices[i].x;
      if (min_y > (int)_vertices[i].y)
	min_y = (int)_vertices[i].y;
      if (max_y < (int)_vertices[i].y)
	max_y = (int)_vertices[i].y;
    }
      
  } /* endfor - i */

#ifdef NOTNOW    
  fprintf(stderr,
	  "Grid: min_x = %d, max_x = %d, min_y = %d, max_y = %d\n",
	  min_x, max_x, min_y, max_y);
  fprintf(stderr,
	  "      grid_nx = %d, grid_ny = %d\n",
	  _grid.getNx(), _grid.getNy());
#endif
  
  // Get the gridded filled polygon.

  long points_filled = EG_fill_polygon(_vertices,
				       _detectionPoly->getNumPts(),
				       _grid.getNx(), _grid.getNy(),
				       0.0, 0.0,
				       1.0, 1.0,
				       _polyGrid,
				       1);
#ifdef NOTNOW      
  fprintf(stderr,
	  "Filled polygon (grid_nx = %d, grid_ny = %d, points_filled = %ld):\n",
	  _grid.getNx(), _grid.getNy(), points_filled);
  fprintf(stderr,
	  "               (min_x = %d, max_x = %d, min_y = %d, max_y = %d)\n",
	  min_x, max_x, min_y, max_y);
  
  for (int y = max_y; y >= min_y; y--)
  {
    fprintf(stderr, "   ");
    
    for (int x = min_x; x < max_x; x++)
    {
      fprintf(stderr,
	      "%2d ", _polyGrid[(y * _grid.getNx()) + x]);
    } /* endfor - x */
	  
    fprintf(stderr, "\n");
    
  } /* endfor - y */
#endif  

  // now process storm 2

  // Convert the SPDB polygon to lat/lon

  if (_verticesAlloc < storm2_polygon->getNumPts())
  {
    _verticesAlloc = storm2_polygon->getNumPts();
      
    if (_vertices == 0)
      _vertices = (Point_d *)umalloc(_verticesAlloc * sizeof(Point_d));
    else
      _vertices = (Point_d *)urealloc(_vertices,
				     _verticesAlloc * sizeof(Point_d));
  }
    
  // Convert the lat/lon polygon to the format needed for the
  // fill_polygon routine.  Get the bounding box while we're
  // here

  for (int i = 0; i < storm2_polygon->getNumPts(); i++)
  {
    int x, y;
      
    _grid.latlon2xyIndex(storm2_polygon->getY(i), 
			storm2_polygon->getX(i), x, y);
      
    if (x < 0)
      _vertices[i].x = 0.0;
    else if (x >= _grid.getNx())
      _vertices[i].x = (double)(_grid.getNx() - 1);
    else
      _vertices[i].x = (double)x;

    if (y < 0)
      _vertices[i].y = 0.0;
    else if (y >= _grid.getNy())
      _vertices[i].y = (double)(_grid.getNy() - 1);
    else
      _vertices[i].y = (double)y;
      
    if (i == 0)
    {
      min_x = (int)_vertices[i].x;
      max_x = (int)_vertices[i].x;
      min_y = (int)_vertices[i].y;
      max_y = (int)_vertices[i].y;
    }
    else
    {
      if (min_x > (int)_vertices[i].x)
	min_x = (int)_vertices[i].x;
      if (max_x < (int)_vertices[i].x)
	max_x = (int)_vertices[i].x;
      if (min_y > (int)_vertices[i].y)
	min_y = (int)_vertices[i].y;
      if (max_y < (int)_vertices[i].y)
	max_y = (int)_vertices[i].y;
    }
      
  } /* endfor - i */
    
#ifdef NOTNOW
  fprintf(stderr,
	  "Grid: min_x = %d, max_x = %d, min_y = %d, max_y = %d\n",
	  min_x, max_x, min_y, max_y);
  fprintf(stderr,
	  "      grid_nx = %d, grid_ny = %d\n",
	  _grid.getNx(), _grid.getNy());
#endif
  
  // Get the gridded filled polygon.
  //
  // point_filled will represent the total area of storm2
  points_filled = EG_fill_polygon(_vertices,
				  storm2_polygon->getNumPts(),
				  _grid.getNx(), _grid.getNy(),
				  0.0, 0.0,
				  1.0, 1.0,
				  _polyGrid,
				  1);
#ifdef NOTNOW      
  fprintf(stderr,
	  "Filled polygon (grid_nx = %d, grid_ny = %d, points_filled = %ld):\n",
	  _grid.getNx(), _grid.getNy(), points_filled);
  fprintf(stderr,
	  "               (min_x = %d, max_x = %d, min_y = %d, max_y = %d)\n",
	  min_x, max_x, min_y, max_y);
  
  for (int y = max_y; y >= min_y; y--)
  {
    fprintf(stderr, "   ");
    
    for (int x = min_x; x < max_x; x++)
    {
      fprintf(stderr,
	      "%2d ", _polyGrid[(y * _grid.getNx()) + x]);
    } /* endfor - x */
    
    fprintf(stderr, "\n");
	  
  } /* endfor - y */
#endif  

  // look for the intersection
  int numPolyGridPts = 0;
  for (int x = min_x; x <= max_x; x++)
  {
    for (int y = min_y; y <= max_y; y++)
    {
      int data_index = (y * _grid.getNx()) + x;
      
      if (_polyGrid[data_index] >=2)
      {
	++numPolyGridPts;
      }
      
    } /* endfor - y */
  } /* endfor - x */

#ifdef NOTNOW
  cerr << "number of points in storm2: " << points_filled << endl;
  cerr << "number of points in intersection: " <<numPolyGridPts  << endl;
#endif

  // calculate and return intersection percentge =  ;
  return (100.0 * ((double)numPolyGridPts / (double)points_filled));
}

void Tstorm::combineTstorms(const Tstorm &storm2)
{
  // First combine the polygons of the storms

  _combinePolygons(storm2);
  
}


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

bool Tstorm::getPolygonGrid(const Pjg &projection,
			    unsigned char *grid,
			    int &min_x, int &min_y,
			    int &max_x, int &max_y,
			    const int forecast_secs) const
{
  // Allocate space for the returned grid and initialize the values

  int grid_nx = projection.getNx();
  int grid_ny = projection.getNy();
  
  // Allocate space for the polygon vertices for the gridding call

  Polyline *polyline = forecastPoly(forecast_secs, false);

  Point_d *vertices = new Point_d[polyline->getNumPts()];
    
  // Convert the lat/lon polygon to the format needed for the
  // fill_polygon routine.  Get the bounding box while we're
  // here

  for (int i = 0; i < polyline->getNumPts(); i++)
  {
    int x, y;
      
    projection.xy2xyIndex(polyline->getX(i), polyline->getY(i),
			  x, y);
    if (x < 0)
      vertices[i].x = 0.0;
    else if (x >= grid_nx)
      vertices[i].x = (double)(grid_nx - 1);
    else
      vertices[i].x = (double)x;

    if (y < 0)
      vertices[i].y = 0.0;
    else if (y >= grid_ny)
      vertices[i].y = (double)(grid_ny - 1);
    else
      vertices[i].y = (double)y;
      
    if (i == 0)
    {
      min_x = (int)vertices[i].x;
      max_x = (int)vertices[i].x;
      min_y = (int)vertices[i].y;
      max_y = (int)vertices[i].y;
    }
    else
    {
      if (min_x > (int)vertices[i].x)
	min_x = (int)vertices[i].x;
      if (max_x < (int)vertices[i].x)
	max_x = (int)vertices[i].x;
      if (min_y > (int)vertices[i].y)
	min_y = (int)vertices[i].y;
      if (max_y < (int)vertices[i].y)
	max_y = (int)vertices[i].y;
    }
      
  } /* endfor - i */
    
#ifdef NOT_NOW
  fprintf(stderr,
	  "Grid: min_x = %d, max_x = %d, min_y = %d, max_y = %d\n",
	  min_x, max_x, min_y, max_y);
  fprintf(stderr,
	  "      grid_nx = %d, grid_ny = %d\n",
	  *grid_nx, *grid_ny);
#endif
      
  // Get the gridded filled polygon.

//  long points_filled =
    EG_fill_polygon(vertices,
		    polyline->getNumPts(),
		    grid_nx, grid_ny,
		    0.0, 0.0,
		    1.0, 1.0,
		    grid,
		    1);

    delete [] vertices;
    delete polyline;
  
    
#ifdef NOT_NOW      
  cerr << "Filled polygon (grid_nx = " << grid_nx
       << ", grid_ny = " << grid_ny
       << ", points_filled = " << points_filled << "):" << endl;
  cerr << "               (min_x = " << min_x << ", max_x = " << max_x
       << ", min_y = " << min_y << ", max_y = " << max_y << ")" << endl;
  
  for (int y = max_y; y >= min_y; y--)
  {
    fprintf(stderr, "   ");
    
    for (int x = min_x; x < max_x; x++)
    {
      fprintf(stderr,
	      "%2d ", grid[(y * grid_nx) + x]);
    } /* endfor - x */
    
    fprintf(stderr, "\n");
	  
  } /* endfor - y */
#endif

  return true;
}


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

bool Tstorm::getPolygonGridGrowKm(const Pjg &projection,
				  unsigned char *grid,
				  int &min_x, int &min_y,
				  int &max_x, int &max_y,
				  const double growth_km,
				  const int forecast_secs) const
{
  // Allocate space for the returned grid and initialize the values

  int grid_nx = projection.getNx();
  int grid_ny = projection.getNy();
  float grid_dx = projection.getDx();
  float grid_minx = projection.getMinx();

  // Get the forecast polygon

  Polyline *polygon = forecastPoly(forecast_secs);
  
  // Allocate space for the polygon vertices for the gridding call

  int num_pts = polygon->getNumPts();
  Point_d *vertices = new Point_d[num_pts + 1];
    
  // Calculate the new polygon centroid for use in the loop
  float fcstX, fcstY;
  polygon->getCenter( &fcstX, &fcstY );

  double forecast_lat, forecast_lon;
  _grid.xy2latlon(fcstX, fcstY,
		  forecast_lat, forecast_lon);
  
  // Loop through the points in the polygon, getting the values we
  // need for gridding it

  for (int i = 0; i < num_pts; i++)
  {
    // Get the new point for this vertex that is the indicated
    // extra distance from the storm centroid.

    double curr_lat, curr_lon;
    
    projection.xy2latlon(polygon->getX(i), polygon->getY(i),
			 curr_lat, curr_lon);
    
    double r, theta;
    
    Pjg::latlon2RTheta(forecast_lat, forecast_lon,
		       curr_lat, curr_lon,
		       r, theta);
    
    r += growth_km;
    if (r < 0.0) r = 0.0;
    
    double new_lat, new_lon;
    
    Pjg::latlonPlusRTheta(forecast_lat, forecast_lon,
			  r, theta,
			  new_lat, new_lon);
    
    // Now convert the new point to x, y indices

    int x, y;
      
    projection.latlon2xyIndex(new_lat, new_lon,
			      x, y);
      
    if (x < 0)
      vertices[i].x = 0.0;
    else if (x >= grid_nx)
      vertices[i].x = (double)(grid_nx - 1);
    else
      vertices[i].x = (double)x;

    if (y < 0)
      vertices[i].y = 0.0;
    else if (y >= grid_ny)
      vertices[i].y = (double)(grid_ny - 1);
    else
      vertices[i].y = (double)y;
      
    // Save the bounding box values

    if (i == 0)
    {
      min_x = (int)vertices[i].x;
      max_x = (int)vertices[i].x;
      min_y = (int)vertices[i].y;
      max_y = (int)vertices[i].y;
    }
    else
    {
      if (min_x > (int)vertices[i].x)
	min_x = (int)vertices[i].x;
      if (max_x < (int)vertices[i].x)
	max_x = (int)vertices[i].x;
      if (min_y > (int)vertices[i].y)
	min_y = (int)vertices[i].y;
      if (max_y < (int)vertices[i].y)
	max_y = (int)vertices[i].y;
    }
      
  } /* endfor - i */

  delete polygon;

  // Close the polygon

  vertices[num_pts].x = vertices[0].x;
  vertices[num_pts].y = vertices[0].y;

  float grid_maxx = grid_minx + grid_dx * grid_nx;

  // Are we on a near global sized grid
  // If so, check for storms passing through the edge
  if( Pjg(projection).getProjType() == PjgTypes::PROJ_LATLON &&
      grid_maxx - grid_minx >= 355) 
  {
    // If the storm has points that stretch across more
    // than 50% of the grid on a near global projection, 
    // then we must have a wrapping storm object
    if( ( max_x - min_x ) >=  grid_nx * .50 ) 
    { 
      Point_d *new_vertices = new Point_d[num_pts + 1];
      for (int i = 0; i < num_pts; i++)
      {
	// We know we have large and small vertex values
	// This round convert the big ones to minx and grid
	// the polygon
	if( vertices[i].x - min_x >= grid_nx * .50 )
	{
	  new_vertices[i].x = min_x;
	}
	else
	{
	  new_vertices[i].x = vertices[i].x;
	}
	new_vertices[i].y = vertices[i].y;
      }
      // close the polygon
      new_vertices[num_pts].x = new_vertices[0].x;
      new_vertices[num_pts].y = new_vertices[0].y;

      EG_fill_polygon(new_vertices, num_pts + 1,
		      grid_nx, grid_ny,
		      0.0, 0.0,
		      1.0, 1.0,
		      grid,
		      1);

      // This round convert the small ones to max_x and grid
      // the polygon on the other end of the grid
      for (int i = 0; i < num_pts; i++)
      {
	// look for a large jump in the x vertices
	// If so try to make into two polygons
	if( max_x - vertices[i].x  >= grid_nx * .50 )
	{
	  new_vertices[i].x = max_x;
	}
	else
	{
	  new_vertices[i].x = vertices[i].x;
	}
	new_vertices[i].y = vertices[i].y;
      }
      // close the polygon
      new_vertices[num_pts].x = new_vertices[0].x;
      new_vertices[num_pts].y = new_vertices[0].y;

      EG_fill_polygon(new_vertices, num_pts + 1,
		      grid_nx, grid_ny,
		      0.0, 0.0,
		      1.0, 1.0,
		      grid,
		      1);


      delete [] vertices;
      delete [] new_vertices;
      return true;
    }// ( max_x - min_x ) >=  grid_nx * .50
  } // grid_maxx - grid_minx > 355

#ifdef NOT_NOW
  fprintf(stderr,
	  "Grid: min_x = %d, max_x = %d, min_y = %d, max_y = %d\n",
	  min_x, max_x, min_y, max_y);
  fprintf(stderr,
	  "      grid_nx = %d, grid_ny = %d\n",
	  *grid_nx, *grid_ny);
#endif
      
  // Get the gridded filled polygon.

//  long points_filled =
    EG_fill_polygon(vertices, num_pts + 1,
		    grid_nx, grid_ny,
		    0.0, 0.0,
		    1.0, 1.0,
		    grid,
		    1);

    delete [] vertices;
  
    
#ifdef NOT_NOW      
  cerr << "Filled polygon (grid_nx = " << grid_nx
       << ", grid_ny = " << grid_ny
       << ", points_filled = " << points_filled << "):" << endl;
  cerr << "               (min_x = " << min_x << ", max_x = " << max_x
       << ", min_y = " << min_y << ", max_y = " << max_y << ")" << endl;
  
  for (int y = max_y; y >= min_y; y--)
  {
    fprintf(stderr, "   ");
    
    for (int x = min_x; x < max_x; x++)
    {
      fprintf(stderr,
	      "%2d ", grid[(y * grid_nx) + x]);
    } /* endfor - x */
    
    fprintf(stderr, "\n");
	  
  } /* endfor - y */
#endif

  return true;
}


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

bool Tstorm::getPolygonGridGrowPercent(const Pjg &projection,
				       unsigned char *grid,
				       int &min_x, int &min_y,
				       int &max_x, int &max_y,
				       const double growth_percent,
				       const int forecast_secs) const
{
  // Allocate space for the returned grid and initialize the values

  int grid_nx = projection.getNx();
  int grid_ny = projection.getNy();
  float grid_dx = projection.getDx();
  float grid_minx = projection.getMinx();
  
  // Get the forecast polygon

  Polyline *polygon = forecastPoly(forecast_secs);
  
  // Allocate space for the polygon vertices for the gridding call

  int num_pts = polygon->getNumPts();
  Point_d *vertices = new Point_d[num_pts + 1];
    
  // Calculate the new polygon centroid for use in the loop
  float fcstX, fcstY;
  polygon->getCenter( &fcstX, &fcstY );
  double forecast_lat, forecast_lon;
  _grid.xy2latlon(fcstX, fcstY,
		  forecast_lat, forecast_lon);
  
  // Loop through the points in the polygon, getting the values we
  // need for gridding it

  for (int i = 0; i < num_pts; i++)
  {
    // Get the new point for this vertex that is the indicated
    // extra distance from the storm centroid.

    double curr_lat, curr_lon;
    
    projection.xy2latlon(polygon->getX(i), polygon->getY(i),
			 curr_lat, curr_lon);
    
    double r, theta;
    
    Pjg::latlon2RTheta(forecast_lat, forecast_lon,
		       curr_lat, curr_lon,
		       r, theta);
    
    r *= (1.0 * growth_percent);
    if (r < 0.0) r = 0.0;
    
    double new_lat, new_lon;
    
    Pjg::latlonPlusRTheta(forecast_lat, forecast_lon,
			  r, theta,
			  new_lat, new_lon);
    
    // Now convert the new point to x, y indices

    int x, y;
      
    projection.latlon2xyIndex(new_lat, new_lon,
			      x, y);
      
    if (x < 0)
      vertices[i].x = 0.0;
    else if (x >= grid_nx)
      vertices[i].x = (double)(grid_nx - 1);
    else
      vertices[i].x = (double)x;

    if (y < 0)
      vertices[i].y = 0.0;
    else if (y >= grid_ny)
      vertices[i].y = (double)(grid_ny - 1);
    else
      vertices[i].y = (double)y;
      
    // Save the bounding box values

    if (i == 0)
    {
      min_x = (int)vertices[i].x;
      max_x = (int)vertices[i].x;
      min_y = (int)vertices[i].y;
      max_y = (int)vertices[i].y;
    }
    else
    {
      if (min_x > (int)vertices[i].x)
	min_x = (int)vertices[i].x;
      if (max_x < (int)vertices[i].x)
	max_x = (int)vertices[i].x;
      if (min_y > (int)vertices[i].y)
	min_y = (int)vertices[i].y;
      if (max_y < (int)vertices[i].y)
	max_y = (int)vertices[i].y;
    }
      
  } /* endfor - i */
    
  // Close the polygon

  vertices[num_pts].x = vertices[0].x;
  vertices[num_pts].y = vertices[0].y;

  float grid_maxx = grid_minx + grid_dx * grid_nx;
  
  // Are we on a near global sized grid
  // If so, check for storms passing through the edge
  if( projection.getProjType() == PjgTypes::PROJ_LATLON &&
      grid_maxx - grid_minx >= 355) 
  {
    // If the storm has points that stretch across more
    // than 50% of the grid on a near global projection, 
    // then we must have a wrapping storm object
    if( ( max_x - min_x ) >=  grid_nx * .50 ) 
    { 
      Point_d *new_vertices = new Point_d[num_pts + 1];
      for (int i = 0; i < num_pts; i++)
      {
	// We know we have large and small vertex values
	// This round convert the big ones to minx and grid
	// the polygon
	if( vertices[i].x - min_x >= grid_nx * .50 )
	{
	  new_vertices[i].x = min_x;
	}
	else
	{
	  new_vertices[i].x = vertices[i].x;
	}
	new_vertices[i].y = vertices[i].y;
	
      }
      // close the polygon
      new_vertices[num_pts].x = new_vertices[0].x;
      new_vertices[num_pts].y = new_vertices[0].y;

      EG_fill_polygon(new_vertices, num_pts + 1,
		      grid_nx, grid_ny,
		      0.0, 0.0,
		      1.0, 1.0,
		      grid,
		      1);

      // This round convert the small ones to max_x and grid
      // the polygon on the other end of the grid
      for (int i = 0; i < num_pts; i++)
      {
	// look for a large jump in the x vertices
	// If so try to make into two polygons
	if( max_x - vertices[i].x  >= grid_nx * .50 )
	{
	  new_vertices[i].x = max_x;
	}
	else
	{
	  new_vertices[i].x = vertices[i].x;
	}
	new_vertices[i].y = vertices[i].y;
      }
      // close the polygon
      new_vertices[num_pts].x = new_vertices[0].x;
      new_vertices[num_pts].y = new_vertices[0].y;

      EG_fill_polygon(new_vertices, num_pts + 1,
		      grid_nx, grid_ny,
		      0.0, 0.0,
		      1.0, 1.0,
		      grid,
		      1);


      delete [] vertices;
      delete [] new_vertices;
      return true;
    }// ( max_x - min_x ) >=  grid_nx * .50
  } // grid_maxx - grid_minx > 355
  
#ifdef NOT_NOW
  fprintf(stderr,
	  "Grid: min_x = %d, max_x = %d, min_y = %d, max_y = %d\n",
	  min_x, max_x, min_y, max_y);
  fprintf(stderr,
	  "      grid_nx = %d, grid_ny = %d\n",
	  *grid_nx, *grid_ny);
#endif
      
  // Get the gridded filled polygon.

//  long points_filled =
    EG_fill_polygon(vertices, num_pts + 1,
		    grid_nx, grid_ny,
		    0.0, 0.0,
		    1.0, 1.0,
		    grid,
		    1);

    delete [] vertices;
  
    
#ifdef NOT_NOW      
  cerr << "Filled polygon (grid_nx = " << grid_nx
       << ", grid_ny = " << grid_ny
       << ", points_filled = " << points_filled << "):" << endl;
  cerr << "               (min_x = " << min_x << ", max_x = " << max_x
       << ", min_y = " << min_y << ", max_y = " << max_y << ")" << endl;
  
  for (int y = max_y; y >= min_y; y--)
  {
    fprintf(stderr, "   ");
    
    for (int x = min_x; x < max_x; x++)
    {
      fprintf(stderr,
	      "%2d ", grid[(y * grid_nx) + x]);
    } /* endfor - x */
    
    fprintf(stderr, "\n");
	  
  } /* endfor - y */
#endif

  return true;
}


/////////////////////
// Private methods //
/////////////////////

void Tstorm::_clearRadials()
{
  _radials.erase( _radials.begin(), _radials.end() ); 
}

void Tstorm::_combinePolygons(const Tstorm &storm2)
{
  // We will combine the polygons by finding a "tangent" line
  // on each side of the polygons, approximately parallel to
  // the line between the polygon centroids.

  // First get the angle between the storm centroids

  double this_centroid_theta, this_centroid_r;
  
  _grid.latlon2RTheta(_centroidLat, _centroidLon,
		     storm2._centroidLat, storm2._centroidLon,
		     this_centroid_r, this_centroid_theta);
  
  // Now loop clockwise on this storm and counter-clockwise on
  // storm2 until we find the "tangent" line on that side.

  _normalizeTheta(this_centroid_theta, _startAz);
  
  double storm2_centroid_theta = this_centroid_theta + 180.0;
  storm2._normalizeTheta(storm2_centroid_theta, storm2._startAz);


  int curr_this_radial = (int)ceil((this_centroid_theta - _startAz) / _deltaAz);
  _normalizeRadialNum(curr_this_radial);
  
  int curr_storm2_radial =
    (int)floor((storm2_centroid_theta - storm2._startAz) / storm2._deltaAz);
  _normalizeRadialNum(curr_storm2_radial);
  
  double curr_this_theta, curr_this_r;
  
  _grid.latlon2RTheta(_detectionPoly->getY(curr_this_radial),
		     _detectionPoly->getX(curr_this_radial),
		     storm2._detectionPoly->getY(curr_storm2_radial),
		     storm2._detectionPoly->getX(curr_storm2_radial),
		     curr_this_r, curr_this_theta);
  
  _normalizeTheta(curr_this_theta, this_centroid_theta);
  
#ifdef NOTNOW
  cerr << "CLOCKWISE SEARCH:" << endl;
  cerr << "   curr_this_radial = " << curr_this_radial << endl;
  cerr << "   curr_this_theta = " << curr_this_theta << endl;
  cerr << "   curr_storm2_radial = " << curr_storm2_radial << endl;
  cerr << "   storm2_centroid_theta = " << storm2_centroid_theta << endl;
#endif

  bool radial_changed = true;
  
  while (radial_changed)
  {
    radial_changed = false;
    
    // Look clockwise around this storm and see if we can go to the next
    // radial.

    double new_r, new_theta;
    int check_radial;
    
    check_radial = curr_this_radial + 1;
    _normalizeRadialNum(check_radial);
    
    _grid.latlon2RTheta(_detectionPoly->getY(check_radial),
		       _detectionPoly->getX(check_radial),
		       storm2._detectionPoly->getY(curr_storm2_radial),
		       storm2._detectionPoly->getX(curr_storm2_radial),
		       new_r, new_theta);
    
    _normalizeTheta(new_theta, this_centroid_theta);

#ifdef NOTNOW    
    cerr << "   this storm check_radial = " << check_radial << endl;
    cerr << "   this storm new_theta = " << new_theta << endl;
#endif

    if (new_theta < curr_this_theta)
    {
      curr_this_theta = new_theta;
      curr_this_radial = check_radial;
      
      radial_changed = true;
    }
    
    // Now look counter-clockwise around storm2

    check_radial = curr_storm2_radial - 1;
    storm2._normalizeRadialNum(check_radial);
    
    _grid.latlon2RTheta(_detectionPoly->getY(curr_this_radial),
		       _detectionPoly->getX(curr_this_radial),
		       storm2._detectionPoly->getY(check_radial),
		       storm2._detectionPoly->getX(check_radial),
		       new_r, new_theta);
    
    
    _normalizeTheta(new_theta, this_centroid_theta);

#ifdef NOTNOW    
    cerr << "   storm2 check_radial = " << check_radial << endl;
    cerr << "   storm2 new_theta = " << new_theta << endl;
#endif

    if (new_theta > curr_this_theta)
    {
      curr_this_theta = new_theta;
      curr_storm2_radial = check_radial;
      
      radial_changed = true;
    }
    
  } /* endwhile - !done */
  
  int this_begin_radial = curr_this_radial;
  int storm2_end_radial = curr_storm2_radial;
  
#ifdef NOTNOW
  cerr << "this_begin_radial  = " << this_begin_radial << endl;
  cerr << "storm2_end_radial = " << storm2_end_radial << endl;
#endif

  // Now look the opposite way -- counter-clockwise around this storm
  // and clockwise around storm2

  curr_this_radial = (int)floor((this_centroid_theta - _startAz) / _deltaAz);
  _normalizeRadialNum(curr_this_radial);
  
  curr_storm2_radial =
    (int)ceil((storm2_centroid_theta - storm2._startAz) / storm2._deltaAz);
  _normalizeRadialNum(curr_storm2_radial);
  
  _grid.latlon2RTheta(_detectionPoly->getY(curr_this_radial),
		     _detectionPoly->getX(curr_this_radial),
		     storm2._detectionPoly->getY(curr_storm2_radial),
		     storm2._detectionPoly->getX(curr_storm2_radial),
		     curr_this_r, curr_this_theta);
  
  _normalizeTheta(curr_this_theta, this_centroid_theta);

#ifdef NOTNOW  
  cerr << "COUNTER-CLOCKWISE SEARCH:" << endl;
  cerr << "   curr_this_radial = " << curr_this_radial << endl;
  cerr << "   curr_this_theta = " << curr_this_theta << endl;
  cerr << "   curr_storm2_radial = " << curr_storm2_radial << endl;
  cerr << "   storm2_centroid_theta = " << storm2_centroid_theta << endl;
#endif

  radial_changed = true;
  
  while (radial_changed)
  {
    radial_changed = false;
    
    // Look clockwise around this storm and see if we can go to the next
    // radial.

    double new_r, new_theta;
    int check_radial;
    
    check_radial = curr_this_radial - 1;
    _normalizeRadialNum(check_radial);
    
    _grid.latlon2RTheta(_detectionPoly->getY(check_radial),
		       _detectionPoly->getX(check_radial),
		       storm2._detectionPoly->getY(curr_storm2_radial),
		       storm2._detectionPoly->getX(curr_storm2_radial),
		       new_r, new_theta);
    
    _normalizeTheta(new_theta, this_centroid_theta);

#ifdef NOTNOW    
    cerr << "   this storm check_radial = " << check_radial << endl;
    cerr << "   this storm new_theta = " << new_theta << endl;
#endif

    if (new_theta > curr_this_theta)
    {
      curr_this_theta = new_theta;
      curr_this_radial = check_radial;
      
      radial_changed = true;
    }
    
    // Now look counter-clockwise around storm2

    check_radial = curr_storm2_radial + 1;
    storm2._normalizeRadialNum(check_radial);
    
    _grid.latlon2RTheta(_detectionPoly->getY(curr_this_radial),
		       _detectionPoly->getX(curr_this_radial),
		       storm2._detectionPoly->getY(check_radial),
		       storm2._detectionPoly->getX(check_radial),
		       new_r, new_theta);
    
    _normalizeTheta(new_theta, this_centroid_theta);
    
#ifdef NOTNOW
    cerr << "   storm2 check_radial = " << check_radial << endl;
    cerr << "   storm2 new_theta = " << new_theta << endl;
#endif

    if (new_theta < curr_this_theta)
    {
      curr_this_theta = new_theta;
      curr_storm2_radial = check_radial;
      
      radial_changed = true;
    }
    
  } /* endwhile - !done */
  
  int this_end_radial = curr_this_radial;
  int storm2_begin_radial = curr_storm2_radial;
  
#ifdef NOTNOW
  cerr << "this_end_radial  = " << this_end_radial << endl;
  cerr << "storm2_begin_radial = " << storm2_begin_radial << endl;
#endif

  Polyline *new_polygon = new Polyline(_grid.getOriginLat(),
				       _grid.getOriginLon(), 0.0,
				       _centroidLon, _centroidLat,
				       Polyline::OPEN);
  
  if (this_begin_radial < this_end_radial)
  {
    for (int i = this_begin_radial; i <= this_end_radial; ++i)
      new_polygon->addPoint(_detectionPoly->getX(i), _detectionPoly->getY(i));
  }
  else
  {
    for (int i = this_begin_radial; i < _nSides; ++i)
      new_polygon->addPoint(_detectionPoly->getX(i), _detectionPoly->getY(i));
    for (int i = 0; i <= this_end_radial; ++i)
      new_polygon->addPoint(_detectionPoly->getX(i), _detectionPoly->getY(i));
  }
  
  if (storm2_begin_radial < storm2_end_radial)
  {
    for (int i = storm2_begin_radial; i <= storm2_end_radial; ++i)
      new_polygon->addPoint(storm2._detectionPoly->getX(i),
			   storm2._detectionPoly->getY(i));
  }
  else
  {
    for (int i = storm2_begin_radial; i < storm2._nSides; ++i)
      new_polygon->addPoint(storm2._detectionPoly->getX(i),
			   storm2._detectionPoly->getY(i));
    for (int i = 0; i <= storm2_end_radial; ++i)
      new_polygon->addPoint(storm2._detectionPoly->getX(i),
			   storm2._detectionPoly->getY(i));
  }
  
  new_polygon->closePolyline();
  
  setDetectionPoly(*new_polygon);
  
  delete new_polygon;
}


void Tstorm::_normalizeTheta(double &theta,
			     const double base_angle)
{
  while (theta < base_angle)
    theta += 360.0;
  
  while (theta >= base_angle + 360.0)
    theta -= 360.0;
}


void Tstorm::_normalizeRadialNum(int &radial_num) const
{
  while (radial_num < 0)
    radial_num += _nSides;
  
  while (radial_num >= _nSides)
    radial_num -= _nSides;
}


/**********************************************************************
 * _polygonsIntersect() - check that polygons have any type of 
 *                        intersection
 *
 */

bool Tstorm::_polygonsIntersect(Polyline *that_poly)
{
  
  _minLat = FLT_MAX;
  _minLon = FLT_MAX;
  _maxLat = -FLT_MAX;
  _maxLon = -FLT_MAX;

  for (int i = 0; i < _detectionPoly->getNumPts(); ++i)
  {
    _minLat = min(_minLat, (double)_detectionPoly->getY(i));
    _minLon = min(_minLon, (double)_detectionPoly->getX(i));
    _maxLat = max(_maxLat, (double)_detectionPoly->getY(i));
    _maxLon = max(_maxLon, (double)_detectionPoly->getX(i));
  }

  
  double that_min_lat = FLT_MAX;
  double that_min_lon = FLT_MAX;
  double that_max_lat = -FLT_MAX;
  double that_max_lon = -FLT_MAX;

  for (int i = 0; i < that_poly->getNumPts(); ++i)
  {
    that_min_lat = min(that_min_lat, (double)that_poly->getY(i));
    that_min_lon = min(that_min_lon, (double)that_poly->getX(i));
    that_max_lat = max(that_max_lat, (double)that_poly->getY(i));
    that_max_lon = max(that_max_lon, (double)that_poly->getX(i));
  }

  if(_intersectTest(_minLat, _minLon, _maxLat, _maxLon, 
		     that_min_lat, that_min_lon, that_max_lat, that_max_lon ) || 
     _intersectTest(that_min_lat, that_min_lon, that_max_lat, that_max_lon,
		     _minLat, _minLon, _maxLat, _maxLon))
  {
    return true;
  }

  return false;
}

bool Tstorm::_intersectTest(const double& a_min_lat, const double& a_min_lon, 
			    const double& a_max_lat, const double& a_max_lon, 
			    const double& b_min_lat, const double& b_min_lon, 
			    const double& b_max_lat, const double& b_max_lon)
{

  if ((a_min_lat <= b_min_lat) && (a_max_lat >= b_max_lat))
  {
    return true;
  }
  else if ((a_min_lon <= b_min_lon) && (a_max_lon >= b_max_lon))
  {
    return true;
  }
  else if ((a_max_lat >= b_min_lat) && (a_max_lon >= b_max_lon))
  {
    return true;
  }
  else if ((a_min_lat <= b_max_lat) && (a_max_lon >= b_min_lon))
  {
    return true;
  }
  else if ((a_min_lat <= b_max_lat) && (a_min_lon <= b_max_lon))
  {
    return true;
  }
  else if ((a_max_lat >= b_min_lat) && (a_min_lon <= b_max_lon))
  {
    return true;
  }

  return true;
}
