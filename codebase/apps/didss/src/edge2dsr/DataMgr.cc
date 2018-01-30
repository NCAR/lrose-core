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
//////////////////////////////////////////////////////////////////////
// $Id: DataMgr.cc,v 1.53 2016/03/06 23:53:42 dixon Exp $
//
// Data managment class
/////////////////////////////////////////////////////////////////////
#include <vector>
#include <climits>
#include <cmath>
#include <unistd.h>
#include <toolsa/file_io.h>
#include <toolsa/udatetime.h>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/MsgLog.hh>
#include <rapformats/DsRadarMsg.hh>
#include <Fmq/DsRadarQueue.hh>
//#include <fmqFormats/DsRadarQueue.hh>

#include "DataMgr.hh"
#include "Edge2Dsr.hh"
#include "EdgeMsg.hh"
#include "EdgeTape.hh"
#include "EdgeTcpip.hh"
#include "EdgeUdp.hh"
#include "ScanStrategy.hh"

#include "DropEOVStrategy.hh"
#include "EndEOVStrategy.hh"
#include "StartEOVStrategy.hh"
using namespace std;


const int DataMgr::MAX_BUF_SIZE   = 5000;

DataMgr::DataMgr() :
  edgeUdp(0),
  edgeFmq(0),
  edgeTape(0),
  buffer(new char[MAX_BUF_SIZE]),
  edgeMsg(new EdgeMsg()),
  paramsRead(false),
  _beamWriter(0),
  archiveQueue(0),
  archiveFp(0)
{
   memset( (void *) buffer, (int) 0, MAX_BUF_SIZE );
}

DataMgr::~DataMgr() 
{
  delete edgeUdp;
  delete edgeFmq;
  delete edgeTape;

  delete[] buffer;
  delete edgeMsg;

  delete _beamWriter;
  
  delete archiveQueue;
  if (archiveFp != 0)
    fclose( archiveFp );
}

int
DataMgr::init( Params& params, MsgLog* msgLog ) 
{ 
   //
   // Are we writing to a log file?
   //
   if( params.archive_file && 
       strcmp( params.archive_file_path, "" ) ) {
      if( (archiveFp = 
           ta_fopen_uncompress( params.archive_file_path, "a" )) == NULL ) {
	 POSTMSG( ERROR, "Could not open log file %s", 
                  params.archive_file );
	 return( FAILURE );
      }
   } 

   switch( params.input_type ) {
      case Params::UDP:
         
         //
         // Set up input udp
         //
         edgeUdp = new EdgeUdp( params.port, MAX_BUF_SIZE );
         if( edgeUdp->init() != SUCCESS )
           return( FAILURE );
         inputMode = UDP;
         break;
  
      case Params::TCPIP:
         
         //
         // Set up input TCP/IP object
         //
         edgeTcpip = new EdgeTcpip(params.tcpip_host, params.port,
				   MAX_BUF_SIZE);
         if (!edgeTcpip->init())
           return( FAILURE );
         inputMode = TCPIP;
         break;
  
      case Params::FMQ:

	//
	// Set up input fmq
	//
        edgeFmq = new DsFmq;
        if ( edgeFmq->init( params.input_fmq_url, 
                            PROGRAM_NAME, DEBUG_ENABLED,
                            DsFmq::BLOCKING_READ_ONLY, 
                            DsFmq::END,
                            false, 1024, 10000, 1000,
                            msgLog ) != 0 ) {
           POSTMSG( ERROR, "Cannot initialize input queue.\n"
                           "Make sure fmq_url (%s) is correct.",
                           params.input_fmq_url );
           return( FAILURE );
        }
        inputMode = FMQ;
        break;

       case Params::TAPE:

	  //
	  // Set up edge tape object
	  //
	  edgeTape = new EdgeTape( params.input_tape_name,
				   params.input_tape_wait,
				   edgeMsg->getHeaderSize() );
	  edgeTape->init();
	  inputMode = TAPE;
          break;

      default:
        return( FAILURE );
        break;

   }

   //
   // Set up archive fmq
   //
   if( params.archive_fmq ) {
     archiveQueue = new DsFmq;
     if (archiveQueue->init( params.archive_fmq_url,
                             PROGRAM_NAME,
                             DEBUG_ENABLED,
                             DsFmq::READ_WRITE, DsFmq::END,
                             (bool) params.archive_fmq_compress,
                             params.archive_fmq_nslots, 
                             params.archive_fmq_size,
                             1000, msgLog )) {
       POSTMSG( ERROR, "Could not initialize archive queue '%s'", 
                params.archive_fmq_url );
       return( FAILURE );
     }
   }

   //
   // Initialize the beam writer object
   //

   _beamWriter = new BeamWriter();
   
   ScanStrategy scan_strategy;
   
   int n_elevations = params.scan_strategy_n;
   for( int i = 0; i < params.scan_strategy_n; ++i )
      scan_strategy.addElevation(params._scan_strategy[i]);

   if (!_beamWriter->init(params.output_fmq_url,
			  (bool)params.output_fmq_compress,
			  params.output_fmq_nslots,
			  params.output_fmq_size,
                          params.write_blocking,
			  archiveQueue,
			  scan_strategy,
			  n_elevations,
			  params.max_elev_diff,
			  params.max_diff_from_scan,
			  params.max_diff_from_target,
			  msgLog))
   {
     POSTMSG(ERROR, "Could not initialize beam writer object");
     return(FAILURE);
   }
   
   if (!_beamWriter->initRadarParams(params.radar_id,
				     params.n_gates_out,
				     params.polarization_code,
				     params.radar_constant,
				     params.gate_spacing,
				     params.beam_width,
				     params.wavelength,
				     params.peak_xmit_pwr,
				     params.receiver_mds,
				     params.receiver_gain,
				     params.antenna_gain,
				     params.system_gain,
				     params.radar_name,
				     params.site_name,
				     params.scan_type_name))
   {
     POSTMSG(ERROR, "Could not initialize beam writer radar parameters");
     return(FAILURE);
   }
   
   if (!_beamWriter->initFieldParams(params.dbz_scaling_info.scale,
				     params.dbz_scaling_info.bias,
				     params.vel_scaling_info.scale,
				     params.vel_scaling_info.bias,
				     params.sw_scaling_info.scale,
				     params.sw_scaling_info.bias,
				     params.dbz_scaling_info.scale,
				     params.dbz_scaling_info.bias))
   {
     POSTMSG(ERROR, "Could not initialize beam writer field parameters");
     return(FAILURE);
   }
   
   EOVStrategy *eov_strategy;

   switch (params.trigger_mode)
   {
   case Params::TRIGGER_BY_ANGLE_DROP :
     eov_strategy = new DropEOVStrategy(params.end_of_volume_trigger);
     break;
     
   case Params::TRIGGER_BY_END_TILT :
     eov_strategy = new EndEOVStrategy(params.end_of_volume_trigger,
				       scan_strategy);
     break;
     
   case Params::TRIGGER_BY_START_TILT :
     eov_strategy = new StartEOVStrategy(params.end_of_volume_trigger,
					 scan_strategy);
     break;
   }
   _beamWriter->setEndOfVolStrategy(eov_strategy);
  
   if (params.override_latlon)
     _beamWriter->setLatLonOverride(params.radar_location.latitude,
				    params.radar_location.longitude,
				    params.radar_location.altitude);
   
   _beamWriter->setDiagnostics(params.print_summary == TRUE,
			       params.summary_interval);
   
   //
   // Set up edge message object
   //
   edgeMsg->init( params );


   return( SUCCESS );
}

int
DataMgr::processData() 
{
   //
   // Clear out the buffer
   //
   memset( (void *) buffer, (int) 0, MAX_BUF_SIZE );
   bufferLen = 0;

   //
   // Read raw data
   //
   switch( inputMode ) {
     case TCPIP:
       PMU_auto_register( "Reading TCP/IP socket" );
       if (edgeTcpip->readTcpip())
       {
	 bufferLen = edgeTcpip->getNextMsg(buffer, MAX_BUF_SIZE);
	 
	 if (bufferLen < 0)
	 {
	   POSTMSG(ERROR,
		   "Error reading EDGE message from TCP/IP sockets");
	   
	   return FAILURE;
	 }
       }
       break;

     case UDP:
       PMU_auto_register( "Reading udp socket" );
       if( (bufferLen = edgeUdp->readUdp( buffer )) <= 0 ) {
          return( FAILURE );
       }
       break;

     case FMQ:
        
        PMU_auto_register( "Reading fmq" );
        if( edgeFmq->readMsgBlocking() != 0 ) {

          POSTMSG( ERROR, "Failed reading input fmq" );
          sleep( 1 );

        } else {

           if( edgeFmq->getMsgType() != DsFmq::eof ) {
              bufferLen = edgeFmq->getMsgLen();
              memcpy( (void *) buffer, 
                      (void *) edgeFmq->getMsg(),
                      bufferLen );
           }
        }
        break;

     case TAPE:

        PMU_auto_register( "Reading tape" );
	if( edgeTape->getMsg( buffer, &bufferLen ) != SUCCESS ) {
	   
	   POSTMSG( ERROR, "Failed reading input tape" );
	   return( FAILURE );
	}
	break;
   }

   //
   // Write udp message to archive fmq
   //
   PMU_auto_register( "Writing to archive fmq" );
   if( archiveQueue ) {
      archiveQueue->writeMsg( DS_MESSAGE_TYPE_EEC_ASCII,
                              0, buffer, bufferLen );
   }

   //
   // Write buffer to archive file
   //
   PMU_auto_register( "Writing to archive file" );
   if( archiveFp ) { 
     char lenString[9];
     sprintf( lenString, "  %4d:  ", bufferLen );
     if( fwrite( (void *) lenString, sizeof(char), 9, 
         archiveFp ) != 9 * sizeof(char) ) {
         POSTMSG( ERROR, "Could not write to archive file" );
         return( FAILURE );
     }
     if( fwrite( (void *) buffer, sizeof(char), (size_t) bufferLen,
             archiveFp ) != bufferLen * sizeof(char) ) {
         POSTMSG( ERROR, "Could not write to archive file" );
         return( FAILURE );
     }
   }

   //
   // Read header
   //
   PMU_auto_register( "Reading header" );
   if( edgeMsg->readHdr( buffer ) != SUCCESS ) {
      return( FAILURE );
   }

   //
   // If the message was a beam, decide if we got
   // a new beam.  If we did, use what is already
   // in the edge message to write out the beam data.
   // This will be the previous beam data.  If we
   // didn't get a new beam, proceed with reading
   // the message and filling out the beam data.
   //
   // If we got a status message, read it and 
   // continue.
   //
   switch( edgeMsg->getMsgType() ) {
       case EdgeMsg::BEAM:
          PMU_auto_register( "Reading beam data" );
	  POSTMSG( DEBUG, "Reading beam data" );
          if( !paramsRead ) {
	     return( SUCCESS );
	  }
	  if( edgeMsg->isNewBeam() ) {
	     _beamWriter->writeBeam(*edgeMsg);
	  }
	  
          if( edgeMsg->readMsg( buffer ) != SUCCESS ) {
	     return( FAILURE );
	  }
	  break;

       case EdgeMsg::STATUS:
          PMU_auto_register( "Reading status information" );
	  POSTMSG( DEBUG, "Reading status information" );
	  if( edgeMsg->readMsg( buffer ) != SUCCESS ) {
	     return( FAILURE );
	  }
          paramsRead = true;
	  POSTMSG( DEBUG, "Parameters read" );
	  break;

       case EdgeMsg::UNKNOWN:
	  break;
   }
   
   return( SUCCESS );
}
