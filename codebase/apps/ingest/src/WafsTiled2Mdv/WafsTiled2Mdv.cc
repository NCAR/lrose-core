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
// WafsTiled2Mdv.cc
//
// WafsTiled2Mdv object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2004
//
///////////////////////////////////////////////////////////////
//
// WafsTiled2Mdv reads in raw WAFS model data files, which are
// tiled to cover the entire globe. Each file covers part of the globe.
// The files are merged and written out into a single MDV file.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <iostream>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <toolsa/Path.hh>
#include "WafsTiled2Mdv.hh"
#include "Ingester.hh"
using namespace std;

////////////////////////////////////////////////////
// Constructor

WafsTiled2Mdv::WafsTiled2Mdv(int argc, char **argv)

{

  _input = NULL;
  isOK = true;

  // set programe name
  
  _progName = "WafsTiled2Mdv";
  ucopyright((char *) _progName.c_str());

  // get command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = const_cast<char*>(string("unknown").c_str());
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  // check that file list set in archive mode
  
  if (_params.mode == Params::FILELIST && _args.inputFileList.size() == 0) {
    cerr << "ERROR: WafsTiled2Mdv::WafsTiled2Mdv." << endl;
    cerr << "  Mode is ARCHIVE."; 
    cerr << "  You must use -f to specify files on the command line."
	 << endl;
    _args.usage(_progName, cerr);
    isOK = false;
  }

  // initialize the data input path object
  
  if (_params.mode == Params::REALTIME) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _params.max_realtime_valid_age,
			     PMU_auto_register,
			     _params.latest_data_info_avail);
  } else if (_params.mode == Params::ARCHIVE) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _args.startTime, _args.endTime);
  } else {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _args.inputFileList);
  }

  // create output fields

  for (int ii = 0; ii < _params.output_fields_n; ii++) {
    _outputFields.push_back
      (new OutputField(_progName, _params,
		       _params._output_fields[ii]. mdv_name,
		       _params._output_fields[ii]. mdv_long_name,
		       _params._output_fields[ii]. mdv_units,
		       _params._output_fields[ii]. grib_field_id,
		       _params._output_fields[ii]. units_conversion,
		       _params._output_fields[ii]. encoding));
  } // ii

  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  return;

}

//////////////////////////////////////////////////////////////////
// destructor

WafsTiled2Mdv::~WafsTiled2Mdv()

{

  if (_input) {
    delete _input;
  }

  for (size_t ii = 0; ii < _outputFields.size(); ii++) {
    delete _outputFields[ii];
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int WafsTiled2Mdv::Run ()
{

  int iret = 0;
  PMU_auto_register("Run");
  
  // loop until end of data
  
  char *inputPath;
  while ((inputPath = _input->next()) != NULL) {
    
    PMU_auto_register("Processing file");
    
    if (_processFile(inputPath)) {
      cerr << "ERROR - WafsTiled2Mdv::Run" << endl;
      cerr << "  Processing file: " << inputPath << endl;
      iret = -1;
    }
    
  }
  
  return iret;

}

//////////////////////////////////////////////////
// get output field object
//
// Returns NULL on failure.

OutputField *WafsTiled2Mdv::getOutputField(const int field_id)
{
  for (size_t ii = 0; ii < _outputFields.size(); ii++) {
    if (field_id == _outputFields[ii]->getGribFieldId()) {
      return _outputFields[ii];
    }
  }
  return NULL;
}
  
///////////////////////////////
// process file

int WafsTiled2Mdv::_processFile(const char *input_path)

{

  if (_params.debug) {
    cerr << "-------------------------------------" << endl;
    cerr << "Checking trigger file: " << input_path << endl;
  }

  // determine the tile ID, which is the last letter in the path
  
  const char *end = input_path + strlen(input_path);
  char tileId = end[-1];

  // get the lead time by parsing the file name

  Path inputPath(input_path);
  const string &inputName = inputPath.getFile();
  int leadHour = -1;
  if (sscanf(inputName.c_str(), "fh.%d", &leadHour) != 1) {
    cerr << "WARNING - WafsTiled2Mdv::_processFile" << endl;
    cerr << "  Input file: " << input_path << endl;
    cerr << "    Cannot parse leadHour" << endl;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  Tile ID: " << tileId << endl;
    cerr << "  LeadHour: " << leadHour << endl;
  }

  // check if all of the tile files exist for this lead time
  // just replace the last character in the file name to
  // check for all tiles
  
  vector<string> pathSet;
  bool allExist = true;
  time_t now = time(NULL);
  for (int ii = 0; ii < _params.tiles_n; ii++) {
    char *id = _params._tiles[ii].id;
    char path[MAX_PATH_LEN];
    strcpy(path, input_path);
    char *pend = path + strlen(path);
    pend[-1] = id[0];
    struct stat fileStat;
    if (!Path::stat(path, fileStat)) {
      allExist = false;
      break;
    }
    int age = now - fileStat.st_mtime;
    if (age > _params.max_realtime_valid_age) {
      allExist = false;
      break;
    }
    pathSet.push_back(path);
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    if (allExist) {
      cerr << "--->> All files exist, ready to process" << endl;
    } else {
      cerr << "--->> Not all files exist, wait" << endl;
    }
  }

  if (allExist) {
    if (_processFileSet(pathSet)) {
      return -1;
    }
  }

  return 0;

}

///////////////////////////////
// process path set

int WafsTiled2Mdv::_processFileSet(const vector<string> &path_set)

{
  
  int iret = 0;

  // clear the output fields

  for (size_t ii = 0; ii < _outputFields.size(); ii++) {
    _outputFields[ii]->clear();
  }

  // read in all of the files
  // as each file is read, a pointer to the FieldTile is added
  // to the relevant _outputFields object

  int forecastLeadTime = -1;
  time_t genTime = -1;

  vector<Ingester *> ingesters;
  for (size_t ii = 0; ii < path_set.size(); ii++) {
    Ingester *ingester = new Ingester(*this, _progName, _params);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "------>> Degribbing file: " << path_set[ii] << endl;
    }
    ingester->readFile(path_set[ii]);
    ingesters.push_back(ingester);
    forecastLeadTime = ingester->getForecastTime();
    genTime = ingester->getGenerateTime();
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    for (size_t ii = 0; ii < _outputFields.size(); ii++) {
      _outputFields[ii]->print(cerr);
      _outputFields[ii]->printFieldTiles(cerr);
    }
  }

  // assemble each output field, using the pointers to
  // the FieldTile in each one

  for (size_t ii = 0; ii < _outputFields.size(); ii++) {
    _outputFields[ii]->assemble();
  }

  // derive WSPD field if applicable

  OutputField *wspd = getOutputField(Params::WSPD);
  if (wspd != NULL) {
    OutputField *uu = getOutputField(Params::UGRD);
    OutputField *vv = getOutputField(Params::VGRD);
    if (uu != NULL && vv != NULL) {
      wspd->deriveWspd(*uu, *vv);
    } else {
      cerr << "ERROR - " << _progName << endl;
      cerr << "  You have specified the WSPD field" << endl;
      cerr << "  This must be computed from UGRD and VGRD" << endl;
      cerr << "  Therefore you must also specify UGRD and VGRD.";
    }
  }

  // interpolate vlevels as appropriate

  for (size_t ii = 0; ii < _outputFields.size(); ii++) {
    _outputFields[ii]->interpVlevels();
  }

  // create the Mdvx output object, and load up the master header
 
  DsMdvx mdv;
  _loadMasterHeader(mdv, genTime, forecastLeadTime);

  // add the fields
  // mdvx object takes ownership of the memory for the fields

  for (size_t ii = 0; ii < _outputFields.size(); ii++) {
    MdvxField *fld =
      _outputFields[ii]->createMdvxField(genTime, forecastLeadTime);
    if (fld != NULL) {
      mdv.addField(fld);
    }
  }

  // write out Mdv file

  if (_params.output_path_in_forecast_format) {
    mdv.setWriteAsForecast();
  }
  if (mdv.writeToDir(_params.output_url)) {
    cerr << "ERROR - " << _progName << endl;
    cerr << "  Cannot write MDV file to url: " << _params.output_url << endl;
    cerr << "  " << mdv.getErrStr() << endl;
    iret = -1;
  }
  if (_params.debug) {
    cerr << "Wrote file: " << mdv.getPathInUse() << endl;
  }

  // clean up ingesters

  for (size_t ii = 0; ii < ingesters.size(); ii++) {
    delete ingesters[ii];
  }
  
  return iret;

}

///////////////////////////////////////////
// load up master header in MDV output file

void WafsTiled2Mdv::_loadMasterHeader(DsMdvx &mdv,
				      time_t gen_time,
				      int forecast_lead_time)

{

  Mdvx::master_header_t mhdr;
  MEM_zero(mhdr);
  
  mhdr.time_gen = gen_time;
  mhdr.time_centroid = gen_time + forecast_lead_time;
  mhdr.time_begin = mhdr.time_centroid;
  mhdr.time_end = mhdr.time_centroid;
  mhdr.time_expire = -1;
  mhdr.num_data_times = 1;
  mhdr.index_number = 1;
  mhdr.data_dimension = 3;
  mhdr.data_collection_type = Mdvx::DATA_FORECAST;
  mhdr.native_vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
  mhdr.vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
  mhdr.vlevel_included = true;
  mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  mhdr.data_ordering = Mdvx::ORDER_XYZ;
  mhdr.n_fields = (int) _outputFields.size();

  mdv.setMasterHeader(mhdr);
  mdv.setDataSetInfo(_params.data_set_info);
  mdv.setDataSetName(_params.data_set_name);
  mdv.setDataSetSource(_params.data_set_source);

}

  


