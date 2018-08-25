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
// SyncDataMap.cc
//
// SyncDataMap object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2002
//
///////////////////////////////////////////////////////////////
//
// SyncDataMap synchronizes a local DataMapper with remote
// DataMappers on other hosts. It queries the remote DataMappers
// and writes their information to the local mapper.
// This is done on a polling basis at regular intervals.";
//
///////////////////////////////////////////////////////////////

#include <cstdlib>
#include <cstdio>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include "SyncDataMap.hh"
#include "Args.hh"
#include "Params.hh"
using namespace std;

// Constructor

SyncDataMap::SyncDataMap(int argc, char **argv)
  
{
  
  OK = TRUE;
  
  // set programe name
  
  _progName = "SyncDataMap";

  // parse command line args
  
  if (_args.parse(argc, argv, _progName)) {
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
    cerr << "Problem with TDRP parameters." << endl;
    OK = FALSE;
    return;
  }

  // initialize procmap registration
  
  PMU_auto_init(_progName.c_str(),
		_params.instance, PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

SyncDataMap::~SyncDataMap()

{

  // unregister with procmap

  PMU_auto_unregister();


}

//////////////////////////////////////////////////
// Run

int SyncDataMap::Run()
{

  DmapAccess accessIn, accessOut;

  while (true) {

    for (int ii = 0; ii < _params.remote_host_list_n; ii++) {

      const char* hostname = _params._remote_host_list[ii];

      if (_params.debug) {
	cerr << "Time: " << DateTime::str() << endl;
	cerr << "  Contacting host: " << hostname << endl;
      }

      string regInfo = "Contacting: ";
      regInfo += hostname;
      PMU_auto_register(regInfo.c_str());
      
      if (accessIn.reqAllInfo(hostname)) {

	cerr << "ERROR - SyncDataMap::Run" << endl;
	cerr << "  Cannot get DataMapper info from host: "
	     << hostname << endl;

      } else {

	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << "=====================================" << endl;
	  _printPlain(cerr, hostname, accessIn);
	}

	if (_params.debug) {
	  cerr << "  Registering with local DataMapper" << endl;
	}

	if (accessOut.regFullInfo(accessIn.getInfoArray())) {
	  cerr << "ERROR - SyncDataMap::Run" << endl;
	  cerr << "  Cannot register with local DataMapper" << endl;
	}

      }

    } // ii

    for (int ii = 0; ii < _params.polling_interval_secs; ii++) {
      PMU_auto_register("Zzzz");
      umsleep(1000);
    }
    
  } // while (true)

  return 0;
  
}

////////////////////////////////////////////////////////////
// _printPlain()

void SyncDataMap::_printPlain(ostream &out,
			      const char* hostname,
			      const DmapAccess &access)

{
  
  const vector<DMAP_info_t> &infoArray = access.getInfoArray();
  
  out << "****** host " << hostname << " ******" << endl;

  for (size_t ii = 0; ii < infoArray.size(); ii++) {

    const DMAP_info_t &info = infoArray[ii];
    
    out << "  "
	<< info.datatype << "   "
	<< info.dir << " "
	<< info.hostname << " "
	<< info.ipaddr << " "
	<< info.latest_time << " "
	<< info.forecast_lead_time << " "
	<< info.last_reg_time << " "
	<< info.start_time << " "
	<< info.end_time << " "
	<< info.nfiles << " "
	<< info.total_bytes << endl;
    
  } // ii

}


