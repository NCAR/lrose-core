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
// DsFileDist.cc
//
// DsFileDist object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
///////////////////////////////////////////////////////////////

#include <sys/stat.h>
#include <sys/wait.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <toolsa/ReadDir.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/MsgLog.hh>
#include <didss/RapDataDir.hh>
#include <didss/RapDataDir_r.hh>
#include <dsserver/DsFileIo.hh>
#include <set>
#include "DsFileDist.hh"
#include "Watcher.hh"
#include "LocalFile.hh"

using namespace std;

bool DsFileDist::_gotHup = false;

// Constructor

DsFileDist::DsFileDist(int argc, char **argv) :
        _progName("DsFileDist"),
        _args(_progName)
  
{
  
  isOK = TRUE;
  _childCount = 0;
  
  // get command line args
  
  if (_args.parse(argc, (const char **) argv)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  Problem with command line args." << endl;
    isOK = FALSE;
    return;
  }
  
  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  Problem with TDRP parameters." << endl;
    isOK = FALSE;
    return;
  }
  
  // process registration
  
  PMU_auto_init(_progName.c_str(), _params.instance,
		PROCMAP_REGISTER_INTERVAL);

  // open error FMQ

  _errorFmqPath = _params.tmp_dir;
  _errorFmqPath += PATH_DELIM;
  _errorFmqPath += "fmq_DsFileDist.";
  _errorFmqPath += _params.instance;

  if (FMQ_init(&_errorFmq, (char *) _errorFmqPath.c_str(),
               false, "DsFileDist")) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot initialize error FMQ, path: " << _errorFmqPath << endl;
    isOK = FALSE;
    return;
  }

  // allow for 2000 messages, 2000 bytes each

  if (FMQ_open_create(&_errorFmq, 2000, 4000000)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot create error FMQ, path: " << _errorFmqPath << endl;
    isOK = FALSE;
    return;
  }

  return;

}

// destructor

DsFileDist::~DsFileDist()

{

  FMQ_close(&_errorFmq);
  FMQ_free(&_errorFmq);
    
}

//////////////////////////////////////////////////
// Run

int DsFileDist::Run()
{

  // initialize

  time_t now = time(NULL);
  _lastDataCheckTime = now;
  
  // if we are using latest data info only, then add the top
  // dir to the search list

  if (_params.find_mode == Params::FIND_LATEST_DATA_INFO) {
    dirForLdataInfoPair_t pp;
    pp.first = _params.source_top_dir;
    pp.second = _params;
    _dirForLdataInfoMap.insert(_dirForLdataInfoMap.begin(), pp);
  }

  // check directories
  
  _checkDirsForParams();
  _checkDirsForLdataInfo();
  time_t lastParamsCheckTime = now;
  time_t lastLdataCheckTime = now;
  bool putPrintDone = false;
  
  while (true) {

    PMU_auto_register("In DsFileDist main loop");
    now = time(NULL);
    
    // if required, check for new directories.
    // This either occurs if the program receives a SIGHUP, or 
    // the direcory delay has expired.
    
    if (_gotHup) {

      _checkDirsForParams();
      _checkDirsForLdataInfo();
      _gotHup = false;

    } else {

      if (_params.check_for_new_directories &&
          (now - lastParamsCheckTime) > _params.new_directory_delay) {
        _checkDirsForParams();
        lastParamsCheckTime = now;
      }

      if (_params.check_for_new_latest_data_info &&
          (now - lastLdataCheckTime) > _params.new_latest_data_info_delay) {
        _checkDirsForLdataInfo();
        lastLdataCheckTime = now;
      }

    }

    // check for new data

    _checkForNewData();

    // put files if available
    
    int nputsDone;

    if (_params.no_threads) {

      nputsDone = _putUnthreaded();

    } else {

      // purge any completed puts from the list

      _purgeCompletedPuts();
      
      // activate the next batch of puts
      
      nputsDone = _putThreaded();

    }

    if (nputsDone == 0) {
      if (_putList.size() > 0) {
        umsleep(50);
      } else {
        umsleep (100);
      }
      _purgeAgedPuts();
    }
    
    if (_params.debug >= Params::DEBUG_EXTRA) {
      if (nputsDone > 0) {
        putPrintDone = false;
      }
      if (!putPrintDone) {
        cerr << "---->> nputsDone: " << nputsDone << endl;
        cerr << "--> N waiting in put list: " << _putList.size() << endl;
        if (nputsDone == 0) {
          putPrintDone = true;
        }
      }
    }

    
  } // while

  return 0;

}

//////////////////////////////////////////////////
// Check the directories for _DsFileDist files

int DsFileDist::_checkDirsForParams()
{

  if (_params.find_mode == Params::FIND_LATEST_DATA_INFO) {
    // no need to check for params since from the top dir down
    // we are looking for _latest_data_info
    return 0;
  }

  PMU_auto_register("In DsFileDist::_checkDirsForParams");

  // start at the top - _checkDirForParams will recurse
  
  string dirPath;
  RapDataDir.fillPath(_params.source_top_dir, dirPath);
  
  if (_params.debug) {
    cerr << "Checking dirs, from top dir: " << dirPath << endl;
  }
  
  _checkDirForParams(dirPath, 0);
  
  return 0;

}

////////////////////////////////////////////////////////////////
// Check directories for latest data info 

void DsFileDist::_checkDirsForLdataInfo()

{
  
  dirForLdataInfoMap_t::iterator ii;
  for (ii = _dirForLdataInfoMap.begin();
       ii != _dirForLdataInfoMap.end(); ii++) {
    const string &dir = (*ii).first;
    const Params &params = (*ii).second;
    _checkDirForLdataInfo(dir, dir, params);
  } // ii
  
}

///////////////////////////////////////////////////////////////////
// _checkDirForParams()
//
// Recurse down the directory tree, checking for existence
// of _DsFileDist files
//
// The findLdataInfo argument indicates whether we are looking
// for (a) _DsFileDist params files (false) or
//     (b) _latest_data_info files (true)
//
// Returns 0 on success, -1 on failure.

int DsFileDist::_checkDirForParams(const string &dirPath, int level)
  
{
  
  PMU_auto_register("In DsFileDist::_checkDirForParams");

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "DsFileDist.checkDirForParams, dir: " << dirPath
         << ", recurse level: " << level << endl;
  }

  if (level > 255) {
    cerr << "ERROR - DsFileDist.checkDirForParams" << endl;
    cerr << "  Too many recursion levels, max 255" << endl;
    return -1;
  }

  // activate this directory if needed.
  // If active, do not recurse further.
  
  if (_activateFromParams(dirPath)) {
    return 0;
  }
  
  // Try to open the directory
  
  ReadDir rdir;
  if (rdir.open(dirPath.c_str())) {
    cerr << "ERROR - " << "DsFileDist::_checkDirForParams()." << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  Cannot open dirPath " << dirPath << endl;
    cerr << "  " << strerror(errno) << endl;
    return (-1);
  }

  // Loop thru directory looking for sub-directories
  
  struct dirent *dp;
  for (dp = rdir.read(); dp != NULL; dp = rdir.read()) {
    
    // exclude the '.' and '..' entries
    
    if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) {
      continue;
    }

    // stat the file
    
    string path = dirPath;
    path += PATH_DELIM;
    path += dp->d_name;
    struct stat dirStat;

    if (stat(path.c_str(), &dirStat)) {
      // file has disappeared, probably cleaned up by the Janitor
      // so ignore
      continue;
    }
    
    // check for directory
    
    if (S_ISDIR(dirStat.st_mode)) {

      // exclude day dirs
      
      int year, month, day;
      if (sscanf(dp->d_name, "%4d%2d%2d", &year, &month, &day) == 3) {
        if (year > 2000 && year < 3000) {
          if (_params.debug >= Params::DEBUG_VERBOSE) {
            cerr << "---->> ignoring day dir, path: " << path << endl;
          }
          continue;
        }
      }

      // this is a directory, so recurse into it

      _checkDirForParams(path, level + 1);
      
    }
    
  } // dp
  
  rdir.close();
  
  return 0;

}

/////////////////////////////////////////////////////////////////
// _checkDirForLdataInfo()
//
// Recurse down the directory tree, checking for existence
// of _latest_data_info
//
// Returns 0 on success, -1 on failure.

int DsFileDist::_checkDirForLdataInfo(const string &dirPath,
                                      const string &paramsDir,
                                      const Params &paramsInUse)
  
{
  
  PMU_auto_register("In DsFileDist::_checkDirForLdataInfo");
  
  // activate this directory if needed.
  // If active, do not recurse further.
  
  if (_activateFromLdataInfo(dirPath, paramsDir, paramsInUse)) {
    return 0;
  }

  // Try to open the directory
  
  ReadDir rdir;
  if (rdir.open(dirPath.c_str())) {
    cerr << "ERROR - " << "DsFileDist::_checkDirForLdataInfo." << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  Cannot open dirPath " << dirPath << endl;
    cerr << "  " << strerror(errno) << endl;
    return (-1);
  }

  // Loop thru directory looking for sub-directories
  
  struct dirent *dp;
  for (dp = rdir.read(); dp != NULL; dp = rdir.read()) {
    
    // exclude the '.' and '..' entries
    
    if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) {
      continue;
    }

    // stat the file
    
    string path = dirPath;
    path += PATH_DELIM;
    path += dp->d_name;
    struct stat dirStat;

    if (stat(path.c_str(), &dirStat)) {
      // file has disappeared, probably cleaned up by the Janitor
      // so ignore
      continue;
    }
    
    // check for directory
    
    if (S_ISDIR(dirStat.st_mode)) {
      // this is a directory, so recurse into it
      _checkDirForLdataInfo(path, paramsDir, paramsInUse);
    }
    
  } // dp
  
  rdir.close();
  
  return 0;

}

///////////////////////////////////////////////////////////
// _activateFromParams()
//
// Check if this directory is active for distribution, by
// checking for a param file.
//
// Returns true if active, false if not.

bool DsFileDist::_activateFromParams(const string &dirPath)
  
{
  
  PMU_auto_register("Checking for params");
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "-->> checking dir: " << dirPath << endl;
  }
  
  // try to find _DsFileDist param file
  
  string paramFilePath(dirPath);
  paramFilePath += PATH_DELIM;
  paramFilePath += "_DsFileDist";
  if (strlen(_params._dsFileDist_ext) > 0) {
    paramFilePath += ".";
    paramFilePath += _params._dsFileDist_ext;
  }
  struct stat paramsStat;
  if (stat(paramFilePath.c_str(), &paramsStat)) {
    return false;
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "---->> _DsFileDist param file found, dir:" << dirPath << endl;
  }
  
  // make a copy of params from original, override with local info
    
  Params paramsInUse(_params);
  if (paramsInUse.load((char *) paramFilePath.c_str(), NULL, true, false)) {
    // failure to read in param file
    cerr << "WARNING - DsFileDist::_activateIfNeeded" << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  Dir: " << dirPath << endl;
    cerr << "  Cannot load local params from file: "
         << paramFilePath << endl;
    return true;
  }
    
  // ignore this directory?
  if (!paramsInUse.process) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "------>> Do not process, ignoring dir: " << dirPath << endl;
    }
    return true;
  }
  
  // switch find mode?

  if (paramsInUse.find_mode == Params::FIND_LATEST_DATA_INFO) {

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "------>> Switching to ldata find mode, dir: " << dirPath << endl;
    }

    // recursively look for _latest_data_info files

    if (_params.debug) {
      cerr << "Adding to ldataParams map, dir: " << dirPath << endl;
    }
    dirForLdataInfoPair_t pp;
    pp.first = dirPath;
    pp.second = paramsInUse;
    _dirForLdataInfoMap.insert(_dirForLdataInfoMap.begin(), pp);
    return true;

  }
    
  // is this directory already active?
  
  activeMap_t::iterator ii = _activeMap.find(dirPath);
  if (ii != _activeMap.end()) {
    // this directory is already active
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "------>> Already in list, dir: " << dirPath << endl;
    }
    return true;
  }
  
  // create distributor object
  
  bool ldataAvail = paramsInUse.latest_data_info_avail;
  Watcher *watcher = new Watcher(_progName, paramsInUse, dirPath,
                                 true, ldataAvail, dirPath, _putList,
                                 _errorFmqPath);
  
  if (!watcher->isOK) {
    cerr << "ERROR - " << "DsFileDist::_checkIfActive()." << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  Could not create distribution object" << endl;
    delete (watcher);
    return -1;
  }
  
  // Add this entry to the active list
  
  if (_params.debug) {
    cerr << "Adding to active list, dir: " << dirPath << endl;
  }
  activePair_t pp;
  pp.first = dirPath;
  pp.second = watcher;
  _activeMap.insert(_activeMap.begin(), pp);
  
  return true;

}

///////////////////////////////////////////////////////////
// _activateFromLdata()
//
// Check if this directory is active for distribution, by
// checking for a latest_data_info file.
//
// Returns true if active, false if not.

bool DsFileDist::_activateFromLdataInfo(const string &dirPath,
                                        const string &paramsDir,
                                        const Params &paramsInUse)
  
{
  
  PMU_auto_register("Checking for ldata info");
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "-->> checking for ldata, dir: " << dirPath << endl;
  }
  
  // try to find _latest_data_info
  
  LdataInfo ldata(dirPath);
  if (ldata.readForced()) {
    return false;
  }
    
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "---->> _latest_data_info.xml file as trigger, dir:"
         << dirPath << endl;
  }
  
  // is this directory already active?
  
  activeMap_t::iterator ii = _activeMap.find(dirPath);
  if (ii != _activeMap.end()) {
    // this directory is already active
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "------>> Already in list, dir: " << dirPath << endl;
    }
    return true;
  }
  
  // create distributor object
  
  Watcher *watcher = new Watcher(_progName, paramsInUse, dirPath,
                                 false, true, paramsDir, _putList,
                                 _errorFmqPath);
  
  if (!watcher->isOK) {
    cerr << "ERROR - " << "DsFileDist::_activateFromLdata." << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  Could not create distribution object" << endl;
    delete (watcher);
    return -1;
  }
  
  // Add this entry to the active list
  
  if (_params.debug) {
    cerr << "Adding to active list, dir: " << dirPath << endl;
  }
  activePair_t pp;
  pp.first = dirPath;
  pp.second = watcher;
  _activeMap.insert(_activeMap.begin(), pp);
  
  return true;

}

//////////////////////////////////////////////////
// Check for new data

int DsFileDist::_checkForNewData()
{

  // time to check?

  struct timeval tval;
  gettimeofday(&tval, NULL);
  double now = (double) tval.tv_sec + tval.tv_usec / 1.0e6;
  if (now - _lastDataCheckTime < _params.new_data_delay) {
    return 0;
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "DsFileDist::_checkForNewData: " << DateTime::str() << endl;
    cerr << "  Checking for new data" << endl;
  }
  _lastDataCheckTime = now;
  
  PMU_auto_register("In DsFileDist::_checkForNewData");

  // purge the list of params files which have gone

  _purgeActiveParamsMap();

  // check for new data in each of the active diretories

  activeMap_t::iterator ii;
  for (ii = _activeMap.begin(); ii != _activeMap.end(); ii++) {
    Watcher *watcher = (*ii).second;
    watcher->findFiles();
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE && _putList.size() > 0) {
    cerr << "--> N waiting in put list: " << _putList.size() << endl;
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "List of waiting entries:" << endl;
      cerr << "==============================================" << endl;
      cerr << "  " << DateTime::str() << endl;
      list<PutArgs *>::iterator jj;
      for (jj = _putList.begin(); jj != _putList.end(); jj++) {
        PutArgs *args = (*jj);
        cerr << "  destHostName: " << args->destHostName << endl;
        cerr << "  destUrl: " << args->destUrl.getURLStr() << endl;
        cerr << "  dirPath: " << args->dirPath << endl;
        cerr << "  fileName: " << args->fileName << endl;
        cerr << "==============================================" << endl;
      }
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "DsFileDist.checkForNewData.exit:  " << DateTime::str() << endl;
  }
  return 0;

}

////////////////////////////////////////////////////////////////
// Purge active list of entries for which params which have gone

void DsFileDist::_purgeActiveParamsMap()

{

  bool done = false;

  while (!done) {

    done = true;

    activeMap_t::iterator ii;
    for (ii = _activeMap.begin(); ii != _activeMap.end(); ii++) {
      Watcher *watcher = (*ii).second;
      if (!watcher->isStillActive()) {
	_activeMap.erase(ii);
	done = false;
	break;
      }
    } // ii

  } // while (!done)

}


////////////////////////////////////////////////////////////////
// Purge completed puts

void DsFileDist::_purgeCompletedPuts()

{

  pid_t dead_pid;
  int status;

  while ((dead_pid = waitpid((pid_t) -1,
                             &status,
                             (int) (WNOHANG | WUNTRACED))) > 0) {

    list<PutArgs *>::iterator ii;
    for (ii = _putList.begin(); ii != _putList.end(); ii++) {
      PutArgs *args = (*ii);

      if (dead_pid == args->putPid) {

        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "  pid done: " << args->putPid << endl;
        }

        // clean up child process entry

        _childCount--;
        delete args;
        _putList.erase(ii);

        // check for error messages
        
        _checkErrorFmq();

        break;

      } // if (dead_pid == args->putPid) 

    } // ii

  } // while

}

////////////////////////////////////////////////////////////////
// Purge put entries which are too old

void DsFileDist::_purgeAgedPuts()

{

  time_t now = time(NULL);
  int maxAge = _params.max_age_on_queue;

  bool done = false;
  while (!done) {
    
    done = true;
    
    list<PutArgs *>::iterator ii;
    for (ii = _putList.begin(); ii != _putList.end(); ii++) {
      PutArgs *args = (*ii);
      if (!args->childRunning) {
        int age = now - args->timeAdded;
        if (age > maxAge) {
          if (_params.debug) {
	    cerr << "-->> removing old put entry: " << endl;
            cerr << "  host: " << args->destUrl.getHost() << endl;  
            cerr << "  dir: " << args->dirPath << endl;  
            cerr << "  file: " << args->fileName << endl;  
	  }
	  delete args;
	  _putList.erase(ii);
	  done = false;
	  break;
	}
      }
    } // ii

  } // while (!done)

}

////////////////////////////////////////////////////////////////
// Check FMQ for errors

void DsFileDist::_checkErrorFmq()

{

  int msgRead;
  while (FMQ_read(&_errorFmq, &msgRead) == 0) {
    if (msgRead) {
      char *msg = (char *) _errorFmq.msg;
      if (strlen(msg) > 0) {
        cerr << "============== Message from put thread =============" << endl;
        cerr << msg << endl;
        cerr << "============== End Message from put thread =========" << endl;
      } else {
        return;
      }
    } else {
      return;
    }
  } // while

}
  
//////////////////////////////////////////////////
// Put the next batch of files threaded
// Returns the number of puts activated

int DsFileDist::_putThreaded()
{

  int nputs = 0;
  PMU_auto_register("In DsFileDist::_putThreaded");
  
  // set up the busy host list, by finding those hosts which
  // already have the limit of puts in progress

  set<string> busyHosts;
  list<PutArgs *>::iterator ii;
  for (ii = _putList.begin(); ii != _putList.end(); ii++) {
    PutArgs *args = (*ii);
    if (args->inProgress) {
      int nSimul = _countSimulToHost(args->destHostName);
      if (nSimul >= _params.max_simultaneous_per_host &&
	  busyHosts.find(args->destHostName) == busyHosts.end()) {
	busyHosts.insert(busyHosts.begin(), args->destHostName);
	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << "-->> Host: " << args->destHostName << " has n active puts: "
	       << nSimul << " ... wait ..." << endl;
	}
      }
    }
  } // ii

  // go through the put list, sending the first file to each non-busy host
  // and then marking that host as busy
  
  for (ii = _putList.begin(); ii != _putList.end(); ii++) {
    
    PutArgs *args = (*ii);
    
    if (!args->inProgress &&
	busyHosts.find(args->destHostName) == busyHosts.end()) {

      // mark this host as busy
      
      busyHosts.insert(busyHosts.begin(), args->destHostName);
      
      // check number of active threads (children)
      // if too many, mark this put as done instead of actually doing it
      
      if (_childCount >= _params.max_n_threads) {
	cerr << "WARNING - " << _progName << ":_putThreaded()" << endl;
	cerr << "  " << DateTime::str() << endl;
	cerr << "  Too many children - max allowed: " <<
	  _params.max_n_threads << endl;
        cerr << "  Need to wait" << endl;
	return nputs;
      }
      
      // mark put as in-progress
      
      args->inProgress = true;

      // spawn a child for communicating
      
      nputs++;
      args->putPid = fork();
      
      // check for parent or child
      
      if (args->putPid == 0) {
        
	// child - do the put and exit
	// Check if this is a file write

	string sendHost = args->destUrl.getHost();

	if (!(strcmp("localfile", sendHost.c_str()))){
	  
	  // It is going to disk.

	  LocalFile lf(_progName, _params);
	  if (lf.doCopy(args)) {
	    cerr << "ERROR - " << _progName << endl;
	    cerr << "  Failed to make local copy to disk" << endl;
	  }

	} else {

	  // It's not going to disk, distribute it
          
	  put(args);
	  
	}

	// child exits

	exit(0);

      } else {

	// parent
	
	_childCount++;
	args->childRunning = true;
	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << "==============================================" << endl;
	  cerr << "  Started child: " << args->putPid << endl;
	  cerr << "    destHostName: " << args->destHostName << endl;
	  cerr << "    destUrl: " << args->destUrl.getURLStr() << endl;
	  cerr << "    dirPath: " << args->dirPath << endl;
	  cerr << "    fileName: " << args->fileName << endl;
	  cerr << "==============================================" << endl;
	}

      }
      
    } // if (busyHosts.find() ...

  } // ii

  return nputs;

}

//////////////////////////////////////////////////
// Put the next batch of files unthreaded
// Returns the number of puts done

int DsFileDist::_putUnthreaded()
{

  int nputs = 0;
  
  PMU_auto_register("In DsFileDist::_putUnthreaded");
  
  // go through the list
  
  list<PutArgs *>::iterator ii;
  for (ii = _putList.begin(); ii != _putList.end(); ii++) {
    
    nputs++;
    PMU_auto_register("Putting next file");

    // put the file

    PutArgs *args = (*ii);
    put(args);

    // check for errors

    _checkErrorFmq();

  }

  // erase the list

  for (ii = _putList.begin(); ii != _putList.end(); ii++) {
    delete *ii;
  }
  _putList.erase(_putList.begin(), _putList.end());

  return nputs;

}
	
//////////////////////////////////////////////////
// Count the number of simultaneous puts to a host

int DsFileDist::_countSimulToHost(const string &host)
{

  int count = 0;

  list<PutArgs *>::iterator ii;
  for (ii = _putList.begin(); ii != _putList.end(); ii++) {
    PutArgs *args = (*ii);
    if (args->destHostName == host && args->inProgress) {
      count++;
    }
  } // ii

  return count;

}

//////////////////////////////////
// functions for put thread
//
// In order to be called by a thread, these are static function on the class.

void *DsFileDist::put(void *args)
  
{

  PutArgs &putArgs = *((PutArgs *) args);
  string errStr;

  // check file age

  if (_checkFileAge(putArgs, errStr) == 0) {

    // set up DsFileCopy object
    
    DsFileCopy copy;
    if (putArgs.Debug >= Params::DEBUG_EXTRA) {
      copy.setDebug();
    }
    copy.setMaxFileAge(putArgs.maxAgeAtCopyTime);
    if (putArgs.openTimeoutMsecs > 0) {
      copy.setOpenTimeoutMsecs(putArgs.openTimeoutMsecs);
    }
    
    // set the ldataInfo to reflect the fact that the file is
    // being distributed by DsFileDist
    
    putArgs.ldataInfo.setWriter("DsFileDist");
    
    RAPDataDir_r rdir;
    string dirName;
    rdir.stripPath(putArgs.dirPath, dirName);
    
    // Try until ntries exceeded.
    // After first try, always do a forced put.
    
    int nTries = 0;
    while (true) {
      int iret = 0;
      if (putArgs.forceCopy || nTries > 0) {
        iret = _putForced(putArgs, dirName, copy, errStr);
      } else {
        iret = _putWithEnquire(putArgs, dirName, copy, errStr);
      }
      if (iret == 0) {
        if (nTries > 0) {
          TaStr::AddStr(errStr, "", "=====>> RETRY successful <<=====");
        }
        break;
      }
      nTries++;
      if (nTries >= putArgs.maxTries) {
        TaStr::AddStr(errStr, "", "=====>> RETRY failure <<=====");
        break;
      }
      TaStr::AddStr(errStr, "", "=====>> RETRY retrying put <<=====");
    }
    

  } // if (_checkFileAge(putArgs) == 0)

  // write any errors or debug messages to FMQ, which the parent
  // will then read

  _writeErrorFmq(putArgs, errStr);

  putArgs.done = true;
  return NULL;
  
}

//////////////////////////////////////////////////////
//
// Perform a forced put to the server.
//
// Returns 0 on success, -1 on failure.

int DsFileDist::_putForced(const PutArgs &putArgs,
                           const string &dirName,
                           DsFileCopy &copy,
                           string &errStr)
  
{

  if (putArgs.Debug >= Params::DEBUG_VERBOSE) {
    TaStr::AddStr(errStr, "", "-->> putForced");
    TaStr::AddStr(errStr, "  Time: ", DateTime::str());
    TaStr::AddStr(errStr, "  dest URL: ", putArgs.destUrl.getURLStr());
    TaStr::AddStr(errStr, "  dirPath: ", putArgs.dirPath);
    TaStr::AddStr(errStr, "  fileName: ", putArgs.fileName);
  }
  
  if (copy.putForced(dirName,
		     putArgs.destUrl,
		     putArgs.ldataInfo,
		     putArgs.fileName,
		     putArgs.compressionType,
		     putArgs.removeAfterCopy)) {

    TaStr::AddStr(errStr, "", "ERROR - _putForced().");
    errStr += copy.getErrorStr();
    _log("error", putArgs.destHostName,
	 copy.getFileLen(), copy.getFilePath());
    if (putArgs.Debug) {
      cerr << "===============>>> ERROR <<<=====================" << endl;
      cerr << errStr << endl;
      cerr << "=================================================" << endl;
    }
    return -1;

  } else {

    _log("success", putArgs.destHostName,
	 copy.getFileLen(), copy.getFilePath());

  }

  return 0;

}

//////////////////////////////////////////////////////
//
// First enquire from the server whether to do the put.
// If requested to proceed, put the file.
//
// Returns 0 on success, -1 on failure.

int DsFileDist::_putWithEnquire(const PutArgs &putArgs,
                                const string &dirName,
                                DsFileCopy &copy,
                                string &errStr)
  
{

  // not forced, so enquire from the server
  
  if (putArgs.Debug >= Params::DEBUG_VERBOSE) {
    TaStr::AddStr(errStr, "", "-->> putWithEnquire");
    TaStr::AddStr(errStr, "  Time: ", DateTime::str());
    TaStr::AddStr(errStr, "  dest URL: ", putArgs.destUrl.getURLStr());
    TaStr::AddStr(errStr, "  dirPath: ", putArgs.dirPath);
    TaStr::AddStr(errStr, "  fileName: ", putArgs.fileName);
    TaStr::AddInt(errStr, "  overwriteAge: ", putArgs.overwriteAge);
  }
  
  if (copy.enquireForPut(dirName, putArgs.destUrl,
			 putArgs.ldataInfo,
			 putArgs.fileName.c_str(),
			 putArgs.overwriteAge)) {
    TaStr::AddStr(errStr, "", "ERROR - _putWithEnquire().");
    TaStr::AddStr(errStr, "  Time: ", DateTime::str());
    errStr += copy.getErrorStr();
    _log("error", putArgs.destHostName,
	 copy.getFileLen(), copy.getFilePath());
    if (putArgs.Debug) {
      cerr << "===============>>> ERROR <<<=====================" << endl;
      cerr << errStr << endl;
      cerr << "=================================================" << endl;
    }
    return -1;
  }

  if (copy.DoPut()) {
    
    if (putArgs.Debug >= Params::DEBUG_VERBOSE) {
      TaStr::AddStr(errStr, "", "---->> doing put");
    }
    
    if (copy.putForced(dirName,
		       putArgs.destUrl,
		       putArgs.ldataInfo,
		       putArgs.fileName,
		       putArgs.compressionType,
		       putArgs.removeAfterCopy)) {
      TaStr::AddStr(errStr, "", "ERROR - _putWithEnquire().");
      TaStr::AddStr(errStr, "  Time: ", DateTime::str());
      errStr += copy.getErrorStr();
      _log("error", putArgs.destHostName,
	   copy.getFileLen(), copy.getFilePath());
      if (putArgs.Debug) {
        cerr << "===============>>> ERROR <<<=====================" << endl;
	cerr << errStr << endl;
        cerr << "=================================================" << endl;
      }
      return -1;
    } else {
      _log("success", putArgs.destHostName,
	   copy.getFileLen(), copy.getFilePath());
    }
    
  } else {
    
    if (putArgs.Debug >= Params::DEBUG_VERBOSE) {
      TaStr::AddStr(errStr, "     ", "---->> put declined by server.");
    }
    _log("declined", putArgs.destHostName,
	 copy.getFileLen(), copy.getFilePath());
  }
  
  return 0;

}

//////////////////////////////////////////////////////
// Check file age is OK
// Returns -1 if file is too old, 0 otherwise

int DsFileDist::_checkFileAge(const PutArgs &putArgs,
                              string &errStr)
  
{

  // stat the file
  
  string filePath =  putArgs.ldataInfo.getDataPath();
  struct stat fileStat;
  if (ta_stat(filePath.c_str(), &fileStat)) {
    TaStr::AddStr(errStr, "ERROR - _checkFileAge", "");
    TaStr::AddStr(errStr, "  Cannot stat file: ", filePath);
    return -1;
  }
  int fileLen = fileStat.st_size;

  int maxFileAge = putArgs.maxAgeAtCopyTime;
  if (maxFileAge > 0) {
    time_t now = time(NULL);
    int fileAge = now - fileStat.st_mtime;
    if (fileAge > maxFileAge) {
      if (putArgs.Debug >= Params::DEBUG_VERBOSE) {
        TaStr::AddStr(errStr, "ERROR - checkFileAge", "");
        TaStr::AddStr(errStr, "  File too old to send: ", filePath);
        TaStr::AddInt(errStr, "    File age (secs): ", fileAge);
        TaStr::AddInt(errStr, "    Max age allowed (secs): ", maxFileAge);
      }
      _log("too_old", putArgs.destHostName, fileLen, filePath);
      return -1;
    }
  }

  return 0;

}

//////////////////////////////////
// write log as appropriate

void DsFileDist::_log(const string &label,
                      const string &host,
                      int nbytes,
                      const string &filePath)
  
{

  char *logDir = getenv("DATA_DISTRIB_LOG_DIR");
  if (logDir == NULL) {
    return;
  }

  string logName("DsFileDist");
  char *logSuffix = getenv("DATA_DISTRIB_LOG_SUFFIX");
  if (logSuffix != NULL) {
    logName += ".";
    logName += logSuffix;
  }
  
  MsgLog log(logName);
  log.setDayMode();
  log.setAppendMode();
  log.setSuffix("log");
  log.setOutputDir(logDir);
  
  time_t now = time(NULL);

  string fileName;
  RapDataDir.stripPath(filePath, fileName);

  log.postMsg("%s, %8s, %18s, %8d, %s",
	      DateTime::str(now, false).c_str(),
	      label.c_str(),
	      host.c_str(),
	      nbytes,
	      fileName.c_str());

}

void DsFileDist::_writeErrorFmq(const PutArgs &putArgs,
                                const string &errStr)
  
{

  // check for error message 
  // write to FMQ as appropriate

  if (errStr.size() == 0) {
    return;
  }

  // shut down procmap registration because the open
  // may call PMU_auto_regsiter
  
  PMU_clear_init();
    
  // initialize FMQ

  FMQ_handle_t errorFmq;
  if (FMQ_init(&errorFmq, (char *) putArgs.errorFmqPath.c_str(),
               false, "DsFileDist_put_thread")) {
    cerr << "ERROR: DsFileDist put thread" << endl;
    cerr << "  Cannot initialize error FMQ, path: "
         << putArgs.errorFmqPath << endl;
    return;
  }

  // open existing FMQ

  if (FMQ_open_rdwr_nocreate(&errorFmq)) {
    cerr << "ERROR: DsFileDist put thread" << endl;
    cerr << "  Cannot open error FMQ for reading, path: "
         << putArgs.errorFmqPath << endl;
    FMQ_free(&errorFmq);
    return;
  }

  // write error message to FMQ

  if (FMQ_write(&errorFmq, (char *) errStr.c_str(), errStr.size(), 0, 0)) {
    cerr << "ERROR: DsFileDist put thread" << endl;
    cerr << "  Cannot write message to FMQ, path: "
         << putArgs.errorFmqPath << endl;
  }

  FMQ_close(&errorFmq);
  FMQ_free(&errorFmq);

}

