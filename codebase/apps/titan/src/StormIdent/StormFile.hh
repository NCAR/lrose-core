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
// StormFile.hh
//
// StormFile class
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#ifndef StormFile_HH
#define StormFile_HH

#include "Params.hh"
#include <titan/storm.h>
using namespace std;

class TimeList;

////////////////////////////////
// StormFile

class StormFile {
  
public:

  // constructor

  StormFile(char *prog_name, Params *params);

  // destructor
  
  virtual ~StormFile();

  // handle

  storm_file_handle_t handle;

  // open file and check params

  int openAndCheck(char *header_file_path,
		   storm_file_params_t *expected_params);

  // get previously used scan number

  int getPrevScan(const TimeList &time_list,
		  int &start_posn);

  // Prepare a new file

  int prepareNew(char *header_file_path,
		 storm_file_params_t *sparams);

  // Prepare a old file

  int prepareOld(char *header_file_path,
		 int current_scan_num);

  // Write latest data info

  void writeLdataInfo();

  // Put an advisory write lock on the header file.
  
  int lockHeaderFile();

  // Remove advisory lock on the header file.

  int unlockHeaderFile();

  // number of dbz histogram intervals

  int nDbzHistIntervals;

  int OK;

protected:
  
private:

  char *_progName;
  Params *_params;

  void _getLastMatch(int initial_scan_match,
		     int *last_scan_match_p,
		     int *last_time_match_p,
		     const TimeList &time_list);

  int _truncate(FILE **fd, char *path, int length);

};

#endif


