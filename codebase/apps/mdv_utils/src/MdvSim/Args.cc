// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1990 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Boulder, Colorado, USA
// ** BSD license applies - redistribution and use in source and binary
// ** forms, with or without modification, are permitted provided that the
// ** copyright notice, conditions, and disclaimer are retained.
// ** THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY.
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
///////////////////////////////////////////////////////////////
//
// Args.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2026
//
///////////////////////////////////////////////////////////////

#include "Args.hh"
#include "Params.hh"

#include <toolsa/DateTime.hh>
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
    } else if (!strcmp(argv[ii], "-debug") || !strcmp(argv[ii], "-d")) {
      TDRP_add_override(&override, "debug = DEBUG_NORM;");
    } else if (!strcmp(argv[ii], "-verbose") || !strcmp(argv[ii], "-v")) {
      TDRP_add_override(&override, "debug = DEBUG_VERBOSE;");
    } else if (!strcmp(argv[ii], "-instance") && ii < argc - 1) {
      snprintf(tmp, sizeof(tmp), "instance = \"%s\";", argv[++ii]);
      TDRP_add_override(&override, tmp);
    } else if (!strcmp(argv[ii], "-inurl") && ii < argc - 1) {
      snprintf(tmp, sizeof(tmp), "input_url = \"%s\";", argv[++ii]);
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
    } else if (!strcmp(argv[ii], "-delay") && ii < argc - 1) {
      snprintf(tmp, sizeof(tmp), "delivery_delay_secs = %s;", argv[++ii]);
      TDRP_add_override(&override, tmp);
    } else if (!strcmp(argv[ii], "-forecast")) {
      TDRP_add_override(&override, "input_data_type = FORECAST;");
    } else if (!strcmp(argv[ii], "-non_forecast")) {
      TDRP_add_override(&override, "input_data_type = NON_FORECAST;");
    } else if (!strcmp(argv[ii], "-loop")) {
      TDRP_add_override(&override, "repeat_replay = TRUE;");
    } else if (!strcmp(argv[ii], "-once")) {
      TDRP_add_override(&override, "repeat_replay = FALSE;");
    } else if (!strcmp(argv[ii], "-start") && ii < argc - 1) {
      startTime = DateTime::parseDateTime(argv[++ii]);
      startTimeSet = startTime != DateTime::NEVER;
      TDRP_add_override(&override, "input_mode = ARCHIVE;");
      ok = ok && startTimeSet;
    } else if (!strcmp(argv[ii], "-end") && ii < argc - 1) {
      endTime = DateTime::parseDateTime(argv[++ii]);
      endTimeSet = endTime != DateTime::NEVER;
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
    } else if (argv[ii][0] == '-' && !tdrpIsArgValid(argv[ii])) {
      cerr << "ERROR - unrecognized command-line option: "
           << argv[ii] << endl;
      ok = false;
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
      << "  -inurl url               archive input URL\n"
      << "  -start \"yyyy mm dd hh mm ss\"\n"
      << "  -end   \"yyyy mm dd hh mm ss\"\n"
      << "  -forecast | -non_forecast\n"
      << "  -outdir path             local output directory\n"
      << "  -rate multiplier         replay speed\n"
      << "  -offset seconds          output data-time offset\n"
      << "  -delay seconds           publication delay\n"
      << "  -loop | -once            loop or stop after one cycle\n"
      << "  -debug | -d              normal debugging\n"
      << "  -verbose | -v            verbose debugging\n"
      << "  -h | -help | -man        this message\n\n";
  Params::usage(out);
}

