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
//   $Id: MdvVertProfile.cc,v 1.5 2016/03/04 02:22:13 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MdvVertProfile: MdvVertProfile program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2008
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
#include <string.h>

#include <toolsa/os_config.h>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "MdvVertProfile.hh"
#include "Params.hh"

#include "AsciiOutput.hh"

using namespace std;

// Global variables

MdvVertProfile *MdvVertProfile::_instance =
     (MdvVertProfile *)NULL;


/*********************************************************************
 * Constructor
 */

MdvVertProfile::MdvVertProfile(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "MdvVertProfile::MdvVertProfile()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (MdvVertProfile *)NULL);
  
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

MdvVertProfile::~MdvVertProfile()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  delete _dataTrigger;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

MdvVertProfile *MdvVertProfile::Inst(int argc, char **argv)
{
  if (_instance == (MdvVertProfile *)NULL)
    new MdvVertProfile(argc, argv);
  
  return(_instance);
}

MdvVertProfile *MdvVertProfile::Inst()
{
  assert(_instance != (MdvVertProfile *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool MdvVertProfile::init()
{
  static const string method_name = "MdvVertProfile::init()";
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
  
  // Initialize the output object

  if (!_initOutput())
    return false;
  
  // initialize process registration

  if (_params->trigger_mode == Params::LATEST_DATA)
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void MdvVertProfile::run()
{
  static const string method_name = "MdvVertProfile::run()";
  
  while (!_dataTrigger->endOfData())
  {
    DateTime trigger_time;
  
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
 * _extractHtProfile() - Extract the vertical profile for the height field.
 */

void MdvVertProfile::_extractHtProfile(const Mdvx::field_header_t field_hdr,
				       const Mdvx::vlevel_header_t &vlevel_hdr,
				       VertProfile &profile) const
{
  static const string method_name = "MdvVertProfile::_extractHtProfile()";

  // Pull out the profile data

  for (int z = 0; z < field_hdr.nz; ++z)
    profile.addValue(vlevel_hdr.level[z]);
}


/*********************************************************************
 * _extractVertProfile() - Extract a vertical profile from the given field
 *                         for the given point.
 */

bool MdvVertProfile::_extractVertProfile(const MdvxField &field,
					 const double lat, const double lon,
					 const double output_missing_value,
					 VertProfile &profile) const
{
  static const string method_name = "MdvVertProfile::_extractVertProfile()";

  // Get the X/Y grid index for the point

  Mdvx::field_header_t field_hdr = field.getFieldHeader();
  MdvxPjg proj(field_hdr);
  int plane_size = field_hdr.nx * field_hdr.ny;
  fl32 *data = (fl32 *)field.getVol();
  
  int plane_index;

  if (proj.latlon2arrayIndex(lat, lon, plane_index) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Point outside of grid" << endl;
    
    return false;
  }
  
  // Pull out the profile data

  for (int z = 0; z < field_hdr.nz; ++z)
  {
    int volume_index = (z * plane_size) + plane_index;
    
    double mdv_value = data[volume_index];
    
    if (mdv_value == field_hdr.bad_data_value ||
	mdv_value == field_hdr.missing_data_value)
      profile.addValue(output_missing_value);
    else
      profile.addValue(mdv_value);
  }
  
  return true;
}


/*********************************************************************
 * _extractVertProfiles() - Extract the vertical profiles for the given
 *                          point.
 *
 * Returns TRUE on success, FALSE on failure.
 */

bool MdvVertProfile::_extractVertProfiles(const DsMdvx &mdvx,
					  const double lat, const double lon,
					  vector< ProfileSet > &profile_sets)
{
  static const string method_name = "MdvVertProfile::_extractVertProfiles()";

  for (int i = 0; i < _params->data_points_n; ++i)
  {
    double lat = _params->_data_points[i].lat;
    double lon = _params->_data_points[i].lon;
    
    ProfileSet set(lat, lon);
    
    // Create the profile for the vertical heights

    VertProfile ht_profile(_params->height_field_name);
    
    _extractHtProfile(mdvx.getField(0)->getFieldHeader(),
		      mdvx.getField(0)->getVlevelHeader(), ht_profile);
    
    set.addProfile(ht_profile);
    
    // Create the profile for each of the fields

    for (int j = 0; j < _params->field_info_n; ++j)
    {
      MdvxField *field = 0;
      
      if (_params->use_mdv_field_names)
	field = mdvx.getField(_params->_field_info[j].mdv_field_name);
      else
	field = mdvx.getField(_params->_field_info[j].mdv_field_num);
      
      if (field == 0)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error extracting data field from MDV file" << endl;
	
	return false;
      }
      
      VertProfile profile(_params->_field_info[j].output_field_name);
      
      _extractVertProfile(*field, lat, lon,
			  _params->_field_info[j].output_missing_value,
			  profile);
      
      set.addProfile(profile);
      
    } /* endfor - j */
    
    profile_sets.push_back(set);
    
  } /* endfor - i */
  
  return true;
}


/*********************************************************************
 * _initOutput() - Initialize the output object.
 *
 * Returns TRUE on success, FALSE on failure.
 */

bool MdvVertProfile::_initOutput()
{
  static const string method_name = "MdvVertProfile::_initOutput()";
  
  _output = new AsciiOutput(_params->output_dir,
			    _params->ascii_output_params.format,
			    _params->ascii_output_params.delimiter,
			    _params->debug);
  
  return true;
}


/*********************************************************************
 * _initTrigger() - Initialize the data trigger object.
 *
 * Returns TRUE on success, FALSE on failure.
 */

bool MdvVertProfile::_initTrigger()
{
  static const string method_name = "MdvVertProfile::_initTrigger()";
  
  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    if (_params->debug)
      cerr << "Initializing LATEST_DATA trigger" << endl;
    
    DsLdataTrigger *trigger = new DsLdataTrigger();
    
    if (trigger->init(_params->input_url,
		      _params->max_valid_secs,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger" << endl;
      cerr << trigger->getErrString() << endl;
      
      return false;
    }

    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::TIME_LIST :
  {
    if (_params->debug)
      cerr << "Initializing TIME_LIST trigger" << endl;
    
    time_t start_time =
      DateTime::parseDateTime(_params->time_list.start_time);
    time_t end_time
      = DateTime::parseDateTime(_params->time_list.end_time);
    
    if (start_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing start_time string for TIME_LIST trigger: " <<
	_params->time_list.start_time << endl;
      
      return false;
    }
    
    if (end_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing end_time string for TIME_LIST trigger: " <<
	_params->time_list.end_time << endl;
      
      return false;
    }
    
    DsTimeListTrigger *trigger = new DsTimeListTrigger();

    if (trigger->init(_params->input_url, start_time, end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger" << endl;
      cerr << "    URL: " << _params->input_url << endl;
      cerr << "    Start time: " << DateTime::str(start_time) << " ("
	   << _params->time_list.start_time << ")" << endl;
      cerr << "    End time: " << DateTime::str(end_time) << " ("
	   << _params->time_list.end_time << ")" << endl;
      cerr << trigger->getErrString() << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  } /* endswitch - _params->trigger_mode */

  return true;
}


/*********************************************************************
 * _processData() - Process the data for the given time.
 *
 * Returns TRUE on success, FALSE on failure.
 */

bool MdvVertProfile::_processData(const DateTime &trigger_time)
{
  static const string method_name = "MdvVertProfile::_processData()";
  
  if (_params->debug)
    cerr << "\n*** Processing data for time: " << trigger_time << endl;
  
  // Read in the MDV file

  DsMdvx mdvx;
  
  if (!_readData(mdvx, trigger_time))
    return false;
  
  // Extract the vertical profiles

  vector< ProfileSet > profile_sets;
  
  for (int i = 0; i < _params->data_points_n; ++i)
  {
    _extractVertProfiles(mdvx,
			 _params->_data_points[i].lat,
			 _params->_data_points[i].lon,
			 profile_sets);
    
  }
  
  // Output the vertical profiles

  if (!_output->writeProfiles(trigger_time, profile_sets))
    return false;
  
  return true;
}


/*********************************************************************
 * _readData() - Read the data for the given time.
 *
 * Returns TRUE on success, FALSE on failure.
 */

bool MdvVertProfile::_readData(DsMdvx &mdvx,
			       const DateTime &read_time) const
{
  static const string method_name = "MdvVertProfile::_readData()";
  
  // Set up the read request

  mdvx.clearRead();
  
  mdvx.setReadTime(Mdvx::READ_CLOSEST,
		   _params->input_url,
		   0, read_time.utime());
  
  for (int i = 0; i < _params->field_info_n; ++i)
  {
    if (_params->use_mdv_field_names)
      mdvx.addReadField(_params->_field_info[i].mdv_field_name);
    else
      mdvx.addReadField(_params->_field_info[i].mdv_field_num);
  }
  
  mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  mdvx.setReadScalingType(Mdvx::SCALING_NONE);
  
  // Read the data

  if (mdvx.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading URL: " << _params->input_url << endl;
    cerr << "    Request time: " << read_time << endl;
    cerr << mdvx.getErrStr() << endl;
    
    return false;
  }
  
  // Make sure the vertical structure is the same for all fields

  bool headers_match = true;
  
  Mdvx::field_header_t field_hdr0 = mdvx.getField(0)->getFieldHeader();
  Mdvx::vlevel_header_t vlevel_hdr0 = mdvx.getField(0)->getVlevelHeader();
  
  for (size_t i = 1; i < mdvx.getNFields(); ++i)
  {
    MdvxField *field = mdvx.getField(i);
    Mdvx::field_header_t field_hdr = field->getFieldHeader();
    Mdvx::vlevel_header_t vlevel_hdr = field->getVlevelHeader();
    
    if (field_hdr0.nz != field_hdr.nz)
    {
      headers_match = false;
      break;
    }
    
    for (int z = 0; z < field_hdr.nz; ++z)
    {
      if (vlevel_hdr0.type[z] != vlevel_hdr.type[z] ||
	  vlevel_hdr0.level[z] != vlevel_hdr.level[z])
      {
	headers_match = false;
	break;
      }
    }
    
    if (!headers_match)
      break;
    
  } /* endfor - i */
  
  return true;
}
