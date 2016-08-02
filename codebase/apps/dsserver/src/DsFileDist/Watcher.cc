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
// Watcher.cc
//
// Object for watching directories for new files
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2010
//
//////////////////////////////////////////////////////////

#include "Watcher.hh"

#include <toolsa/Path.hh>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <dsserver/DsLdataInfo.hh>

using namespace std;

// Constructor
//
// If based_on_params is true, then this is being created based on
// finding a _DsFileDist param file. If not, it is based on finding
// _latest_data_info files. The ldata_top_dir only applies if
// based_on_params is false.

Watcher::Watcher(const string &prog_name,
                 const Params &params,
                 const string &dir_path,
                 bool based_on_params,
                 bool ldata_avail,
                 const string &ldata_top_dir,
                 list<PutArgs *> &put_list,
                 const string &error_fmq_path) :
        _progName(prog_name),
        _params(params),
        _dirPath(dir_path),
        _basedOnParams(based_on_params),
        _ldataAvail(ldata_avail),
        _ldataTopDir(ldata_top_dir),
        _putList(put_list),
        _errorFmqPath(error_fmq_path),
        _input("DsFileDist", params.debug >= Params::DEBUG_EXTRA,
               dir_path, _params.max_valid_age, NULL, ldata_avail, false)
  
{

  isOK = true;
  _paramFileMtime = 0;
  
  // get param file mod time

  if (_basedOnParams) {
    string paramFilePath(_dirPath);
    paramFilePath += PATH_DELIM;
    paramFilePath += "_DsFileDist";
    if (strlen(_params._dsFileDist_ext) > 0) {
      paramFilePath += ".";
      paramFilePath += _params._dsFileDist_ext;
    }
    struct stat paramsStat;
    if (stat(paramFilePath.c_str(), &paramsStat)) {
      cerr << "ERROR - " << _progName
	   << " - Watcher::Watcher" << endl;
      cerr << "  " << DateTime::str() << endl;
      cerr << "  Dir: " << _dirPath << endl;
      cerr << "  No param file." << endl;
      isOK = false;
    }
    _paramFileMtime = paramsStat.st_mtime;
  } else {
    _ldataAvail = true;
  }

  // set DsInputPath properties

  if (_params.check_file_ext) {
    _input.setSearchExt(_params.data_file_ext);
  }
  _input.setFileQuiescence(_params.file_quiescence_age);

  if (_params.debug) {
    cerr << "==============================================" << endl;
    cerr << "Creating new Watcher" << endl;
    cerr << "  dir path: " << _dirPath << endl;
    if (_basedOnParams) {
      cerr << "  based on _DsFileDist params file" << endl;
      if (_ldataAvail) {
        cerr << "  _latest_data_info available" << endl;
      }
    } else {
      cerr << "  based on _latest_data_info files" << endl;
      cerr << "  ldata_top_dir: " << _ldataTopDir << endl;
    }
    cerr << "==============================================" << endl;
  }

}

// Destructor

Watcher::~Watcher()

{

}

//////////////////////////////////////
// find any new files, add to put list

int Watcher::findFiles()

{

  // get next file

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Watcher.findFiles.entry:  " << DateTime::str() << "  dir: "
         << _dirPath << endl;
    cerr << "--->>> Checking dir: " << _dirPath << endl;
  }

  while (_getNextFile() == 0) {

    if (!_processThisFile()) {
      continue;
    }

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "New file found, dir: " << _dirPath << endl;
    }

    // load the destination URL's
    
    DestUrlArray destUrls(_progName, true);
    if (_loadUrlList(destUrls)) {
      cerr << "ERROR - " << _progName << ":Watcher::findFiles()." << endl;
      cerr << "  dir: " << _dirPath << endl;
      cerr << "  " << DateTime::str() << endl;
      return -1;
    }
    
    // we can only remove_after_copy if there is only one destination
    // otherwise we might remove the file before it is read by a
    // destination thread
    
    bool remove_after_copy = _params.remove_after_copy;
    if (remove_after_copy && destUrls.size() > 1) {
      cerr << "WARNING - " << _progName
           << ":Watcher::findFiles()." << endl;
      cerr << "  " << DateTime::str() << endl;
      cerr << "  dir: " << _dirPath << endl;
      cerr << "  You have asked to remove files after copy." << endl;
      cerr << "  However, you are sending data to more than 1 URL." << endl;
      cerr << "  Therefore the delete will not be done." << endl;
      remove_after_copy = false;
    }
    
    PMU_auto_register("Adding file to put list");
    const LdataInfo &ldata = _input.getLdataInfo();
    
    for (size_t ii = 0; ii < destUrls.size(); ii++) {
      
      PutArgs *putArgs =
	new PutArgs(destUrls[ii],
		    _dirPath,
		    ldata,
		    ldata.getRelDataPath(),
		    ldata.getLatestTime(),
		    _params.overwrite_age,
		    _params.max_age_at_copy_time,
		    _params.force_copy,
		    remove_after_copy,
		    (ta_compression_method_t) _params.compression_type,
		    _params.minimum_expected_transfer_speed,
		    _params.max_n_tries_per_transfer,
                    _params.server_open_timeout_msecs,
		    _params.debug,
                    _errorFmqPath);
      
      if (!putArgs->isOK) {
	cerr << "ERROR - " << _progName
	     << ":Watcher::findFiles()." << endl;
	cerr << "  " << DateTime::str() << endl;
	cerr << "  Cannot create args." << endl;
	return (-1);
      }
      
      // add to list
      
      _addPut(putArgs);

    } // ii
    
    // write latest data locally if requested
    
    if (!_params.latest_data_info_avail && _params.write_ldata_to_input_dir) {
	
      DsLdataInfo inLdata(_dirPath,
			  _params.debug >= Params::DEBUG_VERBOSE);
      
      inLdata.setDataFileExt(ldata.getDataFileExt().c_str());
      inLdata.setWriter("DsFileDist");
      inLdata.setRelDataPath(ldata.getRelDataPath().c_str());
      inLdata.setLatestTime(ldata.getLatestTime());
      inLdata.setIsFcast(false);
      
      if (inLdata.write(ldata.getLatestTime())) {
	cerr << "ERROR - DsFileDist::Watcher::findFiles()" << endl;
	cerr << "  Cannot write ldata file to input dir" << endl;
      }
      
    } // if (!_params.latest_data_info_avail ...
    
  } // while

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Watcher.findFiles.exit:  " << DateTime::str() << "  dir: "
         << _dirPath << endl;
  }
  return (0);

}

//////////////////////////////////////
// get next file
//
// Returns 0 on success, -1 on failure

int Watcher::_getNextFile()

{

  char *inputPath;
  if ((inputPath = _input.next(false)) == NULL) {
    return -1;
  }

  return 0;

}

//////////////////////////////////////
// should we process this file?

bool Watcher::_processThisFile()

{

  const LdataInfo &ldata = _input.getLdataInfo();
  
  // check for redistribution
  // If the file has already been distributed once, the writer
  // is "DsFCopyServer" because it is the
  // DsFCopyServer which writes the file to disk.
  
  if (!_params.allow_redistribution &&
      (ldata.getWriter() == "DsFCopyServer")) {
    if (_params.debug) {
      cerr << "Rejecting file - already distributed once" << endl;
      cerr << "  File: " << ldata.getDataPath() << endl;
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	ldata.printFull(cerr);
      }
    }
    return false;
  }
      
  return true;

}

///////////////////////////
// load URL list
  
int Watcher::_loadUrlList(DestUrlArray &destUrls)

{
  
  if (_params.use_dest_host_list_file) {
    // employ strict checking if a host list is used
    // bacause files will be copied to the same dir
    destUrls.setStrictCheckForLocalHost(true);
    if (destUrls.load(_params.dest_host_list_file_path,
 		      _params.dest_url_template)) {
      cerr << "ERROR - " << _progName
 	   << ":Watcher::_loadUrlList()." << endl;
      cerr << "  dir: " << _dirPath << endl;
      cerr << "  " << DateTime::str() << endl;
      cerr << "  " << destUrls.getErrStr() << endl;
      return -1;
    }
  } else {
    // strict host name checking is not employed, so that
    // a user can optionally distribute to a different
    // directory on the local host.
    for (int i = 0; i < _params.dest_url_list_n; i++) {
      if (destUrls.add(_params._dest_url_list[i])) {
	cerr << "ERROR - " << _progName
	     << ":Watcher::_loadUrlList()." << endl;
        cerr << "  dir: " << _dirPath << endl;
	cerr << "  " << DateTime::str() << endl;
	cerr << "  " << destUrls.getErrStr() << endl;
	return -1;
      }
    } // i
  }

  if (!_basedOnParams) {

    // compute path relative to ldata top dir

    string relPath;
    Path::stripDir(_ldataTopDir, _dirPath, relPath);
    
    // set directory from rel path

    destUrls.setDirRelative(relPath);

  }

  return 0;
  
}


//////////////////////////////////
// add to put list

void Watcher::_addPut(PutArgs *put_args)
  
{

  // check the queue size
  // discard entry if too large

  if ((int) _putList.size() > _params.max_file_queue_size) {
    cerr << "WARNING - Watcher::_addPut" << endl;
    cerr << "  File queue size has reached maximum: "
         << _putList.size() << endl;
    cerr << "  ---->> File put entry will be discarded:" << endl;
    cerr << "    Host: " << put_args->destHostName << endl;
    cerr << "    Path: " << put_args->filePath << endl;
    return;
  }

  if (_params.debug) {
    cerr << "---->> Adding file to distrib list:" << endl;
    cerr << "  Host: " << put_args->destHostName << endl;
    cerr << "  Path: " << put_args->filePath << endl;
    cerr << "  Size: " << put_args->nbytesFile << endl;
    cerr << "  Current waiting list length, time: "
         << _putList.size() << ", "
         << DateTime::strm(time(NULL)) << endl;
  }

  if (_params.reorder_by_file_size) {
    
    // insert the file based on it's size and the size of the
    // files already in the list

    double max_delay = _params.max_reorder_delay_fraction;
    
    // look through list for last entry for which the delay exceeds the
    // max
    
    bool insertFound = false;
    int ipos = 0;
    int pos = 0;
    list<PutArgs *>::iterator ii;
    list<PutArgs *>::iterator insertPoint = _putList.begin();
    for (ii = _putList.begin(); ii != _putList.end(); ii++, ipos++) {
      
      double sum = put_args->nbytesFile + (*ii)->nbytesInserted;
      double delay = sum / (*ii)->nbytesFile;

      if (delay > max_delay) {
	insertPoint = ii;
	pos = ipos;
	insertFound = true;
      }
      
    } // ii
    
    if (insertFound) {
      // insert point is one beyond the last found
      insertPoint++;
      pos++;
    }
    
    if (insertPoint == _putList.end()) {
      if (_params.debug) {
 	cerr << "------>> Adding to end of list" << endl;
      }
      _putList.push_back(put_args);
      return;
    }
    
    // go through list adding the file size to all files
    // beyond the insert point
    
    for (ii = insertPoint; ii != _putList.end(); ii++) {
      (*ii)->nbytesInserted += put_args->nbytesFile;
    } // ii
    
    // insert the put args
    
    if (_params.debug) {
      cerr << "------>> Inserting into list, pos: " << pos << endl;
    }
    
    _putList.insert(insertPoint, put_args);

  } else {

    // no reordering - put on end of list

    if (_params.debug) {
      cerr << "------>> Adding to end of list" << endl;
    }

    _putList.push_back(put_args);

  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {

    cerr << "File waiting list, length: " << _putList.size() << endl;

#ifdef DEBUG_PRINT
    cerr << "=================================================" << endl;
    int i = 0;
    list<PutArgs *>::iterator jj;
    for (jj = _putList.begin(); jj != _putList.end(); jj++, i++) {
      cerr << "List entry #: " << i << endl;
      cerr << "    Path: " << (*jj)->filePath << endl;
      cerr << "    Size: " << (*jj)->nbytesFile << endl;
      cerr << "    NbytesInserted: " << (*jj)->nbytesInserted << endl;
    } // jj
    cerr << "=================================================" << endl;
#endif
    
  } // if (_params.debug ...

}

/////////////////////////////////////////////////////
// is this directory still active?
  
bool Watcher::isStillActive()
  
{

  if (_basedOnParams) {
    return _paramsStillActive();
  } else {
    return _ldataStillActive();
  }

}

/////////////////////////////////////////////////////
// check on latest data file
  
bool Watcher::_ldataStillActive()
  
{

  // check on ldata info
  
  LdataInfo ldata(_dirPath);
  if (ldata.readForced()) {
    // no ldata file
    if (_params.debug) {
      cerr << "Dir: " << _dirPath << endl;
      cerr << "  " << DateTime::str() << endl;
      cerr << "  Ldata info lost." << endl;
    }
    return false;
  }

  return true;
  
} 

/////////////////////////////////////////////////////
// check on param file
  
bool Watcher::_paramsStillActive()
  
{

  // check for param file
  
  string paramFilePath(_dirPath);
  paramFilePath += PATH_DELIM;
  paramFilePath += "_DsFileDist";
  if (strlen(_params._dsFileDist_ext) > 0) {
    paramFilePath += ".";
    paramFilePath += _params._dsFileDist_ext;
  }
  struct stat paramsStat;

  if (stat(paramFilePath.c_str(), &paramsStat)) {
    // no params file
    if (_params.debug) {
      cerr << "Dir: " << _dirPath << endl;
      cerr << "  " << DateTime::str() << endl;
      cerr << "  Param file lost." << endl;
    }
    return false;
  }
  
  // check whether the param file has changed - if so
  // read in new version
  
  if (_paramFileMtime < paramsStat.st_mtime) {
    if (_params.load((char *) paramFilePath.c_str(),
		     NULL, true, false)) {
      cerr << "ERROR - " << _progName
	   << " - Watcher::_paramsStillActive" << endl;
      cerr << "  " << DateTime::str() << endl;
      cerr << "  Dir: " << _dirPath << endl;
      cerr << "  Cannot reload local params from file: "
	   << paramFilePath << endl;
      return false;
    } else {
      if (_params.debug) {
	cerr << "Dir: " << _dirPath << endl;
	cerr << "  " << DateTime::str() << endl;
	cerr << "  Reading new version of param file." << endl;
      }
    }
    _paramFileMtime = paramsStat.st_mtime;
  }

  return true;
  
} 

