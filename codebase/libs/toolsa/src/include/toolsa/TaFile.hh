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
// TaFile.hh
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

#ifndef TaFile_hh
#define TaFile_hh


#include <toolsa/file_io.h>
#include <sys/stat.h>
#include <string>
using namespace std;

///////////////////////////////////////////////////////////////////
// NOTES ON USE:
//

class TaFile {

public:

  // default constructor
  
  TaFile();
  
  // destructor

  ~TaFile();

  // Open
  // Returns file pointer on success, NULL on failure.
  // File pointer mostly not needed when using this class.

  FILE *fopen(const string &path, const string &mode);
  FILE *fopen(const char *path, const char *mode);

  // Open and uncompress on the fly
  // Returns file pointer on success, NULL on failure.
  // File pointer mostly not needed when using this class.

  FILE *fopenUncompress(const string &path, const string &mode);
  FILE *fopenUncompress(const char *path, const char *mode);

  // Open and lock file.
  // Returns file pointer on success, NULL on failure.
  // File pointer mostly not needed when using this class.
  // Failure occurs either because the file cannot be
  // opened, or because the file is already write-locked.
  
  FILE *fopenLock(const string &path);
  FILE *fopenLock(const char *path);

  // fclose - standard, unlocks file if locked

  void fclose();
  
  // remove()
  // Close and remove file, unlocking as necessary.
  
  void remove();

  // fread - standard behavior, retries on EINTR
  
  size_t fread(void *ptr, size_t size, size_t nitems);

  // fwrite - standard behavior, retries on EINTR

  size_t fwrite(const void *ptr, size_t size, size_t nitems);

  // fseek - standard behavior
  
  int fseek(long offset, int whence);

  // ftell - standard behavior
  
  long ftell();

  // readSelect
  //
  // waits for read access on a file ptr
  //
  // returns 1 on success, -1 on timeout, -2 on failure
  //
  // Blocks if wait_msecs == -1
  
  int readSelect(long wait_msecs);

  // Sets up read or write lock on entire file.
  //
  // File must already be open.
  //
  // Blocks until lock is obtained.
  //
  // Returns 0 on success, -1 on failure.

  int lock();

  // Sets up read or write lock on entire file.
  // Loops waiting for lock. Reports to procmap while
  // waiting.
  //
  // File must already be open.
  //
  // Blocks until lock is obtained.
  //
  // Returns 0 on success, -1 on failure.

  int lockProcmap();

  // Clear lock on entire file
  //
  // File must already be open.
  //
  // Returns 0 on success, -1 on failure.

  int unlock();

  // fstat
  //
  // fstat opened file
  // Retrieve stat struct with getStat()
  //
  // Returns same as fstat,
  // 0 on success, -1 on failure.
  
  int fstat();

  // stat
  //
  // stat a file
  // Retrieve stat struct with getStat()
  //
  // Returns same as stat,
  // 0 on success, -1 on failure.

  int stat(const string &path);
  int stat(const char *path);

  // File exists?
  // Return true if exists, false otherwise
  
  bool exists(const string &path);

  // statUncompress
  //
  // Stat a file, uncompressing as needed.
  // Retrieve stat struct with getStat()
  //
  // Returns same as stat,
  // 0 on success, -1 on failure.

  int statUncompress(const string &path);
  int statUncompress(const char *path);

  // set/clear removeOnDestruct - this forces removal of the file by
  // the destructor - useful for tmp files
  void setRemoveOnDestruct() { _removeOnDestruct = true; }
  void clearRemoveOnDestruct() { _removeOnDestruct = false; }

  // access to data members

  string &getPath() { return _path; }
  string &getOpenMode() { return _openMode; }
  FILE *getFILE() { return _fp; }
  bool isLocked() { return _isLocked; }
  struct stat &getStat() { return _stat; }

  // uncompress file if required
  // handles .Z, .gz, .bz2
  // returns 0 on success, -1 on failure

  int uncompress(const string &path);

  // methods valid after call to uncompress()

  bool uncompressPerformed() { return _uncompressPerformed; }
  string &getUncompressedPath() { return _uncompressedPath; }

protected:

  string _path;           // the file path
  string _openMode;       // open for read, write, etc.
  FILE *_fp;              // NULL when file is closed
  struct stat _stat;      // typedef wraps 'struct stat'
  bool _isLocked;         // is this a lock file?
  bool _removeOnDestruct; // option to automatically remove the file in
                          // the destructor - useful for tmp files

  bool _uncompressPerformed; /* true if file was previously compressed
                              * and uncompression was performed */
  string _uncompressedPath; // path after uncompression performed

private:

  int _uncompress(const string &path);
  int _gunzip(const string &path);
  int _bunzip2(const string &path);

};
  
#endif 
