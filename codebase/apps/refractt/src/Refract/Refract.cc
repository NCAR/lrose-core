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
//   $Date: 2016/03/07 18:17:27 $
//   $Id: Refract.cc,v 1.19 2016/03/07 18:17:27 dixon Exp $
//   $Revision: 1.19 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Refract: Refract program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 2008
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <assert.h>
#include <iostream>
#include <math.h>
#include <signal.h>
#include <string>

#include <toolsa/os_config.h>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <toolsa/toolsa_macros.h>
#include <toolsa/Path.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Refract.hh"
#include "Params.hh"

using namespace std;

// Global variables

Refract *Refract::_instance =
     (Refract *)NULL;


/*********************************************************************
 * Constructor
 */

Refract::Refract(int argc, char **argv)
{
  static const string method_name = "Refract::Refract()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (Refract *)NULL);
  
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

Refract::~Refract()
{
  if (_params->debug_level > Params::DEBUG_OFF)
    cerr << endl << endl << endl << "N extraction completed!" << endl << endl;

  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  delete _dataTrigger;
  
  // Free included strings

  STRfree(_progName);

  // Remove temporary files

  system("rm -f targets.tmp.*");

}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

Refract *Refract::Inst(int argc, char **argv)
{
  if (_instance == (Refract *)NULL)
    new Refract(argc, argv);
  
  return(_instance);
}

Refract *Refract::Inst()
{
  assert(_instance != (Refract *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool Refract::init()
{
  static const string method_name = "Refract::init()";
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
  
  // Initialize the input handler

  if (!_initInputHandler())
    return true;
  
  // Read in the reference file

  _calibFile.setReadPath(_params->ref_file_name);
  
  if (_calibFile.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading calibration file: "
	 << _params->ref_file_name << endl;
    cerr << _calibFile.getErrStr() << endl;
    
    return false;
  }
  
  // Initialize the processor

  Processor::quality_type_t quality_type;
  
  switch (_params->quality_source)
  {
  case Params::QUALITY_FROM_WIDTH :
    quality_type = Processor::QUALITY_FROM_WIDTH;
    break;
    
  case Params::QUALITY_FROM_CPA :
    quality_type = Processor::QUALITY_FROM_CPA;
    break;
  }
  
  if (!_processor.init(&_calibFile,
		       _params->num_azim,
		       _params->num_range_bins,
		       _params->r_min,
		       _params->frequency,
		       quality_type,
		       _params->min_consistency,
		       _params->do_relax,
		       _params->n_smoothing_side_len,
		       _params->dn_smoothing_side_len,
		       _params->debug_level >= Params::DEBUG_NORM,
		       _params->debug_level >= Params::DEBUG_EXTRA))
    return false;
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run()
 */

void Refract::run()
{
  static const string method_name = "Refract::run()";
  
  DateTime trigger_time;
  
  while (!_dataTrigger->endOfData())
  {
    if (_dataTrigger->nextIssueTime(trigger_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
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
 * _initInputHandler()
 */

bool Refract::_initInputHandler(void)
{
  static const string method_name = "Refract::_initInputHandler()";

  // Figure out which field we are using to calculate the quality

  if (_params->specify_elevation_by_index)
    _inputHandler.init(_params->input_url,
		       _params->raw_iq_in_input,
		       _params->raw_i_field_name,
		       _params->raw_q_field_name,
		       _params->niq_field_name,
		       _params->aiq_field_name,
		       _params->quality_field_name,
		       _params->snr_field_name,
		       _params->input_niq_scale,
		       _params->invert_target_angle_sign,
		       _params->elevation_num,
		       _params->num_azim,
		       _params->num_range_bins,
		       _params->debug_level >= Params::DEBUG_OFF,
		       _params->debug_level >= Params::DEBUG_VERBOSE);
  else
    _inputHandler.init(_params->input_url,
		       _params->raw_iq_in_input,
		       _params->raw_i_field_name,
		       _params->raw_q_field_name,
		       _params->niq_field_name,
		       _params->aiq_field_name,
		       _params->quality_field_name,
		       _params->snr_field_name,
		       _params->input_niq_scale,
		       _params->invert_target_angle_sign,
		       _params->elevation_angle.min_angle,
		       _params->elevation_angle.max_angle,
		       _params->num_azim,
		       _params->num_range_bins,
		       _params->debug_level >= Params::DEBUG_OFF,
		       _params->debug_level >= Params::DEBUG_VERBOSE);

  return true;
}


/*********************************************************************
 * _initTrigger()
 */

bool Refract::_initTrigger(void)
{
  static const string method_name = "Refract::_initTrigger()";
  
  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    if (_params->debug_level >= Params::DEBUG_NORM)
    {
      cerr << "Initializing LATEST_DATA trigger: " << endl;
      cerr << "    URL = " << _params->input_url << endl;
    }
    
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->input_url,
                      _params->max_valid_secs,
                      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger: " << endl;
      cerr << "    URL = " << _params->input_url << endl;
      cerr << trigger->getErrStr() << endl;
      
      return false;
    }

    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::TIME_LIST :
  {
    DateTime start_time = _args->getStartTime();
    DateTime end_time = _args->getEndTime();
    
    if (start_time == DateTime::NEVER ||
        end_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Must specify start and end times on command line" << endl;
      
      return false;
    }
    
    if (_params->debug_level >= Params::DEBUG_NORM)
    {
      cerr << "Initializing TIME_LIST trigger: " << endl;
      cerr << "   URL: " << _params->input_url << endl;
      cerr << "   start time: " << start_time << endl;
      cerr << "   end time: " << end_time << endl;
    }
    
    DsTimeListTrigger *trigger = new DsTimeListTrigger();
    if (trigger->init(_params->input_url,
                      start_time.utime(), end_time.utime()) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger:" << endl;
      cerr << "    URL: " << _params->input_url << endl;
      cerr << "    Start time: " << start_time << endl;
      cerr << "    End time: " << end_time << endl;
      cerr << trigger->getErrStr() << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  } /* endswitch - _params->trigger_mode */

  return true;
}


/*********************************************************************
 * _processData()
 */

bool Refract::_processData(const DateTime &trigger_time)
{
  static const string method_name = "Refract::_processData()";
  
  PMU_auto_register("Processing data");
  
  if (_params->debug_level >= Params::DEBUG_NORM)
    cerr << "**** Processing data for time: " << trigger_time << endl;
  
  // Get the next scan from the input handler

  DsMdvx data_file;
  
  if (!_inputHandler.getNextScan(trigger_time, data_file))
    return false;

  if (!_processor.processScan(data_file))
    return false;
  
  // Generate output file

  if (_params->debug_level >= Params::DEBUG_NORM)
    cerr << "---> Writing data for scan at time "
	 << DateTime::str(data_file.getMasterHeader().time_centroid) << endl;
  
  if (data_file.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing output file to URL: "
	 << _params->output_url << endl;
    cerr << data_file.getErrStr() << endl;
      
    return false;
  }
    
  return true;
}


/*********************************************************************
 * _updateMasterHdr()
 */

void Refract::_updateMasterHdr(DsMdvx &output_file,
			       const DateTime &scan_time,
			       const double radar_lat,
			       const double radar_lon,
			       const double radar_alt) const
{
  Mdvx::master_header_t master_hdr;
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  master_hdr.time_gen = time(0);
  master_hdr.time_begin = scan_time.utime();
  master_hdr.time_end = scan_time.utime();
  master_hdr.time_centroid = scan_time.utime();
  master_hdr.time_expire = scan_time.utime();
  master_hdr.data_dimension = 2;
  master_hdr.data_collection_type = Mdvx::DATA_MEASURED;
  master_hdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
  master_hdr.vlevel_type = Mdvx::VERT_TYPE_ELEV;
  master_hdr.vlevel_included = 1;
  master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;
  master_hdr.sensor_lon = radar_lon;
  master_hdr.sensor_lat = radar_lat;
  master_hdr.sensor_alt = radar_alt;
  STRcopy(master_hdr.data_set_info, "Refract debug fields", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "Refract debug", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, "Refract", MDV_NAME_LEN);
  
  output_file.setMasterHeader(master_hdr);
}
