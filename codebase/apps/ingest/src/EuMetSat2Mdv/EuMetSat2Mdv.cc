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
// EuMetSat2Mdv.cc
//
// EuMetSat2Mdv object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2000
//
///////////////////////////////////////////////////////////////


#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include <didss/DsInputPath.hh>
#include <map>
#include <cerrno>

#include "EuMetSat2Mdv.hh"
#include "Hrit.hh"

using namespace std;

// Constructor

EuMetSat2Mdv::EuMetSat2Mdv(int argc, char **argv)

{

  OK = true;
  _input = NULL;
  _channelSet = NULL;
  _fieldSet = NULL;

  // set programe name

  _progName = "EuMetSat2Mdv";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    fprintf(stderr, "ERROR: %s\n", _progName.c_str());
    fprintf(stderr, "Problem with command line args\n");
    OK = false;
    return;
  }
  
  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list,
			   &_paramsPath)) {
    fprintf(stderr, "ERROR: %s\n", _progName.c_str());
    fprintf(stderr, "Problem with TDRP parameters\n");
    OK = false;
    return;
  }

  if (_params.mode == Params::REALTIME) {
    PMU_auto_init((char *) _progName.c_str(), _params.instance,
		  PROCMAP_REGISTER_INTERVAL);
    PMU_auto_register("In EuMetSat2Mdv constructor");
  }

  // create the ChannelSet object

  _channelSet = new ChannelSet(_progName, _params);
  if (!_channelSet->OK()) {
    OK = false;
    return;
  }

  // create the FieldSet object

  _fieldSet = new FieldSet(_progName, _params, *_channelSet);
  if (!_fieldSet->OK()) {
    OK = false;
    return;
  }

  // initialize the data input object

  if (_params.mode == Params::REALTIME) {

    bool use_ldata_info = _params.use_latest_data_info_file;
    bool latest_file_only = true;
    _input = new DsInputPath((char *) _progName.c_str(),
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.realtime_input_dir,
			     _params.max_realtime_valid_age,
			     PMU_auto_register,
			     use_ldata_info,
			     latest_file_only);

  } else {

    // ARCHIVE mode


    // Create a vector of file paths which are ordered according to the
    // date and time. In the HRIT file names, the field name appears first
    // therefore a sorted list of files will go by field name and then by time.
    // We need a list ordered by time instead.
    
    vector<string> timeOrderedPaths;
    _loadTimeOrderedPaths(timeOrderedPaths);
    
    _input = new DsInputPath((char *) _progName.c_str(),
                             _params.debug >= Params::DEBUG_VERBOSE,
                             timeOrderedPaths, false);

  }

  if (strlen(_params.search_string) > 0) {
    _input->setSubString(_params.search_string);
  }

  // create file sets

  for (int ii = 0; ii < _params.output_files_n; ii++) {
    
    OutputFile *outputFile = new OutputFile(_progName,
                                            _params,
                                            _params._output_files[ii],
                                            *_channelSet,
                                            *_fieldSet);

    _outputFiles.push_back(outputFile);
    
    if (!outputFile->OK()) {
      OK = false;
    }

  } // ii

  return;

}

// destructor

EuMetSat2Mdv::~EuMetSat2Mdv()

{

  // free up

  if (_input) {
    delete _input;
  }

  if (_channelSet) {
    delete _channelSet;
  }

  if (_fieldSet) {
    delete _fieldSet;
  }

  for (int ii = 0; ii < (int) _outputFiles.size(); ii++) {
    delete _outputFiles[ii];
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int EuMetSat2Mdv::Run ()
{

  int iret = 0;
  
  PMU_auto_register("EuMetSat2Mdv::Run");
  
  // loop through available files
  
  char *filePath;
  while ((filePath = _input->next()) != NULL) {

    if (_processFile(filePath)) {
      iret = -1;
    }

    // after we get one file, set the latestFileOnly flag to false
    // so more than one file can be found in a single directory scan

    _input->setLatestFileOnly(false);
    
  }
  
  // write out any pending data

  for (int ii = 0; ii < (int) _outputFiles.size(); ii++) {
    _outputFiles[ii]->write();
  }

  return iret;
  
}

//////////////////////////////////////////////////
// Process the file

int EuMetSat2Mdv::_processFile (const char *file_path)

{

  if (_params.debug) {
    cerr << "Processing file: " << endl;
    cerr << "  " << file_path << endl;
  }

  // _uncompress(file_path);

  Hrit hrit(_progName, _params, *_channelSet);
  if (hrit.read(file_path)) {
    return -1;
  }

  // add the file to each file set
  // the set will reject it if it is the wrong channel

  for (int ii = 0; ii < (int) _outputFiles.size(); ii++) {
    _outputFiles[ii]->processFile(file_path, hrit);
  }

  return 0;

}

//////////////////////////////////////////////////
// Load a list of time-ordered paths.
//
// In the HRIT file names, the field name appears first
// therefore a sorted list of files will go by field name and
// then by time. We need a list ordered by time instead.

void EuMetSat2Mdv::_loadTimeOrderedPaths(vector<string> &timeOrderedPaths)

{

  // create a multimap of strings
  
  multimap<string, string, less<string> > orderedMap;

  // loop through paths from args

  for (int ii = 0; ii < (int) _args.filePaths.size(); ii++) {

    // get the last 15 characters in the path

    string path = _args.filePaths[ii];
    int len = path.size();
    string timePart = path.substr(len - 15);

    // add to map, using the time part as the key

    pair<string, string> pr(timePart, path);
    orderedMap.insert(pr);

  } // ii

  // load up vector of ordered paths

  timeOrderedPaths.clear();
  multimap<string, string, less<string> >::iterator ii;
  for (ii = orderedMap.begin(); ii != orderedMap.end(); ii++) {
    timeOrderedPaths.push_back(ii->second);
  }

}

//////////////////////////////////////////////////
// Load list of file times
//
// In the HRIT file names, the field name appears first
// therefore a sorted list of files will go by field name and
// then by time. We need a list ordered by time instead.

int EuMetSat2Mdv::_loadFileTimes(const vector<string> &filePaths,
                                 vector<time_t> &fileTimes)
  
{
  
  fileTimes.clear();

  // loop through paths from args
  
  for (int ii = 0; ii < (int) filePaths.size(); ii++) {
    
    // get the last 15 characters in the path

    string path = filePaths[ii];
    int len = path.size();
    string timePart = path.substr(len - 15);

    // compute time
    
    int year, month, day, hour, min;
    if (sscanf(timePart.c_str(), "%4d%2d%2d%2d%2d",
               &year, &month, &day, &hour, &min) != 5) {
      return -1;
    }
    
    DateTime dtime(year, month, day, hour, min, 0);
    fileTimes.push_back(dtime.utime());
    cerr << DateTime::strn(dtime.utime()) << endl;

  }

  return 0;

}
