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
// InputPath.cc : Input Wrf file handling
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
//////////////////////////////////////////////////////////
//
// See "InputPath.hh" for details.
//
///////////////////////////////////////////////////////////

#include "InputPath.hh"
#include <toolsa/umisc.h>
#include <string>
using namespace std;

/////////////////////////////
// Constructor - Archive mode
//
// Pass in a list of file paths.
//

InputPath::InputPath (const string &prog_name,
		      const Params &params,
		      int n_files,
		      char **file_paths) :
  _progName(prog_name), _params(params)

{

  // initialize

  _archiveMode = true;
  for (int i = 0; i < n_files; i++) {
    _filePaths.insert(file_paths[i]);
  }
  _currentPath = _filePaths.begin();

  _dsInputPath = NULL;

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

InputPath::InputPath (const string &prog_name,
		      const Params &params,
		      MdvInput_heartbeat_t heartbeat_func) :
  _progName(prog_name), _params(params)

{

  // initialize

  _archiveMode = false;
  _heartbeatFunc = heartbeat_func;

  _dsInputPath = new DsInputPath("Wrf2Mdv",
				 _params.debug >= Params::DEBUG_VERBOSE,
				 _params.input_dir,
				 300,
				 _heartbeatFunc,
				 false,
				 false);

#ifdef JUNK
  // Will periodically search the input dir.
  if (! _params.use_ldata){

	// Don't look too deep, due to circular links in the FDDA data tree.
    _dsInputPath->setMaxRecursionDepth(_params.Dir_search_depth);

	// Make sure Wrf is done with the file.
    _dsInputPath->setFileQuiescence(_params.File_quiescence_secs);

    // This app checks at the
    // lowest rate allowed by
    // DsInputPath
    _dsInputPath->setDirScanSleep(_params.Dir_scan_interval_secs);

	// Set file naming requirements
    if (strlen(_params.DomainString) > 0) {
	   _dsInputPath->setSubString(_params.DomainString);
	}
    if (strlen(_params.File_extension) > 0) {
      _dsInputPath->setSearchExt(_params.File_extension);
    }
  }
#endif

}

/////////////
// Destructor

InputPath::~InputPath()

{
  delete _dsInputPath;
}

/////////////////////
// get next file path
//
// returns empty string on failure

const string &InputPath::next()

{
  
  if (_archiveMode) {
    
    // in archive mode, go through the file list
    
    if (_currentPath != _filePaths.end()) {
      _inputPath = *_currentPath;
      _currentPath++;
    } else {
      _inputPath = "";
    }

  } else {
    
    // in realtime mode wait for change in latest info
    // sleep 1 second between tries.
    //
    // Check to see if the substring _params.DomainString
    // is in the filename. Keep looping until it is,
    // if this parameter has been specified.
    // This change made by Niles for WSMR.
    // This loop needs to be done even if we
    // have setSubString on the DsInputPath object, since
    // setting that only applies if we are not using
    // ldata_info files.
    //

    char *filePath;
    do { // String check loop.

      filePath = _dsInputPath->next();

      if (_params.debug >= Params::DEBUG_NORM){
	cerr << "File " << filePath << " detected." << endl;
      }

      _inputPath = filePath;

    }while (true);
    
	    // (strlen(_params.DomainString) > 0) &&
	    // (NULL == strstr(filePath, _params.DomainString))
	    // );

    if (_params.debug >= Params::DEBUG_NORM){
      cerr << "Input path returned is " << _inputPath  << endl;
    }


  } // if (archiveMode)
  
  return (_inputPath);

}

/////////////////////////
// reset to start of list
// 
// Archive mode only.

void InputPath::reset()

{
  if (_archiveMode) {
    _currentPath = _filePaths.begin();
  }
}



