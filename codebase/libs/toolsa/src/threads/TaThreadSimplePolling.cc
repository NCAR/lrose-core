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
 * @file TaThreadSimplePolling.cc
 */
#include <toolsa/TaThreadSimplePolling.hh>
#include <toolsa/TaThreadPollingQue.hh>
#include <toolsa/LogStream.hh>
#include <cstdio>

//----------------------------------------------------------------
TaThreadSimplePolling::TaThreadSimplePolling(const int index) :
  TaThreadSimple(index)
{

}

//----------------------------------------------------------------
TaThreadSimplePolling::~TaThreadSimplePolling()
{

}

//----------------------------------------------------------------
void TaThreadSimplePolling::_after(void)
{
  TaThreadPollingQue *context = (TaThreadPollingQue *)getThreadContext();
  if (context == NULL)
  {
    LOG(ERROR) << "No context is set";
    return;
  }

  // wait for the que to be waiting for input
  context->waitForQueToWait();

  // let the que know this thread is complete
  int index = getThreadIndex();
  context->signalFromThreadWhenComplete(index);

}    

