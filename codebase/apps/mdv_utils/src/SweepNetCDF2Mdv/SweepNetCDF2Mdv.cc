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
//   $Date: 2016/03/04 02:22:13 $
//   $Id: SweepNetCDF2Mdv.cc,v 1.15 2016/03/04 02:22:13 dixon Exp $
//   $Revision: 1.15 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * SweepNetCDF2Mdv: SweepNetCDF2Mdv program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <assert.h>
#include <iostream>
#include <signal.h>
#include <math.h>
#include <Ncxx/Nc3File.hh>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <toolsa/os_config.h>
#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsInputDirTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "SweepNetCDF2Mdv.hh"
#include "Params.hh"
#include "SweepFile.hh"

#include "StartTimeVolumeTrigger.hh"
#include "VolNumVolumeTrigger.hh"

using namespace std;

// Global variables

SweepNetCDF2Mdv *SweepNetCDF2Mdv::_instance =
     (SweepNetCDF2Mdv *)NULL;


/*********************************************************************
 * Constructors
 */

SweepNetCDF2Mdv::SweepNetCDF2Mdv(int argc, char **argv) :
  _dataTrigger(0),
  _volumeTrigger(0),
  _currVolumeStartTime(DateTime::NEVER),
  _firstFile(true)
{
  static const string method_name = "SweepNetCDF2Mdv::SweepNetCDF2Mdv()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (SweepNetCDF2Mdv *)NULL);
  
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

SweepNetCDF2Mdv::~SweepNetCDF2Mdv()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  delete _dataTrigger;
  delete _volumeTrigger;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

SweepNetCDF2Mdv *SweepNetCDF2Mdv::Inst(int argc, char **argv)
{
  if (_instance == (SweepNetCDF2Mdv *)NULL)
    new SweepNetCDF2Mdv(argc, argv);
  
  return(_instance);
}

SweepNetCDF2Mdv *SweepNetCDF2Mdv::Inst()
{
  assert(_instance != (SweepNetCDF2Mdv *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool SweepNetCDF2Mdv::init()
{
  static const string method_name = "SweepNetCDF2Mdv::init()";
  
  // Initialize the data trigger

  switch (_params->trigger_mode)
  {
  case Params::FILE_LIST :
  {
    DsFileListTrigger *trigger = new DsFileListTrigger();
    if (trigger->init(_args->getFileList()) != 0)
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
		      _params->input_substring,
		      false, PMU_auto_register, false,
		      _params->exclude_substring) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing INPUT_DIR trigger" << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::INPUT_DIR_RECURSE :
  {
    DsInputDirTrigger *trigger = new DsInputDirTrigger();
    if (trigger->init(_params->input_dir,
		      _params->input_substring,
		      false, PMU_auto_register, true,
		      _params->exclude_substring) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing INPUT_DIR_RECURSE trigger" << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::LATEST_DATA :
  {
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->input_dir,
		      _params->max_valid_secs,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger" << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  } /* endswitch - _params->trigger_mode */
  
  // Initialize the volume trigger

  switch (_params->volume_trigger)
  {
  case Params::START_TIME_VOL_TRIGGER :
    _volumeTrigger = new StartTimeVolumeTrigger(_params->debug);
    break;
    
  case Params::VOL_NUMBER_VOL_TRIGGER :
    _volumeTrigger = new VolNumVolumeTrigger(_params->debug);
    break;
  }
  
  // initialize process registration

  if (_params->trigger_mode != Params::FILE_LIST)
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void SweepNetCDF2Mdv::run()
{
  static const string method_name = "SweepNetCDF2Mdv::run()";
  
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
 * _processFile() - Process the given file.
 */

bool SweepNetCDF2Mdv::_processFile(const string &input_file)
{
  static const string method_name = "SweepNetCDF2Mdv::_processFile()";
  
  PMU_auto_register("Processing file...");

  // Construct the sweep file path.  If the given input file doesn't
  // exist, try prepending the input_dir specified in the parameter
  // file.

  struct stat file_stat;
  
  string sweep_file_path = input_file;
  
  if (stat(sweep_file_path.c_str(), &file_stat) != 0)
    sweep_file_path = string(_params->input_dir) + "/" + input_file;
  
  if (stat(sweep_file_path.c_str(), &file_stat) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Specified sweep file doesn't exist: " << input_file << endl;
    
    return false;
  }

  // Process the sweep file

  if (_params->debug)
  {
    cerr << "\n*** Processing sweep file: " << sweep_file_path << endl;
    cerr << "   sweep_file_path = " << sweep_file_path << endl;
    cerr << "   field_list_var_name = " << _params->field_list_var_name << endl;
    cerr << "   missing_data_value_att_name = " << _params->missing_data_value_att_name << endl;
    cerr << "   output_beamwidth = " << _params->output_beamwidth << endl;
    cerr << "   debug = " << _params->debug << endl;
  }

  SweepFile sweep_file(sweep_file_path,
                       _params->num_gates_dim_name,
		       _params->field_list_var_name,
		       _params->missing_data_value_att_name,
		       _params->bias_specified,
		       _params->output_beamwidth,
		       _params->force_negative_longitude,
		       _params->override_file_missing_data_value,
		       _params->missing_data_value,
		       _params->fix_missing_beams,
		       _params->debug);
  
  if (!sweep_file.initialize())
    return false;

  // If this is the first file we've processed, save the start time for
  // calculating the volume start time when the file is written

  if (_firstFile)
  {
    if (!sweep_file.getVolumeStartTime(_currVolumeStartTime))
      return false;
    
    _firstFile = false;
  }
  
  // See if this is a new volume.  If it is, we need to write out the old
  // one and start accumulating a new one.

  if (_volumeTrigger->isNewVolume(sweep_file))
  {
    // Write the MDV file

    DateTime volume_end_time;
    
    if (!sweep_file.getVolumeStartTime(volume_end_time))
      return false;
    
    if (!_writeMdvFile(_mdvFile, volume_end_time))
      return false;
  
    // Clear out the MDV file to prepare for a new volume

    _mdvFile.clearFields();
    _mdvFile.clearChunks();

    // Save the start time for the new volume

    if (!sweep_file.getVolumeStartTime(_currVolumeStartTime))
      return false;
  }
  
  // Add the new sweep to the MDV file

  if (!sweep_file.addSweepToMdv(_mdvFile))
    return false;
  
  return true;
}


/*********************************************************************
 * _writeMdvFile() - Write the MDV file to the appropriate URL
 *
 * Returns true on success, false on failure.
 */

bool SweepNetCDF2Mdv::_writeMdvFile(DsMdvx &mdv_file,
				    const DateTime &volume_end_time)
{
  static const string method_name = "SweepNetCDF2Mdv::_writeMdvFile()";
  
  if (_params->debug)
  {
    cerr << "Writing MDV file to URL: " << _params->output_url << endl;

    Mdvx::master_header_t master_hdr = mdv_file.getMasterHeader();
    cerr << "   time_centroid = "
	 << DateTime::str(master_hdr.time_centroid) << endl;
  }
  
  // Compress all of the fields

  for (int field_num = 0; field_num < mdv_file.getMasterHeader().n_fields;
       ++field_num)
  {
    MdvxField *field = mdv_file.getField(field_num);
    
    field->convertType(Mdvx::ENCODING_INT8,
		       Mdvx::COMPRESSION_BZIP,
		       Mdvx::SCALING_DYNAMIC);
  } /* endfor - field_num */
  
  // Update the times in the master header.  We are doing this here because
  // some of the files we got had bad time offset values.  So, rather than
  // relying on the time offset values we'll just assume that each volume
  // ends at the start of the next volume.

  Mdvx::master_header_t master_hdr = mdv_file.getMasterHeader();
  master_hdr.time_end = volume_end_time.utime();
  master_hdr.time_expire = master_hdr.time_end;
  master_hdr.time_centroid =
    (master_hdr.time_begin / 2) + (master_hdr.time_end / 2);
  mdv_file.setMasterHeader(master_hdr);
  
  // Write out the file

  if (mdv_file.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing MDV file to URL: " << _params->output_url << endl;
    cerr << mdv_file.getErrStr() << endl;
    
    return false;
  }
  
  return true;
}
