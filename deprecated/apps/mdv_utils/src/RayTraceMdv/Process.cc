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
#include <math.h>
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
                        "RayTraceMdv",
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

  _fieldParams = new DsFieldParams(_params->fieldName,
				   _params->units,
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
  // Set up the object that gets us beams.
  //
  Mdv2Beam M(T,
	     P->TriggerUrl,
	     P->fieldName,
	     P->geometry.r0,
	     P->geometry.dR,
	     P->geometry.Rmax,
	     P->geometry.lidarLat,
	     P->geometry.lidarLon );


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
    
    vector <double> b = M.getBeam( az );
    
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

    for (unsigned ig=0; ig < b.size(); ig++){
      beamData[ig] = b[ig];
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










