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
/*********************************************************************
 * InputDirRecurse: InputDirRecurse object code.  This object monitors
 *                  an input directory for new input files or for updates
 *                  to input files.  It will recurse through subdirectories
 *                  also looking for input files.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2006
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <iostream>
#include <cerrno>
#include <cassert>
#include <ctime>
#include <string.h>

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#include <toolsa/str.h>
#include <toolsa/file_io.h>

#include <toolsa/InputDirRecurse.hh>

using namespace std;

/*
 * Global variables
 */

const int Forever = 1;


/*********************************************************************
 * Constructors
 */

InputDirRecurse::InputDirRecurse(const string &dir_name,
				 const string &file_substring,
				 const string &exclude_substring,
				 bool process_old_files,
                                 bool debug,
                                 bool verbose) :
        InputDir(dir_name, 
                 file_substring,
                 process_old_files,
                 exclude_substring,
                 debug,
                 verbose)
{
}


InputDirRecurse::InputDirRecurse(const char *dir_name,
				 const char *file_substring,
				 const char *exclude_substring,
				 int process_old_files,
                                 bool debug,
                                 bool verbose) :
        InputDir(dir_name, 
                 file_substring,
                 process_old_files,
                 exclude_substring,
                 debug,
                 verbose)
{
}


/*********************************************************************
 * Destructor
 */

InputDirRecurse::~InputDirRecurse(void)
{
}


/*********************************************************************
 * getNextFilename() - Gets the next newly detected input filename.
 *                     When there are no new input files, returns
 *                     NULL.  You are expected to call this in a loop
 *                     until there are no new files, so the directory
 *                     is only read on the first call in the loop and
 *                     the update time is only updated when all of the
 *                     new files have been processed.
 */

char *InputDirRecurse::getNextFilename(int check_dir_flag,
				       int max_input_data_age)
{
  static const string method_name = "InputDirRecurse::getNextFilename()";
  
  // If we have a file in our list, just return it

  if (_fileList.size() > 0)
  {
    char *next_file_name = _fileList[0];
    _fileList.erase(_fileList.begin());
    
    return next_file_name;
  }
  
  // If the list is empty, we have to rebuild it recursively.  First
  // rewind the directory then build the file list.

  if (!_rewindDir(1))
    return 0;

  _generateFileList(_dirName, _dirPtr, max_input_data_age);

  // Make sure we don't process the old files again

  _lastDirUpdateTime = _lastDirRewindTime;
    
  // We just updated the file list, so return the next file if there is one

  if (_fileList.size() > 0)
  {
    char *next_file_name = _fileList[0];
    _fileList.erase(_fileList.begin());
    
    return next_file_name;
  }
  
  return 0;
}


/*********************************************************************
 * _generateFileList() - Recurse through the given directory, adding
 *                       files to the file list as appropriate.
 */

void InputDirRecurse::_generateFileList(const string &dir_name,
					DIR *dir_ptr,
					const int max_input_data_age)
{
  static const string method_name = "InputDirRecurse::_generateFileList()";
  
  while (Forever)
  {
    // Read the next entry from the directory

    struct dirent *dir_entry_ptr;
    if ((dir_entry_ptr = readdir(dir_ptr)) == 0)
      return;

    // Make sure we don't return . or ..

    if (STRequal_exact(dir_entry_ptr->d_name, ".") ||
	STRequal_exact(dir_entry_ptr->d_name, ".."))
      continue;
    
    // Determine the full path for the file

    char *next_file = new char[strlen(dir_name.c_str()) +
			       strlen(dir_entry_ptr->d_name) + 2];
    strcpy(next_file, dir_name.c_str());
    strcat(next_file, "/");
    strcat(next_file, dir_entry_ptr->d_name);
  
    // Get the file information

    struct stat file_stat;
    int stat_return = ta_stat(next_file, &file_stat);

    if (stat_return != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error stating file" << endl;
      perror(dir_entry_ptr->d_name);
      
      delete [] next_file;
      
      continue;
    }
    
    // If the file is a directory, recurse into the directory

    if (S_ISDIR(file_stat.st_mode))
    {
      DIR *subdir_ptr = opendir(next_file);
      if (subdir_ptr == 0)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error opening subdirectory: " << next_file << endl;
	
	delete [] next_file;
	
	continue;
      }

      _generateFileList(next_file, subdir_ptr, max_input_data_age);

      closedir(subdir_ptr);
    
      delete [] next_file;  
      continue;
    }
    
    // Make sure the filename contains the appropriate substring

    if (strstr(dir_entry_ptr->d_name, _fileSubstring.c_str()) == 0)
    {
      delete [] next_file;
      continue;
    }
    
    // Make sure the filename doesn't contains the excluded substring

    if (_excludeSubstring.size() > 0 &&
        strstr(dir_entry_ptr->d_name, _excludeSubstring.c_str()) != 0)
    {
      delete [] next_file;
      continue;
    }
    
    // Make sure we don't return old files
    
    time_t current_time = time((time_t *)0);
    
    if (_lastDirUpdateTime >= file_stat.st_mtime)
    {
      delete [] next_file;
      continue;
    }
    else if (max_input_data_age > 0 &&
	     current_time - file_stat.st_mtime > max_input_data_age)
    {
      delete [] next_file;
      continue;
    }
    else
    {
      _fileList.push_back(next_file);
    }
    
  } /* endwhile - Forever */
  
}
