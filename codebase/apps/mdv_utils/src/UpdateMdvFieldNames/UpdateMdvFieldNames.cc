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
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * UpdateMdvFieldNames.cc: UpdateMdvFieldNames program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cassert>
#include <signal.h>

#include <toolsa/os_config.h>
#include <didss/DsInputPath.hh>
#include <didss/LdataInfo.hh>
#include <Mdv/mdv/mdv_handle.h>
#include <Mdv/mdv/mdv_read.h>
#include <Mdv/mdv/mdv_write.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Params.hh"
#include "UpdateMdvFieldNames.hh"
using namespace std;

// Global variables

UpdateMdvFieldNames *UpdateMdvFieldNames::_instance = (UpdateMdvFieldNames *)NULL;

/*********************************************************************
 * Constructor
 */

UpdateMdvFieldNames::UpdateMdvFieldNames(int argc, char **argv)
{
  static const string method_name = "UpdateMdvFieldNames::UpdateMdvFieldNames()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (UpdateMdvFieldNames *)NULL);
  
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
    fprintf(stderr,
	    "ERROR: %s\n", method_name.c_str());
    fprintf(stderr,
	    "Problem with command line arguments.\n");
    
    okay = false;
    
    return;
  }
  
  // Get TDRP parameters.

  _params = new Params();
  char *params_path = (char *) "unknown";
  
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path))
  {
    fprintf(stderr,
	    "ERROR: %s\n", method_name.c_str());
    fprintf(stderr,
	    "Problem with TDRP parameters in file <%s>\n",
	    params_path);
    
    okay = false;
    
    return;
  }

  // Create the input path object

  fprintf(stderr,
	  "---> Creating input path object for directory %s\n", _params->input_dir);

  
  if (_args->getStartTime() <= 0 ||
      _args->getEndTime() <= 0)
  {
    fprintf(stderr, 
	    "**** Start and end time not specified on command line so running in REALTIME mode\n");
    _inputPath = new DsInputPath(_progName,
				 FALSE,
				 _params->input_dir,
				 -1,
				 PMU_auto_register);
  }
  else
  {
    _inputPath = new DsInputPath(_progName,
				 FALSE,
				 _params->input_dir,
				 _args->getStartTime(),
				 _args->getEndTime());
  }
  
  // Initialize procmap registration

  PMU_auto_init(_progName,
		_params->instance,
		PROCMAP_REGISTER_INTERVAL);
  
}


/*********************************************************************
 * Destructor
 */

UpdateMdvFieldNames::~UpdateMdvFieldNames()
{
  delete _inputPath;

  PMU_auto_unregister();
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

UpdateMdvFieldNames *UpdateMdvFieldNames::Inst(int argc, char **argv)
{
  if (_instance == (UpdateMdvFieldNames *)NULL)
    new UpdateMdvFieldNames(argc, argv);
  
  return(_instance);
}

UpdateMdvFieldNames *UpdateMdvFieldNames::Inst()
{
  assert(_instance != (UpdateMdvFieldNames *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * run()
 */

void UpdateMdvFieldNames::run()
{
  static const string method_name = "UpdateMdvFieldNames::run()";
  
  MDV_handle_t mdv_handle;
  LdataInfo ldata_info(_params->output_dir);
  ldata_info.setDataFileExt("mdv");
  
  // Initialize the MDV handle

  if (MDV_init_handle(&mdv_handle) != 0)
  {
    fprintf(stderr, "ERROR: %s\n", method_name.c_str());
    fprintf(stderr, "Error initializing MDV handle structure");
    
    okay = false;
    
    return;
  }
  
  // Process each of the files

  char *file_name;
  
  while ((file_name = _inputPath->next()) != (char *)NULL)
  {
    PMU_auto_register("Processing input file");
    
    fprintf(stderr,
	    "*** Processing input file %s\n", file_name);
    
    // Read in the file

    if (MDV_read_all(&mdv_handle,
		     file_name,
		     MDV_INT8) != 0)
    {
      fprintf(stderr, "ERROR: %s\n", method_name.c_str());
      fprintf(stderr, "Error reading MDV file %s -- SKIPPING\n", file_name);
      
      continue;
    }
    
    if ( mdv_handle.master_hdr.n_fields!= _params->NewFieldNames_n){
      fprintf(stderr,"%d fields in file, %d in list of new names.\n",
	       mdv_handle.master_hdr.n_fields, _params->NewFieldNames_n);
      fprintf(stderr,"Skipping file...\n");
      continue;
    }

    // Update the field names.
    
    for (int i = 0; i < mdv_handle.master_hdr.n_fields; i++)
    {
      sprintf(mdv_handle.fld_hdrs[i].field_name,
	      "%s", _params->_NewFieldNames[i]);
      sprintf(mdv_handle.fld_hdrs[i].field_name_long,
	      "%s", _params->_NewFieldNames[i]);
    } /* endfor - i */
    
    // Write out the new file

    char *output_file_name;
    
    if ((output_file_name = _constructFileName(_params->output_dir,
					       mdv_handle.master_hdr.time_centroid))
	== (char *)NULL)
    {
      fprintf(stderr, "ERROR: %s\n", method_name.c_str());
      fprintf(stderr, "Error constructing output file name for input file %s -- SKIPPING\n",
	      file_name);
      
      continue;
    }
    
    fprintf(stderr,
	    "--->   Writing the output to file %s\n", output_file_name);
    
    if (MDV_write_all(&mdv_handle,
		      output_file_name,
		      MDV_PLANE_RLE8) != MDV_SUCCESS)
    {
      fprintf(stderr, "ERROR: %s\n", method_name.c_str());
      fprintf(stderr, "Error writing output file %s\n", output_file_name);
      
      continue;
    }
    
    // Write out the latest_data_info file

    if (ldata_info.write(mdv_handle.master_hdr.time_centroid) != 0)
    {
      fprintf(stderr, "ERROR: %s\n", method_name.c_str());
      fprintf(stderr, "Error writing latest_data_info file to dir: %s\n",
	      _params->output_dir);
    }
    
  } /* endwhile - file_name */
  
  return;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _constructFileName() - Construct the file name.
 */

char *UpdateMdvFieldNames::_constructFileName(char *directory, time_t data_time)
{
  static char file_path[MAX_PATH_LEN];

  char dir_path[MAX_PATH_LEN];
  
  date_time_t time_struct;
  
  time_struct.unix_time = data_time;
  uconvert_from_utime(&time_struct);
  
  // Make sure the directory exists

  sprintf(dir_path, "%s%s%04d%02d%02d",
	  directory, PATH_DELIM,
	  time_struct.year, time_struct.month, time_struct.day);
  
  makedir(dir_path);
  
  // Now construct the file path

  sprintf(file_path, "%s%s%02d%02d%02d.mdv",
	  dir_path, PATH_DELIM,
	  time_struct.hour, time_struct.min, time_struct.sec);
  
  return file_path;
}
