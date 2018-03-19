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
// Rotate3d.hh
//
// 3-D rotations using vector/matrix operations.
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2012
//
////////////////////////////////////////////////////////////////////

#ifndef Rotate3d_hh
#define Rotate3d_hh

#include <iostream>
#include <euclid/sincos.h>
using namespace std;

class Rotate3d
{

public:

  // 3D vector object

  class Vector {
  public:
    Vector() {
      xx = 0.0;
      yy = 0.0;
      zz = 0.0;
    }
    Vector(double x, double y, double z) {
      xx = x;
      yy = y;
      zz = z;
    }
    double xx, yy, zz;
  };

  // 3D matrix object
  // rows are represented as vectors

  class Matrix {
  public:
    Matrix() {
      row1 = Vector(0.0, 0.0, 0.0);
      row2 = Vector(0.0, 0.0, 0.0);
      row3 = Vector(0.0, 0.0, 0.0);
    }
    Matrix(const Rotate3d::Vector &v1,
           const Rotate3d::Vector &v2,
           const Rotate3d::Vector &v3) {
      row1 = v1;
      row2 = v2;
      row3 = v3;
    }
    Matrix(double x1, double y1, double z1,
           double x2, double y2, double z2,
           double x3, double y3, double z3) {
      row1 = Vector(x1, y1, z1);
      row2 = Vector(x2, y2, z2);
      row3 = Vector(x3, y3, z3);
    }
    Rotate3d::Vector row1, row2, row3;
  };
    
  /// DEG/RAD conversions
  
  static const double Rad2Deg;
  static const double Deg2Rad;

  // vector norm (len)

  static double norm(const Vector &vec);

  // return normalized a vector - so its norm is 1.0
  
  static Vector normalize(const Vector &vec);
  
  // vector dot product

  static double dot(const Vector &vecL, const Vector &vecR);

  // vector cross product

  static Vector cross(const Vector &vecL, const Vector &vecR);

  // multiply vector by matrix
  // vecL on left, matR on right
  // vecL is row vector
  // returns row vector
  
  static Vector mult(const Vector &vecL, const Matrix &matR);
  
  // multiply matrix by vector
  // matL on left, vecR on right
  // vecR is column vector
  // returns column vector
  
  static Vector mult(const Matrix &matL, const Vector &vecR);

  // multiply matrix by matrix
  // matL on left, matR on right
  
  static Matrix mult(const Matrix &matL, const Matrix &matR);

  // Create matrices for rotation around X, Y and Z axes

  static Matrix createRotRadAboutX(double angleRad);
  static Matrix createRotRadAboutY(double angleRad);
  static Matrix createRotRadAboutZ(double angleRad);

  static Matrix createRotDegAboutX(double angleDeg);
  static Matrix createRotDegAboutY(double angleDeg);
  static Matrix createRotDegAboutZ(double angleDeg);

  // printing

  static void print(const Vector &vec, ostream &out);
  static string vector2Str(const Vector &vec);

  static void printElAz(const Vector &vec, ostream &out);
  static string vector2ElAzStr(const Vector &vec);

  static void print(const Matrix &mat, ostream &out);
  static string matrix2Str(const Matrix &mat);

};

#endif











