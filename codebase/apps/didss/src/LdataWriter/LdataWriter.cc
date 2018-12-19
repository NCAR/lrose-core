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
// LdataWriter.cc
//
// LdataWriter object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2000
//
///////////////////////////////////////////////////////////////////////
//
// LdataWriter allows the user to write a latest_data_info file
// using values specified on the command line.
//
///////////////////////////////////////////////////////////////////////

#include <toolsa/umisc.h>
#include <toolsa/DateTime.hh>
#include <toolsa/Path.hh>
#include <dsserver/DsLdataInfo.hh>
#include <didss/DataFileNames.hh>
#include "LdataWriter.hh"
using namespace std;

// Constructor

LdataWriter::LdataWriter(int argc, char **argv)

{

  isOK = true;
  _input = NULL;

  // set programe name

  _progName = "LdataWriter";

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  if (_args.refresh) {
    if (_args.inputFileList.size() > 0) {
      // FILELIST mode
      _input = new DsInputPath(_progName,
                               _args.verbose,
                               _args.inputFileList);
    } else if (_args.startTime != 0 && _args.endTime != 0) {
      // archive mode - start time to end time
      _input = new DsInputPath(_progName,
                               _args.verbose,
                               _args.dir,
                               _args.startTime, _args.endTime);
    } else {
      // single time refresh mode
      _input = NULL;
    }
  }
  
  return;

}

// destructor

LdataWriter::~LdataWriter()

{

  if (_input != NULL) {
    delete _input;
  }

}

//////////////////////////////////////////////////
// Run

int LdataWriter::Run ()
{

  // refresh mode?
  // this just rewrites the latest data info exactly as it is
  // side effect: file mod times are updated

  if (_args.refresh) {
    if (_input == NULL) {
      return _refreshLatestTime();
    } else {
      return _refreshSpecifiedTimes();
    }
  }
  
  // max time mode?
  // only register with DataMapper if latest time exceeds max time

  if (_args.maxDataTime) {
    return _runMaxTimeMode();
  }

  // normal mode

  return _runNormal();

}

//////////////////////////////////////////////////
// run normally

int LdataWriter::_runNormal()
{
  
  // create Ldata file object
  
  string url = _args.url;
  if(url.size() == 0) {
    url = _args.dir;
  }

  DsLdataInfo ldata(url, _args.verbose);

  // set object from command line args

  _setFromArgs(ldata);

  // write the file

  int iret = 0;
  if (ldata.write(_args.latestTime, _args.dataType)) {
    cerr << "ERROR -  LdataWriter::_runNormal" << endl;
    cerr << "  Cannot write ldata file to dir: " << _args.dir << endl;
    iret = -1;
  } else {
    if (_args.debug) {
      cerr << "Wrote ldata to " << _args.dir
           << ", time: " << DateTime::str(_args.latestTime) << endl;
    }
  }

  return iret;

}

///////////////////////////////////////////////////////////
// max data time option
// only register if latest time exceeds max previous time

int LdataWriter::_runMaxTimeMode()
{
  
  // read in the information from
  // any existing latest data info file
  
  string url = _args.url;
  if(url.size() == 0) {
    url = _args.dir;
  }
  
  DsLdataInfo ldata(url, _args.verbose);
  if(ldata.readForced()) {
    // no previous data
    if (_args.debug) {
      cerr << "DEBUG - LdataWriter::_runMaxTimeMode()" << endl;
      cerr << "  No previous latest data file" << endl;
    }
    return _runNormal();
  }
  
  if (_args.debug) {
    cerr << "DEBUG - LdataWriter::_runMaxTimeMode()" << endl;
    cerr << "  max time: " << DateTime::strm(ldata.getMaxTime()) << endl;
  }
  
  // set object from command line args

  _setFromArgs(ldata);

  // Set flag to indicate that we will not register
  // with the DataMapper if the latest time is earlier than
  // the max time - i.e. the latest time has moved backwards.
 
  ldata.setNoRegIfLatestTimeInPast();

  // write the file

  int iret = 0;
  if (ldata.write(_args.latestTime, _args.dataType)) {
    cerr << "ERROR -  LdataWriter::_runMaxTimeMode" << endl;
    cerr << "  Cannot write ldata file to dir: " << _args.dir << endl;
    iret = -1;
  } else {
    if (_args.debug) {
      cerr << "Writing to " << _args.dir
           << ", time: " << DateTime::str(_args.latestTime) << endl;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// refresh option
// single latest time

int LdataWriter::_refreshLatestTime()
{

  // create Ldata file object
  
  DsLdataInfo ldata(_args.dir, _args.debug);
  
  if (ldata.read() == 0) {
    if (ldata.write()) {
      cerr << "ERROR - LdataWriter::_refreshLatestTime" << endl;
      cerr << "  Cannot re-write _latest_data_info" << endl;
      cerr << "  dir: " << _args.dir << endl;
      return -1;
    }
  } else {
    cerr << "ERROR - LdataWriter::_refreshLatestTime" << endl;
    cerr << "  Cannot read _latest_data_info" << endl;
    cerr << "  dir: " << _args.dir << endl;
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// refresh option
// multiple times specified on command line

int LdataWriter::_refreshSpecifiedTimes()
{

  DsLdataInfo ldata(_args.dir, _args.verbose);
  if (ldata.readForced()) {
    cerr << "WARNING - LdataWriter::_refreshSpecifiedTimes" << endl;
    cerr << "  Cannot read _latest_data_info" << endl;
    cerr << "  dir: " << _args.dir << endl;
    cerr << "  not all info will be correct" << endl;
  }
  
  // loop through specified paths
  
  char *inputPath;
  while ((inputPath = _input->next()) != NULL) {
    
    // get the data time for the file
    
    time_t dataTime;
    bool dateOnly;

    if (DataFileNames::getDataTime(inputPath, dataTime, dateOnly, false) == 0) {
      
      // get relative file path

      Path ipath(inputPath);
      string relPath;
      Path::stripDir(_args.dir, inputPath, relPath);

      // rewrite the latest data info

      ldata.setLatestTime(dataTime);
      ldata.setRelDataPath(relPath);
      
      if (ldata.write()) {
        cerr << "ERROR - LdataWriter::_refreshSpecifiedTimes" << endl;
        cerr << "  Cannot write _latest_data_info" << endl;
        cerr << "  dir: " << _args.dir << endl;
        cerr << "  path: " << inputPath << endl;
        cerr << "  data time: " << DateTime::strm(dataTime) << endl;
        return -1;
      }

      if (_args.verbose) {
        cerr << "Refreshed _latest_data_info" << endl;
        cerr << "  dir: " << _args.dir << endl;
        cerr << "  path: " << inputPath << endl;
        cerr << "  data time: " << DateTime::strm(dataTime) << endl;
      } else if (_args.debug) {
        cerr << "Writing ldata to " << _args.dir
             << ", time: " << DateTime::str(dataTime) << endl;
      }

      umsleep(_args.msecsSleep);

    } else {
      
      cerr << "WARNING - LdataWriter::_refreshSpecifiedTimes()" << endl;
      cerr << "  Cannot get time for file: " << inputPath << endl;

    }

  }

  return 0;

}

//////////////////////////////////////////////////
// set object properties from command line args

void LdataWriter::_setFromArgs(DsLdataInfo &ldata)
{
  
  ldata.setDataFileExt(_args.fileExt.c_str());

  if (_args.dataType.size() > 0) {
    ldata.setDataType(_args.dataType.c_str());
  }

  if (_args.relDataPath.size() > 0) {
    ldata.setRelDataPath(_args.relDataPath.c_str());
  }

  if (_args.writer.size() > 0) {
    ldata.setWriter(_args.writer.c_str());
  }

  if (_args.userInfo1.size() > 0) {
    ldata.setUserInfo1(_args.userInfo1.c_str());
  }

  if (_args.userInfo2.size() > 0) {
     ldata.setUserInfo2(_args.userInfo2.c_str());
  }

  // set lead times if appropriate

  if (_args.isFcast) {
    ldata.setIsFcast();
    ldata.setLeadTime(_args.leadTime);
  }

  // set displaced data directory if appropriate
  
  if (_args.displacedDataDir.size() > 0) {
     ldata.setDisplacedDirPath(_args.displacedDataDir);
  }

}

