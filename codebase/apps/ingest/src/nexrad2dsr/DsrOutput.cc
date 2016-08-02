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
//  Class for Dsr output stream
//
//  Terri Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  November 2001
//
//  $Id: DsrOutput.cc,v 1.19 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <toolsa/pmu.h>
#include <toolsa/uusleep.h>
#include <toolsa/DateTime.hh>

#include "Driver.hh"
#include "DsrOutput.hh"
using namespace std;

//
// Number of radar messages written between each output of radar parameters
//
const size_t DsrOutput::RADAR_PARAM_INTERVAL = 5;

const char* DsrOutput::HDR = " VCP Vol Tilt El_tgt El_act     Az Ngat Gspac  "
                             "PRF  Nyqst     Date     Time";

const char* DsrOutput::FMT = "%4ld %3ld %4ld %6.2f %6.2f %6.2f %4ld %5ld "
                           "%4ld %6.2f %s";

DsrOutput::DsrOutput()
{
   waitMsec        = 0;   
   paramCount      = 0;
   summaryCount    = 0;
   summaryInterval = 0;
   filter1KmDbz    = false;
   oneFilePerVolume = false;
   firstRotationOnly = false;
   paramsOnFilteredTilt = true;
   azTol = 3.0;
   numBeamsPostRotation = 10;
   currentElevation = -1000.0;
   movedAlittle = false;
   startAzimuth = 0.0;
   firstRotationDone = false;
   icount = 0;
   currentNumGates = 0;
   currentGateSpacing = 0.0;
   return;
}

DsrOutput::~DsrOutput(){
  if (oneFilePerVolume){
     outputRadarQueue.putEndOfVolume( 0 );
  }
  return;
}

int
DsrOutput::init( Params &params )
{
   PMU_auto_register( "Initilizing DsrOutput" );
   POSTMSG( DEBUG, "Initializing the output radar queue" );

   oneFilePerVolume = params.oneFilePerVolume;
   firstRotationOnly = params.firstRotationOnly;
   azTol = params.azTol;
   numBeamsPostRotation = params.numBeamsPostRotation;
   
   //
   // Initialize the output radar queue
   //
   if ( outputRadarQueue.init( params.output_fmq_url,
                               PROGRAM_NAME,
                               DEBUG_ENABLED,
                               DsFmq::READ_WRITE, DsFmq::END,
                               (bool) params.output_fmq_compress,
                               params.output_fmq_nslots,
                               params.output_fmq_size,
                               1000, &(driver->getMsgLog()) )) {
      POSTMSG( ERROR, "Could not initialize radar queue '%s'",
               params.output_fmq_url );
      return( -1 );
   }
   if (params.mode == Params::ARCHIVE) {
     outputRadarQueue.setBlockingWrite();
   }

   //
   // Hang onto other relevant info for later
   //
   waitMsec  = params.beam_wait;
   filter1KmDbz = params.filter_1km_dbz;
   separateFlags = params.separate_fmq_flags;
   paramsOnFilteredTilt = params.params_on_filtered_tilts;

   if ( params.print_summary == TRUE ) {
      summaryInterval = (size_t)params.summary_interval;
   }

   return( 0 );
}

Status::info_t
DsrOutput::writeDsrMsg( DsRadarMsg& radarMsg, 
                        bool flagsChanged, bool paramsChanged )
{
   int   outputMsgContent = 0;
   bool  outputBeamData = true;
   const DsRadarFlags &radarFlags = radarMsg.getRadarFlags();
   int   status = 0;

   //
   // Write separate start of tilt/volume messages, if necessary
   //
   if ( separateFlags ) {
      if ( radarFlags.newScanType ) {
         POSTMSG( DEBUG, "Change to new scan strategy %d.",
                          radarFlags.scanType );
         status += outputRadarQueue.putNewScanType( radarFlags.scanType );
      }
      if ( radarFlags.startOfVolume ) {
         POSTMSG( DEBUG, "Start of volume %d", radarFlags.volumeNum );
         status += outputRadarQueue.putStartOfVolume( radarFlags.volumeNum );
      }
      if ( radarFlags.startOfTilt ) {
         POSTMSG( DEBUG, "Start of tilt %d", radarFlags.tiltNum );
         status += outputRadarQueue.putStartOfTilt( radarFlags.tiltNum );
      }
   }

   //
   // Add the flags to our outgoing contents only when they change
   //
   if ( flagsChanged ) {
      outputMsgContent |= (int) DsRadarMsg::RADAR_FLAGS;
   }

   //
   // The radar beam data gets added to the message contents
   // as long as we are not filtering out the 1km data
   // NOTE: the filtering of 1Km data is fairly inefficient and was added
   //       for JamesD which is having a hard time with varying numbers
   //       of fields.  Once JamesD is updated to handle this 1Km data,
   //       we should consider removing the filtering option
   //
   if ( filter1KmDbz ) {
      outputBeamData = false;
      vector<DsFieldParams*> &dsrFields = radarMsg.getFieldParams();
      for( size_t i=0; i < dsrFields.size() && !outputBeamData; i++ ) {
         if ( dsrFields[i]->name ==
              FieldTable::fieldTypeInfo[FieldTable::VEL_DATA].name ) {
            outputBeamData = true;
            outputMsgContent |= (int) DsRadarMsg::RADAR_BEAM;
         }
      }
   }
   else {
      //
      // no filtering, add beam data to the contents
      //
      outputBeamData = true;
      outputMsgContent |= (int) DsRadarMsg::RADAR_BEAM;
   }

   //
   // Radar and field parameters only become part of the outgoing contents
   // 1. when the parameters change
   // 2. at the beginning of each new tilt
   // 3. every fifth message, i.e. paramCount == 0
   //
   if ( paramCount == 0 || radarFlags.startOfTilt || paramsChanged ) {
      outputMsgContent |= (int) DsRadarMsg::RADAR_PARAMS;
      outputMsgContent |= (int) DsRadarMsg::FIELD_PARAMS;
      paramCount = RADAR_PARAM_INTERVAL;
   }
   paramCount--;


   //
   // Start of Volume/Tilt debug messages
   //
   if ( DEBUG_ENABLED  &&  outputMsgContent & DsRadarMsg::RADAR_FLAGS 
                       && !separateFlags ) {
      if ( radarFlags.startOfVolume ) {
         POSTMSG( DEBUG, "Start of volume %d", radarFlags.volumeNum );
      }
      if ( radarFlags.startOfTilt ) {
         POSTMSG( DEBUG, "Start of tilt %d", radarFlags.tiltNum );
      }
   }

   //
   // Logic pertaining to only sending the first rotation.
   //
   bool ok2send = true;
   if (firstRotationOnly && outputBeamData){
     //
     // Get access to the beam data.
     //
     const DsRadarBeam &radarBeam = radarMsg.getRadarBeam();
     const DsRadarParams &radarParams = radarMsg.getRadarParams();
     //
     // If the elevation has changed, record the azimuth.
     //
     if (currentElevation != radarBeam.targetElev){
       currentElevation = radarBeam.targetElev;
       ok2send = true;
       movedAlittle = false;
       firstRotationDone = false;
       startAzimuth = radarBeam.azimuth;
       icount = 0;
       POSTMSG( DEBUG, "Rotation for elevation %g started at azimuth %g",
		currentElevation, startAzimuth);
     } else {
       //
       // The elevation has not changed. See if the antenna has
       // moved a little from the start.
       //
       if ( fabs(sin(3.1415927*radarBeam.azimuth/180.0) - sin(3.1415927*startAzimuth/180.0)) > sin(3.1415927*3.0*azTol/180.0)){
	 movedAlittle = true;
       }
       //
       // If the antenna has moved, see if it has come around to
       // close to the start azimuth.
       //
       if (!(firstRotationDone) && movedAlittle){
	 if (
	     ( fabs(sin(3.1415927*radarBeam.azimuth/180.0) - sin(3.1415927*startAzimuth/180.0)) < sin(3.1415927*azTol/180.0)) &&
	     ( fabs(cos(3.1415927*radarBeam.azimuth/180.0) - cos(3.1415927*startAzimuth/180.0)) < sin(3.1415927*azTol/180.0))
	     ){
	   firstRotationDone = true;
	   POSTMSG( DEBUG, "First rotation completed at azimuth %g (started at %g)",
		    radarBeam.azimuth, startAzimuth);
	 }
       }
       //
       // If we have not yet completed a rotation, record the beam geomtery.
       // We use this once we have completed a rotation as an exit criteria.
       //
       if (!(firstRotationDone)){
	 currentNumGates = radarParams.numGates;
	 currentGateSpacing = radarParams.gateSpacing;
       }
       //
       // If we have completed the first rotation, there are three reasons
       // to stop sending beam data :
       // [1] we reach the count of beams to send after rotation in the param file;
       // [2] the beam geometry changes
       // [3] we go to another target elevation (dealt with above).
       //
       if (firstRotationDone){
	 icount++;
	 if (icount > numBeamsPostRotation){
	   ok2send = false;
	   icount = numBeamsPostRotation + 1;
	 }
	 //
	 //
	 if (
	     (currentNumGates != radarParams.numGates) ||
	     (currentGateSpacing != radarParams.gateSpacing)
	     ){
	   ok2send = false;
	 }
       }
     }
   }


   //
   // Write the RadarMsg and message contents to the output radar queue
   //
   if ((ok2send) && 
       (paramsOnFilteredTilt || outputBeamData) && 
       (outputMsgContent) ) {
      status += outputRadarQueue.putDsMsg( radarMsg, outputMsgContent );
   }

   //
   // Summary beam data messages
   //
   if ( !outputBeamData ) {
      if ( DEBUG_ENABLED  &&  radarFlags.startOfTilt ) {
         POSTMSG( DEBUG, "Filtering out 1Km beam data in tilt %d",
                         radarFlags.tiltNum );
      }
   }
   else {
      if ((ok2send) && (summaryInterval > 0 )) {
         printSummary( radarMsg );
      }
   }

   //
   // Write separate start of tilt/volume messages, if necessary
   //
   if ( separateFlags ) {
      if ( radarFlags.endOfTilt ) {
         POSTMSG( DEBUG, "End of tilt %d", radarFlags.tiltNum );
         status += outputRadarQueue.putEndOfTilt( radarFlags.tiltNum );
      }
      if ( radarFlags.endOfVolume ) {
         POSTMSG( DEBUG, "End of volume %d", radarFlags.volumeNum );
         status += outputRadarQueue.putEndOfVolume( radarFlags.volumeNum );
      }
   }
   else {
      //
      // End of Volume/Tilt debug messages
      //
      if ( DEBUG_ENABLED  &&  outputMsgContent & DsRadarMsg::RADAR_FLAGS ) {
         if ( radarFlags.endOfTilt ) {
            POSTMSG( DEBUG, "End of tilt %d", radarFlags.tiltNum );
         }
         if ( radarFlags.endOfVolume ) {
            POSTMSG( DEBUG, "End of volume %d", radarFlags.volumeNum );
         }
      }
   }

   //
   // That's it
   //
   if ( status != 0 ) {
      POSTMSG( ERROR, "Unsuccesful write of contents to output queue." );
      return( Status::BAD_OUTPUT_STREAM );
   }
   if ( waitMsec ) {
      umsleep( waitMsec );
   }

   return( Status::ALL_OK );
}

void
DsrOutput::printSummary( DsRadarMsg& radarMsg )
{
   const DsRadarBeam &radarBeam = radarMsg.getRadarBeam();
   const DsRadarParams &radarParams = radarMsg.getRadarParams();

   if ( summaryCount == 0 ) {

      fprintf( stderr, HDR );
      fprintf( stderr, "\n" );

      //
      // Parse the time of the beam 
      //
      DateTime when( radarBeam.dataTime );

      fprintf( stderr, FMT,
             (long)   radarParams.scanType,
             (long)   radarBeam.volumeNum,
             (long)   radarBeam.tiltNum,
             (double) radarBeam.targetElev,
             (double) radarBeam.elevation,
             (double) radarBeam.azimuth,
             (long)   radarParams.numGates,
             (long)  (radarParams.gateSpacing * 1000),
             (long)   radarParams.pulseRepFreq,
             (double) radarParams.unambigVelocity,
                      when.dtime() );

      fprintf(stderr, "\n");
      summaryCount = summaryInterval;
   }

   summaryCount--;
   return;
}
