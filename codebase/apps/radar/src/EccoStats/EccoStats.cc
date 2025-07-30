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
#include <titan/TitanFile.hh>
#include "EccoStats.hh"
using namespace std;

const fl32 EccoStats::_missingFl32 = -9999.0;

// Constructor

EccoStats::EccoStats(int argc, char **argv)

{

  isOK = true;
  _eccoPaths = NULL;
  
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
  
  // initialize the ecco data input object

  if (_params.mode == Params::ARCHIVE) {
    _eccoPaths = new DsInputPath(_progName,
                                 _params.debug >= Params::DEBUG_EXTRA,
                                 _params.input_dir,
                                 _args.startTime,
                                 _args.endTime);
  } else {
    _eccoPaths = new DsInputPath(_progName,
                                 _params.debug >= Params::DEBUG_EXTRA,
                                 _args.inputFileList);
  }
  if (_params.set_month_range) {
    _eccoPaths->setValidMonthRange(_params.min_month, _params.max_month);
  }

  // init field pointers
  // these point to memory in MdvxField objects and do not need to be freed
  
  _eccoTypeField = NULL;
  _convectivityField = NULL;
  _terrainHtField = NULL;
  _waterFlagField = NULL;
  _mrmsDbzField = NULL;
  // _covMinHtField = NULL;
  // _covMaxHtField = NULL;
  _covHtFractionField = NULL;

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

  _censorUsingTitan = _params.censor_using_titan;

}

// destructor

EccoStats::~EccoStats()

{

  if (_eccoPaths) {
    delete _eccoPaths;
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

  // loop until end of ecco data
  
  _eccoPaths->reset();
  int fileCount = 0;
  char *nextPath = NULL;
  while ((nextPath = _eccoPaths->next()) != NULL) {

    // do the read

    if (_readEcco(nextPath)) {
      // failure - skip
      cerr << "ERROR - EccoStats::_computeEccoStats()" << endl;
      iret = -1;
      continue;
    }

    if (_eccoTypeField == NULL) {
      cerr << "ERROR - EccoStats::_computeEccoStats()" << endl;
      cerr << "  _eccoTypeField is NULL" << endl;
      return -1;
    }
    
    // if this is the first file, allocate and initialize the arrays
    
    Mdvx::field_header_t fhdr = _eccoTypeField->getFieldHeader();
    if (fileCount == 0) {

      // first file, initialize grid

      _firstEccoTime = _eccoValidTime;

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
        cerr << "  skipping this file: " << _eccoMdvx.getPathInUse() << endl;
        iret = -1;
        continue;
      }

    }
    _lastEccoTime = _eccoValidTime;
    fileCount++;
    
    // update the stats, based on the data in the ecco file
    
    _updateStats();
    
    // clear
    
    if (_params.debug) {
      cerr << "Done processing input file: " << _eccoMdvx.getPathInUse() << endl;
    }

  } // while

  if (fileCount < 1) {
    cerr << "ERROR - EccoStats::_computeEccoStats()" << endl;
    cerr << "  No input files found to process" << endl;
    return -1;
  }

  // add the fields to the output file

  _addFieldsToStats();
  
  // write out
  
  if (_writeStats()) {
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
  
  _stratCount = NULL;
  _mixedCount = NULL;
  _convCount = NULL;

  _stratLowCount = NULL;
  _stratMidCount = NULL;
  _stratHighCount = NULL;

  _convShallowCount = NULL;
  _convMidCount = NULL;
  _convDeepCount = NULL;
  _convElevCount = NULL;

  _stratSumConv = NULL;
  _stratLowSumConv = NULL;
  _stratMidSumConv = NULL;
  _stratHighSumConv = NULL;

  _mixedSumConv = NULL;

  _convSumConv = NULL;
  _convShallowSumConv = NULL;
  _convMidSumConv = NULL;
  _convDeepSumConv = NULL;
  _convElevSumConv = NULL;

  _validCount = NULL;
  _totalCount = NULL;
  
  _terrainHt = NULL;
  _waterFlag = NULL;

  _sumCovHtFrac = NULL;
  _countCov = NULL;

  _lat = NULL;
  _lon = NULL;
  _hourOfDay = NULL;

  _titanMask = NULL;

}

/////////////////////////////////////////////////////////
// allocate the arrays

void EccoStats::_allocArrays()
  
{

  // counts
  
  _stratCount = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  _mixedCount = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  _convCount = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));

  _stratLowCount = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  _stratMidCount = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  _stratHighCount = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));

  _convShallowCount = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  _convMidCount = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  _convDeepCount = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  _convElevCount = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));

  // convectivity
  
  _stratSumConv = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  _stratLowSumConv = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  _stratMidSumConv = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  _stratHighSumConv = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));

  _mixedSumConv = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));

  _convSumConv = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  _convShallowSumConv = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  _convMidSumConv = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  _convDeepSumConv = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  _convElevSumConv = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  
  _validCount = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  _totalCount = (fl32 ***) ucalloc3(_nz, _ny, _nx, sizeof(fl32));
  
  // terrain ht

  _terrainHt = (fl32 **) ucalloc2(_ny, _nx, sizeof(fl32));
  _waterFlag = (fl32 **) ucalloc2(_ny, _nx, sizeof(fl32));

  // coverage
  
  _sumCovHtFrac = (fl32 **) ucalloc2(_ny, _nx, sizeof(fl32));
  _countCov = (fl32 **) ucalloc2(_ny, _nx, sizeof(fl32));

  // lat/lon

  _lon = (double **) ucalloc2(_ny, _nx, sizeof(double));
  _lat = (double **) ucalloc2(_ny, _nx, sizeof(double));

  // local hour of day
  
  _hourOfDay = (int **) ucalloc2(_ny, _nx, sizeof(int));

  // titan mask

  if (_params.censor_using_titan) {
    _titanMask = (int ***) ucalloc3(_nz, _ny, _nx, sizeof(int));
  }
  
}

/////////////////////////////////////////////////////////
// free the arrays

void EccoStats::_freeArrays()
  
{

  ufree3((void ***) _stratCount);
  ufree3((void ***) _mixedCount);
  ufree3((void ***) _convCount);

  ufree3((void ***) _stratLowCount);
  ufree3((void ***) _stratMidCount);
  ufree3((void ***) _stratHighCount);

  ufree3((void ***) _convShallowCount);
  ufree3((void ***) _convMidCount);
  ufree3((void ***) _convDeepCount);
  ufree3((void ***) _convElevCount);
  
  ufree3((void ***) _stratSumConv);
  ufree3((void ***) _stratLowSumConv);
  ufree3((void ***) _stratMidSumConv);
  ufree3((void ***) _stratHighSumConv);

  ufree3((void ***) _mixedSumConv);

  ufree3((void ***) _convSumConv);
  ufree3((void ***) _convShallowSumConv);
  ufree3((void ***) _convMidSumConv);
  ufree3((void ***) _convDeepSumConv);
  ufree3((void ***) _convElevSumConv);

  ufree3((void ***) _validCount);
  ufree3((void ***) _totalCount);

  ufree2((void **) _terrainHt);
  ufree2((void **) _waterFlag);

  ufree2((void **) _sumCovHtFrac);
  ufree2((void **) _countCov);

  ufree2((void **) _lat);
  ufree2((void **) _lon);

  ufree2((void **) _hourOfDay);
  
  if (_params.censor_using_titan) {
    ufree3((void ***) _titanMask);
  }

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
  
  _initStatsFile();
    
}

/////////////////////////////////////////////////////////
// load the terrain arrays
// Returns 0 on success, -1 on failure.

void EccoStats::_loadTerrain()
  
{

  // load terrain ht if field is available
  
  if (_terrainHtField != NULL) {
  
    // loop through 2D grid space
    
    fl32 missHt = _terrainHtField->getFieldHeader().missing_data_value;
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
    
    fl32 missFlag = _waterFlagField->getFieldHeader().missing_data_value;
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
// process the file in _eccoMdvx
// Returns 0 on success, -1 on failure.

void EccoStats::_updateStats()
  
{

  // compute hour of day array
  
  DateTime fileTime(_eccoValidTime);
  
  for (int iy = 0; iy < _ny; iy++) {
    for (int ix = 0; ix < _nx; ix++) {
      double lon = _lon[iy][ix];
      double lonSecs = lon * 240.0;
      DateTime lonTime = fileTime + lonSecs;
      int lonHour = lonTime.getHour();
      _hourOfDay[iy][ix] = lonHour;
    } // ix
  } // iy

  // intialize array pointers
  
  fl32 eccoMiss = _eccoTypeField->getFieldHeader().missing_data_value;
  fl32 *echoType2D = (fl32 *) _eccoTypeField->getVol();
  fl32 *convectivity2D = (fl32 *) _convectivityField->getVol();

  fl32 *covHtFrac2D = NULL;
  fl32 covHtFracMiss = -9999;
  if (_covHtFractionField != NULL) {
    covHtFracMiss = _covHtFractionField->getFieldHeader().missing_data_value;
    covHtFrac2D = (fl32 *) _covHtFractionField->getVol();
  }
  
  // loop through 2D grid space
  
  size_t offset = 0;
  for (int jy = 0; jy < _inNy; jy++) {
    for (int jx = 0; jx < _inNx; jx++, offset++) {
      
      int iy = jy / _agNy;
      int ix = jx / _agNx;
      
      int hour = _hourOfDay[iy][ix];
      fl32 echoTypeFl32 = echoType2D[offset];

      // check for coverage ht fraction if active
      
      if (covHtFrac2D != NULL) {
        fl32 covHtFrac = covHtFrac2D[offset];
        if (covHtFrac != covHtFracMiss) {
          // ht fraction
          _sumCovHtFrac[iy][ix] += covHtFrac;
          _countCov[iy][ix]++;
          if (covHtFrac < _params.radar_coverage_min_ht_fraction) {
            // censor because ht fraction too low
            continue;
          }
        } else {
          // censor because ht fraction missing
          continue;
        }
      } // if (covHtFrac2D != NULL) 
      
      if (echoTypeFl32 != eccoMiss) {
        
        int echoType = (int) floor(echoTypeFl32 + 0.5);
        fl32 convectivity = convectivity2D[offset];
        switch ((ConvStratFinder::category_t) echoType) {
          case ConvStratFinder::category_t::CATEGORY_STRATIFORM_LOW: {
            _stratCount[hour][iy][ix]++;
            _stratSumConv[hour][iy][ix] += convectivity;
            _stratLowCount[hour][iy][ix]++;
            _stratLowSumConv[hour][iy][ix] += convectivity;
            break;
          }
          case ConvStratFinder::category_t::CATEGORY_STRATIFORM_MID: {
            _stratCount[hour][iy][ix]++;
            _stratSumConv[hour][iy][ix] += convectivity;
            _stratMidCount[hour][iy][ix]++;
            _stratMidSumConv[hour][iy][ix] += convectivity;
            break;
          }
          case ConvStratFinder::category_t::CATEGORY_STRATIFORM_HIGH: {
            _stratCount[hour][iy][ix]++;
            _stratSumConv[hour][iy][ix] += convectivity;
            _stratHighCount[hour][iy][ix]++;
            _stratHighSumConv[hour][iy][ix] += convectivity;
            break;
          }
          case ConvStratFinder::category_t::CATEGORY_MIXED: {
            _mixedCount[hour][iy][ix]++;
            _mixedSumConv[hour][iy][ix] += convectivity;
            break;
          }
          case ConvStratFinder::category_t::CATEGORY_CONVECTIVE_ELEVATED: {
            _convCount[hour][iy][ix]++;
            _convSumConv[hour][iy][ix] += convectivity;
            _convElevCount[hour][iy][ix]++;
            _convElevSumConv[hour][iy][ix] += convectivity;
            break;
          }
          case ConvStratFinder::category_t::CATEGORY_CONVECTIVE_SHALLOW: {
            _convCount[hour][iy][ix]++;
            _convSumConv[hour][iy][ix] += convectivity;
            _convShallowCount[hour][iy][ix]++;
            _convShallowSumConv[hour][iy][ix] += convectivity;
            break;
          }
          case ConvStratFinder::category_t::CATEGORY_CONVECTIVE_MID: {
            _convCount[hour][iy][ix]++;
            _convSumConv[hour][iy][ix] += convectivity;
            _convMidCount[hour][iy][ix]++;
            _convMidSumConv[hour][iy][ix] += convectivity;
            break;
          }
          case ConvStratFinder::category_t::CATEGORY_CONVECTIVE_DEEP: {
            _convCount[hour][iy][ix]++;
            _convSumConv[hour][iy][ix] += convectivity;
            _convDeepCount[hour][iy][ix]++;
            _convDeepSumConv[hour][iy][ix] += convectivity;
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
  
  _eccoMdvx.clear();
  if (_params.debug >= Params::DEBUG_EXTRA) {
    _eccoMdvx.setDebug(true);
  }
  _eccoMdvx.addReadField(_params.ecco_type_comp_field_name);
  _eccoMdvx.addReadField(_params.convectivity_comp_field_name);
  if (strlen(_params.terrain_height_field_name) > 0) {
    _eccoMdvx.addReadField(_params.terrain_height_field_name);
  }
  if (strlen(_params.water_flag_field_name) > 0) {
    _eccoMdvx.addReadField(_params.water_flag_field_name);
  }
  _eccoMdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _eccoMdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Setting read for ecco data" << endl;
    _eccoMdvx.printReadRequest(cerr);
  }
  _eccoMdvx.setReadPath(path);
  
  // read in
  
  if (_eccoMdvx.readVolume()) {
    cerr << "ERROR - EccoStats::_readEcco" << endl;
    cerr << "  Cannot read in data." << endl;
    cerr << _eccoMdvx.getErrStr() << endl;
    return -1;
  }
  _eccoValidTime = _eccoMdvx.getValidTime();

  _eccoTypeField = _eccoMdvx.getField(_params.ecco_type_comp_field_name);
  _convectivityField = _eccoMdvx.getField(_params.convectivity_comp_field_name);
  if (strlen(_params.terrain_height_field_name) > 0) {
    _terrainHtField = _eccoMdvx.getField(_params.terrain_height_field_name);
  }
  if (strlen(_params.water_flag_field_name) > 0) {
    _waterFlagField = _eccoMdvx.getField(_params.water_flag_field_name);
  }
  if (_eccoTypeField == NULL) {
    cerr << "ERROR - readEcco(), file: " << _eccoMdvx.getPathInUse() << endl;
    cerr << "  Cannot find field: " << _params.ecco_type_comp_field_name << endl;
    return -1;
  }
  if (_convectivityField == NULL) {
    cerr << "ERROR - readEcco(), file: " << _eccoMdvx.getPathInUse() << endl;
    cerr << "  Cannot find field: " << _params.convectivity_comp_field_name << endl;
    return -1;
  }

  // set projection, and lat/lon arrays
  
  Mdvx::field_header_t fhdr = _eccoTypeField->getFieldHeader();
  _proj.init(fhdr);
  
  if (_params.debug) {
    cerr << "Success - read in file: " << _eccoMdvx.getPathInUse() << endl;
  }
  
  // read the coverage data if requested

  if (_params.censor_based_on_radar_coverage) {
    if (_readCoverage()) {
      // failure - skip
      cerr << "ERROR - EccoStats::_readEcco()" << endl;
      return -1;
    }
  }

  // read the titan data if requested

  _censorUsingTitan = _params.censor_using_titan;
  if (_params.censor_using_titan) {
    if (_readTitan()) {
      // failure - skip
      cerr << "WARNING - no Titan masking" << endl;
      _censorUsingTitan = false;
    }
  }

  return 0;

}

/////////////////////////////////////////////////////////
// prepare the output file

void EccoStats::_initStatsFile()
  
{
  
  // create output DsMdvx object
  // copying master header from input object
  
  _statsMdvx.clear();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _statsMdvx.setDebug(true);
  }
  Mdvx::master_header_t mhdr = _eccoMdvx.getMasterHeader();
  mhdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  mhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  mhdr.vlevel_included = 1;
  _statsMdvx.setMasterHeader(mhdr);
  _statsMdvx.setDataSetInfo(_params.stats_data_set_info);
  _statsMdvx.setDataSetSource(_params.stats_data_set_source);
  char name[128];
  if (_params.min_month == _params.max_month) {
    snprintf(name, 128, "EccoStats for month %d\n", _params.min_month);
  } else {
    snprintf(name, 128, "EccoStats for months %d to %d\n",
             _params.min_month, _params.max_month);
  }
  _statsMdvx.setDataSetName(name);
  _statsMdvx.setMdv2NcfOutput(true, true, true, true);
  
}

/////////////////////////////////////////////////////////
// add fields to the output object

void EccoStats::_addFieldsToStats()
  
{

  // clear
  
  _statsMdvx.clearFields();
  
  // add hourly count fields by echo type
  
  _statsMdvx.addField(_make3DField(_stratCount,
                                   "StratCountHourly",
                                   "count_for_stratiform_hourly",
                                   "count", 0.0));
  
  _statsMdvx.addField(_make3DField(_stratLowCount,
                                   "StratLowCountHourly",
                                   "count_for_stratiform_low_hourly",
                                   "count", 0.0));
  
  _statsMdvx.addField(_make3DField(_stratMidCount,
                                   "StratMidCountHourly",
                                   "count_for_stratiform_mid_hourly",
                                   "count", 0.0));
  
  _statsMdvx.addField(_make3DField(_stratHighCount,
                                   "StratHighCountHourly",
                                   "count_for_stratiform_high_hourly",
                                   "count", 0.0));
  
  _statsMdvx.addField(_make3DField(_mixedCount,
                                   "MixedCountHourly",
                                   "count_for_mixed_hourly",
                                   "count", 0.0));
  
  _statsMdvx.addField(_make3DField(_convCount,
                                   "ConvCountHourly",
                                   "count_for_convective_hourly",
                                   "count", 0.0));
  
  _statsMdvx.addField(_make3DField(_convShallowCount,
                                   "ConvShallowCountHourly",
                                   "count_for_convective_shallow_hourly",
                                   "count", 0.0));
  
  _statsMdvx.addField(_make3DField(_convMidCount,
                                   "ConvMidCountHourly",
                                   "count_for_convective_mid_hourly",
                                   "count", 0.0));
  
  _statsMdvx.addField(_make3DField(_convDeepCount,
                                   "ConvDeepCountHourly",
                                   "count_for_convective_deep_hourly",
                                   "count", 0.0));
  
  _statsMdvx.addField(_make3DField(_convElevCount,
                                   "ConvElevCountHourly",
                                   "count_for_convective_elevated_hourly",
                                   "count", 0.0));
  
  // add hourly sum convectivity fields by echo type
  
  _statsMdvx.addField(_make3DField(_stratSumConv,
                                   "StratSumConvHourly",
                                   "sum_convectivity_for_stratiform_hourly",
                                   "", 0.0));
  
  _statsMdvx.addField(_make3DField(_stratLowSumConv,
                                   "StratLowSumConvHourly",
                                   "sum_convectivity_for_stratiform_low_hourly",
                                   "", 0.0));
  
  _statsMdvx.addField(_make3DField(_stratMidSumConv,
                                   "StratMidSumConvHourly",
                                   "sum_convectivity_for_stratiform_mid_hourly",
                                   "", 0.0));
  
  _statsMdvx.addField(_make3DField(_stratHighSumConv,
                                   "StratHighSumConvHourly",
                                   "sum_convectivity_for_stratiform_high_hourly",
                                   "", 0.0));
  
  _statsMdvx.addField(_make3DField(_mixedSumConv,
                                   "MixedSumConvHourly",
                                   "sum_convectivity_for_mixed_hourly",
                                   "", 0.0));
  
  _statsMdvx.addField(_make3DField(_convSumConv,
                                   "ConvSumConvHourly",
                                   "sum_convectivity_for_convective_hourly",
                                   "", 0.0));
  
  _statsMdvx.addField(_make3DField(_convShallowSumConv,
                                   "ConvShallowSumConvHourly",
                                   "sum_convectivity_for_convective_shallow_hourly",
                                   "", 0.0));
  
  _statsMdvx.addField(_make3DField(_convMidSumConv,
                                   "ConvMidSumConvHourly",
                                   "sum_convectivity_for_convective_mid_hourly",
                                   "", 0.0));
  
  _statsMdvx.addField(_make3DField(_convDeepSumConv,
                                   "ConvDeepSumConvHourly",
                                   "sum_convectivity_for_convective_deep_hourly",
                                   "", 0.0));
  
  _statsMdvx.addField(_make3DField(_convElevSumConv,
                                   "ConvElevSumConvHourly",
                                   "sum_convectivity_for_convective_elevated_hourly",
                                   "", 0.0));
  
  // add hourly fractions
  
  _statsMdvx.addField(_computeMean3DField(_stratCount,
                                          _validCount,
                                          "StratFracHourly",
                                          "fraction_for_stratiform_hourly",
                                          "", 0.0));
  
  _statsMdvx.addField(_computeMean3DField(_stratLowCount,
                                          _validCount,
                                          "StratLowFracHourly",
                                          "fraction_for_stratiform_low_hourly",
                                          "", 0.0));
  
  _statsMdvx.addField(_computeMean3DField(_stratMidCount,
                                          _validCount,
                                          "StratMidFracHourly",
                                          "fraction_for_stratiform_mid_hourly",
                                          "count", 0.0));
  
  _statsMdvx.addField(_computeMean3DField(_stratHighCount,
                                          _validCount,
                                          "StratHighFracHourly",
                                          "fraction_for_stratiform_high_hourly",
                                          "count", 0.0));
  
  _statsMdvx.addField(_computeMean3DField(_mixedCount,
                                          _validCount,
                                          "MixedFracHourly",
                                          "fraction_for_mixed_hourly",
                                          "count", 0.0));
  
  _statsMdvx.addField(_computeMean3DField(_convCount,
                                          _validCount,
                                          "ConvFracHourly",
                                          "fraction_for_convective_hourly",
                                          "count", 0.0));
  
  _statsMdvx.addField(_computeMean3DField(_convShallowCount,
                                          _validCount,
                                          "ConvShallowFracHourly",
                                          "fraction_for_convective_shallow_hourly",
                                          "count", 0.0));
  
  _statsMdvx.addField(_computeMean3DField(_convMidCount,
                                          _validCount,
                                          "ConvMidFracHourly",
                                          "fraction_for_convective_mid_hourly",
                                          "count", 0.0));
  
  _statsMdvx.addField(_computeMean3DField(_convDeepCount,
                                          _validCount,
                                          "ConvDeepFracHourly",
                                          "fraction_for_convective_deep_hourly",
                                          "count", 0.0));
  
  _statsMdvx.addField(_computeMean3DField(_convElevCount,
                                          _validCount,
                                          "ConvElevFracHourly",
                                          "fraction_for_convective_elevated_hourly",
                                          "count", 0.0));

  // add mean convectivity hourly
  
  _statsMdvx.addField(_computeMean3DField(_stratSumConv,
                                          _validCount,
                                          "StratMeanConvHourly",
                                          "mean_convectivity_for_stratiform_hourly",
                                          "", 0.0));
  
  _statsMdvx.addField(_computeMean3DField(_stratLowSumConv,
                                          _validCount,
                                          "StratLowMeanConvHourly",
                                          "mean_convectivity_for_stratiform_low_hourly",
                                          "", 0.0));
  
  _statsMdvx.addField(_computeMean3DField(_stratMidSumConv,
                                          _validCount,
                                          "StratMidMeanConvHourly",
                                          "mean_convectivity_for_stratiform_mid_hourly",
                                          "", 0.0));
  
  _statsMdvx.addField(_computeMean3DField(_stratHighSumConv,
                                          _validCount,
                                          "StratHighMeanConvHourly",
                                          "mean_convectivity_for_stratiform_high_hourly",
                                          "", 0.0));
  
  _statsMdvx.addField(_computeMean3DField(_mixedSumConv,
                                          _validCount,
                                          "MixedMeanConvHourly",
                                          "mean_convectivity_for_mixed_hourly",
                                          "", 0.0));
  
  _statsMdvx.addField(_computeMean3DField(_convSumConv,
                                          _validCount,
                                          "ConvMeanConvHourly",
                                          "mean_convectivity_for_convective_shallow_hourly",
                                          "", 0.0));
  
  _statsMdvx.addField(_computeMean3DField(_convShallowSumConv,
                                          _validCount,
                                          "ConvShallowMeanConvHourly",
                                          "mean_convectivity_for_convective_shallow_hourly",
                                          "", 0.0));
  
  _statsMdvx.addField(_computeMean3DField(_convMidSumConv,
                                          _validCount,
                                          "ConvMidMeanConvHourly",
                                          "mean_convectivity_for_convective_mid_hourly",
                                          "", 0.0));
  
  _statsMdvx.addField(_computeMean3DField(_convDeepSumConv,
                                          _validCount,
                                          "ConvDeepMeanConvHourly",
                                          "mean_convectivity_for_convective_deep_hourly",
                                          "", 0.0));
  
  _statsMdvx.addField(_computeMean3DField(_convElevSumConv,
                                          _validCount,
                                          "ConvElevMeanConvHourly",
                                          "mean_convectivity_for_convective_elevated_hourly",
                                          "", 0.0));
  
  // add fraction fields all hours
  
  _statsMdvx.addField(_computeMean2DField(_stratCount,
                                          _validCount,
                                          "StratFrac",
                                          "fraction_for_stratiform_all_hours",
                                          "", 0.0));
  
  _statsMdvx.addField(_computeMean2DField(_stratLowCount,
                                          _validCount,
                                          "StratLowFrac",
                                          "fraction_for_stratiform_low_all_hours",
                                          "", 0.0));
  
  _statsMdvx.addField(_computeMean2DField(_stratMidCount,
                                          _validCount,
                                          "StratMidFrac",
                                          "fraction_for_stratiform_mid_all_hours",
                                          "count", 0.0));
  
  _statsMdvx.addField(_computeMean2DField(_stratHighCount,
                                          _validCount,
                                          "StratHighFrac",
                                          "fraction_for_stratiform_high_all_hours",
                                          "count", 0.0));
  
  _statsMdvx.addField(_computeMean2DField(_mixedCount,
                                          _validCount,
                                          "MixedFrac",
                                          "fraction_for_mixed_all_hours",
                                          "count", 0.0));
  
  _statsMdvx.addField(_computeMean2DField(_convCount,
                                          _validCount,
                                          "ConvFrac",
                                          "fraction_for_convective_all_hours",
                                          "count", 0.0));
  
  _statsMdvx.addField(_computeMean2DField(_convShallowCount,
                                          _validCount,
                                          "ConvShallowFrac",
                                          "fraction_for_convective_shallow_all_hours",
                                          "count", 0.0));
  
  _statsMdvx.addField(_computeMean2DField(_convMidCount,
                                          _validCount,
                                          "ConvMidFrac",
                                          "fraction_for_convective_mid_all_hours",
                                          "count", 0.0));
  
  _statsMdvx.addField(_computeMean2DField(_convDeepCount,
                                          _validCount,
                                          "ConvDeepFrac",
                                          "fraction_for_convective_deep_all_hours",
                                          "count", 0.0));
  
  _statsMdvx.addField(_computeMean2DField(_convElevCount,
                                          _validCount,
                                          "ConvElevFrac",
                                          "fraction_for_convective_elevated_all_hours",
                                          "count", 0.0));

  // add mean convectivity fields, all hours
  
  _statsMdvx.addField(_computeMean2DField(_stratSumConv,
                                          _validCount,
                                          "StratMeanConv",
                                          "mean_convectivity_for_stratiform_all_hours",
                                          "", 0.0));
  
  _statsMdvx.addField(_computeMean2DField(_stratLowSumConv,
                                          _validCount,
                                          "StratLowMeanConv",
                                          "mean_convectivity_for_stratiform_low_all_hours",
                                          "", 0.0));
  
  _statsMdvx.addField(_computeMean2DField(_stratMidSumConv,
                                          _validCount,
                                          "StratMidMeanConv",
                                          "mean_convectivity_for_stratiform_mid_all_hours",
                                          "", 0.0));
  
  _statsMdvx.addField(_computeMean2DField(_stratHighSumConv,
                                          _validCount,
                                          "StratHighMeanConv",
                                          "mean_convectivity_for_stratiform_high_all_hours",
                                          "", 0.0));
  
  _statsMdvx.addField(_computeMean2DField(_mixedSumConv,
                                          _validCount,
                                          "MixedMeanConv",
                                          "mean_convectivity_for_mixed_all_hours",
                                          "", 0.0));
  
  _statsMdvx.addField(_computeMean2DField(_convSumConv,
                                          _validCount,
                                          "ConvMeanConv",
                                          "mean_convectivity_for_convective_all_hours",
                                          "", 0.0));
  
  _statsMdvx.addField(_computeMean2DField(_convShallowSumConv,
                                          _validCount,
                                          "ConvShallowMeanConv",
                                          "mean_convectivity_for_convective_shallow_all_hours",
                                          "", 0.0));
  
  _statsMdvx.addField(_computeMean2DField(_convMidSumConv,
                                          _validCount,
                                          "ConvMidMeanConv",
                                          "mean_convectivity_for_convective_mid_all_hours",
                                          "", 0.0));
  
  _statsMdvx.addField(_computeMean2DField(_convDeepSumConv,
                                          _validCount,
                                          "ConvDeepMeanConv",
                                          "mean_convectivity_for_convective_deep_all_hours",
                                          "", 0.0));
  
  _statsMdvx.addField(_computeMean2DField(_convElevSumConv,
                                          _validCount,
                                          "ConvElevMeanConv",
                                          "mean_convectivity_for_convective_elevated_all_hours",
                                          "", 0.0));
  
  // add 3d summary count fields
  
  _statsMdvx.addField(_make3DField(_validCount,
                                   "ValidCountHourly",
                                   "count_for_valid_obs",
                                   "count", 0.0));
  
  _statsMdvx.addField(_make3DField(_totalCount,
                                   "TotalCountHourly",
                                   "count_for_all_obs",
                                   "count", 0.0));
  
  // add total counts for all hours
  
  _statsMdvx.addField(_sumHourlyField(_validCount,
                                      "ValidCount",
                                      "count_for_valid_obs_all_hours",
                                      "count", 0.0));
  
  _statsMdvx.addField(_sumHourlyField(_totalCount,
                                      "TotalCount",
                                      "count_for_total_obs_all_hours",
                                      "count", 0.0));
  
  // add 2D fields for terrain height and water flag, if available

  if (_terrainHtField) {
    _statsMdvx.addField(_make2DField(_terrainHt,
                                     _terrainHtField->getFieldName(),
                                     _terrainHtField->getFieldNameLong(),
                                     _terrainHtField->getUnits(),
                                     _terrainHtField->getFieldHeader().missing_data_value));
  }
  
  if (_waterFlagField) {
    _statsMdvx.addField(_make2DField(_waterFlag,
                                     _waterFlagField->getFieldName(),
                                     _waterFlagField->getFieldNameLong(),
                                     _waterFlagField->getUnits(),
                                     _waterFlagField->getFieldHeader().missing_data_value));
  }

  // add 2D fields for coverage

  if (_covHtFractionField != NULL) {
    _statsMdvx.addField(_computeCov2DField(_sumCovHtFrac,
                                           _countCov,
                                           _params.coverage_ht_fraction_field_name,
                                           "ht_fraction_of_radar_coverage_in_column",
                                           "",
                                           _covHtFractionField->getFieldHeader().missing_data_value));
  }
  
}

/////////////////////////////////////////////////////////
// write stats to file
// Returns 0 on success, -1 on failure.

int EccoStats::_writeStats()
  
{

  // set times
  
  _statsMdvx.setValidTime(_firstEccoTime);
  _statsMdvx.setBeginTime(_firstEccoTime);
  _statsMdvx.setEndTime(_lastEccoTime);
  _statsMdvx.setGenTime(_firstEccoTime);

  // write out
  
  if(_statsMdvx.writeToDir(_params.stats_dir)) {
    cerr << "ERROR - EccoStats::Run" << endl;
    cerr << "  Cannot write data set to dir: " << _params.stats_dir << endl;
    cerr << _statsMdvx.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Wrote stats file: " << _statsMdvx.getPathInUse() << endl;
  }

  if (_params.write_hour_of_day_stats) {
    for (int ihour = 0; ihour < 24; ihour++) {
      _writeHourlyStats(ihour);
    }
  }
  
  return 0;

}

/////////////////////////////////////////////////////////
// write stats for specified hour to file
// Returns 0 on success, -1 on failure.

int EccoStats::_writeHourlyStats(int hour)
  
{

  // initialize hourly file
  
  DsMdvx hourlyMdvx;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    hourlyMdvx.setDebug(true);
  }
  Mdvx::master_header_t mhdr = _statsMdvx.getMasterHeader();
  _statsMdvx.setMasterHeader(mhdr);
  char name[128];
  if (_params.min_month == _params.max_month) {
    snprintf(name, 128, "EccoStats for month %d, hour of day %d\n", _params.min_month, hour);
  } else {
    snprintf(name, 128, "EccoStats for months %d to %d, hour of day %d\n",
             _params.min_month, _params.max_month, hour);
  }
  hourlyMdvx.setDataSetName(name);
  hourlyMdvx.setMdv2NcfOutput(true, true, true, true);

  DateTime htime(_statsMdvx.getValidTime());
  htime.setHour(hour);
  hourlyMdvx.setValidTime(htime.utime());

  // add fields
  
  hourlyMdvx.addField(_makeHourlyField(hour,
                                       _validCount,
                                       "ValidCountHourly",
                                       "count_for_valid_obs_hourly",
                                       "count", 0.0));
  
  hourlyMdvx.addField(_makeHourlyField(hour,
                                       _totalCount,
                                       "TotalCountHourly",
                                       "count_for_all_obs_hourly",
                                       "count", 0.0));
  
  hourlyMdvx.addField(_computeHourlyMeanField(hour,
                                              _stratCount,
                                              _validCount,
                                              "StratFracHourly",
                                              "fraction_for_stratiform_all_hourly",
                                              "", 0.0));
  
  hourlyMdvx.addField(_computeHourlyMeanField(hour,
                                              _stratLowCount,
                                              _validCount,
                                              "StratLowFracHourly",
                                              "fraction_for_stratiform_low_hourly",
                                              "", 0.0));
  
  hourlyMdvx.addField(_computeHourlyMeanField(hour,
                                              _stratMidCount,
                                              _validCount,
                                              "StratMidFracHourly",
                                              "fraction_for_stratiform_mid_hourly",
                                              "count", 0.0));
  
  hourlyMdvx.addField(_computeHourlyMeanField(hour,
                                              _stratHighCount,
                                              _validCount,
                                              "StratHighFracHourly",
                                              "fraction_for_stratiform_high_hourly",
                                              "count", 0.0));
  
  hourlyMdvx.addField(_computeHourlyMeanField(hour,
                                              _mixedCount,
                                              _validCount,
                                              "MixedFracHourly",
                                              "fraction_for_mixed_hourly",
                                              "count", 0.0));
  
  hourlyMdvx.addField(_computeHourlyMeanField(hour,
                                              _convCount,
                                              _validCount,
                                              "ConvFracHourly",
                                              "fraction_for_convective_all_hourly",
                                              "count", 0.0));
  
  hourlyMdvx.addField(_computeHourlyMeanField(hour,
                                              _convShallowCount,
                                              _validCount,
                                              "ConvShallowFracHourly",
                                              "fraction_for_convective_shallow_hourly",
                                              "count", 0.0));
  
  hourlyMdvx.addField(_computeHourlyMeanField(hour,
                                              _convMidCount,
                                              _validCount,
                                              "ConvMidFracHourly",
                                              "fraction_for_convective_mid_hourly",
                                              "count", 0.0));
  
  hourlyMdvx.addField(_computeHourlyMeanField(hour,
                                              _convDeepCount,
                                              _validCount,
                                              "ConvDeepFracHourly",
                                              "fraction_for_convective_deep_hourly",
                                              "count", 0.0));
  
  hourlyMdvx.addField(_computeHourlyMeanField(hour,
                                              _convElevCount,
                                              _validCount,
                                              "ConvElevFracHourly",
                                              "fraction_for_convective_elevated_hourly",
                                              "count", 0.0));
  
  hourlyMdvx.addField(_computeHourlyMeanField(hour,
                                              _stratSumConv,
                                              _validCount,
                                              "StratMeanConvHourly",
                                              "mean_convectivity_for_stratiform_hourly",
                                              "", 0.0));
  
  hourlyMdvx.addField(_computeHourlyMeanField(hour,
                                              _stratLowSumConv,
                                              _validCount,
                                              "StratLowMeanConvHourly",
                                              "mean_convectivity_for_stratiform_low_hourly",
                                              "", 0.0));
  
  hourlyMdvx.addField(_computeHourlyMeanField(hour,
                                              _stratMidSumConv,
                                              _validCount,
                                              "StratMidMeanConvHourly",
                                              "mean_convectivity_for_stratiform_mid_hourly",
                                              "", 0.0));
  
  hourlyMdvx.addField(_computeHourlyMeanField(hour,
                                              _stratHighSumConv,
                                              _validCount,
                                              "StratHighMeanConvHourly",
                                              "mean_convectivity_for_stratiform_high_hourly",
                                              "", 0.0));
  
  hourlyMdvx.addField(_computeHourlyMeanField(hour,
                                              _mixedSumConv,
                                              _validCount,
                                              "MixedMeanConvHourly",
                                              "mean_convectivity_for_mixed_hourly",
                                              "", 0.0));
  
  hourlyMdvx.addField(_computeHourlyMeanField(hour,
                                              _convShallowSumConv,
                                              _validCount,
                                              "ConvShallowMeanConvHourly",
                                              "mean_convectivity_for_convective_shallow_hourly",
                                              "", 0.0));
  
  hourlyMdvx.addField(_computeHourlyMeanField(hour,
                                              _convSumConv,
                                              _validCount,
                                              "ConvMeanConvHourly",
                                              "mean_convectivity_for_convective_hourly",
                                              "", 0.0));
  
  hourlyMdvx.addField(_computeHourlyMeanField(hour,
                                              _convMidSumConv,
                                              _validCount,
                                              "ConvMidMeanConvHourly",
                                              "mean_convectivity_for_convective_mid_hourly",
                                              "", 0.0));
  
  hourlyMdvx.addField(_computeHourlyMeanField(hour,
                                              _convDeepSumConv,
                                              _validCount,
                                              "ConvDeepMeanConvHourly",
                                              "mean_convectivity_for_convective_deep_hourly",
                                              "", 0.0));
  
  hourlyMdvx.addField(_computeHourlyMeanField(hour,
                                              _convElevSumConv,
                                              _validCount,
                                              "ConvElevMeanConvHourly",
                                              "mean_convectivity_for_convective_elevated_hourly",
                                              "", 0.0));
  
  
  // write out
  
  if(hourlyMdvx.writeToDir(_params.hour_of_day_stats_dir)) {
    cerr << "ERROR - EccoStats::Run" << endl;
    cerr << "  Cannot write hour data set to dir: " << _params.hour_of_day_stats_dir << endl;
    cerr << hourlyMdvx.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Wrote hourly stats file: " << hourlyMdvx.getPathInUse() << endl;
  }
  
  return 0;
  
}


//////////////////////////////////////////////////
// Compute coverage

int EccoStats::_computeCoverage()
{
  
  int iret = 0;

  // loop until end of ecco data
  
  _eccoPaths->reset();
  char *nextPath = NULL;
  while ((nextPath = _eccoPaths->next()) != NULL) {
    
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
    
    if(_covMdvx.writeToDir(_params.coverage_dir)) {
      cerr << "ERROR - EccoStats::Run" << endl;
      cerr << "  Cannot write data set." << endl;
      cerr << _covMdvx.getErrStr() << endl;
      return -1;
    }
    
    if (_params.debug) {
      cerr << "Wrote file: " << _covMdvx.getPathInUse() << endl;
    }

  } // while

  return iret;

}

//////////////////////////////////////////////////
// Add coverage field to output data set

void EccoStats::_addCoverageFields()
{

  // initialize output DsMdvx object
  // copying master header from input object
  
  _covMdvx.clear();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _covMdvx.setDebug(true);
  }
  Mdvx::master_header_t mhdr = _mrmsMdvx.getMasterHeader();
  mhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  mhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  mhdr.vlevel_included = 1;
  _covMdvx.setMasterHeader(mhdr);
  _covMdvx.setDataSetName("Coverage of MRMS radar grid");
  _covMdvx.setDataSetInfo("Intended for use with EccoStats");
  _covMdvx.setDataSetSource("MRMS reflectivity grid");
  _covMdvx.setMdv2NcfOutput(true, true, true, true);
  
  // load up coverage min and max heights
  
  Mdvx::field_header_t fhdrDbz = _mrmsDbzField->getFieldHeader();
  Mdvx::vlevel_header_t vhdrDbz = _mrmsDbzField->getVlevelHeader();
  
  fl32 **minCovHt = (fl32 **) ucalloc2(fhdrDbz.ny, fhdrDbz.nx, sizeof(fl32));
  fl32 **maxCovHt = (fl32 **) ucalloc2(fhdrDbz.ny, fhdrDbz.nx, sizeof(fl32));
  fl32 **covHtFrac = (fl32 **) ucalloc2(fhdrDbz.ny, fhdrDbz.nx, sizeof(fl32));

  fl32 missHt = _missingFl32;
  
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
  
  _covMdvx.addField(_makeMrms2DField(minCovHt, _params.coverage_min_ht_field_name,
                                     "Min ht of radar coverage", "km", _missingFl32));
  
  _covMdvx.addField(_makeMrms2DField(maxCovHt, _params.coverage_max_ht_field_name,
                                     "Max ht of radar coverage", "km", _missingFl32));
  
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
            fl32 tht = terrainHt[offset] / 1000.0; // km
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

    _covMdvx.addField(_makeMrms2DField(covHtFrac, _params.coverage_ht_fraction_field_name,
                                       "Coverage fraction in vert column", "", _missingFl32));
    
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

  _mrmsDbzField = NULL;
  
  _mrmsMdvx.clear();
  if (_params.debug >= Params::DEBUG_EXTRA) {
    _mrmsMdvx.setDebug(true);
  }
  _mrmsMdvx.addReadField(_params.mrms_dbz_field_name);
  _mrmsMdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _mrmsMdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Setting read for mrms data" << endl;
    _mrmsMdvx.printReadRequest(cerr);
  }

  time_t eccoValidTime = _eccoMdvx.getValidTime();

  _mrmsMdvx.setReadTime(Mdvx::READ_CLOSEST,
                        _params.mrms_dbz_mdv_dir,
                        180, eccoValidTime);
  
  // read in
  
  if (_mrmsMdvx.readVolume()) {
    cerr << "ERROR - EccoStats::_readMrms" << endl;
    cerr << "  Cannot read in data." << endl;
    cerr << _mrmsMdvx.getErrStr() << endl;
    return -1;
  }
  if (_params.debug) {
    cerr << "Read MRMS data file: " << _mrmsMdvx.getPathInUse() << endl;
  }

  // get dbz field
  
  _mrmsDbzField = _mrmsMdvx.getField(_params.mrms_dbz_field_name);
  if (_mrmsDbzField == NULL) {
    cerr << "ERROR - EccoStats::_readMrms" << endl;
    cerr << "  Cannot find MRMS DBZ field: " << _params.mrms_dbz_field_name << endl;
    cerr << "  MRMS path: " << _mrmsMdvx.getPathInUse() << endl;
    return -1;
  }

  // check dimensions

  Mdvx::field_header_t fhdrEcco = _eccoTypeField->getFieldHeader();
  Mdvx::field_header_t fhdrMrms = _mrmsDbzField->getFieldHeader();

  if (fhdrEcco.ny != fhdrMrms.ny ||
      fhdrEcco.nx != fhdrMrms.nx) {
    cerr << "ERROR - EccoStats::_readMrms" << endl;
    cerr << "  DBZ nx,ny grid does not match Ecco, file: " << _mrmsMdvx.getPathInUse() << endl;
    cerr << "  MRMS nx, ny: " << fhdrMrms.nx << ", " << fhdrMrms.ny << endl;
    cerr << "  Ecco nx, ny: " << fhdrEcco.nx << ", " << fhdrEcco.ny << endl;
    return -1;
  }
  
  return 0;

}

/////////////////////////////////////////////////////////
// read the coverage fields
// Returns 0 on success, -1 on failure.

int EccoStats::_readCoverage()
  
{

  // _covMinHtField = NULL;
  // _covMaxHtField = NULL;
  _covHtFractionField = NULL;

  _covMdvx.clear();
  if (_params.debug >= Params::DEBUG_EXTRA) {
    _covMdvx.setDebug(true);
  }
  // _covMdvx.addReadField(_params.coverage_min_ht_field_name);
  // _covMdvx.addReadField(_params.coverage_max_ht_field_name);
  _covMdvx.addReadField(_params.coverage_ht_fraction_field_name);
  _covMdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _covMdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Setting read for coverage data" << endl;
    _covMdvx.printReadRequest(cerr);
  }

  time_t eccoValidTime = _eccoMdvx.getValidTime();

  _covMdvx.setReadTime(Mdvx::READ_CLOSEST,
                       _params.coverage_dir,
                       180, eccoValidTime);
  
  // read in
  
  if (_covMdvx.readVolume()) {
    cerr << "ERROR - EccoStats::_readCoverage" << endl;
    cerr << "  Cannot read in data." << endl;
    cerr << _covMdvx.getErrStr() << endl;
    return -1;
  }
  if (_params.debug) {
    cerr << "Read coverage data file: " << _covMdvx.getPathInUse() << endl;
  }

  // get fields
  
  // _covMinHtField = _covMdvx.getField(_params.coverage_min_ht_field_name);
  // if (_covMinHtField == NULL) {
  //   cerr << "WARNING - EccoStats::_readCoverage" << endl;
  //   cerr << "  Cannot find coverage min ht field: "
  //        << _params.coverage_min_ht_field_name << endl;
  //   cerr << "  Coverage path: " << _covMdvx.getPathInUse() << endl;
  // }

  // _covMaxHtField = _covMdvx.getField(_params.coverage_max_ht_field_name);
  // if (_covMinHtField == NULL) {
  //   cerr << "WARNING - EccoStats::_readCoverage" << endl;
  //   cerr << "  Cannot find coverage max ht field: " << _params.coverage_max_ht_field_name << endl;
  //   cerr << "  Coverage path: " << _covMdvx.getPathInUse() << endl;
  // }

  _covHtFractionField = _covMdvx.getField(_params.coverage_ht_fraction_field_name);
  if (_covHtFractionField == NULL) {
    cerr << "ERROR - EccoStats::_readCoverage" << endl;
    cerr << "  Cannot find coverage ht fraction field: "
         << _params.coverage_ht_fraction_field_name << endl;
    cerr << "  Coverage path: " << _covMdvx.getPathInUse() << endl;
    return -1;
  }

  // check dimensions

  Mdvx::field_header_t fhdrEcco = _eccoTypeField->getFieldHeader();
  Mdvx::field_header_t fhdrCov = _covHtFractionField->getFieldHeader();

  if (fhdrEcco.ny != fhdrCov.ny ||
      fhdrEcco.nx != fhdrCov.nx) {
    cerr << "ERROR - EccoStats::_readCoverage" << endl;
    cerr << "  Coverage nx,ny grid does not match Ecco, file: " << _covMdvx.getPathInUse() << endl;
    cerr << "  COV nx, ny: " << fhdrCov.nx << ", " << fhdrCov.ny << endl;
    cerr << "  Ecco nx, ny: " << fhdrEcco.nx << ", " << fhdrEcco.ny << endl;
    return -1;
  }
  
  return 0;

}

/////////////////////////////////////////////////////////
// read in the titan data, create mask
// Returns 0 on success, -1 on failure.

int EccoStats::_readTitan()
  
{

  if (_params.debug) {
    cerr << "Reading titan data, validTime: " << DateTime::str(_eccoValidTime) << endl;
    cerr << "Titan dir: " << _params.titan_data_dir << endl;
  }
    
  // open titan file for the ecco valid time
  
  TitanFile tFile;
  if (tFile.openBestDayFile(_params.titan_data_dir,
                            _eccoValidTime,
                            NcxxFile::read)) {
    cerr << "ERROR - EccoStats::_readTitan" << endl;
    cerr << "  Cannot open titan file for time: "
         << DateTime::strm(_eccoValidTime) << endl;
    return -1;
  }
  
  // read scan headers

  if (tFile.readStormHeader()) {
    cerr << "ERROR - EccoStats::_readTitan" << endl;
    cerr << "  Cannot read storm header, file: " << tFile.getPathInUse() << endl;
    return -1;
  }
  if (tFile.readScanHeaders()) {
    cerr << "ERROR - EccoStats::_readTitan" << endl;
    cerr << "  Cannot read scan headers, file: " << tFile.getPathInUse() << endl;
    return -1;
  }

  // find closest scan in time
  
  int scanNum = 0;
  time_t bestScanTime = 0;
  double minDeltaSecs = 1.0e99;
  DateTime valid(_eccoValidTime);
  const vector<TitanData::ScanHeader> &scans = tFile.scans();
  for (size_t ii = 0; ii < scans.size(); ii++) {
    DateTime scanTime(scans[ii].time);
    double deltaSecs = fabs(scanTime - valid);
    if (deltaSecs < minDeltaSecs) {
      scanNum = ii;
      bestScanTime = scanTime.utime();
      minDeltaSecs = deltaSecs;
    }
  }

  if (minDeltaSecs > 10) {
    cerr << "ERROR - EccoStats::_readTitan" << endl;
    cerr << "  Cannot find titan data for Ecco valid time: "
         << DateTime::str(_eccoValidTime) << endl;
    cerr << "  Closest scan time: "
         << DateTime::str(bestScanTime) << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Found titan data, scan time: " << DateTime::str(bestScanTime) << endl;
  }

  // read in the scan that matches the Ecco data volume

  if (tFile.readScan(scanNum)) {
    cerr << "ERROR - EccoStats::_readTitan" << endl;
    cerr << "  Cannot read scan, file: " << tFile.getPathInUse() << endl;
    cerr << "    scan_num: " << scanNum << endl;
    cerr << "    scan_time: " << DateTime::strm(bestScanTime) << endl;
    cerr << tFile.getErrStr() << endl;
    return -1;
  }

  // check geometry to ensure Ecco and Titan grids match

  MdvxProj projEcco(_eccoTypeField->getFieldHeader());
  MdvxProj projTitan(tFile.scan().grid);
  if (projEcco != projTitan) {
    cerr << "ERROR - EccoStats::_readTitan" << endl;
    cerr << "  grids for Ecco and Titan do not match, file: " << tFile.getPathInUse() << endl;
    cerr << "================= Ecco projection ===========================" << endl;
    projEcco.print(cerr);
    cerr << "================= Titan projection ==========================" << endl;
    projTitan.print(cerr);
    cerr << "=============================================================" << endl;
    return -1;
  }

  // read tracking header

  if (tFile.readTrackHeader()) {
    cerr << "ERROR - EccoStats::_readTitan" << endl;
    cerr << "  Cannot read track header, file: " << tFile.getPathInUse() << endl;
    return -1;
  }

  // read in all of the track entries for this scan

  if (tFile.readScanEntries(scanNum)) {
    cerr << "ERROR - EccoStats::_readTitan" << endl;
    cerr << "  Cannot read scan entries, scan, file: "
         << scanNum << ", " << tFile.getPathInUse() << endl;
    return -1;
  }

  const vector<TitanData::TrackEntry> &scanEntries = tFile.scanEntries();
  cerr << "11111111111111 nStorms: " << tFile.scan().nstorms << endl;
  cerr << "11111111111111 scanEntries.size(): " << scanEntries.size() << endl;
  for (size_t ii = 0; ii < scanEntries.size(); ii++) {
    cerr << "2222222222222222222222222222222 ii: " << ii << endl;
    // scanEntries[ii].print(stderr, "", ii);
    int complexNum = scanEntries[ii].complex_track_num;
    // cerr << "2222222222222222222222222222222" << endl;
    if (tFile.readComplexTrackParams(complexNum, true)) {
      cerr << "ERROR - EccoStats::_readTitan" << endl;
      cerr << "  Cannot read complex track num: "
           << complexNum << endl;
      return -1;
    }
    int durationInSecs = tFile.complexParams().duration_in_secs;
    int nSimpleTracks = tFile.complexParams().n_simple_tracks;
    // tFile.complexParams().print(stderr, "", false, tFile.simplesPerComplex2D()[complexNum]);
    cerr << "  complexNum: " << complexNum << endl;
    cerr << "    durationInSecs: " << durationInSecs << endl;
    cerr << "    nSimpleTracks: " << nSimpleTracks << endl;
    cerr << "2222222222222222222222222222222" << endl;
  }

  // close titan file
  
  tFile.closeFile();

  return 0;
  
}

/////////////////////////////////////////////////////////
// create a 3d field
// third dimension is time

MdvxField *EccoStats::_make3DField(fl32 ***data,
                                   string fieldName,
                                   string longName,
                                   string units,
                                   double missingVal)
                                 
{

  Mdvx::field_header_t fhdr = _eccoTypeField->getFieldHeader();
  
  fhdr.missing_data_value = missingVal;
  fhdr.bad_data_value = missingVal;

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
                                   string units,
                                   double missingVal)
                                 
{

  // create header
  
  Mdvx::field_header_t fhdr = _eccoTypeField->getFieldHeader();
  
  fhdr.missing_data_value = missingVal;
  fhdr.bad_data_value = missingVal;

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

MdvxField *EccoStats::_computeMean3DField(fl32 ***data,
                                          fl32 ***counts,
                                          string fieldName,
                                          string longName,
                                          string units,
                                          double missingVal)
                                 
{

  // create header
  
  Mdvx::field_header_t fhdr = _eccoTypeField->getFieldHeader();
  
  fhdr.missing_data_value = missingVal;
  fhdr.bad_data_value = missingVal;

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
// compute a fractional 2d field - summary for all hours

MdvxField *EccoStats::_computeMean2DField(fl32 ***data,
                                          fl32 ***counts,
                                          string fieldName,
                                          string longName,
                                          string units,
                                          double missingVal)
                                 
{

  // create header
  
  Mdvx::field_header_t fhdr = _eccoTypeField->getFieldHeader();
  
  fhdr.missing_data_value = missingVal;
  fhdr.bad_data_value = missingVal;

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

  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vhdr.level[0] = 0;
  
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

/////////////////////////////////////////////////////////
// create a 2d coverage field

MdvxField *EccoStats::_makeMrms2DField(fl32 **data,
                                       string fieldName,
                                       string longName,
                                       string units,
                                       double missingVal)
                                 
{

  // create header
  
  Mdvx::field_header_t fhdr = _mrmsDbzField->getFieldHeader();
  
  fhdr.missing_data_value = missingVal;
  fhdr.bad_data_value = missingVal;
  
  fhdr.nz = 1;
  fhdr.grid_dz = 1.0;
  fhdr.grid_minz = 0.0;
  
  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;

  fhdr.dz_constant = 1;
  fhdr.data_dimension = 2;
  
  size_t npts = fhdr.nx * fhdr.ny * fhdr.nz;
  size_t volSize = npts * sizeof(fl32);
  fhdr.volume_size = volSize;
  
  Mdvx::vlevel_header_t vhdr = _mrmsDbzField->getVlevelHeader();
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
// compute a coverage stats field

MdvxField *EccoStats::_computeCov2DField(fl32 **sum,
                                         fl32 **counts,
                                         string fieldName,
                                         string longName,
                                         string units,
                                         double missingVal)
  
{
  
  // create header
  
  Mdvx::field_header_t fhdr = _covHtFractionField->getFieldHeader();
  
  fhdr.missing_data_value = missingVal;
  fhdr.bad_data_value = missingVal;

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

  Mdvx::vlevel_header_t vhdr = _covHtFractionField->getVlevelHeader();
  
  MdvxField::setFieldName(fieldName, fhdr);
  MdvxField::setFieldNameLong(longName, fhdr);
  MdvxField::setUnits(units, fhdr);

  // compute fraction field

  fl32 **mean = (fl32 **) ucalloc2(_ny, _nx, sizeof(fl32));
  for (int iy = 0; iy < _ny; iy++) {
    for (int ix = 0; ix < _nx; ix++) {
      if (counts[iy][ix] > 0) {
        mean[iy][ix] = sum[iy][ix] / counts[iy][ix];
      } else {
        mean[iy][ix] = missingVal;
      }
    } // ix
  } // iy

  // create field from header and data
  
  MdvxField *newField =
    new MdvxField(fhdr, vhdr, NULL, false, false, false);
  newField->setVolData(*mean, volSize, Mdvx::ENCODING_FLOAT32);
  newField->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_GZIP);

  // free up

  ufree2((void **) mean);

  // return newly created field
  
  return newField;

}

/////////////////////////////////////////////////////////
// compute 2D sum of counts field for all hours

MdvxField *EccoStats::_sumHourlyField(fl32 ***counts,
                                      string fieldName,
                                      string longName,
                                      string units,
                                      double missingVal)
                                 
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
  
  Mdvx::vlevel_header_t vhdr;
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vhdr.level[0] = 0;
  
  MdvxField::setFieldName(fieldName, fhdr);
  MdvxField::setFieldNameLong(longName, fhdr);
  MdvxField::setUnits(units, fhdr);
  
  // compute total counts for all hours

  fl32 **tot = (fl32 **) ucalloc2(_ny, _nx, sizeof(fl32));
  for (int iy = 0; iy < _ny; iy++) {
    for (int ix = 0; ix < _nx; ix++) {
      fl32 nn = 0.0;
      for (int iz = 0; iz < _nz; iz++) {
        nn += counts[iz][iy][ix];
      } // iz
      tot[iy][ix] = nn;
    } // ix
  } // iy
  
  // create field from header and data
  
  MdvxField *newField =
    new MdvxField(fhdr, vhdr, NULL, false, false, false);
  newField->setVolData(*tot, volSize, Mdvx::ENCODING_FLOAT32);
  newField->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_GZIP);
  
  // free up
  
  ufree2((void **) tot);

  // return newly created field
  
  return newField;

}


/////////////////////////////////////////////////////////
// create an hourly field

MdvxField *EccoStats::_makeHourlyField(int hour,
                                       fl32 ***data,
                                       string fieldName,
                                       string longName,
                                       string units,
                                       double missingVal)
                                 
{

  Mdvx::field_header_t fhdr = _eccoTypeField->getFieldHeader();
  
  fhdr.missing_data_value = _missingFl32;
  fhdr.bad_data_value = _missingFl32;

  fhdr.nx = _nx; // output grid
  fhdr.ny = _ny; // output grid
  fhdr.nz = 1; // single hour

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

  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vhdr.level[0] = 0;

  MdvxField::setFieldName(fieldName, fhdr);
  char text[1024];
  snprintf(text, 1024, "%s_hour_%d", longName.c_str(), hour);
  MdvxField::setFieldNameLong(text, fhdr);
  MdvxField::setUnits(units, fhdr);

  MdvxField *newField =
    new MdvxField(fhdr, vhdr, NULL, false, false, false);
  newField->setVolData(*data[hour], volSize, Mdvx::ENCODING_FLOAT32);
  newField->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_GZIP);

  return newField;

}

/////////////////////////////////////////////////////////
// compute an hourly fractional 3d field

MdvxField *EccoStats::_computeHourlyMeanField(int hour,
                                              fl32 ***data,
                                              fl32 ***counts,
                                              string fieldName,
                                              string longName,
                                              string units,
                                              double missingVal)
                                 
{

  // create header
  
  Mdvx::field_header_t fhdr = _eccoTypeField->getFieldHeader();
  
  fhdr.missing_data_value = missingVal;
  fhdr.bad_data_value = missingVal;

  fhdr.nx = _nx; // output grid
  fhdr.ny = _ny; // output grid
  fhdr.nz = 1; // single hour

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

  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vhdr.level[0] = 0;
  
  MdvxField::setFieldName(fieldName, fhdr);
  char text[1024];
  snprintf(text, 1024, "%s_hour_%d", longName.c_str(), hour);
  MdvxField::setFieldNameLong(text, fhdr);
  MdvxField::setUnits(units, fhdr);

  // compute fraction field

  fl32 **frac = (fl32 **) ucalloc2(_ny, _nx, sizeof(fl32));
  for (int iy = 0; iy < _ny; iy++) {
    for (int ix = 0; ix < _nx; ix++) {
      fl32 nn = counts[hour][iy][ix];
      if (nn != 0) {
        frac[iy][ix] = data[hour][iy][ix] / nn;
      } else {
        frac[iy][ix] = missingVal;
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

