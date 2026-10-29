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
/////////////////////////////////////////////////////
//  Class used to apply the regression from 
//  MosCalibration to the current combo point
//  from MM5
//
//  $id: $
/////////////////////////////////////////////////////
#ifndef _APPLY_REGRESSION_
#define _APPLY_REGRESSION_

#include <string>
#include <rapformats/station_reports.h>
#include <Spdb/DsSpdb.hh>
#include <rapformats/GenPt.hh>
using namespace std;

//
// Forward class declarations
//
class ComboPt;

class ApplyRegression 
{
 public:
   
   ApplyRegression(){};
   ~ApplyRegression(){};

   void init ( double clrVis, double clrCeil, const string& regUrl,
               const string& modelUrl, int timeWindow, int dLeadTime,
               bool ckFcast, bool dbg = false, bool rtime = false, 
			   const string& dbgDir = "");

   int process( ComboPt& comboPt, station_report_t& S, int leadTime, 
                int stationId );
 
 private:

   //
   // Print out debugging information?
   //
   bool debug;
   
   //
   // Mode of operation
   //
   bool realtime;

   //
   // Check forecast values?
   //
   bool checkForecast;
   
   //
   // Values to be used in the case where ceiling and visibility 
   // are clear
   //
   double clearVis;
   double clearCeiling;

   //
   // Get the regression information from spdb
   //
   int    timeMargin;
   string regressionUrl;
   string debugDir;
   DsSpdb regressionSpdbMgr;
   int    getRegData( int dataType, int dataType2, time_t dataTime );

  string debugHeadersWritten;  //comma seperated list of files that have had headers written.
   //
   // Look at the model data to get information to
   // calculate the rain rate
   //
   int    deltaLeadTime;
   string rainRateUrl;
   DsSpdb rainRateSpdbMgr;

   //
   // Find these values
   //
   double rhVal;
   double uVal;
   double vVal;
   double tempVal;
   double prsVal;
   double wspdVal;
   double ceilingVal;
   double visVal;
   void   clear();

   //
   // Intermediate values
   //
   bool   intermediateValid;
   double tempDiff_1000_850mb;
   double tempDiff_900_700mb;
   double avgW;
   double rainRate;
   double liftedIndex;
   void   calcIntermediate( const GenPt& multiLevelPt, 
                            const GenPt& singleLevelPt,
                            time_t dataTime, int leadTime,
                            int stationId);
   void   clearIntermediate();

   //
   // Get the value of the field at the surface out of a 2 dimensional
   // field
   //
   double get2DVal( const string& fieldName, const GenPt& multiLevelPt );

   //
   // Apply the regression equation for a simple linear regression
   //
   double applySimple( double modelVal, const string& regName, 
                       const GenPt& regPt, time_t leadTime );

   //
   // Calculate the ceiling value based on the regression equation
   //
   double calcCeiling( const GenPt& regressionPt, const GenPt& multiLevelPt,
                       double modelRH, double modelWspd, double modelPrs, time_t leadTime );

   //
   // Calculate the visibility value based on the regression equation
   //
   double calcVis( const GenPt& regressionPt, double modelRH, 
                   double modelWspd, double modelPrs, time_t leadTime );

   //
   // Convert pressure to altitude
   //
   double mbToKm( double pressure, double psfc );

   //
   // Find the level number for which the value of the given field
   // is closest to the value passed in as an argument
   //
   int    findLevelClosestToVal( const string& fieldName, 
                                 const GenPt& multiLevelPt,
                                 double value );

   //
   // Calculate the rain rate
   //
   double findRainRate( const GenPt& singleLevelPt, time_t dataTime,
                        int leadTime, int stationId );

   //
   // Used for calculation of lifted index
   //   
   double reverseThetaE( double thte, double p, double tguess );
   double thetaE( double pressure, double temperature, double dewPoint );
   double prTlcl( double t, double td );
   double prMixr( double td, double p );
   double prVapr( double t );
   double findLifted( double *pres, double *temp, double *dp, 
                      int nl, double pli );

   //
   // Fill out the station report
   //
   void   fillStationReport( station_report_t& report, time_t dataTime, 
                             double lat, double lon, int stationId ) ;
   
};


#endif
