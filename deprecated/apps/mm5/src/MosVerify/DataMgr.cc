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
//  Data management
//
//  $Id: DataMgr.cc,v 1.14 2016/03/07 01:33:51 dixon Exp $
//
///////////////////////////////////////////////////////////////
#include <vector>
#include <values.h>       
#include <sys/stat.h>
#include <unistd.h>

#include <rapmath/umath.h>
#include <toolsa/DateTime.hh>
#include <toolsa/MsgLog.hh>

#include "DataMgr.hh"
#include "DataGroup.hh"
#include "MetarData.hh"
#include "MetarPoint.hh"
#include "MM5Data.hh"
#include "MM5Point.hh"
#include "MosVerify.hh"
#include "Params.hh"
using namespace std;

//
// Constants
//
const int    DataMgr::TMP_STR_LEN  = 200;
const int    DataMgr::HOURS_TO_SEC = 3600;

const string DataMgr::WDIR_NAME    = "wdir";
const string DataMgr::WSPD_NAME    = "wspd";
const string DataMgr::TEMP_NAME    = "temp";
const string DataMgr::PRS_NAME     = "prs";
const string DataMgr::RH_NAME      = "rh";
const string DataMgr::CEIL_NAME    = "ceiling";
const string DataMgr::VIS_NAME     = "visibility";

DataMgr::DataMgr()
{
   startTime        = 0;
   endTime          = 0;
   timeTolerance    = 0;
}

DataMgr::~DataMgr() 
{
   map< VariableType, DataGroup*, less<VariableType> >::iterator it;
   
   for( it = dataGroups.begin(); it != dataGroups.end(); it++ ) {
      delete( (*it).second );
   }
   dataGroups.erase( dataGroups.begin(), dataGroups.end() );

   stationIds.erase( stationIds.begin(), stationIds.end() );

   delete truthData;
   delete forecastData;
   delete modelData;
}

void
DataMgr::clear()
{
   map< VariableType, DataGroup*, less<VariableType> >::iterator it;
   
   for( it = dataGroups.begin(); it != dataGroups.end(); it++ ) {
      (*it).second->clear();
   }
}

int
DataMgr::init( Params& params, time_t sTime, time_t eTime ) 
{
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
   // Set the forecast time information
   //
   for( int i = 0; i < params.forecast_times_n; i++ ) {
      forecastTimes.push_back( params._forecast_times[i] * HOURS_TO_SEC );
   }

   //
   // Set the output directory
   //
   outputDir = params.variable_output_dir;

   //
   // Initialize the servers
   //
   truthServer.init( params.truth_url, startTime, endTime );
   forecastServer.init( params.forecast_url, startTime, endTime, true );
   modelServer.init( params.model_url, startTime, endTime, true );


   //
   // Instantiate the data objects
   //
   truthData    = new MetarData( truthServer );
   forecastData = new MetarData( forecastServer );
   modelData    = new MM5Data( modelServer );

   //
   // Set the time tolerance for data blending
   //
   timeTolerance = params.time_tolerance;

   //
   // Set up the data groups
   //
   dataGroups[WSPD] = new DataGroup( WSPD_NAME );
   dataGroups[WDIR] = new DataGroup( WDIR_NAME );
   dataGroups[TEMP] = new DataGroup( TEMP_NAME );
   dataGroups[PRS]  = new DataGroup( PRS_NAME );
   dataGroups[RH]   = new DataGroup( RH_NAME );
   dataGroups[CEIL] = new DataGroup( CEIL_NAME );
   dataGroups[VIS]  = new DataGroup( VIS_NAME );
   
   return( SUCCESS );
}
   
void
DataMgr::processData()
{
  
   for( int i = 0; i < (int) stationIds.size(); i++ ) {

      //
      // Clear out the truth data list
      //
      truthData->clear();

      //
      // Create a list of the truth data
      //
      int nptsTruth = truthData->createList( stationIds[i] );

      //
      // If there isn't any truth data for this time, we're done
      //
      if( nptsTruth < 1 )
         continue;

      //
      // Process the forecast and model data 
      //
      for( int j = 0; j < (int) forecastTimes.size(); j++ ) {
         compareForecast( stationIds[i], forecastTimes[j] );
      }
      
   }
   
}

void
DataMgr::compareForecast( char* stationId, int leadTime ) 
{
   //
   // Clear out the forecast data list
   //
   forecastData->clear();
   modelData->clear();
   
   //
   // Create a list of the forecast data
   //
   int nptsForecast = forecastData->createList( stationId, leadTime );
   int nptsModel    = modelData->createList( stationId, leadTime );
	 
   //
   // If we don't have any points in the forecast data, we're done
   //
   if( nptsForecast < 1 || nptsModel < 1 ) 
      return;

   //
   // Clear out data groups
   //
   clear();
   
   //
   // Blend the data
   //
   PMU_auto_register( "Blending data" );
         
   if( blend( truthData->getData(), 
              forecastData->getData(),
              modelData->getData() ) != SUCCESS ) {

      POSTMSG( INFO, "Could not blend the data for %s with lead "
                     "time = %d", stationId, leadTime );
      return;
   }
   
   //
   // Write the data
   //
   PMU_auto_register( "Writing the data" );
   writeData( stationId, leadTime );

}

int
DataMgr::blend( map< time_t, MetarPoint*, less<time_t> >& truthVariables,
                map< time_t, MetarPoint*, less<time_t> >& forecastVariables,
                map< time_t, MM5Point*, less<time_t> >& modelVariables ) 
{   

   //
   // If there aren't enough points we are done
   //
   if( truthVariables.size() < 2 || forecastVariables.size() < 2 ) {
      POSTMSG( DEBUG, "One or more of the variable lists are empty" );
      return( SUCCESS );
   }

   //
   // Initialize the iterators
   //
   map< time_t, MetarPoint*, less<time_t> >::iterator truthIt = 
      truthVariables.begin();

   map< time_t, MetarPoint*, less<time_t> >::iterator forecastIt = 
      forecastVariables.begin();

   map< time_t, MM5Point*, less<time_t> >::iterator modelIt = 
      modelVariables.begin();
   
   //
   // Initialize the time keepers
   //
   time_t prevTruthTime  = truthIt->first;
   time_t prevModelTime  = modelIt->first;
   time_t forecastTime   = forecastIt->first;

   truthIt++;
   modelIt++;
   
   time_t currentTruthTime = truthIt->first;
   time_t currentModelTime = modelIt->first;

   //
   // Look through the model times
   //
   while( forecastIt != forecastVariables.end() ) {
      
      //
      // Look through the truth times to find a truth time that is
      // greater than or equal to the current forecast time.  Then
      // the previous truth time will be the time less than the
      // current forecast time, while the current truth time will be
      // the time greater than or equal to the current forecast time.
      //
      while( currentTruthTime < forecastTime ) {
	 prevTruthTime = currentTruthTime;

	 truthIt++;
	 if( truthIt == truthVariables.end() ) {
	    currentTruthTime = 0;
	    break;
	 }
	 
	 currentTruthTime = truthIt->first;
      }

      //
      // Look through the model times to find a model time that is
      // greater than or equal to the current forecast time.  Then
      // the previous model time will be the time less than the
      // current forecast time, while the current model time will be
      // the time greater than or equal to the current forecast time.
      //
      while( currentModelTime < forecastTime ) {
	 prevModelTime = currentModelTime;

	 modelIt++;
	 if( modelIt == modelVariables.end() ) {
	    currentModelTime = 0;
	    break;
	 }
	 
	 currentModelTime = modelIt->first;
      }

      //
      // We need to decide whether the previous truth time or the
      // current truth time is the time to use and whether the previous
      // model time or the current model time is the time to use.  In both
      // cases, use the one that is closest in absolute value to the 
      // current forecast time.  Then set the points and times appropriately.
      //
      MetarPoint *forecastPoint = forecastVariables[forecastTime];
      MetarPoint *truthPoint    = NULL;
      MM5Point   *modelPoint    = NULL;

      int currentTruthDiff = abs( currentTruthTime - forecastTime );
      int prevTruthDiff    = abs( prevTruthTime - forecastTime );
      
      if( currentTruthTime == 0 || currentTruthDiff > prevTruthDiff ) {
	 if( prevTruthDiff < timeTolerance ) {
            truthPoint = truthVariables[prevTruthTime];
	 }
      }
      else {
	 if( currentTruthDiff < timeTolerance ) {
            truthPoint = truthVariables[currentTruthTime];
	 }
      }

      int currentModelDiff = abs( currentModelTime - forecastTime );
      int prevModelDiff    = abs( prevModelTime - forecastTime );
      
      if( currentModelTime == 0 || currentModelDiff > prevModelDiff ) {
	 if( prevModelDiff < timeTolerance ) {
            modelPoint = modelVariables[prevModelTime];
	 }
      }
      else {
	 if( currentModelDiff < timeTolerance ) {
            modelPoint = modelVariables[currentTruthTime];
	 }
      }
      //
      // Add data points in order to set up the data groups
      //
      setGroups( truthPoint, forecastPoint, modelPoint );
      
      //
      // Increment the forecast and model iterators, i.e. start over 
      // with the next forecast time.  We know that the truth iterator
      // should pick up where it left off however.
      //
      forecastIt++;
      forecastTime = forecastIt->first;
      
   }

   return( SUCCESS );
   
}

void
DataMgr::setGroups( MetarPoint* truthPoint, MetarPoint* forecastPoint,
                    MM5Point* modelPoint ) 
{
   double truthVal;
   double fcastVal;
   double modelVal;
   
   //
   // Make sure we have data
   //
   if( forecastPoint == NULL || truthPoint == NULL || modelPoint == NULL )
      return;

   //
   // Set the point in the data pair structures if neither of them
   // is missing
   //
   truthVal = truthPoint->getPrs();
   fcastVal = forecastPoint->getPrs();
   modelVal = modelPoint->getPrs();
   if( fcastVal != MetarPoint::MISSING_VAL && 
       truthVal != MetarPoint::MISSING_VAL &&
       modelVal != MM5Point::MISSING_VAL ) {
      dataGroups[PRS]->setPoint( fcastVal, truthVal, modelVal );
   }
   
   truthVal = truthPoint->getRH();
   fcastVal = forecastPoint->getRH();
   modelVal = modelPoint->getRH();
   if( fcastVal != MetarPoint::MISSING_VAL &&
       truthVal != MetarPoint::MISSING_VAL &&
       modelVal != MM5Point::MISSING_VAL ) {
      dataGroups[RH]->setPoint( fcastVal, truthVal, modelVal );
   }
   
   truthVal = truthPoint->getCeiling();
   fcastVal = forecastPoint->getCeiling();
   if( fcastVal != MetarPoint::MISSING_VAL &&
       truthVal != MetarPoint::MISSING_VAL ) {
      dataGroups[CEIL]->setPoint( fcastVal, truthVal, DataGroup::MISSING_VAL );
   }
   
   truthVal = truthPoint->getVis();
   fcastVal = forecastPoint->getVis();
   if( fcastVal != MetarPoint::MISSING_VAL &&
       truthVal != MetarPoint::MISSING_VAL ) {
      dataGroups[VIS]->setPoint( fcastVal, truthVal, DataGroup::MISSING_VAL );
   }
   
   truthVal = truthPoint->getWspd();
   fcastVal = forecastPoint->getWspd();
   modelVal = modelPoint->getWspd();
   if( fcastVal != MetarPoint::MISSING_VAL &&
       truthVal != MetarPoint::MISSING_VAL ) {
      dataGroups[WSPD]->setPoint( fcastVal, truthVal, modelVal );
   }
   
   truthVal = truthPoint->getWdir();
   fcastVal = forecastPoint->getWdir();
   modelVal = modelPoint->getWdir();
   if( fcastVal != MetarPoint::MISSING_VAL &&
       truthVal != MetarPoint::MISSING_VAL ) {
      dataGroups[WDIR]->setPoint( fcastVal, truthVal, modelVal );
   }
   
   truthVal = truthPoint->getTemp();
   fcastVal = forecastPoint->getTemp();
   modelVal = modelPoint->getTemp();
   if( fcastVal != MetarPoint::MISSING_VAL &&
       truthVal != MetarPoint::MISSING_VAL ) {
      dataGroups[TEMP]->setPoint( fcastVal, truthVal, modelVal );
   }

}

void
DataMgr::writeData( const string& stationId, int leadTime ) 
{  
   //
   // Set the directory for the text files which will contain
   // the data associated with each variable
   //
   setOutputPath( stationId, leadTime );

   //
   // Write the files
   //
   map< VariableType, DataGroup*, less<VariableType> >::iterator it;
   for( it = dataGroups.begin(); it != dataGroups.end(); it++ ) {

      //
      // Set up the file name
      //
      string fileStr = ((*it).second)->getName() + ".txt";
	       
      //
      // Set the complete path - with file name
      //
      outputPath.setFile( fileStr );

      //
      // Write the information
      //
      ((*it).second)->writeVars( outputPath.getPath() );
   }
   

   //
   // Clear out the paths
   //
   outputPath.clear();

   //
   // Tell the user what we are doing
   //
   DateTime start( startTime );
   DateTime end( endTime );

   POSTMSG( INFO, "Wrote text data for station %s and lead time = %d "
            "between %s and %s", stationId.c_str(), leadTime, 
            start.dtime(), end.dtime() );
   
}


void
DataMgr::setOutputPath( const string& stationId, int leadTime ) 
{
   //
   // Set up directory
   //
   DateTime start( startTime );
   DateTime end( endTime );

   char dirStr[TMP_STR_LEN];

   sprintf( dirStr, "%s/%s/f_%.6d/%.2d%.2d%.2d%.2d%.2d%.2d_%.2d%.2d%.2d"
            "%.2d%.2d%.2d", outputDir.c_str(), stationId.c_str(), leadTime,
            start.getYear(), start.getMonth(), start.getDay(), 
            start.getHour(), start.getMin(), start.getSec(), 
            end.getYear(), end.getMonth(), end.getDay(), 
            end.getHour(), end.getMin(), end.getSec() );

   outputPath.setDirectory( dirStr );
   outputPath.makeDirRecurse();
}



   
   
