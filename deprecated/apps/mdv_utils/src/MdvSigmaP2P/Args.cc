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
// $Id: Args.cc,v 1.2 2016/03/04 02:22:12 dixon Exp $
//
// Yan Chen, RAL, NCAR
//
// Sept.2008
//
//////////////////////////////////////////////////////////

#include <vector>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include "Args.hh"
#include "Params.hh"

using namespace std;

// constructor

Args::Args()
{
  TDRP_init_override(&override);
}

// destructor

Args::~Args()
{
  TDRP_free_override(&override);
}

// parse

int Args::parse(int argc, char **argv, string &prog_name)

{

  int iret = 0;
  char tmp_str[BUFSIZ];

  vector<string> inputFileList;

  // loop through args

  for (int i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {

      usage(prog_name, cout);
      exit (0);

    } else if (!strcmp(argv[i], "-debug")) {

      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-verbose")) {

      sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-start")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "start_date_time = \"%s\";", argv[++i]);
        TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }

    } else if (!strcmp(argv[i], "-end")) {

      if (i < argc - 1) {
        sprintf(tmp_str, "end_date_time = \"%s\";", argv[++i]);
        TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }

    } else if (!strcmp(argv[i], "-iurl")) {

      if (i < argc - 1) {
        sprintf(tmp_str, "input_url_dir = %s;", argv[++i]);
        TDRP_add_override(&override, tmp_str);

      } else {
        iret = -1;
      }

    } else if (!strcmp(argv[i], "-ourl")) {

      if (i < argc - 1) {
        sprintf(tmp_str, "stats_url_dir = %s;", argv[++i]);
        TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }

    } // if

  } // i

  if (iret) {
    usage(prog_name, cerr);
    return(iret);
  }

  return (iret);

}

void Args::usage(string &prog_name, ostream &out)
{

  out << "Usage: " << prog_name << " [options as below]\n"
      << "Options:\n"
      << "  [ --, -h, -help, -man ] produce this list.\n"
      << "\n"
      << "  [ -debug ] print debug messages\n"
      << "\n"
      << "  [ -verbose ] print verbose debug messages\n"
      << "\n"
      << "  [ -start \"YYYY MM DD HH MM SS\"] start time\n"
      << "\n"
      << "  [ -end \"YYYY MM DD HH MM SS\"] end time\n"
      << "\n"
      << "  [ -iurl ? ] specify input url directory.\n"
      << "\n"
      << "  [ -ourl ? ] specify output stats url directory.\n"
      << "\n"
      << endl << endl;

  Params::usage(out);

}
