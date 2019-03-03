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
// Directory.cc
//
// Process a given directory.
//
// Niles Oien and
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 1998
//
//////////////////////////////////////////////////////////

/**
 * @file Directory.cc
*
 * Object class handles Janitor processing of a directory.
 *
 * @author Niles Oien and Mike Dixon
 * @see something
 */

#include <dirent.h>
#include <math.h> // for ceil function
#include <cstdlib>
#include <cerrno>
#include <string>
#include <map>
#include <unistd.h> // For rmdir function to delete empty directories.
#include <sys/stat.h>

#include <didss/DataFileNames.hh>  // For looking at files generally
#include <toolsa/file_io.h> // For locking.
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Directory.hh"
#include "EventList.hh"      // For looking at eventlists
#include "Params.hh"
using namespace std;

///////////////
// Constructor

Directory::Directory (const string prog_name, 
                      Params *global_params,
		      const string top_dir,
		      const string dir_path,
		      DiskFullDeleteList *delete_list) :
        _progName(prog_name),
        _delete(NULL),
        _topDir(top_dir),
        _dirPath(dir_path),
        _paramsAreLocal(false),
        _diskFullDeleteList(delete_list)
{
  OK = false;

  // Load the local params file, if one exists

  bool found_params = false;
  
  if (_loadLocalParams("_janitor", *global_params))
  {
    found_params = true;
  }
  else
  {
    if (_loadLocalParams("_Janitor", *global_params))
      found_params = true;
  }
  
  //
  // If no parameters found, use existing ones.
  //
  if (!found_params)
  {
    _params = global_params;
    _paramsAreLocal = false;
  }

  // Initialize private members

  _nFilesFound = 0;
  _nFilesRemaining = 0;
  _nFilesDeleted = 0;

  _nFilesCompressed = 0;
  _nBytesUncompressed = 0;
  _nBytesCompressed = 0;
 
  _eventListsOkay = true; 

  // create deletion object

  _delete = new Delete(prog_name, _params);

  // set age in secs for file tests

  if (_params->file_ages_in_days) {
    _maxNoModSecsDelete = (int) (_params->MaxNoModDays * 86400.0 + 0.5);
    _maxDirAgeSecs = (int) (_params->MaxDirAgeDays * 86400.0 + 0.5);
    _maxNoAccessSecsCompress =
      (int) (_params->MaxNoAccessDays * 86400.0 + 0.5);
    _maxNoModSecsCompress =
      (int) (_params->MaxNoModDaysCompress * 86400.0 + 0.5);
  } else {
    _maxNoModSecsDelete = _params->MaxModificationAgeBeforeDelete;
    _maxDirAgeSecs = _params->MaxDirAgeSecs;
    _maxNoAccessSecsCompress = _params->MaxAccessAgeBeforeCompress;
    _maxNoModSecsCompress = _params->MaxNoModSecsCompress;
  }

  if (_params->debug >= Params::DEBUG_EXTRA) {
    cerr << "==================================================================" << endl;
    cerr << "Processing dir: " << _dirPath << endl;
    cerr << "  time : " << utimstr(time(NULL)) << endl;
    cerr << "  maxNoModSecsDelete: " << _maxNoModSecsDelete << endl;
    cerr << "  maxDirAgeSecs: " << _maxDirAgeSecs << endl;
    cerr << "  maxNoAccessSecsCompress: " << _maxNoAccessSecsCompress << endl;
    cerr << "  maxNoModSecsCompress: " << _maxNoModSecsCompress << endl;
    _writeParams(stderr);
  }
  OK = true;

}

/////////////
// Destructor

Directory::~Directory ()
{
  if (_paramsAreLocal && _params) {
    delete (_params);
  }
  if (_delete) {
    delete _delete;
  }
}
  

////////////////////////
// process()
//
// Process the directory
//
// Returns 0 on success, -1 on failure.
// Returns 1 if the directory is locked by another
// instance of the Janitor or fails for some other reason.
// Returning 0 entries may mean the directory will be deleted.
//

int Directory::process()
{

  // return now if process and recurse are both false

  if (!_params->recurse && !_params->process) {
    if (_params->debug) {
      cerr << "Both recurse and process are false, ignoring dir: "
           << _dirPath << endl;
    }
    return 0;
  }

  // If we are to check the hostname, do so, and
  // return if the hostname does not match.

  if (_params->HostnameMustMatch)
  {
    if (!_hostNamesMatch(_params->Hostname))
      return 1;
  }

  // Create the lock file so we can process the directory safely

  string lock_file = _dirPath + PATH_DELIM + "_janitor_dir.lock";
  FILE *lfp=NULL;

  if (_params->debug >= Params::DEBUG_EXTRA) {
    cerr << "Creating lock file: " << lock_file << endl;
  }

  if (_params->UseLockfiles)
  {
    // See if the directory is locked by another Janitor.

    lfp = ta_create_lock_file(lock_file.c_str());

    if (lfp == NULL) {
      // Directory is already locked.
      cerr << "Either directory is locked or an error occurred creating lock file."
           << endl;
      return 1; // return 1 so that directory is not removed.
    }
  }
 
  if (_params->debug >= Params::DEBUG_EXTRA) {
    cerr << "Lock file created: " << lock_file << endl;
  }

  // Try to open the directory
  
  DIR *dirp;
  if ((dirp = opendir(_dirPath.c_str())) == NULL)
  {
    cerr << "ERROR - " << _progName << ":Directory::process" << endl;
    cerr << "  Cannot open directory '" << _dirPath << "'" << endl;
    perror(_dirPath.c_str());
   
    // Unlock directory.
    if (_params->UseLockfiles) {
       ta_remove_lock_file(lock_file.c_str(), lfp);
     }
 
    return -1;
  }

  //
  // Check out the event list, if required.
  //

  string local_event_filename;
  int local_event_return;
  
  string master_event_filename;
  int master_event_return;
  
  _eventListsOkay = true;
  
  if (_params->ignoreEventlists)
  {
    local_event_filename = "None";
    local_event_return = 0;
    
    master_event_filename = "None";
    master_event_return = 0;
  }
  else
  {
    local_event_filename = _dirPath + PATH_DELIM + "_janitor_Eventlist"; 

    if (_params->debug >= Params::DEBUG_EXTRA) {
      cerr << "Checking event list: " << local_event_filename << endl;
    }

    _localEventList.SetVerbosity(EventList::ReportErrors); // Report problems.
    local_event_return = _localEventList.ReadEventList(local_event_filename);

    ////////////////////////////////////////
    // Added the following Master Eventlist
    /////////////////////////////////////

    _masterEventList.SetVerbosity(EventList::ReportErrors); // Report problems.

    // Construe the master eventlist filename.

    master_event_filename = _params->MasterEventsFile;

    master_event_return =
      _masterEventList.ReadEventList(master_event_filename);

    if ((!_params->ProcessIfNoMaster) &&  (master_event_return < 0))
    {
      cerr << "Cannot process without master events list." << endl;

      _eventListsOkay = false;

      return 1;
    }

    //// End of Master eventlist additions.
  } // End of if eventlists are pertinent.

  // See if we want to write a report file into this
  // directory - and if so, do it.

  if (_params->report) {
    _writeReport(_dirPath, local_event_filename, local_event_return,
                 master_event_filename, master_event_return);
  }

  // Loop thru directory looking for the data file names
  
  struct dirent *dp;

  // use this to keep track of all the files, and delete 
  // old ones if _params.MaxNumFilesInDir >= 0
  multimap<time_t, dirent*> timeSortedFiles;
  multimap<time_t, dirent*>::iterator tsfIx; 

  _nFilesFound = 0;
  _nFilesRemaining = 0;
  _nFilesDeleted = 0;

  _nFilesCompressed = 0;
  _nBytesUncompressed = 0;
  _nBytesCompressed = 0;
  
  if (_params->debug) {
    cerr << "*";
  }
  
  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
  {

    //
    // PMU checkin.
    //
    PMU_auto_register("Reading dir ...");

    // Don't process the "." and ".." files.  Also, don't include these
    // in the nFilesRemaining because we want to delete directories that only
    // have these files.

    if (strcmp(dp->d_name, ".") == 0 ||
	strcmp(dp->d_name, "..") == 0)
    {
      if (_params->debug >= Params::DEBUG_EXTRA) {
	cerr << "file name: '" << dp->d_name
             << "' - Janitor will not process this file" << endl;
      }
      continue;
    }

    // Don't process files that begin with a dot or an underscore.
    // These are generally control files that need to be kept.  Directories
    // including these files shouldn't be deleted so include them in
    // nFilesRemaining.

    if (dp->d_name[0]=='.' || dp->d_name[0]== '_')
    {
      if (_params->debug >= Params::DEBUG_EXTRA) {
	cerr << "file name: '" << dp->d_name
             << "' - Janitor will not process this file" << endl;
      }
      _nFilesFound++;
      _nFilesRemaining++;
      continue;
    }

    // Leave CVS directories alone. Niles.

    if (strcmp(dp->d_name, "CVS") == 0)
    {
      _nFilesFound++;
      _nFilesRemaining++;
      continue;
    }

    // Exclude any files named 'latest_data_info'
    // This is a special for Mike Dixon to work in with
    // the data mapper.

    if (strcmp(dp->d_name, "latest_data_info") == 0)
    {
      _nFilesFound++;
      _nFilesRemaining++;
      continue;
    }

	

    // Put together the file name.

    string current_path = _dirPath + PATH_DELIM + dp->d_name;
    
    if (_params->debug >= Params::DEBUG_EXTRA) {
      cerr << "-->> current_path: " << current_path << endl;
    }

    // Look at the file

    DataFileNames current_path_info;

    current_path_info.GetFileFacts(current_path,
				   _params->compressed_ext, _params->ext);

    //
    // The following is a VERY verbose debug.
    // fprintf(stdout,"\n\n%s\n",current_path);
    // current_path_info.PrintFileFacts();
    //

    if (!current_path_info.Exists)
    {
      if (_params->debug >= Params::DEBUG_NORM) {
	cerr << current_path << " - File doesn't exist" << endl;
      }
      continue; // Failed to stat file.
    }

    // Increment this now, so we don't delete directories
    // that have symbolic links in them.

    _nFilesFound++;
    _nFilesRemaining++;

    // See if we need to recurse

    if (current_path_info.Directory && _params->recurse)
    {

      // Bail out on potentially circular, path relative, soft linked dirs.
      if (!_params->FollowDangerousLinks &&
          (current_path_info.IsSoftLinkDir ||
           current_path_info.IsPathRelative)) {
        if (_params->debug >= Params::DEBUG_NORM) {
          cerr << "WARNING - dir is link or relative dir: " << current_path << endl;
          cerr << " Will be ignored" << endl;
        }
        continue;   // Avoid dangerous, possibly circular links
      }

      _processDirectory(current_path);
      
      //
      // Sleep for sleep_factor between directories 
      // to avoid hogging the file system.
      //

      for (int k = 0; k < _params->SleepBetweenDirs; ++k)
      {
	    PMU_auto_register("Sleeping Between Dir Scans");
	    sleep(1);
      }

      PMU_auto_register("End of dir");

      continue;

    } // End of if it's a directory

    // Process regular files

    if (current_path_info.Regular && _params->process)
    {
      if (_params->debug >= Params::DEBUG_EXTRA) {
	cerr << current_path << " - is regular file" << endl;
      }

      bool file_deleted;
      _processFile(current_path, current_path_info, file_deleted);
      
      if (file_deleted) {
	_nFilesRemaining--;
        _nFilesDeleted++;
      } else {
	if (_params->MaxNumFilesInDir >= 0)
	  timeSortedFiles.insert(pair<time_t, dirent*>(current_path_info.ModTime,dp));
      }
      //
      // End of somewhat involved check on if
      // we delete the file.
      //
      continue;
    } // End of if it's a regular file.
    
    if (_params->process && _params->debug >= Params::DEBUG_EXTRA) {
      cerr << current_path << " - is not a regular file" << endl;
    }
    
  } // End of loop through directory.
  
  closedir(dirp);

  //delete old files if necessary
  if (_params->MaxNumFilesInDir >= 0)
    {
      cerr << "Considering deleting files to reduce to " << _params->MaxNumFilesInDir << " files\n";
      int tsfCount;
      for(tsfCount = 1, tsfIx = timeSortedFiles.begin(); 
	  tsfIx != timeSortedFiles.end(); tsfIx++, tsfCount++)
	if (tsfCount > _params->MaxNumFilesInDir)
	  {
	    string current_path = _dirPath + PATH_DELIM + tsfIx->second->d_name;
	    if (_delete->removeFile(current_path) == 0) {
              if (_params->debug >=Params::DEBUG_VERBOSE) {
		cerr << "Deleted file (" << tsfCount << "): " << current_path << endl;
              } else if (_params->debug) {
		cerr << "D";
              }
              _nFilesRemaining--;
              _nFilesDeleted++;
	    }
	  }
    }

  // Unlock directory.
  if (_params->UseLockfiles) {
    ta_remove_lock_file(lock_file.c_str(), lfp);
  }

  PMU_auto_register("Done with dir");

  time_t now = time(NULL);
  bool doDebugPrint = false;
  if (_params->debug >=Params::DEBUG_VERBOSE) {
    doDebugPrint = true;
  }
  if (_params->debug >=Params::DEBUG_NORM && _nFilesDeleted > 0) {
    doDebugPrint = true;
  }
  if (_params->debug >=Params::DEBUG_NORM && _nFilesCompressed > 0) {
    doDebugPrint = true;
  }
  
  if (doDebugPrint) {

    cerr << endl;
    cerr << "Dir: " << _dirPath << endl;

    if (_nFilesDeleted > 0) {
      cerr << "    Number of files found     : " << _nFilesFound << endl;
      cerr << "    Number of files deleted   : " << _nFilesDeleted << endl;
      cerr << "    Number of files remaining : " << _nFilesRemaining << endl;
    }

    if (_nFilesCompressed > 0) {
      cerr << "    Number of files compressed: " << _nFilesCompressed << endl;
      cerr << "    Number of Mbytes uncompressed: "
           << _nBytesUncompressed / 1000000.0 << endl;
      cerr << "    Number of Mbytes compressed: "
           << _nBytesCompressed / 1000000.0 << endl;
      cerr << "    Percent after compression: "
           << ((double) _nBytesCompressed / (double) _nBytesUncompressed) * 100.0
           << endl;
    }
    
  }

  if (_params->debug >= Params::DEBUG_EXTRA) {
    cerr << endl;
    cerr << "==>> Time : " << utimstr(now) << endl;
    cerr << "==>> Done with dir '" << _dirPath << endl;
    cerr << "==================================================================" << endl;
  }

  return 0;
}


/*************************************************************************
 * Private methods
 *************************************************************************/

bool Directory::_loadLocalParams(const string param_filename,
				 const Params &global_params)
{
  string params_path;
  struct stat file_stat;

  params_path = _dirPath + PATH_DELIM + param_filename;

  if (stat(params_path.c_str(), &file_stat) == 0 && S_ISREG(file_stat.st_mode))
  {
    if (global_params.debug) {
      cerr << endl;
      cerr << "==>> Local parameter file found: " << params_path << endl;
    }

    _params = new Params(global_params);
    _paramsAreLocal = true;

    _params->load((char *)params_path.c_str(), NULL, TRUE,
		  (_params->debug >= Params::DEBUG_VERBOSE));

    return true;
  }

  return false;
}


bool Directory::_hostNamesMatch(const string expected_hostname) const
{
  char my_host_name[MAX_HOST_LEN];
  gethostname(my_host_name, MAX_HOST_LEN);

  if (strcmp(my_host_name, expected_hostname.c_str()) == 0)
    return true;
  
  if (_params->debug >= Params::DEBUG_NORM) {
    cerr << "Hostname is " << my_host_name << " not " <<
      expected_hostname << endl;
  }

  return false;
}


bool Directory::_dirMatchesDateFormat(const string dir_path)
{
  string last_bit;

  //
  // First, try YYYYMMDD
  //

  if (dir_path.length() >= strlen("YYYYMMDD"))
  {
    last_bit = string(dir_path,
		      dir_path.length() - strlen("YYYYMMDD"),
		      strlen("YYYYMMDD"));
	  
    int year, month, day;
    
    if ((3 == sscanf(last_bit.c_str(),"%4d%2d%2d",
		     &year, &month, &day)) && 
	(year > 1970) && (year < 2050) &&
	(month > 0) && (month < 13) &&
	(day > 0) && (day < 32))
      return true;
  }

  //
  // Then, try g_hhmmss
  //
  if (dir_path.length() >= strlen("g_hhmmss"))
  {
    last_bit = string(dir_path,
		      dir_path.length() - strlen("g_hhmmss"),
		      strlen("g_hhmmss"));
	  
    int hour, min, sec;
    
    if ((3 == sscanf(last_bit.c_str(), "g_%2d%2d%2d",
		     &hour, &min, &sec)) && 
	(hour > -1) && (hour < 24) &&
	(min > -1) && (min < 61) &&
	(sec > -1) && (sec < 61))
      return true;
  }

  return false;
}

void Directory::_processDirectory(const string current_path)
{
  // If we are recursing, we must save the current _params->process
  // flag and then recurse with process set. Pop the process flag off
  // the stack after recursing.

  tdrp_bool_t process_now;
  process_now = _params->process; // Save current state.

  // If recurse is set, then this is implied.

  _params->process = pTRUE;

  Directory *dir = new Directory(_progName, _params,
				 _topDir, current_path,
				 _diskFullDeleteList);
  int iret = dir->process();

  if (iret < 0)
  {
    cerr << "ERROR processing dir: " << current_path << endl;
  }
  else if ((iret == 0) && (_params->RemoveEmptyDirs))
  {

    // stat this directory to get the age
    
    stat_struct_t dStat;
    int ret = ta_stat(current_path.c_str(), &dStat);
    if (ret == -1)
    {
      cerr << "ERROR processing dir: " << current_path << endl;
    }
    else
    {
       int age = time(NULL) - dStat.st_mtime;
    
       // Determine if we can remove the directory

       bool do_remove = FALSE;

       if (age > _maxDirAgeSecs)
         {
	   if (_params->date_format)
	     {
	       if (_dirMatchesDateFormat(current_path))
	         do_remove = TRUE;
	     }
	   else
	     {
	       do_remove = TRUE;
	     }
         }

       if (!_isEmpty(current_path)) {
         do_remove = FALSE;
       }

       // Remove the directory if we can

       if (do_remove)
       {
         if (_params->debug) {
	   cerr << "Removing empty dir: " << current_path << endl;
         }

         if (rmdir(current_path.c_str())) {
	   perror("rmdir failed");
	   cerr << "Failed to remove empty directory: " << current_path
                << endl;
         }
       }

     }
  }
  
  delete(dir);
   
  _params->process = process_now; // Restore current state.
}


void Directory::_processFile(const string current_path,
			     const DataFileNames &current_path_info,
			     bool &file_deleted)
{

  if (_params->debug >= Params::DEBUG_EXTRA) {
    cerr << "=====================================" << endl;
    cerr << "current_path: " << current_path << endl;
    current_path_info.PrintFileFacts(stderr);
    cerr << "=====================================" << endl;
  }

  // Initialize returned values

  file_deleted = false;
  
  // If we insist on the date being valid in the filename,
  // and it isn't, then continue on out.

  if (!current_path_info.NameDateValid && _params->date_format)
  {
    if (_params->debug >= Params::DEBUG_EXTRA) {
      cerr << "      File not processed because name isn't in date format"
           << endl;
    }
    
    return;
  }
  
  // If we require that the extension matched, and it
  // it didn't, then get out.
  
  if (_params->check_ext && !current_path_info.ExtMatches)
  {
    if (_params->debug >= Params::DEBUG_EXTRA) {
      cerr << "     File not processed because ext doesn't match" << endl;
    }
    
    return;
  }
  
  if (!_params->ignoreEventlists)
  {
    // Don't delete files if the event lists are messed up.  This means
    // that there is no master event list although one is required according
    // to the parameter file.

    if (!_eventListsOkay)
    {
      if (_params->debug >= Params::DEBUG_EXTRA) {
	cerr << "     File not processed because event list not okay" << endl;
      }
      return;
    }
    
    // If the filename contained a valid date and that date is on the
    // event list, get out. This means that the file will not be deleted
    // or compressed in this case - the eventlist as taken to mean that the
    // janitor should leave the file alone, unconditionally.

    if (current_path_info.NameDateValid && 
	(_localEventList.TimeInEventList(current_path_info.NameDate) ||
	 _masterEventList.TimeInEventList(current_path_info.NameDate)))
    {
      if (_params->debug >= Params::DEBUG_EXTRA) {
	cerr << "     File not processed because time is in an event list"
             << endl;
      }
      return;
    }
    
    // Deal with the case in which the filename specified the date, but not
    // the time of day, as in YYYYMMDD.spdb

    if (current_path_info.DateOnly)
    {
      if (current_path_info.NameDateValid && 
	  (_localEventList.DayInEventList(current_path_info.NameDate) ||
	   _masterEventList.DayInEventList(current_path_info.NameDate)))
      {
	if (_params->debug >= Params::DEBUG_EXTRA) {
	  cerr << "     File not processed becase date is in an event list"
               << endl;
        }
	return;
      }
      
    }
    
  }

  PMU_auto_register("Processing file ...");

  // See if the file should be deleted.  Do this before checking to see
  // if the file should be compressed because there is no sense in
  // compressing a file that's just going to be deleted.

  if (_params->delete_files) {

    if (_params->debug >= Params::DEBUG_EXTRA) {
      cerr << "Checking whether to delete" << endl;
      cerr << "  Time since modification: " << current_path_info.ModTime << endl;
      cerr << "  maxNoModSecs: " << _maxNoModSecsDelete << endl;
    }
    
    if (current_path_info.ModTime > _maxNoModSecsDelete) {
      
      if (_params->debug >= Params::DEBUG_EXTRA) {
        cerr << " File too old - WILL delete" << endl;
      }

      if (_delete->removeFile(current_path) == 0) {
        if (_params->debug >=Params::DEBUG_VERBOSE) {
	  cerr << "Deleted file: " << current_path << endl;
        } else if (_params->debug) {
          cerr << "D";
        }
	file_deleted = true;
      }
      return;

    } else {

      if (_params->debug >= Params::DEBUG_EXTRA) {
        cerr << " File too young - WILL NOT delete" << endl;
      }

    }

  } // if (_params->delete_files)

  // Do we compress?

  bool doCompress = false;
  if (_params->compress && !current_path_info.Compressed) {
    if (_params->CompressBasedOnAccessTime) {
      if (current_path_info.AccessTime > _maxNoAccessSecsCompress) {
        doCompress = true;
      }
    } else {
      if (current_path_info.ModTime > _maxNoModSecsCompress) {
        doCompress = true;
      }
    }
  }
  if (doCompress) {
    _compressFile(current_path);
    return;
  } // End of if we compress.

  // If it isn't deleted based on age, add the file to the deletion
  // list to delete if the disk use threshold gets too high.

  if (_params->delete_files &&
      _params->disk_use_threshold < 100)
  {
    int percent_full = current_path_info.PercentFullDisk(current_path);
    if (percent_full == -1)
      return;

    if (percent_full > _params->disk_use_threshold)
    {
      // Add this path to the disk full deletion list.

      _diskFullDeleteList->addFile(current_path, current_path_info.ModTime,
				   _params->disk_use_threshold,
				   _params->disk_delete_threshold);
    }
  }
}


void Directory::_compressFile(const string file_path)
{

  // is this already compressed?

  if (file_path.find(".gz") != string::npos ||
      file_path.find(".tgz") != string::npos ||
      file_path.find(".Z") != string::npos) {
    // file already compressed
    return;
  }

  // get the uncompressed file size

  stat_struct_t fstat;
  if (ta_stat_compressed(file_path.c_str(), &fstat)) {
    int errNum = errno;
    cerr << "ERROR - Directory::_compressFile" << endl;
    cerr << "  Cannot stat file: " << file_path;
    cerr << "  " << strerror(errNum) << endl;
    return;
  }
  _nBytesUncompressed += fstat.st_size;

  // Put together the compress command.

  string command;

  for (size_t i = 0; i < strlen(_params->com_str); ++i)
  {
    if (_params->com_str[i] == '^')
      command += file_path;
    else
      command += _params->com_str[i];
  }

  if (_params->debug >= Params::DEBUG_EXTRA) {
    cerr << "Compressing with command: " << command << endl;
  }

  PMU_auto_register("Compressing ...");

  struct timeval tv;
  gettimeofday(&tv, NULL);
  double startTime = tv.tv_sec + tv.tv_usec * 1.0e-6;
  
  if (_params->debug >= Params::DEBUG_VERBOSE) {
    cerr << "Compressing file: " << file_path << endl;
    cerr << " command: " << command << endl;
  } else if (_params->debug == Params::DEBUG_NORM) {
    cerr << "C";
  }

  if (system(command.c_str()))
  {
    cerr << "The following compress command failed:" << endl;
    cerr << command << endl;
  }

  // get the compressed file size

  if (ta_stat_compressed(file_path.c_str(), &fstat)) {
    int errNum = errno;
    cerr << "ERROR - Directory::_compressFile" << endl;
    cerr << "  Cannot stat file: " << file_path;
    cerr << "  " << strerror(errNum) << endl;
    return;
  }
  _nBytesCompressed += fstat.st_size;
  _nFilesCompressed++;

  gettimeofday(&tv, NULL);
  double endTime = tv.tv_sec + tv.tv_usec * 1.0e-6;
  double elapsedSecs = endTime - startTime;
  int sleepMillisecs = (int) (elapsedSecs * _params->sleep_factor * 1000.0);
  if (_params->debug >= Params::DEBUG_VERBOSE) {
    cerr << "Sleeping after compression, msecs: " << sleepMillisecs << endl;
  }
  
  if (sleepMillisecs < 1000) {
    PMU_auto_register("zzzz ...");
    umsleep(sleepMillisecs);
  } else {
    int nleft = sleepMillisecs;
    while (nleft > 0) {
      PMU_auto_register("zzzz ...");
      umsleep(1000);
      nleft -= 1000;
    }
  }

}

/////////////////////////////////
// determine if directory is empty
// returns trur if empty, false otherwise

bool Directory::_isEmpty(const string dirPath)

{
  
  if (_params-> debug >= Params::DEBUG_EXTRA) {
    cerr << "Checking if dir is empty: " << dirPath << endl;
  }

  DIR *dirp;
  if ((dirp = opendir(dirPath.c_str())) == NULL) {
    cerr << "ERROR - Directory::_isEmpty" << endl;
    perror(dirPath.c_str());
    return false;
  }

  struct dirent *dp;
  int nEntries = 0;
  vector<string> entries;
  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
    if (strcmp(dp->d_name, ".") == 0 ||
	strcmp(dp->d_name, "..") == 0) {
      continue;
    }
    entries.push_back(dp->d_name);
    nEntries++;
  }
  closedir(dirp);

  if (_params-> debug >= Params::DEBUG_EXTRA) {
    cerr << "  Found entries: ";
    for (size_t ii = 0; ii < entries.size(); ii++) {
      cerr << "'" << entries[ii] << "'";
      if (ii != entries.size() - 1) {
        cerr << ", ";
      }
    }
    cerr << endl;
  }

  if (nEntries > 0) {
    if (_params-> debug >= Params::DEBUG_VERBOSE) {
      cerr << "  Dir not empty: dir: " << dirPath << endl;
      cerr << "    nEntries: " << nEntries << endl;
    }
    return false;
  }
  
  return true;

}

//////////////////////////////////////////////////////////////////////

void Directory::_writeReport(const string dirName,
			     const string local_event_filename,
			     int local_event_return,
			     const string master_event_filename,
			     int master_event_return)
{
  
  // Writes a report out to file _janitor_Report
  // Turn the report field off - this doesn't recurse.

  _params->report=pFALSE;

  // Put together the file name.

  string filename = dirName + PATH_DELIM + "_janitor_Report";

  FILE *fp=fopen(filename.c_str(),"wa");
  if (fp==NULL) {
    fprintf(stderr,"Failed to create report %s\n",filename.c_str());
    return;
  }

  date_time_t now;

  ulocaltime(&now);

  fprintf(fp,
	  "\nJanitor report, local time %d/%02d/%02d %02d:%02d:%02d\nFrom %s\n\n",
	  now.year,now.month,now.day,
	  now.hour,now.min,now.sec,
	  dirName.c_str());

  fprintf(fp,"Event file %s :\n\t\t",local_event_filename.c_str());
  if (local_event_return == -2) fprintf(fp,"-- Too many events!\n");
  if (local_event_return == -1) fprintf(fp,"-- Does not exist.\n");
  if (local_event_return == 0) fprintf(fp,"-- File contains no valid events!\n");
  if (local_event_return > 0) fprintf(fp,"-- %d events read.\n",local_event_return);

  fprintf(fp,"MasterEvent file %s :\n\t\t",master_event_filename.c_str());
  if (master_event_return == -2) fprintf(fp,"-- Too many events!\n");
  if (master_event_return == -1) fprintf(fp,"-- Does not exist.\n");
  if (master_event_return == 0) fprintf(fp,"-- File contains no valid events!\n");
  if (master_event_return > 0) fprintf(fp,"-- %d events read.\n",master_event_return);



  if (_params->debug==Params::DEBUG_OFF) {
    fprintf(fp,"Debugging :\tOFF\n");
  }

  if (_params->debug==Params::DEBUG_EXTRA) {
    fprintf(fp,"Debugging :\tEXTRA\n");
  } else if (_params->debug==Params::DEBUG_VERBOSE) {
    fprintf(fp,"Debugging :\tVERBOSE\n");
  } else if (_params->debug==Params::DEBUG_NORM) {
    fprintf(fp,"Debugging :\tNORM\n");
  }

  _writeParams(fp);
  fclose(fp);

}

//////////////////////////////////////////////////////////////////////

void Directory::_writeParams(FILE *fp)

{
  
  fprintf(fp,"Recurse to lower directories : \t");
  if (_params->recurse) fprintf(fp,"TRUE\n");
  else fprintf(fp,"FALSE\n");

  fprintf(fp,"Process this directory : \t");
  if (_params->process) fprintf(fp,"TRUE\n");
  else fprintf(fp,"FALSE\n");

  fprintf(fp,"Compress files : \t");
  if (_params->compress) fprintf(fp,"TRUE\n");
  else fprintf(fp,"FALSE\n");

  fprintf(fp,"Delete files : \t");
  if (_params->delete_files) fprintf(fp,"TRUE\n");
  else fprintf(fp,"FALSE\n");

  fprintf(fp,"Remove empty directories : \t");
  if (_params->RemoveEmptyDirs) fprintf(fp,"TRUE\n");
  else fprintf(fp,"FALSE\n");

  fprintf(fp,"Filenames must contain date : \t");
  if (_params->date_format) fprintf(fp,"TRUE\n");
  else fprintf(fp,"FALSE\n");

  fprintf(fp,"Filenames must have correct extension : \t");
  if (_params->check_ext) {
    fprintf(fp,"TRUE\n");
    fprintf(fp,"Correct extension :\t%s\n",_params->ext);
  }  else {
    fprintf(fp,"FALSE\n");
  }

  fprintf(fp,"Hostname must match : \t");
  if (_params->HostnameMustMatch) {
    fprintf(fp,"TRUE\n");
    fprintf(fp,"Correct hostname :\t%s\n",_params->Hostname);
  }  else { 
    fprintf(fp,"FALSE\n");
  }

  fprintf(fp,"Max modification age before delete (secs) \t %d (%f days)\n",
	  _maxNoModSecsDelete, float(_maxNoModSecsDelete)/86400.0);
  
  fprintf(fp,"Max dir age before delete (secs) \t %d (%f days)\n",
	  _maxDirAgeSecs, float(_maxDirAgeSecs)/86400.0);
  
  fprintf(fp,"Max access age before zip (secs) \t %d (%f days)\n",
	  _maxNoAccessSecsCompress, float(_maxNoAccessSecsCompress)/86400.0);
  
  fprintf(fp,"Sleep factor \t %g\n",
	  _params->sleep_factor);
  fprintf(fp,"Disk use threshold (percent) \t %d\n",
	  _params->disk_use_threshold);
  fprintf(fp, "Disk deletion threshold (percent) \t %d\n",
	  _params->disk_delete_threshold);
  
  fprintf(fp,"Extension added to filenames on compression \t %s\n",
	  _params->compressed_ext);
  fprintf(fp,"Compression command (with carat for filename) : \t %s\n",
	  _params->com_str);

}

