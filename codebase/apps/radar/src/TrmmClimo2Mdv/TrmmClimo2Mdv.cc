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
// TrmmClimo2Mdv.cc
//
// TrmmClimo2Mdv object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
// May 2011 
//
///////////////////////////////////////////////////////////////
//
// TrmmClimo2Mdv reads netCDF files for the TRMM climatology
// data set, converts these to MDV or CF-netCDF, 
// and writes them out as specified.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <toolsa/os_config.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <didss/DataFileNames.hh>
#include <Mdv/MdvxField.hh>
#include "TrmmClimo2Mdv.hh"
using namespace std;

const float TrmmClimo2Mdv::_missingFloat = -9999.0;

// Constructor

TrmmClimo2Mdv::TrmmClimo2Mdv(int argc, char **argv)

{
  
  _input = NULL;
  isOK = true;

  // set programe name

  _progName = "TrmmClimo2Mdv";
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

  // check that file list set in archive and simulate mode
  
  if (_params.input_mode == Params::FILELIST && _args.inputFileList.size() == 0) {
    cerr << "ERROR: TrmmClimo2Mdv::TrmmClimo2Mdv." << endl;
    cerr << "  Mode is FILELIST."; 
    cerr << "  You must use -f to specify files on the command line."
	 << endl;
    _args.usage(_progName, cerr);
    isOK = false;
  }
    
  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  // initialize the data input object
  
  if (_params.input_mode == Params::REALTIME) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _params.max_realtime_valid_age,
			     PMU_auto_register,
			     _params.use_ldata_info_file);
  } else {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _args.inputFileList);
  }

  return;

}

// destructor

TrmmClimo2Mdv::~TrmmClimo2Mdv()

{

  if (_input) {
    delete _input;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int TrmmClimo2Mdv::Run ()
{

  int iret = 0;
  PMU_auto_register("Run");
  
  // loop until end of data
  
  char *inputPath;
  while ((inputPath = _input->next()) != NULL) {
    
    PMU_auto_register("Non-simulate mode");
    
    if (_processFile(inputPath)) {
      cerr << "ERROR = TrmmClimo2Mdv::Run" << endl;
      cerr << "  Processing file: " << inputPath << endl;
      iret = -1;
    }
    
  }
  
  return iret;

}

///////////////////////////////
// process file

int TrmmClimo2Mdv::_processFile(const char *input_path)

{

  if (_params.debug) {
    cerr << "Processing file: " << input_path << endl;
  }

  // open file

  Nc3File ncf(input_path);
  if (!ncf.is_valid()) {
    cerr << "ERROR - TrmmClimo2Mdv::_processFile" << endl;
    cerr << "  File: " << input_path << endl;
    return -1;
  }
  
  // declare an error object
  
  Nc3Error err(Nc3Error::silent_nonfatal);

  // get dimensions
  
  _xDim = ncf.get_dim("X");
  if (_xDim == NULL) {
    cerr << "ERROR - TrmmClimo2Mdv::_processFile" << endl;
    cerr << "  Cannot find dim 'X'" << endl;
    cerr << "  file: " << input_path << endl;
    return -1;
  }

  _yDim = ncf.get_dim("Y");
  if (_yDim == NULL) {
    cerr << "ERROR - TrmmClimo2Mdv::_processFile" << endl;
    cerr << "  Cannot find dim 'Y'" << endl;
    cerr << "  file: " << input_path << endl;
    return -1;
  }

  _tDim = ncf.get_dim("T");
  if (_tDim == NULL) {
    cerr << "ERROR - TrmmClimo2Mdv::_processFile" << endl;
    cerr << "  Cannot find dim 'T'" << endl;
    cerr << "  file: " << input_path << endl;
    return -1;
  }

  // get variables
  
  _xVar = ncf.get_var("X");
  if (_xVar == NULL) {
    cerr << "ERROR - TrmmClimo2Mdv::_processFile" << endl;
    cerr << "  Cannot find var 'X'" << endl;
    cerr << "  file: " << input_path << endl;
    return -1;
  }
  
  _yVar = ncf.get_var("Y");
  if (_yVar == NULL) {
    cerr << "ERROR - TrmmClimo2Mdv::_processFile" << endl;
    cerr << "  Cannot find var 'Y'" << endl;
    cerr << "  file: " << input_path << endl;
    return -1;
  }
  
  _tVar = ncf.get_var("T");
  if (_tVar == NULL) {
    cerr << "ERROR - TrmmClimo2Mdv::_processFile" << endl;
    cerr << "  Cannot find var 'T'" << endl;
    cerr << "  file: " << input_path << endl;
    return -1;
  }
  
  // loop through the variables, looking for data field

  _dataVar = NULL;
  for (int iiv = 0; iiv < ncf.num_vars(); iiv++) {
    Nc3Var* var = ncf.get_var(iiv);
    if (var == NULL) {
      continue;
    }
    int numDims = var->num_dims();
    if (numDims != 3) {
      continue;
    }
    if (var->get_dim(0) == _tDim &&
        var->get_dim(1) == _yDim &&
        var->get_dim(2) == _xDim) {
      _dataVar = var;
      break;
    }
  }

  if (_dataVar == NULL) {
    cerr << "ERROR - TrmmClimo2Mdv::_processFile" << endl;
    cerr << "  Cannot find 'data' var" << endl;
    cerr << "  Must have dims (T, Y, X)" << endl;
    cerr << "  file: " << input_path << endl;
    return -1;
  }
   
  // check dimension of data variable

  if (_dataVar->num_dims() != 3) {
    cerr << "ERROR - TrmmClimo2Mdv::_processFile" << endl;
    cerr << "  'data' var has incorrect number of dimentions" << endl;
    cerr << "  should be 3, n dims: " << _dataVar->num_dims() << endl;
    cerr << "  file: " << input_path << endl;
    return -1;
  }

  if (_dataVar->get_dim(0) != _tDim) {
    cerr << "ERROR - TrmmClimo2Mdv::_processFile" << endl;
    cerr << "  'data' var has incorrect first dimension" << endl;
    cerr << "  should be 'T', dim(0): " << _dataVar->get_dim(0) << endl;
    cerr << "  file: " << input_path << endl;
    return -1;
  }
  
  if (_dataVar->get_dim(1) != _yDim) {
    cerr << "ERROR - TrmmClimo2Mdv::_processFile" << endl;
    cerr << "  'data' var has incorrect second dimension" << endl;
    cerr << "  should be 'Y', dim(1): " << _dataVar->get_dim(1) << endl;
    cerr << "  file: " << input_path << endl;
    return -1;
  }
  
  if (_dataVar->get_dim(2) != _xDim) {
    cerr << "ERROR - TrmmClimo2Mdv::_processFile" << endl;
    cerr << "  'data' var has incorrect third dimension" << endl;
    cerr << "  should be 'X', dim(2): " << _dataVar->get_dim(2) << endl;
    cerr << "  file: " << input_path << endl;
    return -1;
  }

  int iret = 0;
  if (_params.write_as_single_file) {
    if (_writeAsSingleFile()) {
      iret = -1;
    }
  } else {
    if (_writeAsMultFiles()) {
      iret = -1;
    }
  }
  
  return 0;

}

//////////////////////////////////////
// write mult fields to a single file

int TrmmClimo2Mdv::_writeAsSingleFile()

{

  // check params

  if (_params.time_period_n != _tDim->size()) {
    cerr << "ERROR - TrmmClimo2Mdv::_writeAsSingleFile()" << endl;
    cerr << "  T dimension does not match period_name length" << endl;
    cerr << "  T dimension: " << _tDim->size() << endl;
    cerr << "  time_period len: " << _params.time_period_n << endl;
    return -1;
  }

  // write out as single file, with one field per climo time

  DsMdvx mdvx;

  // set the master header
  
  Mdvx::master_header_t mhdr;
  MEM_zero(mhdr);

  DateTime dtime(_params.reference_year, 1, 1, 0, 0, 0);
  mhdr.time_begin = dtime.utime();
  mhdr.time_end = dtime.utime();
  mhdr.time_centroid = dtime.utime();
  
  mhdr.num_data_times = 1;
  mhdr.data_dimension = 2;
  
  mhdr.data_collection_type = Mdvx::DATA_CLIMO_ANA;
  mhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  mhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  mhdr.max_nz = 1;
  mhdr.vlevel_included = TRUE;
  mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  mhdr.data_ordering = Mdvx::ORDER_XYZ;
  mhdr.max_nx = _xDim->size();
  mhdr.max_ny = _yDim->size();
  mhdr.n_chunks = 0;
  mhdr.field_grids_differ = FALSE;
  mhdr.sensor_lon = 0.0;
  mhdr.sensor_lat = 0.0;
  mhdr.sensor_alt = 0.0;

  mdvx.setDataSetInfo(_params.data_set_info);
  mdvx.setDataSetName(_params.data_set_name);
  mdvx.setDataSetSource(_params.data_set_source);
  
  mdvx.setMasterHeader(mhdr);

  // get the data

  TaArray<float> data_;
  int nx = _xDim->size();
  int ny = _yDim->size();
  int nt = _tDim->size();
  int npoints = nx * ny;
  int ndata = npoints * nt;
  float *data = data_.alloc(ndata);
  if (_dataVar->get(data, nt, ny, nx) == false) {
    cerr << "ERROR - TrmmClimo2Mdv::_writeAsSingleFile" << endl;
    cerr << "  'data' var has incorrect type" << endl;
    cerr << "  cannot return floats" << endl;
    return -1;
  }

  // replace nans with missing

  float *dataPtr = data;
  for (int ii = 0; ii < ndata; ii++, dataPtr++) {
    if (!std::isfinite(*dataPtr)) {
      *dataPtr = _missingFloat;
    }
  }

  // get lat/lon geometry

  double lat0 = _yVar->as_float(0);
  double lat1 = _yVar->as_float(1);
  double dlat = lat1 - lat0;
  
  double lon0 = _xVar->as_float(0);
  double lon1 = _xVar->as_float(1);
  double dlon = lon1 - lon0;
  
  // set the fields
  
  for (int itime = 0; itime < _tDim->size(); itime++) {

    // int iperiod = (int) (_tVar->as_float(itime) - 0.5);
    int iperiod = itime;
    string timeStr = _params._time_period[iperiod];
    if (iperiod >= 0 && iperiod < _params.time_period_n) {
      timeStr= _params._time_period[iperiod];
    } else {
      char text[128];
      sprintf(text, "period-%.2d", iperiod + 1);
      timeStr = text;
    }

    string fieldName = _params.field_name;
    fieldName += "_";
    fieldName += timeStr;
    
    string fieldNameLong = _params.field_name_long;
    fieldNameLong += "_";
    fieldNameLong += timeStr;

    // fill in field header and vlevel header
    
    Mdvx::field_header_t fhdr;
    MEM_zero(fhdr);
    Mdvx::vlevel_header_t vhdr;
    MEM_zero(vhdr);
    
    fhdr.proj_type = Mdvx::PROJ_LATLON;
    
    fhdr.proj_origin_lat = 0;
    fhdr.proj_origin_lon = 0;
    fhdr.nx = nx;
    fhdr.ny = ny;
    fhdr.nz = 1;

    fhdr.grid_dx = dlon;
    fhdr.grid_dy = dlat;
    fhdr.grid_minx = lon0;
    fhdr.grid_miny = lat0;

    fhdr.grid_dz = 1.0;
    fhdr.grid_minz = 0.0;
    
    // int npointsPlane = fhdr.nx * fhdr.ny;
    
    fhdr.compression_type = Mdvx::COMPRESSION_NONE;
    fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
    fhdr.scaling_type = Mdvx::SCALING_NONE;
    
    fhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
    fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
    fhdr.dz_constant = true;
    
    fhdr.bad_data_value = _missingFloat;
    fhdr.missing_data_value = _missingFloat;
    
    fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
    fhdr.data_element_nbytes = sizeof(fl32);
    fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(fl32);

    // vlevel header
    
    vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
    vhdr.level[0] = fhdr.grid_minz;

    // create field
    
    MdvxField *field = new MdvxField(fhdr, vhdr, data + itime * npoints);

    // set name etc
    
    field->setFieldName(fieldName.c_str());
    field->setFieldNameLong(fieldNameLong.c_str());
    field->setUnits(_params.field_units);
    field->setTransform("");
    
    Mdvx::compression_type_t compressionType = Mdvx::COMPRESSION_NONE;
    if (_params.compress_output) {
      compressionType = Mdvx::COMPRESSION_GZIP;
    }
    switch (_params.output_encoding) {
      case Params::ENCODING_INT8:
        field->convertDynamic(Mdvx::ENCODING_INT8, compressionType);
        break;
      case Params::ENCODING_FLOAT32:
        field->convertDynamic(Mdvx::ENCODING_FLOAT32, compressionType);
        break;
      case Params::ENCODING_INT16:
      default:
        field->convertDynamic(Mdvx::ENCODING_INT16, compressionType);
    }
    
    // add field to mdvx object
    
    mdvx.addField(field);

    if (_params.debug) {
      cerr << "Adding field: " << fieldName << endl;
    }
    
  } // itime
  
  if (_writeOutput(mdvx)) {
    cerr << "ERROR - TrmmClimo2Mdv::_writeAsSingleFile" << endl;
    return -1;
  }
  
  return 0;

}


//////////////////////////////////////////
// write mult files each with single field

int TrmmClimo2Mdv::_writeAsMultFiles()

{

  // check params

  if (_params.time_period_n != _tDim->size()) {
    cerr << "ERROR - TrmmClimo2Mdv::_writeAsSingleFile()" << endl;
    cerr << "  T dimension does not match period_name length" << endl;
    cerr << "  T dimension: " << _tDim->size() << endl;
    cerr << "  time_period len: " << _params.time_period_n << endl;
    return -1;
  }

  // get the data

  TaArray<float> data_;
  int nx = _xDim->size();
  int ny = _yDim->size();
  int nt = _tDim->size();
  int npoints = nx * ny;
  int ndata = npoints * nt;
  float *data = data_.alloc(ndata);
  if (_dataVar->get(data, nt, ny, nx) == false) {
    cerr << "ERROR - TrmmClimo2Mdv::_writeAsSingleFile" << endl;
    cerr << "  'data' var has incorrect type" << endl;
    cerr << "  cannot return floats" << endl;
    return -1;
  }

  // replace nans with missing

  float *dataPtr = data;
  for (int ii = 0; ii < ndata; ii++, dataPtr++) {
    if (!std::isfinite(*dataPtr)) {
      *dataPtr = _missingFloat;
    }
  }

  // get lat/lon geometry

  double lat0 = _yVar->as_float(0);
  double lat1 = _yVar->as_float(1);
  double dlat = lat1 - lat0;
  
  double lon0 = _xVar->as_float(0);
  double lon1 = _xVar->as_float(1);
  double dlon = lon1 - lon0;
  
  // loop through the times
  
  for (int itime = 0; itime < _tDim->size(); itime++) {

    double timeVal = _tVar->as_float(itime);
    DateTime climoTime;
    if (timeVal < 1) {
      // 3-hourly
      int secsInDay = (int) (timeVal * 86400.0 + 0.5);
      int hour = secsInDay / 3600;
      int min = (secsInDay - hour * 3600) / 60;
      int sec = secsInDay - hour * 3600 - min * 60;
      climoTime.set(2003, 06, 15, hour, min, sec);
    } else {
      // monthly
      int imonth = (int) (timeVal + 0.5);
      climoTime.set(2003, imonth, 15, 12, 00, 00);
    }
    
    // write out as single file, with one field per climo time
    
    DsMdvx mdvx;
    
    // set the master header
    
    Mdvx::master_header_t mhdr;
    MEM_zero(mhdr);
    
    mhdr.time_begin = climoTime.utime();
    mhdr.time_end = climoTime.utime();
    mhdr.time_centroid = climoTime.utime();
    
    mhdr.num_data_times = 1;
    mhdr.data_dimension = 2;
    
    mhdr.data_collection_type = Mdvx::DATA_CLIMO_ANA;
    mhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
    mhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
    mhdr.max_nz = 1;
    mhdr.vlevel_included = TRUE;
    mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
    mhdr.data_ordering = Mdvx::ORDER_XYZ;
    mhdr.max_nx = _xDim->size();
    mhdr.max_ny = _yDim->size();
    mhdr.n_chunks = 0;
    mhdr.field_grids_differ = FALSE;
    mhdr.sensor_lon = 0.0;
    mhdr.sensor_lat = 0.0;
    mhdr.sensor_alt = 0.0;
    
    mdvx.setDataSetInfo(_params.data_set_info);
    mdvx.setDataSetName(_params.data_set_name);
    mdvx.setDataSetSource(_params.data_set_source);
    
    mdvx.setMasterHeader(mhdr);
    
    // set the field
    
    string fieldName = _params.field_name;
    string fieldNameLong = _params.field_name_long;
    
    // fill in field header and vlevel header
    
    Mdvx::field_header_t fhdr;
    MEM_zero(fhdr);
    Mdvx::vlevel_header_t vhdr;
    MEM_zero(vhdr);
    
    fhdr.proj_type = Mdvx::PROJ_LATLON;
    
    fhdr.proj_origin_lat = 0;
    fhdr.proj_origin_lon = 0;
    fhdr.nx = nx;
    fhdr.ny = ny;
    fhdr.nz = 1;

    fhdr.grid_dx = dlon;
    fhdr.grid_dy = dlat;
    fhdr.grid_minx = lon0;
    fhdr.grid_miny = lat0;

    fhdr.grid_dz = 1.0;
    fhdr.grid_minz = 0.0;
    
    // int npointsPlane = fhdr.nx * fhdr.ny;
    
    fhdr.compression_type = Mdvx::COMPRESSION_NONE;
    fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
    fhdr.scaling_type = Mdvx::SCALING_NONE;
    
    fhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
    fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
    fhdr.dz_constant = true;
    
    fhdr.bad_data_value = _missingFloat;
    fhdr.missing_data_value = _missingFloat;
    
    fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
    fhdr.data_element_nbytes = sizeof(fl32);
    fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(fl32);

    // vlevel header
    
    vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
    vhdr.level[0] = fhdr.grid_minz;

    // create field
    
    MdvxField *field = new MdvxField(fhdr, vhdr, data + itime * npoints);

    // set name etc
    
    field->setFieldName(fieldName.c_str());
    field->setFieldNameLong(fieldNameLong.c_str());
    field->setUnits(_params.field_units);
    field->setTransform("");
    
    Mdvx::compression_type_t compressionType = Mdvx::COMPRESSION_NONE;
    if (_params.compress_output) {
      compressionType = Mdvx::COMPRESSION_GZIP;
    }
    switch (_params.output_encoding) {
      case Params::ENCODING_INT8:
        field->convertDynamic(Mdvx::ENCODING_INT8, compressionType);
        break;
      case Params::ENCODING_FLOAT32:
        field->convertDynamic(Mdvx::ENCODING_FLOAT32, compressionType);
        break;
      case Params::ENCODING_INT16:
      default:
        field->convertDynamic(Mdvx::ENCODING_INT16, compressionType);
    }
    
    // add field to mdvx object
    
    mdvx.addField(field);

    if (_params.debug) {
      cerr << "Adding field: " << fieldName << endl;
    }
    
    if (_writeOutput(mdvx)) {
      cerr << "ERROR - TrmmClimo2Mdv::_writeAsSingleFile" << endl;
      return -1;
    }
  
  } // itime
  
  return 0;

}


////////////////////////////////////////////////////
// write output data
//
// returns 0 on success, -1 on failure

int TrmmClimo2Mdv::_writeOutput(DsMdvx &mdvx)

{

  // write out

  if (mdvx.writeToDir(_params.output_url)) {
    cerr << "ERROR - TrmmClimo2Mdv::_writeOutput" << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Wrote file: " << mdvx.getPathInUse() << endl;
  }

  return 0;
    
}

