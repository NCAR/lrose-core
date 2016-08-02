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
// MdvStats.cc
//
// MdvStats object
//
// Yan Chen, RAL, NCAR
//
// Dec. 2007
//
///////////////////////////////////////////////////////////////
//
// MdvStats computes the statistics values from a series of MDV files.
//
///////////////////////////////////////////////////////////////

#include <map>
#include <vector>

#include <Mdv/DsMdvxTimes.hh>
#include <toolsa/pmu.h>
#include "MdvStats.hh"
#include "ComputeMgr.hh"

using namespace std;

// Constructor

MdvStats::MdvStats(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "MdvStats";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
  }

  // get TDRP params
  
  _paramsPath = "unknown";
  if (
    _params.loadFromArgs(
      argc,
      argv,
      _args.override.list,
      &_paramsPath
    )
  ) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  // parse the start and end time

  if (
    sscanf(
      _params.start_date_time, "%d %d %d %d %d %d",
      &startTime.year, &startTime.month, &startTime.day,
      &startTime.hour, &startTime.min, &startTime.sec
    ) != 6
  ) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot parse start_date_time: "
         << "\"" << _params.start_date_time << "\"" << endl;
    cerr << "Problem with TDRP parameters." << endl;
    isOK = false;
  } else {
    uconvert_to_utime(&startTime);
  }

  if (
    sscanf(
      _params.end_date_time, "%d %d %d %d %d %d",
      &endTime.year, &endTime.month, &endTime.day,
      &endTime.hour, &endTime.min, &endTime.sec
    ) != 6
  ) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot parse start_date_time: "
         << "\"" << _params.start_date_time << "\"" << endl;
    cerr << "Problem with TDRP parameters." << endl;
    isOK = false;
  } else {
    uconvert_to_utime(&endTime);
  }

  // check if start and end time are set in ARCHIVE mode

  if (_params.mode == Params::ARCHIVE) {
    if (startTime.unix_time == 0 || endTime.unix_time == 0) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  In ARCHIVE mode, must specify start and end dates."
	   << endl << endl;
      isOK = false;
    }
  }

  // check input files in FILELIST mode

  if (_params.mode == Params::FILELIST) {
    if (_params.input_files_n < 1) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  Must specify input files if in FILELIST mode."
           << endl << endl;
      isOK = false;
    }
  }

  if (_params.fields_stats_n < 1) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Must specify at least one field." << endl << endl;
    isOK = false;
  }

  // check duplicated fields

  map<string, bool> fieldsMap;
  fieldsMap.insert(make_pair(_params._fields_stats[0].field_name, TRUE));
  for (int i = 1; i < _params.fields_stats_n; i++) {
    map<string, bool>::iterator iter = fieldsMap.find(
      _params._fields_stats[i].field_name
    );
    if (iter != fieldsMap.end()) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  Duplicated field names defined in parameter file." << endl;
      isOK = false;
      break;
    }
    fieldsMap.insert(make_pair(_params._fields_stats[i].field_name, TRUE));
  }

  // check covariance fields

  for (int i = 0; i < _params.covariance_fields_n; i++) {
    if (
      strcmp(
        _params._covariance_fields[i].field_name1,
        _params._covariance_fields[i].field_name2
      ) == 0
    ) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  Duplicated field names defined in covariance parameters."
           << endl << endl;
      isOK = false;
      break;
    }
  }

  if (!isOK) {
    _args.usage(_progName, cerr);
    return;
  }

  // init process mapper registration

  PMU_auto_init(
    (char *) _progName.c_str(),
    _params.instance,
    PROCMAP_REGISTER_INTERVAL
  );

  return;

}

// destructor

MdvStats::~MdvStats()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int MdvStats::Run ()
{

  int iret = 0;


  // register with procmap
  
  PMU_auto_register("Run");

  if (_params.mode == Params::ARCHIVE) {
    if (_generateTimeList()) {
      cerr << "ERROR - MdvStats::Run" << endl;
      cerr << "  Cannot setup input files for url: "
           << _params.input_url_dir << endl;
      return -1;
    }

    // process this input
    for (int i = 0; i < vvTimeList.size(); i++) {
      vector<time_t> oneSet = vvTimeList.at(i);
      iret = _processArchive(oneSet);
    }

  } else if (_params.mode == Params::FILELIST) {

    iret = _processFileList();

  }

  return iret;

}

//////////////////////////////////////////////////
// process the input files

int MdvStats::_processArchive(vector<time_t>& inputTimeList) {

  // register with procmap
  
  PMU_auto_register("_processArchive");

  // create the compute manager object

  ComputeMgr computeMgr(_params);
  
  // compute the statistics and write output

  if (computeMgr.computeStatistics(inputTimeList)) {
    cerr << "ERROR - MdvStats::_processArchive" << endl;
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// process the input files

int MdvStats::_processFileList() {

  // register with procmap
  
  PMU_auto_register("_processFileList");

  // create the compute manager object

  ComputeMgr computeMgr(_params);
  
  // compute the statistics and write output

  if (computeMgr.computeStatistics()) {
    cerr << "ERROR - MdvStats::_processFileList" << endl;
    return -1;
  }


  return 0;

}

//////////////////////////////////////////////////
// generate the input file paths

int MdvStats::_generateTimeList() {

  vector<time_t> oneSet;

  char tmpBuffer[256];
  int iret = 0;

  if (_params.debug == Params::DEBUG_VERBOSE) {
    cerr << "start: " << utimstr(startTime.unix_time) << endl;
    cerr << "end: " << utimstr(endTime.unix_time) << endl;
  }

  /*** Loop for YEAR ****/
  if(_params.data_collection_type == Params::YEARLY) {

    for(int m = startTime.month; m <= endTime.month; m++) {

      for(int d = startTime.day; d <= endTime.day; d++) {

        for(int h = startTime.hour; h <= endTime.hour;h++) {

          for(int y = startTime.year; y <= endTime.year; y++) {
            DateTime dt(y, m, d, h, 0, 0);
            time_t tt = dt.utime();
            oneSet.push_back(tt);
          }//year

          vvTimeList.push_back(oneSet);
          oneSet.clear(); //clear data

        } //hour
      } //day
    } //month
  } // if


  /***  Loop For Day ***/
  if(_params.data_collection_type == Params::DAYLY) {

    for(int y = startTime.year; y <= endTime.year; y++) {

      for(int m = startTime.month; m <= endTime.month; m++) {

        for(int h = startTime.hour;h <= endTime.hour;h++) {

          for(int d = startTime.day; d <= endTime.day; d++) {
            DateTime dt(y, m, d, h, 0, 0);
            time_t tt = dt.utime();
            oneSet.push_back(tt);
          }//day

          vvTimeList.push_back(oneSet);
          oneSet.clear(); //clear data

        } //hour
      } //month
    } //year
  } // if


  /*** Loop For Hour ****/
  if(_params.data_collection_type == Params::HOURLY) {

    for(int y = startTime.year; y <= endTime.year; y++) {

      for(int m = startTime.month; m <= endTime.month; m++) {

        for(int d = startTime.day; d <= endTime.day; d++) {

          for(int h = startTime.hour;h <= endTime.hour;h++) {
            DateTime dt(y, m, d, h, 0, 0);
            time_t tt = dt.utime();
            oneSet.push_back(tt);
          }//hour

          vvTimeList.push_back(oneSet);
          oneSet.clear(); //clear data

        } //day
      } //month
    } //year
  } // if


  /*** Loop for All ***/
  if (_params.data_collection_type == Params::ALL) {
    DsMdvxTimes mdv_times;
    iret = mdv_times.setArchive(
      _params.input_url_dir,
      startTime.unix_time,
      endTime.unix_time
    );
    if (iret) {
      cerr << "ERROR: MdvStats::_generateTimeList()" << endl;
      cerr << "url: " << _params.input_url_dir << endl;
      cerr << mdv_times.getErrStr() << endl;
      return -1;
    }

    const vector<time_t> timelist = mdv_times.getArchiveList();
    if (timelist.size() == 0) {
      cerr << "ERROR: MdvStats::_generateTimeList()" << endl;
      cerr << "No files found in url: " << _params.input_url_dir << endl;
      return -1;
    }
    vvTimeList.push_back(timelist);
  }

  return(iret);
}

