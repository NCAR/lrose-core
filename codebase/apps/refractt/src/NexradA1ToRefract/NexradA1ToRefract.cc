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
//   $Author: jcraig $
//   $Locker:  $
//   $Date: 2018/01/26 20:33:40 $
//   $Id: NexradA1ToRefract.cc,v 1.17 2018/01/26 20:33:40 jcraig Exp $
//   $Revision: 1.17 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * NexradA1ToRefract: NexradA1ToRefract program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

///////////////////////////////////////////////////////////////////////
//
// This program reads netCDF floating point time-series files produced
// by the NEXRAD A1DA converter program and calculates the variables
// needed for the Frederic Fabry's refractivity algorithm.
//
////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <iostream>
#include <csignal>
#include <cmath>
#include <string>
#include <cstring>

#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsInputDirTrigger.hh>

#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "NexradA1ToRefract.hh"
#include "SweepFile.hh"

using namespace std;

// Global variables

const string NexradA1ToRefract::TIME_DIM_NAME = "Time";
const string NexradA1ToRefract::GATES_DIM_NAME = "Gates";

const string NexradA1ToRefract::I_VAR_NAME = "I";
const string NexradA1ToRefract::Q_VAR_NAME = "Q";
const string NexradA1ToRefract::SAMPLE_NUM_VAR_NAME = "SampleNum";
const string NexradA1ToRefract::AZIMUTH_VAR_NAME = "Azimuth";
const string NexradA1ToRefract::ELEVATION_VAR_NAME = "Elevation";
const string NexradA1ToRefract::PRT_VAR_NAME = "Prt";
const string NexradA1ToRefract::TIME_VAR_NAME = "Time";
const string NexradA1ToRefract::UNIX_TIME_VAR_NAME = "UnixTime";
const string NexradA1ToRefract::NANO_SEC_VAR_NAME = "NanoSeconds";
const string NexradA1ToRefract::FLAGS_VAR_NAME = "Flags";

const double NexradA1ToRefract::RADAR_LAT = 39.7867;
const double NexradA1ToRefract::RADAR_LON = -104.5458;
const double NexradA1ToRefract::RADAR_ALT = 1.7102;

const int NexradA1ToRefract::NUM_AZIMUTHS = 360;
const double NexradA1ToRefract::WAVELENGTH = 0.1075;
const int NexradA1ToRefract::SAMPLES_PER_BEAM = 64;     /******/
const double NexradA1ToRefract::RADAR_CONSTANT = 11.0;
const double NexradA1ToRefract::POWER_CONSTANT = -57.8;
const double NexradA1ToRefract::START_RANGE = 0.625;
const double NexradA1ToRefract::GATE_SPACING = 0.25;

NexradA1ToRefract *NexradA1ToRefract::_instance =
     (NexradA1ToRefract *)NULL;


/*********************************************************************
 * Constructor
 */

NexradA1ToRefract::NexradA1ToRefract(int argc, char **argv) :
  _numGates(0),
  _beamDuration(new double[NUM_AZIMUTHS]),
  _power(0),
  _velocity(0),
  _velA(0),
  _velB(0),
  _avgI(0),
  _avgQ(0),
  _niq(0),
  _aiq(0),
  _z(0),
  _ncp(0),
  _sw(0),
  _waitingForSweepStart(false),
  _sweepStartAzimuth(-1),
  _sweepStartTime(0),
  _sweepEndTime(0),
  _sweepPrt(0),
  _prevSweepEndTime(0),
  _currOutputAzimuth(-1),
  _beamStartTime(0),
  _beamStartNanoseconds(0),
  _beamEndTime(0),
  _beamEndNanoseconds(0),
  _elevationTotal(0.0),
  _numSamples(0)
{
  static const string method_name = "NexradA1ToRefract::NexradA1ToRefract()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (NexradA1ToRefract *)NULL);
  
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

  _args = new Args(argc, argv, (char *) "NexradA1ToRefract");

  // Get TDRP parameters.

  _params = new Params();
  char *params_path = (char *) "unknown";
  
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

NexradA1ToRefract::~NexradA1ToRefract()
{
  // Free contained objects

  delete _args;

  delete [] _azimuth;
  delete [] _elevation;
  delete [] _beamDuration;

  delete [] _power;
  delete [] _velocity;
  delete [] _velA;
  delete [] _velB;
  delete [] _avgI;
  delete [] _avgQ;
  delete [] _niq;
  delete [] _aiq;
  delete [] _z;
  delete [] _ncp;
  delete [] _sw;
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

NexradA1ToRefract *NexradA1ToRefract::Inst(int argc, char **argv)
{
  if (_instance == (NexradA1ToRefract *)NULL)
    new NexradA1ToRefract(argc, argv);
  
  return(_instance);
}

NexradA1ToRefract *NexradA1ToRefract::Inst()
{
  assert(_instance != (NexradA1ToRefract *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool NexradA1ToRefract::init()
{
  static const string method_name = "NexradA1ToRefract::init()";
  
  // Initialize the azimuth and elevation arrays

  _azimuth = new float[NUM_AZIMUTHS];
  for (int i = 0; i < NUM_AZIMUTHS; ++i)
    _azimuth[i] = (float)i;

  _elevation = new float[NUM_AZIMUTHS];
  memset(_elevation, 0, NUM_AZIMUTHS * sizeof(float));

  // Initialize the data trigger

  switch (_params->trigger_mode)
  {
  case Params::FILE_LIST :
  {
    DsFileListTrigger *trigger = new DsFileListTrigger();
    if (trigger->init(_args->getInputFileList()) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing FILE_LIST trigger" << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::INPUT_DIR :
  {
    DsInputDirTrigger *trigger = new DsInputDirTrigger();
    if (trigger->init(_params->input_dir,
		      "", false,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing INPUT_DIR trigger" << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  } /* endswitch - _params->trigger_mode */
  
    
  // initialize process registration

  if (_params->trigger_mode != Params::FILE_LIST)
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void NexradA1ToRefract::run()
{
  static const string method_name = "NexradA1ToRefract::run()";
  
  while (!_dataTrigger->endOfData())
  {
    TriggerInfo trigger_info;
    
    if (_dataTrigger->next(trigger_info) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger information" << endl;
      cerr << "Trying again...." << endl;
      
      continue;
    }
    
    _processFile(trigger_info.getFilePath());

  } /* endwhile - !_dataTrigger->endOfData() */
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _extractValues() - Extract the values for the specified variable
 *                    from the netCDF file.
 *
 * Returns a pointer to the values on success, 0 on failure.
 */

Nc3Values *NexradA1ToRefract::_extractValues(const string &var_name,
				     Nc3File &input_file,
				     const string &input_file_path) const
{
  static const string method_name = "NexradA1ToRefract::_extractValues()";

  Nc3Var *variable = 0;

  if ((variable = input_file.get_var(var_name.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting " << var_name 
	 << " variable from input file: " << input_file_path << endl;

    return 0;
  }

  if (!variable->is_valid())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << var_name << " var from input file is invalid: "
	 << input_file_path << endl;

    return 0;
  }

  return variable->values();
}


/*********************************************************************
 * _initializeArrays() - Initialize the arrays of calculated fields.
 */

void NexradA1ToRefract::_initializeArrays(const int num_gates)
{
  if (_params->debug)
    cerr << "Initializing arrays at " << num_gates << " gates" << endl;
  
  // Allocate space for the arrays, only if the number of gates
  // has changed.

  if (_numGates != num_gates)
  {
    delete [] _power;
    _power = new float[NUM_AZIMUTHS * num_gates];

    delete [] _velocity;
    _velocity = new float[NUM_AZIMUTHS * num_gates];

    delete [] _velA;
    _velA = new float[NUM_AZIMUTHS *num_gates];
    
    delete [] _velB;
    _velB = new float[NUM_AZIMUTHS * num_gates];

    delete [] _avgI;
    _avgI = new float[NUM_AZIMUTHS * num_gates];

    delete [] _avgQ;
    _avgQ = new float[NUM_AZIMUTHS * num_gates];

    delete [] _niq;
    _niq = new float[NUM_AZIMUTHS * num_gates];

    delete [] _aiq;
    _aiq = new float[NUM_AZIMUTHS * num_gates];

    delete [] _z;
    _z = new float[NUM_AZIMUTHS * num_gates];

    delete [] _ncp;
    _ncp = new float[NUM_AZIMUTHS * num_gates];

    delete [] _sw;
    _sw = new float[NUM_AZIMUTHS * num_gates];

    _numGates = num_gates;
  }

  // Initialize the arrays.

  for (int i = 0; i < NUM_AZIMUTHS * _numGates; ++i)
  {
    _power[i] = SweepFile::MISSING_DATA_VALUE;
    _velocity[i] = SweepFile::MISSING_DATA_VALUE;
    _velA[i] = SweepFile::MISSING_DATA_VALUE;
    _velB[i] = SweepFile::MISSING_DATA_VALUE;
    _avgI[i] = SweepFile::MISSING_DATA_VALUE;
    _avgQ[i] = SweepFile::MISSING_DATA_VALUE;
    _niq[i] = SweepFile::MISSING_DATA_VALUE;
    _aiq[i] = SweepFile::MISSING_DATA_VALUE;
    _z[i] = SweepFile::MISSING_DATA_VALUE;
    _ncp[i] = SweepFile::MISSING_DATA_VALUE;
    _sw[i] = SweepFile::MISSING_DATA_VALUE;
  } /* endfor - i */

//  memset(_power, 0, NUM_AZIMUTHS * _numGates * sizeof(float));
//  memset(_velocity, 0, NUM_AZIMUTHS * _numGates * sizeof(float));
//  memset(_velA, 0,NUM_AZIMUTHS *  _numGates * sizeof(float));
//  memset(_velB, 0, NUM_AZIMUTHS * _numGates * sizeof(float));
//  memset(_avgI, 0, NUM_AZIMUTHS * _numGates * sizeof(float));
//  memset(_avgQ, 0, NUM_AZIMUTHS * _numGates * sizeof(float));
//  memset(_niq, 0, NUM_AZIMUTHS * _numGates * sizeof(float));
//  memset(_aiq, 0, NUM_AZIMUTHS * _numGates * sizeof(float));
//  memset(_z, 0, NUM_AZIMUTHS * _numGates * sizeof(float));
//  memset(_ncp, 0, NUM_AZIMUTHS * _numGates * sizeof(float));
//  memset(_sw, 0, NUM_AZIMUTHS * _numGates * sizeof(float));

  memset(_elevation, 0, NUM_AZIMUTHS * sizeof(float));
  memset(_beamDuration, 0, NUM_AZIMUTHS * sizeof(double));
}


/*********************************************************************
 * _processFile() - Process the given input file
 */

bool NexradA1ToRefract::_processFile(const string &input_file_path)
{
  static const string method_name = "NexradA1ToRefract::_processFile()";
  
  if (_params->debug)
    cerr << "*** Processing file: " << input_file_path << endl;

  // Open the netCDF file

  Nc3File input_file(input_file_path.c_str());

  // Check to see if the file is valid

  if (!input_file.is_valid())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Input file isn't valid: " << input_file_path << endl;

    input_file.close();
    return false;
  }

  // Make sure the file has data

  Nc3Dim *time_dim;
  if ((time_dim = input_file.get_dim(TIME_DIM_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading Time dimension from input file: "
	 << input_file_path << endl;

    input_file.close();
    return false;
  }

  Nc3Dim *gates_dim;
  if ((gates_dim = input_file.get_dim(GATES_DIM_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading Gates dimension from input file: "
	 << input_file_path << endl;

    input_file.close();
    return false;
  }

  if (time_dim->size() <= 0 || gates_dim->size() <= 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Input file has no data: " << input_file_path << endl;
    cerr << "Time dim = " << time_dim->size() << endl;
    cerr << "Gates dim = " << gates_dim->size() << endl;

    input_file.close();
    return false;
  }

  // Check the number of gates.  If it's different from our current
  // number, output the current file and start over again.

  int num_gates = gates_dim->size();

  if (_numGates == 0)
    _initializeArrays(num_gates);
  else if (num_gates != _numGates)
  {
    if (_params->debug)
      cerr << "Num gates changed from " << _numGates
	   << " to " << num_gates
	   << " -- outputting sweep file and starting over again" << endl;
    
    _writeSweepFile();
  }
  
  // Get pointers to all of the needed variables

  Nc3Values *i_values = _extractValues(I_VAR_NAME,
				      input_file, input_file_path);
  Nc3Values *q_values = _extractValues(Q_VAR_NAME,
				      input_file, input_file_path);
  Nc3Values *az_values = _extractValues(AZIMUTH_VAR_NAME,
				       input_file, input_file_path);
  Nc3Values *el_values = _extractValues(ELEVATION_VAR_NAME,
				       input_file, input_file_path);
  Nc3Values *prt_values = _extractValues(PRT_VAR_NAME,
					input_file, input_file_path);
  Nc3Values *time_values = _extractValues(UNIX_TIME_VAR_NAME,
					 input_file, input_file_path);
  Nc3Values *nano_values = _extractValues(NANO_SEC_VAR_NAME,
					 input_file, input_file_path);

  if (i_values == 0 || q_values == 0 || az_values == 0 ||
      el_values == 0 || prt_values == 0 || time_values == 0 ||
      nano_values == 0)
  {
    delete i_values;
    delete q_values;
    delete az_values;
    delete el_values;
    delete prt_values;
    delete time_values;
    delete nano_values;

    input_file.close();
    return false;
  }

  // Collect the pulses into beams and calculate the radar fields for
  // each gat and each beam

  time_t curr_pulse_time = 0, prev_pulse_time = 0;

  for (int time_step = 0; time_step < time_dim->size(); ++time_step)
  {
    // Only process the desired elevations

    float curr_elevation = el_values->as_float(time_step);
    if (curr_elevation < _params->elevation_limits.min ||
	curr_elevation > _params->elevation_limits.max)
      continue;
    
    // Determine the azimuth for this time_step

    double azimuth = az_values->as_float(time_step);

    int output_az = (int)(azimuth + 0.5);
    if (output_az < 0)
      output_az += 360;
    if (output_az >= 360)
      output_az -= 360;

    // Wait the specified time between sweeps

    time_t pulse_time = time_values->as_int(time_step);
    
    if (_sweepStartTime == 0 &&
	(pulse_time - _beamEndTime < _params->min_secs_between_sweeps))
      continue;
    
    // Calculate the PRF and Nyquist velocity for this pulse

    double prt;
    
    if (time_step == 0)
      prt = (double)prt_values->as_int(1);
    else
      prt = (double)prt_values->as_int(time_step);

    if (prt == 0.0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "PRT value is 0.0" << endl;
      cerr << "Skipping pulse" << endl;
      
      continue;
    }
    
    _prf = 1.0e6 / prt;
    _nyquistVel = (WAVELENGTH * _prf) / 4.0;

    // Check the PRF limits

    if (_prf < _params->prf_limits.min ||
	_prf > _params->prf_limits.max)
      continue;
    
    // Save the beam and volume times, if necessary.

    if (_beamStartTime == 0)
    {
      _beamStartTime = pulse_time;
      _beamStartNanoseconds = nano_values->as_int(time_step);
    }
    
    if (_sweepStartTime == 0)
    {
      _sweepStartTime = pulse_time;

      if (time_step == 0)
	_sweepPrt = prt_values->as_int(1);
      else
	_sweepPrt = prt_values->as_int(time_step);
    }
    
    // If there has been more than the specified number of seconds between
    // this pulse and the previous one, then assume we have passed through
    // a volume before getting to this pulse.  So, output the previous sweep
    // and start a new one.

    curr_pulse_time = time_values->as_int(time_step);

    if (prev_pulse_time != 0 && 
	curr_pulse_time - prev_pulse_time >= _params->max_secs_between_pulses)
    {
      if (_params->debug)
	cerr << "---> Outputting sweep because >"
	     << _params->max_secs_between_pulses
	     << " seconds between pulses" << endl;

      double beam_duration = (double)(_beamEndTime - _beamStartTime) +
	((double)(_beamEndNanoseconds - _beamStartNanoseconds) /
	 1000000000.0);

      _processBeam(_iValuesBeam, _qValuesBeam, _numGates,
		   _currOutputAzimuth, _elevationTotal/(float)_numSamples,
		   _prf, beam_duration);

      _iValuesBeam.erase(_iValuesBeam.begin(), _iValuesBeam.end());
      _qValuesBeam.erase(_qValuesBeam.begin(), _qValuesBeam.end());

      _elevationTotal = 0.0;
      _numSamples = 0;

      _beamStartTime = time_values->as_int(time_step);
      _beamStartNanoseconds = nano_values->as_int(time_step);

      _writeSweepFile();
    }
      
//    // If the PRT value has changed, start a new sweep.
//
//    int curr_prt;
//    if (time_step == 0)
//      curr_prt = prt_values->as_int(1);
//    else
//      curr_prt = prt_values->as_int(time_step);
//    
//    if (_sweepPrt != curr_prt)
//    {
//      if (_params->debug)
//	cerr << "---> Outputting sweep because PRT value changed" << endl;
//
//      double beam_duration = (double)(_beamEndTime - _beamStartTime) +
//	((double)(_beamEndNanoseconds - _beamStartNanoseconds) /
//	 1000000000.0);
//
//      _processBeam(_iValuesBeam, _qValuesBeam, _numGates,
//		   _currOutputAzimuth, _elevationTotal/(float)_numSamples,
//		   _prf, beam_duration);
//
//      _iValuesBeam.erase(_iValuesBeam.begin(), _iValuesBeam.end());
//      _qValuesBeam.erase(_qValuesBeam.begin(), _qValuesBeam.end());
//
//      _elevationTotal = 0.0;
//      _numSamples = 0;
//
//      _beamStartTime = time_values->as_int(time_step);
//      _beamStartNanoseconds = nano_values->as_int(time_step);
//
//      _writeSweepFile();
//    }
//      
    // See if we need to output the current sweep or save the beginning
    // azimuth in the sweep because we are starting processing.

    if (_sweepStartAzimuth < 0)
    {
      _sweepStartAzimuth = output_az + _params->num_beams_to_skip;
      if (_sweepStartAzimuth > 360)
	_sweepStartAzimuth -= 360;

      _waitingForSweepStart = true;
    }
    else if (_waitingForSweepStart)
    {
      if (output_az == _sweepStartAzimuth)
	_waitingForSweepStart = false;
      else
	continue;
    }
    else if (output_az == _sweepStartAzimuth &&
	     output_az != _currOutputAzimuth)
    {
      if (_params->debug)
	cerr << "---> Outputting sweep because reached beginning azimuth"
	     << endl;
      
      double beam_duration = (double)(_beamEndTime - _beamStartTime) +
	((double)(_beamEndNanoseconds - _beamStartNanoseconds) /
	 1000000000.0);

      _processBeam(_iValuesBeam, _qValuesBeam, _numGates,
		   _currOutputAzimuth, _elevationTotal/(float)_numSamples,
		   _prf, beam_duration);

      _iValuesBeam.erase(_iValuesBeam.begin(), _iValuesBeam.end());
      _qValuesBeam.erase(_qValuesBeam.begin(), _qValuesBeam.end());

      _elevationTotal = 0.0;
      _numSamples = 0;

      _beamStartTime = time_values->as_int(time_step);
      _beamStartNanoseconds = nano_values->as_int(time_step);

      _writeSweepFile();
    }
    
    // If we've moved on to the next azimuth, process the previous one
    // and clear out the old data

    if (output_az != _currOutputAzimuth &&
	_currOutputAzimuth >= 0)
    {
      double beam_duration = (double)(_beamEndTime - _beamStartTime) +
	((double)(_beamEndNanoseconds - _beamStartNanoseconds) /
	 1000000000.0);

      _processBeam(_iValuesBeam, _qValuesBeam, _numGates,
		   _currOutputAzimuth, _elevationTotal/(float)_numSamples,
		   _prf, beam_duration);

      _iValuesBeam.erase(_iValuesBeam.begin(), _iValuesBeam.end());
      _qValuesBeam.erase(_qValuesBeam.begin(), _qValuesBeam.end());

      _elevationTotal = 0.0;
      _numSamples = 0;

      _beamStartTime = time_values->as_int(time_step);
      _beamStartNanoseconds = nano_values->as_int(time_step);
    }

    // Now save the I and Q values for this pulse so they can be
    // accumulated and the moments calculated when we finish this beam

    for (int gate = 0; gate < _numGates; ++gate)
    {
      int nc_index = (time_step * _numGates) + gate;

      _iValuesBeam.push_back(i_values->as_float(nc_index));
      _qValuesBeam.push_back(q_values->as_float(nc_index));
    }

    // Save the information for this beam

    _beamEndTime = time_values->as_int(time_step);
    _beamEndNanoseconds = nano_values->as_int(time_step);

    _elevationTotal += curr_elevation;
    ++_numSamples;

    _currOutputAzimuth = output_az;

//    if (_params->debug)
//      cerr << "_currOutputAzimuth = " << _currOutputAzimuth << endl;
    
    _sweepEndTime = time_values->as_int(time_step);
    
  } /* endfor - time_step */

  delete i_values;
  delete q_values;
  delete az_values;
  delete el_values;
  delete prt_values;
  delete time_values;
  delete nano_values;

  input_file.close();

  return true;
}


/*********************************************************************
 * _processBeam() - Process the given data, all of which fall within
 *                  the same beam.
 */

void NexradA1ToRefract::_processBeam(const vector< float > &i_values_beam,
			      const vector< float > &q_values_beam,
			      const size_t num_gates,
			      const int azimuth, const float elevation,
			      const double prf,
			      const double beam_duration)
{
  static const string method_name = "NexradA1ToRefract::_processBeam()";

  // Check for errors

  if (i_values_beam.size() == 0)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "No pulses to process" << endl;

    return;
  }

  if (i_values_beam.size() != q_values_beam.size())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "I and Q vectors not of same size" << endl;

    return;
  }

  if ((i_values_beam.size() % num_gates) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Value vectors not an even multiple of num gates" << endl;
    cerr << "vector size: " << i_values_beam.size() << endl;
    cerr << "num gates: " << num_gates << endl;

    return;
  }

  // Calculate the fields

  size_t num_pulses = i_values_beam.size() / num_gates;

  for (size_t pulse = 0; pulse < num_pulses; ++pulse)
  {
    for (size_t gate = 0; gate < num_gates; ++gate)
    {
      int iq_index = (pulse * num_gates) + gate;
      int field_index = (azimuth * num_gates) + gate;

//      if (_params->debug &&
//	  (azimuth == 327))
//	cerr << "   " << azimuth << "   " << pulse << "   "
//	     << field_index << "   " << iq_index << "   "
//	     << _power[field_index] << "   "
//	     << i_values_beam[iq_index] << "   "
//	     << q_values_beam[iq_index] << endl;
      
      if (_power[field_index] < 0.0 ||
	  _power[field_index] == SweepFile::MISSING_DATA_VALUE)
	_power[field_index] =
	  (i_values_beam[iq_index] * i_values_beam[iq_index]) +
	  (q_values_beam[iq_index] * q_values_beam[iq_index]);
      else
	_power[field_index] +=
	  (i_values_beam[iq_index] * i_values_beam[iq_index]) +
	  (q_values_beam[iq_index] * q_values_beam[iq_index]);

      if (pulse > 0)
      {
	int prev_iq_index = ((pulse-1) * num_gates) + gate;

	if (_velA[field_index] == SweepFile::MISSING_DATA_VALUE)
	  _velA[field_index] =
	    (i_values_beam[iq_index] * i_values_beam[prev_iq_index]) +
	    (q_values_beam[iq_index] * q_values_beam[prev_iq_index]);
	else
	  _velA[field_index] +=
	    (i_values_beam[iq_index] * i_values_beam[prev_iq_index]) +
	    (q_values_beam[iq_index] * q_values_beam[prev_iq_index]);

	if (_velB[field_index] == SweepFile::MISSING_DATA_VALUE)
	  _velB[field_index] =
	    (i_values_beam[prev_iq_index] * q_values_beam[iq_index]) -
	    (i_values_beam[iq_index] * q_values_beam[prev_iq_index]);
	else
	  _velB[field_index] +=
	    (i_values_beam[prev_iq_index] * q_values_beam[iq_index]) -
	    (i_values_beam[iq_index] * q_values_beam[prev_iq_index]);
      }

      if (_avgI[field_index] == SweepFile::MISSING_DATA_VALUE)
	_avgI[field_index] = i_values_beam[iq_index];
      else
	_avgI[field_index] += i_values_beam[iq_index];

      if (_avgQ[field_index] == SweepFile::MISSING_DATA_VALUE)
	_avgQ[field_index] = q_values_beam[iq_index];
      else
	_avgQ[field_index] += q_values_beam[iq_index];

    } /* endfor - gate */
  } /* endfor - pulse */

  for (size_t gate = 0; gate < num_gates; ++gate)
  {
    double range = START_RANGE + (gate * GATE_SPACING);
    int field_index = (azimuth * num_gates) + gate;

    if (_power[field_index] != SweepFile::MISSING_DATA_VALUE)
      _power[field_index] /= (double)num_pulses;

    if (_avgI[field_index] != SweepFile::MISSING_DATA_VALUE)
      _avgI[field_index] /= (double)num_pulses;
    if (_avgQ[field_index] != SweepFile::MISSING_DATA_VALUE)
    _avgQ[field_index] /= (double)num_pulses;

    if (_avgI[field_index] == SweepFile::MISSING_DATA_VALUE ||
	_avgQ[field_index] == SweepFile::MISSING_DATA_VALUE ||
	(_avgI[field_index] == 0.0 && _avgQ[field_index] == 0.0))
    {
      _niq[field_index] = SweepFile::MISSING_DATA_VALUE;
      _aiq[field_index] = SweepFile::MISSING_DATA_VALUE;
    }
    else
    {
      _niq[field_index] = 10.0 *
	log10(sqrt((_avgI[field_index] * _avgI[field_index]) +
		   (_avgQ[field_index] * _avgQ[field_index])));
      _aiq[field_index] =
	180.0 / M_PI * atan2(_avgQ[field_index], _avgI[field_index]);
    }

    if (_power[field_index] == SweepFile::MISSING_DATA_VALUE ||
	_power[field_index] <= 0.0)
      _z[field_index] = SweepFile::MISSING_DATA_VALUE;
    else
      _z[field_index] =
	(log10(_power[field_index] * range * range) * 10) + RADAR_CONSTANT;

    if (_velA[field_index] == SweepFile::MISSING_DATA_VALUE ||
	_velB[field_index] == SweepFile::MISSING_DATA_VALUE ||
	(_velA[field_index] == 0.0 && _velB[field_index] == 0.0))
      _velocity[field_index] = SweepFile::MISSING_DATA_VALUE;
    else
      _velocity[field_index] =
	-(WAVELENGTH / (4.0 * M_PI)) * prf *
	atan2(_velB[field_index], _velA[field_index]);

    if (_power[field_index] == SweepFile::MISSING_DATA_VALUE ||
	_power[field_index] == 0.0 || num_pulses <= 1)
      _ncp[field_index] = SweepFile::MISSING_DATA_VALUE;
    else
      _ncp[field_index] =
	sqrt((_velA[field_index] * _velA[field_index]) +
	     (_velB[field_index] * _velB[field_index])) /
	_power[field_index] / (double)(num_pulses-1);
    if (_ncp[field_index] != SweepFile::MISSING_DATA_VALUE &&
	_ncp[field_index] > 0.999)
      _ncp[field_index] = 0.999;

    if (_power[field_index] == SweepFile::MISSING_DATA_VALUE ||
	_power[field_index] <= 0.)
      _power[field_index] = SweepFile::MISSING_DATA_VALUE;
    else
      _power[field_index] = 10. * log10(_power[field_index]) + POWER_CONSTANT;

    if (_ncp[field_index] == SweepFile::MISSING_DATA_VALUE ||
	_ncp[field_index] == 0.0)
      _sw[field_index] = SweepFile::MISSING_DATA_VALUE;
    else
      _sw[field_index] =
	(sqrt(log(1.0 / _ncp[field_index])) * WAVELENGTH * prf) /
	(2.0 * sqrt(2.0) * M_PI);

  } /* endfor - gate */

  // Save the beam information

  _elevation[azimuth] = elevation;
  _beamDuration[azimuth] = beam_duration;
}


/*********************************************************************
 * _writeSweepFile() - Write the saved data to a sweep file.
 */

bool NexradA1ToRefract::_writeSweepFile()
{
  if (_params->debug)
  {
    cerr << "About to write file..." << endl;
    
    for (int i = 0; i < _numGates * NUM_AZIMUTHS; ++i)
    {
      if (_power[i] == SweepFile::MISSING_DATA_VALUE)
	cerr << "   _power[" << i << "] = " << _power[i] << endl;
    }
  } /* endif - _params->debug */
  
  SweepFile sweep_file(_args->getOutputDir(),
		       RADAR_LAT, RADAR_LON, RADAR_ALT,
		       _sweepStartTime, _elevation[0],
		       0, 0,
		       START_RANGE, GATE_SPACING,
		       NUM_AZIMUTHS, _numGates,
		       SAMPLES_PER_BEAM,
		       _nyquistVel,
		       RADAR_CONSTANT,
		       WAVELENGTH,
		       _prf, _params->debug);

  if (!sweep_file.write(_beamDuration,
			_azimuth,
			_elevation,
			_power,
			_velocity,
			_velA, _velB,
			_avgI, _avgQ,
			_niq, _aiq,
			_z,
			_ncp,
			_sw))
    return false;

  // Clear out all of the saved information for the next sweep

  _prevSweepEndTime = _sweepEndTime;
  
  _initializeArrays(_numGates);
  _sweepStartTime = 0;
  _sweepEndTime = 0;
  _sweepPrt = 0;
  _sweepStartAzimuth = -1;
  _currOutputAzimuth = -1;
  _beamStartTime = 0;
  _beamStartNanoseconds = 0;
  _beamEndTime = 0;
  _beamEndNanoseconds = 0;
  _elevationTotal = 0.0;
  _numSamples = 0;

  return true;
}
