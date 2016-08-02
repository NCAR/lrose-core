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


#include "Process.hh"

#include "Mdv2Beam.hh"

#include <iostream>
#include <Mdv/MdvxField.hh>
#include <cmath>
#include <toolsa/str.h>
#include <toolsa/pjg_flat.h>
#include <Mdv/MdvxProj.hh>

using namespace std;

//
// Constructor - opens FMQ URL.
//
Process::Process(Params *_params){
  //
  _beamsSentSoFar = 0;
  //
  // Set up the message log.
  //
  _msgLog.setApplication( "RayTraceMdv" );

  //
  // Enable debug level messaging
  //
  if ( _params->Debug == TRUE ){
    _msgLog.enableMsg( DEBUG, true );
    _msgLog.enableMsg( INFO, true );
  } else {
    _msgLog.enableMsg( DEBUG, false );
    _msgLog.enableMsg( INFO, false );
  }
  //
  //
  //
  if ( _msgLog.setOutputDir( _params->msgLog_dir ) != 0 ) {
    cerr << "Failed to set up message log to directory ";
    cerr << _params->msgLog_dir;
    cerr << endl;
    exit(-1);
  }

  //
  // Initialize the radar queue.
  //
  if( _radarQueue.init( _params->output_fmq_url,
                        "SimLidarAndNoise",
                         _params->Debug,
                         DsFmq::READ_WRITE, DsFmq::END,
                        _params->output_fmq_compress,
                        _params->output_fmq_nslots,
                        _params->output_fmq_size, 1000,
                        &_msgLog )) {
    cerr << "Could not initialize fmq " << _params->output_fmq_url << endl;
    exit(-1);
  }

  //
  // If archive mode is our game, use blocking writes.
  //
  if (_params->Mode == Params::ARCHIVE){
    if (_params->Debug){
      cerr << "Using blocking writes." << endl;
    }
    if ( _radarQueue.setBlockingWrite()){
      cerr << "Failed to set blocking write!" << endl;
      exit(-1);
    }
  }

  _fieldParams = new DsFieldParams("lidar",
				   "none",
				   SCALE,
				   BIAS,
				   DATA_BYTE_WIDTH, Mdv2Beam::badVal);
  return;

}

////////////////////////////////////////////////////
//
// Main method - process data at a given time.
//
int Process::Derive(Params *P, time_t T){


  if (P->Debug){
    cerr << "Data at " << utimstr(T) << endl;
  }

  //
  // Set up the object that gets us beams of conc.
  //
  Mdv2Beam M(T,
	     P->TriggerUrl,
	     P->_fieldNames[0],
	     P->geometry.r0,
	     P->geometry.dR,
	     P->geometry.Rmax,
	     P->geometry.lidarLat,
	     P->geometry.lidarLon );

  //
  // Set up the object that gets us beams of noise.
  //
  Mdv2Beam N(T,
	     P->NoiseUrl,
	     P->_fieldNames[1],
	     P->geometry.r0,
	     P->geometry.dR,
	     P->geometry.Rmax,
	     P->geometry.lidarLat,
	     P->geometry.lidarLon );

  //
  // Request subsampling, if desired.
  //
  if (P->doSubSampling){
    M.setSubSample( P->subSamplingDr );
    N.setSubSample( P->subSamplingDr );
  }

  //
  // Send a start of volume.
  //
  int content;
  //
  // Set up the params part of the message.
  //
  DsRadarParams& radarParams = _radarMsg.getRadarParams();
  DsRadarFlags& radarFlags = _radarMsg.getRadarFlags();

  radarParams.radarId = 1;
  radarParams.numFields = 1;
  radarParams.radarName = "RayTraceMdv";
  radarParams.latitude = P->geometry.lidarLat;
  radarParams.longitude = P->geometry.lidarLon;
  radarParams.altitude = 0.0;

  //
  // Send a start of volume flag.
  //
  radarFlags.startOfVolume = true;
  radarFlags.startOfTilt   = true;
  radarFlags.endOfTilt     = false;
  radarFlags.endOfVolume   = false;

  content = DsRadarMsg::RADAR_FLAGS;
  if( _radarQueue.putDsMsg( _radarMsg, content ) != 0 ) {
    fprintf(stderr," Failed to send start of volume flag.\n");
    exit(-1);
  }

  //
  // Loop through azimuth.
  //

  double az = P->geometry.azMin;

  do {

    if (_beamsSentSoFar == 0) {
      //
      // Send the setup.
      //
      
      DsRadarParams& radarParams = _radarMsg.getRadarParams();
      
      radarParams.numGates = M.getNumGates();
      
      radarParams.gateSpacing = P->geometry.dR / 1000.0; // Meters to Km.
      radarParams.startRange = P->geometry.r0 / 1000.0; // Meters to Km.
    
      
      radarParams.horizBeamWidth = P->horizBeamWidth;
      radarParams.vertBeamWidth = P->vertBeamWidth;
      
      
      //
      // Send the radar and field params.
      //
      int content =  DsRadarMsg::RADAR_PARAMS;
      if( _radarQueue.putDsMsg(_radarMsg, content ) != 0 ) {
	fprintf(stderr," Failed to send radar params.\n");
	exit(-1);
      }
      
      vector< DsFieldParams* >& fieldParams = _radarMsg.getFieldParams();
      fieldParams.clear();
      fieldParams.push_back( _fieldParams );
      
      content =  DsRadarMsg::FIELD_PARAMS;
      if( _radarQueue.putDsMsg(_radarMsg, content ) != 0 ) {
	fprintf(stderr," Failed to send radar flags.\n");
	exit(-1);
      }
    }
    //
    // Send the actual beam data.
    //

    //
    // Combine conc and noise into one beam.
    //    
    vector <double> lidarBeam = M.getBeam( az );
    vector <double> noiseBeam = N.getBeam( az );

    if (lidarBeam.size() != noiseBeam.size()){
      cerr << "Beam sizes differ, I cannot cope." << endl;
      exit(-1);
    }

    //
    // Vector of anthrax backscatter
    //
    vector <double> b;
    b.clear();

    const double pi = acos( -1.0 );

    for (unsigned k=0; k < noiseBeam.size(); k++){
      //
      // If either the scipuff concentration data or the
      // carefully generated noise data are not available,
      // use the bad value, otherwise send lidar plus noise.
      //
      if (
	  (noiseBeam[k] == Mdv2Beam::badVal) ||
	  (lidarBeam[k] == Mdv2Beam::badVal)
	  ){
	b.push_back( Mdv2Beam::badVal );
      } else {
	//
	// Apply scale and bias to the noise.
	//
	noiseBeam[k] = (noiseBeam[k] + P->NoiseBias) * P->NoiseScale;
	//
	// Turn the concentration into a backscatter.
	//
	lidarBeam[k] = lidarBeam[k] * P->ParticleDensity
	  * pi * P->ParticleRadius * P->ParticleRadius * P->Qsc;
	//
	// Don't let backscatter be a negative.
	//
	if (lidarBeam[k] + noiseBeam[k] > 0.0){
	  b.push_back( lidarBeam[k] + noiseBeam[k] );
	} else {
	  b.push_back( 0.0 );
	}
	//
	if (P->Debug == Params::DEBUG_EXTREME){
	  cerr << "GATE : " << k << " NOISE : " << noiseBeam[k];
	  cerr << " BACKSCATTER : " << lidarBeam[k];
	  cerr << " TOTAL : " << b[k] << endl;
	}
	//
      }
    }

    //
    // Convert the total backscatter into the signal.
    //
    vector <double > signal;
    signal.clear();

    double Area = pi * P->sensorRadius *  P->sensorRadius;
    const double c = 3e8;


    double lmax = 0.0;
    for (unsigned k=0; k < noiseBeam.size(); k++){
      //
      //
      //
      if ( b[k] == Mdv2Beam::badVal ) {
	signal.push_back( Mdv2Beam::badVal );
      } else {
	//
	// Get radius in meters.
	//
	double r = P->geometry.r0 + k * P->geometry.dR;
	//
	// Get returned power in mJ. Rather long winded.
	//
	double p =  b[k] * P->RxTrans * P->pulseEnergy 
	  * c / 2.0 * Area /( r*r );

	p = p * exp(-2.0*r*( P->betaExt + P->betaRayleigh ));

	if (P->Debug == Params::DEBUG_SPECIAL){
	  if (fabs(az-45.0) < 0.5){
	    cerr << "At time " << utimstr( T );
	    cerr << " az " << az;
	    cerr << " power at " << k << " gate is " << p;
	    cerr << " noise beam " << noiseBeam[k];
	    cerr << " lidar beam " << lidarBeam[k] << endl;
	    if (lidarBeam[k] > lmax) lmax = lidarBeam[k];
	  }
	}

	//
	// Now convert power in mJ to a count.
	//
	double count = rint( P->ampGain * p );

	//
	// Now, do the lidar processing that we do on the other
	// end. Multiply by r squared, take log.
	//
	if (count == 0){
	  signal.push_back( Mdv2Beam::badVal );
	} else {
	  signal.push_back( 10.0*log10( r*r*count ));
	}
	//
      }
    }
    if (P->Debug == Params::DEBUG_SPECIAL){
      if (fabs(az-45.0) < 0.5){
	cerr << "LMAX IS : " << lmax << endl;
      }
    }
    
    DsRadarBeam& radarBeam = _radarMsg.getRadarBeam();

    radarBeam.dataTime   = T;
    radarBeam.volumeNum  = 0;
    radarBeam.tiltNum    = 0;
    radarBeam.azimuth    = az;
    radarBeam.elevation  = 0.0;
    radarBeam.targetElev = 0.0;


    float *beamData = (float *) malloc(b.size() * DATA_BYTE_WIDTH);
    if ( beamData == NULL){
      cerr << "Malloc failed!" << endl;
      exit(-1);
    }

    for (unsigned ig=0; ig < signal.size(); ig++){
      if (P->sendPower){
	beamData[ig] = signal[ig];
      } else {
	beamData[ig] = b[ig];
      }
    }

    //
    // Convert output to log, if requested.
    //
    if (P->sendLog10){
      for (unsigned ig=0; ig < signal.size(); ig++){
	if (beamData[ig] != Mdv2Beam::badVal){
	  beamData[ig] = log10( beamData[ig] );
	}
      }
    }

    radarBeam.loadData( beamData,
			b.size() * DATA_BYTE_WIDTH,
			DATA_BYTE_WIDTH );

   int content = DsRadarMsg::RADAR_BEAM;
   if( _radarQueue.putDsMsg(_radarMsg, content ) != 0 ) {
     fprintf(stderr," Failed to send beam data.\n");
     exit(-1);
   }
   //
   //
   // Reset the count.
   //
   _beamsSentSoFar = ( _beamsSentSoFar + 1 ) % P->beamsPerMessage;
    
    az += P->geometry.delAz;

  } while ( az <= P->geometry.azMax );


    
  //
  // Send an end of volume flag.
  //
  
  radarFlags.startOfVolume = false;
  radarFlags.startOfTilt   = false;
  radarFlags.endOfTilt     = true;
  radarFlags.endOfVolume   = true;
  
  content = DsRadarMsg::RADAR_FLAGS;
  if( _radarQueue.putDsMsg( _radarMsg, content ) != 0 ) {
    fprintf(stderr," Failed to send end of volume flag.\n");
    exit(-1);
  }
  //
  // Delay for a bit.
  //

  for (int i=0; i < P->fileDelaySecs; i++){
    sleep(1);
    PMU_auto_register("Sleeping....");
  }


  return 0;

}

////////////////////////////////////////////////////
//
// Destructor
//
Process::~Process(){



}










