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
///////////////////////////////////////////////////////
// Ingester - class that manages data ingest and 
//            writing data to output
//
// Jaimi Yee, RAP, NCAR, Boulder, CO, 80307, USA
// September 2004
//
// $Id: Ingester.cc,v 1.35 2016/03/07 01:23:10 dixon Exp $
//
//////////////////////////////////////////////////////
#include <toolsa/pmu.h>
#include <rapformats/swap.h>
#include <toolsa/MsgLog.hh>
#include "Driver.hh"
#include "Ingester.hh"
#include "Params.hh"
#include "SweepData.hh"

Ingester::Ingester() 
{
   firstBeam      = true;
   fileStart      = 0;
   fileEnd        = 0;
   prevElevNumber = -1;
   currentSweep   = NULL;
   prevSweep      = NULL;
   ncOutput       = NULL;
}

Ingester::~Ingester() 
{
   delete currentSweep;
   delete prevSweep;
   delete ncOutput;
}

Status::info_t Ingester::init( Params& params ) 
{
   //
   // Create ncOutput object and initialize it
   //
   ncOutput = new NcOutput( params );
   
   if( ncOutput->init() != Status::ALL_OK ) {
      return( Status::FAILURE );
   }

   //
   // Set up the sweeps
   //
   PMU_auto_register( "Setting up sweeps" ); 

   currentSweep = new SweepData( params );
   prevSweep    = new SweepData( params );

   return( Status::ALL_OK );
}

Status::info_t Ingester::addBeam( ui08* buffer, bool volumeTitleSeen ) 
{
   //
   // Initialize the status
   //
   Status::info_t status = Status::ALL_OK;
   
   //
   // Get a pointer to the header
   //
   RIDDS_data_hdr *nexradData = (RIDDS_data_hdr*) buffer;
   BE_to_RIDDS_data_hdr( nexradData );

   //
   // Find the time associated with this beam
   //
   time_t beamTime = ((nexradData->julian_date - 1) * 86400 +
                      nexradData->millisecs_past_midnight / 1000);

   //
   // If this is the first beam we are reading...
   //
   if( firstBeam ) {

      fileStart = time( NULL );

      PMU_auto_register( "First beam" );

      if( handleFirstBeam( nexradData, beamTime ) != Status::ALL_OK ) {
         return( Status::FAILURE );
      }

      return( Status::ALL_OK );
      
   }

   //
   // If this is the beginning of a new tilt
   //
   if( nexradData->elev_num != prevElevNumber ) {

      //
      // If we have at least some of the data from the fields
      // we want...
      //
      if( currentSweep->sweepIsComplete() ) {

         //
         // Write the current file out and clear the data arrays
         //
         PMU_auto_register( "Writing sweep" );

         if( ncOutput->writeFile( currentSweep ) != 0 ) {
            return( Status::FAILURE );
         }

         //
         // How long did it take to create this file?
         //
         fileEnd = time( NULL );
         POSTMSG( DEBUG, "Process time = %d seconds", fileEnd - fileStart );
         
         //
         // Tell the driver that we have written a file
         //
         status = Status::OUTPUT_WRITTEN;

         //
         // If the previous sweep was not merged, write
         // out a warning
         //
         if( currentSweep->sweepLost() ) {
            POSTMSG( WARNING, "Sweep number %d was not merged", 
                     prevSweep->getElevIndex() );
         }

         //
         // Clean up previous sweep, in case we used it
         //
         prevSweep->clear();

         //
         // Clear out data from the current sweep to get ready
         // for new data
         //
         currentSweep->clear();

         //
         // Keep track of how long it takes to create next file
         //
         fileStart = time( NULL );

         //
         // Increment the sweep number and set the base time.
         // Note that we would want to do this if it was the
         // first beam as well, but that is done above, so it
         // is not necessary to repeat it here.
         //
         ncOutput->setBaseTime( beamTime, 
                                nexradData->millisecs_past_midnight );
         
         //
         // Increment the volume number and set the start of volume
         // time.  Note that if this is the first beam and also the
         // start of a volume, the volume start time will have already
         // been set above anyway.
         //
         if( nexradData->radial_status == 3 || volumeTitleSeen ||
             nexradData->elev_num < prevElevNumber ) {
            ncOutput->setVolStartTime( beamTime );
         }

      }
      else {

         //
         // We need to use data from this sweep to complete the
         // next sweep.  Set the current sweep to the previous
         // and start filling the next sweep.
         //
         PMU_auto_register( "Swapping current and previous sweeps" );

         SweepData *temp = prevSweep;
         
         prevSweep    = currentSweep;
         currentSweep = temp;

         currentSweep->clear();

         POSTMSG( DEBUG, "Setting current sweep to previous and "
                  "setting up current sweep for new input" );
         
      }

      //
      // Set tilt info 
      //
      PMU_auto_register( "Set tilt info" );

      Status::info_t ret = currentSweep->setInfo( nexradData, prevSweep );

      if( ret != Status::ALL_OK ) {
         POSTMSG( ERROR, "Could not get information needed to process "
                  "the sweep" );
         return( Status::FAILURE );
         
      }

      //
      // Keep track of the elevation number for the next pass
      //
      prevElevNumber = nexradData->elev_num;
   }
   
   //
   // Put the data into the sweep
   //
   PMU_auto_register( "Copying data into current sweep" );

   if( currentSweep->copyData( nexradData, beamTime ) != Status::ALL_OK ) {
      return( Status::FAILURE );
   }

   return( status );

}


Status::info_t Ingester::handleFirstBeam( RIDDS_data_hdr* nexradData,
                                          time_t beamTime ) 
{
   //
   // Set times for output file
   //
   PMU_auto_register( "Setting times" );

   ncOutput->setVolStartTime( beamTime );
   ncOutput->setBaseTime( beamTime, nexradData->millisecs_past_midnight );

   //
   // Set information related to the tilt
   //
   if( currentSweep->setInfo( nexradData ) != Status::ALL_OK ) {
      return( Status::FAILURE );
   }

   //
   // Copy the data into the sweep
   //
   PMU_auto_register( "Copying data into current sweep" );

   if( currentSweep->copyData( nexradData, beamTime ) != Status::ALL_OK ) {
      return( Status::FAILURE );
   }

   //
   // Set flags for next message
   //
   firstBeam      = false;
   prevElevNumber = nexradData->elev_num;

   return( Status::ALL_OK );
}


Status::info_t Ingester::endOfData() 
{
  
   PMU_auto_register( "Writing remaining data" );

   if( ncOutput->writeFile( currentSweep ) != 0 ) {
      return( Status::FAILURE );
   }

   fileEnd = time( NULL );
   POSTMSG( DEBUG, "Process time = %d seconds", fileEnd - fileStart );

   return( Status::OUTPUT_WRITTEN );
}





