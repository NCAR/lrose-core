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
// DsrDataMgr.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2022
//
///////////////////////////////////////////////////////////////
//
// Data manager for Dsr moments data.
//
////////////////////////////////////////////////////////////////

#include "DsrDataMgr.hh"

#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/uusleep.h>

using namespace std;

const double DsrDataMgr::_missingDouble = -9999.0;

// Constructor

DsrDataMgr::DsrDataMgr(const string &prog_name,
                       const Args &args,
		       const Params &params) :
        StatsMgr(prog_name, args, params)
  
{
  
  // _totalBeamCount = 0;

  // set up params indices

  // _setMomentsParamsIndex(Params::SNRHC, _snrhc);
  // _setMomentsParamsIndex(Params::SNRHX, _snrhx);
  // _setMomentsParamsIndex(Params::SNRVC, _snrvc);
  // _setMomentsParamsIndex(Params::SNRVX, _snrvx);
  _setMomentsParamsIndex(Params::DBMHC, _dbmhc);
  _setMomentsParamsIndex(Params::DBMHX, _dbmhx);
  _setMomentsParamsIndex(Params::DBMVC, _dbmvc);
  _setMomentsParamsIndex(Params::DBMVX, _dbmvx);
  // _setMomentsParamsIndex(Params::DBZ, _dbz);
  // _setMomentsParamsIndex(Params::VEL, _vel);
  // _setMomentsParamsIndex(Params::WIDTH, _width);
  // _setMomentsParamsIndex(Params::PHIDP, _phidp);
  // _setMomentsParamsIndex(Params::RHOHV, _rhohv);

}

// destructor

DsrDataMgr::~DsrDataMgr()

{


}

//////////////////////////////////////////////////
// Run

int DsrDataMgr::run ()
{

  // open the input queue

  if (_openInputQueue()) {
    return -1;
  }

  while (true) { 
    
    PMU_auto_register("Reading moments queue");
    
    // get a message from the radar queue
    
    int iret = _inputQueue->getDsMsg(_inputMsg, &_inputContents);
    
    if (iret) {
      
      cerr << "ERROR - NoiseMon::DsrDataMgr::run" << endl;
      cerr << "  inputQueue:getDsBeam() failed, retval: " << iret << endl;
      
      // Keep count of consecutive failures.
      _inputNFail++;

      // If we have maxed out, it is safe to assume that the program is
      // constantly failing. Exiting and restarting may solve this,
      // assuming the restarter is running.
      if (_inputNFail > 10000) {
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

int DsrDataMgr::_openInputQueue()

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
  if (_inputQueue->init(_params.input_fmq_name, _progName.c_str(),
			_params.debug,
			DsFmq::READ_ONLY, open_pos)) {
    cerr << "ERROR - NoiseMon::DsrDataMgr" << endl;
    cerr << "  Could not initialize input moments queue: "
	 << _params.input_fmq_name << endl;
    delete _inputQueue;
    _inputQueue = NULL;
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////////
// process the input message
// Returns 0 on success, -1 on failure

int DsrDataMgr::_processInputMessage()

{

  // flags
  
  if (_inputContents & DsRadarMsg::RADAR_FLAGS) {
  }

  // modify radar params for number of fields, write to output queue

  if (_inputContents & DsRadarMsg::RADAR_PARAMS) {
    _inputRadarParams =  _inputMsg.getRadarParams();
    _nFieldsIn = _inputRadarParams.numFields;
    _nGates = _inputRadarParams.numGates;
    DsRadarParams outputParams = _inputRadarParams;
    _startRangeKm = _inputRadarParams.startRange;
    _gateSpacingKm = _inputRadarParams.gateSpacing;
  }

  // write fields params to output queue

  if (_inputContents & DsRadarMsg::FIELD_PARAMS) {
    _inputFieldParams =  _inputMsg.getFieldParams();
  }

  // write radar calib to output queue

  if (_inputContents & DsRadarMsg::RADAR_CALIB) {
    _inputRadarCalib =  _inputMsg.getRadarCalib();
  }

  // process a beam
  
  if (_inputContents & DsRadarMsg::RADAR_BEAM) {
    _processBeam();
  }

  return 0;

}
      
/////////////////////
// process a beam

void DsrDataMgr::_processBeam()

{
  
  // load up the moments data for each field
  
  _loadMomentsData(_dbmhc);
  _loadMomentsData(_dbmhx);
  _loadMomentsData(_dbmvc);
  _loadMomentsData(_dbmvx);

  // get the metadata
  
  const DsRadarBeam &beam = _inputMsg.getRadarBeam();
  // double beamFTime = beam.dataTime + beam.nanoSecs / 1.0e9;
  RadxTime beamTime(beam.dataTime, beam.nanoSecs / 1.0e9);
  
  // at start, print headers
  
  // if (_totalBeamCount == 0) {
  //   setStartTime(beamFTime);
  // }
  // setEndTime(beamFTime);
  // _totalBeamCount++;
  
  // setPrt(1.0 / _inputRadarParams.pulseRepFreq);
  // setEl(beam.elevation);
  // setAz(beam.azimuth);

  // create the fields

  Radx::fl32 missingFl32 = MomentData::missingVal;
  
  RadxField *dbmhcField = new RadxField(_fieldNameMap[Params::DBMHC], "dBm");
  dbmhcField->setRangeGeom(_startRangeKm, _gateSpacingKm);
  dbmhcField->setMissingFl32(missingFl32);

  RadxField *dbmvcField = new RadxField(_fieldNameMap[Params::DBMVC], "dBm");
  dbmvcField->setRangeGeom(_startRangeKm, _gateSpacingKm);
  dbmvcField->setMissingFl32(missingFl32);

  RadxField *dbmhxField = new RadxField(_fieldNameMap[Params::DBMHX], "dBm");
  dbmhxField->setRangeGeom(_startRangeKm, _gateSpacingKm);
  dbmhxField->setMissingFl32(missingFl32);

  RadxField *dbmvxField = new RadxField(_fieldNameMap[Params::DBMVX], "dBm");
  dbmvxField->setRangeGeom(_startRangeKm, _gateSpacingKm);
  dbmvxField->setMissingFl32(missingFl32);

  // fill with data
  
  vector<Radx::fl32> dbmhcData, dbmvcData, dbmhxData, dbmvxData;
  
  for (int igate = 0; igate < _nGates; igate++) {

    // GateData *gate = _gateData[igate];
    // const MomentsFields &flds = gate->fields;

    double dbmhc = _dbmhc.data[igate];
    if (dbmhc == _missingDouble) {
      dbmhcData.push_back(missingFl32);
    } else {
      dbmhcData.push_back(dbmhc);
    }
    
    double dbmvc = _dbmvc.data[igate];
    if (dbmvc == _missingDouble) {
      dbmvcData.push_back(missingFl32);
    } else {
      dbmvcData.push_back(dbmvc);
    }
    
    double dbmhx = _dbmhx.data[igate];
    if (dbmhx == _missingDouble) {
      dbmhxData.push_back(missingFl32);
    } else {
      dbmhxData.push_back(dbmhx);
    }
    
    double dbmvx = _dbmhx.data[igate];
    if (dbmvx == _missingDouble) {
      dbmvxData.push_back(missingFl32);
    } else {
      dbmvxData.push_back(dbmvx);
    }

  }
    
  dbmhcField->setDataFl32(_nGates, dbmhcData.data(), true);
  dbmvcField->setDataFl32(_nGates, dbmvcData.data(), true);
  dbmhxField->setDataFl32(_nGates, dbmhxData.data(), true);
  dbmvxField->setDataFl32(_nGates, dbmvxData.data(), true);

  // create a RadxRay, containing these fields

  RadxRay ray;

  ray.setTime(beamTime);
  ray.setRangeGeom(_startRangeKm, _gateSpacingKm);
  
  ray.setAzimuthDeg(beam.azimuth);
  ray.setElevationDeg(beam.elevation);

  ray.setNSamples(_inputRadarParams.samplesPerBeam);
  ray.setPrtSec(_inputRadarParams.prt);
  
  ray.addField(dbmhcField);
  ray.addField(dbmvcField);
  ray.addField(dbmhxField);
  ray.addField(dbmvxField);

  // create RadxPlatform for radar location

  RadxPlatform platform;
  platform.setInstrumentName(_params.radar_name);
  platform.setLatitudeDeg(_inputRadarParams.latitude);
  platform.setLongitudeDeg(_inputRadarParams.longitude);
  platform.setAltitudeKm(_inputRadarParams.altitude);

  // process this ray
  
  processRay(platform, &ray);

}

////////////////////////////////////////////////////////////////
// set indices for for a specified field
// Sets indices to -1 if field is not in params list.

void DsrDataMgr::_setMomentsIndices(Params::moments_id_t paramId,
				    moments_field_t &field)
  
{
  
  _setMomentsParamsIndex(paramId, field);
  _setMomentsDataIndex(field);
  
}

////////////////////////////////////////////////////////////////
// set index in param file for a specified field
// Sets indices to -1 if field is not in params list.

void DsrDataMgr::_setMomentsParamsIndex(Params::moments_id_t paramId,
					moments_field_t &field)

{
  
  field.id = paramId;
  
  // get the DsrName

  field.dsrName = _getMomentsParamsName(paramId);
  
  // is this field in the param file?
  
  if (field.dsrName.size() < 1) {
    field.paramsIndex = -1;
    field.dataIndex = -1;
  }
  
  field.paramsIndex = _getMomentsParamsIndex(field.dsrName);

}

////////////////////////////////////////////////////////////////
// set index for a specified field in input data
// Sets data index to -1 if field is not in data

void DsrDataMgr::_setMomentsDataIndex(moments_field_t &field)

{
  
  field.dataIndex = _getInputDataIndex(field.dsrName);
}

////////////////////////////////////////////////////////////////
// get Dsr field name for specified moments ID
// Returns empty string on failure

string DsrDataMgr::_getMomentsParamsName(Params::moments_id_t paramId)
  
{

  for (int ii = 0; ii < _params.input_fields_n; ii++) {
    if (_params._input_fields[ii].id == paramId) {
      return _params._input_fields[ii].moments_name;
    }
  } // ii 

  return "";
  
}

////////////////////////////////////////////////////////////////
// get params ID for specified moments name
// Returns -1 on failure

int DsrDataMgr::_getMomentsParamsIndex(const string &dsrName)
  
{

  // find the params index of a moments field

  for (int ii = 0; ii < _params.input_fields_n; ii++) {
    string paramName = _params._input_fields[ii].moments_name;
    if (paramName == dsrName) {
      return ii;
    }
  } // ii 

  return -1;
  
}

////////////////////////////////////////////////////////////////
// get data index for a specified field name
// Returns -1 on failure

int DsrDataMgr::_getInputDataIndex(const string &dsrName)

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
// load up the moments data for each field

void DsrDataMgr::_loadMomentsData()

{

  // load up the moments data for each field

  _loadMomentsData(_dbmhc);
  _loadMomentsData(_dbmhx);
  _loadMomentsData(_dbmvc);
  _loadMomentsData(_dbmvx);

}

////////////////////////////////////////////////////////////////
// load the moments data
// Set to missing if not available

void DsrDataMgr::_loadMomentsData(moments_field_t &field)

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
// load field data

void DsrDataMgr::_loadInputField(const DsRadarBeam &beam, int index, double *fldData)

{

  const DsFieldParams *fparams = _inputFieldParams[index];

  // start by filling with missing values
  
  for (int ii = 0; ii < _nGates; ii++) {
    fldData[ii] = _missingDouble;
  }

  if (index < 0) {
    // field not in input data
    return;
  }
  
  if (beam.byteWidth == 4) {

    fl32 missingVal = (fl32) fparams->missingDataValue;
    fl32 *fval = (fl32 *) beam.getData() + index;
    for (int ii = 0; ii < _nGates; ii++, fval += _nFieldsIn) {
      if (*fval != missingVal) {
        fldData[ii] = *fval;
      }
    }

  } else if (beam.byteWidth == 2) {

    ui16 missingVal = (ui16) fparams->missingDataValue;
    double scale = _inputFieldParams[index]->scale;
    double bias = _inputFieldParams[index]->bias;
    ui16 *sval = (ui16 *) beam.getData() + index;
    for (int ii = 0; ii < _nGates; ii++, sval += _nFieldsIn) {
      if (*sval != missingVal) {
        fldData[ii] = *sval * scale + bias;
      }
    }

  } else {
    
    ui08 missingVal = (ui08) fparams->missingDataValue;
    double scale = _inputFieldParams[index]->scale;
    double bias = _inputFieldParams[index]->bias;
    ui08 *bval = (ui08 *) beam.getData() + index;
    for (int ii = 0; ii < _nGates; ii++, bval += _nFieldsIn) {
      if (*bval != missingVal) {
        fldData[ii] = *bval * scale + bias;
      }
    }

  }

}

////////////////////////////////////////////
// process the moments data in the beam

// void DsrDataMgr::_processMoments(const RadxTime &beamTime)

// {
  
//   double range = _inputRadarParams.startRange;
//   for (int igate = 0; igate < _nGates; igate++, range += _inputRadarParams.gateSpacing) {
    
//     MomentData mdata;
//     mdata.snr = _snr.data[igate];
//     mdata.snrhc = _snrhc.data[igate];
//     mdata.snrhx = _snrhx.data[igate];
//     mdata.snrvc = _snrvc.data[igate];
//     mdata.snrvx = _snrvx.data[igate];
//     mdata.dbm = _dbm.data[igate];
//     mdata.dbmhc = _dbmhc.data[igate];
//     mdata.dbmhx = _dbmhx.data[igate];
//     mdata.dbmvc = _dbmvc.data[igate];
//     mdata.dbmvx = _dbmvx.data[igate];
//     mdata.dbz = _dbz.data[igate];
//     mdata.vel = _vel.data[igate];
//     mdata.width = _width.data[igate];
//     mdata.zdrm = mdata.dbmhc - mdata.dbmvc;
//     mdata.ldrh = _ldrh.data[igate];
//     mdata.ldrv = _ldrv.data[igate];
//     mdata.phidp = _phidp.data[igate];
//     mdata.rhohv = _rhohv.data[igate];

//     // add to layer statss

//     addDataPoint(beamTime, range, mdata);
    
//   } // igate

// }


