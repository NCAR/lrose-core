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
// Feb 2001
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include <cstdio>
#include <cstdlib>
#include <cstring>
using namespace std;

// Constructor

Args::Args()

{
}

// Destructor

Args::~Args()

{
}

////////////////////////////////////////////////////////////////
// parse the command line - returns 0 on success, -1 on failure

int Args::parse(int argc, char **argv, string &prog_name)

{

  // intialize

  int iret = 0;
  _progName = prog_name;
  printFull = false;
  printSummary = false;
  printCsvTable = false;
  printAsXml = false;
  printVerification = false;
  printLegacy = false;
  csvTableType = 1;
  minDuration = 0;
  trackNum = -1;
  dataChoice = not_set;

  // loop through args
  
  for (int i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      _usage(cout);
      exit(0);
      
    } else if (!strcmp(argv[i], "-full")) {
      
      printSummary = false;
      printFull = true;
      printCsvTable = false;
      
    } else if (!strcmp(argv[i], "-summary")) {
      
      printSummary = true;
      printFull = false;
      printCsvTable = false;
      
    } else if (!strcmp(argv[i], "-legacy")) {
      
      printLegacy = true;
      
    } else if (!strcmp(argv[i], "-storms")) {
      
      dataChoice = printStorms;
      
    } else if (!strcmp(argv[i], "-tracks")) {
      
      dataChoice = printTracks;
      
    } else if (!strcmp(argv[i], "-both")) {
      
      dataChoice = printBoth;
      
    } else if (!strcmp(argv[i], "-csv_table")) {
      
      printSummary = false;
      printFull = false;
      printCsvTable = true;
      
      if (i < argc - 1) {
	if (sscanf(argv[++i], "%d", &csvTableType) != 1) {
	  iret = -1;
	}
      } else {
	iret = -1;
      }

      if (csvTableType < 1 || csvTableType > 5) {
        cerr << "ERROR - bad CSV table type: " << csvTableType << endl;
        cerr << "  Must be 1 through 5" << endl;
        iret = -1;
      }

    } else if (!strcmp(argv[i], "-xml")) {
      
      printAsXml = true;

    } else if (!strcmp(argv[i], "-verify")) {
      
      printVerification = true;
      
    } else if (!strcmp(argv[i], "-md")) {
      
      if (i < argc - 1) {
	int md;
	if (sscanf(argv[++i], "%d", &md) != 1) {
	  iret = -1;
	} else {
	  minDuration = md;
	}
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-tn")) {
      
      if (i < argc - 1) {
	int tn;
	if (sscanf(argv[++i], "%d", &tn) != 1) {
	  iret = -1;
	} else {
	  trackNum = tn;
	}
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-path") ||
               !strcmp(argv[i], "-f")) {
      
      if (i < argc - 1) {
	path = argv[++i];
      } else {
	iret = -1;
      }
	
    } // if
      
  } // i

  // check if path is set

  if (path.size() == 0) {
    cerr << endl << "ERROR - you must specify path." << endl;
    iret = -1;
  }
  
  if (iret) {
    _usage(cerr);
    return (-1);
  }

  return (0);
    
}

void Args::_usage(ostream &out)
{

  out << "\n"
      << "Usage: " << _progName << " [args as below]\n"
      << "\n"
      << "Required:\n"
      << "\n"
      << "  [ -f, -path ? ] specify file path\n"
      << "\n"
      << "Options:\n"
      << "\n"
      << "  [ --, -h, -help, -man ] produce this list.\n"
      << "\n"
      << "  [ -full ] print full listing (as applicable).\n"
      << "\n"
      << "  [ -summary ] print summary listing (as applicable).\n"
      << "\n"
      << "  [ -csv_table ? ] print CSV table file for intercomparison study.\n"
      << "    specify table num (1 or 2)\n"
      << "\n"
      << "  [ -md ? ] min duration (secs). Print summary of tracks\n"
      << "     exceeding min duration (as applicable)\n"
      << "\n"
      << "  [ -tn ? ] print full listing for this track number\n"
      << "     (as applicable)\n"
      << "\n"
      << "  [ -verify ] force print of verify structs\n"
      << "\n"
      << "  [ -xml ] print as XML instead of normal ASCII.\n"
      << "\n"
      << "  [ -legacy ] print using legacy code.\n"
      << "    Default is to use new TitanData classes with new data, legacy with old data.\n"
      << "\n"
      << "  [ -storms ] print storms only.\n"
      << "    Default if printing .sh5 files.\n"
      << "\n"
      << "  [ -tracks ] print trackss only.\n"
      << "    Default if printing .th5 files.\n"
      << "\n"
      << "  [ -both ] print storms and tracks.\n"
      << "    Default if printing .nc files.\n"
      << "\n"
      << endl;

  out << _progName
      << " produces ASCII or XML output from TITAN binary files." << endl;
  out << "  Output goes to stdout." << endl << endl;

}







