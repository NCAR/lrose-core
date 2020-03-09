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
// Plain2Mdv.cc
//
// Plain2Mdv object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2005
//
///////////////////////////////////////////////////////////////
//
// Plain2Mdv converts a plain binary array file to MDV.
// The data in the file must be a single field in an ordered binary
// array.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <toolsa/os_config.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaFile.hh>
#include <didss/DataFileNames.hh>
#include <dataport/bigend.h>
#include <dataport/swap.h>
#include <Mdv/MdvxField.hh>
#include "Plain2Mdv.hh"
using namespace std;

const fl32 Plain2Mdv::_missingFloat = -9999.0;

// Constructor

Plain2Mdv::Plain2Mdv(int argc, char **argv)

{

  _input = NULL;
  isOK = true;

  // set programe name

  _progName = "Plain2Mdv";
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
  
  if (_params.mode == Params::ARCHIVE && _args.inputFileList.size() == 0) {
    cerr << "ERROR: Plain2Mdv::Plain2Mdv." << endl;
    cerr << "  Mode is ARCHIVE."; 
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
  
  if (_params.mode == Params::REALTIME) {
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
  if (strlen(_params.input_file_extension) > 0) {
    _input->setSearchExt(_params.input_file_extension);
  }

  // check n_fields

  if (_params.n_fields != _params.fields_n) {
    cerr << "ERROR: Plain2Mdv::Plain2Mdv." << endl;
    cerr << "  n_fields must equal size of fields array."; 
    cerr << "  n_fields: " << _params.n_fields << endl;
    cerr << "  fields[] size: " << _params.fields_n << endl;
    isOK = false;
  }
  
  // check nz

  if (_params.grid_size.nz != _params.vlevels_n) {
    cerr << "ERROR: Plain2Mdv::Plain2Mdv." << endl;
    cerr << "  nz must equal number of vlevels."; 
    cerr << "  nz: " << _params.grid_size.nz << endl;
    cerr << "  n vlevels: " << _params.vlevels_n << endl;
    isOK = false;
  }
  
  if (_params.grid_size.nz > 122) {
    cerr << "ERROR: Plain2Mdv::Plain2Mdv." << endl;
    cerr << "  nz too large: " << _params.grid_size.nz << endl;
    cerr << "  max 122 allowed" << endl;
    isOK = false;
  }
  
  return;

}

// destructor

Plain2Mdv::~Plain2Mdv()

{

  if (_input) {
    delete _input;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Plain2Mdv::Run ()
{

  int iret = 0;
  PMU_auto_register("Run");
  
  // loop until end of data
  
  char *inputPath;
  while ((inputPath = _input->next()) != NULL) {
    
    PMU_auto_register("Non-simulate mode");
    
    if (_processFile(inputPath)) {
      cerr << "ERROR = Plain2Mdv::Run" << endl;
      cerr << "  Processing file: " << inputPath << endl;
      iret = -1;
    }
    
  }
  
  return iret;

}

///////////////////////////////
// process file

int Plain2Mdv::_processFile(const char *input_path)

{

  if (_params.debug) {
    cerr << "Processing file: " << input_path << endl;
  }

  // determine data time from file name

  time_t dataTime;
  if (_computeDataTime(input_path, dataTime)) {
    return -1;
  }
  if (_params.debug) {
    cerr << "  Data time: " << DateTime::strn(dataTime) << endl;
  }

  // open file - file closes automatically when inFile goes
  // out of scope
  
  TaFile inFile;
  FILE *in;
  if ((in = inFile.fopen(input_path, "r")) == NULL) {
    cerr << "ERROR - Plain2Mdv::_processFile" << endl;
    cerr << "  Cannot open input file: " << input_path << endl;
    perror("  ");
    return -1;
  }

  // get file size in bytes

  if (inFile.stat(input_path)) {
    cerr << "ERROR - Plain2Mdv::_processFile" << endl;
    cerr << "  Cannot stat input file: " << input_path << endl;
    perror("  ");
    inFile.fclose();
    return -1;
  }
  const struct stat &fileStat = inFile.getStat();
  size_t nBytesFile = fileStat.st_size;

  // check file size is large enough for expected data fields

  int nx = _params.grid_size.nx;
  int ny = _params.grid_size.ny;
  int nz = _params.grid_size.nz;
  int nxyz = nx * ny * nz;
  int nxy = nx * ny;

  size_t nFields = _params.n_fields;

  size_t nBytesExpected = 0;

  switch (_params.input_encoding) {
    case Params::ENCODING_INT8:
      nBytesExpected = nxyz * nFields * sizeof(ui08);
      break;
    case Params::ENCODING_INT16:
      nBytesExpected = nxyz * nFields * sizeof(ui16);
      break;
    case Params::ENCODING_FLOAT32:
      nBytesExpected = nxyz * nFields * sizeof(fl32);
      break;
  }

  if (nBytesFile < nBytesExpected) {
    cerr << "ERROR - Plain2Mdv::_processFile" << endl;
    cerr << "  File not large enough: " << input_path << endl;
    cerr << "  n bytes expected: " << nBytesExpected << endl;
    cerr << "  n bytes in file: " << nBytesFile << endl;
    inFile.fclose();
    return -1;
  }
  
  // read in input data

  ui08 *inBuf = new ui08[nBytesExpected];
  if (inFile.fread(inBuf, 1, nBytesExpected) != nBytesExpected) {
    int errNum = errno;
    cerr << "ERROR - Plain2Mdv::_processFile" << endl;
    cerr << "  Cannot read input file: " << input_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    delete[] inBuf;
    inFile.fclose();
    return -1;
  }

  // close input file

  inFile.fclose();

  // byte swap input buffer if needed

  if (_params.byte_swap_input_data) {
    switch (_params.input_encoding) {
      case Params::ENCODING_INT16:
        SWAP_array_16((ui16 *) inBuf, nBytesExpected);
        break;
      case Params::ENCODING_FLOAT32:
        SWAP_array_32((ui32 *) inBuf, nBytesExpected);
        break;
      default: {}
    }
  }

  // create output file object, initialize master header

  DsMdvx mdvx;
  _initMasterHeader(mdvx, dataTime);

  // load up field data

  for (size_t ifield = 0; ifield < nFields; ifield++) {

    fl32 *data = new fl32[nxyz];
    ui08 *buf = NULL; 
    switch (_params.input_encoding) {
      case Params::ENCODING_INT8:
        buf = inBuf + (nxyz * ifield * sizeof(ui08));
        break;
      case Params::ENCODING_INT16:
        buf = inBuf + (nxyz * ifield * sizeof(ui16));
        break;
      case Params::ENCODING_FLOAT32:
        buf = inBuf + (nxyz * ifield * sizeof(fl32));
        break;
    }
    
    _loadFieldData(buf, nx, ny, nz, nxy, nxyz,
                   _params._fields[ifield].scale,
                   _params._fields[ifield].bias,
                   _params._fields[ifield].missing_val,
                   _params._fields[ifield].min_val,
                   _params._fields[ifield].max_val,
                   data);
    
    // add field to output object

    _addField(mdvx, ifield, data);

    // free up

    delete[] data;

  } // ifield

  // free up input buffer

  delete[] inBuf;

  // write out file
  
  if (_writeOutput(mdvx)) {
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////
// compute data time from file path
//
// returns 0 on success, -1 on failure

int Plain2Mdv::_computeDataTime(const char *input_path, time_t &dataTime)

{

  if (_params.filename_time == Params::YYMMDDHH) {

    Path ipath(input_path);
    int yy, mm, dd, hh;
    cerr << "File: " << ipath.getFile() << endl;
    if (sscanf(ipath.getFile().c_str(), "%2d%2d%2d%2d",
               &yy, &mm, &dd, &hh) != 4) {
      cerr << "ERROR - Plain2Mdv::_computeDataTime" << endl;
      cerr << "  Cannot determine time from file name: " << input_path << endl;
      return -1;
    }

    if (yy > 70) {
      yy += 1900;
    } else {
      yy += 2000;
    }
    DateTime dtime(yy, mm, dd, hh);
    dataTime = dtime.utime();

  } else {

    bool date_only;
    if (DataFileNames::getDataTime(input_path, dataTime, date_only)) {
      cerr << "ERROR - Plain2Mdv::_computeDataTime" << endl;
      cerr << "  Cannot determine time from file name: " << input_path << endl;
      return -1;
    }

  }

  return 0;

}

////////////////////////////////////////////////////
// load up field data

void Plain2Mdv::_loadFieldData(ui08 *inBuf,
                               int nx, int ny, int nz, int nxy, int nxyz,
                               fl32 scale, fl32 bias, fl32 miss,
			       fl32 minVal, fl32 maxVal,
                               fl32 *data)
  
{
  
  switch (_params.input_encoding) {

    case Params::ENCODING_INT8: {
      ui08 *bb = inBuf;
      for (int ii = 0; ii < nxyz; ii++, bb++) {
        if (miss == (fl32) *bb) {
          data[ii] = _missingFloat;
        } else {
          fl32 ff = *bb * scale + bias;
	  if (ff > minVal && ff < maxVal) {
	    data[ii] = ff;
	  } else {
	    data[ii] = _missingFloat;
	  }
        }
      } // ii
      break;
    }

    case Params::ENCODING_INT16: {
      ui16 *ss = (ui16 *) inBuf;
      for (int ii = 0; ii < nxyz; ii++, ss++) {
        if (miss == (fl32) *ss) {
          data[ii] = _missingFloat;
        } else {
          fl32 ff = *ss * scale + bias;
	  if (ff > minVal && ff < maxVal) {
	    data[ii] = ff;
	  } else {
	    data[ii] = _missingFloat;
	  }
        }
      } // ii
      break;
    }

    default: {
      fl32 *fff = (fl32 *) inBuf;
      for (int ii = 0; ii < nxyz; ii++, fff++) {
	fl32 ff = *fff;
        if (miss == ff) {
          data[ii] = _missingFloat;
        } else {
	  if (ff > minVal && ff < maxVal) {
	    data[ii] = ff;
	  } else {
	    data[ii] = _missingFloat;
	  }
        }
      } // ii
      break;
    }
      
  } // switch

  // re-arrange rows as required

  if (_params.input_row_ordering == Params::NORTH_ROW_FIRST) {

    fl32 *tmp = new fl32[nxyz];

    for (int iz = 0; iz < nz; iz++) {
      for (int iy = 0; iy < ny; iy++) {
        int iIn = iz * nxy + (ny - 1 - iy) * nx;
        int iOut = iz * nxy + iy * nx;
        memcpy(tmp + iOut, data + iIn, nx * sizeof(fl32));
      } // iy
    } // iz
    
    memcpy(data, tmp, nxyz * sizeof(fl32));
    delete[] tmp;
    
  }

}
  
////////////////////////////////////////////////////
// init master header

void Plain2Mdv::_initMasterHeader(DsMdvx &mdvx, time_t dataTime)

{

  // set the master header
  
  Mdvx::master_header_t mhdr;
  MEM_zero(mhdr);

  mhdr.time_begin = dataTime;
  mhdr.time_end = dataTime;
  mhdr.time_centroid = dataTime;
    
  mhdr.num_data_times = 1;
  mhdr.data_dimension = 3;
  
  mhdr.data_collection_type = -1;
  mhdr.native_vlevel_type = _params.vlevel_type;
  mhdr.vlevel_type = _params.vlevel_type;
  mhdr.max_nz = _params.vlevels_n;
  mhdr.vlevel_included = TRUE;
  mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  mhdr.data_ordering = Mdvx::ORDER_XYZ;
  mhdr.max_nx = _params.grid_size.nx;
  mhdr.max_ny = _params.grid_size.ny;
  mhdr.n_chunks = 0;
  mhdr.field_grids_differ = FALSE;
  mhdr.sensor_lon = 0.0;
  mhdr.sensor_lat = 0.0;
  mhdr.sensor_alt = 0.0;

  mdvx.setDataSetInfo(_params.data_set_info);
  mdvx.setDataSetName(_params.data_set_name);
  mdvx.setDataSetSource(_params.data_set_source);

  mdvx.setMasterHeader(mhdr);
  
}

////////////////////////////////////////////////////
// add a field to output file

void Plain2Mdv::_addField(DsMdvx &mdvx, int fieldNum, fl32 *data)

{

  // fill in field header and vlevel header
  
  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  
  switch (_params.grid_projection) {
    
    case Params::PROJ_FLAT: {
      fhdr.proj_type = Mdvx::PROJ_FLAT;
      fhdr.proj_rotation = _params.flat_rotation;
      break;
    }

    case Params::PROJ_LATLON: {
      fhdr.proj_type = Mdvx::PROJ_LATLON;
      break;
    }
    
    case Params::PROJ_LAMBERT_CONF: {
      fhdr.proj_type = Mdvx::PROJ_LAMBERT_CONF;
      fhdr.proj_param[0] = _params.lambert_lat1;
      fhdr.proj_param[1] = _params.lambert_lat2;
      break;
    }
    
    case Params::PROJ_POLAR_STEREO: {
      fhdr.proj_type = Mdvx::PROJ_POLAR_STEREO;
      fhdr.proj_param[0] = _params.polar_stereo_tangent_lon;
      fhdr.proj_param[1] =
        _params.grid_geom.miny >= 0 ? Mdvx::POLE_NORTH : Mdvx::POLE_SOUTH;
      break;
    }

    case Params::PROJ_POLAR_RADAR: {
      fhdr.proj_type = Mdvx::PROJ_POLAR_RADAR;
      break;
    }

  }

  fhdr.proj_origin_lat = _params.grid_origin_lat;
  fhdr.proj_origin_lon = _params.grid_origin_lon;
  fhdr.nx = _params.grid_size.nx;
  fhdr.ny = _params.grid_size.ny;
  fhdr.nz = _params.grid_size.nz;
  fhdr.grid_dx = _params.grid_geom.dx;
  fhdr.grid_dy = _params.grid_geom.dy;
  fhdr.grid_minx = _params.grid_geom.minx;
  fhdr.grid_miny = _params.grid_geom.miny;

  if (_params.vlevels_n > 1) {
    fhdr.grid_dz = _params._vlevels[1] - _params._vlevels[0];
    fhdr.grid_minz = _params._vlevels[0];
  } else if (_params.vlevels_n > 0) {
    fhdr.grid_dz = 1.0;
    fhdr.grid_minz = _params._vlevels[0];
  } else {
    fhdr.grid_dz = 1.0;
    fhdr.grid_minz = 0.0;
  }
      
  // int npointsPlane = fhdr.nx * fhdr.ny;
    
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;

  fhdr.native_vlevel_type = _params.vlevel_type;
  fhdr.vlevel_type = _params.vlevel_type;
  fhdr.dz_constant = false;

  fhdr.bad_data_value = _missingFloat;
  fhdr.missing_data_value = _missingFloat;
  
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(fl32);

  // vlevel header
  
  for (int iz = 0; iz < fhdr.nz; iz++) {
    vhdr.type[iz] = _params.vlevel_type;
    vhdr.level[iz] = _params._vlevels[iz];
  }

  // create field
  
  MdvxField *field = new MdvxField(fhdr, vhdr, data);

  // set name etc
  
  field->setFieldName(_params._fields[fieldNum].name);
  field->setFieldNameLong(_params._fields[fieldNum].name_long);
  field->setUnits(_params._fields[fieldNum].units);
  field->setTransform(_params._fields[fieldNum].transform);
  
  field->convertRounded((Mdvx::encoding_type_t) _params.output_encoding,
                        (Mdvx::compression_type_t) _params.output_compression);

  // add field to mdvx object

  mdvx.addField(field);

}

////////////////////////////////////////////////////
// write output data
//
// returns 0 on success, -1 on failure

int Plain2Mdv::_writeOutput(DsMdvx &mdvx)

{

  // write out

  if (mdvx.writeToDir(_params.output_url)) {
    cerr << "ERROR - Plain2Mdv::_writeOutput" << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Wrote file: " << mdvx.getPathInUse() << endl;
  }

  return 0;
    
}

