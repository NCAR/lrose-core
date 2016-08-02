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
////////////////////////////////////////////////////////////////////////
//  Class for managing the flow of data
//
//  Jaimi Yee, RAP, NCAR, Boulder, CO, 80307, USA.
//  July 2004
//  
//  Adapted from nexrad2dsr application by Terri Betancourt 
//  RAP, NCAR, Boulder, CO, 80307, USA
//
//  $Id: DataMgr.cc,v 1.14 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////////////////////

#include <toolsa/pmu.h>

#include "Driver.hh"
#include "NexradLdm.hh"
#include "NexradTCP.hh"
#include "NexradTape.hh"
#include "DataMgr.hh"
using namespace std;

DataMgr::DataMgr()
{
   inputStream           = NULL;
   inputStatus           = Status::ALL_OK;
   outputStatus          = Status::ALL_OK;
   tiltStatus            = Status::ALL_OK;
   firstCall             = true;
}

DataMgr::~DataMgr()
{
   delete inputStream;
}

int 
DataMgr::init( Params& params )
{
   POSTMSG( DEBUG, "Initializing the data streams" );

   //
   // Create the input data stream
   //
   switch( params.input_mechanism ) {

      case Params::LDM:
           inputStream = new NexradLdm();
           break;

      case Params::TCP_IP:
           inputStream = new NexradTCP();
           break;

      case Params::TAPE:
           inputStream = new NexradTape();
           break;
   }

   //
   // Initialize the input data stream
   //
   PMU_auto_register( "Initializing the input stream" );

   if ( inputStream->init( params ) != 0 )
      return( -1 );

   //
   // Initialize the ingester
   //
   if( ingester.init( params ) != Status::ALL_OK ) {
      return( -1 );
   }

   return( 0 );
}

Status::info_t
DataMgr::processData() 
{
   ui08   *buffer;
   bool    volumeTitleSeen;

   PMU_auto_register( "Begin processing radar data" );

   //
   // Continuous processing steps
   //
   while( true ) {

      //
      // Get a new radar message
      //
      PMU_auto_register( "Reading a new input message" );
      inputStatus = inputStream->readNexradMsg( buffer, volumeTitleSeen );

      //
      // Turn the message over to the ingester to process it
      //
      Status::info_t outputStatus = Status::ALL_OK;

      if ( inputStatus == Status::ALL_OK ) {
         outputStatus = ingester.addBeam( buffer, volumeTitleSeen );
      }
      else if ( inputStatus == Status::END_OF_DATA ) {
	 outputStatus = ingester.endOfData();
      }
         
      //
      // Write the state if appropriate
      //
      ////////////////////////////////////////////////
      // NEED TO TAKE THIS OUT WHEN WE SWITCH TO LDATA
      // INFO COMPLETELY
      /////////////////////////////////////////////////
      if( outputStatus == Status::OUTPUT_WRITTEN ) {
         inputStream->writeState();
      }
        
      //
      // If we failed, return
      // 
      if( outputStatus != Status::ALL_OK && 
          outputStatus != Status::OUTPUT_WRITTEN ) {
         return( Status::FAILURE );
      }

      //
      // If this was the end of the data, finish
      //
      //////////////////////////////////////////////
      // COMBINE THIS WITH SIMILAR IF STATEMENT
      // ABOVE WHEN WE MOVE TO LDATA INFO STUFF
      /////////////////////////////////////////////
      if( inputStatus == Status::END_OF_DATA ) {
	 return( outputStatus );
      }
   }
   
   return( Status::FAILURE );

}
   











