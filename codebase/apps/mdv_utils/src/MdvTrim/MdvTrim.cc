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
// $Id: MdvTrim.cc,v 1.2 2016/03/04 02:22:13 dixon Exp $
//
// MdvTrim
//
// Yan Chen, RAL, NCAR
//
// March 2008
//
///////////////////////////////////////////////////////////////
//
// MdvTrim is a very simple tool which extracts user specified
// fields from MDV files.
//
////////////////////////////////////////////////////////////////

#include <toolsa/pmu.h>
#include <Mdv/DsMdvxTimes.hh>
#include "MdvTrim.hh"

using namespace std;

////////////////////////////////////////////
// Constructor

MdvTrim::MdvTrim(int argc, char **argv)

{

  isOK = true;
  
  // set programe name

  _progName = "MdvTrim";
  ucopyright((char *) _progName.c_str());
  
  // get command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
  }

  // get TDRP params
  
  _paramsPath = "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  // parse the start and end time

  if (
    sscanf(
      _params.start_date_time, "%d %d %d %d %d %d",
      &_startTime.year, &_startTime.month, &_startTime.day,
      &_startTime.hour, &_startTime.min, &_startTime.sec
    ) != 6
  ) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot parse start_date_time: "
         << "\"" << _params.start_date_time << "\"" << endl;
    cerr << "Problem with TDRP parameters." << endl;
    isOK = false;
  } else {
    uconvert_to_utime(&_startTime);
  }

  if (
    sscanf(
      _params.end_date_time, "%d %d %d %d %d %d",
      &_endTime.year, &_endTime.month, &_endTime.day,
      &_endTime.hour, &_endTime.min, &_endTime.sec
    ) != 6
  ) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot parse start_date_time: "
         << "\"" << _params.start_date_time << "\"" << endl;
    cerr << "Problem with TDRP parameters." << endl;
    isOK = false;
  } else {
    uconvert_to_utime(&_endTime);
  }

  if (!isOK) {
    _args.usage(cerr);
    return;
  }

  // init process mapper registration
  
  PMU_auto_init(const_cast<char*>(_progName.c_str()), _params.instance, 
		PROCMAP_REGISTER_INTERVAL);
  
  return;

}

//////////////////////////
// destructor

MdvTrim::~MdvTrim()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int MdvTrim::Run ()
{

  PMU_auto_register("MdvTrim::Run");

  int ret = _performTrim();

  return (ret);
}

//////////////////////////////////////////////////
// trim

int MdvTrim::_performTrim() {

  if (_params.debug) {
    cerr << "Start: " << ctime(&(_startTime.unix_time));
    cerr << "End:   " << ctime(&(_endTime.unix_time));
  }

  DsMdvxTimes mdv_time_list;
  int iret = mdv_time_list.setArchive(
    _params.input_url_dir,
    _startTime.unix_time,
    _endTime.unix_time
  );
  if (iret) {
    cerr << "ERROR: MdvTrim::_performTrim()" << endl;
    cerr << "url: " << _params.input_url_dir << endl;
    cerr << mdv_time_list.getErrStr() << endl;
    return -1;
  }

  vector<time_t> _timelist = mdv_time_list.getArchiveList();
  int n_times = _timelist.size();
  if (n_times < 1)
    return -1;

  if (_params.debug) {
    cerr << "Total files: " << n_times << endl;
  }

  for (int file_index = 0; file_index < n_times; file_index++) {

    time_t mdv_time = _timelist.at(file_index);

    DsMdvx inMdvx;
    inMdvx.setReadEncodingType(Mdvx::ENCODING_ASIS);
    inMdvx.setReadCompressionType(Mdvx::COMPRESSION_ASIS);
    inMdvx.setReadTime(
      Mdvx::READ_CLOSEST,
      _params.input_url_dir,
      0,       // search margin
      mdv_time,// search time
      0        // forecast lead time
    );

    if (_params.field_names_n > 0) {
      for (int i = 0; i < _params.field_names_n; i++) {
        inMdvx.addReadField(_params._field_names[i]);
      }
    }

    // read volume for all fields needed
    if (inMdvx.readVolume()) {
      cerr << "ERROR: readVolume()" << endl;
      cerr << inMdvx.getErrStr() << endl;
      continue; // continue to next file
    }

    if (_params.debug) {
      cerr << "Read in file: " << inMdvx.getPathInUse() << endl;
    }

    // write out

    if (_params.debug) {
      cerr << "Writing out..." << endl;
    }
    inMdvx.setAppName(_progName);

    if (inMdvx.writeToDir(_params.output_url_dir)) {
      cerr << "ERROR: MdvPatch::_performPatch()" << endl;
      cerr << "  Cannot write file, url: "
           << _params.output_url_dir << endl;
      cerr << inMdvx.getErrStr() << endl;
      continue;
    }

  } // files

  return 0;
}

