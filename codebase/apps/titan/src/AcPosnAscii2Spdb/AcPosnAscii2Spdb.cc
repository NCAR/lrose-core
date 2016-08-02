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
// AcPosnAscii2Spdb.cc
//
// AcPosnAscii2Spdb object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 1999
//
///////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include "AcPosnAscii2Spdb.hh"
#include "Filter.hh"
using namespace std;

// Constructor

AcPosnAscii2Spdb::AcPosnAscii2Spdb(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "AcPosnAscii2Spdb";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  // check that start and end time is set

  if (_args.startTime == 0 || _args.endTime == 0) {
    cerr << "ERROR - must specify start and end dates." << endl << endl;
    _args.usage(_progName, cerr);
    isOK = false;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

AcPosnAscii2Spdb::~AcPosnAscii2Spdb()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int AcPosnAscii2Spdb::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // create filter object

  Filter filter(_progName, _params);
  
  if (!filter.isOK()) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Cannot create filter object." << endl;
    return (-1);
  }

  // loop through the days

  int startDay = _args.startTime / 86400;
  int endDay = _args.endTime / 86400;

  for (int iday = startDay; iday <= endDay; iday++) {

    date_time_t thisDay;
    MEM_zero(thisDay);
    thisDay.unix_time = iday * 86400;
    uconvert_from_utime(&thisDay);

    char inputFilePath[MAX_PATH_LEN];
    sprintf(inputFilePath, "%s%s%.4d%.2d%.2d",
	    _params.input_data_dir, PATH_DELIM,
	    thisDay.year, thisDay.month, thisDay.day);

    if (filter.process_file(inputFilePath)) {
      cerr << "WARNING: " << _progName << endl;
      cerr << "  Problem filtering file: " << inputFilePath << endl;
    }

  } // iday

  return (0);

}




