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
//  Class for managing the flow of data: shift, input, reformat, output
//
//  Terri Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  November 2001
//
//  $Id: DataMgr.cc,v 1.12 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////

#include <toolsa/pmu.h>

#include "Driver.hh"
#include "NexradLdm.hh"
#include "NexradTCP.hh"
#include "NexradTape.hh"
#include "DataMgr.hh"
using namespace std;


DataMgr::DataMgr()
 : inputRadarFlags  ( inputRadarMsg.getRadarFlags()   ),
   inputRadarParams ( inputRadarMsg.getRadarParams()  ),
   inputRadarBeam   ( inputRadarMsg.getRadarBeam()    ),
   inputRadarFields ( inputRadarMsg.getFieldParams()  ),

   outputRadarFlags ( outputRadarMsg.getRadarFlags()  ),
   outputRadarParams( outputRadarMsg.getRadarParams() ),
   outputRadarBeam  ( outputRadarMsg.getRadarBeam()   ),
   outputRadarFields( outputRadarMsg.getFieldParams() )
{
   inputStream           = NULL;
   inputParamsChanged    = false;
   outputParamsChanged   = false;
   inputFlagsChanged     = false;
   outputFlagsChanged    = false;
   inputStatus           = Status::ALL_OK;
   outputStatus          = Status::ALL_OK;
   firstCall             = true;
}

DataMgr::~DataMgr()
{
   delete inputStream;

   inputRadarFields.clear();
   outputRadarFields.clear();
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
   // Initialize the input and output data streams
   // Note that the inputStream has been instantiated based on stream type
   // while the outputStream is declared as a member because
   // there is only one type
   //
   PMU_auto_register( "Initializing the data streams" );

   if ( inputStream->init( params ) != 0 )
      return( -1 );
   if ( outputStream.init( params ) != 0 )
      return( -1 );

   //
   // Initialize the reformatter
   //
   if ( reformat.init( params ) != 0 )
      return( -1 );

   //
   // Set application-specific radar characteristics that are constant
   // for WSR-88D
   //
   inputRadarParams.radarConstant  = 58.4;
   inputRadarParams.wavelength     = 10.71;  // cm
   inputRadarParams.xmitPeakPower  = 1000;   // kW
   inputRadarParams.receiverMds    = -113.0; // dBm
   inputRadarParams.horizBeamWidth = 0.95;   // deg
   inputRadarParams.vertBeamWidth  = 0.95;   // deg
   inputRadarParams.antennaGain    = 45.;    // dB
   inputRadarParams.receiverGain   = 0.;     // dB
   inputRadarParams.systemGain     = 0.;     // dB

   inputRadarParams.polarization   = DS_POLARIZATION_HORIZ_TYPE;
   inputRadarParams.radarType      = DS_RADAR_GROUND_TYPE;
   inputRadarParams.scanMode       = DS_RADAR_SURVEILLANCE_MODE;

   //
   // Set user-specified radar characteristics that are constant
   // These characteristics are not available in the radar stream
   //
   inputRadarParams.latitude       = params.radar_location.latitude;
   inputRadarParams.longitude      = params.radar_location.longitude;
   inputRadarParams.altitude       = params.radar_location.altitude;
   inputRadarParams.radarId        = params.radar_id;
   inputRadarParams.samplesPerBeam = params.samples_per_beam;

   string fullName = params.radar_name;
   fullName += " at ";
   fullName += params.site_name;
   inputRadarParams.radarName = fullName;

   inputParamsChanged = true;
   inputFlagsChanged  = true;

   return( 0 );
}

Status::info_t
DataMgr::processData()
{
   //
   // The DataMgr maintains two radar messages -- inputRadarMsg and 
   // outputRadarMsg, so that we can set flags on the outgoing radar message
   // (such as endOfTilt) based on the status of the incoming radar message.
   // Thus, On the first call we load up two radar messages.
   // The data flow looks like:
   //
   //   data        nexrad        input          output          output
   //  stream  -->  buffer  -->  RadarMsg  -->  RadarMsg  -->  RadarQueue
   //     |           |             |______________|               |
   //     |           |                     |                      |
   //  read via    converted             buffered               written
   // NexradInput  to Dsr in                in                   from
   //  sub-class    Reformat              DataMgr              DsrOutput
   //
   // On subsequent calls (not the first) we have to shift 
   // the inputRadarMsg to the outputRadarMsg position before
   // setting new inputRadarMsg values from the input stream.
   //
   ui08   *buffer;
   bool    volumeTitleSeen;

   PMU_auto_register( "Begin processing radar data" );

   //
   // Priming the pump for the first call
   //
   if ( firstCall ) {
      POSTMSG( DEBUG, "Priming the pump on the input stream" );

      firstCall = false;
      inputStatus = inputStream->readNexradMsg( buffer, volumeTitleSeen );

      if ( inputStatus == Status::ALL_OK ) {
         inputStatus = reformat.nexrad2dsr( buffer, inputRadarMsg,
                                            volumeTitleSeen );
      }
   }

   //
   // Continuous processing steps: shift, read, reformat, write
   //
   while( true ) {

      //
      // First shift the I/O status to see if we should continue
      //
      outputStatus = inputStatus;
      if ( outputStatus != Status::ALL_OK ) {
         return( outputStatus );
      }

      //
      // Shift the inputRadarMsg to the outputRadarMsg position
      //
      shiftRadarMsg();

      //
      // Get a new inputRadarMsg
      //
      PMU_auto_register( "Reading a new input message" );
      inputStatus = inputStream->readNexradMsg( buffer, volumeTitleSeen );

      //
      // Reformat the input buffer
      //
      if ( inputStatus == Status::ALL_OK ) {

         PMU_auto_register( "Reformatting the new input message" );
         inputParamsChanged = false;

         inputStatus = reformat.nexrad2dsr( buffer, inputRadarMsg,
                                            volumeTitleSeen );

         if ( inputStatus == Status::ALL_OK ) {
            //
            // Update output radar flags, based on new input radar flags
            //
            if ( inputRadarFlags.startOfVolume ) {
               outputRadarFlags.endOfVolume = true;
               outputFlagsChanged = true;
            }
            if ( inputRadarFlags.startOfTilt ) {
               outputRadarFlags.endOfTilt = true;
               outputFlagsChanged = true;
            }

            //
            // See if the radar parameters or flags have changed
            //
            if ( inputRadarParams != outputRadarParams ) {
               inputParamsChanged = true;
               
            } 
            if ( inputRadarFlags != outputRadarFlags ) {
               inputFlagsChanged = true;
            }
         }
      }

      //
      // Write out the output radar message if there are any writable fields
      // This one goes out the door even if the input status is not all ok
      //
      if ( outputRadarFields.size() > 0 ) {
         PMU_auto_register( "Writing message to output radar queue" );
         outputStatus = outputStream.writeDsrMsg( outputRadarMsg, 
                                                  outputFlagsChanged,
                                                  outputParamsChanged );
      }
   }

   //
   // We should never get here
   //
   POSTMSG( ERROR, "Unexpected processing results!" );
   return( Status::FAILURE );
}

void
DataMgr::shiftRadarMsg()
{
   PMU_auto_register( "Shifting radar message" );

   //
   // DsFieldParams:
   //
   // Clear out the output field params and shift only if necessary
   // Always clear out the input field params
   //
   if ( outputRadarFields != inputRadarFields ) {
      outputRadarFields.clear();
      outputRadarFields = inputRadarFields;
   }
   inputRadarFields.clear();

   //
   // DsRadarParams:
   //
   // The radar parameter change status should always shift over.
   // But we only have to shift the actual radarParams when they change.
   //
   // NOTE: we never want to fully clear out the input radar parameters 
   // because of the persistent parameters which were
   // previously established in init().
   //
   outputParamsChanged = inputParamsChanged;
   inputParamsChanged  = false;

   if ( outputParamsChanged ) {
      outputRadarParams = inputRadarParams;
   }

   //
   // DsRadarBeam:
   //
   // The radarBeam changes every time, so we always do a shift
   // For efficiency, we don't bother to clear out the radar beam
   // since all members will be neatly overwritten.
   //
   // TODO: wholesale copies of the beam data is perhaps not an efficient
   //       technique.  maybe we could do much better by providing pointers
   //       that switch between the two radar messages.  That would have
   //       to be compared to the extra overhead of memory offset addressing   
   //
   outputRadarBeam = inputRadarBeam;

   //
   // DsRadarFlags:
   //
   // The radar flags change status should always shift over.
   // But we only have to shift the actual radarFlags when they change.
   // Make sure to completely clear out the input flags.
   //
   outputFlagsChanged = inputFlagsChanged;
   inputFlagsChanged  = false;

   if ( outputFlagsChanged ) {
      outputRadarFlags = inputRadarFlags;
   }
   inputRadarFlags.clear();
}
