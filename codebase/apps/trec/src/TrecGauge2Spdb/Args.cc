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
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 1997
//
//////////////////////////////////////////////////////////

#include <cstring>
#include <stdlib.h>
#include "Args.hh"
#include <toolsa/udatetime.h>
using namespace std;

// Constructor

Args::Args()

{
  TDRP_init_override(&override);
}

// Destructor

Args::~Args()

{
  TDRP_free_override(&override);
}

// Parse

int Args::parse (int argc, char **argv, const string &prog_name)

{

  char tmp_str[BUFSIZ];

  // intialize

  int iret = 0;
  startTime = -1;
  endTime = -1;

  // loop through args
  
  for (int i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      usage(prog_name, stdout);
      exit(0);
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-verbose")) {
      
      sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-log")) {
      
      sprintf(tmp_str, "debug = DEBUG_LOG;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-mode")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "mode = %s;", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-start")) {
      
      date_time_t start;
      if (i < argc - 1) {
	if (sscanf(argv[++i], "%d %d %d %d %d %d",
		   &start.year, &start.month, &start.day,
		   &start.hour, &start.min, &start.sec) != 6) {
	  iret = -1;
	} else {
	  uconvert_to_utime(&start);
	  startTime = start.unix_time;
	}
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-end")) {
      
      date_time_t end;
      if (i < argc - 1) {
	if (sscanf(argv[++i], "%d %d %d %d %d %d",
		   &end.year, &end.month, &end.day,
		   &end.hour, &end.min, &end.sec) != 6) {
	  iret = -1;
	} else {
	  uconvert_to_utime(&end);
	  endTime = end.unix_time;
	}
      } else {
	iret = -1;
      }
	
    } // if
    
  } // i

  if (iret) {
    usage(prog_name, stderr);
  }

  return iret;
    
}

void Args::usage(const string &prog_name, FILE *out)
{
  
  fprintf(out, "%s%s%s%s",
	  "Usage: ", prog_name.c_str(),
	  " [options as below] [-f file1 file2 file3...]\n",
	  "options:\n"
	  "       [ --, -h, -help, -man ] produce this list.\n"
	  "       [ -debug ] print debug messages\n"
	  "       [ -end \"yyyy mm dd hh mm ss\"] end time\n"
	  "                              ARCHIVE mode only\n"
	  "       [ -log ] set debug logging on\n"
	  "       [ -mode ?] ARCHIVE(Must use -f args) or REALTIME\n"
	  "       [ -params ?] params file path\n"
	  "       [ -start \"yyyy mm dd hh mm ss\"] start time\n"
	  "                                  ARCHIVE mode only\n"
	  "       [ -verbose ] print verbosedebug messages\n"
	  "\n");
  
  TDRP_usage(out);

}
