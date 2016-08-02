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
/*********************************************************************
 * AcarsNc2GenPt: AcarsNc2GenPt program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <assert.h>
#include <iostream>
#include <signal.h>
#include <math.h>
#include <netcdf.hh>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <toolsa/os_config.h>
#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "AcarsNc2GenPt.hh"
#include "Params.hh"

#include "NullConverter.hh"
#include "KtoCConverter.hh"

#include "DataField.hh"

using namespace std;

// Global variables

AcarsNc2GenPt *AcarsNc2GenPt::_instance =
     (AcarsNc2GenPt *)NULL;


/*********************************************************************
 * Constructors
 */

AcarsNc2GenPt::AcarsNc2GenPt(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "AcarsNc2GenPt::AcarsNc2GenPt()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (AcarsNc2GenPt *)NULL);
  
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

AcarsNc2GenPt::~AcarsNc2GenPt()
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

AcarsNc2GenPt *AcarsNc2GenPt::Inst(int argc, char **argv)
{
  if (_instance == (AcarsNc2GenPt *)NULL)
    new AcarsNc2GenPt(argc, argv);
  
  return(_instance);
}

AcarsNc2GenPt *AcarsNc2GenPt::Inst()
{
  assert(_instance != (AcarsNc2GenPt *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool AcarsNc2GenPt::init()
{
  static const string method_name = "AcarsNc2GenPt::init()";
  
  // Initialize the data trigger

  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->input_dir,
		      600, PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger" << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
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
  
  } /* endswitch - _params->trigger_mode */
  
  // Initialize the ACARS file object

  if (!_acarsFile.initialize(_params->num_recs_dim_name,
			     _params->tail_len_dim_name,
			     _params->missing_data_value_att_name,
			     _params->latitude_var_name,
			     _params->longitude_var_name,
			     _params->altitude_var_name,
			     _params->tail_number_var_name,
			     _params->data_source_var_name,
			     _params->data_times_var_name,
			     _params->debug))
    return false;
  
  for (int i = 0; i < _params->data_fields_n; ++i)
  {
    Converter *converter;
    
    switch (_params->_data_fields[i].conversion_type)
    {
    case Params::CONVERT_NONE :
      converter = new NullConverter();
      break;
      
    case Params::CONVERT_K_TO_C :
      converter = new KtoCConverter();
      break;
    } /* endswitch - _params->_data_fields[i].conversion_type */
    
    DataField *field = new DataField(_params->_data_fields[i].netcdf_field_name,
				     _params->_data_fields[i].genpt_field_name,
				     _params->_data_fields[i].genpt_units,
				     converter);
    
    _acarsFile.addDataField(field);
    
  } /* endfor - i */
  
  // initialize process registration

  if (_params->trigger_mode != Params::FILE_LIST)
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void AcarsNc2GenPt::run()
{
  static const string method_name = "AcarsNc2GenPt::run()";
  
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

bool AcarsNc2GenPt::_processFile(const string &input_file)
{
  static const string method_name = "AcarsNc2GenPt::_processFile()";
  
  // Construct the sweep file path.  If the given input file doesn't
  // exist, try prepending the input_dir specified in the parameter
  // file.

  struct stat file_stat;
  
  string acars_file_path = input_file;
  
  if (stat(acars_file_path.c_str(), &file_stat) != 0)
    acars_file_path = string(_params->input_dir) + "/" + input_file;
  
  if (stat(acars_file_path.c_str(), &file_stat) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Specified acars file doesn't exist: " << input_file << endl;
    
    return false;
  }

  // Make sure the file is uncompressed

  char uncompressed_path[BUFSIZ];
  STRcopy(uncompressed_path, acars_file_path.c_str(), BUFSIZ);
  
  if (ta_file_uncompress(uncompressed_path) < 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error uncompressing file: " << acars_file_path << endl;
    
    return false;
  }
  
  // Process the sweep file

  if (_params->debug)
    cerr << "\n*** Processing ACARS file: " << uncompressed_path << endl;
  
  if (!_acarsFile.initializeFile(uncompressed_path))
    return false;
  
  if (!_acarsFile.writeAsSpdb(_params->output_url,
			      !_params->save_all_data_sources,
			      _params->data_source))
    return false;
  
  return true;
}
