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
/***************************************************************************
 * reformat2ds.c
 *
 * Reformat the data stream into DsRadar format
 * (modelled after reformat2ll.c)
 *
 * Terri Betancourt RAP NCAR Boulder CO USA
 *
 * Feb 1998
 *
 **************************************************************************/

#include <rapformats/ds_radar.h>
#include <rapformats/swap.h>
#include "ridds2mom.h"
using namespace std;

/**********************
 * file scope variables
 */
static int Debug;
static si32 Time_correction;
static int VelPosn;

/************************************************************************/

/*
 * file scope prototypes
 */

static int process_beam(RIDDS_data_hdr * riddsData,
                        ui08 ** output_buffer,
                        NEXRAD_vcp_set * vol_cntrl_patterns,
                        int *nwrite);

static int loadDsMessage( RIDDS_data_hdr *riddsData,
                          ui08 **outputBuffer, int *nwrite,
                          int volumeNum, int newVolume,
                          short tiltNum, int newTilt,
                          time_t dataTime, NEXRAD_vcp_set *vol_cntrl_patterns );

static void updateDynamicParams( RIDDS_data_hdr *riddsData );

/*********************************
 * initialize file scope variables
 */

void 
init_reformat2ds(char *radar_name,
		 int radar_id,
		 char *site_name,
		 double latitude,
		 double longitude,
		 double altitude,
		 int time_correction,
		 int polarization,
		 double wavelength,
		 double beam_width,
		 double peak_xmit_pwr,
		 double receiver_mds,
		 double noise_dbz_at_100km,
		 int samples_per_beam,
		 double receiver_gain,
		 double antenna_gain,
		 double system_gain,
		 int debug)
{
  Debug = debug;
  Time_correction = time_correction;

  //
  // Initialize constant radar parameters
  //
  DsRadarParams &radarParams = Glob->radarMsg->getRadarParams();

  char fullName[1024];
  sprintf( fullName, "%s at %s", radar_name, site_name );
  radarParams.radarName = fullName;

  radarParams.radarId           = radar_id;

  radarParams.radarType         = DS_RADAR_GROUND_TYPE;
  radarParams.numFields         = 0;
  if (Glob->params.output_ds_snr) {
    radarParams.numFields++;
  }
  if (Glob->params.output_ds_dbz) {
    radarParams.numFields++;
  }
  if (Glob->params.output_ds_vel) {
    radarParams.numFields++;
  }
  if (Glob->params.output_ds_spw) {
    radarParams.numFields++;
  }
  radarParams.samplesPerBeam    = samples_per_beam;

  radarParams.scanMode          = DS_RADAR_SURVEILLANCE_MODE;
  radarParams.polarization      = polarization;
  radarParams.radarConstant     = 58.4;
  radarParams.altitude          = (float)altitude;
  radarParams.latitude          = (float)latitude;
  radarParams.longitude         = (float)longitude;
  radarParams.horizBeamWidth    = (float)beam_width;
  radarParams.vertBeamWidth     = (float)beam_width;
  radarParams.wavelength        = (float)wavelength;

  radarParams.xmitPeakPower     = peak_xmit_pwr;
  radarParams.receiverMds       = receiver_mds;
  radarParams.receiverGain      = receiver_gain;
  radarParams.antennaGain       = antenna_gain;
  radarParams.systemGain        = system_gain;

  //
  // Initialize output field parameters
  //
  vector< DsFieldParams* > &fieldParams = Glob->radarMsg->getFieldParams();
  
  DsFieldParams*  fparams;
  VelPosn = -1;
  
  if (Glob->params.output_ds_snr) {
    fparams = new DsFieldParams( "SNR", "dB", 
				 ((float) DBZ_SCALE)/
				 ((float) LL_SCALE_AND_BIAS_MULT), 
				 ((float) DBZ_BIAS)/
				 ((float) LL_SCALE_AND_BIAS_MULT) );
    fieldParams.push_back( fparams );
    VelPosn++;
  }

  if (Glob->params.output_ds_dbz) {
    fparams = new DsFieldParams( "DBZ", "dBZ", 
				 ((float) DBZ_SCALE)/
				 ((float) LL_SCALE_AND_BIAS_MULT), 
				 ((float) DBZ_BIAS)/
				 ((float) LL_SCALE_AND_BIAS_MULT) );
    fieldParams.push_back( fparams );
    VelPosn++;
  }

  if (Glob->params.output_ds_vel) {
    fparams = new DsFieldParams( "VEL", "m/s", 
				 ((float) VEL_SCALE_1)/
				 ((float) LL_SCALE_AND_BIAS_MULT), 
				 ((float) VEL_BIAS_1)/
				 ((float) LL_SCALE_AND_BIAS_MULT) );
    fieldParams.push_back( fparams );
    VelPosn++;
  } else {
    VelPosn = -1;
  }

  if (Glob->params.output_ds_spw) {
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
  if ( Glob->params.input_stream_format == NEXRAD_FORMAT ) {
    Glob->gate_ratio = 4;
  }
  else {
    Glob->gate_ratio = 1;
   }

  //
  // Initialize moments calculations
  //
  init_moments( noise_dbz_at_100km );
}


int reformat2ds(ui08 *read_buffer, int nread,
		NEXRAD_vcp_set *vcp_set,
		ui08 **write_buffer,
		int *nwrite)
     
{
  static ui08 *buffer_copy = NULL;
  static int n_copy_alloc = 0;

  ui08 *output_buffer = NULL;
  RIDDS_data_hdr *riddsData;

  /*
   * make copy of buffer
   */

  if (n_copy_alloc < nread) {
    if (buffer_copy == NULL) {
      buffer_copy = (ui08 *) umalloc(nread);
    } else {
      buffer_copy = (ui08 *) urealloc(buffer_copy, nread);
    }
    n_copy_alloc = nread;
  }
  memcpy(buffer_copy, read_buffer, nread);

  /*
   * swap header
   */

  riddsData = (RIDDS_data_hdr *) buffer_copy;
  BE_to_RIDDS_data_hdr(riddsData);

  if (process_beam(riddsData, &output_buffer, vcp_set, nwrite)) {

    return (-1);

  } else {

    *write_buffer = output_buffer;
    return (0);

  }
}

/************************************************************************/

static int process_beam(RIDDS_data_hdr *riddsData,
                        ui08 **output_buffer,
                        NEXRAD_vcp_set *vol_cntrl_patterns,
                        int *nwrite)
{
  static int initial_proc_call = TRUE;
  static int last_tilt_num = 0;
  static int vol_num = 0;

  int new_tilt = FALSE;
  int new_vol  = FALSE;
  int hour = 0;
  int min = 0;
  int seconds = 0;

  /* Get the data time */
  date_time_t ttime;
  get_beam_time( riddsData, Time_correction, &ttime );                  

  /* Determine if this is the start of a new 360 degree tilt */

  if ((riddsData->radial_status == START_OF_NEW_ELEVATION) ||
      (riddsData->radial_status == BEGINNING_OF_VOL_SCAN) ||
      (riddsData->elev_num != last_tilt_num) ||
      initial_proc_call) {

    if (Debug > 1) {
      fprintf(stderr, "\nradial status = %d, elev number = %d\n",
                    riddsData->radial_status, riddsData->elev_num);
    }

    /* Handle start/end-of-tilt condition */
    if (!initial_proc_call) {
      end_of_tilt( last_tilt_num );
    }

    /* Handle start/end-of-volume condition */
    if (riddsData->radial_status == BEGINNING_OF_VOL_SCAN
        && riddsData->elev_num == 1) {

      if (!initial_proc_call) {
        end_of_volume( vol_num );
      }
      vol_num++;
      new_vol = TRUE;
      start_of_volume( vol_num, ttime.unix_time );

      if (Debug) {
        seconds = riddsData->millisecs_past_midnight / 1000;
        hour = seconds / 3600;
        min = (seconds - hour * 3600) / 60;

        fprintf(stderr, "\n*********************************************\n");
        fprintf(stderr, "New Volume %2d :%2d :%2d", hour, min,
                seconds - hour * 3600 - min * 60);
        fprintf(stderr, " - NEXRAD radar\n\n");
      }
    }

    new_tilt = TRUE;
    start_of_tilt( riddsData->elev_num, ttime.unix_time );
    if (initial_proc_call) {
      initial_proc_call = FALSE;
    } 
  } 

  /* remember what tilt we're on */
  last_tilt_num = riddsData->elev_num;

  /* load up the ds message buffer */
  if ( loadDsMessage( riddsData, output_buffer, nwrite,
                      vol_num, new_vol,
                      riddsData->elev_num, new_tilt,
                      ttime.unix_time, vol_cntrl_patterns ))
    return( -1 );
  else
    return( 0 );
}

static int loadDsMessage( RIDDS_data_hdr *riddsData,
                          ui08 **outputBuffer, int *nwrite, 
                          int volumeNum, int newVolume,
                          short tiltNum, int newTilt,
                          time_t dataTime, NEXRAD_vcp_set *vol_cntrl_patterns )
{
  static bool firstCall = true;
  static time_t lastParamsUpdate = 0;
  static si16 prevVCP;

  int iret1 = 0;
  int iret2 = 0;
  int msgContent = 0;
  unsigned short newAzimuth;
  ui08* momentsData = NULL;
  time_t timeSinceUpdate;

  DsRadarBeam &radarBeam     = Glob->radarMsg->getRadarBeam();
  DsRadarParams &radarParams = Glob->radarMsg->getRadarParams();

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

void updateDynamicParams( RIDDS_data_hdr *riddsData )
{
  char scanTypeName[1024];

  //
  // Dynamic radar parameters
  //
  DsRadarParams &radarParams = Glob->radarMsg->getRadarParams();

  radarParams.scanType = riddsData->vol_coverage_pattern;
  sprintf( scanTypeName, "volume coverage pattern %d", radarParams.scanType );
  radarParams.scanTypeName = scanTypeName;

  radarParams.unambigVelocity = ((float)riddsData->nyquist_vel) / 100;
  radarParams.unambigRange    = ((float)riddsData->unamb_range_x10) / 10;
  radarParams.pulseRepFreq    = ((float)SPD_OF_LITE /
                                  (2 * 100 * riddsData->unamb_range_x10));

  if ( riddsData->vol_coverage_pattern == 31 )
    radarParams.pulseWidth = 31000;
  else
    radarParams.pulseWidth = 157000;

  /*
   * NEXRAD velocity and reflectivity fields have different gate
   * spacing and number of gates per beam - during the formatting
   * process everything is transformed to reflect the higher resolution
   * velocity values except low prf long range beams
   *
   * We ignore the first gate (short range scan) and first 4 gates
   * (long range scan) because these have neg ranges which RDI does
   * not like.
   */

  if ( riddsData->vel_data_playback == 0 ) {
    //
    // Reflectivity only
    //
    radarParams.numGates    = riddsData->ref_num_gates;
    radarParams.gateSpacing = riddsData->ref_gate_width / 1000.0;
    radarParams.startRange  = riddsData->ref_gate1 / 1000.0;

    if ( Glob->params.input_stream_format == NEXRAD_FORMAT ) {
       radarParams.startRange += radarParams.gateSpacing;
    }
  }
  else {
    radarParams.numGates    = riddsData->vel_num_gates;
    radarParams.gateSpacing = riddsData->vel_gate_width / 1000.0;
    radarParams.startRange  = riddsData->vel_gate1 / 1000.0;

    if ( Glob->params.input_stream_format == NEXRAD_FORMAT ) {
       radarParams.startRange += Glob->gate_ratio * radarParams.gateSpacing;
    }

  }

  //
  // Dynamic velocity resolution (1 & .5 m/s)
  //

  if (VelPosn >= 0) {
    vector< DsFieldParams* > &fieldParams = Glob->radarMsg->getFieldParams();
    DsFieldParams*  velParams = fieldParams[VelPosn];

    if ( riddsData->velocity_resolution == 4 ) { 
      velParams->scale = ((float) VEL_SCALE_1)/
	((float) LL_SCALE_AND_BIAS_MULT);       
      velParams->bias  = ((float) VEL_BIAS_1)/
	((float) LL_SCALE_AND_BIAS_MULT);
    } 
    else {                          
      velParams->scale = ((float) VEL_SCALE_HALF)/
	((float) LL_SCALE_AND_BIAS_MULT);
      velParams->bias  = ((float) VEL_BIAS_HALF)/
	((float) LL_SCALE_AND_BIAS_MULT);
    }
  }

}
