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
//   $Date: 2018/01/26 20:36:36 $
//   $Id: Chill2netCDFSweep.cc,v 1.11 2018/01/26 20:36:36 jcraig Exp $
//   $Revision: 1.11 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Chill2netCDFSweep: Chill2netCDFSweep program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cassert>
#include <iostream>
#include <csignal>
#include <cmath>
#include <string>

#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsInputDirTrigger.hh>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>

#include "Chill2netCDFSweep.hh"
#include "SweepFile.hh"

using namespace std;

// Global variables

// Calculate the constant used to convert the angle values in the 
// input files to degrees.  This is a conversion of a 16 bit angle
// to degrees.

const int Chill2netCDFSweep::INIT_NUM_GATES = 500;
const int Chill2netCDFSweep::INIT_NUM_BEAMS = 360;

const double Chill2netCDFSweep::ANGLE_TO_DEGREE = 360.0 / 65536.0;

const int Chill2netCDFSweep::PARAM_AVGI_MASK = 1;
const int Chill2netCDFSweep::PARAM_AVGQ_MASK = 2;
const int Chill2netCDFSweep::PARAM_DBZ_MASK = 4;
const int Chill2netCDFSweep::PARAM_VEL_MASK = 8;
const int Chill2netCDFSweep::PARAM_NCP_MASK = 16;
const int Chill2netCDFSweep::PARAM_SW_MASK = 32;

const int Chill2netCDFSweep::CHANNEL_H_MASK = 1;
const int Chill2netCDFSweep::CHANNEL_V_MASK = 2;

const float Chill2netCDFSweep::RADAR_C = 287.69;
const float Chill2netCDFSweep::ANT_GAIN_H = 42.2;
const float Chill2netCDFSweep::ANT_GAIN_V = 42.2;
const float Chill2netCDFSweep::REC_GAIN_H = 116.3;
const float Chill2netCDFSweep::REC_GAIN_V = 116.3;
const float Chill2netCDFSweep::TX_POW_H = 88.5;
const float Chill2netCDFSweep::TX_POW_V = 87.0;


Chill2netCDFSweep *Chill2netCDFSweep::_instance =
     (Chill2netCDFSweep *)NULL;


/*********************************************************************
 * Constructor
 */

Chill2netCDFSweep::Chill2netCDFSweep(int argc, char **argv) :
  _sweepStartTime(0),
  _startElevation(0),
  _sweepNumber(0),
  _startRangeMm(0),
  _gateSpacingMm(0),
  _numGatesInSweep(0),
  _samplesPerBeam(0),
  _nyquistVelocity(0.0),
  _prt(0),
  _azimuth(0),
  _elevation(0),
  _timeOffset(0),
  _hAvgI(0),
  _hAvgQ(0),
  _hDbz(0),
  _hVel(0),
  _hNcp(0),
  _hSw(0),
  _hNiq(0),
  _hAiq(0),
  _hDm(0),
  _vAvgI(0),
  _vAvgQ(0),
  _vDbz(0),
  _vVel(0),
  _vNcp(0),
  _vSw(0),
  _vNiq(0),
  _vAiq(0),
  _vDm(0),
  _beamIndex(0),
  _numGatesAlloc(0),
  _numBeamsAlloc(0)
{
  static const string method_name = "Chill2netCDFSweep::Chill2netCDFSweep()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (Chill2netCDFSweep *)NULL);
  
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

  _args = new Args(argc, argv, (char *) "Chill2netCDFSweep");

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

Chill2netCDFSweep::~Chill2netCDFSweep()
{
  // Free contained objects

  delete _params;
  delete _args;
  delete _dataTrigger;
  
  // Free the data arrays

  delete [] _azimuth;
  delete [] _elevation;
  delete [] _timeOffset;

  delete [] _hAvgI;
  delete [] _hAvgQ;
  delete [] _hDbz;
  delete [] _hVel;
  delete [] _hNcp;
  delete [] _hSw;
  delete [] _hNiq;
  delete [] _hAiq;
  delete [] _hDm;
  
  delete [] _vAvgI;
  delete [] _vAvgQ;
  delete [] _vDbz;
  delete [] _vVel;
  delete [] _vNcp;
  delete [] _vSw;
  delete [] _vNiq;
  delete [] _vAiq;
  delete [] _vDm;
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

Chill2netCDFSweep *Chill2netCDFSweep::Inst(int argc, char **argv)
{
  if (_instance == (Chill2netCDFSweep *)NULL)
    new Chill2netCDFSweep(argc, argv);
  
  return(_instance);
}

Chill2netCDFSweep *Chill2netCDFSweep::Inst()
{
  assert(_instance != (Chill2netCDFSweep *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool Chill2netCDFSweep::init()
{
  static const string method_name = "Chill2netCDFSweep::init()";
  
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
  
  // Allocate a fair size for the data arrays initially.  The array
  // sizes will be increased as needed.

  _allocateDataArrays(INIT_NUM_GATES, INIT_NUM_BEAMS);
  
  // initialize process registration

  if (_params->trigger_mode != Params::FILE_LIST)
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void Chill2netCDFSweep::run()
{
  static const string method_name = "Chill2netCDFSweep::run()";
  
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
    ++_sweepNumber;
    
  } /* endwhile - !_dataTrigger->endOfData() */
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _allocateDataArrays() - Allocate space for the data arrays.
 */

void Chill2netCDFSweep::_allocateDataArrays(const int num_gates,
					    const int num_beams)
{
  // Make sure we aren't reducing the number of gates or beams.  Otherwise,
  // we'll have a segmentation fault inside the realloc method.

  int num_gates_needed = num_gates;
  int num_beams_needed = num_beams;
  
  if (num_gates_needed < _numGatesAlloc)
    num_gates_needed = _numGatesAlloc;
  if (num_beams_needed < _numBeamsAlloc)
    num_beams_needed = _numBeamsAlloc;
  
  // Do the reallocations, if needed.

  if (_numBeamsAlloc < num_beams_needed)
  {
    _reallocInfoArray(_azimuth, num_beams_needed);
    _reallocInfoArray(_elevation, num_beams_needed);
    _reallocInfoArray(_timeOffset, num_beams_needed);
  }
  
  if (_numGatesAlloc < num_gates_needed ||
      _numBeamsAlloc < num_beams_needed)
  {
    _reallocDataArray(_hAvgI, num_gates_needed, num_beams_needed);
    _reallocDataArray(_hAvgQ, num_gates_needed, num_beams_needed);
    _reallocDataArray(_hDbz, num_gates_needed, num_beams_needed);
    _reallocDataArray(_hVel, num_gates_needed, num_beams_needed);
    _reallocDataArray(_hNcp, num_gates_needed, num_beams_needed);
    _reallocDataArray(_hSw, num_gates_needed, num_beams_needed);
    _reallocDataArray(_hNiq, num_gates_needed, num_beams_needed);
    _reallocDataArray(_hAiq, num_gates_needed, num_beams_needed);
    _reallocDataArray(_hDm, num_gates_needed, num_beams_needed);
    
    _reallocDataArray(_vAvgI, num_gates_needed, num_beams_needed);
    _reallocDataArray(_vAvgQ, num_gates_needed, num_beams_needed);
    _reallocDataArray(_vDbz, num_gates_needed, num_beams_needed);
    _reallocDataArray(_vVel, num_gates_needed, num_beams_needed);
    _reallocDataArray(_vNcp, num_gates_needed, num_beams_needed);
    _reallocDataArray(_vSw, num_gates_needed, num_beams_needed);
    _reallocDataArray(_vNiq, num_gates_needed, num_beams_needed);
    _reallocDataArray(_vAiq, num_gates_needed, num_beams_needed);
    _reallocDataArray(_vDm, num_gates_needed, num_beams_needed);
  }
  
  _numGatesAlloc = num_gates_needed;
  _numBeamsAlloc = num_beams_needed;
}


/*********************************************************************
 * _initializeDataArrays() - Initialize all of the gates in the data
 *                           arrays to missing data values.  This is
 *                           done at the beginning of processing each
 *                           file to clear out the old data.
 */

void Chill2netCDFSweep::_initializeDataArrays()
{
  _initInfoArray(_azimuth);
  _initInfoArray(_elevation);
  _initInfoArray(_timeOffset);

  _initDataArray(_hAvgI);
  _initDataArray(_hAvgQ);
  _initDataArray(_hDbz);
  _initDataArray(_hVel);
  _initDataArray(_hNcp);
  _initDataArray(_hSw);
  _initDataArray(_hNiq);
  _initDataArray(_hAiq);
  _initDataArray(_hDm);
  
  _initDataArray(_vAvgI);
  _initDataArray(_vAvgQ);
  _initDataArray(_vDbz);
  _initDataArray(_vVel);
  _initDataArray(_vNcp);
  _initDataArray(_vSw);
  _initDataArray(_vNiq);
  _initDataArray(_vAiq);
  _initDataArray(_vDm);
}


/*********************************************************************
 * _processBeam() - Process the next beam in the given input file.
 *
 * Returns true on success, false on failure.
 */

bool Chill2netCDFSweep::_processBeam(FILE *input_file,
				     const string &input_file_path,
				     const bool first_beam)
{
  static const string method_name = "Chill2netCDFSweep::_processFile()";
  
  // Read the header
    
  header_t header;
  int bytes_read;
  
  if ((bytes_read = fread(&header, 1, sizeof(header), input_file))
      != sizeof(header))
  {
    if (bytes_read == 0)
    {
      if (_params->debug)
	cerr << "Read 0 bytes from file, assuming end of file..." << endl;
      
      return false;
    }
    
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading header from input file: " << input_file_path << endl;
    cerr << "Read " << bytes_read << " bytes from file" << endl;
    cerr << "Expected " << sizeof(header) << " bytes" << endl;
    
    fclose(input_file);
    return false;
  }
  
  // Calculate the constants needed in the DM calculations

  float zhconst = (float)header.zhconst / 1000.0;
  float zvconst = (float)header.zvconst / 1000.0;
  float gate_spacing_km = (float)header.range_inc / 1000000.0;
  
  if (_params->debug)
  {
    cerr << "Header read:" << endl;
    cerr << "   id = " << header.id << endl;
    cerr << "   headlen = " << header.headlen << endl;
    cerr << "   rtime = " << header.rtime
	 << " (" << DateTime::str(header.rtime) << ")" << endl;
    cerr << "   xtime = " << header.xtime << endl;
    cerr << "   channels_recorded = " << header.channels_recorded << endl;
    cerr << "   param_recorded = " << header.param_recorded << endl;
    cerr << "   az_start = " << (header.az_start * ANGLE_TO_DEGREE) << endl;
    cerr << "   az_end = " << (header.az_end * ANGLE_TO_DEGREE) << endl;
    cerr << "   el_start = " << (header.el_start * ANGLE_TO_DEGREE) << endl;
    cerr << "   el_end = " << (header.el_end * ANGLE_TO_DEGREE) << endl;
    cerr << "   prt = " << header.prt << endl;
    cerr << "   nyquist_vel = " << header.nyquist_vel << endl;
    cerr << "   ngates = " << header.ngates << endl;
    cerr << "   range_start = " << header.range_start << endl;
    cerr << "   range_inc = " << gate_spacing_km << " km" << endl;
    cerr << "   pulses = " << header.pulses << endl;
    cerr << "   zhconst = " << zhconst << " (" << header.zhconst << ")" << endl;
    cerr << "   zvconst = " << zvconst << " (" << header.zvconst << ")" << endl;
    cerr << "   rsq100 = " << ((double)header.rsq100/1000.0) << endl;
  }
  
  // Save the information needed from the first beam

  if (first_beam)
  {
    _sweepStartTime = header.rtime;
    _startElevation = header.el_start * ANGLE_TO_DEGREE;
    _startRangeMm = header.range_start;
    _gateSpacingMm = header.range_inc;
    _numGatesInSweep = header.ngates;
    _samplesPerBeam = header.pulses;
    _nyquistVelocity = header.nyquist_vel;
    _prt = header.prt;
  }
  else
  {
    if (_startRangeMm != header.range_start)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Start range changed mid-sweep" << endl;
      cerr << "Originally had start range of " << _startRangeMm
	   << " mm" << endl;
      cerr << "Changed to " << header.range_start << " mm" << endl;
      cerr << "Skipping sweep..." << endl;
      
      return false;
    }
    
    if (_gateSpacingMm != header.range_inc)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Gate spacing changed mid-sweep" << endl;
      cerr << "Originally had gate spacing of " << _gateSpacingMm
	   << " mm" << endl;
      cerr << "Changed to " << header.range_inc << " mm" << endl;
      cerr << "Skipping sweep..." << endl;
      
      return false;
    }
    
    if (_numGatesInSweep != header.ngates)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Number of gates per beam changed mid-sweep" << endl;
      cerr << "Originally had " << _numGatesInSweep << " gates" << endl;
      cerr << "Changed to " << header.ngates << " gates" << endl;
      cerr << "Skipping sweep..." << endl;
      
      return false;
    }
    
    if (_nyquistVelocity != header.nyquist_vel)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Nyquist velocity changed mid-sweep" << endl;
      cerr << "Originally had nyquist velocity of "
	   << _nyquistVelocity << endl;
      cerr << "Changed to " << header.nyquist_vel << endl;
      cerr << "Skipping sweep..." << endl;
      
      return false;
    }
    
    if (_prt != header.prt)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "PRT value changed mid-sweep" << endl;
      cerr << "Originally had PRT value of " << _prt << endl;
      cerr << "Changed to " << header.prt << endl;
      cerr << "Skipping sweep..." << endl;
      
      return false;
    }
    
  }
  
  // Calculate the data dimensions

  int num_fields = _getNumBitsSet(header.param_recorded);
  int num_channels = _getNumBitsSet(header.channels_recorded);

  if (_params->debug)
  {
    cerr << endl;
    cerr << "   num_fields = " << num_fields << endl;
    cerr << "   num_channels = " << num_channels << endl;
    cerr << "   num_gates = " << header.ngates << endl;
  }
    
  size_t data_len = num_fields * num_channels * header.ngates;
  
  // Read the data

  short *data = new short[data_len];
  
  if ((fread(data, sizeof(short), data_len, input_file)) != data_len)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading data from file: " << input_file_path << endl;
    
    fclose(input_file);
    return false;
  }

  // Pull out each of the data fields

  _allocateDataArrays(header.ngates, _beamIndex + 1);
  
  short *data_ptr = data;
  
  for (int gate = 0; gate < header.ngates; ++gate)
  {
    int data_index = (_beamIndex * header.ngates) + gate;
    float range_km = (float)gate * gate_spacing_km;
    
    // Process each of the channels

    if (header.channels_recorded & CHANNEL_H_MASK)
    {
      // Process each of the fields in the data

      if (header.param_recorded & PARAM_AVGI_MASK)
	_hAvgI[data_index] = _calcAvgI(*data_ptr++);

      if (header.param_recorded & PARAM_AVGQ_MASK)
	_hAvgQ[data_index] = _calcAvgQ(*data_ptr++);
      
      if (header.param_recorded & PARAM_DBZ_MASK)
	_hDbz[data_index] = _calcDbz(*data_ptr++);
      
      if (header.param_recorded & PARAM_NCP_MASK)
	_hNcp[data_index] = _calcNcp(*data_ptr++);
      
      if (header.param_recorded & PARAM_VEL_MASK)
	_hVel[data_index] = _calcVel(*data_ptr++);
      
      if (header.param_recorded & PARAM_SW_MASK)
	_hSw[data_index] = _calcSw(*data_ptr++);

      if (header.param_recorded & PARAM_AVGI_MASK &&
	  header.param_recorded & PARAM_AVGQ_MASK)
      {
	_hNiq[data_index] = _calcNiq(_hAvgI[data_index],
				     _hAvgQ[data_index]);
	_hAiq[data_index] = _calcAiq(_hAvgI[data_index],
				     _hAvgQ[data_index]);
      }
      
      if (header.param_recorded & PARAM_DBZ_MASK)
      {
	_hDm[data_index] = _calcDm(_hDbz[data_index], zhconst,
				   REC_GAIN_H, range_km);
      }

    } /* endif - header.channels_recorded & CHANNEL_H_MASK */
    
    if (header.channels_recorded & CHANNEL_V_MASK)
    {
      // Process each of the fields in the data

      if (header.param_recorded & PARAM_AVGI_MASK)
	_vAvgI[data_index] = _calcAvgI(*data_ptr++);

      if (header.param_recorded & PARAM_AVGQ_MASK)
	_vAvgQ[data_index] = _calcAvgQ(*data_ptr++);
      
      if (header.param_recorded & PARAM_DBZ_MASK)
	_vDbz[data_index] = _calcDbz(*data_ptr++);
      
      if (header.param_recorded & PARAM_NCP_MASK)
	_vNcp[data_index] = _calcNcp(*data_ptr++);
      
      if (header.param_recorded & PARAM_VEL_MASK)
	_vVel[data_index] = _calcVel(*data_ptr++);
      
      if (header.param_recorded & PARAM_SW_MASK)
	_vSw[data_index] = _calcSw(*data_ptr++);

      if (header.param_recorded & PARAM_AVGI_MASK &&
	  header.param_recorded & PARAM_AVGQ_MASK)
      {
	_vNiq[data_index] = _calcNiq(_vAvgI[data_index],
				     _vAvgQ[data_index]);
	_vAiq[data_index] = _calcAiq(_vAvgI[data_index],
				     _vAvgQ[data_index]);
      }
      
      if (header.param_recorded & PARAM_DBZ_MASK)
      {
	_vDm[data_index] = _calcDm(_vDbz[data_index], zvconst,
				   REC_GAIN_V, range_km);
      }

    } /* endif - header.channels_recorded & CHANNEL_V_MASK */
    
  } /* endfor - gate */
  
  delete [] data;
  
  // Calculate the azimuth for this beam

  double az_start = header.az_start * ANGLE_TO_DEGREE;
  double az_end = header.az_end * ANGLE_TO_DEGREE;
  
  if (az_end < az_start)
    az_end += 360.0;
  
  _azimuth[_beamIndex] = (az_start + az_end) / 2.0;
  if (_azimuth[_beamIndex] >= 360.0)
    _azimuth[_beamIndex] -= 360.0;
  
  // Calculate the elevation for this beam

  _elevation[_beamIndex] = ((header.el_start * ANGLE_TO_DEGREE) +
     (header.el_end * ANGLE_TO_DEGREE)) / 2.0;
  
  // Calculate the time offset for the beam

  _timeOffset[_beamIndex] = (double)(header.rtime - _sweepStartTime) +
    ((double)header.xtime / 40000000.0);

  if (_params->debug)
  {
    cerr << "   _beamIndex = " << _beamIndex << endl;
    cerr << "   calculated az = " << _azimuth[_beamIndex] << endl;
    cerr << "   calculated el = " << _elevation[_beamIndex] << endl;
    cerr << "   _sweepStartTime = " << DateTime::str(_sweepStartTime) << endl;
    cerr << "   rtime = " << DateTime::str(header.rtime) << endl;
    cerr << "   xtime = " << header.xtime << endl;
    cerr << "   calculated time_offset = " << _timeOffset[_beamIndex] << endl;
  }
  
  return true;
}


/*********************************************************************
 * _processFile() - Process the given input file
 */

bool Chill2netCDFSweep::_processFile(const string &input_file_path)
{
  static const string method_name = "Chill2netCDFSweep::_processFile()";
  
  if (_params->debug)
    cerr << "*** Processing file: " << input_file_path << endl;

  // Open the input file

  FILE *input_file;
  
  if ((input_file = fopen(input_file_path.c_str(), "r")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening input file: " << input_file_path << endl;
    
    return false;
  }
  
  // Process the beams in the file

  _initializeDataArrays();
  
  _beamIndex = 0;
  
  bool first_beam = true;
  
  while (_processBeam(input_file, input_file_path, first_beam))
  {
    first_beam = false;
    _beamIndex++;
  }
  
  // Close the input file

  fclose(input_file);
  
  // Write the output file

  SweepFile sweep_file(_params->output_dir,
		       _params->radar_info.radar_lat,
		       _params->radar_info.radar_lon,
		       _params->radar_info.radar_alt,
		       _sweepStartTime,
		       _startElevation,
		       0,
		       _sweepNumber,
		       (double)_startRangeMm / 1000000.0,
		       (double)_gateSpacingMm / 1000000.0,
		       _beamIndex,
		       _numGatesInSweep,
		       _samplesPerBeam,
		       _nyquistVelocity,
		       _params->radar_info.radar_constant,
		       _params->radar_info.wave_length,
		       1.0 / (double)_prt,
		       _params->debug);
  
  if (!sweep_file.write(_timeOffset,
			_azimuth, _elevation,
			_hAvgI, _hAvgQ,
			_hDbz, _hVel, _hNcp, _hSw,
			_hNiq, _hAiq, _hDm,
			_vAvgI, _vAvgQ,
			_vDbz, _vVel, _vNcp, _vSw,
			_vNiq, _vAiq, _vDm))
    return false;
  
  return true;
}
