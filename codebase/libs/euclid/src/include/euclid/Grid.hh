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


///////////////////////////////////////////////////////////////
// Grid.cc
//
// Container class for the templated TypeGrid<> class, so that the
//   different types can be contained in an MdvField in a generic 
//   way.
//
// Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
// January 1998
//
// Paddy McCarthy, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
// March 1999
//
///////////////////////////////////////////////////////////////

#ifndef GridINCLUDED
#define GridINCLUDED

#include <euclid/GridGeom.hh>

class Polyline;
class IndexPoint;

class Grid {
  public:

    enum DataType {
        CHAR_GRID,
        SHORT_GRID,
        INT_GRID,
        FLOAT_GRID,
        DOUBLE_GRID,
    };

    Grid(DataType type);
    Grid( const Grid& source );
    Grid( DataType type,
          size_t nx,   size_t ny,   size_t nz,
          float  dx,   float  dy,   float  dz,
          float  minx, float  miny, float  minz,
          double latOrigin, double lonOrigin,
          Projection::ProjId projectionId,
          double rotation  = 0.0);
    Grid( DataType type, const GridGeom & geom);
    virtual ~Grid();

    //
    // Duplicating data and characteristics from other grids
    // Original data and geometry are cleared out and
    // reset to match the source grid
    //
    void   copy( const Grid& source );

    size_t    getNx()            const { return geometry.nx; }
    size_t    getNy()            const { return geometry.ny; }
    size_t    getNz()            const { return geometry.nz; }
    size_t    getDimension()     const { return geometry.dimension; }
    size_t    getNumValues()     const { return geometry.getNumValues(); }
    float     getDx()            const { return geometry.dx; }
    float     getDy()            const { return geometry.dy; }
    float     getDz()            const { return geometry.dz; }
    float     getMinx()          const { return geometry.minx; }
    float     getMiny()          const { return geometry.miny; }
    float     getMinz()          const { return geometry.minz; }
    double    getLatOrigin()     const { return geometry.projection.latOrigin; }
    double    getLonOrigin()     const { return geometry.projection.lonOrigin; }
    double    getRotation()      const { return geometry.projection.rotation; }
    double    getLatCorner()     const { return geometry.latCorner; }
    double    getLonCorner()     const { return geometry.lonCorner; }

    //
    // Convert between cell indicies
    //
    // Return: 0 for success -1 for failure
    //
    void   index2xy( size_t cellIndex, 
                     size_t *xIndex, size_t *yIndex ) const
                   { return geometry.index2xy( cellIndex, xIndex, yIndex ); }

    //
    // Convert between cell indicies and world positions
    //
    // Return: 0 for success -1 for failure
    //
    void   index2latlon( size_t cellIndex, 
                         double *lat, double *lon ) const
                       { return geometry.index2latlon( cellIndex, lat, lon ); }

         
    void   xy2latlon( size_t xIndex, size_t yIndex, 
                      double *lat, double *lon ) const
                    { return geometry.xy2latlon( xIndex, yIndex, lat, lon ); }

    //
    // Convert between cell indicies and grid position relative to the origin
    //
    // Return: 0 for success -1 for failure
    //
    void   index2km( size_t cellIndex,
                     double *xKm, double *yKm ) const
                   { return geometry.index2km( cellIndex, xKm, yKm ); }

    void   xy2km( size_t xIndex, size_t yIndex, 
                  double *xKm, double *yKm ) const
                { return geometry.xy2km( xIndex, yIndex, xKm, yKm ); }

    void   km2xy( double xKm, double yKm,
                  size_t *xIndex, size_t *yIndex ) const
                { return geometry.km2xy( xKm, yKm, xIndex, yIndex ); }

    int    kmDelta2xyDelta( double xKmDelta, double yKmDelta,
                            size_t *xDelta, size_t *yDelta ) const
                          { return geometry.kmDelta2xyDelta( xKmDelta, yKmDelta,
                                                             xDelta, yDelta ); }

    //
    // Convert between grid position relative to the origin and world positions
    //
    // Return: 0 for success -1 for failure
    //

    int    xy2latlon( double x, double y, double *lat, double *lon ) const
                    { return geometry.xy2latlon( x, y, lat, lon ); }

    int    latlon2xy( double lat, double lon, double *x, double *y) const
                    { return geometry.latlon2xy( lat, lon, x, y ); }

    //
    // Other miscellaneous stuff
    //
    int    getZLevel( float height ) const;
    void   getOrigin( double *lat, double *lon, double *rotation=NULL ) const
                         { geometry.getOrigin( lat, lon, rotation ); }
    int    getClosestZ( float targetAltitude ) const
                         { return geometry.getClosestZ( targetAltitude ); }
    DataType getDataType() const { return dataType; }

    const GridGeom& getGeometry() const { return geometry; }
    const Projection& getProjection() const { return geometry.projection; }
    Projection::ProjId projectionType() const { return geometry.projection.id; }

    //
    // How much do we know?
    //
    bool isKnown( size_t val ) const { return geometry.isKnown( val ); }
    bool isKnown( float  val ) const { return geometry.isKnown( val ); }
    bool isKnown( Projection projection ) const
                                 { return geometry.isKnown( projection ); }

    bool isGeometryKnown() const { return geometry.isGeometryKnown(); }
    bool areGeometriesEqual(const Grid & other) const
                                 { return geometry == other.geometry; }
    bool outOfBounds( int x, int y, int z=0 ) const;

    //
    // Setting Geometry/Projection info
    // The boolean return indicates whether or not the specified geometry
    // resulted in a change to the grid -- these are destructive changes,
    // i.e., previous data and projections are not preserved
    //
    // Use resampleData() to preserve data with a different geometry/projection
    //
    bool suggestGeometry( const Grid &sourceGrid );
    bool suggestGeometry( const GridGeom &sourceGeom);
    
    bool setGeometry( const Grid &sourceGrid );
    bool setGeometry( const GridGeom &sourceGeom );
    bool setGeometry( size_t nx,   size_t ny,   size_t nz,
                      float  dx,   float dy,   float  dz,
                      float  minx, float miny, float  minz,
                      double latOrigin, double lonOrigin,
                      Projection::ProjId projectionId,
                      double rotation = 0.0 );

    int translateOrigin( float x, float y)
                      { return geometry.translateOrigin( x, y ); }

    bool setProjection( double latOrigin, double lonOrigin,
                        Projection::ProjId id, double rotation = 0.0 )
                      { return geometry.projection.set( latOrigin, lonOrigin,
                                                        id, rotation ); }

    // 
    // Resampling and setting values between grids.
    // 
    //   setData(...) requires that the grid types match.
    // 
    //   resampleData(...) DOES NOT require that the grid types match.
    // 
    //   setAndScaleData(...) and resampleAndScaleData(...) allow
    //     conversion from scaled type to floating type. If both the src
    //     and destination are unscaled types, no worries -- it just copies.
    // 
    //   All set...(...) methods require that geometries match.
    //     The resample...(...) methods don't require matching geometries.
    // 
    //   Use getCharData() and getShortData() to convert
    //     to scaled types. No direct conversion to a 
    //     Grid of these types is supported at this time.
    // 
    int setData(const Grid & src);
    int resampleData(const Grid & src);
    int setAndScaleData(const Grid & src, float scale, float bias);
    int resampleAndScaleData(const Grid & src, float scale, float bias);
    int suggestValueDefs(const Grid & src);


    //
    // Compositing characteristics
    //
    void      setComposite( float maxz );
    bool      isComposite() const { return composite; }
    float     getMaxZ() const { return maxz; }

    // 
    // Methods which *MUST* be defined by subclasses.
    // 

    virtual unsigned char * getCharData(float * scale = NULL,
                                        float * bias = NULL,
                                        unsigned char badOutputChar = 0,
                                        unsigned char missingOutputChar = 0,
                                        bool forceIntegralScaling = false)
            = 0;

    virtual unsigned short * getShortData(float * scale = NULL,
                                          float * bias = NULL,
                                          unsigned short badOutputShort = 0,
                                          unsigned short missingOutputShort = 0,
                                          bool forceIntegralScaling = false)
            = 0;

    virtual int setFromCharArray( const unsigned char* buffer,
                                  const GridGeom &geom,
                                  float scale, float bias,
                                  unsigned char badChar,
                                  unsigned char missingChar ) = 0;

    virtual int resampleFromCharArray( const unsigned char* buffer,
                                       const GridGeom &geom,
                                       float scale, float bias,
                                       unsigned char badChar,
                                       unsigned char missingChar ) = 0;

    virtual int setFromShortArray( const unsigned short * buffer,
                                   const GridGeom &geom, 
                                   float scale, float bias,
                                   unsigned short badShort,
                                   unsigned short missingShort ) = 0;

    virtual int resampleFromShortArray( const unsigned short * buffer,
                                        const GridGeom &geom,
                                        float scale, float bias,
                                        unsigned short badShort,
                                        unsigned short missingShort ) = 0;

    // Single altitude versions.
    // 
    // virtual int setFromCharArray( float altitude,
    //                               const unsigned char* buffer,
    //                               const GridGeom &geom,
    //                               float scale, float bias,
    //                               unsigned char badChar,
    //                               unsigned char missingChar ) = 0;

    virtual int resampleFromCharArray( float altitude,
                                       const unsigned char* buffer,
                                       const GridGeom &geom,
                                       float scale, float bias,
                                       unsigned char badChar,
                                       unsigned char missingChar ) = 0;

    // virtual int setFromShortArray( float altitude,
    //                                const unsigned short * buffer,
    //                                const GridGeom &geom,
    //                                float scale, float bias,
    //                                unsigned short badShort,
    //                                unsigned short missingShort ) = 0;

    virtual int resampleFromShortArray( float altitude,
                                        const unsigned short * buffer,
                                        const GridGeom &geom,
                                        float scale, float bias,
                                        unsigned short badShort,
                                        unsigned short missingShort ) = 0;

    virtual void suggestValueDefsFromChar( unsigned char init,
                                           unsigned char bad,
                                           unsigned char missing ) = 0;

    virtual void suggestValueDefsFromShort( unsigned short init,
                                            unsigned short bad,
                                            unsigned short missing ) = 0;

    virtual void suggestValueDefsFromFloat( float init,
                                            float bad,
                                            float missing ) = 0;

    virtual float getFloatInitValue()    const = 0;
    virtual float getFloatBadValue()     const = 0;
    virtual float getFloatMissingValue() const = 0;

    virtual void clearData() = 0;
    virtual void allocateData() = 0;
    virtual void initializeData() = 0;
    virtual void * getUnscaledData() = 0;

    //
    // Methods added for the brokering of Grid and Mdvx classes via MdvxGrid
    //
            int   getDataElementNbytes() const;
    virtual void  applyFuzzyFcn() = 0;
    virtual void* getVoidData() const = 0;
    virtual int   setFromVoidArray( const void * buffer,
                                    const GridGeom &dataGeom,
                                    float badVal,
                                    float missingVal ) = 0; 



  protected:
    //
    // Data geometry
    // The composite flag is a special case of the grid
    // which forces vertical geometry to be derived
    //
    DataType  dataType;
    bool      composite;
    float     maxz;
    GridGeom  geometry;

    // Virtual method used for forwarding resampling to
    //   TypeGrid<Type>::resampleData(const TypeGrid<Type> &).
    //   This is called from Grid::resampleData(const Grid &), and
    //   is not intended to be part of the public interface.
    // 
    virtual int resampleFromSameTypeGrid(const Grid & src) = 0;

    void            updateDerivedGeom( const Grid *srcGrid = NULL );
    void            computeMaxz(){ maxz = geometry.minz
                                        + ((geometry.nz-1) * geometry.dz); }

  private:
    // Private assignment operator with no body. Do not assign
    //   Grids at this level -- must know intantiation type.
    Grid&  operator=( const Grid &source );

    // Private default constructor with no body.
    //   Default construction not allowed -- must know intantiation type.
    Grid();

    friend class MdvIngest;
};

#endif
