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
// InfoStore.cc
//
// InfoStore object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 1999
//
///////////////////////////////////////////////////////////////
//
// The InfoStore object reads in the info table, modifies it as
// required and then writes it out.
//
///////////////////////////////////////////////////////////////

#include <algorithm>
#include <functional>
#include <cerrno>
#include <iostream>
#include <toolsa/udatetime.h>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/file_io.h>
#include "InfoStore.hh"
using namespace std;

////////////////////////////////////////////////////////////
// Constructor for read-write access

InfoStore::InfoStore(const Params &params,
		     const vector<string> &dataTypes) :
  _params(params),
  _dataTypes(dataTypes)
  
{

  if (_params.save_state) {
    _readState();
  }

}

// destructor

InfoStore::~InfoStore()

{

  // write out state
  
  if (_params.save_state) {
    saveState();
  }

}

//////////////////////
// regLatestDataInfo()
//
// register the latest data info

void InfoStore::regLatestDataInfo(const DMAP_info_t &info)

{

  // fill in data type if necessary

  DMAP_info_t copy = info;
  _fillDataType(copy);

  // find position for data set entry, inserting as required

  infoSet_t::iterator pos = _findInfo(copy);
  if (pos == _infoSet.end()) {
    pos = _insertInfo(copy);
  }

  // get ref to entry in the set

  DMAP_info_t &entry = (DMAP_info_t &) pos->second;

  // set relevant information

  entry.latest_time = copy.latest_time;
  time_t now = time(NULL);
  entry.last_reg_time = now;
  entry.forecast_lead_time = copy.forecast_lead_time;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "regLatestDataInfo - "
	 << entry.hostname << ":" << entry.dir << endl;
  }

}

//////////////////
// regStatusInfo()
//
// register the status info

void InfoStore::regStatusInfo(const DMAP_info_t &info)
  
{
  
  // fill in data type if necessary

  DMAP_info_t copy = info;
  _fillDataType(copy);

  // find position for data set entry, inserting as required

  infoSet_t::iterator pos = _findInfo(copy);
  if (pos == _infoSet.end()) {
    pos = _insertInfo(copy);
  }

  // get ref to entry in the set
  
  DMAP_info_t &entry = (DMAP_info_t &) pos->second;
  
  // set relevant information

  STRncopy(entry.status, copy.status, DMAP_STATUS_LEN);
  time_t now = time(NULL);
  entry.last_reg_time = now;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "regStatusInfo - "
	 << entry.hostname << ":" << entry.dir << endl;
  }
  
}

//////////////////////
// regDataSetInfo()
//
// register the data set info

void InfoStore::regDataSetInfo(const DMAP_info_t &info)

{

  // fill in data type if necessary

  DMAP_info_t copy = info;
  _fillDataType(copy);

  // find position for data set entry, inserting as required

  infoSet_t::iterator pos = _findInfo(copy);
  if (pos == _infoSet.end()) {
    pos = _insertInfo(copy);
  }

  // get ref to entry in the set
  
  DMAP_info_t &entry = (DMAP_info_t &) pos->second;
  
  // set relevant information

  entry.start_time = copy.start_time;
  entry.end_time = copy.end_time;
  entry.nfiles = copy.nfiles;
  entry.total_bytes = copy.total_bytes;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "regDataSetInfo - "
	 << entry.hostname << ":" << entry.dir << endl;
  }

}

//////////////////////
// regFullInfo()

void InfoStore::regFullInfo(const vector<DMAP_info_t> &infoArray)

{

  for (size_t ii = 0; ii < infoArray.size(); ii++) {

    const DMAP_info_t &info = infoArray[ii];

    // find position for data set entry, inserting as required
    
    infoSet_t::iterator pos = _findInfo(info);
    if (pos == _infoSet.end()) {
      pos = _insertInfo(info);
    }
    
    // get ref to entry in the set
  
    DMAP_info_t &entry = (DMAP_info_t &) pos->second;
    
    // set it

    entry = info;

  } // ii

}


//////////////////////
// deleteInfo()
//
// delete entry which matches

void InfoStore::deleteInfo(const DMAP_info_t &info)

{

  // If request is "all", "all", "all", clear the store

  if (!strcmp(info.hostname, "all") && 
      !strcmp(info.datatype, "all") &&
      !strcmp(info.dir, "all")) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "--> Cleaning store" << endl;
    }
    clear();
    return;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "--> Deleting entry:" << endl;
    cerr << "    Data type: " << info.datatype << endl;
    cerr << "    Dir: " << info.dir << endl;
    cerr << "    Hostname: " << info.hostname << endl;
  }

  // delete entries until done

  infoSet_t::iterator pos = _infoSet.begin();
  while (pos != _infoSet.end()) {
    pos = _findInfoWild(info);
    if (pos != _infoSet.end()) {
      _infoSet.erase(pos++);
    }
  }
}

////////////////////////////////
// loadSelected()
//
// Load up selected information into selectedSet

void InfoStore::loadSelected(const char *datatype, const char *dir,
			     infoSet_t &selectedSet)

{

  selectedSet.clear();

  bool matchType, matchDir;
  
  if (strlen(datatype) > 0) {
    matchType = true;
  } else {
    matchType = false;
  }

  if (strlen(dir) > 0) {
    matchDir = true;
  } else {
    matchDir = false;
  }

  if (matchType && matchDir) {

    infoSet_t::iterator ii;
    for (ii = _infoSet.begin(); ii != _infoSet.end(); ii++) {
      if (!strcmp(datatype, ii->second.datatype) &&
	  !strcmp(dir, ii->second.dir)) {
	selectedSet.insert(selectedSet.end(), *ii);
      }
    } // ii

  } else if (matchType) {

    infoSet_t::iterator ii;
    for (ii = _infoSet.begin(); ii != _infoSet.end(); ii++) {
      if (!strcmp(datatype, ii->second.datatype)) {
	selectedSet.insert(selectedSet.end(), *ii);
      }
    } // ii

  } else if (matchDir) {

    infoSet_t::iterator ii;
    for (ii = _infoSet.begin(); ii != _infoSet.end(); ii++) {
      if (!strcmp(dir, ii->second.dir)) {
	selectedSet.insert(selectedSet.end(), *ii);
      }
    } // ii

  } else {
    
    selectedSet = _infoSet;

  }

}

////////////////////////////////
// clear()
//
// Clear out store.
//

void InfoStore::clear()

{
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "--> Clearing out the store" << endl;
  }
  _infoSet.clear();
}

////////////////////////////////
// purge()
//
// Purge the store of old items
//

void InfoStore::purge()

{

  time_t now = time(NULL);
  infoSet_t::iterator ii;
  bool done = false;

  int purge_age_secs = (int) (_params.purge_age_days * 86400.0 + 0.5);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-->Purge age in secs: " << purge_age_secs << endl;
  }

  while (!done) {
    done = true;
    for (ii = _infoSet.begin(); ii != _infoSet.end(); ii++) {

      //
      // Calculate the age. It's worth trying several time fields here
      // since if some time fields are zero, we delete the entry, which
      // is a little heavy-handed in some cases.
      //
      int age = now - ii->second.last_reg_time;
      if (ii->second.last_reg_time == 0L){
	age = now - ii->second.end_time;
	if (ii->second.end_time == 0L){
	  age = now - ii->second.start_time;
	  if (ii->second.start_time == 0L){
	    age = now - ii->second.latest_time;
	  }
	}
      }

      if (age > purge_age_secs) {
	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << "-->Removing old entry from table, age: " << age << endl;
	  cerr << "     DataType: " << ii->second.datatype << endl;
	  cerr << "     Dir: " << ii->second.dir << endl;
	}
	_infoSet.erase(ii);
	done = false;
	break;
      }
    } // for
  } // while
  
}

//////////////////////////////////////////////////
// saveState()
//
// Save state to file
//

int InfoStore::saveState()

{

  const char *path = _params.save_state_path;
  int errNum;
  FILE *tableFile;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "--> saving state to file: " << path << endl;
  }

  if ((tableFile = fopen(path, "wb")) == NULL) {
    errNum = errno;
    cerr << "WARNING - DataMapper:InfoStore:_writeTableFile" << endl;
    cerr << "  Cannot open table file for writing: " << path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  infoSet_t::iterator ii;
  for (ii = _infoSet.begin(); ii != _infoSet.end(); ii++) {

    DMAP_info_t info = ii->second;
    DmapMessage::BE_from_dmap_info(info);

    if (fwrite(&info, sizeof(DMAP_info_t), 1, tableFile) != 1) {
      errNum = errno;
      cerr << "WARNING - DataMapper:InfoStore:_writeTableFile" << endl;
      cerr << "  Cannot write table to file: " << path << endl;
      cerr << "  " << strerror(errNum) << endl;
      fclose(tableFile);
      return -1;
    }

  } // ii

  // close file
  
  fclose(tableFile);
  return 0;
  
}

//////////////////////////////////////////////////
// _readState()
//
// Read in state from file
//
// Returns 0 on success, -1 on failure
//

int InfoStore::_readState()

{

  _infoSet.clear();

  // stat the file to get the number of entries

  const char *path = _params.save_state_path;
  struct stat infoStat;
  if (ta_stat(path, &infoStat)) {
    return 0;
  }
  int nInfo = infoStat.st_size / sizeof(DMAP_info_t);
  int nBytes = nInfo * sizeof(DMAP_info_t);
  if (nBytes != infoStat.st_size) {
    // file is wrong size
    cerr << "WARNING - DataMapper:InfoStore:_readTableFile" << endl;
    cerr << "  Table file wrong size:" << path << endl;
    cerr << "  Starting clean" << endl;
    return -1;
  }
  
  FILE *tableFile;
  if ((tableFile = fopen(path, "rb")) == NULL) {
    // Cannot open file
    return 0;
  }

  // read in Info table

  for (int ii = 0; ii < nInfo; ii++) {

    // read in info struct
    
    DMAP_info_t info;
    if (fread(&info, sizeof(DMAP_info_t), 1, tableFile) != 1) {
      cerr << "WARNING - DataMapper:InfoStore:_readTableFile" << endl;
      cerr << "Cannot read all entries from table file " << path << endl;
      cerr << "  Starting clean" << endl;
      fclose(tableFile);
      _infoSet.clear();
      return -1;
    }

    // swap and insert info

    DmapMessage::BE_to_dmap_info(info);
    infoPair_t ipair;
    ipair.first =  _computeKey(info);
    ipair.second = info;
    
    _infoSet.insert(_infoSet.begin(), ipair);

  } // i

  // close file

  fclose(tableFile);

  return 0;
  
}

/////////////////
// _fillDataType
//
// Fill out the data type if it is not specified.

void InfoStore::_fillDataType(DMAP_info_t &info)
  
{

  // don't fill it in if it is already set
  
  if (strlen(info.datatype) > 0 && !STRequal(info.datatype, "unknown")) {
    return;
  }

  string dir = info.dir;
  string delim = PATH_DELIM;

  for (size_t ii = 0; ii < _dataTypes.size(); ii++) {
    bool typeMatched = false;
    string search1 = delim + _dataTypes[ii] + delim;
    if (dir.find(search1, 0) != string::npos) {
      typeMatched = true;
    } else {
      string search2 = _dataTypes[ii] + delim;
      size_t loc = dir.find(search2, 0);
      if (loc == 0) {
	typeMatched = true;
      }
    }
    if (typeMatched) {
      STRncopy(info.datatype, _dataTypes[ii].c_str(), DMAP_DATATYPE_LEN);
      return;
    }
  } // i
  STRncopy(info.datatype, "unknown", DMAP_DATATYPE_LEN);
}

/////////////////
// _insertInfo()
//
// Insert info struct into table. Table entries are in
// ascending order. Initializes required fields.
//
// Returns pos at which it was inserted

infoSet_t::iterator InfoStore::_insertInfo(const DMAP_info_t &info)

{

  infoPair_t ipair;
  DMAP_info_t &copy = ipair.second;
  MEM_zero(copy);
  STRncopy(copy.hostname, info.hostname, DMAP_HOSTNAME_LEN);
  STRncopy(copy.ipaddr, info.ipaddr, DMAP_IPADDR_LEN);
  STRncopy(copy.datatype, info.datatype, DMAP_DATATYPE_LEN);
  STRncopy(copy.dir, info.dir, DMAP_DIR_LEN);
  STRncopy(copy.status, info.status, DMAP_STATUS_LEN);

  ipair.first = _computeKey(copy);
  
  return _infoSet.insert(_infoSet.begin(), ipair);
  
}

/////////////////
// _computeKey()
//
// Compute the key from the info
//
// Returns string key

string InfoStore::_computeKey(const DMAP_info_t &info)

{

  string key = info.datatype;
  key += ",";
  key += info.dir;
  key += ",";
  key += info.hostname;
  return key;

}

/////////////////
// _findInfo()
//
// Find info in table.
//
// Returns pos on success, end() on failure

infoSet_t::iterator InfoStore::_findInfo(const DMAP_info_t &info)

{

  string key = _computeKey(info);
  return _infoSet.find(key);

}

////////////////////
// _findInfoWild()
//
// Find the first pos of the key, considering wild-card entries for
// data-type, dir and hostname. Wild-cards are 'all'.
//
// Returns iterator, end() on failure.

infoSet_t::iterator InfoStore::_findInfoWild(const DMAP_info_t &info)

{

  // try non-wild first
  
  infoSet_t::iterator itt = _findInfo(info);
  if (itt != _infoSet.end()) {
    return itt;
  }

  string datatype = info.datatype;
  string dir = info.dir;
  string hostname = info.hostname;

  infoSet_t::iterator ii;
  for (ii = _infoSet.begin(); ii != _infoSet.end(); ii++) {

    const DMAP_info_t &thisInfo = ii->second;
    bool matches = true;
    if (strcmp(info.datatype, "all") &&
	strcmp(info.datatype, thisInfo.datatype)) {
      matches = false;
    } else if (strcmp(info.dir, "all") &&
	       strcmp(info.dir, thisInfo.dir)) {
      matches = false;
    } else if (strcmp(info.hostname, "all") &&
	       strcmp(info.hostname, thisInfo.hostname)) {
      matches = false;
    }
    
    if (matches) {
      return ii;
    }

  }

  return _infoSet.end();

}

/////////////////
// _printInfo()
//

void InfoStore::_printInfo(const DMAP_info_t &info,
			   ostream &out)

{

  out << "-----------------------------------------------" << endl;
  out << "  start_time: " << utimstr(info.start_time) << endl;
  out << "  end_time: " << utimstr(info.end_time) << endl;
  out << "  latest_time: " << utimstr(info.latest_time) << endl;
  out << "  last_reg_time: " << utimstr(info.last_reg_time) << endl;
  out << "  nfiles: " << info.nfiles << endl;
  out << "  total_bytes: " << info.total_bytes << endl;
  out << "  hostname: " << info.hostname << endl;
  out << "  ipaddr: " << info.ipaddr << endl;
  out << "  datatype: " << info.datatype << endl;
  out << "  dir: " << info.dir << endl;

}

///////////
// print()
//

void InfoStore::print(ostream &out)

{

  infoSet_t::iterator ii;
  for (ii = _infoSet.begin(); ii != _infoSet.end(); ii++) {
    _printInfo(ii->second, out);
  }

}
