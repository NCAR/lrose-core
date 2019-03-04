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
// InfoStore.cc
//
// Process information store
//////////////////////////////////////////////////////////

#include "InfoStore.hh"
#include <toolsa/str.h>
  
//////////////
// constructor
//

InfoStore::InfoStore ()

{
  _debug = false;
  pthread_mutex_init(&_mutex, NULL);
}

/////////////
// destructor
//

InfoStore::~InfoStore ()
{

}

//////////////////
// get proc info
//

void InfoStore::GetInfo(const PROCMAP_request_t &req,
			MemBuf &info_buf)
  
{

  // lock store

  pthread_mutex_lock(&_mutex);
  
  // make copy of name and instance,
  // remove leading and trailing blanks
  
  char *name = strdup(req.name);
  char *instance = strdup(req.instance);
  STRblnk(name);
  STRblnk(instance); 

  // clear info buffer

  info_buf.free();

  // look for matching procs
  
  InfoIter ii;

  for (ii = _procs.begin(); ii != _procs.end(); ii++) {

    PROCMAP_info_t &info = (*ii).second;
    
    if (_debug) {
      cerr << "-----------------------------" << endl;
      cerr << "Info: " << info.name << ", " << info.instance << endl;
      cerr << "hearbeat_time: " << utimstr(info.heartbeat_time) << endl;
      cerr << "-----------------------------" << endl;
    }
    
    if (_matchName(info.name, name) &&	  
	_matchInstance(info.instance, instance)) {
      
      if (_debug) {
	cerr << "+++++++++++++++++++++++++++++" << endl;
	cerr << "Match: " << info.name << ", " << info.instance << endl;
	cerr << "+++++++++++++++++++++++++++++" << endl;
      }

      // add to info buffer

      info_buf.add(&info, sizeof(info));

    }

  } // ii

  // free up
  free (name);
  free (instance);

  // unlock store

  pthread_mutex_unlock(&_mutex);

}

//////////////////
// register proc
//

void InfoStore::RegisterProc(const PROCMAP_info_t &info)
  
{

  // lock store

  pthread_mutex_lock(&_mutex);
  
  if (_debug) {
    cerr << "*****************************" << endl;
    cerr << "Register:" << info.name << ", " << info.instance << endl;
    cerr << "host: " << info.host << endl;
    cerr << "user: " << info.user << endl;
    cerr << "heartbeat_time: " << utimstr(info.heartbeat_time) << endl;
    cerr << "status_str: " << info.status_str << endl;
    cerr << "*****************************" << endl;
  }

  // search for entry in map

  string key = info.host;
  key += info.user;
  key += info.name;
  key += info.instance;
  InfoIter ii = _procs.find(key);
  bool added = false;

  if (ii == _procs.end()) {

    // not found in map, so add

    if (_debug) {
      cerr << "->->->-> Adding new proc <-<-<-<-" << endl;
    }

    InfoPair pair;
    pair.first = key;
    pair.second = info;
    _procs.insert(pair);
    ii = _procs.find(key);
    added = true;

  }

  // update info in store

  PROCMAP_info_t &storedInfo = (*ii).second;
  if (added) {
    storedInfo.n_reg = 0;
  }
  int nreg = storedInfo.n_reg + 1;
  storedInfo = info;
  storedInfo.n_reg = nreg;
  storedInfo.last_registered = time(NULL);

  // unlock store

  pthread_mutex_unlock(&_mutex);

}

////////////////////
// unregister proc
//

void InfoStore::UnRegisterProc(const PROCMAP_info_t &info)
  
{
  
  // lock store

  pthread_mutex_lock(&_mutex);
  
  if (_debug) {
    cerr << "=============================" << endl;
    cerr << "UnRegister " << info.name << ", " << info.instance
	 << " on " << info.host << " by " << info.user << endl;
    cerr << "=============================" << endl;
  }
  
  // remove if it is in the info map
  
  string key = info.host;
  key += info.user;
  key += info.name;
  key += info.instance;
  InfoIter ii = _procs.find(key);
  if (ii != _procs.end()) {
    _procs.erase(ii);
  }
  
  // unlock store

  pthread_mutex_unlock(&_mutex);

}

////////////////////////////////////////////////
// set proc invalid if no recent registration
//

void InfoStore::Purge(time_t now)
{

  // lock store

  pthread_mutex_lock(&_mutex);

  bool done = false;
  
  while (!done) {

    done = true;
    InfoIter ii;
    for (ii = _procs.begin(); ii != _procs.end(); ii++) {
      PROCMAP_info_t &info = (*ii).second;
      int elapsed = now - info.last_registered;
      if (elapsed > PROCMAP_PURGE_MULTIPLE * info.max_reg_interval) {
	if (_debug) {
	  cerr << "  purging following process ....... " << endl;
          printPretty(cerr, info);
	  cerr << "  now: " << utimstr(now) << endl;
	  cerr << "  lreg: " << utimstr(info.last_registered) << endl;
	}
	_procs.erase(ii);
	done = false;
	break;
      }
      
    } // ii

  }
  
  // unlock store

  pthread_mutex_unlock(&_mutex);

}

//////////////////////
// matching proc name
//

bool InfoStore::_matchName(const char *proc_name, const char *want_name)
{
  // empty string always matches
  if (0 == strlen(want_name)) {
    return true;
  }
  if (!strcmp(proc_name, want_name)) {
    return true;
  } else {
    return false;
  }
}

//////////////////////////
// matching proc instance
//

bool InfoStore::_matchInstance(const char *proc_instance,
			       const char *want_instance)
{
  // empty string always matches
  if (0 == strlen(want_instance)) {
    return TRUE;
  }
  if (!strcmp(proc_instance, want_instance)) {
    return true;
  } else {
    return false;
  }
}


//////////////////
// print table
//

void InfoStore::print(ostream &out)
  
{

  // lock store

  pthread_mutex_lock(&_mutex);

  out << "Nprocs: " << _procs.size() << endl;

  out << "name instance host user pid max_reg_int "
      << "hb up nreg status" << endl;

  InfoIter ii;
  for (ii = _procs.begin(); ii != _procs.end(); ii++) {
    PROCMAP_info_t &info = (*ii).second;
    print(out, info);
  }
  out << endl;

  // unlock store

  pthread_mutex_unlock(&_mutex);

}

////////////////////
// printing entries
//

void InfoStore::print(ostream &out, const PROCMAP_info_t &info)
  
{
  time_t now = time(NULL);
  out << info.name << " "
      << info.instance << " "
      << info.host << " "
      << info.user << " "
      << info.pid << " "
      << info.max_reg_interval << " "
      << (info.heartbeat_time - now) << " "
      << (info.start_time - now) << " "
      << info.n_reg << " "
      << info.status << endl;
}

void InfoStore::printPretty(ostream &out, const PROCMAP_info_t &info)
{
  time_t now = time(NULL);
  out << "    name: " << info.name << endl;
  out << "    instance: " << info.instance << endl;
  out << "    host: " << info.host << endl;
  out << "    user: " << info.user << endl;
  out << "    pid: " << info.pid << endl;
  out << "    max_reg_interval: " << info.max_reg_interval << endl;
  out << "    heartbeat_time: " << (info.heartbeat_time - now) << endl;
  out << "    start_time: " << (info.start_time - now) << endl;
  out << "    n_reg: " << info.n_reg << endl;
  out << "    status: " << info.status << endl;
}

