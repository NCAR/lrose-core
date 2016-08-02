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
// Yan Chen, RAL, NCAR
//
// Dec.2007
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
      sprintf(tmp_str, "mode = ARCHIVE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-end")) {

      if (i < argc - 1) {
        sprintf(tmp_str, "end_date_time = \"%s\";", argv[++i]);
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      sprintf(tmp_str, "mode = ARCHIVE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-lt")) {

      if (i < argc - 1) {
        sprintf(tmp_str, argv[++i]);
        if (STRequal(tmp_str, "Y"))
          sprintf(tmp_str, "data_collection_type = YEARLY;");
        else if (STRequal(tmp_str, "D"))
          sprintf(tmp_str, "data_collection_type = DAYLY;");
        else if (STRequal(tmp_str, "H"))
          sprintf(tmp_str, "data_collection_type = HOURLY;");
        else if (STRequal(tmp_str, "A"))
          sprintf(tmp_str, "data_collection_type = ALL;");
        else
          iret = -1;
        if (!iret)
          TDRP_add_override(&override, tmp_str);

      } else {
        iret = -1;
      }
      sprintf(tmp_str, "mode = ARCHIVE;");
      TDRP_add_override(&override, tmp_str);

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

    } else if (!strcmp(argv[i], "-if")) {

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

  } // i

  if (iret) {
    usage(prog_name, cerr);
    return(iret);
  }

  size_t nFiles = inputFileList.size();
  if (nFiles > 0) {
    string nameStr = "input_files = { ";
    for (size_t i = 0; i < nFiles; i++) {
      nameStr += "\"";
      nameStr += inputFileList[i];
      nameStr += "\"";
      if (i != nFiles -1) {
        nameStr += ", ";
      } else {
        nameStr += " ";
      }
    }
    nameStr += "};";
    TDRP_add_override(&override, nameStr.c_str());
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
      << "  [ -mode ?] ARCHIVE or FILELIST\n"
      << "\n"
      << "  [ -if ? ?] input file list\n"
      << "    Set mode to FILELIST.\n"
      << "    Give a list of paths of input files.\n"
      << "\n"
      << "  [ -start \"YYYY MM DD HH MM SS\"] start time\n"
      << "\n"
      << "  [ -end \"YYYY MM DD HH MM SS\"] end time\n"
      << "\n"
      << "  [ -lt ?] move through files over the years, days, hours or all.\n"
      << "    ARCHIVE mode.\n"
      << "    Options:\n"
      << "      Y or y: move through files over the years,\n"
      << "              at the same month, day and hour.\n"
      << "      D or d: move through files over the days,\n"
      << "              at the same year, month and hour.\n"
      << "      H or h: move through files over the hours,\n"
      << "              at the same year, month and day.\n"
      << "      A or a: move through all the files available.\n"
      << "    Default: over the years.\n"
      << "\n"
      << "  [ -iurl ? ] specify input url directory.\n"
      << "\n"
      << "  [ -ourl ? ] specify output stats url directory.\n"
      << "\n"
      << endl << endl;

  Params::usage(out);

}
