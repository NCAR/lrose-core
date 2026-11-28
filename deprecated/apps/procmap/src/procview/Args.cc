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
// Args.cc : command line args
//////////////////////////////////////////////////////////

#include "Args.h"
#include <assert.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <tdrp/tdrp.h>

#include <stream.h>

#define INTERVAL_DEFAULT 10

// Define an instance to be available throughout the framework

Args *Args::_instance = 0;

// Instance methods for singleton class

Args *Args::Inst(int argc, char **argv)
{
  if (_instance == 0)
    _instance = new Args(argc, argv);
  return _instance;
}

Args *Args::Inst()
{
  assert(_instance);
  return (_instance);
}

// Constructor

Args::Args (int argc, char **argv)
{

  tdrp_override_t override;
  char tmp_str[BUFSIZ];

  // set program name
  
  path_parts_t progname_parts;
  uparse_path(argv[0], &progname_parts);
  progName = new char[strlen(progname_parts.base) + 1];
  strcpy(progName, progname_parts.base);
  ufree_parsed_path(&progname_parts);
  
  // initialize

  OK = TRUE;
  checkParams = FALSE;
  printParams = FALSE;
  printFull = FALSE;
  TDRP_init_override(&override);
  
  paramsFilePath = NULL;
  
  for (int i = 1; i < argc; i++) {
    
    if (STRequal(argv[i], "-h") || STRequal(argv[i], "-help")) {
      
      usage(stdout);
      exit(0);

    } else if (!strcmp(argv[i], "-check_params")) {
      
      checkParams = TRUE;
      
    } else if (STRequal(argv[i],"-debug")) {
      
      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (STRequal(argv[i],"-interval")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "procmapInterval = %s;", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
	OK = FALSE;
      }

    } else if (STRequal(argv[i],"-host")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "procmapHost = \"%s\";", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
	OK = FALSE;
      }

    } else if (STRequal(argv[i], "-params")) {

      if (i < argc - 1) {
        paramsFilePath = argv[i+1];
      } else {
        OK = FALSE;
      }

    } else if (!strcmp(argv[i], "-print_full")) {
      
      printFull = TRUE;
      
    } else if (!strcmp(argv[i], "-print_params")) {
      
      printParams = TRUE;
      
    } /* if (STRequal(argv[i] ... */

  } /* i */

  if (!OK) {
    usage(stderr);
    return;
  }
  
  // get TDRP parameters

  _table = procview_tdrp_init(&params);

  if (FALSE == TDRP_read(paramsFilePath, _table,
                         &params, override.list)) {
    fprintf(stderr, "ERROR - %s:Args\n", progName);
    fprintf(stderr, "Cannot read params file '%s'\n",
            paramsFilePath);
    OK = FALSE;
    return;
  }

  TDRP_free_override(&override);

  // procmapHost has its own pointer since it may be changed later

  procmapHost = params.procmapHost;
  
  if (checkParams) {
    TDRP_check_is_set(_table, &params);
    exit(0);
  }
  
  if (printParams) {
    TDRP_print_params(_table, &params, progName, FALSE);
    exit(0);
  }
  
  if (printFull) {
    TDRP_print_params(_table, &params, progName, TRUE);
    exit(0);
  }

  return;
    
}

// usage()

void Args::usage(FILE *out)
{

  fprintf(out,
	  "Usage: %s [options as below]\n"
	  "  [-h, -help] print usage\n"
          "  [-check_params] check parameter usage\n"
	  "  [-debug] activate debugging\n"
	  "  [-host ?] procmap host (def $PROCMAP_HOST)\n"
	  "  [-interval ?] procmap query interval (def %d)\n"
	  "  [-params ?] params file path\n"
          "  [-print_full] print parameters in full\n"
          "  [-print_params] print parameter summary\n",
	  progName, INTERVAL_DEFAULT);

  fprintf(out, "\n\n");

}

//
// Change procmapHost pointer to point to this static area
// which is loaded with the new host name
//

void Args::setHostCallback ( void *clientData, 
			     char *hostName )
{

  static char host[128];
  
  Args *args = (Args *) clientData;

  strcpy(host, hostName);
  args->procmapHost = host;

}

