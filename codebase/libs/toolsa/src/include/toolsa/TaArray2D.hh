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
// TaArray2D.hh
//
// Array wrapper
//
// Mike Dixon, EOL, NCAR, POBox 3000, Boulder, CO, 80307, USA
//
// March 2016
//
////////////////////////////////////////////////////////////////////
//
// TaArray2D allows you to declare an object, and allocate a 2D array
// of that object, and have the array be automatically freed when
// the destructor is called.
//
// If you make the object an automatic variable, the destructor
// will be called when the object goes out of scope.
//
////////////////////////////////////////////////////////////////////

// NOTE - this latest implementation is from ChatGpt.
// 2026/04/18

#ifndef TA_ARRAY_2D_HH
#define TA_ARRAY_2D_HH

#include <cstddef>
#include <memory>
#include <algorithm>
#include <stdexcept>

template <class T>
class TaArray2D
{
public:

  // default constructor - empty array

  TaArray2D() = default;

  // constructor specifying dimensions of array

  TaArray2D(size_t sizeMajor, size_t sizeMinor)
  {
    _alloc(sizeMajor, sizeMinor);
  }

  // destructor

  ~TaArray2D() = default;

  // copy constructor

  TaArray2D(const TaArray2D &rhs)
  {
    _copy(rhs);
  }

  // move constructor

  TaArray2D(TaArray2D &&rhs) noexcept = default;

  // copy assignment

  TaArray2D &operator=(const TaArray2D &rhs)
  {
    return _copy(rhs);
  }

  // move assignment

  TaArray2D &operator=(TaArray2D &&rhs) noexcept = default;

  // allocate array

  T **alloc(size_t sizeMajor, size_t sizeMinor)
  {
    _alloc(sizeMajor, sizeMinor);
    return _dat2D.get();
  }

  // free array

  void free()
  {
    _dat2D.reset();
    _dat1D.reset();
    _sizeMajor = 0;
    _sizeMinor = 0;
    _size1D = 0;
  }

  // alias for free()

  void clear()
  {
    free();
  }

  // get size

  size_t sizeMajor() const { return _sizeMajor; }
  size_t sizeMinor() const { return _sizeMinor; }
  size_t size1D() const { return _size1D; }

  size_t nRows() const { return _sizeMajor; }
  size_t nCols() const { return _sizeMinor; }

  bool empty() const { return (_size1D == 0); }

  // direct access to row-pointer view

  T **dat2D() { return _dat2D.get(); }
  T * const *dat2D() const { return _dat2D.get(); }

  T **rows() { return _dat2D.get(); }
  T * const *rows() const { return _dat2D.get(); }

  // direct access to contiguous 1D storage

  T *dat1D() { return _dat1D.get(); }
  const T *dat1D() const { return _dat1D.get(); }

  T *data() { return _dat1D.get(); }
  const T *data() const { return _dat1D.get(); }

  // row access

  T *operator[](size_t irow) { return _dat2D[irow]; }
  const T *operator[](size_t irow) const { return _dat2D[irow]; }

  T *row(size_t irow) { return _dat2D[irow]; }
  const T *row(size_t irow) const { return _dat2D[irow]; }

  T *rowAt(size_t irow)
  {
    if (irow >= _sizeMajor) {
      throw std::out_of_range("TaArray2D::rowAt row out of range");
    }
    return _dat2D[irow];
  }

  const T *rowAt(size_t irow) const
  {
    if (irow >= _sizeMajor) {
      throw std::out_of_range("TaArray2D::rowAt row out of range");
    }
    return _dat2D[irow];
  }

  // element access

  T &at(size_t irow, size_t icol)
  {
    if (irow >= _sizeMajor || icol >= _sizeMinor) {
      throw std::out_of_range("TaArray2D::at index out of range");
    }
    return _dat2D[irow][icol];
  }

  const T &at(size_t irow, size_t icol) const
  {
    if (irow >= _sizeMajor || icol >= _sizeMinor) {
      throw std::out_of_range("TaArray2D::at index out of range");
    }
    return _dat2D[irow][icol];
  }

  // flat indexing into contiguous data

  T &flat(size_t idx)
  {
    if (idx >= _size1D) {
      throw std::out_of_range("TaArray2D::flat index out of range");
    }
    return _dat1D[idx];
  }

  const T &flat(size_t idx) const
  {
    if (idx >= _size1D) {
      throw std::out_of_range("TaArray2D::flat index out of range");
    }
    return _dat1D[idx];
  }

  // iteration over contiguous 1D storage

  T *begin() { return _dat1D.get(); }
  const T *begin() const { return _dat1D.get(); }
  const T *cbegin() const { return _dat1D.get(); }

  T *end() { return _dat1D.get() + _size1D; }
  const T *end() const { return _dat1D.get() + _size1D; }
  const T *cend() const { return _dat1D.get() + _size1D; }

private:

  std::unique_ptr<T*[]> _dat2D;
  std::unique_ptr<T[]> _dat1D;
  size_t _sizeMajor = 0;
  size_t _sizeMinor = 0;
  size_t _size1D = 0;

  TaArray2D &_copy(const TaArray2D &rhs)
  {
    if (&rhs == this) {
      return *this;
    }

    if (rhs._size1D == 0 || rhs._dat1D.get() == nullptr) {
      free();
      return *this;
    }

    _alloc(rhs._sizeMajor, rhs._sizeMinor);
    std::copy(rhs._dat1D.get(), rhs._dat1D.get() + _size1D, _dat1D.get());

    return *this;
  }

  void _alloc(size_t sizeMajor, size_t sizeMinor)
  {
    if (sizeMajor == _sizeMajor &&
        sizeMinor == _sizeMinor) {
      return;
    }

    if (sizeMajor == 0 || sizeMinor == 0) {
      free();
      return;
    }

    _sizeMajor = sizeMajor;
    _sizeMinor = sizeMinor;
    _size1D = _sizeMajor * _sizeMinor;

    _dat1D = std::make_unique<T[]>(_size1D);
    _dat2D = std::make_unique<T*[]>(_sizeMajor);

    for (size_t ii = 0; ii < _sizeMajor; ii++) {
      _dat2D[ii] = _dat1D.get() + ii * _sizeMinor;
    }
  }

};

#endif

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#ifdef ORIGINAL_IMPLEMENTATION

#ifndef TA_ARRAY_2D_ORIG_HH
#define TA_ARRAY_2D_ORIG_HH

#include <cstdio>
#include <cstring>
using namespace std;

template <class T>
  class TaArray2D
{
public:

  // default constructor - empty array

  TaArray2D();

  // constructor specifying dimensions of array

  TaArray2D(size_t sizeMajor, size_t sizeMinor);

  // destructor

  ~TaArray2D();

  // copy constructor
  
  TaArray2D(const TaArray2D &rhs);

  // assignment (operator =)

  TaArray2D & operator=(const TaArray2D &rhs);

  // Alloc array

  T **alloc(size_t sizeMajor, size_t sizeMinor);

  // free array

  void free();

  // get size

  size_t sizeMajor() const { return _sizeMajor; }
  size_t sizeMinor() const { return _sizeMinor; }
  size_t size1D() const { return _size1D; }

  // get pointer to data

  inline T **dat2D() const { return _dat2D; }
  inline T *dat1D() const { return _dat1D; }

private:

  T **_dat2D;
  T *_dat1D;
  size_t _sizeMajor;
  size_t _sizeMinor;
  size_t _size1D;
  
  TaArray2D &_copy(const TaArray2D &rhs);
  void _alloc(size_t sizeMajor, size_t sizeMinor);
  void _init();

};

// The Implementation.

// default constructor - empty array

template <class T>
  TaArray2D<T>::TaArray2D()
{
  _init();
}

// constructor specifying array dimension

template <class T>
  TaArray2D<T>::TaArray2D(size_t sizeMajor, size_t sizeMinor)
{
  _init();
  _alloc(sizeMajor, sizeMinor);
}

// destructor

template <class T>
  TaArray2D<T>::~TaArray2D()
{
  free();
}

// copy constructor

template <class T>
  TaArray2D<T>::TaArray2D(const TaArray2D<T> &rhs) {
  _init();
  if (this != &rhs) {
    free();
    _copy(rhs);
  }
}

// assignment

template <class T>
  TaArray2D<T>& TaArray2D<T>::operator=(const TaArray2D<T> &rhs) {
  return _copy(rhs);
}

// allocation

template <class T>
  T **TaArray2D<T>::alloc(size_t sizeMajor, size_t sizeMinor)
{
  _alloc(sizeMajor, sizeMinor);
  return _dat2D;
}

template <class T>
  void TaArray2D<T>::_alloc(size_t sizeMajor, size_t sizeMinor)
{
  if (sizeMajor == _sizeMajor &&
      sizeMinor == _sizeMinor) {
    return;
  }
  free();
  _sizeMajor = sizeMajor;
  _sizeMinor = sizeMinor;
  _size1D = _sizeMajor * _sizeMinor;
  _dat1D = new T[_size1D];
  _dat2D = new T*[_sizeMajor];
  for (size_t ii = 0; ii < _sizeMajor; ii++) {
    _dat2D[ii] = _dat1D + ii * _sizeMinor;
  }
}

// initialize in constructor

template <class T>
  void TaArray2D<T>::_init()
{
  _dat2D = NULL;
  _dat1D = NULL;
  _size1D = 0;
  _sizeMajor = 0;
  _sizeMinor = 0;
}

// free up

template <class T>
  void TaArray2D<T>::free()
{
  if (_dat1D != NULL) {
    delete[] _dat1D;
  }
  if (_dat2D != NULL) {
    delete[] _dat2D;
  }
  _dat1D = NULL;
  _dat2D = NULL;
  _size1D = 0;
  _sizeMajor = 0;
  _sizeMinor = 0;
}

// copy

template <class T>
  TaArray2D<T> &TaArray2D<T>::_copy(const TaArray2D<T> &rhs)

{

  if (&rhs == this) {
    return *this;
  }

  if (rhs._size1D == 0 || rhs._dat2D == NULL) {
    free();
    return *this;
  }

  _alloc(rhs._sizeMajor, rhs._sizeMinor);
  memcpy(_dat2D, rhs._dat2D, _sizeMajor * sizeof(T*));
  memcpy(_dat1D, rhs._dat1D, _size1D * sizeof(T));

  return *this;

}

#endif

#endif // ORIGINAL IMPLEMENTATION

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

