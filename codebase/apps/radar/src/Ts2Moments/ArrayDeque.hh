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
// ArrayDeque.hh
//
// Efficient array-based double-ended queue
//
// EOL, NCAR, Boulder CO
//
// June 2012
//
// Mike Dixon
//
//////////////////////////////////////////////////////////////////////////

#ifndef ArrayDeque_HH
#define ArrayDeque_HH

#include <cstring>
using namespace std;

template <class TT>
class ArrayDeque

{

public:

  typedef TT *iterator;
  
  // constructor
  
  ArrayDeque()
  {
    _buf = NULL;
    clear();
  }

  // destructor
  
  ~ArrayDeque()
  {
    if (_buf) {
      delete[] _buf;
    }
  }
  
  // get the size

  inline size_t size() { return _size; }

  // Clear the queue
  
  inline void clear()
  {
    if (_buf) {
      delete[] _buf;
    }
    _nAlloc = 1024;
    _buf = new TT[_nAlloc];
    _size = 0;
    _posBegin = _nAlloc / 2;
    _posEndPlusOne = _posBegin;
    _begin = _buf + _posBegin;
    _endPlusOne = _buf + _posEndPlusOne;
  }

  // get the beginning and end iterators
  
  inline iterator begin() { return _begin; }
  inline iterator end() { return _endPlusOne; }

  // get items at front and back of queue

  inline TT &front() { return *_begin; }
  inline TT &back() { return *(_endPlusOne - 1); }

  // dereference

  inline TT operator[](size_t posn) {
    return _begin[posn];
  }

  // push onto the front of the queue
  // return address of front of queue

  inline iterator push_front(TT val) {
    if (_posBegin == 0) {
      _recenter();
    }
    _posBegin--;
    _begin--;
    *_begin = val;
    _size++;
    return _begin;
  }

  // push onto the back of the queue
  // return address of front of queue
  
  inline iterator push_back(TT val) {
    if (_posEndPlusOne >= _nAlloc - 1) {
      _recenter();
    }
    *_endPlusOne = val;
    _posEndPlusOne++;
    _endPlusOne++;
    _size++;
    return _begin;
  }

  // pop from front of queue
  // return pointer of val popped, NULL if empty

  inline TT pop_front() {
    if (_size == 0) return NULL;
    TT val = *_begin;
    *_begin = NULL;
    _posBegin++;
    _begin++;
    _size--;
    return val;
  }

  // pop from back of queue
  // return pointer of val popped, NULL if empty

  inline TT pop_back() {
    if (_size == 0) return NULL;
    TT val = *(_endPlusOne-1);
    *(_endPlusOne-1) = NULL;
    _posEndPlusOne--;
    _endPlusOne--;
    _size--;
    return val;
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
           nMove * sizeof(TT));
    _endPlusOne -= nErase;
    _posEndPlusOne -= nErase;
    _size -= nErase;
    return first;
  }

private:

  iterator _buf;
  iterator _begin;
  iterator _endPlusOne;
  size_t _nAlloc;
  size_t _size;
  size_t _posBegin;
  size_t _posEndPlusOne;

  // Recenter data in buffer

  void _recenter()
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
    iterator newBegin = _buf + newPosBegin;
    memcpy(newBegin, _begin, _size * sizeof(TT));
    
    // zero out the old area
    
    memset(_begin, 0, _size * sizeof(TT));
    
    // calculate new status
    
    _posBegin = newPosBegin;
    _posEndPlusOne = _posBegin + _size;
    _begin = _buf + _posBegin;
    _endPlusOne = _buf + _posEndPlusOne;
    
  }

  // Enlarge buffer and center data in resized buffer
  
  void _enlarge()
  {

    // allocate a buffer to at least 5 times the size of
    // the data
    
    int nNewAlloc = _size * 5;
    iterator tmp = new TT[nNewAlloc];
    memset(tmp, 0, nNewAlloc * sizeof(TT));
    
    // copy the existing data to the center of the new buffer
    
    int nFree = nNewAlloc - _size;
    int newPosBegin = nFree / 2;
    iterator newBegin = tmp + newPosBegin;
    memcpy(newBegin, _begin, _size * sizeof(TT));
    
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

};


#endif
