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

#include <cstdio>
#include <iostream>
#include <limits.h>
#include <float.h>
#include <euclid/TypeGrid.hh>
#include <euclid/Polyline.hh>
#include <euclid/FuzzyFcn.hh>
#include <euclid/IndexPoint.hh>
#include <euclid/DistPoint.hh>

//
//
// Put this last 'cause it has some name clashes 
// with standard /usr/include files
//
#include <euclid/geometry.h>
using namespace std;

#define CLIP(x,l,u) (x < l) ? l : ((x > u)? u : x)

Grid::Grid(DataType type)
     : dataType(type),
       geometry()
{
   composite = false;
   maxz      = GridGeom::UNKNOWN_RESOLUTION;
}

Grid::Grid( const Grid& source )
     : dataType(source.dataType),
       geometry()
{
   composite = false;
   maxz      = GridGeom::UNKNOWN_RESOLUTION;

   copy( source );
}

Grid::Grid( DataType type,
            const GridGeom & geom)
     : dataType(type),
       geometry()
{
   composite = false;
   maxz      = GridGeom::UNKNOWN_RESOLUTION;
   geometry.set( geom );
}

Grid::Grid( DataType type,
            size_t nx,   size_t ny,   size_t nz,
            float  dx,   float  dy,   float  dz,
            float  minx, float  miny, float  minz,
            double latOrigin, double lonOrigin,
            Projection::ProjId projectionId,
            double      rotation  /* = 0.0*/ )
     : dataType(type),
       geometry()
{
   composite = false;
   maxz      = GridGeom::UNKNOWN_RESOLUTION;
   geometry.set( nx, ny, nz,
                 dx, dy, dz,
                 minx, miny, minz,
                 latOrigin, lonOrigin,
                 projectionId, rotation);
}

Grid::~Grid()
{
}

void
Grid::copy( const Grid& source )
{
   // Todo: Check instantiation type? Or just blindly copy this part?

   //
   // Copy the attributes.
   //
   composite = source.composite;
   maxz      = source.maxz;
   geometry  = source.geometry;
}

bool
Grid::suggestGeometry( const Grid &sourceGrid )
{
   bool changed = geometry.suggest( sourceGrid.geometry );

   if ( changed ) {
      updateDerivedGeom( &sourceGrid );
   }

   return( changed );
}

bool
Grid::suggestGeometry( const GridGeom &sourceGeom )
{
   bool changed = geometry.suggest( sourceGeom );

   if ( changed ) {
      updateDerivedGeom();
   }

   return( changed );
}

bool
Grid::setGeometry( const Grid &sourceGrid )
{
   bool changed = geometry.set( sourceGrid.geometry );

   if ( changed ) {
      updateDerivedGeom( &sourceGrid );
   }

   return( changed );
}

bool
Grid::setGeometry( const GridGeom &sourceGeom )
{
   bool changed = geometry.set( sourceGeom );

   if ( changed ) {
      updateDerivedGeom();
   }

   return( changed );
}

bool
Grid::setGeometry( size_t nx,   size_t ny,   size_t nz,
                     float  dx,   float  dy,   float  dz,
                     float  minx, float  miny, float  minz,
                     double latOrigin, double lonOrigin,
                     Projection::ProjId projectionId,
                     double rotation /* = 0.0 */ )
{
   bool changed = geometry.set( nx, ny, nz,
                                dx, dy, dz,
                                minx, miny, minz,
                                latOrigin, lonOrigin,
                                projectionId, rotation );

   if ( changed ) {
      updateDerivedGeom();
   }

   return( changed );
}

void
Grid::updateDerivedGeom( const Grid* srcGrid )
{
   //
   // Make sure vertical geometry is consistent with composite flag
   //
   if ( composite == true ) {
      geometry.setSingleZLevel();
      if ( !isKnown( maxz )  &&  srcGrid != NULL ) {
         //
         // We should get our maxz from the source grid
         //
         maxz = srcGrid->getMaxZ();
      }
   }
   else {
      computeMaxz();
   }

   //
   // Now that we're sure about the derived geometry, 
   // allocate and initialize the data array
   // 
   allocateData();
   initializeData();
}

int Grid::setData(const Grid & src)
{
   if (geometry != src.getGeometry()) {
      return -1;
   }

   if (getDataType() == src.getDataType()) {
      resampleFromSameTypeGrid(src);
   }

   cerr << "Resampling between different grid types not supported unless "
        << "source grid is of type <unsigned char> or <unsigned short>." << endl;
   return -1;
}

int Grid::setAndScaleData(const Grid & src, float scale, float bias)
{
   if (geometry != src.getGeometry()) {
      return -1;
   }

   // Check for char grid as src.
   // 
   if (src.getDataType() == CHAR_GRID) {
      const TypeGrid<unsigned char> &typedSrc = (TypeGrid<unsigned char> &) src;
      return resampleFromCharArray(typedSrc.getData(),
                                   typedSrc.getGeometry(),
                                   scale, bias, 
                                   typedSrc.getBadValue(),
                                   typedSrc.getMissingValue());
   }

   // Check for unsigned short grid as src.
   // 
   if (src.getDataType() == SHORT_GRID) {
      const TypeGrid<unsigned short> &typedSrc = (TypeGrid<unsigned short> &) src;
      return resampleFromShortArray(typedSrc.getData(),
                                    typedSrc.getGeometry(),
                                    scale, bias, 
                                    typedSrc.getBadValue(),
                                    typedSrc.getMissingValue());
   }

   cerr << "Grid::setAndScaleData(const Grid & src) called with src "
        << "that is not unsigned char or short."
        << endl;
   return -1;
}

int Grid::resampleData(const Grid & src)
{
   int   status = 0;

   //
   // See if we have identical type grids
   //
   DataType srcDataType = src.getDataType();
   if ( dataType == srcDataType ) {
      status = resampleFromSameTypeGrid(src);
   }
   else {
      //
      // Call the appropriate resampling method 
      //    based on the source Grid's data type
      // Use FLT_MIN for scale and bias
      //    to indicate that that no scaling is required
      //
      switch( src.getDataType() ) {
         case  Grid::CHAR_GRID: {

               const TypeGrid<unsigned char> &typedSrc = 
                    (TypeGrid<unsigned char> &) src;

               status = resampleFromCharArray( typedSrc.getData(),
                                               typedSrc.getGeometry(),
                                               FLT_MIN, FLT_MIN,
                                               typedSrc.getBadValue(),
                                               typedSrc.getMissingValue() );
               break;
               }
         case  Grid::SHORT_GRID: {

               const TypeGrid<unsigned short> &typedSrc = 
                    (TypeGrid<unsigned short> &) src;

               status = resampleFromShortArray( typedSrc.getData(),
                                                typedSrc.getGeometry(),
                                                FLT_MIN, FLT_MIN,
                                                typedSrc.getBadValue(),
                                                typedSrc.getMissingValue() );
               break;
               }
         case  Grid::INT_GRID: {
               //
               // unsupported
               //
               status = -1;
               break;
               }
         case  Grid::FLOAT_GRID: {
               //
               // unsupported
               //
               status = -1;
               break;
               }
         case  Grid::DOUBLE_GRID: {
               //
               // unsupported
               //
               status = -1;
               break;
               }
      }
   }

   return status;
}

// Resample from a scaled data grid. Supported:
//   o Resampling from unsigned char grids to float and double.
//   o Resampling from unsigned short grids to float and double.
// 
// Augment TypeGrid::getCharLookup(...) to support resampling from
//   unsigned char grids to other types.
// 
int Grid::resampleAndScaleData(const Grid & src, float scale, float bias)
{
   //
   // Degenerate case:  Given the same type src and destination grid
   //                   we assume that we don't really need to do scaling
   //
   DataType srcDataType = src.getDataType();
   if ( dataType == srcDataType ) {
      return resampleFromSameTypeGrid(src);
   }

   // Check for char grid as src. This is the fastest way to resample.
   // 
   if (src.getDataType() == CHAR_GRID) {
      const TypeGrid<unsigned char> &typedSrc = (TypeGrid<unsigned char> &) src;
      return resampleFromCharArray(typedSrc.getData(),
                                   typedSrc.getGeometry(),
                                   scale, bias, 
                                   typedSrc.getBadValue(),
                                   typedSrc.getMissingValue());
   }

   if (src.getDataType() == SHORT_GRID) {
      const TypeGrid<unsigned short> &typedSrc = (TypeGrid<unsigned short> &) src;
      return resampleFromShortArray(typedSrc.getData(),
                                    typedSrc.getGeometry(),
                                    scale, bias, 
                                    typedSrc.getBadValue(),
                                    typedSrc.getMissingValue());
   }

   cerr << "Grid::resampleAndScaleData(const Grid & src) called with src "
        << "that is not unsigned char or unsigned short."
        << endl;
   return -1;
}

int
Grid::suggestValueDefs( const Grid & src )
{
   int status = 0;

   //
   // Call the appropriate TypeGrid method 
   // based on the source Grid's data type
   //
   switch( src.getDataType() ) {
      case  Grid::CHAR_GRID: {
            const TypeGrid<unsigned char> &typedSrc = 
                 (TypeGrid<unsigned char> &) src;
            suggestValueDefsFromChar( typedSrc.getInitValue(),
                                      typedSrc.getBadValue(),
                                      typedSrc.getMissingValue() );
            break;
            }
      case  Grid::SHORT_GRID: {
            const TypeGrid<unsigned short> &typedSrc = 
                 (TypeGrid<unsigned short> &) src;
            suggestValueDefsFromShort( typedSrc.getInitValue(),
                                       typedSrc.getBadValue(),
                                       typedSrc.getMissingValue() );
            break;
            }
      case  Grid::INT_GRID: {
            //
            // unsupported
            //
            status = -1;
            break;
            }
      case  Grid::FLOAT_GRID: {
            const TypeGrid<float> &typedSrc = 
                 (TypeGrid<float> &) src;
            suggestValueDefsFromFloat( typedSrc.getInitValue(),
                                       typedSrc.getBadValue(),
                                       typedSrc.getMissingValue() );
            break;
            }
      case  Grid::DOUBLE_GRID: {
            //
            // unsupported
            //
            status = -1;
            break;
            }
   }
   return status;
}

void
Grid::setComposite( float maxzVal )
{
   maxz = maxzVal;
   composite = true;

   bool changed = geometry.setSingleZLevel();
   if ( changed ) {
      updateDerivedGeom();
   }
}

bool
Grid::outOfBounds( int x, int y, int z ) const
{
   //
   // Check for negative indices
   //
   if ( x < 0 || y < 0 || z < 0 )
      return true;

   // //
   // // Check for an under-specified grid
   // //
   // if ( data == NULL )
   //    return true;

   //
   // Check for too large indices
   // The third check allows for single indexing through a 3d grid
   //
   if ( (size_t)x >= geometry.nx || 
        (size_t)y >= geometry.ny || 
        (size_t)z >= geometry.nz )
      return true;

   //
   // Looks good, i.e., NOT out of bounds
   //
   return false;
}

int
Grid::getZLevel( float height ) const
{
   //
   // Is this a single level composite grid?
   //
   if ( composite ) {
      return 0;
   }

   //
   // Make sure that dz isn't zero.
   //
   if ( geometry.dz == 0.0 ) {
      return 0;
   }

   //
   // Otherwise, calculate z from height/minz/dz
   //
   ssize_t zIndex = (ssize_t) ( ((height - geometry.minz) / geometry.dz) + 0.5 );

   if ( zIndex < 0 || zIndex >= (int) geometry.nz ) {
      return -1;
   }
   else {
      return zIndex;
   }
}

int
Grid::getDataElementNbytes() const
{
   int nbytes;

   switch( dataType ) {
     case Grid::CHAR_GRID:
          nbytes = 1;
          break;

     case Grid::SHORT_GRID:
          nbytes = 2;
          break;

     case Grid::FLOAT_GRID:
          nbytes = 4;
          break;

     default:
          nbytes = 0;
   }

   return( nbytes );
}
