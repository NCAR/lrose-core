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

#ifndef TA_ARRAY_HH
#define TA_ARRAY_HH

using namespace std;

template <class T>
class TaArray
{
public:

  TaArray();
  ~TaArray();

  // Alloc array

  T *alloc(int nelem);

  // free array

  void free();

private:

  T *_buf;
  int _nelem;

};

// The Implementation.

template <class T>
TaArray<T>::TaArray()
{
  _buf = NULL;
  _nelem = 0;
}

template <class T>
TaArray<T>::~TaArray()
{
  free();
}

template <class T>
T *TaArray<T>::alloc(int nelem)
{
  free();
  _buf = new T[nelem];
  _nelem = nelem;
  return _buf;
}

template <class T>
void TaArray<T>::free()
{
  if (_buf != NULL) {
    delete[] _buf;
  }
  _buf = NULL;
  _nelem = 0;
}

#endif
