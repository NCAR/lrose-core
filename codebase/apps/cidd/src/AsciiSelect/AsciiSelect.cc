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
// AsciiSelect.cc
//
// AsciiSelect object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 1999
//
///////////////////////////////////////////////////////////////

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/ushmem.h>
#include <toolsa/pjg.h>
#include <toolsa/DateTime.hh>
#include <toolsa/MsgLog.hh>
#include "AsciiSelect.hh"
#include "StationLocate.hh"
using namespace std;

// Constructor

AsciiSelect::AsciiSelect(int argc, char **argv)
  
{
  
  isOK = true;

  // set programe name

  _progName = "AsciiSelect";
  ucopyright((char *) _progName.c_str());
  
  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = "unknown";
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

AsciiSelect::~AsciiSelect()

{

  // detach from shared memory

  ushm_detach(_coordShmem);
  
  // unregister process
  
  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int AsciiSelect::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // Make sure the display is ready before using the shared memory
  
  while (!_coordShmem->shmem_ready && !_params.no_wait_for_shmem) {
    PMU_auto_register("Waiting for shmem_ready flag");
    umsleep(_params.sleep_msecs);
  }
  
  // Store the latest click number at startup
  
  int last_no = _coordShmem->pointer_seq_num;
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
	  
	  if (_getData(data_time,
		       _coordShmem->pointer_lat,
		       _coordShmem->pointer_lon) ||
	      _spdb.getNChunks() == 0) {
	    _doPrintNoData();
	  } else if (_params.trim_for_window) {
	    _doPrintTrimmed();
	  } else {
	    _doPrint();
	  }
	  
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

	if (_getData(data_time,
		     _params.startup_location.lat,
		     _params.startup_location.lon) ||
	    _spdb.getNChunks() == 0) {
	  _doPrintNoData();
	} else if (_params.trim_for_window) {
	  _doPrintTrimmed();
	} else {
	  _doPrint();
	}
	
	last_display_time = curr_time;
	
      }
      
    } // if (_coordShmem->pointer_seq_num != last_no ... 
    
    umsleep(_params.sleep_msecs);
    
  } // while(forever)
  
  return (0);

}

////////////////////////////////////////////
// get the data
//
// Returns 0 on success, -1 on failure

int AsciiSelect::_getData(time_t data_time,
			  double click_lat,
			  double click_lon)
  
{

  // should we check write time?
  
  if (_params.use_cidd_time && _coordShmem &&
      _coordShmem->checkWriteTimeOnRead) {
    _spdb.setCheckWriteTimeOnGet(_coordShmem->latestValidWriteTime);
  } else {
    _spdb.clearCheckWriteTimeOnGet();
  }
  
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
      msgLog.postMsg("ERROR - AsciiSelect::_doPrint");
      msgLog.postMsg("  Retrieving data.");
      msgLog.postMsg("%s", _spdb.getErrStr().c_str());
    } else {
      cerr << "ERROR - AsciiSelect::_doPrint" << endl;
      cerr << "  Retrieving data." << endl;
      cerr << "  " << _spdb.getErrStr() << endl;
    }
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Retrieved " << _spdb.getNChunks() << " chunks from "
	 << _params.input_url << endl;
    cerr << "start_time: "
	 << DateTime::str(data_time - _params.retrieval_period) << endl;
    cerr << "end_time: "
	 << DateTime::str(data_time) << endl;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    Spdb::chunk_ref_t *refs = _spdb.getChunkRefs();
    for (int i = 0; i < _spdb.getNChunks(); i++) {
      cerr << "data_type: " << refs[i].data_type << endl;
      cerr << "  dehashed: "
	   << Spdb::dehashInt32To4Chars(refs[i].data_type) << endl;
    }
  }
  
  // chunks are returned in time order
  
  const vector<Spdb::chunk_t> &chunks = _spdb.getChunks();
  bool sort_by_location = false;
  StationLocate loc;
  if (_params.sort_by_location) {
    if (loc.load(_params.station_location_file) == 0) {
      sort_by_location = true;
    }
  }

  // load up a map of indices in reverse time order, using the
  // distance as a sorting measure if available
  
  double largeDist = 1e5;
  _distMap.clear();
  for (int ii = (int) chunks.size() - 1; ii >= 0; ii--){
    
    if (sort_by_location) {
      double Lat,Lon,Elev;
      string name(Spdb::dehashInt32To4Chars(chunks[ii].data_type));
      
      if (loc.getPos(name, Lat, Lon, Elev)) {
	distpair_t pp(largeDist, ii);
	_distMap.insert(pp);
      } else {
	double dist, dirn;
	PJGLatLon2RTheta(Lat,Lon,click_lat,click_lon,
			 &dist, &dirn);
	distpair_t pp(dist, ii);
	_distMap.insert(pp);
      }
    } else {
      distpair_t pp(largeDist, ii);
      _distMap.insert(pp);
    }
    
  } // i

  return 0;

}

//////////////////////////////////////
// print no data message

void AsciiSelect::_doPrintNoData()
  
{
  
  // scroll down
  
  for (int i = 0; i < _params.nlines_scroll_before_printing; i++) {
    cout << endl;
  }

  // Print no data message
  
  cout << _params.no_data_message << endl;
  cout << endl;
  
}

////////////////////////////////////
// normal print

void AsciiSelect::_doPrint()
  
{

  // scroll down

  for (int i = 0; i < _params.nlines_scroll_before_printing; i++) {
    cout << endl;
  }

  // print delimiter

  if (_params.length_delimiter_line > 0) {
    for (int i = 0; i < _params.length_delimiter_line; i++) {
      cout << "-";
    }
    cout << endl;
  }

  // print out the number of items required
  
  distmap_t::iterator ii;
  int nprint = 0;
  for (ii = _distMap.begin(); ii != _distMap.end(); ii++) {
    
    // get chunk

    const vector<Spdb::chunk_t> &chunks = _spdb.getChunks();
    int chunkIndex = (*ii).second;
    const Spdb::chunk_t &chunk = chunks[chunkIndex];
    if (chunk.len == 0) {
      continue;
    }

    // make sure string is null terminated
    char *text = (char *) chunk.data;
    text[chunk.len - 1] = '\0';

    // check number of items printed

    nprint++;
    if (nprint > _params.nmax_items_print) {
      break;
    }

    // print it out

    if (_params.valid_time_per_item) {
      cout << DateTime::str(chunk.valid_time) << endl;
    }

    cout << text;
    if (text[strlen(text)-1] != '\n') {
      cout << endl;
    }
    if (_params.newline_between_items) {
      cout << endl;
    }
    
  }

}

///////////////////////////////////////  
// print trimmed to fit in window
  
void AsciiSelect::_doPrintTrimmed()
  
{

  // scroll down
  
  for (int i = 0; i < _params.nlines_scroll_before_printing; i++) {
    cout << endl;
  }

  // print delimiter

  int nlines = 0;
  
  if (_params.length_delimiter_line > 0) {
    for (int i = 0; i < _params.length_delimiter_line; i++) {
      cout << "-";
    }
    cout << endl;
    nlines++;
  }

  // loop through the items
  
  distmap_t::iterator ii;
  for (ii = _distMap.begin(); ii != _distMap.end(); ii++) {
    
    // get chunk
    
    const vector<Spdb::chunk_t> &chunks = _spdb.getChunks();
    int chunkIndex = (*ii).second;
    const Spdb::chunk_t &chunk = chunks[chunkIndex];
    if (chunk.len == 0) {
      continue;
    }
    
    // make sure ascii text is null terminated

    char text[chunk.len];
    memcpy(text, chunk.data, chunk.len);
    text[chunk.len - 1] = '\0';
    
    // format the string

    int nlinesThis = 0;
    nlinesThis += _format(text, _params.window_width);
    if (text[strlen(text)-1] != '\n') {
      nlinesThis++;
    }
    if (_params.valid_time_per_item) {
      nlinesThis++;
    }
    if (_params.newline_between_items) {
      nlinesThis++;
    }
    
    // check output space
    
    if (nlines + nlinesThis > _params.window_height) {
      break;
    }
    nlines += nlinesThis;

    // print it out
    
    if (_params.valid_time_per_item) {
      cout << DateTime::str(chunk.valid_time) << endl;
    }
    
    cout << text;
    if (text[strlen(text)-1] != '\n') {
      cout << endl;
    }
    
    if (_params.newline_between_items) {
      cout << endl;
    }
    
  }

  // scroll text to stop of window

  for (int i = 0; i < _params.window_height - nlines - 1; i++) {
    cout << endl;
  }

}

///////////////////////////////////////////////////
// Format up the text, inserting extra \n as needed
//   to keep width to max_width.
// Replace \t with space
// Returns number of new lines
  
int AsciiSelect::_format(char *text, int max_width)
  
{

  //  cerr << "max_width: " << max_width << endl;

  int newLines = 0;
  int width = 0;
  int lastSpace = -1;
  for (size_t i = 0; i < strlen(text); i++) {

    width++;

    if (text[i] == '\t') {
      text[i] = ' ';
    }

    if (text[i] == ' ') {
      lastSpace = i;
    }

    if (text[i] == '\n') {

      width = 0;
      lastSpace = -1;
      newLines++;
      
    } else {

      if (width >= max_width) {
	// cerr << "Text: " << text << endl;
	// cerr << "width: " << width << endl;
	if (lastSpace >= 0) {
	  text[lastSpace] = '\n';
	  width = i - lastSpace;
	  newLines += ((width - 1) / max_width + 1);
	}
      }

    }

  } // i

  return newLines;

}


