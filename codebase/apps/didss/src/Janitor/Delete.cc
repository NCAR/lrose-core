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
//////////////////////////////////////////////////////////
// Delete.cc: handle file deletion
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2016
//
/////////////////////////////////////////////////////////////

#include <csignal>
#include <cstdlib>
#include <unistd.h>
#include <cerrno>
#include <sys/wait.h>
#include <sys/stat.h>
#include <didss/DataFileNames.hh>  // For looking at files generally
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/pmu.h>
#include <toolsa/uusleep.h>

#include "Delete.hh"
#include "Params.hh"
using namespace std;

///////////////
// Constructor

Delete::Delete (const string &progName,
                const Params *params) :
        _progName(progName),
        _params(params)

{

}

/////////////
// Destructor

Delete::~Delete ()
{
  _killRemainingChildren();
}
  

///////////////////////////////////////////////////////////////////
// remove specified file
//
// Returns 0 on success, -1 on failure.

int Delete::removeFile(const string &filePath)

{
  
  // get file mod time
  
  struct stat fstat;
  if (ta_stat(filePath.c_str(), &fstat)) {
    perror("stat failed");
    cerr << "ERROR - Delete::removeFile()" << endl;
    cerr << "  Cannot stat file: " << filePath << endl;
    return -1;
  }
  
  if (remove(filePath.c_str())) {
    perror("remove failed");
    cerr << "ERROR - Delete::removeFile()" << endl;
    cerr << "  Cannot delete file: " << filePath << endl;
    return -1;
  }
  
  if (_params->debug) {
    cerr << "DEBUG - removed file: " << filePath << endl;
  }
  
  if (_params->call_script_on_file_deletion) {
    _callScriptOnFileDeletion(filePath, fstat.st_mtime);
  }

  return 0;

}

///////////////////////////////////////////////////////////////////
// call script on file deletion
//
// Returns 0 on success, -1 on failure.

void Delete::_callScriptOnFileDeletion(const string &filePath,
                                       time_t modTime)
  
{

  Path fpath(filePath);

  // get file data time

  time_t dataTime = 0;
  bool dateOnly;
  if (DataFileNames::getDataTime(filePath, dataTime, dateOnly)) {
    dataTime = -1;
  }

  // set time strings

  char modTimeStr[32];
  sprintf(modTimeStr, "%ld", modTime);

  char dataTimeStr[32];
  sprintf(dataTimeStr, "%ld", dataTime);

  // build arg list
  
  vector<string> args;

  for (int ii = 0; ii < _params->delete_script_arg_list_n; ii++) {

    const Params::delete_script_arg_t &arg = _params->_delete_script_arg_list[ii];

    switch (arg.id) {
      case Params::DELETE_DATA_TIME:
        if (dataTime != -1) {
          args.push_back(arg.str);
          args.push_back(dataTimeStr);
        }
        break;
      case Params::DELETE_MOD_TIME:
        args.push_back(arg.str);
        args.push_back(modTimeStr);
        break;
      case Params::DELETE_DIR_PATH:
        args.push_back(arg.str);
        args.push_back(fpath.getDirectory());
        break;
      case Params::DELETE_FILE_PATH:
        args.push_back(arg.str);
        args.push_back(filePath);
        break;
      case Params::DELETE_FILE_NAME:
        args.push_back(arg.str);
        args.push_back(fpath.getFile());
        break;
      case Params::DELETE_FILE_EXT:
        if (fpath.getExt().size() > 0) {
          args.push_back(arg.str);
          args.push_back(fpath.getExt());
        }
        break;

    } // switch

  } // ii

  // add supplementary args

  for (int ii = 0; ii < _params->delete_supplementary_args_n; ii++) {
    args.push_back(_params->_delete_supplementary_args[ii]);
  }

  _callScript(_params->run_delete_script_in_background, args,
	      _params->script_to_call_on_file_deletion);

}

//////////////////
// call the script

void Delete::_callScript(bool run_in_background,
                         const vector<string> &args,
                         const string &script_to_call)
  
{
  
  char pmuStr[4096];
  sprintf(pmuStr, "Starting %s", script_to_call.c_str());
  PMU_force_register(pmuStr);

  if (_params->debug) {
    cerr << "Calling script: " << script_to_call << endl;
  }
  
  // Fork a child to run the script
  
  time_t start_time = time(NULL);
  time_t terminate_time = start_time + _params->delete_script_max_run_secs;
  pid_t childPid = fork();
  
  if (childPid == 0) {
    
    // this is the child process, so exec the script
    
    _execScript(args, script_to_call);
    
    // exit
    
    if (_params->debug) {
      cerr << "Child process exiting ..." << endl;
    }

    _exit(0);

  }

  // this is the parent

  if (_params->debug) {
    cerr << endl;
    cerr << "Script started, child pid: " << childPid << endl;
  }
  
  if (run_in_background) {
    
    // add the child to the map of active children
    
    activePair_t pp;
    pp.first = childPid;
    pp.second = terminate_time;
    _active.insert(_active.begin(), pp);

    if (_params->debug) {
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
		script_to_call.c_str(), runtime);
	PMU_force_register(pmuStr);
	if (_params->debug) {
	  cerr << "Child exited, pid: " << childPid << endl;
	  cerr << "  Runtime in secs: " << runtime << endl;
	}
	return;
      }
      
      // script is still running
      
      sprintf(pmuStr, "%s running", script_to_call.c_str());
      PMU_auto_register(pmuStr);
      
      // child still running - kill it as required

      _killAsRequired(childPid, terminate_time);

      // sleep for a bit
      
      umsleep(50);
      
    } // while
    
  } // if (run_script_in_background)

}

//////////////////////
// execute the script

void Delete::_execScript(const vector<string> &args,
                         const string &script_to_call)
  
{

  // set up execvp args - this is a null-terminated array of strings
  
  int narray = (int) args.size() + 2;
  TaArray<const char *> argArray_;
  const char **argArray = argArray_.alloc(narray);
  argArray[0] = script_to_call.c_str();
  for (int ii = 0; ii < (int) args.size(); ii++) {
    argArray[ii+1] = args[ii].c_str();
  }
  argArray[narray-1] = NULL;
  
  if (_params->debug) {
    cerr << "Calling execvp with following args:" << endl;
    for (int i = 0; i < narray; i++) {
      cerr << "  " << argArray[i] << endl;
    }
  }
    
  // execute the command
  
  execvp(argArray[0], (char **) argArray);
  
}

////////////////////////////
// reap children as required
//

void Delete::_reapChildren()

{

  // reap any children which have died
  
  pid_t deadPid;
  int status;
  while ((deadPid = waitpid((pid_t) -1, &status,
			    (int) (WNOHANG | WUNTRACED))) > 0) {
    
    if (_params->debug) {
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

void Delete::_killAsRequired(pid_t pid,
                             time_t terminate_time)

{
  
  if (!_params->terminate_delete_script_if_hung) {
    return;
  }

  time_t now = time(NULL);

  if (now < terminate_time) {
    return;
  }

  // Time to terminate script, will be reaped elsewhere
  
  if (_params->debug) {
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

void Delete::_killRemainingChildren()

{
  
  // reap children which have died
  
  _reapChildren();

  // kill any child processes still running
  
  activeMap_t::iterator ii;
  for (ii = _active.begin(); ii != _active.end(); ii++) {
    pid_t pid = ii->first;
    if (_params->debug) {
      cerr << "  Program will exit, kill child, pid: " << pid << endl;
    }
    if(kill(pid,SIGKILL)) {
      perror("kill: ");
    }
  }

}

