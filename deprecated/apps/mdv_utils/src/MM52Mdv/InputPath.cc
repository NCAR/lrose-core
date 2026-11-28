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
//
// See "InputPath.h" for details.
//
///////////////////////////////////////////////////////////

#include "InputPath.hh"
#include <toolsa/umisc.h>
#include <toolsa/str.h>
using namespace std;

/////////////////////////////
// Constructor - Archive mode
//
// Pass in a list of file paths.
//

InputPath::InputPath (char *prog_name,
		      int debug,
		      int n_files,
		      char **file_paths)

{

  // initialize

  _progName = STRdup(prog_name);
  _debug = (debug? 1 : 0);
  
  _fileNum = 0;
  _nFiles = n_files;
  _archiveMode = TRUE;

  // set up membuf for file paths array

  _mbufPaths = MEMbufCreate();
  for (int i = 0; i < _nFiles; i++) {
    char *path = (char *) umalloc(strlen(file_paths[i]) + 1);
    strcpy(path, file_paths[i]);
    MEMbufAdd(_mbufPaths, &path, sizeof(char *));
  }
  _filePaths = (char **) MEMbufPtr(_mbufPaths);

  // sort the paths

  qsort(_filePaths, _nFiles, sizeof(char *), _comparePaths);

}

//////////////////////////////
// Constructor - realtime mode
//
// Pass in (a) the input directory to be watched.
//         (b) the max valid age for a realtime file (secs)
//             the routine will wait for a file with the age
//             less than this.
//         (c) pointer to heartbeat_func. If NULL this is ignored.
//             If non-NULL, this is called once per second while
//             the routine is waiting for new data.

InputPath::InputPath (char *prog_name,
		      int debug,
		      char *input_dir,
		      int max_valid_age,
		      MdvInput_heartbeat_t heartbeat_func)
		      

{

  // initialize

  _progName = STRdup(prog_name);
  _debug = (debug? 1 : 0);
  
  _inputDir = input_dir;
  _maxAge = max_valid_age;
  _heartbeatFunc = heartbeat_func;
  _archiveMode = FALSE;

  // init latest data handle
  
  LDATA_init_handle(&_lastData, _progName, _debug);

}

/////////////
// Destructor

InputPath::~InputPath()

{

  STRfree(_progName);

  if (_archiveMode) {

    // free up paths and membuf
    
    for (int i = 0; i < _nFiles; i++) {
      ufree(_filePaths[i]);
    }
    MEMbufDelete(_mbufPaths);

  } else {

    // free up latest data info handle

    LDATA_free_handle(&_lastData);

  }

}

/////////////////////
// get next file path
//
// returns NULL on failure

char *InputPath::next()

{
  
  if (_archiveMode) {

    // in archive mode, go through the file list

    if (_fileNum < _nFiles) {
      _fileNum++;
      return (_filePaths[_fileNum - 1]);
    } else {
      return (NULL);
    }

  } else {

    // in realtime mode wait for change in latest info
    // sleep 1 second between tries.

    LDATA_info_read_blocking(&_lastData, _inputDir,
			     _maxAge, 1000,
			     _heartbeatFunc);

    // new data
    
    sprintf(_inputPath,
	    "%s%s%.4d%.2d%.2d%s%.2d%.2d%.2d.%s",
	    _inputDir,
	    PATH_DELIM,
	    _lastData.ltime.year, _lastData.ltime.month, _lastData.ltime.day,
	    PATH_DELIM,
	    _lastData.ltime.hour, _lastData.ltime.min, _lastData.ltime.sec,
	    _lastData.info.file_ext);

    return (_inputPath);

  } // if (archiveMode)

  return (NULL);

}

/////////////////////////
// reset to start of list
// 
// Archive mode only.

void InputPath::reset()

{
  if (_archiveMode) {
    _fileNum = 0;
  }
}

///////////////////////////////////////////////
// define function to be used for sorting paths
//

int InputPath::_comparePaths(const void *v1, const void *v2)

{

  char *path1 = *((char **) v1);
  char *path2 = *((char **) v2);

  return (strcmp(path1, path2));

}


