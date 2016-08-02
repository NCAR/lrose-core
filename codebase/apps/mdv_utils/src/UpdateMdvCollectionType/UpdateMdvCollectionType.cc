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

/*********************************************************************
 * UpdateMdvCollectionType: UpdateMdvCollectionType program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2007
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cassert>
#include <signal.h>

#include <toolsa/os_config.h>
#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Params.hh"
#include "UpdateMdvCollectionType.hh"

using namespace std;

// Global variables

UpdateMdvCollectionType *UpdateMdvCollectionType::_instance = NULL;

/*********************************************************************
 * Constructor
 */

UpdateMdvCollectionType::UpdateMdvCollectionType(int argc, char **argv)
{
  static const string method_name = "UpdateMdvCollectionType::UpdateMdvCollectionType()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (UpdateMdvCollectionType *)NULL);
  
  // Set the singleton instance pointer

  _instance = this;

  // Initialize the okay flag.

  okay = true;
  
  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);
  
  // display ucopyright message.

  ucopyright(_progName);

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);
  
  if (!_args->okay)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr  << "Problem with command line arguments." << endl;
    
    okay = false;
    
    return;
  }
  
  // Get TDRP parameters.

  _params = new Params();
  char *params_path = "unknown";
  
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with TDRP parameters in file <"
	 << params_path << ">" << endl;
    
    okay = false;
    
    return;
  }

  // Convert the specified data collection type to its MDV equivalent

  switch (_params->data_collection_type)
  {
  case Params::DATA_MEASURED :
    _dataCollectionType = Mdvx::DATA_MEASURED;
    break;
    
  case Params::DATA_EXTRAPOLATED :
    _dataCollectionType = Mdvx::DATA_EXTRAPOLATED;
    break;
    
  case Params::DATA_FORECAST :
    _dataCollectionType = Mdvx::DATA_FORECAST;
    break;
    
  case Params::DATA_SYNTHESIS :
    _dataCollectionType = Mdvx::DATA_SYNTHESIS;
    break;
    
  case Params::DATA_MIXED :
    _dataCollectionType = Mdvx::DATA_MIXED;
    break;
    
  case Params::DATA_IMAGE :
    _dataCollectionType = Mdvx::DATA_IMAGE;
    break;
    
  case Params::DATA_GRAPHIC :
    _dataCollectionType = Mdvx::DATA_GRAPHIC;
    break;
  }
  
  // Create the data trigger object

  switch (_params->mode)
  {
  case Params::TIME_LIST:
  {
    DsTimeListTrigger *trigger = new DsTimeListTrigger();
    
    if (trigger->init(_params->url,
		      _args->getStartTime(),
		      _args->getEndTime()) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST_TRIGGER:" << endl;
      cerr << "    URL: " << _params->url << endl;
      cerr << "    start time: " << DateTime::str(_args->getStartTime()) << endl;
      cerr << "    end time: " << DateTime::str(_args->getEndTime()) << endl;
      
      exit(-1);
    }
    
    _trigger = trigger;
    
    break;
  }
  
  case Params::FILE_LIST :
  {
    DsFileListTrigger *trigger = new DsFileListTrigger();
    
    if (trigger->init(_args->getFileList()) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing FILE_LIST trigger:" << endl;
      
      vector< string > file_list = _args->getFileList();
      vector< string >::const_iterator file_name;
      
      for (file_name = file_list.begin(); file_name != file_list.end();
	   ++file_name)
	cerr << "    " << (*file_name) << endl;
      
      exit(-1);
    }
    
    _trigger = trigger;
    
    break;
  }
  
  } /* endswitch - _params->mode */
  
}


/*********************************************************************
 * Destructor
 */

UpdateMdvCollectionType::~UpdateMdvCollectionType()
{
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

UpdateMdvCollectionType *UpdateMdvCollectionType::Inst(int argc, char **argv)
{
  if (_instance == (UpdateMdvCollectionType *)NULL)
    new UpdateMdvCollectionType(argc, argv);
  
  return(_instance);
}

UpdateMdvCollectionType *UpdateMdvCollectionType::Inst()
{
  assert(_instance != (UpdateMdvCollectionType *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * run()
 */

int UpdateMdvCollectionType::run()
{
  static const string method_name = "UpdateMdvCollectionType::run()";
  
  TriggerInfo trigger_info;
  
  while (!_trigger->endOfData())
  {
    if (_trigger->next(trigger_info) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger" << endl;
      
      continue;
    }
    
    if (trigger_info.getFilePath() == "")
    {
      if (_processFile(trigger_info.getIssueTime()) != 0)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error processing file for time: "
	     << trigger_info.getIssueTime() << endl;
      
	continue;
      }
    }
    else
    {
      if (_processFile(trigger_info.getFilePath()) != 0)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error processing file: "
	     << trigger_info.getFilePath() << endl;
      
	continue;
      }
    
    }
    
  } /* endwhile - !_trigger->endOfData() */
  
  return 0;
}

  
/*********************************************************************
 * process file
 */

int UpdateMdvCollectionType::_processFile(const string &file_path)
{
  static const string method_name = "UpdateMdvCollectionType::_processFile()";
  
  if (_params->debug >= Params::DEBUG_NORM)
    cerr << "Processing file: " << file_path << endl;

  // Read in the file

  DsMdvx mdvx;
  mdvx.setReadPath(file_path);

  if (_readAndProcessFile(mdvx) != 0)
    return -1;
  
  if (mdvx.writeToPath(file_path.c_str()) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "  Cannot write out file to dir: " << _params->url << endl;
    cerr << "  " << mdvx.getErrStr() << endl;
    return -1;
  }

  if (_params->debug) {
    cerr << "Wrote file: " << mdvx.getPathInUse() << endl;
  }

  return 0;
}


int UpdateMdvCollectionType::_processFile(const DateTime &data_time)
{
  static const string method_name = "UpdateMdvCollectionType::_processFile()";
  
  if (_params->debug >= Params::DEBUG_NORM)
    cerr << "Processing file for time: " << data_time << endl;

  // Read in the file

  DsMdvx mdvx;
  mdvx.setReadTime(Mdvx::READ_CLOSEST,
		   _params->url, 0, data_time.utime());

  if (_readAndProcessFile(mdvx) != 0)
    return -1;
  
  if (mdvx.writeToDir(_params->url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "  Cannot write out file to dir: " << _params->url << endl;
    cerr << "  " << mdvx.getErrStr() << endl;
    return -1;
  }

  if (_params->debug) {
    cerr << "Wrote file: " << mdvx.getPathInUse() << endl;
  }

  return 0;
}


/*********************************************************************
 * read and process file
 */

int UpdateMdvCollectionType::_readAndProcessFile(DsMdvx &mdvx)
{
  static const string method_name = "UpdateMdvCollectionType::_readAndProcessFile()";
  
  // Read in the file

  if (mdvx.readVolume())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "  Cannot read in file" << endl;
    cerr << "  " << mdvx.getErrStr() << endl;

    return -1;
  }
  
  // Update the time fields
  
  Mdvx::master_header_t mhdr = mdvx.getMasterHeader();

  mhdr.data_collection_type = _dataCollectionType;
  
  mdvx.setMasterHeader(mhdr);
  
  if(_params->write_as_forecast) 
      mdvx.setWriteAsForecast();  

  return 0;

}
