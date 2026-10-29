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
///////////////////////////////////////////////////////////////
//
//  Server and ingest data management
//
//  $Id: DataMgr.cc,v 1.70 2016/03/07 01:33:50 dixon Exp $
//
///////////////////////////////////////////////////////////////
#include <vector>
#include <values.h>       
#include <sys/stat.h>
#include <cstdlib>
#include <cassert>

#ifdef SUNOS5
#include <ieeefp.h>
#endif

#include <rapmath/umath.h>
#include <toolsa/DateTime.hh>
#include <toolsa/MsgLog.hh>

#include "DataMgr.hh"
#include "MetarPoint.hh"
#include "MM5Point.hh"
#include "MetarData.hh"
#include "MM5Data.hh"
#include "MultReg.hh"
#include "Regression.hh"
#include "SimpleReg.hh"
#include "MosCalibration.hh"
#include "Params.hh"
using namespace std;

//
// Constants
//
const int    DataMgr::TMP_STR_LEN          = 200;
const int    DataMgr::HOURS_TO_SEC         = 3600;

const int debug = 1;    // debug level

DataMgr::DataMgr()
{
   startTime = 0;
   endTime   = 0;
   writeInfo = false;
}

DataMgr::~DataMgr() 
{
   map< VariableType, Regression*, less<VariableType> >::iterator it;
   
   for( it = regressions.begin(); it != regressions.end(); it++ ) {
      delete( (*it).second );
   }
   regressions.erase( regressions.begin(), regressions.end() );

   forecastTimes.erase( forecastTimes.begin(), forecastTimes.end() );
   stationIds.erase( stationIds.begin(), stationIds.end() );

   delete mm5Data;
   delete metarData;
}

void
DataMgr::clear()
{
   map< VariableType, Regression*, less<VariableType> >::iterator it;
   
   for( it = regressions.begin(); it != regressions.end(); it++ ) {
      ((*it).second)->clear();
   }
}

int
DataMgr::init( Params& params, time_t sTime, time_t eTime ) 
{

   //
   // Forecast times
   //
   for( int i = 0; i < params.forecast_times_n; i++ ) {
      if( i > 0 && params._forecast_times[i] <= params._forecast_times[i-1] ) {
         POSTMSG( ERROR, "Error: Please list forecast times in increasing order" );
         return( FAILURE );
      }
      forecastTimes.push_back( params._forecast_times[i] * HOURS_TO_SEC );
   }
   
   //
   // Station ids
   //
   for( int i = 0; i < params.station_ids_n; i++ ) {
      stationIds.push_back( params._station_ids[i] );
   }

   //
   // Set data times
   //
   startTime = sTime;
   endTime   = eTime;

   //
   // Set thresholds
   //
   clwThresh     = (double) params.cloud_liquid_water_threshold;
   iceThresh     = (double) params.ice_content_threshold;
   ceilingThresh = (double) params.ceiling_threshold;
   visThresh     = (double) params.visibility_threshold;
   
   //
   // Time tolerance for data blending
   //
   timeTolerance = params.time_tolerance;

   //
   // Save the output url
   //
   outputUrl = params.output_url;

   //
   // Find the max valid time for the output data
   //
   maxValidTime = params.max_valid_age * 60;

   //
   // Set the output directories for the regression info if necessary
   //
   if( params.write_regression_info ) {
      writeInfo      = true;
      regOutputDir   = params.regression_output_dir;
      coeffOutputDir = params.coefficient_output_dir;
      corrOutputDir  = params.correlation_output_dir;
      varOutputDir   = params.variable_output_dir;
   }

   //
   // Set up the simple regressions
   //
   regressions[U]    = new SimpleReg( "u", GenPt::missingVal, "u", &params );
   regressions[V]    = new SimpleReg( "v", GenPt::missingVal, "v", &params );
   regressions[WSPD] = new SimpleReg( "wspd", GenPt::missingVal, "wspd", &params );
   regressions[TEMP] = new SimpleReg( "temp", GenPt::missingVal, "temp", &params );
   regressions[PRS]  = new SimpleReg( "prs", GenPt::missingVal, "prs", &params );
   regressions[RH]   = new SimpleReg( "relHum", GenPt::missingVal, "relHum", &params );

   //
   // Set up the variable name vectors for the multiple 
   // linear regressions 
   //   NOTE:  Make sure that the order of the variable names
   //   matches the order in which they are sent into the 
   //   multiple linear regression in setRegressions!
   //
   vector< string > ceilingVars;
   ceilingVars.push_back( "tempDiff_900_700mb" );
   ceilingVars.push_back( "wspd" );
   ceilingVars.push_back( "avgW" );
   ceilingVars.push_back( "prs" );
//###delceil
   ceilingVars.push_back( "clwht" );
   
   vector< string > visVars;
   visVars.push_back( "tempDiff_1000_850mb" );
   visVars.push_back( "tempDiff_900_700mb" );
   visVars.push_back( "wspd" );
   visVars.push_back( "avgW" );
   visVars.push_back( "rainRate" );
   visVars.push_back( "rh" );
   visVars.push_back( "prs" );
   visVars.push_back( "li" );

   //
   // Set up the multiple linear regressions
   //
   regressions[CEIL] = new MultReg( "ceiling", GenPt::missingVal, 
                                    ceilingVars,&params,
                                     params.normalize ? true : false );
   regressions[VIS]  = new MultReg( "vis", GenPt::missingVal,
                                    visVars, &params,
                                    params.normalize ? true : false );

   //
   // Set up inactive lists
   //
   //   NOTE:  Inactive lists are used to keep place holders 
   //   in output database for coefficients that we are no
   //   longer using, but may use again.  This prevents the
   //   database from changing too much and allows us to make
   //   changes to the variables that are used in these
   //   regressions without changing downstream code.
   //
   //   NOTE:  If these are added back in make sure the order
   //   in which they are added to the active list above matches
   //   the order in which the corresponding values are passed
   //   into the multiple linear regression object in the setMultiple
   //   method - called in setRegressions - below.
   //
   //
   vector< string > inactiveCeilingVars;
   inactiveCeilingVars.push_back( "tempDiff_1000_850mb" );
   inactiveCeilingVars.push_back( "rainRate" );
   inactiveCeilingVars.push_back( "rh" );
   inactiveCeilingVars.push_back( "ice" );

   regressions[CEIL]->setInactiveVarNames( inactiveCeilingVars );

   //
   // Clean up
   //
   ceilingVars.erase( ceilingVars.begin(), ceilingVars.end() );
   visVars.erase( visVars.begin(), visVars.end() );

   //
   // Initialize the servers
   //
   metarServer.init( params.metar_url, startTime, endTime );
   mm5Server.init( params.mm5_url, startTime, endTime, true );
   rrServer.init( params.mm5_url, 0, 0, true );

   //
   // Instantiate the data objects
   //
   metarData = new MetarData( metarServer );
   mm5Data   = new MM5Data( mm5Server, rrServer,
//###delceil
                            clwThresh, iceThresh,
                            forecastTimes );

   return( SUCCESS );
}
   
int
DataMgr::processData()
{
   POSTMSG( INFO, "processData: entry");
  
   for( int i = 0; i < (int) stationIds.size(); i++ ) {
     POSTMSG( INFO, "processData: start stationId %s", stationIds[i]);

      //
      // Clear out metar data list
      //
      metarData->clear();
     POSTMSG( INFO, "processData: testa");

      //
      // Make new list of metar data
      //
      PMU_auto_register( "Creating list of metar data" );
      int nptsMetar = metarData->createList( stationIds[i] );
      POSTMSG( INFO, "processData: testc: nptsMetar: %d", nptsMetar);

      if( nptsMetar < 1 )
         continue;
      
      for( int j = 0; j < (int) forecastTimes.size(); j++ ) {

         //
         // Clear out old list of mm5 data
         //
     POSTMSG( INFO, "processData: testb: j: %d", j);
         mm5Data->clear();
         
         int nptsMM5  = mm5Data->createList( stationIds[i],
                                             forecastTimes[j] );
     POSTMSG( INFO, "processData: testc: nptsMM5: %d", nptsMM5);
         
         if( nptsMM5 < 1 )
            continue;

         //
         // Clear out regression objects
         //
         clear();
     POSTMSG( INFO, "processData: testg");

         //
         // Blend the data
         //
         PMU_auto_register( "Blending data" );
         
         if( blend( mm5Data->getData(), metarData->getData() ) != SUCCESS ) {
            POSTMSG( INFO, "Could not blend the data for %s, lead time = %d",
                     stationIds[i], forecastTimes[j] );
            continue;
         }
     POSTMSG( INFO, "processData: testh");

         //
         // Perform regressions
         //
         PMU_auto_register( "Performing regressions" );
         
         map< VariableType, Regression*, less<VariableType> >::iterator it;
         for( it = regressions.begin(); it != regressions.end(); it++ ) {
        if (debug >= 1) {
          cout << "DataMgr.processData: before regression for: "
               << ((*it).second)->getName() << endl;
        }
            if( ((*it).second)->doRegression() != SUCCESS ) {
               POSTMSG( INFO, "Error: Regression not performed for %s",
                        ((*it).second)->getName().c_str() );
            }
        if (debug >= 1) {
          cout << "DataMgr.processData: after regression for: "
               << ((*it).second)->getName() << endl;
        }
         }
     POSTMSG( INFO, "processData: after all regressions");
      
         //
         // Write the data
         //
         PMU_auto_register( "Writing the data" );
         
         int status = writeData( stationIds[i], 
                                 metarData->getLat(), metarData->getLon(),
                                 endTime, forecastTimes[j] );
         if( status != SUCCESS ) {
            DateTime when( endTime );
            POSTMSG( INFO, "Error: Data not written for station %s, lead time = "
                     "%d at %s", stationIds[i], forecastTimes[j],
                     when.dtime() );
            continue;
         }
         

         //
         // Write information to text files if the user asked for this
         //
         if( writeInfo ) {
            PMU_auto_register( "Writing text data" );
            writeTextData( forecastTimes[j], stationIds[i] );
         }

      }
     POSTMSG( INFO, "processData: end stationId %s", stationIds[i]);
   }

   POSTMSG( INFO, "processData: exit");
   return( SUCCESS );
}

int
DataMgr::blend( map< time_t, MM5Point*, less<time_t> >& modelVariables,
                map< time_t, MetarPoint*, less<time_t> >& metarVariables ) 
{   
   //
   // If there aren't enough data points we are done.
   //
   if( modelVariables.size() < 2 || metarVariables.size() < 2 ) {
      POSTMSG( DEBUG, "One or more of the variable lists are empty" );
      return( SUCCESS );
   }

   //
   // Initialize the iterators
   //
   map< time_t, MM5Point*, less<time_t> >::iterator modelIt = 
      modelVariables.begin();

   map< time_t, MetarPoint*, less<time_t> >::iterator metarIt = 
      metarVariables.begin();
      
   //
   // Initialize the time keepers
   //
   time_t modelTime     = modelIt->first;
   time_t prevMetarTime = metarIt->first;

   metarIt++;
   
   time_t currentMetarTime = metarIt->first;

   //
   // Look through the model times
   //
   while( modelIt != modelVariables.end() ) {
      
      //
      // Look through the metar times to find a metar time that is
      // greater than or equal to the current model time.  Then
      // the previous metar time will be the time less than the
      // current model time, while the current metar time will be
      // the time greater than or equal to the current model time.
      //
      while( currentMetarTime < modelTime ) {
         prevMetarTime = currentMetarTime;

         metarIt++;
         if( metarIt == metarVariables.end() ) {
            currentMetarTime = 0;
            break;
         }
         
         currentMetarTime = metarIt->first;
      }

      //
      // We need to decide whether the previous metar time or the
      // current metar time is the time to use.  Use the one that
      // is closest in absolute value to the current model time.
      // Then set the mm5 point and the metar time appropriately.
      //
      MM5Point   *mm5Point    = NULL;
      MetarPoint *metarPoint  = NULL;
      int         currentDiff = abs( currentMetarTime - modelTime );
      int         prevDiff    = abs( prevMetarTime - modelTime );
      
      if( currentMetarTime == 0 || currentDiff > prevDiff ) {
         if( prevDiff < timeTolerance ) {

            mm5Point = modelVariables[modelTime];
            metarPoint = metarVariables[prevMetarTime];

         }
      }
      else {
         if( currentDiff < timeTolerance ) {

            mm5Point = modelVariables[modelTime];
            metarPoint = metarVariables[currentMetarTime];

         }
      }

      //
      // Add data points in order to set up the regressions from the 
      // given model and metar points
      //
      if( setRegressions( mm5Point, metarPoint ) != SUCCESS ) {
         POSTMSG( ERROR, "Error: setRegressions failed");
         return( FAILURE );
      }
      
      //
      // Increment the model iterator, i.e. start over with the
      // the next model time.  We know that the metar iterator
      // should pick up where it left off however.
      //
      modelIt++;
      modelTime = modelIt->first;
      
   }

   return( SUCCESS );
   
}

int
DataMgr::setRegressions( MM5Point* mm5Point, MetarPoint* metarPoint ) 
{

   if (mm5Point == NULL)
     cout << "setRegressions: mm5Point is NULL" << endl;
   if (metarPoint == NULL)
     cout << "setRegressions: metarPoint is NULL" << endl;

   if( mm5Point == NULL || metarPoint == NULL )
      return( SUCCESS );

   if (debug >= 10) {
     cout << "setRegressions:  mm5Point:      metarPoint:" << endl;
     cout << "           lon: " << mm5Point->getLon()
          << "    " << metarPoint->getLon() << endl;
     cout << "           lat: " << mm5Point->getLat()
          << "    " << metarPoint->getLat() << endl;
     cout << "          time: " << mm5Point->getTime()
          << "    " << metarPoint->getTime() << endl;
   }

   //
   // Simple regressions
   //
   setSimple( mm5Point->getU(), metarPoint->getU(), 
              (SimpleReg&) *(regressions[U]),
	      metarPoint->getTime(), mm5Point->getTime(), mm5Point->getForecastLead());
   setSimple( mm5Point->getV(), metarPoint->getV(), 
              (SimpleReg&) *(regressions[V]),
	      metarPoint->getTime(), mm5Point->getTime(), mm5Point->getForecastLead() );
   setSimple( mm5Point->getWspd(), metarPoint->getWspd(),
              (SimpleReg&) *(regressions[WSPD]),
	      metarPoint->getTime(), mm5Point->getTime(), mm5Point->getForecastLead() );
   setSimple( mm5Point->getTemp(), metarPoint->getTemp(), 
              (SimpleReg&) *(regressions[TEMP]),
	      metarPoint->getTime(), mm5Point->getTime(), mm5Point->getForecastLead());
   setSimple( mm5Point->getPrs(), metarPoint->getPrs(), 
              (SimpleReg&) *(regressions[PRS]),
	      metarPoint->getTime(), mm5Point->getTime(), mm5Point->getForecastLead() );
   setSimple( mm5Point->getRH(), metarPoint->getRH(), 
              (SimpleReg&) *(regressions[RH]),
	      metarPoint->getTime(), mm5Point->getTime(), mm5Point->getForecastLead() );

   //
   // Multiple Linear regressions
   //    
   //    Ceiling
   //      If the ceiling value from the metar exceeds the
   //      user defined threshold, do not use this point
   //
   double ceilingVal = metarPoint->getCeiling();
   if (debug >= 10) {
     cout << "setRegressions: ceilingThresh: " << ceilingThresh
          << "  ceilingVal: " << ceilingVal << endl;
   }


   vector< double > ceilRegressors;
   
   ceilRegressors.push_back( mm5Point->getTempDiff_900_700mb() );
   ceilRegressors.push_back( mm5Point->getWspd() );
   ceilRegressors.push_back( mm5Point->getAvgW() );
   ceilRegressors.push_back( mm5Point->getPrs() );
//###delceil
   ceilRegressors.push_back( mm5Point->getClwht() );

    //
    // The following are not currently used but were at one time
    //    tempDiff_1000_850mb
    //    rainRate
    //    rh
    //    ice content
    //
    // If these are added back in make sure the order in this
    // vector matches the order in which the names are added to 
    // the *active* list in the init method!
    //
   
    if (debug >= 10)
      cout << "setRegressions: before call to CEIL setMultiple" << endl;

   // If ceiling is greater than the ceilingThresh, use ceilingThresh in regression
   if( ceilingVal <= ceilingThresh ) {
      if( setMultiple( ceilRegressors, metarPoint->getCeiling(), 
                       (MultReg&) *(regressions[CEIL]),
			  metarPoint->getTime(), mm5Point->getTime(), mm5Point->getForecastLead() ) != SUCCESS) {
         POSTMSG( ERROR, "Error: setMultiple failed");
         return( FAILURE );
      }
   } else {
     if( setMultiple( ceilRegressors, ceilingThresh, 
                       (MultReg&) *(regressions[CEIL]),
			  metarPoint->getTime(), mm5Point->getTime(), mm5Point->getForecastLead() ) != SUCCESS) {
         POSTMSG( ERROR, "Error: setMultiple failed");
         return( FAILURE );
     }
   }
   if (debug >= 10)
     cout << "setRegressions: after call to CEIL setMultiple" << endl;
  
   

   //
   //    Visiblity
   //
   double visVal = metarPoint->getVis();
   
 
      
   vector< double > visRegressors;
   
   visRegressors.push_back( mm5Point->getTempDiff_1000_850mb() );
   visRegressors.push_back( mm5Point->getTempDiff_900_700mb() );
   visRegressors.push_back( mm5Point->getWspd() );
   visRegressors.push_back( mm5Point->getAvgW() );
   visRegressors.push_back( mm5Point->getRainRate() );
   visRegressors.push_back( mm5Point->getRH() );
   visRegressors.push_back( mm5Point->getPrs() );
   visRegressors.push_back( mm5Point->getLI() );
   
   if (debug >= 10)
     cout << "setRegressions: before call to VIS setMultiple" << endl;
   if( visVal <= visThresh ) {
     if( setMultiple( visRegressors, metarPoint->getVis(), 
                       (MultReg&) *(regressions[VIS]),
			  metarPoint->getTime(), mm5Point->getTime(), mm5Point->getForecastLead() ) != SUCCESS ) {
       POSTMSG( ERROR, "Error: setMultiple failed");
       return( FAILURE );
     }
   } else {
     if( setMultiple( visRegressors, visThresh, 
                       (MultReg&) *(regressions[VIS]),
			  metarPoint->getTime(), mm5Point->getTime(), mm5Point->getForecastLead() ) != SUCCESS ) {
       POSTMSG( ERROR, "Error: setMultiple failed");
       return( FAILURE );
     }
   }
   if (debug >= 10)
     cout << "setRegressions: after call to VIS setMultiple" << endl;
   
   
   return( SUCCESS );
}


void
DataMgr::setSimple( double mm5Val, double metarVal, SimpleReg& reg, time_t metarTime, time_t modelTime, int modelLeadSecs ) 
{
   if( mm5Val != MM5Point::MISSING_VAL &&
       metarVal != MetarPoint::MISSING_VAL ) {
     reg.addPoint( metarVal, mm5Val,metarTime, modelTime, modelLeadSecs );
   }
}


int
DataMgr::setMultiple( vector< double >& mm5Vals, double metarVal, 
                      MultReg& reg, time_t metarTime, time_t modelTime, int modelLeadSecs ) 
{
   if (debug >= 10) {
     cout << "DataMgr.setMultiple: mm5Vals (size " << mm5Vals.size() << "): ";
     for( int i = 0; i < (int) mm5Vals.size(); i++ ) {
       cout << "  " << mm5Vals[i];
     }
     cout << endl;
     cout << "DataMgr.setMultiple: metarVal: " << metarVal << endl;
   }

   for( int i = 0; i < (int) mm5Vals.size(); i++ ) {
      if( mm5Vals[i] == MM5Point::MISSING_VAL ) {
         if (debug >= 1) {
           cout << "DataMgr.setMultiple: mm5Vals ele is missing: " << i
                << endl;
         }
         return( SUCCESS );
      }
   }
   if( metarVal == MetarPoint::MISSING_VAL ) {
      if (debug >= 1)
        cout << "DataMgr.setMultiple: metarVal is missing" << endl;
      return( SUCCESS );
   }

   if (debug >= 10)
     cout << "DataMgr.setMultiple: before call to addPoint" << endl;
   if( reg.addPoint( metarVal, mm5Vals, metarTime, modelTime, modelLeadSecs ) != SUCCESS ) {
      POSTMSG( INFO, "Error: Could not set the variables for the multiple "
               "regression" );
      return( FAILURE );
   }
   if (debug >= 10)
     cout << "DataMgr.setMultiple: after call to addPoint" << endl;
   
   return( SUCCESS );

}


int
DataMgr::writeData( const string& stationId, double lat, double lon,
                    time_t dataTime, int leadTime ) 
{
   string name;

   //
   // Clear the output point
   //
   outputPt.clear();
   
   //
   // Set the general information
   //
   outputPt.setName( stationId );
   outputPt.setTime( dataTime );
   outputPt.setLat( lat );
   outputPt.setLon( lon );
   outputPt.setNLevels( 1 );

   //
   // Add field information and values for each field
   //   Note that a "field" in this case refers to each coefficient
   //   for each regression that was performed
   //
   map< VariableType, Regression*, less<VariableType> >::iterator it;
   
   for( it = regressions.begin(); it != regressions.end(); it++ ) {

      Regression* currentRegression = (*it).second;
      
      vector< double >& coefficients = currentRegression->getCoefficients();
      vector< string >& coefficientNames = 
         currentRegression->getCoefficientNames();


      if (debug >= 1) {
        cout << endl;
        cout << "writeData: coefficientNames.size: "
             << coefficientNames.size() << endl;
        cout << "writeData: coefficients.size: " << coefficients.size()
             << endl;
        for( int ii = 0;
          ii < max( coefficientNames.size(), coefficients.size());
          ii++ )
        {
          cout << "    ii: " << ii << "  Name: ";
          if (ii < coefficientNames.size()) cout << coefficientNames[ii];
          else cout << " --- ";
          cout << "    ";
          if (ii < coefficients.size()) cout << coefficients[ii];
          else cout << " --- ";
          cout << endl;
        }
        cout << endl;
      }

      //
      // Do we have the number of coefficients that we were
      // expecting?  Don't write anything if we don't.
      //
      if( coefficients.size() != coefficientNames.size() ) {
         POSTMSG( INFO, "Error: wrong number of coefficients.  Expected: %d  found: %d", coefficientNames.size(), coefficients.size() );
         return( FAILURE );
      }
      
      for( int i = 0; i < (int) coefficients.size(); i++ ) {

         //
         // Check for bad coefficients
         //
         if( !finite( coefficients[i] ) ) {
            POSTMSG( INFO, "Error: Some coefficients were set to inf or nan" );
            return( FAILURE );
         }
         
         name = currentRegression->getName() + "_VAR_" + coefficientNames[i];
         outputPt.addFieldInfo( name, "none" );
         outputPt.addVal( coefficients[i] );
      }

      //
      // Handle coefficients that we used to use - for backward
      // compatability and so that we can put these back in if 
      // we so choose without changing downstream code or the
      // database
      //
      if( currentRegression->hasInactiveCoeffs() ) {
         
         vector< string >& inactiveCoeffs = 
            currentRegression->getInactiveCoeffNames();
        
         //
         // Put in zeros for the inactive coefficient values
         // 
         for( int i = 0; i < (int) inactiveCoeffs.size(); i++ ) {
            name = currentRegression->getName() + "_VAR_" +
               inactiveCoeffs[i];
            outputPt.addFieldInfo( name, "none" );
            outputPt.addVal( 0.0 );
         }
      }
      
      name = currentRegression->getName() + currentRegression->getDetExt();
      outputPt.addFieldInfo( name, "none" );
      outputPt.addVal( currentRegression->getDetermination() );

   }

   //
   // Add threshold information
   //
//###delceil
   outputPt.addFieldInfo( "clwThresh", "kg/kg" );
   outputPt.addVal( clwThresh );
   outputPt.addFieldInfo( "iceThresh", "kg/kg" );
   outputPt.addVal( iceThresh );

   //
   // Assemble the point into the buffer
   //
   if( outputPt.assemble() != 0 ) {
      POSTMSG( INFO, "Error: Could not write the data point to spdb" );
      return( FAILURE );
   }

   //
   // Write the point to the database
   //
   int status = spdbMgr.put( outputUrl, SPDB_GENERIC_POINT_ID,
                             SPDB_GENERIC_POINT_LABEL,
                             Spdb::hash4CharsToInt32( stationId.c_str() ),
                             dataTime, dataTime + maxValidTime,
                             outputPt.getBufLen(), outputPt.getBufPtr(),
                             leadTime );
   if( status != 0 ) {
      POSTMSG( INFO, "Error: Could not write the data point to spdb" );
      return( FAILURE );
   }

   //
   // Tell the user what we are doing
   //
   DateTime when( dataTime );
   POSTMSG( INFO, "Wrote data for station %s, with a lead time"
            "of %d at %s", stationId.c_str(), leadTime, when.dtime() );
   
   return( SUCCESS );
   
}

void
DataMgr::writeTextData( int forecastTime, char* stationId ) 
{

   assert( stationId );

   //
   // Set up the directories for the regression info,  
   // correlation data and coefficient data
   // 
   char dirStr[TMP_STR_LEN];

   sprintf( dirStr, "%s/%s/f_%.6d", regOutputDir.c_str(), stationId, 
            forecastTime );
   regOutputPath.setDirectory( dirStr );
   regOutputPath.makeDirRecurse();

   sprintf( dirStr, "%s/%s/f_%.6d", coeffOutputDir.c_str(), stationId, 
            forecastTime );

   coeffOutputPath.setDirectory( dirStr );
   coeffOutputPath.makeDirRecurse();

   sprintf( dirStr, "%s/%s/f_%.6d", corrOutputDir.c_str(), stationId, 
            forecastTime );

   corrOutputPath.setDirectory( dirStr );
   corrOutputPath.makeDirRecurse();
   
   //
   // Set the directory for the text files which will contain
   // the data associated with each variable
   //
   setOutputPath( varOutputDir, stationId, forecastTime, varOutputPath );

   //
   // Write the files
   //
   map< VariableType, Regression*, less<VariableType> >::iterator it;
   for( it = regressions.begin(); it != regressions.end(); it++ ) {

      //
      // Set up the file name
      //
      string fileStr = ((*it).second)->getName() + ".txt";
               
      //
      // Set the complete path - with file name
      //
      regOutputPath.setFile( fileStr );
      coeffOutputPath.setFile( fileStr );
      corrOutputPath.setFile( fileStr );
      varOutputPath.setFile( fileStr );

      //
      // Write the information
      //
      ((*it).second)->writeVars( varOutputPath.getPath() );
      ((*it).second)->writeInfo( regOutputPath.getPath(), startTime, 
                                 endTime );
      ((*it).second)->writeDetermination( corrOutputPath.getPath() );
      ((*it).second)->writeCoeff( coeffOutputPath.getPath() );
   }
   

   //
   // Clear out the paths
   //
   regOutputPath.clear();
   coeffOutputPath.clear();
   corrOutputPath.clear();
   varOutputPath.clear();

   //
   // Tell the user what we are doing
   //
   DateTime start( startTime );
   DateTime end( endTime );
   
   POSTMSG( INFO, "Wrote text data for station %s, at a forecast time"
            "of %d between %s and %s", stationId, forecastTime,
            start.dtime(), end.dtime() );
}


void
DataMgr::setOutputPath( string& outputDir, char* stationId, 
                        int forecastTime, Path& outputPath ) 
{
   //
   // Set up directory
   //
   DateTime start( startTime );
   DateTime end( endTime );

   char dirStr[TMP_STR_LEN];

   sprintf( dirStr, "%s/%s/f_%.6d/%.2d%.2d%.2d%.2d%.2d%.2d_%.2d%.2d%.2d"
            "%.2d%.2d%.2d", outputDir.c_str(), stationId, forecastTime,
            start.getYear(), start.getMonth(), start.getDay(), 
            start.getHour(), start.getMin(), start.getSec(), 
            end.getYear(), end.getMonth(), end.getDay(), 
            end.getHour(), end.getMin(), end.getSec() );

   outputPath.setDirectory( dirStr );
   outputPath.makeDirRecurse();
}



   
   
