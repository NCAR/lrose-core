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
// TaArray.hh
//
// Array wrapper
//
// Mike Dixon, RAL, NCAR, POBox 3000, Boulder, CO, 80307, USA
//
// Dec 2006
//
////////////////////////////////////////////////////////////////////
//
// TaArray allows you to declare an object, and allocate an array
// in that object, and have the array be automatically freed when
// the destructor is called.
//
// If you make the object an automatic variable, the destructor
// will be called when the object goes out of scope.
//
////////////////////////////////////////////////////////////////////


// NOTE - this latest implementation is from ChatGpt.
// 2026/04/18

#ifndef TA_ARRAY_HH
#define TA_ARRAY_HH

#include <cstddef>
#include <memory>
#include <algorithm>
#include <stdexcept>

template <class T>
class TaArray
{
public:

  // default constructor - empty array

  TaArray() = default;

  // constructor specifying number of elements in array

  explicit TaArray(size_t nelem)
  {
    alloc(nelem);
  }

  // destructor

  ~TaArray() = default;

  // copy constructor

  TaArray(const TaArray &rhs)
  {
    _copy(rhs);
  }

  // move constructor

  TaArray(TaArray &&rhs) noexcept = default;

  // copy assignment

  TaArray &operator=(const TaArray &rhs)
  {
    return _copy(rhs);
  }

  // move assignment

  TaArray &operator=(TaArray &&rhs) noexcept = default;

  // allocate array

  T *alloc(size_t nelem)
  {
    if (nelem == _nelem) {
      return _buf.get();
    }

    if (nelem == 0) {
      free();
      return nullptr;
    }

    _buf = std::make_unique<T[]>(nelem);
    _nelem = nelem;
    return _buf.get();
  }

  // free array

  void free()
  {
    _buf.reset();
    _nelem = 0;
  }

  // alias for free()

  void clear()
  {
    free();
  }

  // get size

  size_t size() const { return _nelem; }
  size_t nElem() const { return _nelem; }
  bool empty() const { return (_nelem == 0); }

  // get pointer to buffer

  T *buf() { return _buf.get(); }
  const T *buf() const { return _buf.get(); }

  T *dat() { return _buf.get(); }
  const T *dat() const { return _buf.get(); }

  T *data() { return _buf.get(); }
  const T *data() const { return _buf.get(); }

  // element access

  T &operator[](size_t idx) { return _buf[idx]; }
  const T &operator[](size_t idx) const { return _buf[idx]; }

  T &at(size_t idx)
  {
    if (idx >= _nelem) {
      throw std::out_of_range("TaArray::at index out of range");
    }
    return _buf[idx];
  }

  const T &at(size_t idx) const
  {
    if (idx >= _nelem) {
      throw std::out_of_range("TaArray::at index out of range");
    }
    return _buf[idx];
  }

  // iteration

  T *begin() { return _buf.get(); }
  const T *begin() const { return _buf.get(); }
  const T *cbegin() const { return _buf.get(); }

  T *end() { return _buf.get() + _nelem; }
  const T *end() const { return _buf.get() + _nelem; }
  const T *cend() const { return _buf.get() + _nelem; }

private:

  std::unique_ptr<T[]> _buf;
  size_t _nelem = 0;

  TaArray &_copy(const TaArray &rhs)
  {
    if (&rhs == this) {
      return *this;
    }

    if (rhs._nelem == 0 || rhs._buf.get() == nullptr) {
      free();
      return *this;
    }

    alloc(rhs._nelem);
    std::copy(rhs._buf.get(), rhs._buf.get() + _nelem, _buf.get());

    return *this;
  }

};

#endif

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#ifdef ORIGINAL_IMPLEMENTATION

#ifndef TA_ARRAY_ORIG_HH
#define TA_ARRAY_ORIG_HH

#include <cstdio>
#include <cstring>
using namespace std;

template <class T>
class TaArray
{
public:

  // default constructor - empty array

  TaArray();

  // constructor specifying number of elements in array

  TaArray(size_t nelem);

  // destructor

  ~TaArray();

  // copy constructor

  TaArray(const TaArray &rhs);

  // assignment (operator =)

  TaArray & operator=(const TaArray &rhs);

  // Alloc array

  T *alloc(size_t nelem);

  // free array

  void free();

  // get size

  size_t size() const { return _nelem; }

  // get pointer to buffer

  inline T *buf() const { return _buf; }
  inline T *dat() const { return _buf; }

private:

  T *_buf;
  size_t _nelem;
  
  TaArray &_copy(const TaArray &rhs);

};

// The Implementation.

// default constructor - empty array

template <class T>
TaArray<T>::TaArray()
{
  _buf = NULL;
  _nelem = 0;
}

// constructor specifying array size

template <class T>
TaArray<T>::TaArray(size_t nelem)
{
  _buf = new T[nelem];
  _nelem = nelem;
}

// destructor

template <class T>
TaArray<T>::~TaArray()
{
  free();
}

// copy constructor

template <class T>
TaArray<T>::TaArray(const TaArray<T> &rhs) {
  if (this != &rhs) {
    _buf = NULL;
    _nelem = 0;
    _copy(rhs);
  }
}

// assignment

template <class T>
TaArray<T>& TaArray<T>::operator=(const TaArray<T> &rhs) {
  return _copy(rhs);
}

// allocation

template <class T>
T *TaArray<T>::alloc(size_t nelem)
{
  if (nelem == _nelem) {
    return _buf;
  }
  free();
  _buf = new T[nelem];
  _nelem = nelem;
  return _buf;
}

// free up

template <class T>
void TaArray<T>::free()
{
  if (_buf != NULL) {
    delete[] _buf;
  }
  _buf = NULL;
  _nelem = 0;
}

// copy

template <class T>
TaArray<T> &TaArray<T>::_copy(const TaArray<T> &rhs)

{

  if (&rhs == this) {
    return *this;
  }

  if (rhs._nelem == 0 || rhs._buf == NULL) {
    free();
    return *this;
  }

  alloc(rhs._nelem);
  memcpy(_buf, rhs._buf, _nelem * sizeof(T));

  return *this;

}

#endif

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#endif // ORIGINAL_IMPLEMENTATION

