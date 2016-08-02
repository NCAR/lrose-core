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
// LdataWatcher.cc
//
// LdataWatcher object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 1999
//
///////////////////////////////////////////////////////////////

#include <iostream>
#include <signal.h>
#include <sys/wait.h>
#include <cstring>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <toolsa/TaArray.hh>
#include <didss/RapDataDir.hh>
#include <dsserver/DmapAccess.hh>
#include "LdataWatcher.hh"
using namespace std;

// Constructor

LdataWatcher::LdataWatcher(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "LdataWatcher";
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
		_params.procmap_register_interval);

  return;

}

// destructor

LdataWatcher::~LdataWatcher()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int LdataWatcher::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // Set up LdataInfo

  LdataInfo *ldata = NULL;
  if (_params.use_url) {
    ldata = new DsLdataInfo(_params.inputUrl,
                            _params.debug >= Params::DEBUG_VERBOSE);
  } else {
    ldata = new LdataInfo(_params.inputPath,
                          _params.debug >= Params::DEBUG_VERBOSE);
  }
  
  if (_params.write_fmq) {
    // do NOT use the FMQ - since we will be writing that
    ldata->setUseFmq(false);
  }

  if (_params.read_ldata_fmq_from_start) {
    ldata->setReadFmqFromStart(true);
  }
  if (_params.save_read_state) {
    string label = "LdataWatcher_";
    label += _params.instance;
    ldata->setSaveLatestReadInfo(label, _params.max_realtime_valid_age);
  }

  // compute directory path relative to RAP_DATA_DIR

  string relDir;
  RapDataDir.stripPath(_params.inputPath, relDir);
  time_t timeLastCall = time(NULL) - _params.script_min_interval_secs;
  time_t timeLastAction = time(NULL);
  bool callPending = false;
  LdataInfo ldataPending;
  time_t timeLastPrint = 0;
  
  // wait for data
  
  while (true) {
    
    if (ldata->read(_params.max_realtime_valid_age) == 0) {
      
      // found new data
      
      PMU_force_register(utimstr(ldata->getLatestTime()));
      timeLastAction = time(NULL);
      timeLastPrint = 0;
      
      if (_params.debug) {
        cerr << "=======================================================" << endl;
	cerr << "LdataWatcher - got latest data" << endl;
        cerr << "Data path: " << ldata->getDataPath() << endl;
        cerr << "-------------------------------------------------------" << endl;
	ldata->printAsXml(cerr);
        cerr << "=======================================================" << endl;
      }
      
      if (_params.register_with_datamapper) {
	
	DmapAccess access;
	if (access.regLatestInfo(ldata->getLatestTime(),
				 relDir, _params.data_type)) {
	  cerr << "ERROR - LdataWatcher::Run." << endl;
	  cerr << "  Failed to register with data mapper." << endl;
	  cerr << "  Dir: " << relDir << endl;
	  time_t now = time(NULL);
	  cerr << "  UTC: " << utimstr(now) << endl;
	}
	
      }
      
      if (_params.write_fmq) {
	if (_params.debug) {
	  cerr << "LdataWatcher - about to write fmq" << endl;
	}
	if (ldata->writeFmq()) {
	  cerr << "ERROR - LdataWatcher::Run." << endl;
	  cerr << "  Could not write FMQ." << endl;
	  cerr << "  Dir: " << relDir << endl;
	  time_t now = time(NULL);
	  cerr << "  UTC: " << utimstr(now) << endl;
	}
      }
      
      // call script as required
      
      if (_params.call_script) {
	time_t now = time(NULL);
	int elapsedSecs = now - timeLastCall;
	int nsecsWait = _params.script_min_interval_secs - elapsedSecs;
	if (nsecsWait <= 0) {
	  _callDataArrivedScript(*ldata);
	  callPending = false;
	  timeLastCall = now;
	} else {
	  if (_params.debug) {
	    cerr << "Not enough time has elapsed for calling script" << endl;
	    cerr << "  Min interval secs: " << _params.script_min_interval_secs;
	    cerr << "  Elapsed time since last call: " << elapsedSecs << endl;
	    cerr << "  Will call pending script in secs: " << nsecsWait << endl;
	  }
	  callPending = true;
	  ldataPending = *ldata;
	}
      }

    } else {

      // data  did not arrive

      if (_params.debug >= Params::DEBUG_VERBOSE) {
        time_t now = time(NULL);
        if ((now - timeLastPrint > 120) || (now % 60 == 0)) {
          cerr << "LdataWatcher - waiting for latest data at "
               << utimstr(now) << " Zzzzz ....." << endl;
          timeLastPrint = now;
        }
      }
      
      // check for pending call

      if (_params.call_script && callPending) {
	time_t now = time(NULL);
	int lapsedSecs = now - timeLastCall;
	if (lapsedSecs >= _params.script_min_interval_secs) {
	  if (_params.debug) {
	    cerr << "Calling pending script" << endl;
	  }
	  _callDataArrivedScript(ldataPending);
	  callPending = false;
	  timeLastCall = now;
	}
      }

      // check for late data
      
      if (_params.call_data_late_script) {

	time_t now = time(NULL);
	int timeSinceLastData = now - timeLastAction;
	if (timeSinceLastData > _params.data_late_secs) {
	  _callDataLateScript(*ldata);
	  timeLastAction = now;
	}

      } // if (_params.call_data_late_script)

      umsleep(1000);

    } // if (ldata->read ...
    
    // reap or kill child processes as required
    
    _reapChildren();

    // register with procmap
    
    PMU_auto_register("Waiting for data");
    
  } // while

  delete ldata;

  return 0;

}

////////////////////////////////
// call script when data arrives

void LdataWatcher::_callDataArrivedScript (const LdataInfo &ldata)
  
{

  _callScript(_params.run_script_in_background,
	      ldata,
	      _params.script_to_call,
	      false);

}

////////////////////////////////
// Add the specified script to the argument list.  Parse through the
// script pulling out command line options so they can be included
// properly in the argument list.

void LdataWatcher::_addScriptToArgVec(const char *script_to_call,
				      vector< string > &arg_vec) const
{
  // Make a copy of the original string so we don't corrupt it

  char *script_copy = new char[strlen(script_to_call) + 1];
  memcpy(script_copy, script_to_call, strlen(script_to_call) + 1);
  
  char *token;
  
  arg_vec.push_back(strtok(script_copy, " "));

  while ((token = strtok(0, " ")) != 0)
    arg_vec.push_back(token);
  
  if (_params.debug)
  {
    cerr << "Token list from script_to_call:" << endl;
    
    vector< string >::const_iterator arg;
    
    for (arg = arg_vec.begin(); arg != arg_vec.end(); ++arg)
      cerr << "   " << *arg << endl;
  }
  
}

////////////////////////////////
// call script when data is late

void LdataWatcher::_callDataLateScript (const LdataInfo &ldata)
  
{

  _callScript(true,
	      ldata,
	      _params.data_late_script,
	      true);
  
}

//////////////
// call script

void LdataWatcher::_callScript (bool run_in_background,
				const LdataInfo &ldata,
				const char *script_to_call,
				bool include_data_late_secs)
  
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
    
    switch (_params.script_style)
    {
    case Params::SCRIPT_WITH_ORDERED_ARGS :
      _execOrderedArgScript(ldata,
			    script_to_call,
			    _params.include_input_path,
			    include_data_late_secs);
      break;

    case Params::SCRIPT_WITH_COMMAND_LINE_OPTIONS :
      _execCommandLineScript(ldata, script_to_call, "-");
      break;

    case Params::SCRIPT_WITH_COMMAND_LINE_OPTIONS_DOUBLE_DASH :
      _execCommandLineScript(ldata, script_to_call, "--");
      break;

    case Params::SCRIPT_WITH_SPECIFIED_ORDERED_ARGS :
      _execSpecifiedOptionsScript(ldata, script_to_call, "");
      break;

    case Params::SCRIPT_WITH_SPECIFIED_OPTIONS :
      _execSpecifiedOptionsScript(ldata, script_to_call, "-");
      break;

    case Params::SCRIPT_WITH_SPECIFIED_OPTIONS_DOUBLE_DASH :
      _execSpecifiedOptionsScript(ldata, script_to_call, "--");
      break;
    } /* endswitch - _params.script_style */

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

      // sleep for a sec
      
      umsleep(1000);
      
    } // while
    
  } // if (run_script_in_background)

}

////////////////////////////////////
// execute the script - ordered args

void LdataWatcher::_execOrderedArgScript(const LdataInfo &ldata,
					 const char *script_to_call,
					 bool include_input_path,
					 bool include_data_late_secs)

{

  // load vector of strings for args

  vector<string> argVec;
  char tmpStr[1024];

  _addScriptToArgVec(script_to_call, argVec);

  if (include_data_late_secs) {
    sprintf(tmpStr, "%d", _params.data_late_secs);
    argVec.push_back(tmpStr);
  }

  string relDir;
  if (include_input_path) {
    RapDataDir.stripPath(_params.inputPath, relDir);
    argVec.push_back(RapDataDir.location());
    argVec.push_back(relDir);
  }

  sprintf(tmpStr, "%ld", ldata.getLatestTime());
  argVec.push_back(tmpStr);

  const date_time_t &latest = ldata.getLatestTimeStruct();

  sprintf(tmpStr, "%.4d", latest.year);
  argVec.push_back(tmpStr);

  sprintf(tmpStr, "%.02d", latest.month);
  argVec.push_back(tmpStr);

  sprintf(tmpStr, "%.02d", latest.day);
  argVec.push_back(tmpStr);

  sprintf(tmpStr, "%.02d", latest.hour);
  argVec.push_back(tmpStr);

  sprintf(tmpStr, "%.02d", latest.min);
  argVec.push_back(tmpStr);

  sprintf(tmpStr, "%.02d", latest.sec);
  argVec.push_back(tmpStr);

  if (ldata.getDataFileExt().size() > 0) {
    argVec.push_back(ldata.getDataFileExt());
  } else {
    argVec.push_back("none");
  }

  if (ldata.getUserInfo1().size() > 0) {
    argVec.push_back(ldata.getUserInfo1());
  } else {
    if (ldata.getWriter().size() > 0) {
      argVec.push_back(ldata.getWriter());
    } else {
      argVec.push_back("unknown");
    }
  }
  if (ldata.getUserInfo2().size() > 0) {
    argVec.push_back(ldata.getUserInfo2());
  } else {
    if (ldata.getRelDataPath().size() > 0) {
      argVec.push_back(ldata.getRelDataPath());
    } else {
      argVec.push_back("unknown");
    }
  }

  if (ldata.isFcast()) {
    argVec.push_back("1");
  } else {
    argVec.push_back("0");
  }
  
  if (ldata.isFcast()) {
    sprintf(tmpStr, " %d", ldata.getLeadTime());
    argVec.push_back(tmpStr);
  }
  
  // set up execvp args - this is a null-terminated array of strings

  TaArray<const char *> args_;
  const char **args = args_.alloc(argVec.size() + 1);
  for (size_t ii = 0; ii < argVec.size(); ii++) {
    args[ii] = argVec[ii].c_str();
  }
  args[argVec.size()] = NULL;

  if (_params.debug) {
    cerr << "Calling execvp with following args:" << endl;
    for (size_t ii = 0; ii < argVec.size(); ii++) {
      cerr << "  " << argVec[ii];
      if (ii != argVec.size() - 1) {
        cerr << " \\" << endl;
      } else {
        cerr << endl;
      }
    }
  }
    
  // execute the command
  
  execvp(args[0], (char **) args);
  
}

////////////////////////////////////
// execute the script - command line

void LdataWatcher::_execCommandLineScript(const LdataInfo &ldata,
					  const char *script_to_call,
					  const char *dash_string)
					  
{

  // load vector of strings for args

  vector<string> argVec;
  char tmpStr[1024];
  char tagStr[1024];
  
  _addScriptToArgVec(script_to_call, argVec);

  sprintf(tagStr, "%s%s", dash_string, "unix_time");
  sprintf(tmpStr, "%ld", ldata.getLatestTime());
  argVec.push_back(tagStr);
  argVec.push_back(tmpStr);

  const date_time_t &latest = ldata.getLatestTimeStruct();

  sprintf(tagStr, "%s%s", dash_string, "year");
  sprintf(tmpStr, "%.4d", latest.year);
  argVec.push_back(tagStr);
  argVec.push_back(tmpStr);

  sprintf(tagStr, "%s%s", dash_string, "month");
  sprintf(tmpStr, "%.02d", latest.month);
  argVec.push_back(tagStr);
  argVec.push_back(tmpStr);

  sprintf(tagStr, "%s%s", dash_string, "day");
  sprintf(tmpStr, "%.02d", latest.day);
  argVec.push_back(tagStr);
  argVec.push_back(tmpStr);

  sprintf(tagStr, "%s%s", dash_string, "hour");
  sprintf(tmpStr, "%.02d", latest.hour);
  argVec.push_back(tagStr);
  argVec.push_back(tmpStr);

  sprintf(tagStr, "%s%s", dash_string, "min");
  sprintf(tmpStr, "%.02d", latest.min);
  argVec.push_back(tagStr);
  argVec.push_back(tmpStr);

  sprintf(tagStr, "%s%s", dash_string, "sec");
  sprintf(tmpStr, "%.02d", latest.sec);
  argVec.push_back(tagStr);
  argVec.push_back(tmpStr);

  sprintf(tagStr, "%s%s", dash_string, "rap_data_dir");
  argVec.push_back(tagStr);
  argVec.push_back(RapDataDir.location());

  string fullPath;
  RapDataDir.fillPath(_params.inputPath, fullPath);

  sprintf(tagStr, "%s%s", dash_string, "full_path");
  argVec.push_back(tagStr);
  argVec.push_back(fullPath);

  sprintf(tagStr, "%s%s", dash_string, "abs_dir_path");
  argVec.push_back(tagStr);
  argVec.push_back(fullPath);

  string subDir;
  RapDataDir.stripPath(_params.inputPath, subDir);

  sprintf(tagStr, "%s%s", dash_string, "sub_dir");
  argVec.push_back(tagStr);
  argVec.push_back(subDir);

  sprintf(tagStr, "%s%s", dash_string, "rel_dir");
  argVec.push_back(tagStr);
  argVec.push_back(subDir);

  sprintf(tagStr, "%s%s", dash_string, "rel_data_path");
  argVec.push_back(tagStr);
  argVec.push_back(ldata.getRelDataPath());

  sprintf(tagStr, "%s%s", dash_string, "file_ext");
  argVec.push_back(tagStr);
  if (ldata.getDataFileExt().size() > 0) {
    argVec.push_back(ldata.getDataFileExt());
  } else {
    argVec.push_back("none");
  }

  sprintf(tagStr, "%s%s", dash_string, "data_type");
  argVec.push_back(tagStr);
  argVec.push_back(ldata.getDataType());

  sprintf(tagStr, "%s%s", dash_string, "user_info1");
  argVec.push_back(tagStr);
  argVec.push_back(ldata.getUserInfo1());

  sprintf(tagStr, "%s%s", dash_string, "user_info2");
  argVec.push_back(tagStr);
  argVec.push_back(ldata.getUserInfo2());
  
  sprintf(tagStr, "%s%s", dash_string, "is_forecast");
  argVec.push_back(tagStr);
  if (ldata.isFcast()) {
    argVec.push_back("true");
  } else {
    argVec.push_back("false");
  }
  
  sprintf(tagStr, "%s%s", dash_string, "forecast_lead_secs");
  sprintf(tmpStr, "%d", ldata.getLeadTime());
  argVec.push_back(tagStr);
  argVec.push_back(tmpStr);
  
  sprintf(tagStr, "%s%s", dash_string, "writer");
  argVec.push_back(tagStr);
  argVec.push_back(ldata.getWriter());

  sprintf(tagStr, "%s%s", dash_string, "data_late_secs");
  argVec.push_back(tagStr);
  sprintf(tmpStr, "%d", _params.data_late_secs);
  argVec.push_back(tmpStr);

  if (_params.debug) {
    sprintf(tagStr, "%s%s", dash_string, "debug");
    argVec.push_back(tagStr);
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    sprintf(tagStr, "%s%s", dash_string, "verbose");
    argVec.push_back(tagStr);
  }

  // add supplementary args if specified

  for (int ii = 0; ii < _params.supplementary_args_n; ii++) {
    argVec.push_back(_params._supplementary_args[ii]);
  }
  
  // set up execvp args - this is a null-terminated array of strings

  TaArray<const char *> args_;
  const char **args = args_.alloc(argVec.size() + 1);
  for (size_t ii = 0; ii < argVec.size(); ii++) {
    args[ii] = argVec[ii].c_str();
  }
  args[argVec.size()] = NULL;

  if (_params.debug) {
    cerr << "Calling execvp with following args:" << endl;
    for (size_t ii = 0; ii < argVec.size(); ii++) {
      cerr << "  " << argVec[ii];
      if (ii != argVec.size() - 1) {
        cerr << " \\" << endl;
      } else {
        cerr << endl;
      }
    }
  }
    
  // execute the command
  
  execvp(args[0], (char **) args);
  
}

////////////////////////////////////
// execute the script - specified options

void LdataWatcher::_execSpecifiedOptionsScript(const LdataInfo &ldata,
					       const char *script_to_call,
					       const char *dash_string)
					  
{
  // load vector of strings for args

  vector<string> argVec;
  char tmpStr[1024];
  char tagStr[1024];
  
  _addScriptToArgVec(script_to_call, argVec);

  // Access data possibly needed in the options list

  const date_time_t &latest = ldata.getLatestTimeStruct();

  string fullPath;
  RapDataDir.fillPath(_params.inputPath, fullPath);

  string subDir;
  RapDataDir.stripPath(_params.inputPath, subDir);

  // Fill in the specified options

  for (int i = 0; i < _params.script_options_n; ++i)
  {
    switch(_params._script_options[i])
    {
    case Params::OPTION_UNIX_TIME :
      if (dash_string[0] != '\0')
      {
	sprintf(tagStr, "%s%s", dash_string, "unix_time");
	argVec.push_back(tagStr);
      }
      sprintf(tmpStr, "%ld", ldata.getLatestTime());
      argVec.push_back(tmpStr);
      break;

    case Params::OPTION_YEAR :
      if (dash_string[0] != '\0')
      {
	sprintf(tagStr, "%s%s", dash_string, "year");
	argVec.push_back(tagStr);
      }
      sprintf(tmpStr, "%.4d", latest.year);
      argVec.push_back(tmpStr);
      break;
      
    case Params::OPTION_MONTH :
      if (dash_string[0] != '\0')
      {
	sprintf(tagStr, "%s%s", dash_string, "month");
	argVec.push_back(tagStr);
      }
      sprintf(tmpStr, "%.02d", latest.month);
      argVec.push_back(tmpStr);
      break;
      
    case Params::OPTION_DAY :
      if (dash_string[0] != '\0')
      {
	sprintf(tagStr, "%s%s", dash_string, "day");
	argVec.push_back(tagStr);
      }
      sprintf(tmpStr, "%.02d", latest.day);
      argVec.push_back(tmpStr);
      break;
      
    case Params::OPTION_HOUR :
      if (dash_string[0] != '\0')
      {
	sprintf(tagStr, "%s%s", dash_string, "hour");
	argVec.push_back(tagStr);
      }
      sprintf(tmpStr, "%.02d", latest.hour);
      argVec.push_back(tmpStr);
      break;
      
    case Params::OPTION_MIN :
      if (dash_string[0] != '\0')
      {
	sprintf(tagStr, "%s%s", dash_string, "min");
	argVec.push_back(tagStr);
      }
      sprintf(tmpStr, "%.02d", latest.min);
      argVec.push_back(tmpStr);
      break;
      
    case Params::OPTION_SEC :
      if (dash_string[0] != '\0')
      {
	sprintf(tagStr, "%s%s", dash_string, "sec");
	argVec.push_back(tagStr);
      }
      sprintf(tmpStr, "%.02d", latest.sec);
      argVec.push_back(tmpStr);
      break;
      
    case Params::OPTION_RAP_DATA_DIR :
      if (dash_string[0] != '\0')
      {
	sprintf(tagStr, "%s%s", dash_string, "rap_data_dir");
	argVec.push_back(tagStr);
      }
      argVec.push_back(RapDataDir.location());
      break;

    case Params::OPTION_FULL_PATH :
      if (dash_string[0] != '\0')
      {
	sprintf(tagStr, "%s%s", dash_string, "full_path");
	argVec.push_back(tagStr);
      }
      argVec.push_back(fullPath);
      break;
      
    case Params::OPTION_ABS_DIR_PATH :
      if (dash_string[0] != '\0')
      {
	sprintf(tagStr, "%s%s", dash_string, "abs_dir_path");
	argVec.push_back(tagStr);
      }
      argVec.push_back(fullPath);
      break;
      
    case Params::OPTION_SUB_DIR :
      if (dash_string[0] != '\0')
      {
	sprintf(tagStr, "%s%s", dash_string, "sub_dir");
	argVec.push_back(tagStr);
      }
      argVec.push_back(subDir);
      break;
      
    case Params::OPTION_REL_DIR :
      if (dash_string[0] != '\0')
      {
	sprintf(tagStr, "%s%s", dash_string, "rel_dir");
	argVec.push_back(tagStr);
      }
      argVec.push_back(subDir);
      break;
      
    case Params::OPTION_REL_DATA_PATH :
      if (dash_string[0] != '\0')
      {
	sprintf(tagStr, "%s%s", dash_string, "rel_data_path");
	argVec.push_back(tagStr);
      }
      argVec.push_back(ldata.getRelDataPath());
      break;
      
    case Params::OPTION_FULL_FILE_PATH :
      if (dash_string[0] != '\0')
      {
	sprintf(tagStr, "%s%s", dash_string, "full_file_path");
	argVec.push_back(tagStr);
      }
      argVec.push_back(fullPath + "/" + ldata.getRelDataPath());
      break;
      
    case Params::OPTION_REL_FILE_PATH :
      if (dash_string[0] != '\0')
      {
	sprintf(tagStr, "%s%s", dash_string, "rel_file_path");
	argVec.push_back(tagStr);
      }
      argVec.push_back(subDir + "/" + ldata.getRelDataPath());
      break;
      
    case Params::OPTION_FILE_NAME :
      if (dash_string[0] != '\0')
      {
	sprintf(tagStr, "%s%s", dash_string, "file_name");
	argVec.push_back(tagStr);
      }
      argVec.push_back(Path(fullPath + "/" + ldata.getRelDataPath()).getFile());
      break;
      
    case Params::OPTION_FILE_EXT :
      if (dash_string[0] != '\0')
      {
	sprintf(tagStr, "%s%s", dash_string, "file_ext");
	argVec.push_back(tagStr);
      }
      if (ldata.getDataFileExt().size() > 0) {
	argVec.push_back(ldata.getDataFileExt());
      } else {
	argVec.push_back("none");
      }
      break;
      
    case Params::OPTION_DATA_TYPE :
      if (dash_string[0] != '\0')
      {
	sprintf(tagStr, "%s%s", dash_string, "data_type");
	argVec.push_back(tagStr);
      }
      argVec.push_back(ldata.getDataType());
      break;
      
    case Params::OPTION_USER_INFO1 :
      if (dash_string[0] != '\0')
      {
	sprintf(tagStr, "%s%s", dash_string, "user_info1");
	argVec.push_back(tagStr);
      }
      argVec.push_back(ldata.getUserInfo1());
      break;
      
    case Params::OPTION_USER_INFO2 :
      if (dash_string[0] != '\0')
      {
	sprintf(tagStr, "%s%s", dash_string, "user_info2");
	argVec.push_back(tagStr);
      }
      argVec.push_back(ldata.getUserInfo2());
      break;
      
    case Params::OPTION_IS_FORECAST :
      if (dash_string[0] != '\0')
      {
	sprintf(tagStr, "%s%s", dash_string, "is_forecast");
	argVec.push_back(tagStr);
      }
      if (ldata.isFcast()) {
	argVec.push_back("true");
      } else {
	argVec.push_back("false");
      }
      break;
      
    case Params::OPTION_FORECAST_LEAD_SECS :
      if (dash_string[0] != '\0')
      {
	sprintf(tagStr, "%s%s", dash_string, "forecast_lead_secs");
	argVec.push_back(tagStr);
      }
      sprintf(tmpStr, "%d", ldata.getLeadTime());
      argVec.push_back(tmpStr);
      break;
      
    case Params::OPTION_WRITER :
      if (dash_string[0] != '\0')
      {
	sprintf(tagStr, "%s%s", dash_string, "writer");
	argVec.push_back(tagStr);
      }
      argVec.push_back(ldata.getWriter());
      break;
      
    case Params::OPTION_DATA_LATE_SECS :
      if (dash_string[0] != '\0')
      {
	sprintf(tagStr, "%s%s", dash_string, "data_late_secs");
	argVec.push_back(tagStr);
      }
      sprintf(tmpStr, "%d", _params.data_late_secs);
      argVec.push_back(tmpStr);
      break;
      
    case Params::OPTION_DEBUG :
      if (_params.debug && dash_string[0] != '\0') {
	sprintf(tagStr, "%s%s", dash_string, "debug");
	argVec.push_back(tagStr);
      }
      break;
      
    case Params::OPTION_VERBOSE :
      if (_params.debug >= Params::DEBUG_VERBOSE &&
	  dash_string[0] != '\0') {
	sprintf(tagStr, "%s%s", dash_string, "verbose");
	argVec.push_back(tagStr);
      }
      break;
    } /* endswitch - _params._script_options[i] */
    
  } /* endfor - i */
  
  // add supplementary args if specified

  for (int ii = 0; ii < _params.supplementary_args_n; ii++) {
    argVec.push_back(_params._supplementary_args[ii]);
  }
  
  // set up execvp args - this is a null-terminated array of strings

  TaArray<const char *> args_;
  const char **args = args_.alloc(argVec.size() + 1);
  for (size_t ii = 0; ii < argVec.size(); ii++) {
    args[ii] = argVec[ii].c_str();
  }
  args[argVec.size()] = NULL;

  if (_params.debug) {
    cerr << "Calling execvp with following args:" << endl;
    for (size_t ii = 0; ii < argVec.size(); ii++) {
      cerr << "  " << argVec[ii];
      if (ii != argVec.size() - 1) {
        cerr << " \\" << endl;
      } else {
        cerr << endl;
      }
    }
  }
    
  // execute the command
  
  execvp(args[0], (char **) args);
  
}

////////////////////////////
// reap children as required
//

void LdataWatcher::_reapChildren()

{

  // reap any children which have died
  
  pid_t deadPid;
  int status;
  while ((deadPid = waitpid((pid_t) -1, &status,
			    (int) (WNOHANG | WUNTRACED))) > 0) {
    
    if (_params.debug) {
      cerr << "Reaped child: " << deadPid << endl;
    }
    
    // remove from active map
    activeMap_t::iterator ii = _active.find(deadPid);
    if (ii != _active.end()) {
      _active.erase(ii);
    }
    
  } // while
  
  // kill any children which have run too long
  
  activeMap_t::iterator ii;
  for (ii = _active.begin(); ii != _active.end(); ii++) {
    _killAsRequired(ii->first, ii->second);
  }
  
}

//////////////////////////
// kill child as required
//

void LdataWatcher::_killAsRequired(pid_t pid,
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

void LdataWatcher::killRemainingChildren()

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

    

