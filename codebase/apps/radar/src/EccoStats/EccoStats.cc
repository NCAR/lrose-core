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
#include "EccoStats.hh"
using namespace std;

const fl32 EccoStats::_missingFl32 = -9999.0;

// Constructor

EccoStats::EccoStats(int argc, char **argv)

{

  isOK = true;
  
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
    if (_input.setArchive(_params.input_dir,
			  _args.startTime,
			  _args.endTime)) {
      isOK = false;
    }
  } else if (_params.mode == Params::FILELIST) {
    if (_input.setFilelist(_args.inputFileList)) {
      isOK = false;
    }
  }

  // init field pointers
  // these point to memory in MdvxField objects and do not need to be freed
  
  _eccoTypeField = NULL;
  _convectivityField = NULL;
  _terrainHtField = NULL;
  _waterFlagField = NULL;

  // init arrays

  _initArrays();
  
}

// destructor

EccoStats::~EccoStats()

{

  // free up arrays

  _freeArrays();
  
}

//////////////////////////////////////////////////
// Run

int EccoStats::Run()
{
  
  int iret = 0;

  // loop until end of data
  
  _input.reset();
  int count = 0;
  while (!_input.endOfData()) {
    
    // do the read

    if (_doRead()) {
      cerr << "ERROR - EccoStats::Run()" << endl;
      umsleep(1000);
      iret = -1;
      continue;
    }

    // create the arrays on the first pass
    
    Mdvx::field_header_t fhdr = _eccoTypeField->getFieldHeader();
    if (count == 0) {
      // first file
      _nx = fhdr.nx;
      _ny = fhdr.ny;
      _nz = 24; // diurnal hour index
      _allocArrays();
    } else {
      if (_nx != fhdr.nx || _ny != fhdr.ny) {
        cerr << "ERROR - grid size has changed" << endl;
        cerr << "  nx, ny found: " << fhdr.nx << ", " << fhdr.ny << endl;
        cerr << "  nx, ny should be: " << _nx << ", " << _ny << endl;
        cerr << "  skipping this file: " << _inMdvx.getPathInUse() << endl;
        iret = -1;
        continue;
      }
    }
    count++;

    // process the file

    _processInputFile();
    
    // clear
    
    if (_params.debug) {
      cerr << "Done processing file: " << _inMdvx.getPathInUse() << endl;
    }

    _inMdvx.clear();
    
  } // while
  
  // write out
  
  if (_doWrite()) {
    cerr << "ERROR - EccoStats::Run()" << endl;
    umsleep(1000);
    return -1;
  }
    
  return iret;

}

/////////////////////////////////////////////////////////
// initialize the arrays

void EccoStats::_initArrays()
  
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
  
  // others

  _lon = (double **) ucalloc2(_ny, _nx, sizeof(double));
  _lat = (double **) ucalloc2(_ny, _nx, sizeof(double));

  Mdvx::field_header_t fhdr = _eccoTypeField->getFieldHeader();
  for (int iy = 0; iy < fhdr.ny; iy++) {
    for (int ix = 0; ix < fhdr.nx; ix++) {
      double lat, lon;
      _proj.xyIndex2latlon(ix, iy, lat, lon);
      _lat[iy][iy] = lat;
      _lon[iy][iy] = lon;
    }
  }

  _hourOfDay = (int **) ucalloc2(_ny, _nx, sizeof(int));

  if (_terrainHtField != NULL) {
    _terrainHt = (fl32 **) ucalloc2(_ny, _nx, sizeof(fl32));
    fl32 *terrainHt2D = (fl32 *) _terrainHtField->getVol();
    size_t offset = 0;
    for (int iy = 0; iy < fhdr.ny; iy++) {
      for (int ix = 0; ix < fhdr.nx; ix++, offset++) {
        _terrainHt[iy][ix] = terrainHt2D[offset];
      }
    }
  }
  
  if (_waterFlagField != NULL) {
    _waterFlag = (fl32 **) ucalloc2(_ny, _nx, sizeof(fl32));
    fl32 *waterFlag2D = (fl32 *) _waterFlagField->getVol();
    size_t offset = 0;
    for (int iy = 0; iy < fhdr.ny; iy++) {
      for (int ix = 0; ix < fhdr.nx; ix++, offset++) {
        _waterFlag[iy][ix] = waterFlag2D[offset];
      }
    }
  }
  
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
// perform the read
//
// Returns 0 on success, -1 on failure.

int EccoStats::_doRead()
  
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
  
  // read in
  
  if (_input.readVolumeNext(_inMdvx)) {
    cerr << "ERROR - EccoStats::_doRead" << endl;
    cerr << "  Cannot read in data." << endl;
    cerr << _input.getErrStr() << endl;
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
    cerr << "ERROR - doRead(), file: " << _inMdvx.getPathInUse() << endl;
    cerr << "  Cannot find field: " << _params.ecco_type_comp_field_name << endl;
    return -1;
  }
  if (_convectivityField == NULL) {
    cerr << "ERROR - doRead(), file: " << _inMdvx.getPathInUse() << endl;
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
// process the file in _inMdvx
// Returns 0 on success, -1 on failure.

int EccoStats::_processInputFile()
  
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
  for (int iy = 0; iy < _ny; iy++) {
    for (int ix = 0; ix < _nx; ix++, offset++) {

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

    } // ix
  } // iy

  
  
  return 0;

}
  
/////////////////////////////////////////////////////////
// add fields to the output object

void EccoStats::_addFieldsToOutput()
  
{

  _outMdvx.clearFields();

  // initial fields are float32

  MdvxField *dbzField = _inMdvx.getField("Ecco");
  Mdvx::field_header_t fhdr2d = dbzField->getFieldHeader();
  fhdr2d.nz = 1;
  fhdr2d.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  size_t planeSize32 = fhdr2d.nx * fhdr2d.ny * sizeof(fl32);
  fhdr2d.volume_size = planeSize32;
  fhdr2d.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr2d.data_element_nbytes = 4;
  fhdr2d.missing_data_value = _missingFl32;
  fhdr2d.bad_data_value = _missingFl32;
  fhdr2d.scale = 1.0;
  fhdr2d.bias = 0.0;
  
  Mdvx::vlevel_header_t vhdr2d;
  MEM_zero(vhdr2d);
  vhdr2d.level[0] = 0;
  vhdr2d.type[0] = Mdvx::VERT_TYPE_SURFACE;

#ifdef JUNK
  
  if (_params.write_texture) {
    // add 2D max texture field
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getTexture2D(),
                                 Mdvx::ENCODING_INT16,
                                 "DbzTextureComp",
                                 "reflectivity_texture_composite",
                                 "dBZ"));
  }
  
  if (_params.write_convectivity) {
    // convectivity max in 2D
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getConvectivity2D(),
                                 Mdvx::ENCODING_INT16,
                                 "ConvectivityComp",
                                 "likelihood_of_convection_composite",
                                 ""));
  }

  if (_params.write_col_max_dbz) {
    // col max dbz
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getDbzColMax(),
                                 Mdvx::ENCODING_INT16,
                                 "DbzComp",
                                 "dbz_composite",
                                 "dBZ"));
  }
    
  if (_params.write_fraction_active) {
    // load up fraction of texture kernel covered
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getFractionActive(),
                                 Mdvx::ENCODING_INT16,
                                 "FractionActive",
                                 "fraction_of_texture_kernel_active",
                                 ""));
  }
    
  if (_params.write_tops) {
    // tops
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getConvTopKm(),
                                 Mdvx::ENCODING_INT16,
                                 "ConvTops",
                                 "convective_tops",
                                 "km"));
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getStratTopKm(),
                                 Mdvx::ENCODING_INT16,
                                 "StratTops",
                                 "stratiform_tops",
                                 "km"));
    char longName[256];
    snprintf(longName, 256, "%g_dbz_echo_tops", _params.dbz_for_echo_tops);
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getEchoTopKm(),
                                 Mdvx::ENCODING_INT16,
                                 "EchoTops", longName, "km"));
  }
  
  if (_params.write_height_grids &&
      _params.vert_levels_type == Params::VERT_LEVELS_BY_TEMP) {
    _shallowHtField.convertType(Mdvx::ENCODING_INT16, Mdvx::COMPRESSION_GZIP);
    _outMdvx.addField(new MdvxField(_shallowHtField));
    _deepHtField.convertType(Mdvx::ENCODING_INT16, Mdvx::COMPRESSION_GZIP);
    _outMdvx.addField(new MdvxField(_deepHtField));
  }
  
  if (_params.write_temperature && _tempField != NULL) {
    MdvxField * tempField = new MdvxField(*_tempField);
    tempField->convertType(Mdvx::ENCODING_INT16, Mdvx::COMPRESSION_GZIP);
    _outMdvx.addField(tempField);
  }

  // the following 2d fields are unsigned bytes
  
  size_t planeSize08 = fhdr2d.nx * fhdr2d.ny * sizeof(ui08);
  fhdr2d.volume_size = planeSize08;
  fhdr2d.encoding_type = Mdvx::ENCODING_INT8;
  fhdr2d.data_element_nbytes = 1;
  fhdr2d.missing_data_value = ConvStratFinder::CATEGORY_MISSING;
  fhdr2d.bad_data_value = ConvStratFinder::CATEGORY_MISSING;
  
  // echoType field
  
  if (_params.write_partition) {
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getEchoType2D(),
                                 Mdvx::ENCODING_INT8,
                                 "EchoTypeComp",
                                 "convective_stratiform_echo_type_composite",
                                 ""));
  }
  
  // the following 3d fields are floats
  
  Mdvx::field_header_t fhdr3d = dbzField->getFieldHeader();
  Mdvx::vlevel_header_t vhdr3d = dbzField->getVlevelHeader();
  fhdr3d.missing_data_value = _missingFl32;
  fhdr3d.bad_data_value = _missingFl32;

  if (_params.write_convective_dbz) {
    // reflectivity only where convection has been identified
    MdvxField *convDbz = _makeField(fhdr3d, vhdr3d,
                                    _finder.getConvectiveDbz(),
                                    Mdvx::ENCODING_INT16,
                                    "DbzConv",
                                    "convective_reflectivity_3D",
                                    "dBZ");
    _outMdvx.addField(convDbz);
  }
  
  if (_params.write_texture) {
    // texture in 3D
    _outMdvx.addField(_makeField(fhdr3d, vhdr3d,
                                 _finder.getTexture3D(),
                                 Mdvx::ENCODING_INT16,
                                 "DbzTexture3D",
                                 "reflectivity_texture_3D",
                                 "dBZ"));
  }

  if (_params.write_convectivity) {
    // convectivity in 3D
    _outMdvx.addField(_makeField(fhdr3d, vhdr3d,
                                 _finder.getConvectivity3D(),
                                 Mdvx::ENCODING_INT16,
                                 "Convectivity3D",
                                 "likelihood_of_convection_3D",
                                 ""));
  }

  if (_params.write_3D_dbz) {
    // echo the input DBZ field
    _outMdvx.addField(_makeField(fhdr3d, vhdr3d,
                                 _finder.getDbz3D(),
                                 Mdvx::ENCODING_INT16,
                                 "Dbz3D",
                                 "reflectivity_3D",
                                 "dBZ"));
  }
  
  if (_params.write_partition) {
    // echoType for full volume
    size_t volSize08 = fhdr3d.nx * fhdr3d.ny * fhdr3d.nz * sizeof(ui08);
    fhdr3d.volume_size = volSize08;
    fhdr3d.encoding_type = Mdvx::ENCODING_INT8;
    fhdr3d.data_element_nbytes = 1;
    fhdr3d.missing_data_value = ConvStratFinder::CATEGORY_MISSING;
    fhdr3d.bad_data_value = ConvStratFinder::CATEGORY_MISSING;
    _outMdvx.addField(_makeField(fhdr3d, vhdr3d,
                                 _finder.getEchoType3D(),
                                 Mdvx::ENCODING_INT8,
                                 "EchoType3D",
                                 "convective_stratiform_echo_type_3D",
                                 ""));
  }

  if (_params.write_clumping_debug_fields) {
    _addClumpingDebugFields();
  }

  // add ht data if available
  
  if (_params.use_terrain_ht_data) {
    MdvxField *htFieldIn = _inMdvx.getField("TerrainHt");
    if (htFieldIn != NULL) {
      MdvxField *htFieldOut = new MdvxField(*htFieldIn);
      htFieldOut->convertType(Mdvx::ENCODING_ASIS, Mdvx::COMPRESSION_GZIP);
      _outMdvx.addField(htFieldOut);
    }
    // add water field
    if (_params.add_water_layer) {
      MdvxField *waterFieldIn = _inMdvx.getField("WaterFlag");
      if (waterFieldIn != NULL) {
        MdvxField *waterFieldOut = new MdvxField(*waterFieldIn);
        waterFieldOut->convertType(Mdvx::ENCODING_ASIS, Mdvx::COMPRESSION_GZIP);
        _outMdvx.addField(waterFieldOut);
      }
    }
  }

#endif
  
}

/////////////////////////////////////////////////////////
// perform the write
//
// Returns 0 on success, -1 on failure.

int EccoStats::_doWrite()
  
{
  
  // create output DsMdvx object
  // copying master header from input object
  
  _outMdvx.clear();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _outMdvx.setDebug(true);
  }
  // _outMdvx.setWriteLdataInfo();
  Mdvx::master_header_t mhdr = _inMdvx.getMasterHeader();
  _outMdvx.setMasterHeader(mhdr);
  string info = _inMdvx.getMasterHeader().data_set_info;
  info += " : Stratfinder used to identify stratiform regions";
  _outMdvx.setDataSetInfo(info.c_str());
  _outMdvx.setMdv2NcfOutput(true, true, true, true);
  
  // copy any chunks
  
  // for (size_t i = 0; i < _inMdvx.getNChunks(); i++) {
  //   MdvxChunk *chunk = new MdvxChunk(*_inMdvx.getChunkByNum(i));
  //   _outMdvx.addChunk(chunk);
  // }
  
  // add fields
  
  _addFieldsToOutput();
  
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
// create a float field

MdvxField *EccoStats::_makeField(Mdvx::field_header_t &fhdrTemplate,
                                 Mdvx::vlevel_header_t &vhdr,
                                 const fl32 *data,
                                 Mdvx::encoding_type_t outputEncoding,
                                 string fieldName,
                                 string longName,
                                 string units)
                                 
{

  Mdvx::field_header_t fhdr = fhdrTemplate;
  MdvxField::setFieldName(fieldName, fhdr);
  MdvxField::setFieldNameLong(longName, fhdr);
  MdvxField::setUnits(units, fhdr);
  MdvxField *newField =
    new MdvxField(fhdr, vhdr, NULL, false, false, false);
  size_t npts = fhdr.nx * fhdr.ny * fhdr.nz;
  size_t volSize = npts * sizeof(fl32);
  newField->setVolData(data, volSize, Mdvx::ENCODING_FLOAT32);
  newField->convertType(outputEncoding, Mdvx::COMPRESSION_GZIP);

  return newField;

}

/////////////////////////////////////////////////////////
// create a byte field

MdvxField *EccoStats::_makeField(Mdvx::field_header_t &fhdrTemplate,
                                 Mdvx::vlevel_header_t &vhdr,
                                 const ui08 *data,
                                 Mdvx::encoding_type_t outputEncoding,
                                 string fieldName,
                                 string longName,
                                 string units)
                                 
{
  
  Mdvx::field_header_t fhdr = fhdrTemplate;
  MdvxField::setFieldName(fieldName, fhdr);
  MdvxField::setFieldNameLong(longName, fhdr);
  MdvxField::setUnits(units, fhdr);
  MdvxField *newField =
    new MdvxField(fhdr, vhdr, NULL, false, false, false);
  size_t npts = fhdr.nx * fhdr.ny * fhdr.nz;
  size_t volSize = npts * sizeof(ui08);
  newField->setVolData(data, volSize, Mdvx::ENCODING_INT8);
  newField->convertType(outputEncoding, Mdvx::COMPRESSION_GZIP);
  
  return newField;

}

#ifdef JUNK

/////////////////////////////////////////////////////////
// addClumpingDebugFields()
//
// add debug fields for dual threshold clumps

void EccoStats::_addClumpingDebugFields()
  
{

  MdvxField *dbzField = _inMdvx.getField(_params.dbz_field_name);
  Mdvx::field_header_t fhdr2d = dbzField->getFieldHeader();
  fhdr2d.nz = 1;
  fhdr2d.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  size_t planeSize32 = fhdr2d.nx * fhdr2d.ny * sizeof(fl32);
  fhdr2d.volume_size = planeSize32;
  fhdr2d.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr2d.data_element_nbytes = 4;
  fhdr2d.missing_data_value = _missingFl32;
  fhdr2d.bad_data_value = _missingFl32;
  fhdr2d.scale = 1.0;
  fhdr2d.bias = 0.0;
  
  Mdvx::vlevel_header_t vhdr2d;
  MEM_zero(vhdr2d);
  vhdr2d.level[0] = 0;
  vhdr2d.type[0] = Mdvx::VERT_TYPE_SURFACE;

  // add clump composite reflectivity

  _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                               _finder.getClumpingMgr().getDualThreshDbzCompOutputGrid(),
                               Mdvx::ENCODING_INT16,
                               "ClumpsCompDbz",
                               "ClumpsCompDbz",
                               "dBZ"));
  
  _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                               _finder.getClumpingMgr().getDualThreshLargeClumpsOutputGrid(),
                               Mdvx::ENCODING_INT8,
                               "LargeClumps",
                               "LargeClumps",
                               "count"));
  
  // add sub clump grids

  _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                               _finder.getClumpingMgr().getDualThreshAllSubclumpsOutputGrid(),
                               Mdvx::ENCODING_INT8,
                               "AllSubclumps",
                               "AllSubclumps",
                               "count"));
  
  _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                               _finder.getClumpingMgr().getDualThreshValidSubclumpsOutputGrid(),
                               Mdvx::ENCODING_INT8,
                               "SmallSubclumps",
                               "SmallSubclumps",
                               "count"));
  
  _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                               _finder.getClumpingMgr().getDualThreshGrownSubclumpsOutputGrid(),
                               Mdvx::ENCODING_INT8,
                               "GrownSubclumps",
                               "GrownSubclumps",
                               "count"));
  
}

#endif
