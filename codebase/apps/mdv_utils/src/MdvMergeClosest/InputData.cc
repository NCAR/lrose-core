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
 * @file InputData.cc
 */

#include "InputData.hh"
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/LogMsg.hh>
#include <toolsa/DateTime.hh>

//----------------------------------------------------------------
InputData::InputData(const ParmInput &dparm, const Parms &parm) :
  Data(dparm, parm), _first(true), _status(BAD)
{
}

//----------------------------------------------------------------
InputData::~InputData()
{
}

//----------------------------------------------------------------
InputData::Status_t InputData::update(const time_t &t)
{
  DsMdvx in; 
  in.setTimeListModeValid(_dataParm._url, t - _parm._triggerMargin, t);
  in.compileTimeList();
  const vector<time_t> times = in.getTimeList();
  if (times.empty())
  {
    if (_first)
    {
      return UNCHANGED;
    }
    else
    {
      if (_status == TIMEOUT)
      {
	return UNCHANGED;
      }
      else
      {
	_status = TIMEOUT;
	return TIMEOUT;
      }
    }
  }
  else
  {
    time_t tlast = *(times.rbegin());
    if (tlast == _time)
    {
      _status = UNCHANGED;
      return UNCHANGED;
    }
    else
    {
      _time = tlast;
      // fallthrough
    }
  }

  // here have a new time, _time.
  // set up read
  if (!read(in))
  {
    _status = BAD;
    return BAD;
  }

  if (_first)
  {
    _first = false;
    _status = FIRST_DATA;
    return FIRST_DATA;
  }
  else
  {
    _status = NEW_DATA;
    return NEW_DATA;
  }
}

