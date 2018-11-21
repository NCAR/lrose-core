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
// DataSet.cc
//
// DataSet object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2006
//
///////////////////////////////////////////////////////////////

#include <iostream>
#include <signal.h>
#include <sys/wait.h>
#include <cstring>
#include <toolsa/str.h>
#include <toolsa/TaStr.hh>
#include <toolsa/pmu.h>
#include <didss/RapDataDir.hh>
#include <dsserver/DmapAccess.hh>
#include "DataSet.hh"
using namespace std;

// Constructor

DataSet::DataSet(const Params &params,
                 int data_set_num) :
        _params(params),
        _dataSetNum(data_set_num)
  
{

  isOK = true;
  _timeLastAction = time(NULL);
  _lateDataScriptCalled = false;
  
  const Params::data_set_t &dataSet = _params._data_sets[_dataSetNum];
  _inputDir = dataSet.input_dir;
  _callNewDataScript = dataSet.call_new_data_script;
  _newDataScriptName = dataSet.new_data_script_name;
  _callLateDataScript = dataSet.call_late_data_script;
  _lateDataScriptName = dataSet.late_data_script_name;
  _lateDataSecs = dataSet.late_data_secs;
  if (strlen(dataSet.trailing_args) > 0) {
    TaStr::tokenize(dataSet.trailing_args, 
                    _params.trailing_args_delimiter, 
                    _trailingArgs);
  }

  if (_params.debug) {
    cerr << "============================================" << endl;
    cerr << "Setting up to watch data set:" << endl;
    cerr << "  inputDir: " << _inputDir << endl;
    if (_callNewDataScript) {
      cerr << "  On new data, calling script: " << _newDataScriptName << endl;
    }
    if (_callLateDataScript) {
      cerr << "  On late data, calling script: " << _lateDataScriptName << endl;
      cerr << "    when data is older than (secs): " << _lateDataSecs << endl;
    }
    if (_trailingArgs.size() > 0) {
      cerr << "  Trailing args:" << endl;
      for (size_t ii = 0; ii < _trailingArgs.size(); ii++) {
        cerr << "    " << _trailingArgs[ii] << endl;
      }
    }
    cerr << "============================================" << endl;
  }
  
  // set up LdataInfo object

  if (_ldata.setDir(_inputDir)) {
    cerr << "ERROR - DataSet::DataSet" << endl;
    cerr << "  Cannot set input dir: " << _inputDir << endl;
    isOK = false;
    return;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _ldata.setDebug();
  }

  if (_params.read_ldata_fmq_from_start) {
    _ldata.setReadFmqFromStart(true);
  }

  if (_params.save_read_state) {
    char label[1024];
    sprintf(label, "LdataMultWatcher_%s_dataSet%.3d",
            _params.instance, _dataSetNum);
    _ldata.setSaveLatestReadInfo(label,
                                 _params.max_realtime_valid_age);
  }

  _ldataPrev = _ldata;

  if (_params.debug) {
    cerr << "LdataMultWatcher - watching dir: " << _inputDir << endl;
  }

}

// destructor

DataSet::~DataSet()

{


}

//////////////////////////////////////////////////
// Check data status
//
// Returns true if data set found, false otherwise

bool DataSet::check()
{

  // register with procmap
  
  PMU_auto_register("DataSet::check");
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "DataSet::check - checking dir: " << _inputDir << endl;
  }
  
  if (_ldata.read(_params.max_realtime_valid_age) == 0) {
    
    if (_params.debug) {
      cerr << "=======================================================" << endl;
      cerr << "LdataMultWatcher - got latest data" << endl;
      cerr << "Input dir: " << _inputDir << endl;
      cerr << "Data path: " << _ldata.getDataPath() << endl;
      cerr << "-------------------------------------------------------" << endl;
      _ldata.printAsXml(cerr);
      cerr << "=======================================================" << endl;
    }
      
    // found new data
    
    _timeLastAction = time(NULL);
    
    if (_params.debug) {
      cerr << "DataSet::check - got latest data" << endl;
      cerr << "  Data path: " << _ldata.getDataPath() << endl;
    }
      
    // call script as required
    
    if (_callNewDataScript) {
      _callScript(_params.run_script_in_background,
                  _ldata, _newDataScriptName.c_str(), false);
    }
    
    _ldataPrev = _ldata;
    _lateDataScriptCalled = false;

    return true;
    
  } else {
    
    // data did not arrive, check for late data
    
    if (_callLateDataScript && !_lateDataScriptCalled) {

      time_t now = time(NULL);
      int timeSinceLastData = now - _timeLastAction;
      if (timeSinceLastData > _lateDataSecs) {
        _callScript(true, _ldataPrev, _lateDataScriptName.c_str(), true);
        _timeLastAction = now;
        _lateDataScriptCalled = true;
      }

    } // if (_ldata.read ...
    
    // reap or kill child processes as required
    
    _reapChildren();
    
  } // while

  return false;
  
}

//////////////
// call script

void DataSet::_callScript(bool run_in_background,
                          const LdataInfo &ldata,
                          const char *script_to_call,
                          bool include_late_data_secs)
  
{

  char pmuStr[4096];
  sprintf(pmuStr, "Starting %s", script_to_call);
  PMU_force_register(pmuStr);
  
  // Fork a child to run the script
  
  time_t start_time = time(NULL);
  time_t terminate_time = start_time + _params.script_max_run_secs;
  pid_t childPid = fork();
  
  if (childPid == 0) {
    
    // this is the child process, so exec the script

    if (_params.specify_script_args) {
      _execScriptSpecifiedArgs(ldata, script_to_call);
    } else {
      _execScriptAllArgs(ldata, script_to_call, include_late_data_secs);
    }

    // exit
    
    if (_params.debug) {
      cerr << "Child process exiting ..." << endl;
    }

    _exit(0);

  }

  // this is the parent

  if (_params.debug) {
    cerr << "Script started, child pid: " << childPid << endl;
  }

  if (run_in_background) {
    
    // add the child to the map of active children
    
    activePair_t pp;
    pp.first = childPid;
    pp.second = terminate_time;
    _active.insert(_active.begin(), pp);
    
    if (_params.debug) {
      activeMap_t::iterator ii;
      for (ii = _active.begin(); ii != _active.end(); ii++) {
	cerr << "pid: " << ii->first << "  terminate_time: "
	     << ii->second << endl;
      }
    }
    
  } else {
    
    // script in the foreground - so we must wait for it
    // to complete
    
    while (true) {
      
      PMU_auto_register(pmuStr);

      int status;
      if (waitpid(childPid, &status,
		  (int) (WNOHANG | WUNTRACED)) == childPid) {
	// child exited
	time_t end_time = time(NULL);
	int runtime = (int) (end_time - start_time);
	sprintf(pmuStr, "%s took %d secs",
		script_to_call, runtime);
	PMU_force_register(pmuStr);
	if (_params.debug) {
	  cerr << "Child exited, pid: " << childPid << endl;
	  cerr << "  Runtime in secs: " << runtime << endl;
	}
	return;
      }
      
      // script is still running
      
      sprintf(pmuStr, "%s running", script_to_call);
      PMU_auto_register(pmuStr);
      
      // child still running - kill it as required

      _killAsRequired(childPid, terminate_time);

      if (_params.sleep_after_script) {
        if(_params.debug) {
          cerr << "Sleeping for " << _params.script_sleep_time 
               << " microseconds" << endl;
        }
        // sleep for a sec
        umsleep(_params.script_sleep_time);
      }

      umsleep(100);

    } // while
    
  } // if (run_script_in_background)

}

////////////////////////////////////
// execute the script - command line
// using all args - the default

void DataSet::_execScriptAllArgs(const LdataInfo &ldata,
                                 const char *script_to_call,
                                 bool include_late_data_secs)
					  
{

  // load vector of strings for args

  vector<string> argVec;
  char tmpStr[1024];
  
  argVec.push_back(script_to_call);

  if (_params.debug) {
    _addCommand("-debug", argVec);
  }

  if (ldata.isFcast()) {

    // forecast data set
    
    _addCommand("-is_forecast", argVec);
    argVec.push_back("true");

    int leadSecs = ldata.getLeadTime();
    _addCommand("-forecast_lead_secs", argVec);
    sprintf(tmpStr, "%d", leadSecs);
    argVec.push_back(tmpStr);
  
    time_t genTime = ldata.getLatestTime();
    time_t validTime = genTime + leadSecs;
    
    DateTime gtime(genTime);
    DateTime vtime(validTime);

    sprintf(tmpStr, "%ld", genTime);
    _addCommand("-gen_utime", argVec);
    argVec.push_back(tmpStr);

    sprintf(tmpStr, "%.4d,%.2d,%.2d,%.2d,%.2d,%.2d",
            gtime.getYear(), gtime.getMonth(), gtime.getDay(),
            gtime.getHour(), gtime.getMin(), gtime.getSec());
    _addCommand("-gen_time", argVec);
    argVec.push_back(tmpStr);

    sprintf(tmpStr, "%ld", validTime);
    _addCommand("-valid_utime", argVec);
    argVec.push_back(tmpStr);
    
    sprintf(tmpStr, "%.4d,%.2d,%.2d,%.2d,%.2d,%.2d",
            vtime.getYear(), vtime.getMonth(), vtime.getDay(),
            vtime.getHour(), vtime.getMin(), vtime.getSec());
    _addCommand("-valid_time", argVec);
    argVec.push_back(tmpStr);
    
  } else {

    _addCommand("-is_forecast", argVec);
    argVec.push_back("false");

    time_t validTime = ldata.getLatestTime();
    
    DateTime vtime(validTime);

    sprintf(tmpStr, "%ld", validTime);
    _addCommand("-valid_utime", argVec);
    argVec.push_back(tmpStr);
    
    sprintf(tmpStr, "%.4d,%.2d,%.2d,%.2d,%.2d,%.2d",
            vtime.getYear(), vtime.getMonth(), vtime.getDay(),
            vtime.getHour(), vtime.getMin(), vtime.getSec());
    _addCommand("-valid_time", argVec);
    argVec.push_back(tmpStr);
    
  }

  _addCommand("-rap_data_dir", argVec);
  argVec.push_back(RapDataDir.location());
  
  string absDir;
  RapDataDir.fillPath(_inputDir, absDir);
  
  _addCommand("-abs_dir_path", argVec);
  argVec.push_back(absDir);

  string relDir;
  RapDataDir.stripPath(_inputDir, relDir);

  _addCommand("-rel_dir", argVec);
  argVec.push_back(relDir);

  _addCommand("-rel_data_path", argVec);
  argVec.push_back(ldata.getRelDataPath());

  _addCommand("-file_ext", argVec);
  if (ldata.getDataFileExt().size() > 0) {
    argVec.push_back(ldata.getDataFileExt());
  } else {
    argVec.push_back("none");
  }

  _addCommand("-data_type", argVec);
  argVec.push_back(ldata.getDataType());
  
  _addCommand("-user_info1", argVec);
  argVec.push_back(ldata.getUserInfo1());

  _addCommand("-user_info2", argVec);
  argVec.push_back(ldata.getUserInfo2());
  
  _addCommand("-writer", argVec);
  argVec.push_back(ldata.getWriter());

  if (include_late_data_secs) {
    _addCommand("-data_late_secs", argVec);
    sprintf(tmpStr, "%d", _lateDataSecs);
    argVec.push_back(tmpStr);
  }

  for (size_t ii = 0; ii < _trailingArgs.size(); ii++) {
    argVec.push_back(_trailingArgs[ii]);
  }

  // set up execvp args - this is a null-terminated array of strings
  
  const char **args = new const char*[argVec.size() + 1];
  for (size_t ii = 0; ii < argVec.size(); ii++) {
    args[ii] = argVec[ii].c_str();
  }
  args[argVec.size()] = NULL;
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Calling execvp with following args:" << endl;
    for (size_t ii = 0; ii < argVec.size(); ii++) {
      cerr << "  " << argVec[ii] << endl;
    }
  }
    
  // execute the command
  
  execvp(args[0], (char **) args);
  delete[] args;
  
}

////////////////////////////////////
// execute the script - command line
// using specified args only

void DataSet::_execScriptSpecifiedArgs(const LdataInfo &ldata,
                                       const char *script_to_call)
					  
{

  time_t validTime = ldata.getLatestTime();
  DateTime vtime(validTime);
  
  int leadSecs = ldata.getLeadTime();
  time_t genTime = ldata.getLatestTime();
  time_t forecastValidTime = genTime + leadSecs;
  DateTime gtime(genTime);
  DateTime fvtime(forecastValidTime);

  string absDir;
  RapDataDir.fillPath(_inputDir, absDir);
  
  string relDir;
  RapDataDir.stripPath(_inputDir, relDir);

  vector<string> argVec;
  char tmpStr[1024];
  argVec.push_back(script_to_call);

  // specified args in order

  for (int iarg = 0; iarg < _params.script_args_n; iarg++) {

    switch (_params._script_args[iarg]) {

      case Params::ARG_VALID_UTIME:
        if (ldata.isFcast()) {
          sprintf(tmpStr, "%ld", forecastValidTime);
        } else {
          sprintf(tmpStr, "%ld", validTime);
        }
        _addCommand("-valid_utime", argVec);
        argVec.push_back(tmpStr);
        break;
      case Params::ARG_VALID_TIME:
        if (ldata.isFcast()) {
          sprintf(tmpStr, "%.4d,%.2d,%.2d,%.2d,%.2d,%.2d",
                  fvtime.getYear(), fvtime.getMonth(), fvtime.getDay(),
                  fvtime.getHour(), fvtime.getMin(), fvtime.getSec());
        } else {
          sprintf(tmpStr, "%.4d,%.2d,%.2d,%.2d,%.2d,%.2d",
                  vtime.getYear(), vtime.getMonth(), vtime.getDay(),
                  vtime.getHour(), vtime.getMin(), vtime.getSec());
        }
        _addCommand("-valid_time", argVec);
        argVec.push_back(tmpStr);
        break;
      case Params::ARG_IS_FORECAST:
        _addCommand("-is_forecast", argVec);
        if (ldata.isFcast()) {
          argVec.push_back("true");
        } else {
          argVec.push_back("false");
        }
        break;
      case Params::ARG_FORECAST_LEAD_SECS:
        _addCommand("-forecast_lead_secs", argVec);
        sprintf(tmpStr, "%d", leadSecs);
        argVec.push_back(tmpStr);
        break;
      case Params::ARG_GEN_UTIME:
        sprintf(tmpStr, "%ld", genTime);
        _addCommand("-gen_utime", argVec);
        argVec.push_back(tmpStr);
        break;
      case Params::ARG_GEN_TIME:
        sprintf(tmpStr, "%.4d,%.2d,%.2d,%.2d,%.2d,%.2d",
                gtime.getYear(), gtime.getMonth(), gtime.getDay(),
                gtime.getHour(), gtime.getMin(), gtime.getSec());
        _addCommand("-gen_time", argVec);
        argVec.push_back(tmpStr);
        break;
      case Params::ARG_RAP_DATA_DIR:
        _addCommand("-rap_data_dir", argVec);
        argVec.push_back(RapDataDir.location());
        break;
      case Params::ARG_ABS_DIR_PATH:
        _addCommand("-abs_dir_path", argVec);
        argVec.push_back(absDir);
        break;
      case Params::ARG_REL_DIR:
        _addCommand("-rel_dir", argVec);
        argVec.push_back(relDir);
        break;
      case Params::ARG_REL_DATA_PATH:
        _addCommand("-rel_data_path", argVec);
        argVec.push_back(ldata.getRelDataPath());
        break;
      case Params::ARG_FILE_EXT:
        _addCommand("-file_ext", argVec);
        if (ldata.getDataFileExt().size() > 0) {
          argVec.push_back(ldata.getDataFileExt());
        } else {
          argVec.push_back("none");
        }
        break;
      case Params::ARG_DATA_TYPE:
        _addCommand("-data_type", argVec);
        argVec.push_back(ldata.getDataType());
        break;
      case Params::ARG_USER_INFO1:
        _addCommand("-user_info1", argVec);
        argVec.push_back(ldata.getUserInfo1());
        break;
      case Params::ARG_USER_INFO2:
        _addCommand("-user_info2", argVec);
        argVec.push_back(ldata.getUserInfo2());
        break;
      case Params::ARG_WRITER:
        _addCommand("-writer", argVec);
        argVec.push_back(ldata.getWriter());
        break;
      case Params::ARG_DATA_LATE_SECS:
        _addCommand("-data_late_secs", argVec);
        sprintf(tmpStr, "%d", _lateDataSecs);
        argVec.push_back(tmpStr);
        break;
      case Params::ARG_DEBUG:
        if (_params.debug) {
          _addCommand("-debug", argVec);
        }
        break;
      case Params::ARG_VERBOSE:
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          _addCommand("-verbose", argVec);
        }
        break;

    } // switch

  } // iarg

  // trailing args

  for (size_t ii = 0; ii < _trailingArgs.size(); ii++) {
    argVec.push_back(_trailingArgs[ii]);
  }

  // set up execvp args - this is a null-terminated array of strings
  
  const char **args = new const char*[argVec.size() + 1];
  for (size_t ii = 0; ii < argVec.size(); ii++) {
    args[ii] = argVec[ii].c_str();
  }
  args[argVec.size()] = NULL;
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Calling execvp with following args:" << endl;
    for (size_t ii = 0; ii < argVec.size(); ii++) {
      cerr << "  " << argVec[ii] << endl;
    }
  }
    
  // execute the command
  
  execvp(args[0], (char **) args);
  delete[] args;
  
}

//////////////////////////////////
// add command argument to vector

void DataSet::_addCommand(const string &command,
                          vector<string> &argVec)

{
  
  string commandStr;
  if (_params.generate_double_dash_args) {
    commandStr += "-";
  }
  commandStr += command;
  argVec.push_back(commandStr);

}

////////////////////////////
// reap children as required
//

void DataSet::_reapChildren()

{

  // reap any children which have died

  bool checkAgain = true;
  while (checkAgain) {
    
    checkAgain = false;

    for (activeMap_t::iterator ii = _active.begin();
         ii != _active.end(); ii++) {

      pid_t pid = ii->first;
      int status;
      if (waitpid(pid, &status,
                  (int) (WNOHANG | WUNTRACED)) == pid) {
    
        if (_params.debug) {
          cerr << "Reaped child pid: " << pid << endl;
        }

        checkAgain = true;

        // remove from active map
        activeMap_t::iterator jj = _active.find(pid);
        if (jj != _active.end()) {
          _active.erase(jj);
        }

        break; // from ii loop into while loop
        
      } // if (waitpid ...

    } // ii

  } // while (checkAgain)

  // kill any children which have run too long
  
  for (activeMap_t::iterator ii = _active.begin(); ii != _active.end(); ii++) {
    _killAsRequired(ii->first, ii->second);
  }
  
}

//////////////////////////
// kill child as required
//

void DataSet::_killAsRequired(pid_t pid,
                              time_t terminate_time)
  
{
  
  if (!_params.terminate_script_if_hung) {
    return;
  }

  time_t now = time(NULL);

  if (now < terminate_time) {
    return;
  }

  // Time to terminate script, will be reaped elsewhere
  
  if (_params.debug) {
    cerr << "Child has run too long, pid: " << pid << endl;
    cerr << "  Sending child kill signal" << endl;
  }
  
  char pmuStr[4096];
  sprintf(pmuStr, "Killing pid %d", pid);
  PMU_force_register(pmuStr);
  
  if(kill(pid,SIGKILL)) {
    perror("kill: ");
  }

}

//////////////////////////
// kill remaining children
//

void DataSet::killRemainingChildren()

{
  
  // reap children which have died
  
  _reapChildren();

  // kill any child processes still running
  
  activeMap_t::iterator ii;
  for (ii = _active.begin(); ii != _active.end(); ii++) {
    pid_t pid = ii->first;
    if (_params.debug) {
      cerr << "  Program will exit, kill child, pid: " << pid << endl;
    }
    if(kill(pid,SIGKILL)) {
      perror("kill: ");
    }
  }

}

    

