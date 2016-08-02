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
// Qmap2Cidd.cc
//
// Qmap2Cidd object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 1999
//
///////////////////////////////////////////////////////////////
//
// Qmap2Cidd changes Queensland map to CIDD map format
//
///////////////////////////////////////////////////////////////

#include "Qmap2Cidd.hh"
#include <string>
#include <vector>
#include <iostream>
using namespace std;

// Constructor

Qmap2Cidd::Qmap2Cidd(int argc, char **argv) :
        _args("Qmap2Cidd")
  
{
  
  OK = TRUE;

  // set programe name
  
  _progName = strdup("Qmap2Cidd");

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

Qmap2Cidd::~Qmap2Cidd()

{

  
}

//////////////////////////////////////////////////
// Run

int Qmap2Cidd::Run()
{

  // read from stdin

  char line[1024];
  int mapId = 0;
  vector<double> lats, lons;
  
  fprintf(stdout, "MAP_NAME %s\n", _params.map_name);

  while (!feof(stdin)) {

    if (fgets(line, 1024, stdin) == NULL) {
      break;
    }

    if (strstr(line, ">") != NULL) {

      if (lats.size() > 0) {
        
        // write out existing polyline
        
        fprintf(stdout, "POLYLINE %d %d\n",
                mapId, (int) lats.size());
        
        for (int ii = 0; ii < (int) lats.size(); ii++) {
          fprintf(stdout, "%.6f %.6f\n", lats[ii], lons[ii]);
        }
        fprintf(stdout, "\n");

        // clear
        
        lats.clear();
        lons.clear();
        
      }
      
      // new polyline
      int id = 0;
      if (sscanf(line, "> %d", &id) == 1) {
        mapId = id;
      }

    } else {

      // read in lat/lon

      double lat, lon;
      if (sscanf(line, "%lg %lg\n", &lon, &lat) == 2) {
        lats.push_back(lat);
        lons.push_back(lon);
      }

    }

  } // while

  return 0;

}

