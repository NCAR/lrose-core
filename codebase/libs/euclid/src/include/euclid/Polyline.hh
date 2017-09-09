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

#ifndef _POLYLINE_INC_
#define _POLYLINE_INC_

#include <vector>
#include <ctime>
#include <euclid/DistPoint.hh>
#include <euclid/Projection.hh>

class Polyline
{
public:
   enum shape { OPEN, CLOSED };

   Polyline( float olat, float olon, float rot, long numPoints, 
             float* xData, float* yData, float centerX, 
             float centerY, time_t t, shape c );

   Polyline( float olat, float olon, float rot, long numPoints,
             double* latData, double* lonData, float centerLat,
             float centerLon, time_t t, shape c );

   Polyline( float olat, float olon, float rot, long numPoints,
             float* xData, float* yData, 
             vector< float * >& vals, float centerX,
             float centerY, time_t t, shape c );

   Polyline( float olat, float olon, float rot, long numPoints,
             double* latData, double* lonData, 
             vector< float * >& vals, float centerLat,
             float centerLon, time_t t, shape c );

   Polyline( float olat, float olon, float rot, 
             float centerX, float centerY, shape c );

   Polyline();
   Polyline( const Polyline& p ){ copy( p ); }
   Polyline& operator=(const Polyline&);
   Polyline& operator+=(const Polyline&);

   virtual ~Polyline(){ clearPoints(); }

   //
   // Copy a polyline
   //
   void          copy( const Polyline& p );

   //
   // Set the points in the polyline
   //
   void          setPoints( float* xData, float* yData, int numPoints,
                            vector< float * >& vals, time_t t );

   //
   // Add a point to the polyline.
   //
   void addPoint(const float x, const float y);
  
   //
   //  Change the origin for the polyline
   //
   int	         changeOrigin( Projection& newProj );

   //
   // Close the polyline
   //
   void          closePolyline();

   //
   // Return values
   //
   void          getOrigin( float *lat, float *lon );
   inline double getRotation(){ return( proj.getRotation() ); }
   inline time_t getTime(){ return( validTime ); }
   void          getCenter( float *x, float *y );
   inline int    getNumPts(){ return( (int) points.size() ); }
   inline bool   isClosed() const { return( pshape==CLOSED ? true : false ); }
   int           getNumVals();
   int           getDistLimits( float* xMin, float* yMin, 
                                float* xMax, float* yMax );

   void          calcCentroid(double &x, double &y) const;
   void          calcCentroid(float &x, float &y) const;
  
   //
   // Get individual point values
   //
   inline float  getX( int idex ){ return( points[idex]->getXDist() ); }
   inline float  getY( int idex ){ return( points[idex]->getYDist() ); }
   inline vector< float >* getValues( int idex )
                 { return( points[idex]->getValues() ); }

   //
   // Set individual point values
   //
   inline void setX( int idex, float xDist )
         { points[idex]->setXDist( xDist ); }
   inline void setY( int idex, float yDist )
         { points[idex]->setYDist( yDist ); }


   void init( float olat, float olon, float rot, long numPoints, 
              float* xData, float* yData, float centerX, 
              float centerY, time_t t, shape c );

   bool inPolyline( double latitude, double longitude );

   //
   // Constants
   //
   static const float MISSING_VAL;

protected:

   vector< DistPoint * > points;

   shape pshape;
   float xCenter, yCenter;
   time_t validTime;
   Projection proj;

   //
   // Clean up
   //
   void clearPoints();

  
};

# endif
