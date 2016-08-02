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
////////////////////////////////////////////////////////////////////////////////
// FmqDevice.cc
//
// Abstract base class for device hanFdling bythe Fmq class
// 
//
// Mike Dixon, RAL, NCAR, Boulder, Colorado
// Jan 2009
// 
////////////////////////////////////////////////////////////////////////////////
                                 
#include <cassert>
#include <cstdarg>
#include <dataport/bigend.h>
#include <toolsa/MsgLog.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <toolsa/uusleep.h>
#include <dsserver/DmapAccess.hh>
#include <Fmq/FmqDevice.hh>
using namespace std;

FmqDevice::FmqDevice(const string &fmqPath,
		     size_t numSlots, 
		     size_t bufSize,
		     TA_heartbeat_t heartbeat_func) :
	_fmqPath(fmqPath),
	_heartbeatFunc(heartbeat_func)
  
{

}

FmqDevice::~FmqDevice()
{
}
