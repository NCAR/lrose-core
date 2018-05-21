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
////////////////////////////////////////////////////////////////////////////////
//
// Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
// January 1998
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _GRID_GEOM_INC_
#define _GRID_GEOM_INC_

#include <limits.h>
#include <float.h>
#include <cstdio>
#include <cstdlib>
#include <euclid/Projection.hh>

// #include <euclid/TypeGrid.hh>

//
// Forward class declarations
//
class Grid;
template <class T> class TypeGrid;

class GridGeom {

public:

   static const size_t  UNKNOWN_SIZE;
   static const float   UNKNOWN_RESOLUTION;
   static const float   UNKNOWN_LOWER_LEFT;
   static const float   FLOAT_TOLERANCE;

   GridGeom();
   GridGeom( const GridGeom& source );

   GridGeom( size_t nx,   size_t ny,   size_t nz,
             float  dx,   float  dy,   float  dz,
             float  minx, float  miny, float  minz,
             double latOrigin, double lonOrigin, 
             Projection::ProjId projectionId, 
             double rotation = 0.0 );

  ~GridGeom();

   void        copy( const GridGeom& source );
   GridGeom&   operator=( const GridGeom& source );
   bool        operator==( const GridGeom& other ) const;
   bool        operator!=( const GridGeom& other ) const;
   bool        planeGeometriesMatch( const GridGeom& other ) const;
   bool        planeCellsCoincide( const GridGeom& other ) const;

   bool        set( const GridGeom &source );

   bool        set( size_t nz, float  dz, float  minz);

   bool        set( size_t nx, size_t ny,
                    float  dx, float  dy,
                    float  minx, float  miny);

   bool        set( size_t nx,   size_t ny,   size_t nz,
                    float  dx,   float  dy,   float  dz,
                    float  minx, float  miny, float  minz,
                    double latOrigin, double lonOrigin, 
                    Projection::ProjId projectionId, 
                    double rotation = 0.0 );

   bool        setSingleZLevel();

   bool        suggest( const GridGeom &source );

   bool        suggest( size_t nx,   size_t ny,   size_t nz,
                        float  dx,   float  dy,   float  dz,
                        float  minx, float  miny, float  minz,
                        double latOrigin, double lonOrigin, 
                        Projection::ProjId projectionId, 
                        double rotation = 0.0 );

   //
   // Fetching info
   //
   size_t    getNx() const { return nx; }
   size_t    getNy() const { return ny; }
   size_t    getNz() const { return nz; }
   float     getDx() const { return dx; }
   float     getDy() const { return dy; }
   float     getDz() const { return dz; }
   float     getMinx() const { return minx; }
   float     getMiny() const { return miny; }
   float     getMinz() const { return minz; }
   int       getClosestZ( float targetAltitude ) const;
   double    getLatCorner() const { return latCorner; }
   double    getLonCorner() const { return lonCorner; }
   size_t    getDimension() const { return dimension; }
   size_t    getNumValues() const;

   //
   // Fetching projection info
   //
   double    getLatOrigin() const
                      { return projection.getLatOrigin(); }

   double    getLonOrigin() const
                      { return projection.getLonOrigin(); }

   double    getRotation() const
                      { return projection.getRotation(); }

   void      getOrigin( double *lat, double *lon, double *rotation = 0 ) const
                      { projection.getOrigin( lat, lon, rotation ); }

   int       translateOrigin( float x, float y)
                      { return projection.translateOrigin( x, y ); }

   //
   // Projection translations
   //
   int       translateOrigin( double x, double y )
                      { return projection.translateOrigin( x, y ); }

   int       xy2latlon( double x, double y, double *lat, double *lon ) const
                      { return projection.xy2latlon( x, y, lat, lon ); }

   int       latlon2xy( double lat, double lon, double *x, double *y) const
                      { return projection.latlon2xy( lat, lon, x, y ); }

    //
    // Convert between cell indicies
    //
    // Return: 0 for success -1 for failure
    //
    void   index2xy( size_t cellIndex,
                     size_t *xIndex, size_t *yIndex ) const;

    //
    // Convert between cell indicies and world positions
    //
    // Return: 0 for success -1 for failure
    //
    void   index2latlon( size_t cellIndex,
                         double *lat, double *lon ) const;

    void   xy2latlon( size_t xIndex, size_t yIndex,
                      double *lat, double *lon ) const;

    //
    // Convert between cell indicies and grid position relative to the origin  
    //
    // Return: 0 for success -1 for failure
    //
    void   index2km( size_t cellIndex,
                     double *xKm, double *yKm ) const;

    void   xy2km( size_t xIndex, size_t yIndex,
                  double *xKm, double *yKm ) const;

    void   km2xy( double xKm, double yKm,
                  size_t *xIndex, size_t *yIndex ) const;

    int    kmDelta2xyDelta( double xKmDelta, double yKmDelta,
                            size_t *xDelta, size_t *yDelta ) const;

   //
   // How much do we know?
   //
   bool isKnown( size_t val ) const
               { return val == UNKNOWN_SIZE ? false : true; }
   bool isKnown( float  val ) const
               { return val == UNKNOWN_RESOLUTION ? false : true; }
   bool isKnown( const Projection &projection ) const
               { return projection.isProjectionKnown(); }
   bool isGeometryKnown() const
               { return isKnown( nx ) && isKnown( ny ) && isKnown( nz ) &&
                        isKnown( dx ) && isKnown( dy ) && isKnown( dz ) &&
                        isKnown( minx ) && isKnown( miny ) && isKnown( minz ) &&
                        isKnown( projection ); }



private:

   //
   // Geometry values specified by the caller
   //
   size_t           nx;
   size_t           ny;
   size_t           nz;
   float            dx;
   float            dy;
   float            dz;
   float            minx;
   float            miny;
   float            minz;
   Projection       projection;

   //
   // Derived values
   //
   size_t           dimension;
   double           latCorner;
   double           lonCorner;

   void             updateDimension();
   void             updateCorners(){ xy2latlon( minx, miny, 
                                                &latCorner, &lonCorner ); }
   void             updateDerived();

   friend class Grid;
   friend class TypeGrid<unsigned char>;
   friend class TypeGrid<unsigned short>;
   friend class TypeGrid<int>;
   friend class TypeGrid<float>;
   friend class TypeGrid<double>;
};

#endif
