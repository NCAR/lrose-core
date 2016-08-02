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
 * @file MotionVector.hh
 * @brief motion represented as a vector
 *
 * @class MotionVector
 * @brief motion represented as a vector
 */

# ifndef    MOTION_VECTOR_H
# define    MOTION_VECTOR_H

#include <string>

class MotionVector
{
public:
  MotionVector(void);
  MotionVector(double, double);
  virtual ~MotionVector();
  inline bool operator==(const MotionVector &v) const
  {
    return _vx == v._vx && _vy == v._vy;
  }

  void average(const MotionVector &v);
  void add(const MotionVector &v);
  void mult(const double v);

  // adjust so rotation by angle gives a vertical (y) velocity.
  void adjustDirection(double angle);

  double angleBetween(const MotionVector &v) const;
  double getAngle(void) const;
  double getAnglePlusMinus180(void) const;
  bool velMissing(void) const;
  void rotate(double angle, bool change_endpts);
  void bias(double b);
  void scale(double s);

  // set speed to this value, which is a rescaling.
  void set(double v);

  double getVx(void) const;
  double getVy(void) const;
  double getSpeed(void) const;
  void reverseHandedness(void);
  void print(void) const;
  void print(FILE *fp) const;
  std::string sprint(void) const;

private:

  double _vx, _vy;
};

# endif

