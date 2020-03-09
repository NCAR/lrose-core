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
// Nimrod2Mdv.cc
//
// Nimrod2Mdv object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2011
//
///////////////////////////////////////////////////////////////
//
// Nimrod2Mdv converts a UK met office NIMROD file to MDV.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/os_config.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaFile.hh>
#include <didss/DataFileNames.hh>
#include <dataport/bigend.h>
#include <dataport/swap.h>
#include <euclid/PjgMath.hh>
#include <Mdv/MdvxField.hh>
#include "Nimrod2Mdv.hh"
using namespace std;

const fl32 Nimrod2Mdv::_missingFloat = -9999.0;

// Constructor

Nimrod2Mdv::Nimrod2Mdv(int argc, char **argv)

{

  _input = NULL;
  isOK = true;

  // set programe name

  _progName = "Nimrod2Mdv";
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
    cerr << "ERROR: Nimrod2Mdv::Nimrod2Mdv." << endl;
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

  return;

}

// destructor

Nimrod2Mdv::~Nimrod2Mdv()

{

  if (_input) {
    delete _input;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Nimrod2Mdv::Run ()
{

  int iret = 0;
  PMU_auto_register("Run");
  
  // loop until end of data
  
  char *inputPath;
  while ((inputPath = _input->next()) != NULL) {
    
    PMU_auto_register("Non-simulate mode");
    
    if (_processFile(inputPath)) {
      cerr << "ERROR = Nimrod2Mdv::Run" << endl;
      cerr << "  Processing file: " << inputPath << endl;
      iret = -1;
    }
    
  }
  
  return iret;

}

///////////////////////////////
// process file

int Nimrod2Mdv::_processFile(const char *input_path)

{

  if (_params.debug) {
    cerr << "Processing file: " << input_path << endl;
  }

  // open file - file closes automatically when inFile goes
  // out of scope
  
  TaFile inFile;
  FILE *in;
  if ((in = inFile.fopen(input_path, "r")) == NULL) {
    cerr << "ERROR - Nimrod2Mdv::_processFile" << endl;
    cerr << "  Cannot open input file: " << input_path << endl;
    perror("  ");
    return -1;
  }

  // get file size in bytes

  if (inFile.stat(input_path)) {
    cerr << "ERROR - Nimrod2Mdv::_processFile" << endl;
    cerr << "  Cannot stat input file: " << input_path << endl;
    perror("  ");
    inFile.fclose();
    return -1;
  }
  const struct stat &fileStat = inFile.getStat();
  int nBytesFile = fileStat.st_size;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "file size: " << nBytesFile << endl;
    cerr << "sizeof(nimrod_hdr_int16_start_t): "
         << sizeof(nimrod_hdr_int16_start_t) << endl;
    cerr << "sizeof(nimrod_hdr_fl32_t): "
         << sizeof(nimrod_hdr_fl32_t) << endl;
    cerr << "sizeof(nimrod_hdr_char_t): "
         << sizeof(nimrod_hdr_char_t) << endl;
    cerr << "sizeof(nimrod_hdr_int16_end_t): "
         << sizeof(nimrod_hdr_int16_end_t) << endl;
    cerr << "sizeof hdr total: "
         << (sizeof(nimrod_hdr_int16_start_t) +
             sizeof(nimrod_hdr_fl32_t) +
             sizeof(nimrod_hdr_char_t) +
             sizeof(nimrod_hdr_int16_end_t)) << endl;
  }

  while (!feof(inFile.getFILE())) {

    // read in leading FORTRAN record length
    
    ui32 recLen1;
    if (inFile.fread(&recLen1, 1, sizeof(recLen1)) != sizeof(recLen1)) {
      if (feof(inFile.getFILE())) {
        inFile.fclose();
        return 0;
      }
      int errNum = errno;
      cerr << "ERROR - Nimrod2Mdv::_processFile" << endl;
      cerr << "  Cannot read start rec len from file: " << input_path << endl;
      cerr << "  " << strerror(errNum) << endl;
      inFile.fclose();
      return -1;
    }
    
    // read in headers
    
    nimrod_hdr_int16_start_t hdrIntStart;
    nimrod_hdr_fl32_t hdrFloat;
    nimrod_hdr_char_t hdrChar;
    nimrod_hdr_int16_end_t hdrIntEnd;
    
    if (inFile.fread(&hdrIntStart, 1, sizeof(hdrIntStart)) != sizeof(hdrIntStart)) {
      int errNum = errno;
      cerr << "ERROR - Nimrod2Mdv::_processFile" << endl;
      cerr << "  Cannot read hdrIntStart from file: " << input_path << endl;
      cerr << "  " << strerror(errNum) << endl;
      inFile.fclose();
      return -1;
    }
    
    if (inFile.fread(&hdrFloat, 1, sizeof(hdrFloat)) != sizeof(hdrFloat)) {
      int errNum = errno;
      cerr << "ERROR - Nimrod2Mdv::_processFile" << endl;
      cerr << "  Cannot read hdrFloat from file: " << input_path << endl;
      cerr << "  " << strerror(errNum) << endl;
      inFile.fclose();
      return -1;
    }
    
    if (inFile.fread(&hdrChar, 1, sizeof(hdrChar)) != sizeof(hdrChar)) {
      int errNum = errno;
      cerr << "ERROR - Nimrod2Mdv::_processFile" << endl;
      cerr << "  Cannot read hdrChar from file: " << input_path << endl;
      cerr << "  " << strerror(errNum) << endl;
      inFile.fclose();
      return -1;
    }
    
    if (inFile.fread(&hdrIntEnd, 1, sizeof(hdrIntEnd)) != sizeof(hdrIntEnd)) {
      int errNum = errno;
      cerr << "ERROR - Nimrod2Mdv::_processFile" << endl;
      cerr << "  Cannot read hdrIntEnd from file: " << input_path << endl;
      cerr << "  " << strerror(errNum) << endl;
      inFile.fclose();
      return -1;
    }
    
    // read in trailing FORTRAN record length
    
    ui32 recLen2;
    if (inFile.fread(&recLen2, 1, sizeof(recLen2)) != sizeof(recLen2)) {
      int errNum = errno;
      cerr << "ERROR - Nimrod2Mdv::_processFile" << endl;
      cerr << "  Cannot read trailing rec len from file: " << input_path << endl;
      cerr << "  " << strerror(errNum) << endl;
      inFile.fclose();
      return -1;
    }
    
    // byte swap

    BE_to_array_32(&recLen1, sizeof(recLen1));
    BE_to_array_16(&hdrIntStart, sizeof(hdrIntStart));
    BE_to_array_32(&hdrFloat, sizeof(hdrFloat));
    BE_to_array_16(&hdrIntEnd, sizeof(hdrIntEnd));
    BE_to_array_32(&recLen2, sizeof(recLen2));
    
    // compute valid time
    
    DateTime validTime(hdrIntStart.vt_year,
                       hdrIntStart.vt_month,
                       hdrIntStart.vt_day,
                       hdrIntStart.vt_hour,
                       hdrIntStart.vt_min,
                       hdrIntStart.vt_sec);

    // debug print
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Leading rec len: " << recLen1 << endl;
      _printHdrInt16Start(cerr, hdrIntStart);
      _printHdrFL32(cerr, hdrFloat);
      _printHdrChar(cerr, hdrChar);
      _printHdrInt16End(cerr, hdrIntEnd);
      cerr << "Trailing rec len: " << recLen2 << endl;
      cerr << "  Valid time: " << DateTime::strn(validTime.utime()) << endl;
    }

    // read in leading rec len for data field
    
    if (inFile.fread(&recLen1, 1, sizeof(recLen1)) != sizeof(recLen1)) {
      int errNum = errno;
      cerr << "ERROR - Nimrod2Mdv::_processFile" << endl;
      cerr << "  Cannot read data start rec len from file: " << input_path << endl;
      cerr << "  " << strerror(errNum) << endl;
      inFile.fclose();
      return -1;
    }
    BE_to_array_32(&recLen1, sizeof(recLen1));
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Leading rec len for data: " << recLen1 << endl;
    }

    // sanity check

    int nx = hdrIntStart.ncols;
    int ny = hdrIntStart.nrows;
    int byteWidth = hdrIntStart.byte_width;
    if (nx * ny * byteWidth != (int) recLen1) {
      cerr << "ERROR - Nimrod2Mdv::_processFile" << endl;
      cerr << "  Incorrect data buffer len: " << recLen1 << endl;
      cerr << "  Should be: " << nx * ny * byteWidth << endl;
      cerr << "  nx: " << nx << endl;
      cerr << "  ny: " << ny << endl;
      cerr << "  byteWidth: " << byteWidth << endl;
      inFile.fclose();
      return -1;
    }

    // does data start at top left of bot left

    bool startsTopLeft = (hdrIntStart.origin_loc == 0);

    // read in data

    char *inBuf = new char[recLen1];
    if (inFile.fread(inBuf, 1, recLen1) != (int) recLen1) {
      int errNum = errno;
      cerr << "ERROR - Nimrod2Mdv::_processFile" << endl;
      cerr << "  Cannot read data from file: " << input_path << endl;
      cerr << "  " << strerror(errNum) << endl;
      delete[] inBuf;
      inFile.fclose();
      return -1;
    }
    
    // read in trailing rec len for data field
    
    if (inFile.fread(&recLen2, 1, sizeof(recLen2)) != sizeof(recLen2)) {
      int errNum = errno;
      cerr << "ERROR - Nimrod2Mdv::_processFile" << endl;
      cerr << "  Cannot read data end rec len from file: " << input_path << endl;
      cerr << "  " << strerror(errNum) << endl;
      inFile.fclose();
      return -1;
    }
    BE_to_array_32(&recLen2, sizeof(recLen2));
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Trailing rec len for data: " << recLen2 << endl;
    }

    // load up output data buffer

    int nPoints = nx * ny;
    fl32 *outBuf = new fl32[nPoints];
    _loadFieldData(inBuf,
                   nx, ny, byteWidth, recLen1,
                   startsTopLeft,
                   outBuf,
                   hdrIntStart.missing_int,
                   hdrFloat.missing_float);
    
    // create output MDV object
    
    DsMdvx mdvx;

    // initialize master header
    
    char sourceStr[128];
    memset(sourceStr, 0, sizeof(sourceStr));
    memcpy(sourceStr, hdrChar.data_source, sizeof(hdrChar.data_source));
    _initMasterHeader(mdvx, validTime.utime(), sourceStr, nx, ny);

    // initialize field headers

    Mdvx::field_header_t fhdr;
    Mdvx::vlevel_header_t vhdr;
    _initFieldHeaders(fhdr, vhdr, hdrIntStart, hdrFloat, hdrChar, hdrIntEnd);

    // create field
    
    MdvxField *field = new MdvxField(fhdr, vhdr, outBuf);

    // set name etc
    
    field->setFieldName(_params.field_name);
    field->setFieldNameLong(_params.field_name_long);
    field->setUnits(_params.field_units);

    // set type and compression

    field->convertRounded((Mdvx::encoding_type_t) _params.output_encoding,
                          (Mdvx::compression_type_t) _params.output_compression);

    // add field to mdvx object
    
    mdvx.addField(field);
    
    // clean up
    
    delete[] inBuf;
    delete[] outBuf;

    // Remap the data in XY if requested
    
    if (_params.remap_xy) {
      if (_params.auto_remap_to_latlon) {
        _autoRemapToLatLon(mdvx);
      } else {
        _remap(mdvx);
      }
    }
  
    // write out file
    
    if (_writeOutput(mdvx)) {
      return -1;
    }

  } // while (!feof ...
  
  return 0;

}

////////////////////////////////////////////////////
// load up output buffer

void Nimrod2Mdv::_loadFieldData(char *inBuf,
                                int nx, int ny,
                                int byteWidth,
                                int nBytes,
                                bool startsTopLeft,
                                fl32 *outBuf,
                                int missingInt,
                                fl32 missingFloat)

{

  if (_params.debug) {
    cerr << "Loading data grid" << endl;
    cerr << "  nx: " << nx << endl;
    cerr << "  ny: " << ny << endl;
    cerr << "  byteWidth: " << byteWidth << endl;
    cerr << "  startsTopLeft: " << (startsTopLeft? "Y" : "N") << endl;
  }

  // swap input as needed

  if (byteWidth == 4) {
    BE_to_array_32(inBuf, nBytes);
  } else if (byteWidth == 2) {
    BE_to_array_16(inBuf, nBytes);
  }

  // reorder the rows in the input data as needed
  
  if (startsTopLeft) {
    int nBytesRow = nx * byteWidth;
    char *tmp = new char[nBytesRow];
    for (int iy = 0; iy < ny / 2; iy++) {
      char *source = inBuf + iy * nBytesRow;
      char *dest = inBuf + (ny - iy - 1) * nBytesRow;
      memcpy(tmp, dest, nBytesRow);
      memcpy(dest, source, nBytesRow);
      memcpy(source, tmp, nBytesRow);
    }
    delete[] tmp;
  }
  
  // convert to floats

  int nPoints = nx * ny;
  if (byteWidth == 4) {
    fl32 *inVal = (fl32 *) inBuf;
    fl32 *outVal = outBuf;
    for (int ii = 0; ii < nPoints; ii++, inVal++, outVal++) {
      if (*inVal == missingFloat) {
        *outVal = _missingFloat;
      } else {
        *outVal = *inVal;
      }
    }
  } else if (byteWidth == 2) {
    si16 *inVal = (si16 *) inBuf;
    fl32 *outVal = outBuf;
    for (int ii = 0; ii < nPoints; ii++, inVal++, outVal++) {
      if (*inVal == missingInt) {
        *outVal = _missingFloat;
      } else {
        *outVal = *inVal;
      }
    }
  } else {
    si08 *inVal = (si08 *) inBuf;
    fl32 *outVal = outBuf;
    for (int ii = 0; ii < nPoints; ii++, inVal++, outVal++) {
      if (*inVal == missingInt) {
        *outVal = _missingFloat;
      } else {
        *outVal = *inVal;
      }
    }
  }

  // multiply by scale factor
  
  double scale = _params.data_scale_factor;
  if (fabs(scale - 1.0) > 0.00001) {
    fl32 *outVal = outBuf;
    for (int ii = 0; ii < nPoints; ii++, outVal++) {
      if (*outVal != _missingFloat) {
        *outVal = (*outVal) * scale;
      }
    }
  }
  
}

////////////////////////////////////////////////////
// init master header

void Nimrod2Mdv::_initMasterHeader(DsMdvx &mdvx, time_t dataTime,
                                   const string &dataSetSource,
                                   int nx, int ny)
  
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
  mhdr.max_nz = 1;
  mhdr.vlevel_included = TRUE;
  mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  mhdr.data_ordering = Mdvx::ORDER_XYZ;
  mhdr.max_nx = nx;
  mhdr.max_ny = ny;
  mhdr.n_chunks = 0;
  mhdr.field_grids_differ = FALSE;
  mhdr.sensor_lon = 0.0;
  mhdr.sensor_lat = 0.0;
  mhdr.sensor_alt = 0.0;

  mdvx.setDataSetInfo(_params.data_set_info);
  mdvx.setDataSetName(_params.data_set_name);
  mdvx.setDataSetSource(dataSetSource.c_str());

  mdvx.setMasterHeader(mhdr);
  
}

////////////////////////////////////////////////////
// initialize field headers

void Nimrod2Mdv::_initFieldHeaders(Mdvx::field_header_t &fhdr,
                                   Mdvx::vlevel_header_t &vhdr,
                                   nimrod_hdr_int16_start_t &hdrIntStart,
                                   nimrod_hdr_fl32_t &hdrFloat,
                                   nimrod_hdr_char_t &hdrChar,
                                   nimrod_hdr_int16_end_t &hdrIntEnd)

{

  MEM_zero(fhdr);
  MEM_zero(vhdr);
  
  fhdr.nx = hdrIntStart.ncols;
  fhdr.ny = hdrIntStart.nrows;
  fhdr.nz = 1;
  fhdr.grid_minz = 0.0;
  fhdr.grid_dz = 1.0;

  if (hdrIntStart.grid_type == 0) {
    // UK National Grid
    fhdr.proj_type = Mdvx::PROJ_TRANS_MERCATOR;
    fhdr.proj_origin_lat = 49.766111;
    fhdr.proj_origin_lon = -7.5563889;
    fhdr.proj_param[0] = 0.9996012727; // central scale
    fhdr.grid_dx = hdrFloat.delta_x / 1000.0; // km
    fhdr.grid_dy = hdrFloat.delta_y / 1000.0; // km
    fhdr.grid_minx = hdrFloat.xpos_of_bot_left / 1000.0;
    fhdr.grid_miny = hdrFloat.ypos_of_bot_left / 1000.0;
  } else if (hdrIntStart.grid_type == 1) {
    // latlon
    fhdr.proj_type = Mdvx::PROJ_LATLON;
    fhdr.grid_dx = hdrFloat.delta_x; // km
    fhdr.grid_dy = hdrFloat.delta_y; // km
    fhdr.grid_minx = hdrFloat.xpos_of_bot_left;
    fhdr.grid_miny = hdrFloat.ypos_of_bot_left;
  } else if (hdrIntStart.grid_type == 2) {
    // space view
    fhdr.proj_type = Mdvx::PROJ_VERT_PERSP;
    fhdr.proj_origin_lat = hdrFloat.origin_lat;
    fhdr.proj_origin_lon = hdrFloat.origin_lon;
    fhdr.proj_param[0] = 35786; // geostationary radius
    fhdr.grid_dx = hdrFloat.delta_x / 1000.0; // km
    fhdr.grid_dy = hdrFloat.delta_y / 1000.0; // km
    fhdr.grid_minx = hdrFloat.xpos_of_bot_left / 1000.0;
    fhdr.grid_miny = hdrFloat.ypos_of_bot_left / 1000.0;
  } else if (hdrIntStart.grid_type == 3) {
    // polar stereo
    fhdr.proj_type = Mdvx::PROJ_POLAR_STEREO;
    fhdr.proj_origin_lat = hdrFloat.origin_lat;
    fhdr.proj_origin_lon = hdrFloat.origin_lon;
    fhdr.proj_param[0] = hdrFloat.origin_lon; // tangent lon
    fhdr.proj_param[1] = 0; // pole 0=north, 1=south
    double standardLat = hdrFloat.origin_lat;
    double centralScale = (1.0 + sin(standardLat * DEG_TO_RAD)) / 2.0;
    fhdr.proj_param[2] = centralScale;
    fhdr.grid_dx = hdrFloat.delta_x / 1000.0; // km
    fhdr.grid_dy = hdrFloat.delta_y / 1000.0; // km
    // Convert the lat/lon of the bottom left corner to kilometers,
    // as needed for the field header
    PjgPolarStereoMath polar_math(hdrFloat.origin_lon, true, centralScale);
    polar_math.setOffsetOrigin(hdrFloat.origin_lat, hdrFloat.origin_lon);
    double x, y;
    polar_math.latlon2xy(hdrFloat.ypos_of_bot_left, hdrFloat.xpos_of_bot_left,
                         x, y);
    fhdr.grid_minx = x;
    fhdr.grid_miny = y;
  } else {
    // flat
    fhdr.proj_type = Mdvx::PROJ_FLAT;
    fhdr.proj_origin_lat = hdrFloat.origin_lat;
    fhdr.proj_origin_lon = hdrFloat.origin_lon;
    fhdr.grid_dx = hdrFloat.delta_x / 1000.0; // km
    fhdr.grid_dy = hdrFloat.delta_y / 1000.0; // km
    fhdr.grid_minx = hdrFloat.xpos_of_bot_left / 1000.0;
    fhdr.grid_miny = hdrFloat.ypos_of_bot_left / 1000.0;
  }

  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;

  fhdr.native_vlevel_type = _params.vlevel_type;
  fhdr.vlevel_type = _params.vlevel_type;
  fhdr.dz_constant = true;

  fhdr.bad_data_value = _missingFloat;
  fhdr.missing_data_value = _missingFloat;
  
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(fl32);

  // vlevel header
  
  vhdr.type[0] = _params.vlevel_type;
  vhdr.level[0] = fhdr.grid_minz;

}

////////////////////////////////////////////////////
// write output data
//
// returns 0 on success, -1 on failure

int Nimrod2Mdv::_writeOutput(DsMdvx &mdvx)

{

  // write out

  if (mdvx.writeToDir(_params.output_url)) {
    cerr << "ERROR - Nimrod2Mdv::_writeOutput" << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Wrote file: " << mdvx.getPathInUse() << endl;
  }

  return 0;
    
}

////////////////////////////////////////////////////
// print NIMROD headers

void Nimrod2Mdv::_printHdrInt16Start(ostream &out,
                                     nimrod_hdr_int16_start_t &hdr)

{
    
  cerr << "  vt_year: " << hdr.vt_year << endl;
  cerr << "  vt_month: " << hdr.vt_month << endl;
  cerr << "  vt_day: " << hdr.vt_day << endl;
  cerr << "  vt_hour: " << hdr.vt_hour << endl;
  cerr << "  vt_min: " << hdr.vt_min << endl;
  cerr << "  vt_sec: " << hdr.vt_sec << endl;
  cerr << "  dt_year: " << hdr.dt_year << endl;
  cerr << "  dt_month: " << hdr.dt_month << endl;
  cerr << "  dt_day: " << hdr.dt_day << endl;
  cerr << "  dt_hour: " << hdr.dt_hour << endl;
  cerr << "  dt_min: " << hdr.dt_min << endl;
  cerr << "  data_type: " << hdr.data_type << endl;
  cerr << "  byte_width: " << hdr.byte_width << endl;
  cerr << "  experiment_number: " << hdr.experiment_number << endl;
  cerr << "  grid_type: " << hdr.grid_type << endl;
  cerr << "  nrows: " << hdr.nrows << endl;
  cerr << "  ncols: " << hdr.ncols << endl;
  cerr << "  release_num: " << hdr.release_num << endl;
  cerr << "  field_code: " << hdr.field_code << endl;
  cerr << "  vert_type: " << hdr.vert_type << endl;
  cerr << "  vert_ref_level: " << hdr.vert_ref_level << endl;
  cerr << "  n_elements_from_60: " << hdr.n_elements_from_60 << endl;
  cerr << "  n_elements_from_109: " << hdr.n_elements_from_109 << endl;
  cerr << "  origin_loc: " << hdr.origin_loc << endl;
  cerr << "  missing_int: " << hdr.missing_int << endl;
  cerr << "  accum_period_min: " << hdr.accum_period_min << endl;
  cerr << "  n_model_levels: " << hdr.n_model_levels << endl;
  cerr << "  ellipsoid: " << hdr.ellipsoid << endl;

}
  
void Nimrod2Mdv::_printHdrFL32(ostream &out,
                               nimrod_hdr_fl32_t &hdr)

{
    
  cerr << "  vlevel: " << hdr.vlevel << endl;
  cerr << "  ref_vlevel: " << hdr.ref_vlevel << endl;
  cerr << "  ypos_of_first_point: " << hdr.ypos_of_first_point << endl;
  cerr << "  delta_y: " << hdr.delta_y << endl;
  cerr << "  xpos_of_first_point: " << hdr.xpos_of_first_point << endl;
  cerr << "  delta_x: " << hdr.delta_x << endl;
  cerr << "  missing_float: " << hdr.missing_float << endl;
  cerr << "  mks_scaling_factor: " << hdr.mks_scaling_factor << endl;
  cerr << "  data_offset: " << hdr.data_offset << endl;
  cerr << "  x_offset_of_model_from_grid: " << hdr.x_offset_of_model_from_grid << endl;
  cerr << "  y_offset_of_model_from_grid: " << hdr.y_offset_of_model_from_grid << endl;
  cerr << "  origin_lat: " << hdr.origin_lat << endl;
  cerr << "  origin_lon: " << hdr.origin_lon << endl;
  cerr << "  origin_easting_meters: " << hdr.origin_easting_meters << endl;
  cerr << "  origin_northing_meters: " << hdr.origin_northing_meters << endl;
  cerr << "  scale_factor_central_meridian: " << hdr.scale_factor_central_meridian << endl;
  cerr << "  ypos_of_top_left: " << hdr.ypos_of_top_left << endl;
  cerr << "  xpos_of_top_left: " << hdr.xpos_of_top_left << endl;
  cerr << "  ypos_of_top_right: " << hdr.ypos_of_top_right << endl;
  cerr << "  xpos_of_top_right: " << hdr.xpos_of_top_right << endl;
  cerr << "  ypos_of_bot_right: " << hdr.ypos_of_bot_right << endl;
  cerr << "  xpos_of_bot_right: " << hdr.xpos_of_bot_right << endl;
  cerr << "  ypos_of_bot_left: " << hdr.ypos_of_bot_left << endl;
  cerr << "  xpos_of_bot_left: " << hdr.xpos_of_bot_left << endl;
  cerr << "  sat_calib_coeff: " << hdr.sat_calib_coeff << endl;
  cerr << "  space_count: " << hdr.space_count << endl;
  cerr << "  ducting_index: " << hdr.ducting_index << endl;
  cerr << "  elevation_angle: " << hdr.elevation_angle << endl;

}

void Nimrod2Mdv::_printHdrChar(ostream &out,
                               nimrod_hdr_char_t &hdr)

{

  char text[128];
  
  memset(text, 0, sizeof(text));
  memcpy(text, hdr.units, sizeof(hdr.units));
  cerr << "  units: " << text << endl;

  memset(text, 0, sizeof(text));
  memcpy(text, hdr.data_source, sizeof(hdr.data_source));
  cerr << "  data_source: " << text << endl;

  memset(text, 0, sizeof(text));
  memcpy(text, hdr.field_title, sizeof(hdr.field_title));
  cerr << "  field_title: " << text << endl;

}

void Nimrod2Mdv::_printHdrInt16End(ostream &out,
                                   nimrod_hdr_int16_end_t &hdr)

{

  cerr << "  radar_num: " << hdr.radar_num << endl;
  cerr << "  radar_sites: " << hdr.radar_sites << endl;
  cerr << "  radar_sites2: " << hdr.radar_sites2 << endl;
  cerr << "  clut_map_num: " << hdr.clut_map_num << endl;
  cerr << "  cal_type: " << hdr.cal_type << endl;
  cerr << "  bright_band_height: " << hdr.bright_band_height << endl;
  cerr << "  bright_band_intensity: " << hdr.bright_band_intensity << endl;
  cerr << "  bright_band_test_param_1: " << hdr.bright_band_test_param_1 << endl;
  cerr << "  bright_band_test_param_2: " << hdr.bright_band_test_param_2 << endl;
  cerr << "  infill_flag: " << hdr.infill_flag << endl;
  cerr << "  stop_elevation: " << hdr.stop_elevation << endl;
  cerr << "  sat_indent: " << hdr.sat_indent << endl;
  cerr << "  meteosat_ident: " << hdr.meteosat_ident << endl;
  cerr << "  synop_availability: " << hdr.synop_availability << endl;

}

////////////////////////////////////////////
// remap

void Nimrod2Mdv::_remap(DsMdvx &mdvx)

{

  for (size_t ifld = 0; ifld < mdvx.getNFields(); ifld++) {
    
    MdvxField *field = mdvx.getField(ifld);
    
    if (field == NULL) {
      cerr << "ERROR - MdvxConvert::_remap" << endl;
      cerr << "  Error remapping field #" << ifld <<
	" in output file" << endl;
      return;
    }
    
    if (_params.remap_projection == Params::PROJ_LATLON) {
      field->remap2Latlon(_remapLut,
			  _params.remap_grid.nx,
			  _params.remap_grid.ny,
			  _params.remap_grid.minx,
			  _params.remap_grid.miny,
			  _params.remap_grid.dx,
			  _params.remap_grid.dy);
    } else if (_params.remap_projection == Params::PROJ_FLAT) {
      field->remap2Flat(_remapLut,
			_params.remap_grid.nx,
			_params.remap_grid.ny,
			_params.remap_grid.minx,
			_params.remap_grid.miny,
			_params.remap_grid.dx,
			_params.remap_grid.dy,
			_params.remap_origin_lat,
			_params.remap_origin_lon,
			_params.remap_rotation,
                        _params.remap_false_northing,
                        _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_LAMBERT_CONF)	{
      field->remap2LambertConf(_remapLut,
			       _params.remap_grid.nx,
			       _params.remap_grid.ny,
			       _params.remap_grid.minx,
			       _params.remap_grid.miny,
			       _params.remap_grid.dx,
			       _params.remap_grid.dy,
			       _params.remap_origin_lat,
			       _params.remap_origin_lon,
			       _params.remap_lat1,
			       _params.remap_lat2,
                               _params.remap_false_northing,
                               _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_POLAR_STEREO) {
      Mdvx::pole_type_t poleType = Mdvx::POLE_NORTH;
      if (!_params.remap_pole_is_north) {
	poleType = Mdvx::POLE_SOUTH;
      }
      field->remap2PolarStereo(_remapLut,
			       _params.remap_grid.nx,
			       _params.remap_grid.ny,
			       _params.remap_grid.minx,
			       _params.remap_grid.miny,
			       _params.remap_grid.dx,
			       _params.remap_grid.dy,
			       _params.remap_origin_lat,
			       _params.remap_origin_lon,
			       _params.remap_tangent_lon,
			       poleType,
			       _params.remap_central_scale,
                               _params.remap_false_northing,
                               _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_OBLIQUE_STEREO) {
      field->remap2ObliqueStereo(_remapLut,
				 _params.remap_grid.nx,
				 _params.remap_grid.ny,
				 _params.remap_grid.minx,
				 _params.remap_grid.miny,
				 _params.remap_grid.dx,
				 _params.remap_grid.dy,
				 _params.remap_origin_lat,
				 _params.remap_origin_lon,
				 _params.remap_tangent_lat,
				 _params.remap_tangent_lon,
                                 _params.remap_false_northing,
                                 _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_MERCATOR) {
      field->remap2Mercator(_remapLut,
			    _params.remap_grid.nx,
			    _params.remap_grid.ny,
			    _params.remap_grid.minx,
			    _params.remap_grid.miny,
			    _params.remap_grid.dx,
			    _params.remap_grid.dy,
			    _params.remap_origin_lat,
			    _params.remap_origin_lon,
                            _params.remap_false_northing,
                            _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_TRANS_MERCATOR) {
      field->remap2TransverseMercator(_remapLut,
				      _params.remap_grid.nx,
				      _params.remap_grid.ny,
				      _params.remap_grid.minx,
				      _params.remap_grid.miny,
				      _params.remap_grid.dx,
				      _params.remap_grid.dy,
				      _params.remap_origin_lat,
				      _params.remap_origin_lon,
				      _params.remap_central_scale,
                                      _params.remap_false_northing,
                                      _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_ALBERS) {
      field->remap2Albers(_remapLut,
			  _params.remap_grid.nx,
			  _params.remap_grid.ny,
			  _params.remap_grid.minx,
			  _params.remap_grid.miny,
			  _params.remap_grid.dx,
			  _params.remap_grid.dy,
			  _params.remap_origin_lat,
			  _params.remap_origin_lon,
			  _params.remap_lat1,
			  _params.remap_lat2,
                          _params.remap_false_northing,
                          _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_LAMBERT_AZIM) {
      field->remap2LambertAzimuthal(_remapLut,
				    _params.remap_grid.nx,
				    _params.remap_grid.ny,
				    _params.remap_grid.minx,
				    _params.remap_grid.miny,
				    _params.remap_grid.dx,
				    _params.remap_grid.dy,
				    _params.remap_origin_lat,
				    _params.remap_origin_lon,
                                    _params.remap_false_northing,
                                    _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_VERT_PERSP) {
      field->remap2VertPersp(_remapLut,
                             _params.remap_grid.nx,
                             _params.remap_grid.ny,
                             _params.remap_grid.minx,
                             _params.remap_grid.miny,
                             _params.remap_grid.dx,
                             _params.remap_grid.dy,
                             _params.remap_origin_lat,
                             _params.remap_origin_lon,
                             _params.remap_persp_radius,
                             _params.remap_false_northing,
                             _params.remap_false_easting);
     }
  }
  
}

////////////////////////////////////////////
// auto remap to latlon grid
//
// Automatically picks the grid resolution and extent
// from the existing data.

void Nimrod2Mdv::_autoRemapToLatLon(DsMdvx &mdvx)
  
{
  
  for (size_t ifld = 0; ifld < mdvx.getNFields(); ifld++) {
    
    MdvxField *field = mdvx.getField(ifld);
    
    if (field == NULL) {
      cerr << "ERROR - MdvxConvert::_autoRemapToLatLon" << endl;
      cerr << "  Error remapping field #" << ifld <<
	" in output file" << endl;
      return;
    }
    
    field->autoRemap2Latlon(_remapLut);
  }
  
}

