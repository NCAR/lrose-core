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
//   $Date: 2016/03/06 23:53:42 $
//   $Id: EdgeVol2Socket.cc,v 1.13 2016/03/06 23:53:42 dixon Exp $
//   $Revision: 1.13 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * EdgeVol2Socket: EdgeVol2Socket program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2001
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include <cassert>
#include <dirent.h>
#include <signal.h>
#include <sys/types.h>

#include <toolsa/os_config.h>
#include <edge/load_data.h>
#include <edge/vol.h>
#include <toolsa/file_io.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/uusleep.h>

#include "EdgeVol2Socket.hh"
#include "Args.hh"
#include "EdgeBeamMsg.hh"
#include "EdgeCommTcpipSingle.hh"
#include "EdgeMsgSupport.hh"
#include "Params.hh"

using namespace std;


// Global variables

EdgeVol2Socket *EdgeVol2Socket::_instance =
     (EdgeVol2Socket *)NULL;


/*********************************************************************
 * Constructor
 */

EdgeVol2Socket::EdgeVol2Socket(int argc, char **argv) :
  _debugFlag(false),
  _azimuthCorrection(0.0)
{
  const string method_name = "EdgeVol2Socket::EdgeVol2Socket()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (EdgeVol2Socket *)NULL);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);
  
  // Display ucopright message.

  ucopyright(_progName);

  // Get the command line arguments.

  Args args(argc, argv, _progName);
  
  // Get TDRP parameters.

  Params params;
  char *params_path = (char *) "unknown";
  
  if (params.loadFromArgs(argc, argv,
			    args.override.list,
			    &params_path))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with TDRP parameters in file <" << params_path << ">" << endl;
    
    okay = false;
    
    return;
  }

  // Set the debug and print flags

  _debugFlag = params.debug;
  _printRadarInfo = params.print_radar_info;
  _azimuthCorrection = params.az_correction;
  
  // Create the list of files to process

  if (!_createFileList(params.input_info.directory, params.input_info.ext))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating file list from input directory: " <<
      params.input_info.directory << endl;
    
    okay = false;
    
    return;
  }
  
  // Create the object for communicating with the client

  switch (params.output_socket_type)
  {
  case Params::UDP :
    cerr << "ERROR: " << method_name << endl;
    cerr << "UDP communications not yet implemented" << endl;
    return;
    
  case Params::TCPIP_SINGLE :
    _communicator = new EdgeCommTcpipSingle(params.tcpip_single_port);
    break;
    
  case Params::TCPIP_MULTI :
    cerr << "ERROR: " << method_name << endl;
    cerr << "TCP/IP multiple socket communications not yet implemented" << endl;
    return;
  } /* endswitch - _params->output_socket_type */
  
  // Calculate the members controlling how often the different messages are
  // sent.

  _beamSleepMsecs = (int)((1.0 / (float)params.beams_per_sec * 1000.0) + 0.5);
  _beamsBeforeStatusMsg = params.beams_per_sec;
  
  _simulateRealtime = params.simulate_realtime;
}


/*********************************************************************
 * Destructor
 */

EdgeVol2Socket::~EdgeVol2Socket()
{
  // Free included strings

  STRfree(_progName);

  // Free object pointers

  delete _communicator;
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

EdgeVol2Socket *EdgeVol2Socket::Inst(int argc, char **argv)
{
  if (_instance == (EdgeVol2Socket *)NULL)
    new EdgeVol2Socket(argc, argv);
  
  return(_instance);
}

EdgeVol2Socket *EdgeVol2Socket::Inst()
{
  assert(_instance != (EdgeVol2Socket *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * run() - run the program.
 */

void EdgeVol2Socket::run()
{
  static const string method_name = "EdgeVol2Socket::run()";
  
  // Initialize the beam data buffers

  unsigned int num_gates_alloc = 0;
  unsigned char *uncorr_refl_data = 0;
  unsigned char *corr_refl_data = 0;
  unsigned char *vel_data = 0;
  unsigned char *sw_data = 0;
  
  // Initialize the socket and connect to the client

  _communicator->init();

  if (_debugFlag)
    cerr << "---> Waiting for client to connect..." << endl;
  
  if (!_communicator->openClient())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening client socket" << endl;
    
    return;
  }
  
  
  if (_debugFlag)
    cerr << "     Client successfully connected" << endl;
  
  // Process each of the volume files

  vector< string >::iterator file_iter;
  
  EdgeStatusMsg status_msg;
  EdgeBeamMsg corr_refl_msg(EdgeMsgSupport::CORRECTED_REFLECTIVITY_MOMENT);
  EdgeBeamMsg uncorr_refl_msg(EdgeMsgSupport::UNCORRECTED_REFLECTIVITY_MOMENT);
  EdgeBeamMsg vel_msg(EdgeMsgSupport::VELOCITY_MOMENT);
  EdgeBeamMsg sw_msg(EdgeMsgSupport::SPECTRUM_WIDTH_MOMENT);
  
  for (file_iter = _fileList.begin();
       file_iter != _fileList.end();
       ++file_iter)
  {
    if (_debugFlag)
      cerr << "*** Processing file: " << *file_iter << endl;
    
    // Make sure the file is uncompressed

    char file_path[MAX_PATH_LEN];
    
    STRcopy(file_path, file_iter->c_str(), MAX_PATH_LEN);
    ta_file_uncompress(file_path);
    
    // Load the volume information for this file

    struct vol_struct *vol_ptr = 0;
    
    load_data((char **)&vol_ptr, file_path, VOL_FILE);

    if (vol_ptr == 0)
    {
      cerr << "Error loading volume from path: " << file_path << endl;
      cerr << "Skipping volume...." << endl;

      continue;
    }

    // Display max range see prod.h for structure definition
    // and other fields available in prod structure

    if (_debugFlag)
      cerr << "Number of sweeps in volume: " << vol_ptr->num_sweeps << endl;

    // Load the status message information that is constant throughout
    // the volume.

    _loadStatusVolInfo(status_msg, *vol_ptr);
    
    // Save the data time from the volume structure and the current time
    // so we can set the time in the status structure whenever it is sent.

    time_t volume_time = vol_ptr->date;
    time_t volume_time_saved = time(0);
    
    // Initialize the beam counter used for triggering when to send status
    // messages.  Set this up so that a status message is sent at the beginning
    // of the volume.

    unsigned int num_beams_since_status = _beamsBeforeStatusMsg;
    
    // Process each sweep in turn, putting out EDGE messages as appropriate

    for (int sweep_num = 0; sweep_num < vol_ptr->num_sweeps; ++sweep_num)
    {
      if (_debugFlag)
      {
	cerr << "sweep: " << sweep_num << endl;
	cerr << "number of range bins in each ray: " <<
	  GATES(vol_ptr, sweep_num) << endl;
	cerr << "range bin size: " << GATE_SIZE(vol_ptr, sweep_num) <<
	  " meters" << endl;
	cerr << "number of rays: " << RAYS(vol_ptr, sweep_num) << endl;
      }
	
      // Load the status message information that is constant throughout
      // the sweep.

      _loadStatusSweepInfo(status_msg, vol_ptr->sweep[sweep_num],
			   vol_ptr->version);
    
      // Process each ray

      for (int ray_num = 0; ray_num < RAYS(vol_ptr, sweep_num); ++ray_num)
      {
	// Get the pointer to the data for this ray

	unsigned char *ray_ptr = RAY_PTR(vol_ptr, sweep_num, ray_num);
	
	// Calculate the elevation and azimuth for this ray

	double elevation = (START_EL_DEGS(ray_ptr) + END_EL_DEGS(ray_ptr)) / 2.0;
	double azimuth =
	  ((START_AZ_DEGS(ray_ptr) + END_AZ_DEGS(ray_ptr)) / 2.0)
	  + _azimuthCorrection;
	while (azimuth >= 360.0)
	  azimuth -= 360.0;
	while (azimuth < 0.0)
	  azimuth += 360.0;

	if (_debugFlag)
	{
	  cerr << "Ray " << ray_num <<
	    ": Azimuth " << START_AZ_DEGS(ray_ptr) <<
	    "-" << END_AZ_DEGS(ray_ptr) <<
	    ", az_correction = " << _azimuthCorrection <<
	    " Elevation " << START_EL_DEGS(ray_ptr) <<
	    "-" << END_EL_DEGS(ray_ptr) << endl;
	  cerr << "    az = " << azimuth << ", el = " << elevation << endl;
	}
	
	// First see if we should be sending a status message now

	if (num_beams_since_status >= _beamsBeforeStatusMsg)
	{
	  if (_debugFlag)
	    cerr << "---> Sending status msg" << endl;
	  
	  status_msg.setAzimuth(azimuth);
	  status_msg.setElevation(elevation);

	  if (_simulateRealtime)
	  {
	    status_msg.setDTime(time(0));
	  }
	  else
	  {
	    time_t current_time = time(0);
	    status_msg.setDTime(volume_time +
				(current_time - volume_time_saved));
	  }
	  
	  if (!_communicator->sendMsg(status_msg))
	    cerr << "***** Error sending status msg to client" << endl;
	  
	  num_beams_since_status = 0;
	}
	
	// Now send the beam data messages

	unsigned int num_gates = GATES(vol_ptr, sweep_num);
	
	if (num_gates_alloc < num_gates)
	{
	  delete uncorr_refl_data;
	  delete corr_refl_data;
	  delete vel_data;
	  delete sw_data;
	  
	  uncorr_refl_data = new unsigned char[num_gates];
	  corr_refl_data = new unsigned char[num_gates];
	  vel_data = new unsigned char[num_gates];
	  sw_data = new unsigned char[num_gates];

	  num_gates_alloc = num_gates;
	}
	
	for (unsigned int gate = 0; gate < num_gates; ++gate)
	{
	  corr_refl_data[gate]   = *(ray_ptr + 8 + (gate * 4));
	  vel_data[gate]         = *(ray_ptr + 8 + (gate * 4) + 1);
	  uncorr_refl_data[gate] = *(ray_ptr + 8 + (gate * 4) + 2);
	  sw_data[gate]          = *(ray_ptr + 8 + (gate * 4) + 3);

	} /* endfor - gate */
	       
	corr_refl_msg.setData(azimuth, elevation,
			      (void *)corr_refl_data, num_gates);
	uncorr_refl_msg.setData(azimuth, elevation,
				(void *)uncorr_refl_data, num_gates);
	vel_msg.setData(azimuth, elevation,
			(void *)vel_data, num_gates);
	sw_msg.setData(azimuth, elevation,
		       (void *)sw_data, num_gates);
	
	_communicator->sendMsg(corr_refl_msg);
	_communicator->sendMsg(uncorr_refl_msg);
	_communicator->sendMsg(vel_msg);
	_communicator->sendMsg(sw_msg);
	
	// Sleep before processing the next beam to simulate the amount of
	// time it takes the radar to process the next beam

	umsleep(_beamSleepMsecs);
	
	// Increment our beam counter

	num_beams_since_status++;
	
      } /* endfor - ray_num */
    } /* endfor - sweep_num */
    
    // Free memory

    free(vol_ptr);

  } /* endfor - file_iter */
  
  // Close the client before exiting

  _communicator->closeClient();

  // Free up memory

  delete uncorr_refl_data;
  delete corr_refl_data;
  delete vel_data;
  delete sw_data;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _createFileList() - Create the list of files to process from the given
 *                     input directory.
 *
 * Returns true if the file list was successfully created, false
 * otherwise.
 *
 * The global member _fileList is updated to include all of the appropriate
 * files from the given directory when this method returns.
 */

bool EdgeVol2Socket::_createFileList(const string &directory,
				     const string &ext)
{
  static const string method_name = "EdgeVol2Socket::_createFileList()";
  
  // Open the input directory

  DIR *dir_ptr;
  
  if ((dir_ptr = opendir(directory.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening input directory: " << directory << endl;
    
    return false;
  }
  
  // Look at each of the files in the directory and save the paths of the
  // volume files we should process.

  struct dirent *dir_entry_ptr;
  
  while ((dir_entry_ptr = readdir(dir_ptr)) != 0)
  {
    // Skip dot files

    if (dir_entry_ptr->d_name[0] == '.')
      continue;
    
    // Skip files whose name doesn't contain the file extension

    if (strstr(dir_entry_ptr->d_name, ext.c_str()) == 0)
      continue;
    
    // If we get here, we want to process this file, so add the path to
    // the file list.

    string file_path = directory + PATH_DELIM + dir_entry_ptr->d_name;
    
    _fileList.push_back(file_path);
    
  } /* endwhile - (dir_entry_ptr = readdir(dir_ptr)) != 0 */
  
  // Close the input directory

  closedir(dir_ptr);
  
  // Make sure our file list wasn't empty

  if (_fileList.size() <= 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "No files with extension (" << ext << ") in input directory (" <<
      directory << ")" << endl;
    
    return false;
  }

  // Sort the file list alphabetically since this should put the files
  // into chronological order.

  sort(_fileList.begin(), _fileList.end());
  
  return true;
}


/*********************************************************************
 * _loadStatusSweepInfo() - Load the status message information that is
 *                          constant throughout the sweep.
 */

void EdgeVol2Socket::_loadStatusSweepInfo(EdgeStatusMsg &status_msg,
					  const sweep_struct &sweep_info,
					  const int version_num)
{
  //
  // Here is where we are looking at the sweep_struct, so print out
  // some values that Cathy is interested in from this struct.
  //

  if (_printRadarInfo)
  {
    cout << "Noise Info" << endl;
    cout << "==========" << endl;
    
    for (int i = 0; i < NUM_PW; ++i)
      cout << "   noise_thresh[" << i << "] = " <<
	sweep_info.rad.noise_thresh[i] << endl;
    
    cout << "   noise_int = " << sweep_info.rad.noise_int << endl;
    cout << "   last_noise = " << sweep_info.rad.last_noise << endl;
  }
  
  //
  // Status message record 1 values
  //

//  status_msg.setVersionNum();  -- Set to constant value by object
  status_msg.setPrf1(sweep_info.rad.prf1);
  status_msg.setPrf2(sweep_info.rad.prf2);
//  status_msg.setRange();
  status_msg.setUiSamples(sweep_info.rad.samples);
//  status_msg.setTimeSeriesRange();
//  status_msg.setProcessingMode();
  
  //
  // Status message record 2 values
  //

  if (version_num < 104)
  {
    status_msg.setGw1(sweep_info.rad.gw1);
    status_msg.setGw2(sweep_info.rad.gw2);
  }
  else
  {
    status_msg.setGw1(sweep_info.rad.gw1/4.0);
    status_msg.setGw2(sweep_info.rad.gw2/4.0);
  }
  
  status_msg.setGwPartition((unsigned int)(sweep_info.rad.gw_partition * 1000.0));
  status_msg.setRangeAvg(sweep_info.rad.range_avg);
  status_msg.setGates(sweep_info.rad.gates);

  //
  // Status message record 3 values
  //

  status_msg.setMomentEnable(sweep_info.rad.moment_enable);
  status_msg.setSoftwareSim(sweep_info.rad.software_sim);
  
  //
  // Status message record 4 values
  //

  status_msg.setUiScanType(sweep_info.rad.scan_type);
  status_msg.setTargetAzimuth(EdgeMsgSupport::binaryToDeg(sweep_info.rad.target_az));
  status_msg.setTargetElevation(EdgeMsgSupport::binaryToDeg(sweep_info.rad.target_el));
  status_msg.setSpeed(sweep_info.rad.az_speed);
  status_msg.setAntennaSpeed(sweep_info.rad.antenna_speed);
  status_msg.setElevationSpeed(sweep_info.rad.el_speed);
  status_msg.setStartAngle(sweep_info.rad.sector_start);
  status_msg.setStopAngle(sweep_info.rad.sector_end);
  
  //
  // Status message record 5 values
  //

//  status_msg.setDTime();    // Info not in sweep struct
  status_msg.setSiteName(sweep_info.rad.site_name);
  status_msg.setRadarType(sweep_info.rad.radar_type);
  status_msg.setJobName(sweep_info.rad.job_name);
  
  //
  // Status message record 6 values
  //

  status_msg.setLon(sweep_info.rad.long_deg,
		    sweep_info.rad.long_min,
		    sweep_info.rad.long_sec);
  status_msg.setLat(sweep_info.rad.lat_deg,
		    sweep_info.rad.lat_min,
		    sweep_info.rad.lat_sec);
//  status_msg.setAntennaHeight();
  
  //
  // Status message record 7 values
  //

  status_msg.setAzimuth(EdgeMsgSupport::binaryToDeg(sweep_info.rad.az));
  status_msg.setElevation(EdgeMsgSupport::binaryToDeg(sweep_info.rad.el));
//  status_msg.setScdFlag();
  
  //
  // Status message record 8 values
  //

  status_msg.setSigprocFlag(sweep_info.rad.sigproc);
  status_msg.setInterfaceType(sweep_info.rad.interface_type);
//  status_msg.setRadarPower();
//  status_msg.setServo();
//  status_msg.setRadiate();
  
  //
  // Status message record 9 values
  //

  status_msg.setFlags(sweep_info.rad.flags);
  status_msg.setTcfZ(sweep_info.rad.corr_reflec_flags);
  status_msg.setTcfU(sweep_info.rad.uncorr_reflec_flags);
  status_msg.setTcfV(sweep_info.rad.vel_thresh_flags);
  status_msg.setTcfW(sweep_info.rad.wid_thresh_flags);
  status_msg.setClutterFilter(sweep_info.rad.clutter_filter);
  status_msg.setSqi(sweep_info.rad.sqi);
//  status_msg.setPw();
  status_msg.setFold(sweep_info.rad.fold);
  status_msg.setRadarWavelength(sweep_info.rad.wavelength);
  
}


/*********************************************************************
 * _loadStatusVolInfo() - Load the status message information that is
 *                        constant throughout the volume.
 */

void EdgeVol2Socket::_loadStatusVolInfo(EdgeStatusMsg &status_msg,
					const vol_struct &vol_info)
{
  status_msg.setMomentEnable(vol_info.moment_enable);
}
