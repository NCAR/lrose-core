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
// ArchiveProcessor.cc: Class which controls a data directory in the
//               Archiver program.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// September 2001
//
//////////////////////////////////////////////////////////

#include <dirent.h>
#include <math.h> // for ceil function
#include <cstdlib>
#include <string>
#include <unistd.h> // For rmdir function to delete empty directories.
#include <sys/stat.h>

#include <didss/DataFileNames.hh>  // For looking at files generally
#include <toolsa/file_io.h> // For locking.
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "ArchiveProcessor.hh"
#include "Params.hh"
using namespace std;

///////////////
// Constructor

ArchiveProcessor::ArchiveProcessor(const string &prog_name,
				   const Params::debug_t debug_level) :
  _progName(prog_name),
  _debugLevel(debug_level)
{
  // Do nothing
}


/////////////
// Destructor

ArchiveProcessor::~ArchiveProcessor ()
{
  // Do nothing
}
  

////////////////////////
// processDir()
//
// Process the directory
//
// Returns number of directory entries on success,
// -1 on failure. Returns 1 if the directory is locked by another
// instance of the Janitor or fails for some other reason.
// Returning 0 entries may mean the directory will be deleted.
//

bool ArchiveProcessor::processDir(const string &data_dir,
				  const string &archive_dir,
				  Params *params,
				  const time_t start_time,
				  const time_t end_time)
{
  // Load the local params file, if one exists

  bool found_params = false;
  Params *local_params;
  
  if ((local_params = _loadLocalParams(data_dir, *params)) == NULL)
  {
    local_params = params;
    found_params = false;
  }
  else
  {
    found_params = true;
  }
  
  // Tell the user which directory we're processing

  if (_debugLevel >= Params::DEBUG_NORM)
    cerr << "---> Processing dir '" << data_dir << "'" << endl;

  // See if we're supposed to process this directory

  if (!params->process_dir)
  {
    if (_debugLevel >= Params::DEBUG_NORM)
      cerr << "     Skipping dir per instructions in local param file" << endl;
    
    return true;
  }
  
//  // If we are to check the hostname, do so, and
//  // return if the hostname does not match.
//
//  if (_params->HostnameMustMatch)
//  {
//    if (!_hostNamesMatch(_params->Hostname))
//      return 1;
//  }

  // Try to open the directory
  
  DIR *dirp;
  if ((dirp = opendir(data_dir.c_str())) == NULL)
  {
    cerr << "ERROR - " << _progName << ":ArchiveProcessor::process" << endl;
    cerr << "Cannot open directory '" << data_dir << "'" << endl;
    perror(data_dir.c_str());

    if (found_params)
      delete local_params;
    
    return -1;
  }

  // Loop thru directory looking for the data file names
  
  struct dirent *dp;

  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
  {
    // Don't process the "." and ".." files.

    if (STRequal_exact(dp->d_name, ".") ||
	STRequal_exact(dp->d_name, ".."))
      continue;

    // Don't process files that begin with a dot or an underscore.
    // These are generally control files that need to be kept.  

    if (dp->d_name[0]=='.' || dp->d_name[0]== '_')
    {
      if (_debugLevel >= Params::DEBUG_VERBOSE)
	cerr << dp->d_name << " - Archiver will not process this file" <<
	  endl;

      continue;
    }

    // Leave CVS directories alone.

    if (STRequal_exact(dp->d_name, "CVS"))
      continue;

    // Exclude any files named 'latest_data_info'

    if (STRequal_exact(dp->d_name, "latest_data_info"))
      continue;

    // Put together the file name.

    string current_file_path = data_dir;
    _addDelim(current_file_path);
    current_file_path += dp->d_name;

    if (_debugLevel >= Params::DEBUG_VERBOSE)
      cerr << "      " << current_file_path << ":" << endl;

    // Look at the file

    DataFileNames current_file_path_info;

    current_file_path_info.GetFileFacts(current_file_path,
				       local_params->compressed_ext, "");

    //
    // The following is a VERY verbose debug.
    // fprintf(stdout,"\n\n%s\n",current_file_path);
    // current_file_path_info.PrintFileFacts();
    //

    if (!current_file_path_info.Exists)
    {
      if (_debugLevel >= Params::DEBUG_NORM)
	cerr << current_file_path << " - File doesn't exist" << endl;

      continue;
    }

    // Process the file based on what type of file it is

    if (current_file_path_info.Directory)
    {
      if (_debugLevel >= Params::DEBUG_VERBOSE)
	cerr << current_file_path << " - is a directory" << endl;

      string new_archive_dir = archive_dir;
      _addDelim(new_archive_dir);
      new_archive_dir += dp->d_name;
    
      processDir(current_file_path, new_archive_dir,
		 local_params, start_time, end_time);
    } // End of if it's a directory
    else if (current_file_path_info.Regular)
    {
      if (_debugLevel >= Params::DEBUG_VERBOSE)
	cerr << current_file_path << " - is a regular file" << endl;

      _processFile(current_file_path, current_file_path_info,
		   start_time, end_time,
		   archive_dir, dp->d_name,
		   local_params->compress, local_params->compress_cmd);
      
    } // End of if it's a regular file.
    else
    {
    if (_debugLevel >= Params::DEBUG_VERBOSE)
      cerr << current_file_path << " - is not a regular file" << endl;
    }
    
  } // End of loop through directory.
  
  closedir(dirp);

  if (found_params)
    delete local_params;
  
  return true;
}


/*************************************************************************
 * Private methods
 *************************************************************************/

Params *ArchiveProcessor::_loadLocalParams(const string data_dir,
				    const Params &global_params)
{
  string params_path = data_dir + "_Archiver";
  struct stat file_stat;

  if (stat(params_path.c_str(), &file_stat) == 0 && S_ISREG(file_stat.st_mode))
  {
    if (_debugLevel >= Params::DEBUG_NORM)
      cerr << "Parameter file " << params_path << " found" << endl;

    Params *local_params = new Params(global_params);

    local_params->load((char *)params_path.c_str(), NULL, TRUE,
		       (_debugLevel >= Params::DEBUG_VERBOSE));

    return local_params;
  }

  return 0;
}


// bool ArchiveProcessor::_hostNamesMatch(const string expected_hostname) const
// {
//   char my_host_name[MAX_HOST_LEN];
//   gethostname(my_host_name, MAX_HOST_LEN);
// 
//   if (strcmp(my_host_name, expected_hostname.c_str()) == 0)
//     return true;
//   
//   if (_params->debug >= Params::DEBUG_NORM)  
//     cerr << "Hostname is " << my_host_name << " not " <<
//       expected_hostname << endl;
// 
//   return false;
// }
// 
// 
// bool ArchiveProcessor::_dirMatchesDateFormat(const string dir_path)
// {
//   string last_bit;
// 
//   //
//   // First, try YYYYMMDD
//   //
// 
//   if (dir_path.length() >= strlen("YYYYMMDD"))
//   {
//     last_bit = string(dir_path,
// 		      dir_path.length() - strlen("YYYYMMDD"),
// 		      strlen("YYYYMMDD"));
// 	  
//     int year, month, day;
//     
//     if ((3 == sscanf(last_bit.c_str(),"%4d%2d%2d",
// 		     &year, &month, &day)) && 
// 	(year > 1970) && (year < 2050) &&
// 	(month > 0) && (month < 13) &&
// 	(day > 0) && (day < 32))
//       return true;
//   }
// 
//   //
//   // Then, try g_hhmmss
//   //
//   if (dir_path.length() >= strlen("g_hhmmss"))
//   {
//     last_bit = string(dir_path,
// 		      dir_path.length() - strlen("g_hhmmss"),
// 		      strlen("g_hhmmss"));
// 	  
//     int hour, min, sec;
//     
//     if ((3 == sscanf(last_bit.c_str(), "g_%2d%2d%2d",
// 		     &hour, &min, &sec)) && 
// 	(hour > -1) && (hour < 24) &&
// 	(min > -1) && (min < 61) &&
// 	(sec > -1) && (sec < 61))
//       return true;
//   }
// 
//   return false;
// }

void ArchiveProcessor::_processFile(const string data_file_path,
				    const DataFileNames &data_file_info,
				    const time_t start_time,
				    const time_t end_time,
				    const string archive_dir,
				    const string filename,
				    const bool compress_file,
				    const char *compress_cmd)
{
  // If we don't have a valid date, we can't process the file

  if (!data_file_info.NameDateValid)
  {
    if (_debugLevel >= Params::DEBUG_VERBOSE)
      cerr << "     File doesn't have valid name/date" << endl;
    
    return;
  }
  
  // Don't process files that don't fall within the specified time range

  if (data_file_info.DateOnly)
  {
    date_time_t current_date = data_file_info.NameDate;
    
    // Set the data time to the end of the day and then if the
    // data time is before the archive start time we know that we
    // shouldn't process the file.

    current_date.hour = 23;
    current_date.min = 59;
    current_date.sec = 59;
    
    uconvert_to_utime(&current_date);
    
    if (current_date.unix_time < start_time)
    {
      if (_debugLevel >= Params::DEBUG_VERBOSE)
	cerr << "     File only has date and is before start time" << endl;
      
      return;
    }
    
    // Now set the date time to the beginning of the day and then if the
    // data time is after the archive end time we know that we
    // shouldn't process the file

    current_date.hour = 0;
    current_date.min = 0;
    current_date.sec = 0;
    
    uconvert_to_utime(&current_date);
    
    if (current_date.unix_time > end_time)
    {
      if (_debugLevel >= Params::DEBUG_VERBOSE)
	cerr << "     File only has date and is past end time" << endl;
      
      return;
    }
    
  }
  else
  {
    if (data_file_info.NameDate.unix_time < start_time ||
	data_file_info.NameDate.unix_time > end_time)
    {
      if (_debugLevel >= Params::DEBUG_VERBOSE)
	cerr << "     File is not within start/end times" << endl;
      
      return;
    }
    
  }
  
  // Create the directory in the archive area

  ta_makedir_recurse(archive_dir.c_str());
  
  // Copy the file to the archive directory

  string archive_filename = archive_dir;
  _addDelim(archive_filename);
  archive_filename += filename;
  
  string copy_cmd = "cp " + data_file_path + " " + archive_filename;
  
  if (_debugLevel >= Params::DEBUG_VERBOSE)
    cerr << "     copy cmd: " << copy_cmd << endl;
  
  if (system(copy_cmd.c_str()))
  {
    cerr << "ERROR: Copy command failed" << endl;
    cerr << "Command: " << copy_cmd << endl;
    
    return;
  }
	
  // See if we need to compress the file

  if (compress_file && 
      !data_file_info.Compressed)
    _compressFile(archive_filename, compress_cmd);

}


void ArchiveProcessor::_compressFile(const string &file_path,
				     const char *compress_cmd) const
{
  // Put together the compress command.

  string command;

  for (size_t i = 0; i < strlen(compress_cmd); ++i)
  {
    if (compress_cmd[i] == '^')
      command += file_path;
    else
      command += compress_cmd[i];
  }

  if (_debugLevel >= Params::DEBUG_VERBOSE) 
    cerr << "Compress command is: " << command << endl;

  if (system(command.c_str()))
  {
    cerr << "The following compress command failed:" << endl;
    cerr << command << endl;
  }
}



////////////////////////
void ArchiveProcessor::_addDelim(string &dir_path) 
{
  // If the directory path is not as long as the delimiter, then it
  // cannot contain the delimiter so we must add it.  This is a special
  // case needed so that we don't try to get a substring beginning outside
  // of the current string.

  if (dir_path.length() < strlen(PATH_DELIM))
  {
    dir_path += PATH_DELIM;
    return;
  }
  
  // Make sure the _dirPath ends in PATH_DELIM

  string lastchars = string(dir_path, dir_path.length() - strlen(PATH_DELIM),
			    strlen(PATH_DELIM));
  if (strcmp(lastchars.c_str(), PATH_DELIM) != 0)
    dir_path += PATH_DELIM;
}
