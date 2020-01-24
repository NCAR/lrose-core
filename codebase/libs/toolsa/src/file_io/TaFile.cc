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
////////////////////////////////////////////////////////////////////
// TaFile.cc
//
// Toolsa file wrapper class.
//
// This class wraps the basic buffered file io.
// Also provides uncompression on the fly.
//
// Closes file in the destructor, so files do not stay open when
// the object goes out of scope.
//
// Derived from the toolsa file_io functions.
//
// Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307, USA
//
// Sept 1999
//
////////////////////////////////////////////////////////////////////


#include <toolsa/TaFile.hh>
#include <iostream>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
using namespace std;

using std::endl;

//////////////////////
// default constructor
//

TaFile::TaFile()

{ 
  _fp = NULL;
  _isLocked = false;
  _removeOnDestruct = false;
} 


//////////////
// Destructor

TaFile::~TaFile()

{
  TaFile::fclose();
  if (_removeOnDestruct) {
    if (std::remove(_path.c_str())) {
      int errNum = errno;
      cerr << "WARNING - TaFile::~TaFile" << endl;
      cerr << "  Failed to remove file: " << _path << endl;
      cerr << "  " << strerror(errNum) << endl;
    }
  }
}

//////////////////////////////////////
// fopen
//
// returns file pointer on success, NULL on failure

FILE *TaFile::fopen(const string &path, const string &mode)

{

  return fopen(path.c_str(), mode.c_str());

}

FILE *TaFile::fopen(const char *path, const char *mode)

{

  _path = path;
  _openMode = mode;
  _fp = std::fopen(path, mode);
  return (_fp);

}

//////////////////////////////////////
// fopenUncompress
//
// Open and uncompress on the fly
//
// returns file pointer on success, NULL on failure

FILE *TaFile::fopenUncompress(const string &path, const string &mode)

{

  return fopenUncompress(path.c_str(), mode.c_str());

}

FILE *TaFile::fopenUncompress(const char *path, const char *mode)

{

  _path = path;
  _openMode = mode;
  _fp = ta_fopen_uncompress((char *) path, (char *) mode);
  return (_fp);

}

//////////////////////////////////////
// fopenLock
//
// Open and lock file
//
// returns file pointer on success, NULL on failure
// Failure occurs either because the file cannot be
// opened, or because the file is already write-locked.

FILE *TaFile::fopenLock(const string &path)

{

  return fopenLock(path.c_str());

}

FILE *TaFile::fopenLock(const char *path)

{

  _path = path;
  _fp = ta_create_lock_file((char *) path);
  return (_fp);

}

//////////////////////////////////////
// fclose()
//

void TaFile::fclose()

{
  if (_fp) {
    if (_isLocked) {
      ta_unlock_file((char *) _path.c_str(), _fp);
      _isLocked = false;
    }
    std::fclose(_fp);
    _fp = NULL;
  }
}

//////////////////////////////////////////////////
// remove()
//
// Close and remove file, unlocking as necessary.
//

void TaFile::remove()

{
  TaFile::fclose();
  unlink((char *) _path.c_str());
}

/////////
// fread

size_t TaFile::fread(void *ptr, size_t size, size_t nitems)

{
  return (ta_fread(ptr, size, nitems, _fp));
}

/////////
// fwrite

size_t TaFile::fwrite(const void *ptr, size_t size, size_t nitems)
  
{
  return (ta_fwrite(ptr, size, nitems, _fp));
}

/////////
// fseek

int TaFile::fseek(long offset, int whence)
  
{
  return (std::fseek(_fp, offset, whence));
}

/////////
// ftell

long TaFile::ftell()
  
{
  return (std::ftell(_fp));
}

////////////////////////////////////////////////////////
// readSelect
//
// waits for read access on a file ptr
//
// returns 1 on success, -1 on timeout, -2 on failure
//
// Blocks if wait_msecs == -1

int TaFile::readSelect(long wait_msecs)
{
  return (ta_read_select(_fp, wait_msecs));
}


/////////////////////////////////////////////////////////
// 
// Sets up read or write lock on entire file.
//
// File must already be open.
//
// Blocks until lock is obtained.
//
// Returns 0 on success, -1 on failure.
//

int TaFile::lock()

{
  return (ta_lock_file((char *) _path.c_str(), _fp,
		       (char *) _openMode.c_str()));
}

/////////////////////////////////////////////////////////////
// 
// Sets up read or write lock on entire file.
// Loops waiting for lock. Reports to procmap while
// waiting.
//
// File must already be open.
//
// Blocks until lock is obtained.
//
// Returns 0 on success, -1 on failure.
//

int TaFile::lockProcmap()

{
  return (ta_lock_file_procmap((char *) _path.c_str(), _fp,
			       (char *) _openMode.c_str()));
}

/////////////////////////////////////////////////////////
// 
// Clear lock on entire file
//
// File must already be open.
//
// Returns 0 on success, -1 on failure.
//

int TaFile::unlock()

{
  return (ta_unlock_file((char *) _path.c_str(), _fp));
}

///////////////////////////////////////
// fstat
//
// fstat opened file
// Retrieve stat struct with getStat()
//
// Returns same as fstat,
// 0 on success, -1 on failure.

int TaFile::fstat()

{
  return (ta_fstat(fileno(_fp), &_stat));
}

///////////////////////////////////////
// stat
//
// stat a file
// Retrieve stat struct with getStat()
//
// Returns same as stat,
// 0 on success, -1 on failure.


int TaFile::stat(const string &path)

{
  _path = path;
  return (ta_stat(path.c_str(), &_stat));
}

int TaFile::stat(const char *path)

{
  _path = path;
  return (ta_stat(path, &_stat));
}

///////////////////////////////////////
// File exists?
// Return true if exists, false otherwise


bool TaFile::exists(const string &path)

{
  _path = path;
  return (TaFile::stat(path) == 0);
}

///////////////////////////////////////
// statUncompress
//
// Stat a file, uncompressing as needed.
// Retrieve stat struct with getStat()
//
// Returns same as stat,
// 0 on success, -1 on failure.

int TaFile::statUncompress(const string &path)

{
  return statUncompress(path.c_str());
}

int TaFile::statUncompress(const char *path)

{
  _path = path;
  return (ta_stat_uncompress((char *) path, &_stat));
}


///////////////////////////////////////////////////////////
//
// uncompress()
//
// Uncompresses file if:
//  (a) file is compressed and
//  (b) the uncompressed file doesn't already exist.
//
// handles .Z, .gz, .bz2
//
// Call:
//   uncompressPerformed()
//   getUncompressedPath()
// after use.
//
// Returns 0 on success, -1 if error

int TaFile::uncompress(const string &path)
  
{

  _uncompressPerformed = false;
  _uncompressedPath = path;

  // if file name indicates that it is compressed (*.Z)
  // use _zUncompress()
  
  if (path.size() >= 3) {
    string ext = path.substr(path.size() - 2, 2);
    if (ext == ".Z") {
      _uncompressedPath = path.substr(0, path.size() - 2);
      if (TaFile::exists(_uncompressedPath)) {
        return 0;
      }
      if (_uncompress(path) == 0) {
        _uncompressPerformed = true;
        return 0;
    } else {
        return -1;
      }
    }
  }
      
  // if file name indicates that it is gzipped (*.gz)
  // us gzUncompress()
  
  if (path.size() >= 4) {
    string ext = path.substr(path.size() - 3, 3);
    if (ext == ".gz") {
      _uncompressedPath = path.substr(0, path.size() - 3);
      if (TaFile::exists(_uncompressedPath)) {
        return 0;
      }
      if (_gunzip(path) == 0) {
        _uncompressPerformed = true;
        return 0;
      } else {
        return -1;
      }
    }
  }
      
  // if file name indicates that it is bzipped (*.bz2)
  // us bz2Uncompress()

  if (path.size() >= 5) {
    string ext = path.substr(path.size() - 4, 4);
    if (ext == ".bz2") {
      _uncompressedPath = path.substr(0, path.size() - 4);
      if (TaFile::exists(_uncompressedPath)) {
        return 0;
      }
      if (_bunzip2(path) == 0) {
        _uncompressPerformed = true;
        return 0;
      } else {
        return -1;
      }
    }
  }

  // no compression, check for existence

  struct stat file_stat;
  if (ta_stat(path.c_str(), &file_stat)) {
    return -1;
  }

  // no uncompression needed

  return 0;

}

/////////////////////////////////////////////////////
// _uncompress()
//
// Returns 0 on success, -1 on failure

int TaFile::_uncompress(const string &path)
  
{
  
  if (TaFile::exists(path)) {
    
    // uncompress file

    char call_str[4096];
    sprintf(call_str, "uncompress -f %s", path.c_str());

    int iret = system(call_str);
    
    if (iret) {
      fprintf(stderr, "ERROR - could not uncompress file\n");
      fprintf(stderr, "  Return from %s: iret = %d\n",
	      call_str, iret);
      return -1;
    } else {
      return 0;
    }

  }

  // file does not exist
  
  return -1;

}

/////////////////////////////////////////////////////
// _gunzip()
//
// Returns 0 on success, -1 on failure

int TaFile::_gunzip(const string &path)
  
{
  
  if (TaFile::exists(path)) {
    
    // uncompress file
    
    char call_str[4096];
    sprintf(call_str, "gunzip -f %s", path.c_str());
    
    int iret = system (call_str);
    
    if (iret) {
      fprintf(stderr, "ERROR - could not gunzip file\n");
      fprintf(stderr, "  Return from %s: iret = %d\n",
	      call_str, iret);
      return -1;
    } else {
      return 0;
    }

  }

  // file does not exist
  
  return -1;

}

/////////////////////////////////////////////////////
// _bunzip2()
//
// Returns 0 on success, -1 on failure

int TaFile::_bunzip2(const string &path)
  
{
  
  if (TaFile::exists(path)) {
    
    // uncompress file
    
    char call_str[4096];
    sprintf(call_str, "bunzip2 -f %s", path.c_str());

    int iret = system (call_str);
    
    if (iret) {
      fprintf(stderr, "ERROR - could not bunzip2 file");
      fprintf(stderr, "  Return from %s: iret = %d\n",
	      call_str, iret);
      return -1;
    } else {
      return 0;
    }

  }

  // file does not exist
  
  return -1;

}

