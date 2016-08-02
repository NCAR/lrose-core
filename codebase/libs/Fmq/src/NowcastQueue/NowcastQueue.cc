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
///////////////////////////////////////////////////////////////////////////////
//
//  Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  February 1999
//
//  $Id: NowcastQueue.cc,v 1.16 2016/03/03 18:08:49 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////

#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/DateTime.hh>
#include <toolsa/MsgLog.hh>
#include <dataport/bigend.h>
#include <Fmq/NowcastQueue.hh>
#include <iostream>
using namespace std;


NowcastQueue::NowcastQueue()
             :DsFmq()
{
   explicitForecast = false;
   issueTime        = DateTime::NEVER;
   forecastTime     = DateTime::NEVER;
   _processInfo.setPID(getpid());
}


////////////////////////////
// Initialization methods //
////////////////////////////

int NowcastQueue::initCreate( const char* fmqURL, 
			      const char* procName, 
			      bool debug,
			      bool compression, 
			      size_t numSlots, 
			      size_t bufSize,
			      MsgLog *msgLog )
{
  return initCreate( fmqURL, procName, "", debug,
		     compression, numSlots, bufSize,
		     msgLog );
}


int NowcastQueue::initCreate( const char* fmqURL, 
			      const char* procName, 
			      const char* procInstance, 
			      bool debug,
			      bool compression, 
			      size_t numSlots, 
			      size_t bufSize,
			      MsgLog *msgLog )
{
  _processInfo.setProcessName(procName);
  _processInfo.setProcessInstance(procInstance);
  
  _fillMsgBuffer();
  
  return DsFmq::initCreate( fmqURL, procName, debug,
			    compression, numSlots, bufSize,
			    msgLog );
}


int NowcastQueue::initReadWrite( const char* fmqURL, 
				 const char* procName, 
				 bool debug,
				 openPosition position,
				 bool compression, 
				 size_t numSlots, 
				 size_t bufSize,
				 int msecSleep,
				 MsgLog *msgLog )
{
  return initReadWrite( fmqURL, procName, "", debug,
			position, compression, numSlots, bufSize,
			msecSleep, msgLog );
}


int NowcastQueue::initReadWrite( const char* fmqURL, 
				 const char* procName, 
				 const char* procInstance, 
				 bool debug,
				 openPosition position,
				 bool compression, 
				 size_t numSlots, 
				 size_t bufSize,
				 int msecSleep,
				 MsgLog *msgLog )
{
  _processInfo.setProcessName(procName);
  _processInfo.setProcessInstance(procInstance);
  
  _fillMsgBuffer();
  
  return DsFmq::initReadWrite( fmqURL, procName, debug,
			       position, compression, numSlots, bufSize,
			       msecSleep, msgLog );
}


int NowcastQueue::initReadOnly( const char* fmqURL, 
				const char* procName, 
				bool debug,
				openPosition position,
				int msecSleep,
				MsgLog *msgLog )
{
  return initReadOnly( fmqURL, procName, "", debug,
		       position, msecSleep, msgLog );
}


int NowcastQueue::initReadOnly( const char* fmqURL, 
				const char* procName, 
				const char* procInstance, 
				bool debug,
				openPosition position,
				int msecSleep,
				MsgLog *msgLog )
{
  _processInfo.setProcessName(procName);
  _processInfo.setProcessInstance(procInstance);
  
  _fillMsgBuffer();
  
  return DsFmq::initReadOnly( fmqURL, procName, debug,
			      position, msecSleep, msgLog );
}


int NowcastQueue::initReadBlocking( const char* fmqURL, 
				    const char* procName, 
				    bool debug,
				    openPosition position,
				    int msecSleep,
				    MsgLog *msgLog )
{
  return initReadBlocking( fmqURL, procName, "", debug,
			   position, msecSleep, msgLog );
}


int NowcastQueue::initReadBlocking( const char* fmqURL, 
				    const char* procName, 
				    const char* procInstance, 
				    bool debug,
				    openPosition position,
				    int msecSleep,
				    MsgLog *msgLog )
{
  _processInfo.setProcessName(procName);
  _processInfo.setProcessInstance(procInstance);
  
  _fillMsgBuffer();
  
  return DsFmq::initReadBlocking( fmqURL, procName, debug,
				  position, msecSleep, msgLog );
}


int NowcastQueue::init( const char* fmqURL, 
			const char* procName, 
			bool debug,
			openMode mode, 
			openPosition position,
			bool compression, 
			size_t numSlots, 
			size_t bufSize,
			int msecSleep,
			MsgLog *msgLog )
{
  return init( fmqURL, procName,  "", debug,
	       mode, position, compression, 
	       numSlots, bufSize, msecSleep, msgLog );
}


int NowcastQueue::init( const char* fmqURL, 
			const char* procName, 
			const char* procInstance, 
			bool debug,
			openMode mode, 
			openPosition position,
			bool compression, 
			size_t numSlots, 
			size_t bufSize,
			int msecSleep,
			MsgLog *msgLog )
{
  _processInfo.setProcessName(procName);
  _processInfo.setProcessInstance(procInstance);
  
  _fillMsgBuffer();
  
  return DsFmq::init( fmqURL, procName,  debug,
		      mode, position, compression, 
		      numSlots, bufSize, msecSleep, msgLog );
}


/////////////////////
// Utility methods //
/////////////////////

int
NowcastQueue::setIssueTime( const char *itime )
{
   DateTime when;

   //
   // Set explicit issue time, overridding fmq communications
   //
   if ( (issueTime = when.parseDateTime( itime )) == -1 ) {
      return( -1 );
   }
   else {
      explicitForecast = true;
      return( 0 );
   }
}

int
NowcastQueue::setForecastTime( const char *ftime )
{
   DateTime when;

   //
   // Set explicit forecast time, overridding fmq communications
   //
   if ( (forecastTime = when.parseDateTime( ftime )) == -1 ) {
      return( -1 );
   }
   else {
      explicitForecast = true;
      return( 0 );
   }
}


///////////////////////
// Messaging methods //
///////////////////////

int                                                                   
NowcastQueue::requestIdentification()
{
  if(_msgLog == NULL)
    fprintf(stderr, "DEBUG: Requesting identification\n");
  else
    _msgLog->postMsg( DEBUG, "Requesting identification." );
  
   if ( writeMsg( IDENTIFY_YOURSELF, 0 ))
      return( -1 );
   else                                     
      return( 0 );
}

int
NowcastQueue::writeIdResponse()
{
  if(_msgLog == NULL)
    fprintf(stderr, "DEBUG: Responding to request for Id: %s (inst= %s) (pid=%d)\n", 
	    _processInfo.getProcessName().c_str(),
	    _processInfo.getProcessInstance().c_str(),
	    (int)_processInfo.getPID() );
  else
    _msgLog->postMsg( DEBUG,
		      "Responding to request for Id: '%s' (inst= '%s') (pid=%d)",
		      _processInfo.getProcessName().c_str(),
		      _processInfo.getProcessInstance().c_str(),
		      (int)_processInfo.getPID() );
  
  if ( writeMsg( PROCESS_ID, (int)_processInfo.getPID(),
		 _msgBuffer.getPtr(), _msgBuffer.getLen()))
    return -1;
  else
    return 0;
}

int
NowcastQueue::nextIdResponse( NowcastProcess &process, pid_t *pid )
{
  bool   gotMsg;
  pid_t  msgPid;

  //
  // Get the next response
  //
  if (nextResponse(&gotMsg, PROCESS_ID) == -1)
  {
    if(_msgLog == NULL)
      fprintf(stderr, "ERROR: Failed in reading fmq for id response.\n");
    else
      _msgLog->postMsg(ERROR, "Failed in reading fmq for id response.");

    return -1;
  }

  if (!gotMsg)
  {
    if(_msgLog == NULL)
      fprintf(stderr, "DEBUG: No more registration responses.\n");
    else
      _msgLog->postMsg(DEBUG, "No more registration responses.");
    
    return -1;
  }

  //
  // Load the message information
  //
  msgPid = (pid_t)getMsgSubtype();
  _loadProcessInfo(process, msgPid, getMsg(), getMsgLen());
  if (pid)
    *pid  = msgPid;

  if(_msgLog == NULL)
    fprintf(stderr, "DEBUG: Registration response from %s (inst=%s) (pid = %d)\n",
	    process.getProcessName().c_str(),
	    process.getProcessInstance().c_str(),
	    msgPid );
  else
    _msgLog->postMsg(DEBUG,
		     "Registration response from '%s' (inst='%s') (pid = %d)",
		     process.getProcessName().c_str(),
		     process.getProcessInstance().c_str(),
		     msgPid );
  
  return 0;
}

int
NowcastQueue::requestForecast( time_t itime, time_t ftime )
{
   //
   // Do the necessary byte swapping
   //
   si32 forecastRequest[2];
   forecastRequest[0] = BE_from_si32( (si32)itime );
   forecastRequest[1] = BE_from_si32( (si32)ftime );

   if(_msgLog == NULL)
     fprintf(stderr, "DEBUG: Requesting a forecast from external processes.\n" );
   else
     _msgLog->postMsg( DEBUG, "Requesting a forecast from external processes." );

   if ( writeMsg( EXECUTE_FORECAST, 0,
                  forecastRequest, 2*sizeof(si32)))
      return( -1 );                                            
   else                    
      return( 0 );
}

int
NowcastQueue::nextForecastRequest( time_t *itime, time_t *ftime )
{
  if(_msgLog == NULL)
    fprintf(stderr, "DEBUG: Waiting for the next forecast request\n");
  else
    _msgLog->postMsg( DEBUG, "Waiting for the next forecast request." );
  
   if ( explicitForecast )
      return getExplicitForecast( itime, ftime );
   else
      return getQueuedForecast( itime, ftime );
}

int                        
NowcastQueue::forecastComplete()
{
   if(_msgLog == NULL)
     fprintf(stderr, "DEBUG: Notifying of completed forecast.\n" );
   else
     _msgLog->postMsg( DEBUG, "Notifying of completed forecast." );
  
   if ( writeMsg( FORECAST_COMPLETE, (int)_processInfo.getPID(),
		  _msgBuffer.getPtr(), _msgBuffer.getLen()))
      return( -1 );
   else
      return( 0 );                                      
}

int
NowcastQueue::forecastIncomplete()
{
   if(_msgLog == NULL)
     fprintf(stderr, "DEBUG: Notifying of incompleted forecast.\n" );
   else
     _msgLog->postMsg( DEBUG, "Notifying of incompleted forecast." );
  
   if ( writeMsg( FORECAST_INCOMPLETE, (int)_processInfo.getPID(),
		  _msgBuffer.getPtr(), _msgBuffer.getLen()))
      return( -1 );
   else
      return( 0 );
}

int
NowcastQueue::nextForecastResponse( NowcastProcess &process, pid_t *pid )
{
  bool  gotMsg;
  int   status;
  pid_t msgPid;
  int   responseType;

  if(_msgLog == NULL)
    fprintf(stderr, "DEBUG: Looking for next forecast response.\n");
  else
    _msgLog->postMsg(DEBUG, "Looking for next forecast response.");

  status = nextResponse(&gotMsg);
  if (status == -1 || !gotMsg)
    return -1;

  //
  // Got a message, make sure it's a forecast response
  //
  responseType = getMsgType();

  if (responseType != FORECAST_COMPLETE &&  
      responseType != FORECAST_INCOMPLETE)
    return -1;

  //
  // Load the message information
  //
  msgPid = (pid_t)getMsgSubtype();
  _loadProcessInfo(process, msgPid, getMsg(), getMsgLen());
  
  if (pid)
    *pid = msgPid;

  if(_msgLog == NULL)
    fprintf(stderr, "DEBUG: Forecast response from %s (inst=%s) (pid=%d)\n",
	    process.getProcessName().c_str(),
	    process.getProcessInstance().c_str(),
	    (int)process.getPID());
  else
    _msgLog->postMsg(DEBUG, "Forecast response from '%s' (inst='%s') (pid=%d)",
		     process.getProcessName().c_str(),
		     process.getProcessInstance().c_str(),
		     (int)process.getPID());
  
  return responseType;
}

int
NowcastQueue::nextResponse( bool *gotMsg, int msgId )
{
   int  status;
   int  tries = 0;

   //
   // Give the fmq read three tries, 
   // in case of collision with multiple respondants
   //
   while( tries < 3 ) {
      status = readMsg( gotMsg, msgId );
      if ( status == 0  &&  *gotMsg )
         break;
      sleep( 1 );
      tries++;
   }
   return 0;
}

int
NowcastQueue::getQueuedForecast( time_t *itime, time_t *ftime )
{
   bool    gotForecast = false;

   //
   // Read until we get a request for a forecast,
   // responding to any identification requests in the meantime
   //
   while( !gotForecast ) {

      if ( readMsgBlocking() != 0 ) {
         return( -1 );
      }

      switch( getMsgType() ) {

         case EXECUTE_FORECAST:
              if ( readForecastRequest( itime, ftime ) != 0 ) {
                 return( -1 );
              }
              gotForecast = true;
              break;

         case IDENTIFY_YOURSELF:
              if ( writeIdResponse() != 0 ) {
                 return( -1 );
              }
              break;

         default:
              break;
      }
   }

   return( 1 );
}

int                                                              
NowcastQueue::readForecastRequest( time_t *itime, time_t *ftime )
{
   //
   // Decode the forecast message
   // Do the necessary byte swapping
   //
   si32 *forecastRequest = (si32*)getMsg();

   *itime = (time_t)BE_to_si32( forecastRequest[0] );
   *ftime = (time_t)BE_to_si32( forecastRequest[1] );

   return( 0 );
}

int
NowcastQueue::getExplicitForecast( time_t *itime, time_t *ftime )
{
   int numRequests;

   //
   // Degenerate case
   //
   if ( issueTime == DateTime::NEVER  &&  
        forecastTime == DateTime::NEVER ) {
      numRequests = 0;
   }

   //
   // Both issue and forecast times have been set explicitly
   //
   else if ( issueTime != DateTime::NEVER  &&  
             forecastTime != DateTime::NEVER ) {
      *itime = issueTime;
      *ftime = forecastTime;
      numRequests = 1;
   }

   //
   // Issue time has been set explicitly.
   // Forecast time defaults to a 30-min offset from the issue time
   //
   else if ( forecastTime == DateTime::NEVER ) {
      *itime = issueTime;
      *ftime = issueTime + 1800;
      numRequests = 1;
   }

   //
   // Forecast time has been set explicitly.
   // Issue time defaults to a 30-min offset from the forecast time
   //
   else {
      *ftime = forecastTime;
      *itime = forecastTime - 1800;
      numRequests = 1;
   }

   //
   // Clear out the request times and return the number of requests
   //
   issueTime    = DateTime::NEVER;
   forecastTime = DateTime::NEVER;
   return( numRequests );
}

int
NowcastQueue::fireTrigger(  const string &saysWho, time_t issueTime, 
                            size_t count, time_t deltaTime )
{
   trigger_t trigger;

   //
   // Setup the trigger request
   // Do the necessary byte swapping
   //
   trigger.issueTime = BE_from_si32( (si32)issueTime );
   trigger.count     = BE_from_ui32( (ui32)count );
   trigger.deltaTime = BE_from_si32( (si32)deltaTime );
   STRncopy( trigger.saysWho, saysWho.c_str(), (int)nameLen );

   if ( writeMsg( NOWCAST_TRIGGER, 0, (void*)(&trigger), sizeof(trigger)))
      return( -1 );
   else
      return( 0 );
}

int
NowcastQueue::nextTrigger( string &saysWho, time_t *issueTime, 
                           size_t *count, time_t *deltaTime )
{
   bool       gotMsg;
   int        status;                                            
   trigger_t  trigger;
   
   status = readMsg( &gotMsg, NOWCAST_TRIGGER );
   if ( status == -1  ||  !gotMsg  ||  getMsgLen() == 0 ) {
      return( -1 );                                               
   }
   else {
      //
      // Decode the trigger message
      // Do the necessary byte swapping
      //
      trigger    = *((trigger_t*)(getMsg()));
      *issueTime = (time_t)BE_to_si32( trigger.issueTime );
      *count     = (size_t)BE_to_ui32( trigger.count );
      *deltaTime = (time_t)BE_to_si32( trigger.deltaTime );
      saysWho    = trigger.saysWho;
      return( 0 );  
   }
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

void NowcastQueue::_fillMsgBuffer()
{
  _fillMsgBuffer(_processInfo.getProcessName(),
		 _processInfo.getProcessInstance());
}


void NowcastQueue::_fillMsgBuffer(const string &proc_name,
				  const string &proc_inst)
{
  _msgBuffer.reset();
  
  _msgBuffer.add(proc_name.c_str(), proc_name.size());
  _msgBuffer.add("\0", 1);
  _msgBuffer.add(proc_inst.c_str(), proc_inst.size());
  _msgBuffer.add("\0", 1);
}


void NowcastQueue::_loadProcessInfo(NowcastProcess &process,
				    const pid_t pid,
				    const void *msg_buffer,
				    const int msg_buffer_len)
{
  char *proc_name = (char *)msg_buffer;
  char *proc_inst = (char *)msg_buffer + strlen(proc_name) + 1;
  
  process.setProcessName(proc_name);
  process.setProcessInstance(proc_inst);
  process.setPID(pid);
}
