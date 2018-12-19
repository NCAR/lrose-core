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
// EdrSelect.cc
//
// EdrSelect object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 1999
//
///////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/ushmem.h>
#include <toolsa/pjg.h>
#include <toolsa/DateTime.hh>
#include <toolsa/MsgLog.hh>
#include <rapmath/math_macros.h>
#include "EdrSelect.hh"
using namespace std;

// Constructor

EdrSelect::EdrSelect(int argc, char **argv)
  
{
  
  isOK = true;
  
  // set programe name

  _progName = "EdrSelect";
  ucopyright((char *) _progName.c_str());
  
  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
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
  
  return;

}

// destructor

EdrSelect::~EdrSelect()
  
{

  // detach from shared memory
  
  ushm_detach(_coordShmem);
  
  // unregister process
  
  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int EdrSelect::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // Make sure the display is ready before using the shared memory
  
  while (!_coordShmem->shmem_ready && !_params.no_wait_for_shmem) {
    PMU_auto_register("Waiting for shmem_ready flag");
    umsleep(_params.sleep_msecs);
  }
  
  // Store the latest click number at startup
  
  int last_no = _coordShmem->pointer_seq_num - 1;
  time_t last_display_time = 0;
  
  // Now, operate forever
  
  while (true) {
    
    // Register with the process mapper
    
    PMU_auto_register("Checking for user input");
    
    // Check for new clicks
    
    time_t curr_time = time(NULL);
    
    if (_coordShmem->pointer_seq_num != last_no ||
	(_params.auto_click_interval > 0 &&
	 abs(curr_time - last_display_time) > _params.auto_click_interval)) {
      
      if (_coordShmem->pointer_seq_num != last_no) {
	
	if (_params.mouse_button == 0 ||
	    _coordShmem->button == _params.mouse_button) {
	  
	  if (_params.debug) {
        
	    fprintf(stderr,
		    "Click - lat = %g, lon = %g, mouse button = %d\n",
		    _coordShmem->pointer_lat,
		    _coordShmem->pointer_lon,
		    (int) _coordShmem->button);
	  }
	  
	  time_t data_time;

	  if(_params.use_cidd_time) {
	    data_time = _coordShmem->time_cent;
	  } else {
	    data_time = curr_time;
	  }
	  
	  _doPrint(data_time,
		   _coordShmem->pointer_lat,
		   _coordShmem->pointer_lon);
	  
	  last_display_time = curr_time;

	} else {

	  if (_params.debug) {
	    fprintf(stderr, "   Not processing clicks for mouse button %ld\n",
		    _coordShmem->button);
	  }

	}
      
	last_no = _coordShmem->pointer_seq_num;
	
      } else {

	time_t data_time;

	if(_params.use_cidd_time) {
	  data_time = (time_t) _coordShmem->time_cent;
	} else {
	  data_time = curr_time;
	}

	_doPrint(data_time,
		 _params.startup_location.lat,
		 _params.startup_location.lon);
	
	last_display_time = curr_time;
	
      }
      
    } // if (_coordShmem->pointer_seq_num != last_no ... 
    
    umsleep(_params.sleep_msecs);
    
  } // while(forever)
  
  return (0);

}

void EdrSelect::_doPrint(time_t data_time,
			   double click_lat,
			   double click_lon)
  
{
  
  cerr <<"************************" << endl;
  cerr <<"   click_lat " << click_lat << endl;
  cerr <<"   click_lon " << click_lon << endl;
  cerr <<"************************" << endl;
  // Request the data from the spdb database
  
  if (_spdb.getInterval(_params.input_url,
			data_time - _params.retrieval_period,
			data_time, 0)) {
    if (_params.log_spdb_errors) {
      MsgLog msgLog;
      msgLog.setApplication(_progName, _params.instance);
      msgLog.setSuffix("log");
      msgLog.setAppendMode();
      msgLog.setDayMode();
      msgLog.setOutputDir(_params.errors_log_dir);
      msgLog.postMsg("ERROR - EdrSelect::_doPrint");
      msgLog.postMsg("  Retrieving data.");
      msgLog.postMsg("%s", _spdb.getErrStr().c_str());
    } else {
      cerr << "ERROR - EdrSelect::_doPrint" << endl;
      cerr << "  Retrieving data." << endl;
      cerr << "  " << _spdb.getErrStr() << endl;
    }
  }
  
  if (_params.debug) {
    cerr << "Retrieved " << _spdb.getNChunks() << " chunks from "
	 << _params.input_url << endl;
    cerr << "start_time: "
	 << DateTime::str(data_time - _params.retrieval_period) << endl;
    cerr << "end_time: "
	 << DateTime::str(data_time) << endl;
  }

  //  if (_params.debug >= Params::DEBUG_VERBOSE) {
//      Spdb::chunk_ref_t *refs = _spdb.getChunkRefs();
//      for (int i = 0; i < _spdb.getNChunks(); i++) {
//        cerr << "data_type: " << refs[i].data_type << endl;
//        cerr << "data_type2: " << refs[i].data_type2 << endl;
//        string str1, str2;
//        str1 = Spdb::dehashInt32To4Chars(refs[i].data_type);
//        str2 = Spdb::dehashInt32To4Chars(refs[i].data_type2);
//        string tailnum = str1 + str2;
//        cerr << "  dehashed(tailnum): "<<  tailnum << endl;
//      }
//    }
  
  //
  // chunks are returned in time order
  //
  int nChunks = _spdb.getNChunks();
  const vector<Spdb::chunk_t> &chunks = _spdb.getChunks();
  vector <Edr*>    edrVec;

  //
  // Disassemble chunks. Store in soundings vector.
  //
  for( int i = 0; i < nChunks; i++ ) 
    {
      
      Edr *edrPtr = new Edr();
      edrPtr->disassemble( chunks[i].data, chunks[i].len );
      Edr::edr_t rep = edrPtr->getRep();
      if (rep.lat != Edr::VALUE_UNKNOWN && rep.lon != Edr::VALUE_UNKNOWN ) 
	edrVec.push_back(edrPtr);
    }
  

  // load up a map of indices in reverse time order, using the
  // distance as a sorting measure if available
  
  double largeDist = 1e5;
  distmap_t distmap;
  
  vector < Edr *> :: const_iterator j;
  for (int ii = edrVec.size() -1; ii>= 0; ii--)
    {
      Edr::edr_t edrRep = edrVec[ii]->getRep();
      if (_params.sort_by_location) 
	{
	  double Lat, Lon, Elev;
	  
	  Lat = edrRep.lat;
	  Lon = edrRep.lon;
	  double dist, dirn;
	  PJGLatLon2RTheta(Lat,Lon,click_lat,click_lon,
			   &dist, &dirn);
	  distpair_t pp(dist, ii);
	  distmap.insert(pp); 
	} 
      else 
	{ 
	  distpair_t pp(largeDist, ii);
	  distmap.insert(pp);
	}
    }
  
 
  // print out the number required

  distmap_t::iterator ii;
  int nprint = 0;
  for (ii = distmap.begin(); ii != distmap.end(); ii++) {

    // check number of items printed

    nprint++;
    if (nprint > _params.nmax_items_print) {
      break;
    }

    edrVec[(*ii).second]->print(cerr);
  }

  // Print no data message, if appropriate.

  if (_spdb.getNChunks() == 0){
    cout << _params.no_data_message << endl;
  }

}

