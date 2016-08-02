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
 * Args.cc: class controlling the command line arguments for the
 *          program.
 *
 * RAP, NCAR, Boulder CO
 *
 *
 *********************************************************************/

#include <string.h>

#include <toolsa/os_config.h>
#include <tdrp/tdrp.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"
/*using namespace std;*/

/**********************************************************************
 * Constructor
 */

Args::Args (int argc, char **argv, char *prog_name)
{
  char tmp_str[BUFSIZ];

  // Intialize

  okay = true;

  TDRP_init_override(&override);
  
  // search for command options
  
  for (int i =  1; i < argc; i++)
  {
    if (STRequal_exact(argv[i], "--") ||
	STRequal_exact(argv[i], "-help") ||
	STRequal_exact(argv[i], "-h") ||
	STRequal_exact(argv[i], "-man"))
    {
      _usage(prog_name, stdout);
      exit(0);
    }
    else if (STRequal_exact(argv[i], "-debug"))
    {
      sprintf(tmp_str, "debug = 1;");
      TDRP_add_override(&override, tmp_str);
    }
    else if ( !strcmp(argv[i], "-start" )) {
      if (!( ++i <= argc )){
	cerr << "Date must follow -start argument." << endl;
	exit(-1);
      }
      startTime = DateTime::parseDateTime( argv[i] );
      if ( startTime == DateTime::NEVER ) {
	cerr << "Bad date/time syntax in -start specification." << endl;
	_usage(prog_name, stdout);
      }
      else {
	TDRP_add_override( &override, "mode = START_END;" );
      }
    }
    else if ( !strcmp(argv[i], "-end" )) {
      if (!( ++i < argc )){
	cerr << "Date must follow -end argument." << endl;
	exit(-1);
      }
      endTime = DateTime::parseDateTime( argv[i] );
      if ( endTime == DateTime::NEVER ) {
	cerr << "Bad date/time syntax in -end specification." << endl;
	_usage(prog_name, stdout);
      }
      else {
	TDRP_add_override( &override, "mode = START_END;" );
      }
    }
    else if ( !strcmp(argv[i], "-file" )) {
      
      //
      // search for next arg which starts with '-'
      //
      int j;
      for( j = i+1; j < argc; j++ ) {
	if (argv[j][0] == '-')
	  break;
	else
	  fileList.push_back( argv[j] );
      }
      TDRP_add_override( &override, "mode = FILE_LIST;" );
      
      if ( fileList.size() == 0 ) {
	_usage(prog_name, stdout);
      }
    }
   } /* i */

  if (!okay)
  {
    _usage(prog_name, stderr);
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

void Args::_usage(char *prog_name, FILE *stream)
{
  fprintf(stream, "%s%s%s",
	  "Usage:\n\n", prog_name, " [options] as below:\n\n"
	  "       [ --, -help, -h, -man ] produce this list.\n"
	  "       [ -debug ] debugging on\n"
	  "       [ -start \"YYYYMMDD HHMMSS\" ] Start time\n"
	  "       [ -end \"YYYYMMDD HHMMSS\" ] End time\n"
	  "       [ -file ] filelist\n"
	  "\n");


  TDRP_usage(stream);
}






