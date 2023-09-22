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
////////////////////////////////////////////////////////////////////////
// Cov2Mom.cc
//
// Cov2Mom object
// Some methods are in transform.cc and geom.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2008
//
///////////////////////////////////////////////////////////////////////
//
// Cov2Mom reads covariances from a DsRadar FMQ,
// computes moments, writes these out to a DsRadar FMQ
//
///////////////////////////////////////////////////////////////////////

#include <cmath>
#include <toolsa/pmu.h>
#include <toolsa/utim.h>
#include <toolsa/uusleep.h>
#include <toolsa/ucopyright.h>
#include <toolsa/MemBuf.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/toolsa_macros.h>
#include <toolsa/sincos.h>
#include <radar/FilterUtils.hh>
#include <radar/MomentsFields.hh>
#include <iomanip>
#include "Cov2Mom.hh"

const double Cov2Mom::_missingDouble = -9999.0;
const double Cov2Mom::_missingTest = -9998.0;

using namespace std;

// Constructor

Cov2Mom::Cov2Mom(int argc, char **argv)

{
  
  isOK = true;
  _inputQueue = NULL;
  _inputContents = 0;
  _inputNFail = 0;
  _outputQueue = NULL;
  _nFieldsIn = 0;
  _nFieldsOut = 0;
  _nGates = 0;
  _volNum = 0;
  _debugPrintNeedsNewline = false;

  // set programe name
  
  _progName = "Cov2Mom";
  ucopyright((char *) _progName.c_str());

  // get command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  // set up params indices

  _setCovParamsIndex(Params::LAG0_HC_DB, _lag0_hc_db);
  _setCovParamsIndex(Params::LAG0_HX_DB, _lag0_hx_db);
  _setCovParamsIndex(Params::LAG0_VC_DB, _lag0_vc_db);
  _setCovParamsIndex(Params::LAG0_VX_DB, _lag0_vx_db);

  _setCovParamsIndex(Params::LAG0_VCHX_DB, _lag0_vchx_db);
  _setCovParamsIndex(Params::LAG0_VCHX_PHASE, _lag0_vchx_phase);

  _setCovParamsIndex(Params::LAG0_HCVX_DB, _lag0_hcvx_db);
  _setCovParamsIndex(Params::LAG0_HCVX_PHASE, _lag0_hcvx_phase);

  _setCovParamsIndex(Params::LAG0_VXHX_DB, _lag0_vxhx_db);
  _setCovParamsIndex(Params::LAG0_VXHX_PHASE, _lag0_vxhx_phase);

  _setCovParamsIndex(Params::LAG1_HC_DB, _lag1_hc_db);
  _setCovParamsIndex(Params::LAG1_HC_PHASE, _lag1_hc_phase);
  _setCovParamsIndex(Params::LAG1_VC_DB, _lag1_vc_db);
  _setCovParamsIndex(Params::LAG1_VC_PHASE, _lag1_vc_phase);

  _setCovParamsIndex(Params::LAG1_HCVC_DB, _lag1_hcvc_db);
  _setCovParamsIndex(Params::LAG1_HCVC_PHASE, _lag1_hcvc_phase);
  _setCovParamsIndex(Params::LAG1_VCHC_DB, _lag1_vchc_db);
  _setCovParamsIndex(Params::LAG1_VCHC_PHASE, _lag1_vchc_phase);
  _setCovParamsIndex(Params::LAG1_VXHX_DB, _lag1_vxhx_db);
  _setCovParamsIndex(Params::LAG1_VXHX_PHASE, _lag1_vxhx_phase);

  _setCovParamsIndex(Params::LAG2_HC_DB, _lag2_hc_db);
  _setCovParamsIndex(Params::LAG2_HC_PHASE, _lag2_hc_phase);
  _setCovParamsIndex(Params::LAG2_VC_DB, _lag2_vc_db);
  _setCovParamsIndex(Params::LAG2_VC_PHASE, _lag2_vc_phase);

  _setCovParamsIndex(Params::LAG3_HC_DB, _lag3_hc_db);
  _setCovParamsIndex(Params::LAG3_HC_PHASE, _lag3_hc_phase);
  _setCovParamsIndex(Params::LAG3_VC_DB, _lag3_vc_db);
  _setCovParamsIndex(Params::LAG3_VC_PHASE, _lag3_vc_phase);
  
}

/////////////////////////////////////////////////////////
// destructor

Cov2Mom::~Cov2Mom()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Cov2Mom::Run ()
{
  
  // register with procmap
  
  PMU_auto_register("Run");
  
  while (true) {
    if (_params.debug) {
      cerr << "Cov2Mom::Run:" << endl;
      cerr << "  Reading input queue at url: "
	   << _params.input_fmq_url << endl;
    }
    _run();
    umsleep(1000);
  }

  return 0;

}

//////////////////////////////////////////////////
// _run

int Cov2Mom::_run ()
{

  // register with procmap
  
  PMU_auto_register("_run");

  // open the input queue

  if (_openInputQueue()) {
    return -1;
  }

  // open the output queue

  if (_openOutputQueue()) {
    return -1;
  }

  while (true) { 
    
    PMU_auto_register("Reading radar queue");
    
    // get a message from the radar queue
    
    int iret = _inputQueue->getDsMsg(_inputMsg, &_inputContents);

    if (iret) {

      cerr << "ERROR - Cov2Mom" << endl;
      cerr << "  inputQueue:getDsBeam() failed, retval: " << iret << endl;
      
      // Keep count of consecuive failures.
      _inputNFail++;

      // If we have maxed out, it is safe to assume that the program is
      // constantly failing. Exiting and restarting may solve this,
      // assuming the restarter is running.
      if (_inputNFail > 100) {
	cerr << "The program is failing consistently, reopen the queues ..." << endl;
	return -1;
      }

      umsleep(1000);

    } else { 

      // getDsBeam succeded, reset the count of consecutive failures.
      _inputNFail = 0;

      // process the message

      _processInputMessage();

    } // if (inputQueue ...

  } // while

  return 0;

}

////////////////////////////////////////////////////////////////
// open the input queue
//
// Returns 0 on success, -1 on failure

int Cov2Mom::_openInputQueue()

{
  
  // if already open, delete queue to close it

  if (_inputQueue != NULL) {
    delete _inputQueue;
  }

  // create queue

  _inputQueue = new DsRadarQueue();

  // initialize the input DsRadar queue and message
  
  DsFmq::openPosition open_pos;
  if (_params.seek_to_start_of_input) {
    open_pos = DsFmq::START;
  } else {
    open_pos = DsFmq::END;
  }
  if (_inputQueue->init(_params.input_fmq_url, _progName.c_str(),
			_params.debug,
			DsFmq::READ_ONLY, open_pos)) {
    cerr << "ERROR - Cov2Mom" << endl;
    cerr << "  Could not initialize input radar queue: "
	 << _params.input_fmq_url << endl;
    delete _inputQueue;
    _inputQueue = NULL;
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////////
// open the output queue
//
// Returns 0 on success, -1 on failure

int Cov2Mom::_openOutputQueue()

{
  
  // if already open, delete queue to close it

  if (_outputQueue != NULL) {
    delete _outputQueue;
  }

  // create queue

  _outputQueue = new DsRadarQueue();

  // initialize the output DsRadar queue
  
  if (_outputQueue->init(_params.output_fmq_url,
			 _progName.c_str(),
			 _params.debug >= Params::DEBUG_VERBOSE,
			 DsFmq::READ_WRITE, DsFmq::END,
			 _params.output_fmq_compress,
			 _params.output_fmq_nslots,
			 _params.output_fmq_size)) {
    cerr << "ERROR - " << _progName << "::OutputFmq" << endl;
    cerr << "  Cannot open fmq, URL: " << _params.output_fmq_url << endl;
    delete _outputQueue;
    _outputQueue = NULL;
    return -1;
  }
  if (_params.output_fmq_compress) {
    _outputQueue->setCompressionMethod(TA_COMPRESSION_ZLIB);
  }

  // set up the output fields

  _nFieldsOut = 0;
  for (int ii = 0; ii < _params.output_fields_n; ii++) {
    const Params::output_field_t &field = _params._output_fields[ii];
    // field params, byte width 2, missing value 0
    string fieldName(field.name);
    if (_params.is_filtered) {
      fieldName += "_F";
    }
    DsFieldParams fparams(fieldName.c_str(), field.units,
			  field.scale, field.bias, 2, 0);
    _outputMsg.addFieldParams(fparams);
    _nFieldsOut++;
  }
    
  return 0;

}

////////////////////////////////////////////////////////////////
// process the input message
// Returns 0 on success, -1 on failure

int Cov2Mom::_processInputMessage()

{

  // copy flags from input queue to output queue
  
  if (_inputContents & DsRadarMsg::RADAR_FLAGS) {
    _copyFlags();
  }

  // modify radar params for number of fields, write to output queue

  if (_inputContents & DsRadarMsg::RADAR_PARAMS) {
    _inputRadarParams =  _inputMsg.getRadarParams();
    _nFieldsIn = _inputRadarParams.numFields;
    _nGates = _inputRadarParams.numGates;
    DsRadarParams outputParams = _inputRadarParams;
    outputParams.numFields = _nFieldsOut;
    _outputMsg.setRadarParams(outputParams);
    if (_outputQueue->putDsMsg(_outputMsg, DsRadarMsg::RADAR_PARAMS)) {
      cerr << "ERROR - OutputFmq::processInputMessage" << endl;
      cerr << "  Cannot put radar params to queue" << endl;
      return -1;
    }
    if (_params.debug) {
      _printDebugMsg("Wrote radar params");
    }
  }

  // write fields params to output queue

  if (_inputContents & DsRadarMsg::FIELD_PARAMS) {
    _inputFieldParams =  _inputMsg.getFieldParams();
    if (_outputQueue->putDsMsg(_outputMsg, DsRadarMsg::FIELD_PARAMS)) {
      cerr << "ERROR - OutputFmq::processInputMessage" << endl;
      cerr << "  Cannot put field params to queue" << endl;
      return -1;
    }
    if (_params.debug) {
      _printDebugMsg("Wrote field params");
    }
  }

  // write radar calib to output queue

  if (_inputContents & DsRadarMsg::RADAR_CALIB) {
    _inputRadarCalib =  _inputMsg.getRadarCalib();
    _outputMsg.setRadarCalib(_inputRadarCalib);
    if (_outputQueue->putDsMsg(_outputMsg, DsRadarMsg::RADAR_CALIB)) {
      cerr << "ERROR - OutputFmq::processInputMessage" << endl;
      cerr << "  Cannot put calib to queue" << endl;
      return -1;
    }
    if (_params.debug) {
      _printDebugMsg("Wrote radar calib");
    }
  }

  // process a beam
  
  if (_inputContents & DsRadarMsg::RADAR_BEAM) {
    if (_processBeam()) {
      cerr << "ERROR - OutputFmq::processInputMessage" << endl;
      cerr << "  Cannot process beam" << endl;
      return -1;
    }
  }

  return 0;

}
      
////////////////////////////////////////////////////////////////
// copy flags in the input message to output message

void Cov2Mom::_copyFlags()

{

  // copy the flags to the output queue

  const DsRadarFlags &flags = _inputMsg.getRadarFlags();
  
  if (flags.startOfTilt) {
    _outputQueue->putStartOfTilt(flags.tiltNum, flags.time);
    if (_params.debug) {
      _printDebugMsg("Start of tilt");
    }
  }
  
  if (flags.endOfTilt) {
    _outputQueue->putEndOfTilt(flags.tiltNum, flags.time);
    if (_params.debug) {
      _printDebugMsg("End of tilt");
    }
  }
  
  if (flags.startOfVolume) {
    _outputQueue->putStartOfVolume(flags.volumeNum, flags.time);
    if (_params.debug) {
      _printDebugMsg("Start of volume");
    }
  }
  
  if (flags.endOfVolume) {
    _outputQueue->putEndOfVolume(flags.volumeNum, flags.time);
    if (_params.debug) {
      _printDebugMsg("End of volume");
    }
  }
  
  if (flags.newScanType) {
    _outputQueue->putNewScanType(flags.scanType, flags.time);
    if (_params.debug) {
      _printDebugMsg("New scan type");
    }
  }
  
}
      
////////////////////////////////////////////////////////////////
// process a beam
// Returns 0 on success, -1 on failure

int Cov2Mom::_processBeam()

{

  const DsRadarBeam &beam = _inputMsg.getRadarBeam();
  
  // check for new vol

  if (_volNum != beam.volumeNum) {
    _volNum = beam.volumeNum;
  }

  // allocate the data arrays
  
  _allocateArrays(_nGates);

  // load up the covariance data for each field

  _loadCovData();

  // compute moments

  _computeMoments();

  // write beam with precip rates
  
  if (_writeBeam()) {
    return -1;
  }
  
  return 0;

}

////////////////////////////////////////////////////////////////
// set indices for for a specified field
// Sets indices to -1 if field is not in params list.

void Cov2Mom::_setCovIndices(Params::covariance_id_t paramId,
			     cov_field_t &field)
  
{

  _setCovParamsIndex(paramId, field);
  _setCovDataIndex(field);

}
  
////////////////////////////////////////////////////////////////
// set index in param file for a specified field
// Sets indices to -1 if field is not in params list.

void Cov2Mom::_setCovParamsIndex(Params::covariance_id_t paramId,
				 cov_field_t &field)

{
  
  field.id = paramId;
  
  // get the DsrName

  field.dsrName = _getCovParamsName(paramId);
  
  // is this field in the param file?
  
  if (field.dsrName.size() < 1) {
    field.paramsIndex = -1;
    field.dataIndex = -1;
  }
  
  field.paramsIndex = _getCovParamsIndex(field.dsrName);

}

////////////////////////////////////////////////////////////////
// set index for a specified field in input data
// Sets data index to -1 if field is not in data

void Cov2Mom::_setCovDataIndex(cov_field_t &field)

{
  
  field.dataIndex = _getInputDataIndex(field.dsrName);
}

////////////////////////////////////////////////////////////////
// load up the covariance data for each field

void Cov2Mom::_loadCovData()

{

  // load up the covariance data for each field

  _loadCovData(_lag0_hc_db);
  _loadCovData(_lag0_hx_db);
  _loadCovData(_lag0_vc_db);
  _loadCovData(_lag0_vx_db);
  _loadCovData(_lag0_hcvx_db);
  _loadCovData(_lag0_hcvx_phase);
  _loadCovData(_lag0_vchx_db);
  _loadCovData(_lag0_vchx_phase);
  _loadCovData(_lag0_vxhx_db);
  _loadCovData(_lag0_vxhx_phase);
  _loadCovData(_lag1_hc_db);
  _loadCovData(_lag1_hc_phase);
  _loadCovData(_lag1_vc_db);
  _loadCovData(_lag1_vc_phase);
  _loadCovData(_lag1_hcvc_db);
  _loadCovData(_lag1_hcvc_phase);
  _loadCovData(_lag1_vchc_db);
  _loadCovData(_lag1_vchc_phase);
  _loadCovData(_lag1_vxhx_db);
  _loadCovData(_lag1_vxhx_phase);
  _loadCovData(_lag2_hc_db);
  _loadCovData(_lag2_hc_phase);
  _loadCovData(_lag2_vc_db);
  _loadCovData(_lag2_vc_phase);
  _loadCovData(_lag3_hc_db);
  _loadCovData(_lag3_hc_phase);
  _loadCovData(_lag3_vc_db);
  _loadCovData(_lag3_vc_phase);

  // compute powers
  
  _loadPower(_lag0_hc_db.data, _lag0_hc);
  _loadPower(_lag0_hx_db.data, _lag0_hx);
  _loadPower(_lag0_vc_db.data, _lag0_vc);
  _loadPower(_lag0_vx_db.data, _lag0_vx);

  // compute the complex data values

  _loadComplex(_lag0_hcvx_db.data, _lag0_hcvx_phase.data, _lag0_hcvx);
  _loadComplex(_lag0_vchx_db.data, _lag0_vchx_phase.data, _lag0_vchx);
  
  _loadComplex(_lag1_hc_db.data, _lag1_hc_phase.data, _lag1_hc);
  _loadComplex(_lag1_vc_db.data, _lag1_vc_phase.data, _lag1_vc);

  _loadComplex(_lag1_hcvc_db.data, _lag1_hcvc_phase.data, _lag1_hcvc);
  _loadComplex(_lag1_vchc_db.data, _lag1_vchc_phase.data, _lag1_vchc);
  _loadComplex(_lag1_vxhx_db.data, _lag1_vxhx_phase.data, _lag1_vxhx);

  _loadComplex(_lag2_hc_db.data, _lag2_hc_phase.data, _lag2_hc);
  _loadComplex(_lag2_vc_db.data, _lag2_vc_phase.data, _lag2_vc);

  _loadComplex(_lag3_hc_db.data, _lag3_hc_phase.data, _lag3_hc);
  _loadComplex(_lag3_vc_db.data, _lag3_vc_phase.data, _lag3_vc);

}

////////////////////////////////////////////////////////////////
// load the covariance data
// Set to missing if not available

void Cov2Mom::_loadCovData(cov_field_t &field)

{

  // allocate data array

  if (_nGates > (int) field.data_.size()) {
    field.data_.free();
    field.data = (double *) field.data_.alloc(_nGates);
  }
  
  // get index in data

  field.dataIndex = _getInputDataIndex(field.dsrName);

  // load up data

  const DsRadarBeam &beam = _inputMsg.getRadarBeam();
  _loadInputField(beam, field.dataIndex, field.data);

}

////////////////////////////////////////////////////////////////
// get Dsr field name for specified covariance ID
// Returns empty string on failure

string Cov2Mom::_getCovParamsName(Params::covariance_id_t paramId)
  
{

  for (int ii = 0; ii < _params.input_fields_n; ii++) {
    if (_params._input_fields[ii].id == paramId) {
      return _params._input_fields[ii].dsr_name;
    }
  } // ii 

  return "";
  
}

////////////////////////////////////////////////////////////////
// get params ID for specified covariance name
// Returns -1 on failure

int Cov2Mom::_getCovParamsIndex(const string &dsrName)
  
{

  // find the params index of a covariance field

  for (int ii = 0; ii < _params.input_fields_n; ii++) {
    string paramName = _params._input_fields[ii].dsr_name;
    if (paramName == dsrName) {
      return ii;
    }
  } // ii 

  return -1;
  
}

////////////////////////////////////////////////////////////////
// get data index for a specified field name
// Returns -1 on failure

int Cov2Mom::_getInputDataIndex(const string &dsrName)

{

  // compute the indices of each input field in the Dsr beam
  
  const vector<DsFieldParams*> &fieldParams = _inputMsg.getFieldParams();
  for (int ii = 0; ii < (int) fieldParams.size(); ii++) {
    if (dsrName == fieldParams[ii]->name) {
      return ii;
    }
  }

  return -1;

}

////////////////////////////////////////////////////////////////
// load field data

void Cov2Mom::_loadInputField(const DsRadarBeam &beam, int index, double *fldData)

{

  // start by filling with missing values

  for (int ii = 0; ii < _nGates; ii++) {
    fldData[ii] = _missingDouble;
  }

  if (index < 0) {
    // field not in input data
    return;
  }

  if (beam.byteWidth == 4) {

    fl32 *fval = (fl32 *) beam.getData() + index;
    for (int ii = 0; ii < _nGates; ii++, fval += _nFieldsIn) {
      fldData[ii] = *fval;
    }

  } else if (beam.byteWidth == 2) {

    double scale = _inputFieldParams[index]->scale;
    double bias = _inputFieldParams[index]->bias;
    ui16 *sval = (ui16 *) beam.getData() + index;
    for (int ii = 0; ii < _nGates; ii++, sval += _nFieldsIn) {
      fldData[ii] = *sval * scale + bias;
    }

  } else {

    double scale = _inputFieldParams[index]->scale;
    double bias = _inputFieldParams[index]->bias;
    ui08 *bval = (ui08 *) beam.getData() + index;
    for (int ii = 0; ii < _nGates; ii++, bval += _nFieldsIn) {
      fldData[ii] = *bval * scale + bias;
    }

  }

}

////////////////////////////////////////////////////////////////
// load power values from db

void Cov2Mom::_loadPower(const double *db, double *power)
  
{
  
  for (int ii = 0; ii < _nGates; ii++) {
    power[ii] = pow(10.0, db[ii] / 10.0);
  }

}

////////////////////////////////////////////////////////////////
// load complex values from db and phase

void Cov2Mom::_loadComplex(const double *db,
			   const double *phase,
			   RadarComplex_t *complex)
  
{
  
  for (int ii = 0; ii < _nGates; ii++) {

    double mag = pow(10.0, db[ii] / 20.0);
    double sinval, cosval;
    ta_sincos(phase[ii] * DEG_TO_RAD, &sinval, &cosval);

    complex[ii].re = mag * cosval;
    complex[ii].im = mag * sinval;

  }

}

///////////////////////
// write out beam

int Cov2Mom::_writeBeam()

{

  // input beam

  const DsRadarBeam &inBeam = _inputMsg.getRadarBeam();

  // output beam

  DsRadarBeam &outBeam = _outputMsg.getRadarBeam();
  outBeam.copyHeader(inBeam);

  // set output byte width - we are using 16-bit integers
  
  outBeam.byteWidth = 2;
  
  // create output data array
  
  int nData = _nFieldsOut * _nGates;
  TaArray<ui16> outData_;
  ui16 *outData = (ui16 *) outData_.alloc(nData);
  memset(outData, 0, nData * sizeof(ui16));
  
  // load up field data

  for (int ii = 0; ii < _params.output_fields_n; ii++) {

    const Params::output_field_t &fld = _params._output_fields[ii];

    switch (fld.id) {

    case Params::NCP:
      _loadOutputField(_ncp, ii, fld.scale, fld.bias, outData);
      break;

    case Params::SNR:
      _loadOutputField(_snr, ii, fld.scale, fld.bias, outData);
      break;

    case Params::DBM:
      _loadOutputField(_dbm, ii, fld.scale, fld.bias, outData);
      break;

    case Params::DBZ:
      _loadOutputField(_dbz, ii, fld.scale, fld.bias, outData);
      break;

    case Params::VEL:
      _loadOutputField(_vel, ii, fld.scale, fld.bias, outData);
      break;

    case Params::WIDTH:
      _loadOutputField(_width, ii, fld.scale, fld.bias, outData);
      break;

    case Params::ZDR:
      _loadOutputField(_zdr, ii, fld.scale, fld.bias, outData);
      break;

    case Params::LDRH:
      _loadOutputField(_ldrh, ii, fld.scale, fld.bias, outData);
      break;

    case Params::LDRV:
      _loadOutputField(_ldrv, ii, fld.scale, fld.bias, outData);
      break;

    case Params::RHOHV:
      _loadOutputField(_rhohv, ii, fld.scale, fld.bias, outData);
      break;

    case Params::PHIDP:
      _loadOutputField(_phidp, ii, fld.scale, fld.bias, outData);
      break;

    case Params::KDP:
      _loadOutputField(_kdp, ii, fld.scale, fld.bias, outData);
      break;

    case Params::SNRHC:
      _loadOutputField(_snrhc, ii, fld.scale, fld.bias, outData);
      break;

    case Params::SNRHX:
      _loadOutputField(_snrhx, ii, fld.scale, fld.bias, outData);
      break;

    case Params::SNRVC:
      _loadOutputField(_snrvc, ii, fld.scale, fld.bias, outData);
      break;

    case Params::SNRVX:
      _loadOutputField(_snrvx, ii, fld.scale, fld.bias, outData);
      break;

    case Params::DBMHC:
      _loadOutputField(_dbmhc, ii, fld.scale, fld.bias, outData);
      break;

    case Params::DBMHX:
      _loadOutputField(_dbmhx, ii, fld.scale, fld.bias, outData);
      break;

    case Params::DBMVC:
      _loadOutputField(_dbmvc, ii, fld.scale, fld.bias, outData);
      break;

    case Params::DBMVX:
      _loadOutputField(_dbmvx, ii, fld.scale, fld.bias, outData);
      break;

    default: {}

    }

  } // ii

  // load output data into beam

  outBeam.loadData(outData, nData * sizeof(ui16), sizeof(ui16));

  // write the beam

  if (_outputQueue->putDsMsg(_outputMsg, DsRadarMsg::RADAR_BEAM)) {
    cerr << "ERROR - OutputFmq::writeBeam" << endl;
    cerr << "  Cannot write output beam to queue" << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _printDebugMsg("Wrote beam, beam_time: ", false);
    _printDebugMsg(DateTime::strm(outBeam.dataTime));
  } else if (_params.debug) {
    cerr << ".";
    _debugPrintNeedsNewline = true;
  }

  return 0;

}

/////////////////////////////////////////////
// load output field data into output array

void Cov2Mom::_loadOutputField(double *fld, int index,
			       double scale, double bias,
			       ui16 *outData)

{

  for (int ii = 0, loc = index; ii < _nGates; ii++, loc += _nFieldsOut) {
    
    double fldVal = fld[ii];
    int scaledVal = 0;
    if (fldVal > _missingTest) {
      scaledVal = (int) floor(((fldVal - bias) / scale) + 0.5);
    }
    if (scaledVal < 0) {
      scaledVal = 0;
    } else if (scaledVal > 65535) {
     scaledVal = 65535;
    }

    outData[loc] = (ui16) scaledVal;

  }

}

////////////////////////////////////////////////////////////////////////
// allocate data arrays

void Cov2Mom::_allocateArrays(int nGates)

{

  if (nGates <= (int) _ncp_.size()) {
    return;
  }

  _ncp_.free();
  _ncp = (double *) _ncp_.alloc(nGates);

  _snr_.free();
  _snr = (double *) _snr_.alloc(nGates);

  _dbm_.free();
  _dbm = (double *) _dbm_.alloc(nGates);

  _dbz_.free();
  _dbz = (double *) _dbz_.alloc(nGates);

  _vel_.free();
  _vel = (double *) _vel_.alloc(nGates);

  _width_.free();
  _width = (double *) _width_.alloc(nGates);

  _zdr_.free();
  _zdr = (double *) _zdr_.alloc(nGates);

  _ldrh_.free();
  _ldrh = (double *) _ldrh_.alloc(nGates);

  _ldrv_.free();
  _ldrv = (double *) _ldrv_.alloc(nGates);

  _rhohv_.free();
  _rhohv = (double *) _rhohv_.alloc(nGates);

  _phidp_.free();
  _phidp = (double *) _phidp_.alloc(nGates);

  _kdp_.free();
  _kdp = (double *) _kdp_.alloc(nGates);

  _snrhc_.free();
  _snrhc = (double *) _snrhc_.alloc(nGates);

  _snrhx_.free();
  _snrhx = (double *) _snrhx_.alloc(nGates);

  _snrvc_.free();
  _snrvc = (double *) _snrvc_.alloc(nGates);

  _snrvx_.free();
  _snrvx = (double *) _snrvx_.alloc(nGates);

  _dbmhc_.free();
  _dbmhc = (double *) _dbmhc_.alloc(nGates);

  _dbmhx_.free();
  _dbmhx = (double *) _dbmhx_.alloc(nGates);

  _dbmvc_.free();
  _dbmvc = (double *) _dbmvc_.alloc(nGates);

  _dbmvx_.free();
  _dbmvx = (double *) _dbmvx_.alloc(nGates);

  _lag0_hc_.free();
  _lag0_hc = (double *) _lag0_hc_.alloc(nGates);

  _lag0_hx_.free();
  _lag0_hx = (double *) _lag0_hx_.alloc(nGates);

  _lag0_vc_.free();
  _lag0_vc = (double *) _lag0_vc_.alloc(nGates);

  _lag0_vx_.free();
  _lag0_vx = (double *) _lag0_vx_.alloc(nGates);

  _lag0_hcvx_.free();
  _lag0_hcvx = (RadarComplex_t *) _lag0_hcvx_.alloc(nGates);

  _lag0_vchx_.free();
  _lag0_vchx = (RadarComplex_t *) _lag0_vchx_.alloc(nGates);

  _lag1_hc_.free();
  _lag1_hc = (RadarComplex_t *) _lag1_hc_.alloc(nGates);

  _lag1_vc_.free();
  _lag1_vc = (RadarComplex_t *) _lag1_vc_.alloc(nGates);

  _lag1_hcvc_.free();
  _lag1_hcvc = (RadarComplex_t *) _lag1_hcvc_.alloc(nGates);

  _lag1_vchc_.free();
  _lag1_vchc = (RadarComplex_t *) _lag1_vchc_.alloc(nGates);

  _lag1_vxhx_.free();
  _lag1_vxhx = (RadarComplex_t *) _lag1_vxhx_.alloc(nGates);

  _lag2_hc_.free();
  _lag2_hc = (RadarComplex_t *) _lag2_hc_.alloc(nGates);

  _lag2_vc_.free();
  _lag2_vc = (RadarComplex_t *) _lag2_vc_.alloc(nGates);

  _lag3_hc_.free();
  _lag3_hc = (RadarComplex_t *) _lag3_hc_.alloc(nGates);

  _lag3_vc_.free();
  _lag3_vc = (RadarComplex_t *) _lag3_vc_.alloc(nGates);

}

////////////////////////////////////////////////////////////////////////
// print a debug message, taking account of whether a newline is needed

void Cov2Mom::_printDebugMsg(const string &msg, bool addNewline /* = true */)

{

  if (_debugPrintNeedsNewline) {
    cerr << endl;
    _debugPrintNeedsNewline = false;
  }
  cerr << msg;
  if (addNewline) {
    cerr << endl;
  }

}

///////////////////////
// compute the moments

void Cov2Mom::_computeMoments()

{

  RadarMoments rmom(_nGates,
		    _params.debug,
		    _params.debug >= Params::DEBUG_VERBOSE);

  const DsRadarParams &rparams = _inputMsg.getRadarParams();
  const DsRadarCalib &calib = _inputMsg.getRadarCalib();

  double prt = 1.0 / rparams.pulseRepFreq;
  double wavelengthKm = rparams.wavelength / 100.0;
  rmom.init(prt, wavelengthKm,
	    rparams.startRange, rparams.gateSpacing);
  
  rmom.setCorrectForSystemPhidp(_params.correct_for_system_phidp);
  rmom.setCalib(calib);

  switch (_params.xmit_rcv_mode) {
    
    // single pol
  case Params::SP:
    _computeMomentsSp(rmom);
    break;
    
    // Dual pol, alternating transmission, copolar receiver only
    // (CP2 SBand)
  case Params::DP_ALT_HV_CO_ONLY:
    _computeMomentsDpAltHvCoOnly(rmom);
    break;
    
    // Dual pol, alternating transmission, co-polar and cross-polar
    // receivers (SPOL with Mitch Switch and receiver in 
    // switching mode, CHILL)
  case Params::DP_ALT_HV_CO_CROSS:
    _computeMomentsDpAltHvCoCross(rmom);
    break;
    
    // Dual pol, alternating transmission, fixed H and V receivers (SPOL
    // with Mitch Switch and receivers in fixed mode)
  case Params::DP_ALT_HV_FIXED_HV:
    _computeMomentsDpAltHvFixedHv(rmom);
    break;
    
    // Dual pol, simultaneous transmission, fixed H and V receivers (NEXRAD
    // upgrade, SPOL with T and receivers in fixed mode)
  case Params::DP_SIM_HV_FIXED_HV:
    _computeMomentsDpSimHvFixedHv(rmom);
    break;
    
    // Dual pol, simultaneous transmission, switching H and V receivers
    // (SPOL with T and receivers in switching mode)
  case Params::DP_SIM_HV_SWITCHED_HV:
    _computeMomentsDpSimHvSwitchedHv(rmom);
    break;
    
    // Dual pol, H transmission, fixed H and V receivers (CP2 X band)
  case Params::DP_H_ONLY_FIXED_HV:
    _computeMomentsDpHOnlyFixedHv(rmom);
    break;
    
    // Dual pol, V transmission, fixed H and V receivers (CP2 X band)
  case Params::DP_V_ONLY_FIXED_HV:
    _computeMomentsDpVOnlyFixedHv(rmom);
    break;
    
  }
  
}

///////////////////////
// compute the moments

// single pol
// SP
    
void Cov2Mom::_computeMomentsSp(RadarMoments &rmom)

{

  for (int ii = 0; ii < _nGates; ii++) {

    MomentsFields fields;
    
    rmom.computeMomSinglePolH(_lag0_hc[ii],
                              _lag1_hc[ii],
                              _lag2_hc[ii],
                              _lag3_hc[ii],
                              ii, fields);

    _ncp[ii] = fields.ncp;
    _snr[ii] = fields.snr;
    _dbm[ii] = fields.dbm;
    _dbz[ii] = fields.dbz;
    _vel[ii] = fields.vel;
    _width[ii] = fields.width;
    _snrhc[ii] = fields.snrhc;
    _dbmhc[ii] = fields.dbmhc;
    
  }

}

// Dual pol, alternating transmission, copolar receiver only
// (CP2 SBand)
// DP_ALT_HV_CO_ONLY
    
void Cov2Mom::_computeMomentsDpAltHvCoOnly(RadarMoments &rmom)

{

  for (int ii = 0; ii < _nGates; ii++) {

    MomentsFields fields;
    
    rmom.computeMomDpAltHvCoOnly(_lag0_hc[ii],
                                 _lag0_vc[ii],
                                 _lag1_vchc[ii],
                                 _lag1_hcvc[ii],
                                 _lag2_hc[ii],
                                 _lag2_vc[ii],
                                 ii, fields);
    
    _ncp[ii] = fields.ncp;
    _snr[ii] = fields.snr;
    _dbm[ii] = fields.dbm;
    _dbz[ii] = fields.dbz;
    _vel[ii] = fields.vel;
    _width[ii] = fields.width;
    _zdr[ii] = fields.zdr;
    _rhohv[ii] = fields.rhohv;
    _phidp[ii] = fields.phidp;
    _snrhc[ii] = fields.snrhc;
    _snrvc[ii] = fields.snrvc;
    _dbmhc[ii] = fields.dbmhc;
    _dbmvc[ii] = fields.dbmvc;
    
  }

}

// Dual pol, alternating transmission, co-polar and cross-polar
// receivers (SPOL with Mitch Switch and receiver in 
// switching mode, CHILL)
// DP_ALT_HV_CO_CROSS:
    
void Cov2Mom::_computeMomentsDpAltHvCoCross(RadarMoments &rmom)

{

  for (int ii = 0; ii < _nGates; ii++) {

    MomentsFields fields;
    
    rmom.computeMomDpAltHvCoCross(_lag0_hc[ii],
                                  _lag0_hx[ii],
                                  _lag0_vc[ii],
                                  _lag0_vx[ii],
                                  _lag0_vchx[ii],
                                  _lag0_hcvx[ii],
                                  _lag0_vxhx[ii],
                                  _lag1_vchc[ii],
                                  _lag1_hcvc[ii],
                                  _lag2_hc[ii],
                                  _lag2_vc[ii],
                                  ii, fields);

    _ncp[ii] = fields.ncp;
    _snr[ii] = fields.snr;
    _dbm[ii] = fields.dbm;
    _dbz[ii] = fields.dbz;
    _vel[ii] = fields.vel;
    _width[ii] = fields.width;
    _zdr[ii] = fields.zdr;
    _ldrh[ii] = fields.ldrh;
    _ldrv[ii] = fields.ldrv;
    _rhohv[ii] = fields.rhohv;
    _phidp[ii] = fields.phidp;
    _kdp[ii] = fields.kdp;
    _snrhc[ii] = fields.snrhc;
    _snrhx[ii] = fields.snrhx;
    _snrvc[ii] = fields.snrvc;
    _snrvx[ii] = fields.snrvx;
    _dbmhc[ii] = fields.dbmhc;
    _dbmhx[ii] = fields.dbmhx;
    _dbmvc[ii] = fields.dbmvc;
    _dbmvx[ii] = fields.dbmvx;
    
  }

}

// Dual pol, alternating transmission, fixed H and V receivers (SPOL
// with Mitch Switch and receivers in fixed mode)
// DP_ALT_HV_FIXED_HV:
    
void Cov2Mom::_computeMomentsDpAltHvFixedHv(RadarMoments &rmom)

{

  for (int ii = 0; ii < _nGates; ii++) {

    MomentsFields fields;
    
    _ncp[ii] = fields.ncp;
    _snr[ii] = fields.snr;
    _dbm[ii] = fields.dbm;
    _dbz[ii] = fields.dbz;
    _vel[ii] = fields.vel;
    _width[ii] = fields.width;
    _zdr[ii] = fields.zdr;
    _rhohv[ii] = fields.rhohv;
    _phidp[ii] = fields.phidp;
    _snrhc[ii] = fields.snrhc;
    _snrvc[ii] = fields.snrvc;
    _dbmhc[ii] = fields.dbmhc;
    _dbmvc[ii] = fields.dbmvc;
    
  }

}

// Dual pol, simultaneous transmission, fixed H and V receivers (NEXRAD
// upgrade, SPOL with T and receivers in fixed mode)
// DP_SIM_HV_FIXED_HV:
    
void Cov2Mom::_computeMomentsDpSimHvFixedHv(RadarMoments &rmom)

{

  for (int ii = 0; ii < _nGates; ii++) {

    MomentsFields momFlds;
    
  }

}

// Dual pol, simultaneous transmission, switching H and V receivers
// (SPOL with T and receivers in switching mode)
// DP_SIM_HV_SWITCHED_HV:
    
void Cov2Mom::_computeMomentsDpSimHvSwitchedHv(RadarMoments &rmom)

{

  for (int ii = 0; ii < _nGates; ii++) {

    MomentsFields momFlds;
    
  }

}

// Dual pol, H transmission, fixed H and V receivers (CP2 X band)
// DP_H_ONLY_FIXED_HV:
    
void Cov2Mom::_computeMomentsDpHOnlyFixedHv(RadarMoments &rmom)

{

  for (int ii = 0; ii < _nGates; ii++) {

    MomentsFields momFlds;
    
  }

}

// Dual pol, V transmission, fixed H and V receivers (CP2 X band)
// DP_V_ONLY_FIXED_HV:

void Cov2Mom::_computeMomentsDpVOnlyFixedHv(RadarMoments &rmom)

{

  for (int ii = 0; ii < _nGates; ii++) {

    MomentsFields momFlds;
    
  }

}

