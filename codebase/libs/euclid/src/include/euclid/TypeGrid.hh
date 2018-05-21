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

#ifndef TypeGridINCLUDED
#define TypeGridINCLUDED

#include <string>
#include <string.h>
#include <vector>

#include <euclid/GridGeom.hh>

#include <cstdio>
#include <iostream>
#include <limits.h>
#include <float.h>
#include <euclid/geometry.h>
#include <euclid/point.h>
#include <euclid/Polyline.hh>
#include <euclid/FuzzyFcn.hh>
#include <euclid/IndexPoint.hh>
#include <euclid/DistPoint.hh>
#include <cassert>

#include <euclid/Grid.hh>

using namespace std;

//
// Forward class declaration
//
// class TypeGrid;
class Polyline;
template <class T> class FuzzyFcn;
class IndexPoint;

template <class T>
class TypeGrid : public Grid {

public:

   TypeGrid( Grid::DataType type );
   TypeGrid( const TypeGrid<T> & source, bool copyData = true );
   TypeGrid( Grid::DataType type,
             const GridGeom & geom, T initVal, T badVal, T missingVal );
   TypeGrid( Grid::DataType type,
             size_t nx,   size_t ny,   size_t nz,
             float  dx,   float  dy,   float  dz,
             float  minx, float  miny, float  minz,
             double latOrigin, double lonOrigin,
             Projection::ProjId projectionId,
             T initVal, T badVal, T missingVal,
             double      rotation  = 0.0);

  virtual ~TypeGrid();

   //
   // Initializing and setting grid values
   //
   void   setToInitValue( int x, int y, int z=0 );
   int    set( T value, int x, int y, int z=0 );

   //
   // Duplicating data and characteristics from other grids
   // Original data and geometry are cleared out and 
   // reset to match the source grid
   //
   void   copy( const TypeGrid<T> & source, bool copyData = true );
   TypeGrid<T> &  operator=( const TypeGrid<T> & source );

   //
   // Duplicating data only from other grids.
   // Efficient but unsafe -- do not call this method unless you know that
   // the geometry of the source data matches the current grid geometry
   // *AND* that source.geometry.getNumValues() != UNKNOWN_SIZE
   //
   void   copyData( const TypeGrid<T> & source )
                  { assert( data );
                    memcpy( data, source.data,
                            sizeof(T) * geometry.getNumValues() ); }
   
   //
   // Resampling grid data from another TypeGrid of same type.
   //   Data from the source are resampled to match 
   //   the existing geometry and projection for all z-levels
   //
   int    resampleData( const TypeGrid<T> & sourceGrid );

   //
   // Set grid values from an unscaled buffer.
   //   Geometry of data must match geometry of this object.
   //   Data scaling will happen if scale not set to FLT_MIN.
   //
   virtual int setFromCharArray( const unsigned char* buffer,
                                 const GridGeom &geom, 
                                 float scale, float bias,
                                 unsigned char badChar,
                                 unsigned char missingChar );
   virtual int resampleFromCharArray( const unsigned char* buffer,
                                      const GridGeom &geom,
                                      float scale, float bias,
                                      unsigned char badChar,
                                      unsigned char missingChar );
   virtual int setFromShortArray( const unsigned short * buffer,
                                  const GridGeom &geom, 
                                  float scale, float bias,
                                  unsigned short badShort,
                                  unsigned short missingShort );
   virtual int resampleFromShortArray( const unsigned short * buffer,
                                       const GridGeom &geom,
                                       float scale, float bias,
                                       unsigned short badShort,
                                       unsigned short missingShort );
   virtual int setFromTArray( const T * buffer,
                              const GridGeom &dataGeom, 
                              T badVal,
                              T missingVal );
   virtual int resampleFromTArray( const T * buffer,
                                   const GridGeom &dataGeom,
                                   T badVal,
                                   T missingVal );

   // Single altitude versions.
   // 
   // virtual int setFromCharArray( float altitude,
   //                               const unsigned char* buffer,
   //                               const GridGeom &geom,
   //                               float scale, float bias,
   //                               unsigned char badChar,
   //                               unsigned char missingChar );
   virtual int resampleFromCharArray( float altitude,
                                      const unsigned char* buffer,
                                      const GridGeom &geom,
                                      float scale, float bias,
                                      unsigned char badChar,
                                      unsigned char missingChar );
   // virtual int setFromShortArray( float altitude,
   //                                const unsigned short * buffer,
   //                                const GridGeom &geom,
   //                                float scale, float bias,
   //                                unsigned short badShort,
   //                                unsigned short missingShort );
   virtual int resampleFromShortArray( float altitude,
                                       const unsigned short * buffer,
                                       const GridGeom &geom,
                                       float scale, float bias,
                                       unsigned short badShort,
                                       unsigned short missingShort );

    virtual int setFromVoidArray( const void * buffer,
                                  const GridGeom &dataGeom,
                                  float badVal,
                                  float missingVal )
                                { return( setFromTArray( (const T *)buffer,
                                                         dataGeom,
                                                         (T) badVal,
                                                         (T) missingVal )); }

   // 
   // Plane mapping. Given a transformation matrix, resample the
   //   plane and convert the values.
   // 
   int    mapPlaneFromTArray(const T * srcData,
                             int * planeMap, T* tmpData,
                             T badVal = (T) 0.0,
                             T missingVal = (T) 0.0 );
   int    mapPlaneFromCharArray(const unsigned char * srcData,
                                int * planeMap, T* tmpData,
                                float scale = FLT_MIN,
                                float bias = FLT_MIN,
                                unsigned char badChar = 0,
                                unsigned char missingChar = 0 );
   int    mapPlaneFromShortArray(const unsigned short * srcData,
                                 int * planeMap, T* tmpData,
                                 float scale = FLT_MIN,
                                 float bias = FLT_MIN,
                                 unsigned short badShort = 0,
                                 unsigned short missingShort = 0 );

   //
   // A generic resample interface that works for resampling 
   // a single z-level from either a Grid buffer or from a cdata buffer
   //
   // Slow. Always does high-overhead resampling.
   //
   int resampleData( size_t z, void* buffer,
                     const GridGeom &geom, 
                     float scale = FLT_MIN,
                     float bias = FLT_MIN,
                     unsigned char badChar = 0,
                     unsigned char missingChar = 0 );

   //
   // Setting grid values using other grids
   // No resampling involved and will fail if ny, ny, or nz differ
   //
   int    combineGrids( vector< TypeGrid <T> *>& gridList, 
                        bool sumOnOverlap=false );
   int    combineGrids( TypeGrid<T> & sourceGrid, 
                        bool sumOnOverlap=false );
   int    maskData( const TypeGrid<T> &maskGrid, 
                    bool inverse=false );

   //
   // General remapping routines.
   //
   int getPlaneMapping( const GridGeom &srcGeom, int planeMap[]);
   int getCharLookup(T * charLookup, float scale, float bias,
                     unsigned char badChar, 
                     unsigned char missingChar );
   int getShortLookup(T * shortLookup, float scale, float bias,
                      unsigned short badShort,
                      unsigned short missingShort );

   //
   // Setting grid data from lines and Polylines
   //
   int    fillPoly( Polyline &p, T v );
   int    fillPolyFast( Polyline &p, T v );
   int    interpPoly( Polyline& p, vector< IndexPoint *>& gridPts,
		      T badVal, T missingVal );
   int    interpPoly( Polyline& p, vector< IndexPoint *>& gridPts );
   int    fillLine( int x1, int y1, int x2, int y2,
                    T fillValue = 1, int zLevel = 0 );

   //
   // Fetching grid data values
   // Be very careful when grabbing the address of the data via getData()
   // that the grid geometry is not resized out from under you, 
   // leaving you with an invalid data pointer.
   //
   T   get( int x, int y, int z=0 );
   T*  getData() const { return ( ((TypeGrid<T> *) this)->data); } // cast const
   T   getInitValue() const { return(initValueDef); }
   T   getBadValue() const { return(badValueDef); }
   T   getMissingValue() const { return(missingValueDef); }

   virtual void* getVoidData() const { return ( (void*)(this->data) ); }
   virtual float getFloatInitValue() const { return (float) getInitValue(); }
   virtual float getFloatBadValue() const { return (float) getBadValue(); }
   virtual float getFloatMissingValue() const { return (float) getMissingValue(); }

   // Get data as a void pointer.
   virtual void * getUnscaledData() { return ( ((TypeGrid<T> *) this)->data); }

   //
   // Fetching info about grid data
   //
   bool      isSet( T value ) const
                  { return ( data &&
                             value != initValueDef &&
                             value != badValueDef &&
                             value != missingValueDef ); }
   bool      isSet( int x, int y, int z=0 ) const;

   //
   // Scaling
   //
   void                    getScaleBias( float *scale, float *bias,
                                         bool forceIntegralScaling = false ) 
                           const;

   virtual unsigned char*  getCharData( float *scale = NULL, 
                                        float *bias = NULL,
                                        unsigned char badOutputChar = 0,
                                        unsigned char missingOutputChar = 0,
                                        bool forceIntegralScaling = false );

   virtual unsigned short* getShortData( float *scale = NULL, 
                                         float *bias = NULL,
                                         unsigned short badOutputShort = 0,
                                         unsigned short missingOutputShort = 0,
                                         bool forceIntegralScaling = false );

   bool            value2byte( T gridValue, unsigned char &byteValue,
                               float scale, float bias,
                               unsigned char badOutputByte,
                               unsigned char missingOutputByte );

   bool            byte2value( unsigned char byteValue, T &gridValue,
                               float scale, float bias,
                               unsigned char badInputByte,
                               unsigned char missingInputByte );


   //
   // Other misc stuff
   //
   void      setFuzzyFcn( FuzzyFcn<T> *fcn );
   void      applyFuzzyFcn();
   void      setInitValue( T init ) { initValueDef = init; }
   void      setValueDefs( T init, T bad, T missing);
   void      suggestValueDefs( T init, T bad, T missing);
   void      setValueDefs( T unknown )
                         { setValueDefs( unknown, unknown, unknown ); }
   void      suggestValueDefs( T unknown )
                         { suggestValueDefs( unknown, unknown, unknown ); }

   virtual void clearData();
   virtual void allocateData();
   virtual void initializeData();
   void allocateData( const TypeGrid<T> * source );
   void initializeData( const TypeGrid<T> * source );

protected:
    // Virtual method used for forwarding resampling to
    //   TypeGrid<Type>::resampleData(const TypeGrid<Type> &).
    //   This is called from resampleData(const Grid &), and
    //   is not intended to be part of the public interface.
    // 
    virtual int resampleFromSameTypeGrid(const Grid & src);

   //
   // Called from Grid::setValueDefs( const Grid & )
   // Not intended to be part of the public interface.
   //
   void suggestValueDefsFromChar( unsigned char init,
                                  unsigned char bad,
                                  unsigned char missing )
      { suggestValueDefs( (T)init, (T)bad, (T)missing ); }

   void suggestValueDefsFromShort( unsigned short init,
                                   unsigned short bad,
                                   unsigned short missing )
      { suggestValueDefs( (T)init, (T)bad, (T)missing ); }

   void suggestValueDefsFromFloat( float init,
                                   float bad,
                                   float missing )
      { suggestValueDefs( (T)init, (T)bad, (T)missing ); }

private:

   //
   // Data management
   //
   T     initValueDef;
   T     badValueDef;
   T     missingValueDef;
   bool  hasValueDefs;

   T  *data;

   FuzzyFcn<T> *fuzzyFcn;

   friend class MdvIngest;

   T*  getPlaneData( float height ) const;
};

//
// Macros for testing grid values
//

// Todo: Fix these.
// #define ISBAD(a) ( a == TypeGrid::BAD_DATA )
// #define ISMISSING(a) ( a == TypeGrid::MISSING_DATA )
// #define ISVALID(a) ( !ISBAD(a) && !ISMISSING(a) )
// #define ISBAD(a) ( true )
// #define ISMISSING(a) ( true )
// #define ISVALID(a) ( !ISBAD(a) && !ISMISSING(a) )

// 
// The Implementation.
// 

#define CLIP(x,l,u) (x < l) ? l : ((x > u)? u : x)

template <class T>
TypeGrid<T>::TypeGrid( Grid::DataType type ) : Grid( type )
{
   initValueDef    = (T) 0.0;
   badValueDef     = (T) 0.0;
   missingValueDef = (T) 0.0;
   hasValueDefs = false;
   data      = NULL;
   fuzzyFcn  = NULL;
}

template <class T>
TypeGrid<T>::TypeGrid( const TypeGrid<T>& source, bool copyData )
            : Grid ( (const Grid &) source )
{
   initValueDef    = (T) 0.0;
   badValueDef     = (T) 0.0;
   missingValueDef = (T) 0.0;
   hasValueDefs = false;
   data     = NULL;
   fuzzyFcn = NULL;
   copy( source, copyData );
}

template <class T>
TypeGrid<T>::TypeGrid( Grid::DataType type, const GridGeom & geom,
                       T initVal, T badVal, T missingVal )
            : Grid(type, geom)
{
   //
   // Set up the value definitions.
   //
   initValueDef    = initVal;
   badValueDef     = badVal;
   missingValueDef = missingVal;
   hasValueDefs    = true;

   fuzzyFcn = NULL;

   //
   // Construct and initialize the data grid.
   //
   data = NULL;
   allocateData();
   initializeData();
}

template <class T>
TypeGrid<T>::TypeGrid( Grid::DataType type,
                       size_t nx,   size_t ny,   size_t nz,
                       float  dx,   float  dy,   float  dz,
                       float  minx, float  miny, float  minz,
                       double latOrigin, double lonOrigin,
                       Projection::ProjId projectionId,
                       T initVal, T badVal, T missingVal,
                       double      rotation )
         : Grid( type, nx, ny, nz, dx, dy, dz, minx, miny, minz,
                 latOrigin, lonOrigin, projectionId, rotation )
{
   //
   // Set up the value definitions.
   //
   initValueDef    = initVal;
   badValueDef     = badVal;
   missingValueDef = missingVal;
   hasValueDefs    = true;

   fuzzyFcn = NULL;

   //
   // Construct and initialize the data grid
   //
   data = NULL;
   allocateData();
   initializeData();
}

template <class T>
TypeGrid<T>::~TypeGrid()
{
   delete []data;
//   delete fuzzyFcn;
}

template <class T>
TypeGrid<T>& TypeGrid<T>::operator=( const TypeGrid<T> &source )
{
   copy( source );
   return( *this );
}

template <class T>
void
TypeGrid<T>::copy( const TypeGrid<T>& source, bool copyData )
{
   //
   // Copy the data attributes except the fuzzyFcn
   //
   initValueDef    = source.initValueDef;
   badValueDef     = source.badValueDef;
   missingValueDef = source.missingValueDef;
   hasValueDefs    = source.hasValueDefs;

   // Don't copy the fuzzy fcn.
   // fuzzyFcn        = source.fuzzyFcn;

   composite       = source.composite;
   maxz            = source.maxz;

   //
   // Allocate the data based on the source's geometry
   // Then initialize the new data
   //
   allocateData( &source );
   if ( copyData )
      initializeData( &source );
   else
      initializeData();
}

// virtual
template <class T>
void TypeGrid<T>::allocateData()
{
   //
   // Get rid of the old data, if necessary
   //
   if ( data )
      delete []data;

   //
   // Allocate the new data, if we know enough
   //
   size_t gridSize = geometry.getNumValues();
   if ( geometry.isKnown( gridSize ) && hasValueDefs ) 
      data = new T[(int)gridSize];
   else
      data = NULL;
}

template <class T>
void TypeGrid<T>::allocateData( const TypeGrid<T> *source )
{
   //
   // Set our own geometry based on the "outside" geometry, if specified
   //
   if ( source )
      geometry = source->geometry;

   //
   // Allocate the new data, if we know enough
   //
   allocateData();
}

// virtual
template <class T>
void TypeGrid<T>::initializeData()
{
   //
   // Degenerate case
   //
   if( data == NULL )
      return;

   clearData();
}

template <class T>
void TypeGrid<T>::initializeData( const TypeGrid<T> *source )
{
   //
   // Degenerate case
   //
   if( data == NULL )
      return;

   if ( source == NULL )
      clearData();
   else
      copyData( *source );
}

template <class T>
void
TypeGrid<T>::clearData()
{
   //
   // We can't assert here that the data array exists
   //
   if ( data == NULL )
      return;

   // Do not init if the defined init value does not exist.
   if ( !hasValueDefs)
      return;

   size_t   i;
   size_t   numValues = geometry.getNumValues();

   //
   // Set the whole grid to initial values
   // and clear out the data time
   //
   for ( i=0; i < numValues; i++ ) {
      data[i] = initValueDef;
   }
}

template <class T>
int TypeGrid<T>::mapPlaneFromTArray(const T * srcData,
                                    int * planeMap, T* tmpData,
                                    T badVal /* = (T) 0.0 */,
                                    T missingVal /* = (T) 0.0 */)
{
   if (data == NULL) {
      return -1;
   }

   size_t numVals = geometry.nx * geometry.ny;
   for (size_t i = 0; i < numVals; i++) {
      if (planeMap[i] >= 0) {

         if ( composite && isSet( tmpData[i] )) {
            //
            // Replace the value in a composite plane only if
            // the new value is legit. and is greater than the existing 
            // value, i.e., we only support MAX composites
            // 
            if ( srcData[planeMap[i]] == badVal ||
                 srcData[planeMap[i]] == missingVal ||
                 tmpData[i] >= srcData[planeMap[i]] ) {
               continue;
            }
         }

         if (srcData[planeMap[i]] == badVal) {
            tmpData[i] = badValueDef;
         }
         else if (srcData[planeMap[i]] == missingVal) {
            tmpData[i] = missingValueDef;
         }
         else {
            tmpData[i] = srcData[planeMap[i]];
         }
      }
      else {
         tmpData[i] = missingValueDef;
      }
   }

   return 0;
}

template <class T>
int TypeGrid<T>::mapPlaneFromCharArray(const unsigned char * srcData,
                                       int * planeMap, T* tmpData,
                                       float scale /* = FLT_MIN */,
                                       float bias /* = FLT_MIN */,
                                       unsigned char badChar /* = 0 */,
                                       unsigned char missingChar /* = 0 */)
{
   T charLookup[UCHAR_MAX + 1];
   int status = getCharLookup(charLookup, scale, bias, badChar, missingChar);
   if (status < 0) {
      cerr << "TypeGrid<T>::mapPlaneFromCharArray(): "
           << "couldn't obtain char lookup." << endl;
      return -1;
   }

   size_t numVals = geometry.nx * geometry.ny;
   for (size_t i = 0; i < numVals; i++) {
      if (planeMap[i] >= 0) {

         // Temporary assertions for testing.
         //assert(srcData[planeMap[i]] <= UCHAR_MAX);

         if ( composite && isSet( tmpData[i] )) {
            //
            // Replace the value in a composite plane only if
            // the new value is legit. and is greater than the existing
            // value, i.e., we only support MAX composites
            //
            if ( srcData[planeMap[i]] == badChar ||
                 srcData[planeMap[i]] == missingChar ||
                 tmpData[i] >= charLookup[srcData[planeMap[i]]] ) {
               continue;
            }
         }

         if (srcData[planeMap[i]] == badChar) {
            tmpData[i] = badValueDef;
         }
         else if (srcData[planeMap[i]] == missingChar) {
            tmpData[i] = missingValueDef;
         }
         else {
            tmpData[i] = charLookup[srcData[planeMap[i]]];
         }

// TEST
//          if (i < 10) {
//             cerr << "New Value: " << tmpData[i] << " Location within plane: " << i << " planeMap index: " << planeMap[i] << " Raw src value: " << srcData[planeMap[i]] << " looked-up value: " << charLookup[srcData[planeMap[i]]] << endl;
//          }
      }
      else {
         tmpData[i] = missingValueDef;
      }
   }

   return 0;
}

template <class T>
int TypeGrid<T>::mapPlaneFromShortArray(const unsigned short * srcData,
                                        int * planeMap, T* tmpData,
                                        float scale /* = FLT_MIN */,
                                        float bias /* = FLT_MIN */,
                                        unsigned short badShort /* = 0 */,
                                        unsigned short missingShort /* = 0 */)
{

   // Todo: Use some mechanism besides a short lookup? This is big!

   T shortLookup[USHRT_MAX + 1];
   int status = getShortLookup(shortLookup, scale, bias, 
                               badShort, missingShort);
   if (status < 0) {
      cerr << "TypeGrid<T>::mapPlaneFromShortArray(): "
           << "couldn't obtain short lookup." << endl;
      return -1;
   }

   size_t numVals = geometry.nx * geometry.ny;
   for (size_t i = 0; i < numVals; i++) {
      if (planeMap[i] >= 0) {

         // Temporary assertions for testing.
         //assert(srcData[planeMap[i]] <= USHRT_MAX);

         if ( composite && isSet( tmpData[i] )) {
            //
            // Replace the value in a composite plane only if
            // the new value is legit. and is greater than the existing
            // value, i.e., we only support MAX composites
            //
            if ( srcData[planeMap[i]] == badShort ||
                 srcData[planeMap[i]] == missingShort ||
                 tmpData[i] >= shortLookup[srcData[planeMap[i]]] ) {
               continue;
            }
         }

         if (srcData[planeMap[i]] == badShort) {
            tmpData[i] = badValueDef;
         }
         else if (srcData[planeMap[i]] == missingShort) {
            tmpData[i] = missingValueDef;
         }
         else {
            tmpData[i] = shortLookup[srcData[planeMap[i]]];
         }

// TEST
//          if (i < 10) {
//             cerr << "New Value: " << tmpData[i] << " Location within plane: " << i << " planeMap index: " << planeMap[i] << " Raw src value: " << srcData[planeMap[i]] << " looked-up value: " << shortLookup[srcData[planeMap[i]]] << endl;
//          }
      }
      else {
         tmpData[i] = missingValueDef;
      }
   }

   return 0;
}

template <class T>
int
TypeGrid<T>::combineGrids( vector< TypeGrid<T> *>& gridList, bool sumOnOverlap )
{
   assert( data );

   // dixon - g++3.2 indicates that this iterator construct is deprecated
   // therefore used [] notation instead
   //     vector< TypeGrid<T> *>::iterator it;
   //     TypeGrid<T> *sourceGrid;
   //     for (it = gridList.begin(); it != gridList.end(); it++) {
   //       sourceGrid = *it;
   //       if( combineGrids( *sourceGrid, sumOnOverlap ) != 0 )
   //         return( -1 );
   //     }
   
   for (size_t ii = 0; ii < gridList.size(); ii++) {
     TypeGrid<T> *sourceGrid = gridList[ii];
     if( combineGrids( *sourceGrid, sumOnOverlap ) != 0 )
       return( -1 );
   }
   
   return( 0 );
   
}

template <class T>
int
TypeGrid<T>::combineGrids( TypeGrid<T> &sourceGrid, bool sumOnOverlap )
{
   T *sourceData = sourceGrid.getData();
   assert ( sourceData  &&  data );

   size_t ix, iy, iz;

   size_t sourceNx = sourceGrid.geometry.nx;
   size_t sourceNy = sourceGrid.geometry.ny;
   size_t sourceNz = sourceGrid.geometry.nz;

   size_t thisNx = geometry.nx;
   size_t thisNy = geometry.ny;
   size_t thisNz = geometry.nz;

   T sourceInitValue = sourceGrid.getInitValue();
   T sourceVal, destVal;
   
   if ((sourceNx != thisNx) || (sourceNy != thisNy) || (sourceNz != thisNz))
       return( -1 );
      
   if (sumOnOverlap) {
     for (iz = 0; iz < thisNz; iz++) {
    for (iy = 0; iy < thisNy; iy++) {
       for (ix = 0; ix < thisNx; ix++) {

          sourceVal = sourceData[ix + iy*thisNx + iz*thisNx*thisNy];
          destVal = data[ix + iy*thisNx + iz*thisNx*thisNy];

          if (  sourceVal == badValueDef ||
            sourceVal == missingValueDef ||
            sourceVal == sourceInitValue )
         continue;

          if (  destVal == badValueDef || 
            destVal == missingValueDef ||
            destVal == initValueDef )
         data[ix + iy*thisNx + iz*thisNx*thisNy] = sourceVal;

          else
         data[ix + iy*thisNx + iz*thisNx*thisNy] += sourceVal;
       }
    }
     }
  } else {
     //
     // Take max instead of summing
     //
     for (iz = 0; iz < thisNz; iz++) {
    for (iy = 0; iy < thisNy; iy++) {
       for (ix = 0; ix < thisNx; ix++) {

          sourceVal = sourceData[ix + iy*thisNx + iz*thisNx*thisNy];
          destVal = data[ix + iy*thisNx + iz*thisNx*thisNy];
        
              if (  sourceVal == badValueDef ||
                    sourceVal == missingValueDef ||
                    sourceVal == sourceInitValue )
         continue;
          
          if (  destVal == badValueDef || 
            destVal == missingValueDef ||
            destVal == initValueDef ||
            destVal < sourceVal )
         data[ix + iy*thisNx + iz*thisNx*thisNy] = sourceVal;
       }
    }
     }
  }
   
  return( 0 );
}

template <class T>
int
TypeGrid<T>::maskData( const TypeGrid<T> &maskGrid, bool inverse )
{
   T *maskData = maskGrid.getData();
   assert ( maskData  &&  data );

   if ( geometry != maskGrid.getGeometry() )
       return( -1 );
      
   T      maskValue;
   bool   maskIsSet;
   size_t ix, iy, iz;
   size_t nx = geometry.nx;
   size_t ny = geometry.ny;
   size_t nz = geometry.nz;

   for( iz=0; iz < nz; iz++ ) {
      for( iy=0; iy < ny; iy++ ) {
         for( ix=0; ix < nx; ix++ ) {

            maskValue = maskData[ix + iy*nx + iz*nx*ny];
            maskIsSet = maskGrid.isSet( maskValue );
            if ( (!maskIsSet  &&  !inverse) ||
                 ( maskIsSet  &&   inverse )) {
               //
               // Remove the data value, i.e., set to initial value
               //
               data[ix + iy*nx + iz*nx*ny] = initValueDef;
            }
         }
      }
   }
   
   return( 0 );
}

template <class T>
void TypeGrid<T>::getScaleBias( float *s, float *b,
                                bool forceIntegralScaling ) const
{
   assert( data );

   size_t   i, numValues;
   T        max=0, min=0, value;
   float    scale, bias;

   bool initialized = false;
   numValues = geometry.getNumValues();

   //
   // Get the min and max
   //
   for ( i=0; i < numValues; i++ ) {
      value = data[i];
      if ( value == badValueDef  ||  value == missingValueDef )
         continue;

      if ( !initialized) {
         max = value;
         min = value;
         initialized = true;
         continue;
      }

      if ( value > max )
         max = value;
      if ( value < min )
         min = value;
   }
   
   //
   // Set the scale and bias
   //
   unsigned char charRange = 250;
   T             dataRange = max - min;

   if ( !initialized ) {
      scale = 0;
      bias  = 0;
   }
   else {
      if ( forceIntegralScaling ) {
         //
         // Make sure the values come out as integers
         // (assuming, of course, that they go in as integers)
         //
         if ( dataRange <= charRange ) {
            //
            // Degenerate case where every integral value
            // can be stored as is
            //
            scale = 1.0;
         }
         else {
            //
            // Calculate scale as a factor of the dynamic data range
            // There is an upper limit on the dynamic data range
            // which will allow for integral scaling
            //
            assert( dataRange <= (charRange*charRange) );

            //
            // Find the maximum scaling factor 
            // that fits in our character representation
            //
            int    factor;
            int    maxFactor = 1;
            size_t iLimit = (int)( sqrt((double) dataRange) );
            for ( i=2; i <= iLimit; i++ ) {
               if ( (((int)(dataRange)) % i) == 0 ) {
                  factor = (((int)(dataRange)) / i);
                  if ( factor <= charRange ) {
                     maxFactor = factor;
                     break;
                  }
               }
            }
            assert( maxFactor != 1 );
            scale = (float)(dataRange/maxFactor);
         }
      }
      else {
         //
         // Do our normal scaling calculations
         //
         scale = (float)(dataRange/charRange);
      }


      if ( fabs( scale ) <= 0.0001 ) {
         scale = 1.0;
      }
      bias = (float)min - (2.0*scale);
   }

   //
   // And return them, if requested
   //
   if ( b )
      *b  = bias;
   if ( s )
      *s = scale;

   // cerr << "Calculated scale: " << scale << " bias: " << bias << " based on min: " << min << " max: " << max << endl;
}

// Get the character representation of the data.
//   Caller is responsible for deleting the data.
// 
//   For TypeGrid<unsigned char>, this:
//     o Returns a copy of the data.
//     o Leaves float and bias as they were when passed in.
// 
//   For other instantiation types:
//     o Calculates an appropriate scale and bias.
//     o Scales each data value into the returned array.
// 
template <class T>
unsigned char*
TypeGrid<T>::getCharData( float *s, float *b, 
                          unsigned char badOutputChar,
                          unsigned char missingOutputChar,
                          bool forceIntegralScaling )
{
   assert( data );

   unsigned char* byteData;
   size_t         i, nvalues;
   float          scale = 1.0, bias = 0.0;

   //
   // Create a new data array
   // NOTE: the caller "owns" this scaled data array
   //
   nvalues = geometry.getNumValues();
   byteData = new unsigned char[nvalues];

   if (dataType == Grid::CHAR_GRID) {
      if (s != NULL && b != NULL) {
         scale = *s;
         bias  = *b;
      }

      // Copy the data to the output array.
      memcpy(byteData, data, nvalues);

      if (badOutputChar != (unsigned char) badValueDef  ||
          missingOutputChar != (unsigned char) missingValueDef) {

         // Caller is asking for bad or missing value output that's different
         //   from the internal representation. 
         //   Change any bad or missing values.
         // 
         for ( i=0; i < nvalues; i++ ) {                               
            if (byteData[i] == badValueDef) 
               byteData[i] = badOutputChar;
            else if (byteData[i] == missingValueDef) 
               byteData[i] = missingOutputChar;
         }
      }
   }
   else {
      //                                                               
      // Scale the grid data
      //                                                                 
      getScaleBias( &scale, &bias, forceIntegralScaling );
   
      for ( i=0; i < nvalues; i++ ) {                               
         value2byte( data[i], byteData[i], scale, bias, 
                     badOutputChar, missingOutputChar );
      }
   
   }

   //
   // Return the scale and bias, if requested
   //
   if ( b )
      *b = bias;
   if ( s )
      *s = scale;
   
   return( byteData );
}

// Not implemented yet.
template <class T>
unsigned short* TypeGrid<T>::getShortData( float * /* s */, float * /* b */,
                                       unsigned short /* badOutputShort */,
                                       unsigned short /* missingOutputShort */,
                                       bool /* forceIntegralScaling */ )
{
   assert( data );

   // Not implemented yet.

   // unsigned char* byteData;
   // size_t         i, nvalues;
   // float          scale = 1.0, bias = 0.0;

   // Not implemented yet.

   return NULL;
}

template <class T>
bool
TypeGrid<T>::value2byte( T gridValue, unsigned char &byteValue, 
                         float scale, float bias, 
                         unsigned char badOutputByte,
                         unsigned char missingOutputByte )
{
   if ( gridValue == badValueDef ) {
      byteValue = badOutputByte;
      return( false );
   }
   else if ( gridValue == missingValueDef ) {
      byteValue = missingOutputByte;
      return( false );
   }
   else {
      byteValue = (unsigned char)(((gridValue - bias) / scale) + 0.5);
      return( true );
   }
}

template <class T>
bool TypeGrid<T>::byte2value( unsigned char byteValue, T &gridValue,
                              float scale, float bias,
                              unsigned char badInputByte,
                              unsigned char missingInputByte )
{
   if ( byteValue == badInputByte ) {
      gridValue = badValueDef;
      return( false );
   }
   else if ( byteValue == missingInputByte ) {
      gridValue = missingValueDef;
      return( false );
   }
   else {
      gridValue = (T)((byteValue * scale) + bias);
      return( true );
   }
}

template <class T>
T TypeGrid<T>::get( int x, int y, int z )
{
   assert( data );

   //
   // Invalid indicies
   //
   if ( outOfBounds( x, y, z ) )
      return badValueDef;

   //
   // Valid indicies
   //
   else
      return( data[(z*geometry.nx*geometry.ny) + (y*geometry.nx) + x] );
}

template <class T>
void
TypeGrid<T>::setToInitValue( int x, int y, int z ) 
{
   assert( data );

   //
   // Setting a value outside the bounds does nothing
   //
   if( outOfBounds( x, y, z ) )
      return;
   
   data[(z*geometry.nx*geometry.ny) + (y*geometry.nx) + x] = initValueDef;
}

template <class T>
int TypeGrid<T>::set( T newValue, int x, int y, int z )
{
   assert( data );

   //
   // Setting a value outside the bounds does nothing
   //
   if ( outOfBounds( x, y, z ) )
      return -1;

   //
   // Valid indicies
   //
   T oldValue;
   bool replace = true;

   if ( composite ) {
      //
      // Compare old value against new value
      //
      oldValue = get( x, y, z );
      if ( isSet( oldValue ) ) {
         replace = ( oldValue < newValue );
      }
   }

   if ( replace ) {
      data[(z*geometry.nx*geometry.ny) + (y*geometry.nx) + x] = newValue;
   }

   return 0;
}

template <class T>
bool
TypeGrid<T>::isSet( int x, int y, int z )  const
{
   if ( !data) 
      return false;

   if( outOfBounds( x, y, z ) ) 
      return false;
   
   return( isSet( data[x + y*geometry.nx + 
              z*geometry.nx*geometry.ny] ) );
}

template <class T>
T * TypeGrid<T>::getPlaneData( float height ) const
{
   int z = getZLevel( height );

   if( z < 0  ||  (size_t)z > geometry.nz-1 )
      return NULL;

   return( data + (z * geometry.nx * geometry.ny));
}

template <class T>
int TypeGrid<T>::resampleData( const TypeGrid<T> &source )
{
   T        *sourcePlane;

   // 
   // Todo: Check/Return whether any data was found!
   //         (all of these resample methods).
   // 

   //
   // Process each z-level on the destination grid
   //   Have to check for too many z iterations, b/c sometimes
   //   dz == 0.0 in mdv files!
   //
   size_t planeDataLen = geometry.nx * geometry.ny;
   int * planeMap = new int[planeDataLen];
   size_t zCount, maxZCount, zIndex;

   // Set up stuff for iterating through z levels.
   if ( composite ) {
      assert( geometry.nz == 1  &&  geometry.dz != 0.0 );
      maxZCount = (size_t)(((maxz - geometry.minz + 1) / geometry.dz) + 0.5);
      if ( maxZCount > source.geometry.nz ) {
         maxZCount = source.geometry.nz;
      }
   }
   else {
      maxZCount = geometry.nz;
   }

   for ( zCount = 0; zCount < maxZCount; zCount++ ) {

      if ( composite )
         zIndex = 0;
      else
         zIndex = zCount;

      T* dataPtr = data + (planeDataLen * zIndex);

      // Get the src data plane for the current height.
      float height = geometry.minz + ((float) zCount) * geometry.dz;
      sourcePlane = source.getPlaneData( height );

      if ( sourcePlane ) {
         int status;
         if (zCount == 0) {
            status = getPlaneMapping( source.getGeometry(), planeMap);
            if (status < 0) {
               cerr << "WARNING: No mappable cells found." << endl;
            }
         }
   
         status = mapPlaneFromTArray(sourcePlane, planeMap, dataPtr,
                                     source.getBadValue(),
                                     source.getMissingValue());
         if (status < 0) { 
            cerr << "Could not map plane: " << zCount << " from chars." << endl;
            delete [] planeMap;
            return -1;
         }
      }
   }
 
   delete [] planeMap;
   return(  0  );
}

//
// Set grid values from an unscaled buffer.
//   Geometry of data must match geometry of this object.
//   Data scaling will happen if scale not set to FLT_MIN.
//
// virtual
template <class T>
int TypeGrid<T>::setFromCharArray( const unsigned char* buffer,
                                   const GridGeom &dataGeom,
                                   float scale, float bias,
                                   unsigned char badChar,
                                   unsigned char missingChar ) 
{
   assert( data );

   if (geometry != dataGeom) {
      return -1;
   }

   // Supported destination types: char, float, and double.
   //  
   if (dataType != Grid::CHAR_GRID &&
       dataType != Grid::FLOAT_GRID &&
       dataType != Grid::DOUBLE_GRID) {

      cerr << "Conversion from char not supported." << endl;
      return -1;
   }

   if (dataType == Grid::CHAR_GRID && scale != FLT_MIN) {
      cerr << "WARNING: Setting char data on TypeGrid<char> with ";
      cerr << "scaling turned on. This is a weird thing to do.";
      cerr << endl;
   }

   //
   // Process each z-level.
   //
   size_t planeDataLen = geometry.nx * geometry.ny;
   int * planeMap = new int[planeDataLen];
   for ( size_t zCount = 0; zCount < geometry.nz; zCount++) {

      T* dataPtr = data + (planeDataLen * zCount);

      // Todo: Don't use this planeMap stuff -- just copy values.

      int status;
      if (zCount == 0) {
         status = getPlaneMapping( dataGeom, planeMap);
         if (status < 0) {
            cerr << "WARNING: No mappable cells found." << endl;
         }
      }
   
      status = mapPlaneFromCharArray(buffer + (zCount * planeDataLen),
                                     planeMap, dataPtr, scale, bias, 
                                     badChar, missingChar);
      if (status < 0) { 
         cerr << "Could not map plane: " << zCount << " from chars." << endl;
         delete [] planeMap;
         return -1;
      }
   }

   delete [] planeMap;
   return(  0  );
}

//
// Resample from an unscaled buffer to set the grid values.
//   Geometry of data need not match the grid.
//   Data scaling will happen if scale not set to FLT_MIN.
//
// virtual
template <class T>
int TypeGrid<T>::resampleFromCharArray( const unsigned char* buffer,
                                        const GridGeom &dataGeom,
                                        float scale, float bias,
                                        unsigned char badChar,
                                        unsigned char missingChar ) 
{
   int       srcZ;
   const unsigned char *sourcePlane;

   assert( data );

   // Supported destination types: char, float, and double.
   //  
   if (dataType != Grid::CHAR_GRID &&
       dataType != Grid::FLOAT_GRID &&
       dataType != Grid::DOUBLE_GRID) {

      cerr << "Conversion from char not supported." << endl;
      return -1;
   }

   if (dataType == Grid::CHAR_GRID && scale != FLT_MIN) {
      cerr << "WARNING: Resampling char data on TypeGrid<char> with ";
      cerr << "scaling turned on. This is a weird thing to do.";
      cerr << endl;
   }

   //
   // Process each z-level on the destination grid
   //   Have to check for too many z iterations, b/c sometimes
   //   dz == 0.0 in mdv files!
   //
   size_t planeDataLen = geometry.nx * geometry.ny;
   int * planeMap = new int[planeDataLen];
   size_t zCount, maxZCount, zIndex;

   // Set up stuff for iterating through z levels.
   if ( composite ) {
      assert( geometry.nz == 1  &&  geometry.dz != 0.0 );
      maxZCount = (size_t)(((maxz - geometry.minz + 1) / geometry.dz) + 0.5);
      if ( maxZCount > dataGeom.nz ) {
         maxZCount = dataGeom.nz;
      }
   }
   else {
      maxZCount = geometry.nz;
   }

   for ( zCount = 0; zCount < maxZCount; zCount++ ) {

      // Get the height for the current z level.
      float height = geometry.minz + ((float) zCount) * geometry.dz;

      // Get the z level from the dataGeom (code from Grid::getZLevel()).
      srcZ = (size_t)(((height - dataGeom.minz) / dataGeom.dz) + 0.5);
      if ( srcZ < 0 ) {
         return -1;
      }

      if ( composite )
         zIndex = 0;
      else
         zIndex = zCount;

      T* dataPtr = data + (planeDataLen * zIndex);

      sourcePlane = buffer + (srcZ * dataGeom.nx * dataGeom.ny);
      if ( sourcePlane ) {

         int status;
         if (zCount == 0) {
            status = getPlaneMapping( dataGeom, planeMap);
            if (status < 0) {
               cerr << "WARNING: No mappable cells found." << endl;
            }
         }
      
         status = mapPlaneFromCharArray(sourcePlane,
                                        planeMap, dataPtr, scale, bias, 
                                        badChar, missingChar);
         if (status < 0) { 
            cerr << "Could not map plane: " << zCount << " from chars." << endl;
            delete [] planeMap;
            return -1;
         }
      }
   }

   delete [] planeMap;
   return(  0  );
}

// Set grid values from an unscaled buffer.
//   Geometry of data must match geometry of this object.
//   Data scaling will happen if scale not set to FLT_MIN.
//
// virtual
template <class T>
int TypeGrid<T>::setFromShortArray( const unsigned short* buffer,
                                    const GridGeom &dataGeom,
                                    float scale, float bias,
                                    unsigned short badShort,
                                    unsigned short missingShort ) 
{
   assert( data );

   if (geometry != dataGeom) {
      return -1;
   }

   // Supported destination types: shorts, float, and double.
   //  
   if (dataType != Grid::SHORT_GRID &&
       dataType != Grid::FLOAT_GRID &&
       dataType != Grid::DOUBLE_GRID) {

      cerr << "Conversion from short not supported." << endl;
      return -1;
   }

   if (dataType == Grid::CHAR_GRID && scale != FLT_MIN) {
      cerr << "WARNING: Setting short data on TypeGrid<short> with ";
      cerr << "scaling turned on. This is a weird thing to do.";
      cerr << endl;
   }

   //
   // Process each z-level.
   //
   size_t planeDataLen = geometry.nx * geometry.ny;
   int * planeMap = new int[planeDataLen];
   for ( size_t zCount = 0; zCount < geometry.nz; zCount++) {

      T * dataPtr = data + (planeDataLen * zCount);

      // Todo: Don't use this planeMap stuff -- just copy values.

      int status;
      if (zCount == 0) {
         status = getPlaneMapping( dataGeom, planeMap);
         if (status < 0) {
            cerr << "WARNING: No mappable cells found." << endl;
         }
      }
   
      status = mapPlaneFromShortArray(buffer + (zCount * planeDataLen),
                                      planeMap, dataPtr, scale, bias, 
                                      badShort, missingShort);
      if (status < 0) { 
         cerr << "Could not map plane: " << zCount << " from short." << endl;
         delete [] planeMap;
         return -1;
      }
   }

   delete [] planeMap;
   return(  0  );
}

//
// Resample from an unscaled buffer to set the grid values.
//   Geometry of data need not match the grid.
//   Data scaling will happen if scale not set to FLT_MIN.
//
// virtual
template <class T>
int TypeGrid<T>::resampleFromShortArray( const unsigned short * buffer,
                                         const GridGeom &dataGeom,
                                         float scale, float bias,
                                         unsigned short badShort,
                                         unsigned short missingShort ) 
{
   int       srcZ;
   const unsigned short *sourcePlane;

   assert( data );

   // Supported destination types: short, float, and double.
   //  
   if (dataType != Grid::SHORT_GRID &&
       dataType != Grid::FLOAT_GRID &&
       dataType != Grid::DOUBLE_GRID) {

      cerr << "Conversion from short not supported." << endl;
      return -1;
   }

   if (dataType == Grid::CHAR_GRID && scale != FLT_MIN) {
      cerr << "WARNING: Resampling short data on TypeGrid<short> with ";
      cerr << "scaling turned on. This is a weird thing to do.";
      cerr << endl;
   }

   //
   // Process each z-level on the destination grid
   //   Have to check for too many z iterations, b/c sometimes
   //   dz == 0.0 in mdv files!
   //
   size_t planeDataLen = geometry.nx * geometry.ny;
   int * planeMap = new int[planeDataLen];
   size_t zCount, maxZCount, zIndex;

   // Set up stuff for iterating through z levels.
   if ( composite ) {
      assert( geometry.nz == 1  &&  geometry.dz != 0.0 );
      maxZCount = (size_t)(((maxz - geometry.minz + 1) / geometry.dz) + 0.5);
      if ( maxZCount > dataGeom.nz ) {
         maxZCount = dataGeom.nz;
      }
   }
   else {
      maxZCount = geometry.nz;
   }

   for ( zCount = 0; zCount < maxZCount; zCount++ ) {

      // Get the height for the current z level.
      float height = geometry.minz + ((float) zCount) * geometry.dz;

      // Get the z level from the dataGeom (code from Grid::getZLevel()).
      srcZ = (size_t)(((height - dataGeom.minz) / dataGeom.dz) + 0.5);
      if ( srcZ < 0 ) {
         cerr << "Got illegal srcZ." << endl;
         delete [] planeMap;
         return -1;
      }

      if ( composite )
         zIndex = 0;
      else
         zIndex = zCount;

      T * dataPtr = data + (planeDataLen * zIndex);

      sourcePlane = buffer + (srcZ * dataGeom.nx * dataGeom.ny);

      int status;
      if (zCount == 0) {
         status = getPlaneMapping( dataGeom, planeMap);
         if (status < 0) {
            cerr << "WARNING: No mappable cells found." << endl;
         }
      }
   
      status = mapPlaneFromShortArray(sourcePlane,
                                      planeMap, dataPtr, scale, bias, 
                                      badShort, missingShort);
      if (status < 0) { 
         cerr << "Could not map plane: " << zCount << " from short." << endl;
         delete [] planeMap;
         return -1;
      }
   }

   delete [] planeMap;
   return(  0  );
}

//
// Set grid values from array of same type as this grid.
//   Geometry of data must match geometry of this object.
//
// virtual
template <class T>
int TypeGrid<T>::setFromTArray( const T * buffer,
                                const GridGeom &dataGeom,
                                T badVal,
                                T missingVal )
{
   assert( data );

   if (geometry != dataGeom) {
      return -1;
   }

   clearData();
   size_t numVals = geometry.nx * geometry.ny;
   for (size_t i = 0; i < numVals; i++) {

      if ( composite && isSet( data[i] )) {
         //
         // Replace the value in a composite plane only if
         // the new value is legit. and is greater than the existing
         // value, i.e., we only support MAX composites
         //
         if ( buffer[i] == badVal ||
              buffer[i] == missingVal ||
              data[i] >= buffer[i] ) {
            continue;
         }
      }

      if (buffer[i] == badVal) {
         data[i] = badValueDef;
      }
      else if (buffer[i] == missingVal) {
         data[i] = missingValueDef;
      }
      else {
         data[i] = buffer[i];
      }
   }

   return 0;
}

//
// Resample from an array of the same type as this grid.
//   Geometry of data need not match the grid.
//
// virtual
template <class T>
int TypeGrid<T>::resampleFromTArray( const T * buffer,
                                     const GridGeom &dataGeom,
                                     T badVal,
                                     T missingVal )
{
   int       srcZ;
   const T *sourcePlane;

   assert( data );

   //
   // Process each z-level on the destination grid
   //   Have to check for too many z iterations, b/c sometimes
   //   dz == 0.0 in mdv files!
   //
   size_t planeDataLen = geometry.nx * geometry.ny;
   int * planeMap = new int[planeDataLen];
   size_t zCount, maxZCount, zIndex;

   // Set up stuff for iterating through z levels.
   if ( composite ) {
      assert( geometry.nz == 1  &&  geometry.dz != 0.0 );
      maxZCount = (size_t)(((maxz - geometry.minz + 1) / geometry.dz) + 0.5);
      if ( maxZCount > dataGeom.nz ) {
         maxZCount = dataGeom.nz;
      }
   }
   else {
      maxZCount = geometry.nz;
   }

   for ( zCount = 0; zCount < maxZCount; zCount++ ) {

      // Get the height for the current z level.
      float height = geometry.minz + ((float) zCount) * geometry.dz;

      // Get the z level from the dataGeom (code from Grid::getZLevel()).
      srcZ = (size_t)(((height - dataGeom.minz) / dataGeom.dz) + 0.5);
      if ( srcZ < 0 ) {
         return -1;
      }

      if ( composite )
         zIndex = 0;
      else
         zIndex = zCount;

      T* dataPtr = data + (planeDataLen * zIndex);

      sourcePlane = buffer + (srcZ * dataGeom.nx * dataGeom.ny);
      if ( sourcePlane ) {

         int status;
         if (zCount == 0) {
            status = getPlaneMapping( dataGeom, planeMap);
            if (status < 0) {
               cerr << "WARNING: No mappable cells found." << endl;
            }
         }
      
         status = mapPlaneFromTArray(sourcePlane,
                                     planeMap, dataPtr,
                                     badVal, missingVal);
         if (status < 0) { 
            cerr << "Could not map plane: " << zCount << " from T values."
                 << endl;
            delete [] planeMap;
            return -1;
         }
      }
   }

   delete [] planeMap;
   return(  0  );
}

//
//
// Set grid values from an unscaled buffer
// Maps a single altitude plane.
// Currently used as the interface to the old-style cdata server
//
template <class T>
int TypeGrid<T>::resampleFromCharArray( float altitude,
                                        const unsigned char* buffer,
                                        const GridGeom &dataGeom,
                                        float scale, float bias,
                                        unsigned char badChar,
                                        unsigned char missingChar )
{
   assert( data );                             

   // Supported destination types: char, float, and double.
   //  
   if (dataType != Grid::CHAR_GRID &&
       dataType != Grid::FLOAT_GRID &&
       dataType != Grid::DOUBLE_GRID) {

      cerr << "Conversion from char not supported." << endl;
      return -1;
   }

   if (dataType == Grid::CHAR_GRID && scale != FLT_MIN) {
      cerr << "WARNING: Resampling char data on TypeGrid<char> with ";
      cerr << "scaling turned on. This is a weird thing to do.";
      cerr << endl;
   }

   //                                         
   // Determine the z-level to resample onto.
   // A z-level outside of our range means there's no work to be done.
   //    This is not an error condition
   //
   
   int z = getZLevel( altitude );
   if ( z < 0  ||  (size_t)z > geometry.nz - 1 ) {
      return( 0 );
   }

   size_t planeDataLen = geometry.nx * geometry.ny;
   T* dataPtr = data + (planeDataLen * z);

   int * planeMap = new int[planeDataLen];
   int status = getPlaneMapping( dataGeom, planeMap);
   if (status < 0) {
      cerr << "WARNING: No mappable cells found." << endl;
   }

   status = mapPlaneFromCharArray(buffer,
                                  planeMap, dataPtr, scale, bias, 
                                  badChar, missingChar);
   if (status < 0) { 
      cerr << "Could not map plane: " << z << " from chars." << endl;
      delete [] planeMap;
      return -1;
   }

   delete [] planeMap;
   return(  0  );
}

//
// Set grid values from an unscaled buffer of shorts
// 
template <class T>
int TypeGrid<T>::resampleFromShortArray( float altitude,
                                         const unsigned short * buffer,
                                         const GridGeom &dataGeom,
                                         float scale, float bias,
                                         unsigned short badShort,
                                         unsigned short missingShort )
{
   assert( data );                             

   // Supported destination types: short, float, and double.
   //  
   if (dataType != Grid::SHORT_GRID &&
       dataType != Grid::FLOAT_GRID &&
       dataType != Grid::DOUBLE_GRID) {

      cerr << "Conversion from short not supported." << endl;
      return -1;
   }

   if (dataType == Grid::CHAR_GRID && scale != FLT_MIN) {
      cerr << "WARNING: Resampling short data on TypeGrid<short> with ";
      cerr << "scaling turned on. This is a weird thing to do.";
      cerr << endl;
   }

   //                                         
   // Determine the z-level to resample onto.
   // A z-level outside of our range means there's no work to be done.
   //    This is not an error condition
   //
   
   int z = getZLevel( altitude );
   if ( z < 0  ||  (size_t)z > geometry.nz - 1 ) {
      return( 0 );
   }

   size_t planeDataLen = geometry.nx * geometry.ny;
   T* dataPtr = data + (planeDataLen * z);

   int * planeMap = new int[planeDataLen];
   int status = getPlaneMapping( dataGeom, planeMap);
   if (status < 0) {
      cerr << "WARNING: No mappable cells found." << endl;
   }

   status = mapPlaneFromShortArray(buffer,
                                  planeMap, dataPtr, scale, bias, 
                                  badShort, missingShort);
   if (status < 0) { 
      cerr << "Could not map plane: " << z << " from shorts." << endl;
      delete [] planeMap;
      return -1;
   }

   delete [] planeMap;
   return(  0  );
}

// Old resampling method, for a single Z.
// 
template <class T>
int
TypeGrid<T>::resampleData( size_t z, void* buffer, const GridGeom &dataGeom,
                           float scale, float bias, 
                           unsigned char badChar,
                           unsigned char missingChar )
{
   assert( data );

   //
   // Degenerate case
   //
   if ( z > geometry.nz - 1 )
      return( 0 );

   //
   // Assign grid values by performing both data scaling and resampling
   //
   int              x, y, x1, y1, x2, y2;
   int              xData, yData;
   double           xPos, yPos, lat, lon;
   bool             doScaling, scaled;
   unsigned char    charValue, *charBuffer;
   T                gridValue, *gridBuffer, newValue;

   //
   // See if we need to scale the data
   //
   if ( scale == FLT_MIN ) {
      doScaling = false;
      gridBuffer = (T*)buffer;
   }
   else {
      doScaling = true;
      charBuffer = (unsigned char*)buffer;
   }

   //
   // Get the position of the incoming data corner
   // relative to the origin of the current grid geometry
   //
   geometry.latlon2xy( dataGeom.latCorner, dataGeom.lonCorner, &xPos, &yPos );

   //
   // Get the x,y bounds of the current grid to use for resampling
   // (x1, y1) = lower left       
   // (x2, y2) = upper right       
   //
   x1 = (int)rint( (xPos - geometry.minx) / geometry.dx );
   y1 = (int)rint( (yPos - geometry.miny) / geometry.dy );

   xPos += dataGeom.nx * dataGeom.dx;
   yPos += dataGeom.ny * dataGeom.dy;

   x2 = (int)rint( (xPos - geometry.minx) / geometry.dx );
   y2 = (int)rint( (yPos - geometry.miny) / geometry.dy );

   x1 = CLIP( x1, 0, (int)geometry.nx - 1 );
   y1 = CLIP( y1, 0, (int)geometry.ny - 1 );
   x2 = CLIP( x2, 0, (int)geometry.nx - 1 );
   y2 = CLIP( y2, 0, (int)geometry.ny - 1 );

   //
   // Process each cell in the current grid
   //
   for ( x=x1; x <= x2; x++ ) {
      for ( y=y1; y <= y2; y++ ) {

         //
         // Get the world coordinates of the current grid cell
         //
         xPos = geometry.minx + (x * geometry.dx);
         yPos = geometry.miny + (y * geometry.dy);
         geometry.xy2latlon( xPos, yPos, &lat, &lon );

         //
         // Get the corresponding x,y indicies for the incoming data
         // and make sure they're within bounds
         //
         dataGeom.latlon2xy( lat, lon, &xPos, &yPos );
         xData = (int)rint( (xPos - dataGeom.minx) / dataGeom.dx );
         yData = (int)rint( (yPos - dataGeom.miny) / dataGeom.dy );
if (x == 10 && y == 230) {
cerr << "x: 10, y: 230, xPos: " << xPos << ", yPos: " << yPos << ", dataGeom.minx: " << dataGeom.minx << ", dataGeom.miny: " << dataGeom.miny << ", dataGeom.dx: " << dataGeom.dx << ", dataGeom.dy: " << dataGeom.dy << ", xData: " << xData << ", yData: " << yData << endl;
}

         if ( xData < 0  ||  yData < 0  ||
              (size_t)xData > dataGeom.nx-1 ||  (size_t)yData > dataGeom.ny-1 )
            continue;

         //
         // Un-scale and fuzzify the data value, if necessary
         //
         size_t index = (yData * dataGeom.nx) + xData;

// TEST
// if (index == 74310 || index == 74311 || index == 74770) {
//     cout << "Got the non-zero value!" << endl;
// }

         if ( doScaling ) {
            charValue = charBuffer[index];
            scaled = byte2value( charValue, newValue, scale, bias, 
                                 badChar, missingChar );
            if ( scaled && fuzzyFcn )
               newValue = fuzzyFcn->apply( newValue );
         }
         else {
            //
            // No scaling, just fuzzify if necessary
            //
            gridValue = gridBuffer[index];
            if ( fuzzyFcn )
               newValue = fuzzyFcn->apply( gridValue );
            else
               newValue = gridValue;
         }

         //
         // Set the grid data with our new value
         //
         set( newValue, x, y, z );
      }
   }
   return(  0  );
}

// 
// Get the transformation matrix from the data to a plane in this grid.
//   This is an array of ints, each element a position in the data plane array.
// 
template <class T>
int TypeGrid<T>::getPlaneMapping( const GridGeom &srcGeom, int planeMap[])
{
   // 
   // Determine what type of work needs to be done to create the map:
   //   o Geometries Match: Create identity mapping.
   //   o Resolutions And Cell Locations Match: Just do cropping.
   //   o Everything Else: Calculate location of each cell.
   // 
   bool foundData = false;
   if (geometry.planeGeometriesMatch(srcGeom)) {
      // 
      // Geometries match. Create an identity mapping.
      // 
      size_t numVals = getNx() * getNy();
      for (size_t i = 0; i < numVals; i++) {
         planeMap[i] = i;
      }
      foundData = true;
   }
   else if (geometry.planeCellsCoincide(srcGeom)) {
      size_t numX = getNx();
      size_t numY = getNy();
      size_t maxX = srcGeom.nx;
      size_t maxY = srcGeom.ny;

      float xdiff = getMinx() - srcGeom.minx;
      float ydiff = getMiny() - srcGeom.miny;
      size_t xOff = (int) floor(xdiff / getDx());
      size_t yOff = (int) floor(ydiff / getDy());

      for ( size_t currX = 0; currX < numX; currX++ ) {
         for ( size_t currY = 0; currY < numY; currY++ ) {

            size_t thisIndex = (currY * numX) + currX;

            int newX = currX + xOff;
            int newY = currY + yOff;
            if (newX < 0 || newX > (int) maxX ||
                newY < 0 || newY > (int) maxY) {

               planeMap[thisIndex] = -1;
            }
            else {
               size_t otherIndex = (newY * srcGeom.nx) + newX;
               planeMap[thisIndex] = (int) otherIndex;
               foundData = true;
            }
         }
      }
   }
   else {
      //
      // Assign transformation mapping values by looking at the location 
      //   of each destination cell.
      //
      int              currX, currY,
                       minX, minY,
                       maxX, maxY,
                       srcX, srcY,
                       mapIndex;
      double           xPos, yPos, lat, lon;
   
      //
      // Get the position of the incoming data corner
      // relative to the origin of the current grid geometry
      //
      geometry.latlon2xy( srcGeom.latCorner, srcGeom.lonCorner, &xPos, &yPos );
   
      //
      // Get the x,y bounds of the current grid to use for resampling
      // (minX, minY) = lower left       
      // (maxX, maxY) = upper right       
      //
      minX = (int)rint( (xPos - geometry.minx) / geometry.dx );
      minY = (int)rint( (yPos - geometry.miny) / geometry.dy );
   
      xPos += srcGeom.nx * srcGeom.dx;
      yPos += srcGeom.ny * srcGeom.dy;
   
      maxX = (int)rint( (xPos - geometry.minx) / geometry.dx );
      maxY = (int)rint( (yPos - geometry.miny) / geometry.dy );
   
      minX = CLIP( minX, 0, (int)geometry.nx - 1 );
      minY = CLIP( minY, 0, (int)geometry.ny - 1 );
      maxX = CLIP( maxX, 0, (int)geometry.nx - 1 );
      maxY = CLIP( maxY, 0, (int)geometry.ny - 1 );
   
      //
      // Process each cell in the current grid
      //
      for ( currX = minX; currX <= maxX; currX++ ) {
         for ( currY = minY; currY <= maxY; currY++ ) {
   
            mapIndex = (currY * getNx()) + currX;
   
            //
            // Get the world coordinates of the current grid cell
            //
            xPos = geometry.minx + (currX * geometry.dx);
            yPos = geometry.miny + (currY * geometry.dy);
            geometry.xy2latlon( xPos, yPos, &lat, &lon );
   
            //
            // Get the corresponding x,y indicies for the incoming data
            // and make sure they're within bounds
            //
            srcGeom.latlon2xy( lat, lon, &xPos, &yPos );
            srcX = (int)rint( (xPos - srcGeom.minx) / srcGeom.dx );
            srcY = (int)rint( (yPos - srcGeom.miny) / srcGeom.dy );
// TEST
// if (currX == 10 && currY == 230) {
// cerr << "currX: 10, currX: 230, xPos: " << xPos << ", yPos: " << yPos << ", srcGeom.minx: " << srcGeom.minx << ", srcGeom.miny: " << srcGeom.miny << ", srcGeom.dx: " << srcGeom.dx << ", srcGeom.dy: " << srcGeom.dy << ", srcX: " << srcX << ", srcY: " << srcY << endl;
// }

            if ( srcX < 0  ||  srcY < 0  ||
                 (size_t) srcX > srcGeom.nx-1 ||  (size_t) srcY > srcGeom.ny-1 ) {
               planeMap[mapIndex] = -1;
               continue;
            }
            else {
               foundData = true;
               planeMap[mapIndex] = (srcY * srcGeom.nx) + srcX;
            }
         }
      }
   }

   if (foundData) {
      return(  0  );
   }
   else {
      // Failed to find any mappable grid cells.
      return( -1 );
   }
}

template <class T>
int TypeGrid<T>::getCharLookup(T * charLookup, float scale, float bias, 
                               unsigned char badChar,
                               unsigned char missingChar)
{
   if (dataType == CHAR_GRID) {
      unsigned char * l = (unsigned char *) charLookup;

      if (scale != FLT_MIN) {
         cerr << "TypeGrid<char>: "
              << "Cannot scale char data when converting to char values."
              << endl;
         return -1;
      }

      for (int i = 0; i <= UCHAR_MAX; i++) {
         l[i] = i;
         if ( fuzzyFcn )
            l[i] = (unsigned char)fuzzyFcn->apply( l[i] );
      }

      //
      // Do a cast here just to appease the compiler
      //
      l[badChar] = (unsigned char)badValueDef;
      l[missingChar] = (unsigned char)missingValueDef;
   }
   else if (dataType == FLOAT_GRID) {
      float * l = (float *) charLookup;

      for (int i = 0; i <= UCHAR_MAX; i++) {
         l[i] = (float) ((i * scale) + bias);
         if ( fuzzyFcn )
            l[i] = fuzzyFcn->apply( (T) l[i] );
      }

      l[badChar] = badValueDef;
      l[missingChar] = missingValueDef;
   }
   else if (dataType == DOUBLE_GRID) {
      double  * l = (double *) charLookup;

      for (int i = 0; i <= UCHAR_MAX; i++) {
         l[i] = (double) ((i * scale) + bias);
         if ( fuzzyFcn )
            l[i] = fuzzyFcn->apply( (T) l[i] );
      }

      l[badChar] = badValueDef;
      l[missingChar] = missingValueDef;
   }
   else {
      cerr << "TypeGrid<T>::getCharLookup(): This grid type not supported."
           << endl;
      return -1;
   }

   return 0;
}

template <class T>
int TypeGrid<T>::getShortLookup(T * shortLookup, float scale, float bias, 
                                unsigned short badShort,
                                unsigned short missingShort)
{
   if (dataType == SHORT_GRID) {
      unsigned short * l = (unsigned short *) shortLookup;

      if (scale != FLT_MIN) {
         cerr << "TypeGrid<unsigned short>: "
              << "Cannot scale short data when converting to short values."
              << endl;
         return -1;
      }

      for (int i = 0; i <= USHRT_MAX; i++) {
         l[i] = i;
         if ( fuzzyFcn )
            l[i] = (unsigned short)fuzzyFcn->apply( l[i] );
      }

      //
      // Do a cast here just to appease the compiler
      //
      l[badShort] = (unsigned short) badValueDef;
      l[missingShort] = (unsigned short) missingValueDef;
   }
   else if (dataType == FLOAT_GRID) {
      float * l = (float *) shortLookup;

      for (int i = 0; i <= USHRT_MAX; i++) {
         l[i] = (float) ((i * scale) + bias);
         if ( fuzzyFcn )
            l[i] = fuzzyFcn->apply( (T) l[i] );
      }

      l[badShort] = badValueDef;
      l[missingShort] = missingValueDef;
   }
   else if (dataType == DOUBLE_GRID) {
      double  * l = (double *) shortLookup;

      for (int i = 0; i <= USHRT_MAX; i++) {
         l[i] = (double) ((i * scale) + bias);
         if ( fuzzyFcn )
            l[i] = fuzzyFcn->apply( (T) l[i] );
      }

      l[badShort] = badValueDef;
      l[missingShort] = missingValueDef;
   }
   else {
      cerr << "TypeGrid<T>::getCharLookup(): This grid type not supported."
           << endl;
      return -1;
   }

   return 0;
}

template <class T>
void TypeGrid<T>::applyFuzzyFcn()
{
   if ( fuzzyFcn == NULL ) 
      return;

   size_t i, numValues = getNumValues();

   for( i=0; i<numValues; i++ ) {
      data[i] = fuzzyFcn->apply( data[i] );
   }
}

template <class T>
int TypeGrid<T>::fillPoly( Polyline &p, T v )
{
   assert( data );

   //
   // Sets grid points within a closed polygon to a given value
   //
   Point_d currentPoint;
   int numPoints;
   int iz = 0;
   double xMin = DBL_MAX;
   double xMax = DBL_MIN;
   double yMin = DBL_MAX;
   double yMax = DBL_MIN;
   int xStart, yStart, xStop, yStop;


   //
   // Make the origin of the polyline the same as the grid.
   //
   p.changeOrigin( geometry.projection );

   //
   // Make sure polyline is closed
   //
   if (!p.isClosed())
      return ( -1 );

   //
   // Convert polyline to array of Point_d structures and
   // find rectangle which encloses the polyline
   //
   
   numPoints = p.getNumPts();
   Point_d polygon[numPoints];
  
   for (int i = 0; i < numPoints; i++)
   {
      polygon[i].x = (double) p.getX(i);
      polygon[i].y = (double) p.getY(i);

      if (polygon[i].x < xMin)
         xMin = polygon[i].x;
      if (polygon[i].x > xMax)
         xMax = polygon[i].x;
      if (polygon[i].y < yMin)
         yMin = polygon[i].y;
      if (polygon[i].y > yMax)
         yMax = polygon[i].y;
      
   }

   xStart = (int) ((xMin - geometry.minx)/geometry.dx + 0.5);
   xStop  = (int) ((xMax - geometry.minx)/geometry.dx + 0.5);
   yStart = (int) ((yMin - geometry.miny)/geometry.dy + 0.5);
   yStop  = (int) ((yMax - geometry.miny)/geometry.dy + 0.5);

   //
   // Check if point is in grid - if it is, set the point to v
   //
   for (int iy = yStart; iy < yStop; iy++)
   {
      for (int ix = xStart; ix < xStop; ix++)
      {
         currentPoint.x = ix*geometry.dx+geometry.minx;
     currentPoint.y = iy*geometry.dx+geometry.miny;
     
     if (EG_point_in_polygon(currentPoint, polygon, numPoints))
         {
        set(v, ix, iy, iz);
     }
      }
   }
      
   return (0);
}

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

////////////////////////////////////////////
// fast polygon fill
//
// Sets grid points within a closed polygon to a given value
//

template <class T>
int TypeGrid<T>::fillPolyFast( Polyline &p, T v )
{
  
  assert( data );
  
  // Make the origin of the polyline the same as the grid.
  p.changeOrigin( geometry.projection );

  // Make sure polyline is closed
  if (!p.isClosed()) {
    return ( -1 );
  }

  // Convert polyline to array of Point_d structures and
  // find rectangle which encloses the polyline
  
  int numPoints = p.getNumPts();
  Point_d polygon[numPoints];
  for (int i = 0; i < numPoints; i++) {
    polygon[i].x = (double) p.getX(i);
    polygon[i].y = (double) p.getY(i);
  }

  // ensure that no vertex lies on a horizintal grid line
  // if so, move point slightly
  
  Point_d *pt = polygon;
  for (int i = 0; i < numPoints; i++, pt++) {
    double y = (pt->y - geometry.miny) / geometry.dy;
    if (fabs(fmod(y, 1.0)) < 0.0001) {
      pt->y += geometry.dy / 1000.0;
    }
  }

   // arrays for sides and crossings
  
  int nsides = numPoints - 1;
  EG_poly_side_t sides[nsides];
  EG_crossing_t crossings[nsides];

  // load sides, ensuring start has lesser y
  
  double poly_y_min = 1.0e99;
  double poly_y_max = -1.0e99;
  EG_poly_side_t *sd = sides;
  Point_d *pt1 = polygon;
  Point_d *pt2 = pt1 + 1;
  
  for (int i = 0; i < nsides; i++, sd++, pt1++, pt2++) {
    if (pt1->y < pt2->y) {
      sd->start = *pt1;
      sd->end = *pt2;
    } else {
      sd->start = *pt2;
      sd->end = *pt1;
    }
    poly_y_min = MIN(poly_y_min, sd->start.y);
    poly_y_max = MAX(poly_y_max, sd->end.y);
  }

  // loop through the y values in the grid
  
  for (int iy = 0; iy < (int) geometry.ny; iy++) {

    double y = geometry.miny + (double) iy * geometry.dy;

    if (y < poly_y_min || y > poly_y_max) {
      continue;
    }
    
    // find the segments which straddle the y value, and load
    // these into the crossings array

    int ncrossings = 0;
    sd = sides;
    for (int i = 0; i < nsides; i++, sd++) {
      if (sd->start.y < y && sd->end.y > y) {
        crossings[ncrossings].side = *sd;
        ncrossings++;
      }
    } // i

    if ((ncrossings % 2) != 0) {
      fprintf(stderr, "ERROR - TypeGrid<T>::fillPolyFast\n");
      fprintf(stderr, "ncrossings should always be even\n");
      continue;
    }
    
    // for each crossing, compute the x val of the crossing

    EG_crossing_t *cr = crossings;
    for (int i = 0; i < ncrossings; i++, cr++) {
      double slope = ((cr->side.end.y - cr->side.start.y) /
		      (cr->side.end.x - cr->side.start.x));
      cr->x = cr->side.start.x + (y - cr->side.start.y) / slope;
    }
    
    // sort the crossings in ascending order of x
    
    qsort((void *) crossings, ncrossings,
          sizeof(EG_crossing_t), EG_polygon_crossing_compare);
    
    // for each pair of crossings, compute the range in x and
    // set the relevant grid points

    EG_crossing_t *cr1 = crossings;
    EG_crossing_t *cr2 = cr1 + 1;
    for (int i = 0; i < ncrossings; i += 2, cr1 += 2, cr2 += 2) {
      
      int ix1 = (int) ((cr1->x - geometry.minx) / geometry.dx + 1.0);
      int ix2 = (int) ((cr2->x - geometry.minx) / geometry.dx);
      
      ix1 = MAX(ix1, 0);
      ix1 = MIN(ix1, (int) geometry.nx - 1);
      
      ix2 = MAX(ix2, 0);
      ix2 = MIN(ix2, (int) geometry.nx - 1);

      for (int ix = ix1; ix <= ix2; ix++) {
	set(v, ix, iy, 0);
      }
      
    } // i
    
  } // iy

  return (0);

}

//
// Interpolate open polyline over the grid
//
template <class T>
int
TypeGrid<T>::interpPoly( Polyline& p, vector< IndexPoint *>& gridPts,
                         T badVal, T missingVal )
{
   assert( data );

   int i, j, nPolyPts, nVals;
   int firstXIdex, firstYIdex, secondXIdex, secondYIdex;
   int xStartIdex, yStartIdex, xStopIdex, yStopIdex;
   int xIdex, yIdex;

   float xVal, yVal;
   float dist2first, segDist;
   float firstVal, secondVal;
   double slope;
   T val;

   IndexPoint *pt;
   vector< float > *firstValues;
   vector< float > *secondValues;
   vector< float >  outputValues;

   //
   // This routine is to be used for polylines that are not closed
   //
   if (p.isClosed ())
      return( -1 );

   //
   // Make the origin of the polyline the same as the grid.
   //
   if( p.changeOrigin( geometry.projection ) )
      return( -1 );
   
   nPolyPts = p.getNumPts();
   nVals    = p.getNumVals();

   //
   // Find the indeces (in the grid) of the first point
   // 
   firstXIdex = (int) ((p.getX(0) - geometry.minx)/geometry.dx + 0.5);
   firstYIdex = (int) ((p.getY(0) - geometry.miny)/geometry.dy + 0.5);

   //
   // Set up the first point - use this opportunity
   // to set up the output value vector
   //
   firstValues = p.getValues(0);
   for ( j = 0; j < nVals; j++ ) {
      firstVal = (*firstValues)[j];
      if( firstVal == badVal )
         outputValues.push_back( badValueDef );
      else if( firstVal == missingVal )
	 outputValues.push_back( missingValueDef );
      else       
	 outputValues.push_back( firstVal );
   }

   if( !outOfBounds( firstXIdex, firstYIdex ) ) {
      pt = new IndexPoint(outputValues, firstXIdex, firstYIdex, 0, this);
      gridPts.push_back( pt );
   }

   for (i = 1; i < nPolyPts; i++)
   {
      
      //
      // Find the indeces (in the grid) of the second point in
      // the segment
      //
      secondXIdex = (int) ((p.getX(i) - geometry.minx)/geometry.dx + 0.5);
      secondYIdex = (int) ((p.getY(i) - geometry.miny)/geometry.dy + 0.5);

      //
      // If the indeces (in the grid) of the second point are the
      // same as those of the first point, don't do anything
      //
      if( secondXIdex == firstXIdex && secondYIdex == firstYIdex )
         continue;

      //
      // Consider the case of a vertical line first
      //
      if( p.getX(i) == p.getX(i-1) ) {

         if( firstYIdex < secondYIdex ) {
            yStartIdex = firstYIdex;
            yStopIdex = secondYIdex;
         } else {
            yStartIdex = secondYIdex;
            yStopIdex = firstYIdex;
         }

         xIdex   = firstYIdex;

         firstValues  = p.getValues(i-1);
         secondValues = p.getValues(i);
         segDist = p.getY(i) - p.getY(i-1);

         for ( yIdex = yStartIdex+1; yIdex < yStopIdex; yIdex++ ) {
            yVal = yIdex * geometry.dy + geometry.miny;

            //
            // Calculate values at point
            //
            dist2first = yVal - p.getY(i-1);

            for ( j = 0; j < nVals; j++ ) {
               firstVal = (*firstValues)[j];
	       secondVal = (*secondValues)[j];

               if( firstVal == badVal || secondVal == badVal )
		  val = badValueDef;
	       else if( firstVal == missingVal || secondVal == missingVal )
		  val = missingValueDef;
	       else
                  val = (T) ((dist2first/segDist)*(secondVal - firstVal) + 
			     firstVal);

               outputValues[j] = val;
            }

            if( !outOfBounds( xIdex, yIdex ) ) {
               pt = new IndexPoint(outputValues, xIdex, yIdex, 0, this);
               gridPts.push_back( pt );
            }
         }

      }   // End vertical line case.
      else {

         //
         // Not a vertical line. 
         //   Find the slope of the segment
         //
         slope = (p.getY(i) - p.getY(i-1))/(p.getX(i) - p.getX(i-1));
         
         //
         // If the slope is greater than one, find an x for a given y
         //
         if (fabs(slope) > 1)
         {
   
            if (firstYIdex <= secondYIdex)
            {
               yStartIdex = firstYIdex;
               yStopIdex  = secondYIdex;
            }
            else
            {
               yStartIdex = secondYIdex;
               yStopIdex  = firstYIdex;
            }
   
            firstValues  = p.getValues(i-1);
            secondValues = p.getValues(i);
	    double dx = p.getX(i) - p.getX(i-1);
	    double dy = p.getY(i) - p.getY(i-1);
            segDist = sqrt(dx * dx + dy * dy);

            for (yIdex = yStartIdex+1; yIdex < yStopIdex; yIdex++)
            {
               yVal  = yIdex * geometry.dy + geometry.miny;
               xVal  = (yVal - p.getY(i))/slope + p.getX(i);
               xIdex = (int) ((xVal - geometry.minx)/geometry.dx + 0.5);
   
               //
               // Calculate values at point
               //
	       dx = xVal - p.getX(i-1);
	       dy = yVal - p.getY(i-1);
               dist2first = sqrt(dx * dx + dy * dy);
   
               for ( j = 0; j < nVals; j++ ) {

		  firstVal = (*firstValues)[j];
	          secondVal = (*secondValues)[j];

                  if( firstVal == badVal || 
                      secondVal == badVal )
		     val = badValueDef;
	          else if( firstVal == missingVal || 
                           secondVal == missingVal )
		     val = missingValueDef;
	          else
                     val = (T) ((dist2first/segDist)*(secondVal - firstVal) + 
				firstVal);

                  outputValues[j] = val;

               }
   
               if( !outOfBounds( xIdex, yIdex ) ) {
                  pt = new IndexPoint(outputValues, xIdex, yIdex, 0, this);
                  gridPts.push_back( pt );
               }
            }
         }
   
         //
         // If the slope is less than or equal to one, find a y for a given x
         //
         if (fabs(slope) <= 1)
         {
            if (firstXIdex <= secondXIdex)
            {
               xStartIdex = firstXIdex;
               xStopIdex  = secondXIdex;
            }
            else
            {
               xStartIdex = secondXIdex;
               xStopIdex  = firstXIdex;
            }
               
            firstValues  = p.getValues(i-1);
            secondValues = p.getValues(i);
	    double dx = p.getX(i) - p.getX(i-1);
	    double dy = p.getY(i) - p.getY(i-1);
            segDist  = sqrt(dx * dx + dy * dy);

            for (xIdex = xStartIdex+1; xIdex < xStopIdex; xIdex++)
            {
               xVal  = xIdex * geometry.dx + geometry.minx;
               yVal  = (xVal - p.getX(i))*slope + p.getY(i);
               yIdex = (int) ((yVal - geometry.miny)/geometry.dy + 0.5);
   
               //
               // Calculate value at point
               //
	       dx = xVal - p.getX(i-1);
	       dy = yVal - p.getY(i-1);
               dist2first = sqrt(dx * dx + dy * dy);

               for ( j = 0; j < nVals; j++ ) {

		  firstVal = (*firstValues)[j];
	          secondVal = (*secondValues)[j];

                  if( firstVal == badVal || 
                      secondVal == badVal )
		     val = badValueDef;
	          else if( firstVal == missingVal || 
                           secondVal == missingVal )
		     val = missingValueDef;
	          else
                     val = (T) ((dist2first/segDist)*(secondVal - firstVal) + 
				firstVal);

                  outputValues[j] = val;
               }
   
               if( !outOfBounds( xIdex, yIdex ) ) {
                  pt = new IndexPoint(outputValues, xIdex, yIdex, 0, this);
                  gridPts.push_back( pt );
               }
            }
         }
      }  // End non-vertical line case.

      //
      // Add the last point in the segment
      //
      secondValues = p.getValues(i);
      for ( j = 0; j < nVals; j++ ) {
	 secondVal = (*secondValues)[j];
         if( secondVal == badVal )
	    val = badValueDef;
         else if( secondVal == missingVal )
	    val = missingValueDef;
         else       
	    val = (T) ( secondVal );

         outputValues[j] = val;
      }

      if( !outOfBounds( secondXIdex, secondYIdex ) ) {
         pt = new IndexPoint(outputValues, secondXIdex, secondYIdex,
                             0, this);
         gridPts.push_back( pt );
      }

      //
      // Set the first point of the next segment to be the last point
      // of this segment
      //
      firstXIdex = secondXIdex;
      firstYIdex = secondYIdex;

   } // End for (each nPolyPts)

   outputValues.erase( outputValues.begin(), outputValues.end() );

   return(0);
}

//
// Interpolate open polyline over the grid
//
template <class T>
int
TypeGrid<T>::interpPoly( Polyline& p, vector< IndexPoint *>& gridPts )
{
   assert( data );

   int i, nPolyPts;
   int firstXIdex, firstYIdex, secondXIdex, secondYIdex;
   int xStartIdex, yStartIdex, xStopIdex, yStopIdex;
   int xIdex, yIdex;

   float xVal, yVal;
   double slope;

   IndexPoint *pt;

   //
   // This routine is to be used for polylines that are not closed
   //
   if (p.isClosed ())
      return( -1 );

   //
   // Make the origin of the polyline the same as the grid.
   //
   if( p.changeOrigin( geometry.projection ) )
      return( -1 );
   
   nPolyPts = p.getNumPts();

   //
   // Find the indeces (in the grid) of the first point
   // 
   firstXIdex = (int) ((p.getX(0) - geometry.minx)/geometry.dx + 0.5);
   firstYIdex = (int) ((p.getY(0) - geometry.miny)/geometry.dy + 0.5);

   if( !outOfBounds( firstXIdex, firstYIdex ) ) {
      pt = new IndexPoint(firstXIdex, firstYIdex, 0, this);
      gridPts.push_back( pt );
   }

   for (i = 1; i < nPolyPts; i++)
   {
      
      //
      // Find the indeces (in the grid) of the second point in
      // the segment
      //
      secondXIdex = (int) ((p.getX(i) - geometry.minx)/geometry.dx + 0.5);
      secondYIdex = (int) ((p.getY(i) - geometry.miny)/geometry.dy + 0.5);

      //
      // If the indeces (in the grid) of the second point are the
      // same as those of the first point, don't do anything
      //
      if( secondXIdex == firstXIdex && secondYIdex == firstYIdex )
         continue;

      //
      // Consider the case of a vertical line first
      //
      if( p.getX(i) == p.getX(i-1) ) {

         if( firstYIdex < secondYIdex ) {
            yStartIdex = firstYIdex;
            yStopIdex = secondYIdex;
         } else {
            yStartIdex = secondYIdex;
            yStopIdex = firstYIdex;
         }

         xIdex   = firstYIdex;

         for ( yIdex = yStartIdex+1; yIdex < yStopIdex; yIdex++ ) {

            if( !outOfBounds( xIdex, yIdex ) ) {
               pt = new IndexPoint(xIdex, yIdex, 0, this);
               gridPts.push_back( pt );
            }
         }

      }   // End vertical line case.
      else {

         //
         // Not a vertical line. 
         //   Find the slope of the segment
         //
         slope = (p.getY(i) - p.getY(i-1))/(p.getX(i) - p.getX(i-1));
         
         //
         // If the slope is greater than one, find an x for a given y
         //
         if (fabs(slope) > 1)
         {
   
            if (firstYIdex <= secondYIdex)
            {
               yStartIdex = firstYIdex;
               yStopIdex  = secondYIdex;
            }
            else
            {
               yStartIdex = secondYIdex;
               yStopIdex  = firstYIdex;
            }
   
            for (yIdex = yStartIdex+1; yIdex < yStopIdex; yIdex++)
            {
               yVal  = yIdex * geometry.dy + geometry.miny;
               xVal  = (yVal - p.getY(i))/slope + p.getX(i);
               xIdex = (int) ((xVal - geometry.minx)/geometry.dx + 0.5);
   
               if( !outOfBounds( xIdex, yIdex ) ) {
                  pt = new IndexPoint(xIdex, yIdex, 0, this);
                  gridPts.push_back( pt );
               }
            }
         }
   
         //
         // If the slope is less than or equal to one, 
	 // find a y for a given x
         //
         if (fabs(slope) <= 1)
         {
            if (firstXIdex <= secondXIdex)
            {
               xStartIdex = firstXIdex;
               xStopIdex  = secondXIdex;
            }
            else
            {
               xStartIdex = secondXIdex;
               xStopIdex  = firstXIdex;
            }
   
            for (xIdex = xStartIdex+1; xIdex < xStopIdex; xIdex++)
            {
               xVal  = xIdex * geometry.dx + geometry.minx;
               yVal  = (xVal - p.getX(i))*slope + p.getY(i);
               yIdex = (int) ((yVal - geometry.miny)/geometry.dy + 0.5);
   
               if( !outOfBounds( xIdex, yIdex ) ) {
                  pt = new IndexPoint(xIdex, yIdex, 0, this);
                  gridPts.push_back( pt );
               }
            }
         }
      }  // End non-vertical line case.

      //
      // Add the last point in the segment
      //
      if( !outOfBounds( secondXIdex, secondYIdex ) ) {
         pt = new IndexPoint(secondXIdex, secondYIdex,
                             0, this);
         gridPts.push_back( pt );
      }

      //
      // Set the first point of the next segment to be the last point
      // of this segment
      //
      firstXIdex = secondXIdex;
      firstYIdex = secondYIdex;

   } // End for (each nPolyPts)

   return(0);
}

template <class T>
int TypeGrid<T>::fillLine( int x1, int y1, int x2, int y2,
                           T fillValue, int zLevel ) 
{
   assert( data );

   int pos, xIdex, yIdex;
   int yStartIdex, xStartIdex;
   int yStopIdex, xStopIdex;

   int firstX = x1;
   int firstY = y1;
   int secondX = x2;
   int secondY = y2;
  
   float slope;
   float xVal, yVal;
   float startXVal, startYVal;

   //
   // Make sure firstX, firstY, secondX and secondY are in range
   //
   if( firstX < 0 )
      firstX = 0;
   if( firstX >= (int) geometry.nx )
      firstX = (int) geometry.nx - 1;
   if( secondX < 0 )
      secondX = 0;
   if( secondX >= (int) geometry.nx )
      secondX = (int) geometry.nx - 1;
   if( firstY < 0 )
      firstY = 0;
   if( firstY >= (int) geometry.ny )
      firstY  = (int) geometry.ny - 1;
   if( secondY < 0 )
      secondY = 0;
   if( secondY >= (int) geometry.ny )
      secondY = (int) geometry.ny - 1;
   
   //
   // Check that z index is in range
   //
   if( zLevel < 0 || zLevel >= (int) geometry.nz )
      return( -1 );

   //
   // Fill in the first point
   //
   pos = firstX + firstY*geometry.nx + zLevel*geometry.nx*geometry.ny;
   data[pos] = fillValue;
   
   //
   // Find the slope of the line between the two points
   //
   slope = ((secondY - firstY) * geometry.dx) / 
           ((secondX - firstX) * geometry.dy);
      
   //
   // If the slope is greater than one, find an x for a given y
   //
   if (fabs(slope) > 1.0)
   {
      if (firstY <= secondY)
      {
         yStartIdex = firstY;
         yStopIdex  = secondY;
         startYVal  = firstY * geometry.dy + geometry.miny;
         startXVal  = firstX * geometry.dx + geometry.minx;
      }
      else
      {
         yStartIdex = secondY;
         yStopIdex  = firstY;
         startYVal  = secondY * geometry.dy + geometry.miny;
         startXVal  = secondX * geometry.dx + geometry.minx;
      }

     
      for (yIdex = yStartIdex+1; yIdex < yStopIdex; yIdex++)
      {
         yVal  = yIdex * geometry.dy + geometry.miny - 0.5*geometry.dy;
         xVal  = (yVal - startYVal)/slope + startXVal;
         xIdex = (int) ((xVal - geometry.minx)/geometry.dx + 0.5);

         if( xIdex >= 0 && xIdex < (int) geometry.nx ) {
            if( yIdex >= 0 && yIdex < (int) geometry.ny ) {   
               pos = xIdex + yIdex*geometry.nx 
                           + zLevel*geometry.nx*geometry.ny;
               data[pos] = fillValue;
            }
            if( yIdex-1 >= 0 && yIdex-1 < (int) geometry.ny ) {
               pos = xIdex + (yIdex-1)*geometry.nx 
                           + zLevel*geometry.nx*geometry.ny;
               data[pos] = fillValue;
            }
         }
      }
   }
      
   //
   // If the slope is less than or equal to one, find a y for a given x
   //
   if (fabs(slope) <= 1.0)
   {
      if (firstX <= secondX)
      {
         xStartIdex = firstX;
         xStopIdex  = secondX;
         startYVal  = firstY * geometry.dy + geometry.miny;
         startXVal  = firstX * geometry.dx + geometry.minx;
      }
      else
      {
         xStartIdex = secondX;
         xStopIdex  = firstX;
         startYVal  = secondY * geometry.dy + geometry.miny;
         startXVal  = secondX * geometry.dx + geometry.minx;
      }

      for (xIdex = xStartIdex+1; xIdex < xStopIdex; xIdex++)
      {
         xVal  = xIdex * geometry.dx + geometry.minx - 0.5*geometry.dx;
         yVal  = (xVal - startXVal)*slope + startYVal;
         yIdex = (int) ((yVal - geometry.miny)/geometry.dy + 0.5);

         if( yIdex >= 0 && yIdex < (int) geometry.ny ) {
            if( xIdex >= 0 && xIdex < (int) geometry.ny ) {
               pos = xIdex + yIdex*geometry.nx 
                           + zLevel*geometry.nx*geometry.ny;
               data[pos] = fillValue;
            }
            if( xIdex-1 >= 0 && xIdex-1 < (int) geometry.ny ) {
               pos = xIdex-1 + yIdex*geometry.nx 
                             + zLevel*geometry.nx*geometry.ny;
               data[pos] = fillValue;
            }
         }
      }
   }

   //
   // fill in the second point
   //
   pos = secondX + secondY*geometry.nx 
                 + zLevel*geometry.nx*geometry.ny;
   data[pos] = fillValue;
   
   return(0);
}

template <class T>
void TypeGrid<T>::setValueDefs( T init, T bad, T missing)
{
   // Keep the fuzzy fcn in sync if there is one...
   if (fuzzyFcn != NULL) {
      fuzzyFcn->setValueDefs(bad, missing);
   }

   bool hadDefs = hasValueDefs;

   initValueDef    = init;
   badValueDef     = bad;
   missingValueDef = missing;
   hasValueDefs = true;

   if (!hadDefs) {
      allocateData();
   }
   initializeData();
}

template <class T>
void TypeGrid<T>::suggestValueDefs( T init, T bad, T missing)
{
   //
   // Degenerate case 
   //   We already have our value defs assigned -- bail out.
   //
   if ( hasValueDefs )
      return;

   //
   // Keep the fuzzy fcn in sync if there is one...
   // Todo:  make this a suggestion rather than a set
   //
   if (fuzzyFcn != NULL) {
      fuzzyFcn->setValueDefs(bad, missing);
   }

   initValueDef    = init;
   badValueDef     = bad;
   missingValueDef = missing;

   hasValueDefs    = true;

   allocateData();
   initializeData();
}

template <class T>
void TypeGrid<T>::setFuzzyFcn( FuzzyFcn<T> *fcn )
{
   // Set the data member.
   fuzzyFcn = fcn;

   // Make sure the fnc's bad and missing defs match this object.
   if (fuzzyFcn != NULL && hasValueDefs) {
      fuzzyFcn->setValueDefs(badValueDef, missingValueDef);
   }
}

// Provides access to TypeGrid<T>::resampleData(const TypeGrid<T> &)
//   via a virtual call in Grid::resampleData(const Grid &).
// 
// protected
// Virtual
template <class T>
int TypeGrid<T>::resampleFromSameTypeGrid(const Grid & src)
{
   if (getDataType() != src.getDataType()) {
      cerr << "ERROR: "
           << "In TypeGrid<T>::resampleFromSameTypeGrid(const Grid & src) "
           << "with grids of different types." << endl;
      return -1;
   }

   const TypeGrid<T> & typedSrc = (TypeGrid<T> &) src;
   return resampleData(typedSrc);
}

#endif
