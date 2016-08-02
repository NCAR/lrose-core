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
/************************************************************************
 * ComplexNumber.hh : header file for ComplexNumber object
 *
 * RAP, NCAR, Boulder CO
 *
 * December 1996
 *
 * Nancy Rehak
 *
 * Taken from "Advanced C++: Programming Styles and Idioms"
 * by James O. Coplien.
 ************************************************************************/

#ifndef ComplexNumber_HH
#define ComplexNumber_HH

/*
 **************************** includes ***********************************
 */


#include <iostream>
#include <cstdio>
#include <math.h>
using namespace std;

/*
 ******************************** defines ********************************
 */

/*
 ******************************** types **********************************
 */

/*
 ************************* class definitions *****************************
 */

class ComplexNumber
{
 public:
  
  // Constructors

  ComplexNumber(double real_coeff = 0, double imaginary_coeff = 0):
    _rpart(real_coeff), _ipart(imaginary_coeff)
  {
  }
  
  ComplexNumber(const ComplexNumber &c): _rpart(c._rpart), _ipart(c._ipart)
  {
  }
  
  // Operators

  ComplexNumber& operator=(const ComplexNumber &c)
  {
    _rpart = c._rpart;
    _ipart = c._ipart;
    return(*this);
  }
  
  ComplexNumber operator+(const ComplexNumber &c) const
  {
    return(ComplexNumber(_rpart + c._rpart,
		   _ipart + c._ipart));
  }
  
  friend ComplexNumber operator+(double d, const ComplexNumber &c)
  {
    return(c + ComplexNumber(d));
  }
  
  friend ComplexNumber operator+(int i, const ComplexNumber &c)
  {
    return(c + ComplexNumber(i));
  }
  
  ComplexNumber operator-(const ComplexNumber &c1) const
  {
    return(ComplexNumber(_rpart - c1._rpart, _ipart - c1._ipart));
  }
  
  friend ComplexNumber operator-(double d, const ComplexNumber &c)
  {
    return(-c + ComplexNumber(d));
  }
  
  friend ComplexNumber operator-(int i, const ComplexNumber &c)
  {
    return(-c + ComplexNumber(i));
  }
  
  ComplexNumber operator*(const ComplexNumber &c) const 
  {
    return(ComplexNumber(_rpart * c._rpart - _ipart * c._ipart,
			 _rpart * c._ipart + _ipart * c._rpart));
  }
  
  friend ComplexNumber operator*(double d, const ComplexNumber& c)
  {
    return(c * ComplexNumber(d));
  }
  
  friend ComplexNumber operator*(int i, const ComplexNumber& c)
  {
    return(c * ComplexNumber(i));
  }
  
  operator double()
  {
    return(::sqrt(_rpart * _rpart + _ipart * _ipart));
  }
  
  ComplexNumber operator-() const
  {
    return(ComplexNumber(-_rpart, -_ipart));
  }
  
  ostream& operator<<(ostream&s)
  {
    return(s << _rpart << " + " << _ipart << 'i');
  }
    
  double getReal(void)
  {
    return(_rpart);
  }
  
  double getImaginary(void)
  {
    return(_ipart);
  }
  
  double magnitude(void);
  double angle(void);
  
 private:
  
  double _rpart, _ipart;
};


#endif
