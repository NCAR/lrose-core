// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2026                                         
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
// Args.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2026
//
///////////////////////////////////////////////////////////////

#include "Args.hh"
#include "Params.hh"

#include <Radx/RadxTime.hh>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace std;

Args::Args() :
        startTimeSet(false),
        endTimeSet(false),
        startTime(0),
        endTime(0)
{
  TDRP_init_override(&override);
}

Args::~Args()
{
  TDRP_free_override(&override);
}

int Args::parse(int argc, char **argv, const string &progName)
{
  _progName = progName;
  bool ok = true;
  char tmp[BUFSIZ];

  for (int ii = 1; ii < argc; ++ii) {
    if (!strcmp(argv[ii], "--") || !strcmp(argv[ii], "-h") ||
        !strcmp(argv[ii], "-help") || !strcmp(argv[ii], "-man")) {
      _usage(cout);
      exit(0);
    } else if (!strcmp(argv[ii], "-debug")) {
      TDRP_add_override(&override, "debug = DEBUG_NORM;");
    } else if (!strcmp(argv[ii], "-verbose")) {
      TDRP_add_override(&override, "debug = DEBUG_VERBOSE;");
    } else if (!strcmp(argv[ii], "-instance") && ii < argc - 1) {
      snprintf(tmp, sizeof(tmp), "instance = \"%s\";", argv[++ii]);
      TDRP_add_override(&override, tmp);
    } else if (!strcmp(argv[ii], "-indir") && ii < argc - 1) {
      snprintf(tmp, sizeof(tmp), "input_dir = \"%s\";", argv[++ii]);
      TDRP_add_override(&override, tmp);
    } else if (!strcmp(argv[ii], "-outdir") && ii < argc - 1) {
      snprintf(tmp, sizeof(tmp), "output_dir = \"%s\";", argv[++ii]);
      TDRP_add_override(&override, tmp);
    } else if (!strcmp(argv[ii], "-rate") && ii < argc - 1) {
      snprintf(tmp, sizeof(tmp), "replay_rate = %s;", argv[++ii]);
      TDRP_add_override(&override, tmp);
    } else if (!strcmp(argv[ii], "-offset") && ii < argc - 1) {
      snprintf(tmp, sizeof(tmp), "time_offset_secs = %s;", argv[++ii]);
      TDRP_add_override(&override, tmp);
    } else if (!strcmp(argv[ii], "-loop")) {
      TDRP_add_override(&override, "repeat_replay = TRUE;");
    } else if (!strcmp(argv[ii], "-once")) {
      TDRP_add_override(&override, "repeat_replay = FALSE;");
    } else if (!strcmp(argv[ii], "-start") && ii < argc - 1) {
      startTime = RadxTime::parseDateTime(argv[++ii]);
      startTimeSet = startTime != RadxTime::NEVER;
      TDRP_add_override(&override, "input_mode = ARCHIVE;");
      ok = ok && startTimeSet;
    } else if (!strcmp(argv[ii], "-end") && ii < argc - 1) {
      endTime = RadxTime::parseDateTime(argv[++ii]);
      endTimeSet = endTime != RadxTime::NEVER;
      TDRP_add_override(&override, "input_mode = ARCHIVE;");
      ok = ok && endTimeSet;
    } else if (!strcmp(argv[ii], "-f")) {
      TDRP_add_override(&override, "input_mode = FILE_LIST;");
      int nPaths = 0;
      while (ii + 1 < argc && argv[ii + 1][0] != '-') {
        inputFileList.push_back(argv[++ii]);
        ++nPaths;
      }
      ok = ok && nPaths > 0;
    }
  }

  if (!ok) {
    _usage(cerr);
    return -1;
  }
  return 0;
}

void Args::_usage(ostream &out)
{
  out << "Usage: " << _progName << " [options]\n"
      << "  -f path [path ...]       replay an explicit file list\n"
      << "  -indir path              archive input directory\n"
      << "  -start \"yyyy mm dd hh mm ss\"\n"
      << "  -end   \"yyyy mm dd hh mm ss\"\n"
      << "  -outdir path             output directory\n"
      << "  -rate multiplier         replay speed\n"
      << "  -offset seconds          output data-time offset\n"
      << "  -loop | -once            loop or stop after one cycle\n"
      << "  -debug | -verbose        diagnostics\n"
      << "  -h | -help | -man        this message\n\n";
  Params::usage(out);
}
