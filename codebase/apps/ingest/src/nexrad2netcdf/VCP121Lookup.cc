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
////////////////////////////////////////////////////////////////
// VCP121Lookup - class that decides if we should use a given
//                tilt in the vcp 121 scan strategy. 
//
//
// Jaimi Yee,  RAP, NCAR, P.O.Box 3000, Boulder, CO, 
//   80307-3000, USA
//
// June 2005
//
// $Id: VCP121Lookup.cc,v 1.3 2016/03/07 01:23:10 dixon Exp $
//
/////////////////////////////////////////////////////////////////
#include <math.h>
#include "VCP121Lookup.hh"
using namespace std;

//
// Constants
//
const float RngLookup::N_MI_TO_KM = 1.8514;
const float RngLookup::RNG_TOL    = 1.0;

RngLookup::RngLookup() 
{
}

RngLookup::~RngLookup() 
{
}

void RngLookup::addRng( float uaRng, bool use ) 
{
   unambRngList[uaRng] = use;
}

bool RngLookup::useUnambRng( float uaRng ) 
{
   //
   // Look for this unambiguous range in our list
   //
   map< float, bool, less<float> >::iterator it;

   for( it = unambRngList.begin(); it != unambRngList.end(); it++ ) {
      if( fabs( (*it).first - uaRng ) < RNG_TOL ) {
         return( (*it).second );
      }
   }
   
   //
   // If we can't find the unambiguous range in the list, the default
   // behavior is to use this unambiguous range
   //
   return( true );
}

VCP121Lookup::VCP121Lookup( float elevTol ) 
{
   //
   // Set the elevation tolerance
   //
   elevTolerance = elevTol;
   
   //
   // 0.5 degree tilts
   //
   RngLookup *elev1 = new RngLookup();
   
   elev1->addRng( 466, true );  // surveillance
   elev1->addRng( 117, true );
   elev1->addRng( 137, false );
   elev1->addRng( 175, false );

   lookupTable[0.5] = elev1; 

   //
   // 1.5 degree tilts
   //
   RngLookup *elev2 = new RngLookup();
   
   elev2->addRng( 466, true );  // surveillance
   elev2->addRng( 117, true );
   elev2->addRng( 137, false );
   elev2->addRng( 175, false );

   lookupTable[1.5] = elev2;
   
   //
   // 2.4 degree tilts
   //
   RngLookup *elev3 = new RngLookup();
   
   elev3->addRng( 117, true );
   elev3->addRng( 137, false );
   elev3->addRng( 175, false );
   
   lookupTable[2.4] = elev3;
   
   //
   // 3.4 degree tilts
   //
   RngLookup *elev4 = new RngLookup();
   
   elev4->addRng( 117, true );
   elev4->addRng( 137, false );
   elev4->addRng( 175, false );
   
   lookupTable[3.4] = elev4;

   //
   // 4.3 degree tilts
   //
   RngLookup *elev5 = new RngLookup();
   
   elev5->addRng( 175, true );
   elev5->addRng( 127, false );
   
   lookupTable[4.3] = elev5;
   
   //
   // 6.0 degree tilts
   //
   RngLookup *elev6 = new RngLookup();
   
   elev6->addRng( 148, true );
   
   lookupTable[6.0] = elev6;
   
   //
   // 9.9 degree tilts
   //
   RngLookup *elev7 = new RngLookup();
   
   elev7->addRng( 127, true );
   
   lookupTable[9.9] = elev7;
   
   //
   // 14.6 degree tilts
   //
   RngLookup *elev8 = new RngLookup();
   
   elev8->addRng( 117, true );
   
   lookupTable[14.6] = elev8;
   
   //
   // 19.5 degree tilts
   //
   RngLookup *elev9 = new RngLookup();
   
   elev9->addRng( 117, true );
   
   lookupTable[19.5] = elev9;
   
}

VCP121Lookup::~VCP121Lookup() 
{
   map< float, RngLookup*, less<float> >::iterator it;
   
   for( it = lookupTable.begin(); it != lookupTable.end(); it++ ) {
      delete( (*it).second );
   }
}

bool VCP121Lookup::useUnambRng( float elev, float uaRng ) 
{
   map< float, RngLookup*, less<float> >::iterator it;

   //
   // First find elevation that is closest to incoming elevation
   //
   float nearestElev = -1.0;
   float minDistance = elevTolerance;
   
   for( it = lookupTable.begin(); it != lookupTable.end(); it++ ) {
      float distance = fabs( (*it).first - elev );
      
      if( distance < minDistance ) {
         minDistance = distance;
         nearestElev = (*it).first;
      }
   }

   //
   // If we don't know anything about this elevation, say that
   // we should use this unambiguous range as the default
   //
   if( nearestElev == -1.0 ) {
      return( true );
   }
   
   return( lookupTable[nearestElev]->useUnambRng( uaRng ) );
   
}

   


