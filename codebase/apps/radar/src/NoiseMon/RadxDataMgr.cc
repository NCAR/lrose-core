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
// RadxDataMgr.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2022
//
///////////////////////////////////////////////////////////////
//
// Data manager for Radx moments data
//
////////////////////////////////////////////////////////////////

#include "RadxDataMgr.hh"
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <Mdv/GenericRadxFile.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxRay.hh>

using namespace std;

const double RadxDataMgr::_missingDouble = -9999.0;
const double RadxDataMgr::_missingTest = -9998.0;

// Constructor

RadxDataMgr::RadxDataMgr(const string &prog_name,
                         const Args &args,
                         const Params &params) :
        StatsMgr(prog_name, args, params)
  
{
  
  _totalRayCount = 0;

  // set up field name map

  for (int ii = 0; ii < _params.input_fields_n; ii++) {
    Params::input_field_t &infield = _params._input_fields[ii];
    _fieldNameMap[infield.id] = infield.moments_name;
  }

}

// destructor

RadxDataMgr::~RadxDataMgr()

{

}

//////////////////////////////////////////////////
// Run

int RadxDataMgr::run ()
{

  if (_params.debug) {
    cerr << "Running NoiseMon in RADX_MOMENTS_INPUT mode" << endl;
  }

  // check if start and end times are set

  bool startTimeSet = true;
  time_t startTime = RadxTime::parseDateTime(_params.start_time);
  if (startTime == RadxTime::NEVER || startTime < 1) {
    startTimeSet = false;
  }

  bool endTimeSet = true;
  time_t endTime = RadxTime::parseDateTime(_params.end_time);
  if (endTime == RadxTime::NEVER || endTime < 1) {
    endTimeSet = false;
  }
  
  vector<string> paths = _args.inputFileList;
  if (paths.size() == 0) {

    if (startTimeSet && endTimeSet) {
      
      if (_params.debug) {
        cerr << "  Input dir: " << _params.input_dir << endl;
        cerr << "  Start time: " << RadxTime::strm(startTime) << endl;
        cerr << "  End time: " << RadxTime::strm(endTime) << endl;
      }
      
      // get the files to be processed
      
      RadxTimeList tlist;
      tlist.setDir(_params.input_dir);
      tlist.setModeInterval(startTime, endTime);
      if (tlist.compile()) {
        cerr << "ERROR - NoiseMon::RadxDataMgr::run()" << endl;
        cerr << "  Cannot compile time list, dir: " << _params.input_dir << endl;
        cerr << "  Start time: " << RadxTime::strm(startTime) << endl;
        cerr << "  End time: " << RadxTime::strm(endTime) << endl;
        cerr << tlist.getErrStr() << endl;
        return -1;
      }
      
      paths = tlist.getPathList();
      
      if (paths.size() < 1) {
        cerr << "ERROR - NoiseMon::RadxDataMgr::run()" << endl;
        cerr << "  No files found, dir: " << _params.input_dir << endl;
        return -1;
      }
    
    } // if (startTimeSet && endTimeSet)

  }

  // loop through the input file list
  
  int iret = 0;
  for (size_t ipath = 0; ipath < paths.size(); ipath++) {
    if (_processFile(paths[ipath])) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// Process a file
// Returns 0 on success, -1 on failure

int RadxDataMgr::_processFile(const string &filePath)
{
  
  PMU_auto_register(((string) "Processing file: " + filePath).c_str());

  // ensure memory is freed up
  
  _readVol.clear();

  if (_params.debug) {
    cerr << "INFO - RadxDataMgr::_processFile" << endl;
    cerr << "  Input file path: " << filePath << endl;
    cerr << "  Reading in file ..." << endl;
  }
  
  // read in file
  
  if (_readFile(filePath)) {
    return -1;
  }

  // check we have at least 2 rays

  if (_readVol.getNRays() < 2) {
    cerr << "ERROR - RadxDataMgr::_processFile" << endl;
    cerr << "  Too few rays: " << _readVol.getNRays() << endl;
    return -1;
  }

  // loop through the rays, processing them

  const vector<RadxRay *> &rays = _readVol.getRays();
  for (size_t ii = 0; ii < rays.size(); ii++) {
    _processRay(rays[ii]);
  }

  // free up

  _readVol.clear();

  return 0;

}

//////////////////////////////////////////////////
// Read in a RADX file
// Returns 0 on success, -1 on failure

int RadxDataMgr::_readFile(const string &filePath)
{

  GenericRadxFile inFile;
  _setupRead(inFile);
  
  // read in file
  
  if (inFile.readFromPath(filePath, _readVol)) {
    cerr << "ERROR - RadxDataMgr::_readFile" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }

  // convert data to floats

  _readVol.convertToFl32();

  return 0;

}

//////////////////////////////////////////////////
// set up read

void RadxDataMgr::_setupRead(RadxFile &file)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }

  for (int ii = 0; ii < _params.input_fields_n; ii++) {
    file.addReadField(_params._input_fields[ii].moments_name);
  } // ii 

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.printReadRequest(cerr);
  }
  
}

/////////////////////
// process a ray

void RadxDataMgr::_processRay(const RadxRay *ray)

{
  
  _nGates = ray->getNGates();

  // check elevation angle

  double elev = ray->getElevationDeg();
  if (elev > 90) {
    elev = 180.0 - elev;
  }
  if (elev < _params.min_elevation) {
    // clearStats();
    return;
  }

  // at start, print headers
  
  double rayTime = ray->getTimeDouble();
  if (_totalRayCount == 0) {
    setStartTime(rayTime);
  }
  setEndTime(rayTime);
  _totalRayCount++;
  
  setPrt(ray->getPrtSec());
  setEl(ray->getElevationDeg());
  setAz(ray->getAzimuthDeg());

  // load up the moments data for each field

  _loadMomentsData(ray, Params::SNR, _snr);
  _loadMomentsData(ray, Params::SNRHC, _snrhc);
  _loadMomentsData(ray, Params::SNRHX, _snrhx);
  _loadMomentsData(ray, Params::SNRVC, _snrvc);
  _loadMomentsData(ray, Params::SNRVX, _snrvx);
  _loadMomentsData(ray, Params::DBM, _dbm);
  _loadMomentsData(ray, Params::DBMHC, _dbmhc);
  _loadMomentsData(ray, Params::DBMHX, _dbmhx);
  _loadMomentsData(ray, Params::DBMVC, _dbmvc);
  _loadMomentsData(ray, Params::DBMVX, _dbmvx);
  _loadMomentsData(ray, Params::DBZ, _dbz);
  _loadMomentsData(ray, Params::VEL, _vel);
  _loadMomentsData(ray, Params::WIDTH, _width);
  _loadMomentsData(ray, Params::ZDRM, _zdrm);
  _loadMomentsData(ray, Params::LDRH, _ldrh);
  _loadMomentsData(ray, Params::LDRV, _ldrv);
  _loadMomentsData(ray, Params::PHIDP, _phidp);
  _loadMomentsData(ray, Params::RHOHV, _rhohv);

  // process the moments

  _processMoments(ray);

  // if we have done a full rotation, process the data

  checkCompute();

}

////////////////////////////////////////////////////////////////
// load the moments data
// Set to missing if not available

void RadxDataMgr::_loadMomentsData(const RadxRay *ray,
                                   Params::moments_id_t id,
                                   moments_field_t &field)

{

  field.id = id;
  field.dsrName = _fieldNameMap[id];
  
  // allocate data array

  if (_nGates > (int) field.data_.size()) {
    field.data_.free();
    field.data = (double *) field.data_.alloc(_nGates);
  }
  
  // initialize
  
  for (int ii = 0; ii < _nGates; ii++) {
    field.data[ii] = _missingDouble;
  }
  
  // get field
  
  const RadxField *radxField = ray->getField(field.dsrName);
  
  if (radxField == NULL) {
    // field not in input data
    return;
  }
  
  Radx::fl32 missingFl32 = radxField->getMissingFl32();
  const Radx::fl32 *fvals = radxField->getDataFl32();
  for (int ii = 0; ii < _nGates; ii++) {
    Radx::fl32 fval = fvals[ii];
    if (fval != missingFl32) {
      field.data[ii] = fvals[ii];
    }
  }

}

////////////////////////////////////////////
// process the moments data in the beam

void RadxDataMgr::_processMoments(const RadxRay *ray)

{
  
  double range = ray->getStartRangeKm();
  for (int igate = 0; igate < _nGates; igate++, range += ray->getGateSpacingKm()) {
    
    MomentData mdata;
    mdata.snr = _snr.data[igate];
    mdata.snrhc = _snrhc.data[igate];
    mdata.snrhx = _snrhx.data[igate];
    mdata.snrvc = _snrvc.data[igate];
    mdata.snrvx = _snrvx.data[igate];
    mdata.dbm = _dbm.data[igate];
    mdata.dbmhc = _dbmhc.data[igate];
    mdata.dbmhx = _dbmhx.data[igate];
    mdata.dbmvc = _dbmvc.data[igate];
    mdata.dbmvx = _dbmvx.data[igate];
    mdata.dbz = _dbz.data[igate];
    mdata.vel = _vel.data[igate];
    mdata.width = _width.data[igate];
    mdata.zdrm = mdata.dbmhc - mdata.dbmvc;
    mdata.ldrh = _ldrh.data[igate];
    mdata.ldrv = _ldrv.data[igate];
    mdata.phidp = _phidp.data[igate];
    mdata.rhohv = _rhohv.data[igate];

    // add to layer statss

    addDataPoint(range, mdata);
    
  } // igate

}

#ifdef JUNK

////////////////////////////////////////////////////////////////
// set indices for for a specified field
// Sets indices to -1 if field is not in params list.

void RadxDataMgr::_setMomentsIndices(Params::moments_id_t paramId,
                                     moments_field_t &field)
  
{
  
  _setMomentsParamsIndex(paramId, field);
  _setMomentsDataIndex(field);
  
}

////////////////////////////////////////////////////////////////
// set index in param file for a specified field
// Sets indices to -1 if field is not in params list.

void RadxDataMgr::_setMomentsParamsIndex(Params::moments_id_t paramId,
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

void RadxDataMgr::_setMomentsDataIndex(moments_field_t &field)

{
  
  field.dataIndex = _getInputDataIndex(field.dsrName);
}

////////////////////////////////////////////////////////////////
// get Dsr field name for specified moments ID
// Returns empty string on failure

string RadxDataMgr::_getMomentsParamsName(Params::moments_id_t paramId)
  
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

int RadxDataMgr::_getMomentsParamsIndex(const string &dsrName)
  
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

int RadxDataMgr::_getInputDataIndex(const string &dsrName)

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
// set indices for for a specified field
// Sets indices to -1 if field is not in params list.

void RadxDataMgr::_setMomentsIndices(Params::moments_id_t paramId,
                                     moments_field_t &field)
  
{
  
  _setMomentsParamsIndex(paramId, field);
  _setMomentsDataIndex(field);
  
}

////////////////////////////////////////////////////////////////
// set index in param file for a specified field
// Sets indices to -1 if field is not in params list.

void RadxDataMgr::_setMomentsParamsIndex(Params::moments_id_t paramId,
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

void RadxDataMgr::_setMomentsDataIndex(moments_field_t &field)

{
  
  field.dataIndex = _getInputDataIndex(field.dsrName);
}

////////////////////////////////////////////////////////////////
// get Dsr field name for specified moments ID
// Returns empty string on failure

string RadxDataMgr::_getMomentsParamsName(Params::moments_id_t paramId)
  
)

{

  // load up the moments data for each field

  _loadMomentsData(_snr);
  _loadMomentsData(_snrhc);
  _loadMomentsData(_snrhx);
  _loadMomentsData(_snrvc);
  _loadMomentsData(_snrvx);
  _loadMomentsData(_dbm);
  _loadMomentsData(_dbmhc);
  _loadMomentsData(_dbmhx);
  _loadMomentsData(_dbmvc);
  _loadMomentsData(_dbmvx);
  _loadMomentsData(_dbz);
  _loadMomentsData(_vel);
  _loadMomentsData(_width);
  _loadMomentsData(_zdrc);
  _loadMomentsData(_zdrm);
  _loadMomentsData(_ldrh);
  _loadMomentsData(_ldrv);
  _loadMomentsData(_phidp);
  _loadMomentsData(_rhohv);

}

#endif
