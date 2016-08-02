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
/**
 *
 * @file RefractCalib.cc
 *
 * @class RefractCalib
 *
 * Main application class
 *  
 * @date 1/15/2009
 *
 */

#include <assert.h>
#include <iostream>
#include <math.h>
#include <memory>
#include <signal.h>
#include <string>

#include <toolsa/os_config.h>
#include <rapmath/math_macros.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "RefractCalib.hh"
#include "Params.hh"

#include "Input.hh"

using namespace std;

// Global variables

RefractCalib *RefractCalib::_instance =
     (RefractCalib *)NULL;


/*********************************************************************
 * Constructor
 */

RefractCalib::RefractCalib(int argc, char **argv)
{
  static const string method_name = "RefractCalib::RefractCalib()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (RefractCalib *)NULL);
  
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
  
  // Get TDRP parameters.

  _params = new Params();
  char *params_path = new char[strlen("unknown") + 1];
  strcpy(params_path, "unknown");
  
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with TDRP parameters in file: " << params_path << endl;
    
    okay = false;
    
    return;
  }
  
}


/*********************************************************************
 * Destructor
 */

RefractCalib::~RefractCalib()
{
  // Free contained objects

  delete _params;
  delete _args;
  
  // Free included strings

  STRfree(_progName);

}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

RefractCalib *RefractCalib::Inst(int argc, char **argv)
{
  if (_instance == (RefractCalib *)NULL)
    new RefractCalib(argc, argv);
  
  return(_instance);
}

RefractCalib *RefractCalib::Inst()
{
  assert(_instance != (RefractCalib *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool RefractCalib::init()
{
  static const string method_name = "RefractCalib::init()";
  
  // Create the requested colorscale files

  if (_params->create_strength_colorscale)
    _createStrengthColorscale();

  if (_params->create_quality_colorscale)
    _createQualityColorscale();
  
  // Initialize the input handler

  Input *input_handler = 0;
  
  if (_params->specify_elevation_by_index)
    input_handler = new Input(_params->raw_iq_in_input,
			      _params->raw_i_field_name,
			      _params->raw_q_field_name,
			      _params->niq_field_name,
			      _params->aiq_field_name,
			      _params->snr_field_name,
			      _params->input_niq_scale,
			      _params->invert_target_angle_sign,
			      _params->elevation_num,
			      _params->num_azim,
			      _params->num_range_bins,
			      _params->debug_level >= Params::DEBUG_NORM,
			      _params->debug_level >= Params::DEBUG_VERBOSE);
  else
    input_handler = new Input(_params->raw_iq_in_input,
			      _params->raw_i_field_name,
			      _params->raw_q_field_name,
			      _params->niq_field_name,
			      _params->aiq_field_name,
			      _params->snr_field_name,
			      _params->input_niq_scale,
			      _params->invert_target_angle_sign,
			      _params->elevation_angle.min_angle,
			      _params->elevation_angle.max_angle,
			      _params->num_azim,
			      _params->num_range_bins,
			      _params->debug_level >= Params::DEBUG_NORM,
			      _params->debug_level >= Params::DEBUG_VERBOSE);

  // Initialize the processor object

  if (!_processor.init(_params->num_azim, _params->num_range_bins,
		       _params->r_min, _params->beam_width,
		       _params->side_lobe_pow,
		       _params->ref_file_path,
		       input_handler,
		       _params->debug_level >= Params::DEBUG_NORM,
		       _params->debug_level >= Params::DEBUG_EXTRA))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error initializing processor object" << endl;
    
    return false;
  }
  
  if (_params->write_debug_mdv_files)
    _processor.setDebugMdvUrl(_params->debug_mdv_url);
  
  switch (_params->entry_type)
  {
  case Params::ENTER_N :
    _processor.setNValue(_params->calib_n);
    break;
    
  case Params::ENTER_P_T_TD :
    _processor.setNValue(_params->calib_pressure,
			 _params->calib_temperature,
			 _params->calib_dewpoint_temperature);
    break;
  }
  
  return true;
}


/*********************************************************************
 * run()
 */

void RefractCalib::run()
{
  static const string method_name = "RefractCalib::run()";
  
  // Do the target identification step

  vector< string > target_id_file_list;
  
  for (int i = 0; i < _params->target_id_file_list_n; ++i)
    target_id_file_list.push_back(_params->_target_id_file_list[i]);
  
  double input_gate_spacing;
  
  if (!_processor.findReliableTargets(target_id_file_list, input_gate_spacing))
    return;
  
  // Do the calibration step

  vector< string > calib_file_list;
  
  for (int i = 0; i < _params->calibration_file_list_n; ++i)
    calib_file_list.push_back(_params->_calibration_file_list[i]);
  
  if (!_processor.calibTargets(calib_file_list, input_gate_spacing))
    return;
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _createStrengthColorscale()
 */

void RefractCalib::_createStrengthColorscale() const
{
  static const string method_name = "RefractCalib::_createStrengthColorscale()";
  
  FILE *file;

  if ((file = fopen(_params->strength_colorscale_path, "w")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening colorscale file for output" << endl;
    perror(_params->strength_colorscale_path);
    
    return;
  }
  
  for (int i = 0; i < 120; ++i)
  {
    float step = 6.0 * ((float)i / 120.0);
    
    short int red, green, blue;

    red = (short int)(step * 255.0 / 6.0);
    green = red;
    blue = red;
    
    double min_value = ((80.0 / 120.0) * (double)i) - 10.0;
    double max_value = ((80.0 / 120.0) * (double)(i+1)) - 10.0;
    
    fprintf(file, "%8.7f  %8.7f   #%02x%02x%02x\n",
	    min_value, max_value, red, green, blue);
  }
  
  fclose(file);
}


/*********************************************************************
 * _createQualityColorscale()
 */

void RefractCalib::_createQualityColorscale() const
{
  static const string method_name = "RefractCalib::_createQualithColorscale()";
  
  FILE *file;

  if ((file = fopen(_params->quality_colorscale_path, "w")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening colorscale file for output" << endl;
    perror(_params->quality_colorscale_path);
    
    return;
  }
  
  for (int i = 0; i < 120; ++i)
  {
    float step = 6.0 * ((float)i / 120.0);
    
    short int red, green, blue;

    red = (short int)(step * 255.0 / 6.0);
    green = red;
    blue = red;
    
    double min_value = ((1.0 / 120.0) * (double)i);
    double max_value = ((1.0 / 120.0) * (double)(i+1));
    
    fprintf(file, "%8.7f  %8.7f   #%02x%02x%02x\n",
	    min_value, max_value, red, green, blue);
  }
  
  fclose(file);
}
