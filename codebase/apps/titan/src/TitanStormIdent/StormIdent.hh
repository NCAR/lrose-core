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
// Dave Albo
//
// August 2014
//
///////////////////////////////////////////////////////////////

#ifndef StormIdent_H
#define StormIdent_H

#include <titan/TitanStormFile.hh>
#include "Worker.hh"
#include "InputMdv.hh"
#include "Identify.hh"
#include "ThresholdManager.hh"

////////////////////////
// This class

class StormIdent : public Worker {
  
public:

  // constructor

  StormIdent (const string &prog_name,
	      const Params &params);

  // destructor
  
  ~StormIdent();

  // run, realtime

  int runRealtime();

  // run, archive

  int runArchive(time_t start_time,
		 time_t end_time);

  // data members
  
  bool OK;
  static const int maxEigDim;
  static const float missingVal;

protected:
  
private:

  TitanStormFile _sfile;
  InputMdv _inputMdv;
  ThresholdManager _thresh;
  Identify _identify;

  bool _fatalError;

  void _loadStormParams(storm_file_params_t *sparams);
  int _runForecast(void);
  int _runFlat(void);
  int _runForecast(time_t start_time, time_t end_time);
  int _runFlat(time_t start_time, time_t end_time);
  int _processScan(int scan_num, time_t scan_time);
  int _processScan(int scan_num, time_t scan_gen_time, int scan_lead_time);
 
};

#endif

