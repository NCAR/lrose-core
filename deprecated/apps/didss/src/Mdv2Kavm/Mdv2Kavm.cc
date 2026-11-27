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
//   $Date: 2016/03/06 23:53:41 $
//   $Id: Mdv2Kavm.cc,v 1.4 2016/03/06 23:53:41 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Mdv2Kavm.cc: Mdv2Kavm program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2001
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cassert>
#include <iostream>
#include <signal.h>

#include <toolsa/os_config.h>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Mdv2Kavm.hh"
#include "MdvKavMosaic.hh"
#include "Params.hh"
using namespace std;


// Global variables

Mdv2Kavm *Mdv2Kavm::_instance =
     (Mdv2Kavm *)NULL;


/*********************************************************************
 * Constructor
 */

Mdv2Kavm::Mdv2Kavm(int argc, char **argv)
{
  static char *routine_name = "Constructor";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (Mdv2Kavm *)NULL);
  
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

  _args = new Args(argc, argv, _progName);
  
  // Get TDRP parameters.

  _params = new Params();
  char *params_path = (char *) "unknown";
  
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path))
  {
    fprintf(stderr,
	    "ERROR: %s::%s\n", _className(), routine_name);
    fprintf(stderr,
	    "Problem with TDRP parameters in file <%s>\n",
	    params_path);
    
    okay = false;
    
    return;
  }

  // Initialize the input data object

  switch (_params->mode)
  {
  case Params::REALTIME :
    if (_fileRetriever.setRealtime(_params->input_url,
                                   _params->max_valid_age,
                                   PMU_auto_register) != 0)
    {
      okay = false;
      return;
    }
    break;
    
  case Params::ARCHIVE :
    if (_fileRetriever.setArchive(_params->input_url,
                                  _args->startTime,
                                  _args->endTime) != 0)
    {
      okay = false;
      return;
    }
    break;

  } /* endswitch - _params->mode */

  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);
}


/*********************************************************************
 * Destructor
 */

Mdv2Kavm::~Mdv2Kavm()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

Mdv2Kavm *Mdv2Kavm::Inst(int argc, char **argv)
{
  if (_instance == (Mdv2Kavm *)NULL)
    new Mdv2Kavm(argc, argv);
  
  return(_instance);
}

Mdv2Kavm *Mdv2Kavm::Inst()
{
  assert(_instance != (Mdv2Kavm *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * run() - run the program.
 */

void Mdv2Kavm::run()
{
  const string method_name = "Mdv2Kavm::run()";
  
  // Create the input file object and set up the read

  DsMdvx input_file;
  
  input_file.clearReadFields();
  input_file.addReadField(_params->input_field_num);
  
  input_file.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  input_file.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  input_file.setReadScalingType(Mdvx::SCALING_NONE);
  
  // Reset the file retriever

  _fileRetriever.reset();
  
  // Process each input file

  while (!_fileRetriever.endOfData())
  {
    // Read the next input file

    if (_fileRetriever.readVolumeNext(input_file) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error reading next MDV file" << endl;
      cerr << "Data time: " << utimstr(_fileRetriever.getDataTime()) << endl;

      continue;
    }
    
    if (_params->debug)
      cerr << method_name << ": Processing file for time " <<
	utimstr(input_file.getMasterHeader().time_centroid) << endl;
      
    // Retrieve a pointer to the field we are converting

    MdvxField *field = input_file.getField(0);
    
    // Create the Kavouras file

    MdvKavMosaic mosaic;
    
    if (!mosaic.loadFile(input_file.getMasterHeader(),
			 *field,
			 _params->output_prefix,
			 _params->output_ext))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error converting MDV file to Kavouras format." << endl;
      cerr << "MDV file time: " <<
	utimstr(input_file.getMasterHeader().time_centroid) << endl;
      
      continue;
    }
    
    
    // Write the output file

    if (!mosaic.writeFile(_params->output_dir))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error writing output Kavouras file to dir: " <<
	_params->output_dir << endl;
      
      continue;
    }
    
    
  } /* endwhile - !_fileRetriever.endOfData() */
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

