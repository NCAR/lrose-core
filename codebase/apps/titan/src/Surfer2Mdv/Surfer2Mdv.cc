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
// Surfer2Mdv.cc
//
// Surfer2Mdv object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// September 1999
//
///////////////////////////////////////////////////////////////

#include <iomanip>
#include <cerrno>
#include <string>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/MemBuf.hh>
#include <toolsa/TaFile.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxFieldCode.hh>
#include "Surfer2Mdv.hh"
using namespace std;

// Constructor

Surfer2Mdv::Surfer2Mdv(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "Surfer2Mdv";
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

  return;

}

// destructor

Surfer2Mdv::~Surfer2Mdv()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Surfer2Mdv::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // loop through input files

  for (size_t i = 0; i < _args.inputFileList.size(); i++) {

    PMU_auto_register("In main loop");
    
    if (_processFile(_args.inputFileList[i])) {
      cerr << "ERROR - Surfer2Mdv::Run" << endl;
      cerr << "  Processing file: " << _args.inputFileList[i] << endl;
      return -1;
    }
    
  } // i

  return 0;

}

//////////////////////////////////////////////////
// processs this file

int Surfer2Mdv::_processFile(const string &inputPath)

{

  if (_params.debug) {
    cerr << "======================================================" << endl;
    cerr << "  processing file: " << inputPath << endl;
  }

  // compute date and time from path

  time_t startTime, midTime, endTime;

  if (_computeTime(inputPath, startTime, midTime, endTime)) {
    cerr << "ERROR - Surfer2Mdv::_processFile" << endl;
    cerr << "  Cannot parse file name for date and times" << endl;
    cerr << "  File path: " << inputPath << endl;
    return -1;
  }

  // read the file

  int nx, ny;
  double minx, miny;
  double dx, dy;
  MemBuf dataBuf;

  if (_readInput(inputPath,
		 nx, ny,
		 minx, miny,
		 dx, dy,
		 dataBuf)) {
    cerr << "ERROR - Surfer2Mdv::_processFile" << endl;
    cerr << "  Cannot read file" << endl;
    cerr << "  File path: " << inputPath << endl;
    return -1;
  }

  // convert the data

  _convertData(dataBuf);

  // write out the Mdv file

  if (_writeOutput(startTime, midTime, endTime,
		   nx, ny,
		   minx, miny,
		   dx, dy,
		   dataBuf)) {
    cerr << "ERROR - Surfer2Mdv::_processFile" << endl;
    cerr << "  Cannot write output" << endl;
    return -1;
  }

  return 0;
}


//////////////////////////////////////////////////
// compute time from the file path

int Surfer2Mdv::_computeTime(const string &inputPath,
			     time_t &start_time,
			     time_t &mid_time,
			     time_t &end_time)

{

  // compute date and time from path

  size_t hyphen1 = inputPath.find("-", 0);
  size_t hyphen2 = inputPath.find("-", hyphen1);
  size_t hyphen3 = inputPath.find("-", hyphen2);
  size_t hyphen4 = inputPath.find("-", hyphen3);
  size_t hyphen5 = inputPath.find("-", hyphen4);

  if (hyphen1 == string::npos ||
      hyphen2 == string::npos ||
      hyphen3 == string::npos ||
      hyphen4 == string::npos ||
      hyphen5 == string::npos) {
    cerr << "ERROR - Surfer2Mdv::_computeTime" << endl;
    cerr << "  Cannot parse file name for date and times" << endl;
    cerr << "  File path: " << inputPath << endl;
    cerr << "  Should be at least 5 hyphens in the name" << endl;
    return -1;
  }

  const char *startPos = inputPath.c_str() + hyphen1 + 1;

  date_time_t startTime, endTime;
  MEM_zero(startTime);
  MEM_zero(endTime);

  if (sscanf(startPos, "%2d%2d%4d-%2d%2d-to-%2d%2d%4d-%2d%2d",
	     &startTime.month, &startTime.day, &startTime.year,
	     &startTime.hour, &startTime.min,
	     &endTime.month, &endTime.day, &endTime.year,
	     &endTime.hour, &endTime.min) != 10) {
    if (sscanf(startPos, "%2d%2d%4d-%2d%2d",
	       &startTime.month, &startTime.day, &startTime.year,
	       &startTime.hour, &startTime.min) == 5) {
      endTime = startTime;
    } else {
      cerr << "ERROR - Surfer2Mdv::_computeTime" << endl;
      cerr << "  Cannot parse file name for date and times" << endl;
      cerr << "  File: " << inputPath << endl;
      return -1;
    }
  }

  uconvert_to_utime(&startTime);
  uconvert_to_utime(&endTime);

  start_time = startTime.unix_time;
  end_time = endTime.unix_time;
  mid_time = (start_time + end_time) / 2;
  
  if (_params.debug) {
    cerr << "  start time: " << utimstr(start_time) << endl;
    cerr << "  mid time: " << utimstr(mid_time) << endl;
    cerr << "  end time: " << utimstr(end_time) << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// read input file

int Surfer2Mdv::_readInput(const string &inputPath,
			   int &nx, int &ny,
			   double &minx, double &miny,
			   double &dx, double &dy,
			   MemBuf &dataBuf)

{

  // open the file

  TaFile in;

  if (in.fopen(inputPath.c_str(), "r") == NULL) {
    int errNum = errno;
    cerr << "ERROR - Surfer2Mdv::_readInput" << endl;
    cerr << "  Cannot open file for reading: " << inputPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // read in header info
  
  FILE *inFile = in.getFILE();

  char label[64];
  double minLon, maxLon;
  double minLat, maxLat;
  double minDataVal, maxDataVal;

  if (fscanf(inFile, "%s%d%d%lf%lf%lf%lf%lf%lf",
	     label,
	     &nx, &ny,
	     &minLon, &maxLon,
	     &minLat, &maxLat,
	     &minDataVal, &maxDataVal) != 9) {
    cerr << "ERROR - Surfer2Mdv::_readInput" << endl;
    cerr << "  Cannot parse file header" << endl;
    cerr << "  File: " << inputPath << endl;
    return -1;
  }

  // compute grid info

  dx = (maxLon - minLon) / (nx - 1.0);
  dy = (maxLat - minLat) / (ny - 1.0);
  minx = minLon - dx / 2.0;
  miny = minLat - dy / 2.0;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  label: " << label << endl;
    cerr << "  nx, ny: " << nx << ", " << ny << endl;
    cerr << "  minLon, maxLon: "
	 << setprecision(8) << minLon << ", "
	 << setprecision(8) << maxLon << endl;
    cerr << "  minLat, maxLat: "
	 << setprecision(8) << minLat << ", "
	 << setprecision(8) << maxLat << endl;
    cerr << "  minDataVal, maxDataVal: "
	 << minDataVal << ", " << maxDataVal << endl;
    cerr << "  dx, dy: " << dx << ", " << dy << endl;
    cerr << "  minx, miny: " << minx << ", " << miny << endl;
  }

  // read in array

  dataBuf.free();
  double dataVal;
  int npoints = nx * ny;

  for (int i = 0; i < npoints; i++) {
    if (fscanf(inFile, "%lf", &dataVal) != 1) {
      cerr << "ERROR - Surfer2Mdv::_readInput" << endl;
      cerr << "  Cannot parse file data array" << endl;
      cerr << "  File: " << inputPath << endl;
      cerr << "  i: " << i << endl;
      return -1;
    }
    fl32 fval = dataVal;
    dataBuf.add(&fval, sizeof(fl32));
  }

  in.fclose();

  return 0;

}

//////////////////////////////////////////////////
// convert the data

void Surfer2Mdv::_convertData(MemBuf &dataBuf)

{

  fl32 *fdata = (fl32 *) dataBuf.getPtr();
  int npoints = dataBuf.getLen() / sizeof(fl32);

  for (int i = 0; i < npoints; i++, fdata++) {

    double accum = *fdata;

    if (accum == 0) {
      continue;
    }

    // convert to mm
    
    if (_params.input_units == Params::INCHES) {
      accum *= 25.4;
    }

    switch (_params.output_units) {
      
    case Params::PRECIP_DEPTH: {
      *fdata = accum;
      break;
    }
    
    case Params::PRECIP_RATE: {
      double rate = (accum / _params.input_accum_period) * 3600.0;
      *fdata = rate;
      break;
    }

    case Params::DBZ: {
      double rate = (accum / _params.input_accum_period) * 3600.0;
      double z = _params.zr_coefficient * pow((double) rate,
					      (double) _params.zr_exponent);
      double dbz = 10.0 * log10(z);
      *fdata = dbz;
      break;
    }

    case Params::PRECIP_INCHES: {
      *fdata = accum / 25.4;
      break;
    }
    
    } // switch

  } // i

}

//////////////////////////////////////////////////
// write the output MDV file

int Surfer2Mdv::_writeOutput(time_t start_time,
			     time_t mid_time,
			     time_t end_time,
			     int nx, int ny,
			     double minx, double miny,
			     double dx, double dy,
			     MemBuf &dataBuf)

{

  PMU_auto_register("In Surfer2Mdv::_writeOutput");
  
  // create DsMdvx object
  
  DsMdvx mdvx;
  mdvx.setDebug(_params.debug >= Params::DEBUG_VERBOSE);
  
  // set the master header
  
  Mdvx::master_header_t mhdr;
  MEM_zero(mhdr);
  
  mhdr.time_gen = time(NULL);
  mhdr.time_begin = start_time;
  mhdr.time_end = end_time;
  mhdr.time_centroid = mid_time;
  mhdr.time_expire = end_time;
  
  mhdr.num_data_times = 1;
  mhdr.data_dimension = 3;
  
  mhdr.data_collection_type = Mdvx::DATA_MEASURED;
  mhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
  mhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  mhdr.vlevel_included = TRUE;
  mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  mhdr.data_ordering = Mdvx::ORDER_XYZ;
  mhdr.max_nx = nx;
  mhdr.max_ny = ny;
  mhdr.max_nz = 1;
  mhdr.n_chunks = 0;
  mhdr.field_grids_differ = FALSE;
  mhdr.sensor_lon = 0.0;
  mhdr.sensor_lat = 0.0;
  mhdr.sensor_alt = 0.0;

  STRncopy(mhdr.data_set_info, _params.data_set_info, MDV_INFO_LEN);
  STRncopy(mhdr.data_set_name, _params.data_set_name, MDV_NAME_LEN);
  STRncopy(mhdr.data_set_source, _params.data_set_source, MDV_NAME_LEN);

  mdvx.setMasterHeader(mhdr);
  
  // fill in field header and vlevel header
  
  mdvx.clearFields();
  
  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);

  fhdr.nx = nx;
  fhdr.ny = ny;
  fhdr.nz = 1;

  fhdr.proj_type = Mdvx::PROJ_LATLON;

  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  fhdr.volume_size = dataBuf.getLen();
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;

  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;

  fhdr.dz_constant = true;
  
  fhdr.proj_origin_lat = 0.0;
  fhdr.proj_origin_lon = 0.0;
  fhdr.grid_dx = dx;
  fhdr.grid_dy = dy;
  fhdr.grid_dz = 1.0;
  fhdr.grid_minx = minx;
  fhdr.grid_miny = miny;
  fhdr.grid_minz = 0.0;
  fhdr.proj_rotation = 0.0;
    
  fhdr.bad_data_value = 0.0;
  fhdr.missing_data_value = 0.0;

  switch (_params.output_units) {

  case Params::DBZ: {
    MdvxFieldCode::entry_t codeEntry;
    MdvxFieldCode::getEntryByAbbrev("DBZ", codeEntry);
    _setFieldName(fhdr, "DBZ", "Radar reflectivity", "dBZ",
		  "none", codeEntry.code);
    break;
  }
  
  case Params::PRECIP_DEPTH: {
    MdvxFieldCode::entry_t codeEntry;
    MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
    char str[128];
    sprintf(str, "%g-hr Precip accum", _params.input_accum_period / 3600.0);
    _setFieldName(fhdr, "PrecipAccum", str, "mm",
		  "none", codeEntry.code);
    break;
  }
    
  case Params::PRECIP_RATE: {
    MdvxFieldCode::entry_t codeEntry;
    MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
    char str[128];
    sprintf(str, "%g-hr Precip accum", _params.input_accum_period / 3600.0);
    _setFieldName(fhdr, "PrecipAccum", str, "mm",
		  "none", codeEntry.code);
    break;
  }

  case Params::PRECIP_INCHES: {
    MdvxFieldCode::entry_t codeEntry;
    MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
    char str[128];
    sprintf(str, "%g-hr Precip accum", _params.input_accum_period / 3600.0);
    _setFieldName(fhdr, "PrecipAccum", str, "in",
		  "none", codeEntry.code);
    break;
  }
    
  } // switch
      
  // vlevel header
  
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vhdr.level[0] = 0;

  // create field
  
  MdvxField *field = new MdvxField(fhdr, vhdr, dataBuf.getPtr(), true);
  
  // convert for writing

  if (_params.output_units == Params::PRECIP_DEPTH) {
    field->transform2Log();
  }

  field->convertType((Mdvx::encoding_type_t) _params.output_encoding,
		     (Mdvx::compression_type_t) _params.output_compression,
		     (Mdvx::scaling_type_t) _params.output_scaling,
		     (double) _params.output_scale,
		     (double) _params.output_bias);
  
  // add field to mdvx object
  
  mdvx.addField(field);

  // write out

  if (_params.debug) {
    cerr << "Writing MDV file, time : "
	 << utimstr(mdvx.getMasterHeader().time_centroid)
	 << " to url " << _params.output_url << endl;
  }
  
  // write to directory
  
  mdvx.setWriteLdataInfo();
  if (mdvx.writeToDir(_params.output_url)) {
    cerr << "ERROR - Surfer2Mdv::_writeOutput" << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////
// _setFieldName()
//
// Sets the field name, units etc
//

void Surfer2Mdv::_setFieldName(Mdvx::field_header_t &fhdr,
			       const char *name,
			       const char *name_long,
			       const char *units,
			       const char *transform,
			       const int field_code)
  
{

  STRncopy(fhdr.field_name, name, MDV_SHORT_FIELD_LEN);
  STRncopy(fhdr.field_name_long, name_long, MDV_LONG_FIELD_LEN);
  STRncopy(fhdr.units, units, MDV_UNITS_LEN);
  STRncopy(fhdr.transform, transform, MDV_TRANSFORM_LEN);
  fhdr.field_code = field_code;
  
}

