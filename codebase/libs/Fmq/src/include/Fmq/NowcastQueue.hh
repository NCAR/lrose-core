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
////////////////////////////////////////////////////////////////////////////////
//
//  Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  February 1999
//
//  $Id: NowcastQueue.hh,v 1.14 2016/03/03 18:56:47 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _NOWCAST_QUEUE_INC_
#define _NOWCAST_QUEUE_INC_

#include <string>
#include <unistd.h>
#include <Fmq/DsFmq.hh>
#include <Fmq/NowcastProcess.hh>
#include <toolsa/MemBuf.hh>
using namespace std;


class NowcastQueue : public DsFmq
{
public:
   NowcastQueue();

  enum msgType
  {
    IDENTIFY_YOURSELF,   // Msg subtype = 0, msg has no body

    PROCESS_ID,          // Msg subtype = PID,
                         // Msg body is process name string
                         //             NULL
                         //             process instance string
                         //             NULL

    EXECUTE_FORECAST,    // Msg subtype = 0
                         // Msg body is issue time (4 bytes)
                         //             forecast time (4 bytes)

    FORECAST_COMPLETE,   // Msg subtype = PID,
                         // Msg body is process name string
                         //             NULL
                         //             process instance string
                         //             NULL

    NOWCAST_TRIGGER,     // Msg subtype = 0
                         // Msg body is trigger_t

    FORECAST_INCOMPLETE  // Msg subtype = PID,
                         // Msg body is process name string
                         //             NULL
                         //             process instance string
                         //             NULL
 };

   //
   // Initialization methods.  These methods override the DsFmq
   // init methods.  This is done because we need to save the 
   // process identification information locally.
   //

   // The following init functions allow you to set up a NowcastQueue
   // in one of the following 4 modes:
   //   CREATE, READ_WRITE, READ_ONLY, (read)BLOCKING
   // These specialized init functions all call the generic init()
   // function. However, they omit the arguments which are not
   // necessary for the particular type of init desired.

   // The following arguments are used in the init functions:
   //
   //   fmqURL: URL for local/remote FMQ
   //   procName: name of your program, for error logging
   //   debug: debug flag, for debug logging
   //   openPosition: for read opens, whether to position the
   //                 queue at the start or end, or ready to
   //                 read last item
   //   compression: for writes, do compression or not?
   //                Compression method defaults to GZIP.
   //                See setCompressionMethod().
   //   numSlots: for creates, number of slots in queue.
   //   bufSize: for creates, total size of data buffer.
   //   msecSleep: for blocking reads, number of milli-seconds
   //              to wait while polling.
   //              If -1, default behavior is 10 msecs sleep for local
   //              access, 500 msecs sleep for remote access.
   //   msgLog: optional pointer to a message log. If NULL, a log
   //           is created by this object.

   // initCreate()
   // Create an FMQ, opening the files in mode "w+". Any existing
   // queue is overwritten.
   // Returns 0 on success, -1 on error
  
   int initCreate( const char* fmqURL, 
		   const char* procName, 
		   bool debug = false,
		   bool compression = false, 
		   size_t numSlots = 1024, 
		   size_t bufSize = 1000000,
		   MsgLog *msgLog = NULL );

   int initCreate( const char* fmqURL, 
		   const char* procName, 
		   const char* procInstance, 
		   bool debug = false,
		   bool compression = false, 
		   size_t numSlots = 1024, 
		   size_t bufSize = 1000000,
		   MsgLog *msgLog = NULL );

   // initReadWrite()
   // If FMQ already exists, it is opened in mode "r+".
   // If not it is created by opening in mode "w+".
   //
   // msecSleep is used for any subsequent blocking reads.
   //   If -1, default behavior is 10 msecs sleep for local
   //   access, 500 msecs sleep for remote access.
   //
   // Returns 0 on success, -1 on error
 
   int initReadWrite( const char* fmqURL, 
		      const char* procName, 
		      bool debug = false,
		      openPosition position = END,
		      bool compression = false, 
		      size_t numSlots = 1024, 
		      size_t bufSize = 1000000,
		      int msecSleep = -1,
		      MsgLog *msgLog = NULL );

   int initReadWrite( const char* fmqURL, 
		      const char* procName, 
		      const char* procInstance, 
		      bool debug = false,
		      openPosition position = END,
		      bool compression = false, 
		      size_t numSlots = 1024, 
		      size_t bufSize = 1000000,
		      int msecSleep = -1,
		      MsgLog *msgLog = NULL );

   // initReadOnly()
   // Open for reading, in mode "r".
   // If no queue exists, returns an error.
   //
   // msecSleep is used for any subsequent blocking reads.
   //   If -1, default behavior is 10 msecs sleep for local
   //   access, 500 msecs sleep for remote access.
   //
   // Returns 0 on success, -1 on error
  
   int initReadOnly( const char* fmqURL, 
		     const char* procName, 
		     bool debug = false,
		     openPosition position = END,
		     int msecSleep = -1,
		     MsgLog *msgLog = NULL );

   int initReadOnly( const char* fmqURL, 
		     const char* procName, 
		     const char* procInstance, 
		     bool debug = false,
		     openPosition position = END,
		     int msecSleep = -1,
		     MsgLog *msgLog = NULL );

   // initReadBlocking()
   // If queue exists, opens for reading, in mode "r".
   // If the queue does not exist, blocks while waiting for it to be
   // created by another process. While waiting, registers with procmap
   // if PMU module has been initialized.
   //
   // msecSleep is for blocking reads.
   //   If -1, default behavior is 10 msecs sleep for local
   //   access, 500 msecs sleep for remote access.
   //
   // Returns 0 on success, -1 on error
  
   int initReadBlocking( const char* fmqURL, 
			 const char* procName, 
			 bool debug = false,
			 openPosition position = END,
			 int msecSleep = -1,
			 MsgLog *msgLog = NULL );

   int initReadBlocking( const char* fmqURL, 
			 const char* procName, 
			 const char* procInstance, 
			 bool debug = false,
			 openPosition position = END,
			 int msecSleep = -1,
			 MsgLog *msgLog = NULL );

   // generic init() - allows full control of the init
   // Returns 0 on success, -1 on error

   int init( const char* fmqURL, 
	     const char* procName, 
	     bool debug = false,
	     openMode mode = READ_WRITE, 
	     openPosition position = END,
	     bool compression = false, 
	     size_t numSlots = 1024, 
	     size_t bufSize = 1000000,
	     int msecSleep = -1,
	     MsgLog *msgLog = NULL );

   int init( const char* fmqURL, 
	     const char* procName, 
	     const char* procInstance, 
	     bool debug = false,
	     openMode mode = READ_WRITE, 
	     openPosition position = END,
	     bool compression = false, 
	     size_t numSlots = 1024, 
	     size_t bufSize = 1000000,
	     int msecSleep = -1,
	     MsgLog *msgLog = NULL );


   //
   // FMQ messaging from server application
   //
   int              requestIdentification();
   int              requestForecast( time_t issueTime, time_t forecastTime );
   int              nextIdResponse( NowcastProcess &processInfo, 
                                    pid_t *processId = NULL );
   int              nextForecastResponse( NowcastProcess &processName, 
                                          pid_t *processId = NULL );
   int              nextTrigger( string &saysWho, time_t *issueTime, 
                                 size_t *count, time_t *deltaTime );

   //
   // FMQ messaging from client applications
   //
   int              nextForecastRequest( time_t *issueTime, 
                                         time_t *forecastTime );
   int              forecastComplete();
   int              forecastIncomplete();
   int              fireTrigger( const string &saysWho, time_t issueTime, 
                                 size_t count = 1, time_t deltaTime = 0 );

   //
   // Explicit forecast request
   //
   int              setIssueTime( const char *itime );
   int              setForecastTime( const char *ftime );

   time_t           getIssueTime(){ return issueTime; }
   time_t           getForecastTime(){ return forecastTime; }

   static const int nameLen=128;

protected:

  NowcastProcess _processInfo;
  
  MemBuf _msgBuffer;

  //
  // Fill the message buffer with the process information
  //
  void _fillMsgBuffer();
  void _fillMsgBuffer(const string &proc_name, const string &proc_inst);
  
  //
  // Load the process information received in a message
  //
  void _loadProcessInfo(NowcastProcess &process,
			const pid_t pid,
			const void *msg_buffer,
			const int msg_buffer_len);
  

private:

   //
   // Request times
   //
   bool             explicitForecast;
   time_t           issueTime;
   time_t           forecastTime;


   //
   // Process management
   //
   int              getExplicitForecast( time_t *itime, time_t *ftime );
   int              getQueuedForecast( time_t *itime, time_t *ftime );

   int              readForecastRequest( time_t *itime, time_t *ftime );
   int              writeIdResponse();
   int              nextResponse(bool *gotMsg, int msgType = -1);

   //
   // Triggering
   //
   typedef struct {
      si32  issueTime;
      ui32  count;
      si32  deltaTime;
      char  saysWho[nameLen];
   } trigger_t;

};

#endif
