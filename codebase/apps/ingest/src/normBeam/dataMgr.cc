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


#include <Fmq/DsRadarQueue.hh>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>

#include "dataMgr.hh"
#include "normBeam.hh"

using namespace std;


// Constructor

dataMgr::dataMgr(int argc, char **argv)

{

  isOK = true;
  _numGetDsBeamFails = 0;

 _startRange = 0.0;
  _gateSpacing = -1.0;
  _numGates = 0;
  _scale = NULL;
  _bias = NULL;
  _missingVal = NULL;
  _byteWidth = NULL;
  _fieldNumToProcess = -1;
  _numFields = 0;

  for (int i=0; i < MAX_FIELDS; i++){
    _outFieldParams[i] = NULL;
  }

  // set programe name

  _progName = "normBeam";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

dataMgr::~dataMgr()

{

  if (_scale != NULL) free(_scale);
  if (_bias != NULL) free(_bias);
  if (_missingVal != NULL) free(_missingVal);
  if (_byteWidth != NULL) free(_byteWidth);

  /* -------- Don't need to worry about this delete I think --
  for (int i=0; i < MAX_FIELDS; i++){
    if (_outFieldParams[i] != NULL){
      delete _outFieldParams[i];
      _outFieldParams[i] = NULL;
    }
  }
  ------------------------------------*/


  // unregister process
  PMU_auto_unregister();
  return;
}

//////////////////////////////////////////////////
// Run

int dataMgr::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  while (true) {
    _run();
    sleep(10);
    cerr << "dataMgr::Run:" << endl;
    cerr << "  Trying to contact input server at url: "
	 << _params.fmq_url << endl;
  }

  return (0);

}

//////////////////////////////////////////////////
// _run

int dataMgr::_run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // Instantiate and initialize the DsRadar queue and message
  
  DsRadarQueue radarQueue;
  DsRadarQueue outputQueue;

  DsRadarMsg radarMsg;
  DsFmq::openPosition open_pos;
  
  if (_params.seek_to_start_of_input) {
    open_pos = DsFmq::START;
  } else {
    open_pos = DsFmq::END;
  }

  //
  // Init the input queue. Do this repeatedly sine it may be
  // that the writer has not yet started.
  //
  int iret;
  do {
  
    PMU_auto_register("Init input queue");

    iret = radarQueue.init(_params.fmq_url, _progName.c_str(),
			   _params.debug,
			   DsFmq::READ_ONLY, open_pos);

    if (iret) {
      cerr << "ERROR - dataMgr::_run()" << endl;
      cerr << "  Could not initialize radar queue: "
	   << _params.fmq_url << endl;
      cerr << "Could be that writer has not started - trying again..." << endl;
      sleep(1);
    }

  } while (iret);


  //
  // Init the output queue.
  //
  iret = outputQueue.init(_params.output_fmq_url,
			  _progName.c_str(),
			  _params.debug,           
			  DsFmq::READ_WRITE, DsFmq::END,
			  _params.output_fmq_compress,
			  _params.output_fmq_nslots,
			  _params.output_fmq_size, -1);
  
  if (iret) {
    cerr << "ERROR - dataMgr::dataMgr()" << endl;
    cerr << "  Could not initialize radar queue: "
         << _params.output_fmq_url << endl;
    return -1;
  }
  
  if (_params.blockingFmqWrites)
    outputQueue.setBlockingWrite();

  int contents;
  while (true) { 

    PMU_auto_register("Reading radar queue");
    
    // get a message from the radar queue

    int radarQueueRetVal = radarQueue.getDsMsg( radarMsg, &contents );

    if (radarQueueRetVal) {
      
      cerr << "radarQueue:getDsBeam() failed, returned "
	   << radarQueueRetVal << endl;
      //
      // Keep count of consecuive failures.
      //
      _numGetDsBeamFails++;
      //
      // If we have maxed out, it is safe to assume that the program is
      // constantly failing. Exiting and restarting may solve this,
      // assuming the restarter is running.
      //
      if (_numGetDsBeamFails == _maxNumGetDsBeamFails){
	cerr << "The program is failing consistently, exiting ..." << endl;
	exit(-1);
      }
      sleep (1);

    } else { 

      //
      // getDsBeam succeded, reset the count of consecutive failures.
      //
      _numGetDsBeamFails = 0;

      // print beam or flag info

      if (_params.debug){
	if (contents & DsRadarMsg::RADAR_PARAMS)   cerr << "RECEIVED RADAR PARAMS" << endl;
	if (contents & DsRadarMsg::FIELD_PARAMS)   cerr << "RECEIVED FIELD PARAMS" << endl;
	if (contents & DsRadarMsg::RADAR_FLAGS)   cerr << "RECEIVED RADAR FLAGS" << endl;
	if (contents & DsRadarMsg::RADAR_BEAM)   cerr << "RECEIVED BEAM DATA" << endl;
      }

      int outputContents = 0;
      DsRadarMsg outputMsg;
      //
      // !!!!!!!!!!!!!! RADAR PARAMS !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      //  
      if (contents & DsRadarMsg::RADAR_PARAMS) {

	const DsRadarParams &radarParams = radarMsg.getRadarParams();	
	_gateSpacing = radarParams.gateSpacing;
	_numGates = radarParams.numGates;
	_startRange = radarParams.startRange;

	DsRadarParams& outputParams = outputMsg.getRadarParams();

	outputParams = radarParams;
	
	outputContents = outputContents | DsRadarMsg::RADAR_PARAMS;
	
      }

      //
      // !!!!!!!!!!!!!! RADAR FLAGS !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      //  
      if (contents & DsRadarMsg::RADAR_FLAGS) {

	outputContents =  DsRadarMsg::RADAR_FLAGS;
	DsRadarFlags& outputFlags = outputMsg.getRadarFlags();
	outputFlags = radarMsg.getRadarFlags();
	outputContents = outputContents | DsRadarMsg::RADAR_FLAGS;

      }


      //
      // !!!!!!!!!!!!!! FIELD PARAMS !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      //
      if (contents & DsRadarMsg::FIELD_PARAMS){

	const vector< DsFieldParams* > &fieldParams = radarMsg.getFieldParams();
	vector< DsFieldParams* > &outParams = outputMsg.getFieldParams();

	if (_scale != NULL) free(_scale);
	if (_bias != NULL) free(_bias);
	if (_missingVal != NULL) free(_missingVal);
	if (_byteWidth != NULL) free(_byteWidth);

	_scale = (float *) malloc(sizeof(float)*fieldParams.size());
	_bias  = (float *) malloc(sizeof(float)*fieldParams.size());

	_byteWidth = (int *) malloc(sizeof(int)*fieldParams.size());
	_missingVal = (int *) malloc(sizeof(int)*fieldParams.size());

	/* ----- It looks like I don't need to do this delete, see note below ----------
	for (int i=0; i < MAX_FIELDS; i++){
	  if (_outFieldParams[i] != NULL){
	    cerr << "Deleting field params " << i+1 << " which is " <<  _outFieldParams[i] << endl;
	    delete _outFieldParams[i];
	    cerr << "Overwriting field params " << i+1 << endl;
	    _outFieldParams[i] = NULL;
	  }
	}
	----------------------------------------------------------*/

	if ((_bias == NULL) || (_scale == NULL) || (_byteWidth == NULL) || (_missingVal == NULL)){
	  cerr << "Malloc failed!" << endl;
	  exit(-1);
	}

	outParams.clear();
	_numFields = fieldParams.size();

	for (unsigned i=0; i < fieldParams.size(); i++){

	  const DsFieldParams *rfld = radarMsg.getFieldParams(i);

	  //
	  // Keep a record of what came in for when we have to unwrap the beam data.
	  //
	  _scale[i] = rfld->scale;
	  _bias[i] = rfld->bias;
	  _missingVal[i] = rfld->missingDataValue;
	  _byteWidth[i] = rfld->byteWidth;
	  
	  //
	  // Overwrite what came in with what we know we'll be writing out.
	  //
	  _outFieldParams[i] = new DsFieldParams(rfld->name.c_str(), rfld->units.c_str(), _outputScale, _outputBias,
						 _outputByteWidth, _outputMissingVal );

	  //
	  // If this is the field number to process, remember that.
	  //
	  if (!(strcmp(_params.fieldName, rfld->name.c_str()))){
	    _fieldNumToProcess = i;
	  }
	
	  //
	  // It seems like once I do this push_back, I don't have to
	  // concern myself with deleting the pointers I allocated with new();
	  // It seems like the outParams object owns them now.
	  //
	  outParams.push_back( _outFieldParams[i] );
	}

	if (_params.debug){
	  cerr << fieldParams.size() << " field params :" << endl;
	  for (unsigned i=0; i < fieldParams.size(); i++){
	    cerr << "Field params " << i+1 << " :" << endl;
	    _outFieldParams[i]->print( cerr );
	  }
	  cerr << "Sending on params for " << outParams.size() << " fields." << endl;

	  cerr << endl << endl << endl;
	  
	  cerr << "OUTPUT FIELD PARAMS :" << endl;
	  for (unsigned i=0; i < outParams.size(); i++){
	    cerr << "Output field params " << i+1 << " :" << endl;
	    const DsFieldParams *ofld = outputMsg.getFieldParams(i);
	    ofld->print( cerr );
	  }
	  
	}
       
	outputContents = outputContents | DsRadarMsg::FIELD_PARAMS;

      } // End of dealing with field parameters.

      //
      // !!!!!!!!!!!!!! BEAM DATA !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      //
      if (contents & DsRadarMsg::RADAR_BEAM){

	float *newData = (float *)malloc(sizeof(float)*_numFields*_numGates);
	if (newData == NULL){
	  cerr << "Malloc failed!" << endl;
	  exit(-1);
	}

	const DsRadarBeam &radarBeam = radarMsg.getRadarBeam();
	
	int iRet = _getNewBeam( radarBeam.getData(), newData);
	//
	// If that went well, send the beam.
	//
	if (iRet == 0){
	  DsRadarBeam& outputBeam = outputMsg.getRadarBeam();

	  outputBeam.volumeNum  = radarBeam.volumeNum;
	  outputBeam.tiltNum  = radarBeam.tiltNum;
	  outputBeam.azimuth  = radarBeam.azimuth;
	  outputBeam.elevation  = radarBeam.elevation;
	  outputBeam.targetElev  = radarBeam.targetElev;
	  outputBeam.byteWidth  = _outputByteWidth;

	  long timeDiff = radarBeam.dataTime - radarBeam.referenceTime;

	  if (_params.useRealTime){
	    outputBeam.dataTime  = time(NULL);
	  } else {
	    outputBeam.dataTime  = radarBeam.dataTime;
	  }
	  outputBeam.referenceTime = outputBeam.dataTime - timeDiff;

	  int dataLen = _numFields * _numGates * _outputByteWidth;

	  outputBeam.loadData( newData, dataLen, _outputByteWidth);

	  if (_params.debug >= Params::DEBUG_DATA){
	    for (int ifld=0; ifld < _numFields; ifld++){
	      char fieldName[256];
	      sprintf(fieldName,"Field %d", ifld);
	      outputBeam.printFl32( stderr, fieldName, ifld, _numFields, _outputMissingVal );
	    }
	  }
	    

	  outputContents = outputContents | DsRadarMsg::RADAR_BEAM;
 
	} else {
	  cerr << "WARNING - normalization processing failed." << endl;
	}
	
	free(newData);

      }

      if (outputContents){

	if (_params.debug){
	  if (outputContents & DsRadarMsg::RADAR_PARAMS)   cerr << "SENDING RADAR PARAMS" << endl;
	  if (outputContents & DsRadarMsg::FIELD_PARAMS)   cerr << "SENDING FIELD PARAMS" << endl;
	  if (outputContents & DsRadarMsg::RADAR_FLAGS)   cerr << "SENDING RADAR FLAGS" << endl;
	  if (outputContents & DsRadarMsg::RADAR_BEAM)   cerr << "SENDING BEAM DATA" << endl;
	}
	if( outputQueue.putDsMsg( outputMsg, outputContents ) != 0 ) {  
	  cerr << " Failed to send a radar beam message." << endl;
	  exit(-1);
	}
      }

    } // if (radarQueue ...

  } // while

  return 0;

}

int dataMgr::_getNewBeam(void *oldData, float *newData){


  //
  // Bail out now if thigs are not set up (probably we
  // have just go recived all the messages yet).
  //

  if ((_scale == NULL) || (_bias == NULL) || (_missingVal == NULL)) return -1;
  if ((_numGates == 0) || (_gateSpacing < 0.0) || (_numFields < 1)) return -1;

  //
  // Get a byte pointer to the existing data.
  //
  unsigned char *dataPtr = (unsigned char *) oldData;

  //
  // Assemble the existing data value into a 4 byte wide buffer
  // with uniform missing values. Loop through the gates.
  //
  for (int iGate=0; iGate < _numGates; iGate++){
    
    //
    // Loop through the fields.
    //
    for (int iField=0; iField < _numFields; iField++){

      //
      // Get the data value. Depends on the byte width.
      //
      float val = _outputMissingVal;
      
      float *fdata;
      ui16 *idata;
      ui08 *bdata;

      switch ( _byteWidth[iField] ){

      case 4 :
        fdata = (fl32 *) dataPtr;
        val = *fdata;
        if (val == _missingVal[iField]) val = _outputMissingVal;
        dataPtr +=4;
        break;

    case 2 :
        idata = (ui16 *) dataPtr;
        if ( *idata == _missingVal[iField]) 
          val = _outputMissingVal;
        else
          val = *idata * _scale[iField] + _bias[iField];
        dataPtr+=2;
        break;

      case 1 :
        bdata = (ui08 *) dataPtr;
        if ( *bdata == _missingVal[iField]) 
          val = _outputMissingVal;
        else
          val = *bdata * _scale[iField] + _bias[iField];
        dataPtr++;
        break;
        
      default :
        cerr << "Datawidth of " << _byteWidth[iField] << " encountered, I cannot cope." << endl;
        exit(-1); // Corrupt FMQ?
        break;

      }
      newData[_numFields*iGate + iField] = val;
    }
  }


  //
  // OK, now pick off the one field that we want to normalize, and
  // normalize it.
  //
  // If we don't have a field number to process - ie. we didn't
  // see the specified field name - just bail now.
  //

  if (_fieldNumToProcess < 0) return 0;

  float *inField = (float *) malloc(sizeof(float) * _numGates);
  float *outField = (float *) malloc(sizeof(float) * _numGates);
  if ((inField == NULL) || (outField == NULL)){
    cerr << "Malloc failed!" << endl;
    exit(-1); // Unlikely.
  }

  for (int iGate=0; iGate < _numGates; iGate++){
    inField[iGate] = newData[_numFields*iGate + _fieldNumToProcess];
  }

  if (!(_params.allowMissing)){
    _interpBeam(inField, _numGates, _params.dataWindow.minVal, _params.dataWindow.maxVal);
  }

  int debug = 0;
  if (_params.debug) debug = 1;
  if (_params.debug >= Params::DEBUG_NORMALIZATION) debug = 3;

  int iRet = normBeam(inField, outField, _gateSpacing*1000.0, 
                      _params.startRange, _params.endRange, _params.percentToTake,
                      _params.displayOffset, _numGates, debug,
                      _params.dataWindow.minVal,
		      _params.dataWindow.maxVal, _startRange,
		      _params.minPercentGood);

  if (iRet) return -1;

  //
  // Insert this back into the data.
  //
  for (int iGate=0; iGate < _numGates; iGate++){
    newData[_numFields*iGate + _fieldNumToProcess] = outField[iGate];
  }

  free(inField); free(outField);

  return 0;

}

void dataMgr::_interpBeam(float *inField, 
			  int numGates,
			  double minVal,
			  double maxVal){

  //
  // Find the first non-missing value in the beam,
  // return if there are none.
  //

  int firstNonMissingIndex = -1;
  do {
    firstNonMissingIndex++;
  } while ((firstNonMissingIndex < numGates) &
	   ((inField[firstNonMissingIndex] < minVal) || (inField[firstNonMissingIndex] > maxVal)));

  //
  // If we did not find one, fill beam with minVal and return.
  //
  if (!(firstNonMissingIndex < numGates)){
    for (int i=0; i < numGates; i++){
      inField[i] = minVal;
    }
    return;
  }

  //
  // Replace any leading missing values with the first non-missing value.
  //
  for (int i=0; i < firstNonMissingIndex; i++){
    inField[i] = inField[firstNonMissingIndex];
  }

  //
  // Search for gaps, and interpolate.
  //
  for (int i=firstNonMissingIndex+1; i < numGates; i++){

    if ((inField[i] < minVal) || (inField[i] > maxVal)){

      //
      // Race ahead to find the next non-missing value.
      //
      int j = i;
      do {
	j++;
      } while ((j < numGates) &
	       ((inField[j] < minVal) || (inField[j] > maxVal)));

      //
      // If we did not find one, fill the rest of the beam
      // with the last value and leave.
      //
      if (!(j<numGates)){
	for (int k=i; k < numGates; k++){
	  inField[k] = inField[i-1];
	}
	return;
      } else {
	//
	// If we did find one, interpolate between i-1 and j
	//
	for (int k=i; k < j; k++){
	  double t = (k - (i-1))/(j - (i-1));
	  inField[k] = t*inField[j] + (1.0-t)*inField[i-1];
	}
      }
    }
  }

  return;

}
