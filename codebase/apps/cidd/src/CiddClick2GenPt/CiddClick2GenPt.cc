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
// CiddClick2GenPt.cc
//
// CiddClick2GenPt object
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2012
//
///////////////////////////////////////////////////////////////

#include <rapformats/GenPt.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/ushmem.h>

#include "CiddClick2GenPt.hh"

using namespace std;



//////////////////////////////////////////////////
// Constructor

CiddClick2GenPt::CiddClick2GenPt(int argc, char **argv)
  
{
  isOK = true;
  
  // set programe name

  _progName = "CiddClick2GenPt";
  ucopyright((char *) _progName.c_str());
  
  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *)"unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  // Initialize the output SPDB database

  _spdb.setPutMode(Spdb::putModeAddUnique);

  // Attach to Cidd's shared memory segment
  
  if ((_coordShmem =
       (coord_export_t *) ushm_create(_params.coord_shmem_key,
				      sizeof(coord_export_t),
				      0666)) == NULL) {
    cerr << "ERROR: " << _progName << endl;
    cerr <<
      "  Cannot create/attach Cidd's coord export shmem segment." << endl;
    isOK = false;
  }
  
}


//////////////////////////////////////////////////
// destructor

CiddClick2GenPt::~CiddClick2GenPt()
  
{

  // detach from shared memory
  
  ushm_detach(_coordShmem);
  
  // unregister process
  
  PMU_auto_unregister();
}


//////////////////////////////////////////////////
// Run

void CiddClick2GenPt::Run()
{
  // register with procmap
  
  PMU_auto_register("Run");

  // Make sure the display is ready before using the shared memory
  
  while (!_coordShmem->shmem_ready)
  {
    PMU_auto_register("Waiting for shmem_ready flag");
    umsleep(_params.sleep_msecs);
  }
  
  // Store the latest click number at startup
  
  int last_seq_num = _coordShmem->pointer_seq_num - 1;
  
  // Now, operate forever
  
  while (true)
  {
    // Register with the process mapper
    
    PMU_auto_register("Checking for user input");
    
    // Check for new clicks
    
    if (_coordShmem->pointer_seq_num != last_seq_num)
    {
      _processClick(*_coordShmem);

      last_seq_num = _coordShmem->pointer_seq_num;

    } // if (_coordShmem->pointer_seq_num != last_seq_num)
    
    umsleep(_params.sleep_msecs);
    
  } // while(true)
}

//////////////////////////////////////////////////
// _processClick

void CiddClick2GenPt::_processClick(const coord_export_t click_info)
{
  static const string method_name = "CiddClick2GenPt::_processClick()";
  
  // Only process user clicks of the left mouse button
  
  if (click_info.button != 1 ||
      click_info.click_type != CIDD_USER_CLICK)
    return;
  
  // Print out the important click info

  if (_params.debug >= Params::DEBUG_NORM)
  {
    cerr << "Click - lat = " << click_info.pointer_lat
	 << ", lon = " << click_info.pointer_lon
	 << ", mouse button = " << (int)click_info.button << endl;
    cerr << "        time_data_start = "
	 << DateTime::str(click_info.time_data_start) << endl;
    cerr << "        time_data_end = "
	 << DateTime::str(click_info.time_data_end) << endl;
    cerr << "        time_cent = "
	 << DateTime::str(click_info.time_cent) << endl;
    cerr << "        time_min = "
	 << DateTime::str(click_info.time_min) << endl;
    cerr << "        time_max = "
	 << DateTime::str(click_info.time_max) << endl;
    cerr << "        click_type = " << click_info.click_type << endl;
  }

  // Create the GenPt object

  GenPt point;
  
  point.setName(_params.point_name);
  point.setTime(click_info.time_max);
  point.setLat(click_info.pointer_lat);
  point.setLon(click_info.pointer_lon);
  
  // Add a bogus field to the object since the code requires at least one
  // field

  point.addFieldInfo("bogus value", "none");
  point.addVal(0.0);
  
  if (_params.debug >= Params::DEBUG_VERBOSE)
    point.print(cerr);
  
  // Write the object to the database

  point.assemble();
  
  if (_spdb.put(_params.spdb_url,
		SPDB_GENERIC_POINT_ID,
		SPDB_GENERIC_POINT_LABEL,
		0,
		click_info.time_max,
		click_info.time_max + _params.expire_secs,
		point.getBufLen(),
		point.getBufPtr()) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing click to output database: "
	 << _params.spdb_url << endl;
    cerr << _spdb.getErrStr() << endl;
  }
  
}
