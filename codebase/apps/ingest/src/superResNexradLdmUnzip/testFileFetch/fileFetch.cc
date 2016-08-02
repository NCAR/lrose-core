/////////////////////////////////////////////////////////////
// fileFetch.cc
//
// Implementation of the class that searches for BZIP2 super res
// nexrad data
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
/////////////////////////////////////////////////////////////

#include <dirent.h>
#include <math.h> // for ceil function
#include <cstdlib>
#include <string>
#include <unistd.h> // For rmdir function to delete empty directories.
#include <sys/stat.h>

#include <toolsa/pmu.h>

#include "fileFetch.hh"

// Constructor. Makes copies of inputs, initializes.
fileFetch::fileFetch ( char *dirName, int maxFileAgeSecs, 
		       bool deleteOldFiles, char *subString, bool debug,
		       int timeoutSecs){

  _inDir = dirName;
  _deleteOldFiles = deleteOldFiles;
  _maxFileAge = maxFileAgeSecs;
  _fileNames.clear();
  _mode = SEARCH_NEW_VOL_MODE;
  _lastTime = 0L;
  _subString = subString;
  _debug = debug;
  _timeoutSecs = timeoutSecs;
  _timeOfLastSuccess = 0L;

  return;
}

// return next filename, if any. Caller to allocate space.
void  fileFetch::getNextFile(char *filename, date_time_t *filenameTime,
			     int *seqNum, bool *isEOV){

  char buf[1024];

  *isEOV = false;

  do {

    PMU_auto_register("scanning...");

    _scanDir( _inDir, 0 );
    
    if (_mode == SEARCH_NEW_VOL_MODE){
      //
      // Go through the files we have and return one that
      // is the start of a volume (name contains the substring "_S_")
      // and has a time greater than the last one we returned.
      //
      for (unsigned k=0; k < _fileNames.size(); k++){
	if (NULL != strstr(_fileNames[k].c_str(), "_S_")){
	  sprintf(buf,"%s", _fileNames[k].c_str());
	  _parseElements( buf );
	  
	  if (_filenameTime.unix_time > _lastTime){

	    if (_debug){
	      fprintf(stderr, "Start of new volume detected.\n");
	    }

	    sprintf(filename,"%s", _fileNames[k].c_str());
	    *seqNum = 1;
	    filenameTime->unix_time = _filenameTime.unix_time;
	    uconvert_from_utime( filenameTime );

	    _lastTime = _filenameTime.unix_time;
	    _mode = SEARCH_NEXT_FILE_MODE;
	    if (_debug) fprintf(stderr,"Going to SEARCH_NEXT_FILE_MODE\n");
	    _seqNum = 1;
	    _timeOfLastSuccess = time(NULL);
	    return;
	  }
	}
      }
    } else {
      //
      // Have a volume, looking for new file in volume.
      //
      for (unsigned k=0; k < _fileNames.size(); k++){
	sprintf(buf,"%s", _fileNames[k].c_str());
	_parseElements( buf );
	
	if (_filenameTime.unix_time == _lastTime){
	  if (_fileSeqNum == _seqNum+1){
	    sprintf(filename,"%s", _fileNames[k].c_str());
	    _seqNum++;
	    *seqNum = _seqNum;
	    filenameTime->unix_time = _filenameTime.unix_time;
	    uconvert_from_utime( filenameTime );

	    if (_fileVolChar == 'E'){

	      if (_debug){
		fprintf(stderr, "End of volume detected.\n");
	      }

	      *isEOV = true;
	      _seqNum = 1;
	      _mode = SEARCH_NEW_VOL_MODE;
	      if (_debug) fprintf(stderr,"Going to SEARCH_NEW_VOL_MODE\n");
	    }
	    _timeOfLastSuccess = time(NULL);
	    return;
	  }
	}
      }
      //
      // If we got here, we were looking for a new file in a volume
      // and did not find it. See if it has just plain been too long to
      // wait around - if so, set the _lastTime so that we igonre
      // this volume and all before it and then go back to
      // SEARCH_NEW_VOL_MODE.
      //
      long howLongWaited = time(NULL) - _timeOfLastSuccess;
      if (howLongWaited > _timeoutSecs){
	_lastTime += 1;
	_mode = SEARCH_NEW_VOL_MODE;
	_seqNum = 1;
	if (_debug) fprintf(stderr,"Timed out, going to SEARCH_NEW_VOL_MODE\n");
      }
    }

    sleep(2);
    
  } while (1);

  return;
}

// Destructor.
fileFetch::~fileFetch (){
  return;
}


int fileFetch::_scanDir(char *topDir, int depth){

  if (depth == 0) _fileNames.clear();
  //
  // Scan the dir
  //
  DIR *dirp = opendir( topDir );
  if (dirp == NULL)
    {
      fprintf(stderr, "Cannot open directory %s\n", topDir);
      perror(topDir);
      return -1;
    }

  struct dirent *dp;
  int numFiles = 0;
  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {

    if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) continue;

    numFiles++;

    char fullFileName[1024];
    sprintf(fullFileName,"%s/%s", topDir, dp->d_name);

    struct stat buf;
    if (stat(fullFileName, &buf)) continue; // Not likely;

    if (S_ISDIR(buf.st_mode)){
      //
      // It's a directory, recurse into it.
      //
      int numFilesInDir = _scanDir( fullFileName, depth+1 );
      if ((numFilesInDir == 0) && (_deleteOldFiles)){
	if (_debug) fprintf(stderr,"Deleting empty dir %s\n", fullFileName);
	if (0==unlink( fullFileName )) numFiles--;
      }

    } else {
      //
      // It's a file, check its age.
      //
      if (strlen(_subString) > 0){
	if (strstr(dp->d_name, _subString) == NULL) continue;
      }

      long age = time(NULL)-buf.st_ctime;
      if (age > _maxFileAge){
	if (_deleteOldFiles){
	  if (_debug) fprintf(stderr, "Deleting old file %s\n", fullFileName);
	  if (0==unlink( fullFileName )) numFiles--;
	}
      } else {
	string fn(fullFileName);
	_fileNames.push_back(fn);
      }
    }
  }

  closedir(dirp);

  if ((_debug) && (depth == 0)){
    fprintf(stderr, "\nAt %s file list is as follows : \n", utimstr(time(NULL)));
    for (unsigned ik=0; ik < _fileNames.size(); ik++){
      fprintf(stderr,"%d : %s\n", ik+1, _fileNames[ik].c_str());
    }
    fprintf(stderr,"\n");
  }
  
  return numFiles;
}
 
 
void fileFetch::_parseElements(char *filename ){
  
  _filenameTime.unix_time = 0L;
  
  if (strlen(filename) < strlen("YYYYMMDD/hhmmss_nn_c_Vdd.BZIP2")) return;
  
  char *p = filename + strlen(filename) - strlen("YYYYMMDD/hhmmss_nn_c_Vdd.BZIP2");

  
  if (9 != sscanf(p,"%4d%2d%2d/%2d%2d%2d_%2d_%c_V%2d.BZIP2",
		  &_filenameTime.year, &_filenameTime.month, &_filenameTime.day, 
		  &_filenameTime.hour, &_filenameTime.min, &_filenameTime.sec, 
		  &_fileSeqNum, &_fileVolChar, &_fileVersion)){
    fprintf(stderr,"Failed to parse data from %s\n", p);
    return;
  }

  uconvert_to_utime( &_filenameTime );
  
  return;
  
}


