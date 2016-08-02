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
// $Id: Regression.hh,v 1.29 2016/03/07 01:33:50 dixon Exp $
//
///////////////////////////////////////////////////////////////////
#ifndef _REGRESSION_INC_
#define _REGRESSION_INC_

#include <string>
#include <vector>

#include "Params.hh"
using namespace std;

class Regression {

 public:

  Regression( const string& baseName, double missVal, Params* params );
   virtual ~Regression();

   void              setInactiveVarNames( vector< string >& varNames );

   virtual void      clear() = 0;

   virtual int       doRegression() = 0;
 
   bool              hasInactiveCoeffs(){ return inactiveCoeffs; }

   vector< double >& getCoefficients(){ return coefficients; }
   vector< string >& getCoefficientNames(){ return coefficientNames; }
   vector< string >& getInactiveCoeffNames(){ return inactiveCoeffNames; }
   double            getDetermination(){ return determination; }
   double            getAdjDetermination(){ return adjDetermination; }
   const string&     getDetExt(){ return detExt; }
   const string&     getName(){ return name; }

   virtual int       writeInfo( const string& outputFile, time_t startTime,
                                time_t endTime );
   virtual int       writeCoeff( const string& outputFile );
   virtual int       writeVars( const string& outputFile ) = 0;
   virtual int       writeDetermination( const string& outputFile );
   virtual int       writeAdjDetermination( const string& outputFile );

 protected:

   bool             inactiveCoeffs;
   int              numPts;
   int              numVars;
   string           name;
   double           missingVal;
   vector< double > coefficients;
   vector< string > coefficientNames;
   vector< string > inactiveCoeffNames;
   double           determination;
   double           adjDetermination;
   string           detString;
   string           detExt;
   double           maxError;

   vector< time_t > modelUnixTime; 
   vector< time_t > metarUnixTime; 
   vector< int > forecastLeadSecs; 

   void             clearReg();

  Params			*_params;

 private:

};

#endif
