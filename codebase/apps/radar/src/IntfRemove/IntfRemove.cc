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
// IntfRemove.cc
//
// IntfRemove object
// Some methods are in transform.cc and geom.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2008
//
///////////////////////////////////////////////////////////////////////
//
// IntfRemove reads DBZ and SNR data in an input DsRadar FMQ,
// identifies interference and removes the interference power
// from the power fields, and writes the cleaned up data out to
// a DsRadar queue
//
///////////////////////////////////////////////////////////////////////

#include <toolsa/pmu.h>
#include <toolsa/utim.h>
#include <toolsa/uusleep.h>
#include <toolsa/ucopyright.h>
#include <toolsa/MemBuf.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/toolsa_macros.h>
#include <radar/FilterUtils.hh>
#include <iomanip>
#include "IntfRemove.hh"

const double IntfRemove::_missingDouble = -9999.0;
const double IntfRemove::_missingTest = -9998.0;

using namespace std;

// Constructor

IntfRemove::IntfRemove(int argc, char **argv)

{
  
  isOK = true;
  _inputQueue = NULL;
  _inputContents = 0;
  _inputNFail = 0;
  _outputQueue = NULL;
  _nFieldsIn = 0;
  _nGates = 0;
  _nGatesAlloc = 0;
  _debugPrintNeedsNewline = false;
  _interferenceSnrDb = 0;
  _backgroundSnrDb = 0;
  _backgroundSnrSum = 0;
  _backgroundSnrCount = 0;

  _snrIndex = -1;
  _dbzIndex = -1;

  // set programe name
  
  _progName = "IntfRemove";
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

  
}

/////////////////////////////////////////////////////////
// destructor

IntfRemove::~IntfRemove()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int IntfRemove::Run ()
{
  
  // register with procmap
  
  PMU_auto_register("Run");
  
  while (true) {
    if (_params.debug) {
      cerr << "IntfRemove::Run:" << endl;
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

int IntfRemove::_run ()
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

      cerr << "ERROR - IntfRemove" << endl;
      cerr << "  inputQueue:getDsBeam() failed, retval: " << iret << endl;
      
      // Keep count of consecuive failures.
      _inputNFail++;

      // If we have maxed out, it is safe to assume that the program is
      // constantly failing. Exiting and restarting may solve this,
      // assuming the restarter is running.
      if (_inputNFail > 100) {
	cerr << "The program is failing consistently, reopen the queues ..."
	     << endl;
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

int IntfRemove::_openInputQueue()

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
    cerr << "ERROR - IntfRemove" << endl;
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

int IntfRemove::_openOutputQueue()

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

  return 0;

}

////////////////////////////////////////////////////////////////
// process the input message
// Returns 0 on success, -1 on failure

int IntfRemove::_processInputMessage()

{

  // copy flags from input queue to output queue
  
  if (_inputContents & DsRadarMsg::RADAR_FLAGS) {
    _copyFlags();
  }

  if (_inputContents & DsRadarMsg::RADAR_PARAMS) {

    // save input radar params

    _inputRadarParams =  _inputMsg.getRadarParams();
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "\n========= Radar params =========" << endl;
      _inputRadarParams.print(cerr);
    }
    _nFieldsIn = _inputRadarParams.numFields;
    _nGates = _inputRadarParams.numGates;

    // there are 2 more output fields than input fields
    // corrected DBZ and SNR

    _nFieldsOut = _nFieldsIn + 2;

    // copy radar params to output queue

    if (_writeRadarParams()) {
      return -1;
    }

  }

  if (_inputContents & DsRadarMsg::FIELD_PARAMS) {

    // save input params
    
    _inputFieldParams =  _inputMsg.getFieldParams();
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "\n========= Field params =========" << endl;
      for (int ii = 0; ii < _nFieldsIn; ii++) {
	_inputFieldParams[ii]->print(cerr);
      }
    }

    if (_getFieldIndices()) {
      return -1;
    }

    // write field params to output queue, including extra
    // 2 fields, corrected DBZ and SNR
    
    if (_writeFieldParams()) {
      return -1;
    }

  }

  if (_inputContents & DsRadarMsg::RADAR_CALIB) {

    // save radar calib

    _inputRadarCalib =  _inputMsg.getRadarCalib();
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "\n========= Radar calib =========" << endl;
      _inputRadarCalib.print(cerr);
    }

    // copy calib to output queue

    if (_writeRadarCalib()) {
      return -1;
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

void IntfRemove::_copyFlags()

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

int IntfRemove::_processBeam()

{

  // compute the indices of each input field in the Dsr beam
  // if field is missing, these are set to -1

  if (_getFieldIndices()) {
    return -1;
  }

  // allocate the data arrays

  _allocateArrays(_nGates);

  // load up input field arrays

  const DsRadarBeam &beam = _inputMsg.getRadarBeam();
  _dbzFieldParams = *_inputFieldParams[_dbzIndex];
  _loadInputField(beam, _dbzIndex, _dbzFieldParams, _dbz); 

  if (_snrIndex < 0) {
    _computeSnrFromDbz();
  } else {
    _snrFieldParams = *_inputFieldParams[_snrIndex];
    _loadInputField(beam, _snrIndex, _snrFieldParams, _snr);
  }

  // apply median filter to the SNR field

  memcpy(_snrMedian, _snr, _nGates * sizeof(double));
  FilterUtils::applyMedianFilter(_snrMedian, _nGates, 5);

  // remove interference if required

  int nSegments = _params.number_of_range_segments;
  int nGatesPerSeg = _nGates / nSegments;
  int startGate = 0;

  for (int iseg = 0; iseg < nSegments; iseg++) {
    
    int endGate = startGate + nGatesPerSeg - 1;

    if (iseg == nSegments - 1) {
      endGate = _nGates - 1;
    }

    if (_interferencePresent(startGate, endGate)) {
      _removeInterference(startGate, endGate);
    }

    startGate = endGate + 1;

  }

  // apply median filter as appropriate
  
  if (_params.apply_median_filter_to_output_DBZ) {
    FilterUtils::applyMedianFilter(_dbz, _nGates, _params.DBZ_median_filter_len);
    _applyNexradSpikeFilter();
  }

  // apply NEXRAD spike filter

  if (_params.apply_nexrad_spike_filter) {
    _applyNexradSpikeFilter();
  }

  // write beam with precip rates
  
  if (_writeBeam()) {
    return -1;
  }
  
  return 0;

}

////////////////////////////////////////////////////////////////
// get field indices
// Returns -1 on failure

int IntfRemove::_getFieldIndices()

{

  // compute the indices of each input field in the Dsr beam
  // if field is missing, these are set to -1
  
  _dbzIndex = _getInputFieldIndex(_params.dsr_name_DBZ);
  if (_dbzIndex < 0) {
    cerr << "ERROR - IntfRemove::_getFieldIndices" << endl;
    cerr << "  DBZ field missing, dsr name: " << _params.dsr_name_DBZ << endl;
    cerr << "  Cannot process this beam" << endl;
    return -1;
  }

  if (_params.SNR_available) {
    _snrIndex = _getInputFieldIndex(_params.dsr_name_SNR);
  } else {
    _snrIndex = -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////////
// get index for a specified field name
// Returns -1 on failure

int IntfRemove::_getInputFieldIndex(const string &dsrName)

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

void IntfRemove::_loadInputField(const DsRadarBeam &beam,
				 int index,
				 const DsFieldParams &fieldParams,
				 double *fldData)

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
    fl32 missing = fieldParams.missingDataValue;
    for (int ii = 0; ii < _nGates; ii++, fval += _nFieldsIn) {
      if (*fval == missing) {
	fldData[ii] = _missingDouble;
      } else {
	fldData[ii] = *fval;
      }
    }

  } else if (beam.byteWidth == 2) {

    double scale = fieldParams.scale;
    double bias = fieldParams.bias;
    ui16 *sval = (ui16 *) beam.getData() + index;
    ui16 missing = (ui16) fieldParams.missingDataValue;
    for (int ii = 0; ii < _nGates; ii++, sval += _nFieldsIn) {
      if (*sval == missing) {
	fldData[ii] = _missingDouble;
      } else {
	fldData[ii] = *sval * scale + bias;
      }
    }

  } else {

    double scale = fieldParams.scale;
    double bias = fieldParams.bias;
    ui08 *bval = (ui08 *) beam.getData() + index;
    ui08 missing = (ui08) fieldParams.missingDataValue;
    for (int ii = 0; ii < _nGates; ii++, bval += _nFieldsIn) {
      if (*bval == missing) {
	fldData[ii] = _missingDouble;
      } else {
	fldData[ii] = *bval * scale + bias;
      }
    }

  }

}

///////////////////////
// compute SNR from DBZ

void IntfRemove::_computeSnrFromDbz()

{

  double startRange = _inputRadarParams.startRange;
  double gateSpacing = _inputRadarParams.gateSpacing;

  double range = startRange;
  double dbz100 = _params.noise_dbz_at_100km;
  double log100 = log10(100);

  for (int ii = 0; ii < _nGates; ii++, range += gateSpacing) {
    double rangeCorrection = 20.0 * (log100 - log10(range));
    if (_dbz[ii] == _missingDouble) {
      _snr[ii] = _missingDouble;
    } else {
      _snr[ii] = _dbz[ii] - dbz100 + rangeCorrection;
    }
  }

}

////////////////////////////////////
// check if interference is present

bool IntfRemove::_interferencePresent(int startGate, int endGate)

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << endl;
    cerr << "==>> Computing histogram for beam segment" << endl;
    cerr << "     startGate, endGate: " << startGate << ", " << endGate << endl;
    cerr << endl;
    _inputMsg.getRadarBeam().print(cerr);
  }
    
  // compute histogram of SNR data

  double histMin = _params.snr_hist_min;
  double histMax = _params.snr_hist_max;
  double histDelta = _params.snr_hist_delta;
  int nHist = (int) ((histMax - histMin) / histDelta) + 1;

  TaArray<double> hist_, density_, val_;
  double *hist = hist_.alloc(nHist);
  double *density = density_.alloc(nHist);
  double *val = val_.alloc(nHist);
  for(int ii = 0; ii < nHist; ii++) {
    hist[ii] = 0.0;
    density[ii] = 0.0;
    val[ii] = histMin + (ii + 0.5) * histDelta;
  }
  double count = 0.0;

  for (int igate = startGate; igate <= endGate; igate++) {
    double snr = _snrMedian[igate];
    if (snr == _missingDouble) {
      continue;
    }
    int index = (int) ((snr - histMin) / histDelta);
    if (index >= 0 && index < nHist) {
      hist[index]++;
      count++;
    }
  }

  // compute density distribution

  for(int ii = 0; ii < nHist; ii++) {
    density[ii] = (hist[ii] / count) / histDelta;
  }
  
  // find mode of density distribution

  double mode = 0;
  int modeIndex = 0;

  for(int ii = 0; ii < nHist; ii++) {
    if (density[ii] > mode) {
      mode = density[ii];
      modeIndex = ii;
    }
  }

  if (_params.print_histogram) {
    cerr << endl;
    cerr << "============ COMPLETE SNR HISTOGRAM =================" << endl;
    for(int ii = 0; ii < nHist; ii++) {
      cerr << "bin, snr, density: " << setw(5) << ii
	   << setw(10) << val[ii]
	   << setw(20) << density[ii] << endl;
    }
  }

  // find tails of distribution around the mode

  int lowerIndex = modeIndex;
  for(int ii = modeIndex + 1; ii >= 0; ii--) {
    if (density[ii] < _params.tail_threshold) {
      break;
    }
    lowerIndex = ii;
  }
  double lowerLimit = val[lowerIndex];

  int upperIndex = modeIndex;
  for(int ii = modeIndex + 1; ii < nHist; ii++) {
    if (density[ii] < _params.tail_threshold) {
      break;
    }
    upperIndex = ii;
  }
  double upperLimit = val[upperIndex];

  // compute mean SNR around mode

  double sum = 0.0;
  count = 0.0;

  for (int igate = startGate; igate <= endGate; igate++) {
    double snr = _snrMedian[igate];
    if (snr == _missingDouble) {
      continue;
    }
    if (snr >= lowerLimit && snr <= upperLimit) {
      sum += snr;
      count++;
    }
  }

  double modeSnr = 0.0;
  if (count > 0) {
    modeSnr = sum / count;
  }

  // compute background snr

  if (modeSnr < _params.interference_snr_threshold) {
    _backgroundSnrSum += modeSnr;
    _backgroundSnrCount++;
    _backgroundSnrDb = _backgroundSnrSum / _backgroundSnrCount;
    if (_backgroundSnrCount > 499.5) {
      _backgroundSnrSum /= 2.0;
      _backgroundSnrCount /= 2.0;
    }
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "=====>>> background snr: " << _backgroundSnrDb << endl;
    }
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << endl;
    cerr << "=== HISTOGRAM REGION FOR COMPUTING MEAN INTF SNR ====" << endl;
    for(int ii = lowerIndex; ii <= upperIndex; ii++) {
      cerr << "bin, snr, density: " << setw(5) << ii
	   << setw(10) << val[ii]
	   << setw(20) << density[ii] << endl;
    }
  }

  if (modeSnr > _params.interference_snr_threshold) {
    _interferenceSnrDb = modeSnr;
    if (_params.debug) {
      const DsRadarBeam &beam = _inputMsg.getRadarBeam();
      cerr << "Found intf, el, az, startGate, endGate, snr: "
           << beam.elevation << ", "
           << beam.azimuth << ", "
           << startGate << ", " << endGate << ", "
           << _interferenceSnrDb << endl;
    }
    return true;
  } else {
    _interferenceSnrDb = 0.0;
    return false;
  }
  
}

/////////////////////////////////
// remove interference if needed

void IntfRemove::_removeInterference(int startGate, int endGate)

{

  for (int igate = startGate; igate <= endGate; igate++) {

    if (_snr[igate] == _missingDouble || _dbz[igate] == _missingDouble) {
      continue;
    }

    // compute snr in linear space

    double snrDb = _snr[igate];
    double snr = pow(10.0, snrDb / 10.0);

    double intfMarginDb = _params.interference_margin;
    double interferenceSnr = pow(10.0, (_interferenceSnrDb + intfMarginDb) / 10.0);
    double backgroundSnr = pow(10.0, _backgroundSnrDb / 10.0);

    // compute target snr in linear space

    double targetSnr = snr - interferenceSnr + backgroundSnr;
    double targetSnrDb = -10.0;
    if (targetSnr > 0) {
      targetSnrDb = 10.0 * log10(targetSnr);
    }

    // compute the difference in db space

    double diffDb = snrDb - targetSnrDb;

    // apply the same correction to snr and dbz

    _snr[igate] -= diffDb;
    _dbz[igate] -= diffDb;

  }

}
  
///////////////////////
// print input data

void IntfRemove::_printInputData(ostream &out)

{

  out << endl;
  out << "=========== Input data =============" << endl;

  const DsRadarBeam &inBeam = _inputMsg.getRadarBeam();
  inBeam.print(out);

  // set precision

  int orig_precision = out.precision();
  out << setprecision(4);

  _dbzFieldParams.print(out);
  out << "input DBZ data:" << endl;
  for (int igate = 0; igate < _nGates; igate++) {
    out << _dbz[igate] << " ";
  }
  out << endl << endl;

  if (_snrIndex >= 0) {
    _snrFieldParams.print(out);
  }
  out << "input SNR data:" << endl;
  for (int igate = 0; igate < _nGates; igate++) {
    out << _snr[igate] << " ";
  }
  out << endl << endl;

  // restore precision
  out << setprecision(orig_precision);

}

//////////////////////////////////////
// copy radar params to output queue

int IntfRemove::_writeRadarParams()

{

  DsRadarParams outputParams = _inputRadarParams;

  // there are 2 more output fields than input fields
  // corrected DBZ and SNR

  outputParams.numFields = _nFieldsOut;
  _outputMsg.setRadarParams(outputParams);
  
  if (_outputQueue->putDsMsg(_outputMsg, DsRadarMsg::RADAR_PARAMS)) {
    cerr << "ERROR - OutputFmq::writeRadarParams" << endl;
    cerr << "  Cannot put radar params to queue" << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _printDebugMsg("Wrote radar params");
  }

  return 0;

}

///////////////////////////////////////////////
// copy field params to output queue
// plus 2 derived fields: corrected DBZ and SNR

int IntfRemove::_writeFieldParams()

{

  // clear field params

  _outputMsg.clearFieldParams();
  
  // add in input field params

  for (int ii = 0; ii < (int) _inputFieldParams.size(); ii++) {
    DsFieldParams fParams = *_inputFieldParams[ii];
    if (ii == _snrIndex) {
      fParams.name = "SNR_IN";
    } else if (ii == _dbzIndex) {
      fParams.name = "DBZ_IN";
    }
    _outputMsg.addFieldParams(fParams);
  }

  // add in dbz and snr field params

  DsFieldParams dbzParams = *_inputFieldParams[_dbzIndex];
  _outputMsg.addFieldParams(dbzParams);

  if (_snrIndex >= 0) {

    DsFieldParams snrParams = *_inputFieldParams[_snrIndex];
    _outputMsg.addFieldParams(snrParams);

  } else {

    DsFieldParams snrParams = dbzParams;
    snrParams.name = "SNR";
    snrParams.units = "dB";
    _outputMsg.addFieldParams(snrParams);
    
  }

  if (_outputQueue->putDsMsg(_outputMsg, DsRadarMsg::FIELD_PARAMS)) {
    cerr << "ERROR - OutputFmq::writeFieldParams" << endl;
    cerr << "  Cannot put field params to queue" << endl;
    return -1;
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _printDebugMsg("Wrote field params");
  }
  
  return 0;

}

///////////////////////////////////
// copy radar calib to output queue

int IntfRemove::_writeRadarCalib()

{

 _outputMsg.setRadarCalib(_inputRadarCalib);
  if (_outputQueue->putDsMsg(_outputMsg, DsRadarMsg::RADAR_CALIB)) {
    cerr << "ERROR - OutputFmq::writeRadarCalib" << endl;
    cerr << "  Cannot put calib to queue" << endl;
    return -1;
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _printDebugMsg("Wrote radar calib");
  }

  return 0;

}

///////////////////////
// write out beam

int IntfRemove::_writeBeam()

{

  // input beam

  const DsRadarBeam &inBeam = _inputMsg.getRadarBeam();
  ui08 *inData = inBeam.data();

  // output beam

  DsRadarBeam &outBeam = _outputMsg.getRadarBeam();
  outBeam.copyHeader(inBeam);

  // set output byte width - we are using 16-bit integers
  
  int byteWidth = outBeam.byteWidth;
  
  // create output data array
  
  int nData = _nFieldsOut * _nGates;
  int nBytes = nData * byteWidth;
  TaArray<ui08> outData_;
  ui08 *outData = (ui08 *) outData_.alloc(nBytes);
  memset(outData, 0, nBytes);
  
  // copy over input data

  int nBytesGate = _nFieldsIn * byteWidth;
  for (int igate = 0; igate < _nGates; igate++) {
    int inIndex = (igate * _nFieldsIn) * byteWidth;
    int outIndex = (igate * _nFieldsOut) * byteWidth;
    memcpy(outData + outIndex, inData + inIndex, nBytesGate);
  }

  // load up dbz field

  DsFieldParams dbzParams = *_inputFieldParams[_dbzIndex];
  _loadOutputField(dbzParams.scale, dbzParams.bias,
 		   dbzParams.missingDataValue, byteWidth,
 		   _dbz, _nFieldsIn, outData);

  // load up snr field

  if (_snrIndex >= 0) {

    DsFieldParams snrParams = *_inputFieldParams[_snrIndex];
    _loadOutputField(snrParams.scale, snrParams.bias,
		     snrParams.missingDataValue, byteWidth,
		     _snr, _nFieldsIn + 1, outData);
    
  } else {

    // use dbz params
    
    DsFieldParams dbzParams = *_inputFieldParams[_dbzIndex];
    _loadOutputField(dbzParams.scale, dbzParams.bias,
 		     dbzParams.missingDataValue, byteWidth,
 		     _snr, _nFieldsIn + 1, outData);
    
  }

  // load output data into beam

  outBeam.loadData(outData, nBytes, byteWidth);

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
    _debugPrintNeedsNewline = true;
  }

  return 0;

}

/////////////////////////////////////////////
// load output field data into output array

void IntfRemove::_loadOutputField(float scale, float bias,
				  int missingDataValue, int byteWidth,
				  double *fld,
				  int outPos, ui08 *outData)

{

  if (byteWidth == 4) {

    // fl32
    
    fl32 *flData = (fl32 *) outData;
    fl32 outVal;
    int index = outPos;
    for (int igate = 0; igate < _nGates; igate++, index += _nFieldsOut) {
      double val = fld[igate];
      if (val == _missingDouble) {
	outVal = (fl32) missingDataValue;
      } else {
	outVal = (fl32) fld[igate];
      }
      memcpy(flData + index, &outVal, sizeof(fl32));
    }

  } else if (byteWidth == 2) {

    // ui16

    ui16 *flData = (ui16 *) outData;
    int index = outPos;
    ui16 outVal;
    for (int igate = 0; igate < _nGates; igate++, index += _nFieldsOut) {
      double val = fld[igate];
      if (val == _missingDouble) {
	outVal = (ui16) missingDataValue;
      } else {
	int iVal = (int) ((fld[igate] - bias) / scale + 0.5);
	if (iVal < 0) {
	  iVal = 0;
	} else if (iVal > 65535) {
	  iVal = 65535;
	}
	outVal = (ui16) iVal;
      }
      memcpy(flData + index, &outVal, sizeof(ui16));
    }

  } else {

    // ui08

    ui08 *flData = (ui08 *) outData;
    int index = outPos;
    ui08 outVal;
    for (int igate = 0; igate < _nGates; igate++, index += _nFieldsOut) {
      double val = fld[igate];
      if (val == _missingDouble) {
	outVal = (ui08) missingDataValue;
      } else {
	int iVal = (int) ((fld[igate] - bias) / scale + 0.5);
	if (iVal < 0) {
	  iVal = 0;
	} else if (iVal > 65535) {
	  iVal = 65535;
	}
	outVal = (ui08) iVal;
      }
      memcpy(flData + index, &outVal, sizeof(ui08));
    }

  }

}

////////////////////////////////////////////////////////////////////////
// print a debug message, taking account of whether a newline is needed

void IntfRemove::_printDebugMsg(const string &msg, bool addNewline /* = true */)

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

////////////////////////////////////////////////////////////////////////
// allocate data arrays

void IntfRemove::_allocateArrays(int nGates)

{

  if (nGates == _nGatesAlloc) {
    return;
  }

  _snr_.free();
  _snr = (double *) _snr_.alloc(nGates);
  
  _snrMedian_.free();
  _snrMedian = (double *) _snrMedian_.alloc(nGates);
  
  _dbz_.free();
  _dbz = (double *) _dbz_.alloc(nGates);
  
  _nGatesAlloc = nGates;

}

////////////////////////////////////////////////////////////////////////
// Filter fields for spikes in dbz
//
// This routine filters the reflectivity data according to the
// NEXRAD specification DV1208621F, section 3.2.1.2.2, page 3-15.
//
// The algorithm is stated as follows:
//
// Clutter detection:
//
// The nth bin is declared to be a point clutter cell if its power value
// exceeds those of both its second nearest neighbors by a threshold
// value TCN. In other words:
//
//    if   P(n) exceeds TCN * P(n-2)
//    and  P(n) exceeds TCN * p(n+2)
//
//  where
//
//   TCN is the point clutter threshold factor, which is always
//       greater than 1, and typically has a value of 8 (9 dB)
//
//   P(n) if the poiwer sum value for the nth range cell
//
//   n is the range gate number
//
// Clutter censoring:
//
// The formulas for censoring detected strong point clutter in an
// arbitrary array A via data substitution are as follows. If the nth
// range cell is an isolated clutter cell (i.e., it si a clutter cell but
// neither of its immediate neighboring cells is a clutter cell) then the 
// replacement scheme is as follows:
//
//   Replace A(n-1) with  A(n-2)
//   Replace A(n)   with  0.5 * A(n-2) * A(n+2)
//   Replace A(n+1) with  A(n+2)
//
// If the nth and (n+1)th range bins constitute an isolated clutter pair,
// the bin replacement scheme is as follows:
//
//   Replace A(n-1) with  A(n-2)
//   Replace A(n)   with  A(n+2)
//   Replace A(n+1) with  A(n+3)
//   Replace A(n+2) with  A(n+3)
//
// Note that runs of more than 2 successive clutter cells cannot occur
// because of the nature of the algorithm.
 
void IntfRemove::_applyNexradSpikeFilter()
  
{
  
  // set clutter threshold

  double tcn = 9.0;

  // loop through gates

  for (int ii = 2; ii < _nGates - 3; ii++) {

    double &dbzMinus2 = _dbz[ii - 2];
    double &dbzMinus1 = _dbz[ii - 1];
    double &dbz = _dbz[ii];
    double &dbzPlus1 = _dbz[ii + 1];
    double &dbzPlus2 = _dbz[ii + 2];
    double &dbzPlus3 = _dbz[ii + 3];

    double &snrMinus2 = _snr[ii - 2];
    double &snrMinus1 = _snr[ii - 1];
    double &snr = _snr[ii];
    double &snrPlus1 = _snr[ii + 1];
    double &snrPlus2 = _snr[ii + 2];
    double &snrPlus3 = _snr[ii + 3];

    // check for clutter at ii and ii + 1

    bool this_gate = false, next_gate = false;
    
    if ((dbz - dbzMinus2) > tcn &&
	(dbz - dbzPlus2) > tcn) {
      this_gate = true;
    }
    if ((dbzPlus1 - dbzMinus1) > tcn &&
	(dbzPlus1 - dbzPlus3) > tcn) {
      next_gate = true;
    }

    if (this_gate) {

      if (!next_gate) {

	// only gate ii has clutter, substitute accordingly
	
	dbzMinus1 = dbzMinus2;
	dbzPlus1 = dbzPlus2;
	if (dbzMinus2 == _missingDouble ||
            dbzPlus2 == _missingDouble) {
	  dbz = _missingDouble;
	  snr = _missingDouble;
	} else {
	  dbz = dbzMinus2;
	  snr = snrMinus2;
	}
	
      } else {

	// both gate ii and ii+1 has clutter, substitute accordingly

	dbzMinus1 = dbzMinus2;
	dbz = dbzMinus2;
	dbzPlus1 = dbzPlus3;
	dbzPlus2 = dbzPlus3;

	snrMinus1 = snrMinus2;
	snr = snrMinus2;
	snrPlus1 = snrPlus3;
	snrPlus2 = snrPlus3;

      }

    }
    
  } // ii

}

