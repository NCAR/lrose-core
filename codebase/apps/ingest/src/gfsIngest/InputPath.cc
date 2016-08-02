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
// InputPath.cc : Input MM5 file handling
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
//////////////////////////////////////////////////////////
#include <string>
#include <sys/stat.h>
#include <dirent.h>
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>

#include "InputPath.hh"
#include "Params.hh"
using namespace std;

//
// Constants
//
const int InputPath::MAX_BUF_SIZE = 11;

InputPath::InputPath( Params *params, int nFiles, char **paths )
{
  _paramsPtr = params;
  _firstPass = true;
  _archiveMode = true;
  _maxValidAge = 0;
  _inputDir = "";
  _lastModTime = 0;
  _lastGenTime = 0;
  _debug = _paramsPtr->debug;

  if( _paramsPtr->input_suffix[0] == '.' ) {
    _inputFileSuffix = _paramsPtr->input_suffix;
  } else {
    _inputFileSuffix  = ".";
    _inputFileSuffix += _paramsPtr->input_suffix;
  }

  for( int i = 0; i < nFiles; i++ ) {
    _filePaths.insert( paths[i] );
  }
  _pathPtr = _filePaths.begin();
}

InputPath::InputPath( Params *params, MdvInput_heartbeat_t hbFunc )
{
  _paramsPtr = params;
  _firstPass = true;
  _archiveMode = false;
  _maxValidAge = _paramsPtr->max_input_data_age;
  _inputDir = _paramsPtr->input_dir;
  _heartbeatFunc = hbFunc;
  _lastModTime = 0;
  _ldata.setDir( _inputDir );
  _debug = _paramsPtr->debug;

  if( _paramsPtr->input_suffix[0] == '.' ) {
    _inputFileSuffix = _paramsPtr->input_suffix;
  } else {
    _inputFileSuffix  = ".";
    _inputFileSuffix += _paramsPtr->input_suffix;
  }

}

InputPath::~InputPath()
{
}

const string&
InputPath::next()
{  
  if( _archiveMode ) {
     
    //
    // Get next file path in the list.  If we are finished
    // with the list, return the null string.
    //
    if( _pathPtr != _filePaths.end() ) {
      _inputPath = *_pathPtr;
      _pathPtr++;
    }
    else {
      _inputPath = "";
    }
  }
  else {

    //
    // Handle the special case of the first time we are looking
    // for a file
    //
    if( _firstPass ) {

      //
      // Wait until we get ldata info 
      //
      _ldata.readBlocking( _maxValidAge, 1000, _heartbeatFunc );

      //
      // Set the last file to the file listed in the
      // user info 1 section of the ldata info file
      //
      string *lastFile = new string( _ldata.getUserInfo1() );
	 
      //
      // Create the generate path and set the last 
      // generate time
      //
      _createGenPath();
      _lastGenTime = _ldata.getLatestTime();

      //
      // Get a list of files and the last modification time
      // for that list
      //
      _lastModTime = _fileList( lastFile );

      // 
      // Set the path pointer
      //
      _pathPtr = _filePaths.begin();
      if( _pathPtr == _filePaths.end() ) {
	cerr << "ERROR: No new files in current directory" << endl << flush;
	_inputPath = "";
	return( _inputPath );
      }
	 
      _firstPass   = false;
      delete lastFile;

    }

    // 
    // If this isn't the first time, but we are at the end
    // of the current file list, get a new list
    //
    else if( _pathPtr == _filePaths.end() ) {

      //
      // Clean out the set of file paths
      //
      _filePaths.erase( _filePaths.begin(), _filePaths.end() );

      //
      // Tell the user what we're doing
      //
      if(_debug) {
	cout << "Waiting for next file" << endl << flush;
      }

      //
      // Wait for ldata info
      //
      _ldata.readBlocking( _maxValidAge, 1000, _heartbeatFunc );

      //
      // Check the sequence of the files
      //
      if( _ldata.getLatestTime() < _lastGenTime ) {
	cerr << "ERROR: File arrived out of order" << endl << flush;
	_inputPath = "";
	return( _inputPath );
      }

      //
      // Put any new files in the current directory into 
      // the list and get the lastest modification time
      //
      _lastModTime = _fileList();
	    
      //
      // If the ldata info file suggests that we have a 
      // new generate time, set the new path, new generate
      // time and append any new files in that directory
      // to the file list
      //
      if( _ldata.getLatestTime() > _lastGenTime ) {
	_createGenPath();
	_lastGenTime = _ldata.getLatestTime();
	_lastModTime = _fileList();
      }
	 
      //
      // Set the path pointer to the beginning of the list
      //
      _pathPtr = _filePaths.begin();
      if( _pathPtr == _filePaths.end() ) {
	cerr << "ERROR: No new files in current directory" << endl << flush;
	_inputPath = "";
	return( _inputPath );
      }
    }

    //
    // Return the next file in the list
    //
    _inputPath = *_pathPtr;
    _pathPtr++;
  }
   
  return( _inputPath );
}

time_t
InputPath::_fileList( string* lastFile ) 
{  
  struct stat fileStat;
  struct dirent *dp;
  DIR           *dirp;
  string         currentPath;
  string         dirPath;
  int            age;
  time_t         maxModTime = 0;
   
  if( (dirp = opendir(_genPath.c_str())) == NULL ) {
    cerr << "ERROR: Error opening directory" << _genPath << endl << flush;
    return( 0 );
  }

  for( dp = readdir(dirp); dp != NULL; dp = readdir(dirp) ) {

    //if( strstr(dp->d_name, "f_") != dp->d_name ) 
    //  continue;

    //if( strstr(dp->d_name, _inputFileSuffix.c_str() ) == NULL )
    //  continue;
      
    currentPath = _genPath + PATH_DELIM + dp->d_name;

    if( ta_stat(currentPath.c_str(), &fileStat) ) {
      cerr << "WARNING: Cannot stat file" <<  currentPath << endl << flush;
      continue;
    }
	
    if( S_ISDIR(fileStat.st_mode) || !S_ISREG(fileStat.st_mode) ) 
      continue;

    age = time(NULL) - fileStat.st_mtime;
    if( age > _maxValidAge ) {
      continue;
    }

    if( lastFile != NULL && dp->d_name < (*lastFile) )
      continue;
        
    if( fileStat.st_mtime > _lastModTime ) {
      _filePaths.insert( currentPath );
      if( fileStat.st_mtime > maxModTime )
	maxModTime = fileStat.st_mtime;
    } 
  }

  free( dirp );
  return( maxModTime );
}

void
InputPath::_createGenPath() 
{
   char        buffer[MAX_BUF_SIZE];
   date_time_t latestTime = _ldata.getLatestTimeStruct();
	    
   _genPath  = _inputDir;
   _genPath += PATH_DELIM;
	    
   sprintf( buffer, "%.4d%.2d%.2d", 
            latestTime.year, latestTime.month, latestTime.day );
   _genPath += buffer;
   _genPath += PATH_DELIM;

   //sprintf( buffer, "%.2d%.2d%.2d",
   //         latestTime.hour, latestTime.min, latestTime.sec );
   //_genPath += buffer; 
}

