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
// Projection calculations from Mike Dixon's libs/mdv/mdv_proj.c
// 
// Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
// February 1999
//
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <cassert>
#include <euclid/ProjFlat.hh>
#include <euclid/ProjLatlon.hh>
#include <euclid/ProjRUC2Lambert.hh>
#include <euclid/Projection.hh>
using namespace std;

//
// Static definitions
//   UNKNOWN values must be set to MAX to take advantage of the
//   feature in TDRP which allows the user to specify the "MAX" keyword
//   as a parameter value as an indicator of UNKNOWN.
//
const double   Projection::UNKNOWN_ORIGIN        = DBL_MAX;
const double   Projection::UNKNOWN_ROTATION      = DBL_MAX;

const double   Projection::UNKNOWN_ORIGIN_FL32   = FLT_MAX;
const double   Projection::UNKNOWN_ROTATION_FL32 = FLT_MAX;

// Temporary way to look up this class's version
//   of the projection id from the value in mdv file.
// 
//   Use this or assertions will fail.
// 
// static 
Projection::ProjId Projection::lookupProjId(int mdvFileProjId)
{
   switch (mdvFileProjId) {
     case 0: return LATLON;
     case 3: return LAMBERT;
     case 8: return FLAT;
     default: return UNKNOWN_PROJECTION;
   }
}

// Temporary way to look up the mdv file projection id 
//   from this class's version.
// 
// static 
int Projection::lookupMdvFileProjId(Projection::ProjId projId)
{
   switch (projId) {
      case     LATLON:             return  0;
      case     LAMBERT:            return  3;
      case     FLAT:               return  8;
      case     UNKNOWN_PROJECTION: return -1;
      default: return -2; // Why not?
   }
}

Projection::Projection()
{
   latOrigin  = UNKNOWN_ORIGIN;
   lonOrigin  = UNKNOWN_ORIGIN;
   rotation   = UNKNOWN_ROTATION;
   id         = UNKNOWN_PROJECTION;
   projection = 0;

   set( UNKNOWN_ORIGIN, UNKNOWN_ORIGIN,
        UNKNOWN_PROJECTION, UNKNOWN_ROTATION );
}

Projection::Projection( const Projection &source )
{
   latOrigin  = UNKNOWN_ORIGIN;
   lonOrigin  = UNKNOWN_ORIGIN;
   rotation   = UNKNOWN_ROTATION;
   id         = UNKNOWN_PROJECTION;
   projection = 0;
   set( source );
}

Projection::Projection( double latOrigin, double lonOrigin,
                        ProjId id, double rotation )
{
   latOrigin  = UNKNOWN_ORIGIN;
   lonOrigin  = UNKNOWN_ORIGIN;
   rotation   = UNKNOWN_ROTATION;
   id         = UNKNOWN_PROJECTION;
   projection = 0;
   set( latOrigin, lonOrigin, id, rotation );
}

Projection::~Projection()
{
   erase();
}

void
Projection::erase()
{
   delete projection;
   projection = 0;
   id = UNKNOWN_PROJECTION;
}

Projection&
Projection::operator=( const Projection &source )
{
   set( source );
   return( *this );
}

void
Projection::copy( const Projection& source )
{
   set( source );
}

bool
Projection::set( const Projection &source )
{
   return set( source.latOrigin, source.lonOrigin,
               source.id, source.rotation );
}

bool
Projection::set( double latVal, double lonVal, 
                 ProjId idVal, double rotationVal )
{
   latOrigin = latVal;
   lonOrigin = lonVal;
   rotation  = rotationVal;

   setProjectionType( idVal );

   return true;
}

bool
Projection::suggest( const Projection &source )
{
   return suggest( source.latOrigin, source.lonOrigin,
                   source.id, source.rotation );
}

bool
Projection::suggest( double latVal, double lonVal, 
                     ProjId idVal, double rotationVal )
{
   bool changed = false;

   if ( !isKnown( latOrigin )) {
      latOrigin = latVal;
      changed = true;
   }

   if ( !isKnown( lonOrigin )) {
      lonOrigin = lonVal;
      changed = true;
   }

   if ( !isKnown( rotation )) {
      rotation  = rotationVal;
      changed = true;
   }

   if ( !isKnown( id )) {
      changed = changed  &&  setProjectionType( idVal );
   }

   return( changed );
}

bool
Projection::setProjectionType( ProjId idVal )
{
   //
   // See if we're changing projections
   //
   bool changed = false;

   if ( id != idVal ) {
      changed = true;
   }

   //
   // Get rid of the old one before setting the new one
   //
   erase();
   id = idVal;

   //
   // Set up a new projection if we know what type it is
   //
   if ( isKnown( id )) {
      switch( id ) {
         case FLAT:
              projection = new ProjFlat( this );
              break;
         case LATLON:
              projection = new ProjLatlon( this );
              break;
         case LAMBERT:
              projection = new ProjRUC2Lambert( this );
              break;
         default:
              break;
      }
      updateOrigin();
   }

   return( changed );
}

void
Projection::updateOrigin()
{
   if ( isProjectionKnown() ) {
      assert( projection );
      projection->updateOrigin();
   }
}

void
Projection::getOrigin( double *lat, double *lon, double *rotate ) const
{
   if ( lat )
      *lat = latOrigin;
   if ( lon )
      *lon = lonOrigin;
   if ( rotate )
      *rotate = rotation;
}

int
Projection::translateOrigin( double x, double y )
{
   if ( !isProjectionKnown() ) {
      return -1;
   }

   //
   // Hmmm.  Not sure if this makes sense for lat/lon projections
   //   Or if the semantics are different
   //
   assert( projection );
   projection->xy2latlon( x, y, &latOrigin, &lonOrigin );
   updateOrigin();
   return 0;
}

int
Projection::xy2latlon( double x, double y, double *lat, double *lon ) const
{
   if ( !isProjectionKnown() ) {
      return -1;
   }

   assert( projection );
   projection->xy2latlon( x, y, lat, lon );
   return 0;
}

int
Projection::latlon2xy( double lat, double lon, double *x, double *y) const
{
   if ( !isProjectionKnown() ) {
      return -1;
   }

   assert( projection );
   projection->latlon2xy( lat, lon, x, y);
   return 0;
}
