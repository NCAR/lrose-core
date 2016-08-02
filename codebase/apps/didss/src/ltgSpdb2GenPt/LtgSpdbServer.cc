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
#include <toolsa/utim.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/DateTime.hh>
#include <toolsa/MsgLog.hh> 
#include <toolsa/pmu.h>
#include "LtgSpdbServer.hh"
#include "LtgSpdb2GenPt.hh"
#include <ctime>
#include <Spdb/Spdb_typedefs.hh>
#include <Spdb/DsSpdb.hh>
using namespace std;

//
// Constructor
//
LtgSpdbServer::LtgSpdbServer( Params  &parameters)
{
   params = parameters;
  
   dataType = params.ltgType;
}

// 
// Read point data, return success or failure based on read.
// 
int LtgSpdbServer::readData()
{

  //
  // set requestTime, startTime, endTime.
  // If times are not set or invalid, the times are
  // set to the default values: requestTime gets set to now.
  //                            startTime gets set to now -1 day.
  //                            endTime gets set to now.
  //
    
  DateTime      requestTime( params.requestTime );
  if (!requestTime.isValid())
    requestTime.set( time(0));
  
  DateTime      startTime(params.startTime);
  if (!startTime.isValid())
    startTime.set( time(0) - 86400);

  DateTime      endTime(params.endTime);
   if (!endTime.isValid())
    endTime.set( time(0) - 86400);

   //
   // set time margin (used in CLOSEST,LATEST,BEFORE,AFTER modes) 
   //
   int timeMargin = params.timeMargin * 60; // (seconds)
   
   POSTMSG(INFO, "Reading ltg data in %s \n", params.inputUrl);  

   PMU_auto_register( "Reading lightning data.");
 
   switch( params.dataMode)
     {

     case Params::EXACT:
       POSTMSG(DEBUG, "Reading ltg data in EXACT mode, request time: %s", requestTime.dtime());
       if (spdb.getExact(params.inputUrl, requestTime.utime(), dataType)) 
	 {
	   POSTMSG(ERROR, "%s LtgSpdbServer::readData():", PROGRAM_NAME );
	   POSTMSG(ERROR, "  Calling getExact for url: %s, time: \n", params.inputUrl);
	   return (-1);
	 }          
       break;

     case Params::CLOSEST:
       POSTMSG(DEBUG, "Reading ltg data in CLOSEST mode, request time: %s,  time margin %d ", requestTime.dtime(), timeMargin);
       if (spdb.getClosest(params.inputUrl,
			   requestTime.utime(), timeMargin, dataType)) 
	 {
	   POSTMSG(ERROR, "%s LtgSpdbServer::readData():", PROGRAM_NAME );
	   POSTMSG(ERROR, "  Calling getClosest for url: %s\n",  params.inputUrl);
	   return (-1);
	 }
	   break;

     case Params::INTERVAL:
        POSTMSG(DEBUG, "Reading ltg data in INTERVAL mode, start time: %s, end time %s", startTime.dtime(), endTime.dtime() );
       if (spdb.getInterval(params.inputUrl,
			    startTime.utime(), endTime.utime(), dataType)) 
	 {
	   POSTMSG(ERROR, "%s LtgSpdbServer::readData():", PROGRAM_NAME );
	   POSTMSG(ERROR, "  Calling getInterval for url: %s\n",  params.inputUrl);
	   return (-1);
	 }       
       break;

     case Params::VALID:
        POSTMSG(DEBUG, "Reading ltg data in VALID mode, request time: %s", requestTime.dtime());
       if (spdb.getValid(params.inputUrl, requestTime.utime(), dataType)) 
	 {
	   POSTMSG(ERROR, "%s LtgSpdbServer::readData():", PROGRAM_NAME );
	   POSTMSG(ERROR, "  Calling getValid for url: %s\n",  params.inputUrl);
	   return (-1);
	 }
       break;    
       
     case Params::LATEST:
        POSTMSG(DEBUG, "Reading ltg data in LATEST mode, request time: %s, time margin %d ", requestTime.dtime(), timeMargin);
       if (spdb.getLatest(params.inputUrl, timeMargin, dataType)) 
	 {
	    POSTMSG(ERROR, "%s LtgSpdbServer::readData():", PROGRAM_NAME );
	    POSTMSG(ERROR, "  Calling getLatest for url: %s\n",  params.inputUrl);
	   return (-1);
	 }        
       break;

     case Params::BEFORE:
        POSTMSG(DEBUG, "Reading ltg data in BEFORE mode, request time: %s,  time margin %d ", requestTime.dtime(), timeMargin);
       if (spdb.getFirstBefore(params.inputUrl,
				requestTime.utime(), timeMargin, dataType)) 
	 {
	   POSTMSG(ERROR, "%s LtgSpdbServer::readData():", PROGRAM_NAME );
	   POSTMSG(ERROR, "  Calling   getFirstBefore for url: %s\n",  params.inputUrl);
	   return (-1);
	 }
       break;   
  
     case Params::AFTER:
        POSTMSG(DEBUG, "Reading ltg data in AFTER mode, request time: %s,  time margin %d ", requestTime.dtime(), timeMargin);
       if (spdb.getFirstAfter(params.inputUrl,
			       requestTime.utime(), timeMargin, dataType)) 
	 {
	   POSTMSG(ERROR, "%s LtgSpdbServer::readData():", PROGRAM_NAME );
	   POSTMSG(ERROR, "  Calling   getFirstAfter for url: %s\n",  params.inputUrl);
	   return (-1);
	 }          
       break;
         
       default:
	 {
	   POSTMSG(ERROR, "%s LtgSpdbServer::readData():", PROGRAM_NAME );
	   POSTMSG(ERROR, " Option for data retrieval not recognized. Options are EXACT,CLOSEST,INTERVAL,VALID,LATEST,BEFORE,AFTER. \n");
	   return (-1);
	 }
     }

   return (0);
}

int LtgSpdbServer::readData(time_t timeBegin, time_t timeEnd)
{

  DateTime startTime(timeBegin);
  DateTime endTime(timeEnd);

  POSTMSG(INFO, "Reading ltg data from %s.", params.inputUrl);
  
  POSTMSG(DEBUG, " Data retrieval: INTERVAL mode, start time: %s", startTime.dtime() );
  
  POSTMSG(DEBUG, " end time: %s", endTime.dtime());
                
  PMU_auto_register( "Reading lightning data");

  if (spdb.getInterval(params.inputUrl,
			timeBegin, timeEnd, dataType)) 
	 {
	   POSTMSG(ERROR, "%s LtgSpdbServer::readData():", PROGRAM_NAME );
	   POSTMSG(ERROR, "  Calling getInterval for url: %s\n",  params.inputUrl);
	   return (-1);
	 }       

  return (0);
}

int LtgSpdbServer::writeGenPt( GenPt &genPt)
{ 
  PMU_auto_register( "Writing GenPt output data");

  if (spdb.put(params.outputUrl,
		SPDB_GENERIC_POINT_ID,
		SPDB_GENERIC_POINT_LABEL,
		0,
		genPt.getTime(),
		genPt.getTime() + params.ltg_valid_time,
		genPt.getBufLen(),
		genPt.getBufPtr()),
                0) 
    {
      POSTMSG(ERROR, "%s LtgSpdbServer::writeGenPt():", PROGRAM_NAME );
      POSTMSG( ERROR," Cannot write to output url: %s", params.outputUrl);
      return (-1);
    }
  
  return(0);
}
  




