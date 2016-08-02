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
// Mike Dixon, EOL, NCAR, P.O.Box 3000,
// Boulder, CO, 80307-3000, USA
//
// Sept 2012
//
///////////////////////////////////////////////////////////////
//
// cedric2mdv converts Cedric-format files to MDV.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <rapformats/Cedric.hh>
#include <Mdv/MdvxField.hh>
#include "cedric2mdv.hh"
using namespace std;

const fl32 cedric2mdv::_missingFl32 = -9999.0;

// Constructor

cedric2mdv::cedric2mdv(int argc, char **argv)
  
{
  
  _input = NULL;
  isOK = true;

  // set programe name

  _progName = "cedric2mdv";
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

  // check that file list set in filelist mode
  
  if (_params.mode == Params::FILELIST && _args.inputFileList.size() == 0) {
    cerr << "ERROR: cedric2mdv::cedric2mdv." << endl;
    cerr << "  Mode is FILELIST"; 
    cerr << "  You must use -f to specify files on the command line."
	 << endl;
    _args.usage(_progName, cerr);
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

  if (_params.negate_latitude) {
    _latSign = -1.0;
  } else {
    _latSign = 1.0;
  }

  if (_params.negate_longitude) {
    _lonSign = -1.0;
  } else {
    _lonSign = 1.0;
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
  } else if (_params.mode == Params::ARCHIVE) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _args.startTime,
                             _args.endTime);
  } else if (_params.mode == Params::FILELIST) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _args.inputFileList);
  }
  _input->setSearchExt(_params.input_file_extension);

  return;

}

// destructor

cedric2mdv::~cedric2mdv()

{

  if (_input) {
    delete _input;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int cedric2mdv::Run ()
{

  int iret = 0;
  PMU_auto_register("Run");
  
  // loop until end of data
  
  char *inputPath;
  while ((inputPath = _input->next()) != NULL) {
    
    PMU_auto_register("Non-simulate mode");
    
    if (_processFile(inputPath)) {
      cerr << "ERROR = cedric2mdv::Run" << endl;
      cerr << "  Processing file: " << inputPath << endl;
      iret = -1;
    }
    
  }
  
  return iret;

}

///////////////////////////////
// process file

int cedric2mdv::_processFile(const char *input_path)

{

  if (_params.debug) {
    cerr << "Processing file: " << input_path << endl;
  }

  // read in the cedric data

  Cedric ced;
  if (ced.readFromPath(input_path)) {
    cerr << "ERROR - cedric2mdv::_processFile" << endl;
    cerr << "  Cannot read in path: " << input_path << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    ced.print(cerr);
  }

  // create output file object, initialize master header

  DsMdvx mdvx;
  if (_params.debug) {
    mdvx.setDebug(true);
  }
  _initMasterHeader(mdvx, ced);

  // add fields

  if (_params.specify_output_fields) {

    // process specified fields

    for (int ifield = 0; ifield < _params.output_fields_n; ifield++) {

      const Params::output_field_t &ofld = _params._output_fields[ifield];
      int cedFieldNum = ced.getFieldNum(ofld.cedric_name);
      if (cedFieldNum >= 0) {
        fl32 *fieldData = ced.getFieldData(cedFieldNum, _missingFl32);
        _addField(mdvx, ced,
                  ofld.output_name,
                  ofld.units,
                  fieldData);
        delete[] fieldData;
      } else {
        cerr << "WARNING - cedric2mdv::_processFile" << endl;
        cerr << "  Cannot find field in cedric file" << endl;
        cerr << "  path: " << input_path << endl;
        cerr << "  field name: " << ofld.cedric_name << endl;
        cerr << "  ignoring this field" << endl;
      }

    } // ifield

  } else {

    // output all fields

    for (int ifield = 0; ifield < ced.getNumFields(); ifield++) {
      string name = ced.getFieldName(ifield);
      // convert to lower case
      string nameLower(name);
      for (size_t ii = 0; ii < name.size(); ii++) {
        nameLower[ii] = tolower(name[ii]);
      }
      string units = _guessUnits(nameLower);
      fl32 *fieldData = ced.getFieldData(ifield, _missingFl32);
      _addField(mdvx, ced, name, units, fieldData);
      delete[] fieldData;
      
    } // ifield

  } // if (_params.specify_output_fields) {

  // write out file
  
  if (_writeOutput(mdvx)) {
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////
// init master header

void cedric2mdv::_initMasterHeader(DsMdvx &mdvx, const Cedric &ced)

{

  // determine data time
  
  if (_params.debug) {
    cerr << "  Data time: " << DateTime::strm(ced.getVolStartTime()) << endl;
  }

  // set the master header
  
  Mdvx::master_header_t mhdr;
  MEM_zero(mhdr);
  
  mhdr.time_begin = ced.getVolStartTime();
  mhdr.time_end = ced.getVolEndTime();
  mhdr.time_centroid = mhdr.time_end;
    
  mhdr.num_data_times = 1;
  mhdr.data_dimension = 3;
  
  mhdr.data_collection_type = -1;
  mhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
  if (ced.getCoordType() == "ELEV" ||
      ced.getCoordType() == "LLE") {
    mhdr.vlevel_type = Mdvx::VERT_TYPE_ELEV;
  } else {
    mhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  }
  mhdr.max_nz = ced.getNz();
  mhdr.vlevel_included = TRUE;
  mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  mhdr.data_ordering = Mdvx::ORDER_XYZ;
  mhdr.max_nx = ced.getNx();
  mhdr.max_ny = ced.getNy();
  mhdr.n_chunks = 0;
  mhdr.field_grids_differ = FALSE;
  mhdr.sensor_lon = ced.getLongitudeDeg() * _lonSign;
  mhdr.sensor_lat = ced.getLatitudeDeg() * _latSign;
  mhdr.sensor_alt = ced.getOriginHtM() / 1000.0;

  mdvx.setDataSetInfo(_params.data_set_info);
  mdvx.setDataSetName(ced.getVolumeLabel().c_str());
  mdvx.setDataSetSource(_params.data_set_source);

  mdvx.setMasterHeader(mhdr);
  
}

////////////////////////////////////////////////////
// add a field to output file

void cedric2mdv::_addField(DsMdvx &mdvx,
                           const Cedric &ced,
                           const std::string &name,
                           const std::string &units,
                           const fl32 *data)

{

  if (_params.debug) {
    cerr << "  adding field name: " << name << ", " << units << endl;
  }

  // fill in field header and vlevel header
  
  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  
  string coordType = ced.getCoordType();
  bool latlon = false;
  if (coordType == "LLE" || coordType == "LLZ") {
    latlon = true;
  }
  
  if (latlon) {
    fhdr.proj_type = Mdvx::PROJ_LATLON;
  } else {
    fhdr.proj_type = Mdvx::PROJ_FLAT;
    fhdr.proj_rotation = 0.0;
  }
  
  fhdr.proj_origin_lat = ced.getLatitudeDeg() * _latSign;
  fhdr.proj_origin_lon = ced.getLongitudeDeg() * _lonSign;
  fhdr.nx = ced.getNx();
  fhdr.ny = ced.getNy();
  fhdr.nz = ced.getNz();
  fhdr.grid_dx = ced.getDx();
  fhdr.grid_dy = ced.getDy();
  fhdr.grid_minx = ced.getMinX();
  fhdr.grid_miny = ced.getMinY();
  
  if (ced.getNz() > 1) {
    fhdr.grid_dz = (ced.getZ(1) - ced.getZ(0)) / 1000.0;
    fhdr.grid_minz = ced.getZ(0) / 1000.0;
  } else if (ced.getNz() > 0) {
    fhdr.grid_dz = 1.0;
    fhdr.grid_minz = ced.getZ(0) / 1000.0;
  } else {
    fhdr.grid_dz = 1.0;
    fhdr.grid_minz = 0.0;
  }
      
  // int npointsPlane = fhdr.nx * fhdr.ny;
    
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;

  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;

  if (coordType == "ELEV" || coordType == "LLE") {
    fhdr.vlevel_type = Mdvx::VERT_TYPE_ELEV;
  } else {
    fhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  }

  fhdr.dz_constant = false;

  fhdr.bad_data_value = _missingFl32;
  fhdr.missing_data_value = _missingFl32;
  
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(fl32);

  // vlevel header
  
  for (int iz = 0; iz < fhdr.nz; iz++) {
    vhdr.type[iz] = fhdr.vlevel_type;
    vhdr.level[iz] = ced.getZ(iz) / 1000.0;
  }

  // create field
  
  MdvxField *field = new MdvxField(fhdr, vhdr, data);

  // set name etc
  
  field->setFieldName(name.c_str());
  field->setFieldNameLong(name.c_str());
  field->setUnits(units.c_str());
  field->setTransform("");
  
  field->convertRounded((Mdvx::encoding_type_t) _params.output_encoding,
                        Mdvx::COMPRESSION_GZIP);

  // add field to mdvx object

  mdvx.addField(field);

}

////////////////////////////////////////////////
// guess the units from the field name

string cedric2mdv::_guessUnits(const string &fieldName) 
{

  string units = "";
  if (fieldName.find("dbz") != string::npos) {
    units = "dBZ";
  } else if (fieldName.find("dz") != string::npos) {
    units = "dBZ";
  } else if (fieldName.find("vel") != string::npos) {
    units = "m/s";
  } else if (fieldName.find("wid") != string::npos) {
    units = "m/s";
  } else if (fieldName.find("sw") != string::npos) {
    units = "m/s";
  } else if (fieldName.find("db") != string::npos) {
    units = "dB";
  } else if (fieldName.find("dbm") != string::npos) {
    units = "dBm";
  } else if (fieldName.find("dm") != string::npos) {
    units = "dBm";
  } else if (fieldName.find("ph") != string::npos) {
    units = "deg";
  } else if (fieldName.find("kd") != string::npos) {
    units = "deg/km";
  } else if (fieldName.find("time") != string::npos) {
    units = "s";
  } else if (fieldName.find("conv") != string::npos) {
    units = "/s";
  } else if (fieldName.find("vor") != string::npos) {
    units = "/s";
  } else if (fieldName[0] == 'u') {
    units = "m/s";
  } else if (fieldName[0] == 'v') {
    units = "m/s";
  } else if (fieldName[0] == 'w') {
    units = "m/s";
  }
  return units;

}

////////////////////////////////////////////////////
// write output data
//
// returns 0 on success, -1 on failure

int cedric2mdv::_writeOutput(DsMdvx &mdvx)

{

  // write out

  if (mdvx.writeToDir(_params.output_dir)) {
    cerr << "ERROR - cedric2mdv::_writeOutput" << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Wrote file: " << mdvx.getPathInUse() << endl;
  }

  return 0;
    
}

