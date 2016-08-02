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
// McidasNcf2Mdv.cc
//
// McidasNcf2Mdv object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2008
//
///////////////////////////////////////////////////////////////
//
// McidasNcf2Mdv reads McIdas data in NetCDF format, and
// converts to MDV
//
////////////////////////////////////////////////////////////////

#include <toolsa/toolsa_macros.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/TaArray.hh>
#include <Mdv/MdvxField.hh>
#include "McidasNcf2Mdv.hh"
#include "DataSet.hh"
using namespace std;

const fl32 McidasNcf2Mdv::_missingFloat = -9999.0;

// Constructor

McidasNcf2Mdv::McidasNcf2Mdv(int argc, char **argv)

{

  isOK = true;
  _input = NULL;

  // set programe name

  _progName = "McidasNcf2Mdv";
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

  // check that file list set in archive mode
  
  if (_params.mode == Params::FILELIST && _args.inputFileList.size() == 0) {
    cerr << "ERROR: McidasNcf2Mdv" << endl;
    cerr << "  Mode is ARCHIVE."; 
    cerr << "  You must use -f to specify files on the command line."
	 << endl;
    _args.usage(_progName, cerr);
    isOK = false;
  }
    
  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		_params.reg_interval);

  // initialize the data input object
  
  if (_params.mode == Params::REALTIME) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _params.max_realtime_age,
			     PMU_auto_register,
			     _params.use_ldata_info_file,
			     _params.process_latest_file_only);
  } else if (_params.mode == Params::ARCHIVE) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _args.startTime, _args.endTime);
  } else if (_params.mode == Params::FILELIST) {
    if (_params.debug) {
      for (int ii = 0; ii < (int) _args.inputFileList.size(); ii++) {
        cerr << "Adding path: " << _args.inputFileList[ii] << endl;
      }
    }
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _args.inputFileList);
  }

  if (strlen(_params.file_name_ext) > 0) {
    _input->setSearchExt(_params.file_name_ext);
  }

  if (strlen(_params.file_name_substr) > 0) {
    _input->setSubString(_params.file_name_substr);
  }

  // initialize data sets
  
  for (int ii = 0; ii < _params.datasets_n; ii++) {
    const Params::dataset_t &set = _params._datasets[ii];
    DataSet *dset = new DataSet(_params,
				set.file_name_sub_string,
				set.field_name,
				set.field_name_long,
				set.units,
				set.data_scale,
				set.data_offset,
				set.grid_nx,
				set.grid_ny,
				set.grid_minx,
				set.grid_miny,
				set.grid_dx,
				set.grid_dy,
				set.output_url);
    _dataSets.push_back(dset);
  }
  
}

// destructor

McidasNcf2Mdv::~McidasNcf2Mdv()

{

  // unregister process

  PMU_auto_unregister();

  // free up

  if (_input) {
    delete _input;
  }

  for (int ii = 0; ii < (int) _dataSets.size(); ii++) {
    delete _dataSets[ii];
  }

}

//////////////////////////////////////////////////
// Run

int McidasNcf2Mdv::Run ()
{
  
  int iret = 0;
  PMU_auto_register("Run");
  
  // loop until end of data
  
  char *inputPath;
  while ((inputPath = _input->next()) != NULL) {
    
    PMU_auto_register("Reading file");
    ta_file_uncompress(inputPath);
    if (_processFile(inputPath)) {
      cerr << "ERROR = McidasNcf2Mdv::Run" << endl;
      cerr << "  Processing file: " << inputPath << endl;
      iret = -1;
    }
    
  }
  
  return iret;

}

///////////////////////////////
// process file

int McidasNcf2Mdv::_processFile(const char *input_path)

{

  if (_params.debug) {
    cerr << "Processing file: " << input_path << endl;
  }

  // find the first appropriate data set
  // and use that to process the file

  for (int ii = 0; ii < (int) _dataSets.size(); ii++) {
    if (_dataSets[ii]->appropriateForFile(input_path)) {
      if (_dataSets[ii]->processFile(input_path)) {
	cerr << "ERROR - McidasNcf2Mdv::_processFile" << endl;
	cerr << "  processing file: " << input_path << endl;
	return -1;
      }
      return 0;
    }
  }

  return -1;
  
}
