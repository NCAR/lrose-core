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
// PulseQueue.hh
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

#ifndef PulseQueue_HH
#define PulseQueue_HH

#include <cstring>
#include <radar/IwrfTsPulse.hh>
using namespace std;

class PulseQueue

{

public:

  typedef IwrfTsPulse* *iterator;
  typedef const IwrfTsPulse* *const_iterator;
  
  // constructor and destructor
  
  PulseQueue();
  ~PulseQueue();
  
  // all other methods are inline for efficiency

  // get the size

  inline size_t size() { return _size; }

  // clear the queue

  void clear();

  // get the beginning and end iterators
  
  inline iterator begin() { return _begin; }
  inline iterator end() { return _endPlusOne; }

  // get items at front and back of queue

  inline IwrfTsPulse* &front() { return *_begin; }
  inline IwrfTsPulse* &back() { return *(_endPlusOne - 1); }

  // dereference

  inline IwrfTsPulse* operator[](size_t posn) {
    return _begin[posn];
  }

  // push onto the front of the queue
  // return address of front of queue

  inline iterator push_front(IwrfTsPulse *pulse) {
    if (_posBegin == 0) {
      _recenter();
    }
    _posBegin--;
    _begin--;
    *_begin = pulse;
    _size++;
    return _begin;
  }

  // push onto the back of the queue
  // return address of front of queue
  
  inline iterator push_back(IwrfTsPulse *pulse) {
    if (_posEndPlusOne >= _nAlloc - 1) {
      _recenter();
    }
    *_endPlusOne = pulse;
    _posEndPlusOne++;
    _endPlusOne++;
    _size++;
    return _begin;
  }

  // pop from front of queue
  // return pointer of pulse popped, NULL if empty

  inline IwrfTsPulse *pop_front() {
    if (_size == 0) return NULL;
    IwrfTsPulse *pulse = *_begin;
    *_begin = NULL;
    _posBegin++;
    _begin++;
    _size--;
    return pulse;
  }

  // pop from back of queue
  // return pointer of pulse popped, NULL if empty

  inline IwrfTsPulse *pop_back() {
    if (_size == 0) return NULL;
    IwrfTsPulse *pulse = *(_endPlusOne-1);
    *(_endPlusOne-1) = NULL;
    _posEndPlusOne--;
    _endPlusOne--;
    _size--;
    return pulse;
  }

  // erase an element from the queue
  // return iterator to elemant one past the item erased

  inline iterator erase(iterator position) {
    return erase(position, position);
  }

  // erase a number of elements from the queue
  // return iterator to elemant one past the item erased

  inline iterator erase(iterator first, iterator last) {
    if (first < _begin || last >= _endPlusOne) {
      return NULL;
    }
    int nErase = last - first + 1;
    int nMove = _endPlusOne - last - 1;
    memcpy(first, last + 1,
           nMove * sizeof(IwrfTsPulse *));
    _endPlusOne -= nErase;
    _posEndPlusOne -= nErase;
    _size -= nErase;
    return first;
  }

private:

  IwrfTsPulse **_buf;
  IwrfTsPulse **_begin;
  IwrfTsPulse **_endPlusOne;
  size_t _nAlloc;
  size_t _size;
  size_t _posBegin;
  size_t _posEndPlusOne;

  void _enlarge();
  void _recenter();
  

};


#endif
