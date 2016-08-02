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
 * @file TaThreadSimple.cc
 */

#include <toolsa/TaThreadSimple.hh>
#include <toolsa/LogStream.hh>
#include <cstdio>

//----------------------------------------------------------------
TaThreadSimple::TaThreadSimple(const int index) : TaThread(),
						  _threadIndex(index)
{

}

//----------------------------------------------------------------
TaThreadSimple::~TaThreadSimple()
{

}

//----------------------------------------------------------------
void TaThreadSimple::run(void)
{
  TaThread::ThreadMethod_t m = getThreadMethod();
  void *info = getThreadInfo();
  if (m == NULL)
  {
    LOG(ERROR) << "method pointer not set";
  }
  if (info == NULL)
  {
    LOG(ERROR) << "info pointer not set";
  }
  if (m != NULL && info != NULL)
  {
    _before();
    m(info);
    _after();
  }
}    

