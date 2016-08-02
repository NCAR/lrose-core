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
//////////////////////////////////////////////////////////
//
// Mdv2MatlabArgs.cc : command line args
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 1998
//
//////////////////////////////////////////////////////////

#include <stdio.h>

#include <toolsa/os_config.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Mdv2MatlabArgs.h"

/**************************************************************
 * Constructor
 */

Mdv2MatlabArgs::Mdv2MatlabArgs(int argc, char **argv, char *prog_name)
{
  char tmp_str[BUFSIZ];

  // intialize

  okay = TRUE;
  done = FALSE;
  checkParams = FALSE;
  printParams = FALSE;
  paramsFilePath = (char *)NULL;
  numInputFiles = 0;
  inputFileList = (char **)NULL;
  mdvDataExpand = FALSE;

  mdvInt16 = false;
  makeOutputSubdir = true;
  
  TDRP_init_override(&override);

  // loop through args
  
  for (int i =  1; i < argc; i++)
  {
    if (STRequal_exact(argv[i], "--") ||
	STRequal_exact(argv[i], "-h") ||
	STRequal_exact(argv[i], "-help") ||
	STRequal_exact(argv[i], "-man"))
    {
      _usage(prog_name, stdout);
      done = TRUE;
    }
    else if (STRequal_exact(argv[i], "-check_params"))
    {
      checkParams = TRUE;
    }
    else if (STRequal_exact(argv[i], "-debug"))
    {
      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);
    }
    else if (!strcmp(argv[i], "-if"))
    {
      if (i < argc - 1)
      {
	int j;
	
	// Search for next arg which starts with '-'

	for (j = i + 1; j < argc; j++)
	  if (argv[j][0] == '-')
	    break;
	
	// Compute number of files

	numInputFiles = j - i - 1;

	// Set file name array

	inputFileList = argv + i + 1;
      }
      else
      {
	okay = TRUE;
      }
    }
    else if (STRequal_exact(argv[i], "-mdebug"))
    {
      if (i < argc - 1)
      {
	sprintf(tmp_str, "malloc_debug_level = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      }
      else
      {
	okay = FALSE;
      }
    }
    else if (STRequal_exact(argv[i], "-MDV_INT16"))
    {
      mdvInt16 = true;
    }
    else if (STRequal_exact(argv[i], "-no_subdir"))
    {
      makeOutputSubdir = false;
    }
    else if (STRequal_exact(argv[i], "-output_dir"))
    {
      outputDir = argv[i+1];
      sprintf(tmp_str, "output_dir = %s;", outputDir);
      TDRP_add_override(&override, tmp_str);
    }
    else if (STRequal_exact(argv[i], "-params"))
    {
      if (i < argc - 1)
      {
	paramsFilePath = argv[++i];
      }
      else
      {
	okay = FALSE;
      }
    } // if
    else if (STRequal_exact(argv[i], "-print_params"))
    {
      printParams = TRUE;
    }
    else if (STRequal_exact(argv[i], "-e") ||
	     STRequal_exact(argv[i], "-expand"))
    {
      mdvDataExpand = TRUE;
    }
  } // i

  if (!okay)
    _usage(prog_name, stderr);
    
}


/**************************************************************
 * _usage()
 */

void Mdv2MatlabArgs::_usage(char *prog_name, FILE *out)
{
  fprintf(out, "%s%s%s%s",
	  "Usage: ", prog_name, " [options as below]\n",
	  "options:\n"
	  "   [ --, -h, -help, -man ] produce this list.\n"
          "   [ -MDV_INT16 ]          read files in MDV_INT16 format\n"
          "                           (default is MDV_INT8)\n"
	  "   [ -check_params ]       check parameter usage\n"
	  "   [ -debug ]              print debug messages\n"
	  "   [ -if input_file_list ] (ARCHIVE mode only)\n"
	  "   [ -mdebug level ]       set malloc debug level\n"
	  "   [ -params path ]        params file path\n"
	  "   [ -print_params ]       print parameter usage\n"
	  "   [ -e, -expand ]         expand mdv data to doubles\n"
          "   [ -output_dir dir ]     override output_dir in param file\n"
          "   [ -no_subdir ]          do not create YYYYMMDD subdirectory\n"
	  "\n"
	  "\n"
          "Each .mdv file will be converted to a Matlab .mat file and placed in a\n"
          "subdirectory named according to its date.  The .mat file may be loaded\n"
          "within Matlab using the 'load' command.  This will create a struct\n"
          "called 'mdv' containing the entire contents of the mdv file in the\n"
          "following fields\n"
	  "\n"
          " mdv.src                                            : source file name\n"
          "    .hdr.(header fields)                            : master header\n"
          "    .fhdr.(field name).(field header fields)        : field headers\n"
          "    .vlevel.(field name).(vlevel fields)            : vlevel headers\n"
          "    .data.(field name)                              : field data\n"
          "    .chunk{(chunk number)}.hdr.(chunk header fields): chunk headers\n"
          "                          .data                     : chunk data\n"
	  "\n"
          "The following are exceptions: if no vlevels (resp. chunks) exist, then\n"
          "mdv.vlevel (resp. mdv.chunk) will be empty.  If a chunk is not of a\n"
          "supported type, thenmdv.chunk{(chunk number)}.data will be left empty.\n"
          "mdv.chunk{(chunk number)}.data will be left empty. Currently supported\n"
          "chunk types are MDV_CHUNK_VARIABLE_ELEV and MDV_CHUNK_TEXT. In conversion to\n"
          "doubles, bad or missing values are converted to NaNs. In field names,\n"
          "non-alphanumeric characters are converted to underscores ('_').\n"
	  "\n");
  
  fprintf(out, "\n");

}






