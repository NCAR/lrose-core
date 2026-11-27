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
// DsFmq2Tape top level application class
//
// $Id: DsFmq2Tape.cc,v 1.5 2016/03/06 23:53:39 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////

#include <fcntl.h>
#include <sys/file.h>

#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <toolsa/ttape.h>
#include <toolsa/membuf.h>
#include <toolsa/MsgLog.hh>
#include <Fmq/DsFmq.hh>

#include "DsFmq2Tape.hh"
using namespace std;

//
// static definitions
//
const string DsFmq2Tape::version = "1.0";


DsFmq2Tape::DsFmq2Tape()
{
   tapeId = -1;
   fmq    = new DsFmq;
   msgLog = new MsgLog;
}

DsFmq2Tape::~DsFmq2Tape()
{
   delete fmq;
   delete msgLog;
}

int
DsFmq2Tape::init( int argc, char **argv )
{
   //
   // Some general stuff
   //
   program.setPath( argv[0] );
   msgLog->setApplication( getProgramName() );

   if ( processArgs( argc, argv ) != 0 )
      return( -1 );

   ucopyright( (char*)PROGRAM_NAME );
   if ( readParams( argc, argv ) != 0 )
      return( -1 );

   //
   // Register with procmap now that we have the instance name
   //
   if ( params.instance == NULL )
      params.instance = "generic";
   PMU_auto_init( (char*)PROGRAM_NAME, params.instance,
                  PROCMAP_REGISTER_INTERVAL );
   PMU_auto_register( "starting up advectGrid" );

   //
   // Process the parameters
   //
   if ( processParams() != 0 )
      return( -1 );

   return( 0 );
}

void
DsFmq2Tape::usage()
{
   cerr << "Usage: " << PROGRAM_NAME   << " [options as below]\n"
        << "       [ -check_params ]        check parameter settings\n"
        << "       [ -debug ]               print verbose debug messages\n"
        << "       [ --, -h, -help, -man ]  produce this list\n"
        << "       [ -params params_file ]  set parameter file name\n"
        << "       [ -print_params ]        produce parameter listing\n"
        << endl;
   dieGracefully( -1 );
}

int 
DsFmq2Tape::processArgs( int argc, char **argv )
{
   //
   // Process each argument
   //
   for( int i=1; i < argc; i++ ) {
      //
      // request for usage information
      //
      if ( !strcmp(argv[i], "--" ) ||
           !strcmp(argv[i], "-h" ) ||
           !strcmp(argv[i], "-help" ) ||
           !strcmp(argv[i], "-man" )) {
         usage();
      }

      //
      // request for version number
      //
      if ( !strcmp(argv[i], "-v" ) ||
           !strcmp(argv[i], "-version" )) {
         POSTMSG( "version %s", version.c_str() );
         dieGracefully( 0 );
      }

      //
      // request for verbose debug messaging
      //
      if ( !strcmp(argv[i], "-debug" )) {
         msgLog->enableMsg( DEBUG, true );
      }
   }

   return( 0 );
}

int
DsFmq2Tape::readParams( int argc, char **argv )
{
   int             status = 0;
   char           *readPath = NULL;
   tdrp_override_t override;

   //
   // Read the parameter file
   //
   TDRP_init_override( &override );
   if ( params.loadFromArgs( argc, argv,
                             override.list, &readPath ) != 0 ) {
      status = -1;
   }
   if ( readPath ) {
      paramPath = readPath;
   }
   TDRP_free_override( &override );

   //
   // Make sure the read worked
   //
   if ( status == -1 ) {
      POSTMSG( ERROR, "Unable to load parameters." );
      if ( paramPath.isValid() )
         POSTMSG( ERROR, "Check syntax of parameter file: %s in %s",
                         paramPath.getFile().c_str(),
                         paramPath.getDirectory().c_str() );
      return( status );
   }

   return( 0 );
}

int
DsFmq2Tape::processParams()
{
   //
   // User-specified parameter file -- do validity checks
   //
   if ( paramPath.isValid() ) {
      POSTMSG( DEBUG, "Loading Parameter file: %s",
                     paramPath.getFile().c_str() );
   }

   //
   // Initialize the messaging
   //
   POSTMSG( DEBUG, "Initializing message log." );
   if ( initLog() != 0 )
      return( -1 );

   //
   // Initialize the fmq for blocking reads
   //
   POSTMSG( DEBUG, "Initializing the request queue." );
   if ( fmq->init( params.fmq_url,
                   PROGRAM_NAME, 
                   DEBUG_ENABLED,
                   DsFmq::BLOCKING_READ_ONLY ) != 0 ) {
      POSTMSG( ERROR, "Cannot initialize fmq.\n"
                      "Make sure fmq_url (%s) is correct.",
                      params.fmq_url );
      return( -1 );
   }

   //
   // Open the tape device for writing
   //
   tapeId = open( params.tape_device_name, O_WRONLY );
   if ( tapeId == -1 ) {
      POSTMSG( ERROR, "Failed opening tape device" );
      perror( params.tape_device_name );
      dieGracefully( -1 );
   }
   TTAPE_set_var( tapeId );

   return 0;
}

int
DsFmq2Tape::initLog()
{
   //
   // Set debug level messaging
   //
   if( params.debug )
      msgLog->enableMsg( DEBUG, true );

   //
   // Direct message logging to a file
   //
   if ( msgLog->setOutputDir( params.log_dir ) != 0 ) {
      POSTMSG( WARNING, "Cannot write log messages to output directory '%s'",
                        params.log_dir );
   }

   return( 0 );
}

void
DsFmq2Tape::run()
{
   MEMbuf *buffer;
   char   *msg;
   int     msgLen;
   int     bytesWritten;
   int     spaceAvail;
   int     blockSize;
   int     numMsgInBlock = 0;

   //
   // Create the in-memory buffer
   //
   buffer = MEMbufCreate();
   MEMbufReset( buffer );
  
   while( true ) {

      PMU_auto_register( "Reading from the FMQ" );
   
      //
      // Blocking read of fmq
      //
      if ( fmq->readMsgBlocking() != 0 ) {
         POSTMSG( ERROR, "Failed reading fmq." );
         sleep( 1 );
      }
      else {
         //
         // Got a message from the fmq
         //
         msg = (char *)fmq->getMsg();
   
         if ( fmq->getMsgType() == DsFmq::eof ) {
            //
            // Got an end-of-file marker
            //
            TTAPE_write_eof( tapeId, 1 );
            POSTMSG( DEBUG, "Writing EOF marker to tape" );
         }
         else {
            //
            // Check for valid message length
            //
            msgLen = fmq->getMsgLen();
            if( msgLen > params.max_block_size ) {
               POSTMSG( WARNING, 
                        "Skipping message which exceeds max_block_size." );
               continue;
            }

            //
            // Check blocksize to decide if it's time to do a write
            //
            numMsgInBlock++;
            blockSize = MEMbufLen( buffer );
            spaceAvail = params.max_block_size - blockSize;
            if ( spaceAvail < msgLen  ||
                 numMsgInBlock > params.max_message_per_block ) {
               //
               // The buffer is full - write it to tape
               //
               bytesWritten = write( tapeId, MEMbufPtr(buffer), blockSize );
               if ( bytesWritten != blockSize ) {
                   POSTMSG( ERROR, "Failed writing to tape device." );
                   perror( params.tape_device_name );
                   POSTMSG( ERROR, "Rewinding and ejecting tape." );
                   TTAPE_rewoffl( tapeId );
                   dieGracefully( -1 );
               }

               //
               // Reset the buffered blocks
               //
               POSTMSG( DEBUG, "Wrote block of size %d to tape", blockSize );
               MEMbufReset( buffer );
               numMsgInBlock = 1;
            }

            POSTMSG( DEBUG, "Adding message of length %d to buffer", msgLen);
            MEMbufAdd( buffer, (void *)msg, (size_t)msgLen );
         }
      }
   }
}
