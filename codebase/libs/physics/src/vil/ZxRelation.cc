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
// Computes the Z-x relationships for radar reflectivity 
// i.e., Z = aX^b where X is typically mass, kinetic energy, or rate
//
// If you prefer to specify the relationship in the form X = aZ^b,
// set the inverseForm flag to true (default is false).
//
// Terri L. Betancourt
// October 2001
//
// $Id: ZxRelation.cc,v 1.7 2016/03/03 18:43:53 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////


#include <float.h>
#include <physics/ZxRelation.hh>
using namespace std;

const double ZxRelation::UNDEFINED = DBL_MIN;

ZxRelation::ZxRelation()
{
   coeff     = UNDEFINED;
   exponent  = UNDEFINED;
   minDbz    = UNDEFINED;
   maxDbz    = UNDEFINED;
}

ZxRelation::ZxRelation( double a, double b,
                        double minThreshold,
                        double maxThreshold,
                        bool inverseForm )
{
   setRelationship( a, b, minThreshold, maxThreshold, inverseForm );
}

void
ZxRelation::setRelationship( double a, double b,
                             double minThreshold,
                             double maxThreshold,
                             bool inverseForm )
{
   minDbz = minThreshold;
   maxDbz = maxThreshold;

   if ( inverseForm ) {
      coeff        = 1.0 / a;
      inverseCoeff = a;
      exponent     = 1.0 / b;
      inverseExp   = b;
   }
   else {
      coeff        = a;
      inverseCoeff = 1.0 / a;
      exponent     = b;
      inverseExp   = 1.0 / b;
   }
}

void
ZxRelation::addDbz( double dbz )
{
   double thisDbz = dbz;

   //
   // See if dbz is below the min threshold, if so, ignore
   //
   if ( minDbz != UNDEFINED  &&  thisDbz < minDbz ) {
      return;
   }

   //
   // See if dbz is above the max threshold, if so, set to max
   //
   if ( maxDbz != UNDEFINED  &&  thisDbz > maxDbz ) {
      thisDbz = maxDbz;
   }

   //
   // Accumulate
   //
   sumX += pow( 10.0, thisDbz/10.0*inverseExp );
}

void
ZxRelation::addZ( double Z )
{
   double thisZ = Z;

   //
   // See if Z is below the min threshold, if so, ignore
   //
   if ( minDbz != UNDEFINED  &&  getDbzfromZ( thisZ ) < minDbz ) {
      return;
   }

   //
   // See if Z is above the max threshold, if so, set to max          
   //
   if ( maxDbz != UNDEFINED  &&  getDbzfromZ( thisZ ) > maxDbz ) {
      thisZ = Z;
   }

   //
   // Accumulate         
   //
   sumX += pow( thisZ, inverseExp );
}
