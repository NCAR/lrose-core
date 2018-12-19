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
// Args.cc
//
// command line args
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2011
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include "Params.hh"
#include <cstring>
#include <cstdlib>
#include <toolsa/DateTime.hh>
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
  char tmp_str[4096];

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
      
    } else if (!strcmp(argv[i], "-verbose") ||
	       !strcmp(argv[i], "-v")) {

      sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-extra") ||
	       !strcmp(argv[i], "-vv")) {

      sprintf(tmp_str, "debug = DEBUG_EXTRA;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-extra")) {

      sprintf(tmp_str, "debug = DEBUG_EXTRA;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-url")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "input_url = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-data_type")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "data_type = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-data_type_2")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "data_type_2 = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-start")) {
      
      if (i < argc - 1) {
        string arg = argv[++i];
        time_t startTime = DateTime::parseDateTime(arg.c_str());
        if (startTime == DateTime::NEVER) {
          cerr << "ERROR on command line" << endl;
          cerr << "  Bad start time arg: " << arg << endl;
          iret = -1;
        }
        DateTime stime(startTime);
	sprintf(tmp_str, "start_time = { %.4d, %.2d, %.2d, %.2d, %.2d, %.2d };",
                stime.getYear(), stime.getMonth(), stime.getDay(),
                stime.getHour(), stime.getMin(), stime.getSec());
	TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }
        
    } else if (!strcmp(argv[i], "-end")) {
      
      if (i < argc - 1) {
        string arg = argv[++i];
        time_t endTime = DateTime::parseDateTime(arg.c_str());
        if (endTime == DateTime::NEVER) {
          cerr << "ERROR on command line" << endl;
          cerr << "  Bad end time arg: " << arg << endl;
          iret = -1;
        }
        DateTime etime(endTime);
	sprintf(tmp_str, "end_time = { %.4d, %.2d, %.2d, %.2d, %.2d, %.2d };",
                etime.getYear(), etime.getMonth(), etime.getDay(),
                etime.getHour(), etime.getMin(), etime.getSec());
	TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }
        
    } else if (!strcmp(argv[i], "-data_type")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "data_type = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-data_type2")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "data_type2 = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } // if
    
  } // i
  
  if (iret) {
    usage(prog_name, cerr);
  }

  return (iret);
    
}

void Args::usage(string &prog_name, ostream &out)
{

  out << "Usage: " << prog_name << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -data_type ?] specify the data type\n"
      << "         default is 0 - i.e. all data\n"
      << "       [ -data_type_2 ?] specify the data type 2\n"
      << "         default is 0 - i.e. all data\n"
      << "       [ -debug ] print debug messages\n"
      << "       [ -end \"yyyy mm dd hh mm ss\"] end time\n"
      << "       [ -start \"yyyy mm dd hh mm ss\"] start time\n"
      << "       [ -url ?] specify input url for SPDB data\n"
      << "       [ -v, -verbose ] print verbose debug messages\n"
      << "       [ -vv, -extra ]  print extra verbose debug messages\n"
      << endl;
  
  Params::usage(out);

}

