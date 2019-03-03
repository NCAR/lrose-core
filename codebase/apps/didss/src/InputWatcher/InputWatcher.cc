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
// InputWatcher.cc
//
// InputWatcher object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2003
//
///////////////////////////////////////////////////////////////
//
// InputWatcher watches for new data arriving in a directory.
// It optionally registers the latest time with the DataMapper,
// optionally copies the file to a file named with the modify time,
// and optionally runs a script.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <iostream>
#include <signal.h>
#include <sys/wait.h>
#include <cstring>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <toolsa/TaArray.hh>
#include <didss/RapDataDir.hh>
#include <dsserver/DsLdataInfo.hh>
#include <dsserver/DmapAccess.hh>
#include "InputWatcher.hh"
using namespace std;

// Constructor

InputWatcher::InputWatcher(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "InputWatcher";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char*)"unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
  }
  if (_params.latest_data_info_avail) {
    _params.write_latest_data_info_file = pFALSE;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance, _params.procmap_register_interval);

  return;

}

// destructor

InputWatcher::~InputWatcher()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int InputWatcher::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // Set up input path

  DsInputPath input(_progName,
		    _params.debug >= Params::DEBUG_EXTRA,
		    _params.input_dir,
		    _params.max_valid_age,
		    PMU_auto_register,
		    _params.latest_data_info_avail,
		    _params.latest_file_only);
  
  input.setFileQuiescence(_params.file_quiescence);
  input.setSearchExt(_params.search_ext);
  input.setSubString(_params.search_substr);
  input.setRecursion(_params.search_recursively);
  input.setMaxRecursionDepth(_params.max_recursion_depth);
  input.setMaxDirAge(_params.max_dir_age);
  input.setFollowLinks(_params.follow_links);
  input.setUseInotify(_params.use_inotify);
  if(_params.save_latest_read_time) {
    string label = _progName + "." + _params.instance;
    input.setSaveLatestReadInfo(label);
  }
  // input.setDirScanSleep(_params.wait_between_checks);
  time_t timeLastAction = time(NULL);

  if (_params.debug) {
    cerr << "InputWatcher - watching dir: " << _params.input_dir << endl;
    cerr << "  FileQuiescence: " << _params.file_quiescence << endl;
    cerr << "  SearchExt: " << _params.search_ext << endl;
    cerr << "  SubString: " << _params.search_substr << endl;
    cerr << "  Recursion: " << _params.search_recursively << endl;
    cerr << "  MaxRecursionDepth: " << _params.max_recursion_depth << endl;
    cerr << "  MaxDirAge: " << _params.max_dir_age << endl;
    cerr << "  FollowLinks: " << _params.follow_links << endl;
    cerr << "  UseInotify: " << _params.use_inotify << endl;
  }
  
  while(true) {
    // check for new data

    char *inputPath = input.next(false);

    if (_params.skip_to_latest_file) {
      // skip ahead to latest file
      char *latest;
      while ((latest = input.next(false)) != NULL) {
        inputPath = latest;
      }
    }
    
    if (inputPath != NULL) {

      if (_params.ignore_hidden == true) {
	if (_params.debug) {
	  cerr << "checking for a hidden file." << endl;
	}
	// find the location for the start of the file name
	char* ret_val = strrchr(inputPath, '/');
	if (ret_val != NULL) {
	  char* fn_start = ret_val + 1;
	  ret_val = strchr(fn_start, '.');
	  if ((ret_val - fn_start) == 0) {
	    if (_params.debug) {
	      cerr << "Ignoring file, it is a hidden file: " << inputPath << endl;
	    }
	    continue;
	  }
  	}
      }    
      
      if (strlen(_params.search_substr) > 0) {
	if (strstr(inputPath, _params.search_substr) == NULL) {
	  if (_params.debug) {
	    cerr << "Ignoring file, no substr: " << inputPath << endl;
	  }
	  continue;
	}
      }
    
      if (_params.debug) {
	cerr << "Processing file: " << inputPath << endl;
      }
      
      // found new data
      
      Path ipath(inputPath);
      timeLastAction = time(NULL);
      PMU_force_register(ipath.getFile().c_str());
      struct stat fileStat;
      int ret = ta_stat(inputPath, &fileStat);
      if (ret == -1 ) {
        if (_params.debug) {
           cerr << "stat of file failed: " << inputPath << endl;
        }
        continue;
      }
      time_t modTime = fileStat.st_mtime;
      
      // call script as required
      
      if (_params.call_data_arrived_script) {
	_callDataArrivedScript(inputPath, modTime);
      }

      // write latest data info file if required

      if (_params.write_latest_data_info_file) {
	_writeLdataInfoFile(inputPath, modTime);
      }

      // register with data mapper if required

      if (_params.register_with_datamapper) {
	_registerWithDataMapper(modTime);
      }
      
      // copy file if required
      
      if (_params.copy_to_time_stamped_file || _params.copy_using_original_name) {
	_copyFile(inputPath, modTime);
      }

    } else {
      
      // no new data
      
      if (_params.call_data_late_script) {
	
	time_t now = time(NULL);
	int timeSinceLastData = now - timeLastAction;
	if (timeSinceLastData > _params.data_late_secs) {
	  _callDataLateScript();
	  timeLastAction = now;
	}
	
      } // if (_params.call_data_late_script)

      umsleep(_params.wait_between_checks * 1000);
      
    } // if (inputPath ...
    
    // reap or kill child processes as required
    
    _reapChildren();

    // register with procmap
    
    PMU_auto_register("Waiting for data");
    
  } // while

  return 0;

}

////////////////////////////////
// register with DataMapper

void InputWatcher::_registerWithDataMapper(time_t modTime)
  
{
  
  // compute directory path relative to RAP_DATA_DIR
  
  string relDir;
  RapDataDir.stripPath(_params.input_dir, relDir);

  DmapAccess access;
  if (access.regLatestInfo(modTime, relDir, _params.data_type)) {
    cerr << "ERROR - InputWatcher" << endl;
    cerr << "  Failed to register with data mapper." << endl;
    cerr << "  Dir: " << relDir << endl;
  }

}

////////////////////////////////
// write latest_data_info file

int InputWatcher::_writeLdataInfoFile(const char *inputPath, time_t modTime)
  
{

  DsLdataInfo ldata;
  if (_params.debug) {
    ldata.setDebug(true);
  }
  if (_params.write_latest_data_info_to_proxy_dir) {
    ldata.setDir(_params.latest_data_info_proxy_dir);
    ldata.setDisplacedDirPath(_params.input_dir);
  } else {
    ldata.setDir(_params.input_dir);
  }

  Path fullPath(inputPath);
  string relPath;
  Path::stripDir(_params.input_dir, inputPath, relPath);
  
  ldata.setDataFileExt(fullPath.getExt().c_str());
  ldata.setWriter(_progName.c_str());
  ldata.setRelDataPath(relPath.c_str());
  if (strlen(_params.data_type) > 0) {
    ldata.setDataType(_params.data_type);
  }

  if (ldata.write(modTime)) {
    cerr << "ERROR - InputWatcher" << endl;
    cerr << "  Cannot write latest data file to input dir: "
	 << _params.input_dir << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "===== wrote ldata info ====" << endl;
    ldata.printAsXml(cerr);
  }

  return 0;

}

////////////////////////////////
// copy the file

int InputWatcher::_copyFile(const char *inputPath, time_t modTime)
  
{

  Path ppath(inputPath);

  // compressed?
  
  bool isCompressed = false;
  bool isGzipped = false;
  bool isBzipped = false;

  const char *inputEnd = inputPath + strlen(inputPath);
  if (strstr(inputEnd - 3, ".gz") != NULL) {
    isGzipped = true;
  } else if (strstr(inputEnd - 4, ".bz2") != NULL) {
    isBzipped = true;
  } else if (strstr(inputEnd - 2, ".Z") != NULL) {
    isCompressed = true;
  }

  if (_params.debug) {
    cerr << "Copying file: " << inputPath << endl;
    if (isCompressed) {
      cerr << "  isCompressed" << endl;
    } else if (isGzipped) {
      cerr << "  isGzipped " << endl;
    } else if (isBzipped) {
      cerr << "  isBzipped " << endl;
    }
  }

  // subdir for copied file

  DateTime mtime(modTime);
  char copySubDir[MAX_PATH_LEN];
  if (_params.without_date_directory) {
    // no date
    sprintf(copySubDir, "%s", _params.copy_dir);
  } else {
    // add in date
    sprintf(copySubDir, "%s%s%.4d%.2d%.2d",
	    _params.copy_dir, PATH_DELIM,
	    mtime.getYear(), mtime.getMonth(), mtime.getDay());
  }

  // prefix if needed

  string prefixStr(_params.copy_prefix);
  if (prefixStr.size() > 0) {
    prefixStr += "_";
  }
  
  // ext if needed
  
  string extStr;
  if (strlen(_params.copy_ext) > 0) {
    extStr += ".";
    extStr += _params.copy_ext;
  }

  // copy file name
  
  char copyName[MAX_PATH_LEN];
  if (_params.copy_to_time_stamped_file) {
    if (_params.add_day_to_filename) {
      // YYYYMMDD_HHMMSS
      snprintf(copyName, MAX_PATH_LEN,
               "%s%.4d%.2d%.2d_%.2d%.2d%.2d%s",
               prefixStr.c_str(),
               mtime.getYear(), mtime.getMonth(), mtime.getDay(),
               mtime.getHour(), mtime.getMin(), mtime.getSec(),
               extStr.c_str());
    } else {
      // HHMMSS
      snprintf(copyName, MAX_PATH_LEN,
               "%s%.2d%.2d%.2d%s",
               prefixStr.c_str(),
               mtime.getHour(), mtime.getMin(), mtime.getSec(),
               extStr.c_str());
    }
  } else {
    if (_params.append_date_time_to_original_name) {
      // append date_time to original name
      snprintf(copyName, MAX_PATH_LEN,
               "%s_%.4d%.2d%.2d_%.2d%.2d%.2d",
               ppath.getFile().c_str(),
               mtime.getYear(), mtime.getMonth(), mtime.getDay(),
               mtime.getHour(), mtime.getMin(), mtime.getSec());
    } else {
      // use original name unchanged
      snprintf(copyName, MAX_PATH_LEN,
               "%s", ppath.getFile().c_str());
    }
  }

  // compression extensions
  
  char uncompressedName[MAX_PATH_LEN];
  if (_params.copy_to_time_stamped_file) {
    if (isGzipped) {
      sprintf(uncompressedName, "%s", copyName);
      sprintf(copyName, "%s.gz", uncompressedName);
    } else if (isBzipped) {
      sprintf(uncompressedName, "%s", copyName);
      sprintf(copyName, "%s.bz2", uncompressedName);
    } else if (isCompressed) {
      sprintf(uncompressedName, "%s", copyName);
      sprintf(copyName, "%s.Z", uncompressedName);
    }
  }

  // copy path

  char copyPath[MAX_PATH_LEN];
  sprintf(copyPath, "%s%s%s",
	  copySubDir, PATH_DELIM, copyName);
  
  if (_params.debug) {
    cerr << "Copying file: " << inputPath << endl;
    cerr << "     to file: " << copyPath << endl;
    cerr << "  making dir: " << copySubDir << endl;
  }
  
  if (ta_makedir_recurse(copySubDir)) {
    int errNum = errno;
    cerr << "ERROR - InputWatcher" << endl;
    cerr << "  Could not make subdir: " << copySubDir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // perform the copy

  FILE *in;
  if ((in = fopen(inputPath, "rb")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - InputWatcher" << endl;
    cerr << "  Could not open input file: " << inputPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  FILE *out;
  if ((out = fopen(copyPath, "wb")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - InputWatcher" << endl;
    cerr << "  Could not open output file: " << copyPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    fclose(in);
    return -1;
  }
  
  ui08 buf[BUFSIZ];
  while (!feof(in)) {
    size_t nread = ufread(buf, 1, BUFSIZ, in);
    if (nread > 0) {
      if (fwrite(buf, 1, nread, out) != nread) {
	int errNum = errno;
	cerr << "ERROR - InputWatcher" << endl;
	cerr << "  Could write data to output file: " << copyPath << endl;
	cerr << "  " << strerror(errNum) << endl;
	fclose(in);
	fclose(out);
	return -1;
      }
    }
  }
  
  fclose(in);
  fclose(out);

  if (_params.remove_after_copy) {
    if (_params.debug) {
      cerr << "Removing file: " << inputPath << endl;
    }
    int ret = remove(inputPath);

    if (ret != 0) {
      cerr << "Failure to remove file " << inputPath << endl;
    }
  }
  
  // uncompress as required
  // this also amends the copyPath
  
  if (_params.uncompress_after_copy && (isGzipped || isBzipped || isCompressed)) {
    if (ta_file_uncompress(copyPath)) {
      cerr << "WARNING - InputWatcher" << endl;
      cerr << "  Could not uncompress file: " << copyPath << endl;
    }
    if (_params.debug) {
      cerr << "Uncompressed file: " << copyPath << endl;
    }
  }

  // write latest data info

  string relPath;
  Path::stripDir(_params.copy_dir, copyPath, relPath);
  DsLdataInfo ldata(_params.copy_dir);
  ldata.setDataFileExt(_params.copy_ext);
  ldata.setWriter(_progName.c_str());
  ldata.setRelDataPath(relPath);

  if (ldata.write(modTime)) {
    cerr << "ERROR - InputWatcher" << endl;
    cerr << "  Cannot write latest data file to copy dir: "
	 << _params.copy_dir << endl;
    return -1;
  }

  return 0;

}

////////////////////////////////
// call script when data arrives

void InputWatcher::_callDataArrivedScript(const char *inputPath,
					  time_t modTime)
  
{

  char modTimeStr[32];
  sprintf(modTimeStr, "%ld", (long) modTime);
  
  vector<string> args;

  if (_params.script_style == Params::SCRIPT_WITH_COMMAND_LINE_OPTIONS) {
    args.push_back("-input_file_path");
    args.push_back(inputPath);
    args.push_back("-file_modify_time");
    args.push_back(modTimeStr);
    if (_params.debug) {
      args.push_back("-debug");
    }
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      args.push_back("-verbose");
    }
  } else if (_params.script_style == Params::SCRIPT_WITH_COMMAND_LINE_OPTIONS_DOUBLE_DASH) {
    args.push_back("--input_file_path");
    args.push_back(inputPath);
    args.push_back("--file_modify_time");
    args.push_back(modTimeStr);
    if (_params.debug) {
      args.push_back("--debug");
    }
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      args.push_back("--verbose");
    }
  } else {
    args.push_back(inputPath);
    args.push_back(modTimeStr);
  }

  if (_params.include_arrived_script_args)
    for (int i = 0; i < _params.arrived_script_args_n; i++)
      args.push_back(_params._arrived_script_args[i]);

  _callScript(_params.run_script_in_background, args,
	      _params.data_arrived_script);
  
}

////////////////////////////////
// call script when data is late

void InputWatcher::_callDataLateScript ()
  
{
  
  char dataLateSecsStr[32];
  sprintf(dataLateSecsStr, "%d", _params.data_late_secs);
  
  char nowStr[32];
  sprintf(nowStr, "%ld", (long) time(NULL));
  
  vector<string> args;

  if (_params.script_style == Params::SCRIPT_WITH_COMMAND_LINE_OPTIONS) {
    args.push_back("-input_dir");
    args.push_back(_params.input_dir);
    args.push_back("-data_late_secs");
    args.push_back(dataLateSecsStr);
    args.push_back("-unix_time");
    args.push_back(nowStr);
    if (_params.debug) {
      args.push_back("-debug");
    }
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      args.push_back("-verbose");
    }
  } else if (_params.script_style == Params::SCRIPT_WITH_COMMAND_LINE_OPTIONS_DOUBLE_DASH) {
    args.push_back("--input_dir");
    args.push_back(_params.input_dir);
    args.push_back("--data_late_secs");
    args.push_back(dataLateSecsStr);
    args.push_back("--unix_time");
    args.push_back(nowStr);
    if (_params.debug) {
      args.push_back("--debug");
    }
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      args.push_back("--verbose");
    }
  } else {
    args.push_back(_params.input_dir);
    args.push_back(dataLateSecsStr);
    args.push_back(nowStr);
  }
  
 if (_params.include_late_script_args)
    for (int i = 0; i < _params.late_script_args_n; i++)
      args.push_back(_params._late_script_args[i]);


  _callScript(true, args, _params.data_late_script);

}

//////////////////
// call the script

void InputWatcher::_callScript (bool run_in_background,
                                const vector<string> &args,
				const char *script_to_call)
  
{
  
  char pmuStr[4096];
  sprintf(pmuStr, "Starting %s", script_to_call);
  PMU_force_register(pmuStr);

  if (_params.debug) {
    cerr << "Calling script: " << script_to_call << endl;
  }
  
  // Fork a child to run the script
  
  time_t start_time = time(NULL);
  time_t terminate_time = start_time + _params.script_max_run_secs;
  pid_t childPid = fork();
  
  if (childPid == 0) {
    
    // this is the child process, so exec the script
    
    _execScript(args, script_to_call);
    
    // exit
    
    if (_params.debug) {
      cerr << "Child process exiting ..." << endl;
    }

    _exit(0);

  }

  // this is the parent

  if (_params.debug) {
    cerr << endl;
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

      // sleep for a bit
      
      umsleep(50);
      
    } // while
    
  } // if (run_script_in_background)

}

//////////////////////
// execute the script

void InputWatcher::_execScript(const vector<string> &args,
                               const char *script_to_call)
  
{

  // set up execvp args - this is a null-terminated array of strings
  
  int narray = (int) args.size() + 2;
  TaArray<const char *> argArray_;
  const char **argArray = argArray_.alloc(narray);
  argArray[0] = script_to_call;
  for (int ii = 0; ii < (int) args.size(); ii++) {
    argArray[ii+1] = args[ii].c_str();
  }
  argArray[narray-1] = NULL;
  
  if (_params.debug) {
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

void InputWatcher::_reapChildren()

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

void InputWatcher::_killAsRequired(pid_t pid,
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

void InputWatcher::killRemainingChildren()

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

