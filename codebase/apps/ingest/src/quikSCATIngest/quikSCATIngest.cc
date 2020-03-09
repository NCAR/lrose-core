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
 * quikSCATIngest: quikSCATIngest program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 2006
 *
 * Kay Levesque
 *
 *********************************************************************/

#include <iostream>
#include <string>

#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsInputDirTrigger.hh>
#include <Mdv/MdvxPjg.hh>
#include <Spdb/DsSpdb.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>

#include "quikSCATIngest.hh"
#include "Params.hh"

#include "AsciiReader.hh"

#include "Writer.hh"
#include "SpdbWriter.hh"
#include "GenPtWriter.hh"
#include "MdvWriter.hh"

using namespace std;


// Global variables

quikSCATIngest *quikSCATIngest::_instance =
     (quikSCATIngest *)NULL;



/*********************************************************************
 * Constructor
 */

quikSCATIngest::quikSCATIngest(int argc, char **argv) :
  _dataTrigger(0),
  _reader(0),
  _writer(0)
{
  static const string method_name = "quikSCATIngest::quikSCATIngest()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (quikSCATIngest *)NULL);
  
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

quikSCATIngest::~quikSCATIngest()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  delete _dataTrigger;
  
  delete _reader;
  delete _writer;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

quikSCATIngest *quikSCATIngest::Inst(int argc, char **argv)
{
  if (_instance == (quikSCATIngest *)NULL)
    new quikSCATIngest(argc, argv);
  
  return(_instance);
}

quikSCATIngest *quikSCATIngest::Inst()
{
  assert(_instance != (quikSCATIngest *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool quikSCATIngest::init()
{
  static const string method_name = "quikSCATIngest::init()";
  
  // Initialize the data trigger

  switch (_params->trigger_mode)
    {
    case Params::REALTIME :
      {
	if (_params->debug)
	  cerr << "Initializing REALTIME trigger using directory: " <<
	    _params->input_dir << endl;
    
	DsInputDirTrigger *trigger = new DsInputDirTrigger();
	if (trigger->init(_params->input_dir,
			  _params->input_substring,
			  _params->process_old_files,
			  PMU_auto_register) != 0)
	  {
	    cerr << "ERROR: " << method_name << endl;
	    cerr << "Error initializing REALTIME trigger using directory: " <<
	      _params->input_dir << endl;
	    
	    return false;
	  }

	_dataTrigger = trigger;
    
	if (_params->debug)
	  cerr << "Successfully initialized REALTIME trigger." << endl;
	
	break;
      }
  
    case Params::ARCHIVE :
      {
	const vector< string > file_list = _args->getInputFileList();
	
	if (_params->debug)
	  {
	    cerr << "Initializing ARCHIVE trigger with files: " << endl;
	    vector< string >::const_iterator file;
	    for (file = file_list.begin(); file != file_list.end(); ++file)
	      cerr << "    " << *file << endl;
	  }
    
	DsFileListTrigger *trigger = new DsFileListTrigger();
	
	if (trigger->init(file_list) != 0)
	  {
	    cerr << "ERROR: " << method_name << endl;
	    cerr << "Error initializing ARCHIVE trigger. " << endl;
	    cerr << "File list:" << endl;
	    vector< string >::const_iterator file;
	    for (file = file_list.begin(); file != file_list.end(); ++file)
	      cerr << "    " << *file << endl;
	    
	    return false;
	  }
	
	_dataTrigger = trigger;
    
	if (_params->debug)
	  cerr << "Successfully initialized ARCHIVE trigger." << endl;
	
	break;
      }
    
    } /* endswitch - _params->trigger_mode */
  
  // Initialize the ASCII reader object.

  _reader = new AsciiReader(_params->debug);
  
  // Initialize the writer object

  switch (_params->output_format)
    {
    case Params::SPDB_GEN_PT_FORMAT :
      {
	if (_params->debug)
	  cerr << "Instantiating a GenPtWriter." << endl;
	_writer = new GenPtWriter(_params->output_url,
				  _params->output_expire_secs,
				  _params->debug);
	break;
      }

    case Params::MDV_FORMAT :
      {
	if (_params->debug)
	  cerr << "Output format is set to MDV." << endl;
	MdvxPjg projection;
	switch(_params->xy_grid.proj_type)
	  {
	  case Params::PROJ_FLAT:
	    if (_params->debug)
	      cerr << "Initializing flat projection." << endl;
	    projection.initFlat(_params->xy_grid.origin_lat, 
				_params->xy_grid.origin_lon,
				_params->xy_grid.rotation,
				_params->xy_grid.nx,
				_params->xy_grid.ny,
				//				1.0,
				1,
				_params->xy_grid.dx,
				_params->xy_grid.dy,
				1.0,
				_params->xy_grid.minx,
				_params->xy_grid.miny,
				0.0);
	    break;
	  case Params::PROJ_LATLON:
	    if (_params->debug)
	      cerr << "Initializing lat/lon projection." << endl;
	    projection.initLatlon(_params->xy_grid.nx,
				  _params->xy_grid.ny,
				  1,
				  _params->xy_grid.dx,
				  _params->xy_grid.dy,
				  1.0,
				  _params->xy_grid.minx,
				  _params->xy_grid.miny,
				  0.0);
	    break;
	  }
	if (_params->debug)
	  cerr << "Instantiating an MdvWriter." << endl;
	_writer = new MdvWriter(_params->output_url,
				_params->output_expire_secs,
				projection,
				_params->debug);
	break;
      }
    default:
      cerr << "default" << endl;

    } /* endswitch - _params->output_format */

  
  // initialize process registration

  if (_params->trigger_mode == Params::REALTIME)
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);
  
  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void quikSCATIngest::run()
{
  static const string method_name = "quikSCATIngest::run()";
  
  TriggerInfo trigger_info;

  while (!_dataTrigger->endOfData())
  {
    if (_dataTrigger->next(trigger_info) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    if (!_processData(trigger_info.getFilePath()))
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
 * _processData() - Process the data for the given time.
 */

bool quikSCATIngest::_processData(const string &input_file_path)
{
  static const string method_name = "quikSCATIngest::_processData()";
  
  PMU_auto_register("Processing file...");
  
  if (_params->debug)
    cerr << endl
	 << "*** Processing file: " << input_file_path << endl;

  // Open the input file

  if (!_reader->openFile(input_file_path))
    return false;

  // Process all of the observations from the input file

  while (!_reader->endOfFile()) // until we reach the end of the file...
    {
      quikSCATObs * obs;
      if ((obs = _reader->getNextObs()) == 0) // process a line in the file
	{
	  delete obs;
	  continue;  // if the line isn't processed successfully, go to the next line
	}

      if (_params->debug)
	obs->print(cerr);
      if (!_writer->addInfo(*obs))
	{
	  delete obs;
	  return false;
	}
      delete obs;
    }

  if (!_writer->writeInfo())
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error writing file." << endl;
      return false;
    }
  cerr << "Finished processing file: " << input_file_path << endl;

  return true;
}
