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
//////////////////////////////////////////////////////////////////////////
// PulseQueue.cc
//
// Efficient array-based queue for pulses
//
// EOL, NCAR, Boulder CO
//
// Oct 2010
//
// Mike Dixon
//
//////////////////////////////////////////////////////////////////////////

#include "PulseQueue.hh"
#include <iostream>

//////////////////////////////////////////////////////////////////////////
// Constructor

PulseQueue::PulseQueue()
{
  _buf = NULL;
  clear();
}

//////////////////////////////////////////////////////////////////////////
// Destructor

PulseQueue::~PulseQueue()
{
  if (_buf) {
    delete[] _buf;
  }
}

//////////////////////////////////////////////////////////////////////////
// Clear the queue

void PulseQueue::clear()
{
  if (_buf) {
    delete[] _buf;
  }
  _nAlloc = 1024;
  _buf = new IwrfTsPulse*[_nAlloc];
  _size = 0;
  _posBegin = _nAlloc / 2;
  _posEndPlusOne = _posBegin;
  _begin = _buf + _posBegin;
  _endPlusOne = _buf + _posEndPlusOne;
}

//////////////////////////////////////////////////////////////////////////
// Recenter data in buffer

void PulseQueue::_recenter()
{

  // First we check that the buffer size is at least 5 times
  // as large as the data. This is necessary so that when
  // using memcpy() we do not have overlapping ranges.

  if (_nAlloc < _size * 5) {
    _enlarge();
    return;
  }
  
  // copy the existing data to the center of the buffer
  
  int nFree = _nAlloc - _size;
  int newPosBegin = nFree / 2;
  IwrfTsPulse **newBegin = _buf + newPosBegin;
  memcpy(newBegin, _begin, _size * sizeof(IwrfTsPulse *));

  // zero out the old area

  memset(_begin, 0, _size * sizeof(IwrfTsPulse *));

  // calculate new status

  _posBegin = newPosBegin;
  _posEndPlusOne = _posBegin + _size;
  _begin = _buf + _posBegin;
  _endPlusOne = _buf + _posEndPlusOne;
  
}

//////////////////////////////////////////////////////////////////////////
// Enlarge buffer and center data in resized buffer

void PulseQueue::_enlarge()
{

  // allocate a buffer to at least 5 times the size of
  // the data

  int nNewAlloc = _size * 5;
  IwrfTsPulse **tmp = new IwrfTsPulse*[nNewAlloc];
  memset(tmp, 0, nNewAlloc * sizeof(IwrfTsPulse *));
  
  // copy the existing data to the center of the new buffer

  int nFree = nNewAlloc - _size;
  int newPosBegin = nFree / 2;
  IwrfTsPulse **newBegin = tmp + newPosBegin;
  memcpy(newBegin, _begin, _size * sizeof(IwrfTsPulse *));

  // swap buffers

  delete[] _buf;
  _buf = tmp;

  // calculate new status

  _nAlloc = nNewAlloc;
  _posBegin = newPosBegin;
  _posEndPlusOne = _posBegin + _size;
  _begin = _buf + _posBegin;
  _endPlusOne = _buf + _posEndPlusOne;

}

