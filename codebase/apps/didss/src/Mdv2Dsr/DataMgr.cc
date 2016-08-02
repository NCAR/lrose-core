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
// DataMgr - Data Manager
//
// $Id: DataMgr.cc,v 1.17 2016/03/06 23:53:41 dixon Exp $
//
///////////////////////////////////////////////////////////////
#include <toolsa/utim.h>
#include <toolsa/MsgLog.hh>
#include <rapmath/math_macros.h>
#include <didss/DsInputPath.hh>
#include <didss/ds_input_path.h>

#include "DataMgr.hh"
#include "Ingester.hh"
#include "Mdv2Dsr.hh"
using namespace std;

DataMgr::DataMgr()
{
   inputPath         = NULL;
   ingester          = NULL;
   useSimulatedTimes = false;
}

DataMgr::~DataMgr()
{
   delete inputPath;
   delete ingester;

   vector< pair< time_t, time_t>* >::iterator it;
   for( it = simulatedTimes.begin(); it != simulatedTimes.end(); it++ ) {
      delete (*it);
   }
   
}

int DataMgr::init( Params& params, MsgLog* msgLog,
                   vector< string > &fileList )
{
   if( radarQueue.init( params.output_fmq_url,
		        PROGRAM_NAME,
		        DEBUG_ENABLED,           
		        DsFmq::READ_WRITE, DsFmq::END, 
		        params.output_fmq_compress,
		        params.output_fmq_nslots,
		        params.output_fmq_size, 1000,
                        msgLog )) {
      POSTMSG( ERROR, "Could not initialize fmq %s", 
	       params.output_fmq_url );
      return( FAILURE );
   }
   
   if (params.write_blocking) {
     radarQueue.setBlockingWrite();
   }

   if( fileList.size() > 0 ) {

      inputPath = new DsInputPath( PROGRAM_NAME,
				   params.debug,
                                   fileList,
				   true );

      if( params.simulate_time && !params.use_current_time ) {

         useSimulatedTimes = true;

	 if( params.start_end_times_n != (int)fileList.size() ) {
	    POSTMSG( ERROR, "Number of start and end times does not "
		     "match the number of files" );
	    return( FAILURE );
	 }

         for( int i = 0; i < params.start_end_times_n; i++ ) {
	   time_t startTime = 
	    _stringToTime(params._start_end_times[i].simulate_start_time);
	    
           time_t endTime = 
	      _stringToTime(params._start_end_times[i].simulate_end_time);
	   
           pair< time_t, time_t >* newPair = new pair< time_t, time_t >
	      ( startTime, endTime );
	   
           simulatedTimes.push_back( newPair );
	 }
      }
      
   }
   else {

      time_t start_time = _stringToTime( params.start_time );
      time_t end_time   = _stringToTime( params.end_time );

      if( start_time == 0 || end_time == 0 ) {
	// Get the start and end times from the data itself

	time_t data_start_time, data_end_time;
	DSINP_handle_t input_handle;
	
	DSINP_create_triggered(&input_handle,
			       (char *)PROGRAM_NAME,
			       DEBUG_ENABLED,
			       params.input_dir);
	
	if (DSINP_get_begin_and_end_times(&input_handle,
					  &data_start_time,
					  &data_end_time) != 0)
	{
	  POSTMSG( ERROR, "Cannot get start or end times from data" );
	  return( FAILURE );
	}
	
	if (start_time == 0)
	  start_time = data_start_time;
	
	if (end_time == 0)
	  end_time = data_end_time;
	
	DSINP_free(&input_handle);
      }
      
      POSTMSG(DEBUG, "Start time = %s", UTIMstr(start_time));
      POSTMSG(DEBUG, "End time = %s", UTIMstr(end_time));
      
      inputPath = new DsInputPath( (char *) PROGRAM_NAME,
                                   DEBUG_ENABLED,
                                   params.input_dir,
                                   start_time,
                                   end_time );
   }

   ingester = new Ingester( radarQueue, params );

   return( SUCCESS );
}

int
DataMgr::processData() 
{
   char *filePath;
   vector< pair< time_t, time_t>* >::iterator it = 
      simulatedTimes.begin();
   
   int volNum = 0;
   int iret = 0;

   while( (filePath = inputPath->next()) != NULL ) {

      PMU_auto_register("Processing data");
     
      POSTMSG( INFO, "Processing file %s", filePath );

      if( useSimulatedTimes ) {
         if( it == simulatedTimes.end() ) {
	    POSTMSG( ERROR, "Could not find simulated times for this file" );
	    return( FAILURE );
	 }
	 if( ingester->processFile( filePath, 
                                    (int) (*it)->first, 
                                    (int) (*it)->second ) != SUCCESS ) {
	    POSTMSG( ERROR, "Could not process file %s", filePath );
	    iret = -1;
	 }
         it++;
      }
      else {
	 
         if( ingester->processFile( filePath ) != SUCCESS ) {
	    POSTMSG( ERROR, "Could not process file %s", filePath );
	    iret = -1;
	 }
	 
      }

      volNum++;

   }

   POSTMSG( INFO, "No more files" );
   return( iret );
}

//////////////////////////////
// reset input simulate data

void DataMgr::resetInputDataQueue() 
{
  inputPath->reset();
}

//////////////////////////////////////////
// parse a string to compute the date/time
//
// Acceptable formats are:
//
//   hh:mm:ss mm/dd/yyyy
//   yyyy mm dd hh mm ss
//
// Returns 0 on error

time_t DataMgr::_stringToTime(const char *timeStr)

{

  time_t ttime = 0;
  
  if (strchr(timeStr, ':') != NULL) {

    // format is hh:mm:ss mm/dd/yyyy
    
    ttime = UTIMstring_US_to_time(timeStr);

  } else {

    // format is yyyy mm dd hh mm ss

    int year, month, day, hour, min, sec;

    if (sscanf(timeStr, "%d %d %d %d %d %d", 
	       &year, &month, &day, &hour, &min, &sec) == 6) {
      DateTime dtime(year, month, day, hour, min, sec);
      ttime = dtime.utime();
    }

  }

  return ttime;
    
}
