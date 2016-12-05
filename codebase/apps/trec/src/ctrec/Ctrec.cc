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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/06 23:28:57 $
//   $Id: Ctrec.cc,v 1.42 2016/03/06 23:28:57 dixon Exp $
//   $Revision: 1.42 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Ctrec.cc: ctrec program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cassert>
#include <signal.h>
#include <vector>

#include <toolsa/os_config.h>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/udatetime.h>
#include <toolsa/DataScaler.hh>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <rapmath/stats.h>

#include "Ctrec.hh"
#include "CtrecAlg.hh"
#include "GridSearcher.hh"
#include "Params.hh"

#include "DiffTemporalSmoother.hh"
#include "NullTemporalSmoother.hh"
#include "PrevTemporalSmoother.hh"
using namespace std;


// Global variables

Ctrec *Ctrec::_instance = (Ctrec *)NULL;

// Global constants

const int FOREVER = true;

/*********************************************************************
 * Constructor
 */

Ctrec::Ctrec(int argc, char **argv) :
  _trigger(0),
  _thrDbzReset(false)
{
  static const string method_name = "Ctrec::Ctrec()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (Ctrec *)NULL);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);
  
  // Display ucopyright message.

  ucopyright(_progName);

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);
  
  if (!_args->okay)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with command line arguments." << endl;
    
    okay = false;
    
    return;
  }
  
  // Get TDRP parameters.

  _params = new Params();
  char *params_path = (char *)"unknown";
  
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with TDRP parameters in file <" << params_path <<
      ">" << endl;
    
    okay = false;
    
    return;
  }

  // Make sure the TDRP parameters are consistent.
  
  if (_params->mode == Params::ARCHIVE)
  {
    if (_args->getArchiveStartTime() == 0 ||
	_args->getArchiveEndTime() == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "ARCHIVE mode requires -start and -end on the command line." <<
	endl;

      exit(-1);
    }
  }
  
  if (_params->min_echo < _params->thr_dbz)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error in parameter file" << endl;
    cerr << "min_echo must be greater than or equal to thr_dbz" << endl;
    cerr << "Reset parameters and try again" << endl;
    
    okay = false;
    
    return;
  }

  // Initialize the input data object

  switch (_params->mode)
  {
  case Params::REALTIME :
  {
    if (_params->debug)
    {
      cerr << "Initializing LDATA trigger:" << endl;
      cerr << "    url: " << _params->input_url << endl;
      cerr << "    max_input_secs: " << _params->max_time_between_images << endl;
    }
    
    DsLdataTrigger *ldata_trigger = new DsLdataTrigger();
    if (ldata_trigger->init(_params->input_url,
			    _params->max_time_between_images,
			    PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LDATA trigger" << endl;

      okay = false;
      return;
    }

    _trigger = ldata_trigger;
    
    if (_params->debug)
      cerr << "    LDATA trigger successfully initialized..." << endl;
    
    break;
  }
  
  case Params::ARCHIVE :
  {
    if (_params->debug)
    {
      cerr << "Initializing TIME_LIST trigger:" << endl;
      cerr << "    url: " << _params->input_url << endl;
      cerr << "    start time: "
	   << DateTime::str(_args->getArchiveStartTime()) << endl;
      cerr << "    end time: "
	   << DateTime::str(_args->getArchiveEndTime()) << endl;
    }
    
    DsTimeListTrigger *time_list_trigger = new DsTimeListTrigger();
    if (time_list_trigger->init(_params->input_url,
				_args->getArchiveStartTime(),
				_args->getArchiveEndTime()) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger" << endl;
      cerr << "URL : " << _params->input_url << endl;
      cerr << "Start : " << utimstr(_args->getArchiveStartTime()) << endl;
      cerr << "End : " << utimstr(_args->getArchiveEndTime()) << endl;
      cerr << time_list_trigger->getErrStr() << endl;
      cerr << "NOTE : The URL may need to be fully defined so that" << endl;
      cerr << "       it is clear that the mdvp protocol is in use." << endl;

      okay = false;
      return;
    }

    _trigger = time_list_trigger;
    
    if (_params->debug)
      cerr << "    TIME_LIST trigger successfully initialized..." << endl;
    
    break;
  }
  
  } /* endswitch - _params->mode */

  
  // Set the noise values in the noise generator object

  _noiseGenerator.setCenterValue(_params->thr_dbz);
  _noiseGenerator.setPerturbationValue(_params->del_dbz);
  

  // Create the clutter remover and detrender algorithms
     
  if (_params->dtrnd_flg)
    _detrender = new DataDetrender(_params->thr_dbz, _params->debug);
  else
    _detrender = 0;
  
  _clutterRemover = new ClutterRemover(_params->thr_sd, 2,
				       _params->min_echo, _params->max_echo,
				       _detrender, _params->debug);
  
  // Create the list of points for outputting correlation values

  vector< CtrecAlg::grid_pt_t > correlation_pt_list;
  
  for (int i = 0; i < _params->output_correlation_locations_n; ++i)
  {
    CtrecAlg::grid_pt_t grid_pt;
    
    grid_pt.x = _params->_output_correlation_locations[i].x;
    grid_pt.y = _params->_output_correlation_locations[i].y;
    
    correlation_pt_list.push_back(grid_pt);
  } /* endfor - i */
  
  // Create the internal Ctrec algorithm object.

  GridSearcherTypes::search_start_t cormax_search_start =
    GridSearcherTypes::SEARCH_START_LOWER_LEFT;
  GridSearcherTypes::search_direction_t cormax_search_direction =
    GridSearcherTypes::SEARCH_COLUMN_FIRST;
  GridSearcherTypes::search_result_t cormax_search_result =
    GridSearcherTypes::SEARCH_TAKE_FIRST;
  
  switch(_params->cormax_search_params.start)
  {
  case Params::LOWER_LEFT_CORNER :
    cormax_search_start = GridSearcherTypes::SEARCH_START_LOWER_LEFT;
    break;
    
  case Params::LOWER_RIGHT_CORNER :
    cormax_search_start = GridSearcherTypes::SEARCH_START_LOWER_RIGHT;
    break;
    
  case Params::UPPER_LEFT_CORNER :
    cormax_search_start = GridSearcherTypes::SEARCH_START_UPPER_LEFT;
    break;
    
  case Params::UPPER_RIGHT_CORNER :
    cormax_search_start = GridSearcherTypes::SEARCH_START_UPPER_RIGHT;
    break;
  }
  
  switch(_params->cormax_search_params.direction)
  {
  case Params::ROW_FIRST_SEARCH :
    cormax_search_direction = GridSearcherTypes::SEARCH_COLUMN_FIRST;
    break;
    
  case Params::COLUMN_FIRST_SEARCH :
    cormax_search_direction = GridSearcherTypes::SEARCH_ROW_FIRST;
    break;
  }
  
  switch(_params->cormax_search_params.result)
  {
  case Params::TAKE_FIRST_FOUND :
    cormax_search_result = GridSearcherTypes::SEARCH_TAKE_FIRST;
    break;
    
  case Params::TAKE_LAST_FOUND :
    cormax_search_result = GridSearcherTypes::SEARCH_TAKE_LAST;
    break;
    
  case Params::TAKE_MIDDLE_FOUND :
    cormax_search_result = GridSearcherTypes::SEARCH_TAKE_MIDDLE;
    break;
  }
  
  GridSearcher<fl32> cormax_searcher(cormax_search_start,
				     cormax_search_direction,
				     cormax_search_result,
				     _params->debug);
  
  _ctrecAlg = new CtrecAlg(_params->min_echo,
			   _params->max_echo,
			   _params->track_top_percentage,
			   _params->top_percentage,
			   _params->top_percentage_increasing,
			   _params->max_speed_echo,
			   _params->cbox_fract / 100.0,
			   _params->cbox_size,
			   _params->cbox_space,
			   _params->thr_cor / 100.0,
			   cormax_searcher,
			   &_noiseGenerator,
			   _params->thrvec_flg,
			   _params->rad_mean,
			   _params->thr_vec,
			   _params->thr_dif,
			   _params->fillvec_flg,
			   _params->rad_vec,
			   _params->nquad_vec,
			   _params->min_vec_pts,
			   correlation_pt_list,
			   _params->debug,
			   _params->print_global_mean,
			   _params->output_correlation_grid,
			   _params->output_cormax_count_grid,
			   _params->output_original_vectors,
			   _params->output_local_mean_grids,
			   _params->output_local_mean_thresh_vectors,
			   _params->output_global_mean_thresh_vectors);
  
  // Create the temporal smoother

  string u_field_name, v_field_name;
    
  switch (_params->temporal_prev_grid_type)
  {
  case Params::PREV_GRID_SMOOTHED :
    u_field_name = "U wind component";
    v_field_name = "V wind component";
    break;
    
  case Params::PREV_GRID_UNSMOOTHED :
    u_field_name = "U w/o temporal smoothing";
    v_field_name = "V w/o temporal smoothing";
    break;
    
  case Params::PREV_GRID_AVERAGE :
    u_field_name = "U temporal smoothing average";
    v_field_name = "V temporal smoothing average";
    break;
  }
  
  switch (_params->temporal_smoothing_type)
  {
  case Params::SMOOTH_NONE :
    _smoother = new NullTemporalSmoother();
    break;
    
  case Params::SMOOTH_WITH_PREVIOUS_VECTOR :
    _smoother = new PrevTemporalSmoother(_params->output_url,
					 u_field_name,
					 v_field_name,
					 _params->temporal_u_percent,
					 _params->temporal_v_percent,
					 _params->temporal_prev_u_min,
					 _params->temporal_prev_v_min,
					 _params->debug);
    break;
    
  case Params::SMOOTH_WITH_VECTOR_DIFFERENCE :
    _smoother = new DiffTemporalSmoother(_params->output_url,
					 u_field_name,
					 v_field_name,
					 _params->temporal_u_percent,
					 _params->temporal_v_percent,
					 _params->debug);
    break;
  }
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);
}


/*********************************************************************
 * Destructor
 */

Ctrec::~Ctrec()
{
  // Free contained objects

  delete _params;
  delete _args;

  delete _trigger;
  
  // Free algorithm objects

  delete _detrender;
  delete _clutterRemover;
  delete _ctrecAlg;
  
  // Free included strings

  STRfree(_progName);

}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

Ctrec *Ctrec::Inst(int argc, char **argv)
{
  if (_instance == (Ctrec *)NULL)
    new Ctrec(argc, argv);
  
  return(_instance);
}

Ctrec *Ctrec::Inst()
{
  assert(_instance != (Ctrec *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * run()
 */

void Ctrec::run()
{
  static const string method_name = "Ctrec::run()";
  
  string status_string;
  
  // Reset the file retriever

//  _trigger->reset();
  
  // Process new files forever.  In archive mode, we will break out
  // of this loop when the last file is processed.

  while (true)
  {
    // Get the next trigger time

    DateTime data_time;
    
    if (_trigger->nextIssueTime(data_time) != 0)
    {
      if (_params->mode != Params::REALTIME)
      {
	cerr << "... Finished processing files" << endl;
	break;
      }
      
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next data time" << endl;
      cerr << "Trying again..." << endl;
      
      continue;
    }
    
    if (_params->debug)
      cerr << "===> Triggered for time: " << data_time << endl;
    
    DsMdvx prev_mdv_file;
    DsMdvx curr_mdv_file;
    
    // Register with the process mapper

    status_string = "Checking ";
    status_string += _params->input_url;
  
    if (_params->debug)
      cout << status_string << endl;
    
    PMU_auto_register((char *)status_string.c_str());
    
    // Read the current data file

    if (!_readNextFile(curr_mdv_file,
		       data_time.utime()))
    {
      if (_params->mode == Params::REALTIME)
	continue;
      else
      {
	if (_params->debug)
	  cout << "No more files to process -- exiting" << endl;
	  
	break;
      }
    }
      
    // Read in the matching previous file.

    if (!_readPrevFile(prev_mdv_file,
		       curr_mdv_file.getMasterHeader().time_centroid))
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "No matching previous file for data time: "
	   << DateTime::str(curr_mdv_file.getMasterHeader().time_centroid)
	   << endl;
      
      continue;
    }
    
    // Make sure the files are timed correctly

    time_t curr_data_time = 0;
    time_t prev_data_time = 0;
    
    switch (_params->time_value)
    {
    case Params::TIME_BEGIN :
      curr_data_time = curr_mdv_file.getMasterHeader().time_begin;
      prev_data_time = prev_mdv_file.getMasterHeader().time_begin;
      break;
      
    case Params::TIME_CENTROID :
      curr_data_time = curr_mdv_file.getMasterHeader().time_centroid;
      prev_data_time = prev_mdv_file.getMasterHeader().time_centroid;
      break;
      
    case Params::TIME_END :
      curr_data_time = curr_mdv_file.getMasterHeader().time_end;
      prev_data_time = prev_mdv_file.getMasterHeader().time_end;
      break;
    } /* endswitch - _params->time_value */
    
    int time_diff = curr_data_time - prev_data_time;
    
    if (time_diff > _params->max_time_between_images ||
	time_diff < _params->min_time_between_images)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Incorrect time difference to process fields" << endl;
      cerr << "Time difference: " << time_diff << endl;
      
      continue;
    }
    
    // Get pointers to the input fields for convenience.

    MdvxField *curr_field = curr_mdv_file.getFieldByNum(0);
    MdvxField *prev_field = prev_mdv_file.getFieldByNum(0);
    
    if (curr_field == 0 || prev_field == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Couldn't get pointers to the data fields" << endl;
      
      delete curr_field;
      delete prev_field;
      
      continue;
    }
    
    // Create the output file so we can add fields to it for debugging.

    DsMdvx *output_mdv_file =
      _createOutputFile(curr_mdv_file.getMasterHeader(),
			curr_field->getFieldHeader().nx,
			curr_field->getFieldHeader().ny,
			curr_mdv_file.getPathInUse());
    
    // Add the current input field to the output file.

    MdvxField *curr_field_orig = new MdvxField(*curr_field);

    curr_field_orig->convertType(Mdvx::ENCODING_INT8,
				 Mdvx::COMPRESSION_GZIP);
   
    output_mdv_file->addField(curr_field_orig);
    
    // Cleanup the data in the field before processing

    _cleanupData(*prev_field, output_mdv_file);
    _cleanupData(*curr_field, output_mdv_file);

    // Process the files

    if (_params->debug)
    {
      cout << endl;
      cout << "curr_data_time = " << utimstr(curr_data_time) << endl;
      cout << "prev_data_time = " << utimstr(prev_data_time) << endl;
      cout << "time_diff = " << time_diff << endl;
    }
	   
    status_string = "Processing files";
    
    PMU_force_register((char *)status_string.c_str());

    if (!_processFiles(prev_mdv_file, curr_mdv_file, time_diff,
		       *output_mdv_file))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing file " <<  curr_mdv_file.getPathInUse()
	   << endl;
    }
    
    fflush(stdout);
    
    delete output_mdv_file;
    
    PMU_force_register("Finished Processing images.");
    
    sleep(1);
    
  }  /* endwhile - true */
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _addDebugOutputField() - Add the given field to the output file.
 */

void Ctrec::_addDebugOutputField(DsMdvx &output_mdv_file,
				 const MdvxField &field,
				 const string &transform) const
{
  MdvxField *new_field = new MdvxField(field);
  
  Mdvx::field_header_t new_field_hdr = field.getFieldHeader();
  
  new_field_hdr.min_value = 0.0;
  new_field_hdr.max_value = 0.0;
  
  STRcopy(new_field_hdr.transform, transform.c_str(),
	  MDV_TRANSFORM_LEN);
  
  new_field->setFieldHeader(new_field_hdr);
  new_field->convertType(Mdvx::ENCODING_INT8,
			 Mdvx::COMPRESSION_GZIP);
  
  output_mdv_file.addField(new_field);
}


/*********************************************************************
 * _addVectorOutputField() - Add the given vector field to the output
 *                           file.
 */

void Ctrec::_addVectorOutputField(DsMdvx &output_mdv_file,
				  const Mdvx::field_header_t data_field_hdr,
				  const Mdvx::vlevel_header_t data_vlevel_hdr,
				  const fl32 *data,
				  const int field_code,
				  const string &field_name_long,
				  const string &field_name_short) const
{
  // Set up the field header

  Mdvx::field_header_t output_field_hdr;
   
  memset(&output_field_hdr, 0, sizeof(output_field_hdr));
  
  output_field_hdr.field_code = field_code;
  output_field_hdr.nx = data_field_hdr.nx;
  output_field_hdr.ny = data_field_hdr.ny;
  output_field_hdr.nz = 1;
  output_field_hdr.proj_type = data_field_hdr.proj_type;
  output_field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  output_field_hdr.data_element_nbytes = 4;
  output_field_hdr.volume_size =
    output_field_hdr.nx * output_field_hdr.ny *
    output_field_hdr.data_element_nbytes;
  output_field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  output_field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  output_field_hdr.scaling_type = Mdvx::SCALING_NONE;
  output_field_hdr.native_vlevel_type = data_field_hdr.vlevel_type;
  output_field_hdr.vlevel_type = data_field_hdr.vlevel_type;
  output_field_hdr.dz_constant = data_field_hdr.dz_constant;
  
  output_field_hdr.proj_origin_lat = data_field_hdr.proj_origin_lat;
  output_field_hdr.proj_origin_lon = data_field_hdr.proj_origin_lon;
  for (int i = 0; i < MDV_MAX_PROJ_PARAMS; i++)
    output_field_hdr.proj_param[i] = data_field_hdr.proj_param[i];
  output_field_hdr.vert_reference = data_field_hdr.vert_reference;
  output_field_hdr.grid_dx = data_field_hdr.grid_dx;
  output_field_hdr.grid_dy = data_field_hdr.grid_dy;
  output_field_hdr.grid_dz = data_field_hdr.grid_dz;
  output_field_hdr.grid_minx = data_field_hdr.grid_minx;
  output_field_hdr.grid_miny = data_field_hdr.grid_miny;
  output_field_hdr.grid_minz = data_vlevel_hdr.level[0];
  output_field_hdr.scale = 1.0;
  output_field_hdr.bias = 0.0;
  output_field_hdr.bad_data_value = CtrecAlg::BAD_OUTPUT_VALUE;
  output_field_hdr.missing_data_value = CtrecAlg::BAD_OUTPUT_VALUE;
  output_field_hdr.proj_rotation = data_field_hdr.proj_rotation;
  output_field_hdr.min_value = 0.0;
  output_field_hdr.max_value = 0.0;

  STRcopy(output_field_hdr.field_name_long, field_name_long.c_str(),
	  MDV_LONG_FIELD_LEN);
  STRcopy(output_field_hdr.field_name, field_name_short.c_str(),
	  MDV_SHORT_FIELD_LEN);
  STRcopy(output_field_hdr.units, "m/sec", MDV_UNITS_LEN);
  output_field_hdr.transform[0] = '\0';
   
  // Create the new field.

  MdvxField *u_field = new MdvxField(output_field_hdr,
				     data_vlevel_hdr,
				     data);
   
  // Compress the data.
  
  u_field->convertType(Mdvx::ENCODING_INT8,
		       Mdvx::COMPRESSION_GZIP);
  
  // Add the field to the output file.

  output_mdv_file.addField(u_field);
}


/*********************************************************************
 * _cleanupData() - Cleanup the data so it is ready for processing.
 */

void Ctrec::_cleanupData(MdvxField &field, DsMdvx *output_mdv_file)
{ 
  if (_params->debug)
  {
    cout << "thr_dbz is :" << (int)_params->thr_dbz << endl;
    cout << "Bias value is:" << field.getFieldHeader().bias << endl;
    cout << "Will use " << (int)_params->thr_dbz << 
      " for constant missing data value" << endl;
    cout << "Will use " << (int)_params->min_echo <<
      " for minimum echo to track" << endl;
    cout << "Will use " << (int)_params->max_echo <<
      " for maximum echo to track" << endl;
    cout << endl;

    cout << "bad0 = " << field.getFieldHeader().bad_data_value << endl;
  }

   // Replace missing data in the grids with noise.

  Mdvx::field_header_t field_hdr = field.getFieldHeader();
  float *data = (float *)field.getVol();
   
  for (int index = 0; index < field_hdr.nx * field_hdr.ny; ++index)
    if (data[index] == field_hdr.bad_data_value ||
	data[index] == field_hdr.missing_data_value)
//      data[index] = _noiseGenerator.getNoiseValue();
      data[index] = _params->thr_dbz;
   
  // Remove clutter from the image and fill the data forecast grid with the
  // current data.

  if (_params->clutter_flg)
  {
    if (_params->debug)
      cout << "Removing clutter on image number 2" << endl;
    
    _clutterRemover->run(field,
			 CtrecAlg::BAD_OUTPUT_VALUE);

    if (_params->output_after_clutter_removal &&
	output_mdv_file != (DsMdvx *)NULL)
      _addDebugOutputField(*output_mdv_file, field,
			   "After clutter removal");
  }
  
  if (_params->floor_flg)
  {
    if (_params->debug)
      cout << "Calling trec_floor for image 2" << endl;
    
    _trecFloor(field);

    if (_params->output_after_random_noise &&
	output_mdv_file != (DsMdvx *)NULL)
      _addDebugOutputField(*output_mdv_file, field,
			   "After filling with random noise");
  }
}


/*********************************************************************
 * _createOutputFile() - Create the output file.
 */

DsMdvx *Ctrec::_createOutputFile(const Mdvx::master_header_t input_master_hdr,
				 const int nx, const int ny,
				 const string dataset_source) const
{
   // Create the output file

   DsMdvx *output_mdv_file = new DsMdvx();
   
   // Put the needed data in the output file.

   Mdvx::master_header_t output_master_hdr;
   
   memset(&output_master_hdr, 0, sizeof(output_master_hdr));

   output_master_hdr.time_gen = time((time_t *)NULL);
   output_master_hdr.time_begin = input_master_hdr.time_begin;
   output_master_hdr.time_end = input_master_hdr.time_end;
   output_master_hdr.time_centroid = input_master_hdr.time_centroid;
   output_master_hdr.time_expire = input_master_hdr.time_expire;
   output_master_hdr.num_data_times = 0;
   output_master_hdr.index_number = 0;
   output_master_hdr.data_dimension = 2;
   output_master_hdr.data_collection_type = Mdvx::DATA_MIXED;
   output_master_hdr.native_vlevel_type = input_master_hdr.vlevel_type;
   output_master_hdr.vlevel_type = input_master_hdr.vlevel_type;
   output_master_hdr.vlevel_included = 0;
   output_master_hdr.grid_orientation = input_master_hdr.grid_orientation;
   output_master_hdr.data_ordering = input_master_hdr.data_ordering;
   output_master_hdr.n_fields = 0;
   output_master_hdr.max_nx = nx;
   output_master_hdr.max_ny = ny;
   output_master_hdr.max_nz = 1;
   output_master_hdr.n_chunks = 0;
   output_master_hdr.field_grids_differ = 0;
   
   output_master_hdr.sensor_lon = input_master_hdr.sensor_lon;
   output_master_hdr.sensor_lat = input_master_hdr.sensor_lat;
   output_master_hdr.sensor_alt = input_master_hdr.sensor_alt;
   
   STRcopy(output_master_hdr.data_set_info,
	   "CTREC output", MDV_INFO_LEN);
   STRcopy(output_master_hdr.data_set_name,
	   "CTREC output", MDV_NAME_LEN);
   STRcopy(output_master_hdr.data_set_source,
	   dataset_source.c_str(), MDV_NAME_LEN);
   
   output_mdv_file->setMasterHeader(output_master_hdr);

   return output_mdv_file;
}


/*********************************************************************
 * _processFiles() - Run the ctrec algorithm on the 2 specified files
 *                   and create the output file.
 *
 * Returns true if the algorithm ran successfully, false otherwise.
 *
 * Note that the curr_field_orig pointer is added to the output Mdvx
 * object so the object is freed at the end of this method.
 */

bool Ctrec::_processFiles(const DsMdvx &prev_mdv_file,
			  const DsMdvx &curr_mdv_file,
			  const int image_delta_secs,
		          DsMdvx &output_mdv_file)
{
  static const string method_name = "Ctrec::_processFiles()";

/*-------------------------------------------------------------------------*/
     
  PMU_auto_register("Beginning to process files");
  
  if (_params->debug || _params->print_global_mean)
  {
    date_time_t *data_time;
     
    data_time = udate_time(curr_mdv_file.getMasterHeader().time_centroid);

    fprintf(stdout,
	    "Current file time centroid: %.2d/%.2d/%.4d %.2d:%.2d:%.2d\n\n",
	    data_time->month, data_time->day, data_time->year,
	    data_time->hour, data_time->min, data_time->sec);
  }

  // Set variables for easier access to the data and headers

  MdvxField *prev_field = prev_mdv_file.getFieldByNum(0);
  MdvxField *curr_field = curr_mdv_file.getFieldByNum(0);
  Mdvx::field_header_t prev_field_hdr = prev_field->getFieldHeader();
  Mdvx::field_header_t curr_field_hdr = curr_field->getFieldHeader();
     
   
  // Set the algorithm subgrid

  if (_params->debug)
  {
    cout << "sg.nx = " << curr_field_hdr.nx <<
      ", sg.ny = " << curr_field_hdr.ny << endl << endl;
    cout << "sub_grid_minx = " << curr_field_hdr.grid_minx <<
      ", sg.grid_miny = " << curr_field_hdr.grid_miny << endl;
  }

  // Run the algorithm

  if (!_ctrecAlg->run(*prev_field, *curr_field, image_delta_secs,
		      output_mdv_file))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error running CTREC algorithm" << endl;
     
    return false;
  }
   
  // Add the U data to the output file

  PMU_auto_register("Adding U/V fields to output file");
  
  int grid_size = curr_field_hdr.nx * curr_field_hdr.ny;

  fl32 *smoothed_u_grid = (fl32 *)umalloc(grid_size * sizeof(fl32));
  fl32 *smoothed_v_grid = (fl32 *)umalloc(grid_size * sizeof(fl32));

  memcpy(smoothed_u_grid, _ctrecAlg->getUGrid(), grid_size * sizeof(fl32));
  memcpy(smoothed_v_grid, _ctrecAlg->getVGrid(), grid_size * sizeof(fl32));
  
  // Perform temporal smoothing

  PMU_auto_register("Performing temporal smoothing");
  
  if (_params->output_vectors_before_temporal_smoothing ||
      _params->temporal_prev_grid_type == Params::PREV_GRID_UNSMOOTHED)
  {
    _addVectorOutputField(output_mdv_file,
			  curr_field_hdr, curr_field->getVlevelHeader(),
			  smoothed_u_grid,
			  33,
			  "U w/o temporal smoothing",
			  "U_temporal");
   
    // Add the V data to the output file

    _addVectorOutputField(output_mdv_file,
			  curr_field_hdr, curr_field->getVlevelHeader(),
			  smoothed_v_grid,
			  34,
			  "V w/o temporal smoothing",
			  "V_temporal");
  }
    
  _smoother->smoothData(curr_mdv_file.getMasterHeader().time_centroid -
			_params->min_time_between_vectors,
			_params->max_time_between_vectors -
			_params->min_time_between_vectors,
			smoothed_u_grid, smoothed_v_grid,
			grid_size, CtrecAlg::BAD_OUTPUT_VALUE);
    
  // Save the average field for temporal smoothing, if requested

  if (_params->temporal_prev_grid_type == Params::PREV_GRID_AVERAGE)
  {
    fl32 *avg_u_grid = (fl32 *)umalloc(grid_size * sizeof(fl32));
    fl32 *avg_v_grid = (fl32 *)umalloc(grid_size * sizeof(fl32));

    memcpy(avg_u_grid, _ctrecAlg->getUGrid(), grid_size * sizeof(fl32));
    memcpy(avg_v_grid, _ctrecAlg->getVGrid(), grid_size * sizeof(fl32));
  
    for (int i = 0; i < grid_size; ++i)
    {
      if (avg_u_grid[i] == CtrecAlg::BAD_OUTPUT_VALUE ||
	  avg_v_grid[i] == CtrecAlg::BAD_OUTPUT_VALUE ||
	  smoothed_u_grid[i] == CtrecAlg::BAD_OUTPUT_VALUE ||
	  smoothed_v_grid[i] == CtrecAlg::BAD_OUTPUT_VALUE)
      {
	avg_u_grid[i] = CtrecAlg::BAD_OUTPUT_VALUE;
	avg_v_grid[i] = CtrecAlg::BAD_OUTPUT_VALUE;
      }
      else
      {
	avg_u_grid[i] = (avg_u_grid[i] + smoothed_u_grid[i]) / 2.0;
	avg_v_grid[i] = (avg_v_grid[i] + smoothed_v_grid[i]) / 2.0;
      }
    } /* endfor - i */
    
    _addVectorOutputField(output_mdv_file,
			  curr_field_hdr, curr_field->getVlevelHeader(),
			  avg_u_grid,
			  33,
			  "U temporal smoothing average",
			  "U_avg");
    
    _addVectorOutputField(output_mdv_file,
			  curr_field_hdr, curr_field->getVlevelHeader(),
			  avg_v_grid,
			  34,
			  "V temporal smoothing average",
			  "V_avg");
    
    delete avg_u_grid;
    delete avg_v_grid;
  }
  
  // Add the final vectors to the output file

  _addVectorOutputField(output_mdv_file,
			curr_field_hdr, curr_field->getVlevelHeader(),
			smoothed_u_grid,
			33,
			"U wind component",
			"U_comp");
   
  _addVectorOutputField(output_mdv_file,
			curr_field_hdr, curr_field->getVlevelHeader(),
			smoothed_v_grid,
			34,
			"V wind component",
			"V_comp");
   
  // Reclaim the space from the smoothed grids

  ufree(smoothed_u_grid);
  ufree(smoothed_v_grid);
  
  // Write the output file

  output_mdv_file.setWriteLdataInfo();
   
  if (output_mdv_file.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name;
    cerr << "Error writing data to output URL: " <<
      _params->output_url << endl;
     
    return false;
  }
   
  return true;
}


/**********************************************************************
 * _ranf() - Random number generator of Park and Miller with Bays-Durham
 *           shuffle and added safeguards.
 */

double Ctrec::_ranf(void)
{
  const int ia = 16807;
  const int im = 2147483647;
  const double am = 1.0 / (double)im;
  const int iq = 127773;
  const int ir = 2836;
  const int ntab = 32;
  const int ndiv = 1 + (im - 1) / ntab;
  const double eps = 1.2e-7;
  const double rnmx = 1.0 - eps;
  
  static int iseed = -5;
  static int iy = 0;
  static int iv[ntab];
  static bool first_time = true;
  
  int j, k;
  
  if (first_time)
  {
    for (int i = 0; i < ntab; i++)
      iv[i] = 0;
    
    first_time = false;
  }

  if (iseed <= 0 || iy == 0)
  {
    iseed = MAX(-iseed, 1);
    
    for (j = ntab + 8; j >= 1; --j)
    {
      k = iseed / iq;
      iseed = ia * (iseed - k * iq) - ir * k;
      if (iseed < 0)
	iseed = iseed + im;
      if (j <= ntab)
	iv[j - 1] = iseed;
      
    } /* endfor - j */
    
    iy = iv[0];
  } /* endif */
  
  k = iseed / iq;
  iseed = ia * (iseed - k * iq) - ir * k;
  
  if (iseed < 0)
    iseed = iseed + im;
  
  j = 1 + iy / ndiv;
  iy = iv[j-1];
  iv[j-1] = iseed;
  
  return MIN(am * (double)iy, rnmx);
  
}


/**********************************************************************
 * _readNextFile() - Read the next file to be processed.  In realtime
 *                   mode, blocks until a new file is available.
 *
 * Returns true if successful, false otherwise.
 */

bool Ctrec::_readNextFile(DsMdvx &mdv_file,
			  const time_t data_time)
{
  static const string method_name = "Ctrec::_readNextFile()";
  
  if (_params->debug)
    cerr << "Reading current data file..." << endl;

  // Set up the request

  mdv_file.clearRead();
  mdv_file.clearReadFields();

  // Set the read time

  mdv_file.setReadTime(Mdvx::READ_CLOSEST,
		       _params->input_url,
		       0,
		       data_time);
  
  // If the field name starts with '#' then interpret it as a request
  // for a field number.

  if (strncmp("#", _params->field_name, 1))
    mdv_file.addReadField(_params->field_name);
  else
    mdv_file.addReadField(atoi(_params->field_name + 1));

  mdv_file.setReadVlevelLimits(_params->analyze_alt,
			       _params->analyze_alt);
  mdv_file.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdv_file.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  mdv_file.setReadScalingType(Mdvx::SCALING_NONE);
   
  if (_params->define_output_grid)
  {
    switch (_params->output_grid.grid_type)
    {
    case Params::FLAT :
      if (_params->output_grid.dx != _params->output_grid.dy)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Output grid must have dx == dy" << endl;
	 
	exit(-1);
      }
       
      mdv_file.setReadRemapFlat(_params->output_grid.nx,
				_params->output_grid.ny,
				_params->output_grid.minx,
				_params->output_grid.miny,
				_params->output_grid.dx,
				_params->output_grid.dy,
				_params->output_grid.origin_lat,
				_params->output_grid.origin_lon,
				0.0);
       
      break;
       
    case Params::LATLON :
      mdv_file.setReadRemapLatlon(_params->output_grid.nx,
				  _params->output_grid.ny,
				  _params->output_grid.minx,
				  _params->output_grid.miny,
				  _params->output_grid.dx,
				  _params->output_grid.dy);
       
      break;
    }
    
  }
   
  if (_params->debug)
    mdv_file.printReadRequest(cerr);
   
  // Read the data file

  if (mdv_file.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading in new input file: "
	 << mdv_file.getErrStr() << endl;
    
    return false;
  }
  
  if (_params->debug)
    cout << "Read in input file: " << mdv_file.getPathInUse() << endl;
  
  return true;
}


/**********************************************************************
 * _readPrevFile() - Read the previous file to be processed.
 *
 * Returns true if successful, false otherwise.
 */

bool Ctrec::_readPrevFile(DsMdvx &mdv_file,
			  const time_t data_time)
{
  static const string method_name = "Ctrec::_readPrevFile()";
  
  if (_params->debug)
    cerr << "Reading previous data file -- data time: "
	 << DateTime::str(data_time) << endl;
  
  // Set up the request

  mdv_file.clearRead();
  mdv_file.clearReadFields();

  // Set the read time

  int search_margin = _params->max_time_between_images -
		       _params->min_time_between_images;
  
  mdv_file.setReadTime(Mdvx::READ_FIRST_BEFORE,
		       _params->input_url,
		       search_margin,
		       data_time - 
		         _params->min_time_between_images);
  
  // If the field name starts with '#' then interpret it as a request
  // for a field number.

  if (strncmp("#", _params->field_name, 1))
    mdv_file.addReadField(_params->field_name);
  else
    mdv_file.addReadField(atoi(_params->field_name + 1));

  mdv_file.setReadVlevelLimits(_params->analyze_alt,
			       _params->analyze_alt);
  mdv_file.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdv_file.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  mdv_file.setReadScalingType(Mdvx::SCALING_NONE);
   
  if (_params->define_output_grid)
  {
    switch (_params->output_grid.grid_type)
    {
    case Params::FLAT :
      if (_params->output_grid.dx != _params->output_grid.dy)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Output grid must have dx == dy" << endl;
	 
	exit(-1);
      }
       
      mdv_file.setReadRemapFlat(_params->output_grid.nx,
				_params->output_grid.ny,
				_params->output_grid.minx,
				_params->output_grid.miny,
				_params->output_grid.dx,
				_params->output_grid.dy,
				_params->output_grid.origin_lat,
				_params->output_grid.origin_lon,
				0.0);
       
      break;
       
    case Params::LATLON :
      mdv_file.setReadRemapLatlon(_params->output_grid.nx,
				  _params->output_grid.ny,
				  _params->output_grid.minx,
				  _params->output_grid.miny,
				  _params->output_grid.dx,
				  _params->output_grid.dy);
       
      break;
    }
    
  }
   
  if (_params->debug)
    mdv_file.printReadRequest(cerr);
   
  // Read the data file

  if (mdv_file.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading in new input file: " <<
      mdv_file.getErrStr() << endl;
    
    return false;
  }
  
  if (_params->debug)
    cout << "Read in input file: " << mdv_file.getPathInUse() << endl;
  
  return true;
}


/**********************************************************************
 * _trecFloor() - Replace data below the threshold (i.e. missing or weak
 *                data) with noise.
 */

void Ctrec::_trecFloor(MdvxField &field)
{
  static const string method_name = "Ctrec::_trecFloor()";
  
  int seed = time((time_t *)NULL);
  STATS_uniform_seed(seed);
  
  if (field.getFieldHeader().encoding_type != Mdvx::ENCODING_FLOAT32)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "floor method can only be run on FLOAT32 data" << endl;
    cerr << "floor method not run" << endl;
    
    return;
  }
  
  if (_params->debug)
    cout << "replacing data below threshold with noise." << endl;
  
  int nx = field.getFieldHeader().nx;
  int ny = field.getFieldHeader().ny;
  
  fl32 *image = (fl32 *)field.getVol();
  
  for (int index = 0; index < nx * ny; ++index)
  {

    if (image[index] < _params->min_echo ||
	image[index] > _params->max_echo)
      image[index] = _noiseGenerator.getNoiseValue();

  } /* endfor - index */
  
}

