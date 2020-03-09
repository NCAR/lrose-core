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

/* RCS info
 *   $Author: dixon $
 *   $Date: 2016/03/04 02:22:13 $
 *   $Revision: 1.7 $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Args.cc: class controlling the command line arguments for the
 *          program.
 *
 * RAP, NCAR, Boulder CO
 * Jason Craig
 * Sept 2006
 *
 *********************************************************************/

#include <iostream>
#include <string>

#include <string.h>

#include <toolsa/os_config.h>
#include <tdrp/tdrp.h>
#include <toolsa/DateTime.hh>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Args.hh"

using namespace std;

/**********************************************************************
 * Constructor
 */

Args::Args (int argc, char **argv, char *prog_name) :
  start_time(DateTime::NEVER),
  end_time(DateTime::NEVER),
  _progName(prog_name)
{
  string tmp_str;

  string interval_str;
  string mode;
  
  // Intialize

  bool okay = true;

  additional_tdrp_file = NULL;

  TDRP_init_override(&override);
  
  // search for command options
  
  for (int i =  1; i < argc; i++)
  {

    if (STRequal_exact(argv[i], "-additional_tdrp_file")){

      i++;
      if (i >= argc){
	fprintf(stderr,"-additional_tdrp_file needs an argument.\n");
	exit(-1);
      }

    additional_tdrp_file = argv[i];

   } 
   else if (STRequal_exact(argv[i], "--") ||
	STRequal_exact(argv[i], "-h") ||
	STRequal_exact(argv[i], "-help") ||
	STRequal_exact(argv[i], "--help") ||
	STRequal_exact(argv[i], "-man"))
    {
      _usage(stdout);
      exit(0);
    }
    else if (STRequal_exact(argv[i], "-debug"))
    {
      tmp_str = "debug = true;";
      TDRP_add_override(&override, tmp_str.c_str());
    }
    else if (STRequal_exact(argv[i], "-end") ||
	     STRequal_exact(argv[i], "-endtime"))
    {
      mode = "TIME_LIST";
      TDRP_add_override( &override, "trigger_mode = TIME_LIST;" );
      if (i < argc - 1)
      {
	if (end_time.set(argv[++i]) == DateTime::NEVER)
	{
	  cerr << "*** Invalid end time string entered: " <<
	    argv[i] << endl << endl;
	  
	  okay = false;
	}
      }
      else
      {
	okay = false;
      }
    }	
    else if (STRequal_exact(argv[i], "-file") || STRequal_exact(argv[i], "-f") || 
	     STRequal_exact(argv[i], "-if")) {

      if (i < argc - 1) {
        // load up file list vector. Break at next arg which
        // start with -
        for (int j = i + 1; j < argc; j++) {
          if (argv[j][0] == '-') {
            break;
          } else {
            _inputFileList.push_back(argv[j]);
          }
        }
	TDRP_add_override(&override, "trigger_mode = FILE_LIST;");
      } else {
	cerr << "Invalid input file list" << endl;
	
	okay = false;

      }
      
    }
    else if (STRequal_exact(argv[i], "-out_dir") ||
	     STRequal_exact(argv[i], "-o"))
    {
      if (i < argc - 1)
      {
	++i;
	tmp_str = "output_dir = " + string(argv[i]) + ";";
	TDRP_add_override(&override, tmp_str.c_str());
      }
      else
      {
	okay = false;
      }
    }
     else if (STRequal_exact(argv[i], "-start") ||
	     STRequal_exact(argv[i], "-starttime"))
    {
      mode = "TIME_LIST";
      TDRP_add_override( &override, "trigger_mode = TIME_LIST;" );
      if (i < argc - 1)
      {
	if (start_time.set(argv[++i]) == DateTime::NEVER)
	{
	  cerr << "*** Invalid start time string entered: " <<
	    argv[i] << endl << endl;
	  
	  okay = false;
	}
      }
      else
      {
	okay = false;
      }
    }	
    else if (STRequal_exact(argv[i], "-url"))
    {
      if (i < argc - 1)
      {
	tmp_str = "input_url = \"" + string(argv[++i]) + "\";";
	TDRP_add_override(&override, tmp_str.c_str());
      }
      else
      {
	okay = false;
      }
    }	
  } /* i */

  if (!okay)
  {
    _usage(stderr);
    exit(-1);
  }
    
  // If a mode was entered on the command line, make sure that
  // the other appropriate information was also entered.

  if (mode == "TIME_LIST")
  {
    if (start_time == DateTime::NEVER)
    {
      cerr <<
	"*** Must include -start in command line when using TIME_LIST mode" <<
	endl << endl;
      _usage(stderr);
      exit(-1);
    }
    
    if (end_time == DateTime::NEVER)
    {
      cerr <<
	"*** Must include -end in command line when using TIME_LIST mode" <<
	endl << endl;
      _usage(stderr);
      exit(-1);
    }
    
  }
  
}


/**********************************************************************
 * Destructor
 */

Args::~Args(void)
{
  TDRP_free_override(&override);
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _usage() - Print the usage for this program.
 */

void Args::_usage(FILE *stream)
{
  fprintf(stream, "%s%s%s",
	  "Usage:\n\n", _progName.c_str(), " [options] as below:\n\n"
	  "       [ --, -help, -man ] produce this list.\n"
	  "       [ -debug ] debugging on\n"
	  "       [ -file <files> ] list of input files (sets FILE_LIST mode)\n"
	  "       [ -end yyyy/mm/dd_hh:mm:ss ] end time (sets TIME_LIST mode)\n"
	  "       [ -out_dir dir] output directory\n"
	  "       [ -start yyyy/mm/dd_hh:mm:ss ] start time (sets TIME_LIST mode)\n"
	  "       [ -url url ] input url\n"
	  "       [ -additional_tdrp_file ] specify a TDRP file that will be loaded over the\n"
	  "                                 params file.\n"
	  "\n"
    );


  TDRP_usage(stream);
}






