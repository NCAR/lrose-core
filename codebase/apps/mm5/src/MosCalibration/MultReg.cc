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
// Multiple Linear Regression 
//
// $Id: MultReg.cc,v 1.45 2016/03/07 01:33:50 dixon Exp $
//
///////////////////////////////////////////////////////////////////

#include "MultReg.hh"
#include "MultRegVar.hh"
#include "MosCalibration.hh"

#include <iostream>
#include <iomanip>
#include <toolsa/DateTime.hh>
#include <rapmath/math_macros.h>
#include <clapack/clapack.h>

using namespace std;

//
// Constants
const int MultReg::NRHS      = 1;
const int MultReg::MIN_N_PTS = 3;

const int debug = 1;    // debug level

MultReg::MultReg( const string& baseName, double missVal,
                  vector< string >& varNames, Params* params, bool useNorm ) 
    : Regression( baseName, missVal, params )
{
   numVars   = 0;
   normalize = useNorm;
   detString = "coefficient of determination";
   detExt    = "Determination";

   coefficientNames.push_back( "constant" );
   for( int i = 0; i < (int) varNames.size(); i++ ) {
      coefficientNames.push_back( varNames[i] );
   }
   
}

void
MultReg::clear() 
{
   clearReg();
   
   y.clear();
   
   vector< MultRegVar* >::iterator it;
   for( it = x.begin(); it != x.end(); it++ ) {
      delete( (*it) );
   }
   x.erase( x.begin(), x.end() );

   modelUnixTime.clear();
   metarUnixTime.clear();
   forecastLeadSecs.clear();
   numPts  = 0;
   numVars = 0;
}

int
MultReg::addPoint( double yVal, vector< double >& xVals, time_t metarTime, time_t modelTime, int forecastLead ) 
{
   if( xVals.size() != coefficientNames.size() - 1 ) {
      POSTMSG( ERROR, "Error: Adding the wrong number of variables according "
               " to the number of names you have set for the coefficients" );
      return( FAILURE );
   }

   if (debug >= 10) {
     cout << "MultReg.addPoint.entry:" << endl;
     cout << "   coefficientNames.size: " << coefficientNames.size()
          << endl;   
     cout << "   xVals.size: " << xVals.size() << endl;   
     cout << "   x.size: " << x.size() << endl;   
     cout << "   numPts: " << numPts << endl;   
     cout << "   yVal: " << yVal << endl;   
     cout << "   xVals: ";
     for (int ii = 0; ii < xVals.size(); ii++) {
        cout << " " << xVals[ii];
     }
     cout << endl;
   }

   if( x.size() < 1 ) {
     
      //
      // Create a column of ones for the multiple
      // linear regression calculations
      //
      MultRegVar *newVar = new MultRegVar();
      newVar->addVal( 1.0 );
      x.push_back( newVar );
      
      for( int i = 0; i < (int) xVals.size(); i++ ) {
         newVar = new MultRegVar();
         newVar->addVal( xVals[i] );
         x.push_back( newVar );
      }
   }
   else {
      
      x[0]->addVal( 1.0 );

      for( int i = 0; i < (int) xVals.size(); i++ ) {
         x[i+1]->addVal( xVals[i] );
      }
   }

   y.addVal( yVal );

   metarUnixTime.push_back(metarTime);
   modelUnixTime.push_back(modelTime);
   forecastLeadSecs.push_back(forecastLead);

   numPts++;

   if (debug >= 10) {
     cout << "MultReg.addPoint.exit:" << endl;
     cout << "   x.size: " << x.size() << endl;   
     cout << "   numPts: " << numPts << endl;
     cout << "   metarTime.size: " << metarUnixTime.size() << endl;
   }

   return( SUCCESS );
}


int
MultReg::doRegression() 
{
   POSTMSG( INFO, "MultReg: doRegression: entry");
   if (debug >= 1) {
     cout << "MultReg: doRegression: entry" << endl;
     cout << "  numPts: " << numPts << endl;
     cout << "  num dimensions: " << coefficientNames.size() << endl;
     cout << "  coefficientNames: " << endl;
     for (int ii = 0; ii < coefficientNames.size(); ii++) {
       cout << "    \"" << coefficientNames[ii] << "\"" << endl;
     }
   }
   clearReg();

   //
   // Get rid of variables for which all observations are constant
   // with the exception of the first variable which should be set
   // to one 
   //   This handles two problems.  The first problem is that if
   //   a variable is set to zero for all observations, dgels gags.
   //   The second problem is that if a variable besides the first
   //   is set to a constant value, the matrix could be illconditioned,
   //   since the matrix could be reduced and the rank would no longer
   //   equal the number of variables.  e.g. If the jth column of matrix A,
   //   where j != 1, is such that a(i,j) = c for all i, where c is a real
   //   number, we could use column reduction to reduce the matrix, since
   //   -c * a(i,1) + a(i,j) = 0 for all i.
   //
   vector< MultRegVar* > nonZeroX;
   if ( x.size() > 0) {
     nonZeroX.push_back( x[0] );
   }
   
   for( int i = 1; i < (int) x.size(); i++ ) {
      if( x[i]->getStdDev() > 0 ) {
         nonZeroX.push_back( x[i] );
      }
   }

   //
   // Check data
   //
   if (debug >= 1) {
     cout << "MultReg.doRegression: MIN_N_PTS: " << MIN_N_PTS << endl;
     cout << "MultReg.doRegression: nonZeroX.size: " << nonZeroX.size()
          << endl;
     cout << "MultReg.doRegression: y.getSize: " << y.getSize() << endl;
   }
   if( nonZeroX.size() <= 1 || y.getSize() < MIN_N_PTS || 
       nonZeroX[0]->getSize() < MIN_N_PTS ) {
      POSTMSG( INFO, "Error: Not enough data for the regression" );
      POSTMSG( INFO, "MultReg: doRegression: ok exit A");
      return( SUCCESS );
   }

   //
   // Set the number of variables that will be used
   //   Note:  Do not include the constant term in this
   //   count
   //
   numVars = nonZeroX.size() - 1;

   //
   // Array dimensions
   //
   long n = nonZeroX.size();
   long m = nonZeroX[0]->getSize();

   //
   // Check array dimensions 
   //   Multiple linear regression routine 
   //   below does not handle the case when
   //   m < n.
   //
   if( m < n ) {
      POSTMSG( WARNING, "Error: Could not compute multiple linear"
               "regression - not enough data" );
      POSTMSG( WARNING, "m, n: %d, %d", m, n );
      missingCoeffs();
      POSTMSG( INFO, "MultReg: doRegression: error exit B");
      return( FAILURE );
   }

   //
   // Translate the y data into an array 
   //
   double *yMatrix = new double[m];
   
   double yDiv = 1.0;
   if( normalize ) {
      yDiv = y.getVecLen();
   }

   if (debug >= 20) {
     cout << "MultReg.doRegression: dump data:" << endl;
     cout << "  n: " << n << endl;
     cout << "  m: " << m << endl;
   }
   for( long j = 0; j < m; j++ ) {
      yMatrix[j] = y.getVal(j) / yDiv;
      if (debug >= 20) {
        cout << "yMatrix: j: " << j << " val: "
             << setw(20) << setprecision(15) << yMatrix[j] << endl;
      }
   } 
   
   //
   // Put the x matrix in column order
   //
   double *xMatrix = new double[m*n];

   for( long i = 0; i < n; i++ ) {

      double xiDiv = 1.0;
      if( normalize ) {
         xiDiv = nonZeroX[i]->getVecLen();
      }
      
      for( long j = 0; j < m; j++ ) {
         xMatrix[i*m + j] = nonZeroX[i]->getVal(j) / xiDiv;
      }
   }
   
   for( long i = 0; i < n; i++ ) {
      for( long j = 0; j < m; j++ ) {
         if (debug >= 20) {
           cout << "xMatrix: i: " << i << " j: " << j << " val: "
                << setw(20) << setprecision(15) << xMatrix[i*m+j] << endl;
         }
      }
   }

   //
   // Calculate variables to pass into least squares solver
   //
   long   nrhsVar     = NRHS;
   long   leadingDimX = m;
   long   leadingDimY = m;
   long   info        = -1;
   char   trans       = 'n';
   long   lwork       = m*n;
   
   //
   // Allocate workspace arrays 
   //
   double work[lwork];
   
   if (debug >= 5) {
     cout << "MultReg: doRegression: before dgels_:" << endl;
     cout << "  trans: " << trans << endl;
     cout << "  m: " << m << endl;
     cout << "  n: " << n << endl;
     cout << "  nrhsVar: " << nrhsVar << endl;
     cout << "  leadingDimX: " << leadingDimX << endl;
     cout << "  leadingDimY: " << leadingDimY << endl;
     cout << "  lwork: " << lwork << endl;
   }

   POSTMSG( INFO, "MultReg: doRegression: before dgels_: m: %ld  n: %ld  nrhsVar: %ld", m, n, nrhsVar);
   int status = dgels_( &trans, &m, &n, &nrhsVar, xMatrix, 
                        &leadingDimX, yMatrix, &leadingDimY, 
                        work, &lwork, &info );
   POSTMSG( INFO, "MultReg: doRegression: after dgels_: status: %d  info: %d",
     status, info);
   if (debug >= 1) {
     cout << "MultReg: doRegression: after dgels_: status: " << status
          << "  info: " << info << endl;
   }

   if( status != 0 ) {
      POSTMSG( INFO, "Error: Could not perform multiple linear regression" );
      POSTMSG( INFO, "MultReg: doRegression: error exit C");
      return( FAILURE );
   }

   if( info < 0 ) {
      POSTMSG( INFO, "Error: Illegal value" );
      missingCoeffs();
      POSTMSG( INFO, "MultReg: doRegression: error exit D");
      return( FAILURE );
   }
   if( info > 0 ) {
      POSTMSG( INFO, "Multiple linear regression did not converge" );
      
      for( int i = 0; i < n; i++ ) {
         coefficients.push_back( 0.0 );
      }
      
      POSTMSG( INFO, "MultReg: doRegression: ok exit E");
      return( SUCCESS );
   }
   
   //
   // Set up the coefficient vector 
   //   Put zero coefficients in for those variables that were
   //   equal to zero for every observation
   //
   int k = 0;
   for( int i = 0; i < (int) x.size(); i++ ) {

      if( i == 0 || x[i]->getStdDev() > 0 ) {
      
         double coefMult = 1.0;
         
         if( normalize ) {
            coefMult = y.getVecLen() / nonZeroX[k]->getVecLen();
            
         }
         
         coefficients.push_back( yMatrix[k] * coefMult );
         k++;
      }
      else {
         coefficients.push_back( 0.0 );
      }
      
   }

   //
   // Calculate the coefficient of determination
   //
   findDetermination();
  
   POSTMSG( INFO, "MultReg: doRegression: ok exit F");
   return( SUCCESS );
}

int
MultReg::writeVars( const string& outputFile )
{
   //
   // Open the file
   //
   FILE *fp = fopen( outputFile.c_str(), "wt" );
   if ( fp == NULL ) {
      POSTMSG( ERROR, "Error: Could not open file %s", outputFile.c_str() );
      return( FAILURE );
   }

   //
   // Write the data
   //
   if (_params->variable_output_format == Params::VARIABLE_OUT_CSV){
	 //header first
     fprintf(fp, "metarHumanTime, metarUnixTime, modelHumanTime, modelUnixTime, modelForecastLead, modelInitHumanTime, modelInitUnixTime, %s-metar",name.c_str());

	 /*	 
	 for( int i = 0; i < y.getSize(); i++ ) {
	   fprintf( fp, ", y%d",i);
	 }
	 */
	 for( int j = 0; j < (int) coefficientNames.size(); j++ ) {
	   fprintf( fp, ", %s-model",coefficientNames[j].c_str());
	 }
	 fprintf(fp, "\n");
   }	

   
   if (_params->variable_output_format == Params::VARIABLE_OUT_CSV){
	 for( int i = 0; i < y.getSize(); i++ ) {
	   fprintf( fp, "%s, %ld, %s, %ld, %ld, %s, %ld", utimstr(metarUnixTime[i]),metarUnixTime[i],
		    utimstr(modelUnixTime[i]),modelUnixTime[i],forecastLeadSecs[i],
		    utimstr(modelUnixTime[i]-forecastLeadSecs[i]),modelUnixTime[i]-forecastLeadSecs[i]);

	   fprintf( fp, ", %11.7g", y.getVal(i) );
	      
	   for( int j = 0; j < (int) x.size(); j++ ) {
		 fprintf( fp, ", %11.7g", x[j]->getVal(i) );
	   }
	   fprintf( fp, "\n" );
	 }	 
   }
   else{
	 for( int i = 0; i < y.getSize(); i++ ) {
	   fprintf( fp, "%11.7g\t", y.getVal(i) );
	      
	   for( int j = 0; j < (int) x.size(); j++ ) {
		 fprintf( fp, "%11.7g\t", x[j]->getVal(i) );
	   }
	   fprintf( fp, "\n" );
	 }
   }
   
   fclose( fp );

   return( SUCCESS );
}

void 
MultReg::findDetermination() 
{
   
   int    m     = x[0]->getSize();
   int    n     = x.size();
   double SSE   = 0.0;
   double ySum  = 0.0;
   double yySum = 0.0;
   double maxE  = missingVal;
   
   for( int j = 0; j < m; j++ ) {
      //
      // Calculate the estimation for y
      //
      double yHat = 0.0;
      for( int i = 0; i < n; i++ ) {
         yHat += coefficients[i] * (x[i]->getVal(j));
      }

      //
      // Calculate the error
      //
      double error = fabs( y.getVal(j) - yHat );

      // 
      // Find the maximum error
      //
      if( error > maxE )
         maxE = error;
      
      //
      // SSE
      //
      SSE += (error * error);
      
      //
      // For SSTO
      //
      ySum  += y.getVal(j);
      yySum += y.getVal(j) * y.getVal(j);
   }

   //
   // Set maxError
   //
   maxError = maxE;
   
   //
   // Calculate SSTO
   //
   double SSTO = yySum - ((ySum * ySum) / y.getSize());

   //
   // Calculate R^2
   //
   determination = 1.0 - (SSE / SSTO);

   //
   // Calculate Ra^2  - adjused coefficient of determination
   //
   double adjustment = ((double) (m - 1)) / ((double) (m - numVars));
   adjDetermination = 1.0 - ( adjustment * (SSE / SSTO));
   
}


void
MultReg::missingCoeffs() 
{
   for( int i = 0; i < (int) coefficientNames.size(); i++ ) {
      coefficients.push_back( missingVal );
   }
}







