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
// This class provides a client interface to the supported projection types.
// The ProjId enumeration indicates which projections are currently supported.
// To add support for a new projection, define an new id to the ProjId
// enumeration and see the ProjType abstract base class for the inheritance
// scheme.  The Projection, ProjType base class, and ProjType subclasses
// follow the STATE pattern described in the _Design Patterns_ text.
//
// Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA   
// February 1999
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _PROJECTION_INC_
#define _PROJECTION_INC_

#include <limits.h>
#include <float.h>

//
// Forward class declaration
//
class Grid;
class GridGeom;
class ProjType;


class Projection {

public:

   //
   // Special UNKNOWN values
   //    The FL32 versions are required for the MdvField class 
   //    which needs to store UNKNOWN values in a fl32 member 
   //    of the mdv_field_hdr_t
   //
   static const double  UNKNOWN_ORIGIN;
   static const double  UNKNOWN_ROTATION;
   static const double  UNKNOWN_ORIGIN_FL32;
   static const double  UNKNOWN_ROTATION_FL32;

   //
   // For backwards compatibility, make the match 
   // the original mdv file definition
   //
   enum ProjId { LATLON  = 0,
                 LAMBERT = 3, 
                 FLAT    = 8, 
                 UNKNOWN_PROJECTION = INT_MAX };

   static ProjId lookupProjId(int mdvFileProjId);
   static int    lookupMdvFileProjId(Projection::ProjId projId);

   Projection();
   Projection( const Projection& source );

   Projection( double latOrigin, double lonOrigin, 
               ProjId id, double rotation = 0.0 );

  ~Projection();

   //
   // Setting the projection
   //
   void          copy( const Projection& source );
   Projection&   operator=( const Projection& source );
   bool          operator==( const Projection& other ) const
                           { return( latOrigin == other.latOrigin &&
                                     lonOrigin == other.lonOrigin &&
                                     rotation == other.rotation &&
                                     id == other.id ); }
   bool          operator!=( const Projection& other ) const
                           { return !(*this == other); }

   bool          set( const Projection &source );

   bool          set( double latOrigin, double lonOrigin, 
                      ProjId id, double rotation = 0.0 );

   bool          suggest( const Projection &source );

   bool          suggest( double latOrigin, double lonOrigin, 
                          ProjId id, double rotation = 0.0 );

   //
   // Fetching info
   //
   double        getLatOrigin() const { return latOrigin; }
   double        getLonOrigin() const { return lonOrigin; }
   double        getRotation() const { return rotation; }
   ProjId        getType() const { return id; }

   void          getOrigin( double *lat, 
                            double *lon, 
                            double *rotation = 0 ) const;

   //
   // Projection translations
   //
   int       translateOrigin( double x, double y );
   int       xy2latlon( double x, double y, double *lat, double *lon ) const;
   int       latlon2xy( double lat, double lon, double *x, double *y) const;

   //
   // How much do we know?
   //
   static bool      isKnown( double val )
                       { return val == UNKNOWN_ORIGIN ? false : true; }
   static bool      isKnown( int val )
                       { return val == UNKNOWN_PROJECTION ? false : true; }
   static bool      isKnown( float val )
                       { return val == UNKNOWN_ORIGIN_FL32 ? false : true; }
   bool             isProjectionKnown() const
                       { return ( isKnown( latOrigin ) && 
                                  isKnown( lonOrigin ) && 
                                  isKnown( rotation )  &&
                                  isKnown( id ) ); }

private:

   //
   // Geometry values specified by the caller
   //
   double           latOrigin;
   double           lonOrigin;
   double           rotation;
   ProjId           id;

   //
   // Derived projection class which morphs in accordance with the
   // user-specified projection id
   //
   ProjType        *projection;

   void             erase();
   void             updateOrigin();
   bool             setProjectionType( ProjId idVal );

   friend class Grid;
   friend class CGrid;
   friend class GridGeom;
};

#endif
