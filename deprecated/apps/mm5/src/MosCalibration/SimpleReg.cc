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
// Simple Linear Regression 
//
// $Id: SimpleReg.cc,v 1.21 2016/03/07 01:33:50 dixon Exp $
//
///////////////////////////////////////////////////////////////////
#include <toolsa/DateTime.hh>

#include "SimpleReg.hh"
#include "MosCalibration.hh"
using namespace std;

SimpleReg::SimpleReg( const string& baseName, double missVal,
                      const string& xName, Params* params )
    :Regression( baseName, missVal, params )
{
   numVars   = 1;
   detString = "correlation";
   detExt    = "Corr";
   coefficientNames.push_back( "constant" );
   coefficientNames.push_back( xName );
}

void
SimpleReg::clear() 
{
   clearReg();
   
   y.erase( y.begin(), y.end() );
   x.erase( x.begin(), x.end() );
   modelUnixTime.clear();
   metarUnixTime.clear();
   forecastLeadSecs.clear();

   numPts = 0;
}

void
SimpleReg::addPoint( double yVal, double xVal, time_t metarTime, time_t modelTime, int forecastLead   ) 
{
   y.push_back( yVal );
   x.push_back( xVal );
   metarUnixTime.push_back(metarTime);
   modelUnixTime.push_back(modelTime);
   forecastLeadSecs.push_back(forecastLead);
   numPts++;
}

int
SimpleReg::doRegression() 
{
   POSTMSG( INFO, "SimpleReg: doRegression: entry");

   //
   // Clear out coefficients, etc.
   //
   clearReg();
   POSTMSG( INFO, "SimpleReg: doRegression: test b");

   //
   // Check for data
   //
   if( x.size() < 1 || y.size() < 1 ) {
      POSTMSG( INFO, "No data for regression" );
      POSTMSG( INFO, "SimpleReg: doRegression: exit a");
      return( SUCCESS );
   }
   POSTMSG( INFO, "SimpleReg: doRegression: test c");

   //
   // Calculate sums
   //
   int    n      = numPts;
   double sumX   = 0.0;
   double sumY   = 0.0;
   double sumXSq = 0.0;
   double sumYSq = 0.0;
   double sumXY  = 0.0;
   
   for( int i = 0; i < n; i++ ) {
      sumX   += x[i];
      sumY   += y[i];
      sumXSq += x[i] * x[i];
      sumYSq += y[i] * y[i];
      sumXY  += x[i] * y[i];
   }
   POSTMSG( INFO, "SimpleReg: doRegression: test d");
   
   //
   // Find the coefficients
   //
   double a1 = ( n * sumXY - sumX * sumY ) / ( n * sumXSq - sumX * sumX );
   double a0 = ( sumY - a1 * sumX ) / n;
   
   coefficients.push_back( a0 );
   coefficients.push_back( a1 );
   POSTMSG( INFO, "SimpleReg: doRegression: test e");

   //
   // Find the correlation
   //
   double correlation = ( n * sumXY - sumX * sumY ) / 
      (sqrt( n * sumXSq - sumX * sumX ) * 
       sqrt( n * sumYSq - sumY * sumY ));

   //
   // Set the coefficient of determination
   //   Note that in the case of simple linear regression,
   //   the coefficient of determination = the correlation
   //   coefficient
   //
   determination = correlation;
   POSTMSG( INFO, "SimpleReg: doRegression: test g");

   //
   // Find the maximum error
   //   Set this to some negative number so that the
   //   absolute value of the error will always be greater
   //   than this -- assures us that we will indeed get the
   //   max error.
   //
   double maxE = -9999.0;
   
   for( int i = 0; i < n; i++ ) {
      double error = fabs( y[i] - (a0 + a1*x[i]) );
      if( error > maxE )
         maxE = error;
   }
   maxError = maxE;

   POSTMSG( INFO, "SimpleReg: doRegression: exit b");
   return( SUCCESS );
   
}

int
SimpleReg::writeVars( const string& outputFile )
{
   //
   // Open the file
   //
   FILE *fp = fopen( outputFile.c_str(), "wt" );
   if ( fp == NULL ) {
      POSTMSG( ERROR, "Could not open file %s", outputFile.c_str() );
      return( FAILURE );
   }

   if (x.size() != modelUnixTime.size())
     cerr << "X size (" << x.size() << ") != modelUnixTime size (" << modelUnixTime.size() << ")" << endl;

   //
   // Write the data 
   if (_params->variable_output_format == Params::VARIABLE_OUT_CSV){
     fprintf(fp, "metarHumanTime, metarUnixTime, modelHumanTime, modelUnixTime, modelForecastLead, modelInitHumanTime, modelInitUnixTime, %s-metar, %s-model\n",name.c_str(),name.c_str());
	 for( int i = 0; i < numPts; i++ ) {
	   fprintf( fp, "%s,%ld,%s,%ld,%ld,%s,%ld,%11.7g,%11.7g\n", utimstr(metarUnixTime[i]),metarUnixTime[i],
		    utimstr(modelUnixTime[i]),modelUnixTime[i],forecastLeadSecs[i],
		    utimstr(modelUnixTime[i]-forecastLeadSecs[i]),modelUnixTime[i]-forecastLeadSecs[i],
		    y[i], x[i] );
	 }   
   }
   else{
	 //   add a column of ones to be consistent with multiple linear
	 //   regression output and so that we don't have to do that
	 //   in programs like matlab
	 //
	 for( int i = 0; i < numPts; i++ ) {
	   fprintf( fp, "%11.7g\t1\t%11.7g\n", y[i], x[i] );
	 }
   }

   fclose( fp );
   
   return( SUCCESS );
}

   
   








