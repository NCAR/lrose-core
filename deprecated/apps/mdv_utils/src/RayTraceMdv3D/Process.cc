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


  _numFields = 0;
  for (int fld=0; fld < _params->radials_n; fld++){
    _fieldParams[_numFields] = new DsFieldParams(_params->_radials[fld].radialName,
						 _params->_radials[fld].units,
						 SCALE,
						 BIAS,
						 DATA_BYTE_WIDTH, (int)Mdv2Beam::badVal);
    _numFields++;
  }
  
  for (int fld=0; fld < _params->fields_n; fld++){
    _fieldParams[_numFields] = new DsFieldParams(_params->_fields[fld].outName,
						 _params->_fields[fld].units,
						 SCALE,
						 BIAS,
						 DATA_BYTE_WIDTH, (int)Mdv2Beam::badVal);
    _numFields++;
  }

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
  Mdv2Beam M(T, P);


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
  radarParams.numFields = _numFields;
  radarParams.radarName = "RayTraceMdv3D";
  radarParams.latitude = P->sensorGeometry.sensorLat;
  radarParams.longitude = P->sensorGeometry.sensorLon;
  radarParams.altitude = P->sensorGeometry.sensorAlt/1000.0; // m to Km

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
  // Loop through tilts.
  //
  for (int itilt=0; itilt < P->tilts_n; itilt++){

    if (P->Debug){
      cerr << "Tilt " << itilt+1 << " of " << P->tilts_n;
      cerr << " elevation " << P->_tilts[itilt].tiltElevation;
      cerr << " azimuth from " << P->_tilts[itilt].tiltMinAz;
      cerr << " to " <<  P->_tilts[itilt].tiltMaxAz;
      cerr << " in steps of " << P->_tilts[itilt].tiltDelAz;
      cerr << " from " << P->_tilts[itilt].tiltFirstGateRange;
      cerr << " to " << P->_tilts[itilt].tiltLastGateRange << " meters." << endl;
    }

    //
    // Loop through azimuths.
    //
    double az = P->_tilts[itilt].tiltMinAz;
    _beamsSentSoFar = 0;  // Ensure we send params at start of tilt.
    int numGates = M.getNumGates( itilt );

    // Allocate space for a beam at this tilt.

    float *beamData = (fl32 *) malloc(numGates * _numFields * DATA_BYTE_WIDTH);
    if ( beamData == NULL){
      cerr << "Malloc failed!" << endl;
      exit(-1);
    }
    
    do {
      
      if (_beamsSentSoFar == 0) {
	//
	// Send the setup.
	//
	
	DsRadarParams& radarParams = _radarMsg.getRadarParams();
	
	radarParams.numGates = numGates;
      
	radarParams.gateSpacing = P->sensorGeometry.sensorRangeSpacing / 1000.0; // Meters to Km.
	radarParams.startRange = P->_tilts[itilt].tiltFirstGateRange / 1000.0; // Meters to Km.
          
	radarParams.horizBeamWidth = P->sensorGeometry.horizBeamWidth;
	radarParams.vertBeamWidth = P->sensorGeometry.vertBeamWidth;
            
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
	for (int k=0; k < _numFields; k++){
	  fieldParams.push_back( _fieldParams[k] );
	}

	content =  DsRadarMsg::FIELD_PARAMS;
	if( _radarQueue.putDsMsg(_radarMsg, content ) != 0 ) {
	  fprintf(stderr," Failed to send radar flags.\n");
	  exit(-1);
	}
      }
      //
      // Send the actual beam data.
      //
    
      M.getBeam( az, itilt, _numFields, numGates, beamData );
    
      DsRadarBeam& radarBeam = _radarMsg.getRadarBeam();

      radarBeam.dataTime   = T;
      radarBeam.volumeNum  = 0;
      radarBeam.tiltNum    = itilt;
      radarBeam.azimuth    = az;
      radarBeam.elevation  = P->_tilts[itilt].tiltElevation;
      radarBeam.targetElev = radarBeam.elevation;


      radarBeam.loadData( beamData,
			  _numFields * numGates * DATA_BYTE_WIDTH,
			  DATA_BYTE_WIDTH );
      
      int content = DsRadarMsg::RADAR_BEAM;
      if( _radarQueue.putDsMsg(_radarMsg, content ) != 0 ) {
	fprintf(stderr," Failed to send beam data.\n");
	exit(-1);
      }

      //
      //
      // Increment the beam count.
      //
      _beamsSentSoFar = ( _beamsSentSoFar + 1 ) % P->beamsPerMessage;
      
      az += P->_tilts[itilt].tiltDelAz;

    } while ( az <= P->_tilts[itilt].tiltMaxAz );

    free(beamData);

  } // End of loop through tilts.
    
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
  // Delay for a bit in archive mode.
  //
  if (P->Mode == Params::ARCHIVE){
    if ((P->Debug) && (P->fileDelaySecs)){
      cerr << "Sleeping for " << P->fileDelaySecs << " seconds";
    }
    for (int i=0; i < P->fileDelaySecs; i++){
      sleep(1);
      PMU_auto_register("Sleeping between files in archive mode....");
      if (P->Debug) cerr << ".";
    }
    if ((P->Debug) && (P->fileDelaySecs)){
      cerr << endl;
    }
  }

  if (P->Debug) cerr << endl;

  // Done with processing at this time.

  return 0;

}

////////////////////////////////////////////////////
//
// Destructor
//
Process::~Process(){
  return;
}










