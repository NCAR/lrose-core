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
// ScoutOut.cc: Does the work of the scout program. Recursive.
//
// Niles Oien from
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 1998
//
/////////////////////////////////////////////////////////////

#include <cstdio>
#include <dirent.h>
#include <toolsa/umisc.h>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>

#include <didss/RapDataDir.hh>
#include <didss/DataFileNames.hh>
#include <toolsa/port.h> 
#include <toolsa/pmu.h>

#include "ScoutOut.hh"
using namespace std;

// Constructor

ScoutOut::ScoutOut(const Params &params,
		   const string &dir_path,
		   int level) :
  _params(params),
  _dirPath(dir_path),
  _level(level)

{

  // get DataMapper info if available
  if (_dmap.reqAllInfo("localhost") == 0) {
    _dmapAvail = true;
    if (_params.Debug) {
      cerr << "SUCCESS - DataMapper info is available" << endl;
    }
  } else {
    _dmapAvail = false;
    if (_params.Debug) {
      cerr << "WARNING - cannot contact the DataMapper" << endl;
      cerr << "  Will guess at data types" << endl;
    }
  }
}

///////////////////////////////////////////
//
// recursive search engine
//

void ScoutOut::Recurse()

{

  PMU_auto_register( "Parsing upper level directory" );
  
  // initialize
  
  _start.unix_time = 0; 
  _end.unix_time = 0;
  _timeAssigned = false;
  _nBytes = 0.0;
  _nFiles = 0.0;
  _logFile = NULL;

  if (!(_params.Recurse)){
    return;
  }
  
  if ((_params.Debug) || (_params.ReportDirEntry)){
    fprintf(stderr, "---> Entering dir %s\n", _dirPath.c_str());
  }

  // check for '_scout' file to override params
  
  string paramsPath = _dirPath + PATH_DELIM + "_scout";
  struct stat fileStat;
  if (stat(paramsPath.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {

    if (_params.Debug) {
      fprintf(stderr, "........ _scout file found, overriding params\n");
    }
    _params.load((char *) paramsPath.c_str(),
	   NULL, TRUE, FALSE);
    if (_params.Debug >= Params::DEBUG_VERBOSE) {
      _params.print(stderr, PRINT_SHORT);
    }

  } else {

    // check for '_Scout' file to override params if _scout not there.
  
    paramsPath = _dirPath + PATH_DELIM + "_Scout";
    if (stat(paramsPath.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)){
      if (_params.Debug) {
	fprintf(stderr, "........ _Scout file found, overriding params\n");
      }
      _params.load((char *) paramsPath.c_str(),
	     NULL, TRUE, FALSE);
      if (_params.Debug >= Params::DEBUG_VERBOSE) {
	_params.print(stderr, PRINT_SHORT);
      }
    }
  }
  
  if (!_params.Recurse) {
    return;
  }
  
  // Set up logging report file, if any

  if (_params.Debug >= Params::DEBUG_VERBOSE) {
    _logFile = stderr;
  } else if (_params.Report) {
    PMU_auto_register( "Setting up report file" );
    string ReportFileName = _dirPath;
    ReportFileName += PATH_DELIM;
    ReportFileName += "_Scout_Report";
    _logFile=fopen(ReportFileName.c_str(),"wa");
  }

  if (_logFile){
    
    // Compute relative directory. Has RAP_DATA_DIR parsed out.
    string relDir;
    RapDataDir.stripPath(_dirPath, relDir);

    date_time_t now;
    ulocaltime(&now);
    fprintf(_logFile,"\n\nScout report at %s\n\n", utimestr(&now));

    fprintf(_logFile,"In directory %s\n",_dirPath.c_str());
    fprintf(_logFile,"Relative directory name : %s\n", relDir.c_str());
    fprintf(_logFile,"Extension indicating compressed file : %s\n",
	    _params.CompressedExt);

  }

  _nFilesCounted = 0;
  if (_params.BetweenFileDelay == 0) {
    _nFilesPerSecDelay = 1000000000;
  } else {
    _nFilesPerSecDelay = 1000000 / _params.BetweenFileDelay;
  }

  // Read directory.
  
  DIR *dirp;
  
  if ((dirp = opendir(_dirPath.c_str())) == NULL) {
    fprintf(stderr,"Failed to open directory %s\n", _dirPath.c_str());
    perror(_dirPath.c_str());
    if (_logFile) {
      fprintf(_logFile,"Failed to open directory %s\n", _dirPath.c_str());
      if (!_params.Debug) {
        fclose(_logFile);
      }
    }
    return;
  }
  
  struct dirent *dp;
  DataFileNames Q;
  string FileName;
  
  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)){
    
    PMU_auto_register( "Looping through the directory" );

    // Skip the . and .. dirs, also anything that starts
    // with an underscore.

    if (
	(!strcmp(dp->d_name, ".")) || 
	(!strcmp(dp->d_name, "..")) ||
	(!strncmp(dp->d_name, "_", 1))
	){
      continue;
    }

    // skip CVS subdirs

    if (!strcmp(dp->d_name, "CVS")) continue;

    // compute subdir or file name

    FileName = _dirPath + PATH_DELIM + dp->d_name;
    if (_logFile) fprintf(_logFile,"%s : ",FileName.c_str());
    
    string dataSetName;
    RapDataDir.stripPath(_dirPath, dataSetName);

    // Try to match a pattern
    bool patternFound = false;
    for( int i = 0; i < _params.Patterns_n; i++ )
    {
      if( strcmp( dataSetName.c_str(), _params._Patterns[i].dataSetName ) == 0 )
      {
        // get specs on the file using a file name pattern
        Q.GetFileFacts( FileName.c_str(), _params.CompressedExt,
                        "DontCare", _params._Patterns[i].filePattern );
        patternFound = true;
      }
    }
  
    // if no pattern was found, then get specs on the file without a pattern
    if( ! patternFound )
    {
      Q.GetFileFacts(FileName.c_str(), _params.CompressedExt, "DontCare");
    }
    if (!Q.Exists){ // should not get here
      if (_logFile) fprintf(_logFile,"Does not exist.\n");
      continue;
    }
 
    if (_params.Debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr,"\t--> %s\n",FileName.c_str());
    }
    
    PMU_auto_register( "Processing..." );

    if (Q.Directory) {
      HandleDir(FileName, Q);
    } else {
      HandleFile(FileName, Q);
    }
    
    // stop CPU hogging.
    
    _nFilesCounted++;
    if (_nFilesCounted > _nFilesPerSecDelay) {
      umsleep(1000);
      _nFilesCounted = 0;
    }

  } // End of loop through directory.

  closedir(dirp);

  // Sleep - don't hog the system.

  for (int k=0; k < _params.BetweenDirDelay; k++){
    PMU_auto_register("Sleeping between directories");
    umsleep(1000);
  }
  PMU_auto_register("Processing...");

  if ((_logFile) && (_start.unix_time < _end.unix_time) && (_nBytes > 0.5)){

    fprintf(_logFile,"\nScout Summary : \n");
    fprintf(_logFile,"%g bytes in %g files.\n\n", _nBytes, _nFiles);

    fprintf(_logFile,"TIME RANGE OF DATA :\n");
    
    fprintf(_logFile," start %s\t", utimestr(&_start));
    fprintf(_logFile," end %s\n", utimestr(&_end));

  }
  
  if (!_params.Debug && _logFile) {
    fclose(_logFile);
  }

  // Now send this off to the data mapper.

  if ((_start.unix_time <= _end.unix_time) && (_nBytes > 0.5)) { 

    // Compute relative directory. Has RAP_DATA_DIR parsed out.
    string relDir;
    RapDataDir.stripPath(_dirPath, relDir);

    // get data type from data mapper if available

    string dataType = _params.DataType;
    if (_dmapAvail) {
      int nInfo = _dmap.getNInfo();
      for (int ii = 0; ii < nInfo; ii++) {
        const DMAP_info_t &info = _dmap.getInfo(ii);
        if (relDir == info.dir) {
          dataType = info.datatype;
          break;
        }
      } // ii
    }
    
    if (_params.Debug) {
      cerr << "Reg with data mapper" << endl;
      cerr << "  Start time: " << utimestr(&_start) << endl;
      cerr << "  End   time: " << utimestr(&_end) << endl;
      cerr << "  Nfiles: " << _nFiles << endl;
      cerr << "  Nbytes: " << _nBytes << endl;
      cerr << "  Directory relative to $RAP_DATA_DIR : " << relDir << endl;
      cerr << "  DataType: " << dataType << endl;
    }

    PMU_auto_register( "Check in with data mapper" );

    DmapAccess access;
    if (access.regDataSetInfo(_start.unix_time, _end.unix_time,
			      _nFiles, _nBytes, relDir, dataType)) {
      fprintf(stderr,"ERROR - failed to send data to mapper.\n");
      fprintf(stderr,"In directory : %s\n",_dirPath.c_str());
      date_time_t Now;
      ulocaltime(&Now);
      fprintf(stderr,"Time : %s\n", utimestr(&Now));
    }
    
  }

}

////////////////////////////
// handle a directory entry

void ScoutOut::HandleDir(const string &DirName,
			 const DataFileNames &Q)

{
  
  if (_params.SummarizeFromHereDown) {
    
    if (_logFile) fprintf(_logFile,"Summary from here - recursing.\n");
    
    RecurseSubDir(DirName);
      
  } else if (Q.NameDateValid) {

    // The directory name is of the form YYYYMMDD
    
    if (_logFile) fprintf(_logFile,"Dated directory - recursing.\n");

    RecurseDatedSubDir(DirName);
    
  } else {
    
      if (_logFile) fprintf(_logFile,"  .... is a directory.\n");
      ScoutOut r(_params, DirName, _level + 1);
      r.Recurse();
      
  }

}

////////////////////////////
// handle a file entry

void ScoutOut::HandleFile(const string &FileName,
			  const DataFileNames &Q)
  
{
  
  if (_params.SeeAllFiles) {
    
    // if (_logFile) fprintf(_logFile,"Counting all files.\n");
    
    // SeeAllFiles is set, use 'stat' to "see" this file.
    
    struct stat fileStat;
    if (stat(FileName.c_str(), &fileStat)){
      fprintf(stderr, "--> ERROR : FAILED TO STAT %s - skipping\n",
	      FileName.c_str());
      return;
    }
    _nFiles++;
    _nBytes += fileStat.st_size;
    AssignTimes(fileStat);
    
  } else if (Q.NameDateValid) {
    
    if (_logFile) fprintf(_logFile,"Counting file %s.\n", FileName.c_str());
    _nFiles++;
    _nBytes += Q.FileSize;
    AssignTimes(Q);
    
  } else {

    // The file does not follow our naming conventions, so we ignore
    // it unless SeeAllFiles is set
    
  }
  
}


//////////////////////////////////////////
//
// Recurse into a dated directory.

void ScoutOut::RecurseDatedSubDir(const string &dir)
{

  PMU_auto_register( "Processing..." );

  DIR *dirp;
  
  if ((dirp = opendir(dir.c_str())) == NULL) {
    fprintf(stderr,"Failed to open directory %s\n",dir.c_str());
    perror(dir.c_str());
    return;
  }

  // Loop through the directory.

  struct dirent *dp;
  DataFileNames Q;
  
  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)){
    
    if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) continue;
    
    string FileName = dir + PATH_DELIM + dp->d_name;
    PMU_auto_register("Processing...");  

    // Stat the file.

    Q.GetFileFacts(FileName.c_str() , _params.CompressedExt, "DontCare");
    
    // stop CPU hogging.
    
    _nFilesCounted++;
    if (_nFilesCounted > _nFilesPerSecDelay) {
      umsleep(1000);
      _nFilesCounted = 0;
    }
    
    if (!Q.Exists) {
      // Not likely in this case.
      continue;
    }
    
    if (Q.Directory) {

      // Recurse - call ourselves.
      
      RecurseDatedSubDir(FileName);
      
    } else {
      
      // It's not a directory - must be a normal file.

      if (Q.NameDateValid){
	_nFiles++;
	_nBytes += Q.FileSize;
	AssignTimes(Q);
      }

    } // if (Q.Directory)

  } // readdir
  
  if (closedir(dirp)) {
    fprintf(stderr,"Failed to close directory %s\n",dir.c_str());
    perror(dir.c_str());
  }

}

//////////////////////////////////////////
//
// Recurse into a non-dated sub directory.
//

void ScoutOut::RecurseSubDir(const string &dir)
{

  PMU_auto_register( "Processing..." );
  DIR *dirp;

  if ((dirp = opendir(dir.c_str())) == NULL) {
    fprintf(stderr,"Failed to open directory %s\n",dir.c_str());
    perror(dir.c_str());
    return;
  }


  //
  // Loop through the directory.
  //
  struct dirent *dp;
  DataFileNames Q;

  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)){

    if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) continue;

    string FileName = dir + PATH_DELIM + dp->d_name;
    //
    PMU_auto_register("Processing...");  
    //
    // Stat the file.
    //
    Q.GetFileFacts(FileName.c_str() , _params.CompressedExt, "DontCare");

    //
    // stop CPU hogging.
    //

    _nFilesCounted++;
    if (_nFilesCounted > _nFilesPerSecDelay) {
      umsleep(10000);
      _nFilesCounted = 0;
    }
    
    //
    //
    if (!Q.Exists){ // Huh!? Not likely in this case.
      // fprintf(stderr,"Could not stat %s\n",FileName.c_str());
      continue;
    }
    

    if (Q.Directory){ // Recurse - call ourselves.
      
      RecurseSubDir(FileName);
      
    } else {
      
      // It's not a directory - must be a normal file.
      
      HandleFile(FileName, Q);

    } // end of if it's a file or directory.

  } // End of loop through directory.

  if (closedir(dirp)) {
    fprintf(stderr,"Failed to close directory %s\n",dir.c_str());
    perror(dir.c_str());
  }


  return;

}

//////////////////////////////
// assign start and end times

void ScoutOut::AssignTimes(const DataFileNames &Q)

{

  if (!_timeAssigned){ // First time read.
    _start=Q.NameDate;
    _end=Q.NameDate;
    _timeAssigned=true;
  } else { // Record it if it is on the extrema.
    if (_start.unix_time > Q.NameDate.unix_time)
      _start=Q.NameDate;
    if (_end.unix_time < Q.NameDate.unix_time)
      _end=Q.NameDate;
  }

}

void ScoutOut::AssignTimes(const struct stat &fileStat)

{

  date_time_t statTime;
  statTime.unix_time = fileStat.st_ctime;
  uconvert_from_utime(&statTime);
  
  if (!_timeAssigned){ // First time read.
    _start=statTime;
    _end= _start;
    _timeAssigned=true;
  } else { // Record it if it is on the extrema.
    if (_start.unix_time > statTime.unix_time)
      _start=statTime;
    if (_end.unix_time < statTime.unix_time)
      _end=statTime;
  }

}
