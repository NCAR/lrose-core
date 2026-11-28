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
/**
 * @file Parms.cc
 */

//------------------------------------------------------------------
#include "Parms.hh"
#include "Params.hh"
#include <dsdata/DsUrlTrigger.hh>
#include <algorithm>

//------------------------------------------------------------------
Parms::Parms(void) : _ok(false)
{
}

//------------------------------------------------------------------
Parms::Parms(int argc, char **argv) : _ok(true)
{
  Params P;
  char *path=NULL;
  bool error;
  if (!DsUrlTrigger::checkArgs(argc, argv, _archiveT0, _archiveT1, _isArchive,
			       error))
  {
    if (error)
    {
      // error in parsing
      printf("ERROR checking args\n");
      _ok = false;
      return;
    }
    else
    {
      // a help request
      return;
    }
  }

  if (P.loadFromArgs(argc, argv, NULL, &path))
  {
    printf("ERROR loading params\n");
    _ok = false;
    return;
  }

  _debug = P.debug;
  _debugVerbose = P.debug_verbose;
  // _numThreads = 1; //P.num_threads;
  // _threadDebug = false; //P.thread_debug;
  _instance = P.instance;
  _registerSeconds = P.register_seconds;

  for (int i=0; i<P.input_n; ++i)
  {
    _input.push_back(ParmInput(P._input[i]));
  }
  for (int i=0; i<P.fields_n; ++i)
  {
    _field.push_back(P._fields[i]);
  }
  
  _triggerInterval = P.time_trigger_interval;
  _triggerOffset = P.time_trigger_offset;
  _triggerMargin = P.time_trigger_margin;
  _timeoutSeconds = P.timeout_seconds;
  _outputUrl = P.output_url;
}

//------------------------------------------------------------------
Parms::~Parms(void)
{
}
