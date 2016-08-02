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
 * @file Looper.cc
 */
#include <cstdlib>
#include <FiltAlg/Looper.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
Looper::Looper(const int nx, const int ny)
{
  _nx = nx;
  _ny = ny;
  if (_nx < 2 || _ny < 2)
  {
    LOG(ERROR) << "too few x,y " << nx << " " << ny;
    exit(-1);
  }
  _x = 0;
  _y = 0;
  _state = INIT;
}

//------------------------------------------------------------------
Looper::~Looper()
{
}

//------------------------------------------------------------------
Looper::State_t Looper::get_xy(int &x, int &y) const
{
  x = _x;
  y = _y;
  return _state;
}

//------------------------------------------------------------------
bool Looper::increment(void)
{
  switch (_state)
  {
  case INIT:
    _state = INC_Y;
    _y++;
    break;
  case INC_Y:
    if (_y+1 < _ny)
    {
      ++_y;
    }
    else
    {
      if (++_x >= _nx)
      {
	return false;
      }
      _state = INC_X;
    }
    break;
  case DEC_Y:
    if (_y-1 >= 0)
    {
      --_y;
    }
    else
    {
      if (++_x >= _nx)
      {
	return false;
      }
      _state = INC_X;
    }
    break;
  case INC_X:
    if (_y >= _ny-1)
    {
      _state = DEC_Y;
      --_y;
    }
    else if (_y == 0)
    {
      _state = INC_Y;
      ++_y;
    }
    else
    {
      LOG(ERROR) << "state bad";
    }
    break;
  default:
    LOG(ERROR) << "state bad";
    break;
  }
  return true;
}

