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
// May 2005
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include <cstring>
#include <cstdlib>
using namespace std;

// constructor

Args::Args()

{
  debug = FALSE;
  verbose = FALSE;
  verbose2 = FALSE;
  realtime = TRUE;
  saveSecond = FALSE;
  inDir = "./input";
  outDir = "./output";
}

// destructor

Args::~Args()

{

}

// parse

int Args::parse(int argc, char **argv, string &prog_name)

{

  int iret = 0;

  // loop through args
  
  for (int i =  1; i < argc; i++) {
    
    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      usage(prog_name, cout);
      exit (0);
      
    } else if (!strcmp(argv[i], "-debug") || !strcmp(argv[i], "-d")) {

      debug = TRUE;

    } else if (!strcmp(argv[i], "-verbose") || !strcmp(argv[i], "-v")) {

      debug = TRUE;
      verbose = TRUE;

    } else if (!strcmp(argv[i], "-verbose2") || !strcmp(argv[i], "-v2")) {

      debug = TRUE;
      verbose = TRUE;
      verbose2 = TRUE;

    } else if (!strcmp(argv[i], "-f")) {
	
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
      } else {
	iret = -1;
      }
      realtime = FALSE;
      
    } else if (!strcmp(argv[i], "-indir")) {
      
      realtime = TRUE;
      if (i < argc - 1) {
	inDir = argv[++i];
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-outdir")) {
      
      if (i < argc - 1) {
	outDir = argv[++i];
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-second")) {

      saveSecond = TRUE;

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
      << "       [ -d, -debug ] print debug messages\n"
      << "       [ -f ? ?] specify input file list\n"
      << "         forces ARCHIVE mode\n"
      << "       [ -indir ?] input directory to watch\n"
      << "         forces REALTIME mode\n"
      << "       [ -outdir ] specify output dir, defaults to './output'\n"
      << "       [ -second ] save data from second geom found.\n"
      << "                   By default, first geom is saved.\n"
      << "       [ -v, -verbose ] print verbose debug messages\n"
      << "       [ -v2, -verbose2 ] print extra verbose debug messages\n"
      << endl;
  
  out << prog_name << " converts raw LIRP time-series IQ files to"
      << " netCDF format.\n" << endl;

  out << "The files are either specified on the command line with the\n"
      << "  -f arg, or the input directory is specified with the -indir\n"
      << "  arg. If -indir is specified, this directory is watched and\n"
      << "  when new files arrive they are converted.\n" << endl;

  out << "In ARCHIVE mode, you specify a list of files using the -f arg.\n\n"
      << "In REALTIME mode, you specify an input directory, and the program\n"
      << "  will watch that directory for new files and convert them as they\n"
      << "  arrive.\n" << endl;

  out << "The -second option is used to specify that you want to save out\n"
      << "  data from the SECOND beam geometry found.\n"
      << "  Some files contain data with differing PRT and nGates.\n"
      << "  By default, this app will discard the second geom found.\n"
      << "  Use -second to select the second geometry and discard the first.\n"
      << endl;

}
