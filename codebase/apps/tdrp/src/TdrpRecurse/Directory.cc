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
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 1998
//
//////////////////////////////////////////////////////////

#include "Directory.hh"
#include "Params.hh"
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <dirent.h>
#include <sys/stat.h>

///////////////
// Constructor

Directory::Directory (char *prog_name, Params *params, char *dir_path)

{

  _progName = STRdup(prog_name);
  _dirPath = STRdup(dir_path);

  if (params->debug) {
    fprintf(stderr, "\n---> Processing dir '%s'\n", _dirPath);
  }

  // check for '_params' file

  char paramsPath[MAX_PATH_LEN];
  struct stat fileStat;

  sprintf(paramsPath, "%s%s%s", _dirPath, PATH_DELIM, "_params");
  if (stat(paramsPath, &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
    
    if (params->debug) {
      fprintf(stderr, "........ _params file found\n");
    }

    _params = new Params(*params);
    _paramsAreLocal = TRUE;

    _params->load(paramsPath, NULL, TRUE,
		  _params->debug >= Params::DEBUG_VERBOSE);
    
  } else {
    
    _params = params;
    _paramsAreLocal = FALSE;

  }
  
  if (params->debug) {
    fprintf(stderr, "==========>> your_age: %d\n", _params->your_age);
  }

}

/////////////
// Destructor

Directory::~Directory ()

{

  if (_progName) {
    STRfree(_progName);
  }
	
  if (_paramsAreLocal && _params) {
    delete (_params);
  }

  if (_dirPath) {
    STRfree(_dirPath);
  }
  
}
  
////////////////////////
// process()
//
// Process the directory
//
// Returns number of directory entries on success,
// -1 on failure.

int Directory::process()

{

  char path[MAX_PATH_LEN];
  struct stat fileStat;

  // check for '_params' file

  sprintf(path, "%s%s%s", _dirPath, PATH_DELIM, "_params");
    
  if (stat(path, &fileStat) == 0 &&
      S_ISREG(fileStat.st_mode)) {
    fprintf(stderr, ".................... _params file found\n");
  }

  // Try to open the directory
  
  DIR *dirp;
  if ((dirp = opendir(_dirPath)) == NULL) {
    fprintf(stderr,
	    "ERROR - %s:Directory::process\n", _progName);
    fprintf(stderr,
	    "Cannot open directory '%s'\n", _dirPath);
    perror(_dirPath);
    return (-1);
  }

  // Loop thru directory looking for the data file names
  
  struct dirent *dp;
  int nEntries = 0;
  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
    
    // exclude the '.' and '..' entries
    
    if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) {
      continue;
    }

    sprintf(path, "%s%s%s", _dirPath, PATH_DELIM, dp->d_name);
    
    if (_params->debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "      %s:", path);
    }

    // stat the file

    struct stat fileStat;
    
    if (stat(path, &fileStat)) {
      perror(path);
      continue;
    }

    // check for directory

    if (S_ISDIR(fileStat.st_mode)) {

      if (_params->debug >= Params::DEBUG_VERBOSE) {
	fprintf(stderr, " - is directory");
      }
      nEntries++;

      Directory *dir = new Directory(_progName, _params, path);
      int iret = dir->process();
      if (iret < 0) {
	fprintf(stderr, "ERROR processing dir: %s\n", path);
      } else if (iret == 0) {
	if (_params->debug) {
	  fprintf(stderr, "Empty dir: %s\n", path);
	}
      } else {
	if (_params->debug) {
	  fprintf(stderr, "Dir %s has %d entries\n", path, iret);
	}
      }
      delete(dir);

    }

    // check for regular file

    if (S_ISREG(fileStat.st_mode)) {
      if (_params->debug >= Params::DEBUG_VERBOSE) {
	fprintf(stderr, " - is regular file");
      }
      nEntries++;
    }

    if (_params->debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "\n");
    }

  } /* dp */
  
  closedir(dirp);
  
  return (nEntries);

}
