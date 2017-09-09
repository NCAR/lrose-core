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
//----------------------------------------------------------------
//
// Polyline routines.
//
//----------------------------------------------------------------

#include <algorithm>
#include <cfloat>
#include <euclid/Polyline.hh>
using namespace std;

//
// Constants
//
const float Polyline::MISSING_VAL = FLT_MAX;

Polyline::Polyline( float olat, float olon, float rot, long numPoints, 
		    float* xData, float* yData, float centerX, 
                    float centerY, time_t t, shape c )
{ 
   pshape    = c;
   validTime = t;
   xCenter   = centerX;
   yCenter   = centerY;

   if( !isClosed() ) {
      xCenter = MISSING_VAL;
      yCenter = MISSING_VAL;
   }

   proj.set( olat, olon, Projection::FLAT, rot );
   
   DistPoint *newPoint;
   for( int i = 0; i < numPoints; i++ ) {
      newPoint = new DistPoint( xData[i], yData[i], 0.0, this );
      points.push_back( newPoint );
   }

   //
   // If the polyline is supposed to be closed, then make sure it is.
   //
   int lastPt = points.size() - 1;
   if( isClosed() && 
       ((points[0]->getXDist() != points[lastPt]->getXDist()) || 
        (points[0]->getYDist() != points[lastPt]->getYDist()))) {
      newPoint = new DistPoint( *(points[0]) );
      points.push_back( newPoint );
   }

}

Polyline::Polyline( float olat, float olon, float rot, long numPoints,
                    double* latData, double* lonData, float centerLat,
                    float centerLon, time_t t, shape c ) 
{
   pshape    = c;
   validTime = t;
   xCenter   = MISSING_VAL;
   yCenter   = MISSING_VAL;

   proj.set( olat, olon, Projection::FLAT, rot );
   
   double     xData, yData;
   DistPoint *newPoint;

   //
   // Find the x and y values of the center
   //
   if( isClosed() ) {
      proj.latlon2xy( (double) centerLat, (double) centerLon,
                      (double *) &xCenter, (double *) &yCenter );
   }

   //
   // Set up value vector and first point
   //
   proj.latlon2xy( latData[0], lonData[0], &xData, &yData );
   newPoint = new DistPoint( xData, yData, 0.0, this );
   points.push_back( newPoint );

   //
   // Set up remaining points
   //
   for( int i = 1; i < numPoints; i++ ) {
      proj.latlon2xy( latData[i], lonData[i], &xData, &yData );
      newPoint = new DistPoint( xData, yData, 0.0, this );
      points.push_back( newPoint );
   }

   //
   // If the polyline is supposed to be closed, then make sure it is.
   //
   int lastPt = points.size() - 1;
   if( isClosed() && 
       ((points[0]->getXDist() != points[lastPt]->getXDist()) || 
        (points[0]->getYDist() != points[lastPt]->getYDist()))) {
      newPoint = new DistPoint( *(points[0]) );
      points.push_back( newPoint );
   }

}

Polyline::Polyline( float olat, float olon, float rot, long numPoints,
                    float* xData, float* yData, 
                    vector< float * >& vals, float centerX,
                    float centerY, time_t t, shape c ) 
{
   pshape    = c;
   validTime = t;
   xCenter   = centerX;
   yCenter   = centerY;

   if( !isClosed() ) {
      xCenter = MISSING_VAL;
      yCenter = MISSING_VAL;
   }

   proj.set( olat, olon, Projection::FLAT, rot );

   
   float     *dataField;
   DistPoint *newPoint;
   vector< float > values;
   vector< float * >::iterator it;

   //
   // Set up value vector and first point
   //
   for( it = vals.begin(); it != vals.end(); it++ ) {
      dataField = *it;
      values.push_back( dataField[0] );
   }
   newPoint = new DistPoint( values, xData[0], yData[0], 0.0, this );
   points.push_back( newPoint );

   //
   // Set up remaining points
   //
   for( int i = 1; i < numPoints; i++ ) {
      for( int j = 0; j < (int) values.size(); j++ ) {
	 values[j] = vals[j][i];
      }
      newPoint = new DistPoint( values, xData[i], yData[i], 0.0, this );
      points.push_back( newPoint );
   }

   //
   // If the polyline is supposed to be closed, then make sure it is.
   //
   int lastPt = points.size() - 1;
   if( isClosed() && 
       ((points[0]->getXDist() != points[lastPt]->getXDist()) || 
        (points[0]->getYDist() != points[lastPt]->getYDist()))) {
      newPoint = new DistPoint( *(points[0]) );
      points.push_back( newPoint );
   }

   //
   // remove temporary vector
   //
   values.erase( values.begin(), values.end() );
}

Polyline::Polyline( float olat, float olon, float rot, long numPoints,
                    double* latData, double* lonData, 
                    vector< float * >& vals, float centerLat,
                    float centerLon, time_t t, shape c ) 
{
   pshape    = c;
   validTime = t;
   xCenter   = MISSING_VAL;
   yCenter   = MISSING_VAL;

   proj.set( olat, olon, Projection::FLAT, rot );

   float     *dataField;
   double     xData, yData;
   DistPoint *newPoint;
   vector< float > values;
   vector< float * >::iterator it;

   //
   // Find the x and y values of the center
   //
   if( isClosed() ) {
      proj.latlon2xy( (double) centerLat, (double) centerLon, 
                      (double *) &xCenter, (double *) &yCenter );
   }

   //
   // Set up value vector and first point
   //
   for( it = vals.begin(); it != vals.end(); it++ ) {
      dataField = *it;
      values.push_back( dataField[0] );
   }
   proj.latlon2xy( latData[0], lonData[0], &xData, &yData );
   newPoint = new DistPoint( values, xData, yData, 0.0, this );
   points.push_back( newPoint );

   //
   // Set up remaining points
   //
   for( int i = 1; i < numPoints; i++ ) {
      for( int j = 0; j < (int) values.size(); j++ ) {
	 values[j] = vals[j][i];
      }
      proj.latlon2xy( latData[i], lonData[i], &xData, &yData );
      newPoint = new DistPoint( values, xData, yData, 0.0, this );
      points.push_back( newPoint );
   }

   //
   // If the polyline is supposed to be closed, then make sure it is.
   //
   int lastPt = points.size() - 1;
   if( isClosed() && 
       ((points[0]->getXDist() != points[lastPt]->getXDist()) || 
        (points[0]->getYDist() != points[lastPt]->getYDist()))) {
      newPoint = new DistPoint( *(points[0]) );
      points.push_back( newPoint );
   }

   //
   // remove temporary vector
   //
   values.erase( values.begin(), values.end() );
}
   

Polyline::Polyline( float olat, float olon, float rot, float centerX,
                    float centerY, shape c ) 
{
   pshape    = c;
   validTime = 0;
   xCenter   = centerX;
   yCenter   = centerY;

   if( !isClosed() ) {
      xCenter = MISSING_VAL;
      yCenter = MISSING_VAL;
   }

   proj.set( olat, olon, Projection::FLAT, rot );
   
}

Polyline::Polyline ()
{
   pshape    = Polyline::CLOSED;
   xCenter   = 0.0;
   yCenter   = 0.0;
   validTime = 0;
}

Polyline&
Polyline::operator=(const Polyline &p)
{
   copy( p );
   return( *this );
}

Polyline&
Polyline::operator+=(const Polyline &p) 
{

   if( !isClosed() && !p.isClosed() && proj == p.proj ) {
      vector< DistPoint* >::const_iterator it;
      for( it = p.points.begin();
	   it != p.points.end();
	   it++ ) {
	 DistPoint *newPoint = new DistPoint( *(*it) );
	 points.push_back( newPoint );
      }
   }

   return( *this );
   
}

void
Polyline::copy( const Polyline &p )
{
   clearPoints();
   
   pshape    = p.pshape;
   xCenter   = p.xCenter;
   yCenter   = p.yCenter;
   validTime = p.validTime;

   proj.set( p.proj );
   
   DistPoint *pointCopy;
   int nPts = p.points.size();
   for( int i = 0; i < nPts; i++ ) {
      pointCopy = new DistPoint( *(p.points[i]) );
      points.push_back( pointCopy );
   }

}

void
Polyline::addPoint(const float x, const float y)
{
  DistPoint *new_point = new DistPoint(x, y);
  
  // If the polyline is supposed to be closed, make sure it
  // stays closed

  if (isClosed())
  {
    delete points[points.size() - 1];
    points[points.size() - 1] = new_point;
    
    DistPoint *end_point = new DistPoint(*(points[0]));
    points.push_back(end_point);
  }
  else
  {
    points.push_back(new_point);
  }
}


void
Polyline::setPoints( float* xData, float* yData, int numPoints,
                     vector< float * >& vals, time_t t )
{
   DistPoint *nextPoint;
   float     *dataField;

   vector< float > values;
   vector< float * >::iterator it;

   validTime = t;

   //
   // Get rid of old points
   //
   clearPoints();

   //
   // Set up first point
   //
   for( it = vals.begin(); it != vals.end(); it++ ) {
      dataField = *it;
      values.push_back( dataField[0] );
   }
   nextPoint = new DistPoint( values, xData[0], yData[0], 0.0, this );
   points.push_back( nextPoint );
   
   //
   // Set up the rest of the points
   //
   for( int i = 1; i < numPoints; i++ ) {
      for( int j = 0; j < (int) values.size(); j++ ) {
	 values[j] = vals[j][i];
      }
      nextPoint = new DistPoint( values, xData[i], yData[i], 0.0, this );
      points.push_back( nextPoint );
   }

   //
   // If the polyline is supposed to be closed, then make sure it is.
   //
   int lastPt = points.size() - 1;
   if( isClosed() && 
       ((points[0]->getXDist() != points[lastPt]->getXDist()) || 
        (points[0]->getYDist() != points[lastPt]->getYDist()))) {
      nextPoint = new DistPoint( *(points[0]) );
      points.push_back( nextPoint );
   }

   //
   // remove temporary vector
   //
   values.erase( values.begin(), values.end() );
   
}

int
Polyline::changeOrigin( Projection& newProj )
{
  double tempLat, tempLon;
  double newX, newY;

  //
  // Make sure new projection is FLAT too
  //
  if( newProj.getType() != Projection::FLAT )
     return( -1 );

  //
  // If projections are the same, do nothing
  //
  if( proj == newProj )
     return( 0 );
  
  //
  // Translate the coordinates.
  //
  int nPts = points.size();
  for (int i = 0; i < nPts; ++i) {
    proj.xy2latlon( points[i]->getXDist(), points[i]->getYDist(), 
                    &tempLat, &tempLon );
    newProj.latlon2xy( tempLat, tempLon, &newX, &newY );
    points[i]->setXDist( newX );
    points[i]->setYDist( newY );
  } 

  //
  // Translate the center.
  //
  if( isClosed() ) {
     proj.xy2latlon( xCenter, yCenter, &tempLat, &tempLon );
     newProj.latlon2xy( tempLat, tempLon, &newX, &newY );
     xCenter = newX;
     yCenter = newY;
  }

  //
  // Replace the projection
  //
  proj.set( newProj );

  return( 0 );
  
}

void
Polyline::closePolyline() 
{
   if( isClosed() )
      return;
   
   int lastPt = points.size() - 1;
   if( ((points[0]->getXDist() != points[lastPt]->getXDist()) || 
        (points[0]->getYDist() != points[lastPt]->getYDist()))) {
      DistPoint *newPoint = new DistPoint( *(points[0]) );
      points.push_back( newPoint );
   }

   pshape = CLOSED;
}


void 
Polyline::getOrigin( float *lat, float *lon ) 
{
   *lat = proj.getLatOrigin();
   *lon = proj.getLonOrigin();
}
 
void 
Polyline::getCenter( float *x, float *y ) 
{
   *x = xCenter;
   *y = yCenter;
}

int
Polyline::getNumVals() 
{
   if( points.size() < 1 ) {
      return( 0 );
   } 

   return( points[0]->getNumVals() );
   
}

int
Polyline::getDistLimits( float* xMin, float* yMin, float* xMax, float* yMax ) 
{
   float minx = FLT_MAX;
   float miny = FLT_MAX;
   float maxx = FLT_MIN;
   float maxy = FLT_MIN;

   float xDist, yDist;

   vector< DistPoint* >::iterator it;
   for( it = points.begin(); it != points.end(); it++ ) {
      xDist = (*it)->getXDist();
      yDist = (*it)->getYDist();
      
      if( xDist < minx )
	 minx = xDist;
      if( yDist < miny )
	 miny = yDist;
      if( xDist > maxx )
	 maxx = xDist;
      if( yDist > maxy )
	 maxy = yDist;
   }

   *xMin = minx;
   *yMin = miny;
   *xMax = maxx;
   *yMax = maxy;

   if( points.size() < 1 )
      return( -1 );
   
   return( 0 );
   
}


void
Polyline::calcCentroid(double &x, double &y) const
{
  // Check for degenerate case

  if (points.size() <= 0)
  {
    x = 0.0;
    y = 0.0;
    
    return;
  }
  
  double x_sum = 0.0;
  double y_sum = 0.0;
  
  for (size_t i = 0; i < points.size(); ++i)
  {
    x_sum += points[i]->getXDist();
    y_sum += points[i]->getYDist();
  }
  
  x = x_sum / points.size();
  y = y_sum / points.size();
  
}


void
Polyline::calcCentroid(float &x, float &y) const
{
  double x_double, y_double;
  
  calcCentroid(x_double, y_double);
  
  x = (float)x_double;
  y = (float)y_double;
}


void
Polyline::clearPoints() 
{
   vector< DistPoint * >::iterator it;
   
   for( it = points.begin(); it != points.end(); it++ ) {
      delete (*it);
   }
   points.erase( points.begin(), points.end() );
}

void Polyline::init( float olat, float olon, float rot, long numPoints, 
		     float* xData, float* yData, float centerX, 
                     float centerY, time_t t, shape c )
{

   
   pshape    = c;
   validTime = t;
   xCenter   = centerX;
   yCenter   = centerY;

   if( !isClosed() ) {
      xCenter = MISSING_VAL;
      yCenter = MISSING_VAL;
   }

   proj.set( olat, olon, Projection::FLAT, rot );
   
   DistPoint *newPoint;
   for( int i = 0; i < numPoints; i++ ) {
      newPoint = new DistPoint( xData[i], yData[i], 0.0, this );
      points.push_back( newPoint );
   }

   //
   // If the polyline is supposed to be closed, then make sure it is.
   //
   int lastPt = points.size() - 1;
   if( isClosed() && 
       ((points[0]->getXDist() != points[lastPt]->getXDist()) || 
        (points[0]->getYDist() != points[lastPt]->getYDist()))) {
      newPoint = new DistPoint( *(points[0]) );
      points.push_back( newPoint );
   }

}

//
// This is the same algorithm as  EG_inside_poly(...)
// Determine if the point (x,y) is inside the closed Polyline
//
bool Polyline::inPolyline( double latitude, double longitude )
{   
  //
  // If the Polyline is NOT closed, return false.
  //
  if( !(isClosed()) )
      return (false);

  double x_coord;
  double y_coord;

  
  proj.latlon2xy( latitude,longitude, 
		  &x_coord, &y_coord);

  //
  // n is the number of vertices of the polyline
  // which is the number of points in the polyline
  // minus 1 since the last point == first point.
  //
  int n =  points.size() - 1;

  //
  // point index is i, i1 = i-1 mod n   
  //
  int   i, i1;          

  //
  // x intersection of e = edge(i-1,i), with ray parallel to the
  // x-axis through (x_coord, y_coord) 
  //
  float        x;     

  //
  // number of edge/horizontal ray crossings
  //
  int   crossings = 0;  

  //
  // For each edge e=(i-1,i), see if horizontal ray crosses.
  //
  for( i = 0; i < n; i++ )
    {
      i1 = ( i + n - 1 ) % n;
 
      //
      // coordinates of one vertex (one end of edge)
      //
      float x1 = points[i]->getXDist();
      float y1 = points[i]->getYDist();

      //
      // coordinates of the other vertex (other end of edge)
      //
      float x2 =  points[i1]->getXDist();
      float y2 = points[i1]->getYDist();

      //
      // if e straddles the x-axis...
      //
      if( (( y1 > y_coord ) && ( y2 <= y_coord )) ||
          (( y2 > y_coord ) && ( y1 <= y_coord )) )
        {
	  //
          // e straddles horizontal ray, so compute intersection with ray.
	  //
          x = x2 + (y_coord - y2) * (x1 - x2) / (y1 - y2);
  
	  //
          // crosses ray if x_coord (x coordinate of head of ray)
          // is less than x coordinate of intersection. (This is
          // true if ray points toward the y axis!).
          //
          if (x_coord < x) crossings++;
	}
      
    } //end for(...)
  
  //
  // (x_coord,y_coord) inside Polyline if the number of crossings is odd. 
  //
  if( (crossings % 2) == 1 )
    return(true);
  
  //
  // Otherwise return false
  //
  return (false);
}


