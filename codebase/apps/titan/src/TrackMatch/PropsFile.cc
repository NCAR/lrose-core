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
// PropsFile.cc: PropsFile handling
//
// Processes a properties file for track matching.
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 1998
//
//////////////////////////////////////////////////////////

#include "PropsFile.hh"
#include <toolsa/str.h>
using namespace std;

// Constructor

PropsFile::PropsFile (const char *prog_name,
		      const Params &params,
		      case_track_t *this_case,
		      char *file_path,
		      PropsList *list) :
        _params(params)

{

  // initialize
  
  OK = TRUE;
  _progName = STRdup(prog_name);
  _this_case = this_case;
  _filePath = STRdup(file_path);
  _list = list;

  return;

}

// Destructor

PropsFile::~PropsFile()

{

  // free strings

  STRfree(_progName);
  STRfree(_filePath);

}

///////////////////////////////////////////
// search_for_case()
//
// Search for the props for the given case.
//
// returns 0 on success, -1 on failure
//

int PropsFile::search_for_case(case_track_t *this_case,
			       initial_props_t *case_props)
  
{

  // open file

  FILE *fp;
  if ((fp = fopen(_filePath, "r")) == NULL) {
    fprintf(stderr, "ERROR - %s:PropsFile::process\n", _progName);
    fprintf(stderr, "Cannot open props file '%s'\n", _filePath);
    perror(_filePath);
    return (-1);
  }
  
  // read through file
  
  char line[1024];
  while (fgets(line, 1024, fp) != NULL) {
    if (_list->scan(line, case_props) == 0) {
      date_time_t ref_time;
      ref_time.unix_time = this_case->ref_time;
      uconvert_from_utime(&ref_time);
      if (ref_time.year == case_props->year &&
	  ref_time.month == case_props->month &&
	  ref_time.day == case_props->day &&
	  this_case->complex_track_num == case_props->complex_track_num) {
	fclose(fp);
	return (0);
      }
    }
  }
  
  // close file

  fclose(fp);

  return (-1);
  
}

/////////////////////////////////////////////////
// update_list()
//
// Update the list using the entries in the file
//
// returns 0 on success, -1 on failure
//

int PropsFile::update_list(initial_props_t *case_props)

{

  // open file

  FILE *fp;
  if ((fp = fopen(_filePath, "r")) == NULL) {
    fprintf(stderr, "ERROR - %s:PropsFile::process\n", _progName);
    fprintf(stderr, "Cannot open props file '%s'\n", _filePath);
    perror(_filePath);
    return (-1);
  }

  // read through file

  char line[1024];
  while (fgets(line, 1024, fp) != NULL) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "%s", line);
    }
    _list->update(line, case_props);
  }

  // close file

  fclose(fp);

  return (0);
  
}

