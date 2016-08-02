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
#include <toolsa/copyright.h>

/**
 * @file MdvMergeClosestMgr.cc
 */

#include "MdvMergeClosestMgr.hh"
#include "Trigger.hh"
#include <toolsa/LogMsg.hh>
#include <toolsa/DateTime.hh>

//----------------------------------------------------------------
MdvMergeClosestMgr::MdvMergeClosestMgr(const Parms &parms, 
				 const std::string &appName,
				 void tidyAndExit(int)) :
  _parms(parms), _trigger(NULL), _alg(parms)
{

  if (_parms._isArchive)
  {
    ArchiveTimeTrigger *t = new ArchiveTimeTrigger(appName, parms);
    _trigger = (Trigger *)t;
  }
  else
  {
    RealtimeTimeTrigger *t = new RealtimeTimeTrigger(appName, parms);
    _trigger = (Trigger *)t;
  }
}

//----------------------------------------------------------------
MdvMergeClosestMgr::~MdvMergeClosestMgr()
{
  if (_trigger != NULL)
  {
    delete _trigger;
  }
}

//----------------------------------------------------------------
int MdvMergeClosestMgr::run(void)
{
  if (_trigger == NULL)
  {
    // nothing to do without data
    return 1;
  }

  for (;;)
  {
    time_t t = _trigger->next();
    if (t < 0)
    {
      break;
    }
    LOGF(LogMsg::DEBUG, "------Triggered %s------", DateTime::strn(t).c_str());
    _alg.update(t);

  }
  return 0;
}

