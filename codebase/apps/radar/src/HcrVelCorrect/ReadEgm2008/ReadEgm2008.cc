/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1997
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1997/9/26 14:18:54
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
///////////////////////////////////////////////////////////////
// ReadEgm2008.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2019
//
///////////////////////////////////////////////////////////////
//
// ReadEgm2008 reads in Geoid corrections from Egm2008 files
//
////////////////////////////////////////////////////////////////

#include "ReadEgm2008.hh"
#include <string>
#include <cmath>
#include <malloc.h>
#include <iostream>
#include <dataport/port_types.h>
#include <dataport/bigend.h>
#include <toolsa/uusleep.h>
#include <toolsa/file_io.h>
#include <toolsa/DateTime.hh>
#include <didss/DsInputPath.hh>
#include <didss/DataFileNames.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/TaFile.hh>
#include <sys/stat.h>
#include <radar/Egm2008.hh>

using namespace std;

// Constructor

ReadEgm2008::ReadEgm2008(int argc, char **argv) :
        _args("ReadEgm2008")
  
{

  OK = TRUE;

  // set programe name

  _progName = strdup("ReadEgm2008");

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

ReadEgm2008::~ReadEgm2008()

{

  
}

//////////////////////////////////////////////////
// Run

int ReadEgm2008::Run()
{

  Egm2008 egm;
  if (egm.readGeoid(_params.egm_path)) {
    cerr << "ERROR - ReadEgm2008::Run()" << endl;
    cerr << "  Cannot read geoid file: " << _params.egm_path << endl;
    return -1;
  }

  // get geoid for lat/lon for point of interest

  cerr << "====>> lat, lon, geoidClosest, geoidInterp: " 
       << _params.lat << ", " 
       << _params.lon << ", " 
       << egm.getClosestGeoidM(_params.lat, _params.lon) << ", "
       << egm.getInterpGeoidM(_params.lat, _params.lon) << endl;

  return 0;

}

