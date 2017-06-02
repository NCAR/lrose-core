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
// Mdv2ZebraNetCDF.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2011
//
///////////////////////////////////////////////////////////////
//
// Mdv2ZebraNetCDF converts MDV files to NetCDF format suitable
// for use in Zebra
//
///////////////////////////////////////////////////////////////

#include "Mdv2ZebraNetCDF.hh"
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/TaArray.hh>
#include <toolsa/Path.hh>
#include <didss/DsInputPath.hh>
#include <dsserver/DsLdataInfo.hh>
#include <Ncxx/Nc3File.hh>
using namespace std;

// Constructor

Mdv2ZebraNetCDF::Mdv2ZebraNetCDF(int argc, char **argv)

{

  isOK = true;
  _isLatLon = false;
  
  // set programe name

  _progName = "Mdv2ZebraNetCDF";
  ucopyright(_progName.c_str());

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

  // check that start and end time is set in archive mode

  if (_params.mode == Params::ARCHIVE) {
    if (_args.startTime == 0 || _args.endTime == 0) {
      cerr << "ERROR - must specify start and end dates." << endl << endl;
      _args.usage(_progName, cerr);
      isOK = false;
    }
  }
  
  if (!isOK) {
    return;
  }

  // input file object

  if (_params.mode == Params::FILELIST) {

    if (_args.inputFileList.size() > 0) {
      _input = new DsInputPath(_progName,
			       _params.debug >= Params::DEBUG_VERBOSE,
			       _args.inputFileList);
    } else {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  In FILELIST mode you must specify the files using -f arg." << endl;
      _args.usage(_progName, cerr);
      isOK = false;
      return;
    }

  } else if (_params.mode == Params::ARCHIVE) {
      
    if (_args.startTime != 0 && _args.endTime != 0) {
      _input = new DsInputPath(_progName,
			       _params.debug >= Params::DEBUG_VERBOSE,
			       _params.input_dir,
			       _args.startTime,
			       _args.endTime);
    } else {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  In ARCHIVE mode you must specify start and end dates." << endl << endl;
      _args.usage(_progName, cerr);
      isOK = false;
      return;
    }
      
  } else {

    // realtime mode
    
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _params.max_realtime_valid_age,
			     PMU_auto_register,
                             _params.latest_data_info_avail);
    
  } // if (_params.mode == Params::FILELIST)

  // auto register with procmap
  
  PMU_auto_init(_progName.c_str(),
                _params.instance,
                _params.procmap_register_interval);
  
}

// destructor

Mdv2ZebraNetCDF::~Mdv2ZebraNetCDF()

{

  // unregister process

  PMU_auto_unregister();
  
  // free up
  
  delete _input;

}

//////////////////////////////////////////////////
// Run

int Mdv2ZebraNetCDF::Run ()
{

  PMU_auto_register("Mdv2ZebraNetCDF::Run");

  int iret = 0;
  const char *filePath;
  while ((filePath = _input->next()) != NULL) {
    if (_processFile(filePath)) {
      iret = -1;
    }
  }

  return iret;

}

////////////////////////////
// Process the file
//
// Returns 0 on success, -1 on failure

int Mdv2ZebraNetCDF::_processFile(const char *filePath)

{

  if (_params.debug) {
    fprintf(stderr, "Processing file: %s\n", filePath);
  }

  PMU_auto_register("Processing file");

  // set up MDV object for reading
  
  DsMdvx mdvx;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    mdvx.setDebug(true);
  }
  _setupRead(mdvx);
  mdvx.setReadPath(filePath);

  if (mdvx.readVolume()) {
    cerr << "ERROR - Mdv2ZebraNetCDF::_processFile" << endl;
    cerr << "  Cannot read in data." << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }

  if (mdvx.getNFields() < 1) {
    cerr << "ERROR - Mdv2ZebraNetCDF::_processFile" << endl;
    cerr << "  No fields found in file." << endl;
    cerr << "  Path: " << mdvx.getPathInUse() << endl;
    return -1;
  }

  // remap so that all fields have the same projection

  if (_remapFields(mdvx)) {
    cerr << "ERROR - Mdv2ZebraNetCDF::_processFile" << endl;
    cerr << "  Cannot remap fields." << endl;
    cerr << "  Path: " << mdvx.getPathInUse() << endl;
    return -1;
  }

  // write out

  int iret = 0;
  if (_isLatLon) {
    iret = _writeLatLonNetCDF(mdvx);
  } else {
    iret = _writeXYNetCDF(mdvx);
  }

  return iret;

}

////////////////////////////////////////////
// set up the read

void Mdv2ZebraNetCDF::_setupRead(DsMdvx &mdvx)

{
  
  if (_params.debug) {
    mdvx.setDebug(true);
  }

  // set up the Mdvx read
  
  mdvx.clearRead();
  
  // get file headers, to save encoding and compression info
  
  mdvx.setReadFieldFileHeaders();

  // set the field list

  if (_params.set_field_names) {
    for (int ii = 0; ii < _params.fields_n; ii++) {
      mdvx.addReadField(_params._fields[ii].input_name);
    }
  }

  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    mdvx.printReadRequest(cerr);
  }

}

//////////////////////////////////////////////////////////////////
// remap the fields so all fields have the same projection and grid
// returns 0 on success, -1 on failure

int Mdv2ZebraNetCDF::_remapFields(DsMdvx &mdvx)

{

  const Mdvx::master_header_t &mhdr = mdvx.getMasterHeader();
  MdvxField *field0 = mdvx.getField(0);
  
  // remap to latlon if requested

  MdvxRemapLut lut;
  if (_params.remap_xy_to_latlon) {
    field0->autoRemap2Latlon(lut);
  }

  // set all fields to same projection

  const Mdvx::field_header_t &fhdr0 = field0->getFieldHeader();
  if (fhdr0.proj_type == Mdvx::PROJ_LATLON) {
    _isLatLon = true;
  } else {
    _isLatLon = false;
  }
  MdvxProj proj(mhdr, fhdr0);
  for (int ifield = 1; ifield < mdvx.getNFields(); ifield++) {
    MdvxField *field = mdvx.getField(ifield);
    field->remap(lut, proj);
  }

  int nz = fhdr0.nz;
  for (int ifield = 1; ifield < mdvx.getNFields(); ifield++) {
    MdvxField *field = mdvx.getField(ifield);
    if (field->getFieldHeader().nz != nz) {
      cerr << "ERROR - field nz differs, field name: "
           << field->getFieldName() << endl;
      cerr << "Field 0 has nz: " << nz << endl;
      return -1;
    }
  }
  
  return 0;

}

/////////////////////////////////
// write Zebra-style netCDF file

int Mdv2ZebraNetCDF::_writeLatLonNetCDF(const DsMdvx mdvx)
  
{

  time_t validTime = mdvx.getValidTime();
  DateTime outputTime(validTime);

  // compute dir path

  string zebDir;
  zebDir += _params.output_dir;

  if (_params.write_to_day_dir) {
    char dayDir[32];
    sprintf(dayDir, "%.4d%.2d%.2d",
            outputTime.getYear(), outputTime.getMonth(), outputTime.getDay());
    zebDir += PATH_DELIM;
    zebDir += dayDir;
  }

  // make dir

  if (ta_makedir_recurse(zebDir.c_str())) {
    int errNum = errno;
    cerr << "ERROR - Mdv2ZebraNetCDF::_writeLatLonNetCDF" << endl;
    cerr << "  Cannot make zebra output dir: " << zebDir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // compute file name
  
  char fileName[1024];
  sprintf(fileName, "%s.%.4d%.2d%.2d.%.2d%.2d%.2d.nc",
          _params.data_set_label,
          outputTime.getYear(), outputTime.getMonth(), outputTime.getDay(),
          outputTime.getHour(), outputTime.getMin(), outputTime.getSec());

  // compute file path
          
  string zebPath(zebDir);
  zebPath += PATH_DELIM;
  zebPath += fileName;

  // open file

  Nc3File zebFile(zebPath.c_str(), Nc3File::Replace, NULL, 0, Nc3File::Classic);
  if (!zebFile.is_valid()) {
    cerr << "ERROR - Mdv2ZebraNetCDF::_writeLatLonNetCDF" << endl;
    cerr << "  Cannot open netCDF file: " << zebPath << endl;
    return -1;
  }

  // create error object

  Nc3Error zebErr(Nc3Error::silent_nonfatal);

  // add global attributes

  int iret = 0;
  iret |= !zebFile.add_att("projection" , (int) 2);
  iret |= !zebFile.add_att("projection_name" , "rectangular");
  iret |= !zebFile.add_att("data_set_name", _params.data_set_name);
  iret |= !zebFile.add_att("data_set_source", _params.data_set_source);

  if (iret) {
    cerr << "ERROR - Mdv2ZebraNetCDF::_writeLatLonNetCDF" << endl;
    cerr << "  Cannot add attributes to  file: " << zebPath << endl;
    return -1;
  }

  // add time dimension

  Nc3Dim *timeDim = zebFile.add_dim("time", 1);
  if (timeDim == NULL) {
    cerr << "ERROR - Mdv2ZebraNetCDF::_writeLatLonNetCDF" << endl;
    cerr << "  Cannot add time dim to  file: " << zebPath << endl;
    return -1;
  }

  // add altitude, latitude and longitude dimensions

  MdvxField *field0 = mdvx.getField(0);
  const Mdvx::field_header_t &fhdr0 = field0->getFieldHeader();
  const Mdvx::vlevel_header_t &vhdr0 = field0->getVlevelHeader();

  Nc3Dim *altDim = zebFile.add_dim("altitude", fhdr0.nz);
  if (altDim == NULL) {
    cerr << "ERROR - Mdv2ZebraNetCDF::_writeLatLonNetCDF" << endl;
    cerr << "  Cannot add altitude dim to file: " << zebPath << endl;
    return -1;
  }

  Nc3Dim *latDim = zebFile.add_dim("latitude", fhdr0.ny);
  if (latDim == NULL) {
    cerr << "ERROR - Mdv2ZebraNetCDF::_writeLatLonNetCDF" << endl;
    cerr << "  Cannot add latitude dim to file: " << zebPath << endl;
    return -1;
  }

  Nc3Dim *lonDim = zebFile.add_dim("longitude", fhdr0.nx);
  if (lonDim == NULL) {
    cerr << "ERROR - Mdv2ZebraNetCDF::_writeLatLonNetCDF" << endl;
    cerr << "  Cannot add longitude dim to file: " << zebPath << endl;
    return -1;
  }

  // add variables

  Nc3Var *baseTimeVar = zebFile.add_var("base_time", nc3Int);
  baseTimeVar->add_att("units", "seconds since 1970-01-01 00:00:00 +0000");

  Nc3Var *timeOffsetVar = zebFile.add_var("time_offset", nc3Float, timeDim);
  timeOffsetVar->add_att("units", "seconds");
  
  //   Nc3Var *timeVar = zebFile.add_var("time", nc3Int, timeDim);
  //   timeVar->add_att("units", "seconds since 1970-01-01 00:00:00 +0000");
  //   timeVar->add_att("missing_value", -2147483647);
  //   timeVar->add_att("valid_min", -2147483646);
  //   timeVar->add_att("valid_max", 2147483647);

  Nc3Var *altVar = zebFile.add_var("altitude", nc3Float, altDim);
  altVar->add_att("units", "km");
  altVar->add_att("missing_value", -9999.0);
  altVar->add_att("valid_min", -2.0);
  altVar->add_att("valid_max", 100.0);

  Nc3Var *latVar = zebFile.add_var("latitude", nc3Float, latDim);
  latVar->add_att("units", "degrees_north");
  latVar->add_att("missing_value", -9999.0);
  latVar->add_att("valid_min", -90.0);
  latVar->add_att("valid_max", 90.0);

  Nc3Var *lonVar = zebFile.add_var("longitude", nc3Float, lonDim);
  lonVar->add_att("units", "degrees_east");
  lonVar->add_att("missing_value", -9999.0);
  lonVar->add_att("valid_min", -360.0);
  lonVar->add_att("valid_max", 360.0);

  vector<Nc3Var *> dataVars;
  for (int ifield = 0; ifield < mdvx.getNFields(); ifield++) {
    MdvxField *field = mdvx.getField(ifield);
    const Mdvx::field_header_t &fhdr = field->getFieldHeader();
    Nc3Var *dataVar =
      zebFile.add_var(field->getFieldName(),
                      nc3Float, timeDim, altDim, latDim, lonDim);
    dataVar->add_att("units", field->getUnits());
    dataVar->add_att("_FillValue", fhdr.missing_data_value);
    dataVar->add_att("missing_value", fhdr.missing_data_value);
    dataVar->add_att("valid_min", -1.0e30);
    dataVar->add_att("valid_max", 1.0e30);
    dataVars.push_back(dataVar);
  }

  // load up alt, lat and lon arrays

  TaArray<float> alts_;
  float *alts = alts_.alloc(fhdr0.nz);
  for (int ii = 0; ii < fhdr0.nz; ii++) {
    alts[ii] = vhdr0.level[ii];
  }

  TaArray<float> lats_;
  float *lats = lats_.alloc(fhdr0.ny);
  float lat = fhdr0.grid_miny;
  for (int ii = 0; ii < fhdr0.ny; ii++, lat += fhdr0.grid_dy) {
    lats[ii] = lat;
  }

  TaArray<float> lons_;
  float *lons = lons_.alloc(fhdr0.nx);
  float lon = fhdr0.grid_minx;
  for (int ii = 0; ii < fhdr0.nx; ii++, lon += fhdr0.grid_dx) {
    lons[ii] = lon;
  }

  // load up time
  
  int baseTime[1];
  baseTime[0] = (int) validTime;
  iret |= !baseTimeVar->put(baseTime, 1);
  float timeOffsets[1];
  timeOffsets[0] = 0.0;
  iret |= !timeOffsetVar->put(timeOffsets, 1);

  //   int times[1];
  //   times[0] = (int) validTime;
  //   iret |= !timeVar->put(times, 1);

  // write meta data variables

  iret |= !altVar->put(alts, fhdr0.nz);
  iret |= !latVar->put(lats, fhdr0.ny);
  iret |= !lonVar->put(lons, fhdr0.nx);

  // write the field data

  for (int ifield = 0; ifield < mdvx.getNFields(); ifield++) {
    MdvxField *field = mdvx.getField(ifield);
    const Mdvx::field_header_t &fhdr = field->getFieldHeader();
    if (_params.debug) {
      cerr << "adding field: " << field->getFieldName() << endl;
    }
    iret |= !dataVars[ifield]->put((float *) field->getVol(),
                                   1, fhdr.nz, fhdr.ny, fhdr.nx);
  }

  // close file

  zebFile.close();

  if (_params.debug) {
    cerr << "Wrote zebra nc file: " << zebPath << endl;
  }

  // check
  
  if (iret) {
    cerr << "ERROR - Mdv2ZebraNetCDF::_writeLatLonNetCDF" << endl;
    cerr << "  Cannot write netCDF file: " << zebPath << endl;
    cerr << zebErr.get_errmsg() << endl;
    return -1;
  } 

  // write latest data info file
  
  DsLdataInfo ldata(_params.output_dir, _params.debug);
  ldata.setWriter("Mdv2ZebraNetCDF");
  ldata.setDataFileExt("nc");
  ldata.setDataType("nc");
  
  string relPath;
  Path::stripDir(_params.output_dir, zebPath, relPath);
  ldata.setRelDataPath(relPath);
  ldata.write(validTime);

  return 0;

}

//////////////////////////////////////////////
// write Zebra-style netCDF file - km coords

int Mdv2ZebraNetCDF::_writeXYNetCDF(const DsMdvx mdvx)
  
{

  int iret = 0;
  time_t validTime = mdvx.getValidTime();
  DateTime outputTime(validTime);

  // compute dir path

  string zebDir;
  zebDir += _params.output_dir;

  if (_params.write_to_day_dir) {
    char dayDir[32];
    sprintf(dayDir, "%.4d%.2d%.2d",
            outputTime.getYear(), outputTime.getMonth(), outputTime.getDay());
    zebDir += PATH_DELIM;
    zebDir += dayDir;
  }

  // make dir

  if (ta_makedir_recurse(zebDir.c_str())) {
    int errNum = errno;
    cerr << "ERROR - Mdv2ZebraNetCDF::_writeLatLonNetCDF" << endl;
    cerr << "  Cannot make zebra output dir: " << zebDir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // compute file name
  
  char fileName[1024];
  sprintf(fileName, "%s.%.4d%.2d%.2d.%.2d%.2d%.2d.nc",
          _params.data_set_label,
          outputTime.getYear(), outputTime.getMonth(), outputTime.getDay(),
          outputTime.getHour(), outputTime.getMin(), outputTime.getSec());

  // compute file path
          
  string zebPath(zebDir);
  zebPath += PATH_DELIM;
  zebPath += fileName;

  if (_params.debug) {
    cerr << "Writing zebra file: " << zebPath << endl;
  }

  // open file

  Nc3File zebFile(zebPath.c_str(), Nc3File::Replace, NULL, 0, Nc3File::Classic);
  if (!zebFile.is_valid()) {
    cerr << "ERROR - Mdv2ZebraNetCDF::_writeLatLonNetCDF" << endl;
    cerr << "  Cannot open netCDF file: " << zebPath << endl;
    return -1;
  }

  // create error object

  Nc3Error zebErr(Nc3Error::silent_nonfatal);

  // add time dimension

  Nc3Dim *timeDim = zebFile.add_dim("time", 1);
  if (timeDim == NULL) {
    cerr << "ERROR - Mdv2ZebraNetCDF::_writeLatLonNetCDF" << endl;
    cerr << "  Cannot add time dim to  file: " << zebPath << endl;
    return -1;
  }

  // add z, y, z dimensions

  const Mdvx::master_header_t &mhdr = mdvx.getMasterHeader();
  MdvxField *field0 = mdvx.getField(0);
  const Mdvx::field_header_t &fhdr0 = field0->getFieldHeader();
  // const Mdvx::vlevel_header_t &vhdr0 = field0->getVlevelHeader();
  
  Nc3Dim *zDim = zebFile.add_dim("z", fhdr0.nz);
  if (zDim == NULL) {
    cerr << "ERROR - Mdv2ZebraNetCDF::_writeLatLonNetCDF" << endl;
    cerr << "  Cannot add z dim to file: " << zebPath << endl;
    return -1;
  }
  
  Nc3Dim *yDim = zebFile.add_dim("y", fhdr0.ny);
  if (yDim == NULL) {
    cerr << "ERROR - Mdv2ZebraNetCDF::_writeLatLonNetCDF" << endl;
    cerr << "  Cannot add y dim to file: " << zebPath << endl;
    return -1;
  }

  Nc3Dim *xDim = zebFile.add_dim("x", fhdr0.nx);
  if (xDim == NULL) {
    cerr << "ERROR - Mdv2ZebraNetCDF::_writeLatLonNetCDF" << endl;
    cerr << "  Cannot add x dim to file: " << zebPath << endl;
    return -1;
  }

  // add variables
  
  Nc3Var *baseTimeVar = zebFile.add_var("base_time", nc3Int);
  baseTimeVar->add_att("units", "seconds since 1970-01-01 00:00:00 +0000");

  Nc3Var *timeOffsetVar = zebFile.add_var("time_offset", nc3Float, timeDim);
  timeOffsetVar->add_att("units", "seconds since base_time");
  
  Nc3Var *latVar = zebFile.add_var("lat", nc3Float);
  latVar->add_att("units", "degrees_north");
  
  Nc3Var *lonVar = zebFile.add_var("lon", nc3Float);
  lonVar->add_att("units", "degrees_east");
  
  Nc3Var *altVar = zebFile.add_var("alt", nc3Float);
  altVar->add_att("units", "km");
  
  Nc3Var *xSpacingVar = zebFile.add_var("x_spacing", nc3Float);
  xSpacingVar->add_att("units", "km");
  
  Nc3Var *ySpacingVar = zebFile.add_var("y_spacing", nc3Float);
  ySpacingVar->add_att("units", "km");
  
  Nc3Var *zSpacingVar = zebFile.add_var("z_spacing", nc3Float);
  zSpacingVar->add_att("units", "km");
  
  vector<Nc3Var *> dataVars;
  for (int ifield = 0; ifield < mdvx.getNFields(); ifield++) {
    MdvxField *field = mdvx.getField(ifield);
    const Mdvx::field_header_t &fhdr = field->getFieldHeader();
    Nc3Var *dataVar =
      zebFile.add_var(field->getFieldName(),
                      nc3Float, timeDim, zDim, yDim, xDim);
    dataVar->add_att("units", field->getUnits());
    dataVar->add_att("_FillValue", fhdr.missing_data_value);
    dataVar->add_att("missing_value", fhdr.missing_data_value);
    dataVars.push_back(dataVar);
  }

  // load up time
  
  int baseTime[1];
  baseTime[0] = (int) validTime;
  iret |= !baseTimeVar->put(baseTime, 1);
  float timeOffsets[1];
  timeOffsets[0] = 0.0;
  iret |= !timeOffsetVar->put(timeOffsets, 1);

  // compute location of lower left grid point

  MdvxProj proj(mhdr, fhdr0);
  double xLowerLeft = fhdr0.grid_minx - fhdr0.grid_dx / 2.0;
  double yLowerLeft = fhdr0.grid_miny - fhdr0.grid_dy / 2.0;
  double latLowerLeft, lonLowerLeft;
  proj.xy2latlon(xLowerLeft, yLowerLeft,
		 latLowerLeft, lonLowerLeft);
  
  // load up geometry variables
  
  float refLat[1]; refLat[0] = latLowerLeft;
  float refLon[1]; refLon[0] = lonLowerLeft;
  float refAlt[1]; refAlt[0] = fhdr0.grid_minz;
  float xSpacing[1]; xSpacing[0] = fhdr0.grid_dx;
  float ySpacing[1]; ySpacing[0] = fhdr0.grid_dy;
  float zSpacing[1]; zSpacing[0] = fhdr0.grid_dz;
  
  iret |= !altVar->put(refAlt, 1);
  iret |= !latVar->put(refLat, 1);
  iret |= !lonVar->put(refLon, 1);
  iret |= !xSpacingVar->put(xSpacing, 1);
  iret |= !ySpacingVar->put(ySpacing, 1);
  iret |= !zSpacingVar->put(zSpacing, 1);

  // write the field data
  
  for (int ifield = 0; ifield < mdvx.getNFields(); ifield++) {
    MdvxField *field = mdvx.getField(ifield);
    const Mdvx::field_header_t &fhdr = field->getFieldHeader();
    if (_params.debug) {
      cerr << "adding field: " << field->getFieldName() << endl;
    }
    iret |= !dataVars[ifield]->put((float *) field->getVol(),
                                   1, fhdr.nz, fhdr.ny, fhdr.nx);
  }

  // close file

  zebFile.close();

  // check
  
  if (iret) {
    cerr << "ERROR - Mdv2ZebraNetCDF::_writeLatLonNetCDF" << endl;
    cerr << "  Cannot write netCDF file: " << zebPath << endl;
    cerr << zebErr.get_errmsg() << endl;
    return -1;
  } 

  // write latest data info file
  
  DsLdataInfo ldata(_params.output_dir, _params.debug);
  ldata.setWriter("Mdv2ZebraNetCDF");
  ldata.setDataFileExt("nc");
  ldata.setDataType("nc");
  
  string relPath;
  Path::stripDir(_params.output_dir, zebPath, relPath);
  ldata.setRelDataPath(relPath);
  ldata.write(validTime);

  return 0;

}

