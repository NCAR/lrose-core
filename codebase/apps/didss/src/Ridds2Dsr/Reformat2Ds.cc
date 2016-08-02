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
///////////////////////////////////////////////////////////
// Reformat2Ds - object that reformats ridds data into 
//               ds radar data (dsr data)
//
// $Id: Reformat2Ds.cc,v 1.7 2016/03/06 23:53:41 dixon Exp $
//
///////////////////////////////////////////////////////////

#include "Reformat2Ds.hh"
#include "Params.hh"
#include "Ridds2Dsr.hh"
using namespace std;

Reformat2Ds::Reformat2Ds() 
{
   velocityPosition   = -1;
   gateRatio          = 1;
   nCopyAlloc         = 0;
   bufferCopy         = NULL;
   firstBeamProcessed = true;
   lastTiltNum        = 0;
   volNum             = 0;
}

Reformat2Ds::~Reformat2Ds() 
{
   delete bufferCopy;
}

void
Reformat2Ds::init( Params& params ) 
{
   timeCorrection = params.time_correction;
   
   //
   // Initialize the radar params
   //
   DsRadarParams &radarParams = radarMsg.getRadarParams();

   radarParams.radarName = params.radar_name + " at " + params.site_name;
   radarParams.radarId   = params.radar_id;
   radarParams.radarType = DS_RADAR_GROUND_TYPE;

   radarParams.numFields = 0;
   
   if( params.output_snr ) {
      radarParams.numFields++;
   }
   if( params.output_dbz ) {
      radarParams.numFields++;
   }
   if( params.output_vel ) {
      radarParams.numFields++;
   }
   if( params.output_spw ) {
      radarParams.numFields++;
   }
   
   radarParams.samplesPerBeam = params.samples_per_beam;
   radarParams.scanMode       = DS_RADAR_SURVEILLANCE_MODE;
   radarParams.polarization   = params.polarization_code;
   radarParams.radarConstant  = 58.4;
   radarParams.altitude       = (float) params.radar_location.altitude;
   radarParams.latitude       = (float) params.radar_location.latitude;
   radarParams.longitude      = (float) params.radar_location.longitude;
   radarParams.horizBeamWidth = (float) params.beam_width;
   radarParams.vertBeamWidth  = (float) params.beam_width;
   radarParams.wavelength     = (float) params.wavelength;
   radarParams.xmitPeakPower  = params.peak_xmit_pwr;
   radarParams.receiverMds    = params.receiver_mds;
   radarParams.receiverGain   = params.receiver_gain;
   radarParams.antennaGain    = params.antenna_gain;
   radarParams.systemGain     = params.system_gain;

   //
   // Initialize the field params
   //
   vector< DsFieldParams* > &fieldParams = radarMsg.getFieldParams();
   
   DsFieldParams *fparams;
   
   if( params.output_snr ) {
      fparams = new DsFieldParams( "SNR", "dB",
                                   ((float) DBZ/SCALE)/
                                   ((float) LL_SCALE_AND_BIAS_MULT),
                                   ((float) DBZ_BIAS)/
                                   ((float) LL_SCALE_AND_BIAS_MULT) );
      
      fieldParams.push_back( fparams );
      velocityPosition++;
   }
   
   if( params.output_dbz ) {
      fparams = new DsFieldParams( "DBZ", "dBZ",
                                   ((float) DBZ/SCALE)/
                                   ((float) LL_SCALE_AND_BIAS_MULT),
                                   ((float) DBZ_BIAS)/
                                   ((float) LL_SCALE_AND_BIAS_MULT) );
      fieldParams.push_back( fparams );
      velocityPosition++;
   }
   
   if( params.output_vel ) {
      fparams = new DsFieldParams( "VEL", "m/s",, 
				 ((float) VEL_SCALE_1)/
				 ((float) LL_SCALE_AND_BIAS_MULT), 
				 ((float) VEL_BIAS_1)/
				 ((float) LL_SCALE_AND_BIAS_MULT) );
      fieldParams.push_back( fparams );
      velocityPosition++;
   }
   else {
      velocityPosition = -1;
   }
   

   if( params.output_spw ) {
      fparams = new DsFieldParams( "SPW", "m/s",
				   ((float) SW_SCALE)/
				   ((float) LL_SCALE_AND_BIAS_MULT), 
				   ((float) SW_BIAS)/
				   ((float) LL_SCALE_AND_BIAS_MULT) );
      fieldParams.push_back( fparams );
   }
   

   //
   // Set ratio between reflectivity and velocity data resolution
   //
   if ( params.input_stream_format == Params::NEXRAD_FORMAT ) {
      gateRatio = 4;
   }
   else {
      gateRatio = 1;
   }
   
   //
   // Initialize moments calculations
   //
   initMoments( params.noise_dbz_at_100km );
   
}

int
Reformat2Ds::reformat( ui08* readBuffer, int nread, NEXRAD_vcp_set* vcpSet,
                       ui08* writeBuffer, int& nwrite ) 
{

   RIDDS_data_hdr *riddsData;
   ui08           *output_buffer = NULL;

   //
   // Make a copy of the buffer
   //
   if( nCopyAlloc < nread ) {
      delete bufferCopy;
      bufferCopy = new ui08[nread];
      nCopyAlloc = nread;
   }
   memcpy( bufferCopy, readBuffer, nread );

   //
   // Swap header
   //
   riddsData = (RIDDS_data_hdr *) bufferCopy;
   BE_to_RIDDS_data_hdr( riddsData );

   if( processBeam( riddsData, &outputBuffer, vcpSet, nwrite ) ) {
      return( -1 );
   }
   
   *writeBuffer = outputBuffer;
   return( 0 );
}

int
Reformat2Ds::processBeam( RIDDS_data_hdr *riddsData,
                           ui08 **outputBuffer,
                           NEXRAD_vcp_set *volCntrlPatterns,
                           int *nwrite )
{

  bool newTilt = false;
  bool newVol  = false;
  int  hour    = 0;
  int  min     = 0;
  int  seconds = 0;

  //
  // Get the data time
  //
  date_time_t ttime;
  get_beam_time( riddsData, timeCorrection, &ttime );                  

  //
  // Determine if this is the start of a new 360 degree tilt 
  //
  if ((riddsData->radial_status == START_OF_NEW_ELEVATION) ||
      (riddsData->radial_status == BEGINNING_OF_VOL_SCAN) ||
      (riddsData->elev_num != last_tilt_num) ||
      firstBeamProcessed ) {

     POSTMSG( DEBUG, "radial status = %d, elev number = %d\n",
              riddsData->radial_status, riddsData->elev_num );

     //
     // Handle start/end-of-tilt condition 
     //
     if( !firstBeamProcessed ) {
        end_of_tilt( lastTiltNum );
     }

     //
     // Handle start/end-of-volume condition 
     //
     if( riddsData->radial_status == BEGINNING_OF_VOL_SCAN
         && riddsData->elev_num == 1 ) {

      if( !firstBeamProcessed ) {
         end_of_volume( volNum );
      }

      volNum++;
      newVol = true;
      start_of_volume( volNum, ttime.unix_time );

      if( DEBUG_ENABLED ) {
        seconds = riddsData->millisecs_past_midnight / 1000;
        hour    = seconds / 3600;
        min     = (seconds - hour * 3600) / 60;

        POSTMSG( DEBUG, "\n*********************************************\n");
        POSTMSG( DEBUG, "New Volume %2d :%2d :%2d", hour, min,
                 seconds - hour * 3600 - min * 60);
        POSTMSG( DEBUG, " - NEXRAD radar\n\n");
      }
     }

     newTilt = true;
     start_of_tilt( riddsData->elev_num, ttime.unix_time );
     if( firstBeamProcessed ) {
        firstBeamProcessed = false;
     }
  }
  
  //
  // Remember what tilt we're on 
  //
  lastTiltNum = riddsData->elev_num;

  //
  // Load up the ds message buffer 
  //
  if( loadDsMessage( riddsData, outputBuffer, nwrite,
                     volNum, newVol, riddsData->elev_num, new_tilt,
                     ttime.unix_time, volCntrlPatterns ))
     return( -1 );

  return( 0 );
}


static int loadDsMessage( RIDDS_data_hdr *riddsData,
                          ui08 **outputBuffer, int *nwrite, 
                          int volumeNum, int newVolume,
                          short tiltNum, int newTilt,
                          time_t dataTime, NEXRAD_vcp_set *volCntrlPatterns )
{
  static si16   prevVCP;
  static bool   firstCall        = true;
  static time_t lastParamsUpdate = 0;

  unsigned short newAzimuth;
  time_t         timeSinceUpdate;
  int            iret1       = 0;
  int            iret2       = 0;
  int            msgContent  = 0;
  ui08*          momentsData = NULL;

  DsRadarBeam   &radarBeam   = radarMsg->getRadarBeam();
  DsRadarParams &radarParams = radarMsg->getRadarParams();

  //
  // Send radar and field parameters every 5 secs
  //
  timeSinceUpdate = dataTime - lastParamsUpdate;
  if ( firstCall || newTilt || timeSinceUpdate > 5) {
    updateDynamicParams( riddsData );
    msgContent |= (int)DsRadarMsg::RADAR_PARAMS;
    msgContent |= (int)DsRadarMsg::FIELD_PARAMS;
    lastParamsUpdate = dataTime;
  }

  //
  // Set beam data every time
  //
  msgContent |= (int)DsRadarMsg::RADAR_BEAM;
  radarBeam.dataTime      = dataTime;
  radarBeam.volumeNum     = volumeNum;
  radarBeam.tiltNum       = (int)tiltNum;

  radarBeam.azimuth       = (((float)riddsData->azimuth / 8.) *
                            (180. / 4096. ));

  radarBeam.elevation     = (((float)riddsData->elevation / 8.) *
                            (180. / 4096. ));

  radarBeam.targetElev    = get_target_elev( &riddsData->vol_coverage_pattern,
                                             &tiltNum, 
                                             vol_cntrl_patterns ) / 100.;

  if ( firstCall || riddsData->vol_coverage_pattern != prevVCP ) {
    new_scan_type(riddsData->vol_coverage_pattern);
    prevVCP = riddsData->vol_coverage_pattern;
  }
  firstCall = false;

  //
  // Convert the ridds data to moments data                                 
  //            

  int  dataLen = radarParams.numFields*radarParams.numGates;

  if ( dataLen ) {
    momentsData = new ui08[dataLen];
    newAzimuth = (unsigned short)(radarBeam.azimuth * 100 + 0.5);
    iret1 = store_data( riddsData, momentsData, &newAzimuth, newVolume );
    radarBeam.azimuth = newAzimuth / 100.;
  }                                        

  radarBeam.loadData(momentsData, dataLen);

  //
  // Now encode the radar message into the output buffer
  //
  *outputBuffer = Glob->radarMsg->assemble( msgContent );
  *nwrite = Glob->radarMsg->lengthAssembled();

  if ( momentsData )
    delete []momentsData;

  if (iret1 || iret2) {
    return (-1);
  } else {
    return (0);
  }

}
