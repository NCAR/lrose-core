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
// TrackFile.h: TrackFile handling
//
// Processes one track file
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 1998
//
//////////////////////////////////////////////////////////

#ifndef TrackFile_h
#define TrackFile_h

#include <toolsa/umisc.h>
#include <titan/track.h>
#include <string>
#include "Params.hh"
#include "Entity.hh"
using namespace std;

class TrackFile {
  
public:

  // constructor
       
  TrackFile (const string &prog_name, const Params &params,
	     Entity *entity,
	     char *file_path);
  
  // Destructor
  
  virtual ~TrackFile();
  
  // process file
  
  int process();
  
  // flag to indicate construction was successful

  int OK;
  
  // number of tracks

  int ncomplex() { return _nComplex; }
  int nsimple() { return _nSimple; }

protected:
  
private:

  const string &_progName;
  const Params &_params;
  Entity *_entity;
  char *_filePath;
  
  int _nComplex;
  int _nSimple;

  storm_file_handle_t _s_handle;
  track_file_handle_t _t_handle;

};

#endif
