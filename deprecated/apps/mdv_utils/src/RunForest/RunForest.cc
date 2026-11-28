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
/*********************************************************************
 * RunForest: RunForest program object.
 *
 * Jason Craig
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2008
 *
 *********************************************************************/

#include <iostream>
#include <string>
#include <assert.h>

#ifdef _OPENMP
#include <omp.h>
#include <pthread.h>
#endif

#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/utim.h>

#include <dsdata/DsIntervalTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>

#include <Mdv/MdvxChunk.hh>

#include "RunForest.hh"
#include "ReadForest.hh"
#include "ReadParf.hh"
#include "InputField.hh"
#include "InPolygon.hh"

// Only one of these should be un-commented
#define  _DATA3D     // Signals data is 3d and threads should work on nz variable
//#define  _DATA2D   // Signals data is 2d and threads should work on nx variable

using namespace std;

// Global variables
RunForest *RunForest::_instance = (RunForest *)NULL;


RunForest::RunForest(int argc, char **argv)
{
  static const string method_name = "RunForest::RunForest()";

  // Make sure the singleton wasn't already created.

  assert(_instance == (RunForest *)NULL);

  // Initialize the okay flag.

  okay = true;

  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;

  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);

  // display ucopyright message.

  ucopyright(_progName);

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);

  // Get TDRP parameters.

  _params = new Params();

  char params[300];
  strcpy(params, "unknown");
  char *params_ptr = params;

  if (_params->loadFromArgs(argc, argv,
                            _args->override.list,
                            &params_ptr))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with TDRP parameters in file: " << *params_ptr << endl;

    okay = false;

    return;
  }

  return;
}


RunForest::~RunForest()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  if(_input)
    delete _input;
  if(_dataTrigger)
    delete _dataTrigger;
  delete _params;
  delete _args;

  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

RunForest *RunForest::Inst(int argc, char **argv)
{
  if (_instance == (RunForest *)NULL)
    new RunForest(argc, argv);

  return(_instance);
}

RunForest *RunForest::Inst()
{
  assert(_instance != (RunForest *)NULL);

  return(_instance);
}



/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */
bool RunForest::init()
{
  static const string method_name = "RunForest::init()";

  _nx = _params->grid_info.nx;
  _ny = _params->grid_info.ny;
  _nz = _params->output_nz;
  

  // Initialize the data trigger

  if(_params->input_list_n == 0) {
    cerr << "ERROR: " << method_name << endl;
    cerr << "No input Mdv urls specified." << endl;
    return false;
  }

  if(_params->run_mode == Params::REALTIME)
  {
    if(_params->trigger_mode == Params::DATA)
    {
      if (_params->debug)
	cerr << "Initializing DATA trigger in realtime using url: " <<
	  _params->_input_list[0].mdv_url << endl;
    
      DsLdataTrigger *trigger = new DsLdataTrigger();
      if (trigger->init(_params->_input_list[0].mdv_url,
			-1, PMU_auto_register) != 0)
	{
	  cerr << "ERROR: " << method_name << endl;
	  cerr << "Error initializing DATA trigger in realtime using url: " <<
	    _params->_input_list[0].mdv_url << endl;
	  
	  return false;
	}

      _dataTrigger = trigger;

    } else if(_params->trigger_mode == Params::INTERVAL)
      {
	if (_params->debug)
	  cerr << "Initializing INTERVAL trigger in realtime using interval: " <<
	    _params->interval_trigger << " minutes." << endl;

	DsIntervalTrigger *trigger = new DsIntervalTrigger();
	if (trigger->init(_params->interval_trigger * 60,
			  0, 1, PMU_auto_register) != 0)
	  {
	    cerr << "ERROR: " << method_name << endl;
	    cerr << "Error initializing INTERVAL trigger in realtime using interval: " <<
	    _params->interval_trigger << " minutes." << endl;
	    
	    return false;
	  }

	_dataTrigger = trigger;

      }

  } else {  // ARCHIVE Modes

    time_t start_time = _args->getStartTime().utime();
    time_t end_time = _args->getEndTime().utime();
    
    if (start_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing start_time string for ARCHIVE mode: " <<
	_args->getStartTime() << endl;
      
      return false;
    }
    if (end_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing end_time string for ARCHIVE mode: " <<
	_args->getEndTime() << endl;
      
      return false;
    }
    
    if(_params->trigger_mode == Params::DATA)
    {
      if (_params->debug)
	cerr << "Initializing DATA trigger in ARCHIVE mode using url: " <<
	  _params->_input_list[0].mdv_url << endl;
      
      DsTimeListTrigger *trigger = new DsTimeListTrigger();
      if (trigger->init(_params->_input_list[0].mdv_url,
			start_time, end_time) != 0)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error initializing ARCHIVE trigger for url: " <<
	  _params->_input_list[0].mdv_url << endl;
	
	return false;
      }
      
      _dataTrigger = trigger;

    } else if(_params->trigger_mode == Params::INTERVAL)
    {
      
      DsIntervalTrigger *trigger = new DsIntervalTrigger();
      if (trigger->init(_params->interval_trigger * 60,
			start_time, end_time) != 0)
	{
	  cerr << "ERROR: " << method_name << endl;
	  cerr << "Error initializing INTERVAL trigger" << endl;
	  cerr << "     Start time: " << _args->getStartTime() << endl;
	  cerr << "     End time: " << _args->getEndTime() << endl;
	  cerr << "     Interval: " << _params->interval_trigger <<
	    " minutes" << endl;
	    
	  return false;
	}
      
      _dataTrigger = trigger;
      
    }

  }

  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
	5*PROCMAP_REGISTER_INTERVAL);

  //
  // OpenMP OPT
#ifdef _OPENMP
#pragma omp parallel
 {
   int thread = omp_get_thread_num();
   if(thread == 0)
     cout << "Number of Threads = " << omp_get_num_threads() << endl;
 }
#endif

  // Read Forest(s)
  if(!_read())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading in forest from file: " << 
      _params->forest_name << endl;
    return false;
  }

  // Create Polygon if requested
  _poly_lats = NULL;
  _poly_lons = NULL;
  if(_params->polygon_mask_n % 2 != 0) {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading in polygon, must contain Lat/Lon pairs" << endl;
    return false;
  }
  _nPolygonPoints = _params->polygon_mask_n / 2;
  if(_nPolygonPoints > 0) {
    _poly_lats = new float[_nPolygonPoints];
    _poly_lons = new float[_nPolygonPoints];
    for(int a = 0; a < _nPolygonPoints; a++) {
      _poly_lats[a] = _params->_polygon_mask[a*2];
      _poly_lons[a] = _params->_polygon_mask[(a*2)+1];
    }
  }
    // Create an output projection to use with polygon
    switch( _params->projection_info.type) {
    case Params::PROJ_FLAT:
      _outproj.initFlat(_params->projection_info.origin_lat, 
			_params->projection_info.origin_lon,
			_params->projection_info.rotation);
      break;
    case Params::PROJ_LATLON:
      _outproj.initLatlon();
      break;
    case Params::PROJ_LAMBERT_CONF:
      _outproj.initLambertConf(_params->projection_info.origin_lat, 
			       _params->projection_info.origin_lon,
			       _params->projection_info.ref_lat_1,
			       _params->projection_info.ref_lat_2);
      break;
    }
    _outproj.setGrid(_params->grid_info.nx, _params->grid_info.ny,
		     _params->grid_info.dx, _params->grid_info.dy,
		     _params->grid_info.minx, _params->grid_info.miny);
    //}

  return true;
}

/*********************************************************************
 * read() - Read in the Forest from file
 *
 * Returns true if the read was successful, false otherwise.
 */
bool RunForest::_read()
{
  static const string method_name = "RunForest::_read()";

  cout << "Loading forest: " << _params->forest_name << endl;

  if(_params->forest_type == Params::PARF_FOREST) {
    _input = new ReadParf();
  } else if(_params->forest_type == Params::RFOREST) {
    //_input = new ReadRforest();
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error: Read RForest function not yet implemented." << endl;
    return false;
  } else {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error: Unknown read forest_type." << endl;
    return false;
  }
  PMU_auto_register("Reading Forest");
  if(_input->readForest(_params->forest_name))
  {
    return false;
  }

  _attributes = _input->getAttributes();

  if(_params->var_list_n != _input->getNumAttributes()) {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Forest number of attributes: "<< _input->getNumAttributes() << 
      " does not match params number of attributes: " << _params->var_list_n << endl;
    return false;
  }

  if(_params->use_vote_remaping) {
    int nTrees = 0;
    if(_input->isSubForest()) {
      for(int a = 0; a < _input->getNumCategories(); a++)
	nTrees += _input->getNumSubTrees(a)+1;
      if(nTrees != _params->vote_remap_n) {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Vote remaping array size: "<< _params->vote_remap_n << 
	  " does not match number of SubForests * NumTrees: " << 
	  nTrees << endl;
	return false;
      }
    } else {
      if(_input->getNumClassCategories()-1 != 1) {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Vote remaping for categorical variables not yet implemented!" << endl;
	return false;
      }
      for(int a = 0; a < _input->getNumClassCategories()-1; a++)
	nTrees += _input->getNumTrees()+1;
      if(nTrees != _params->vote_remap_n) {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Vote remaping array size: "<< _params->vote_remap_n << 
	  " does not match number of Trees+1: " << 
	  nTrees << endl;
	return false;
      }
    }
  }

  for(int a = 0; a < _params->var_list_n; a++) {
    if(_params->_var_list[a].input_list_index == -1 && _attributes[a].index != -1) {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Variable " << _attributes[a].name <<
	" is listed as used in the forest but unused in the params file." << endl;
      return false;
    }
    
    if(_attributes[a].index == -1 && _params->_var_list[a].input_list_index != -1) {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Variable " << _attributes[a].name <<
	" is listed as unused in the forest but used in the params file." << endl;
      return false;
    }
  }

  cout << "Finished loading forest." << endl;

  return true;
}



int RunForest::run()
{
  static const string method_name = "RunForest::run()";

  // Setup Mdv Inputs
  MdvReader **mdv = new MdvReader*[_params->input_list_n];
  for(int i = 0; i < _params->input_list_n; i++)
    mdv[i] = new MdvReader(_params, _params->_input_list[i]);

  for(int a = 0; a < _params->var_list_n; a++)
    if(_params->_var_list[a].input_list_index != -1)
      if(_params->_var_list[a].mdv_name[0] == char(0))
	mdv[_params->_var_list[a].input_list_index]->addReadField(_params->_var_list[a].mdv_number);
      else
	mdv[_params->_var_list[a].input_list_index]->addReadField(_params->_var_list[a].mdv_name);

  string *filePaths = new string[_params->input_list_n];

  Mdvx::master_header_t mhdr;
  Mdvx::vlevel_header_t *outVHeader = NULL;

  time_t trigger_time;

  bool endOfData = _dataTrigger->endOfData();
  bool beginRun  = false;
  bool loadOK = true;

  //
  // OpenMP OPT
#ifdef _OPENMP
#pragma omp parallel 
  {
    int threadNum = omp_get_thread_num();
#else
    int threadNum = 0;
#endif

    // Loop over input
    while (!endOfData && loadOK)
    {

      if(threadNum == 0)
      {
	TriggerInfo trigger_info;
	if (_dataTrigger->next(trigger_info) != 0)
	{
	  cerr << "ERROR: " << method_name << endl;
	  cerr << "Error getting next trigger time" << endl;
	  loadOK = false;
	} else {

	  trigger_time = trigger_info.getIssueTime();

	  bool required_missing = false;
	  // Loop over input Mdv urls
	  for(int a = 0; a < _params->input_list_n; a++)
	  {
	    PMU_auto_register("Loading Input");
	    // Sets the current trigger time to each mdv class
	    int ret = mdv[a]->setTime(trigger_time);
	    if(ret == MdvReader::NO_MDV_AT_TIME) {
	      cerr << "Warning: Failed to load mdv file for url: " << _params->_input_list[a].mdv_url <<
		" at time: " << UTIMstr(trigger_time) << endl;
	      if(_params->_input_list[a].required)
		required_missing = true;
	    }
	    filePaths[a] = mdv[a]->getPathInUse();
	  }

	  if(required_missing) {
	    cerr << "ERROR: " << method_name << endl;
	    cerr << "One or more required inputs is missing." << endl;
	    continue;
	  }

	  _attributes = _input->getAttributes();
	  bool nz_set = false;
	  cout << "Loading Fields" << endl;

	  // Loop over input fields, loading them into an array
	  for(int a = 0; a < _params->var_list_n; a++)
	  {
	    PMU_auto_register("Loading Input Fields");
	    InputField field(_params, _attributes[a].name);
	    bool ret;

	    field.setMissing(_params->_var_list[a].fill_value, _params->_var_list[a].no_data_value);
	    field.setRemap(_params->_var_list[a].remap_option);

	    if(_params->_var_list[a].input_list_index == -1) 
	    {
	      field.setDummy();
	    } else 
	    {
	      //
	      // Read field by either name or mdv number
	      if(_params->_var_list[a].mdv_name[0] == char(0)) 
	      {
		if(_params->debug)
		  cout << "Reading " << _params->_var_list[a].mdv_number << endl;
		ret = mdv[_params->_var_list[a].input_list_index]->getField(_params->_var_list[a].mdv_number,
									    &field,
									    _params->_var_list[a].remap_option,
									    _params->_var_list[a].vertical_level, 
									    outVHeader);
	      } else 
	      {
		if(_params->debug)
		  cout << "Reading " << _params->_var_list[a].mdv_name << endl;
		ret = mdv[_params->_var_list[a].input_list_index]->getField(_params->_var_list[a].mdv_name,
									    &field,
									    _params->_var_list[a].remap_option,
									    _params->_var_list[a].vertical_level, 
									    outVHeader);
	      }
	      //
	      // Verify field loaded and matches output grid (unless basic remaping has been selected)
	      if(!ret || _params->_var_list[a].input_list_index == -1) 
	      {
		field.setField(NULL, NULL, NULL);
	      } else 
	      {
		Mdvx::field_header_t *field_header = field.getHeader();
		if(_params->_var_list[a].remap_option != Params::BASIC &&
		   _params->_var_list[a].remap_option != Params::INTERP &&
		   _params->_var_list[a].remap_option != Params::INTERP_FLAT
		   && (field_header->nx != _params->grid_info.nx || 
		       field_header->ny != _params->grid_info.ny ||
		       field_header->proj_type != _params->projection_info.type) )
		{
		    cerr << "ERROR: " << method_name << endl;
		    cerr << "Mdv field " <<_params->_var_list[a].mdv_name <<
		      " failed to convert to projection as specified in params." << endl;
		    loadOK = false;
		} else 
		{
		  // Get the headers from first mdv file that matches output vertical levels
		  // This will be used to set the vertical information.
		  if(!nz_set && field_header->nz == _params->output_nz) 
		  {
		    mhdr = mdv[_params->_var_list[a].input_list_index]->getMasterHeader();
		    outVHeader = field.getVHeader();
		    nz_set = true;
		  }
		}		
	      }
	    }

	    _inFields.push_back(field);
	  
	  } // End for var_list_n
	
	  if(!nz_set) {
	    cerr << "ERROR: " << method_name << endl;
	    cerr << "No input mdv field has " << _nz << " vertical levels." << endl;
	    continue;
	  }
	  
	  int numCategories = _input->getNumClassCategories();
	  // Loop over number of output categories, these will be our output fields
	  for(int a = 0; a < numCategories; a++) {
	    category cat = _input->getClassCategory(a);
	    float *out_data = new float[_nx * _ny * _nz];
	    
	    for(int ijk = 0; ijk < _nx * _ny * _nz; ijk++)
	      out_data[ijk] = 0;
	    
	    _outFields.push_back(out_data);
	    _outFieldsNames.push_back(cat.name);
	    string tmp = "count";
	    _outFieldsUnits.push_back(tmp);
	  }
	  // If SubForests exist we need an extra output field for the subforest index used
	  if(_input->isSubForest()) {
	    float *out_data = new float[_nx * _ny * _nz];
	    
	    for(int ijk = 0; ijk < _nx * _ny * _nz; ijk++)
	      out_data[ijk] = -1;
	    
	    _outFields.push_back(out_data);

	    string tmp = "SubForestsIndex";
	    _outFieldsNames.push_back(tmp);
	    tmp = "index";
	    _outFieldsUnits.push_back(tmp);
	  }
	  // If vote remaping is on we need an extra output field for the remap
	  if(_params->use_vote_remaping) {
	    float *out_data = new float[_nx * _ny * _nz];
	    
	    for(int ijk = 0; ijk < _nx * _ny * _nz; ijk++)
	      out_data[ijk] = -1;
	    
	    _outFields.push_back(out_data);
	    string tmp = _params->remap_field_name;
	    _outFieldsNames.push_back(tmp);
	    tmp = _params->remap_field_units;
	    _outFieldsUnits.push_back(tmp);
	  }

	  beginRun = true;
	  cout << "Begining forest run" << endl;
	} // End if else loadOK

      } else { 
	// Threads 1+ sit and wait till data loaded
	while(!beginRun)
	  sleep(5);
      }

      if(loadOK) 
      {

	_traverseForestGrid( );

	if(threadNum == 0)
	{
	  if(!_params->output_input_fields)
	    for(int a = 0; a < _params->input_list_n; a++) {
	      mdv[a]->clearRead();
	    }
	  
	  cout << "Creating output grids" << endl;
	  PMU_auto_register("Saving output");
	  _createOutputFields(trigger_time, mhdr, 
			      outVHeader, filePaths);
	  cout << "Finished Run" << endl;
	  
	  if(_params->output_input_fields)
	    for(int a = 0; a < _params->input_list_n; a++) {
	      mdv[a]->clearRead();
	    }

	  // Clean up 
	  for(int a = 0; a < _inFields.size(); a++) {
	    _inFields[a].clear();
	  }
	  _inFields.clear();
	  
	  
	  endOfData = _dataTrigger->endOfData();
	  beginRun = false;

	} else {
	  // Threads 1+ sit and wait till output is written
	  while(beginRun)
	    sleep(5);
	}

      } // End if loadOK

    }  // While !endOfData && loadOK

#ifdef _OPENMP
  } // End OMP Parallel section
#endif

  delete [] filePaths;
  for(int a = 0; a < _params->input_list_n; a++)
    delete mdv[a];
  delete [] mdv;

  if(!loadOK)
    return -1;
  return 0;
}

/*********************************************************************
 * _traverseForestGrid() - Loop through all grid points, 
 * traverse the forest at each point.
 */
void RunForest::_traverseForestGrid( )
{
    double lat, lon;
    int countIndex = _input->getNumClassCategories();
#ifdef _DATA3D
  #ifdef _OPENMP
    #pragma omp for
  #endif
#endif
    for(int k = 0; k < _nz; k++) {
      PMU_force_register("Traversing Trees");
#ifdef _DATA2D
  #ifdef _OPENMP
    #pragma omp for
  #endif
#endif
      for(int i = 0; i < _nx; i++)
	for(int j = 0; j < _ny; j++) {

          // Create structure of fields at this location
          float fieldData[_inFields.size()];
          for (int fn=0;fn < _inFields.size(); fn++) {
            fieldData[fn] = _inFields.at(fn).getVal(i,j,k);
          }

	  if(_nPolygonPoints > 0) _outproj.xyIndex2latlon(i, j, lat, lon);
	  // If we are not in the requested polygon stop and set points missing
	  if(_nPolygonPoints > 0 &&
	     !InPolygon(lat, lon, _poly_lats, _poly_lons,
			_poly_lats+_nPolygonPoints,
			_poly_lons+_nPolygonPoints) )
	    for(int r = 0; r < countIndex; r++)
	      _outFields[r][(k*_ny*_nx)+(j*_nx)+i] = -999.0;
	  else
           _input->traverseForest(&_outFields, i, j, k, _nx, _ny, fieldData);
	  
	  // Vote remaping when requested
	  if(_params->use_vote_remaping) {
	    int count = _outFields[1][(k*_ny*_nx)+(j*_nx)+i];

	    if(_input->isSubForest()) {
	      if(count == -999.0) {
		_outFields[countIndex+1][(k*_ny*_nx)+(j*_nx)+i] = -999.0;
	      } else {
		int subForest = _outFields[countIndex][(k*_ny*_nx)+(j*_nx)+i]-1;
		int nTrees = 0;
		for(int a = 0; a < subForest; a++)
		  nTrees += _input->getNumSubTrees(a)+1;
		_outFields[countIndex+1][(k*_ny*_nx)+(j*_nx)+i] = _params->_vote_remap[nTrees + count];
	      }
	    } else {
	      if(count == -999.0)
		_outFields[countIndex][(k*_ny*_nx)+(j*_nx)+i] = -999.0;
	      else
		_outFields[countIndex][(k*_ny*_nx)+(j*_nx)+i] = _params->_vote_remap[count];
	    }
	  }

	}

    }

}

/*********************************************************************
 * _createOutputFields() - Using the outFields vector of data
 *  save each field to mdv, write out the mdv file.
 */
void RunForest::_createOutputFields(time_t trigger_time, Mdvx::master_header_t &mhdr, 
				    Mdvx::vlevel_header_t *vHeader,
				    string *filePaths)
{
  DsMdvx out;
  out.setAppName(_progName);

  Mdvx::master_header_t out_mhdr;
  MEM_zero(out_mhdr);
 
  if (_params->output_fcst)
  {
    out_mhdr.time_gen = trigger_time;
    out_mhdr.time_begin = trigger_time + _params->fcst_lead_secs;
    out_mhdr.time_end = trigger_time + _params->fcst_lead_secs;
    out_mhdr.time_centroid = trigger_time + _params->fcst_lead_secs;
    out_mhdr.time_centroid = trigger_time + _params->fcst_lead_secs;
  }
  else 
  {
    out_mhdr.time_gen = time(NULL);
    out_mhdr.time_begin = trigger_time;
    out_mhdr.time_end = trigger_time;
    out_mhdr.time_centroid = trigger_time;
    out_mhdr.time_expire = trigger_time + (60*60);
  }
  
  out_mhdr.num_data_times = 1;
  out_mhdr.data_dimension = 3;

  if (_params->output_fcst)
    out_mhdr.data_collection_type = Mdvx::DATA_FORECAST;
  else
    out_mhdr.data_collection_type = mhdr.data_collection_type;

  out_mhdr.native_vlevel_type = vHeader->type[0];
  out_mhdr.vlevel_type = vHeader->type[0];
  out_mhdr.vlevel_included = TRUE;
  out_mhdr.grid_orientation = mhdr.grid_orientation;
  out_mhdr.data_ordering = mhdr.data_ordering;
  out_mhdr.n_fields = _outFields.size();
  out_mhdr.max_nx = _nx;
  out_mhdr.max_ny = _ny;
  out_mhdr.max_nz = _nz;
  out_mhdr.field_grids_differ = FALSE;

  Mdvx::field_header_t out_fHeader;
  MEM_zero(out_fHeader);

  if (_params->output_fcst) 
  {
    out_fHeader.forecast_delta = _params->fcst_lead_secs;
    out_fHeader.forecast_time = trigger_time + _params->fcst_lead_secs;
  }

  out_fHeader.field_code = 0;
  out_fHeader.nx = _nx;
  out_fHeader.ny = _ny;
  out_fHeader.nz = _nz;
  out_fHeader.proj_type = _params->projection_info.type;
  
  out_fHeader.encoding_type = Mdvx::ENCODING_FLOAT32;
  out_fHeader.data_element_nbytes = 4;
  out_fHeader.scale = 0.0;
  out_fHeader.bias = 0.0;
  out_fHeader.bad_data_value = -998.0;
  out_fHeader.missing_data_value = -999.0;
  
  out_fHeader.volume_size =
    out_fHeader.nx * out_fHeader.ny * out_fHeader.nz * out_fHeader.data_element_nbytes;
  
  out_fHeader.compression_type = Mdvx::COMPRESSION_NONE;
  out_fHeader.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  out_fHeader.scaling_type = Mdvx::SCALING_NONE;
  
  out_fHeader.native_vlevel_type = vHeader->type[0];
  out_fHeader.vlevel_type = vHeader->type[0];
  out_fHeader.dz_constant = FALSE;
  
  out_fHeader.proj_origin_lat = _params->projection_info.origin_lat;
  out_fHeader.proj_origin_lon = _params->projection_info.origin_lon;
  out_fHeader.proj_param[0] = _params->projection_info.ref_lat_1;
  out_fHeader.proj_param[1] = _params->projection_info.ref_lat_2;
  out_fHeader.proj_rotation = _params->projection_info.rotation;
  
  out_fHeader.grid_dx = _params->grid_info.dx;
  out_fHeader.grid_dy = _params->grid_info.dy;
  out_fHeader.grid_dz = 0.0;
  
  out_fHeader.grid_minx = _params->grid_info.minx;
  out_fHeader.grid_miny = _params->grid_info.miny;
  out_fHeader.grid_minz = vHeader->level[0];
  
  cout << "Making MdvFields" << endl;

  string dataInfo = "Data created using the following forest file:\n";
  dataInfo = dataInfo + _params->forest_name + "\n";
  dataInfo = dataInfo + "Data created from the following mdv files:\n";
  for(int a = 0; a < _params->input_list_n; a++) {
    dataInfo =  dataInfo + "   " + filePaths[a] + "\n";
  }

  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  for(int a = 0; a < _nz; a++) {
    vhdr.type[a] = vHeader->type[a];
    vhdr.level[a] = vHeader->level[a];
  }

  int nSubCats = _input->getNumClassCategories();
  for(int a = 0; a < _outFields.size(); a++)
  {
    MdvxField *outField = new MdvxField(out_fHeader, vhdr, _outFields[a]);

    outField->setFieldName(_outFieldsNames[a].c_str());
    outField->setFieldNameLong(_outFieldsNames[a].c_str());
    outField->setUnits(_outFieldsUnits[a].c_str());

    outField->convertType
      ((Mdvx::encoding_type_t) _params->output_encoding,
       (Mdvx::compression_type_t) _params->output_compression);
    
    out.addField(outField);

    delete[] _outFields[a];
  }

  if(_params->output_input_fields) {
    out_mhdr.n_fields += _inFields.size();
    for(int a = 0; a < _inFields.size(); a++)
    {
      float *data = _inFields[a].getData();
      Mdvx::field_header_t *header = _inFields[a].getHeader();
      Mdvx::vlevel_header_t *vHeader = _inFields[a].getVHeader();
      string name = _inFields[a].getName();

      if(_params->_var_list[a].input_list_index != -1 && _inFields[a].getData() != NULL && 
	 header != NULL && vHeader != NULL) {

	MdvxField *outField = new MdvxField(*(header), *(vHeader), (void*)data);
	
	outField->setFieldName(name.c_str());
	outField->setFieldNameLong(name.c_str());
	outField->setUnits(header->units);
		
	outField->convertType
	  ((Mdvx::encoding_type_t) _params->output_encoding,
	   (Mdvx::compression_type_t) _params->output_compression);
	
	out.addField(outField);
      }
    }
  }

  out.setMasterHeader(out_mhdr);
  out.setDataSetName(_params->output_data_set_name);
  out.setDataSetSource(_params->output_data_set_source);

   if (_params->output_fcst)
   {
     out.setWriteAsForecast();
   }
  string dataSetInfo = "Data inputs list stored in Mdv Chunk as a comment\n";
  out.setDataSetInfo(dataSetInfo.c_str());

  Mdvx::chunk_header_t chunk; 
  chunk.chunk_id = Mdvx::CHUNK_COMMENT; 
  chunk.size = dataInfo.size();
  sprintf(chunk.info, "MetaData"); 
  MdvxChunk *c = new MdvxChunk(chunk, dataInfo.c_str());
  out.addChunk(c); 

  _outFields.clear();
  _outFieldsNames.clear();
  _outFieldsUnits.clear();

  cout << "Writing to dir" << endl;
  if (out.writeToDir(_params->output_url) != 0) {
    cerr << "ERROR - " <<  _progName << "::OutputFile::write" << endl;
    cerr << out.getErrStr() << endl;
    return;
  }

}

