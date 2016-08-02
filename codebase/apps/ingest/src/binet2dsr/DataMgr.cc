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
//  Working class for application
//
//  Terri Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  September 2001
//
//  $Id: DataMgr.cc,v 1.5 2016/03/07 01:23:07 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////

#include <cassert>

#include "Driver.hh"
#include "DataMgr.hh"
#include "Params.hh"
using namespace std;

//
// Number of radar messages written between each output of radar parameters
//
const size_t DataMgr::RADAR_PARAM_INTERVAL = 5;

const char* DataMgr::HDR = " Vol Tilt El_tgt El_act     Az Ngat Gspac  "
                           "PRF  Nyqst     Date     Time";

const char* DataMgr::FMT = "%3ld %4ld %6.2f %6.2f %6.2f %4ld %5ld "
                           "%4ld %6.2f %.2ld/%.2ld/%.2ld %.2ld:%.2ld:%.2ld";

DataMgr::DataMgr()
{
   paramCount      = 0;
   summaryCount    = 0;
   summaryInterval = 0;
}

int
DataMgr::init( Params &params, DsInputPath *trigger )
{
   PMU_auto_register( "Initilizing DataMgr" );

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

   //
   // Pass along the user parameters and input data trigger to the input stream
   //
   if ( inputStream.init( params, trigger ) != 0 ) {
      return( -1 );
   }

   //
   // Hang onto other relevant info for later
   //
   if ( params.print_summary == TRUE ) {
      summaryInterval = (size_t)params.summary_interval;
   }

   return( 0 );
}

int
DataMgr::processData()
{
   int outputStatus = 0;
   BinetNetCDF::inputStatus_t  inputStatus = BinetNetCDF::ALL_OK;

   //
   // Continuously read from the input stream
   //
   while( inputStatus == BinetNetCDF::ALL_OK  &&  outputStatus == 0 ) {

      //
      // Get the next radar message
      //
      PMU_auto_register( "Reading radar message from input stream" );
      inputStatus = inputStream.readRadarMsg();

      switch( inputStatus ) {
         case BinetNetCDF::ALL_OK:
              //
              // Write the latest radar message to the output radar queue
              //
              PMU_auto_register( "Writing radar message to output queue" );
              outputStatus = writeRadarMsg();
              break;

         case BinetNetCDF::END_OF_DATA:
              outputStatus = 0;
              break;

         default:
              outputStatus = -1;
              break;
      }
   }

   return( outputStatus );
}

int
DataMgr::writeRadarMsg()
{
   int               status = 0;
   int               outputMsgContent = 0;

   /////////////////////////////////////////////////////////////////////////////
   // Radar flags
   /////////////////////////////////////////////////////////////////////////////
   // Radar flags always get written to the output queue
   // In addition they generate separate messages to the queue
   //
   const DsRadarFlags &radarFlags = inputStream.getRadarFlags();
   outputMsgContent |= (int) DsRadarMsg::RADAR_FLAGS;

   //
   // Write newScanType message to the output queue, if necessary
   //
   if ( radarFlags.newScanType ) {
      POSTMSG( DEBUG, "Change to new scan strategy %d.",
                       radarFlags.scanType );
      status += outputRadarQueue.putNewScanType( radarFlags.scanType );
   }

   //
   // Write start of tilt/volume messages to the output queue, if necessary
   //
   if ( radarFlags.startOfVolume ) {
      POSTMSG( DEBUG, "Start of volume %d", radarFlags.volumeNum );
      status += outputRadarQueue.putStartOfVolume( radarFlags.volumeNum );
   }
   if ( radarFlags.startOfTilt ) {
      POSTMSG( DEBUG, "Start of tilt %d", radarFlags.tiltNum );
      status += outputRadarQueue.putStartOfTilt( radarFlags.tiltNum );
   }

   /////////////////////////////////////////////////////////////////////////////
   // Radar & Field Parameters
   /////////////////////////////////////////////////////////////////////////////
   // Radar and field parameters only get sent out to the queue
   // 1. whenever the parameters change
   // 2. at the beginning of each new tilt
   // 3. every fifth message, i.e. paramCount == 0
   //
   bool changes = inputStream.radarParamsChanged();
   if ( changes ) {
      POSTMSG( DEBUG, "Radar parameter settings have changed." );
   }

   if ( paramCount == 0 || radarFlags.startOfTilt || changes ) {
      outputMsgContent |= (int) DsRadarMsg::RADAR_PARAMS;
      outputMsgContent |= (int) DsRadarMsg::FIELD_PARAMS;
      paramCount = RADAR_PARAM_INTERVAL;
   }
   paramCount--;

   /////////////////////////////////////////////////////////////////////////////
   // Radar Beam and RadarMsg output
   /////////////////////////////////////////////////////////////////////////////
   //
   // The radar beam data always gets written to the output queue
   // Write the message contents to the output radar queue
   //
   outputMsgContent |= (int) DsRadarMsg::RADAR_BEAM;
   status += outputRadarQueue.putDsMsg( inputStream.getRadarMsg(), 
                                        outputMsgContent );

   /////////////////////////////////////////////////////////////////////////////
   // More Radar flags
   /////////////////////////////////////////////////////////////////////////////
   //
   // Write end of tilt/volume messages to the output queue, if necessary
   //
   if ( radarFlags.endOfTilt ) {
      POSTMSG( DEBUG, "End of tilt %d", radarFlags.tiltNum );
      status += outputRadarQueue.putEndOfTilt( radarFlags.tiltNum );
   }
   if ( radarFlags.endOfVolume ) {
      POSTMSG( DEBUG, "End of volume %d", radarFlags.volumeNum );
      status += outputRadarQueue.putEndOfVolume( radarFlags.volumeNum );
   }

   //
   // How did we fare?
   //
   if ( status == 0 ) {
      if ( summaryInterval > 0 ) {
         printSummary();
      }
   }
   else {
      POSTMSG( ERROR, "Unsuccesful write of contents to output queue." );
      return( -1 );
   }

   return( status );
}

void
DataMgr::printSummary()
{
   const DsRadarBeam &radarBeam = inputStream.getRadarBeam();
   const DsRadarParams &radarParams = inputStream.getRadarParams();

   if ( summaryCount == 0 ) {

      fprintf( stderr, HDR );
      fprintf( stderr, "\n" );

      //
      // Parse the time of the beam 
      //
      date_time_t  dataTime;
      dataTime.unix_time = radarBeam.dataTime;
      uconvert_from_utime( &dataTime );

      fprintf( stderr, FMT,
             (long)   radarBeam.volumeNum,
             (long)   radarBeam.tiltNum,
             (double) radarBeam.targetElev,
             (double) radarBeam.elevation,
             (double) radarBeam.azimuth,
             (long)   radarParams.numGates,
             (long)  (radarParams.gateSpacing * 1000),
             (long)   radarParams.pulseRepFreq,
             (double) radarParams.unambigVelocity,
             (long)   dataTime.year,
             (long)   dataTime.month,
             (long)   dataTime.day,
             (long)   dataTime.hour,
             (long)   dataTime.min,
             (long)   dataTime.sec);

      fprintf(stderr, "\n");
      summaryCount = summaryInterval;
   }

   summaryCount--;
   return;
}
