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
// MdvMerge.cc
//
// MdvMerge object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 1998
//
///////////////////////////////////////////////////////////////

#include <algorithm>
#include <string>
#include <cstring>

#include "MdvMerge.hh"
#include "Params.hh"
#include "Args.hh"
#include "Trigger.hh"
#include "Merger.hh"
#include "MergerInt8.hh"
#include "MergerFloat32.hh"
#include "InputFile.hh"
#include "OutputFile.hh"
#include <toolsa/Path.hh>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <Mdv/MdvxProj.hh>
#include <toolsa/toolsa_macros.h>
using namespace std;

// Constructor

MdvMerge::MdvMerge(int argc, char **argv)

{

  OK = true;
  Done = false;

  // set programe name
  Path pathParts(argv[0]);
  _progName = pathParts.getBase();

  // display ucopyright message and RCSid
  ucopyright(const_cast<char*>(_progName.c_str()));

  // get command line args

  _args = new Args(argc, argv, const_cast<char*>(_progName.c_str()));
  if (!_args->OK) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    OK = false;
    return;
  }
  if (_args->Done) {
    Done = true;
    return;
  }

  // get TDRP params

  _params = new Params();
  char *params_path = (char *) "unknown";
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDPR parameters in file <" << params_path << ">" << endl;
    return;
  }

  //
  // Initialize output nowcast FMQ, if desired.
  //
  if ( _params->fire_fmq_trigger ){
    if ( _nowcastQueue.init( _params->fmq_url,
			    "MdvMerge", 
			    _params->instance,
			    _params->debug ) != 0 ) {
      cerr << "Failed to initialize FMQ " << _params->fmq_url << endl;
      exit(-1);
    }
  }

  // check start and end in ARCHIVE mode

  if ((_params->mode == Params::ARCHIVE) &&
      (_args->startTime == 0 || _args->endTime == 0)) {
    cerr << "ERROR - " <<  _progName << endl;
    cerr << "In ARCHIVE mode start and end times must be specified." << endl;
    cerr << "Run '" << _progName << " -h' for usage" << endl;
    OK = false;
  }

  if (!OK) {
    return;
  }

  // Check sizes of arrays
  int nFields;
  if (_params->set_field_nums) {
   nFields = _params->field_nums_n1;
  } else {
   nFields = _params->field_names_n1;
  }

  if (_params->use_scaling_info)
  {
    if (_params->scaling_info_n != nFields)
    {
      cerr << "ERROR - " <<  _progName << endl;
      cerr << "When using scaling_info parameter, the scaling_info array length" << endl;
      cerr << "must equal the field_list array length (" << nFields << ")." << endl << endl;
      cerr << "Fix your parameter file and try again." << endl;
      
      OK = false;
    }
    
    if (!OK)
      return;
  }
  
  if (_params->use_compression_info)
  {
    if (_params->compression_info_n != nFields)
    {
      cerr << "ERROR - " <<  _progName << endl;
      cerr << "When using compression_info parameter, the compression_info array length" << endl;
      cerr << "must equal the field_list array length (" << nFields << ")." << endl << endl;
      cerr << "Fix your parameter file and try again." << endl;
      
      OK = false;
    }
    
    if (!OK)
      return;
  }
  

  // check the field list array

  if ((nFields != 1) && 
      (nFields != _params->input_urls_n)) {
      cerr << "ERROR - " <<  _progName << endl;
      cerr << "When using field_list parameter, the field_list array length" << endl;
      cerr << "must equal the input_urls array length or one." << endl << endl;
      cerr << "Fix your parameter file and try again." << endl;
      OK = false;
  }
  if (!OK)
    return;

  // setup output grid params

  _loadGrid();

  // init process mapper registration

  PMU_auto_init(const_cast<char*>(_progName.c_str()), _params->instance, 
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

MdvMerge::~MdvMerge()

{

  // unregister process

  PMU_auto_unregister();

  // free up

  delete _params;
  delete _args;
  delete _outGrid;

}

//////////////////////////////////////////////////
// Run

int MdvMerge::Run ()
{

  PMU_auto_register("MdvMerge::Run");
  
  // create the trigger
  
  Trigger *trigger = _createTrigger();
  
  // create the Merger object
  
  Merger *merger = _createMerger();
  
  // create the input file objects

  int nInput = _params->input_urls_n;
  vector<InputFile*> inFiles;
  for (int i = 0; i < nInput; i++) {
    if (_params->set_field_nums) {
      vector<int> field_nums;
      _getFieldList(i, field_nums);
      string url = _params->_input_urls[i];
      inFiles.push_back(new InputFile(_progName, _params, _args,
				      field_nums, url,
				      (i == 0), _outGrid));
    } else {
      vector<string> field_names;
      _getFieldList(i, field_names);
      string url = _params->_input_urls[i];
      inFiles.push_back(new InputFile(_progName, _params, _args,
				      field_names, url,
				      (i == 0), _outGrid));
    }
    if(!inFiles[i]->OK()) {
      return -1;
    }
  }
  
  // create output file object

  OutputFile *outFile = new OutputFile(_progName, _params,
				       _outGrid, _nowcastQueue);
  bool outInit = false;
  
  // loop through times

  time_t triggerTime;

  while ((triggerTime = trigger->next()) >= 0) {
    
    if (_params->debug) {
      cerr << "----> Trigger time: " << utimstr(triggerTime) << endl;
    }
    
    // read relevant input files
    
    bool dataAvail = false;
    for (int i = 0; i < nInput; i++) {
      inFiles[i]->read(triggerTime);
      if (inFiles[i]->readSuccess()) {
	dataAvail = true;
	// on first successful read, initialize the output file headers
	if (!outInit) {
	  outFile->initHeaders(inFiles[i]->handle());
	  outInit = true;
	}
      }
    }
    
    if (!dataAvail) {
      continue;
    }
    
    // clear vol
    outFile->clearVol();

    // set info string
    outFile->addToInfo("Merged data set from the following files:\n");
    for (int i = 0; i < nInput; i++) {
      if (inFiles[i]->readSuccess()) {
	outFile->addToInfo((inFiles[i]->path()).c_str());
	outFile->addToInfo("\n");
      }
    }

    // set the times

    time_t startTime = -1;
    time_t endTime = -1;
    for (int i = 0; i < nInput; i++) {
      if (inFiles[i]->readSuccess()) {
	inFiles[i]->updateTimes(startTime, endTime);
      }
    } // input

    // loop through fields
    int nFields;
    if (_params->set_field_nums) {
      nFields = _params->field_nums_n2;
    } else {
      nFields = _params->field_names_n2;
    }
    
    for (int i = 0; i < nFields; i++) {

      string pmuMsg;
      pmuMsg = "Processing: " + outFile->getFieldName(i);
      
      //cerr << "Processing: " << outFile->getFieldName(i) << endl;

      PMU_force_register(pmuMsg.c_str());
      if (_params->debug)
	cerr << pmuMsg << endl;

      // compute scale and bias - if error returned there was no data in
      // this field in any of the input files, so jump to end of loop

      double out_scale, out_bias;

      if (_params->use_scaling_info)
      {
	out_scale = _params->_scaling_info[i].scale;
	out_bias = _params->_scaling_info[i].bias;
	
	outFile->loadScaleAndBias(i, out_scale, out_bias);
      }
      else
      {
	if (_computeScaleAndBias(i, nInput, inFiles,
				 out_scale, out_bias) == 0) {
	  if (_params->debug) {
	    cerr << "out_scale, out_bias: " << out_scale << ", " << out_bias << endl;
	  }
	  outFile->loadScaleAndBias(i, out_scale, out_bias);
	} else {
	  if (_params->debug) {
	    cerr << "No data found, field " << i << endl;
	  }
	  outFile->loadScaleAndBias(i, 1.0, 0.0);
	  continue;
	}
      }
      
      // for each input file, merge the field data into the output grid

      for (int j = 0; j < nInput; j++) {
	if (_params->debug)
	  cerr << "Processing merge field #" << j << endl;
	if (inFiles[j]->readSuccess()) {
	  if (_params->debug)
	    cerr << "Merging in field " << j << endl;
	  merger->mergeField(inFiles[j], i, outFile);
	  //	  inFiles[j]->mergeField(outHandle, _outGrid, i,
	  //				 out_scale, out_bias);
	}
      } // input
      
      if (_params->debug)
	cerr <<  "Finished processing: " + outFile->getFieldName(i) << endl;
    } // out_field

    // write out volume

    if (_params->debug) {
      cerr << "Writing data : triggerTime, startTime, endTime : " << endl;
      cerr << utimstr(triggerTime) << endl;
      cerr << utimstr(startTime) << endl;
      cerr << utimstr(endTime) << endl;
    }

    outFile->writeVol(triggerTime, startTime, endTime);
    outInit = false;

  } // while

  // free up

  delete trigger;
  delete merger;
  
  for (int i = 0; i < nInput; i++) {
    delete (inFiles[i]);
  }
 delete outFile;

  return (0);

}

//////////////////////////////////////////////////
// _createTrigger

Trigger *MdvMerge::_createTrigger()
{

  Trigger *trigger;

  // create the trigger

  if (_params->mode == Params::REALTIME) {
    // REALTIME mode
    if (_params->trigger == Params::TIME_TRIGGER) {
      trigger = new RealtimeTimeTrigger(const_cast<char*>(_progName.c_str()), _params);
    } else {
      trigger = new RealtimeFileTrigger(const_cast<char*>(_progName.c_str()), _params);
    }
  } else {
    // ARCHIVE mode
    if (_params->trigger == Params::TIME_TRIGGER) {
      trigger = new ArchiveTimeTrigger(const_cast<char*>(_progName.c_str()), _params,
				       _args->startTime,  _args->endTime);
    } else {
      trigger = new ArchiveFileTrigger(const_cast<char*>(_progName.c_str()), _params,
				       _args->startTime, _args->endTime);
    }
  }

  return (trigger);

}

//////////////////////////////////////////////////
// _createTrigger

Merger *MdvMerge::_createMerger()
{

  Merger *merger;

  // create the trigger

  if (_params->merge_type == Params::MERGE_INT8) {
    merger = new MergerInt8(_progName, _params);
  } else {
    merger = new MergerFloat32(_progName, _params);
  }

  return (merger);
}

//////////////////////////////////////////////////
// _loadGrid

void MdvMerge::_loadGrid()
{

 // load up grid params
  Mdvx::coord_t coords;
  coords.nx = _params->output_grid.nx;
  coords.ny = _params->output_grid.ny;
  coords.nz = _params->output_grid.nz;
  coords.minx = _params->output_grid.minx;
  coords.miny = _params->output_grid.miny;
  coords.minz = _params->output_grid.minz;
  coords.dx = _params->output_grid.dx;
  coords.dy = _params->output_grid.dy;
  coords.dz = _params->output_grid.dz;

  if (_params->output_projection == Params::OUTPUT_PROJ_FLAT) {
    coords.proj_origin_lat = _params->output_origin.lat;
    coords.proj_origin_lon = _params->output_origin.lon;
    coords.proj_params.flat.rotation = _params->output_rotation;
    coords.proj_type = Mdvx::PROJ_FLAT;
  }
  else if (_params->output_projection == Params::OUTPUT_PROJ_LAMBERT) {
    coords.proj_origin_lat = _params->output_origin.lat;
    coords.proj_origin_lon = _params->output_origin.lon;
    coords.proj_params.lc2.lat1 = _params->output_std_parallels.lat_1;
    coords.proj_params.lc2.lat2 = _params->output_std_parallels.lat_2;
    coords.proj_type = Mdvx::PROJ_LAMBERT_CONF;
  } 
  else {
    coords.proj_type = Mdvx::PROJ_LATLON;
  }

  // alloc location array for (lat, lon) pairs
  _outGrid = new MdvxProj(coords);

}

//////////////////////////////////////////////////
// _computeScaleAndBias
//
// Compute scale and bias for a field.
//
// Returns 0 on success, -1 on failure.
//

int MdvMerge::_computeScaleAndBias(const int& in_field, const int&  nInput, 
				   const vector<InputFile*>& inFiles,
				   double& scale, double& bias)

{
  
  // get min and max for field 
  
  double min_val = 1.0e99;
  double max_val = -1.0e99;
  
  for (int i = 0; i < nInput; i++) {
    if (inFiles[i]->readSuccess()) {
      double file_min, file_max;
      if (inFiles[i]->getMinAndMax(in_field, file_min, file_max) == 0) {
	min_val = min(min_val, file_min);
	max_val = max(max_val, file_max);
      }
    }
  } // i
  
  if (min_val > max_val) {
    // no valid data found
    return (-1);
  }
  
  // compute scale and bias
  double range = max_val - min_val;
  scale = range / 250.0;
  bias = min_val - scale * 4.0;
  return (0);

}


//////////////////////////////////////////////////
// _getFieldList
//
// retrieves field list for input of idx.
//
// Returns void.
//
void MdvMerge::_getFieldList(const int& idx, vector<int>& field_list)

{
  field_list.erase(field_list.begin(), field_list.end());
  int index;
  if (_params->field_nums_n1 == 1) {
    index = 0;
  }
  else {
    index = idx;
  }


  for(int i = 0; i < _params->field_nums_n2; i++) {
    field_list.push_back(_params->__field_nums[index][i]);
  }

}

void MdvMerge::_getFieldList(const int& idx, vector<string>& field_list)

{
  field_list.erase(field_list.begin(), field_list.end());
  int index;
  if (_params->field_names_n1 == 1) {
    index = 0;
  }
  else {
    index = idx;
  }


  for(int i = 0; i < _params->field_names_n2; i++) {
    field_list.push_back(_params->__field_names[index][i]);
  }

}
