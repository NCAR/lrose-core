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
////////////////////////////////////////////////////////////////////
// RadxArray.hh
//
// Array wrapper
//
// Mike Dixon, RAL, NCAR, POBox 3000, Boulder, CO, 80307, USA
//
// Dec 2006
//
////////////////////////////////////////////////////////////////////
//
// RadxArray allows you to declare an object, and allocate an array
// in that object, and have the array be automatically freed when
// the destructor is called.
//
// If you make the object an automatic variable, the destructor
// will be called when the object goes out of scope.
//
////////////////////////////////////////////////////////////////////

#ifndef RADX_ARRAY_HH
#define RADX_ARRAY_HH

#include <cstdio>
using namespace std;

template <class T>
class RadxArray
{
public:

  // default constructor - empty array

  RadxArray();

  // constructor specifying number of elements in array

  RadxArray(int nelem);

  // destructor

  ~RadxArray();

  // copy constructor

  RadxArray(const RadxArray &rhs);

  // assignment (operator =)

  RadxArray & operator=(const RadxArray &rhs);

  // Alloc array

  T *alloc(int nelem);

  // free array

  void clear();

  // get size

  int size() const { return _nelem; }

  // get pointer to buffer

  inline T *buf() const { return _buf; }
  inline T *dat() const { return _buf; }

private:

  T *_buf;
  int _nelem;
  
  RadxArray &_copy(const RadxArray &rhs);

};

// The Implementation.

// default constructor - empty array

template <class T>
RadxArray<T>::RadxArray()
{
  _buf = NULL;
  _nelem = 0;
}

// constructor specifying array size

template <class T>
RadxArray<T>::RadxArray(int nelem)
{
  _buf = new T[nelem];
  _nelem = nelem;
}

// destructor

template <class T>
RadxArray<T>::~RadxArray()
{
  clear();
}

// copy constructor

template <class T>
RadxArray<T>::RadxArray(const RadxArray<T> &rhs) {
  if (this != &rhs) {
    _buf = NULL;
    _nelem = 0;
    _copy(rhs);
  }
}

// assignment

template <class T>
RadxArray<T>& RadxArray<T>::operator=(const RadxArray<T> &rhs) {
  return _copy(rhs);
}

// allocation

template <class T>
T *RadxArray<T>::alloc(int nelem)
{
  if (nelem == _nelem) {
    return _buf;
  }
  clear();
  _buf = new T[nelem];
  _nelem = nelem;
  return _buf;
}

// free up

template <class T>
void RadxArray<T>::clear()
{
  if (_buf != NULL) {
    delete[] _buf;
  }
  _buf = NULL;
  _nelem = 0;
}

// copy

template <class T>
RadxArray<T> &RadxArray<T>::_copy(const RadxArray<T> &rhs)

{

  if (&rhs == this) {
    return *this;
  }

  if (rhs._nelem == 0 || rhs._buf == NULL) {
    clear();
    return *this;
  }

  alloc(rhs._nelem);
  memcpy(_buf, rhs._buf, _nelem * sizeof(T));

  return *this;

}

#endif
