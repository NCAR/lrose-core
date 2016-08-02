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
// fileFetch.hh
//
// Class that searches for the next file in LDM delivered
// nexrad super res data.
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
/////////////////////////////////////////////////////////////

#ifndef FILE_FETCH_H
#define FILE_FETCH_H

#include <toolsa/umisc.h>
#include <vector>

using namespace std;

class fileFetch {
  
public:

  // Constructor. Makes copies of inputs.
  fileFetch ( char *dirName, int maxFileAgeSecs, bool deleteOldFiles,
	      char *subString, bool debug, int timeoutSecs, int skipSecs,
	      bool checkFilenameTime, int maxTimeDiffSecs);

  // return next filename, if any. Caller to allocate space.
  void getNextFile(char *filename, date_time_t *filenameTime,
		   int *seqNum, bool *isEOV);

  // Destructor.
  ~fileFetch ();


protected:
  
private:

  int _maxFileAge;
  bool _deleteOldFiles;
  char *_inDir;
  vector <string> _fileNames;
  int _mode;
  time_t _lastTime;
  char *_subString;
  bool _debug;
  int _timeoutSecs;
  time_t _timeOfLastSuccess;

  date_time_t _filenameTime;
  int _fileSeqNum;
  int _fileVersion;
  char _fileVolChar;
  int _seqNum;
  int _skipSecs;

  bool _checkFilenameTime;
  int _maxTimeDiffSecs;

  const static int SEARCH_NEW_VOL_MODE = 0;
  const static int SEARCH_NEXT_FILE_MODE = 1;
  

  int _scanDir(char *topDir, int depth);
  void _parseElements(char *filename );

};

#endif





