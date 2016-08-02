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

#include <dataport/swap.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>

#include "read_dBZ.hh"
#include "read_dBZc.hh"
#include "read_dBT.hh"
#include "read_V.hh"
#include "read_W.hh"

#include "processIrisVol.hh"
#include "Params.hh"

using namespace std;

const float processIrisVol::SCALE = 1.0;
const float processIrisVol::BIAS = 0.0;  

//
// Constructor. Makes copies of params. Opens
// output FMQ.
//
processIrisVol::processIrisVol(Params *P){

  _tiltNum = 0; _volNum = 0; _beamsSent = 0;
  //
  // Point to the TDRP params.
  //
  _params = P;

  //
  // Set up the message log.
  //
  _msgLog.setApplication( "iris2dsr" );

  //
  // Enable debug level messaging
  //
  if ( _params->debug == TRUE ){
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
  // Open our output FMQ.
  //
   if( _radarQueue.init( _params->fmq.url,
                        "iris2dsr",
                         _params->debug,           
                         DsFmq::READ_WRITE, DsFmq::END,
                        _params->fmq.compress,
                        _params->fmq.numSlots,
                        _params->fmq.sizeBytes, 1000,
                        &_msgLog )) {
    cerr << "Could not initialize fmq " << _params->fmq.url << endl; 
    exit(-1);
  }
  //
  // If archive mode is our game, use blocking writes.
  //
  if (_params->mode == Params::ARCHIVE){
    if (_params->debug){
      cerr << "Using blocking writes." << endl;
    }
    if ( _radarQueue.setBlockingWrite()){
      cerr << "Failed to set blocking write!" << endl;
      exit(-1);
    }
  }


  //
  // Count the number of fields we are outputting as
  // we set up the field params.
  //
  _numFieldsOut = 0;

  if (_params->ingestFields.ingest_dBZ){
    _fieldParams[_numFieldsOut] = new DsFieldParams("dBZ", "dBZ",
						    SCALE,
						    BIAS,
						    DATA_BYTE_WIDTH,
						    MISSING);
    _numFieldsOut++;
  }


  if (_params->ingestFields.ingest_V){
    _fieldParams[_numFieldsOut] = new DsFieldParams("V", "m/s",
						    SCALE,
						    BIAS,
						    DATA_BYTE_WIDTH,
						    MISSING);
    _numFieldsOut++;
  }

  if (_params->ingestFields.ingest_W){
    _fieldParams[_numFieldsOut] = new DsFieldParams("W", "m",
						    SCALE,
						    BIAS,
						    DATA_BYTE_WIDTH,
						    MISSING);
    _numFieldsOut++;
  }

  if (_params->ingestFields.ingest_dBZc){
    _fieldParams[_numFieldsOut] = new DsFieldParams("dBZc", "dBZ",
						    SCALE,
						    BIAS,
						    DATA_BYTE_WIDTH,
						    MISSING);
    _numFieldsOut++;
  }

  if (_params->ingestFields.ingest_dBT){
    _fieldParams[_numFieldsOut] = new DsFieldParams("dBT", "dB",
						    SCALE,
						    BIAS,
						    DATA_BYTE_WIDTH,
						    MISSING);
    _numFieldsOut++;
  }

  if (_params->debug)
    cerr << _numFieldsOut << " fields to output." << endl;

  return;
}

//
// Init a new volume. Reads the base file, gets beam
// spacing and some other things. Returns 0 on
// success, else -1.
//
int processIrisVol::initVol(char *baseName){

  _tiltNum = 0;

  //
  // Parse the time from the base name, which
  // will be something like "TIA070724053502."
  //
  if (6 != sscanf(baseName+3,
		  "%2d%2d%2d%2d%2d%2d",
		  &_dataTime.year, &_dataTime.month, &_dataTime.day,
		  &_dataTime.hour, &_dataTime.min, &_dataTime.sec)){
    cerr << " Failed to parse time from " << baseName << endl;
    return -1;
  }

  if (_dataTime.year < 50){
    _dataTime.year += 2000;
  } else {
    _dataTime.year += 1900;
  }
  uconvert_to_utime( &_dataTime );

  if (_params->debug)
    cerr << "Data time is " << utimstr(_dataTime.unix_time) << endl;

  //
  // Construe the header file name and open it.
  //
  char fullFileName[MAX_PATH_LEN];
  sprintf(fullFileName,"%s/%s", _params->inputDir, baseName);

  cerr << "Attempting to open " << fullFileName  << endl;

  FILE *fp = fopen(fullFileName,"r");
  if (fp == NULL){
    cerr << "Failed to open " << fullFileName << endl;
    return -1;
  }

  if (fseek(fp, 1264, SEEK_SET)){
    cerr << "fseek() failed on " << fullFileName << endl;
    return -1;
  }

  //
  // Read the range to first gate.
  //
  si32 firstBinCm;
  fread(&firstBinCm, 1, sizeof(si32), fp);
  if (_params->byteSwap) SWAP_array_32((ui32 *) &firstBinCm, 4);
  _rangeFirstBinKm = double(firstBinCm)/100000.0;

  //
  // Read the gate spacing.
  //
  if (fseek(fp, 1280, SEEK_SET)){
    cerr << "firstGate fseek() failed on " << fullFileName << endl;
    return -1;
  }

  si32 binStepCm;
  fread(&binStepCm, 1, sizeof(si32), fp);
  if (_params->byteSwap) SWAP_array_32((ui32 *) &binStepCm, 4);
  _binSpacingKm = double(binStepCm)/100000.0;


  //
  // Overwrite range to first bin, spacing if resampling.
  // In any event save the original values.
  //
  _origRangeFirstBinKm = _rangeFirstBinKm;
  _origBinSpacingKm = _binSpacingKm;
  //
  if (_params->resample.doResample){
    _rangeFirstBinKm = _params->resample.firstGateKm;
    _binSpacingKm = _params->resample.incKm;
  }




  //
  // Number of bins.
  //
  if (fseek(fp, 1272, SEEK_SET)){
    cerr << "numBins fseek() failed on " << fullFileName << endl;
    return -1;
  }

  si16 numBins;
  fread(&numBins, 1, sizeof(si16), fp);
  if (_params->byteSwap) SWAP_array_16((ui16 *) &numBins, 2);
  _numBins = numBins;


  //
  // PRF
  //
   if (fseek(fp, 492+268, SEEK_SET)){
    cerr << "prf fseek() failed on " << fullFileName << endl;
    return -1;
  } 

  si32 prf;
  fread(&prf, 1, sizeof(si32), fp);
  if (_params->byteSwap) SWAP_array_32((ui32 *) &prf, 4);
  _prfHz = double(prf);
  //
  // Pulse width immediately after
  //
  si32 pw;
  fread(&pw, 1, sizeof(si32), fp);
  if (_params->byteSwap) SWAP_array_32((ui32 *) &pw, 4);
  _pulseWidthMicroSec = double(pw)/100.0;


  si16 multiPrfFlag;
  fread(&multiPrfFlag, 1, sizeof(si16), fp);
  if (_params->byteSwap) SWAP_array_16((ui16 *) &multiPrfFlag, 2);
  _multiPrfFlag = multiPrfFlag;

  fseek(fp, 1744, SEEK_SET);

  ui32 wavelenHdthCm;
  fread(&wavelenHdthCm, 1, sizeof(ui32), fp);
  if (_params->byteSwap) SWAP_array_32((ui32 *) &wavelenHdthCm, 4);
  _waveLen = double(wavelenHdthCm) / 100.0;

  fseek(fp, 168+12, SEEK_SET);

  ui32 latBin4;
  fread(&latBin4, 1, sizeof(ui32), fp);
  if (_params->byteSwap) SWAP_array_32((ui32 *) &latBin4, 4);

  ui32 lonBin4;
  fread(&lonBin4, 1, sizeof(ui32), fp);
  if (_params->byteSwap) SWAP_array_32((ui32 *) &lonBin4, 4);

  si16 HtM; // Height of ground
  fread(&HtM, 1, sizeof(si16), fp);
  if (_params->byteSwap) SWAP_array_16((ui16 *) &HtM, 2);
 
  si16 HtM2; // Height of tower
  fread(&HtM2, 1, sizeof(si16), fp);
  if (_params->byteSwap) SWAP_array_16((ui16 *) &HtM2, 2);


  _lat = 360.0*double(latBin4)/4294967296.0;
  _lon = 360.0*double(lonBin4)/4294967296.0;
  _alt = double(HtM + HtM2) / 1000.0; // M to Km


  //
  // horiz, vert beam width
  //
   if (fseek(fp, 1744+64, SEEK_SET)){
    cerr << "beamWidth fseek() failed on " << fullFileName << endl;
    return -1;
  } 

  si32 hbw;
  fread(&hbw, 1, sizeof(si32), fp);
  if (_params->byteSwap) SWAP_array_32((ui32 *) &hbw, 4);
  _horizWidth = double(hbw) * 360.0 / 4294967296.0;

  si32 vbw;
  fread(&vbw, 1, sizeof(si32), fp);
  if (_params->byteSwap) SWAP_array_32((ui32 *) &vbw, 4);
  _vertWidth = double(vbw) * 360.0 / 4294967296.0;

  _nyquistVel = _prfHz * _waveLen / 400.0; // Conversion from cm to m and factor of 4

  //
  // Account for multi-PRF mode.
  //
  // 2:3 doubled
  // 4:3 tripled
  // 4:5 quadrupled

  switch( multiPrfFlag ){

  case 0 :
    // 1:1, No action
    break;

  case 1 :
    // 2:3, Doubled
    _nyquistVel *= 2.0;
    break;

  case 2 :
    // 3:4, not clearly documented but I think tripled
    _nyquistVel *= 3.0;
    break;

  case 3 :
    // 4:5, quadrupled
    _nyquistVel *= 4.0;
    break;

    default :
      fprintf(stderr,"Unrecognised code for multi-PRF mode : %d\n",
	      (int) multiPrfFlag);
      return -1;
      break;
      
  }



  fclose(fp);

  if (_params->debug){
    cerr << "Number of bins : " << _numBins << endl;
    cerr << "Number of fields output : " << _numFieldsOut << endl;
    cerr << "Bin spacing, Km : " << _binSpacingKm << endl;
    cerr << "Range to first bin, Km : " << _rangeFirstBinKm << endl;
    cerr << "PRF, Hz : " << _prfHz << endl;
    cerr << "Pulse width, micro sec : " << _pulseWidthMicroSec << endl;
    cerr << "Horizontal beam width : " << _horizWidth << endl;
    cerr << "Vertical beam width : " << _vertWidth << endl;
    cerr << "Wavelen, cm : " << _waveLen << endl;
    cerr << "Lat, Lon, alt (Km) : " << _lat << ", ";
    cerr << _lon << ", " << _alt << endl;
    cerr << "Multi PRF flag : " << _multiPrfFlag << endl;
    cerr << "Nyquist velocity : " << _nyquistVel << endl;
  }

  return 0;
}

//
// Main gig. Process a tilt. Tilt numbers start at 0.
// returns 0 if everything went OK, else -1.
//
int processIrisVol::processTilt(char *baseName){

  _beamsSent = 0; // Count of beams sent this tilt.

  //
  // See if we can read the files for this tilt for all
  // fields. This may be a mistake; it may be better to
  // only read the files we need. At the moment, though,
  // I try to read the DBZ and if that fails I assume
  // we are at the end of the volume and waiting for the
  // next tilt.
  //
  DsRadarFlags& radarFlags = _radarMsg.getRadarFlags();
  read_dBZ  dBZ(_params, baseName, _tiltNum,
		_origBinSpacingKm, _origRangeFirstBinKm );


  if (dBZ.getNBeams() == 0){
    //
    // We must be at the end of this volume.
    // Send the right flags and return.
    //
    radarFlags.startOfVolume = false;
    radarFlags.startOfTilt   = false;
    radarFlags.endOfTilt     = true;
    radarFlags.endOfVolume   = true;
    
    int content = DsRadarMsg::RADAR_FLAGS;
    if( _radarQueue.putDsMsg( _radarMsg, content ) != 0 ) {  
      fprintf(stderr," Failed to send end of volume flag.\n");
      exit(-1);
    }
    return -1;
  }

  _numBins = dBZ.getNBins();
  
  if (_tiltNum == 0){
    //
    // First tilt? send a start-of-vol flag.
    //
    
    _volNum++;
    if (_volNum == 100) _volNum = 0;
    
    radarFlags.startOfVolume = true;
    radarFlags.startOfTilt   = true;
    radarFlags.endOfTilt     = false;
    radarFlags.endOfVolume   = false;
    
    int content = DsRadarMsg::RADAR_FLAGS;
    if( _radarQueue.putDsMsg( _radarMsg, content ) != 0 ) {  
      fprintf(stderr," Failed to send start of volume flag.\n");
      exit(-1);
    }
  } else {
    //
    // Not the first tilt. Send a start of tilt flag.
    //
    radarFlags.startOfVolume = false;
    radarFlags.startOfTilt   = true;
    radarFlags.endOfTilt     = true;
    radarFlags.endOfVolume   = false;
    
    int content = DsRadarMsg::RADAR_FLAGS;
    if( _radarQueue.putDsMsg( _radarMsg, content ) != 0 ) {  
      fprintf(stderr," Failed to send start of volume flag.\n");
      exit(-1);
    }
  }

  read_V    V(_params, baseName, _tiltNum, _nyquistVel,
		_origBinSpacingKm, _origRangeFirstBinKm );
  read_W    W(_params, baseName, _tiltNum, _nyquistVel,
	      _origBinSpacingKm, _origRangeFirstBinKm);
  read_dBZc dBZc(_params, baseName, _tiltNum,
		_origBinSpacingKm, _origRangeFirstBinKm);
  read_dBT  dBT(_params, baseName, _tiltNum,
		_origBinSpacingKm, _origRangeFirstBinKm);  
  
  //
  // Send the radar configuration at the start of each tilt.
  //
  DsRadarParams& radarParams = _radarMsg.getRadarParams();
  radarParams.numFields = _numFieldsOut;
  radarParams.radarName = _params->radarSpec.name;
  radarParams.latitude = _lat;
  radarParams.longitude = _lon;
  radarParams.altitude = _alt;
  radarParams.radarId = _params->radarSpec.ID;
  radarParams.numGates = _numBins;
  
  radarParams.gateSpacing = _binSpacingKm;
  radarParams.startRange = _rangeFirstBinKm;
  
  radarParams.horizBeamWidth = _horizWidth;
  radarParams.vertBeamWidth = _vertWidth;
  radarParams.pulseWidth = _pulseWidthMicroSec;
  radarParams.pulseRepFreq = _prfHz;
  radarParams.wavelength = _waveLen;
  radarParams.unambigVelocity = _nyquistVel;
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
  
  for(int ifld=0; ifld <  _numFieldsOut; ifld++){
    fieldParams.push_back( _fieldParams[ifld] );
  }
  
  
  content =  DsRadarMsg::FIELD_PARAMS;
  if( _radarQueue.putDsMsg(_radarMsg, content ) != 0 ) {  
    fprintf(stderr," Failed to send radar flags.\n");
    exit(-1);
  }
  
  //
  // Delay.
  //
  for (int k=0; k < _params->delaySecs; k++){
    sleep(1);
    if ( (k % 15) == 0){
      PMU_auto_register("Delaying after beam parameters...");
    }
  }

  //
  // If we got here, we know we have a dBZ file. We need to check that all the
  // files that we are ingesting have turned up (or we get a seg fault). If
  // one of them has not, then we need to skip this tilt - but increment
  // the tilt number and return 0 (no error) so that the next tilt (if any)
  // can be processed.
  //

  if ((_params->ingestFields.ingest_dBZc) && (dBZc.getNBeams() == 0)){
    cerr << "No dBZc file found for " << baseName << " for tilt " << _tiltNum +1 << endl;
    cerr << "Ignoring this tilt..." << endl;
    _tiltNum++;
    return 0;
  }

  if ((_params->ingestFields.ingest_dBT) && (dBT.getNBeams() == 0)){
    cerr << "No dBT file found for " << baseName << " for tilt " << _tiltNum +1 << endl;
    cerr << "Ignoring this tilt..." << endl;
    _tiltNum++;
    return 0;
  }

  if ((_params->ingestFields.ingest_W) && (W.getNBeams() == 0)){
    cerr << "No W file found for " << baseName << " for tilt " << _tiltNum +1 << endl;
    cerr << "Ignoring this tilt..." << endl;
    _tiltNum++;
    return 0;
  }

  if ((_params->ingestFields.ingest_V) && (V.getNBeams() == 0)){
    cerr << "No V file found for " << baseName << " for tilt " << _tiltNum +1 << endl;
    cerr << "Ignoring this tilt..." << endl;
    _tiltNum++;
    return 0;
  }

  ////////////////// End of checks to see if all files have turned up ///////////////
  
  int numBeams = dBZ.getNBeams(); // Assume all fields have the same number of beams.

  _beamData = NULL;

  _beamsSent = 0;
  
  for (int ib=0; ib < numBeams; ib++){
    
    int numFieldsInserted = 0;
    double az=0.0, el=0.0;
    
    if (_params->ingestFields.ingest_dBZ){
      aBeam beam = dBZ.getBeam(ib);
      if (beam.beamValues.size() == 0){
	continue; // Probably a null beam
      }
      
      if (_beamData == NULL) {
	_numBins = beam.beamValues.size();
	_beamData = (fl32 *) malloc(sizeof(fl32) * _numFieldsOut * _numBins);
	if (_beamData == NULL){
	  cerr << "malloc failed." << endl;
	  exit(-1);
	}
      }

      for (int ig=0; ig < _numBins; ig++){
	_beamData[ig * _numFieldsOut + numFieldsInserted] = beam.beamValues[ig];
      }
      numFieldsInserted++; az = beam.az; el = beam.el;
    }
    
    if (_params->ingestFields.ingest_V){
      aBeam beam = V.getBeam(ib);
      if (beam.beamValues.size() == 0){
	continue;
      }
      
      if (_beamData == NULL) {
	_numBins = beam.beamValues.size();
	_beamData = (fl32 *) malloc(sizeof(fl32) * _numFieldsOut * _numBins);
	if (_beamData == NULL){
	  cerr << "malloc failed." << endl;
	  exit(-1);
	}
      }
      
      for (int ig=0; ig < _numBins; ig++){
	_beamData[ig * _numFieldsOut + numFieldsInserted] = beam.beamValues[ig];
      }
      numFieldsInserted++; az = beam.az; el = beam.el;
    }
    
    if (_params->ingestFields.ingest_W){
      aBeam beam = W.getBeam(ib);
      if (beam.beamValues.size() == 0){
	continue;
      }
      
      if (_beamData == NULL) {
	_numBins = beam.beamValues.size();
	_beamData = (fl32 *) malloc(sizeof(fl32) * _numFieldsOut * _numBins);
	if (_beamData == NULL){
	  cerr << "malloc failed." << endl;
	  exit(-1);
	}
      }
      
      for (int ig=0; ig < _numBins; ig++){
	_beamData[ig * _numFieldsOut + numFieldsInserted] = beam.beamValues[ig];
      }
      numFieldsInserted++; az = beam.az; el = beam.el;
    }
    
    if (_params->ingestFields.ingest_dBZc){
      aBeam beam = dBZc.getBeam(ib);
      if (beam.beamValues.size() == 0){
	continue;
      }
      
      if (_beamData == NULL) {
	_numBins = beam.beamValues.size();
	_beamData = (fl32 *) malloc(sizeof(fl32) * _numFieldsOut * _numBins);
	if (_beamData == NULL){
	  cerr << "malloc failed." << endl;
	  exit(-1);
	}
      }

      for (int ig=0; ig < _numBins; ig++){
	_beamData[ig * _numFieldsOut + numFieldsInserted] = beam.beamValues[ig];
      }
      numFieldsInserted++; az = beam.az; el = beam.el;
    }
    
    if (_params->ingestFields.ingest_dBT){
      aBeam beam = dBT.getBeam(ib);
      if (beam.beamValues.size() == 0){
	continue;
      }
      
      if (_beamData == NULL) {
	_numBins = beam.beamValues.size();
	_beamData = (fl32 *) malloc(sizeof(fl32) * _numFieldsOut * _numBins);
	if (_beamData == NULL){
	  cerr << "malloc failed." << endl;
	  exit(-1);
	}
      }
      
      for (int ig=0; ig < _numBins; ig++){
	_beamData[ig * _numFieldsOut + numFieldsInserted] = beam.beamValues[ig];
      }
      numFieldsInserted++; az = beam.az; el = beam.el;
    }
    
    if (numFieldsInserted != _numFieldsOut) continue; // null beams were found.
    
    //
    // And send the beam.
    //
    DsRadarBeam& radarBeam = _radarMsg.getRadarBeam();
   
    if (_params->useRealtime){
      radarBeam.dataTime   = time(NULL);
    } else {
      radarBeam.dataTime   = _dataTime.unix_time;
    }
    radarBeam.dataTime += _params->timeOffset;

    radarBeam.volumeNum  = _volNum;
    radarBeam.tiltNum    = _tiltNum+1;
    radarBeam.azimuth    = az;
    radarBeam.elevation  = el;
    radarBeam.targetElev = radarBeam.elevation;
    radarBeam.loadData( _beamData,
			_numFieldsOut * _numBins * DATA_BYTE_WIDTH, 
			DATA_BYTE_WIDTH ); 

    int content = DsRadarMsg::RADAR_BEAM;  
    if( _radarQueue.putDsMsg(_radarMsg, content ) != 0 ) {  
      fprintf(stderr," Failed to send beam data.\n");
      exit(-1);
    }

    _beamsSent++;

    if (_beamsSent % _params->beamsPerParams == 0){

      //
      // Send the radar configuration at the start of each tilt.
      //
      DsRadarParams& radarParams = _radarMsg.getRadarParams();
      radarParams.numFields = _numFieldsOut;
      radarParams.radarName = _params->radarSpec.name;
      radarParams.latitude = _lat;
      radarParams.longitude = _lon;
      radarParams.altitude = _alt;
      radarParams.radarId = _params->radarSpec.ID;
      radarParams.numGates = _numBins;
      
      radarParams.gateSpacing = _binSpacingKm;
      radarParams.startRange = _rangeFirstBinKm;
      
      radarParams.horizBeamWidth = _horizWidth;
      radarParams.vertBeamWidth = _vertWidth;
      radarParams.pulseWidth = _pulseWidthMicroSec;
      radarParams.pulseRepFreq = _prfHz;
      radarParams.wavelength = _waveLen;
      radarParams.unambigVelocity = _nyquistVel;
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
      
      for(int ifld=0; ifld <  _numFieldsOut; ifld++){
	fieldParams.push_back( _fieldParams[ifld] );
      }
      
      
      content =  DsRadarMsg::FIELD_PARAMS;
      if( _radarQueue.putDsMsg(_radarMsg, content ) != 0 ) {  
	fprintf(stderr," Failed to send radar flags.\n");
	exit(-1);
      }
    }


    //    sleep(1);
    //    cerr << "BEAM SENT AT " << az << ", " << el << endl;
  }

  if (_beamData != NULL) free(_beamData);

  _tiltNum++;
  return 0;

}

//
// Destructor. Does little, but avoids default destructor.
//
processIrisVol::~ processIrisVol(){
  return;
}



