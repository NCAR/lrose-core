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
///////////////////////////////////////////////////////////////////
//
// Regression methods and coefficients 
//
// $Id: Regression.cc,v 1.29 2016/03/07 01:33:50 dixon Exp $
//
///////////////////////////////////////////////////////////////////
#include <toolsa/DateTime.hh>

#include "Regression.hh"
#include "MosCalibration.hh"
using namespace std;

Regression::Regression( const string& baseName, double missVal, Params* p )
{
   missingVal       = missVal;

   inactiveCoeffs   = false;
   numPts           = 0;
   numVars          = 0;
   name             = baseName;
   determination    = missingVal; 
   adjDetermination = missingVal;
   detString        = "";
   detExt           = "";
   _params 			= p;
}

Regression::~Regression() 
{
   clearReg();
   coefficientNames.erase( coefficientNames.begin(), coefficientNames.end() );
}

void
Regression::setInactiveVarNames( vector< string >& varNames ) 
{
   inactiveCoeffs = true;
   
   for( int i = 0; i < (int) varNames.size(); i++ ) {
      inactiveCoeffNames.push_back( varNames[i] );
   }
}

void
Regression::clearReg() 
{
   coefficients.erase( coefficients.begin(), coefficients.end() ) ;
     
   determination    = missingVal;
   adjDetermination = missingVal;
   maxError         = missingVal;
}

int
Regression::writeDetermination( const string& outputFile )
{
   //
   // Open the file
   //
   FILE *fp = fopen( outputFile.c_str(), "a" );
   if ( fp == NULL ) {
      POSTMSG( ERROR, "Could not open file %s", outputFile.c_str() );
      return( FAILURE );
   }

   //
   // Write the coefficient of determination
   //
   fprintf( fp, "%11.7g\n", determination );
   
   fclose( fp );
   
   return( SUCCESS );
}

int
Regression::writeAdjDetermination( const string& outputFile )
{
   //
   // Open the file
   //
   FILE *fp = fopen( outputFile.c_str(), "a" );
   if ( fp == NULL ) {
      POSTMSG( ERROR, "Could not open file %s", outputFile.c_str() );
      return( FAILURE );
   }

   //
   // Write the coefficient of determination
   //
   fprintf( fp, "%11.7g\n", adjDetermination );
   
   fclose( fp );
   
   return( SUCCESS );
}


int
Regression::writeInfo( const string& outputFile, time_t startTime, 
                       time_t endTime )
{
   //
   // Open the file
   //
   FILE *fp = fopen( outputFile.c_str(), "a" );
   if ( fp == NULL ) {
      POSTMSG( ERROR, "Could not open file %s", outputFile.c_str() );
      return( FAILURE );
   }

   //
   // Write a header
   //
   DateTime start( startTime );
   DateTime end( endTime );
   
   fprintf( fp, "From %s to %s\n", start.dtime(), end.dtime() );
   
   //
   // Write the coefficients
   //
   if( coefficients.size() < 1 ) {
      fprintf( fp, "\tcoefficients undefined\n");
   }
   else {
      fprintf( fp, "\tregression coefficients:\n" );
      for( int i = 0; i < (int) coefficients.size(); i++ ) {
         fprintf( fp, "\t   %11.7g\n", coefficients[i] );
      }
   }

   //
   // Write the coefficient of determination
   //   Note that in the case of simple linear regression,
   //   the coefficient of determination = the correlation
   //   coefficient.
   //
   if( determination == missingVal ) {
      fprintf( fp, "\n\n\t%s = undefined\n", detString.c_str() );
   }
   else {
      fprintf( fp, "\n\n\t%s = %11.7g\n", detString.c_str(), determination );
   }

   //
   // Write the adjused coefficient of determination if it exists
   //
   if( adjDetermination != missingVal ) {
      fprintf( fp, "\n\n\tadjusted coefficient of determination = %11.7g\n",
               adjDetermination );
   }
   
   //
   // Write the maximum error
   //
   if( maxError == missingVal ) {
      fprintf( fp, "\n\n\tmaximum error = undefined\n" );
   }
   else {
      fprintf( fp, "\n\n\tmaximum error = %11.7g\n", maxError );
   }

   //
   // Write the number of points
   //
   fprintf( fp, "\n\n\tnumber of points = %d\n", numPts );

   //
   // Write the number of variables
   //
   fprintf( fp, "\n\n\tnumber of variables = %d\n", numVars );

   fclose( fp );
   
   return( SUCCESS );
}

int
Regression::writeCoeff( const string& outputFile )
{

   //
   // Open the file
   //
   FILE *fp = fopen( outputFile.c_str(), "a" );
   if ( fp == NULL ) {
      POSTMSG( ERROR, "Could not open file %s", outputFile.c_str() );
      return( FAILURE );
   }
   
   //
   // Write the coefficients
   //
   for( int i = 0; i < (int) coefficients.size(); i++ ) {
      fprintf( fp, "%11.7g\t", coefficients[i] );
   }
   fprintf( fp, "\n" );
   

   fclose( fp );
   
   return( SUCCESS );
}



