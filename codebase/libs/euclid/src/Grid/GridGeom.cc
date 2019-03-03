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

#include <cassert>
#include <math.h>
#include <toolsa/toolsa_macros.h>
#include <euclid/GridGeom.hh>
using namespace std;

//
// Static definitions
//   UNKNOWN values must be set to MAX to take advantage of the
//   feature in TDRP which allows the user to specify the "MAX" keyword
//   as a parameter value as an indicator of UNKNOWN.
//
//
const size_t   GridGeom::UNKNOWN_SIZE       = INT_MAX;
const float    GridGeom::UNKNOWN_RESOLUTION = FLT_MAX;
const float    GridGeom::UNKNOWN_LOWER_LEFT = FLT_MAX;
const float    GridGeom::FLOAT_TOLERANCE    = 0.001;

bool EQUAL(float _a, float _b);
#define EQUAL(_a, _b) (fabs(_a-_b) < GridGeom::FLOAT_TOLERANCE)

GridGeom::GridGeom()
{
   nx = UNKNOWN_SIZE;
   ny = UNKNOWN_SIZE;
   nz = UNKNOWN_SIZE;

   dx = UNKNOWN_RESOLUTION;
   dy = UNKNOWN_RESOLUTION;
   dz = UNKNOWN_RESOLUTION;

   minx = UNKNOWN_LOWER_LEFT;
   miny = UNKNOWN_LOWER_LEFT;
   minz = UNKNOWN_LOWER_LEFT;

   dimension  = 0;
   latCorner  = Projection::UNKNOWN_ORIGIN;
   lonCorner  = Projection::UNKNOWN_ORIGIN;
}

GridGeom::GridGeom( const GridGeom &source )
{
   copy( source );
}

GridGeom::GridGeom( size_t nx,   size_t ny,   size_t nz,
                    float  dx,   float  dy,   float  dz,
                    float  minx, float  miny, float  minz,
                    double latOrigin, double lonOrigin,
                    Projection::ProjId projectionId, double rotation )
{
   set( nx, ny, nz,
        dx, dy, dz,
        minx, miny, minz,
        latOrigin, lonOrigin, 
        projectionId, rotation );
}

GridGeom::~GridGeom()
{
}

GridGeom&
GridGeom::operator=( const GridGeom &source )
{
   copy( source );
   return( *this );
}

bool
GridGeom::operator!=( const GridGeom& other ) const
{
   return !(*this == other);
}

bool
GridGeom::operator==( const GridGeom& other ) const
{
   // Todo: Add tolerence to these comparisons!

   if ( !planeGeometriesMatch(other) ||
        nz != other.nz ||                      
        !EQUAL(dz, other.dz) ||
        !EQUAL(minz, other.minz))
   {
      return false;
   }

   return true;
}

bool
GridGeom::planeGeometriesMatch( const GridGeom& other ) const
{
   if ( nx != other.nx ||                                                
        ny != other.ny ||                            
        !EQUAL(dx, other.dx) ||
        !EQUAL(dy, other.dy) ||
        !EQUAL(minx, other.minx) ||
        !EQUAL(miny, other.miny) ||
        projection != other.projection )
   {
      return false;
   }

   return true;
}

// Tests if a cells in the planes are the same size,
//   and if the origin of one geom falls on the corner af a cell in other.
// 
bool
GridGeom::planeCellsCoincide( const GridGeom& other ) const
{
   // Test that the basic plane cell geometries match.
   // 
   if (projection != other.projection ||
       !EQUAL(dx, other.dx) ||
       !EQUAL(dy, other.dy))
   {
      return false;
   }

   // Test that the origin of one grid would land on the cell of the other,
   //   if infinately extended.
   // 
   float xdiff = minx - other.minx;
   float ydiff = miny - other.miny;
   float xnum = fabs(xdiff / dx);
   float ynum = fabs(ydiff / dy);
   float xrem = xnum - (float) floor(xnum);
   float yrem = ynum - (float) floor(ynum);
   if (!EQUAL(0.0, xrem) || !EQUAL(0.0, yrem)) {
      return false;
   }
   
   return true;
}

void
GridGeom::copy( const GridGeom& source )
{
   set( source );
}

bool
GridGeom::set( const GridGeom &source )
{
   return set( source.nx, source.ny, source.nz,
               source.dx, source.dy, source.dz,
               source.minx, source.miny, source.minz,
               source.projection.getLatOrigin(),
               source.projection.getLonOrigin(),
               source.projection.getType(),
               source.projection.getRotation() );
}

bool
GridGeom::set( size_t nzVal, float  dzVal, float  minzVal)
{
   nz = nzVal;
   dz = dzVal;
   minz = minzVal;
   updateDerived();
   return true;
}

bool
GridGeom::set( size_t nxVal, size_t nyVal,
               float dxVal, float dyVal,
               float minxVal, float minyVal)
{
   nx = nxVal;
   ny = nyVal;
   dx = dxVal;
   dy = dyVal;
   minx = minxVal;
   miny = minyVal;
   updateDerived();
   return true;
}

bool
GridGeom::set( size_t nxVal, size_t nyVal, size_t nzVal,
               float dxVal, float dyVal, float dzVal,
               float minxVal, float minyVal, float minzVal,
               double latOrigin, double lonOrigin, 
               Projection::ProjId projectionId, double rotation )
{
   nx = nxVal;
   ny = nyVal;
   nz = nzVal;
   dx = dxVal;
   dy = dyVal;
   dz = dzVal;
   minx = minxVal;
   miny = minyVal;
   minz = minzVal;

   projection.set( latOrigin, lonOrigin, projectionId, rotation );
   updateDerived();

   return true;
}

bool
GridGeom::suggest( const GridGeom &source )
{
   return suggest( source.nx,
                  source.ny,
                  source.nz,
                  source.dx,
                  source.dy,
                  source.dz,
                  source.minx,
                  source.miny,
                  source.minz,
                  source.projection.getLatOrigin(),
                  source.projection.getLonOrigin(),
                  source.projection.getType(),
                  source.projection.getRotation() );
}

bool
GridGeom::suggest( size_t nxVal, size_t nyVal, size_t nzVal,
                   float dxVal, float dyVal, float dzVal,
                   float minxVal, float minyVal, float minzVal,
                   double latOrigin, double lonOrigin, 
                   Projection::ProjId projectionId, double rotation )
{
   //
   // The logic here may seem a bit strange.  Here's the thinking...
   // If the number of values in any particular dimension (x, y, z)
   // is unknown, determining the correct number, for say nx,
   // will depend upon two things:
   //   1. dx -- so we set that first
   //   2. whether or not minx is known
   // In the case that minx in unknown, we simply accept the suggestion
   // If however, minx is already known, a better suggestion for nx
   // than the one passed in can be calculated from minx and dx.
   //
   float  max;
   bool   changed = false;

   //
   // X - dimension
   //
   if ( !isKnown( dx )) {
      dx = dxVal;
      changed = true;
   }

   if ( !isKnown( minx )) {
      minx = minxVal;
      changed = true;

      if ( !isKnown( nx )) {
         nx = nxVal;
         changed = true;
      }
   }
   else {
      if ( !isKnown( nx )) {
         max = minxVal + ((nxVal - 1) * dxVal);
         nx  = (size_t)(((max - minx + 1) / dx) + 0.5);
         changed = true;
      }
   }

   //
   // Y - dimension
   //
   if ( !isKnown( dy )) {
      dy = dyVal;
      changed = true;
   }

   if ( !isKnown( miny )) {
      miny = minyVal;
      changed = true;

      if ( !isKnown( ny )) {
         ny = nyVal;
         changed = true;
      }
   }
   else {
      if ( !isKnown( ny )) {
         max = minyVal + ((nyVal - 1) * dyVal);
         ny  = (size_t)(((max - miny + 1) / dy) + 0.5);
         changed = true;
      }
   }

   //
   // Z - dimension
   //
   if ( !isKnown( dz )) {
      dz = dzVal;
      changed = true;
   }

   if ( !isKnown( minz )) {
      minz = minzVal;
      changed = true;

      if ( !isKnown( nz )) {
         nz = nzVal;
         changed = true;
      }
   }
   else {
      if ( !isKnown( nz )) {
         max = minzVal + ((nzVal - 1) * dzVal);
         nz  = (size_t)(((max - minz + 1) / dz) + 0.5);
         changed = true;
      }
   }

   bool projectionChanged = projection.suggest( latOrigin, lonOrigin,
                                                projectionId, rotation );
   changed = changed  ||  projectionChanged;
   if ( changed ) {
      updateDerived();
   }

   return( changed );
}

void
GridGeom::updateDimension()
{
   dimension = 0;

   if ( isKnown( nx )  &&  isKnown( ny )  && isKnown( nz ) ) {
      //
      // Figure out what dimension the data is
      //
      if ( nx > 1 )
         dimension++;
      if ( ny > 1 )
         dimension++;
      if ( nz > 1 )
         dimension++;
   }
}

void
GridGeom::updateDerived()
{
   //
   // The projection's origin must be updated before the corners
   // since the caculation of the grid's corner locations is
   // dependent upon the projection's origin
   //
   projection.updateOrigin();

   updateCorners();
   updateDimension(); 
}

bool
GridGeom::setSingleZLevel()
{
   bool changed = false;

   if ( nz != 1 ) {
      nz = 1;
      changed = true;
   }
   return( changed );
}

int 
GridGeom::getClosestZ( float targetAltitude ) const
{
   assert( isKnown( minz ) && isKnown( nz ) && isKnown( dz ) );

   size_t   i = 0, closestZ = 0;
   float    currentAltitude, nextAltitude;

   //
   // Check for targetAltitude below the 0th level
   //
   if ( targetAltitude <= minz ) {
      return 0;
   }

   //
   // Now check the rest of the altitudes
   //
   for( i=1, currentAltitude=minz; i <= nz; i++, currentAltitude+=dz ) {
      if ( currentAltitude == targetAltitude || i == nz ) {
         closestZ = i-1;
         break;
      }
      else {
         //
         // Compare with the level above the current altitude
         //
         nextAltitude = currentAltitude + dz;
         if ( fabs( targetAltitude - currentAltitude) <=
              fabs( targetAltitude - nextAltitude) ) {
            closestZ = i-1;
            break;
         }
      }
   }
   return( closestZ );
}

size_t
GridGeom::getNumValues() const
{
   if ( isKnown( nx )  && isKnown( ny ) && isKnown( nz ) )
      return( nx * ny * nz );
   else
      return( UNKNOWN_SIZE );
}

void
GridGeom::index2xy( size_t cellIndex, size_t *xIndex, size_t *yIndex ) const
{
   assert( xIndex != NULL  &&  
           yIndex != NULL &&
           isGeometryKnown() &&
           dimension >= 2 );

   *yIndex = cellIndex / nx;
   *xIndex = cellIndex % nx;
}

void
GridGeom::index2km( size_t cellIndex, double *xKm, double *yKm ) const
{
   size_t xIndex, yIndex;

   assert( xKm != NULL  &&
           yKm != NULL );

   index2xy( cellIndex, &xIndex, &yIndex );
   xy2km( xIndex, yIndex, xKm, yKm );
}

void
GridGeom::index2latlon( size_t cellIndex, double *lat, double *lon ) const
{
   double xKm, yKm;

   assert( lat != NULL  &&
           lon != NULL );

   index2km( cellIndex, &xKm, &yKm );
   xy2latlon( xKm, yKm, lat, lon );
}

void
GridGeom::xy2km( size_t xIndex, size_t yIndex, double *xKm, double *yKm ) const
{
   assert( xKm != NULL  &&
           yKm != NULL );

   *xKm = minx + (xIndex * dx);
   *yKm = miny + (yIndex * dy);
}

void
GridGeom::km2xy( double xKm, double yKm,
                 size_t *xIndex, size_t *yIndex ) const
{
   //
   // The index must be calculated into an int rather than size_t
   // because of the possibility of a negative result
   //
   int index;

   index = (int)((xKm - minx) / dx + 0.5);
   if ( index <= 0 )
      *xIndex = 0;
   else
      *xIndex = MIN( (int) nx-1, index );

   index = (int)((yKm - miny) / dy + 0.5);
   if ( index <= 0 )
      *yIndex = 0;
   else
      *yIndex = MIN( (int) ny-1, index );
}

void
GridGeom::xy2latlon( size_t xIndex, size_t yIndex, 
                     double *lat, double *lon ) const
{
   double xKm, yKm;

   assert( lat != NULL  &&
           lon != NULL );

   xy2km( xIndex, yIndex,  &xKm, &yKm );
   xy2latlon( xKm, yKm, lat, lon );
}

int
GridGeom::kmDelta2xyDelta( double xKmDelta, double yKmDelta,
                           size_t *xDelta, size_t *yDelta ) const
{
   int status = 0;

   assert( xDelta != NULL  &&
           yDelta != NULL );

   //
   // Adapted from Mike Dixon's code in apps/trec/src/GridForecast/Vectors.cc
   //
   switch( projection.id ) {
      case Projection::LATLON :
           {
           double midLat = miny + dy * ny / 2.0;
           double latitude_factor = cos(midLat * DEG_TO_RAD);

           double dxKm = dx * KM_PER_DEG_AT_EQ * latitude_factor;
           double dyKm = dy * KM_PER_DEG_AT_EQ;

           *xDelta = (int)(dxKm / xKmDelta);
           *yDelta = (int)(dyKm / yKmDelta);
           break;
           }

      case Projection::FLAT :

           *xDelta = (int)((xKmDelta / dx) + 0.5);
           *yDelta = (int)((yKmDelta / dy) + 0.5);
           break;

      default:
           //
           // Set the spacing to something unreasonably high
           //
           *xDelta = 2 * nx;
           *yDelta = 2 * ny;
           status = -1;
   }

   return( status );
}
