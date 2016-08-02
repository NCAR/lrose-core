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
/////////////////////////////////////////////////////////////
// StormFile.cc
//
// StormFile class
//
// Prepares the storm data file. If the file does not exist, it is created
// and the header is prepared and written. If the file exists and the params
// in the header do not match the current ones, the old file is overwritten.
// If the file exists and the parameters do match, the old file is opened for
// read/write. The file is read and the scans in it are compared with those
// in the time_list. The scan number and file markers are positioned
// immediately after the last scan which matches the file list.
// Processing will continue from this point.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#include "StormFile.hh"
#include "TimeList.hh"
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <toolsa/ldata_info.h>
#include <toolsa/file_io.h>
#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>
#include <dsserver/DsLdataInfo.hh>
using namespace std;

//////////////
// constructor
//

StormFile::StormFile(char *prog_name, Params *params)

{

  OK = TRUE;
  _progName = STRdup(prog_name);
  _params = params;
  RfInitStormFileHandle(&handle, _progName);

  // compute number of dBZ hist intervals

  nDbzHistIntervals = (int)
    ((_params->high_dbz_threshold - _params->low_dbz_threshold) /
     (_params->dbz_hist_interval) + 1);

}

/////////////
// destructor
//

StormFile::~StormFile()

{

  RfCloseStormFiles(&handle, "StormFile::~StormFile");
  RfFreeStormFileHandle(&handle);
  STRfree(_progName);

}

////////////////////////////////////////////////
// openAndCheck()
//
// Open file and check parameters. If params do not
// match, or there are no scans in file, we need to
// start a new file.
//
// Returns 0 on success, -1 on failure

int StormFile::openAndCheck(char *header_file_path,
			    storm_file_params_t *expected_params)
     
{
  
  // check if the file exists - if it does not exist, go to
  // end of loop
  
  struct stat file_stat;

  if (stat(header_file_path, &file_stat)) {
    // file does not exist, so no scans match
    return (-1);
  }
  
  // try to open the file read/write
  
  if (RfOpenStormFiles (&handle, "r",
			header_file_path,
			STORM_DATA_FILE_EXT,
			"StormFile::openAndCheck")) {
    
    // cannot successfully open files - no scans match
    
    fprintf(stderr,
	    "WARNING ONLY - %s:StormFile::openAndCheck\n",
	    _progName);
    fprintf(stderr, "IGNORE PREVIOUS ERROR from RfOpenStormFiles\n");
    fprintf(stderr, "New file will be created\n");
    
    return (-1);
    
  }
  
  // read in storm properties file header
  
  if (RfReadStormHeader (&handle, "StormFile::openAndCheck")) {
    if (_params->debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr,
	      "WARNING ONLY - %s:StormFile::openAndCheck\n",
	      _progName);
      fprintf(stderr, "IGNORE PREVIOUS ERROR from RfReadStormHeader\n");
      fprintf(stderr, "File corrupted - preparing new file.\n");
    }
    return (-1);
  }
  
  // compare the old file params with the current ones - if they do
  // not match, no scans match
    
  if (memcmp(&handle.header->params, expected_params,
	     (size_t) sizeof(storm_file_params_t))) {
    if (_params->debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr,
	      "WARNING - %s:StormFile::openAndCheck\n",
	      _progName);
      fprintf(stderr, "Parameters do not match.\n");
      fprintf(stderr, "New file will be started.\n");
    }
    return (-1);
  }

  return (0);

}
  
/////////////////////////////////////////////////////////////////////
// getPrevScan()
//
// Returns the prev used scan number.
// This is the scan up to and including which the storm
// file matches the file list.
// If no scans applicable, returns -1
//
// Also adjusts the time list to start at the next scan to be processed,
// and adjusts ntimes_in_list to the number of unused entries.
//

int StormFile::getPrevScan(const TimeList &time_list,
			   int &start_posn)
     
{
  
  int initial_match_found;
  int initial_scan_match;
  int last_scan_match;
  int last_time_match;
  start_posn = 0;

  // Search through the storms file for last time
  // which matches time list
  
  initial_match_found = FALSE;
  
  for (int iscan = 0; iscan < handle.header->n_scans; iscan++) {
    
    // read in scan
    
    if (RfReadStormScan(&handle, iscan,
			"get_prev_scan")) {
      // error in old file, no scans match
      return (-1L);
    }

    if (time_list.times[0] == handle.scan->time) {
      initial_scan_match = iscan;
      initial_match_found = TRUE;
      break;
    }
    
  } // iscan
  
  if (initial_match_found) {
      
    // Found a scan which matches the first file in the list.
    // Now match up the file list with the scan list to determine
    // at which scan the two lists differ.
      
    _getLastMatch(initial_scan_match,
		  &last_scan_match, &last_time_match,
		  time_list);

    start_posn = last_time_match + 1;
    
    // print out file list
    
    if (_params->debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "\nActive time list:\n");
      time_list.print(stderr);
    }
    
    return (last_scan_match);

  } else {
      
    // fell through loop
    //
    // the first time in the list does not match any of the
    // scans in the storms file, so no scans match
    
    return (-1);
    
  } // if (initial_match_found)
  
}

//////////////////////
// prepareNew()
//
// Prepare a new file
//
// Returns 0 on success, -1 on failure

int StormFile::prepareNew(char *header_file_path,
			  storm_file_params_t *sparams)
     
{
  
  // open the file write/read
  
  if (RfOpenStormFiles (&handle, "w+", header_file_path,
			STORM_DATA_FILE_EXT, "StormFile::prepareNew")) {
    fprintf(stderr, "ERROR - %s:StormFile::prepareNew\n",  _progName);
    fprintf(stderr, "Cannot open storm file mode 'w+'.\n");
    perror(header_file_path);
    return(-1);
  }
  
  if (RfAllocStormHeader(&handle, "StormFile::prepareNew")) {
    return(-1);
  }
  
  // copy storm file params to header
  
  handle.header->params = *sparams;
  
  // write storm file header
  
  if (RfWriteStormHeader(&handle, "StormFile::prepareNew")) {
    return(-1);
  }
  
  // set the data file at the correct point for data writes -
  // after the label
  
  if (RfSeekStartStormData(&handle, "StormFile::prepareNew")) {
    return(-1);
  }
  
  // flush the buffer
  
  if (RfFlushStormFiles(&handle,
			"StormFile::prepareNew")) {
    fprintf(stderr,
	    "ERROR - %s:StormFile::prepareNew\n", _progName);
    fprintf(stderr, "Cannot flush storm props files.\n");
    perror(header_file_path);
    return(-1);
  }
  
  return (0);
  
}

//////////////////////
// prepareOld()
//
// Prepare a old file
//
// Returns 0 on success, -1 on failure

int StormFile::prepareOld(char *header_file_path,
			  int current_scan_num)
     
{

  int trunc_flag;
  int init_data_len;
  int init_header_len;

  //   * open the file read/write
  
  if (RfOpenStormFiles (&handle, "r+", header_file_path,
			STORM_DATA_FILE_EXT,
			"StormFile::prepareOld")) {
    fprintf(stderr, "ERROR - %s:StormFile::prepareOld\n", _progName);
    fprintf(stderr, "Cannot open storm file mode 'r+'.\n");
    perror(header_file_path);
    return(-1);
  }
  
  // read in header
  
  if (RfReadStormHeader(&handle,"StormFile::prepareOld")) {
    return(-1);
  }
  
  // read in last valid scan
  
  if (RfReadStormScan(&handle, current_scan_num,
		      "StormFile::prepareOld")) {
    fprintf(stderr, "Reading in last valid scan # %d.\n", current_scan_num);
    return(-1);
  }
  
  // check whether file will need truncation
  
  if (current_scan_num < handle.header->n_scans - 1) {
    init_data_len = handle.scan->last_offset + 1;
    init_header_len = (R_FILE_LABEL_LEN + sizeof(storm_file_header_t) +
		       (current_scan_num + 1) * sizeof(si32));
    trunc_flag = TRUE;
  } else {
    trunc_flag =  FALSE;
  }
  
  // copy scan time to header as end time
  
  handle.header->end_time = handle.scan->time;
  
  // set other parameters
  
  handle.header->n_scans = current_scan_num + 1;
  
  // write storm file header
  
  if (RfWriteStormHeader(&handle, "StormFile::prepareOld")) {
    return(-1);
  }
  
  // truncate if necessary, and position the file
  
  if (trunc_flag) {
    
    if (_truncate(&handle.header_file, handle.header_file_path,
		  init_header_len)) {
      return (-1);
    }

    if (_truncate(&handle.data_file, handle.data_file_path,
		  init_data_len)) {
      return (-1);
    }
    
  }
  
  // position at end of file
  
  if (RfSeekEndStormData(&handle, "StormFile::prepareOld")) {
    return(-1);
  }
  
  // flush the buffer
  
  if (RfFlushStormFiles(&handle,"StormFile::prepareOld")) {
    fprintf(stderr, "ERROR - %s:StormFile::prepareOld\n", _progName);
    fprintf(stderr, "Cannot flush storm props files.\n");
    perror(header_file_path);
    return(-1);
  }
  
  return (0);
  
}

////////////////////////////////////////////////////////////////////
// _getLastMatch()
//
// Move through the storm file, matching up successive scans to the
// available times in the list, returning the scan number of the
// last match.

void StormFile::_getLastMatch(int initial_scan_match,
			      int *last_scan_match_p,
			      int *last_time_match_p,
			      const TimeList &time_list)

{

  int itime;
  int iscan = initial_scan_match;
  
  for (itime = 0; itime < time_list.nTimes; itime++, iscan++) {
    
    // read in scan
    
    if (RfReadStormScan(&handle, iscan,
			"StormFile::_getLastMatch")) {
      
      if (_params->debug >= Params::DEBUG_VERBOSE) {
	fprintf(stderr,
		"WARNING ONLY - %s:StormFile::_getLastMatch\n",
		_progName);
	fprintf(stderr, "IGNORE PREVIOUS ERROR from RfReadStormScan\n");
	fprintf(stderr, "Time corrupted - preparing new file.\n");
      }
      
      *last_time_match_p = itime - 1;
      *last_scan_match_p = iscan - 1;
      return;
      
    }
    
    // compare the time time with the scan time, to see if they match.
    
    if (time_list.times[itime] != handle.scan->time) {
      
      // Times do not match. Therefore, the scan in the storms
      // file is not in sequence.
      
      *last_time_match_p = itime - 1;
      *last_scan_match_p = iscan - 1;
      return;
      
    }
    
    if (iscan == handle.header->n_scans - 1) {
      
      // we are at the end of the storm file
      
      *last_time_match_p = itime;
      *last_scan_match_p = iscan;
      return;
      
    }
    
  } // itime
  
  // all scans match
  
  *last_time_match_p = itime - 1;
  *last_scan_match_p = iscan - 1;
  return;
  
}

//////////////////////
// _truncate
//
// Truncate open file - close, truncate and reopen
//
// Returns 0 on success, -1 on failure
//

int StormFile::_truncate(FILE **fd, char *path, int length)
     
{
  
  int low_d;
  
  // close the buffered file
  
  fclose(*fd);
  
  // open for low-level io
  
  if ((low_d = open(path, O_WRONLY)) < 0) {
    fprintf(stderr, "ERROR - %s:StormFile::_truncate\n", _progName);
    fprintf(stderr,
	    "Cannot open storm data file - low level - for truncation.\n");
    perror(path);
    return(-1);
  }
  
  // truncate the file
  
  if (ftruncate(low_d, length) != 0) {
    fprintf(stderr, "ERROR - %s:StormFile::_truncate\n",  _progName);
    fprintf(stderr, "Cannot truncate storm props file.\n");
    perror(path);
    return(-1);
  }
  
  // close low-level io
  
  close(low_d);
  
  // re-open the file for buffered i/o
  
  if ((*fd = fopen(path, "r+")) == NULL) {
    fprintf(stderr, "ERROR - %s:StormFile::_truncate\n", _progName);
    fprintf(stderr, "Cannot re-open file.\n");
    perror(path);
    return(-1);
  }

  return (0);
  
}

//////////////////////////////
// writeLdataInfo()
//
// Write latest data info

void StormFile::writeLdataInfo()

{

  // parse storm file path to get base part of path
  // (i.e. name without extension)
  
  path_parts_t parts;
  uparse_path(handle.header_file_path, &parts);

  // write info
  
  //
  // Write out an index file
  //
  DsLdataInfo ldata(_params->storm_data_dir);
  ldata.setDataFileExt(STORM_HEADER_FILE_EXT);
  ldata.setWriter(_progName);
  ldata.setRelDataPath(parts.name);
  ldata.setUserInfo1(parts.base);

  if (ldata.write(handle.header->end_time)) {
    fprintf(stderr, "WARNING - StormFile::writeLdataInfo\n");
    fprintf(stderr, "DsLdata write failed\n");
  }
  
  // free path parts
  
  ufree_parsed_path(&parts);
  
  return;
  
}

///////////////////////////////////////////////////////////////
// lockHeaderFile()
//
// Put an advisory write lock on the header file.
// This is used for synchronization with client programs reading
// the storm files. Client programs should not read the file
// while this lock is in place. The clients should place a 
// read lock on the header file while reading.
//
// Returns 0 on success, -1 on failure.

int StormFile::lockHeaderFile()

{

  if (ta_lock_file_procmap(handle.header_file_path,
			   handle.header_file, "w")) {
    fprintf(stderr, "ERROR - %s:StormFile::lockHeaderFile\n", _progName);
    return (-1);
  }
  return (0);
}

///////////////////////////////////////////////////////////////
// unlockHeaderFile()
//
// Remove advisory lock on the header file.
// See lockHeaderFile().
//
// Returns 0 on success, -1 on failure.

int StormFile::unlockHeaderFile()

{

  if (ta_unlock_file(handle.header_file_path,
		     handle.header_file)) {
    fprintf(stderr, "ERROR - %s:StormFile::unlockHeaderFile\n", _progName);
    return (-1);
  }
  return (0);

}



