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
////////////////////////////////////////////////////////////////////////
// SpdbCollate.cc
//
// SpdbCollate object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2002
//
///////////////////////////////////////////////////////////////////////
//
// SpdbCollate reads an input FMQ and copies the contents unchanged to an 
// output FMQ. It is useful for reading data from a remote queue and 
// copying it to a local queue. The clients can then read the local 
// queue rather than all access the remote queue.
//
///////////////////////////////////////////////////////////////////////

#include <iomanip>
#include <dataport/bigend.h>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include "SpdbCollate.hh"
using namespace std;

// Constructor

SpdbCollate::SpdbCollate(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "SpdbCollate";
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

  return;

}

// destructor

SpdbCollate::~SpdbCollate()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int SpdbCollate::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // compute times

  DateTime startTime(_params.start_time.year,
		     _params.start_time.month,
		     _params.start_time.day,
		     _params.start_time.hour,
		     _params.start_time.min,
		     _params.start_time.sec);
  
  DateTime endTime(_params.end_time.year,
		   _params.end_time.month,
		   _params.end_time.day,
		   _params.end_time.hour,
		   _params.end_time.min,
		   _params.end_time.sec);

  // write header info

  cout << "#year month day hour min sec";
  for (int i = 0; i < _params.data_sets_n; i++) {
    cout << " " << _params._data_sets[i].label;
  } // i
  cout << endl;

  // create DsSpdb object

  DsSpdb spdb;
  spdb.setAppName("SpdbCollate");

  for (time_t tt = startTime.utime(); tt <= endTime.utime(); 
       tt += _params.data_interval) {

    if (_params.debug) {
      cerr << "Processing time: " << DateTime::str(tt) << endl;
    }

    _printForTime(tt, spdb);

  } // tt
		 
  return 0;

}

//////////////////////////////////////////////////
// process data for the given time
//
// All data must be present to print

int SpdbCollate::_printForTime(time_t tt, DsSpdb &spdb1)

{

  if (_params.debug) {
    cerr << "Data set info:" << endl;
    for (int i = 0; i < _params.data_sets_n; i++) {
      cerr << "  url: " << _params._data_sets[i].url << endl;
      cerr << "  time_margin: " << _params._data_sets[i].time_margin << endl;
      cerr << "  data_type: " << _params._data_sets[i].data_type << endl;
      cerr << "  data_type2: " << _params._data_sets[i].data_type2 << endl;
      cerr << "  entry_type: ";
      switch (_params._data_sets[i].entry_type) {
      case Params::UI08:
	cerr << "ui08" << endl;
	break;
      case Params::SI08:
	cerr << "si08" << endl;
	break;
      case Params::UI16:
	cerr << "ui16" << endl;
	break;
      case Params::SI16:
	cerr << "si16" << endl;
	break;
      case Params::UI32:
	cerr << "ui32" << endl;
	break;
      case Params::SI32:
	cerr << "ui32" << endl;
	break;
      case Params::FL32:
	cerr << "fl32" << endl;
	break;
      case Params::STRING:
	cerr << "string" << endl;
	break;
      }

      cerr << "  offset_in_chunk: "
	   << _params._data_sets[i].offset_in_chunk << endl;
      cerr << "  label: " << _params._data_sets[i].label << endl;

    } // i
  }

  // first the time

  DateTime dtime(tt);
  char timeStr[128];
  sprintf(timeStr, "%.4d %.2d %.2d %.2d %.2d %.2d",
	  dtime.getYear(), dtime.getMonth(), dtime.getDay(),
	  dtime.getHour(), dtime.getMin(), dtime.getSec());

  // loop through data sets

  vector<string> dataText;
  bool foundSome = false;
  bool foundAll = true;

  for (int i = 0; i < _params.data_sets_n; i++) {

    // compute data types

    ui32 data_type;
    if (sscanf(_params._data_sets[i].data_type, "%d", &data_type) != 1) {
      data_type = Spdb::hash4CharsToInt32(_params._data_sets[i].data_type);
    }
    ui32 data_type2;
    if (sscanf(_params._data_sets[i].data_type2, "%d", &data_type2) != 1) {
      data_type = Spdb::hash4CharsToInt32(_params._data_sets[i].data_type2);
    }
    
    // get data

    DsSpdb spdb;

    if (spdb.getClosest(_params._data_sets[i].url,
			tt,
			_params._data_sets[i].time_margin,
			data_type,
			data_type2)) {
      dataText.push_back(_params._data_sets[i].missing_str);
      foundAll = false;
      continue;
    }

    // get the first chunk
    
    const vector<Spdb::chunk_t> &chunks = spdb.getChunks();
    if (chunks.size() < 1) {
      dataText.push_back(_params._data_sets[i].missing_str);
      foundAll = false;
      continue;
    }

    foundSome = true;

    void *ptr = (void *)
      ((char *) chunks[0].data + _params._data_sets[i].offset_in_chunk);

    char tmpStr[128];
    switch (_params._data_sets[i].entry_type) {

    case Params::UI08: {
      ui08 val;
      memcpy(&val, ptr, sizeof(val));
      sprintf(tmpStr, _params._data_sets[i].format, val);
    }
    break;

    case Params::SI08: {
      si08 val;
      memcpy(&val, ptr, sizeof(val));
      sprintf(tmpStr, _params._data_sets[i].format, val);
    }
    break;

    case Params::UI16: {
      ui16 val;
      memcpy(&val, ptr, sizeof(val));
      BE_from_array_16(&val, sizeof(val));
      sprintf(tmpStr, _params._data_sets[i].format, val);
    }
    break;

    case Params::SI16: {
      si16 val;
      memcpy(&val, ptr, sizeof(val));
      BE_from_array_16(&val, sizeof(val));
      sprintf(tmpStr, _params._data_sets[i].format, val);
    }
    break;

    case Params::UI32: {
      ui32 val;
      memcpy(&val, ptr, sizeof(val));
      BE_from_array_32(&val, sizeof(val));
      sprintf(tmpStr, _params._data_sets[i].format, val);
    }
    break;

    case Params::SI32: {
      si32 val;
      memcpy(&val, ptr, sizeof(val));
      BE_from_array_32(&val, sizeof(val));
      sprintf(tmpStr, _params._data_sets[i].format, val);
    }
    break;

    case Params::FL32: {
      fl32 val;
      memcpy(&val, ptr, sizeof(val));
      BE_from_array_32(&val, sizeof(val));
      sprintf(tmpStr, _params._data_sets[i].format, val);
    }
    break;

    case Params::STRING: {
      int maxLen = chunks[0].len - _params._data_sets[i].offset_in_chunk;
      char val[maxLen + 1];
      MEM_zero(val);
      memcpy(val, ptr, maxLen);
      sprintf(tmpStr, _params._data_sets[i].format, val);
    }
    break;

    } // switch
    
    dataText.push_back(tmpStr);

  } // i

  if (!foundSome) {
    return -1;
  }

  if (_params.print_only_if_all_available && !foundAll) {
    return -1;
  }

  // got data, so print

  cout << timeStr;
  for (size_t ii = 0; ii < dataText.size(); ii++) {
    cout << " " << dataText[ii];
  }
  cout << endl;

  return 0;

}


