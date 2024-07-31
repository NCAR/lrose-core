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
// EccoStats.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2024
//
////////////////////////////////////////////////////////////////////
//
// EccoStats computes statistics from the Ecco output files.
// See the Ecco app for details.
//
/////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <dataport/bigend.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/mem.h>
#include <radar/ConvStratFinder.hh>
#include <didss/DsInputPath.hh>
#include <didss/DataFileNames.hh>
#include "EccoStats.hh"
using namespace std;

const fl32 EccoStats::_missingFl32 = -9999.0;

// Constructor

EccoStats::EccoStats(int argc, char **argv)

{

  isOK = true;
  _inputPaths = NULL;
  
  // set programe name

  _progName = "EccoStats";
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
    return;
  }

  // check that start and end time is set in archive mode

  if (_params.mode == Params::ARCHIVE) {
    if (_args.startTime == 0 || _args.endTime == 0) {
      cerr << "ERROR - must specify start and end dates." << endl << endl;
      _args.usage(_progName, cerr);
      isOK = false;
      return;
    }
  }
  
  // initialize the data input object

  if (_params.mode == Params::ARCHIVE) {
    _inputPaths = new DsInputPath(_progName,
                                  _params.debug >= Params::DEBUG_EXTRA,
                                  _params.input_dir,
                                  _args.startTime,
                                  _args.endTime);
  } else {
    _inputPaths = new DsInputPath(_progName,
                                  _params.debug >= Params::DEBUG_EXTRA,
                                  _args.inputFileList);
  }
  if (_params.set_month_range) {
    _inputPaths->setValidMonthRange(_params.min_month, _params.max_month);
  }

  // init field pointers
  // these point to memory in MdvxField objects and do not need to be freed
  
  _eccoTypeField = NULL;
  _convectivityField = NULL;
  _terrainHtField = NULL;
  _waterFlagField = NULL;

  // init arrays

  _initArraysToNull();

  // init aggregation

  if (_params.aggregate_grid_cells) {
    _agNx = _params.aggregate_nx;
    _agNy = _params.aggregate_ny;
  } else {
    _agNx = 1;
    _agNy = 1;
  }

}

// destructor

EccoStats::~EccoStats()

{

  if (_inputPaths) {
    delete _inputPaths;
  }
  
  // free up arrays

  _freeArrays();
  
}

//////////////////////////////////////////////////
// Run

int EccoStats::Run()
{

  if (_params.compute_mrms_coverage) {
    return _computeCoverage();
  } else {
    return _computeEccoStats();
  }
  
}

//////////////////////////////////////////////////
// Compute ecco stats

int EccoStats::_computeEccoStats()
{
  
  int iret = 0;

  // loop until end of data
  
  _inputPaths->reset();
  int fileCount = 0;
  char *nextPath = NULL;
  while ((nextPath = _inputPaths->next()) != NULL) {
    
    // do the read

    if (_readEcco(nextPath)) {
      // failure - skip
      cerr << "ERROR - EccoStats::_computeEccoStats()" << endl;
      iret = -1;
      continue;
    }
    
    // if this is the first file, allocate and initialize the arrays
    
    Mdvx::field_header_t fhdr = _eccoTypeField->getFieldHeader();
    if (fileCount == 0) {

      // first file, initialize grid

      _firstTime = _inMdvx.getValidTime();

      _inNx = fhdr.nx;
      _inNy = fhdr.ny;
      
      _nx = ((fhdr.nx - 1) / _agNx) + 1;
      _ny = ((fhdr.ny - 1) / _agNy) + 1;
      _nz = 24; // diurnal hour index

      _minx = fhdr.grid_minx + (_agNx - 1) * 0.5 * fhdr.grid_dx;
      _miny = fhdr.grid_miny + (_agNy - 1) * 0.5 * fhdr.grid_dy;
      _minz = 0.0;

      _dx = fhdr.grid_dx * _agNx;
      _dy = fhdr.grid_dy * _agNy;
      _dz = 1.0;

      _allocArrays();
      _initForStats();
      
    } else {

      // check grid size has not changed
      
      if (_inNx != fhdr.nx || _inNy != fhdr.ny) {
        cerr << "ERROR - grid size has changed" << endl;
        cerr << "  nx, ny found: " << fhdr.nx << ", " << fhdr.ny << endl;
        cerr << "  nx, ny should be: " << _inNx << ", " << _inNy << endl;
        cerr << "  skipping this file: " << _inMdvx.getPathInUse() << endl;
        iret = -1;
        continue;
      }

    }
    _lastTime = _inMdvx.getValidTime();
    fileCount++;
    
    // update the stats, based on the data in the file
    
    _updateStatsFromInputFile();
    
    // clear
    
    if (_params.debug) {
      cerr << "Done processing input file: " << _inMdvx.getPathInUse() << endl;
    }

  } // while

  // add the fields to the output file

  _addFieldsToOutput();
  
  // write out
  
  if (_doWrite()) {
    cerr << "ERROR - EccoStats::_computeEccoStats()" << endl;
    return -1;
  }
    
  return iret;

}

/////////////////////////////////////////////////////////
// initialize the arrays

void EccoStats::_initArraysToNull()
  
{

  // init arrays
  
  _stratLowCount = NULL;
  _stratMidCount = NULL;
  _stratHighCount = NULL;
  _mixedCount = NULL;
  _convShallowCount = NULL;
  _convMidCount = NULL;
  _convDeepCount = NULL;
  _convElevCount = NULL;

  _stratLowConv = NULL;
  _stratMidConv = NULL;
  _stratHighConv = NULL;
  _mixedConv = NULL;
  _convShallowConv = NULL;
  _convMidConv = NULL;
  _convDeepConv = NULL;
  _convElevConv = NULL;

  _validCount = NULL;
  _totalCount = NULL;
  
  _terrainHt = NULL;
  _waterFlag = NULL;

  _lat = NULL;
  _lon = NULL;
  _hourOfDay = NULL;

}

/////////////////////////////////////////////////////////
// allocate the arrays

void EccoStats::_allocArrays()
  
{

  // counts
  
  _stratLowCount = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  _stratMidCount = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  _stratHighCount = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  _mixedCount = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  _convShallowCount = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  _convMidCount = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  _convDeepCount = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  _convElevCount = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));

  // convectivity
  
  _stratLowConv = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  _stratMidConv = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  _stratHighConv = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  _mixedConv = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  _convShallowConv = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  _convMidConv = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  _convDeepConv = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  _convElevConv = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));

  _validCount = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  _totalCount = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  
  // terrain ht

  _terrainHt = (fl32 **) ucalloc2(_ny, _nx, sizeof(fl32));
  _waterFlag = (fl32 **) ucalloc2(_ny, _nx, sizeof(fl32));

 // lat/lon

  _lon = (double **) ucalloc2(_ny, _nx, sizeof(double));
  _lat = (double **) ucalloc2(_ny, _nx, sizeof(double));

  // local hour of day
  
  _hourOfDay = (int **) ucalloc2(_ny, _nx, sizeof(int));

}

/////////////////////////////////////////////////////////
// free the arrays

void EccoStats::_freeArrays()
  
{

  ufree3((void ***) _stratLowCount);
  ufree3((void ***) _stratMidCount);
  ufree3((void ***) _stratHighCount);
  ufree3((void ***) _mixedCount);
  ufree3((void ***) _convShallowCount);
  ufree3((void ***) _convMidCount);
  ufree3((void ***) _convDeepCount);
  ufree3((void ***) _convElevCount);
  
  ufree3((void ***) _stratLowConv);
  ufree3((void ***) _stratMidConv);
  ufree3((void ***) _stratHighConv);
  ufree3((void ***) _mixedConv);
  ufree3((void ***) _convShallowConv);
  ufree3((void ***) _convMidConv);
  ufree3((void ***) _convDeepConv);
  ufree3((void ***) _convElevConv);

  ufree3((void ***) _validCount);
  ufree3((void ***) _totalCount);

  ufree2((void **) _terrainHt);
  ufree2((void **) _waterFlag);

  ufree2((void **) _lat);
  ufree2((void **) _lon);

  ufree2((void **) _hourOfDay);
  
}

/////////////////////////////////////////////////////////
// initialize for stats

void EccoStats::_initForStats()
  
{

  // init lat and lon arrays
  
  for (int iy = 0; iy < _ny; iy++) {
    for (int ix = 0; ix < _nx; ix++) {
      double xx = _minx + ix * _dx;
      double yy = _miny + iy * _dy;
      double lat, lon;
      _proj.xy2latlon(xx, yy, lat, lon);
      _lat[iy][ix] = lat;
      _lon[iy][ix] = lon;
    }
  }

  // load terrain height data if available
  
  _loadTerrain();
  
  // initialize the output file object
  
  _initOutputFile();
    
}

/////////////////////////////////////////////////////////
// load the terrain arrays
// Returns 0 on success, -1 on failure.

void EccoStats::_loadTerrain()
  
{

  // load terrain ht if field is available
  
  if (_terrainHtField != NULL) {
  
    // loop through 2D grid space
    
    Mdvx::field_header_t fhdrHt = _terrainHtField->getFieldHeader();
    fl32 missHt = fhdrHt.missing_data_value;
    
    fl32 *height = (fl32 *) _terrainHtField->getVol();
    
    fl32 **count = (fl32 **) ucalloc2(_ny, _nx, sizeof(double));
    fl32 **sum = (fl32 **) ucalloc2(_ny, _nx, sizeof(double));
    
    // sum up heights within each grid block
    
    size_t offset = 0;
    for (int jy = 0; jy < _inNy; jy++) {
      for (int jx = 0; jx < _inNx; jx++, offset++) {
        int iy = jy / _agNy;
        int ix = jx / _agNx;
        fl32 ht = height[offset];
        if (ht != missHt) {
          sum[iy][ix] += ht;
          count[iy][ix]++;
        }
      } // jx
    } // jy

    // compute mean

    for (int iy = 0; iy < _ny; iy++) {
      for (int ix = 0; ix < _nx; ix++) {
        if (count[iy][ix] > 0) {
          _terrainHt[iy][ix] = sum[iy][ix] / count[iy][ix];
        }
      } // ix
    } // jy

    // free upp

    ufree2((void **) count);
    ufree2((void **) sum);

  } // if (_terrainHtField != NULL) 
    
  // load water flag if field is available
  
  if (_waterFlagField != NULL) {
  
    // loop through 2D grid space
    
    Mdvx::field_header_t fhdrFlag = _waterFlagField->getFieldHeader();
    fl32 missFlag = fhdrFlag.missing_data_value;
    
    fl32 *waterFlag = (fl32 *) _waterFlagField->getVol();
    
    fl32 **count = (fl32 **) ucalloc2(_ny, _nx, sizeof(double));
    fl32 **sum = (fl32 **) ucalloc2(_ny, _nx, sizeof(double));
    
    // sum up heigflags within each grid block
    
    size_t offset = 0;
    for (int jy = 0; jy < _inNy; jy++) {
      for (int jx = 0; jx < _inNx; jx++, offset++) {
        int iy = jy / _agNy;
        int ix = jx / _agNx;
        fl32 flag = waterFlag[offset];
        if (flag != missFlag) {
          sum[iy][ix] += flag;
          count[iy][ix]++;
        }
      } // jx
    } // jy

    // compute mean
    
    for (int iy = 0; iy < _ny; iy++) {
      for (int ix = 0; ix < _nx; ix++) {
        if (count[iy][ix] > 0) {
          double mean = sum[iy][ix] / count[iy][ix];
          if (mean >= 0.5) {
            _waterFlag[iy][ix] = 1.0;
          } else {
            _waterFlag[iy][ix] = 0.0;
          }
        }
      } // ix
    } // jy
    
    // free upp

    ufree2((void **) count);
    ufree2((void **) sum);

  } // if (_waterFlagField != NULL) 
    
}
  
/////////////////////////////////////////////////////////
// process the file in _inMdvx
// Returns 0 on success, -1 on failure.

void EccoStats::_updateStatsFromInputFile()
  
{

  // compute hour of day array
  
  DateTime fileTime(_inMdvx.getValidTime());
  
  for (int iy = 0; iy < _ny; iy++) {
    for (int ix = 0; ix < _nx; ix++) {
      double lon = _lon[iy][ix];
      double lonSecs = lon * 240.0;
      DateTime lonTime = fileTime + lonSecs;
      int lonHour = lonTime.getHour();
      _hourOfDay[iy][ix] = lonHour;
    } // ix
  } // iy

  // loop through 2D grid space
  
  Mdvx::field_header_t etHdr = _eccoTypeField->getFieldHeader();
  fl32 etMiss = etHdr.missing_data_value;
  
  fl32 *echoType2D = (fl32 *) _eccoTypeField->getVol();
  fl32 *convectivity2D = (fl32 *) _convectivityField->getVol();

  size_t offset = 0;
  for (int jy = 0; jy < _inNy; jy++) {
    for (int jx = 0; jx < _inNx; jx++, offset++) {
      
      int iy = jy / _agNy;
      int ix = jx / _agNx;
      
      int hour = _hourOfDay[iy][ix];
      fl32 echoTypeFl32 = echoType2D[offset];
      
      if (echoTypeFl32 != etMiss) {
        
        int echoType = (int) floor(echoTypeFl32 + 0.5);
        fl32 convectivity = convectivity2D[offset];
        switch ((ConvStratFinder::category_t) echoType) {
          case ConvStratFinder::category_t::CATEGORY_STRATIFORM_LOW: {
            _stratLowCount[hour][iy][ix]++;
            _stratLowConv[hour][iy][ix] += convectivity;
            break;
          }
          case ConvStratFinder::category_t::CATEGORY_STRATIFORM_MID: {
            _stratMidCount[hour][iy][ix]++;
            _stratMidConv[hour][iy][ix] += convectivity;
            break;
          }
          case ConvStratFinder::category_t::CATEGORY_STRATIFORM_HIGH: {
            _stratHighCount[hour][iy][ix]++;
            _stratHighConv[hour][iy][ix] += convectivity;
            break;
          }
          case ConvStratFinder::category_t::CATEGORY_MIXED: {
            _mixedCount[hour][iy][ix]++;
            _mixedConv[hour][iy][ix] += convectivity;
            break;
          }
          case ConvStratFinder::category_t::CATEGORY_CONVECTIVE_ELEVATED: {
            _convElevCount[hour][iy][ix]++;
            _convElevConv[hour][iy][ix] += convectivity;
            break;
          }
          case ConvStratFinder::category_t::CATEGORY_CONVECTIVE_SHALLOW: {
            _convShallowCount[hour][iy][ix]++;
            _convShallowConv[hour][iy][ix] += convectivity;
            break;
          }
          case ConvStratFinder::category_t::CATEGORY_CONVECTIVE_MID: {
            _convMidCount[hour][iy][ix]++;
            _convMidConv[hour][iy][ix] += convectivity;
            break;
          }
          case ConvStratFinder::category_t::CATEGORY_CONVECTIVE_DEEP: {
            _convDeepCount[hour][iy][ix]++;
            _convDeepConv[hour][iy][ix] += convectivity;
            break;
          }
          default: {}
        }

        _validCount[hour][iy][ix]++;

      } // check for missing

      _totalCount[hour][iy][ix]++;

    } // jx
  } // jy

}
  
/////////////////////////////////////////////////////////
// perform the read
//
// Returns 0 on success, -1 on failure.

int EccoStats::_readEcco(const char *path)
  
{
  
  _inMdvx.clear();
  if (_params.debug >= Params::DEBUG_EXTRA) {
    _inMdvx.setDebug(true);
  }
  _inMdvx.addReadField(_params.ecco_type_comp_field_name);
  _inMdvx.addReadField(_params.convectivity_comp_field_name);
  if (strlen(_params.terrain_height_field_name) > 0) {
    _inMdvx.addReadField(_params.terrain_height_field_name);
  }
  if (strlen(_params.water_flag_field_name) > 0) {
    _inMdvx.addReadField(_params.water_flag_field_name);
  }
  _inMdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _inMdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _inMdvx.printReadRequest(cerr);
  }
  _inMdvx.setReadPath(path);
  
  // read in
  
  if (_inMdvx.readVolume()) {
    cerr << "ERROR - EccoStats::_readEcco" << endl;
    cerr << "  Cannot read in data." << endl;
    cerr << _inMdvx.getErrStr() << endl;
    return -1;
  }

  _eccoTypeField = _inMdvx.getField(_params.ecco_type_comp_field_name);
  _convectivityField = _inMdvx.getField(_params.convectivity_comp_field_name);
  if (strlen(_params.terrain_height_field_name) > 0) {
    _terrainHtField = _inMdvx.getField(_params.terrain_height_field_name);
  }
  if (strlen(_params.water_flag_field_name) > 0) {
    _waterFlagField = _inMdvx.getField(_params.water_flag_field_name);
  }
  if (_eccoTypeField == NULL) {
    cerr << "ERROR - readEcco(), file: " << _inMdvx.getPathInUse() << endl;
    cerr << "  Cannot find field: " << _params.ecco_type_comp_field_name << endl;
    return -1;
  }
  if (_convectivityField == NULL) {
    cerr << "ERROR - readEcco(), file: " << _inMdvx.getPathInUse() << endl;
    cerr << "  Cannot find field: " << _params.convectivity_comp_field_name << endl;
    return -1;
  }

  // set projection, and lat/lon arrays
  
  Mdvx::field_header_t fhdr = _eccoTypeField->getFieldHeader();
  _proj.init(fhdr);
  
  if (_params.debug) {
    cerr << "Success - read in file: " << _inMdvx.getPathInUse() << endl;
  }

  return 0;

}

/////////////////////////////////////////////////////////
// prepare the output file

void EccoStats::_initOutputFile()
  
{
  
  // create output DsMdvx object
  // copying master header from input object
  
  _outMdvx.clear();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _outMdvx.setDebug(true);
  }
  Mdvx::master_header_t mhdr = _inMdvx.getMasterHeader();
  mhdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  mhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  mhdr.vlevel_included = 1;
  _outMdvx.setMasterHeader(mhdr);
  _outMdvx.setDataSetInfo(_params.output_data_set_info);
  _outMdvx.setDataSetSource(_params.output_data_set_source);
  char name[128];
  if (_params.min_month == _params.max_month) {
    snprintf(name, 128, "EccoStats for month %d\n", _params.min_month);
  } else {
    snprintf(name, 128, "EccoStats for months %d to %d\n",
             _params.min_month, _params.max_month);
  }
  _outMdvx.setDataSetName(name);
  _outMdvx.setMdv2NcfOutput(true, true, true, true);
  
}

/////////////////////////////////////////////////////////
// add fields to the output object

void EccoStats::_addFieldsToOutput()
  
{

  // clear
  
  _outMdvx.clearFields();

  // add 3d summary count fields
  
  _outMdvx.addField(_make3DField(_validCount,
                                 "ValidCount",
                                 "count_for_valid_obs",
                                 "count"));
  
  _outMdvx.addField(_make3DField(_totalCount,
                                 "TotalCount",
                                 "count_for_all_obs",
                                 "count"));
  
  // add 3d count fields by echo type

  _outMdvx.addField(_make3DField(_stratLowCount,
                                 "StratLowCount",
                                 "count_for_stratiform_low",
                                 "count"));
  
  _outMdvx.addField(_make3DField(_stratMidCount,
                                 "StratMidCount",
                                 "count_for_stratiform_mid",
                                 "count"));
  
  _outMdvx.addField(_make3DField(_stratHighCount,
                                 "StratHighCount",
                                 "count_for_stratiform_high",
                                 "count"));
  
  _outMdvx.addField(_make3DField(_mixedCount,
                                 "MixedCount",
                                 "count_for_mixed",
                                 "count"));
  
  _outMdvx.addField(_make3DField(_convShallowCount,
                                 "ConvShallowCount",
                                 "count_for_convective_shallow",
                                 "count"));
  
  _outMdvx.addField(_make3DField(_convMidCount,
                                 "ConvMidCount",
                                 "count_for_convective_mid",
                                 "count"));
  
  _outMdvx.addField(_make3DField(_convDeepCount,
                                 "ConvDeepCount",
                                 "count_for_convective_deep",
                                 "count"));
  
  _outMdvx.addField(_make3DField(_convElevCount,
                                 "ConvElevCount",
                                 "count_for_convective_elevated",
                                 "count"));
  
  // add 3d convectivity fields by echo type
  
  _outMdvx.addField(_make3DField(_stratLowConv,
                                 "StratLowConv",
                                 "convectivity_for_stratiform_low",
                                 ""));
  
  _outMdvx.addField(_make3DField(_stratMidConv,
                                 "StratMidConv",
                                 "convectivity_for_stratiform_mid",
                                 ""));
  
  _outMdvx.addField(_make3DField(_stratHighConv,
                                 "StratHighConv",
                                 "convectivity_for_stratiform_high",
                                 ""));
  
  _outMdvx.addField(_make3DField(_mixedConv,
                                 "MixedConv",
                                 "convectivity_for_mixed",
                                 ""));
  
  _outMdvx.addField(_make3DField(_convShallowConv,
                                 "ConvShallowConv",
                                 "convectivity_for_convective_shallow",
                                 ""));
  
  _outMdvx.addField(_make3DField(_convMidConv,
                                 "ConvMidConv",
                                 "convectivity_for_convective_mid",
                                 ""));
  
  _outMdvx.addField(_make3DField(_convDeepConv,
                                 "ConvDeepConv",
                                 "convectivity_for_convective_deep",
                                 ""));
  
  _outMdvx.addField(_make3DField(_convElevConv,
                                 "ConvElevConv",
                                 "convectivity_for_convective_elevated",
                                 ""));
  
  // add 3D valid fractional fields

  _outMdvx.addField(_computeFrac3DField(_stratLowCount,
                                        _validCount,
                                        "StratLowValidFrac3D",
                                        "valid_fraction_for_stratiform_low",
                                        ""));
  
  _outMdvx.addField(_computeFrac3DField(_stratMidCount,
                                        _validCount,
                                        "StratMidValidFrac3D",
                                        "valid_fraction_for_stratiform_mid",
                                        "count"));
  
  _outMdvx.addField(_computeFrac3DField(_stratHighCount,
                                        _validCount,
                                        "StratHighValidFrac3D",
                                        "valid_fraction_for_stratiform_high",
                                        "count"));
  
  _outMdvx.addField(_computeFrac3DField(_mixedCount,
                                        _validCount,
                                        "MixedValidFrac3D",
                                        "valid_fraction_for_mixed",
                                        "count"));
  
  _outMdvx.addField(_computeFrac3DField(_convShallowCount,
                                        _validCount,
                                        "ConvShallowValidFrac3D",
                                        "valid_fraction_for_convective_shallow",
                                        "count"));
  
  _outMdvx.addField(_computeFrac3DField(_convMidCount,
                                        _validCount,
                                        "ConvMidValidFrac3D",
                                        "valid_fraction_for_convective_mid",
                                        "count"));
  
  _outMdvx.addField(_computeFrac3DField(_convDeepCount,
                                        _validCount,
                                        "ConvDeepValidFrac3D",
                                        "valid_fraction_for_convective_deep",
                                        "count"));
  
  _outMdvx.addField(_computeFrac3DField(_convElevCount,
                                        _validCount,
                                        "ConvElevValidFrac3D",
                                        "valid_fraction_for_convective_elevated",
                                        "count"));
  
  _outMdvx.addField(_computeFrac3DField(_stratLowConv,
                                        _validCount,
                                        "StratLowConvMean3D",
                                        "mean_convectivity_for_stratiform_low",
                                        ""));
  
  _outMdvx.addField(_computeFrac3DField(_stratMidConv,
                                        _validCount,
                                        "StratMidConvMean3D",
                                        "mean_convectivity_for_stratiform_mid",
                                        ""));
  
  _outMdvx.addField(_computeFrac3DField(_stratHighConv,
                                        _validCount,
                                        "StratHighConvMean3D",
                                        "mean_convectivity_for_stratiform_high",
                                        ""));
  
  _outMdvx.addField(_computeFrac3DField(_mixedConv,
                                        _validCount,
                                        "MixedConvMean3D",
                                        "mean_convectivity_for_mixed",
                                        ""));
  
  _outMdvx.addField(_computeFrac3DField(_convShallowConv,
                                        _validCount,
                                        "ConvShallowConvMean3D",
                                        "mean_convectivity_for_convective_shallow",
                                        ""));
  
  _outMdvx.addField(_computeFrac3DField(_convMidConv,
                                        _validCount,
                                        "ConvMidConvMean3D",
                                        "mean_convectivity_for_convective_mid",
                                        ""));
  
  _outMdvx.addField(_computeFrac3DField(_convDeepConv,
                                        _validCount,
                                        "ConvDeepConvMean3D",
                                        "mean_convectivity_for_convective_deep",
                                        ""));
  
  _outMdvx.addField(_computeFrac3DField(_convElevConv,
                                        _validCount,
                                        "ConvElevConvMean3D",
                                        "mean_convectivity_for_convective_elevated",
                                        ""));
  
  // add 2D valid fractional fields

  _outMdvx.addField(_computeFrac2DField(_stratLowCount,
                                        _validCount,
                                        "StratLowValidFrac2D",
                                        "valid_fraction_for_stratiform_low",
                                        ""));
  
  _outMdvx.addField(_computeFrac2DField(_stratMidCount,
                                        _validCount,
                                        "StratMidValidFrac2D",
                                        "valid_fraction_for_stratiform_mid",
                                        "count"));
  
  _outMdvx.addField(_computeFrac2DField(_stratHighCount,
                                        _validCount,
                                        "StratHighValidFrac2D",
                                        "valid_fraction_for_stratiform_high",
                                        "count"));
  
  _outMdvx.addField(_computeFrac2DField(_mixedCount,
                                        _validCount,
                                        "MixedValidFrac2D",
                                        "valid_fraction_for_mixed",
                                        "count"));
  
  _outMdvx.addField(_computeFrac2DField(_convShallowCount,
                                        _validCount,
                                        "ConvShallowValidFrac2D",
                                        "valid_fraction_for_convective_shallow",
                                        "count"));
  
  _outMdvx.addField(_computeFrac2DField(_convMidCount,
                                        _validCount,
                                        "ConvMidValidFrac2D",
                                        "valid_fraction_for_convective_mid",
                                        "count"));
  
  _outMdvx.addField(_computeFrac2DField(_convDeepCount,
                                        _validCount,
                                        "ConvDeepValidFrac2D",
                                        "valid_fraction_for_convective_deep",
                                        "count"));
  
  _outMdvx.addField(_computeFrac2DField(_convElevCount,
                                        _validCount,
                                        "ConvElevValidFrac2D",
                                        "valid_fraction_for_convective_elevated",
                                        "count"));
  
  _outMdvx.addField(_computeFrac2DField(_stratLowConv,
                                        _validCount,
                                        "StratLowConvMean2D",
                                        "mean_convectivity_for_stratiform_low",
                                        ""));
  
  _outMdvx.addField(_computeFrac2DField(_stratMidConv,
                                        _validCount,
                                        "StratMidConvMean2D",
                                        "mean_convectivity_for_stratiform_mid",
                                        ""));
  
  _outMdvx.addField(_computeFrac2DField(_stratHighConv,
                                        _validCount,
                                        "StratHighConvMean2D",
                                        "mean_convectivity_for_stratiform_high",
                                        ""));
  
  _outMdvx.addField(_computeFrac2DField(_mixedConv,
                                        _validCount,
                                        "MixedConvMean2D",
                                        "mean_convectivity_for_mixed",
                                        ""));
  
  _outMdvx.addField(_computeFrac2DField(_convShallowConv,
                                        _validCount,
                                        "ConvShallowConvMean2D",
                                        "mean_convectivity_for_convective_shallow",
                                        ""));
  
  _outMdvx.addField(_computeFrac2DField(_convMidConv,
                                        _validCount,
                                        "ConvMidConvMean2D",
                                        "mean_convectivity_for_convective_mid",
                                        ""));
  
  _outMdvx.addField(_computeFrac2DField(_convDeepConv,
                                        _validCount,
                                        "ConvDeepConvMean2D",
                                        "mean_convectivity_for_convective_deep",
                                        ""));
  
  _outMdvx.addField(_computeFrac2DField(_convElevConv,
                                        _validCount,
                                        "ConvElevConvMean2D",
                                        "mean_convectivity_for_convective_elevated",
                                        ""));
  
  // add 2d fields for terrain height and water flag, if available

  if (_terrainHtField) {
    _outMdvx.addField(_make2DField(_terrainHt,
                                   _terrainHtField->getFieldName(),
                                   _terrainHtField->getFieldNameLong(),
                                   _terrainHtField->getUnits()));
  }
  
  if (_waterFlagField) {
    _outMdvx.addField(_make2DField(_waterFlag,
                                   _waterFlagField->getFieldName(),
                                   _waterFlagField->getFieldNameLong(),
                                   _waterFlagField->getUnits()));
  }
  
}

/////////////////////////////////////////////////////////
// perform the write
// Returns 0 on success, -1 on failure.

int EccoStats::_doWrite()
  
{

  // set times
  
  _outMdvx.setValidTime(_firstTime);
  _outMdvx.setBeginTime(_firstTime);
  _outMdvx.setEndTime(_lastTime);
  _outMdvx.setGenTime(_firstTime);

  // write out
  
  if(_outMdvx.writeToDir(_params.output_dir)) {
    cerr << "ERROR - EccoStats::Run" << endl;
    cerr << "  Cannot write data set." << endl;
    cerr << _outMdvx.getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Wrote file: " << _outMdvx.getPathInUse() << endl;
  }
  
  return 0;

}

/////////////////////////////////////////////////////////
// create a 3d field

MdvxField *EccoStats::_make3DField(fl32 ***data,
                                   string fieldName,
                                   string longName,
                                   string units)
                                 
{

  Mdvx::field_header_t fhdr = _eccoTypeField->getFieldHeader();
  
  fhdr.missing_data_value = _missingFl32;
  fhdr.bad_data_value = _missingFl32;

  fhdr.nx = _nx; // output grid
  fhdr.ny = _ny; // output grid
  fhdr.nz = _nz; // 24 hours in day

  fhdr.grid_dx = _dx;
  fhdr.grid_dy = _dy;
  fhdr.grid_dz = _dz;

  fhdr.grid_minx = _minx;
  fhdr.grid_miny = _miny;
  fhdr.grid_minz = _minz;

  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_Z;

  fhdr.dz_constant = 1;
  fhdr.data_dimension = 3;
  
  size_t npts = fhdr.nx * fhdr.ny * fhdr.nz;
  size_t volSize = npts * sizeof(fl32);
  fhdr.volume_size = volSize;

  Mdvx::vlevel_header_t vhdr = _eccoTypeField->getVlevelHeader();
  for (int ii = 0; ii < _nz; ii++) {
    vhdr.type[ii] = Mdvx::VERT_TYPE_Z;
    vhdr.level[ii] = ii;
  }

  MdvxField::setFieldName(fieldName, fhdr);
  MdvxField::setFieldNameLong(longName, fhdr);
  MdvxField::setUnits(units, fhdr);
  MdvxField *newField =
    new MdvxField(fhdr, vhdr, NULL, false, false, false);
  newField->setVolData(**data, volSize, Mdvx::ENCODING_FLOAT32);
  newField->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_GZIP);

  return newField;

}

/////////////////////////////////////////////////////////
// create a 2d field

MdvxField *EccoStats::_make2DField(fl32 **data,
                                   string fieldName,
                                   string longName,
                                   string units)
                                 
{

  // create header
  
  Mdvx::field_header_t fhdr = _eccoTypeField->getFieldHeader();
  
  fhdr.missing_data_value = _missingFl32;
  fhdr.bad_data_value = _missingFl32;

  fhdr.nx = _nx; // output grid
  fhdr.ny = _ny; // output grid
  fhdr.nz = 1;

  fhdr.grid_dx = _dx;
  fhdr.grid_dy = _dy;
  fhdr.grid_dz = 1.0;

  fhdr.grid_minx = _minx;
  fhdr.grid_miny = _miny;
  fhdr.grid_minz = 0.0;
  
  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;

  fhdr.dz_constant = 1;
  fhdr.data_dimension = 2;

  size_t npts = fhdr.nx * fhdr.ny * fhdr.nz;
  size_t volSize = npts * sizeof(fl32);
  fhdr.volume_size = volSize;

  Mdvx::vlevel_header_t vhdr = _eccoTypeField->getVlevelHeader();
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vhdr.level[0] = 0;

  MdvxField::setFieldName(fieldName, fhdr);
  MdvxField::setFieldNameLong(longName, fhdr);
  MdvxField::setUnits(units, fhdr);

  // create field from header and data
  
  MdvxField *newField =
    new MdvxField(fhdr, vhdr, NULL, false, false, false);
  newField->setVolData(*data, volSize, Mdvx::ENCODING_FLOAT32);
  newField->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_GZIP);

  return newField;

}

/////////////////////////////////////////////////////////
// compute a fractional 3d field

MdvxField *EccoStats::_computeFrac3DField(fl32 ***data,
                                          fl32 ***counts,
                                          string fieldName,
                                          string longName,
                                          string units)
                                 
{

  // create header
  
  Mdvx::field_header_t fhdr = _eccoTypeField->getFieldHeader();
  
  fhdr.missing_data_value = _missingFl32;
  fhdr.bad_data_value = _missingFl32;

  fhdr.nx = _nx; // output grid
  fhdr.ny = _ny; // output grid
  fhdr.nz = _nz; // 24 hours in day

  fhdr.grid_dx = _dx;
  fhdr.grid_dy = _dy;
  fhdr.grid_dz = _dz;

  fhdr.grid_minx = _minx;
  fhdr.grid_miny = _miny;
  fhdr.grid_minz = _minz;

  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_Z;

  fhdr.dz_constant = 1;
  fhdr.data_dimension = 3;
  
  size_t npts = fhdr.nx * fhdr.ny * fhdr.nz;
  size_t volSize = npts * sizeof(fl32);
  fhdr.volume_size = volSize;

  Mdvx::vlevel_header_t vhdr = _eccoTypeField->getVlevelHeader();
  for (int ii = 0; ii < _nz; ii++) {
    vhdr.type[ii] = Mdvx::VERT_TYPE_Z;
    vhdr.level[ii] = ii;
  }

  MdvxField::setFieldName(fieldName, fhdr);
  MdvxField::setFieldNameLong(longName, fhdr);
  MdvxField::setUnits(units, fhdr);

  // compute fraction field

  fl32 ***frac = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  for (int iz = 0; iz < _nz; iz++) {
    for (int iy = 0; iy < _ny; iy++) {
      for (int ix = 0; ix < _nx; ix++) {
        fl32 nn = counts[iz][iy][ix];
        if (nn != 0) {
          frac[iz][iy][ix] = data[iz][iy][ix] / nn;
        }
      } // ix
    } // iy
  } // iz

  // create field from header and data
  
  MdvxField *newField =
    new MdvxField(fhdr, vhdr, NULL, false, false, false);
  newField->setVolData(**frac, volSize, Mdvx::ENCODING_FLOAT32);
  newField->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_GZIP);

  // free up

  ufree3((void ***) frac);

  // return newly created field
  
  return newField;

}

/////////////////////////////////////////////////////////
// compute a fractional 2d field - summary for column

MdvxField *EccoStats::_computeFrac2DField(fl32 ***data,
                                          fl32 ***counts,
                                          string fieldName,
                                          string longName,
                                          string units)
                                 
{

  // create header
  
  Mdvx::field_header_t fhdr = _eccoTypeField->getFieldHeader();
  
  fhdr.missing_data_value = _missingFl32;
  fhdr.bad_data_value = _missingFl32;

  fhdr.nx = _nx; // output grid
  fhdr.ny = _ny; // output grid
  fhdr.nz = 1;

  fhdr.grid_dx = _dx;
  fhdr.grid_dy = _dy;
  fhdr.grid_dz = 1.0;

  fhdr.grid_minx = _minx;
  fhdr.grid_miny = _miny;
  fhdr.grid_minz = 0.0;

  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;

  fhdr.dz_constant = 1;
  fhdr.data_dimension = 2;
  
  size_t npts = fhdr.nx * fhdr.ny * fhdr.nz;
  size_t volSize = npts * sizeof(fl32);
  fhdr.volume_size = volSize;

  Mdvx::vlevel_header_t vhdr = _eccoTypeField->getVlevelHeader();
  for (int ii = 0; ii < _nz; ii++) {
    vhdr.type[ii] = Mdvx::VERT_TYPE_SURFACE;
    vhdr.level[ii] = ii;
  }
  
  MdvxField::setFieldName(fieldName, fhdr);
  MdvxField::setFieldNameLong(longName, fhdr);
  MdvxField::setUnits(units, fhdr);

  // compute fraction field

  fl32 **frac = (fl32 **) ucalloc2(_ny, _nx, sizeof(fl32));
  for (int iy = 0; iy < _ny; iy++) {
    for (int ix = 0; ix < _nx; ix++) {
      fl32 sum = 0.0;
      fl32 nn = 0.0;
      for (int iz = 0; iz < _nz; iz++) {
        nn += counts[iz][iy][ix];
        sum += data[iz][iy][ix];
      } // iz
      if (nn != 0) {
        frac[iy][ix] = sum / nn;
      }
    } // ix
  } // iy

  // create field from header and data
  
  MdvxField *newField =
    new MdvxField(fhdr, vhdr, NULL, false, false, false);
  newField->setVolData(*frac, volSize, Mdvx::ENCODING_FLOAT32);
  newField->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_GZIP);

  // free up

  ufree2((void **) frac);

  // return newly created field
  
  return newField;

}

//////////////////////////////////////////////////
// Compute coverage

int EccoStats::_computeCoverage()
{
  
  int iret = 0;

  // loop until end of data
  
  _inputPaths->reset();
  char *nextPath = NULL;
  while ((nextPath = _inputPaths->next()) != NULL) {
    
    // read the Ecco data
    
    if (_readEcco(nextPath)) {
      // failure - skip
      cerr << "ERROR - EccoStats::_computeCoverage()" << endl;
      iret = -1;
      continue;
    }
    
    // read the MRMS data, for the same time as the ecco data
    
    if (_readMrms()) {
      // failure - skip
      cerr << "ERROR - EccoStats::_computeCoverage()" << endl;
      iret = -1;
      continue;
    }

    // add coverage fields to output file
    
    _addCoverageFields();

    // write file
    
    // set times
    
    _outMdvx.setValidTime(_firstTime);
    _outMdvx.setBeginTime(_firstTime);
    _outMdvx.setEndTime(_lastTime);
    _outMdvx.setGenTime(_firstTime);
    
    // write out
    
    if(_outMdvx.writeToDir(_params.output_dir)) {
      cerr << "ERROR - EccoStats::Run" << endl;
      cerr << "  Cannot write data set." << endl;
      cerr << _outMdvx.getErrStr() << endl;
      return -1;
    }

    if (_params.debug) {
      cerr << "Done processing input file: " << _inMdvx.getPathInUse() << endl;
    }

  } // while

  // add the fields to the output file

  _addFieldsToOutput();
  
  // write out
  
  if (_doWrite()) {
    cerr << "ERROR - EccoStats::Run()" << endl;
    return -1;
  }
    
  return iret;

}

//////////////////////////////////////////////////
// Add coverage field to output data set

void EccoStats::_addCoverageFields()
{

  // initialize output DsMdvx object
  // copying master header from input object
  
  _outCov.clear();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _outCov.setDebug(true);
  }
  Mdvx::master_header_t mhdr = _inMrms.getMasterHeader();
  mhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  mhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  mhdr.vlevel_included = 1;
  _outCov.setMasterHeader(mhdr);
  _outCov.setDataSetName("Coverage of MRMS radar grid");
  _outCov.setDataSetInfo("Intended for use with EccoStats");
  _outCov.setDataSetSource("MRMS reflectivity grid");
  _outCov.setMdv2NcfOutput(true, true, true, true);
  
  // load up coverage min and max heights
  
  Mdvx::field_header_t fhdrDbz = _mrmsDbzField->getFieldHeader();
  Mdvx::vlevel_header_t vhdrDbz = _mrmsDbzField->getVlevelHeader();
  
  fl32 **minCovHt = (fl32 **) ucalloc2(fhdrDbz.ny, fhdrDbz.nx, sizeof(fl32));
  fl32 **maxCovHt = (fl32 **) ucalloc2(fhdrDbz.ny, fhdrDbz.nx, sizeof(fl32));
  fl32 **covHtFrac = (fl32 **) ucalloc2(fhdrDbz.ny, fhdrDbz.nx, sizeof(fl32));

  fl32 missHt = -9999.0;
  
  fl32 *dbzVals = (fl32 *) _mrmsDbzField->getVol();

  size_t nPtsLine = fhdrDbz.nx;
  size_t nPtsPlane = fhdrDbz.ny * fhdrDbz.nx;

  for (int iy = 0; iy < fhdrDbz.ny; iy++) {
    for (int ix = 0; ix < fhdrDbz.nx; ix++) {
      
      fl32 minHt = missHt;
      fl32 maxHt = missHt;
      
      for (int iz = 0; iz < fhdrDbz.nz; iz++) {
        
        size_t offset = iz * nPtsPlane + iy * nPtsLine + ix;
        fl32 dbz = dbzVals[offset];
        if (dbz > -100) {
          // not missing, have coverage
          if (minHt == missHt) {
            minHt = vhdrDbz.level[iz];
          }
          maxHt = vhdrDbz.level[iz];
        }

      } // iz
      
      minCovHt[iy][ix] = minHt;
      maxCovHt[iy][ix] = maxHt;

    } // ix
  } // iy

  // make fields and add to output object
  
  _outCov.addField(_make2DField(minCovHt, "CovMinHt",
                                "Min ht of radar coverage", "km"));
  
  _outCov.addField(_make2DField(maxCovHt, "CovMaxHt",
                                "Max ht of radar coverage", "km"));
  
  // load up height fraction
  
  if (_terrainHtField != NULL) {

    for (int iy = 0; iy < fhdrDbz.ny; iy++) {
      for (int ix = 0; ix < fhdrDbz.nx; ix++) {
        covHtFrac[iy][ix] = missHt;
      } // ix
    } // iy
    
    Mdvx::field_header_t fhdrTerrain = _terrainHtField->getFieldHeader();
    if (fhdrTerrain.ny == fhdrDbz.ny && fhdrTerrain.nx == fhdrDbz.nx) {
      fl32 *terrainHt = (fl32 *) _terrainHtField->getVol();
      int offset = 0;
      for (int iy = 0; iy < fhdrDbz.ny; iy++) {
        for (int ix = 0; ix < fhdrDbz.nx; ix++, offset++) {
          if (minCovHt[iy][ix] != missHt && maxCovHt[iy][ix] != missHt) {
            fl32 tht = terrainHt[offset];
            fl32 maxDepth = maxCovHt[iy][ix] - tht;
            fl32 depth = maxCovHt[iy][ix] - minCovHt[iy][ix];
            fl32 fraction = depth / maxDepth;
            if (fraction > 1.0) {
              fraction = 1.0;
            }
            covHtFrac[iy][ix] = fraction;
          }
        } // ix
      } // iy
    } // if (fhdrTerrain.ny == fhdrDbz.ny ...

    _outCov.addField(_make2DField(covHtFrac, "CovHtFraction",
                                  "Coverage fraction in vert column", ""));
    
  } // if (_terrainHtField != NULL)

  // free up arrays
  
  ufree2((void **) minCovHt);
  ufree2((void **) maxCovHt);
  ufree2((void **) covHtFrac);
  
}

/////////////////////////////////////////////////////////
// read the MRMS grid
// Returns 0 on success, -1 on failure.

int EccoStats::_readMrms()
  
{
  
  _inMrms.clear();
  if (_params.debug >= Params::DEBUG_EXTRA) {
    _inMrms.setDebug(true);
  }
  _inMrms.addReadField(_params.mrms_dbz_field_name);
  _inMrms.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _inMrms.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _inMrms.printReadRequest(cerr);
  }

  time_t eccoValidTime = _inMdvx.getValidTime();

  _inMrms.setReadTime(Mdvx::READ_CLOSEST,
                      _params.mrms_mdv_input_dir,
                      180, eccoValidTime);
  
  // read in
  
  if (_inMrms.readVolume()) {
    cerr << "ERROR - EccoStats::_readMrms" << endl;
    cerr << "  Cannot read in data." << endl;
    cerr << _inMrms.getErrStr() << endl;
    return -1;
  }
  if (_params.debug) {
    cerr << "Read MRMS data file: " << _inMrms.getPathInUse() << endl;
  }

  // get dbz field
  
  _mrmsDbzField = _inMrms.getField(_params.mrms_dbz_field_name);
  if (_mrmsDbzField == NULL) {
    cerr << "ERROR - EccoStats::_readMrms" << endl;
    cerr << "  Cannot find MRMS DBZ field: " << _params.mrms_dbz_field_name << endl;
    cerr << "  MRMS path: " << _inMrms.getPathInUse() << endl;
    return -1;
  }

  // check dimensions

  Mdvx::field_header_t fhdrEcco = _eccoTypeField->getFieldHeader();
  Mdvx::field_header_t fhdrMrms = _mrmsDbzField->getFieldHeader();

  if (fhdrEcco.ny != fhdrMrms.ny ||
      fhdrEcco.nx != fhdrMrms.nx) {
    cerr << "ERROR - EccoStats::_readMrms" << endl;
    cerr << "  DBZ nx,ny grid does not match Ecco, file: " << _inMrms.getPathInUse() << endl;
    cerr << "  MRMS nx, ny: " << fhdrMrms.nx << ", " << fhdrMrms.ny << endl;
    cerr << "  Ecco nx, ny: " << fhdrEcco.nx << ", " << fhdrEcco.ny << endl;
    return -1;
  }
  
  return 0;

}

  
