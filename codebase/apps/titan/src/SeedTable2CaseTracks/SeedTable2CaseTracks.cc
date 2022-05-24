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
// SeedTable2CaseTracks.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2009
//
///////////////////////////////////////////////////////////////
//
// SeedTable2CaseTracks reads a seed table file, captured from a
// web page, and reformats it into a case table.
//
///////////////////////////////////////////////////////////////

#include "SeedTable2CaseTracks.hh"
#include <cerrno>
#include <string>
#include <iostream>
#include <toolsa/DateTime.hh>
using namespace std;

// Constructor

SeedTable2CaseTracks::SeedTable2CaseTracks(int argc, char **argv) :
        _args("SeedTable2CaseTracks")
  
{
  
  OK = TRUE;

  // set programe name
  
  _progName = strdup("SeedTable2CaseTracks");

  // get command line args
  
  if (_args.parse(argc, (const char **) argv)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    OK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    OK = FALSE;
    return;
  }

  return;

}

// destructor

SeedTable2CaseTracks::~SeedTable2CaseTracks()

{

  
}

//////////////////////////////////////////////////
// Run

int SeedTable2CaseTracks::Run()
{

  if (_params.debug) {
    cerr << "input file name %s" << _params.input_file_name << endl;
  }

  // open input file

  FILE *in;
  if ((in = fopen(_params.input_file_name, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - SeedTable2CaseTracks::Run" << endl;
    cerr << "  Cannot open input file: " << _params.input_file_name << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  // headers


  fprintf(stdout, "# CASE TRACKS - QUEENSLAND 2008/2009 season - 35 DBZ threshold\n");
  fprintf(stdout, "#\n");
  fprintf(stdout, "#case   Seed?   N.    Seed   Decision time         track ref time       Ref-   End-  Compl Simpl Cloud Mixing  CCL    L.I.\n");
  fprintf(stdout, "#num    Y/N     of    Dur    YYYY MM DD HH MM SS   YYYY MM DD HH MM SS  start  ref   track track Base  Ratio   Temp   500mb\n");
  fprintf(stdout, "#               Flrs  (min)                                             (min)  (min) num   num   (km)  (g/kg)  (C)    (C)\n");
  fprintf(stdout, "#----   -----   ----  -----  -------------------   -------------------  -----  ----- ----- ----- ----- ------  ----   ------\n");
  fprintf(stdout, "\n");

  // read from stdin

  char line[4096];
  
  while (!feof(in)) {

    if (fgets(line, 4096, in) == NULL) {
      break;
    }

    int caseNum;
    int year, month, day;
    int startHour, startMin;
    char junk1[128];
    int endHour, endMin;
    char junk2[128];
    double lat, lon;
    char seedDecision[32];
    char seedType[32];
    char cloudType[128];
    int cloudBaseFt;
    char updraft[128];

    if (sscanf(line,
	       "%d %d-%d-%d %d:%d %s %d:%d %s %lg %lg %s %s %s %d %s",
	       &caseNum,
	       &year, &month, &day,
	       &startHour, &startMin,
	       junk1,
	       &endHour, &endMin,
	       junk2,
	       &lat, &lon,
	       seedDecision,
	       seedType,
	       cloudType,
	       &cloudBaseFt,
	       updraft) == 17) {

      DateTime startTime(year, month, day, startHour, startMin, 0);
      DateTime endTime(year, month, day, endHour, endMin, 0);
      int durationMins = (endTime.utime() - startTime.utime()) / 60;
      double cloudBaseKm = -9.99;
      if (cloudBaseFt > 0) {
	cloudBaseKm = (cloudBaseFt * 0.3048) / 1000.0;
      }
      int complexTrackNum = -1;
      int simpleTrackNum = -1;
      double mixingRatio = -9.99;
      double ccl = -9.99;
      double liftedIndex = -9.99;

      fprintf(stdout,
	      "%4.3d %5c %8.2d %5.2d"
	      " %7.4d %.2d %.2d %.2d %.2d %.2d"
	      " %5.4d %.2d %.2d %.2d %.2d %.2d"
	      " %4d %6d"
	      " %5d %5d %8.2f"
	      " %6.2f %6.2f %6.2f"
	      "\n",
	      caseNum,
	      toupper(seedDecision[0]),
	      0,
	      durationMins,
	      startTime.getYear(), startTime.getMonth(), startTime.getDay(),
	      startTime.getHour(), startTime.getMin(), startTime.getSec(),
	      startTime.getYear(), startTime.getMonth(), startTime.getDay(),
	      startTime.getHour(), startTime.getMin(), startTime.getSec(),
	      _params.mins_before_decision,
	      _params.mins_after_decision,
	      complexTrackNum,
	      simpleTrackNum,
	      cloudBaseKm,
	      mixingRatio,
	      ccl,
	      liftedIndex);
      fflush(stdout);

    } else {

      cerr << "ERROR - SeedTable2CaseTracks" << endl;
      cerr << "  Cannot parse line: " << line << endl;

    }
	       

  } // while

  fclose(in);

  return 0;

}

