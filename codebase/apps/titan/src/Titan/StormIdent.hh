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
// StormIdent.hh
//
// StormIdent object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
///////////////////////////////////////////////////////////////

#ifndef StormIdent_H
#define StormIdent_H

#include <titan/TitanStormFile.hh>
#include "Worker.hh"
#include "InputMdv.hh"
#include "StormTrack.hh"
#include "Identify.hh"
using namespace std;
class DsMdvxTimes;

////////////////////////
// This class

class StormIdent : public Worker {
  
public:

  // constructor

  StormIdent (const string &prog_name,
	      const Params &params);

  // destructor
  
  ~StormIdent();

  // run 

  int runRealtime();

  int runArchive(time_t overlap_start_time,
		 time_t start_time,
		 time_t end_time);

  int runForecast(time_t gen_time);

  // data members
  
  bool OK;
  static const int maxEigDim;
  static const float missingVal;

protected:
  
private:

  char _headerFilePath[MAX_PATH_LEN];

  TitanStormFile _sfile;
  InputMdv _inputMdv;
  Identify _identify;

  bool _fatalError;

  void _loadHeaderFilePath(const time_t ftime);
  void _loadStormParams(storm_file_params_t *sparams);
  int _processScan(int scan_num, time_t scan_time,
		   StormTrack &tracking);

  void _getLastMatch(int initial_scan_match,
		     int *last_scan_match_p,
		     int *last_time_match_p,
		     const DsMdvxTimes &time_list);

  int _openAndCheck(const char *header_file_path,
		    const storm_file_params_t *expected_params);
  
  int _findCurrentScan(const DsMdvxTimes &time_list,
		       int &current_scan,
		       time_t &current_time);

  int _prepareNew(const char *header_file_path,
		  const storm_file_params_t *sparams);

  int _prepareOld(const char *header_file_path,
		  int current_scan_num);

  int _writeLdataInfo();
 
};

#endif

