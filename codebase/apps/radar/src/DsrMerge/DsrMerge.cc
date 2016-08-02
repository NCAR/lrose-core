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
///////////////////////////////////////////////////////////////
// DsrMerge.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2010
//
///////////////////////////////////////////////////////////////
//
// DsrMerge reads moments data from 2 Dsr file message queues,
// which contain data from 2 channels of the same radar system,
// in which there are differences. For example, there may be
// 2 transmitters operating at different frequencies, each of
// which has a separate moments data set. DsrMerge merges 
// these two data streams, and produces a single combined 
// data stream. In doing so, some fields are copied unchanged
// into the output queue. Other fields may be combined using 
// the mean of the two incoming fields.
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <cmath>
#include <toolsa/uusleep.h>
#include <toolsa/pmu.h>
#include "DsrMerge.hh"
#include "Field.hh"

using namespace std;

// Constructor

DsrMerge::DsrMerge(int argc, char **argv)
  
{

  isOK = true;

  // set programe name
  
  _progName = "DsrMerge";
  
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
    return;
  }

  // compile the list of output fields

  _compileFieldList();

  // init process mapper registration
  
  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }
  
}

// destructor

DsrMerge::~DsrMerge()

{

  // clean up

  _clearFields();

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Outer run method

int DsrMerge::Run ()
{
  
  PMU_auto_register("Run");
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Running DsrMerge - verbose debug mode" << endl;
  } else if (_params.debug) {
    cerr << "Running DsrMerge - debug mode" << endl;
  }

  int iret = 0;

  while (true) {

    PMU_auto_register("Run");

    if (_init()) {
      iret = -1;
      break;
    }

    if (_run()) {
      umsleep(1000);
      iret = -1;
    }
    
  } // while

  return iret;

}

//////////////////////////////////////////////////
// inner run method

int DsrMerge::_run()
{

  // initialize by reading one beam from each queue

  if (_readQueue1()) {
    return -1;
  }
  
  if (_readQueue2()) {
    return -1;
  }
      
  // main loop

  while (true) {
    
    PMU_auto_register("_run");
  
    if (_beamsMatch()) {
      
      // the incoming beams match,
      // so perform the merge and write out

      if (_params.print_active_channel) {
        cerr << "M";
      }
      if (_mergeAndWrite()) {
        return -1;
      }
      
      // read 1 beam from each queue

      if (_readQueue1()) {
        return -1;
      }

      if (_readQueue2()) {
        return -1;
      }
      
    } else if (_queue1IsBehind()) {

      if (!_params.require_both_channels) {
        // channel 2 has missing data and has skipped ahead
        // write data from channel 1 only, before reading next beam
        // from chan 1
        if (_params.print_active_channel) {
          cerr << "1";
        }
        if (_writeForChan1Only()) {
          return -1;
        }
      }

      // queue 1 is behind, so must catch up
      
      if (_readQueue1()) {
        return -1;
      }

    } else {

      if (!_params.require_both_channels) {
        // channel 1 has missing data and has skipped ahead
        // write data from channel 2 only, before reading next beam
        // from chan 2
        if (_params.print_active_channel) {
          cerr << "2";
        }
        if (_writeForChan2Only()) {
          return -1;
        }
      }

      // queue2 is behind, so must catch up
      
      if (_readQueue2()) {
        return -1;
      }
      
    }

  } // while

  return 0;

}

//////////////////////////////////////////////////
// initialize
// returns 0 on success, -1 on failure

int DsrMerge::_init()
{

  _inputContents1 = 0;
  _inputContents2 = 0;

  _paramsFoundSinceLastBeam = false;
  
  // close as needed

  _inputQueue1.closeMsgQueue();
  _inputQueue2.closeMsgQueue();
  _outputQueue.closeMsgQueue();

  // create the input FMQs

  Fmq::openPosition startPos = Fmq::END;
  if (_params.start_reading_at_fmq_start) {
    startPos = Fmq::START;
  }

  if (_inputQueue1.init(_params.input_fmq_name_chan1,
                        _progName.c_str(),
                        _params.debug,
                        DsFmq::BLOCKING_READ_WRITE, startPos)) {
    cerr << "ERROR - DsrMerge::_init()" << endl;
    cerr << "  Cannot not initialize queue 1: "
         << _params.input_fmq_name_chan1 << endl;
    return -1;
  }
  if (_params.debug) {
    cerr << "-->> Opened input queue channel 1: "
         << _params.input_fmq_name_chan1 << endl;
  }
  
  if (_inputQueue2.init(_params.input_fmq_name_chan2,
                        _progName.c_str(),
                        _params.debug,
                        DsFmq::BLOCKING_READ_WRITE, startPos)) {
    cerr << "ERROR - DsrMerge::_init()" << endl;
    cerr << "  Cannot not initialize queue 2: "
         << _params.input_fmq_name_chan2 << endl;
    return -1;
  }
  if (_params.debug) {
    cerr << "-->> Opened input queue channel 2: "
         << _params.input_fmq_name_chan2 << endl;
  }
  
  return 0;

}
  
///////////////////////////////////////
// compile the output fields

void DsrMerge::_compileFieldList()
  
{

  _clearFields();

  for (int ii = 0; ii < _params.output_fields_n; ii++) {

    const Params::output_field_t &fld = _params._output_fields[ii];
    string inputName = fld.input_name;

    if (fld.output_from_queue_1 || fld.output_mean) {
      string outputName = inputName + _params.field_name_suffix_1;
      Field *field = new Field(inputName, outputName,
                               _radarParams, Field::CHAN1);
      if (fld.output_from_queue_1) {
        field->setIsOutput(true);
      }
      _fields.push_back(field);
    }

    if (fld.output_from_queue_2 || fld.output_mean) {
      string outputName = inputName + _params.field_name_suffix_2;
      Field *field = new Field(inputName, outputName,
                               _radarParams, Field::CHAN2);
      if (fld.output_from_queue_2) {
        field->setIsOutput(true);
      }
      _fields.push_back(field);
    }
    
    if (fld.output_mean) {
      string outputName = inputName + _params.field_name_suffix_mean;
      Field *field = new Field(inputName, outputName,
                               _radarParams, Field::MEAN);
      field->setIsOutput(true);
      _fields.push_back(field);
    }

  }

  if (_params.debug) {
    cerr << "Output field list:" << endl;
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      cerr << "  Field number: " << ii << endl;
      _fields[ii]->print(cerr);
    }
  }

}

   
///////////////////////////////////////
// clear the field vector

void DsrMerge::_clearFields()
  
{

  for (size_t ii = 0; ii < _fields.size(); ii++) {
    delete _fields[ii];
  }
  _fields.clear();

}

///////////////////////////////////////////////////
// check if beams match

bool DsrMerge::_beamsMatch()

{

  const DsRadarBeam &beam1 = _inputMsg1.getRadarBeam();
  const DsRadarBeam &beam2 = _inputMsg2.getRadarBeam();

  double time1 = beam1.getDoubleTime();
  double time2 = beam2.getDoubleTime();

  if (fabs(time1 - time2) > _params.max_beam_time_difference) {
    return false;
  }

  double azimuth1 = beam1.azimuth;
  double azimuth2 = beam2.azimuth;

  if (fabs(azimuth1 - azimuth2) > _params.max_beam_azimuth_difference) {
    return false;
  }

  double elevation1 = beam1.elevation;
  double elevation2 = beam2.elevation;

  if (fabs(elevation1 - elevation2) > _params.max_beam_elevation_difference) {
    return false;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-->> beams match" << endl;
    cerr << "---->> beam1 time, az, el: "
         << time1 << ", " << azimuth1 << ", " << elevation1 << endl;
    cerr << "---->> beam2 time, az, el: "
         << time2 << ", " << azimuth2 << ", " << elevation2 << endl;
  }

  return true;

}

///////////////////////////////////////////////////
// check if queue 1 is behind

bool DsrMerge::_queue1IsBehind()

{
  
  const DsRadarBeam &beam1 = _inputMsg1.getRadarBeam();
  const DsRadarBeam &beam2 = _inputMsg2.getRadarBeam();
  
  double time1 = beam1.getDoubleTime();
  double time2 = beam2.getDoubleTime();

  if (time1 < time2) {
    return true;
  } else {
    return false;
  }

}

///////////////////////////////////////////////////
// read from queue 1
// returns 0 on success, -1 on failure

int DsrMerge::_readQueue1()

{

  while (true) {

    PMU_auto_register("Reading input fmq 1");
  
    // get next message

    if (_inputQueue1.getDsMsg(_inputMsg1, &_inputContents1)) {
      return -1;
    }
    
    // success on getting beam?
    
    if ((_inputContents1 & DsRadarMsg::RADAR_BEAM) &&
        _inputMsg1.allParamsSet()) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "-->> got beam from queue 1" << endl;
      }
      return 0;
    }
    
    // save radar params
    
    if (_inputContents1 & DsRadarMsg::RADAR_PARAMS) {
      _radarParams = _inputMsg1.getRadarParams();
      _paramsFoundSinceLastBeam = true;
      if (_params.override_radar_name) {
        _radarParams.radarName = _params.radar_name;
      }
    }

    // set params in field objects
    
    if (_inputContents1 & DsRadarMsg::FIELD_PARAMS) {

      for (size_t ii = 0; ii < _inputMsg1.getFieldParams().size(); ii++) {
        const DsFieldParams &rfld = *_inputMsg1.getFieldParams(ii);
        for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
          Field &field = *_fields[ifield];
          if (field.getInputName() == rfld.name) {
            if (field.getSource() == Field::CHAN1 ||
                field.getSource() == Field::MEAN) {
              field.setParams(rfld);
            }
          }
        } // ifield
      } // ii

      _paramsFoundSinceLastBeam = true;

      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "-->> got field params from queue 1" << endl;
      }

    } // if (_inputContents1 & DsRadarMsg::FIELD_PARAMS)
    
    // copy calibration and flags to output queue

    if (_inputContents1 & DsRadarMsg::RADAR_CALIB) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "-->> got calibration from queue 1" << endl;
      }
      if (_writeCalib(_inputMsg1.getRadarCalib())) {
        return -1;
      }
    }
    
    if (_inputContents1 & DsRadarMsg::RADAR_FLAGS) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "-->> got flags from queue 1" << endl;
      }
      if (_writeFlags(_inputMsg1.getRadarFlags())) {
        return -1;
      }
    }

  } // while
    
  return -1;

}

///////////////////////////////////////////////////
// read from queue 2
// returns 0 on success, -1 on failure

int DsrMerge::_readQueue2()

{

  while (true) {

    PMU_auto_register("Reading input fmq 2");
  
    // get next input message

    if (_inputQueue2.getDsMsg(_inputMsg2, &_inputContents2)) {
      return -1;
    }
    
    // success on getting beam?
    
    if ((_inputContents2 & DsRadarMsg::RADAR_BEAM) &&
        _inputMsg2.allParamsSet()) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "-->> got beam from queue 2" << endl;
      }
      return 0;
    }
    
    // set params in field objects
    
    if (_inputContents2 & DsRadarMsg::FIELD_PARAMS) {

      for (size_t ii = 0; ii < _inputMsg2.getFieldParams().size(); ii++) {
        const DsFieldParams &rfld = *_inputMsg2.getFieldParams(ii);
        for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
          Field &field = *_fields[ifield];
          if (field.getInputName() == rfld.name) {
            if (field.getSource() == Field::CHAN2) {
              field.setParams(rfld);
            }
          }
        } // ifield
      } // ii

      _paramsFoundSinceLastBeam = true;

      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "-->> got field params from queue 2" << endl;
      }

    } // if (_inputContents2 & DsRadarMsg::FIELD_PARAMS)
    
  } // while
    
  return -1;

}

//////////////////////////////////////////////////
// merge information and write out to fmq

int DsrMerge::_mergeAndWrite()
{
  
  PMU_auto_register("_mergeAndWrite");

  // perform the merge, loading up output fields

  if (_performMerge()) {
    return -1;
  }

  // write output beam

  if (_writeBeam()) {
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// write from channel 1 only

int DsrMerge::_writeForChan1Only()
{
  
  PMU_auto_register("_writeForChan1Only");

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-->> writing chan1 only" << endl;
  }

  const DsRadarBeam &beam1 = _inputMsg1.getRadarBeam();

  // loop through fields, loading up data

  const vector<DsFieldParams*> &fp1 = _inputMsg1.getFieldParams();
  
  for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
    
    Field &field = *_fields[ifield];
    
    if (field.getSource() == Field::CHAN1) {
      field.loadUnchanged(_radarParams, fp1, beam1);
    } else if (field.getSource() == Field::MEAN &&
               _params.allow_merge_from_single_channel) {
      field.loadUnchanged(_radarParams, fp1, beam1);
    } else {
      field.loadMissing(_radarParams);
    }

  } // ifield

  // load up output buffer
  
  _loadOutputBuffer(beam1);
  
  // write output beam

  if (_writeBeam()) {
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// write from channel 2 only

int DsrMerge::_writeForChan2Only()
{
  
  PMU_auto_register("_writeForChan2Only");

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-->> writing chan2 only" << endl;
  }

  const DsRadarBeam &beam2 = _inputMsg2.getRadarBeam();

  // loop through fields, loading up data

  const vector<DsFieldParams*> &fp2 = _inputMsg2.getFieldParams();
  
  for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
    
    Field &field = *_fields[ifield];
    
    if (field.getSource() == Field::CHAN2) {
      field.loadUnchanged(_radarParams, fp2, beam2);
    } else if (field.getSource() == Field::MEAN &&
               _params.allow_merge_from_single_channel) {
      field.loadUnchanged(_radarParams, fp2, beam2);
    } else {
      field.loadMissing(_radarParams);
    }

  } // ifield

  // load up output buffer
  
  _loadOutputBuffer(beam2);
  
  // write output beam

  if (_writeBeam()) {
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// perform the merge

int DsrMerge::_performMerge()
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-->> performing merge" << endl;
  }

  const DsRadarBeam &beam1 = _inputMsg1.getRadarBeam();
  const DsRadarBeam &beam2 = _inputMsg2.getRadarBeam();

  // check that the packing is the same in both queues

  if (beam1.byteWidth != beam2.byteWidth) {
    cerr << "ERROR - DsrMerge::_performMerge" << endl;
    cerr << "  Byte width differs between input queues" << endl;
    cerr << "  Byte width queue 1: " << beam1.byteWidth << endl;
    cerr << "  Byte width queue 2: " << beam2.byteWidth << endl;
    cerr << "These must be the same - change output upstream" << endl;
    return -1;
  }
 
  // loop through fields, loading up data

  const vector<DsFieldParams*> &fp1 = _inputMsg1.getFieldParams();
  const vector<DsFieldParams*> &fp2 = _inputMsg2.getFieldParams();
  
  for (size_t ifield = 0; ifield < _fields.size(); ifield++) {

    Field &field = *_fields[ifield];
    
    if (field.getSource() == Field::CHAN1) {
      field.loadUnchanged(_radarParams, fp1, beam1);
    } else if (field.getSource() == Field::CHAN2) {
      field.loadUnchanged(_radarParams, fp2, beam2);
    } else if (field.getSource() == Field::MEAN) {
      field.loadMean(_radarParams, fp1, beam1, fp2, beam2);
    } else {
      field.loadMissing(_radarParams);
    }

  } // ifield

  // load up output buffer
  // using metadata from beam1

  _loadOutputBuffer(beam1);

  return 0;

}

///////////////////////////////////////
// load up output buffer

void DsrMerge::_loadOutputBuffer(const DsRadarBeam &beam)
  
{

  int nBytes = beam.byteWidth * _radarParams.numGates * _fields.size();
  _outBuf.reserve(nBytes);
  memset(_outBuf.getPtr(), 0, nBytes);
  int nFields = _fields.size();
  
  if (beam.byteWidth == 1) {
    
    for (int ifield = 0; ifield < nFields; ifield++) {
      ui08 *outData = (ui08 *) _outBuf.getPtr() + ifield;
      ui08 *inData = (ui08 *) _fields[ifield]->getData();
      for (int igate = 0; igate < _radarParams.numGates; igate++) {
        *outData = *inData;
        outData += nFields;
        inData++;
      } // igate
    } // ifield

  } else if (beam.byteWidth == 2) {

    for (int ifield = 0; ifield < nFields; ifield++) {
      ui16 *outData = (ui16 *) _outBuf.getPtr() + ifield;
      ui16 *inData = (ui16 *) _fields[ifield]->getData();
      for (int igate = 0; igate < _radarParams.numGates; igate++) {
        *outData = *inData;
        outData += nFields;
        inData++;
      } // igate
    } // ifield

  } else if (beam.byteWidth == 4) {

    for (int ifield = 0; ifield < nFields; ifield++) {
      fl32 *outData = (fl32 *) _outBuf.getPtr() + ifield;
      fl32 *inData = (fl32 *) _fields[ifield]->getData();
      for (int igate = 0; igate < _radarParams.numGates; igate++) {
        *outData = *inData;
        outData += nFields;
        inData++;
      } // igate
    } // ifield

  }

}

///////////////////////////////////////
// initialize the output queue

int DsrMerge::_openOutputFmq()
  
{

  if (_outputQueue.isOpen()) {
    // already open
    return 0;
  }

  if (!_inputQueue1.isOpen()) {
    cerr << "ERROR - DsrMerge::_openOutputFmq()" << endl;
    cerr << "  Input FMQ 1 not yet open" << endl;
    return -1;
  }

  // compute output FMQ size from input size

  //   int numSlots = _inputQueue1.getNumSlots();
  //   int bufSize = 2 * _inputQueue1.getBufSize() + _inputQueue2.getBufSize();
  int numSlots = _params.output_fmq_nslots;
  int bufSize = _params.output_fmq_size;

  if (numSlots == 0 || bufSize == 0) {
    cerr << "ERROR - DsrMerge::_openOutputFmq()" << endl;
    cerr << "  Input FMQ 1 not yet initialized" << endl;
    return -1;
  }
  
  if (_outputQueue.initReadWrite(_params.output_fmq_name,
                                 _progName.c_str(),
                                 _params.debug >= Params::DEBUG_EXTRA,
                                 Fmq::END, // start position
                                 false,    // compression
                                 numSlots, bufSize)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot initialize FMQ: "
         << _params.output_fmq_name << endl;
    cerr << "  nSlots: " << numSlots << endl;
    cerr << "  nBytes: " << bufSize << endl;
    cerr << _outputQueue.getErrStr() << endl;
    return -1;
  }

  _outputQueue.setSingleWriter();
  if (_params.write_blocking) {
    _outputQueue.setBlockingWrite();
  }
  if (_params.data_mapper_report_interval > 0) {
    _outputQueue.setRegisterWithDmap
      (true, _params.data_mapper_report_interval);
  }
  _outputMsg.clearAll();
  _outputMsg.setType(0);

  if (_params.debug) {
    cerr << "-->> Opened output queue: " << _params.output_fmq_name << endl;
    cerr << "  nSlots: " << numSlots << endl;
    cerr << "  nBytes: " << bufSize << endl;
  }

  return 0;

}

///////////////////////////////////////
// write beam to output FMQ
// returns 0 on success, -1 on failure

int DsrMerge::_writeBeam()
  
{

  // make sure it's open
  
  if (_openOutputFmq()) {
    return -1;
  }
  
  if (_paramsFoundSinceLastBeam) {
    if (_writeParams()) {
      return -1;
    }
    _paramsFoundSinceLastBeam = false;
  }
  
  const DsRadarBeam &beam1 = _inputMsg1.getRadarBeam();
  DsRadarMsg msg;
  DsRadarBeam &beamOut = msg.getRadarBeam();
  beamOut.copyHeader(beam1);
  beamOut.loadData(_outBuf.getPtr(), _outBuf.getLen(), beam1.byteWidth);
  
  // write the beam message
  
  if (_outputQueue.putDsMsg(msg, DsRadarMsg::RADAR_BEAM)) {
    cerr << "ERROR - DsrMerge::_writeBeam" << endl;
    cerr << "  Cannot put beam to queue" << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "-->> Wrote beam" << endl;
  }

  return 0;

}
    
////////////////////////////////////////////
// write params to output FMQ
// returns 0 on success, -1 on failure

int DsrMerge::_writeParams()
  
{

  // make sure fmq is open
  
  if (_openOutputFmq()) {
    return -1;
  }

  DsRadarMsg msg;
  
  // load field parameters to the message
  
  vector<DsFieldParams*> &fp = msg.getFieldParams();
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    DsFieldParams *fparams = new DsFieldParams(_fields[ii]->getParams());
    fparams->name = _fields[ii]->getOutputName();
    fp.push_back(fparams);
  }

  // load radar params into message
  
  DsRadarParams &outRadarParams = msg.getRadarParams();
  outRadarParams = _radarParams;
  outRadarParams.numFields = _fields.size();

  // write the parameter
  
  if (_outputQueue.putDsMsg
      (msg, DsRadarMsg::RADAR_PARAMS | DsRadarMsg::FIELD_PARAMS)) {
    cerr << "ERROR - DsrMerge::_writeParams" << endl;
    cerr << "  Cannot put params to queue" << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-->> Wrote params" << endl;
  }

  return 0;

}

////////////////////////////////////////////
// write calibration to output FMQ 
// returns 0 on success, -1 on failure

int DsrMerge::_writeCalib(const DsRadarCalib &calib)
  
{

  // make sure fmq is open
  
  if (_openOutputFmq()) {
    return -1;
  }

  // load calibration into message

  DsRadarMsg msg;
  DsRadarCalib &outCalib = msg.getRadarCalib();
  outCalib = calib;

  // write the calibration message
  
  if (_outputQueue.putDsMsg(msg, DsRadarMsg::RADAR_CALIB)) {
    cerr << "ERROR - DsrMerge::_writeCalib" << endl;
    cerr << "  Cannot put calib to queue" << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-->> Wrote calib" << endl;
  }

  return 0;

}

////////////////////////////////////////////
// write flags to output FMQ
// returns 0 on success, -1 on failure

int DsrMerge::_writeFlags(const DsRadarFlags &flags)
  
{

  // make sure fmq is open
  
  if (_openOutputFmq()) {
    return -1;
  }

  // load flags into message

  DsRadarMsg msg;
  DsRadarFlags &outFlags = msg.getRadarFlags();
  outFlags = flags;

  // write the flags message
  
  if (_outputQueue.putDsMsg(msg, DsRadarMsg::RADAR_FLAGS)) {
    cerr << "ERROR - DsrMerge::_writeFlags" << endl;
    cerr << "  Cannot put flags to queue" << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-->> Wrote flags" << endl;
  }

  return 0;

}

