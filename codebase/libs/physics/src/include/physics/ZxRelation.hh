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
// $Id: ZxRelation.hh,v 1.7 2016/03/03 19:23:21 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _ZXRELATION_INC_
#define _ZXRELATION_INC_


#include <math.h>
using namespace std;

class ZxRelation
{
public:
   ZxRelation();
  ~ZxRelation(){};

   //
   // Used for default arguments
   //
   static const double UNDEFINED;

   //
   // Coefficient (a) and exponent (b) are required
   // Thresholds are used only when integrating X
   //   values below the minThreshold are ignored
   //   values above the maxThreshold are set to maxThreshold
   //   maxThreshold is used for removing the effects of hail
   // Set inverseForm to true when using the form X = aZ^b
   // default form of the relationship is Z = aX^b
   //
   ZxRelation( double coeff, double exponent, 
               double minThreshold = UNDEFINED, 
               double maxThreshold = UNDEFINED, 
               bool inverseForm = false );

   //
   // Setting individual components of the relationship after instantiation
   //
   void setRelationship( double coeff, double exponent, 
                         double minThreshold = UNDEFINED, 
                         double maxThreshold = UNDEFINED, 
                         bool inverseForm = false );

   void setMinThreshold( double threshold ){ minDbz = threshold; }
   void setMaxThreshold( double threshold ){ maxDbz = threshold; }

   //
   // Solve for X at a point, given Z
   //
   double getXfromZ( double Z )
                   { return( pow( Z/coeff, inverseExp )); }

   //
   // Solve for X at a point, given dbz
   //
   double getXfromDbz( double dbz )
                     { return( pow( getZfromDbz(dbz)/coeff, inverseExp )); }

   //
   // This is just the basic form of the equation: Z = aX^b
   //
   double getZfromX( double X )
                   { return( coeff * pow( X, exponent )); }

   //
   // Nothing to do with Z-x relationships
   // but this is a hand class to put these static method
   //
   static double getZfromDbz( double dbz )
                            { return( pow( 10.0, dbz/10.0 )); }

   static double getDbzfromZ( double Z ) 
                            { return( log10(Z) * 10.0 ); }

   //
   // Integrating X
   // The sequence of calls for each integration of X is:
   //   initIntegration()
   //   addDbz() or addZ() for as all points to be included in the integration
   //   integralOfX()
   //
   void   initIntegration(){ sumX = 0; }
   void   addDbz( double dbz );
   void   addZ( double Z );

   double integralOfX( double deltaPerUnit = 1.0 )
                     { return( sumX * pow( inverseCoeff, inverseExp ) 
                                    * deltaPerUnit ); }

private:

   double coeff;
   double exponent;
   double minDbz;
   double maxDbz;

   double inverseExp;
   double inverseCoeff;
   double sumX;
};

#endif
