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
///////////////////////////////////////////////////////////////
//
// Args.cc: class controlling the command line arguments
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2015
//
///////////////////////////////////////////////////////////////

#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Args.hh"
#include "Params.hh"

// Constructor

Args::Args()

{
}


// Destructor

Args::~Args()

{
  TDRP_free_override(&override);
}

// Parse command line
// Returns 0 on success, -1 on failure

int Args::parse (int argc, char **argv, const string &prog_name)
  
{

  int iret = 0;
  char tmp_str[BUFSIZ];

  // intialize
  
  _progName = prog_name;
  TDRP_init_override(&override);
  inputFileList.clear();
  startTime = 0;
  endTime = 0;

  // search for command options
  
  for (int i =  1; i < argc; i++) {

    if (STRequal_exact(argv[i], "--") ||
        STRequal_exact(argv[i], "-help") ||
        STRequal_exact(argv[i], "-h") ||
        STRequal_exact(argv[i], "-man")) {

      _usage(cout);
      exit(0);

    } else if (STRequal_exact(argv[i], "-d") ||
               STRequal_exact(argv[i], "-debug")) {
      
      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (STRequal_exact(argv[i], "-v") ||
               STRequal_exact(argv[i], "-verbose")) {
      
      sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (STRequal_exact(argv[i], "-vv") ||
               STRequal_exact(argv[i], "-extra")) {
      
      sprintf(tmp_str, "debug = DEBUG_EXTRA;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-i")) {
      if (i < argc - 1) {
        sprintf(tmp_str, "instance = \"%s\";", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-mode")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "mode = %s;", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }
        
    } else if (!strcmp(argv[i], "-in_dir")) {

      if(i < argc - 1) {
        sprintf(tmp_str, "input_dir = \"%s\";", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }

    } else if (!strcmp(argv[i], "-out_url")) {

      if(i < argc - 1) {
        sprintf(tmp_str, "output_url = \"%s\";", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      }
      else {
        iret = -1;
      }

    } else if (STRequal_exact(argv[i], "-print_summary")) {
      
      sprintf(tmp_str, "print_summary = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (STRequal_exact(argv[i], "-print_var_list")) {
      
      sprintf(tmp_str, "print_var_list = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (STRequal_exact(argv[i], "-print_sections")) {
      
      sprintf(tmp_str, "print_sections = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-start")) {
      
      if (i < argc - 1) {
	date_time_t start;
	if (sscanf(argv[++i], "%d %d %d %d %d %d",
		   &start.year, &start.month, &start.day,
		   &start.hour, &start.min, &start.sec) != 6) {
	  iret = -1;
	} else {
	  uconvert_to_utime(&start);
	  startTime = start.unix_time;
	  sprintf(tmp_str, "mode = ARCHIVE;");
	  TDRP_add_override(&override, tmp_str);
	}
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-end")) {
      
      if (i < argc - 1) {
	date_time_t end;
	if (sscanf(argv[++i], "%d %d %d %d %d %d",
		   &end.year, &end.month, &end.day,
		   &end.hour, &end.min, &end.sec) != 6) {
	  iret = -1;
	} else {
	  uconvert_to_utime(&end);
	  endTime = end.unix_time;
	  sprintf(tmp_str, "mode = ARCHIVE;");
	  TDRP_add_override(&override, tmp_str);
	}
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-files") ||
               !strcmp(argv[i], "-f")) {
      
      if (i < argc - 1) {
        // load up file list vector. Break at next arg which
        // start with -
        for (int j = i + 1; j < argc; j++) {
          if (argv[j][0] == '-') {
            break;
          } else {
            inputFileList.push_back(argv[j]);
          }
        }
        sprintf(tmp_str, "mode = FILELIST;");
        TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }
      
    } // if
    
  } /* i */

  if (iret) {
    _usage(cerr);
  }

  return iret;

}

// usage

void Args::_usage(ostream &out)
{
  
  out << "Usage: " << _progName << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -d, -debug ] print debug messages\n"
      << "       [ -end \"yyyy mm dd hh mm ss\"] end time\n"
      << "         ARCHIVE mode only\n"
      << "       [ -in_url url] Input URL\n"
      << "       [ -f ?, -files ?] input file list\n"
      << "         Sets mode to FILELIST\n"
      << "       [ -mode ?] ARCHIVE, ARCHIVE_FCST, REALTIME  or FILELIST\n"
      << "       [ -out_url url] Output URL\n"
      << "       [ -print_var_list ] print list of variables in the grib files\n"
      << "       [ -print_summary ] print a summary of fields in the grib files\n"
      << "       [ -print_sections ]print the sections in the grib files (can be large)\n"
      << "       [ -start \"yyyy mm dd hh mm ss\"] start time\n"
      << "         ARCHIVE mode only\n"
      << "       [ -v, -verbose ] print verbose debug messages\n"
      << "       [ -vv, -extra ] print extra verbose debug messages\n"
      << endl;

  Params::usage(out);

}

