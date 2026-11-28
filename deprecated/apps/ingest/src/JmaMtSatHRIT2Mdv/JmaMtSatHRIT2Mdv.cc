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
 * @file JmaMtSatHRIT2Mdv.cc
 *
 * @class JmaMtSatHRIT2Mdv
 *
 * JmaMtSatHRIT2Mdv is the top level application class.
 *  
 * @date 1/30/2012
 *
 */

#include <iostream>
#include <math.h>
#include <string>
#include <vector>

#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <Mdv/MdvxPjg.hh>
#include <rapmath/math_macros.h>
#include <toolsa/os_config.h>
#include <toolsa/Path.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "JmaMtSatHRIT2Mdv.hh"
#include "Params.hh"

using namespace std;


// Global variables

JmaMtSatHRIT2Mdv *JmaMtSatHRIT2Mdv::_instance = (JmaMtSatHRIT2Mdv *)NULL;


/*********************************************************************
 * Constructor
 */

JmaMtSatHRIT2Mdv::JmaMtSatHRIT2Mdv(int argc, char **argv) :
  _outputVlevelType(Mdvx::VERT_SATELLITE_IMAGE)
{
  static const string method_name = "JmaMtSatHRIT2Mdv::JmaMtSatHRIT2Mdv()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (JmaMtSatHRIT2Mdv *)NULL);
  
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

JmaMtSatHRIT2Mdv::~JmaMtSatHRIT2Mdv()
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
 * Inst()
 */

JmaMtSatHRIT2Mdv *JmaMtSatHRIT2Mdv::Inst(int argc, char **argv)
{
  if (_instance == (JmaMtSatHRIT2Mdv *)NULL)
    new JmaMtSatHRIT2Mdv(argc, argv);
  
  return(_instance);
}

JmaMtSatHRIT2Mdv *JmaMtSatHRIT2Mdv::Inst()
{
  assert(_instance != (JmaMtSatHRIT2Mdv *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init()
 */

bool JmaMtSatHRIT2Mdv::init()
{
  static const string method_name = "JmaMtSatHRIT2Mdv::init()";
  
  // Initialize the input trigger

  if (!_initTrigger())
    return false;
  
  // Initialize the output domains

  if (!_initDomains())
    return false;
  
  // Initialize the output vertical level type

  if (!_initOutputVlevelType())
    return false;
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}
  


/*********************************************************************
 * run()
 */

void JmaMtSatHRIT2Mdv::run()
{
  static const string method_name = "JmaMtSatHRIT2Mdv::run()";
  
  TriggerInfo trigger_info;
  
  while (!_dataTrigger->endOfData())
  {
    if (_dataTrigger->next(trigger_info) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    if (!_processData(trigger_info))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing file: " << trigger_info.getFilePath() << endl;
      
      continue;
    }
    
  } /* endwhile - !_dataTrigger->endOfData() */

}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _convertUnits()
 */

void JmaMtSatHRIT2Mdv::_convertUnits(MdvxField &field) const
{
  static const string method_name = "JmaMtSatHRIT2Mdv::_convertUnits()";
  
  Mdvx::field_header_t field_hdr = field.getFieldHeader();
  fl32 *data = (fl32 *)field.getVol();

  for (int i = 0; i < field_hdr.nx * field_hdr.ny * field_hdr.nz; ++i)
  {
    // Skip missing data values

    if (data[i] == field_hdr.bad_data_value ||
	data[i] == field_hdr.missing_data_value)
      continue;

    switch (_params->units_convert_type)
    {
    case Params::CONVERT_K_TO_C :
      data[i] = TEMP_K_TO_C(data[i]);
      break;
    } /* endswitch - _params->units_convert_type */
  } /* endfor - i */

  // Update the units string in the field header

  switch (_params->units_convert_type)
  {
  case Params::CONVERT_K_TO_C :
    STRcopy(field_hdr.units, "C", MDV_UNITS_LEN);
    break;
  } /* endswitch - _params->units_convert_type */

  field.setFieldHeader(field_hdr);
}


/*********************************************************************
 * _createField()
 */

MdvxField *JmaMtSatHRIT2Mdv::_createField(const string &field_name,
					  const string &units,
					  const MdvxProj &proj)
{
  static const string method_name = "JmaMtSatHRIT2Mdv::_createField()";
  
  // Create the field header

  Mdvx::field_header_t field_hdr;
  memset(&field_hdr, 0, sizeof(field_hdr));

  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = _outputVlevelType;
  field_hdr.vlevel_type = _outputVlevelType;
  field_hdr.dz_constant = 1;
  field_hdr.data_dimension = 2;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = -999.0;
  field_hdr.missing_data_value = -999.0;
  if (_params->override_field_names)
  {
    STRcopy(field_hdr.field_name_long,
	    _params->output_field_names.field_name_long, MDV_LONG_FIELD_LEN);
    STRcopy(field_hdr.field_name,
	    _params->output_field_names.field_name, MDV_SHORT_FIELD_LEN);
  }
  else
  {
    STRcopy(field_hdr.field_name_long, field_name.c_str(), MDV_LONG_FIELD_LEN);
    STRcopy(field_hdr.field_name, field_name.c_str(), MDV_SHORT_FIELD_LEN);
  }
  STRcopy(field_hdr.units, units.c_str(), MDV_UNITS_LEN);
  proj.syncToFieldHdr(field_hdr);

  // Create the vlevel header

  Mdvx::vlevel_header_t vlevel_hdr;
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
  vlevel_hdr.type[0] = _outputVlevelType;
  vlevel_hdr.level[0] = 0.0;
  
  return new MdvxField(field_hdr, vlevel_hdr, (void *)0, true);
}


/*********************************************************************
 * _initDomains()
 */

bool JmaMtSatHRIT2Mdv::_initDomains(void)
{
  static const string method_name = "JmaMtSatHRIT2Mdv::_initDomains()";
  
  // Make sure we have at least one output domain.  If there aren't any
  // output domains then there's no reason to run the program.

  if (_params->domains_n < 1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Parameter file doesn't define any output domains." << endl;
    cerr << "Add an output domain and try again." << endl;
    
    return false;
  }
  
  // Save the projections

  domain_info_t domain_info;
  
  for (int i = 0; i < _params->domains_n; ++i)
  {
    Params::output_params_t domain = _params->_domains[i];
    
    switch (domain.proj_type)
    {
    case Params::PROJ_LATLON :
    {
      domain_info.proj.initLatlon();
      domain_info.proj.setGrid(domain.nx, domain.ny,
			       domain.dx, domain.dy,
			       domain.minx, domain.miny);
      domain_info.url = domain.url;

      _outputDomains.push_back(domain_info);

      break;
    }

    case Params::PROJ_LC :
    {
      domain_info.proj.initLambertConf(domain.origin_lat, domain.origin_lon,
				       domain.lat1, domain.lat2);
      domain_info.proj.setGrid(domain.nx, domain.ny,
			       domain.dx, domain.dy,
			       domain.minx, domain.miny);
      domain_info.url = domain.url;

      _outputDomains.push_back(domain_info);

      break;
    }
    
    } /* endswitch - proj_type */
  } /* endfor - i */
  
  return true;
}


/*********************************************************************
 * _initOutputVlevelType()
 */

bool JmaMtSatHRIT2Mdv::_initOutputVlevelType(void)
{
  static const string method_name = "JmaMtSatHRIT2Mdv::_initOutputVlevelType()";
  
  switch (_params->output_vlevel_type)
  {
  case Params::VERT_TYPE_SURFACE :
    _outputVlevelType = Mdvx::VERT_TYPE_SURFACE;
    break;
    
  case Params::VERT_TYPE_Z :
    _outputVlevelType = Mdvx::VERT_TYPE_Z;
    break;
    
  case Params::VERT_SATELLITE_IMAGE :
    _outputVlevelType = Mdvx::VERT_SATELLITE_IMAGE;
    break;
    
  case Params::VERT_FLIGHT_LEVEL :
    _outputVlevelType = Mdvx::VERT_FLIGHT_LEVEL;
    break;
  }
  
  return true;
}


/*********************************************************************
 * _initTrigger()
 */

bool JmaMtSatHRIT2Mdv::_initTrigger(void)
{
  static const string method_name = "JmaMtSatHRIT2Mdv::_initTrigger()";
  
  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->input_dir,
		      _params->max_valid_secs,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger:" << endl;
      cerr << "   input dir = " << _params->input_dir << endl;
      cerr << "   max valid secs = " << _params->max_valid_secs << endl;

      return false;
    }

    _dataTrigger = trigger;

    break;
  }

  case Params::FILE_LIST :
  {
    const vector< string > file_list = _args->getFileList();
    
    if (file_list.size() == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Must specify file paths on command line" << endl;
      
      return false;
    }
    
    if (_params->debug >= Params::DEBUG_NORM)
    {
      cerr << "Initializing FILE_LIST trigger: " << endl;
      vector< string >::const_iterator file;
      for (file = file_list.begin(); file != file_list.end(); ++file)
	cerr << "   " << *file << endl;
    }
    
    DsFileListTrigger *trigger = new DsFileListTrigger();
    if (trigger->init(file_list) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing FILE_LIST trigger:" << endl;
      vector< string >::const_iterator file;
      for (file = file_list.begin(); file != file_list.end(); ++file)
	cerr << "   " << *file << endl;
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

bool JmaMtSatHRIT2Mdv::_processData(const TriggerInfo &trigger_info)
{
  static const string method_name = "JmaMtSatHRIT2Mdv::_processData()";
  
  string file_path = trigger_info.getFilePath();

  if (_params->debug >= Params::DEBUG_NORM)
    cerr << endl << endl
	 << "*** Processing trigger: " << file_path << endl << endl;
  
  PMU_auto_register("Processing trigger");

  // Read in the HRIT files.

  HRITFile combined_hrit_file;
  if (!_readHritFiles(file_path, combined_hrit_file))
    return false;

  // If we want to use scaling to calibrate the data rather than using the
  // provided look-up table, set that here.
  
  if (_params->calib_type == Params::CALIBRATE_SCALE)
    combined_hrit_file.setCalibScale(_params->calib_scale.scale,
				     _params->calib_scale.bias);
  
  // Generate an MDV file for each specified domain

  for (vector< domain_info_t >::const_iterator domain = _outputDomains.begin();
       domain != _outputDomains.end(); ++domain)
  {
    // Create the output file

    DsMdvx mdvx;
    _updateMasterHeader(mdvx, combined_hrit_file);
    
    // Create the output field and add it to the file

    MdvxField *field;

    if ((field = _createField(combined_hrit_file.getFieldName(),
			      combined_hrit_file.getUnits(),
			      domain->proj)) == 0)
      return false;
    
    if (!combined_hrit_file.addData(MdvxPjg(field->getFieldHeader()),
				    (fl32 *)field->getVol()))
      continue;
    
    // Convert the units in the field, if requested

    if (_params->convert_units)
      _convertUnits(*field);

    // Add the field to the output file

    field->convertType(Mdvx::ENCODING_INT8,
		       Mdvx::COMPRESSION_ZLIB,
		       Mdvx::SCALING_DYNAMIC);
    mdvx.addField(field);
    
    // Write the output file

    if (_params->debug >= Params::DEBUG_NORM)
      cerr << "    Writing output to: " << domain->url << " for time "
	   << DateTime::str(mdvx.getMasterHeader().time_centroid) << endl;
    
    if (mdvx.writeToDir(domain->url) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error writing MDV file to output URL: " << domain->url << endl;
      cerr << mdvx.getErrStr() << endl;
      
      continue;
    }
    
  } /* endfor - domain */
  
  return true;
}


/*********************************************************************
 * _readHritFiles()
 */

bool JmaMtSatHRIT2Mdv::_readHritFiles(const string &trigger_file_path,
				      HRITFile &combined_hrit_file) const
{
  static const string method_name = "JmaMtSatHRIT2Mdv::_readHritFiles()";
  
  if (_params->debug >= Params::DEBUG_VERBOSE)
  {
    cerr << "*** Entering JmaMtSatHRIT2Mdv::_readHritFiles()" << endl;
    cerr << "    trigger_file_path = " << trigger_file_path << endl;
  }

  // Extract the data directory and the base file name from the trigger path.
  // The file names end in "_nnn" where nnn is the segment number.

  Path path(trigger_file_path);
  string trigger_file_name = path.getFile();
  
  size_t underline_pos = trigger_file_name.rfind('_');
  
  if (underline_pos == string::npos)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Cannot get file names from trigger file name" << endl;
    cerr << "No underscore in trigger file name: " << trigger_file_name << endl;
    
    return false;
  }
  
  string base_file_name = trigger_file_name.substr(0, underline_pos);
  
  if (_params->debug >= Params::DEBUG_VERBOSE)
    cerr << "    base_file_name = " << base_file_name << endl;
  
  string segment_number_string = trigger_file_name.substr(underline_pos+1);
  int first_segment_number = atoi(segment_number_string.c_str());

  if (_params->debug >= Params::DEBUG_VERBOSE)
  {
    cerr << "     segment_number_string = " << segment_number_string << endl;
    cerr << "     first_segment_number = " << first_segment_number << endl;
  }

  // Now read each of the files in turn.  We read them in numerical order,
  // ignoring which file was used as the trigger, because all of them must
  // be here or there is an error.  Reading them in order allows us to
  // concatenate them easily.  We have to process the first file separately
  // to get the number of total number of segments, so we might as well read
  // that one directly into the combined file as a start.

  PMU_auto_register("Reading input file");

  char file_path[1024];
  
  sprintf(file_path, "%s/%s_001",
  	  path.getDirectory().c_str(), base_file_name.c_str());
  
  if (_params->debug >= Params::DEBUG_NORM)
    cerr << "   Reading file: " << file_path << endl;
  
  if (!combined_hrit_file.read(file_path))
    return false;
  
//  if (_params->debug >= Params::DEBUG_VERBOSE)
//    combined_hrit_file.print(cerr);
  
  int num_segments = combined_hrit_file.getNumSegments();

  if (_params->debug >= Params::DEBUG_NORM)
    cerr << "    Image has " << num_segments << " segments" << endl;
  
  // Now read the read of the image files and concatenate them to the
  // combined file

  for (int i = 1; i < num_segments; ++i)
  {
    // Construct the file path

    sprintf(file_path, "%s/%s_%03d",
	    path.getDirectory().c_str(), base_file_name.c_str(), i + 1);
  
    if (_params->debug >= Params::DEBUG_NORM)
      cerr << "   Reading file: " << file_path << endl;
  
    // Read the file

    HRITFile current_hrit_file;
  
    if (!current_hrit_file.read(file_path))
      return false;
  
//    if (_params->debug >= Params::DEBUG_VERBOSE)
//      current_hrit_file.print(cerr);

    // Add it to the combined file

    if (!combined_hrit_file.concatenate(current_hrit_file))
      return false;
    
  }
  
  if (_params->debug >= Params::DEBUG_VERBOSE)
    combined_hrit_file.print(cerr);
  
  return true;
}


/*********************************************************************
 * _updateMasterHeader()
 */

void JmaMtSatHRIT2Mdv::_updateMasterHeader(DsMdvx &mdvx,
					   const HRITFile &hrif_file)
{
  static const string method_name = "JmaMtSatHRIT2Mdv::_updateMasterHeader()";
  
  Mdvx::master_header_t master_hdr;
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  master_hdr.time_gen = time(0);
  master_hdr.time_begin = hrif_file.getStartTime().utime();
  master_hdr.time_end = hrif_file.getEndTime().utime();
  switch (_params->output_timestamp)
  {
  case Params::TIMESTAMP_BEGIN :
    master_hdr.time_centroid = master_hdr.time_begin;
    break;
  case Params::TIMESTAMP_END :
    master_hdr.time_centroid = master_hdr.time_end;
    break;
  case Params::TIMESTAMP_MIDDLE :
    // Note this form of the calculation avoids integer overflow

    master_hdr.time_centroid =
      (master_hdr.time_begin / 2) + (master_hdr.time_end / 2);
    break;
  }
  master_hdr.time_expire = master_hdr.time_end;
  master_hdr.data_dimension = 2;
  master_hdr.data_collection_type = Mdvx::DATA_MEASURED;
  master_hdr.native_vlevel_type = _outputVlevelType;
  master_hdr.vlevel_type = _outputVlevelType;
  master_hdr.vlevel_included = 1;
  master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;
  master_hdr.sensor_lon = hrif_file.getSubLon();
  STRcopy(master_hdr.data_set_info, "JmaMtSatHRIT2Mdv", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "JmaMtSatHRIT2Mdv", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, "JmaMtSatHRIT2Mdv", MDV_NAME_LEN);
  
  mdvx.setMasterHeader(master_hdr);
}
