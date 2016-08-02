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
//   $Id: VariationalEchoTracker.cc,v 1.3 2016/03/06 23:28:57 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * VariationalEchoTracker: VariationalEchoTracker program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <algorithm>
#include <assert.h>
#include <iostream>
#include <signal.h>
#include <math.h>
#include <string>

#include <toolsa/os_config.h>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "VariationalEchoTracker.hh"
#include "Params.hh"

// Declare the FORTRAN routine that is called below

extern "C"
{
  extern void va15ad_(int *n,
		      int *m,
		      float *x,
		      float *f,
		      float *g,
		      int *diagco_int,
		      float *diag,
		      int *iprint,
		      float *eps,
		      float *s,
		      float *y,
		      int *point,
		      float *w,
		      int *iflag);
}


using namespace std;

// Global variables

VariationalEchoTracker *VariationalEchoTracker::_instance =
     (VariationalEchoTracker *)NULL;

const int VariationalEchoTracker::MC = 3;
const int VariationalEchoTracker::MSAVE = 3;
const bool VariationalEchoTracker::DIAGCO = false;
const int VariationalEchoTracker::IPRINT[2] = { 1, 0 };
const float VariationalEchoTracker::EPSILON = 1.0e-6;


/*********************************************************************
 * Constructor
 */

VariationalEchoTracker::VariationalEchoTracker(int argc, char **argv) :
  _dataTrigger(0),
  _prevBaseField(0),
  _gnu(0),
  _gnv(0),
  _ffun1(0),
  _gm(0),
  _nunk(0),
  _conservationMatrix(0),
  _xx(0),
  _gg(0),
  _diag(0),
  _s(0),
  _y(0),
  _ww(0)
{
  static const string method_name = "VariationalEchoTracker::VariationalEchoTracker()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (VariationalEchoTracker *)NULL);
  
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
  char *params_path = "unknown";
  
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

VariationalEchoTracker::~VariationalEchoTracker()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  delete _dataTrigger;
  
  delete _prevBaseField;
  
  delete [] _gnu;
  delete [] _gnv;
  delete [] _ffun1;
  delete [] _gm;
  
  delete [] _conservationMatrix;
  
  delete [] _xx;
  delete [] _gg;
  delete [] _diag;
  delete [] _s;
  delete [] _y;
  delete [] _ww;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

VariationalEchoTracker *VariationalEchoTracker::Inst(int argc, char **argv)
{
  if (_instance == (VariationalEchoTracker *)NULL)
    new VariationalEchoTracker(argc, argv);
  
  return(_instance);
}

VariationalEchoTracker *VariationalEchoTracker::Inst()
{
  assert(_instance != (VariationalEchoTracker *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool VariationalEchoTracker::init()
{
  static const string method_name = "VariationalEchoTracker::init()";
  
  // Initialize the data trigger

  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    if (_params->debug)
      cerr << "Initializing LATEST_DATA trigger using url: " <<
	_params->base_field_info.url << endl;
    
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->base_field_info.url,
		      -1,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger using url: " <<
	_params->base_field_info.url << endl;
      
      return false;
    }

    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::TIME_LIST :
  {
    time_t start_time =
      DateTime::parseDateTime(_params->time_list_trigger.start_time);
    time_t end_time
      = DateTime::parseDateTime(_params->time_list_trigger.end_time);
    
    if (start_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing start_time string for TIME_LIST trigger: " <<
	_params->time_list_trigger.start_time << endl;
      
      return false;
    }
    
    if (end_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing end_time string for TIME_LIST trigger: " <<
	_params->time_list_trigger.end_time << endl;
      
      return false;
    }
    
    DsTimeListTrigger *trigger = new DsTimeListTrigger();
    if (trigger->init(_params->base_field_info.url,
		      start_time, end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger for url: " <<
	_params->base_field_info.url << endl;
      cerr << "    Start time: " << _params->time_list_trigger.start_time <<
	endl;
      cerr << "    End time: " << _params->time_list_trigger.end_time << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  } /* endswitch - _params->trigger_mode */
  
  // Allocate space for the diagnostic arrays

  _gnu = new double[_params->max_iterations];
  _gnv = new double[_params->max_iterations];
  _ffun1 = new double[_params->max_iterations];
  _gm = new double[_params->max_iterations];
  
  // initialize process registration

  if (_params->trigger_mode == Params::LATEST_DATA)
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void VariationalEchoTracker::run()
{
  static const string method_name = "VariationalEchoTracker::run()";
  
  while (!_dataTrigger->endOfData())
  {
    DateTime trigger_time;
  
    if (_dataTrigger->nextIssueTime(trigger_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    if (trigger_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Invalid trigger time received" << endl;
      
      continue;
    }
    
    if (!_processData(trigger_time))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing data for time: " << trigger_time << endl;
      
      continue;
    }
    
  } /* endwhile - !_dataTrigger->endOfData() */
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _calcCostFuncGradient() - Calculate the current iteration cost function
 *                           and gradients.
 */

void VariationalEchoTracker::_calcCostFuncGradient(const int iteration_num,
						   const MdvxField &prev_base_field,
						   const MdvxField &curr_base_field,
						   MdvxField &u_field,
						   MdvxField &v_field,
						   double &cost_function,
						   MdvxField &u_grad_field,
						   MdvxField &v_grad_field,
						   double *conservation_matrix) const
{
  static const string method_name = "VariationalEchoTracker::_calcCostFuncGradient()";
  
  // Calculate the cost function

  _calcCostFunction(prev_base_field, curr_base_field,
		    u_field, v_field, cost_function, conservation_matrix);
  
  // Calculate the gradients

  _calcGradients(prev_base_field, u_field, v_field,
		 u_grad_field, v_grad_field, conservation_matrix);
  
  // Calculate and print out diagnostics

  Mdvx::field_header_t u_field_hdr = u_grad_field.getFieldHeader();
  Mdvx::field_header_t v_field_hdr = v_grad_field.getFieldHeader();
  
  int u_grid_size = u_field_hdr.nx * u_field_hdr.ny;
  int v_grid_size = v_field_hdr.nx * v_field_hdr.ny;
  
  fl32 *u_grad_data = (fl32 *)u_grad_field.getVol();
  fl32 *v_grad_data = (fl32 *)v_grad_field.getVol();
  
  _gnu[iteration_num] = 0.0;
  _gnv[iteration_num] = 0.0;
  
  for (int i = 0; i < u_grid_size; ++i)
    _gnu[iteration_num] += u_grad_data[i] * u_grad_data[i];
  
  for (int i = 0; i < v_grid_size; ++i)
    _gnv[iteration_num] += v_grad_data[i] * v_grad_data[i];

  _ffun1[iteration_num] = cost_function;

  double ffd;
  if (iteration_num > 0)
    ffd = _ffun1[iteration_num] - _ffun1[iteration_num - 1];
  else
    ffd = 0.0;
  
  double ffr = _ffun1[iteration_num] / _ffun1[0];
  
  _gm[iteration_num] = sqrt(_gnv[iteration_num] + _gnu[iteration_num]);
  
  double ggr = _gm[iteration_num] / _gm[0];
  
  if (_params->debug)
  {
    cerr << "cost_function = " << cost_function << endl;
    cerr << "_gm = " << _gm[iteration_num] << endl;
    cerr << "ffd = " << ffd << endl;
    cerr << "ffr = " << ffr << endl;
    cerr << "ggr = " << ggr << endl;
    cerr << endl;
    cerr << "||del J/del x||" << endl;
    cerr << "     U         V" << endl;
    cerr << sqrt(_gnu[iteration_num]) << " " << sqrt(_gnv[iteration_num]) << endl;
    cerr << endl;
  }
}


/*********************************************************************
 * _calcCostFunction() - Calculate the current iteration cost function.
 */

void VariationalEchoTracker::_calcCostFunction(const MdvxField &prev_base_field,
					       const MdvxField &curr_base_field,
					       MdvxField &u_field,
					       MdvxField &v_field,
					       double &cost_function,
					       double *conservation_matrix) const
{
  static const string method_name = "VariationalEchoTracker::_calcCostFunction()";
  
  // Retrieve some needed information

  Mdvx::field_header_t prev_field_hdr = prev_base_field.getFieldHeader();
  Mdvx::field_header_t curr_field_hdr = curr_base_field.getFieldHeader();

  MdvxPjg projection(curr_field_hdr);
  
  fl32 *prev_data = (fl32 *)prev_base_field.getVol();
  fl32 *curr_data = (fl32 *)curr_base_field.getVol();
  
  double dt = (double)(curr_field_hdr.user_time1 - prev_field_hdr.user_time1);
  
  double dx = projection.x2km(curr_field_hdr.grid_dx);
  double dy = projection.x2km(curr_field_hdr.grid_dy);
  
  Mdvx::field_header_t u_field_hdr = u_field.getFieldHeader();
  Mdvx::field_header_t v_field_hdr = v_field.getFieldHeader();
  
  fl32 *u_data = (fl32 *)u_field.getVol();
  fl32 *v_data = (fl32 *)v_field.getVol();
  
  if (_params->debug)
  {
    cerr << "dt = " << dt << endl;
    cerr << "dx = " << dx << ", dy = " << dy << endl;
  }
  
  // Compute the cost function of reflectivity conservation

  cost_function = 0.0;
  double ffz = 0.0;
  
  for (int y = 1; y < curr_field_hdr.ny - 1; ++y)
  {
    for (int x = 1; x < curr_field_hdr.nx - 1; ++x)
    {
      // Compute the indices for all of the surrounding points

      int idx = (y * curr_field_hdr.nx) + x;
      int idxn = ((y+1) * curr_field_hdr.nx) + x;
      int idxs = ((y-1) * curr_field_hdr.nx) + x;
      int idxe = (y * curr_field_hdr.nx) + (x+1);
      int idxw = (y * curr_field_hdr.nx) + (x-1);

      int uidx = (y * u_field_hdr.nx) + x;
      int uidxw = (y * u_field_hdr.nx) + (x-1);
      
      int vidx = (y * v_field_hdr.nx) + x;
      int vidxs = ((y-1) * v_field_hdr.nx) + x;

      // Don't process missing data values

      if (curr_data[idx] == curr_field_hdr.missing_data_value ||
	  curr_data[idx] == curr_field_hdr.bad_data_value ||
	  prev_data[idx] == prev_field_hdr.missing_data_value ||
	  prev_data[idx] == prev_field_hdr.bad_data_value ||
	  prev_data[idxe] == prev_field_hdr.missing_data_value ||
	  prev_data[idxe] == prev_field_hdr.bad_data_value ||
	  prev_data[idxw] == prev_field_hdr.missing_data_value ||
	  prev_data[idxw] == prev_field_hdr.bad_data_value ||
	  prev_data[idxn] == prev_field_hdr.missing_data_value ||
	  prev_data[idxn] == prev_field_hdr.bad_data_value ||
	  prev_data[idxs] == prev_field_hdr.missing_data_value ||
	  prev_data[idxs] == prev_field_hdr.bad_data_value)
	continue;
      
      double ffz_increment = conservation_matrix[idx] *
	((curr_data[idx] - prev_data[idx]) / dt +
	 0.25 / dx * (u_data[uidx] + u_data[uidxw]) *
	 (prev_data[idxe] - prev_data[idxw]) +
	 0.25 / dy * (v_data[vidx] + v_data[vidxs]) *
	 (prev_data[idxn] - prev_data[idxs]));
      
      ffz += ffz_increment;
      
    } /* endfor - x */
  } /* endfor - y */
  
  // Compute the V-relative smoothness constraint

  double ffsmsv = 0.0;

  for (int y = 1; y < v_field_hdr.ny - 1; ++y)
  {
    for (int x = 1; x < v_field_hdr.nx - 1; ++x)
    {
      // Compute the indices for all of the surrounding points

      int idx = (y * v_field_hdr.nx) + x;
      int idxn = ((y+1) * v_field_hdr.nx) + x;
      int idxs = ((y-1) * v_field_hdr.nx) + x;
      int idxe = (y * v_field_hdr.nx) + (x+1);
      int idxw = (y * v_field_hdr.nx) + (x-1);

      double vfactor1 = v_data[idxn] - 2.0 * v_data[idx] + v_data[idxs];
      double vfactor2 = v_data[idxe] - 2.0 * v_data[idx] + v_data[idxw];
      
      ffsmsv += _params->smoothness_constraint_weights.v_weight *
	((vfactor1 * vfactor1) + (vfactor2 * vfactor2));
      
    } /* endfor - x */
  } /* endfor - y */
  
  // Compute the U-relative smoothness constraint

  double ffsmsu = 0.0;

  for (int y = 1; y < u_field_hdr.ny - 1; ++y)
  {
    for (int x = 1; x < u_field_hdr.nx - 1; ++x)
    {
      // Compute the indices for all of the surrounding points

      int idx = (y * u_field_hdr.nx) + x;
      int idxn = ((y+1) * u_field_hdr.nx) + x;
      int idxs = ((y-1) * u_field_hdr.nx) + x;
      int idxe = (y * u_field_hdr.nx) + (x+1);
      int idxw = (y * u_field_hdr.nx) + (x-1);

      double ufactor1 = u_data[idxn] - 2.0 * u_data[idx] + u_data[idxs];
      double ufactor2 = u_data[idxe] - 2.0 * u_data[idx] + u_data[idxw];
      
      ffsmsu +=	_params->smoothness_constraint_weights.u_weight *
	((ufactor1 * ufactor1) + (ufactor2 * ufactor2));

    } /* endfor - x */
  } /* endfor - y */
  
  double ffsms = ffsmsu + ffsmsv;
  
  cost_function = ffz + ffsms;
  
  if (_params->debug)
  {
    cerr << "Cost Function Total: " << cost_function << endl;
    cerr << "Cost Reflectivity: " << ffz << endl;
    cerr << endl;
    cerr << "Spatial smoothness constraint: " << ffsms << endl;
    cerr << "  U portion = " << ffsmsu << endl;
    cerr << "  V portion = " << ffsmsv << endl;
    cerr << endl;
  }
}


/*********************************************************************
 * _calcGradients() - Calculate the current iteration gradients.
 */

void VariationalEchoTracker::_calcGradients(const MdvxField &prev_base_field,
					    MdvxField &u_field,
					    MdvxField &v_field,
					    MdvxField &u_grad_field,
					    MdvxField &v_grad_field,
					    double *conservation_matrix) const
{
  // Get some information we'll need below

  Mdvx::field_header_t prev_field_hdr = prev_base_field.getFieldHeader();
  Mdvx::field_header_t u_field_hdr = u_field.getFieldHeader();
  Mdvx::field_header_t v_field_hdr = v_field.getFieldHeader();
  
  fl32 *prev_data = (fl32 *)prev_base_field.getVol();
  
  fl32 *u_data = (fl32 *)u_field.getVol();
  fl32 *v_data = (fl32 *)v_field.getVol();
  
  fl32 *u_grad_data = (fl32 *)u_grad_field.getVol();
  fl32 *v_grad_data = (fl32 *)v_grad_field.getVol();
  
  MdvxPjg projection(u_field_hdr);
  
  double dx = projection.x2km(u_field_hdr.grid_dx);
  double dy = projection.x2km(u_field_hdr.grid_dy);
  
  // Calculate the gradient of the reflectivity conservation equation

  for (int x = 1; x < prev_field_hdr.nx - 2; ++x)
  {
    for (int y = 1; y < prev_field_hdr.ny - 1; ++y)
    {
      int idx = (y * prev_field_hdr.nx) + x;
      int idxe = (y * prev_field_hdr.nx) + (x+1);
      int idxee = (y * prev_field_hdr.nx) + (x+2);
      int idxw = (y * prev_field_hdr.nx) + (x-1);
      
      int uidx = (y * u_field_hdr.nx) + x;
      
      u_grad_data[uidx] = 0.5 / dx *
	(conservation_matrix[idx] * (prev_data[idxe] - prev_data[idxw]) +
	 conservation_matrix[idxe] * (prev_data[idxee] - prev_data[idx]));
      
    } /* endfor - y */
  } /* endfor - x */
  
  for (int x = 1; x < prev_field_hdr.nx - 1; ++x)
  {
    for (int y = 1; y < prev_field_hdr.ny - 2; ++y)
    {
      int idx = (y * prev_field_hdr.nx) + x;
      int idxn = ((y+1) * prev_field_hdr.nx) + x;
      int idxnn = ((y+2) * prev_field_hdr.nx) + x;
      int idxs = ((y-1) * prev_field_hdr.nx) + x;
      
      int vidx = (y * v_field_hdr.nx) + x;
      
      v_grad_data[vidx] = 0.5 / dy *
	(conservation_matrix[idx] * (prev_data[idxn] - prev_data[idxs]) +
	 conservation_matrix[idxn] * (prev_data[idxnn] - prev_data[idx]));
      
    } /* endfor - y */
  } /* endfor - x */
  
  // Calculate the gradients of the smoothness constraint

  for (int y = 2; y < u_field_hdr.ny - 2; ++y)
  {
    for (int x = 2; x < u_field_hdr.nx - 2; ++x)
    {
      int idx = (y * u_field_hdr.nx) + x;
      int idxe = (y * u_field_hdr.nx) + (x+1);
      int idxee = (y * u_field_hdr.nx) + (x+2);
      int idxw = (y * u_field_hdr.nx) + (x-1);
      int idxww = (y * u_field_hdr.nx) + (x-2);
      int idxn = ((y+1) * u_field_hdr.nx) + x;
      int idxnn = ((y+2) * u_field_hdr.nx) + x;
      int idxs = ((y-1) * u_field_hdr.nx) + x;
      int idxss = ((y-2) * u_field_hdr.nx) + x;

      double uym2;
      double uyp2;
      double uxm2;
      double uxp2;

      if (y > 1)
	uym2 = u_data[idxss];
      else
	uym2 = 2.0 * u_data[idxs] - u_data[idx];
      
      if (y < u_field_hdr.ny - 1)
	uyp2 = u_data[idxnn];
      else
	uyp2 = 2.0 * u_data[idxn] - u_data[idx];
      
      if (x > 1)
	uxm2 = u_data[idxww];
      else
	uxm2 = 2.0 * u_data[idxw] - u_data[idx];
      
      if (x < u_field_hdr.nx)
	uxp2 = u_data[idxee];
      else
	uxp2 = 2.0 * u_data[idxe] - u_data[idx];
      
      u_grad_data[idx] = u_grad_data[idx] +
	2.0 * _params->smoothness_constraint_weights.u_weight *
	(-2.0 * (u_data[idxn] + u_data[idxs] - 2.0 * u_data[idx]) +
	 (u_data[idx] + uym2 - 2.0 * u_data[idxs]) +
	 (uyp2 + u_data[idx] - 2.0 * u_data[idxn])) +
	2.0 * _params->smoothness_constraint_weights.u_weight *
	(-2.0 * (u_data[idxe] + u_data[idxw] - 2.0 * u_data[idx]) +
	(u_data[idx] + uxm2 - 2.0 * u_data[idxw]) +
	(uxp2 + u_data[idx] - 2.0 * u_data[idxe]));
      
    } /* endfor - x */
  } /* endfor - y */
  
  for (int y = 2; y < v_field_hdr.ny - 2; ++y)
  {
    for (int x = 2; x < v_field_hdr.nx - 2; ++x)
    {
      int idx = (y * v_field_hdr.nx) + x;
      int idxe = (y * v_field_hdr.nx) + (x+1);
      int idxee = (y * v_field_hdr.nx) + (x+2);
      int idxw = (y * v_field_hdr.nx) + (x-1);
      int idxww = (y * v_field_hdr.nx) + (x-2);
      int idxn = ((y+1) * v_field_hdr.nx) + x;
      int idxnn = ((y+2) * v_field_hdr.nx) + x;
      int idxs = ((y-1) * v_field_hdr.nx) + x;
      int idxss = ((y-2) * v_field_hdr.nx) + x;

      double vym2;
      double vyp2;
      double vxm2;
      double vxp2;

      if (y > 1)
	vym2 = v_data[idxss];
      else
	vym2 = 2.0 * v_data[idxs] - v_data[idx];
      
      if (y < v_field_hdr.ny - 1)
	vyp2 = v_data[idxnn];
      else
	vyp2 = 2.0 * v_data[idxn] - v_data[idx];
      
      if (x > 1)
	vxm2 = v_data[idxww];
      else
	vxm2 = 2.0 * v_data[idxw] - v_data[idx];
      
      if (x < v_field_hdr.nx)
	vxp2 = v_data[idxee];
      else
	vxp2 = 2.0 * v_data[idxe] - v_data[idx];
      
      v_grad_data[idx] = v_grad_data[idx] +
	_params->smoothness_constraint_weights.v_weight *
	(-4.0 * (v_data[idxn] + v_data[idxs] - 2.0 * v_data[idx]) +
	 2.0 * (v_data[idx] + vym2 - 2.0 * v_data[idxs]) +
	 2.0 * (vyp2 + v_data[idx] - 2.0 * v_data[idxn])) +
	_params->smoothness_constraint_weights.v_weight *
	(-4.0 * (v_data[idxe] + v_data[idxw] - 2.0 * v_data[idx]) +
	 2.0 * (v_data[idx] + vxm2 - 2.0 * v_data[idxw]) +
	 2.0 * (vxp2 + v_data[idx] - 2.0 * v_data[idxe]));
      
    } /* endfor - x */
  } /* endfor - y */
  
}


/*********************************************************************
 * _generateMotionVectors() - Generate the U and V fields based on the
 *                            given previous and current base fields.
 *
 * Returns TRUE on success, FALSE on failure.
 */

bool VariationalEchoTracker::_generateMotionVectors(const MdvxField &prev_base_field,
						    const MdvxField &curr_base_field)
{
  static const string method_name = "VariationalEchoTracker::_processData()";
  
  // Extract some needed field information

  Mdvx::field_header_t prev_field_hdr = prev_base_field.getFieldHeader();
  Mdvx::field_header_t curr_field_hdr = curr_base_field.getFieldHeader();
  
  Mdvx::vlevel_header_t curr_vlevel_hdr = curr_base_field.getVlevelHeader();
  
  // Specify the first guess U and V fields.  Note that the U and V fields
  // are offset from the base field so that the center of the base grid square
  // is on the border between two U grid squares in the X direction and on
  // the border between two V grid squares in the Y direction.  This makes
  // the U grid larger by 1 in the X direction and the V grid larger by one
  // in the Y direction.  In the end, the final U/V fields will be calculated
  // by averaging the 2 U grid values in the X direction and the 2 V grid
  // values in the Y direction.

  Mdvx::field_header_t u_field_hdr = curr_field_hdr;
  u_field_hdr.nx = u_field_hdr.nx + 1;
  u_field_hdr.nz = 1;
  u_field_hdr.grid_minx = u_field_hdr.grid_minx - (u_field_hdr.grid_dx / 2.0);
  u_field_hdr.volume_size =
    u_field_hdr.nx * u_field_hdr.ny * u_field_hdr.data_element_nbytes;
  u_field_hdr.bad_data_value = -999.0;
  u_field_hdr.missing_data_value = -999.0;
  u_field_hdr.min_value = 0.0;
  u_field_hdr.max_value = 0.0;
  STRcopy(u_field_hdr.field_name_long, "U", MDV_LONG_FIELD_LEN);
  STRcopy(u_field_hdr.field_name, "U", MDV_SHORT_FIELD_LEN);
  STRcopy(u_field_hdr.units, "m/s", MDV_UNITS_LEN);
  u_field_hdr.transform[0] = '\0';
  MdvxField u_field(u_field_hdr, curr_vlevel_hdr);

  Mdvx::field_header_t v_field_hdr = curr_field_hdr;
  v_field_hdr.ny = v_field_hdr.ny + 1;
  v_field_hdr.nz = 1;
  v_field_hdr.grid_miny = v_field_hdr.grid_miny - (v_field_hdr.grid_dy / 2.0);
  v_field_hdr.volume_size =
    v_field_hdr.nx * v_field_hdr.ny * v_field_hdr.data_element_nbytes;
  v_field_hdr.bad_data_value = -999.0;
  v_field_hdr.missing_data_value = -999.0;
  v_field_hdr.min_value = 0.0;
  v_field_hdr.max_value = 0.0;
  STRcopy(v_field_hdr.field_name_long, "V", MDV_LONG_FIELD_LEN);
  STRcopy(v_field_hdr.field_name, "V", MDV_SHORT_FIELD_LEN);
  STRcopy(v_field_hdr.units, "m/s", MDV_UNITS_LEN);
  v_field_hdr.transform[0] = '\0';
  MdvxField v_field(v_field_hdr, curr_vlevel_hdr);
  
  if (!_getFirstGuessUV(u_field, v_field))
    return false;
  
  // Create and initialize the U/V gradient fields.  Initialize all of
  // the field values to 0 because we don't calculate gradients along the
  // edges in the later code.

  Mdvx::field_header_t u_grad_hdr = u_field_hdr;
  STRcopy(u_grad_hdr.field_name_long, "U gradient", MDV_LONG_FIELD_LEN);
  STRcopy(u_grad_hdr.field_name, "U grad", MDV_SHORT_FIELD_LEN);
  STRcopy(u_grad_hdr.units, "none", MDV_UNITS_LEN);
  MdvxField u_grad_field(u_grad_hdr, curr_vlevel_hdr);
  fl32 *u_grad_data = (fl32 *)u_grad_field.getVol();
  memset(u_grad_data, 0, u_grad_hdr.nx * u_grad_hdr.ny * sizeof(fl32));
  
  Mdvx::field_header_t v_grad_hdr = v_field_hdr;
  STRcopy(v_grad_hdr.field_name_long, "V gradient", MDV_LONG_FIELD_LEN);
  STRcopy(v_grad_hdr.field_name, "V grad", MDV_SHORT_FIELD_LEN);
  STRcopy(v_grad_hdr.units, "none", MDV_UNITS_LEN);
  MdvxField v_grad_field(v_grad_hdr, curr_vlevel_hdr);
  fl32 *v_grad_data = (fl32 *)v_grad_field.getVol();
  memset(v_grad_data, 0, v_grad_hdr.nx * v_grad_hdr.ny * sizeof(fl32));
  
  // Initialize work arrays before the iterations begin

  _initializeWorkArrays(u_field_hdr, v_field_hdr, MdvxPjg(prev_field_hdr));
  
  // Minimize the cost function and gradients

  double cost_function;
    
  for (int iteration_num = 0; iteration_num < _params->max_iterations;
       ++iteration_num)
  {
    if (_params->debug)
      cerr << "----- ITERATION " << iteration_num << " -----" << endl;
    
    // Calculate the current iteration cost function and gradients

    _calcCostFuncGradient(iteration_num, prev_base_field, curr_base_field,
			  u_field, v_field,
			  cost_function, u_grad_field, v_grad_field,
			  _conservationMatrix);
    
    // Minimize the functions

    int ret = _performMinimization(u_field, v_field,
				   u_grad_field, v_grad_field,
				   cost_function);
    
    if (_params->debug)
      cerr << "minimization return = " << ret << endl;
    
    if (ret <= 0)
      break;
    
    if (_params->debug)
      cerr << " FINISHED ITERATION " << iteration_num << endl;
    
  } /* endfor - iteration_num */
  
  // Write out the motion vectors

  if (!_writeVectors(u_field, v_field, u_grad_field, v_grad_field))
    return false;
  
  if (_params->debug)
  {
    cerr << "*****************************" << endl;
    cerr << "** Minimization terminated **" << endl;
    cerr << "*****************************" << endl;
    cerr << endl;
  }
  
  return true;
}


/*********************************************************************
 * _getFirstGuessUV() - Specify the first guess U and V fields.
 *
 * Returns TRUE on success, FALSE on failure.
 */

bool VariationalEchoTracker::_getFirstGuessUV(MdvxField &u_field,
					      MdvxField &v_field) const
{
  static const string method_name = "VariationalEchoTracker::_getFirstGuessUV()";
  
  Mdvx::field_header_t u_field_hdr = u_field.getFieldHeader();
  fl32 *u_data = (fl32 *)u_field.getVol();
  int u_grid_size = u_field_hdr.nx * u_field_hdr.ny;

  for (int i = 0; i < u_grid_size; ++i)
    u_data[i] = _params->constant_motion.u;
  
  Mdvx::field_header_t v_field_hdr = v_field.getFieldHeader();
  fl32 *v_data = (fl32 *)v_field.getVol();
  int v_grid_size = v_field_hdr.nx * v_field_hdr.ny;

  for (int i = 0; i < v_grid_size; ++i)
    v_data[i] = _params->constant_motion.v;
  
  return true;
}


/*********************************************************************
 * _initializeWorkArrays() - Initialize the arrays used in the minimization
 * process.
 */

void VariationalEchoTracker::_initializeWorkArrays(const Mdvx::field_header_t &u_field_hdr,
						   const Mdvx::field_header_t &v_field_hdr,
						   const MdvxPjg &base_projection)
{
  delete [] _conservationMatrix;
  
  delete [] _xx;
  delete [] _gg;
  delete [] _diag;
  delete [] _s;
  delete [] _y;
  delete [] _ww;
  
  int u_grid_size = (u_field_hdr.nx - 2) * (u_field_hdr.ny - 2);
  int v_grid_size = (v_field_hdr.nx - 2) * (v_field_hdr.ny - 2);
  _nunk = u_grid_size + v_grid_size;
  int nxm = _nunk * MSAVE;
    
  // Initialize the reflectivity conservation term matrix.

  int matrix_size = base_projection.getNx() * base_projection.getNy();
  
  _conservationMatrix = new double[matrix_size];
  
  for (int i = 0; i < matrix_size; ++i)
    _conservationMatrix[i] = _params->conservation_constraint_weight;
  
  // Initialize the arrays used in the minimization routine

  _xx = new float[_nunk];
  _gg = new float[_nunk];
  _diag = new float[_nunk];
  _s = new float[nxm];
  _y = new float[nxm];
  _ww = new float[_nunk + 2 * MSAVE];

  memset(_xx, 0, _nunk * sizeof(float));
  memset(_gg, 0, _nunk * sizeof(float));
  
}


/*********************************************************************
 * _performMinimization() - Perform the minimization
 *
 * Returns the minimization flag.
 */

int VariationalEchoTracker::_performMinimization(MdvxField &u_field,
						 MdvxField &v_field,
						 const MdvxField &u_grad_field,
						 const MdvxField &v_grad_field,
						 float cost_function)
{
  Mdvx::field_header_t u_field_hdr = u_field.getFieldHeader();
  Mdvx::field_header_t v_field_hdr = v_field.getFieldHeader();
  
  // Fill in the XX and GG arrays

  fl32 *u_data = (fl32 *)u_field.getVol();
  fl32 *v_data = (fl32 *)v_field.getVol();
  fl32 *u_grad_data = (fl32 *)u_grad_field.getVol();
  fl32 *v_grad_data = (fl32 *)v_grad_field.getVol();
    
  for (int x = 1; x < u_field_hdr.nx - 1; ++x)
  {
    for (int y = 1; y < u_field_hdr.ny - 1; ++y)
    {
      int xx_idx = (y - 1) + (x - 1) * (u_field_hdr.ny - 2);
      int u_idx = y * u_field_hdr.nx + x;
      
      _xx[xx_idx] = u_data[u_idx];
      _gg[xx_idx] = u_grad_data[u_idx];
    } /* endfor - y */
  } /* endfor - x */
  
  int u_xx_grid_size = (u_field_hdr.nx - 2) * (u_field_hdr.ny - 2);
  
  for (int x = 1; x < v_field_hdr.nx - 1; ++x)
  {
    for (int y = 1; y < v_field_hdr.ny - 1; ++y)
    {
      int xx_idx = (y - 1) + (x - 1) * (v_field_hdr.ny - 2) + u_xx_grid_size;
      int v_idx = y * v_field_hdr.nx + x;
      
      _xx[xx_idx] = v_data[v_idx];
      _gg[xx_idx] = v_grad_data[v_idx];
    } /* endfor - y */
  } /* endfor - x */
  
  // Call the minimization routine

  int mc = MC;
  int point;
  int iflag = 0;
  int iprint[2];
  iprint[0] = IPRINT[0];
  iprint[1] = IPRINT[1];
  float epsilon = EPSILON;
  int diagco_int;
  
  if (DIAGCO)
    diagco_int = 1;
  else
    diagco_int = 0;
  
  va15ad_(&_nunk, &mc, _xx, &cost_function, _gg, &diagco_int, _diag, iprint,
	  &epsilon, _s, _y, &point, _ww, &iflag);

  if (_params->debug)
    cerr << "  IFLAG = " << iflag << endl;
    
  // Save the updated U/V values

  for (int x = 1; x < u_field_hdr.nx - 1; ++x)
  {
    for (int y = 1; y < u_field_hdr.ny - 1; ++y)
    {
      int xx_idx = (y - 1) + (x - 1) * (u_field_hdr.ny - 2);
      int u_idx = y * u_field_hdr.nx + x;
      
      u_data[u_idx] = _xx[xx_idx];
    } /* endfor - y */
  } /* endfor - x */
  
  for (int x = 1; x < v_field_hdr.nx - 1; ++x)
  {
    for (int y = 1; y < v_field_hdr.ny - 1; ++y)
    {
      int xx_idx = (y - 1) + (x - 1) * (v_field_hdr.ny - 2) + u_xx_grid_size;
      int v_idx = y * v_field_hdr.nx + x;
      
      v_data[v_idx] = _xx[xx_idx];
    } /* endfor - y */
  } /* endfor - x */
  
  return iflag;
}


/*********************************************************************
 * _processData() - Process the data for the given time.
 *
 * Returns TRUE on success, FALSE on failure.
 */

bool VariationalEchoTracker::_processData(const DateTime &trigger_time)
{
  static const string method_name = "VariationalEchoTracker::_processData()";
  
  if (_params->debug)
    cerr << "\n*** Processing data for time: " << trigger_time << endl;
  
  // Read in the current base field information

  MdvxField *curr_base_field;
  
  if ((curr_base_field = _readBaseField(trigger_time)) == 0)
  {
    delete _prevBaseField;
    _prevBaseField = 0;
    
    return false;
  }
  
  // If we don't have a previous field, save the current field for future
  // processing and return

  if (_prevBaseField == 0)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "No previous base field information available for processing." << endl;
    cerr << "Saving the current field and waiting for new data." << endl;
    
    _prevBaseField = curr_base_field;
    
    return true;
  }

  // Make sure that the projections of the two fields match so we can
  // process them

  Mdvx::field_header_t prev_field_hdr = _prevBaseField->getFieldHeader();
  Mdvx::field_header_t curr_field_hdr = curr_base_field->getFieldHeader();
  
  MdvxPjg prev_proj(prev_field_hdr);
  MdvxPjg curr_proj(curr_field_hdr);
  
  if (prev_proj != curr_proj)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Projections for previous and current base fields don't match" << endl;
    cerr << "Skipping this time period..." << endl;
    
    delete _prevBaseField;
    _prevBaseField = curr_base_field;
    
    return false;
  }
  
  // Generate the motion vectors based on the base field values

  if (!_generateMotionVectors(*_prevBaseField, *curr_base_field))
  {
    delete _prevBaseField;
    _prevBaseField = curr_base_field;
    
    return false;
  }
  
  // Save the current base field for processing in the next time period

  delete _prevBaseField;
  _prevBaseField = curr_base_field;
  
  return true;
}


/*********************************************************************
 * _readBaseField() - Read the current base field data.
 *
 * Returns a pointer to the read field on success, 0 on failure.
 */

MdvxField *VariationalEchoTracker::_readBaseField(const DateTime &data_time)
{
  static const string method_name = "VariationalEchoTracker::_readBaseField()";
  
  // Read the data volume

  DsMdvx mdv;
  
  mdv.setReadTime(Mdvx::READ_CLOSEST,
		  _params->base_field_info.url,
		  0, data_time.utime());
  
  if (_params->base_field_info.use_field_name)
    mdv.addReadField(_params->base_field_info.field_name);
  else
    mdv.addReadField(_params->base_field_info.field_num);
  
  mdv.setReadNoChunks();
  
  mdv.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdv.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  mdv.setReadScalingType(Mdvx::SCALING_NONE);
  
  if (_params->debug)
    mdv.printReadRequest(cerr);
  
  if (mdv.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading volume:" << endl;
    cerr << "   URL: " << _params->base_field_info.url << endl;
    cerr << "   time: " << data_time << endl;
    
    return 0;
  }
  
  // Save a copy of the field

  MdvxField *base_field = mdv.getField(0);
  
  if (base_field == 0)
    return 0;
  
  MdvxField *return_field = new MdvxField(*base_field);
  
  // Set the user_time1 field in the field header to be the master header
  // time centroid so we can use this field later on to calculate dt.  Save
  // the master header so that we can use some of the information in our
  // output file.

  _currMasterHdr = mdv.getMasterHeader();
  Mdvx::field_header_t field_hdr = return_field->getFieldHeader();
  
  field_hdr.user_time1 = _currMasterHdr.time_centroid;
  return_field->setFieldHeader(field_hdr);
  
  return return_field;
}


/*********************************************************************
 * _writeVectors() - Write the calculated vectors out to the
 *                   appropriate URL.
 *
 * Returns true on success, false on failure
 */

bool VariationalEchoTracker::_writeVectors(const MdvxField &u_field,
					   const MdvxField &v_field,
					   const MdvxField &u_grad_field,
					   const MdvxField &v_grad_field) const
{
  static const string method_name = "VariationalEchoTracker::_writeVectors()";
  
  DsMdvx mdvx;
  
  // Generate the output master header

  Mdvx::master_header_t master_hdr = _currMasterHdr;
  
  master_hdr.num_data_times = 0;
  master_hdr.index_number = 0;
  master_hdr.data_dimension = 2;
  master_hdr.data_collection_type = Mdvx::DATA_EXTRAPOLATED;
  master_hdr.n_fields = 0;
  master_hdr.max_nx = 0;
  master_hdr.max_ny = 0;
  master_hdr.max_nz = 0;
  STRcopy(master_hdr.data_set_info, "Generated by VariationalEchoTracker",
	  MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "VariationalEchoTracker", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, "VariationalEchoTracker", MDV_NAME_LEN);
  
  mdvx.setMasterHeader(master_hdr);
  
  // Generate the output U field.  The algorithm uses an offset U field,
  // so here we must average the calculated U fields to get the output U
  // field.  The algorithm U field is offset in the X direction.

  Mdvx::field_header_t out_u_field_hdr = u_field.getFieldHeader();
  
  out_u_field_hdr.nx -= 1;
  out_u_field_hdr.volume_size =
    out_u_field_hdr.nx * out_u_field_hdr.ny * out_u_field_hdr.nz *
    out_u_field_hdr.data_element_nbytes;
  out_u_field_hdr.grid_minx += out_u_field_hdr.grid_dx / 2.0;
  
  MdvxField *out_u_field = new MdvxField(out_u_field_hdr,
					 u_field.getVlevelHeader());
  
  fl32 *u_data = (fl32 *)u_field.getVol();
  fl32 *out_u_data = (fl32 *)out_u_field->getVol();
  
  for (int x = 0; x < out_u_field_hdr.nx; ++x)
  {
    for (int y = 0; y < out_u_field_hdr.ny; ++y)
    {
      int out_idx = (y * out_u_field_hdr.nx) + x;
      int idx1 = (y * (out_u_field_hdr.nx + 1)) + x;
      int idx2 = (y * (out_u_field_hdr.nx + 1)) + (x + 1);
      
      out_u_data[out_idx] = (u_data[idx1] + u_data[idx2]) / 2.0;

    } /* endfor - y */
  } /* endfor - x */
  
  out_u_field->convertType(Mdvx::ENCODING_INT8,
			   Mdvx::COMPRESSION_BZIP,
			   Mdvx::SCALING_DYNAMIC);
  
  mdvx.addField(out_u_field);
  
  // Generate the output V field.  The algorithm V field is offset in
  // the Y direction.

  Mdvx::field_header_t out_v_field_hdr = v_field.getFieldHeader();
  
  out_v_field_hdr.ny -= 1;
  out_v_field_hdr.volume_size =
    out_v_field_hdr.nx * out_v_field_hdr.ny * out_v_field_hdr.nz *
    out_v_field_hdr.data_element_nbytes;
  out_v_field_hdr.grid_miny += out_v_field_hdr.grid_dy / 2.0;
  
  MdvxField *out_v_field = new MdvxField(out_v_field_hdr,
					 v_field.getVlevelHeader());
  
  fl32 *v_data = (fl32 *)v_field.getVol();
  fl32 *out_v_data = (fl32 *)out_v_field->getVol();
  
  for (int x = 0; x < out_v_field_hdr.nx; ++x)
  {
    for (int y = 0; y < out_v_field_hdr.ny; ++y)
    {
      int out_idx = (y * out_v_field_hdr.nx) + x;
      int idx1 = (y * out_v_field_hdr.nx) + x;
      int idx2 = ((y + 1) * out_v_field_hdr.nx) + x;
      
      out_v_data[out_idx] = (v_data[idx1] + v_data[idx2]) / 2.0;
    } /* endfor - y */
  } /* endfor - x */
  
  out_v_field->convertType(Mdvx::ENCODING_INT8,
			   Mdvx::COMPRESSION_BZIP,
			   Mdvx::SCALING_DYNAMIC);
  
  mdvx.addField(out_v_field);
  
  // Add the U and V gradient fields to the output file, if requested

  if (_params->output_gradients)
  {
    MdvxField *out_u_grad_field = new MdvxField(u_grad_field);
    
    out_u_grad_field->convertType(Mdvx::ENCODING_INT8,
				  Mdvx::COMPRESSION_BZIP,
				  Mdvx::SCALING_DYNAMIC);
    
    mdvx.addField(out_u_grad_field);
    
    MdvxField *out_v_grad_field = new MdvxField(v_grad_field);
    
    out_v_grad_field->convertType(Mdvx::ENCODING_INT8,
				  Mdvx::COMPRESSION_BZIP,
				  Mdvx::SCALING_DYNAMIC);
    
    mdvx.addField(out_v_grad_field);
  }
  
  // Write the file to the specified URL

  if (mdvx.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing vectors to output URL: "
	 << _params->output_url << endl;
    
    return false;
  }
  
  return true;
}
