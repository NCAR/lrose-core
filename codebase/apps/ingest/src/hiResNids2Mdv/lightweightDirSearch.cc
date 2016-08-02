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
// lightweightDirSearch.cc
//
// Implementation of class
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
/////////////////////////////////////////////////////////////

#include "lightweightDirSearch.hh"
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>

#include <cstdio>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


// Constructor. Makes a copy of things we need.
lightweightDirSearch::lightweightDirSearch (char *dir, 
					    int maxAgeSecs, 
					    bool debug,
					    int sleepSecs){


  if (debug)
    fprintf(stderr,"Searching %s file age < %d sec sleeping %d sec\n",
	    dir, maxAgeSecs, sleepSecs);

  sprintf(_topDir,"%s", dir);
  _maxAgeSecs = maxAgeSecs;
  _debug = debug;
  _lastTimeReturned = time(NULL) - _maxAgeSecs;
  sprintf(_fileName,"%s", "none");
  _sleepSecs = sleepSecs;

  return;
}


// Destructor. Does nothing, but avoids use of the default destructor.
lightweightDirSearch::~lightweightDirSearch (){
  return;
}

// Main routine - gets next file. Blocks, registers with procmap.
char *lightweightDirSearch::nextFile( ){
  while (1){

    _foundFile = false;
    _bestTimeSoFar = time(NULL);

    _procDir( _topDir );

    if (_foundFile){
      if (_debug) fprintf(stderr,"lightweightDirSearch::nextFile( ) returning %s\n",
			  _fileName);
      return _fileName;
    }

    //
    // Sleep, and try again.
    //
    for(int i=0; i < _sleepSecs; i++){
      sleep(1);
      PMU_auto_register("Searching for files");
    }

  }

}



void lightweightDirSearch::_procDir(char *dirName){


  if (_debug) fprintf(stderr, "Looking under directory %s\n", dirName);

  DIR *dirp = opendir( dirName );

  if (dirp == NULL){
    if (_debug){
      fprintf(stderr,"Directory %s does not exist.... yet.\n", dirName);
    }
    return;
  }


  const int secsPerDay = 86400;
  const int secsPerHalfHour = 1800;

  int maxDirAge=secsPerDay;
  if (time(NULL) % secsPerDay < secsPerHalfHour) maxDirAge *= 2; // If is is the first half hour of the day, go into previous day's directory as well as today's

  struct dirent *dp;
  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)){

    //
    // Don't process anything starting with
    // and underscore or dot.
    //
    if ((dp->d_name[0] == '.') ||
        (dp->d_name[0] == '_')){
      if (_debug) fprintf(stderr,"Ignoring %s in %s\n", dp->d_name, dirName);
      continue;
    }


    char fullName[1024];
    sprintf(fullName, "%s/%s", dirName, dp->d_name);

    struct stat buf;
    if (0==stat(fullName, &buf)){
      umsleep(5); // Just so we don't hammer the file system


      if (S_ISREG( buf.st_mode)){
	// It's a file.
	if (buf.st_ctime == time(NULL)) continue; // Too new. May need time to be written.

	if (buf.st_ctime <= _lastTimeReturned){
	  //  if (_debug) fprintf(stderr, "File %s/%s is too old\n", dirName, dp->d_name);
	  continue; // Too old
	}

	//
	// It's a file that is a candidate for returning. We want
	// to return the oldest one we can.
	//
        if (buf.st_ctime <= _bestTimeSoFar){
	  _bestTimeSoFar=buf.st_ctime;
	  sprintf(_fileName, "%s/%s", dirName, dp->d_name);
	  _foundFile = true;
	}
      }
    }

    if (S_ISDIR( buf.st_mode)){
      // It's a directory - if it seems to follow a YYYYMMDD
      // naming convention, and that name indicates that it's
      // old, skip the dir (we are in realtime mode).
      //
      date_time_t dirNameTime;
      if ((strlen(dp->d_name) == 8) &&
	  (3==sscanf(dp->d_name, "%4d%2d%2d", &dirNameTime.year, &dirNameTime.month, &dirNameTime.day ))){
	date_time_t now;
	now.unix_time = time(NULL);
	uconvert_from_utime( &now );
	
	if ( (fabs(now.year-dirNameTime.year) < 10) &&
	     (dirNameTime.month > 0) && (dirNameTime.month < 12) &&
	     (dirNameTime.day > 0) && (dirNameTime.day < 60)
	     ){
	  dirNameTime.hour=0; dirNameTime.min=0; dirNameTime.sec=0;
	  uconvert_to_utime( &dirNameTime );

	  time_t age = now.unix_time - dirNameTime.unix_time;
	  if (_debug) fprintf(stderr,"Directory %s/%s has age %ld seconds, max age is %d\n", 
			      dirName,  dp->d_name, age, maxDirAge);

	  if (age > maxDirAge){
	    if (_debug) fprintf(stderr, "Skipping directory %s/%s - too old\n", dirName,  dp->d_name);
	    continue;
	  }
	}
      }
      char subDir[1024];
      sprintf(subDir,"%s/%s", dirName,  dp->d_name);
      _procDir( subDir );
    }
  }
  closedir(dirp);

  if (_foundFile){
    _lastTimeReturned = _bestTimeSoFar;
    if (_debug){
      fprintf(stderr,"Best file found is %s with time %s\n", _fileName, utimstr( _bestTimeSoFar ));
    }
  } else {
     if (_debug)  fprintf(stderr,"No suitable file found in %s\n", dirName);
  }

  return;

};







