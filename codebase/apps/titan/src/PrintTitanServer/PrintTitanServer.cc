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
// PrintTitanServer.cc
//
// PrintTitanServer object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2001
//
///////////////////////////////////////////////////////////////
//
// PrintTitanServer produces ASCII output from TITAN server.
// Output goes to stdout.
//
///////////////////////////////////////////////////////////////

#include "PrintTitanServer.hh"
#include "Args.hh"
#include <toolsa/file_io.h>
#include <toolsa/str.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include <titan/DsTitan.hh>
#include <cerrno>
using namespace std;

// Constructor

PrintTitanServer::PrintTitanServer(int argc, char **argv)
  
{
  
  // initialize

  OK = true;
  
  // set programe name
  
  _progName = "PrintTitanServer";

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Problem with command line args" << endl;
    OK = false;
    return;
  }

  return;

}

// destructor

PrintTitanServer::~PrintTitanServer()

{

}

//////////////////////////////////////////////////
// Run

int PrintTitanServer::Run()
{

  // set up the DsTitan server object

  DsTitan titan;
  if (_args.debug) {
    titan.setDebug(true);
  }

  switch (_args.readTimeMode) {
  case TITAN_SERVER_READ_LATEST_TIME:
    titan.setReadLatestTime();
    break;
  case TITAN_SERVER_READ_LATEST:
    titan.setReadLatest();
    break;
  case TITAN_SERVER_READ_CLOSEST:
    titan.setReadClosest(_args.requestTime, _args.readTimeMargin);
    break;
  case TITAN_SERVER_READ_FIRST_BEFORE:
    titan.setReadFirstBefore(_args.requestTime, _args.readTimeMargin);
    break;
  case TITAN_SERVER_READ_NEXT_SCAN:
    titan.setReadNext(_args.requestTime, _args.readTimeMargin);
    break;
  case TITAN_SERVER_READ_PREV_SCAN:
    titan.setReadPrev(_args.requestTime, _args.readTimeMargin);
    break;
  case TITAN_SERVER_READ_INTERVAL:
    titan.setReadInterval(_args.startTime, _args.endTime);
    break;
    break;
  } // switch

  switch (_args.trackSet) {
  case TITAN_SERVER_ALL_AT_TIME:
    titan.setReadAllAtTime();
    break;
  case TITAN_SERVER_ALL_IN_FILE:
    titan.setReadAllInFile();
    break;
  case TITAN_SERVER_SINGLE_TRACK:
    titan.setReadSingleTrack(_args.requestComplexNum);
    break;
  case TITAN_SERVER_CURRENT_ENTRIES:
    titan.setReadCurrentEntries();
    break;
  } // switch
  
  if (_args.readLprops) {
    titan.setReadLprops();
  }
  if (_args.readDbzHist) {
    titan.setReadDbzHist();
  }
  if (_args.readRuns) {
    titan.setReadRuns();
  }
  if (_args.readProjRuns) {
    titan.setReadProjRuns();
  }
  if (_args.readCompressed) {
    titan.setReadCompressed();
  }

  if (_args.debug) {
    titan.printReadRequest(cerr);
  }

  // do the read

  if (titan.read(_args.url)) {
    cerr << "ERROR - PrintTitanServer::Run()" << endl;
    cerr << titan.getErrStr() << endl;
    return -1;
  }

  if (_args.printAsXml) {

    cout << titan.convertToXML();
    
  } else {
    
    // print out
    
    cout << "dirInUse: "
         << titan.getDirInUse() << endl;
    cout << "timeInUse: "
         << DateTime::str(titan.getTimeInUse()) << endl;
    cout << endl;
    
    if (_args.readTimeMode != TITAN_SERVER_READ_LATEST_TIME) {
      
      cout << "scanInUse: "
           << titan.getScanInUse() << endl;
      cout << "idayInUse: "
           << DateTime::str(titan.getIdayInUse() * SECS_IN_DAY) << endl;
      
      cout << "stormPathInUse: " << titan.getStormPathInUse() << endl;
      cout << "trackPathInUse: " << titan.getTrackPathInUse() << endl;
      cout << endl;
      
    }
    
    titan.print(stdout);

  }
    
  return 0;

}

