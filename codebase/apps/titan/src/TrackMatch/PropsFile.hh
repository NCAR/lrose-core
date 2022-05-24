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
// PropsFile.h
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 1998
//
//////////////////////////////////////////////////////////

#ifndef PropsFile_h
#define PropsFile_h

#include <toolsa/umisc.h>
#include <titan/case_tracks.h>
#include "Params.hh"
#include "PropsList.hh"
using namespace std;

class PropsFile {
  
public:

  // constructor
  
  PropsFile (const char *prog_name,
	     const Params &params,
	     case_track_t *this_case,
	     char *file_path,
	     PropsList *list);
  
  // Destructor
  
  virtual ~PropsFile();
  
  // process file
  
  int update_list(initial_props_t *case_props);
  
  int search_for_case(case_track_t *this_case,
		      initial_props_t *case_props);
  
  // flag to indicate construction was successful

  int OK;
  
protected:
  
private:

  char *_progName;
  int _debug;
  const Params &_params;
  case_track_t *_this_case;
  char *_filePath;
  PropsList *_list;
  
};

#endif
